# To force a rebuild and a fresh vcpkg binary-cache entry for this triplet, increment the
# number below. Triplet file contents feed into vcpkg's ABI hash, so bumping this value
# invalidates the cached binaries and forces the library to be rebuilt.
# CACHE_BUST = 1

set(VCPKG_TARGET_ARCHITECTURE x64)
set(VCPKG_CRT_LINKAGE dynamic)
set(VCPKG_LIBRARY_LINKAGE static)

set(VCPKG_CMAKE_SYSTEM_NAME Linux)

# Build OpenSSL with hidden symbol visibility so its symbols are not exported from the final
# imodeljs.node. This matches the previous file-by-file build (which used
# GCC_DEFAULT_VISIBILITY=hidden) and prevents clashes with the OpenSSL that Node bundles
# (the Node 18 segfault). OpenSSL does not annotate its public API with explicit default
# visibility, so -fvisibility=hidden hides all of its symbols.
set(VCPKG_C_FLAGS "${VCPKG_C_FLAGS} -fvisibility=hidden -fvisibility-inlines-hidden")
set(VCPKG_CXX_FLAGS "${VCPKG_CXX_FLAGS} -fvisibility=hidden -fvisibility-inlines-hidden")
