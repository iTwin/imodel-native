# To force a rebuild and a fresh vcpkg binary-cache entry for this triplet, increment the
# number below. Triplet file contents feed into vcpkg's ABI hash, so bumping this value
# invalidates the cached binaries and forces the library to be rebuilt.
# CACHE_BUST = 0

set(VCPKG_TARGET_ARCHITECTURE arm64)
set(VCPKG_CRT_LINKAGE dynamic)
set(VCPKG_LIBRARY_LINKAGE static)

set(VCPKG_CMAKE_SYSTEM_NAME iOS)
set(VCPKG_OSX_ARCHITECTURES arm64)
set(VCPKG_OSX_SYSROOT iphoneos)
set(VCPKG_OSX_DEPLOYMENT_TARGET 13.0)

# No visibility flags are needed here. FreeType's CMake build already compiles with hidden
# visibility (C_VISIBILITY_PRESET hidden) on non-Windows targets, and its public FT_* API is
# annotated with default visibility (FT_EXPORT). That keeps the public API that
# iTwinFreetype2 consumers (TrueTypeFont.cpp) link against visible, as the previous
# file-by-file build did, while FreeType's internal symbols stay hidden.
