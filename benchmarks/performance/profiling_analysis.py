# Profiling Analysis - turns the perf / MAQAO / MALT outputs into figures and a PDF report.
#
# Data sources (each section is skipped if its files are missing):
#   perf/results/perf_stat_<profile>.txt      hardware counters      (perf/profile_perf.sh)
#   perf/results/perf_topdown_<profile>.txt   Top-Down breakdown
#   perf/results/perf_report_<profile>.txt    hottest functions
#   maqao/results/cqa_<profile>/cqa.txt       per-loop vectorisation  (maqao/profile_maqao.sh)
#   maqao/results/lprof_<profile>/loops.txt   per-loop time share
#   malt/results/malt_<profile>/*.json        heap allocations        (malt/profile_malt.sh)
#
# Output: profiling_report/fig_*.png and profiling_report/profiling_report.pdf (figures + how-to-read notes)
#
# Usage: python3 benchmarks/performance/profiling_analysis.py   (any working directory)

import json
import re
import textwrap
from pathlib import Path

import matplotlib

matplotlib.use('Agg')
import matplotlib.pyplot as plt
from matplotlib.backends.backend_pdf import PdfPages

REPO     = Path(__file__).resolve().parent
PERF     = REPO / 'perf' / 'results'
MAQAO    = REPO / 'maqao' / 'results'
MALT     = REPO / 'malt' / 'results'
OUT      = REPO / 'profiling_report'
PROFILES = ['scalar', 'optimised']

# Colours: validated categorical palette (light surface). Profiles always keep the same hue.
SURFACE   = '#fcfcfb'
TEXT      = '#0b0b0b'
TEXT_2    = '#52514e'
GRID      = '#e4e3df'
PROFILE_COLOR = {'scalar': '#eb6834', 'optimised': '#2a78d6'}
TOPDOWN = [  # (perf metric suffix, label, colour) in stacking order
    ('retiring',        'Retiring (useful work)', '#2a78d6'),
    ('frontend_bound',  'Front-End Bound',        '#eb6834'),
    ('backend_bound',   'Back-End Bound',         '#1baf7a'),
    ('bad_speculation', 'Bad Speculation',        '#eda100'),
]

plt.rcParams.update({
    'figure.facecolor': SURFACE, 'axes.facecolor': SURFACE, 'savefig.facecolor': SURFACE,
    'text.color': TEXT, 'axes.labelcolor': TEXT_2, 'xtick.color': TEXT_2, 'ytick.color': TEXT_2,
    'axes.edgecolor': GRID, 'axes.spines.top': False, 'axes.spines.right': False,
    'axes.grid': True, 'grid.color': GRID, 'grid.linewidth': 0.8, 'axes.axisbelow': True,
    'font.size': 10, 'axes.titlesize': 11, 'axes.titleweight': 'bold', 'figure.dpi': 110,
})


# =============================================================================
# Parsing helpers
# =============================================================================
def _int(text: str) -> int:
    """Counter value printed with any locale's thousands separator ('1 234', '1,234')."""
    return int(re.sub(r'[^\d]', '', text))


def _float(text: str) -> float:
    """Decimal printed with either separator ('1,47' or '1.47')."""
    return float(text.replace(',', '.'))


def _event(token: str) -> str:
    """'cpu_core/cycles/u' or 'cycles:u' -> 'cycles'."""
    token = token.replace('cpu_core/', '')
    return re.sub(r'(/u|/|:u)$', '', token).rstrip('/')


def short_symbol(name: str, width: int = 58) -> str:
    """Readable C++ symbol: drop std::string / const& noise, keep argument types (overloads must
    stay distinguishable), truncate only what is still too long."""
    name = re.sub(r'std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> >',
                  'std::string', name)
    name = re.sub(r'std::allocator<[^<>]*>', '', name).replace(', >', '>')
    name = re.sub(r'\s*const&|\s*const\b(?!\w)|&', '', name).replace('unsigned long', 'size_t')
    return name if len(name) <= width else name[:width - 1] + '…'


