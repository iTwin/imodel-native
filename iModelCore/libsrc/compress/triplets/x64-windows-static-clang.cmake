# x64-windows-static-clang: 64-bit Windows, static library linkage, built with BentleyBuild's
# LLVM clang-cl via the shared chainload toolchain (../../windows-clang-cl.toolchain.cmake).
# The clang (WINDOWS_CLANG) toolset selects this triplet instead of x64-windows-static so it
# gets its own vcpkg ABI hash and never shares binary-cache entries with the MSVC build.
set(VCPKG_TARGET_ARCHITECTURE x64)
set(VCPKG_CRT_LINKAGE dynamic)
set(VCPKG_LIBRARY_LINKAGE static)

set(VCPKG_CHAINLOAD_TOOLCHAIN_FILE "${CMAKE_CURRENT_LIST_DIR}/../../windows-clang-cl.toolchain.cmake")

# Match our previous build: minizip without crypt/uncrypt support
set(VCPKG_C_FLAGS "${VCPKG_C_FLAGS} /DNOCRYPT /DNOUNCRYPT")
set(VCPKG_CXX_FLAGS "${VCPKG_CXX_FLAGS} /DNOCRYPT /DNOUNCRYPT")

# The chainload toolchain includes vcpkg's windows.cmake, which still adds /RTC1 to debug
# builds. clang-cl does not implement the MSVC runtime checks and ignores /RTC1 with an
# "unused argument" warning, so no /RTC handling is needed here.
