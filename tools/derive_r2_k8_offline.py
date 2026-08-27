#!/usr/bin/env python3
"""Derive a read-only K=8 regret preview from an existing R2 ledger."""

from __future__ import annotations

import argparse
import csv
import json
from collections import defaultdict
from pathlib import Path


MODES = [
    "stored", "predictive", "zstd", "fse", "lzma", "donor-match",
    "bwt-zstd", "bwt-mtf-zstd", "bwt-rlt-zstd", "x86-bcj-zstd",
    "shuffle-zstd", "bitshuffle-zstd", "delta-zstd", "fastpfor", "rans",
    "bcj2-zstd", "record-transpose-zstd", "jpegls", "flac-residual",
    "brotli-text", "cmix-word-zstd", "neural-lstm", "shared-neural-lstm",
    "lstm-compress", "delta-of-delta-zstd", "bgpt-shared-prior",
    "jax-compress-portable", "ppmd7", "ppmd8", "zpaq", "ctw",
    "paq8px-apm", "paq8px-record-model", "paq8px-linear-prediction",
    "paq8px-similarity", "paq8px-similarity-sse", "paq8px-generic-sse",
    "paq8px-detected-sse", "wavpack", "lz4", "kanzi-ans",
    "lmic-arithmetic", "delta-binary-packed-zstd",
]


def per_mille(count: int, total: int) -> int:
    return 0 if total == 0 else count * 1000 // total


def equal_at(data: bytes, lag: int) -> int:
    if len(data) <= lag:
        return 0
    return per_mille(
        sum(data[i] == data[i - lag] for i in range(lag, len(data))),
        len(data) - lag,
    )


def classify(data: bytes) -> str:
    printable = sum((0x20 <= value <= 0x7E) or value in (9, 10, 13)
                    for value in data)
    whitespace = sum(value in (9, 10, 12, 13, 32, 11) for value in data)
    markup = sum(value in b"<>{}[]();=:/\\#*" for value in data)
    zeros = data.count(0)
    branches = sum(
        value in (0xE8, 0xE9) and index + 4 < len(data)
        for index, value in enumerate(data)
    )
    printable_pm = per_mille(printable, len(data))
    if per_mille(branches, len(data)) >= 20 and printable_pm < 700:
        return "x86"
    if printable_pm >= 700 and (
        per_mille(whitespace, len(data)) >= 10 or
        per_mille(markup, len(data)) >= 20
    ):
        return "text"
    equal = max(equal_at(data, 1), equal_at(data, 2), equal_at(data, 4),
                equal_at(data, 8))
    if equal >= 120 or per_mille(zeros, len(data)) >= 80:
        return "numeric"
    return "generic"


def shortlist(data: bytes) -> tuple[str, list[str]]:
    category = classify(data)
    modes = ["stored", "zstd", "paq8px-generic-sse", "paq8px-detected-sse"]
    additions = {
        "text": ["ppmd7", "ppmd8", "brotli-text", "bwt-zstd"],
        "x86": ["fse", "lzma", "x86-bcj-zstd", "bcj2-zstd"],
        "numeric": ["fse", "lzma", "delta-zstd", "shuffle-zstd"],
        "generic": ["fse", "lzma", "ppmd7", "ppmd8"],
    }
    modes.extend(additions[category])
    assert len(modes) == 8 and len(set(modes)) == 8
    return category, modes


def read_tsv(path: Path) -> list[dict[str, str]]:
    with path.open("r", encoding="utf-8-sig", newline="") as handle:
        return list(csv.DictReader(handle, delimiter="\t"))


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--mode-rows", type=Path, required=True)
    parser.add_argument("--package-manifest", type=Path, required=True)
    parser.add_argument("--output-dir", type=Path, required=True)
    args = parser.parse_args()

    mode_rows = [row for row in read_tsv(args.mode_rows)
                 if row["scope_kib"] == "32"]
    package_rows = read_tsv(args.package_manifest)
    package_paths = {
        row["mode"]: Path(row["package_path"])
        for row in package_rows
    }
    if "auto" not in package_paths:
        raise RuntimeError("package manifest has no auto package")

    by_case: dict[tuple[str, str], dict[str, dict[str, str]]] = defaultdict(dict)
    for row in mode_rows:
        by_case[(row["file"], row["scope_kib"])][row["mode"]] = row
    expected_modes = set(MODES) | {"auto"}
    if any(set(rows) != expected_modes for rows in by_case.values()):
        raise RuntimeError("mode ledger does not contain Auto plus all 43 modes")

    args.output_dir.mkdir(parents=True, exist_ok=False)
    detail_rows = []
    total_regret = 0
    recalled = 0
    for (file_name, scope), rows in sorted(by_case.items()):
        input_path = (package_paths["auto"] / "inputs" / f"{scope}KiB" /
                      f"{file_name}.bin")
        data = input_path.read_bytes()
        category, modes = shortlist(data)
        forced = {mode: int(rows[mode]["archive_bytes"]) for mode in MODES}
        oracle_bytes = min(forced.values())
        winners = sorted(mode for mode, value in forced.items()
                         if value == oracle_bytes)
        shortlist_bytes = min(forced[mode] for mode in modes)
        regret = shortlist_bytes - oracle_bytes
        has_winner = any(mode in modes for mode in winners)
        total_regret += regret
        recalled += int(has_winner)
        detail_rows.append({
            "file": file_name,
            "scope_kib": int(scope),
            "input_bytes": len(data),
            "input_sha256": rows["auto"]["input_sha256"],
            "category": category,
            "shortlist_modes": ";".join(modes),
            "shortlist_archive_bytes": shortlist_bytes,
            "oracle_archive_bytes": oracle_bytes,
            "regret_bytes": regret,
            "oracle_winner_modes": ";".join(winners),
            "winner_recalled": has_winner,
        })

    total_oracle = sum(row["oracle_archive_bytes"] for row in detail_rows)
    summary = {
        "status": "PREVIEW",
        "scope": "existing current-hash 12-file leading 32 KiB ledger",
        "cases": len(detail_rows),
        "total_oracle_bytes": total_oracle,
        "total_regret_bytes": total_regret,
        "regret_percent": (100.0 * total_regret / total_oracle
                            if total_oracle else 0.0),
        "winner_recall": recalled / len(detail_rows) if detail_rows else 0.0,
        "acceptance_claim": False,
        "boundary": "Offline derivation; not held-out runtime evidence.",
    }
    (args.output_dir / "summary.json").write_text(
        json.dumps(summary, indent=2, sort_keys=True) + "\n", encoding="utf-8"
    )
    with (args.output_dir / "per_case.tsv").open("w", encoding="utf-8",
                                                   newline="") as handle:
        writer = csv.DictWriter(handle, fieldnames=detail_rows[0].keys(),
                                delimiter="\t")
        writer.writeheader()
        writer.writerows(detail_rows)
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