def unique_labels(labels: list[str]) -> list[str]:
    """Matplotlib merges bars with identical category labels - suffix duplicates."""
    seen: dict[str, int] = {}
    out = []
    for lab in labels:
        seen[lab] = seen.get(lab, 0) + 1
        out.append(lab if seen[lab] == 1 else f'{lab} [{seen[lab]}]')
    return out


def parse_perf_stat(path: Path) -> dict:
    """Event counts + mean elapsed time of `perf stat -r N` output."""
    out = {}
    if not path.exists():
        return out
    for line in path.read_text().splitlines():
        if 'cpu_atom/' in line or '<not' in line:
            continue
        m = re.match(r'^\s*(\d[\d\s  .,]*\d|\d)\s+(\S+)', line)
        if m and not m.group(2).startswith(('seconds', '+-', '#')):
            out[_event(m.group(2))] = _int(m.group(1))
        m = re.match(r'^\s*([\d.,]+)\s+(?:\+-\s+[\d.,]+\s+)?seconds time elapsed', line)
        if m:
            out['elapsed_s'] = _float(m.group(1))
    return out


def parse_topdown(path: Path) -> dict:
    """Top-Down level-1 percentages (P-core lines only)."""
    out = {}
    if not path.exists():
        return out
    for line in path.read_text().splitlines():
        if 'cpu_atom/' in line:
            continue
        for m in re.finditer(r'#\s*([\d]+[.,]?\d*)\s*%\s*tma_(\w+)', line):
            out[m.group(2)] = _float(m.group(1))
    return out


def parse_perf_report(path: Path, top: int = 12) -> list[tuple[str, float]]:
    """Hottest functions by self time, from the event section with the most samples."""
    if not path.exists():
        return []
    sections, current = [], None
    for line in path.read_text().splitlines():
        m = re.match(r'^# Samples: ([\d.,]+)([KMG]?)', line)
        if m:
            scale = {'': 1, 'K': 1e3, 'M': 1e6, 'G': 1e9}[m.group(2)]
            current = {'samples': _float(m.group(1)) * scale, 'rows': []}
            sections.append(current)
            continue
        m = re.match(r'^\s+([\d.,]+)%\s+\[[.k]\]\s+(.+?)(?:\s{2,}|$)', line)
        if m and current is not None:
            current['rows'].append((m.group(2).strip(), _float(m.group(1))))
    if not sections:
        return []
    best = max(sections, key=lambda s: s['samples'])
    return best['rows'][:top]


def parse_cqa(path: Path) -> dict[int, dict]:
    """Per binary loop: function, source, vectorisation status, register use, potential speedup."""
    loops: dict[int, dict] = {}
    if not path.exists():
        return loops
    func, loop, in_defined = '', None, False
    for line in path.read_text().splitlines():
        m = re.match(r'^Section [\d.]+: Function: (.+)$', line)
        if m:
            func, loop = m.group(1), None
            continue
        m = re.match(r'^Section [\d.]+: Binary loop #(\d+)$', line)
        if m:
            loop = {'function': func, 'source': '', 'status': '', 'register_pct': None,
                    'speedup': None, 'peak_pct': None}
            loops[int(m.group(1))] = loop
            continue
        if loop is None:
            continue
        if line.startswith('The loop is defined in:'):
            in_defined = True
            continue
        if in_defined:
            if line.startswith(' - '):
                if '3DPhysicsEngine/' in line and not loop['source']:
                    loop['source'] = line[3:].split('3DPhysicsEngine/')[-1]
                continue
            in_defined = False
        m = re.match(r'^(\d+)% of peak computational performance', line)
        if m and loop['peak_pct'] is None:
            loop['peak_pct'] = int(m.group(1))
        m = re.match(r'^Your loop is (.+?)\.?$', line)
        if m and not loop['status']:
            loop['status'] = m.group(1)
        m = re.match(r'^Only (\d+)% of vector register length is used', line)
        if m and loop['register_pct'] is None:
            loop['register_pct'] = int(m.group(1))
        m = re.search(r'\(([\d.]+)x speedup\)', line)
        if m and loop['speedup'] is None and 'vectoriz' in line.lower():
            loop['speedup'] = float(m.group(1))
    return loops


