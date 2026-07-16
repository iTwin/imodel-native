# x64-windows-static-veracode-clang: veracode + clang-cl variant, built with BentleyBuild's
# LLVM clang-cl via the shared chainload toolchain (../../windows-clang-cl.toolchain.cmake) so
# the clang (WINDOWS_CLANG) toolset gets its own vcpkg ABI hash, separate from the MSVC build.
set(VCPKG_TARGET_ARCHITECTURE x64)
set(VCPKG_CRT_LINKAGE dynamic)
set(VCPKG_LIBRARY_LINKAGE static)

set(VCPKG_CHAINLOAD_TOOLCHAIN_FILE "${CMAKE_CURRENT_LIST_DIR}/../../windows-clang-cl.toolchain.cmake")

# Force full release builds in vcpkg for this triplet (see x64-windows-static-veracode.cmake).
set(VCPKG_BUILD_TYPE release)

# Veracode: no runtime checks (and clang-cl has none regardless).
