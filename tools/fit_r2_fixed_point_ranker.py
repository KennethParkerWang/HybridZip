#!/usr/bin/env python3
"""Fit and freeze a candidate R2 fixed-point ranker without installing it.

The tool consumes only the file-level split exported by
export_r2_ranker_training_set.ps1. It writes the exact 2,644-byte C++ model
image, CRC32, SHA-256, fit configuration, and validation-only top-1 tie
recall. The output is deliberately not imported by the encoder.
"""

from __future__ import annotations

import argparse
import binascii
import csv
import hashlib
import json
import struct
from dataclasses import dataclass
from pathlib import Path
from typing import Iterable


MODES = (
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
)
MODE_INDEX = {name: index for index, name in enumerate(MODES)}
FEATURE_COUNT = 28
MODEL_BYTE_COUNT = 2644

# This is the production V1 shift contract copied from mode_ranker.cpp. The
# fitter freezes the same shifts, so its scores can be replayed by C++ later.
FEATURE_SHIFTS = (
    0, 12, 12, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 12,
    6, 6, 6, 6, 6, 6, 0, 6, 12, 0, 6, 0,
)


@dataclass(frozen=True)
class Example:
    file: str
    split: str
    features: tuple[int, ...]
    winners: tuple[int, ...]
    input_sha256: str


def sha256_file(path: Path) -> str:
    return hashlib.sha256(path.read_bytes()).hexdigest().upper()


def read_json(path: Path) -> object:
    with path.open("r", encoding="utf-8") as handle:
        return json.load(handle)


def require_object(value: object, description: str) -> dict[str, object]:
    if not isinstance(value, dict):
        raise ValueError(f"{description} must be a JSON object")
    return value


def require_string(mapping: dict[str, object], key: str, description: str) -> str:
    value = mapping.get(key)
    if not isinstance(value, str) or not value:
        raise ValueError(f"{description} is missing a nonempty {key}")
    return value


def require_string_list(
    mapping: dict[str, object], key: str, description: str
) -> tuple[str, ...]:
    value = mapping.get(key)
    if not isinstance(value, list) or not value or any(
        not isinstance(item, str) or not item for item in value
    ):
        raise ValueError(f"{description} is missing a nonempty string list {key}")
    if len(set(value)) != len(value):
        raise ValueError(f"{description} has duplicate files in {key}")
    return tuple(value)


def parse_examples(training_dir: Path) -> tuple[list[Example], dict[str, object]]:
    summary_path = training_dir / "summary.json"
    split_path = training_dir / "split.json"
    csv_path = training_dir / "ranker_examples.csv"
    for path in (summary_path, split_path, csv_path):
        if not path.is_file():
            raise ValueError(f"Required training artifact is missing: {path}")

    summary = require_object(read_json(summary_path), "Training summary")
    split = require_object(read_json(split_path), "Training split")
    if summary.get("status") != "COMPLETE":
        raise ValueError("Training summary is not COMPLETE")
    if summary.get("no_leakage_partition") != "file-level":
        raise ValueError("Training summary does not declare file-level isolation")
    if summary.get("codec_invocations") != 0 or summary.get("runtime_started") is not False:
        raise ValueError("Training package must not contain codec runtime work")
    training_files = require_string_list(summary, "training_files", "Training summary")
    validation_files = require_string_list(summary, "validation_files", "Training summary")
    if set(training_files) & set(validation_files):
        raise ValueError("Training and validation file memberships overlap")
    if tuple(split.get("training_files", ())) != training_files or tuple(
        split.get("validation_files", ())
    ) != validation_files:
        raise ValueError("split.json membership differs from summary.json")

    examples: list[Example] = []
    seen_files: set[str] = set()
    with csv_path.open("r", encoding="utf-8-sig", newline="") as handle:
        reader = csv.DictReader(handle)
        if reader.fieldnames is None:
            raise ValueError("ranker_examples.csv has no header")
        required = {"schema", "split", "file", "input_sha256", "tied_winner_modes"}
        required.update(f"f{index:02d}" for index in range(FEATURE_COUNT))
        missing = required - set(reader.fieldnames)
        if missing:
            raise ValueError(f"ranker_examples.csv is missing columns: {sorted(missing)}")
        for row in reader:
            if row.get("schema") != "r2-ranker-example-v1":
                raise ValueError("ranker_examples.csv has an unexpected schema")
            file_name = row["file"]
            split_name = row["split"]
            if not file_name or file_name in seen_files:
                raise ValueError("ranker_examples.csv has an empty or duplicate file")
            expected_split = "training" if file_name in training_files else "validation"
            if file_name not in training_files and file_name not in validation_files:
                raise ValueError(f"CSV file is absent from the declared split: {file_name}")
            if split_name != expected_split:
                raise ValueError(f"CSV split disagrees with declared split: {file_name}")
            input_sha256 = row["input_sha256"].upper()
            if len(input_sha256) != 64 or any(char not in "0123456789ABCDEF" for char in input_sha256):
                raise ValueError(f"CSV input SHA-256 is malformed: {file_name}")
            try:
                features = tuple(int(row[f"f{index:02d}"]) for index in range(FEATURE_COUNT))
            except ValueError as error:
                raise ValueError(f"CSV feature is not an integer: {file_name}") from error
            names = tuple(name for name in row["tied_winner_modes"].split(",") if name)
            if not names or len(set(names)) != len(names) or any(name not in MODE_INDEX for name in names):
                raise ValueError(f"CSV tied winner modes are malformed: {file_name}")
            examples.append(Example(
                file=file_name,
                split=split_name,
                features=features,
                winners=tuple(MODE_INDEX[name] for name in names),
                input_sha256=input_sha256,
            ))
            seen_files.add(file_name)

    if seen_files != set(training_files) | set(validation_files):
        raise ValueError("CSV file membership differs from the declared split")
    if not any(example.split == "training" for example in examples):
        raise ValueError("Training split has no examples")
    if not any(example.split == "validation" for example in examples):
        raise ValueError("Validation split has no examples")
    return examples, summary


