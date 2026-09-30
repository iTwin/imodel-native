# To force a rebuild and a fresh vcpkg binary-cache entry for this triplet, increment the
# number below. Triplet file contents feed into vcpkg's ABI hash, so bumping this value
# invalidates the cached binaries and forces the library to be rebuilt.
# CACHE_BUST = 0

# x64-windows-static-md: 64-bit Windows, static library linkage, dynamic CRT (/MD).
# curl is re-linked into iTwinCurl.dll (which uses the dynamic CRT, msvcrt.lib) and links
# against iTwinOpenSSL.dll (also /MD), so curl must use the dynamic-CRT static triplet to keep
# CRT linkage consistent and avoid "unresolved external" errors for CRT symbols.
#
# This is the MSVC (Visual Studio cl.exe) toolset variant. The clang (WINDOWS_CLANG) toolset
# uses x64-windows-static-md-clang instead; keeping this triplet MSVC-specific (this marker
# also changes the vcpkg ABI hash) ensures the clang and Visual Studio builds never share
# binary-cache entries.
set(VCPKG_TARGET_ARCHITECTURE x64)
set(VCPKG_CRT_LINKAGE dynamic)
set(VCPKG_LIBRARY_LINKAGE static)
