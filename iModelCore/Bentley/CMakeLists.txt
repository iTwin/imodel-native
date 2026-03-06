cmake_minimum_required(VERSION 3.20)
project(iTwinBentley CXX)

# BentleyBuild integration (also serves as the cmake auto-detection marker)
list(APPEND CMAKE_MODULE_PATH "$ENV{SrcRoot}/BentleyBuild/cmake")
include(BentleyBuild)

set(BENTLEY_DIR "${CMAKE_CURRENT_SOURCE_DIR}/Bentley")
set(NONPORT_DIR "${BENTLEY_DIR}/nonport")

set(SOURCES
    # Core Bentley sources
    ${BENTLEY_DIR}/WString.cpp
    ${BENTLEY_DIR}/GlobalHandleContainer.cpp
    ${BENTLEY_DIR}/DateTime.cpp
    ${BENTLEY_DIR}/DateTimeConverter.cpp
    ${BENTLEY_DIR}/RefCounted.cpp
    ${BENTLEY_DIR}/md5.cpp
    ${BENTLEY_DIR}/sha1.cpp
    ${BENTLEY_DIR}/Logging.cpp

    # Nonport sources
    ${NONPORT_DIR}/BeTest.cpp
    ${NONPORT_DIR}/BeFile.cpp
    ${NONPORT_DIR}/BeFileName.cpp
    ${NONPORT_DIR}/BeFileListIterator.cpp
    ${NONPORT_DIR}/BeDirectoryIterator.cpp
    ${NONPORT_DIR}/BeTextFile.cpp
    ${NONPORT_DIR}/BeStringUtilities.cpp
    ${NONPORT_DIR}/BeSystemInfo.cpp
    ${NONPORT_DIR}/BeGetProcAddress.cpp
    ${NONPORT_DIR}/BeTimeUtilities.cpp
    ${NONPORT_DIR}/BeThread.cpp
    ${NONPORT_DIR}/BeDebugLog.cpp
    ${NONPORT_DIR}/BeDebugUtilities.cpp
    ${NONPORT_DIR}/ValueFormat.cpp
    ${NONPORT_DIR}/Base64Utilities.cpp
)

if(CREATE_STATIC_LIBRARIES)
    bb_add_library(iTwinBentley STATIC ${SOURCES})
else()
    bb_add_library(iTwinBentley SHARED ${SOURCES})
endif()

target_include_directories(iTwinBentley PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}"
    "${CMAKE_CURRENT_SOURCE_DIR}/PublicAPI"
    "${BENTLEY_DIR}"
    "${NONPORT_DIR}"
)

target_compile_definitions(iTwinBentley PRIVATE
    __BENTLEYDLL_BUILD__
    BC__RWSTD_LIB_SRC
    $<$<CONFIG:Debug>:DEBUG>
)

target_compile_options(iTwinBentley PRIVATE -Wno-unknown-pragmas)

bb_target_include_subpart_api(iTwinBentley Bentley)

# ICU headers live under VendorAPI/icu4c/unicode/ but sources use bare <unicode/...> includes
target_include_directories(iTwinBentley BEFORE PRIVATE "${BB_VENDOR_API_DIR}/icu4c")

# Link ICU
bb_find_subpart_library(ICU4C_LIB iTwinIcu4c)
if(ICU4C_LIB)
    target_link_libraries(iTwinBentley PRIVATE ${ICU4C_LIB})
endif()

# ---------------------------------------------------------------------------
# Resource delivery (replaces prewire.mke + BeRapidJson.mke)
# ---------------------------------------------------------------------------
bb_deliver_public_api("${CMAKE_CURRENT_SOURCE_DIR}/PublicAPI" Bentley)
bb_deliver_public_api("${CMAKE_CURRENT_SOURCE_DIR}/BeRapidJson/PublicAPI" BeRapidJson)
bb_deliver_file("Delivery/bentley-notice.txt" "${CMAKE_CURRENT_SOURCE_DIR}/bentley-notice.txt")
bb_deliver_file("Delivery/btree-notice.txt" "${CMAKE_CURRENT_SOURCE_DIR}/btree-notice.txt")
bb_deliver_file("Delivery/btree-license.txt" "${CMAKE_CURRENT_SOURCE_DIR}/btree-license.txt")
