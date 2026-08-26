# R2 Smoke Evidence Index

This index scans existing verification.json files only. It accepts one-byte-
exact 1 KiB record per HZ02 mode and does not execute the codec.

- candidate modes: 0..42 (43 total)
- qualifying records found: 14
- unique modes covered: 11
- missing modes: 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 24, 25, 26, 27, 28, 29, 36, 37, 38
- codec hash filter: none

latest_by_mode.tsv keeps the newest qualifying record per mode. Historical
records and rebuild duplicates are intentionally reduced rather than treated
as independent final evidence.