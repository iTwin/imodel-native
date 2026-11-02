# To force a rebuild and a fresh vcpkg binary-cache entry for this triplet, increment the
# number below. Triplet file contents feed into vcpkg's ABI hash, so bumping this value
# invalidates the cached binaries and forces the library to be rebuilt.
# CACHE_BUST = 0

set(VCPKG_TARGET_ARCHITECTURE arm64)
set(VCPKG_CRT_LINKAGE dynamic)
set(VCPKG_LIBRARY_LINKAGE static)
set(VCPKG_CMAKE_SYSTEM_NAME Android)
set(VCPKG_CMAKE_SYSTEM_VERSION 28)
set(VCPKG_MAKE_BUILD_TRIPLET "--host=aarch64-linux-android")
set(VCPKG_CMAKE_CONFIGURE_OPTIONS -DANDROID_ABI=arm64-v8a)

# vcpkg's android toolchain looks for ANDROID_NDK_HOME, but our build sets ANDROID_NDK_ROOT.
if(NOT DEFINED ENV{ANDROID_NDK_HOME} AND DEFINED ENV{ANDROID_NDK_ROOT})
    set(ENV{ANDROID_NDK_HOME} $ENV{ANDROID_NDK_ROOT})
endif()

# No visibility flags are needed here. FreeType's CMake build already compiles with hidden
# visibility (C_VISIBILITY_PRESET hidden) on non-Windows targets, and its public FT_* API is
# annotated with default visibility (FT_EXPORT). That keeps the public API that
# iTwinFreetype2 consumers (TrueTypeFont.cpp) link against visible, as the previous
# file-by-file build did, while FreeType's internal symbols stay hidden.
