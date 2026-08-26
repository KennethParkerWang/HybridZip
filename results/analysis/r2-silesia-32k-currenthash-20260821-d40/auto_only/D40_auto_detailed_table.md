# D40 Auto-only Detailed Result Table

Source: `../mode_rows.tsv`, filtered to `mode=auto`; source files are unchanged.

- Files: 12 Silesia files, 32 KiB each
- Input bytes: 393216
- Archive bytes: 121293
- Aggregate ratio: 0.308464
- Aggregate Bpb: 2.467712
- PASS: 12/12
- Codec SHA-256: `DDD852EF0744740735E6D32EE0FFCB197C3C8349C0D695AD192C7CB96BF298BA`

| File | Selected path | Archive bytes | Ratio | Bpb | Encode s | Decode s | Peak RAM MiB | PASS |
| --- | --- | ---: | ---: | ---: | ---: | ---: | ---: | ---: |
| dickens | cmix-word-zstd | 10890 | 0.332336 | 2.658691 | 84.861919 | 0.067097 | 130.125000 | PASS |
| mozilla | lzma | 8601 | 0.262482 | 2.099854 | 89.326556 | 0.038484 | 130.289062 | PASS |
| mr | lstm-compress | 6266 | 0.191223 | 1.529785 | 50.925094 | 11.628773 | 130.320312 | PASS |
| nci | brotli-text | 2366 | 0.072205 | 0.577637 | 85.204489 | 0.038081 | 130.281250 | PASS |
| ooffice | x86-bcj-zstd | 11362 | 0.346741 | 2.773926 | 38.253950 | 0.037545 | 130.214844 | PASS |
| osdb | predictive | 13929 | 0.425079 | 3.400635 | 88.403524 | 18.940301 | 130.246094 | PASS |
| reymont | predictive | 7263 | 0.221649 | 1.773193 | 83.863598 | 18.161165 | 130.085938 | PASS |
| samba | cmix-word-zstd | 9020 | 0.275269 | 2.202148 | 84.499550 | 0.051223 | 130.175781 | PASS |
| sao | lstm-compress | 21710 | 0.662537 | 5.300293 | 89.199899 | 16.581258 | 130.140625 | PASS |
| webster | predictive | 10867 | 0.331635 | 2.653076 | 88.602156 | 18.746481 | 130.132812 | PASS |
| x-ray | lstm-compress | 16668 | 0.508667 | 4.069336 | 54.457486 | 17.564162 | 130.363281 | PASS |
| xml | predictive | 2351 | 0.071747 | 0.573975 | 90.433926 | 19.204485 | 130.300781 | PASS |

## Aggregate

| Input bytes | Archive bytes | Ratio | Bpb | Encode s | Decode s | Peak RAM MiB | PASS |
| ---: | ---: | ---: | ---: | ---: | ---: | ---: | ---: |
| 393216 | 121293 | 0.308464 | 2.467712 | 928.032148 | 121.059056 | 130.363281 | 12/12 |

This is a derived Auto view. It does not claim broader-corpus performance or statistical generalization.