def parse_lprof_loops(path: Path) -> list[dict]:
    """Lprof loop table: loop id, function, source, coverage (% of samples)."""
    rows = []
    if not path.exists():
        return rows
    for line in path.read_text().splitlines():
        m = re.match(r'^#\s+(\d+)\s+\|', line)
        if not m:
            continue
        cells = [c.strip() for c in line.strip('# ').split('|')]
        if len(cells) >= 6:
            rows.append({'id': int(cells[0]), 'source': cells[2], 'function': cells[3],
                         'coverage': _float(cells[5])})
    return sorted(rows, key=lambda r: -r['coverage'])


ALLOC_FRAMES = re.compile(r'^(operator new|malloc|calloc|realloc|__libc|std::__new_allocator|'
                          r'std::allocator|std::_Vector_base|std::vector<.*>::_M_|__gnu_cxx|\?\?)')


def parse_malt(directory: Path) -> dict:
    """Allocation sites, size histogram, memory timeline and allocations per step."""
    files = sorted(directory.glob('malt-*.json'))
    if not files:
        return {}
    d = json.loads(files[-1].read_text())
    strings, instr = d['sites']['strings'], d['sites']['instr']

    def fname(addr: str) -> str:
        e = instr.get(addr)
        return strings[e['function']] if e else addr

    sites: dict[str, int] = {}
    total = 0
    for st in d['stacks']['stats']:
        count = st['infos']['alloc']['count']
        if not count:
            continue
        total += count
        frames = [fname(a) for a in st['stack']]
        alloc_type = next((re.search(r'__new_allocator<(.+?)>::allocate', f).group(1)
                           for f in frames if re.search(r'__new_allocator<(.+?)>::allocate', f)), '')
        caller = next((f for f in frames if not ALLOC_FRAMES.match(f)), frames[0] if frames else '?')
        # MALT names a frame after the innermost inlined function, so the "caller" of an inlined
        # std::vector allocation is often misreported - label those sites by allocated type.
        label = f'std::vector<{alloc_type}> buffers' if alloc_type else short_symbol(caller, 48)
        sites[label] = sites.get(label, 0) + count

    tl = d['timeline']['memoryTimeline']
    tps = d['globals']['ticksPerSecond']
    req = [v[0] for v in tl['values']]
    times = [i * tl['perPoints'] / tps for i in range(len(req))]

    ranked = sorted(sites.items(), key=lambda kv: -kv[1])
    major = [kv for kv in ranked if kv[1] >= 0.001 * total]
    minor = ranked[len(major):]
    if minor:
        major.append((f'{len(minor)} other sites', sum(c for _, c in minor)))
    return {'sites': major,
            'sizes': sorted(((int(k), v) for k, v in d['memStats']['sizeMap'].items()),
                            key=lambda kv: -kv[1]),
            'times': times, 'requested': req}


# =============================================================================
# Figures - each returns (figure, title, how-to-read) or None
# =============================================================================
def _bar_labels(ax, bars, fmt):
    for b in bars:
        ax.annotate(fmt(b.get_height()), (b.get_x() + b.get_width() / 2, b.get_height()),
                    xytext=(0, 3), textcoords='offset points', ha='center', va='bottom',
                    fontsize=9, color=TEXT)


def fig_perf_overview(stat):
    have = [p for p in PROFILES if stat.get(p, {}).get('cycles')]
    if not have:
        return None
    metrics = [
        ('elapsed_s', 'Time per execution (s)', lambda v: f'{v:.2f}'),
        ('instructions', 'Instructions (billions)', lambda v: f'{v:.1f}'),
        ('ipc', 'IPC (instructions / cycle)', lambda v: f'{v:.2f}'),
    ]
    for p in have:
        stat[p]['ipc'] = stat[p]['instructions'] / stat[p]['cycles']
    fig, axes = plt.subplots(1, 3, figsize=(11, 3.6))
    for ax, (key, title, fmt) in zip(axes, metrics):
        vals = [stat[p][key] / (1e9 if key == 'instructions' else 1) for p in have]
        bars = ax.bar(have, vals, width=0.55, color=[PROFILE_COLOR[p] for p in have])
        _bar_labels(ax, bars, fmt)
        ax.set_title(title, loc='left')
        ax.set_ylim(0, max(vals) * 1.2)
        ax.grid(axis='x', visible=False)
    fig.tight_layout()
    how = 'Time is the mean of 5 repeated executions (perf stat -r 5). Fewer instructions means less work. IPC (instructions per cycle) shows how well the pipeline is fed, up to ~4-5 on this CPU; it says nothing about how much work each instruction does, so it does not show vectorisation.'
    return fig, 'perf - time, instructions and IPC', how


