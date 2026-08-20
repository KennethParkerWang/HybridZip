# Kanzi SBRT MTF provenance

Imported unchanged from Kanzi-cpp commit `66a80678f1a32bceb2d7949fbde05033d4d448e4`:

- `LICENSE`: `4802925085B262835797A02BFC603B04F18188856DD4501A12D482383A09125F`
- `transform/SBRT.hpp`: `9D3ED35C9E09B37E3CC8BDA1B9729B09C4E1D0D435FFAD195BEE09818C4FCADE`
- `transform/SBRT.cpp`: `478ADA34C17D95C0350BFE9ABA5A7DE5CC9B7610E323BB83770D22CEFD189D51`

The supporting Kanzi type, context, slice, and transform headers are copied
unchanged solely to compile `SBRT.cpp`. HybridZip invokes only
`kanzi::SBRT::MODE_MTF` through the owned
`src/r2/representation/kanzi_mtf_transform.cpp` adapter. The path is Apache-2.0
and single-threaded; it carries no decoder-visible side information.
