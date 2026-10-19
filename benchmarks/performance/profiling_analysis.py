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
# Output: profiling_report/fig_*.png and profiling_report/profiling_report.pdf
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


def parse_lprof_alloc_caller(path: Path) -> str:
    """Function that calls operator new, from Lprof's caller chains (functions.txt)."""
    if not path.exists():
        return ''
    m = re.search(r'operator new\(unsigned long\) \([^)]*\) <-- ([\w:~<>]+\([^)]*\))', path.read_text())
    return m.group(1) if m else ''


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

    # Every Performance invocation integrates (n_warmup + n_runs + 1 step-timing run) × dur/dt steps.
    cmd = d['run'].get('command', '')
    def arg(name, default):
        m = re.search(rf'--{name}\s+(\S+)', cmd)
        return float(m.group(1)) if m else default
    runs = arg('n_warmup', 1) + arg('n_runs', 5) + 1
    steps = runs * int(arg('dur', 22.0) / arg('dt', 1e-3))

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
            'total': total, 'steps': int(steps), 'leaks': len(d.get('leaks', [])),
            'times': times, 'requested': req, 'n_objects': int(arg('n_objects', 1))}


# =============================================================================
# Figures - each returns (figure, title, how-to-read, finding) or None
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

    finding = ''
    if len(have) == 2:
        s, o = stat['scalar'], stat['optimised']
        finding = (f"The optimised build runs {s['elapsed_s'] / o['elapsed_s']:.1f}x faster. It executes "
                   f"{s['instructions'] / o['instructions']:.1f}x fewer instructions, at IPC "
                   f"{o['ipc']:.2f} vs {s['ipc']:.2f}. Fewer instructions is the main gain; "
                   f"MAQAO (below) tells whether they are vector instructions.")
    how = ('Time is the mean of 5 repeated executions (perf stat -r 5). IPC above ~3 means the pipeline '
           'is well fed; it says nothing about how much work each instruction does.')
    return fig, 'perf - time, instructions and IPC', how, finding


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
    mpki = {}
    for ax, (title, ev) in zip(axes, events):
        vals = [1000 * stat[p].get(ev, 0) / stat[p]['instructions'] for p in have]
        mpki[ev] = dict(zip(have, vals))
        bars = ax.bar(have, vals, width=0.55, color=[PROFILE_COLOR[p] for p in have])
        _bar_labels(ax, bars, lambda v: f'{v:.2g}')
        ax.set_title(f'{title}\nper 1000 instructions', loc='left')
        ax.set_ylim(0, max(max(vals) * 1.25, 1e-4))
        ax.grid(axis='x', visible=False)
    fig.tight_layout()
    worst = max(max(d.values()) for d in mpki.values())
    finding = ('Every value is far below 1 miss per 1000 instructions: at this problem size the code is '
               'not limited by caches, RAM or branch prediction - the data fits in L1. Data-layout work '
               '(SoA) should therefore be measured at larger N, where it can matter.'
               if worst < 1 else
               'At least one counter exceeds 1 miss per 1000 instructions: LLC misses point to RAM '
               'traffic and data layout, branch mispredictions to unpredictable collision tests.')
    how = ('Misses per 1000 instructions (MPKI), from exact grouped counters. MPKI is preferred over a '
           'miss *rate*: a high rate on a handful of accesses costs nothing. Rough guide: LLC MPKI > 1 '
           'or branch MPKI > 5 deserve attention.')
    return fig, 'perf - cache and branch misses', how, finding


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
    ax.legend(ncol=4, loc='upper center', bbox_to_anchor=(0.5, -0.45 if len(have) > 1 else -0.9),
              frameon=False)
    fig.tight_layout()
    o = td.get('optimised') or td[have[-1]]
    main = max((k for k, _, _ in TOPDOWN if k != 'retiring'), key=lambda k: o.get(k, 0))
    names = {k: lab for k, lab, _ in TOPDOWN}
    finding = (f"{'Optimised' if 'optimised' in td else have[-1]}: {o.get('retiring', 0):.0f}% of slots do "
               f"useful work; the main loss is {names[main]} ({o.get(main, 0):.0f}%). "
               + ('Front-End Bound means instruction fetch/decode cannot keep up - typically many small '
                  'calls or branchy code, not memory.' if main == 'frontend_bound' else
                  'Back-End Bound means waiting on execution units or memory.' if main == 'backend_bound'
                  else 'Bad Speculation means mispredicted branches waste work.'))
    how = ('Top-Down sorts every pipeline slot into four buckets. Retiring is useful work; the other '
           'three are losses and say where to look: Front-End (code), Back-End (memory/compute), '
           'Bad Speculation (branches).')
    return fig, 'perf - Top-Down bottleneck classification', how, finding


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
    o = reports.get('optimised') or reports[have[-1]]
    alloc = sum(v for n, v in o if re.match(r'(operator new|operator delete|malloc|cfree|free|_int_)', n))
    finding = (f"Optimised build: the top functions are {short_symbol(o[0][0], 60)} ({o[0][1]:.0f}%), "
               f"{short_symbol(o[1][0], 60)} ({o[1][1]:.0f}%) and {short_symbol(o[2][0], 60)} "
               f"({o[2][1]:.0f}%)." if len(o) >= 3
               else f"Hottest function: {short_symbol(o[0][0], 70)} ({o[0][1]:.0f}%).")
    getters = [n for n, _ in o[:10] if re.search(r'::(get|is)\w*\(\)', n)]
    if getters:
        finding += (f" Trivial accessors such as {short_symbol(getters[0], 40)} appear as separate "
                    f"functions: they are defined in .cpp files, so they cannot be inlined into other "
                    f"files without link-time optimisation (-flto).")
    if alloc >= 3:
        finding += (f" Heap management (new/delete/malloc/free) takes {alloc:.0f}% of the time - "
                    f"see the MALT figures for where these allocations come from.")
    how = ('perf record samples the call stack ~1000 times per second; a function\'s share of samples '
           'is its share of run time (self time, callees excluded). Statistical: small shares are noisy.')
    return fig, 'perf - hottest functions', how, finding


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
    hot = [loops.get(r['id'], {}) for r in rows]
    vectorised = sum(1 for c in hot if c.get('status') and not re.search(r'not', c['status']))
    finding = (f"{vectorised} of the {len(rows)} hottest loops of the optimised build are vectorised. "
               f"CQA estimates up to {max(speed):.1f}x per loop if fully vectorised. Its standard "
               f"workarounds for such loops include switching from arrays of structures (AoS, the "
               f"current Vector3D-per-object layout) to structures of arrays (SoA).")
    how = ('Lprof (left) measures where time goes, per source loop. CQA (right) reads the assembly of '
           'the same loop and estimates the gain if it used the full vector width. Prioritise loops '
           'that are both hot and have a large potential speedup.')
    return fig, 'MAQAO - hottest loops and vectorisation potential (optimised build)', how, finding


