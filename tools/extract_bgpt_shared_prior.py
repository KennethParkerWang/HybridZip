#!/usr/bin/env python3
"""Project a pinned bGPT checkpoint into a deterministic byte CDF table.

The projection runs the donor's 12-layer patch decoder once on a fixed
16-byte special-token bootstrap patch. It then runs the 3-layer byte decoder
for the first-byte state and for each possible previous byte. The special
token output is discarded and the remaining 256-way posterior is quantized
to a 16-bit frequency table with every byte assigned at least one count.

This is deliberately a bounded shared-prior adapter, not a claim that the
complete bGPT context runtime has been ported.
"""

from __future__ import annotations

import argparse
import collections
import hashlib
import io
import json
import math
import pickle
import zipfile
from dataclasses import dataclass
from pathlib import Path

import numpy as np


EXPECTED_CHECKPOINT_SHA256 = (
    "f30ed5a814086c5b9e64f56a76ccfcded00a82fc71c3ba6de322b708d29f6ac7"
)
MODEL_REVISION = "56b98d647b97c086bf9b3c0b840f0d662545e81c"
MODEL_REPOSITORY = "https://github.com/sanderwood/bgpt"
CHECKPOINT_REPOSITORY = "https://huggingface.co/sander-wood/bgpt"
CHECKPOINT_REVISION = "7b3fc8b7fe0b4fec4f40dd0bdeb39b2cacf0aa96"
PROJECTION_VERSION = 1
HIDDEN_SIZE = 768
HEAD_COUNT = 12
PATCH_SIZE = 16
CDF_TOTAL = 1 << 16


@dataclass(frozen=True)
class StorageRef:
    kind: str
    key: str
    location: str
    size: int


@dataclass(frozen=True)
class TensorRef:
    storage: StorageRef
    offset: int
    shape: tuple[int, ...]
    stride: tuple[int, ...]


class _IgnoredGlobal:
    def __init__(self, name: str):
        self.name = name

    def __call__(self, *args, **kwargs):
        return {"ignored_global": self.name, "args": args, "kwargs": kwargs}


class _MetadataUnpickler(pickle.Unpickler):
    def persistent_load(self, persistent_id):
        if (isinstance(persistent_id, tuple) and persistent_id and
                persistent_id[0] == "storage"):
            _, kind, key, location, size = persistent_id
            return StorageRef(
                getattr(kind, "__name__", str(kind)), str(key),
                str(location), int(size))
        raise pickle.UnpicklingError("unsupported persistent checkpoint ID")

    def find_class(self, module: str, name: str):
        if module == "collections" and name == "OrderedDict":
            return collections.OrderedDict
        if module == "torch._utils" and name in (
                "_rebuild_tensor", "_rebuild_tensor_v2"):
            return lambda storage, offset, shape, stride, *unused: TensorRef(
                storage, int(offset), tuple(shape), tuple(stride))
        if module == "torch._utils" and name == "_rebuild_parameter":
            return lambda tensor, *unused: tensor
        if module == "torch" and name.endswith("Storage"):
            return type(name, (), {})
        return _IgnoredGlobal(f"{module}.{name}")


class Checkpoint:
    def __init__(self, path: Path):
        self.archive = zipfile.ZipFile(path)
        self.root = self.archive.namelist()[0].split("/")[0]
        metadata = _MetadataUnpickler(io.BytesIO(
            self.archive.read(f"{self.root}/data.pkl"))).load()
        self.state = metadata["model"]

    def close(self) -> None:
        self.archive.close()

    def tensor(self, name: str) -> np.ndarray:
        ref = self.state[name]
        if not isinstance(ref, TensorRef):
            raise TypeError(f"checkpoint entry is not a tensor: {name}")
        dtype_by_storage = {
            "FloatStorage": np.dtype("<f4"),
            "DoubleStorage": np.dtype("<f8"),
            "HalfStorage": np.dtype("<f2"),
            "LongStorage": np.dtype("<i8"),
            "IntStorage": np.dtype("<i4"),
            "ByteStorage": np.dtype("u1"),
        }
        dtype = dtype_by_storage.get(ref.storage.kind)
        if dtype is None:
            raise TypeError(f"unsupported storage type: {ref.storage.kind}")
        raw = self.archive.read(f"{self.root}/data/{ref.storage.key}")
        values = np.frombuffer(raw, dtype=dtype)
        count = math.prod(ref.shape) if ref.shape else 1
        contiguous_stride = []
        stride = 1
        for dimension in reversed(ref.shape):
            contiguous_stride.append(stride)
            stride *= dimension
        if ref.shape and tuple(reversed(contiguous_stride)) != ref.stride:
            raise ValueError(f"non-contiguous checkpoint tensor: {name}")
        end = ref.offset + count
        if end > values.size:
            raise ValueError(f"checkpoint tensor exceeds storage: {name}")
        return values[ref.offset:end].reshape(ref.shape).astype(
            np.float32, copy=False)


