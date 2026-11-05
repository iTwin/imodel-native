# To force a rebuild and a fresh vcpkg binary-cache entry for this triplet, increment the
# number below. Triplet file contents feed into vcpkg's ABI hash, so bumping this value
# invalidates the cached binaries and forces the library to be rebuilt.
# CACHE_BUST = 3

# x64-windows-static-veracode: MSVC (Visual Studio cl.exe) toolset, veracode variant. The
# clang (WINDOWS_CLANG) toolset uses x64-windows-static-veracode-clang instead; keeping this
# triplet MSVC-specific (this marker also changes the vcpkg ABI hash) ensures the clang and
# Visual Studio builds never share binary-cache entries.
set(VCPKG_TARGET_ARCHITECTURE x64)
set(VCPKG_CRT_LINKAGE dynamic)
set(VCPKG_LIBRARY_LINKAGE static)

# Match our previous build: minizip without crypt/uncrypt support
set(VCPKG_C_FLAGS "${VCPKG_C_FLAGS} /DNOCRYPT /DNOUNCRYPT")
set(VCPKG_CXX_FLAGS "${VCPKG_CXX_FLAGS} /DNOCRYPT /DNOUNCRYPT")

# Build BOTH configs. Bentley bmake links the release CRT even in DEBUG, so force the vcpkg
# Debug config onto the release CRT (/MD) to avoid MSVCRTD-vs-MSVCRT mismatches when zsd and
# minizipsd are merged into BeZlib and linked into our release-CRT DEBUG binaries, while
# keeping the Debug config's asserts (no NDEBUG) and unoptimized codegen. zlib
# (cmake_minimum_required 3.12...3.31) and minizip (3.25) both honor a cache-set
# CMAKE_MSVC_RUNTIME_LIBRARY (CMP0091). No _ITERATOR_DEBUG_LEVEL pin is needed here: both
# ports are pure C, so no STL headers are involved.
set(VCPKG_CMAKE_CONFIGURE_OPTIONS_DEBUG
    "-DCMAKE_MSVC_RUNTIME_LIBRARY=MultiThreadedDLL")

# Veracode: no runtime checks (cannot explicitly set any -RTC options)
