# To force a rebuild and a fresh vcpkg binary-cache entry for this triplet, increment the
# number below. Triplet file contents feed into vcpkg's ABI hash, so bumping this value
# invalidates the cached binaries and forces the library to be rebuilt.
# CACHE_BUST = 1

# x64-windows-static-clang: 64-bit Windows, static library linkage, built with BentleyBuild's
# LLVM clang-cl via the shared chainload toolchain (../../windows-clang-cl.toolchain.cmake).
# The clang (WINDOWS_CLANG) toolset selects this triplet instead of x64-windows-static so it
# gets its own vcpkg ABI hash and never shares binary-cache entries with the MSVC build.
set(VCPKG_TARGET_ARCHITECTURE x64)
set(VCPKG_CRT_LINKAGE dynamic)
set(VCPKG_LIBRARY_LINKAGE static)

set(VCPKG_CHAINLOAD_TOOLCHAIN_FILE "${CMAKE_CURRENT_LIST_DIR}/../../windows-clang-cl.toolchain.cmake")

# vcpkg builds ports in a scrubbed environment and does not forward arbitrary environment
# variables into the toolchain/build process. The chainload toolchain locates clang-cl via
# $ENV{LLVM_DIR}, so that variable must be whitelisted or it arrives empty and configuration
# fails with "LLVM_DIR is not set". VCPKG_ENV_PASSTHROUGH makes it available for the whole
# build (including the detect_compiler and compiler-ABI try_compile steps).
set(VCPKG_ENV_PASSTHROUGH "LLVM_DIR")

# Setting VCPKG_CHAINLOAD_TOOLCHAIN_FILE disables vcpkg's automatic Visual Studio (vcvars)
# environment setup; re-enable it so clang-cl still gets the MSVC SDK headers and CRT import
# libraries (INCLUDE/LIB). Without it, vcpkg's compiler detection fails ("unable to detect the
# active compiler's information").
set(VCPKG_LOAD_VCVARS_ENV ON)

# Match our previous build: minizip without crypt/uncrypt support
set(VCPKG_C_FLAGS "${VCPKG_C_FLAGS} /DNOCRYPT /DNOUNCRYPT")
set(VCPKG_CXX_FLAGS "${VCPKG_CXX_FLAGS} /DNOCRYPT /DNOUNCRYPT")

# The chainload toolchain includes vcpkg's windows.cmake, which still adds /RTC1 to debug
# builds. clang-cl does not implement the MSVC runtime checks and ignores /RTC1 with an
# "unused argument" warning, so no /RTC handling is needed here.
