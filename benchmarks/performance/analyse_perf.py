#!/usr/bin/env python3
"""
analyze.py — Automated hotspot analysis of perf.script output.

Usage:
    python3 profiling/perf/analyze.py
    python3 profiling/perf/analyze.py --input profiling/perf/results/perf.script
    python3 profiling/perf/analyze.py --top 30 --no-plot
"""

import argparse
import re
import sys
from collections import Counter
from pathlib import Path

import matplotlib.pyplot as plt
import numpy as np

# ---------------------------------------------------------------------------
# Subsystem classification — patterns matched against raw symbol strings.
# Order matters: first match wins.
# ---------------------------------------------------------------------------
SUBSYSTEMS = [
    ("Integration",      r"integrat|PhysicsWorld|physicsWorld"),
    ("Force/Contact",    r"computeAcceleration|computeContactForce|applyGravity|applyForce|solveContact"),
    ("Collision",        r"solveCollision|reboundCollision|broadPhase|narrowPhase|detectCollision|Collision"),
    ("Math primitives",  r"Vector3D|Matrix3|Quaternion|dotProduct|crossProduct|normaliz|magnitud|applyVector|operator[\+\-\*\/]"),
    ("Memory / STL",     r"operator new|malloc|_M_realloc|_M_insert|push_back|emplace|std::vector|allocat"),
    ("I/O / Config",     r"loadFromFile|yaml|YAML|ofstream|ifstream|basic_string|decode|loadConfig"),
    ("Other / unknown",  r""),   # catch-all — always last
]

SUBSYSTEM_COLORS = {
    "Integration":      "#E24B4A",
    "Force/Contact":    "#FF8C42",
    "Collision":        "#F5C518",
    "Math primitives":  "#378ADD",
    "Memory / STL":     "#9B59B6",
    "I/O / Config":     "#888888",
    "Other / unknown":  "#CCCCCC",
}

# ---------------------------------------------------------------------------
# Parsing
# ---------------------------------------------------------------------------
# Header line: "Bouncing  605032 13665.621684:     500500 cpu_atom/cycles/Pu: "
_HEADER_RE = re.compile(r"^\S+\s+\d+\s+[\d.]+:")
# Frame line: "\t    deadbeef symbol_name+0xNN (inlined)" or "(...path...)"
_FRAME_RE = re.compile(r"^\s+[0-9a-f]+ (.+)")
# Extract symbol name: strip "+0x..." and " (..." suffix
_SYMBOL_STRIP_RE = re.compile(r"\+0x[0-9a-f]+.*$")


def _clean_symbol(raw: str) -> str:
    """Return the function name from a raw frame symbol string."""
    name = _SYMBOL_STRIP_RE.sub("", raw).strip()
    # Drop the trailing binary path annotation e.g. " (/usr/lib/libc.so.6)"
    name = re.sub(r"\s+\(.*\)$", "", name).strip()
    return name or "[unknown]"


def classify(symbol: str) -> str:
    for subsystem, pattern in SUBSYSTEMS[:-1]:
        if re.search(pattern, symbol):
            return subsystem
    return "Other / unknown"


def parse_script(path: Path) -> tuple[list[str], list[list[str]]]:
    """
    Parse perf.script into (leaf_symbols, stacks).

    Returns:
        leaf_symbols: list of the first (innermost) frame per sample
        stacks:       list of full frame lists per sample (leaf first)
    """
    leaf_symbols: list[str] = []
    stacks: list[list[str]] = []

    current_frames: list[str] = []
    in_sample = False

    with path.open(encoding="utf-8", errors="replace") as fh:
        for line in fh:
            if _HEADER_RE.match(line):
                if current_frames:
                    leaf_symbols.append(current_frames[0])
                    stacks.append(current_frames)
                current_frames = []
                in_sample = True
                continue

            if in_sample:
                m = _FRAME_RE.match(line)
                if m:
                    current_frames.append(_clean_symbol(m.group(1)))
                elif line.strip() == "" and current_frames:
                    leaf_symbols.append(current_frames[0])
                    stacks.append(current_frames)
                    current_frames = []
                    in_sample = False

    if current_frames:
        leaf_symbols.append(current_frames[0])
        stacks.append(current_frames)

    return leaf_symbols, stacks


