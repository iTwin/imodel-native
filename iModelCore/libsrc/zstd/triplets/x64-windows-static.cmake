# To force a rebuild and a fresh vcpkg binary-cache entry for this triplet, increment the
# number below. Triplet file contents feed into vcpkg's ABI hash, so bumping this value
# invalidates the cached binaries and forces the library to be rebuilt.
# CACHE_BUST = 1

# x64-windows-static: 64-bit Windows, static library linkage, MSVC (Visual Studio cl.exe)
# toolset. The clang (WINDOWS_CLANG) toolset uses x64-windows-static-clang instead; keeping
# this triplet MSVC-specific (this marker also changes the vcpkg ABI hash) ensures the clang
# and Visual Studio builds never share binary-cache entries.
set(VCPKG_TARGET_ARCHITECTURE x64)
set(VCPKG_CRT_LINKAGE dynamic)
set(VCPKG_LIBRARY_LINKAGE static)

# Build only the release configuration, and link it in every configuration (see Zstd.mke).
# Bentley links the release CRT even in DEBUG builds, but zstd's Debug configuration would use
# the debug CRT: zstd's CMake sets policy version 3.13, so it ignores CMAKE_MSVC_RUNTIME_LIBRARY
# (CMP0091) and the Debug configuration cannot be moved onto the release CRT.
set(VCPKG_BUILD_TYPE release)

# Leave out the code for the legacy zstd formats and for multithreaded compression, which nothing uses.
set(VCPKG_CMAKE_CONFIGURE_OPTIONS "-DZSTD_LEGACY_SUPPORT=OFF" "-DZSTD_MULTITHREAD_SUPPORT=OFF")
