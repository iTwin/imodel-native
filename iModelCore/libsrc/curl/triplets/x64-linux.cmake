# To force a rebuild and a fresh vcpkg binary-cache entry for this triplet, increment the
# number below. Triplet file contents feed into vcpkg's ABI hash, so bumping this value
# invalidates the cached binaries and forces the library to be rebuilt.
# CACHE_BUST = 1

set(VCPKG_TARGET_ARCHITECTURE x64)
set(VCPKG_CRT_LINKAGE dynamic)
set(VCPKG_LIBRARY_LINKAGE static)

set(VCPKG_CMAKE_SYSTEM_NAME Linux)

# Build curl and c-ares with hidden symbol visibility so their symbols are not exported from
# the final imodeljs.node. This matches the previous file-by-file build (BeCurl.mke set
# GCC_DEFAULT_VISIBILITY=hidden on __unix) and prevents our c-ares symbols from clashing with
# the c-ares that Node bundles (which caused a segmentation fault).
set(VCPKG_C_FLAGS "${VCPKG_C_FLAGS} -fvisibility=hidden -fvisibility-inlines-hidden")
set(VCPKG_CXX_FLAGS "${VCPKG_CXX_FLAGS} -fvisibility=hidden -fvisibility-inlines-hidden")

# arc4random()/arc4random_buf() were first versioned at GLIBC_2.36. If vcpkg builds curl/c-ares
# on a glibc >= 2.36 host, CMake detects them and the objects gain a GLIBC_2.36 symbol reference,
# so imodeljs.node fails to load on older-glibc runtimes ("version `GLIBC_2.36' not found"). Force
# the checks off so curl (HAVE_ARC4RANDOM) and c-ares (HAVE_ARC4RANDOM_BUF) fall back to
# getrandom()/dev/urandom (glibc <= 2.25), matching the old file-by-file build's portability
# baseline. Pre-defining the result vars makes CMake's check_symbol_exists skip the probe.
list(APPEND VCPKG_CMAKE_CONFIGURE_OPTIONS "-DHAVE_ARC4RANDOM=OFF" "-DHAVE_ARC4RANDOM_BUF=OFF")
