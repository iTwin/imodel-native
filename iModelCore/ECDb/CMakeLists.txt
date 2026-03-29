#---------------------------------------------------------------------------------------------
#  Temporary CMake build for ECDb library + tests.
#  Uses pre-built dependency .a libraries from the BentleyBuild debug output.
#
#  Usage:
#    cd iModelCore/ECDb/cmake-build
#    cmake .. -DCMAKE_BUILD_TYPE=Debug
#    cmake --build . --target ECDb-Tests -- -j$(sysctl -n hw.ncpu)
#    # Run tests:
#    ./ECDb-Tests --gtest_filter="ECSqlStatement*"
#
#  This is NOT a production build system. It exists solely to iterate on
#  ECSqlAst / ECSqlCodeGen changes and verify existing tests still pass.
#---------------------------------------------------------------------------------------------
cmake_minimum_required(VERSION 3.20)
project(ECDb-Temp LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# ── Paths ──────────────────────────────────────────────────────────────────────
set(IMODEL_NATIVE  "${CMAKE_SOURCE_DIR}/../..")                      # iModelCore parent
set(ECDB_SRC       "${CMAKE_SOURCE_DIR}/ECDb")                       # ECDb impl sources
set(ECDB_ECSQL     "${ECDB_SRC}/ECSql")
set(ECDB_PARSER    "${ECDB_ECSQL}/Parser")
set(ECDB_PUBLIC    "${CMAKE_SOURCE_DIR}/PublicAPI")
set(ECDB_TESTS     "${CMAKE_SOURCE_DIR}/Tests")

# Pre-built context from BentleyBuild (debug)
set(BUILD_CTX      "$ENV{HOME}/bsw/git-a1/out/debug/MacOSARM64/static/BuildContexts/ECDb")
set(PREBUILT_LIBS  "${BUILD_CTX}/SubParts/Libs")
set(PUBLIC_API     "${BUILD_CTX}/PublicAPI")
set(VENDOR_API     "${BUILD_CTX}/VendorAPI")

# ── Include paths ──────────────────────────────────────────────────────────────
# Order matters: ECDb private headers first, then public APIs from build context
set(ECDB_INCLUDE_DIRS
    ${ECDB_SRC}                       # ECDb internal headers (ECDbPch.h, ClassMap.h, etc.)
    ${ECDB_ECSQL}                     # ECSql subdirectory headers
    ${ECDB_PARSER}                    # Parser subdirectory (SqlScan.h, etc.)
    ${ECDB_PUBLIC}                    # ECDb public API (ECDb/ECDb.h etc.)
    ${PUBLIC_API}                     # All public APIs: <Bentley/...>, <BeSQLite/...>, <ECObjects/...>, etc.
    ${VENDOR_API}                     # Vendor APIs: <rapidjson/...>, <gtest/...>, <flatbuffers/...>, etc.
    ${VENDOR_API}/icu4c               # ICU uses bare includes like <unicode/uchar.h>
)

# ── Compile definitions ────────────────────────────────────────────────────────
set(ECDB_DEFINES
    __ECDB_BUILD__
    __unix__
    CREATE_STATIC_LIBRARIES
    DEBUG=1
    BETHREAD_USE_PTHREAD
    BENTLEYCONFIG_NO_JAVASCRIPT
)

# ── ECDb library source files ─────────────────────────────────────────────────
# Root .cpp files (alphabetical, matching the .o files from the existing build)
file(GLOB ECDB_ROOT_SOURCES   "${ECDB_SRC}/*.cpp")
file(GLOB ECDB_ECSQL_SOURCES  "${ECDB_ECSQL}/*.cpp")
file(GLOB ECDB_PARSER_SOURCES "${ECDB_PARSER}/*.cpp")

set(ECDB_ALL_SOURCES
    ${ECDB_ROOT_SOURCES}
    ${ECDB_ECSQL_SOURCES}
    ${ECDB_PARSER_SOURCES}
)

# Exclude the PCH compilation unit (it's just `#include "ECDbPch.h"`)
list(FILTER ECDB_ALL_SOURCES EXCLUDE REGEX "ECDbPch\\.cpp$")

# ── ECDb static library ───────────────────────────────────────────────────────
add_library(ECDb STATIC ${ECDB_ALL_SOURCES})
target_include_directories(ECDb PRIVATE ${ECDB_INCLUDE_DIRS})
target_compile_definitions(ECDb PRIVATE ${ECDB_DEFINES})
target_compile_options(ECDb PRIVATE
    -w                        # Suppress all warnings (matches real build's -w for speed)
    -fvisibility=hidden
    -fvisibility-inlines-hidden
    -fexceptions
    -fasm-blocks
    -frtti
    -stdlib=libc++
)

# ── Pre-built dependency libraries ─────────────────────────────────────────────
# Order matters for static linking: dependents before dependencies
set(PREBUILT_LIB_LIST
    ${PREBUILT_LIBS}/libiTwinSQLite.a
    ${PREBUILT_LIBS}/libiTwinECObjects.a
    ${PREBUILT_LIBS}/libiTwinBentley.a
    ${PREBUILT_LIBS}/libiTwinUnits.a
    ${PREBUILT_LIBS}/libiTwinGeom.a
    ${PREBUILT_LIBS}/libiTwinGeomSerialization.a
    ${PREBUILT_LIBS}/libiTwinJsonCpp.a
    ${PREBUILT_LIBS}/libiTwinXml.a
    ${PREBUILT_LIBS}/libiTwin_google_re2.a
    ${PREBUILT_LIBS}/libiTwinPugixml.a
    ${PREBUILT_LIBS}/libiTwinLibxml2.a
    ${PREBUILT_LIBS}/libiTwinIcu4c.a
    ${PREBUILT_LIBS}/libiTwinOpenSSL.a
    ${PREBUILT_LIBS}/libiTwinCurl.a
    ${PREBUILT_LIBS}/libiTwinNapi.a
    ${PREBUILT_LIBS}/libBeZlib.a
    ${PREBUILT_LIBS}/liblzma.a
    ${PREBUILT_LIBS}/libsnappy.a
    ${PREBUILT_LIBS}/libproxyres.a
)

# ── Test executable ────────────────────────────────────────────────────────────
# BackDoor (test infrastructure)
file(GLOB TEST_BACKDOOR_SOURCES "${ECDB_TESTS}/BackDoor/*.cpp")

# NonPublished (actual tests)
file(GLOB TEST_NONPUBLISHED_SOURCES "${ECDB_TESTS}/NonPublished/*.cpp")

# Test runner main()
set(TEST_MAIN "${IMODEL_NATIVE}/iModelCore/BeGTest/gtest/BeGTestExe.cpp")

add_executable(ECDb-Tests
    ${TEST_MAIN}
    ${TEST_BACKDOOR_SOURCES}
    ${TEST_NONPUBLISHED_SOURCES}
)

target_include_directories(ECDb-Tests PRIVATE
    ${ECDB_INCLUDE_DIRS}
    ${ECDB_TESTS}/BackDoor/PublicAPI/BackDoor/ECDb   # ECDbTestFixture.h etc.
)

target_compile_definitions(ECDb-Tests PRIVATE
    ${ECDB_DEFINES}
    USE_GTEST=1
)
target_compile_options(ECDb-Tests PRIVATE
    -w
    -fvisibility=hidden
    -fvisibility-inlines-hidden
    -fexceptions
    -fasm-blocks
    -frtti
    -stdlib=libc++
)

target_link_libraries(ECDb-Tests PRIVATE
    ECDb
    ${PREBUILT_LIBS}/libgtest.a
    ${PREBUILT_LIBS}/libgmock.a
    ${PREBUILT_LIB_LIST}
    "-framework CoreFoundation"
    "-framework Security"
    "-framework SystemConfiguration"
    "-framework CoreServices"
    dl
    pthread
    z
)

# ── Convenience: compile-only target for just the new AST files ────────────────
# Use: cmake --build . --target check-ast
add_library(check-ast OBJECT
    ${ECDB_ECSQL}/ECSqlAst.cpp
    ${ECDB_ECSQL}/ECSqlCodeGen.cpp
)
target_include_directories(check-ast PRIVATE ${ECDB_INCLUDE_DIRS})
target_compile_definitions(check-ast PRIVATE ${ECDB_DEFINES})
target_compile_options(check-ast PRIVATE
    -w
    -fvisibility=hidden
    -fvisibility-inlines-hidden
    -fexceptions
    -fasm-blocks
    -frtti
    -stdlib=libc++
)

# ── Post-build: symlink test assets ───────────────────────────────────────────
# Tests expect Assets/ dir next to the executable
set(PRODUCT_ASSETS "$ENV{HOME}/bsw/git-a1/out/debug/MacOSARM64/Product/ECDb-Tests/Assets")
add_custom_command(TARGET ECDb-Tests POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E create_symlink "${PRODUCT_ASSETS}" "$<TARGET_FILE_DIR:ECDb-Tests>/Assets"
    COMMENT "Symlinking test assets"
)
