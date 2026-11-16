# To force a rebuild and a fresh vcpkg binary-cache entry for this triplet, increment the
# number below. Triplet file contents feed into vcpkg's ABI hash, so bumping this value
# invalidates the cached binaries and forces the library to be rebuilt.
# CACHE_BUST = 1

set(VCPKG_TARGET_ARCHITECTURE arm64)
set(VCPKG_CRT_LINKAGE dynamic)
set(VCPKG_LIBRARY_LINKAGE static)

set(VCPKG_CMAKE_SYSTEM_NAME Darwin)
set(VCPKG_OSX_ARCHITECTURES arm64)
set(VCPKG_OSX_DEPLOYMENT_TARGET 15.0)

# Leave out the code for the legacy zstd formats and for multithreaded compression, which nothing uses.
set(VCPKG_CMAKE_CONFIGURE_OPTIONS "-DZSTD_LEGACY_SUPPORT=OFF" "-DZSTD_MULTITHREAD_SUPPORT=OFF")

# Hide zstd's symbols, so that they cannot bind to another zstd loaded in the same process (Node
# bundles one). zstd gives its API default visibility unless its *_VISIBLE options are set, so
# -fvisibility=hidden alone, which hides the rest, would leave the API visible.
list(APPEND VCPKG_CMAKE_CONFIGURE_OPTIONS "-DZSTDLIB_VISIBLE=hidden" "-DZSTDERRORLIB_VISIBLE=hidden" "-DZDICTLIB_VISIBLE=hidden")
set(VCPKG_C_FLAGS "${VCPKG_C_FLAGS} -fvisibility=hidden")
set(VCPKG_CXX_FLAGS "${VCPKG_CXX_FLAGS} -fvisibility=hidden")