def shifted_features(example: Example) -> tuple[int, ...]:
    return tuple(value >> shift for value, shift in zip(example.features, FEATURE_SHIFTS))


def score(weights: list[list[int]], biases: list[int], features: tuple[int, ...], mode: int) -> int:
    return biases[mode] + sum(weight * value for weight, value in zip(weights[mode], features))


def top_mode(weights: list[list[int]], biases: list[int], features: tuple[int, ...]) -> int:
    return max(range(len(MODES)), key=lambda mode: (score(weights, biases, features, mode), -mode))


def best_tied_mode(
    weights: list[list[int]], biases: list[int], features: tuple[int, ...], winners: tuple[int, ...]
) -> int:
    return max(winners, key=lambda mode: (score(weights, biases, features, mode), -mode))


def clamp_i16(value: int) -> int:
    return max(-32768, min(32767, value))


def fit(examples: Iterable[Example], epochs: int, learning_rate: int) -> tuple[list[list[int]], list[int], list[int]]:
    weights = [[0 for _ in range(FEATURE_COUNT)] for _ in MODES]
    biases = [0 for _ in MODES]
    errors_by_epoch: list[int] = []
    training = sorted((example for example in examples if example.split == "training"), key=lambda item: item.file)
    for _ in range(epochs):
        errors = 0
        for example in training:
            values = shifted_features(example)
            predicted = top_mode(weights, biases, values)
            if predicted in example.winners:
                continue
            target = best_tied_mode(weights, biases, values, example.winners)
            for index, value in enumerate(values):
                delta = learning_rate * value
                weights[target][index] = clamp_i16(weights[target][index] + delta)
                weights[predicted][index] = clamp_i16(weights[predicted][index] - delta)
            biases[target] += learning_rate
            biases[predicted] -= learning_rate
            errors += 1
        errors_by_epoch.append(errors)
    return weights, biases, errors_by_epoch


def recall(examples: Iterable[Example], weights: list[list[int]], biases: list[int], split: str) -> dict[str, object]:
    selected = [example for example in examples if example.split == split]
    hits = 0
    rows: list[dict[str, object]] = []
    for example in sorted(selected, key=lambda item: item.file):
        predicted = top_mode(weights, biases, shifted_features(example))
        hit = predicted in example.winners
        hits += int(hit)
        rows.append({
            "file": example.file,
            "input_sha256": example.input_sha256,
            "predicted_mode": MODES[predicted],
            "tied_winner_modes": [MODES[index] for index in example.winners],
            "top1_tie_hit": hit,
        })
    return {
        "rows": len(selected),
        "top1_tie_hits": hits,
        "top1_tie_recall": hits / len(selected) if selected else 0.0,
        "per_file": rows,
    }


