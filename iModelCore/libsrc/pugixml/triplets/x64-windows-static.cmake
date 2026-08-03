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

# Build pugixml release-only in vcpkg for this triplet.
#
# Primary reason (CRT / iterator-debug-level): bmake link settings in this pipeline use the
# release CRT even in DEBUG builds, so pugixml must be release artifacts to avoid CRT
# ('MDd_DynamicDebug' vs 'MD_DynamicRelease') and _ITERATOR_DEBUG_LEVEL (2 vs 0) link
# mismatches against our release-compiled BePugiXml wrapper object.
#
# Accepted tradeoff: the Release CMake config also adds optimization and defines NDEBUG, which
# strips pugixml's internal assert(...) checks. Before the vcpkg migration bmake compiled
# pugixml.cpp with asserts active in DEBUG, so this drops those library diagnostics -- but only
# on Windows DEBUG. The non-Windows triplets still build a debug archive with asserts enabled,
# and pugixml's asserts guard internal invariants (tree/allocation/xpath consistency) rather
# than acting as a correctness mechanism, so the lost coverage is minor and platform-local.
# Reproducing a debug archive with /MD + _ITERATOR_DEBUG_LEVEL=0 but without NDEBUG is not
# reachable through triplet variables (they append; they cannot strip vcpkg's /MDd runtime
# selection or CMake's /DNDEBUG) and would require a chainload toolchain override; the
# diagnostic value did not justify that fragility.
set(VCPKG_BUILD_TYPE release)
