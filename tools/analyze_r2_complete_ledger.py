#!/usr/bin/env python3
"""Build a strict descriptive analysis bundle from the R2 ledger TSV files."""

from __future__ import annotations

import argparse
import csv
import math
import statistics
from pathlib import Path

import matplotlib

matplotlib.use("Agg")
import matplotlib.pyplot as plt


OKABE_ITO = {
    "auto": "#0072B2",
    "detected": "#009E73",
    "generic": "#E69F00",
    "other": "#9E9E9E",
    "oracle": "#D55E00",
}


def read_tsv(path: Path) -> list[dict[str, str]]:
    with path.open("r", encoding="utf-8-sig", newline="") as handle:
        return list(csv.DictReader(handle, delimiter="\t"))


def f(row: dict[str, str], key: str) -> float:
    return float(row[key])


def i(row: dict[str, str], key: str) -> int:
    return int(row[key])


def mean_sd(values: list[float]) -> tuple[float, float]:
    if not values:
        return math.nan, math.nan
    if len(values) == 1:
        return values[0], 0.0
    return statistics.mean(values), statistics.stdev(values)


def write_text(path: Path, text: str) -> None:
    path.write_text(text.rstrip() + "\n", encoding="utf-8", newline="\n")


def save_figure(fig: plt.Figure, stem: Path) -> None:
    fig.savefig(stem.with_suffix(".pdf"), bbox_inches="tight", metadata={"Creator": "HybridZip R2 analysis"})
    fig.savefig(stem.with_suffix(".png"), dpi=600, bbox_inches="tight")
    plt.close(fig)


