# To force a rebuild and a fresh vcpkg binary-cache entry for this triplet, increment the
# number below. Triplet file contents feed into vcpkg's ABI hash, so bumping this value
# invalidates the cached binaries and forces the library to be rebuilt.
# CACHE_BUST = 3

# x64-windows-static: 64-bit Windows, static library linkage, MSVC (Visual Studio cl.exe)
# toolset. The clang (WINDOWS_CLANG) toolset uses x64-windows-static-clang instead; keeping
# this triplet MSVC-specific (this marker also changes the vcpkg ABI hash) ensures the clang
# and Visual Studio builds never share binary-cache entries.
set(VCPKG_TARGET_ARCHITECTURE x64)
set(VCPKG_CRT_LINKAGE dynamic)
set(VCPKG_LIBRARY_LINKAGE static)
