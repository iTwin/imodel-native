# x64-windows-static-md: 64-bit Windows, static library linkage, dynamic CRT (/MD).
# Use this triplet when OpenSSL will be re-linked into a DLL that uses the dynamic CRT
# (msvcrt.lib), which is the standard for all Bentley Windows builds.  This keeps the
# CRT linkage consistent and avoids "unresolved external" errors for CRT symbols.
set(VCPKG_TARGET_ARCHITECTURE x64)
set(VCPKG_CRT_LINKAGE dynamic)
set(VCPKG_LIBRARY_LINKAGE static)
