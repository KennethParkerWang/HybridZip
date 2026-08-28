#!/usr/bin/env python3
"""Synthetic no-codec verification for fit_r2_fixed_point_ranker.py."""

from __future__ import annotations

import csv
import json
import subprocess
import sys
import tempfile
from pathlib import Path


ROOT = Path(__file__).resolve().parent
FITTER = ROOT / "fit_r2_fixed_point_ranker.py"


def write_json(path: Path, value: object) -> None:
    path.write_text(json.dumps(value, indent=2, sort_keys=True) + "\n", encoding="utf-8")


def main() -> int:
    with tempfile.TemporaryDirectory(prefix="hybridzip-ranker-fit-") as temporary:
        root = Path(temporary)
        training = root / "training"
        training.mkdir()
        files = (("alpha", "training", "zstd", 120), ("beta", "training", "fse", 240), ("gamma", "validation", "zstd", 160))
        write_json(training / "summary.json", {
            "status": "COMPLETE", "runtime_started": False, "codec_invocations": 0,
            "no_leakage_partition": "file-level", "training_files": ["alpha", "beta"],
            "validation_files": ["gamma"], "ranker_identity": "00010000|1025B343|" + "A" * 64,
        })
        write_json(training / "split.json", {
            "training_files": ["alpha", "beta"], "validation_files": ["gamma"],
        })
        fields = ["schema", "split", "file", "input_sha256", "tied_winner_modes"] + [f"f{index:02d}" for index in range(28)]
        with (training / "ranker_examples.csv").open("w", encoding="utf-8", newline="") as handle:
            writer = csv.DictWriter(handle, fieldnames=fields)
            writer.writeheader()
            hash_prefix = {"alpha": "A", "beta": "B", "gamma": "C"}
            for name, split, winner, feature in files:
                row = {"schema": "r2-ranker-example-v1", "split": split, "file": name,
                       "input_sha256": (hash_prefix[name] * 64), "tied_winner_modes": winner}
                row.update({f"f{index:02d}": feature if index == 3 else 0 for index in range(28)})
                writer.writerow(row)
        outputs = [root / "fit-a", root / "fit-b"]
        for output in outputs:
            subprocess.run([sys.executable, str(FITTER), "--training-data", str(training), "--output-dir", str(output)], check=True, capture_output=True, text=True)
        first = (outputs[0] / "model_image.bin").read_bytes()
        second = (outputs[1] / "model_image.bin").read_bytes()
        if len(first) != 2644 or first != second:
            raise RuntimeError("Fitted fixed-point model is not deterministic 2,644-byte output")
        manifest = json.loads((outputs[0] / "fit_manifest.json").read_text(encoding="utf-8"))
        if manifest["status"] != "CANDIDATE_FROZEN_NOT_INSTALLED" or not manifest["not_a_k8_promotion_metric"]:
            raise RuntimeError("Candidate model promotion boundary is missing")
        if manifest["validation_top1_tie_recall"]["rows"] != 1:
            raise RuntimeError("Validation membership was not isolated")
    print("fit_r2_fixed_point_ranker: PASS")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
