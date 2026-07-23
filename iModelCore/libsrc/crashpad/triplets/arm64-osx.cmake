# To force a rebuild and a fresh vcpkg binary-cache entry for this triplet, increment the
# number below. Triplet file contents feed into vcpkg's ABI hash, so bumping this value
# invalidates the cached binaries and forces the library to be rebuilt.
# CACHE_BUST = 3

# Crashpad only builds as a static library (see crashpad.PartFile.xml ExcludeLibType="Dynamic"),
# so force static library linkage here.  Without this overlay triplet vcpkg would fall back to
# its built-in arm64-osx triplet, which defaults to dynamic library linkage.
set(VCPKG_TARGET_ARCHITECTURE arm64)
set(VCPKG_CRT_LINKAGE dynamic)
set(VCPKG_LIBRARY_LINKAGE static)

set(VCPKG_CMAKE_SYSTEM_NAME Darwin)
set(VCPKG_OSX_ARCHITECTURES arm64)

# No extra C/C++ flags are needed on macOS: the Linux triplets' -D_FILE_OFFSET_BITS=64 is a
# Linux large-file ABI concern (macOS off_t is already 64-bit), and the Windows triplets'
# release-only / -RTC settings are MSVC CRT concerns that do not apply to the clang/Darwin build.