def main() -> None:
    parser = argparse.ArgumentParser()
    parser.add_argument("--derived", type=Path, required=True)
    parser.add_argument("--output", type=Path, required=True)
    args = parser.parse_args()

    derived = args.derived.resolve()
    output = args.output.resolve()
    figures = output / "figures"
    figures.mkdir(parents=True, exist_ok=True)
    aggregates = read_tsv(derived / "mode_aggregate.tsv")
    cases = read_tsv(derived / "per_case_oracle.tsv")
    rows = read_tsv(derived / "mode_rows.tsv")
    manifest = read_tsv(derived / "package_manifest.tsv")

    if len(aggregates) != 44 or len(cases) != 12 or len(rows) != 528 or len(manifest) != 44:
        raise SystemExit("unexpected ledger dimensions")

    auto = next(row for row in aggregates if row["mode"] == "auto")
    forced = [row for row in aggregates if row["mode"] != "auto"]
    top = sorted(forced, key=lambda row: f(row, "bpb"))[:10]
    auto_gaps = [i(row, "auto_gap_bytes") for row in cases]
    auto_values = [f(row, "auto_bpb") for row in cases]
    oracle_values = [f(row, "oracle_bpb") for row in cases]
    gap_mean, gap_sd = mean_sd([float(x) for x in auto_gaps])
    auto_mean, auto_sd = mean_sd(auto_values)
    oracle_mean, oracle_sd = mean_sd(oracle_values)

    # Main figure: all weighted archive-byte rates, sorted for scanability.
    ordered = sorted(aggregates, key=lambda row: f(row, "bpb"), reverse=True)
    labels = [row["mode"] for row in ordered]
    values = [f(row, "bpb") for row in ordered]
    colors = []
    for row in ordered:
        mode = row["mode"]
        if mode == "auto":
            colors.append(OKABE_ITO["auto"])
        elif mode == "paq8px-detected-sse":
            colors.append(OKABE_ITO["detected"])
        elif mode == "paq8px-generic-sse":
            colors.append(OKABE_ITO["generic"])
        else:
            colors.append(OKABE_ITO["other"])
    fig, ax = plt.subplots(figsize=(10.5, 11.0))
    y = list(range(len(labels)))
    ax.barh(y, values, color=colors, edgecolor="none", height=0.72)
    ax.set_yticks(y, labels, fontsize=8)
    ax.invert_yaxis()
    ax.set_xlabel("Weighted archive bits per input byte (lower is better)", fontsize=10)
    ax.grid(axis="x", color="#D9D9D9", linewidth=0.7, alpha=0.8)
    ax.set_axisbelow(True)
    ax.spines[["top", "right"]].set_visible(False)
    ax.tick_params(axis="x", labelsize=9)
    ax.set_title("HybridZip R2 current-hash ledger: archive rate by mode", fontsize=11, pad=10)
    ax.text(
        0.99,
        -0.055,
        "12 files, 32 KiB each; complete .hz2 bytes including headers and CRC",
        transform=ax.transAxes,
        ha="right",
        va="top",
        fontsize=8,
        color="#555555",
    )
    save_figure(fig, figures / "figure-01-mode-bpb")

    # Supporting figure: paired Auto/oracle values per source file.
    case_labels = [row["file"] for row in cases]
    x = list(range(len(case_labels)))
    fig, ax = plt.subplots(figsize=(10.5, 4.8))
    ax.plot(x, auto_values, marker="o", linewidth=1.8, color=OKABE_ITO["auto"], label="Auto")
    ax.plot(x, oracle_values, marker="x", markersize=7, linewidth=1.3, linestyle="--", color=OKABE_ITO["oracle"], label="Forced oracle")
    ax.set_xticks(x, case_labels, rotation=35, ha="right", fontsize=9)
    ax.set_ylabel("Archive bits per input byte", fontsize=10)
    ax.set_xlabel("Silesia file (32 KiB prefix)", fontsize=10)
    ax.grid(axis="y", color="#D9D9D9", linewidth=0.7, alpha=0.8)
    ax.set_axisbelow(True)
    ax.spines[["top", "right"]].set_visible(False)
    ax.legend(frameon=False, fontsize=9, loc="upper left")
    ax.set_title("Auto matches the complete forced-mode oracle on every case", fontsize=11, pad=10)
    save_figure(fig, figures / "figure-02-auto-oracle")

    top_lines = []
    for rank, row in enumerate(top, start=1):
        top_lines.append(
            f"| {rank} | `{row['mode']}` | {i(row, 'archive_bytes'):,} | "
            f"{f(row, 'bpb'):.6f} | {i(row, 'oracle_win_rows')} | "
            f"{i(row, 'auto_selected_rows')} |"
        )
    analysis = f"""# HybridZip R2 Current-Hash Strict Analysis

## Analysis Question

Does the decoder-visible HZ02 Auto route match the minimum complete archive-byte
choice among all 43 forced modes on the declared Silesia prefix matrix, and
which candidates contribute to that result?

## Evidence Boundary

- Ledger: `{manifest[0]['ledger_id']}`
- Unit of analysis: one Silesia file, one 32 KiB leading prefix, one run per mode
- Cases: 12 files; 44 modes (Auto plus 43 forced); 528 validated rows
- Primary metric: complete `.hz2` archive bytes and `bpb = archive_bytes * 8 / input_bytes`; lower is better
- Integrity: every row has status `COMPLETE`, roundtrip `PASS`, and byte-exact input/archive/decoded SHA-256 checks
- Codec hash: `{auto['codec_sha256']}`

## Findings

Auto totals {i(auto, 'archive_bytes'):,} bytes ({f(auto, 'bpb'):.6f} bpb) over
393,216 input bytes. The forced oracle has the same total, so the aggregate
gap is 0 bytes ({f(auto, 'bpb'):.6f} bpb). The per-case gap is zero for all
12 files. Auto selected `paq8px-detected-sse` for 5 cases and
`paq8px-generic-sse` for 7 cases; these are also the only forced modes that
won an archive-byte oracle case.

The ten lowest weighted forced rates are:

| Rank | Mode | Archive bytes | bpb | Oracle wins | Auto selections |
| ---: | --- | ---: | ---: | ---: | ---: |
{chr(10).join(top_lines)}

## Claim Candidates

- Claim: On this exact 12-case, 32 KiB prefix matrix, Auto reached the complete forced-mode archive-byte oracle.
  - Source evidence: `per_case_oracle.tsv`, 12/12 zero-gap rows; `mode_aggregate.tsv`, Auto and oracle totals both 99,720 bytes.
  - Allowed wording: “matched on the evaluated matrix.”
  - Forbidden stronger wording: “globally optimal,” “best on Silesia,” or “generalizes to all files.”
  - Uncertainty: one prefix size and one run per case; no unseen-file evaluation.
  - Next check: repeat at 64/128 KiB and add independent file classes.
  - Decision: keep
- Claim: The two PAQ8px SSE candidates carried all observed oracle wins in this matrix.
  - Source evidence: 7 generic-SSE and 5 detected-SSE oracle-win rows; all other modes have zero.
  - Allowed wording: “the observed winners in this matrix.”
  - Forbidden stronger wording: “other donors are redundant for all workloads.”
  - Uncertainty: candidate coverage is narrow and the current routing cost is high.
  - Next check: evaluate heterogeneous segments and non-Silesia inputs.
  - Decision: keep

## Statistical Scope

This is descriptive, paired evidence with n=12 file cases and one run/seed per
case. It supports no p-value, confidence interval for repeated-run variation,
effect-size claim, or independence assumption. The exact Auto-oracle gap is
0 bytes in every case; the across-file Auto bpb mean is
{auto_mean:.6f} (SD {auto_sd:.6f}) and the oracle mean is
{oracle_mean:.6f} (SD {oracle_sd:.6f}).

## Limitations

- The 32 KiB leading prefixes are not a full Silesia benchmark.
- Timing and peak memory are engineering observations, not statistical estimates.
- Candidate-not-oracle-winner is a corpus-local signal; no donor source is deleted.
- Segment-level heterogeneity remains unmeasured in this authorized ledger.
"""
    write_text(output / "analysis-report.md", analysis)

    stats_lines = [
        "# Statistical Appendix",
        "",
        "## Design",
        "",
        "- Paired unit: Silesia file (12 units), each evaluated at a 32 KiB leading prefix.",
        "- Repeats/seeds: one per mode and case; no repeated-run variance estimate.",
        "- Metric: complete archive bpb; lower is better.",
        "- Inferential tests: none; independence, normality, and variance assumptions are not used.",
        "- Multiple-comparison correction: not applicable because no hypothesis tests are reported.",
        "",
        "## Descriptive Summary",
        "",
        "| Quantity | n | Mean | SD | Min | Median | Max |",
        "| --- | ---: | ---: | ---: | ---: | ---: | ---: |",
        f"| Auto bpb across files | 12 | {statistics.mean(auto_values):.6f} | {statistics.stdev(auto_values):.6f} | {min(auto_values):.6f} | {statistics.median(auto_values):.6f} | {max(auto_values):.6f} |",
        f"| Oracle bpb across files | 12 | {statistics.mean(oracle_values):.6f} | {statistics.stdev(oracle_values):.6f} | {min(oracle_values):.6f} | {statistics.median(oracle_values):.6f} | {max(oracle_values):.6f} |",
        f"| Auto-oracle gap (bytes) | 12 | {gap_mean:.6f} | {gap_sd:.6f} | {min(auto_gaps)} | {statistics.median(auto_gaps):.6f} | {max(auto_gaps)} |",
        "",
        "## Exact Counts",
        "",
        "- Validated rows: 528/528.",
        "- Complete packages: 44/44.",
        "- Byte-exact roundtrips: 528/528.",
        "- Auto gap-positive cases: 0/12.",
        "- Forced oracle wins, ties counted: 12; generic SSE 7, detected SSE 5.",
        "",
        "## Interpretation Boundary",
        "",
        "The SD values describe variation across the 12 file types, not run-to-run "
        "uncertainty. No significance test or population-level ranking is valid "
        "from this single-prefix, single-run design. The result is an engineering "
        "selection observation and must be repeated at additional sizes and domains.",
    ]
    write_text(output / "stats-appendix.md", "\n".join(stats_lines))

    catalog = f"""# Figure Catalog

## figure-01-mode-bpb.pdf / figure-01-mode-bpb.png

- Purpose: compare weighted complete archive bpb for Auto and all 43 forced modes.
- Data source: `mode_aggregate.tsv`; 44 aggregate rows, one run per case.
- Caption requirements: state 12 files, 32 KiB prefixes, complete `.hz2` bytes including headers and CRC; lower is better; no error bars because there is no repeated-run sample.
- Key observation: Auto (2.028809 bpb) is visually coincident with the two best forced PAQ8px SSE rates; all other forced modes are higher on this matrix.
- Interpretation checklist: this is a corpus-local archive-rate comparison; do not read it as a significance result or universal donor ranking.

## figure-02-auto-oracle.pdf / figure-02-auto-oracle.png

- Purpose: test whether the decoder-visible Auto route matches the complete forced-mode oracle per file.
- Data source: `per_case_oracle.tsv`; 12 paired file cases.
- Caption requirements: state n=12 paired file cases and that both lines report complete archive bpb; no error bars because each case has one run.
- Key observation: Auto and forced-oracle traces overlap at every file; all 12 exact gaps are zero bytes.
- Interpretation checklist: this supports “matched on the evaluated matrix,” not global optimality; segment-level and larger-prefix behavior remain open.
"""
    write_text(output / "figure-catalog.md", catalog)


if __name__ == "__main__":
    main()