def fig_perf_rates(stat):
    have = [p for p in PROFILES if stat.get(p, {}).get('instructions') and stat[p].get('branches')]
    if not have:
        return None
    events = [  # (title, counter) - shown as misses per 1000 instructions (MPKI)
        ('Branch mispredictions', 'branch-misses'),
        ('L1 data-cache misses', 'L1-dcache-load-misses'),
        ('LLC misses (go to RAM)', 'LLC-load-misses'),
    ]
    fig, axes = plt.subplots(1, 3, figsize=(11, 3.6))
    for ax, (title, ev) in zip(axes, events):
        vals = [1000 * stat[p].get(ev, 0) / stat[p]['instructions'] for p in have]
        bars = ax.bar(have, vals, width=0.55, color=[PROFILE_COLOR[p] for p in have])
        _bar_labels(ax, bars, lambda v: f'{v:.2g}')
        ax.set_title(f'{title}\nper 1000 instructions', loc='left')
        ax.set_ylim(0, max(max(vals) * 1.25, 1e-4))
        ax.grid(axis='x', visible=False)
    fig.tight_layout()
    how = 'Misses per 1000 instructions (MPKI), from exact grouped counters. MPKI is preferred over a miss rate: a high rate on a handful of accesses costs nothing. Rough guide: LLC MPKI above 1 (RAM traffic) or branch MPKI above 5 deserve attention.'
    return fig, 'perf - cache and branch misses', how


def fig_perf_topdown(td):
    have = [p for p in PROFILES if td.get(p)]
    if not have:
        return None
    fig, ax = plt.subplots(figsize=(11, 1.2 + 0.7 * len(have)))
    for row, p in enumerate(have):
        left = 0.0
        for key, label, color in TOPDOWN:
            v = td[p].get(key, 0.0)
            ax.barh(row, v, left=left, height=0.55, color=color,
                    edgecolor=SURFACE, linewidth=2, label=label if row == 0 else None)
            if v >= 4:
                ax.text(left + v / 2, row, f'{v:.0f}%', ha='center', va='center',
                        color='white', fontsize=9, fontweight='bold')
            left += v
    ax.set_yticks(range(len(have)), have)
    ax.set_xlim(0, 100)
    ax.set_xlabel('% of pipeline slots')
    ax.grid(axis='y', visible=False)
    ax.invert_yaxis()
    ax.legend(ncol=4, loc='lower left', bbox_to_anchor=(0, 1.02), frameon=False)
    fig.tight_layout()
    how = 'Every pipeline slot is sorted into four buckets. Retiring is useful work; the others are losses and say where to look: Front-End Bound (instruction fetch/decode: many small calls, code size), Back-End Bound (waiting for data or execution units), Bad Speculation (mispredicted branches).'
    return fig, 'perf - Top-Down bottleneck classification', how


def fig_perf_hotspots(reports):
    have = [p for p in PROFILES if reports.get(p)]
    if not have:
        return None
    fig, axes = plt.subplots(1, len(have), figsize=(11, 4.6), squeeze=False)
    for ax, p in zip(axes[0], have):
        rows = reports[p][:10][::-1]
        bars = ax.barh(unique_labels([short_symbol(n, 48) for n, _ in rows]), [v for _, v in rows],
                       height=0.6, color=PROFILE_COLOR[p])
        for b in bars:
            ax.text(b.get_width(), b.get_y() + b.get_height() / 2, f' {b.get_width():.1f}%',
                    va='center', fontsize=8, color=TEXT)
        ax.set_title(f'{p} - % of samples (self time)', loc='left')
        ax.set_xlim(0, max(v for _, v in rows) * 1.25)
        ax.tick_params(axis='y', labelsize=8)
        ax.grid(axis='y', visible=False)
    fig.tight_layout()
    how = "perf record samples the call stack ~1000 times per second; a function's share of samples is its share of run time (self time, callees excluded). Statistical: small shares are noisy. Trivial getters near the top are calls the compiler could not inline."
    return fig, 'perf - hottest functions', how


