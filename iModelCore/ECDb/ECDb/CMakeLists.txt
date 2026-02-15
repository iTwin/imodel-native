#---------------------------------------------------------------------------------------------
#  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
#  See LICENSE.md in the repository root for full copyright notice.
#
#  BentleyBuild-integrated CMakeLists.txt for ECDb (iTwinSQLiteEC)
#  This replaces ECDb.mke when USE_CMAKE=1 is set.
#---------------------------------------------------------------------------------------------

cmake_minimum_required(VERSION 3.20)
project(iTwinSQLiteEC CXX)

# BentleyBuild integration
list(APPEND CMAKE_MODULE_PATH "$ENV{SrcRoot}/BentleyBuild/cmake")
include(BentleyBuild)

set(ECDB_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}")
set(ECDB_PUBLIC_API_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../PublicAPI/ECDb")

# ---------------------------------------------------------------------------
# Source files  (mirrors ECDb.mke compiland list)
# ---------------------------------------------------------------------------
set(ECDB_CORE_SOURCES
    ECDbLogger.cpp
    DbUtilities.cpp
    JsonPersistenceHelper.cpp
    DbMappingManager.cpp
    DbMapValidator.cpp
    LightweightCache.cpp
    MapStrategy.cpp
    ClassMappingInfo.cpp
    ClassMap.cpp
    RelationshipClassMap.cpp
    PropertyMap.cpp
    SystemPropertyMap.cpp
    PropertyMapVisitor.cpp
    ClassMapPersistenceManager.cpp
    ECDbImpl.cpp
    BuiltInFuncs.cpp
    ECDb.cpp
    FeatureManager.cpp
    IssueReporter.cpp
    SchemaPersistenceHelper.cpp
    SchemaManager.cpp
    SchemaManagerDispatcher.cpp
    RemapManager.cpp
    SchemaReader.cpp
    SchemaWriter.cpp
    SchemaImportContext.cpp
    SchemaValidator.cpp
    PolicyManager.cpp
    ProfileManager.cpp
    ProfileUpgrader.cpp
    ECDbSystemSchemaHelper.cpp
    ECDbMapSchemaHelper.cpp
    ECInstanceFinder.cpp
    ViewGenerator.cpp
    ChangeManager.cpp
    ChangeSummaryExtractor.cpp
    ChangedIdsIteratorImpl.cpp
    ChangeIteratorImpl.cpp
    ChangeSummaryImpl.cpp
    DbSchema.cpp
    DbSchemaPersistenceManager.cpp
    ECDbExpressionSymbolProvider.cpp
    ClassMapColumnFactory.cpp
    ConcurrentQueryManagerImpl.cpp
    ECSqlRowAdaptor.cpp
    InstanceReaderImpl.cpp
    SchemaSync.cpp
    IntegrityChecker.cpp
    BuiltInVTabs.cpp
    ClassViews.cpp
    InstanceWriterImpl.cpp
    InstanceRepository.cpp
)

set(ECDB_ECSQL_SOURCES
    ECSql/ECSqlStatus.cpp
    ECSql/DynamicSelectClauseECClass.cpp
    ECSql/ECSqlParser.cpp
    ECSql/ECSqlTypeInfo.cpp
    ECSql/Exp.cpp
    ECSql/ComputedExp.cpp
    ECSql/ValueExp.cpp
    ECSql/WindowFunctionExp.cpp
    ECSql/ListExp.cpp
    ECSql/ClassRefExp.cpp
    ECSql/PropertyNameExp.cpp
    ECSql/WhereExp.cpp
    ECSql/JoinExp.cpp
    ECSql/SelectStatementExp.cpp
    ECSql/InsertStatementExp.cpp
    ECSql/UpdateStatementExp.cpp
    ECSql/DeleteStatementExp.cpp
    ECSql/PragmaECSqlPreparedStatement.cpp
    ECSql/OptionsExp.cpp
    ECSql/ExpHelper.cpp
    ECSql/ECSqlPrepareContext.cpp
    ECSql/NativeSqlBuilder.cpp
    ECSql/ECSqlPreparer.cpp
    ECSql/ECSqlPropertyNameExpPreparer.cpp
    ECSql/ECSqlSelectPreparer.cpp
    ECSql/ECSqlInsertPreparer.cpp
    ECSql/ECSqlUpdatePreparer.cpp
    ECSql/ECSqlDeletePreparer.cpp
    ECSql/IECSqlValue.cpp
    ECSql/IECSqlBinder.cpp
    ECSql/ECSqlColumnInfo.cpp
    ECSql/ECSqlStatement.cpp
    ECSql/ECSqlPreparedStatement.cpp
    ECSql/ECSqlStatementImpl.cpp
    ECSql/ECSqlStatementCache.cpp
    ECSql/ECInstanceECSqlSelectAdapter.cpp
    ECSql/ECInstanceAdapterHelper.cpp
    ECSql/ECInstanceDeleter.cpp
    ECSql/ECInstanceInserter.cpp
    ECSql/ECInstanceUpdater.cpp
    ECSql/JsonECSqlSelectAdapter.cpp
    ECSql/JsonReader.cpp
    ECSql/JsonInserter.cpp
    ECSql/JsonUpdater.cpp
    ECSql/JsonECSqlBinder.cpp
    ECSql/ECSqlStatementNoopImpls.cpp
    ECSql/ECSqlBinder.cpp
    ECSql/PrimitiveECSqlBinder.cpp
    ECSql/PointECSqlBinder.cpp
    ECSql/StructECSqlBinder.cpp
    ECSql/ArrayECSqlBinder.cpp
    ECSql/NavigationPropertyECSqlBinder.cpp
    ECSql/IdECSqlBinder.cpp
    ECSql/VirtualSetBinder.cpp
    ECSql/ECSqlParseTreeFormatter.cpp
    ECSql/ECSqlField.cpp
    ECSql/ECSqlFieldFactory.cpp
    ECSql/PrimitiveECSqlField.cpp
    ECSql/PointECSqlField.cpp
    ECSql/StructECSqlField.cpp
    ECSql/ArrayECSqlField.cpp
    ECSql/CommonTableExp.cpp
    ECSql/NavigationPropertyECSqlField.cpp
    ECSql/ECSqlPragmas.cpp
    ECSql/ValueCreationFuncExp.cpp
)

# Parser sources (third-party, need relaxed warnings)
set(ECDB_PARSER_SOURCES
    ECSql/Parser/SqlNode.cpp
    ECSql/Parser/SqlParse.cpp
    ECSql/Parser/SqlFlex.cpp
    ECSql/Parser/SqlBison.cpp
)

# Precompiled header
set(ECDB_PCH_HEADER ECDbPch.h)

# ---------------------------------------------------------------------------
# Library target
# ---------------------------------------------------------------------------
set(ALL_SOURCES ${ECDB_CORE_SOURCES} ${ECDB_ECSQL_SOURCES} ${ECDB_PARSER_SOURCES})

if(CREATE_STATIC_LIBRARIES)
    bb_add_library(iTwinSQLiteEC STATIC ${ALL_SOURCES})
else()
    bb_add_library(iTwinSQLiteEC SHARED ${ALL_SOURCES})
endif()

# Compile definitions
target_compile_definitions(iTwinSQLiteEC PRIVATE __ECDB_BUILD__)

# Precompiled header support (CMake 3.16+)
target_precompile_headers(iTwinSQLiteEC PRIVATE "${ECDB_SOURCE_DIR}/${ECDB_PCH_HEADER}")

# ---------------------------------------------------------------------------
# Include directories
# ---------------------------------------------------------------------------
target_include_directories(iTwinSQLiteEC PRIVATE
    "${ECDB_SOURCE_DIR}"
    "${ECDB_PUBLIC_API_DIR}"
    "${CMAKE_CURRENT_SOURCE_DIR}/../PublicAPI"
)

# SubPart public APIs (populated by BentleyBuild's BuildContext before we run)
# This adds BuildContext/PublicAPI/ and BuildContext/VendorAPI/ as include roots,
# matching bmake's -I${BuildContext}/PublicAPI/ -I${BuildContext}/VendorAPI/
bb_target_include_subpart_api(iTwinSQLiteEC ECDb)

# ---------------------------------------------------------------------------
# Link dependencies (from SubParts/Libs in the BuildContext)
# ---------------------------------------------------------------------------
set(ECDB_LINK_LIBS
    iTwinSQLite
    iTwinECObjects
    iTwinUnits
    iTwinGeom
    iTwinBentley
    iTwinXml
    iTwinLibxml2
    iTwinJsonCpp
    iTwinGeomSerialization
    iTwin_google_re2
)

foreach(LIB_NAME ${ECDB_LINK_LIBS})
    bb_find_subpart_library(FOUND_${LIB_NAME} ${LIB_NAME})
    if(FOUND_${LIB_NAME})
        target_link_libraries(iTwinSQLiteEC PRIVATE ${FOUND_${LIB_NAME}})
    else()
        # Fall back to looking in the subparts dir directly
        target_link_directories(iTwinSQLiteEC PRIVATE "${BB_CONTEXT_SUBPARTS_LIBS}")
        target_link_libraries(iTwinSQLiteEC PRIVATE ${LIB_NAME})
    endif()
endforeach()

# ---------------------------------------------------------------------------
# Relaxed warning flags for third-party parser sources
# ---------------------------------------------------------------------------
if(NOT MSVC)
    set_source_files_properties(${ECDB_PARSER_SOURCES} PROPERTIES
        COMPILE_OPTIONS "-Wno-unknown-pragmas;-Wno-unused-function;-Wno-address;-Wno-deprecated-register;-Wno-unused-const-variable"
    )
else()
    # MSVC: disable analysis for parser sources
    set_source_files_properties(${ECDB_PARSER_SOURCES} PROPERTIES
        COMPILE_OPTIONS "/analyze-"
    )
endif()

# ---------------------------------------------------------------------------
# Resource delivery (replaces ECDb.prewire.mke)
# ---------------------------------------------------------------------------
bb_deliver_public_api("${CMAKE_CURRENT_SOURCE_DIR}/../PublicAPI" ECDb)
bb_deliver_glob("ECSchemas/ECDb" "${CMAKE_CURRENT_SOURCE_DIR}/ECSchemas/*.ecschema.xml")