def layer_norm(x: np.ndarray, weight: np.ndarray,
               bias: np.ndarray) -> np.ndarray:
    mean = x.mean(axis=-1, keepdims=True, dtype=np.float32)
    variance = ((x - mean) ** 2).mean(
        axis=-1, keepdims=True, dtype=np.float32)
    return (x - mean) / np.sqrt(variance + np.float32(1e-5)) * weight + bias


def gelu_new(x: np.ndarray) -> np.ndarray:
    coefficient = np.float32(math.sqrt(2.0 / math.pi))
    return np.float32(0.5) * x * (
        np.float32(1.0) + np.tanh(
            coefficient * (x + np.float32(0.044715) * x * x * x)))


def transformer(checkpoint: Checkpoint, prefix: str, x: np.ndarray,
                layer_count: int) -> np.ndarray:
    batch, sequence, _ = x.shape
    head_size = HIDDEN_SIZE // HEAD_COUNT
    causal = np.triu(np.ones((sequence, sequence), dtype=bool), 1)
    for layer in range(layer_count):
        block = f"{prefix}.h.{layer}"
        normalized = layer_norm(
            x, checkpoint.tensor(f"{block}.ln_1.weight"),
            checkpoint.tensor(f"{block}.ln_1.bias"))
        qkv = (normalized @ checkpoint.tensor(
            f"{block}.attn.c_attn.weight") + checkpoint.tensor(
                f"{block}.attn.c_attn.bias"))
        query, key, value = np.split(qkv, 3, axis=-1)
        query = query.reshape(batch, sequence, HEAD_COUNT, head_size).transpose(
            0, 2, 1, 3)
        key = key.reshape(batch, sequence, HEAD_COUNT, head_size).transpose(
            0, 2, 1, 3)
        value = value.reshape(batch, sequence, HEAD_COUNT, head_size).transpose(
            0, 2, 1, 3)
        scores = (query @ key.transpose(0, 1, 3, 2)) / np.float32(
            math.sqrt(head_size))
        scores = np.where(causal[None, None, :, :], np.float32(-1e4), scores)
        scores -= scores.max(axis=-1, keepdims=True)
        attention = np.exp(scores, dtype=np.float32)
        attention /= attention.sum(axis=-1, keepdims=True, dtype=np.float32)
        context = (attention @ value).transpose(0, 2, 1, 3).reshape(
            batch, sequence, HIDDEN_SIZE)
        x = x + context @ checkpoint.tensor(
            f"{block}.attn.c_proj.weight") + checkpoint.tensor(
                f"{block}.attn.c_proj.bias")

        normalized = layer_norm(
            x, checkpoint.tensor(f"{block}.ln_2.weight"),
            checkpoint.tensor(f"{block}.ln_2.bias"))
        hidden = gelu_new(normalized @ checkpoint.tensor(
            f"{block}.mlp.c_fc.weight") + checkpoint.tensor(
                f"{block}.mlp.c_fc.bias"))
        x = x + hidden @ checkpoint.tensor(
            f"{block}.mlp.c_proj.weight") + checkpoint.tensor(
                f"{block}.mlp.c_proj.bias")
    return layer_norm(
        x, checkpoint.tensor(f"{prefix}.ln_f.weight"),
        checkpoint.tensor(f"{prefix}.ln_f.bias"))


def quantize(probabilities: np.ndarray) -> np.ndarray:
    probabilities = probabilities.astype(np.float64)
    probabilities /= probabilities.sum()
    distributable = CDF_TOTAL - probabilities.size
    exact = probabilities * distributable
    base = np.floor(exact).astype(np.int64)
    frequencies = base + 1
    missing = CDF_TOTAL - int(frequencies.sum())
    order = np.lexsort((np.arange(probabilities.size), -(exact - base)))
    frequencies[order[:missing]] += 1
    if frequencies.min() < 1 or frequencies.max() > np.iinfo(np.uint16).max:
        raise ValueError("projected frequency is outside uint16 range")
    if int(frequencies.sum()) != CDF_TOTAL:
        raise ValueError("projected row does not sum to the CDF total")
    return frequencies.astype("<u2")


