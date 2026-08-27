# R2 E2 Same-Input Smoke

## Scope

One frozen leading Silesia prefix was used to verify the PAQ8px reference path
against an existing current-hash HybridZip Auto record. This is an
infrastructure smoke, not a corpus-level compression comparison.

| Field | Value |
| --- | --- |
| Case | `dickens-leading-32k` |
| Input bytes | 32,768 |
| Input SHA-256 | `FC42DCB9849222C8704C9DCAE606D075B389B66244FB215035148D6409EC0B31` |
| Frozen manifest | `bench/manifests/silesia-leading-32-64-128.tsv` |
| Manifest SHA-256 | `65830E0F72A90AF4623EFB220E510CEE66B4DA9A87C38D63A532E92B5000A55D` |
| PAQ8px binary | `F:\paq8px\experiment\build\paq8px.exe` |
| PAQ8px SHA-256 | `F79343702F596A4FA6C7CC3E25F2FA9C05199EAF11F06655B065F687E5F42533` |
| HybridZip executable SHA-256 | `CC6DA8404E3A2789A0E98BED460C4FAA90822BAC0EA362C67E261774BD0BF191` |

## Result

| Compressor | Complete archive bytes | bpb | Round-trip |
| --- | ---: | ---: | --- |
| PAQ8px v216 `-1` | 9,502 | 2.319824 | PASS |
| HybridZip R2 Auto | 9,598 | 2.343262 | PASS |

PAQ8px is smaller by 96 archive bytes for this one fixed input. Both decoded
outputs equal the frozen input SHA-256. PAQ8px encode/decode wall times were
3.529947 and 3.566450 seconds, with 372.270 and 372.035 MiB peak RAM.

## Evidence

- PAQ package:
  `results/experiments/paq8px-v216-level1-silesia-leading-dickens-32k-e2-20260828`
- Existing HybridZip package:
  `results/experiments/hybridzip-r2-currenthash-cc6d-20260827-r2-auto`

The PAQ package contains one `COMPLETE/PASS` row and 35 intentionally
`PENDING` canonical rows. It is not a completed Experiment Ledger import
package.

## Boundary

This result cannot establish average Silesia performance, PAQ8px superiority,
or the project-level compression target. E3 must run all 36 frozen manifest
cases before aggregate archive bytes can be compared.
