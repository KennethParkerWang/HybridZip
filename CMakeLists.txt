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
    third_party/kanzi/entropy/ANSRangeEncoder.cpp
    third_party/kanzi/entropy/ANSRangeDecoder.cpp
    third_party/kanzi/entropy/EntropyUtils.cpp
    third_party/kanzi/transform/RLT.cpp
    third_party/kanzi/transform/SBRT.cpp
)
target_include_directories(hz_kanzi_mtf_donor PUBLIC third_party/kanzi)
hz_enable_warnings(hz_kanzi_mtf_donor)

add_library(hz_xz_bcj_donor STATIC third_party/xz/bcj_x86.c)
target_include_directories(hz_xz_bcj_donor PUBLIC third_party/xz)
hz_enable_warnings(hz_xz_bcj_donor)

add_library(hz_blosc_shuffle_donor STATIC third_party/blosc/shuffle_generic.c)
target_include_directories(hz_blosc_shuffle_donor PUBLIC third_party/blosc)
hz_enable_warnings(hz_blosc_shuffle_donor)

add_library(hz_fastpfor_donor STATIC third_party/fastpfor/src/bitpacking.cpp)
target_include_directories(hz_fastpfor_donor PUBLIC third_party/fastpfor/headers)
hz_enable_warnings(hz_fastpfor_donor)

add_library(hz_charls_donor STATIC
    third_party/charls/src/charls_jpegls_decoder.cpp
    third_party/charls/src/charls_jpegls_encoder.cpp
    third_party/charls/src/golomb_lut.cpp
    third_party/charls/src/jpegls_error.cpp
    third_party/charls/src/jpeg_stream_reader.cpp
    third_party/charls/src/jpeg_stream_writer.cpp
    third_party/charls/src/make_scan_codec.cpp
    third_party/charls/src/pch.cpp
    third_party/charls/src/quantization_lut.cpp
    third_party/charls/src/validate_spiff_header.cpp
    third_party/charls/src/version.cpp
)
target_include_directories(hz_charls_donor PUBLIC third_party/charls/include)
target_compile_definitions(hz_charls_donor
    PUBLIC CHARLS_STATIC
    PRIVATE CHARLS_LIBRARY_BUILD
)
hz_enable_warnings(hz_charls_donor)

add_library(hz_flac_donor STATIC
    src/r2/representation/flac_donor_config.c
    third_party/flac/src/libFLAC/bitmath.c
    third_party/flac/src/libFLAC/fixed.c
    third_party/flac/src/libFLAC/lpc.c
)
target_include_directories(hz_flac_donor PUBLIC
    third_party/flac/include
    third_party/flac/src/libFLAC/include
)
target_compile_definitions(hz_flac_donor PRIVATE
    FLAC__NO_ASM
    FLAC__NO_DLL
    HAVE_FSEEKO=1
    HAVE_LROUND=1
)
hz_enable_warnings(hz_flac_donor)

add_library(hz_wavpack_donor STATIC
    third_party/wavpack/src/common_utils.c
    third_party/wavpack/src/decorr_utils.c
    third_party/wavpack/src/entropy_utils.c
    third_party/wavpack/src/extra1.c
    third_party/wavpack/src/extra2.c
    third_party/wavpack/src/open_utils.c
    third_party/wavpack/src/open_raw.c
    third_party/wavpack/src/pack.c
    third_party/wavpack/src/pack_dns.c
    third_party/wavpack/src/pack_floats.c
    third_party/wavpack/src/pack_utils.c
    third_party/wavpack/src/read_words.c
    third_party/wavpack/src/tags.c
    third_party/wavpack/src/tag_utils.c
    third_party/wavpack/src/unpack.c
    third_party/wavpack/src/unpack_floats.c
    third_party/wavpack/src/unpack_seek.c
    third_party/wavpack/src/unpack_utils.c
    third_party/wavpack/src/write_words.c
)
target_include_directories(hz_wavpack_donor PUBLIC
    third_party/wavpack/include
)
target_compile_definitions(hz_wavpack_donor PRIVATE
    _CRT_SECURE_NO_WARNINGS
    HAVE___BUILTIN_CLZ=1
)
hz_enable_warnings(hz_wavpack_donor)

add_library(hz_lz4_donor STATIC
    third_party/lz4/lz4.c
    third_party/lz4/lz4hc.c
)
target_include_directories(hz_lz4_donor PUBLIC third_party/lz4)
hz_enable_warnings(hz_lz4_donor)