def project(checkpoint: Checkpoint) -> np.ndarray:
    patch_weight = checkpoint.tensor(
        "patch_level_decoder.patch_embedding.weight")
    patch_bias = checkpoint.tensor(
        "patch_level_decoder.patch_embedding.bias")
    special_columns = np.arange(PATCH_SIZE) * 257 + 256
    bootstrap = patch_weight[:, special_columns].sum(axis=1) + patch_bias
    bootstrap = bootstrap.reshape(1, 1, HIDDEN_SIZE)
    bootstrap += checkpoint.tensor("patch_level_decoder.base.wpe.weight")[
        :1][None, :, :]
    encoded_patch = transformer(
        checkpoint, "patch_level_decoder.base", bootstrap, 12)[0, 0]

    byte_prefix = "byte_level_decoder.base.transformer"
    byte_embedding = checkpoint.tensor(f"{byte_prefix}.wte.weight")
    byte_position = checkpoint.tensor(f"{byte_prefix}.wpe.weight")
    output_weight = checkpoint.tensor("byte_level_decoder.base.lm_head.weight")

    first_input = encoded_patch.reshape(1, 1, HIDDEN_SIZE) + byte_position[
        :1][None, :, :]
    first_hidden = transformer(checkpoint, byte_prefix, first_input, 3)
    first_logits = first_hidden[0, -1] @ output_weight.T

    conditional_input = np.empty((256, 2, HIDDEN_SIZE), dtype=np.float32)
    conditional_input[:, 0, :] = encoded_patch
    conditional_input[:, 1, :] = byte_embedding[:256]
    conditional_input += byte_position[:2][None, :, :]
    conditional_hidden = transformer(
        checkpoint, byte_prefix, conditional_input, 3)
    conditional_logits = conditional_hidden[:, -1, :] @ output_weight.T

    logits = np.vstack((conditional_logits[:, :256], first_logits[None, :256]))
    logits = logits.astype(np.float64)
    logits -= logits.max(axis=1, keepdims=True)
    probabilities = np.exp(logits)
    probabilities /= probabilities.sum(axis=1, keepdims=True)
    return np.vstack([quantize(row) for row in probabilities])


def write_cpp(path: Path, frequencies: np.ndarray, table_sha256: str) -> None:
    values = frequencies.reshape(-1)
    lines = []
    for offset in range(0, values.size, 16):
        chunk = ", ".join(str(int(value)) for value in values[offset:offset + 16])
        lines.append(f"    {chunk},")
    source = f'''// Generated by tools/extract_bgpt_shared_prior.py.
// bGPT source revision: {MODEL_REVISION}
// bGPT text checkpoint SHA-256: {EXPECTED_CHECKPOINT_SHA256}
// Projection version: {PROJECTION_VERSION}
// Frequency table SHA-256 (little-endian uint16): {table_sha256}

#include "r2/entropy/bgpt_shared_prior_data.h"

#include <array>

namespace hz::r2 {{
namespace {{

constexpr std::array<std::uint16_t, kBgptSharedPriorContextCount * 256U>
    kFrequencies{{{{
{chr(10).join(lines)}
}}}};

}}  // namespace

const std::uint16_t* bgpt_shared_prior_frequencies() noexcept {{
    return kFrequencies.data();
}}

}}  // namespace hz::r2
'''
    path.write_text(source, encoding="ascii", newline="\n")


def main() -> None:
    parser = argparse.ArgumentParser()
    parser.add_argument("--checkpoint", required=True, type=Path)
    parser.add_argument("--output", required=True, type=Path)
    parser.add_argument("--manifest", required=True, type=Path)
    args = parser.parse_args()

    checkpoint_sha256 = hashlib.sha256(args.checkpoint.read_bytes()).hexdigest()
    if checkpoint_sha256 != EXPECTED_CHECKPOINT_SHA256:
        raise ValueError(
            f"checkpoint SHA-256 mismatch: {checkpoint_sha256}")
    checkpoint = Checkpoint(args.checkpoint)
    try:
        frequencies = project(checkpoint)
    finally:
        checkpoint.close()
    if frequencies.shape != (257, 256):
        raise ValueError(f"unexpected projected table shape: {frequencies.shape}")
    table_sha256 = hashlib.sha256(frequencies.tobytes(order="C")).hexdigest()
    args.output.parent.mkdir(parents=True, exist_ok=True)
    write_cpp(args.output, frequencies, table_sha256)
    manifest = {
        "schema_version": 1,
        "model": "bGPT text",
        "source_repository": MODEL_REPOSITORY,
        "source_revision": MODEL_REVISION,
        "checkpoint_repository": CHECKPOINT_REPOSITORY,
        "checkpoint_revision": CHECKPOINT_REVISION,
        "checkpoint_file": args.checkpoint.name,
        "checkpoint_bytes": args.checkpoint.stat().st_size,
        "checkpoint_sha256": checkpoint_sha256,
        "license": "MIT",
        "projection": "fixed-special-patch byte bigram posterior",
        "projection_version": PROJECTION_VERSION,
        "contexts": 257,
        "symbols": 256,
        "cdf_total": CDF_TOTAL,
        "table_sha256": table_sha256,
        "output": str(args.output),
    }
    args.manifest.parent.mkdir(parents=True, exist_ok=True)
    args.manifest.write_text(
        json.dumps(manifest, indent=2, sort_keys=True) + "\n",
        encoding="ascii", newline="\n")
    print(json.dumps(manifest, sort_keys=True))


if __name__ == "__main__":
    main()
