set(VCPKG_TARGET_ARCHITECTURE x64)
set(VCPKG_CRT_LINKAGE dynamic)
set(VCPKG_LIBRARY_LINKAGE static)

# Match our previous build: minizip without crypt/uncrypt support
set(VCPKG_C_FLAGS "${VCPKG_C_FLAGS} /DNOCRYPT /DNOUNCRYPT")
set(VCPKG_CXX_FLAGS "${VCPKG_CXX_FLAGS} /DNOCRYPT /DNOUNCRYPT")

# Runtime checks for debug builds (matches original Zlib.mke behavior)
set(VCPKG_C_FLAGS_DEBUG "${VCPKG_C_FLAGS_DEBUG} /RTCsu")
set(VCPKG_CXX_FLAGS_DEBUG "${VCPKG_CXX_FLAGS_DEBUG} /RTCsu")
