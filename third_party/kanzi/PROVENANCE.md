# Kanzi post-BWT transform provenance

Imported unchanged from Kanzi-cpp commit `66a80678f1a32bceb2d7949fbde05033d4d448e4`:

- `LICENSE`: `4802925085B262835797A02BFC603B04F18188856DD4501A12D482383A09125F`
- `transform/SBRT.hpp`: `9D3ED35C9E09B37E3CC8BDA1B9729B09C4E1D0D435FFAD195BEE09818C4FCADE`
- `transform/SBRT.cpp`: `478ADA34C17D95C0350BFE9ABA5A7DE5CC9B7610E323BB83770D22CEFD189D51`

Imported unchanged from the same commit for the variable-length RLT path:

- `Global.hpp`: `68578F90F5F0AEB68F38C4864F3C4193D4B05067FC3C18FA56F6898E76BF86AD`
- `Global.cpp`: `F32ED08C0703CB7EDF66098F7C2AC03977512ADF66A53F718D97F2F71AA7EFFE`
- `Memory.hpp`: `5CB3991815453FFAD2B93AF096916FB3198E6266DD9F3D0BE056EF3E4470CF09`
- `transform/RLT.hpp`: `3A63FEDEAC0A3F96902A7AB9826508B268CEF8B937849DA5AE280081ED1ABDB2`
- `transform/RLT.cpp`: `802E0F3CD666F94064799808CD4ABCD6B3BEEEC87A4046C6F7E2D5DF6605F9F4`
- `util/strings.hpp`: `0A8B8DF7481CC8403FEB22606BD8E89D4513975E4C7E20B3B0BFEC31B222A94A`

The supporting Kanzi type, context, slice, and transform headers are copied
unchanged solely to compile `SBRT.cpp`. HybridZip invokes `kanzi::SBRT::MODE_MTF`
through the owned `src/r2/representation/kanzi_mtf_transform.cpp` adapter and
`kanzi::RLT` through the owned `src/r2/representation/kanzi_rlt_transform.cpp`
adapter only when the donor reduces the BWT bytes. The RLT transformed length
is decoder-visible HZ02 side information. Both paths are Apache-2.0 and
single-threaded.