# ---------------------------------------------------------------------------
# Analysis
# ---------------------------------------------------------------------------

def self_time_table(leaf_symbols: list[str], top_n: int) -> list[tuple[str, int, float]]:
    """Return [(symbol, count, pct), ...] sorted by count descending."""
    total = len(leaf_symbols)
    if total == 0:
        return []
    counts = Counter(leaf_symbols)
    return [(sym, cnt, 100.0 * cnt / total) for sym, cnt in counts.most_common(top_n)]


def subsystem_breakdown(leaf_symbols: list[str]) -> dict[str, float]:
    total = len(leaf_symbols)
    if total == 0:
        return {}
    counts: Counter[str] = Counter()
    for sym in leaf_symbols:
        counts[classify(sym)] += 1
    return {sub: 100.0 * counts[sub] / total for sub, _ in SUBSYSTEMS}


def red_flags(table: list[tuple[str, int, float]], leaf_symbols: list[str]) -> list[str]:
    warnings: list[str] = []
    total = len(leaf_symbols)

    # Memory allocation in hot path
    alloc_pct = sum(pct for sym, _, pct in table[:50] if re.search(r"operator new|malloc|_M_realloc", sym))
    if alloc_pct > 0.5:
        warnings.append(
            f"MEMORY ALLOCATION in hot path: {alloc_pct:.1f}% of samples hit operator new / malloc.\n"
            "  → Pre-allocate with reserve() or use object pools."
        )

    # Unknown frames
    unk_count = sum(1 for s in leaf_symbols if s == "[unknown]")
    unk_pct = 100.0 * unk_count / total if total else 0
    if unk_pct > 10:
        warnings.append(
            f"HIGH [unknown] RATE: {unk_pct:.1f}% of leaf frames have no symbol.\n"
            "  → Verify the binary was built with -g. Consider --call-graph fp instead of dwarf."
        )

    # Single dominant bottleneck
    if table and table[0][2] > 40:
        warnings.append(
            f"SINGLE BOTTLENECK: '{table[0][0]}' accounts for {table[0][2]:.1f}% of all samples.\n"
            "  → Focus optimization effort here before looking elsewhere."
        )

    return warnings


# ---------------------------------------------------------------------------
# Plotting
# ---------------------------------------------------------------------------

def plot_hotspots(table: list[tuple[str, int, float]], output_path: Path) -> None:
    if not table:
        return

    symbols = [sym for sym, _, _ in table]
    pcts    = [pct for _, _, pct in table]
    colors  = [SUBSYSTEM_COLORS[classify(sym)] for sym in symbols]

    # Shorten long C++ names for display
    def shorten(name: str, max_len: int = 55) -> str:
        return name if len(name) <= max_len else name[:max_len - 3] + "..."

    labels = [shorten(s) for s in symbols]

    fig, ax = plt.subplots(figsize=(14, max(5, len(table) * 0.42)))
    y_pos = np.arange(len(table))
    bars = ax.barh(y_pos, pcts, color=colors, edgecolor="white", linewidth=0.5)

    # Value labels
    for bar, pct in zip(bars, pcts):
        ax.text(bar.get_width() + 0.1, bar.get_y() + bar.get_height() / 2,
                f"{pct:.1f}%", va="center", fontsize=8)

    ax.set_yticks(y_pos)
    ax.set_yticklabels(labels, fontsize=8)
    ax.invert_yaxis()
    ax.set_xlabel("Self-time  (%  of total samples)")
    ax.set_title("Hotspot functions — self-time", fontsize=12)
    ax.spines["top"].set_visible(False)
    ax.spines["right"].set_visible(False)

    # Legend
    seen: set[str] = set()
    legend_patches = []
    import matplotlib.patches as mpatches
    for sym in symbols:
        sub = classify(sym)
        if sub not in seen:
            seen.add(sub)
            legend_patches.append(mpatches.Patch(color=SUBSYSTEM_COLORS[sub], label=sub))
    ax.legend(handles=legend_patches, loc="lower right", fontsize=8, framealpha=0.7)

    plt.tight_layout()
    plt.savefig(output_path, dpi=150, bbox_inches="tight")
    plt.close()


