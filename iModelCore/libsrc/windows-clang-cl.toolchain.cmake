#---------------------------------------------------------------------------------------------
#  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
#  See LICENSE.md in the repository root for full copyright notice.
#---------------------------------------------------------------------------------------------
# windows-clang-cl.toolchain.cmake
#
# Chainload toolchain used by the *-clang Windows overlay triplets. It makes vcpkg build the
# packages with BentleyBuild's own LLVM clang-cl (located via the LLVM_DIR environment
# variable) instead of MSVC cl.exe, so that:
#
#   * the clang (WINDOWS_CLANG) toolset gets its own vcpkg ABI hash and never shares
#     binary-cache entries with the MSVC (Visual Studio) build, and
#   * the cached binaries are genuinely clang-built, matching the compiler the rest of the
#     native Windows build uses.
#
# Setting VCPKG_CHAINLOAD_TOOLCHAIN_FILE *replaces* vcpkg's default windows toolchain, so this
# file establishes the clang-cl toolchain and the CRT runtime-library selection itself.
# clang-cl consumes MSVC-style switches, so the /D..., /MD, etc. flags the triplets and vcpkg
# pass through keep working; /RTC is intentionally NOT used by any *-clang triplet because
# clang-cl does not implement the MSVC runtime checks.
#
# These are static-library builds (no link step), so only the compilers and the archiver are
# set here; the linker is left to CMake's clang-cl defaults.
#---------------------------------------------------------------------------------------------
if(NOT DEFINED ENV{LLVM_DIR})
    message(FATAL_ERROR "windows-clang-cl.toolchain.cmake: LLVM_DIR is not set; cannot locate clang-cl.")
endif()

file(TO_CMAKE_PATH "$ENV{LLVM_DIR}" _bsi_llvm_dir)

if(NOT EXISTS "${_bsi_llvm_dir}/bin/clang-cl.exe")
    message(FATAL_ERROR "windows-clang-cl.toolchain.cmake: clang-cl.exe not found under '${_bsi_llvm_dir}/bin'.")
endif()

set(CMAKE_C_COMPILER   "${_bsi_llvm_dir}/bin/clang-cl.exe")
set(CMAKE_CXX_COMPILER "${_bsi_llvm_dir}/bin/clang-cl.exe")
set(CMAKE_RC_COMPILER  "${_bsi_llvm_dir}/bin/llvm-rc.exe")
set(CMAKE_AR           "${_bsi_llvm_dir}/bin/llvm-lib.exe")

# Mirror vcpkg's default CRT selection (VCPKG_CRT_LINKAGE) so /MD(d) vs /MT(d) matches the
# corresponding MSVC triplet. All current *-clang triplets use the dynamic CRT. Relies on
# CMP0091 (NEW) being in effect, which vcpkg's CMake configuration establishes.
if(NOT DEFINED VCPKG_CRT_LINKAGE OR VCPKG_CRT_LINKAGE STREQUAL "dynamic")
    set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>DLL")
else()
    set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
endif()