add_library(hz_brotli_common STATIC
    third_party/brotli/c/common/constants.c
    third_party/brotli/c/common/context.c
    third_party/brotli/c/common/dictionary.c
    third_party/brotli/c/common/platform.c
    third_party/brotli/c/common/shared_dictionary.c
    third_party/brotli/c/common/transform.c
)
target_include_directories(hz_brotli_common
    PUBLIC third_party/brotli/c/include
    PRIVATE third_party/brotli/c
)
hz_enable_warnings(hz_brotli_common)

add_library(hz_brotli_dec STATIC
    third_party/brotli/c/dec/bit_reader.c
    third_party/brotli/c/dec/decode.c
    third_party/brotli/c/dec/huffman.c
    third_party/brotli/c/dec/prefix.c
    third_party/brotli/c/dec/state.c
    third_party/brotli/c/dec/static_init.c
)
target_include_directories(hz_brotli_dec
    PUBLIC third_party/brotli/c/include
    PRIVATE third_party/brotli/c
)
target_link_libraries(hz_brotli_dec PUBLIC hz_brotli_common)
hz_enable_warnings(hz_brotli_dec)

add_library(hz_brotli_enc STATIC
    third_party/brotli/c/enc/backward_references.c
    third_party/brotli/c/enc/backward_references_hq.c
    third_party/brotli/c/enc/bit_cost.c
    third_party/brotli/c/enc/block_splitter.c
    third_party/brotli/c/enc/brotli_bit_stream.c
    third_party/brotli/c/enc/cluster.c
    third_party/brotli/c/enc/command.c
    third_party/brotli/c/enc/compound_dictionary.c
    third_party/brotli/c/enc/compress_fragment.c
    third_party/brotli/c/enc/compress_fragment_two_pass.c
    third_party/brotli/c/enc/dictionary_hash.c
    third_party/brotli/c/enc/encode.c
    third_party/brotli/c/enc/encoder_dict.c
    third_party/brotli/c/enc/entropy_encode.c
    third_party/brotli/c/enc/fast_log.c
    third_party/brotli/c/enc/histogram.c
    third_party/brotli/c/enc/literal_cost.c
    third_party/brotli/c/enc/memory.c
    third_party/brotli/c/enc/metablock.c
    third_party/brotli/c/enc/static_dict.c
    third_party/brotli/c/enc/static_dict_lut.c
    third_party/brotli/c/enc/static_init.c
    third_party/brotli/c/enc/utf8_util.c
)
target_include_directories(hz_brotli_enc
    PUBLIC third_party/brotli/c/include
    PRIVATE third_party/brotli/c
)
target_link_libraries(hz_brotli_enc PUBLIC hz_brotli_common)
hz_enable_warnings(hz_brotli_enc)

add_library(hz_7zip_lzma STATIC
    third_party/7zip-lzma/C/Alloc.c
    third_party/7zip-lzma/C/Bcj2.c
    third_party/7zip-lzma/C/Bcj2Enc.c
    third_party/7zip-lzma/C/CpuArch.c
    third_party/7zip-lzma/C/LzFind.c
    third_party/7zip-lzma/C/LzFindOpt.c
    third_party/7zip-lzma/C/LzmaDec.c
    third_party/7zip-lzma/C/LzmaEnc.c
    third_party/7zip-lzma/C/Ppmd7.c
    third_party/7zip-lzma/C/Ppmd7Dec.c
    third_party/7zip-lzma/C/Ppmd7Enc.c
    third_party/7zip-lzma/C/Ppmd8.c
    third_party/7zip-lzma/C/Ppmd8Dec.c
    third_party/7zip-lzma/C/Ppmd8Enc.c
)
target_include_directories(hz_7zip_lzma PUBLIC third_party/7zip-lzma/C)
target_compile_definitions(hz_7zip_lzma PRIVATE Z7_ST)
hz_enable_warnings(hz_7zip_lzma)

add_library(hz_zpaq_donor STATIC third_party/zpaq/libzpaq.cpp)
target_include_directories(hz_zpaq_donor PUBLIC third_party/zpaq)
target_compile_definitions(hz_zpaq_donor PRIVATE NOJIT)
hz_enable_warnings(hz_zpaq_donor)

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

set(HZ_CMIX_DICTIONARY_SOURCE
    "${CMAKE_CURRENT_SOURCE_DIR}/third_party/cmix/preprocess/english.dic")
set(HZ_CMIX_DICTIONARY_GENERATED_DIR
    "${CMAKE_CURRENT_BINARY_DIR}/generated")
set(HZ_CMIX_DICTIONARY_GENERATED_SOURCE
    "${HZ_CMIX_DICTIONARY_GENERATED_DIR}/cmix_dictionary_data.cpp")
