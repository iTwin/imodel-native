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
#   * the clang (WINDOWS_CLANG) toolset gets its own vcpkg ABI hash (the compiler binary is
#     hashed by vcpkg's detect_compiler) and never shares binary-cache entries with the MSVC
#     (Visual Studio) build, and
#   * the cached binaries are genuinely clang-built, matching the compiler the rest of the
#     native Windows build uses.
#
# IMPORTANT: we only want to swap the *compiler*, not lose vcpkg's Windows setup. vcpkg's stock
# windows toolchain (scripts/toolchains/windows.cmake) sets CMAKE_SYSTEM_NAME/PROCESSOR,
# CMAKE_CROSSCOMPILING, the CRT runtime library, and all the /MD, /Z7, /O2, /RTC1, … compile
# and link flags, and it already special-cases clang-cl (e.g. it omits /MP). It must run, or
# vcpkg's compiler detection fails ("unable to detect the active compiler's information").
# Therefore we select clang-cl and then *include* windows.cmake rather than replacing it.
# NOTE: setting VCPKG_CHAINLOAD_TOOLCHAIN_FILE also disables vcpkg's automatic vcvars setup, so
# each *-clang triplet sets VCPKG_LOAD_VCVARS_ENV ON to restore the MSVC developer environment
# (INCLUDE/LIB) that clang-cl (MSVC-ABI) needs to find the Windows SDK and CRT.
#---------------------------------------------------------------------------------------------
if(NOT DEFINED ENV{LLVM_DIR})
    message(FATAL_ERROR "windows-clang-cl.toolchain.cmake: LLVM_DIR is not set; cannot locate clang-cl. "
        "The *-clang triplets must whitelist it via VCPKG_ENV_PASSTHROUGH so it survives vcpkg's "
        "scrubbed build environment.")
endif()

file(TO_CMAKE_PATH "$ENV{LLVM_DIR}" _bsi_llvm_dir)

if(NOT EXISTS "${_bsi_llvm_dir}/bin/clang-cl.exe")
    message(FATAL_ERROR "windows-clang-cl.toolchain.cmake: clang-cl.exe not found under '${_bsi_llvm_dir}/bin'.")
endif()

# Select clang-cl BEFORE including windows.cmake so its clang-cl detection (e.g. omitting /MP)
# takes effect. Setting CMAKE_C[XX]_COMPILER directly (rather than via the CC/CXX env vars)
# tolerates the space in the default install path "C:/Program Files/LLVM".
set(CMAKE_C_COMPILER   "${_bsi_llvm_dir}/bin/clang-cl.exe")
set(CMAKE_CXX_COMPILER "${_bsi_llvm_dir}/bin/clang-cl.exe")

# Resolve vcpkg's stock Windows toolchain (<vcpkg>/scripts/toolchains/windows.cmake) from the
# vcpkg root. CMAKE_TOOLCHAIN_FILE is NOT reliable here: during the compiler-ABI try_compile
# CMake re-runs this toolchain with CMAKE_TOOLCHAIN_FILE empty, which previously produced the
# bogus path "/toolchains/windows.cmake". vcpkg exposes its root differently per phase:
#   * port configure       -> _VCPKG_ROOT_DIR is passed as a -D cache entry
#   * try_compile (ABI)     -> Z_VCPKG_ROOT_DIR is forwarded via CMAKE_TRY_COMPILE_PLATFORM_VARIABLES
# (this chainload runs at vcpkg.cmake:208, before Z_VCPKG_ROOT_DIR is computed at :398, so we
# cannot rely on Z_VCPKG_ROOT_DIR during the outer port configure). Prefer whichever is set.
set(_bsi_vcpkg_root "")
if(DEFINED Z_VCPKG_ROOT_DIR AND Z_VCPKG_ROOT_DIR)
    set(_bsi_vcpkg_root "${Z_VCPKG_ROOT_DIR}")
elseif(DEFINED _VCPKG_ROOT_DIR AND _VCPKG_ROOT_DIR)
    set(_bsi_vcpkg_root "${_VCPKG_ROOT_DIR}")
endif()

if(NOT _bsi_vcpkg_root)
    message(FATAL_ERROR "windows-clang-cl.toolchain.cmake: could not determine the vcpkg root "
        "(neither Z_VCPKG_ROOT_DIR nor _VCPKG_ROOT_DIR is set).")
endif()

file(TO_CMAKE_PATH "${_bsi_vcpkg_root}" _bsi_vcpkg_root)
set(_bsi_windows_toolchain "${_bsi_vcpkg_root}/scripts/toolchains/windows.cmake")

if(NOT EXISTS "${_bsi_windows_toolchain}")
    message(FATAL_ERROR "windows-clang-cl.toolchain.cmake: could not locate vcpkg's windows.cmake at '${_bsi_windows_toolchain}' (vcpkg root='${_bsi_vcpkg_root}').")
endif()

include("${_bsi_windows_toolchain}")

