/*--------------------------------------------------------------------------------------+
|
|     $Source: BuildingCommonSchema/PublicAPI/BuildingCommon/BuildingCommonDefinitions.h $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

//__PUBLISH_SECTION_START__
#include <Bentley/Bentley.h>
#include <ECDb/ECDbTypes.h>
#include <ECDb/ECDbApi.h>
#include <ECObjects/ECSchema.h>
#include <DgnPlatform/DgnPlatformApi.h>
#include <DgnPlatform/DgnDb.h>
#include <DgnPlatform/DgnModel.h>


#ifdef __BUILDING_DOMAIN_BUILD__
#define BUILDING_COMMON_EXPORT EXPORT_ATTRIBUTE
#else
#define BUILDING_COMMON_EXPORT IMPORT_ATTRIBUTE
#endif

/** @namespace BentleyApi::BuildingCommon %Building_Common data types */
#define BEGIN_BENTLEY_BUILDING_COMMON_NAMESPACE        BEGIN_BENTLEY_NAMESPACE namespace BuildingCommon {
#define END_BENTLEY_BUILDING_COMMON_NAMESPACE          } END_BENTLEY_NAMESPACE
#define USING_NAMESPACE_BENTLEY_BUILDING_COMMON        using namespace BentleyApi::BuildingCommon;

#define BUILDING_COMMON_TYPEDEFS(_structname_) \
    BEGIN_BENTLEY_BUILDING_COMMON_NAMESPACE DEFINE_POINTER_SUFFIX_TYPEDEFS(_structname_) END_BENTLEY_BUILDING_COMMON_NAMESPACE

#define BUILDING_COMMON_REFCOUNTED_TYPEDEFS(_structname_) \
    BEGIN_BENTLEY_BUILDING_COMMON_NAMESPACE struct _structname_; DEFINE_REF_COUNTED_PTR(_structname_) END_BENTLEY_BUILDING_COMMON_NAMESPACE 


//-----------------------------------------------------------------------------------------
// ECSchema macros
//-----------------------------------------------------------------------------------------
#define BENTLEY_BUILDING_COMMON_SCHEMA_NAME                        "BuildingCommon"
#define BENTLEY_BUILDING_COMMON_SCHEMA_PATH                        L"ECSchemas/Domain/BuildingCommon.ecschema.xml"
#define BENTLEY_BUILDING_COMMON_SCHEMA(className)                  BENTLEY_BUILDING_COMMON_SCHEMA_NAME "." className
#define BENTLEY_BUILDING_COMMON_SCHEMA_CODE                        BENTLEY_BUILDING_COMMON_SCHEMA_NAME 
#define BENTLEY_BUILDING_COMMON_AUTHORITY                          BENTLEY_BUILDING_COMMON_SCHEMA_NAME

//-----------------------------------------------------------------------------------------
// ECClass names (combine with ARCHITECTURAL_PHYSICAL_SCHEMA macro for use in ECSql)
//-----------------------------------------------------------------------------------------
#define BC_CLASS_Classification                             "Classification"
#define BC_CLASS_Manufacturer                               "Manufacturer"



