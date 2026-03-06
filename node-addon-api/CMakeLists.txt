cmake_minimum_required(VERSION 3.20)
project(iTwinNapi CXX)

# BentleyBuild integration
list(APPEND CMAKE_MODULE_PATH "$ENV{SrcRoot}/BentleyBuild/cmake")
include(BentleyBuild)

set(SOURCES
    napi_stub.cpp
)

if(CREATE_STATIC_LIBRARIES)
    bb_add_library(iTwinNapi STATIC ${SOURCES})
else()
    bb_add_library(iTwinNapi SHARED ${SOURCES})
endif()

target_include_directories(iTwinNapi PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}")

target_compile_definitions(iTwinNapi PRIVATE EXTERNAL_NAPI=1)

bb_target_include_subpart_api(iTwinNapi Bentley)

bb_find_subpart_library(BENTLEY_LIB iTwinBentley)
target_link_libraries(iTwinNapi PRIVATE ${BENTLEY_LIB})

# Resource delivery (replaces prewire-api.mke)
bb_deliver_directory("VendorAPI/Napi" "${CMAKE_CURRENT_SOURCE_DIR}/Napi")
