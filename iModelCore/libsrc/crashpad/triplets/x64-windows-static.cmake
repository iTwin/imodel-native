set(VCPKG_TARGET_ARCHITECTURE x64)
set(VCPKG_CRT_LINKAGE dynamic)
set(VCPKG_LIBRARY_LINKAGE static)

# Match BentleyBuild runtime behavior for Windows third-party static libs.
set(VCPKG_C_FLAGS_DEBUG "${VCPKG_C_FLAGS_DEBUG} /RTCsu")
set(VCPKG_CXX_FLAGS_DEBUG "${VCPKG_CXX_FLAGS_DEBUG} /RTCsu")
