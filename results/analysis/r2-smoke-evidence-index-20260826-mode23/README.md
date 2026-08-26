# R2 Smoke Evidence Index

This index scans existing verification.json files only. It accepts one-byte-
exact 1 KiB record per HZ02 mode and does not execute the codec.

- candidate modes: 0..42 (43 total)
- qualifying records found: 20
- unique modes covered: 20
- missing modes: 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22
- codec hash filter: `FDE6F9ABC0F831CC9E35BF6B53C24654E06FBB2EE232856924E211A17B04A75B`

latest_by_mode.tsv keeps the newest qualifying record per mode. Historical
records and rebuild duplicates are intentionally reduced rather than treated
as independent final evidence.