def fig_maqao_loops(cqa, lprof):
    rows = [r for r in lprof.get('optimised', []) if r['coverage'] > 0][:10]
    if not rows or not cqa.get('optimised'):
        return None
    loops = cqa['optimised']
    labels, cover, speed = [], [], []
    for r in rows:
        c = loops.get(r['id'], {})
        src = (c.get('source') or r['source']).split(',')[0]
        labels.append(f"#{r['id']} {short_symbol(r['function'], 34)}\n{src}")
        cover.append(r['coverage'])
        speed.append(c.get('speedup') or 1.0)
    fig, (a1, a2) = plt.subplots(1, 2, figsize=(11, 0.9 + 0.55 * len(rows)), sharey=True)
    y = range(len(rows))
    a1.barh(y, cover, height=0.6, color=PROFILE_COLOR['optimised'])
    a2.barh(y, speed, height=0.6, color='#1baf7a')
    for ax, vals, fmt in ((a1, cover, '{:.1f}%'), (a2, speed, '{:.1f}x')):
        for i, v in enumerate(vals):
            ax.text(v, i, ' ' + fmt.format(v), va='center', fontsize=8, color=TEXT)
        ax.set_xlim(0, max(vals) * 1.3)
        ax.grid(axis='y', visible=False)
    a1.set_yticks(list(y), labels, fontsize=7.5)
    a1.invert_yaxis()
    a1.set_title('Share of run time (Lprof)', loc='left')
    a2.set_title('Speedup if fully vectorised (CQA estimate)', loc='left')
    a2.axvline(1, color=TEXT_2, lw=1)
    fig.tight_layout()
    how = "Left: share of run time per source loop (Lprof, measured). Right: CQA's estimate of the speedup if the same loop used the full vector width (static analysis of the assembly). Loops that are both hot and have a large potential speedup are the first targets."
    return fig, 'MAQAO - hottest loops and vectorisation potential (optimised build)', how


def fig_malt(malt):
    if not malt or not malt['sites']:
        return None
    fig, (a1, a2) = plt.subplots(1, 2, figsize=(11, 3.8), gridspec_kw={'width_ratios': [3, 2]})
    sites = malt['sites'][:6][::-1]
    bars = a1.barh([s for s, _ in sites], [c / 1e6 for _, c in sites], height=0.6,
                   color=PROFILE_COLOR['optimised'])
    for b in bars:
        a1.text(b.get_width(), b.get_y() + b.get_height() / 2, f' {b.get_width():.2f} M',
                va='center', fontsize=8, color=TEXT)
    a1.set_title('Allocations per call site (millions)', loc='left')
    a1.set_xlim(0, max(c for _, c in sites) / 1e6 * 1.3)
    a1.tick_params(axis='y', labelsize=8.5)
    a1.grid(axis='y', visible=False)

    sizes = malt['sizes'][:6]
    bars = a2.bar([f'{s} B' for s, _ in sizes], [c for _, c in sizes], width=0.6, color='#1baf7a')
    a2.set_yscale('log')
    a2.set_title('Allocations per size (log scale)', loc='left')
    a2.tick_params(axis='x', labelsize=8, rotation=30)
    a2.grid(axis='x', visible=False)
    fig.tight_layout()
    how = 'MALT intercepts every malloc/new, so counts are exact. Left: allocations per call site; right: allocations per size (log scale). Allocations repeated at every time step are the pattern to remove (reuse the buffer, or a pool allocator).'
    return fig, 'MALT - where and how often the code allocates', how


