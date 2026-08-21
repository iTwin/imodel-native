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

# Build BOTH configs. Bentley bmake links the release CRT even in DEBUG, so force the vcpkg
# Debug config onto the release CRT (/MD, _ITERATOR_DEBUG_LEVEL=0) to avoid CRT
# ('MDd_DynamicDebug' vs 'MD_DynamicRelease') and _ITERATOR_DEBUG_LEVEL (2 vs 0) LNK2038
# mismatches against our release-compiled BePugiXml wrapper object, while keeping the Debug
# config's asserts (no NDEBUG) and unoptimized codegen. pugixml honors a cache-set
# CMAKE_MSVC_RUNTIME_LIBRARY (CMP0091); vcpkg_cmake_configure appends
# VCPKG_CMAKE_CONFIGURE_OPTIONS_DEBUG at the pinned baseline.
set(VCPKG_CMAKE_CONFIGURE_OPTIONS_DEBUG
    "-DCMAKE_MSVC_RUNTIME_LIBRARY=MultiThreadedDLL")
# Belt-and-suspenders: /MD already defaults IDL=0, but pin it explicitly.
set(VCPKG_CXX_FLAGS_DEBUG "${VCPKG_CXX_FLAGS_DEBUG} -D_ITERATOR_DEBUG_LEVEL=0")
set(VCPKG_C_FLAGS_DEBUG   "${VCPKG_C_FLAGS_DEBUG} -D_ITERATOR_DEBUG_LEVEL=0")
