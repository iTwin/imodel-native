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

# Android keeps its system trust store as individually hashed files in
# /system/etc/security/cacerts (there is no CA bundle file), which is what the previous
# file-by-file build hard-coded via config-android.h. curl's CMake only auto-detects CA
# locations when not cross-compiling, so point it there explicitly; without this, HTTPS
# verification would fall back to iTwinOpenSSL's built-in /etc/ssl paths, which do not
# exist on Android.
list(APPEND VCPKG_CMAKE_CONFIGURE_OPTIONS "-DCURL_CA_PATH=/system/etc/security/cacerts" "-DCURL_CA_BUNDLE=none")

# vcpkg's android toolchain looks for ANDROID_NDK_HOME, but our build sets ANDROID_NDK_ROOT.
if(NOT DEFINED ENV{ANDROID_NDK_HOME} AND DEFINED ENV{ANDROID_NDK_ROOT})
    set(ENV{ANDROID_NDK_HOME} $ENV{ANDROID_NDK_ROOT})
endif()

# Build curl with hidden symbol visibility so its symbols are not exported from the final
# imodeljs.node. This matches the previous file-by-file build (BeCurl.mke set
# GCC_DEFAULT_VISIBILITY=hidden on __unix). (c-ares is not built on Android.)
set(VCPKG_C_FLAGS "${VCPKG_C_FLAGS} -fvisibility=hidden -fvisibility-inlines-hidden")
set(VCPKG_CXX_FLAGS "${VCPKG_CXX_FLAGS} -fvisibility=hidden -fvisibility-inlines-hidden")
