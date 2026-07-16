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
# vcpkg still activates the MSVC developer environment (INCLUDE/LIB) for the build, which
# clang-cl (MSVC-ABI) consumes.
#---------------------------------------------------------------------------------------------
if(NOT DEFINED ENV{LLVM_DIR})
    message(FATAL_ERROR "windows-clang-cl.toolchain.cmake: LLVM_DIR is not set; cannot locate clang-cl.")
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

# Resolve vcpkg's stock Windows toolchain relative to the main vcpkg toolchain file
# (<vcpkg>/scripts/buildsystems/vcpkg.cmake -> <vcpkg>/scripts/toolchains/windows.cmake) and
# include it so it provides CMAKE_SYSTEM_NAME, the CRT selection, and all compile/link flags.
get_filename_component(_bsi_vcpkg_scripts_dir "${CMAKE_TOOLCHAIN_FILE}" DIRECTORY)
get_filename_component(_bsi_vcpkg_scripts_dir "${_bsi_vcpkg_scripts_dir}" DIRECTORY)
set(_bsi_windows_toolchain "${_bsi_vcpkg_scripts_dir}/toolchains/windows.cmake")

if(NOT EXISTS "${_bsi_windows_toolchain}")
    message(FATAL_ERROR "windows-clang-cl.toolchain.cmake: could not locate vcpkg's windows.cmake at '${_bsi_windows_toolchain}' (CMAKE_TOOLCHAIN_FILE='${CMAKE_TOOLCHAIN_FILE}').")
endif()

include("${_bsi_windows_toolchain}")

