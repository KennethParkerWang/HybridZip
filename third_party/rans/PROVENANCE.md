# ryg-rans provenance

This directory contains the byte-identical scalar byte rANS closure from
ryg-rans revision `c9d162d996fd600315af9ae8eb89d832576cb32d`.

- Upstream: <https://github.com/rygorous/ryg_rans>
- License: CC0-1.0; `LICENSE` SHA-256:
  `518937FD5BBBDD56A3E56801CEF003997B247456BDB6E1726C8E4CB41CA41835`
- Imported source: `rans_byte.h`, SHA-256:
  `4965171BE0F0E2277C729A9218BBDB5748208CE45FCBBD580EDB56E3BE9A2AE9`

The donor is header-only. A future HybridZip-owned HZ02 adapter must serialize
the normalized symbol table and scale parameter as decoder-visible metadata,
then validate rANS payload consumption before publishing output.
