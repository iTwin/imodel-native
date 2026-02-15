cmake_minimum_required(VERSION 3.20)
project(iTwinUnits CXX)

# BentleyBuild integration
list(APPEND CMAKE_MODULE_PATH "$ENV{SrcRoot}/BentleyBuild/cmake")
include(BentleyBuild)

set(SRC_DIR "${CMAKE_CURRENT_SOURCE_DIR}/src")
set(FMT_DIR "${CMAKE_CURRENT_SOURCE_DIR}/src/Formatting")

set(SOURCES
    ${SRC_DIR}/UnitRegistry.cpp
    ${SRC_DIR}/UnitTypes.cpp
    ${SRC_DIR}/SymbolicExpression.cpp
    ${SRC_DIR}/Quantity.cpp
    ${SRC_DIR}/UnitNameMappings.cpp
    ${FMT_DIR}/CompositeValueSpec.cpp
    ${FMT_DIR}/FormatParsing.cpp
    ${FMT_DIR}/FormatUtils.cpp
    ${FMT_DIR}/Format.cpp
    ${FMT_DIR}/NumericFormatSpec.cpp
    ${FMT_DIR}/NumericFormatUtils.cpp
    ${FMT_DIR}/FormatMappings.cpp
)

if(CREATE_STATIC_LIBRARIES)
    bb_add_library(iTwinUnits STATIC ${SOURCES})
else()
    bb_add_library(iTwinUnits SHARED ${SOURCES})
endif()

# PCH
target_precompile_headers(iTwinUnits PRIVATE "${SRC_DIR}/UnitsPCH.cpp")

target_include_directories(iTwinUnits PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}"
    "${CMAKE_CURRENT_SOURCE_DIR}/PublicAPI"
    "${CMAKE_CURRENT_SOURCE_DIR}/src"
)

target_compile_definitions(iTwinUnits PRIVATE __UNITS_BUILD__)

bb_target_include_subpart_api(iTwinUnits Units)

bb_find_subpart_library(JSONCPP_LIB iTwinJsonCpp)
bb_find_subpart_library(BENTLEY_LIB iTwinBentley)
target_link_libraries(iTwinUnits PRIVATE
    ${JSONCPP_LIB}
    ${BENTLEY_LIB}
)

# Resource delivery (replaces Units.Prewire.mke)
bb_deliver_public_api("${CMAKE_CURRENT_SOURCE_DIR}/PublicAPI" Units Formatting)