def fig_malt_timeline(malt):
    if not malt or not malt['times']:
        return None
    fig, ax = plt.subplots(figsize=(11, 3.2))
    ax.plot(malt['times'], [r / 1024 for r in malt['requested']], color=PROFILE_COLOR['optimised'], lw=2)
    ax.set_xlabel('time (s)')
    ax.set_ylabel('heap in use (KiB)')
    ax.set_ylim(0, max(malt['requested']) / 1024 * 1.2)
    ax.set_title('Heap memory in use over the run', loc='left')
    fig.tight_layout()
    how = 'Bytes requested from the heap and still allocated, sampled over the run. A flat curve means no growth; a rising curve means something accumulates.'
    return fig, 'MALT - heap memory over time', how

# =============================================================================
# Report
# =============================================================================
def pdf_page(pdf, png: Path, title: str, how: str) -> None:
    """One PDF page: title, the figure's PNG scaled to the page width (so it can never overflow
    or overlap the text), and a generic "How to read" note below it."""
    img = plt.imread(str(png))
    page_w, margin = 11.0, 0.35
    img_w = page_w - 2 * margin
    img_h = img_w * img.shape[0] / img.shape[1]
    lines = textwrap.wrap(how, 135)
    title_h, note_h = 0.6, 0.35 + 0.2 * len(lines)
    page_h = title_h + img_h + note_h + margin
    page = plt.figure(figsize=(page_w, page_h))
    page.text(margin / page_w, 1 - 0.4 / page_h, title, fontsize=13, fontweight='bold', color=TEXT)
    ax = page.add_axes([margin / page_w, (note_h + margin) / page_h, img_w / page_w, img_h / page_h])
    ax.imshow(img)
    ax.axis('off')
    y = note_h + margin - 0.25
    page.text(margin / page_w, y / page_h, 'How to read', fontsize=9.5, fontweight='bold', color=TEXT)
    for line in lines:
        y -= 0.2
        page.text(margin / page_w, y / page_h, line, fontsize=9, color=TEXT_2)
    pdf.savefig(page)
    plt.close(page)


def main() -> None:
    OUT.mkdir(parents=True, exist_ok=True)
    stat     = {p: parse_perf_stat(PERF / f'perf_stat_{p}.txt') for p in PROFILES}
    topdown  = {p: parse_topdown(PERF / f'perf_topdown_{p}.txt') for p in PROFILES}
    reports  = {p: parse_perf_report(PERF / f'perf_report_{p}.txt') for p in PROFILES}
    cqa      = {p: parse_cqa(MAQAO / f'cqa_{p}' / 'cqa.txt') for p in PROFILES}
    lprof    = {p: parse_lprof_loops(MAQAO / f'lprof_{p}' / 'loops.txt') for p in PROFILES}
    malt     = parse_malt(MALT / 'malt_optimised') or parse_malt(MALT / 'malt_scalar')

    builders = [
        ('fig_perf_overview',  lambda: fig_perf_overview(stat)),
        ('fig_perf_topdown',   lambda: fig_perf_topdown(topdown)),
        ('fig_perf_rates',     lambda: fig_perf_rates(stat)),
        ('fig_perf_hotspots',  lambda: fig_perf_hotspots(reports)),
        ('fig_maqao_loops',    lambda: fig_maqao_loops(cqa, lprof)),
        ('fig_malt_sites',     lambda: fig_malt(malt)),
        ('fig_malt_timeline',  lambda: fig_malt_timeline(malt)),
    ]
    pdf_path = OUT / 'profiling_report.pdf'
    written = 0
    with PdfPages(pdf_path) as pdf:
        for name, build in builders:
            result = build()
            if result is None:
                print(f'  [skip] {name} - input files not found (run the matching profile_*.sh first)')
                continue
            fig, title, how = result
            fig.savefig(OUT / f'{name}.png', dpi=150, bbox_inches='tight')
            plt.close(fig)
            pdf_page(pdf, OUT / f'{name}.png', title, how)
            written += 1
            print(f'  -> {OUT / name}.png')

    if not written:
        pdf_path.unlink(missing_ok=True)
        raise SystemExit('No profiling results found. Run ./scripts/profile.sh first.')
    print(f'\nPDF report -> {pdf_path}')


if __name__ == '__main__':
    main()
