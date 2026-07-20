# To force a rebuild and a fresh vcpkg binary-cache entry for this triplet, increment the
# number below. Triplet file contents feed into vcpkg's ABI hash, so bumping this value
# invalidates the cached binaries and forces the library to be rebuilt.
# CACHE_BUST = 1

set(VCPKG_TARGET_ARCHITECTURE arm64)
set(VCPKG_CRT_LINKAGE dynamic)
set(VCPKG_LIBRARY_LINKAGE static)

set(VCPKG_CMAKE_SYSTEM_NAME Darwin)
set(VCPKG_OSX_ARCHITECTURES arm64)

# Match our previous build: minizip without crypt/uncrypt support
set(VCPKG_C_FLAGS "${VCPKG_C_FLAGS} -DNOCRYPT -DNOUNCRYPT")
set(VCPKG_CXX_FLAGS "${VCPKG_CXX_FLAGS} -DNOCRYPT -DNOUNCRYPT")
