# To force a rebuild and a fresh vcpkg binary-cache entry for this triplet, increment the
# number below. Triplet file contents feed into vcpkg's ABI hash, so bumping this value
# invalidates the cached binaries and forces the library to be rebuilt.
# CACHE_BUST = 0

set(VCPKG_TARGET_ARCHITECTURE arm64)
set(VCPKG_CRT_LINKAGE dynamic)
set(VCPKG_LIBRARY_LINKAGE static)

set(VCPKG_CMAKE_SYSTEM_NAME Darwin)
set(VCPKG_OSX_ARCHITECTURES arm64)
set(VCPKG_OSX_DEPLOYMENT_TARGET 15.0)

# curl's OpenSSL backend verifies the peer certificate chain against the macOS system trust
# store (Keychain) via Apple SecTrust. CURL_HIDDEN_SYMBOLS normally marks curl's public API with
# default visibility, overriding the triplet's hidden visibility flags below. Disable that
# selective hiding so all symbols in the static library remain private to imodeljs.node. These
# options are also passed to curl's transitive cmake deps (zlib, c-ares), where they are unused.
set(VCPKG_CMAKE_CONFIGURE_OPTIONS "-DUSE_APPLE_SECTRUST=ON" "-DCURL_HIDDEN_SYMBOLS=OFF")

# Build curl and c-ares with hidden symbol visibility so their symbols are not exported from
# the final imodeljs.node. This matches the previous file-by-file build (BeCurl.mke set
# GCC_DEFAULT_VISIBILITY=hidden on __unix) and prevents our c-ares symbols from clashing with
# the c-ares that Node bundles (which caused a segmentation fault).
set(VCPKG_C_FLAGS "${VCPKG_C_FLAGS} -fvisibility=hidden -fvisibility-inlines-hidden")
set(VCPKG_CXX_FLAGS "${VCPKG_CXX_FLAGS} -fvisibility=hidden -fvisibility-inlines-hidden")
