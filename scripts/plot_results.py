#!/usr/bin/env python3
"""Turn real benchmark CSVs into PNG graphs. Never invent numbers."""

from __future__ import annotations

import argparse
import sys
from pathlib import Path


def main() -> int:
    parser = argparse.ArgumentParser(
        description="Plot PSC benchmark CSVs. Fails if no real CSV data is present."
    )
    parser.add_argument("--input", required=True, help="Directory containing *.csv")
    parser.add_argument("--output", required=True, help="Directory for PNG files")
    args = parser.parse_args()

    src = Path(args.input)
    dst = Path(args.output)
    csvs = sorted(src.glob("*.csv")) if src.is_dir() else []
    if not csvs:
        print(
            "No CSV files found. Plots are generated only from Colab benchmark output. "
            "Run notebooks/02_benchmark_cpu.ipynb (and 03 if GPU) first.",
            file=sys.stderr,
        )
        return 1

    try:
        import pandas as pd  # noqa: F401
        import matplotlib  # noqa: F401
    except ImportError as exc:
        print(f"Missing Python plotting dependency: {exc}", file=sys.stderr)
        return 1

    print(
        f"Found {len(csvs)} CSV file(s). Plot generation for real data is Phase 10c "
        f"(notebooks/04_plots_and_report.ipynb). Refusing to draw empty/placeholder graphs."
    )
    dst.mkdir(parents=True, exist_ok=True)
    return 1


if __name__ == "__main__":
    raise SystemExit(main())