# ---------------------------------------------------------------------------
# Report printing
# ---------------------------------------------------------------------------

def print_report(
    table: list[tuple[str, int, float]],
    breakdown: dict[str, float],
    flags: list[str],
    total_samples: int,
    input_path: Path,
    plot_path: Path | None,
) -> None:
    sep = "─" * 72

    print(f"\n{sep}")
    print(f"  perf hotspot analysis   {input_path.name}   ({total_samples:,} samples)")
    print(sep)

    # ── Top-N self-time ──────────────────────────────────────────────────
    print(f"\n{'Rank':<5} {'Self%':>6}  {'Subsystem':<20}  Function")
    print("─" * 72)
    for rank, (sym, _cnt, pct) in enumerate(table, 1):
        sub = classify(sym)
        display = sym if len(sym) <= 48 else sym[:45] + "..."
        print(f"{rank:<5} {pct:>5.1f}%  {sub:<20}  {display}")

    # ── Subsystem breakdown ──────────────────────────────────────────────
    print(f"\n{sep}")
    print("  Subsystem breakdown")
    print(sep)
    for sub, pct in breakdown.items():
        if pct > 0:
            bar = "█" * int(pct / 2)
            print(f"  {sub:<22} {pct:>5.1f}%  {bar}")

    # ── Red flags ────────────────────────────────────────────────────────
    if flags:
        print(f"\n{sep}")
        print("  ⚠  Red flags")
        print(sep)
        for flag in flags:
            for line in flag.splitlines():
                print(f"  {line}")
            print()
    else:
        print("\n  ✓  No red flags detected.")

    # ── Output files ─────────────────────────────────────────────────────
    if plot_path:
        print(f"\n  Hotspot chart saved → {plot_path}")
    print()


# ---------------------------------------------------------------------------
# Entry point
# ---------------------------------------------------------------------------

def main() -> None:
    repo_root = Path(__file__).resolve().parent.parent.parent
    default_input  = repo_root / "profiling/perf/results/perf.script"
    default_output = repo_root / "profiling/perf/results/hotspots.png"

    parser = argparse.ArgumentParser(description="Analyze perf.script and report hotspots.")
    parser.add_argument("--input",   default=str(default_input),  help="Path to perf.script")
    parser.add_argument("--output",  default=str(default_output), help="Path to save hotspots.png")
    parser.add_argument("--top",     type=int, default=20,        help="Number of top functions to show")
    parser.add_argument("--no-plot", action="store_true",         help="Skip generating the bar chart")
    args = parser.parse_args()

    input_path  = Path(args.input)
    output_path = Path(args.output) if not args.no_plot else None

    if not input_path.exists():
        print(f"Error: {input_path} not found.\n"
              "Run 'perf script -i results/perf.data > results/perf.script' first,\n"
              "or use ./profiling/perf/profile.sh to record and convert in one step.",
              file=sys.stderr)
        sys.exit(1)

    print(f"Parsing {input_path} …", end=" ", flush=True)
    leaf_symbols, _stacks = parse_script(input_path)
    print(f"{len(leaf_symbols):,} samples read.")

    if not leaf_symbols:
        print("No samples found. The script file may be empty or in an unexpected format.")
        sys.exit(1)

    table     = self_time_table(leaf_symbols, args.top)
    breakdown = subsystem_breakdown(leaf_symbols)
    flags     = red_flags(table, leaf_symbols)

    if output_path:
        output_path.parent.mkdir(parents=True, exist_ok=True)
        plot_hotspots(table, output_path)

    print_report(table, breakdown, flags, len(leaf_symbols), input_path, output_path)


if __name__ == "__main__":
    main()
