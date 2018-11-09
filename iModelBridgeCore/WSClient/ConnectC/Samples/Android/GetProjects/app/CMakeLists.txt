# Sets the minimum version of CMake required to build the native
# library. You should either keep the default value or only pass a
# value of 3.4.0 or lower.

cmake_minimum_required(VERSION 3.4.1)

SET (MY_CMAKE_FLAGS "-std=c++14 ")
SET(CMAKE_CXX_FLAGS "${MY_CMAKE_FLAGS} ${CMAKE_CXX_FLAGS}" )
#SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14 " )

include_directories(
        ${BUILD_CONTEXTS}/PublicAPI
        ${BUILD_CONTEXTS}/VendorAPI
        )

add_library(connectwebservicesclientc-lib STATIC IMPORTED)
add_library(besqliteec-lib STATIC IMPORTED)
add_library(webservicesclient-lib STATIC IMPORTED)
add_library(ecobjects-lib STATIC IMPORTED)
add_library(units-lib STATIC IMPORTED)
add_library(bentleygeomserialization-lib STATIC IMPORTED)
add_library(bentleygeom-lib STATIC IMPORTED)
add_library(bexml-lib STATIC IMPORTED)
add_library(belibxml2-lib STATIC IMPORTED)
add_library(besqlite-lib  STATIC IMPORTED)
add_library(bezlib-lib STATIC IMPORTED)
add_library(snappy-lib STATIC IMPORTED)
add_library(lzma-lib STATIC IMPORTED)
add_library(behttp-lib STATIC IMPORTED)
add_library(befolly-lib STATIC IMPORTED)
add_library(becurl-lib  STATIC IMPORTED)
add_library(beopenssl-lib STATIC IMPORTED)
add_library(bejsoncpp-lib STATIC IMPORTED)
add_library(besecurity-lib STATIC IMPORTED)
add_library(bentley-lib STATIC IMPORTED)
add_library(beicu4c-lib  STATIC IMPORTED)

set_property(TARGET connectwebservicesclientc-lib PROPERTY IMPORTED_LOCATION ${BUILD_CONTEXTS}SubParts/Libs/libConnectWebServicesClientC.a)
set_property(TARGET besqliteec-lib PROPERTY IMPORTED_LOCATION ${BUILD_CONTEXTS}SubParts/Libs/libBeSQLiteEC.a)
set_property(TARGET webservicesclient-lib PROPERTY IMPORTED_LOCATION ${BUILD_CONTEXTS}SubParts/Libs/libWebServicesClient.a)
set_property(TARGET ecobjects-lib PROPERTY IMPORTED_LOCATION ${BUILD_CONTEXTS}SubParts/Libs/libECObjects.a)
set_property(TARGET units-lib PROPERTY IMPORTED_LOCATION ${BUILD_CONTEXTS}SubParts/Libs/libUnits.a)
set_property(TARGET bentleygeomserialization-lib PROPERTY IMPORTED_LOCATION ${BUILD_CONTEXTS}SubParts/Libs/libBentleyGeomSerialization.a)
set_property(TARGET bentleygeom-lib PROPERTY IMPORTED_LOCATION ${BUILD_CONTEXTS}SubParts/Libs/libBentleyGeom.a)
set_property(TARGET bexml-lib PROPERTY IMPORTED_LOCATION ${BUILD_CONTEXTS}SubParts/Libs/libBeXml.a)
set_property(TARGET belibxml2-lib PROPERTY IMPORTED_LOCATION ${BUILD_CONTEXTS}SubParts/Libs/libBeLibxml2.a)
set_property(TARGET besqlite-lib PROPERTY IMPORTED_LOCATION ${BUILD_CONTEXTS}SubParts/Libs/libBeSQLite.a)
set_property(TARGET bezlib-lib PROPERTY IMPORTED_LOCATION ${BUILD_CONTEXTS}SubParts/Libs/libBeZlib.a)
set_property(TARGET snappy-lib PROPERTY IMPORTED_LOCATION ${BUILD_CONTEXTS}SubParts/Libs/libsnappy.a)
set_property(TARGET lzma-lib PROPERTY IMPORTED_LOCATION ${BUILD_CONTEXTS}SubParts/Libs/liblzma.a)
set_property(TARGET behttp-lib PROPERTY IMPORTED_LOCATION ${BUILD_CONTEXTS}SubParts/Libs/libBeHttp.a)
set_property(TARGET befolly-lib PROPERTY IMPORTED_LOCATION ${BUILD_CONTEXTS}SubParts/Libs/libBeFolly.a)
set_property(TARGET becurl-lib PROPERTY IMPORTED_LOCATION ${BUILD_CONTEXTS}SubParts/Libs/libBeCurl.a)
set_property(TARGET beopenssl-lib PROPERTY IMPORTED_LOCATION ${BUILD_CONTEXTS}SubParts/Libs/libBeOpenSSL.a)
set_property(TARGET bejsoncpp-lib PROPERTY IMPORTED_LOCATION ${BUILD_CONTEXTS}SubParts/Libs/libBeJsonCpp.a)
set_property(TARGET besecurity-lib PROPERTY IMPORTED_LOCATION ${BUILD_CONTEXTS}SubParts/Libs/libBeSecurity.a)
set_property(TARGET bentley-lib PROPERTY IMPORTED_LOCATION ${BUILD_CONTEXTS}SubParts/Libs/libBentley.a)
set_property(TARGET beicu4c-lib PROPERTY IMPORTED_LOCATION ${BUILD_CONTEXTS}SubParts/Libs/libBeIcu4c.a)

# Creates and names a library, sets it as either STATIC
# or SHARED, and provides the relative paths to its source code.
# You can define multiple libraries, and CMake builds it for you.
# Gradle automatically packages shared libraries with your APK.

add_library( # Sets the name of the library.
             native-lib

             # Sets the library as a shared library.
             SHARED

             # Provides a relative path to your source file(s).
             # Associated headers in the same location as their source
             # file are automatically included.
             src/main/cpp/ConnectJNI.cpp
             src/main/cpp/ConnectInterfaceNative.cpp)

# Searches for a specified prebuilt library and stores the path as a
# variable. Because system libraries are included in the search path by
# default, you only need to specify the name of the public NDK library
# you want to add. CMake verifies that the library exists before
# completing its build.

find_library( # Sets the name of the path variable.
              log-lib

              # Specifies the name of the NDK library that
              # you want CMake to locate.
              log )

# Specifies libraries CMake should link to your target library. You
# can link multiple libraries, such as libraries you define in the
# build script, prebuilt third-party libraries, or system libraries.
# NOTE: Order matters here
target_link_libraries( # Specifies the target library.
                       native-lib

                       connectwebservicesclientc-lib
                       besqliteec-lib
                       webservicesclient-lib
                       ecobjects-lib
                       units-lib
                       bentleygeomserialization-lib
                       bentleygeom-lib
                       bexml-lib
                       belibxml2-lib
                       besqlite-lib
                       bezlib-lib
                       snappy-lib
                       lzma-lib
                       behttp-lib
                       befolly-lib
                       becurl-lib
                       beopenssl-lib
                       bejsoncpp-lib
                       besecurity-lib
                       bentley-lib
                       beicu4c-lib

                       # Links the target library to the log library
                       # included in the NDK.
                       ${log-lib})

