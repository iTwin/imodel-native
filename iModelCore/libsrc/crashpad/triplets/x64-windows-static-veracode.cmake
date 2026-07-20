# x64-windows-static-veracode: MSVC (Visual Studio cl.exe) toolset, veracode variant. The
# clang (WINDOWS_CLANG) toolset uses x64-windows-static-veracode-clang instead; keeping this
# triplet MSVC-specific (this marker also changes the vcpkg ABI hash) ensures the clang and
# Visual Studio builds never share binary-cache entries.
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

# Veracode: avoid explicit runtime-check switches.
