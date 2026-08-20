cmake_minimum_required(VERSION 3.20)

project(hybridzip VERSION 1.0.0 LANGUAGES C CXX)

option(HYBRIDZIP_BUILD_TESTS "Build HybridZip tests" ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(ZSTD_BUILD_PROGRAMS OFF CACHE BOOL "" FORCE)
set(ZSTD_BUILD_CONTRIB OFF CACHE BOOL "" FORCE)
set(ZSTD_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(ZSTD_BUILD_STATIC ON CACHE BOOL "" FORCE)
set(ZSTD_BUILD_SHARED OFF CACHE BOOL "" FORCE)
set(ZSTD_MULTITHREAD_SUPPORT OFF CACHE BOOL "" FORCE)
add_subdirectory(third_party/zstd/build/cmake
                 third_party/zstd-build EXCLUDE_FROM_ALL)

add_library(hz_fse_donor STATIC
    third_party/fse/lib/entropy_common.c
    third_party/fse/lib/fse_compress.c
    third_party/fse/lib/fse_decompress.c
    third_party/fse/lib/hist.c
)
target_include_directories(hz_fse_donor PUBLIC
    third_party/fse
    third_party/fse/lib
)
target_compile_definitions(hz_fse_donor PRIVATE
    FSE_buildCTable=HZFSE_buildCTable
    FSE_buildCTable_raw=HZFSE_buildCTable_raw
    FSE_buildCTable_rle=HZFSE_buildCTable_rle
    FSE_buildCTable_wksp=HZFSE_buildCTable_wksp
    FSE_buildDTable=HZFSE_buildDTable
    FSE_buildDTable_raw=HZFSE_buildDTable_raw
    FSE_buildDTable_rle=HZFSE_buildDTable_rle
    FSE_compress=HZFSE_compress
    FSE_compress_usingCTable=HZFSE_compress_usingCTable
    FSE_compress_wksp=HZFSE_compress_wksp
    FSE_compress2=HZFSE_compress2
    FSE_compressBound=HZFSE_compressBound
    FSE_createCTable=HZFSE_createCTable
    FSE_createDTable=HZFSE_createDTable
    FSE_decompress=HZFSE_decompress
    FSE_decompress_usingDTable=HZFSE_decompress_usingDTable
    FSE_decompress_wksp=HZFSE_decompress_wksp
    FSE_freeCTable=HZFSE_freeCTable
    FSE_freeDTable=HZFSE_freeDTable
    FSE_getErrorName=HZFSE_getErrorName
    FSE_NCountWriteBound=HZFSE_NCountWriteBound
    FSE_normalizeCount=HZFSE_normalizeCount
    FSE_optimalTableLog=HZFSE_optimalTableLog
    FSE_optimalTableLog_internal=HZFSE_optimalTableLog_internal
    FSE_readNCount=HZFSE_readNCount
    FSE_versionNumber=HZFSE_versionNumber
    FSE_writeNCount=HZFSE_writeNCount
    HIST_count=HZHIST_count
    HIST_count_simple=HZHIST_count_simple
    HIST_count_wksp=HZHIST_count_wksp
    HIST_countFast=HZHIST_countFast
    HIST_countFast_wksp=HZHIST_countFast_wksp
    HIST_isError=HZHIST_isError
    HUF_getErrorName=HZHUF_getErrorName
    HUF_isError=HZHUF_isError
    HUF_readStats=HZHUF_readStats
)
set_source_files_properties(
    third_party/fse/lib/entropy_common.c
    PROPERTIES COMPILE_DEFINITIONS "FSE_isError=HZFSE_isError")

function(hz_enable_warnings target)
    if(MSVC)
        target_compile_options(${target} PRIVATE /W4)
    else()
        target_compile_options(${target} PRIVATE -Wall -Wextra -Wpedantic)
    endif()
endfunction()

function(hz_enable_portable_runtime target)
    if(MINGW)
        target_link_options(${target} PRIVATE -static -static-libgcc -static-libstdc++)
    endif()
endfunction()

hz_enable_warnings(hz_fse_donor)

add_library(hz_libsais_donor STATIC third_party/libsais/src/libsais.c)
target_include_directories(hz_libsais_donor PUBLIC third_party/libsais/include)
hz_enable_warnings(hz_libsais_donor)

add_library(hz_kanzi_mtf_donor STATIC
    third_party/kanzi/Global.cpp
    third_party/kanzi/transform/RLT.cpp
    third_party/kanzi/transform/SBRT.cpp
)
target_include_directories(hz_kanzi_mtf_donor PUBLIC third_party/kanzi)
hz_enable_warnings(hz_kanzi_mtf_donor)

add_library(hz_xz_bcj_donor STATIC third_party/xz/bcj_x86.c)
target_include_directories(hz_xz_bcj_donor PUBLIC third_party/xz)
hz_enable_warnings(hz_xz_bcj_donor)

add_library(hz_7zip_lzma STATIC
    third_party/7zip-lzma/C/Alloc.c
    third_party/7zip-lzma/C/CpuArch.c
    third_party/7zip-lzma/C/LzFind.c
    third_party/7zip-lzma/C/LzFindOpt.c
    third_party/7zip-lzma/C/LzmaDec.c
    third_party/7zip-lzma/C/LzmaEnc.c
)
target_include_directories(hz_7zip_lzma PUBLIC third_party/7zip-lzma/C)
target_compile_definitions(hz_7zip_lzma PRIVATE Z7_ST)
hz_enable_warnings(hz_7zip_lzma)

add_library(hz_nayuki
    third_party/nayuki/ArithmeticCoder.cpp
    third_party/nayuki/BitIoStream.cpp
    third_party/nayuki/FrequencyTable.cpp
)
target_include_directories(hz_nayuki PUBLIC third_party/nayuki)
hz_enable_warnings(hz_nayuki)

add_library(hz_core
    src/archive/archive_header.cpp
    src/core/byte_history.cpp
    src/core/cdf.cpp
    src/core/probability.cpp
    src/core/profile.cpp
    src/mixer/adaptive_linear_mixer.cpp
)
target_include_directories(hz_core PUBLIC src)
hz_enable_warnings(hz_core)

add_library(hz_codec
    src/codec/decoder.cpp
    src/codec/encoder.cpp
    src/codec/model_pipeline.cpp
    src/entropy/arithmetic_codec.cpp
    src/entropy/cdf_frequency_table.cpp
)
target_include_directories(hz_codec PUBLIC src)
target_link_libraries(hz_codec PUBLIC hz_core hz_nayuki PRIVATE hz_models)
hz_enable_warnings(hz_codec)

add_library(hz_cmix_lstm
    third_party/cmix/lstm.cpp
    third_party/cmix/lstm_layer.cpp
    third_party/cmix/sigmoid.cpp
)
target_include_directories(hz_cmix_lstm PUBLIC third_party/cmix)
hz_enable_warnings(hz_cmix_lstm)

add_library(hz_cmix_match
    third_party/cmix/match_core.cpp
    src/r2/experts/cmix_match_expert.cpp
)
target_include_directories(hz_cmix_match
    PUBLIC src
    PRIVATE third_party/cmix
)
hz_enable_warnings(hz_cmix_match)

add_library(hz_paq8px_match
    third_party/paq8px/match_core.cpp
    src/r2/experts/paq8px_match_expert.cpp
    src/r2/match/paq8px_match_service.cpp
)
target_include_directories(hz_paq8px_match PUBLIC src)
hz_enable_warnings(hz_paq8px_match)

add_library(hz_models
    third_party/cmix/ppmd_core.cpp
    src/predictors/match_predictor.cpp
    src/predictors/ngram_predictor.cpp
    src/predictors/online_lstm_predictor.cpp
    src/predictors/ppmd_predictor.cpp
)
target_include_directories(hz_models PUBLIC src PRIVATE third_party/cmix)
target_link_libraries(hz_models PUBLIC hz_core PRIVATE hz_cmix_lstm)
hz_enable_warnings(hz_models)

add_library(hz_analysis
    src/analysis/oracle_analyzer.cpp
    src/analysis/variant_mixer.cpp
)
target_include_directories(hz_analysis PUBLIC src)
target_link_libraries(hz_analysis PUBLIC hz_core hz_codec)
hz_enable_warnings(hz_analysis)

add_library(hz_r2
    src/r2/archive/r2_archive.cpp
    src/r2/block/block_planner.cpp
    src/r2/codec/r2_codec.cpp
    src/r2/entropy/binary_arithmetic_codec.cpp
    src/r2/entropy/donor_match_predictive_backend.cpp
    src/r2/entropy/fse_backend.cpp
    src/r2/entropy/lzma_backend.cpp
    src/r2/entropy/predictive_v1_backend.cpp
    src/r2/entropy/stored_backend.cpp
    src/r2/entropy/zstd_backend.cpp
    src/r2/lz/lzma_match_finder_service.cpp
    src/r2/lz/zstd_sequence_service.cpp
    src/r2/representation/bwt_transform.cpp
    src/r2/representation/kanzi_mtf_transform.cpp
    src/r2/representation/kanzi_rlt_transform.cpp
    src/r2/representation/xz_x86_bcj_transform.cpp
)
target_include_directories(hz_r2 PUBLIC src)
target_link_libraries(hz_r2 PUBLIC hz_codec
                         PRIVATE hz_7zip_lzma hz_cmix_match hz_fse_donor
                                 hz_paq8px_match hz_libsais_donor hz_kanzi_mtf_donor libzstd_static)
target_link_libraries(hz_r2 PRIVATE hz_xz_bcj_donor)
hz_enable_warnings(hz_r2)

add_executable(hybridzip
    src/app/cli.cpp
    src/main.cpp
)
target_link_libraries(hybridzip PRIVATE hz_codec hz_r2)
hz_enable_warnings(hybridzip)
hz_enable_portable_runtime(hybridzip)
set_target_properties(hybridzip PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/Release"
    RUNTIME_OUTPUT_DIRECTORY_RELEASE "${CMAKE_BINARY_DIR}/Release"
)

add_executable(hybridzip_oracle
    src/app/oracle_cli.cpp
)
target_link_libraries(hybridzip_oracle PRIVATE hz_analysis)
hz_enable_warnings(hybridzip_oracle)
hz_enable_portable_runtime(hybridzip_oracle)
set_target_properties(hybridzip_oracle PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/Release"
    RUNTIME_OUTPUT_DIRECTORY_RELEASE "${CMAKE_BINARY_DIR}/Release"
)

add_executable(hz_profile_lstm_smoke EXCLUDE_FROM_ALL
    tests/profile_lstm_smoke.cpp
)
target_link_libraries(hz_profile_lstm_smoke PRIVATE hz_models)
hz_enable_warnings(hz_profile_lstm_smoke)
hz_enable_portable_runtime(hz_profile_lstm_smoke)

if(HYBRIDZIP_BUILD_TESTS)
    include(CTest)
    add_executable(hz_core_tests tests/core_tests.cpp)
    target_link_libraries(hz_core_tests PRIVATE hz_core hz_codec)
    hz_enable_warnings(hz_core_tests)
    hz_enable_portable_runtime(hz_core_tests)
    add_test(NAME hz_core_tests COMMAND hz_core_tests)

    add_executable(hz_lstm_predictor_tests tests/lstm_predictor_tests.cpp)
    target_link_libraries(hz_lstm_predictor_tests PRIVATE hz_models)
    hz_enable_warnings(hz_lstm_predictor_tests)
    hz_enable_portable_runtime(hz_lstm_predictor_tests)
    add_test(NAME hz_lstm_predictor_tests COMMAND hz_lstm_predictor_tests)

    add_executable(hz_predictor_tests tests/predictor_tests.cpp)
    target_link_libraries(hz_predictor_tests PRIVATE hz_models)
    hz_enable_warnings(hz_predictor_tests)
    hz_enable_portable_runtime(hz_predictor_tests)
    add_test(NAME hz_predictor_tests COMMAND hz_predictor_tests)

    add_executable(hz_ppmd_predictor_tests tests/ppmd_predictor_tests.cpp)
    target_link_libraries(hz_ppmd_predictor_tests PRIVATE hz_models)
    hz_enable_warnings(hz_ppmd_predictor_tests)
    hz_enable_portable_runtime(hz_ppmd_predictor_tests)
    add_test(NAME hz_ppmd_predictor_tests COMMAND hz_ppmd_predictor_tests)

    add_executable(hz_pipeline_tests tests/pipeline_tests.cpp)
    target_link_libraries(hz_pipeline_tests PRIVATE hz_codec)
    hz_enable_warnings(hz_pipeline_tests)
    hz_enable_portable_runtime(hz_pipeline_tests)
    add_test(NAME hz_pipeline_tests COMMAND hz_pipeline_tests)

    add_executable(hz_pipeline_evidence_tests
        tests/pipeline_evidence_tests.cpp)
    target_link_libraries(hz_pipeline_evidence_tests PRIVATE hz_codec)
    hz_enable_warnings(hz_pipeline_evidence_tests)
    hz_enable_portable_runtime(hz_pipeline_evidence_tests)
    add_test(NAME hz_pipeline_evidence_tests
             COMMAND hz_pipeline_evidence_tests)

    add_executable(hz_match_diagnostics_tests
        tests/match_diagnostics_tests.cpp)
    target_link_libraries(hz_match_diagnostics_tests PRIVATE hz_models)
    hz_enable_warnings(hz_match_diagnostics_tests)
    hz_enable_portable_runtime(hz_match_diagnostics_tests)
    add_test(NAME hz_match_diagnostics_tests
             COMMAND hz_match_diagnostics_tests)

    add_executable(hz_variant_mixer_tests tests/variant_mixer_tests.cpp)
    target_link_libraries(hz_variant_mixer_tests PRIVATE hz_analysis)
    hz_enable_warnings(hz_variant_mixer_tests)
    hz_enable_portable_runtime(hz_variant_mixer_tests)
    add_test(NAME hz_variant_mixer_tests COMMAND hz_variant_mixer_tests)

    add_executable(hz_r2_codec_tests tests/r2_codec_tests.cpp)
    target_link_libraries(hz_r2_codec_tests PRIVATE hz_r2)
    hz_enable_warnings(hz_r2_codec_tests)
    hz_enable_portable_runtime(hz_r2_codec_tests)
    add_test(NAME hz_r2_codec_tests COMMAND hz_r2_codec_tests)

    add_executable(hz_oracle_analyzer_tests
        tests/oracle_analyzer_tests.cpp)
    target_link_libraries(hz_oracle_analyzer_tests PRIVATE hz_analysis)
    hz_enable_warnings(hz_oracle_analyzer_tests)
    hz_enable_portable_runtime(hz_oracle_analyzer_tests)
    add_test(NAME hz_oracle_analyzer_tests
             COMMAND hz_oracle_analyzer_tests)

    add_executable(hz_fse_backend_tests tests/fse_backend_tests.cpp)
    target_link_libraries(hz_fse_backend_tests PRIVATE hz_r2)
    hz_enable_warnings(hz_fse_backend_tests)
    hz_enable_portable_runtime(hz_fse_backend_tests)
    add_test(NAME hz_fse_backend_tests COMMAND hz_fse_backend_tests)

    add_executable(hz_lzma_backend_tests tests/lzma_backend_tests.cpp)
    target_link_libraries(hz_lzma_backend_tests PRIVATE hz_r2)
    hz_enable_warnings(hz_lzma_backend_tests)
    hz_enable_portable_runtime(hz_lzma_backend_tests)
    add_test(NAME hz_lzma_backend_tests COMMAND hz_lzma_backend_tests)

    add_executable(hz_cmix_match_expert_tests
        tests/cmix_match_expert_tests.cpp)
    target_include_directories(hz_cmix_match_expert_tests
        PRIVATE third_party/cmix)
    target_link_libraries(hz_cmix_match_expert_tests PRIVATE hz_cmix_match)
    hz_enable_warnings(hz_cmix_match_expert_tests)
    hz_enable_portable_runtime(hz_cmix_match_expert_tests)
    add_test(NAME hz_cmix_match_expert_tests
             COMMAND hz_cmix_match_expert_tests)

    add_executable(hz_paq8px_match_tests
        tests/paq8px_match_tests.cpp)
    target_link_libraries(hz_paq8px_match_tests PRIVATE hz_paq8px_match)
    hz_enable_warnings(hz_paq8px_match_tests)
    hz_enable_portable_runtime(hz_paq8px_match_tests)
    add_test(NAME hz_paq8px_match_tests COMMAND hz_paq8px_match_tests)

    add_executable(hz_donor_match_predictive_backend_tests
        tests/donor_match_predictive_backend_tests.cpp)
    target_link_libraries(hz_donor_match_predictive_backend_tests
        PRIVATE hz_r2)
    hz_enable_warnings(hz_donor_match_predictive_backend_tests)
    hz_enable_portable_runtime(hz_donor_match_predictive_backend_tests)
    add_test(NAME hz_donor_match_predictive_backend_tests
             COMMAND hz_donor_match_predictive_backend_tests)

    add_executable(hz_zstd_sequence_service_tests
        tests/zstd_sequence_service_tests.cpp)
    target_link_libraries(hz_zstd_sequence_service_tests PRIVATE hz_r2)
    hz_enable_warnings(hz_zstd_sequence_service_tests)
    hz_enable_portable_runtime(hz_zstd_sequence_service_tests)
    add_test(NAME hz_zstd_sequence_service_tests
             COMMAND hz_zstd_sequence_service_tests)

    add_executable(hz_lzma_match_finder_service_tests
        tests/lzma_match_finder_service_tests.cpp)
    target_link_libraries(hz_lzma_match_finder_service_tests PRIVATE hz_r2)
    hz_enable_warnings(hz_lzma_match_finder_service_tests)
    hz_enable_portable_runtime(hz_lzma_match_finder_service_tests)
    add_test(NAME hz_lzma_match_finder_service_tests
             COMMAND hz_lzma_match_finder_service_tests)
endif()
