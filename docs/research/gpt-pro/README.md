# HybridZip GPT Pro Research Packet

## Purpose

This directory prepares the current HybridZip facts for an external GPT Pro
research pass. It is a research handoff, not a claim that the stated product
targets have been achieved.

Read [RESEARCH_BRIEF.md](RESEARCH_BRIEF.md) first. It defines the required
acceptance criteria, current evidence, gaps, and research questions. Then use
[GPT_PRO_PROMPT.md](GPT_PRO_PROMPT.md) as the copy-ready request.

## Material Upload Order

Upload the following small files to GPT Pro in this order. The local paths are
authoritative because the current R2 ledger and documentation are not yet all
committed to the public GitHub remote.

| Priority | Material | Local path | Why it is needed |
| --- | --- | --- | --- |
| P0 | This brief | `E:\MIXER\hybridzip\docs\research\gpt-pro\RESEARCH_BRIEF.md` | Requirements, verified state, gaps, research questions |
| P0 | R2 decision report | `C:\Users\Administrator\.codex\attachments\6651cadc-1895-4fb5-af73-2f4eeac7b405\pasted-text.txt` | The approved donor-first R2 architecture and constraints |
| P0 | Current product status | `E:\MIXER\hybridzip\docs\PRODUCT_STATUS.md` | Exact current implementation/evidence boundary |
| P0 | R2 analysis | `E:\MIXER\hybridzip\results\analysis\r2-complete-ledger\hybridzip-r2-currenthash-cc6d-20260827-r2\analysis-final\analysis-report.md` | Claim limits and 32 KiB ledger findings |
| P0 | R2 aggregates | `E:\MIXER\hybridzip\results\analysis\r2-complete-ledger\hybridzip-r2-currenthash-cc6d-20260827-r2\derived-final\mode_aggregate.tsv` | Archive bytes, bpb, encode/decode time, RAM per mode |
| P1 | Router/planner source | `E:\MIXER\hybridzip\src\r2\block\block_planner.cpp`; `E:\MIXER\hybridzip\src\r2\routing\activation_router.{h,cpp}` | Current Auto selection cost and available features |
| P1 | PAQ8px reference protocol | `F:\paq8px\benchmark_paq8px_32KiB_parallel\PROTOCOL.md`; `F:\paq8px\benchmark_paq8px_32KiB_parallel\REPORT_CN.md` | Baseline configuration and a known comparability caveat |
| P1 | Donor/licensing record | `E:\MIXER\hybridzip\docs\SOURCES.md`; `E:\MIXER\KU\hybridzip-r2\` | Existing donor provenance and local material warehouse |
| P1 | Result contract | `F:\paq8px\experiment-ledger-desktop\OTHER_COMPRESSOR_RESULT_FORMAT.md` | Required Silesia 32/64/128 KiB result evidence |
| P2 | Architecture figures | `E:\MIXER\hybridzip\figures\hybridzip-current-architecture.png`; `E:\MIXER\hybridzip\figures\hybridzip-full-r2-architecture.png` | Visual orientation only; status text is older than the completed ledger |

Do not upload the entire `results/experiments/` tree unless GPT Pro explicitly
needs raw per-case logs. The two TSV/Markdown artifacts above carry the result
matrix needed for architectural research, and the full tree is large.

## Known Evidence Boundary

The current R2 ledger covers all 12 Silesia files at a leading 32 KiB prefix,
with Auto plus 43 forced HZ02 modes. It has 528 `COMPLETE/PASS` byte-exact
round trips. It does not cover the Tencent dataset, R2 at 64/128 KiB, a GPU
implementation, or a fair same-input PAQ8px v216 `-1` comparison.

The current release executable is
`E:\MIXER\hybridzip\build\Release\hybridzip.exe`, SHA-256
`CC6DA8404E3A2789A0E98BED460C4FAA90822BAC0EA362C67E261774BD0BF191`.
