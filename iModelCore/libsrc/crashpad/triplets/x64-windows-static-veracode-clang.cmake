# To force a rebuild and a fresh vcpkg binary-cache entry for this triplet, increment the
# number below. Triplet file contents feed into vcpkg's ABI hash, so bumping this value
# invalidates the cached binaries and forces the library to be rebuilt.
# CACHE_BUST = 1

# x64-windows-static-veracode-clang: veracode + clang-cl variant, built with BentleyBuild's
# LLVM clang-cl via the shared chainload toolchain (../../windows-clang-cl.toolchain.cmake) so
# the clang (WINDOWS_CLANG) toolset gets its own vcpkg ABI hash, separate from the MSVC build.
set(VCPKG_TARGET_ARCHITECTURE x64)
set(VCPKG_CRT_LINKAGE dynamic)
set(VCPKG_LIBRARY_LINKAGE static)

set(VCPKG_CHAINLOAD_TOOLCHAIN_FILE "${CMAKE_CURRENT_LIST_DIR}/../../windows-clang-cl.toolchain.cmake")

# Setting VCPKG_CHAINLOAD_TOOLCHAIN_FILE disables vcpkg's automatic Visual Studio (vcvars)
# environment setup; re-enable it so clang-cl still gets the MSVC SDK headers and CRT import
# libraries (INCLUDE/LIB). Without it, vcpkg's compiler detection fails ("unable to detect the
# active compiler's information").
set(VCPKG_LOAD_VCVARS_ENV ON)

# vcpkg scrubs the build environment; whitelist LLVM_DIR so the chainload toolchain can locate
# clang-cl via $ENV{LLVM_DIR} in every phase (detect_compiler, compiler-ABI try_compile, build).
set(VCPKG_ENV_PASSTHROUGH "LLVM_DIR")

# Force full release builds in vcpkg for this triplet (see x64-windows-static-veracode.cmake).
set(VCPKG_BUILD_TYPE release)

# Veracode: no runtime checks (and clang-cl has none regardless).
