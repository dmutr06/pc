#!/usr/bin/env python3
"""
Benchmark runner — calls the C++ benchmark binary with different
matrix sizes and thread counts, collects results, generates graphs.
"""

import subprocess
import sys
from pathlib import Path
from collections import defaultdict

try:
    import matplotlib
    matplotlib.use("Agg")
    import matplotlib.pyplot as plt
except ImportError:
    print("matplotlib is required. Install it with:  pip install matplotlib")
    sys.exit(1)

SCRIPT_DIR = Path(__file__).resolve().parent
BENCH_BIN = SCRIPT_DIR / "build" / "benchmark"
OUTPUT_DIR = SCRIPT_DIR / "benchmark_results"

MATRIX_SIZES = [100, 500, 1000, 2000, 5000, 10000, 15000, 20000]
THREAD_COUNTS = [5, 10, 20, 40, 80, 160]


def build():
    print("⚙  Building benchmark…")
    r = subprocess.run(["make", "build/benchmark"], cwd=SCRIPT_DIR,
                       capture_output=True, text=True)
    if r.returncode != 0:
        print("❌ Build failed:\n", r.stderr)
        sys.exit(1)
    print("✅ Build successful.\n")


def run_one(size: int, threads: int) -> tuple[float, float, float]:
    """Run benchmark binary and return (single_ms, multi_ms, speedup)."""
    r = subprocess.run(
        [str(BENCH_BIN), str(size), str(threads)],
        capture_output=True, text=True,
    )
    if r.returncode != 0:
        print(f"❌ Failed for size={size} threads={threads}: {r.stderr}")
        sys.exit(1)

    parts = r.stdout.strip().split()
    return float(parts[0]), float(parts[1]), float(parts[2])


def run_benchmark() -> list[dict]:
    header = (
        f"{'Matrix Size':>12} | {'Threads':>7} | "
        f"{'Single (ms)':>14} | {'Multi (ms)':>14} | {'Speedup':>7}"
    )
    print(header)
    print("-" * len(header))

    rows: list[dict] = []

    for size in MATRIX_SIZES:
        for threads in THREAD_COUNTS:
            single_ms, multi_ms, speedup = run_one(size, threads)

            print(
                f"{size:>12} | {threads:>7} | "
                f"{single_ms:>14.3f} | {multi_ms:>14.3f} | {speedup:>6.2f}x"
            )

            rows.append({
                "size": size,
                "threads": threads,
                "single_ms": single_ms,
                "multi_ms": multi_ms,
                "speedup": speedup,
            })

    return rows


def group_by_size(rows):
    groups = defaultdict(list)
    for r in rows:
        groups[r["size"]].append(r)
    return dict(sorted(groups.items()))


PALETTE = [
    "#6366f1", "#06b6d4", "#f59e0b", "#ef4444",
    "#10b981", "#8b5cf6", "#ec4899", "#14b8a6",
]