def canonical_model_image(weights: list[list[int]], biases: list[int], version: int) -> tuple[bytes, int]:
    prefix = bytearray()
    for row in weights:
        for weight in row:
            prefix.extend(struct.pack("<h", weight))
    for bias in biases:
        prefix.extend(struct.pack("<i", bias))
    for shift in FEATURE_SHIFTS:
        prefix.extend(struct.pack("<h", shift))
    prefix.extend(struct.pack("<I", version))
    crc32 = binascii.crc32(prefix) & 0xFFFFFFFF
    image = bytes(prefix) + struct.pack("<I", crc32)
    if len(image) != MODEL_BYTE_COUNT:
        raise AssertionError(f"Canonical model has {len(image)} bytes, expected {MODEL_BYTE_COUNT}")
    return image, crc32


def write_json(path: Path, value: object) -> None:
    path.write_text(json.dumps(value, indent=2, sort_keys=True) + "\n", encoding="utf-8")


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--training-data", type=Path, required=True)
    parser.add_argument("--output-dir", type=Path, required=True)
    parser.add_argument("--epochs", type=int, default=16)
    parser.add_argument("--learning-rate", type=int, default=1)
    parser.add_argument("--model-version", type=lambda value: int(value, 0), default=0x00010001)
    args = parser.parse_args()
    if args.epochs < 1 or args.epochs > 10000:
        raise ValueError("epochs must be in [1, 10000]")
    if args.learning_rate < 1 or args.learning_rate > 1024:
        raise ValueError("learning-rate must be in [1, 1024]")
    if args.model_version < 0 or args.model_version > 0xFFFFFFFF:
        raise ValueError("model-version must fit uint32")
    training_dir = args.training_data.resolve()
    output_dir = args.output_dir.resolve()
    if not training_dir.is_dir():
        raise ValueError(f"Training data directory does not exist: {training_dir}")
    if output_dir.exists():
        raise ValueError(f"Refusing to overwrite fitted model directory: {output_dir}")

    examples, summary = parse_examples(training_dir)
    weights, biases, errors_by_epoch = fit(examples, args.epochs, args.learning_rate)
    image, crc32 = canonical_model_image(weights, biases, args.model_version)
    image_sha256 = hashlib.sha256(image).hexdigest().upper()
    output_dir.mkdir(parents=True)
    (output_dir / "model_image.bin").write_bytes(image)
    model = {
        "schema": "r2-fixed-point-ranker-model-v1",
        "mode_names": list(MODES),
        "feature_shifts": list(FEATURE_SHIFTS),
        "weights": weights,
        "biases": biases,
        "version": f"{args.model_version:08X}",
        "crc32": f"{crc32:08X}",
        "canonical_byte_count": len(image),
        "sha256": image_sha256,
    }
    write_json(output_dir / "model.json", model)
    validation = recall(examples, weights, biases, "validation")
    training = recall(examples, weights, biases, "training")
    manifest = {
        "status": "CANDIDATE_FROZEN_NOT_INSTALLED",
        "model_installation": "prohibited until a separately recorded held-out E5 gate passes",
        "training_data_path": str(training_dir),
        "training_data_summary_sha256": sha256_file(training_dir / "summary.json"),
        "training_data_split_sha256": sha256_file(training_dir / "split.json"),
        "training_data_examples_sha256": sha256_file(training_dir / "ranker_examples.csv"),
        "training_input_ranker_identity": require_string(summary, "ranker_identity", "Training summary"),
        "no_leakage_partition": "file-level",
        "epochs": args.epochs,
        "learning_rate": args.learning_rate,
        "model_version": f"{args.model_version:08X}",
        "model_crc32": f"{crc32:08X}",
        "model_sha256": image_sha256,
        "canonical_model_bytes": len(image),
        "training_perceptron_errors_by_epoch": errors_by_epoch,
        "training_top1_tie_recall": training,
        "validation_top1_tie_recall": validation,
        "not_a_k8_promotion_metric": True,
    }
    write_json(output_dir / "fit_manifest.json", manifest)
    print(json.dumps({
        "output_dir": str(output_dir),
        "model_sha256": image_sha256,
        "model_crc32": f"{crc32:08X}",
        "validation_top1_tie_recall": validation["top1_tie_recall"],
        "candidate_only": True,
    }, sort_keys=True))
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