file(READ "${HZ_CMIX_DICTIONARY_SOURCE}" HZ_CMIX_DICTIONARY_HEX HEX)
string(REGEX REPLACE "([0-9A-Fa-f][0-9A-Fa-f])" "0x\\1, "
       HZ_CMIX_DICTIONARY_INITIALIZERS "${HZ_CMIX_DICTIONARY_HEX}")
file(MAKE_DIRECTORY "${HZ_CMIX_DICTIONARY_GENERATED_DIR}")
file(SIZE "${HZ_CMIX_DICTIONARY_SOURCE}" HZ_CMIX_DICTIONARY_SIZE)
file(WRITE "${HZ_CMIX_DICTIONARY_GENERATED_SOURCE}" "#include \"r2/representation/cmix_dictionary_data.h\"\n\n#include <array>\n\nnamespace hz::r2 {\nnamespace {\n\nconst std::array<std::uint8_t, ${HZ_CMIX_DICTIONARY_SIZE}> kDictionary{\n${HZ_CMIX_DICTIONARY_INITIALIZERS}\n};\n\n}  // namespace\n\nconst std::uint8_t* cmix_dictionary_data() noexcept {\n    return kDictionary.data();\n}\n\nstd::size_t cmix_dictionary_size() noexcept {\n    return kDictionary.size();\n}\n\n}  // namespace hz::r2\n")

add_library(hz_cmix_dictionary_donor STATIC
    third_party/cmix/preprocess/dictionary.cpp
    "${HZ_CMIX_DICTIONARY_GENERATED_SOURCE}"
)
target_include_directories(hz_cmix_dictionary_donor PUBLIC
    src
    third_party/cmix/preprocess
)
hz_enable_warnings(hz_cmix_dictionary_donor)

add_library(hz_paq8px_match
    third_party/paq8px/match_core.cpp
    src/r2/experts/paq8px_match_expert.cpp
    src/r2/match/paq8px_match_service.cpp
)
target_include_directories(hz_paq8px_match PUBLIC src)
hz_enable_warnings(hz_paq8px_match)

add_library(hz_paq8px_apm STATIC third_party/paq8px/apm1.cpp)
target_include_directories(hz_paq8px_apm PUBLIC third_party/paq8px)
hz_enable_warnings(hz_paq8px_apm)

add_library(hz_paq8px_record_model STATIC
    third_party/paq8px/record_model/AdaptiveMap.cpp
    third_party/paq8px/record_model/BitCount.cpp
    third_party/paq8px/record_model/BlockType.cpp
    third_party/paq8px/record_model/ContextMap.cpp
    third_party/paq8px/record_model/Ilog.cpp
    third_party/paq8px/record_model/IndirectMap.cpp
    third_party/paq8px/record_model/Mixer.cpp
    third_party/paq8px/record_model/Mixer_Scalar.cpp
    third_party/paq8px/record_model/ProgramChecker.cpp
    third_party/paq8px/record_model/Random.cpp
    third_party/paq8px/record_model/Shared.cpp
    third_party/paq8px/record_model/SmallStationaryContextMap.cpp
    third_party/paq8px/record_model/Squash.cpp
    third_party/paq8px/record_model/StateMap.cpp
    third_party/paq8px/record_model/StateTable.cpp
    third_party/paq8px/record_model/StationaryMap.cpp
    third_party/paq8px/record_model/Stretch.cpp
    third_party/paq8px/record_model/UpdateBroadcaster.cpp
    third_party/paq8px/record_model/model/RecordModel.cpp
)
target_include_directories(hz_paq8px_record_model PUBLIC
    third_party/paq8px/record_model
)
hz_enable_warnings(hz_paq8px_record_model)

add_library(hz_paq8px_linear_prediction_model STATIC
    third_party/paq8px/record_model/OLS.cpp
    third_party/paq8px/record_model/OLS_factory.cpp
    third_party/paq8px/record_model/OLS_float_Scalar.cpp
    third_party/paq8px/record_model/OLS_double_Scalar.cpp
    third_party/paq8px/record_model/ResidualMap.cpp
    third_party/paq8px/record_model/model/LinearPredictionModel.cpp
)
target_include_directories(hz_paq8px_linear_prediction_model PUBLIC
    third_party/paq8px/record_model
)
target_link_libraries(hz_paq8px_linear_prediction_model PUBLIC
    hz_paq8px_record_model
)
hz_enable_warnings(hz_paq8px_linear_prediction_model)

