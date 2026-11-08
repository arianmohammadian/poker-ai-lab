#!/usr/bin/env python3
"""Convert Poker AI Lab preflop equity CSV into a C++ constexpr table."""

import csv
import math
import os
from pathlib import Path
import sys
import tempfile

RANKS = 13


def generate(source: Path, destination: Path) -> None:
    cells: list[list[str | None]] = [[None] * RANKS for _ in range(RANKS)]
    samples_seen: set[int] = set()

    with source.open(newline="", encoding="utf-8") as stream:
        reader = csv.DictReader(stream)
        required = {"row", "col", "hand", "equity", "samples", "seed"}
        if not required.issubset(reader.fieldnames or []):
            raise ValueError(f"CSV must contain columns: {', '.join(sorted(required))}")

        for line_number, record in enumerate(reader, start=2):
            try:
                row = int(record["row"])
                col = int(record["col"])
                value = float(record["equity"])
                samples = int(record["samples"])
            except (TypeError, ValueError) as error:
                raise ValueError(f"Invalid data on CSV line {line_number}") from error

            if not (0 <= row < RANKS and 0 <= col < RANKS):
                raise ValueError(f"Out-of-range cell ({row}, {col}) on line {line_number}")
            if not math.isfinite(value) or not 0.0 <= value <= 1.0:
                raise ValueError(f"Invalid equity at ({row}, {col}): {value}")
            if samples <= 0:
                raise ValueError(f"Invalid samples on line {line_number}")
            if cells[row][col] is not None:
                raise ValueError(f"Duplicate cell ({row}, {col})")
            cells[row][col] = record["equity"].strip()
            samples_seen.add(samples)

    missing = [(i, j) for i in range(RANKS) for j in range(RANKS) if cells[i][j] is None]
    if missing:
        raise ValueError(f"CSV incomplete: missing {len(missing)} cells (first: {missing[0]})")
    if len(samples_seen) != 1:
        raise ValueError("All entries must use the same sample count")

    lines = [
        "// GENERATED FILE — DO NOT EDIT MANUALLY.",
        "// Source: data/preflop_equities.csv",
        f"// Monte Carlo samples per class: {next(iter(samples_seen))}",
        "constexpr PreflopEquityTable::Table kEquities = {{",
    ]
    for row in cells:
        lines.append("    {{" + ", ".join(value for value in row if value is not None) + "}},")
    lines.append("}};")
    output = "\n".join(lines) + "\n"

    destination.parent.mkdir(parents=True, exist_ok=True)
    fd, temp_name = tempfile.mkstemp(dir=destination.parent, prefix=".preflop_", suffix=".tmp")
    try:
        with os.fdopen(fd, "w", encoding="utf-8") as stream:
            stream.write(output)
        os.replace(temp_name, destination)
    finally:
        if os.path.exists(temp_name):
            os.unlink(temp_name)

    print(f"Generated {destination} (169 equities, {next(iter(samples_seen))} samples each)")


if __name__ == "__main__":
    if len(sys.argv) != 1:
        raise SystemExit("Usage: python3 tools/csv_to_preflop_inc.py (run from repository root)")
    generate(Path("data/preflop_equities.csv"), Path("src/preflop_equities_generated.inc"))
