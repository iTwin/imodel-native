cmake_minimum_required(VERSION 3.20)
project(iTwinECObjects CXX)

# BentleyBuild integration
list(APPEND CMAKE_MODULE_PATH "$ENV{SrcRoot}/BentleyBuild/cmake")
include(BentleyBuild)

set(SOURCES
    CalculatedProperty.cpp
    DateTimeInfoAccessor.cpp
    DesignByContract.cpp
    ECClass.cpp
    ECcontext.cpp
    ECCustomAttribute.cpp
    ECDBuffer.cpp
    ECEnabler.cpp
    ECEnumeration.cpp
    ECInstance.cpp
    ECJsonUtilities.cpp
    ECProperty.cpp
    ECQuantityFormatting.cpp
    ECRelationshipPath.cpp
    ECSchema.cpp
    ECSchemaConverter.cpp
    ECSchemaDownConverter.cpp
    ECSchemaUnitsContext.cpp
    ECSchemaValidator.cpp
    ECType.cpp
    ECUnit.cpp
    ECValue.cpp
    ExpressionContext.cpp
    ExpressionHandler.cpp
    ExpressionNode.cpp
    Format.cpp
    IssueReporter.cpp
    KindOfQuantity.cpp
    LegacyUnits.cpp
    Phenomenon.cpp
    PresentationMetadataHelper.cpp
    PropertyCategory.cpp
    SchemaComparer.cpp
    SchemaConflictHelper.cpp
    SchemaJson.cpp
    SchemaMerger.cpp
    SchemaParseUtils.cpp
    SchemaXml.cpp
    StandaloneECInstance.cpp
    StandaloneECRelationshipInstance.cpp
    StandardCustomAttributeHelper.cpp
    SupplementalSchema.cpp
    SystemSymbolProvider.cpp
    UnitSystem.cpp
)

if(CREATE_STATIC_LIBRARIES)
    bb_add_library(iTwinECObjects STATIC ${SOURCES})
else()
    bb_add_library(iTwinECObjects SHARED ${SOURCES})
endif()

# PCH
target_precompile_headers(iTwinECObjects PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/ECObjectsPch.cpp")

target_include_directories(iTwinECObjects PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}"
    "${CMAKE_CURRENT_SOURCE_DIR}/../PrivateApi/ECObjects"
)

target_compile_definitions(iTwinECObjects PRIVATE __ECOBJECTS_BUILD__)

bb_target_include_subpart_api(iTwinECObjects ECObjects)
bb_target_include_subpart_api(iTwinECObjects Bentley)
bb_target_include_subpart_api(iTwinECObjects Geom)
bb_target_include_subpart_api(iTwinECObjects BeXml)
bb_target_include_subpart_api(iTwinECObjects Units)

bb_find_subpart_library(BENTLEY_LIB iTwinBentley)
bb_find_subpart_library(XML_LIB iTwinXml)
bb_find_subpart_library(LIBXML2_LIB iTwinLibxml2)
bb_find_subpart_library(GEOM_LIB iTwinGeom)
bb_find_subpart_library(GEOMSERIALIZATION_LIB iTwinGeomSerialization)
bb_find_subpart_library(JSONCPP_LIB iTwinJsonCpp)
bb_find_subpart_library(UNITS_LIB iTwinUnits)
bb_find_subpart_library(PUGIXML_LIB iTwinPugixml)
target_link_libraries(iTwinECObjects PRIVATE
    ${BENTLEY_LIB}
    ${XML_LIB}
    ${LIBXML2_LIB}
    ${GEOM_LIB}
    ${GEOMSERIALIZATION_LIB}
    ${JSONCPP_LIB}
    ${UNITS_LIB}
    ${PUGIXML_LIB}
)

# Resource delivery (replaces ecobjects.prewire.mke)
# ecobjects uses PublicApi (lowercase 'a') directory layout
bb_deliver_public_api("${CMAKE_CURRENT_SOURCE_DIR}/../PublicApi" ECObjects)