def fig_malt(malt, alloc_caller=''):
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

    per_step = malt['total'] / malt['steps'] if malt['steps'] else 0
    top_size, top_count = malt['sizes'][0]
    finding = (f"{malt['total']:,} heap allocations for {malt['steps']:,} integration steps = "
               f"{per_step:.1f} per step ({malt['n_objects']} objects). The dominant size is {top_size} B "
               f"({100 * top_count / malt['total']:.0f}% of all allocations). "
               + (f"Lprof attributes the operator new calls to {short_symbol(alloc_caller, 60)}. "
                  if alloc_caller else '')
               + f"A steady-state time step should allocate nothing: these temporaries are a direct "
               f"optimisation target (reuse buffers across steps). Blocks still allocated at exit: "
               f"{malt['leaks']} (typically static objects).")
    how = ('MALT intercepts every malloc/new, so counts are exact. Repeated allocations of one size '
           'inside the time loop are the pattern to remove (reuse buffers, or a pool allocator).')
    return fig, 'MALT - where and how often the code allocates', how, finding


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
    first, last, peak = malt['requested'][0], malt['requested'][-1], max(malt['requested'])
    flat = peak <= 1.2 * max(first, last, 1)
    finding = (f"Peak heap in use: {peak / 1024:.0f} KiB. "
               + ('The curve stays flat during the run: allocations are freed as fast as they are made '
                  '(no growth) - the cost is the allocation churn, not the footprint.'
                  if flat else 'Memory grows during the run: look for containers that keep growing.'))
    how = 'Bytes requested from the heap and still allocated, sampled over the run.'
    return fig, 'MALT - heap memory over time', how, finding


