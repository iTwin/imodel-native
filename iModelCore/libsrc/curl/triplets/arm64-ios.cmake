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

# curl's OpenSSL backend verifies the peer certificate chain against the iOS system trust store
# (Keychain) via Apple SecTrust. vcpkg_cmake_configure forwards this triplet option to curl's
# CMake, which then auto-links Security/CoreFoundation/CoreServices. The option is also passed
# to curl's transitive cmake deps (zlib), where it is an unused, harmless define.
set(VCPKG_CMAKE_CONFIGURE_OPTIONS "-DUSE_APPLE_SECTRUST=ON")

# Build curl with hidden symbol visibility so its symbols are not exported from the final
# imodeljs.node. This matches the previous file-by-file build (BeCurl.mke set
# GCC_DEFAULT_VISIBILITY=hidden on __unix). curl does not annotate its public API with default
# visibility in the static build, so -fvisibility=hidden hides its symbols.
set(VCPKG_C_FLAGS "${VCPKG_C_FLAGS} -fvisibility=hidden -fvisibility-inlines-hidden")
set(VCPKG_CXX_FLAGS "${VCPKG_CXX_FLAGS} -fvisibility=hidden -fvisibility-inlines-hidden")
