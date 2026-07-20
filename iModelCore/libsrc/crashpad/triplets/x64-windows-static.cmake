# To force a rebuild and a fresh vcpkg binary-cache entry for this triplet, increment the
# number below. Triplet file contents feed into vcpkg's ABI hash, so bumping this value
# invalidates the cached binaries and forces the library to be rebuilt.
# CACHE_BUST = 1

# x64-windows-static: MSVC (Visual Studio cl.exe) toolset. The clang (WINDOWS_CLANG) toolset
# uses x64-windows-static-clang instead; keeping this triplet MSVC-specific (this marker also
# changes the vcpkg ABI hash) ensures the clang and Visual Studio builds never share
# binary-cache entries.
set(VCPKG_TARGET_ARCHITECTURE x64)
set(VCPKG_CRT_LINKAGE dynamic)
set(VCPKG_LIBRARY_LINKAGE static)

# Force full release builds in vcpkg for this triplet.
# Reason: bmake link settings in this pipeline use the release CRT, so crashpad must be
# produced as release artifacts to avoid CRT and iterator-debug-level link mismatches.
# Note: NODEFAULTLIB on the link command can fix the problem and allow debug builds
# from vcpkg to work, but crashpad calls debug-only CRT functionality when compiled in
# debug mode, so that doesn't work here.
set(VCPKG_BUILD_TYPE release)

# Match BentleyBuild runtime behavior for Windows third-party static libs.
set(VCPKG_C_FLAGS_DEBUG "${VCPKG_C_FLAGS_DEBUG} /RTCsu")
set(VCPKG_CXX_FLAGS_DEBUG "${VCPKG_CXX_FLAGS_DEBUG} /RTCsu")
