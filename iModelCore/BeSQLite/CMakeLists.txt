cmake_minimum_required(VERSION 3.20)
project(iTwinSQLite C CXX)

# BentleyBuild integration
list(APPEND CMAKE_MODULE_PATH "$ENV{SrcRoot}/BentleyBuild/cmake")
include(BentleyBuild)

set(CXX_SOURCES
    BeSQLiteCurlHandleConfig.cpp
    BeSQLite.cpp
    ChangeSet.cpp
    CloudSqlite.cpp
    ChangesetFile.cpp
    BeSQLiteProfileManager.cpp
    Profiler.cpp
    VirtualTab.cpp
    BeLzma.cpp
    BeBriefcaseBasedIdSequence.cpp
)

# Bundled third-party C sources (part of the BeSQLite library)
set(C_SOURCES
    SQLite/bentley-sqlite.c
    SQLite/bcvutil.c
    SQLite/bcvlog.c
    SQLite/simplexml.c
    SQLite/bcvmodule.c
    SQLite/bcvencrypt.c
)

if(CREATE_STATIC_LIBRARIES)
    bb_add_library(iTwinSQLite STATIC ${CXX_SOURCES} ${C_SOURCES})
else()
    bb_add_library(iTwinSQLite SHARED ${CXX_SOURCES} ${C_SOURCES})
endif()

# Generate build version header (prg.h) — used by BeSQLite.cpp
bb_generate_prg_header()

target_include_directories(iTwinSQLite PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}"
    "${CMAKE_CURRENT_SOURCE_DIR}/PublicAPI"
    "${CMAKE_CURRENT_SOURCE_DIR}/SQLite"
    "${BB_PRG_H_DIR}"
)

# ICU headers live under VendorAPI/icu4c/unicode/ but sources use bare <unicode/...> includes
target_include_directories(iTwinSQLite BEFORE PRIVATE "${BB_VENDOR_API_DIR}/icu4c")

target_compile_definitions(iTwinSQLite PRIVATE
    __BE_SQLITE_HOST_DLL__
    SQLITE_BCV_CURL_HANDLE_CONFIG=besqlite_bcv_curl_handle_config
)

# Suppress warnings on the bundled C sources
set_source_files_properties(${C_SOURCES} PROPERTIES
    COMPILE_OPTIONS "-w"
)

bb_target_include_subpart_api(iTwinSQLite BeSQLite)
bb_target_include_subpart_api(iTwinSQLite Bentley)

# Link dependencies
bb_find_subpart_library(BEZLIB_LIB BeZlib)
bb_find_subpart_library(SNAPPY_LIB snappy)
bb_find_subpart_library(LZMA_LIB lzma)
bb_find_subpart_library(BENTLEY_LIB iTwinBentley)
bb_find_subpart_library(CURL_LIB iTwinCurl)
bb_find_subpart_library(OPENSSL_LIB iTwinOpenSSL)
bb_find_subpart_library(RE2_LIB iTwin_google_re2)
bb_find_subpart_library(ICU4C_LIB iTwinIcu4c)

target_link_libraries(iTwinSQLite PRIVATE
    ${BEZLIB_LIB}
    ${SNAPPY_LIB}
    ${LZMA_LIB}
    ${BENTLEY_LIB}
    ${CURL_LIB}
    ${OPENSSL_LIB}
    ${RE2_LIB}
    ${ICU4C_LIB}
)

# Resource delivery (replaces BeSQLite.prewire.mke)
bb_deliver_public_api("${CMAKE_CURRENT_SOURCE_DIR}/PublicAPI" BeSQLite)
bb_deliver_file("Delivery/sqlite-license.txt" "${CMAKE_CURRENT_SOURCE_DIR}/SQLite/sqlite-notice.txt")
