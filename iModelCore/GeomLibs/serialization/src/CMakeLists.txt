cmake_minimum_required(VERSION 3.20)
project(iTwinGeomSerialization CXX)

# BentleyBuild integration
list(APPEND CMAKE_MODULE_PATH "$ENV{SrcRoot}/BentleyBuild/cmake")
include(BentleyBuild)

set(SOURCES
    FlatBuffer/FixedStructs.cpp
    BeCGWriter.cpp
    BeJsonToCG.cpp
    BeJsonValueBuilder.cpp
    IModelJsonWriter.cpp
    IModelJsonReader.cpp
)

if(CREATE_STATIC_LIBRARIES)
    bb_add_library(iTwinGeomSerialization STATIC ${SOURCES})
else()
    bb_add_library(iTwinGeomSerialization SHARED ${SOURCES})
endif()

# PCH
target_precompile_headers(iTwinGeomSerialization PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/serializationPCH.cpp")

target_include_directories(iTwinGeomSerialization PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}"
    "${CMAKE_CURRENT_SOURCE_DIR}/.."
    "${CMAKE_CURRENT_SOURCE_DIR}/../../PublicAPI"
)

target_compile_definitions(iTwinGeomSerialization PRIVATE
    __GEOMLIBS_SERIALIZATION_BUILD__
    BGFB_INTERNAL
    BuildCGJsonValueBuilder
)

bb_target_include_subpart_api(iTwinGeomSerialization GeomSerialization)
bb_target_include_subpart_api(iTwinGeomSerialization Geom)
bb_target_include_subpart_api(iTwinGeomSerialization Bentley)

bb_find_subpart_library(GEOM_LIB iTwinGeom)
bb_find_subpart_library(JSONCPP_LIB iTwinJsonCpp)
bb_find_subpart_library(BENTLEY_LIB iTwinBentley)
target_link_libraries(iTwinGeomSerialization PRIVATE
    ${GEOM_LIB}
    ${JSONCPP_LIB}
    ${BENTLEY_LIB}
)
