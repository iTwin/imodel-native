cmake_minimum_required(VERSION 3.20)
project(iTwinXml CXX)

# BentleyBuild integration
list(APPEND CMAKE_MODULE_PATH "$ENV{SrcRoot}/BentleyBuild/cmake")
include(BentleyBuild)

set(SOURCES
    BeXml.cpp
)

if(CREATE_STATIC_LIBRARIES)
    bb_add_library(iTwinXml STATIC ${SOURCES})
else()
    bb_add_library(iTwinXml SHARED ${SOURCES})
endif()

target_include_directories(iTwinXml PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}"
    "${CMAKE_CURRENT_SOURCE_DIR}/PublicApi"
    "${CMAKE_CURRENT_SOURCE_DIR}/../PublicAPI"
)

target_compile_definitions(iTwinXml PRIVATE
    __BEXMLDLL_BUILD__
    $<$<BOOL:${CREATE_STATIC_LIBRARIES}>:LIBXML_STATIC>
)

bb_target_include_subpart_api(iTwinXml BeXml)

bb_find_subpart_library(LIBXML2_LIB iTwinLibxml2)
bb_find_subpart_library(BENTLEY_LIB iTwinBentley)
target_link_libraries(iTwinXml PRIVATE
    ${LIBXML2_LIB}
    ${BENTLEY_LIB}
)

# Resource delivery (PublicAPI/BeXml)
bb_deliver_public_api("${CMAKE_CURRENT_SOURCE_DIR}/PublicApi" BeXml)
