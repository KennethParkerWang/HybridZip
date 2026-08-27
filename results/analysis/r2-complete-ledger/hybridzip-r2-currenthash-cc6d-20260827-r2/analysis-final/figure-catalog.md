# Figure Catalog

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
