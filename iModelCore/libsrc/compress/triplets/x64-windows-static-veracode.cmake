# To force a rebuild and a fresh vcpkg binary-cache entry for this triplet, increment the
# number below. Triplet file contents feed into vcpkg's ABI hash, so bumping this value
# invalidates the cached binaries and forces the library to be rebuilt.
# CACHE_BUST = 2

# x64-windows-static-veracode: MSVC (Visual Studio cl.exe) toolset, veracode variant. The
# clang (WINDOWS_CLANG) toolset uses x64-windows-static-veracode-clang instead; keeping this
# triplet MSVC-specific (this marker also changes the vcpkg ABI hash) ensures the clang and
# Visual Studio builds never share binary-cache entries.
set(VCPKG_TARGET_ARCHITECTURE x64)
set(VCPKG_CRT_LINKAGE dynamic)
set(VCPKG_LIBRARY_LINKAGE static)

# Match our previous build: minizip without crypt/uncrypt support
set(VCPKG_C_FLAGS "${VCPKG_C_FLAGS} /DNOCRYPT /DNOUNCRYPT")
set(VCPKG_CXX_FLAGS "${VCPKG_CXX_FLAGS} /DNOCRYPT /DNOUNCRYPT")

# Veracode: no runtime checks (cannot explicitly set any -RTC options)