# =============================================================================
# Report
# =============================================================================
def text_page(pdf, title, paragraphs):
    """Headed paragraphs, continued on as many pages as needed."""
    def new_page(heading):
        page = plt.figure(figsize=(11, 8.5))
        page.text(0.06, 0.93, heading, fontsize=18, fontweight='bold', color=TEXT)
        return page, 0.86

    fig, y = new_page(title)
    for heading, body in paragraphs:
        lines = textwrap.wrap(body, 125)
        if y - 0.035 - 0.028 * len(lines) < 0.05:
            pdf.savefig(fig)
            plt.close(fig)
            fig, y = new_page(f'{title} (continued)')
        fig.text(0.06, y, heading, fontsize=11.5, fontweight='bold', color=TEXT)
        y -= 0.035
        for line in lines:
            fig.text(0.06, y, line, fontsize=10, color=TEXT_2)
            y -= 0.028
        y -= 0.02
    pdf.savefig(fig)
    plt.close(fig)


def figure_page(pdf, fig, title, how, finding):
    w, h = fig.get_size_inches()
    extra = 1.9
    fig.set_size_inches(w, h + extra)
    fig.subplots_adjust(top=1 - 0.55 / (h + extra), bottom=extra / (h + extra) + 0.06 * h / (h + extra))
    fig.suptitle(title, x=0.02, ha='left', fontsize=13, fontweight='bold', color=TEXT)
    y = (extra - 0.35) / (h + extra)
    for heading, body in (('How to read', how), ('What it shows', finding)):
        fig.text(0.02, y, heading, fontsize=9.5, fontweight='bold', color=TEXT)
        y -= 0.2 / (h + extra)
        for line in textwrap.wrap(body, 140):
            fig.text(0.02, y, line, fontsize=9, color=TEXT_2)
            y -= 0.18 / (h + extra)
        y -= 0.12 / (h + extra)
    pdf.savefig(fig)


def main() -> None:
    OUT.mkdir(parents=True, exist_ok=True)
    stat     = {p: parse_perf_stat(PERF / f'perf_stat_{p}.txt') for p in PROFILES}
    topdown  = {p: parse_topdown(PERF / f'perf_topdown_{p}.txt') for p in PROFILES}
    reports  = {p: parse_perf_report(PERF / f'perf_report_{p}.txt') for p in PROFILES}
    cqa      = {p: parse_cqa(MAQAO / f'cqa_{p}' / 'cqa.txt') for p in PROFILES}
    lprof    = {p: parse_lprof_loops(MAQAO / f'lprof_{p}' / 'loops.txt') for p in PROFILES}
    malt     = parse_malt(MALT / 'malt_optimised') or parse_malt(MALT / 'malt_scalar')
    alloc_caller = parse_lprof_alloc_caller(MAQAO / 'lprof_optimised' / 'functions.txt')

    builders = [
        ('fig_perf_overview',  lambda: fig_perf_overview(stat)),
        ('fig_perf_topdown',   lambda: fig_perf_topdown(topdown)),
        ('fig_perf_rates',     lambda: fig_perf_rates(stat)),
        ('fig_perf_hotspots',  lambda: fig_perf_hotspots(reports)),
        ('fig_maqao_loops',    lambda: fig_maqao_loops(cqa, lprof)),
        ('fig_malt_sites',     lambda: fig_malt(malt, alloc_caller)),
        ('fig_malt_timeline',  lambda: fig_malt_timeline(malt)),
    ]
    pages = []
    for name, build in builders:
        result = build()
        if result is None:
            print(f'  [skip] {name} - input files not found (run the matching profile_*.sh first)')
            continue
        fig, title, how, finding = result
        fig.savefig(OUT / f'{name}.png', dpi=150, bbox_inches='tight')
        print(f'  -> {OUT / name}.png')
        pages.append((fig, title, how, finding))

    if not pages:
        raise SystemExit('No profiling results found. Run ./scripts/profile.sh first.')

    vec = {p: sum(1 for l in cqa[p].values() if l['status'] and 'not' not in l['status'])
           for p in PROFILES if cqa[p]}
    summary = [(title, finding) for _, title, _, finding in pages if finding]
    if vec:
        summary.append(('MAQAO - sanity check of the build profiles',
                        ', '.join(f'{p}: {n} of {len(cqa[p])} analysed loops vectorised'
                                  for p, n in vec.items())
                        + '. The scalar build must show 0; otherwise its flags are not applied.'))
    pdf_path = OUT / 'profiling_report.pdf'
    with PdfPages(pdf_path) as pdf:
        text_page(pdf, 'Profiling report - key findings', summary)
        for fig, title, how, finding in pages:
            figure_page(pdf, fig, title, how, finding)
            plt.close(fig)
    print(f'\nPDF report -> {pdf_path}')
    for title, finding in summary:
        print(f'\n{title}\n  ' + '\n  '.join(textwrap.wrap(finding, 100)))


if __name__ == '__main__':
    main()
