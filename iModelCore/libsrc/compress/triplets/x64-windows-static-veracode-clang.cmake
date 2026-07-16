# x64-windows-static-veracode-clang: veracode variant of x64-windows-static-clang, built with
# BentleyBuild's LLVM clang-cl via the shared chainload toolchain
# (../../windows-clang-cl.toolchain.cmake). clang-cl never emits /RTC, so its contents match
# x64-windows-static-clang; it exists so the WINDOWS_CLANG toolset has a veracode-named triplet
# parallel to the MSVC x64-windows-static-veracode, keeping the vcpkg.mki selection symmetric.
set(VCPKG_TARGET_ARCHITECTURE x64)
set(VCPKG_CRT_LINKAGE dynamic)
set(VCPKG_LIBRARY_LINKAGE static)

set(VCPKG_CHAINLOAD_TOOLCHAIN_FILE "${CMAKE_CURRENT_LIST_DIR}/../../windows-clang-cl.toolchain.cmake")

# Match our previous build: minizip without crypt/uncrypt support
set(VCPKG_C_FLAGS "${VCPKG_C_FLAGS} /DNOCRYPT /DNOUNCRYPT")
set(VCPKG_CXX_FLAGS "${VCPKG_CXX_FLAGS} /DNOCRYPT /DNOUNCRYPT")

# Veracode: no runtime checks (and clang-cl has none regardless).