add_library(hz_paq8px_similarity_model STATIC
    third_party/paq8px/record_model/ContextMap2.cpp
    third_party/paq8px/record_model/model/SimilarityEmaFunctions_Scalar.cpp
    third_party/paq8px/record_model/model/SimilarityEmaFunctionsFactory.cpp
    third_party/paq8px/record_model/model/SimilarityModel.cpp
    third_party/paq8px/record_model/model/SimilarityModelPair.cpp
)
target_include_directories(hz_paq8px_similarity_model PUBLIC
    third_party/paq8px/record_model
)
target_link_libraries(hz_paq8px_similarity_model PUBLIC
    hz_paq8px_record_model
)
hz_enable_warnings(hz_paq8px_similarity_model)

add_library(hz_paq8px_sse STATIC
    third_party/paq8px/record_model/APM.cpp
    third_party/paq8px/record_model/APM1.cpp
    third_party/paq8px/record_model/APMPost.cpp
    third_party/paq8px/record_model/SSE.cpp
)
target_include_directories(hz_paq8px_sse PUBLIC
    third_party/paq8px/record_model
)
target_link_libraries(hz_paq8px_sse PUBLIC
    hz_paq8px_record_model
)
hz_enable_warnings(hz_paq8px_sse)

add_library(hz_paq8px_generic_model STATIC
    third_party/paq8px/record_model/LargeStationaryMap.cpp
    third_party/paq8px/record_model/LMS.cpp
    third_party/paq8px/record_model/LMS_Scalar.cpp
    third_party/paq8px/record_model/MTFList.cpp
    third_party/paq8px/record_model/MixerFactory.cpp
    third_party/paq8px/record_model/StateMapPair.cpp
    third_party/paq8px/record_model/String.cpp
    third_party/paq8px/record_model/file/File.cpp
    third_party/paq8px/record_model/file/FileDisk.cpp
    third_party/paq8px/record_model/file/OpenFromMyFolder.cpp
    third_party/paq8px/record_model/model/CharGroupModel.cpp
    third_party/paq8px/record_model/model/AudioModel.cpp
    third_party/paq8px/record_model/model/Audio8BitModel.cpp
    third_party/paq8px/record_model/model/Audio16BitModel.cpp
    third_party/paq8px/record_model/model/ChartModel.cpp
    third_party/paq8px/record_model/model/DmcForest.cpp
    third_party/paq8px/record_model/model/DmcModel.cpp
    third_party/paq8px/record_model/model/DmcNode.cpp
    third_party/paq8px/record_model/model/ExeModel.cpp
    third_party/paq8px/record_model/model/IndirectModel.cpp
    third_party/paq8px/record_model/model/Image8BitModel.cpp
    third_party/paq8px/record_model/model/Image24BitModel.cpp
    third_party/paq8px/record_model/model/JpegModel.cpp
    third_party/paq8px/record_model/model/MatchInfo.cpp
    third_party/paq8px/record_model/model/MatchModel.cpp
    third_party/paq8px/record_model/model/NestModel.cpp
    third_party/paq8px/record_model/model/NormalModel.cpp
    third_party/paq8px/record_model/model/SparseBitModel.cpp
    third_party/paq8px/record_model/model/SparseMatchModel.cpp
    third_party/paq8px/record_model/model/SparseModel.cpp
    third_party/paq8px/record_model/model/WordModel.cpp
    third_party/paq8px/record_model/model/WordModelInfo.cpp
    third_party/paq8px/record_model/model/XMLModel.cpp
    third_party/paq8px/record_model/text/English.cpp
    third_party/paq8px/record_model/text/EnglishStemmer.cpp
    third_party/paq8px/record_model/text/French.cpp
    third_party/paq8px/record_model/text/FrenchStemmer.cpp
    third_party/paq8px/record_model/text/German.cpp
    third_party/paq8px/record_model/text/GermanStemmer.cpp
    third_party/paq8px/record_model/text/Stemmer.cpp
    third_party/paq8px/record_model/text/TextModel.cpp
    third_party/paq8px/record_model/text/Word.cpp
    third_party/paq8px/record_model/text/WordEmbeddingDictionary.cpp
    third_party/paq8px/record_model/filter/DecAlpha.cpp
    third_party/paq8px/record_model/filter/TextParserStateInfo.cpp
)
target_include_directories(hz_paq8px_generic_model PUBLIC
    third_party/paq8px/record_model
)
target_link_libraries(hz_paq8px_generic_model PUBLIC
    hz_paq8px_record_model
    hz_paq8px_linear_prediction_model
    hz_paq8px_similarity_model
    hz_paq8px_sse
)
hz_enable_warnings(hz_paq8px_generic_model)

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
    src/r2/entropy/fastpfor_backend.cpp
    src/r2/entropy/rans_backend.cpp
    src/r2/entropy/fse_backend.cpp
    src/r2/entropy/lzma_backend.cpp
    src/r2/entropy/lz4_backend.cpp
    src/r2/entropy/kanzi_ans_backend.cpp
    src/r2/entropy/ppmd7_backend.cpp
    src/r2/entropy/ppmd8_backend.cpp
    src/r2/entropy/zpaq_backend.cpp
    src/r2/entropy/ctw_backend.cpp
    src/r2/entropy/paq8px_apm_backend.cpp
    src/r2/entropy/paq8px_record_model_backend.cpp
    src/r2/entropy/paq8px_linear_prediction_backend.cpp
    src/r2/entropy/paq8px_similarity_backend.cpp
    src/r2/entropy/paq8px_similarity_sse_backend.cpp
    src/r2/entropy/paq8px_generic_sse_backend.cpp
    src/r2/entropy/paq8px_block_detector.cpp
    src/r2/entropy/paq8px_detected_sse_backend.cpp
    src/r2/entropy/wavpack_backend.cpp
    src/r2/entropy/neural_lstm_backend.cpp
    src/r2/entropy/lstm_compress_backend.cpp
    src/r2/entropy/bgpt_shared_prior_backend.cpp
    src/r2/entropy/bgpt_shared_prior_data.cpp
    src/r2/entropy/jax_compress_portable_backend.cpp
    src/r2/entropy/lmic_arithmetic_backend.cpp
    src/r2/entropy/predictive_v1_backend.cpp
    src/r2/entropy/stored_backend.cpp
    src/r2/entropy/zstd_backend.cpp
    src/r2/lz/lzma_match_finder_service.cpp
    src/r2/lz/zstd_sequence_service.cpp
    src/r2/representation/bwt_transform.cpp
    src/r2/representation/kanzi_mtf_transform.cpp
    src/r2/representation/kanzi_rlt_transform.cpp
    src/r2/representation/xz_x86_bcj_transform.cpp
    src/r2/representation/bcj2_transform.cpp
    src/r2/representation/jpegls_transform.cpp
    src/r2/representation/flac_residual_transform.cpp
    src/r2/representation/brotli_text_transform.cpp
    src/r2/representation/cmix_word_dictionary_transform.cpp
    src/r2/representation/record_transpose_transform.cpp
    src/r2/representation/blosc_shuffle_transform.cpp
    src/r2/representation/blosc_bitshuffle_transform.cpp
    src/r2/representation/blosc_delta_transform.cpp
    src/r2/representation/delta_of_delta_transform.cpp
    src/r2/representation/delta_binary_packed_transform.cpp
    src/r2/representation/structure_analyzer.cpp
    src/r2/routing/activation_router.cpp
    third_party/lstm-compress/port/lstm_compress_donor_port.cpp
)
target_include_directories(hz_r2 PUBLIC src)
target_include_directories(hz_r2 PRIVATE third_party/lstm-compress/port)
target_include_directories(hz_r2 PRIVATE third_party/rans)
target_link_libraries(hz_r2 PUBLIC hz_codec
                         PRIVATE hz_models hz_7zip_lzma hz_cmix_match hz_fse_donor
                                 hz_paq8px_match hz_paq8px_apm hz_paq8px_record_model
                                 hz_paq8px_linear_prediction_model hz_paq8px_similarity_model
                                 hz_paq8px_sse hz_paq8px_generic_model
                                 hz_libsais_donor hz_kanzi_mtf_donor
                                 hz_zpaq_donor libzstd_static)
target_link_libraries(hz_r2 PRIVATE hz_xz_bcj_donor)
target_link_libraries(hz_r2 PRIVATE hz_blosc_shuffle_donor)
target_link_libraries(hz_r2 PRIVATE hz_fastpfor_donor)
target_link_libraries(hz_r2 PRIVATE hz_charls_donor)
target_link_libraries(hz_r2 PRIVATE hz_flac_donor)
target_link_libraries(hz_r2 PRIVATE hz_wavpack_donor)
target_link_libraries(hz_r2 PRIVATE hz_lz4_donor)
target_link_libraries(hz_r2 PRIVATE hz_brotli_enc hz_brotli_dec)
target_link_libraries(hz_r2 PRIVATE hz_cmix_dictionary_donor)
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

    add_executable(hz_structure_routing_tests tests/structure_routing_tests.cpp)
    target_link_libraries(hz_structure_routing_tests PRIVATE hz_r2)
    hz_enable_warnings(hz_structure_routing_tests)
    hz_enable_portable_runtime(hz_structure_routing_tests)
    add_test(NAME hz_structure_routing_tests COMMAND hz_structure_routing_tests)
endif()