def plot_individual(groups):
    OUTPUT_DIR.mkdir(parents=True, exist_ok=True)
    for idx, (size, entries) in enumerate(groups.items()):
        threads = [e["threads"] for e in entries]
        speedups = [e["speedup"] for e in entries]
        colour = PALETTE[idx % len(PALETTE)]

        fig, ax = plt.subplots(figsize=(8, 5))
        fig.patch.set_facecolor("#0f172a")
        ax.set_facecolor("#1e293b")
        ax.plot(threads, speedups, marker="o", markersize=8, linewidth=2.5,
                color=colour, markeredgecolor="white", markeredgewidth=1.2, zorder=3)
        ax.fill_between(threads, speedups, alpha=0.15, color=colour)
        ax.axhline(y=1.0, linestyle="--", linewidth=1, color="#64748b", alpha=0.6)
        ax.set_title(f"Speedup — Matrix {size}×{size}",
                     fontsize=16, fontweight="bold", color="white", pad=14)
        ax.set_xlabel("Number of Threads", fontsize=12, color="#cbd5e1", labelpad=10)
        ax.set_ylabel("Speedup (vs. single thread)", fontsize=12, color="#cbd5e1", labelpad=10)
        ax.set_xticks(threads)
        ax.tick_params(colors="#94a3b8", labelsize=10)
        ax.grid(axis="y", linestyle="--", linewidth=0.5, alpha=0.3, color="#94a3b8")
        ax.grid(axis="x", linestyle="--", linewidth=0.5, alpha=0.15, color="#94a3b8")
        for t, s in zip(threads, speedups):
            ax.annotate(f"{s:.2f}×", xy=(t, s), textcoords="offset points",
                        xytext=(0, 12), ha="center", fontsize=9, fontweight="bold",
                        color="white", bbox=dict(boxstyle="round,pad=0.3",
                        facecolor=colour, edgecolor="none", alpha=0.85))
        for spine in ax.spines.values():
            spine.set_color("#334155")
            spine.set_linewidth(0.7)
        fig.tight_layout()
        fig.savefig(OUTPUT_DIR / f"speedup_{size}.png", dpi=150,
                    facecolor=fig.get_facecolor())
        plt.close(fig)
        print(f"  📊 Saved speedup_{size}.png")


def plot_combined(groups):
    fig, ax = plt.subplots(figsize=(12, 7))
    fig.patch.set_facecolor("#0f172a")
    ax.set_facecolor("#1e293b")
    for idx, (size, entries) in enumerate(groups.items()):
        threads = [e["threads"] for e in entries]
        speedups = [e["speedup"] for e in entries]
        colour = PALETTE[idx % len(PALETTE)]
        ax.plot(threads, speedups, marker="o", markersize=6, linewidth=2,
                color=colour, markeredgecolor="white", markeredgewidth=0.8,
                label=f"{size}×{size}", zorder=3)
    ax.axhline(y=1.0, linestyle="--", linewidth=1, color="#64748b", alpha=0.6)
    ax.set_title("Speedup Comparison — All Matrix Sizes",
                 fontsize=18, fontweight="bold", color="white", pad=16)
    ax.set_xlabel("Number of Threads", fontsize=13, color="#cbd5e1", labelpad=10)
    ax.set_ylabel("Speedup (vs. single thread)", fontsize=13, color="#cbd5e1", labelpad=10)
    all_threads = sorted({e["threads"] for es in groups.values() for e in es})
    ax.set_xticks(all_threads)
    ax.tick_params(colors="#94a3b8", labelsize=11)
    ax.grid(axis="y", linestyle="--", linewidth=0.5, alpha=0.3, color="#94a3b8")
    ax.grid(axis="x", linestyle="--", linewidth=0.5, alpha=0.15, color="#94a3b8")
    for spine in ax.spines.values():
        spine.set_color("#334155")
        spine.set_linewidth(0.7)
    legend = ax.legend(loc="upper left", fontsize=10, framealpha=0.6,
                       facecolor="#1e293b", edgecolor="#334155", labelcolor="white",
                       title="Matrix Size", title_fontsize=11)
    legend.get_title().set_color("white")
    fig.tight_layout()
    fig.savefig(OUTPUT_DIR / "speedup_combined.png", dpi=150,
                facecolor=fig.get_facecolor())
    plt.close(fig)
    print(f"  📊 Saved speedup_combined.png")


def main():
    if not BENCH_BIN.exists():
        build()
    else:
        print(f"✅ Binary found at {BENCH_BIN}\n")

    print("🚀 Running benchmark…\n")
    rows = run_benchmark()

    groups = group_by_size(rows)
    print(f"\n📈 Generating graphs for {len(groups)} matrix sizes…\n")
    plot_individual(groups)
    plot_combined(groups)
    print(f"\n✨ All done!  Results saved to {OUTPUT_DIR}/")


if __name__ == "__main__":
    main()
