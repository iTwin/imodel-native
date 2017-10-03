/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicApi/AecUnits/AecUnitsDefinitions.h $
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


#ifdef __AEC_UNITS_DOMAIN_BUILD__
#define AEC_UNITS_EXPORT EXPORT_ATTRIBUTE
#else
#define AEC_UNITS_EXPORT IMPORT_ATTRIBUTE
#endif

/** @namespace BentleyApi::AecModelingUnits %AecUnits data types */
#define BEGIN_BENTLEY_AEC_UNITS_NAMESPACE        BEGIN_BENTLEY_NAMESPACE namespace AecUnits {
#define END_BENTLEY_AEC_UNITS_NAMESPACE          } END_BENTLEY_NAMESPACE
#define USING_NAMESPACE_BENTLEY_AEC_UNITS        using namespace BentleyApi::AecUnits;

#define AEC_UNITS_TYPEDEFS(_structname_) \
    BEGIN_BENTLEY_AEC_UNITS_NAMESPACE DEFINE_POINTER_SUFFIX_TYPEDEFS(_structname_) END_BENTLEY_AEC_UNITS_NAMESPACE

#define AEC_UNITS_REFCOUNTED_TYPEDEFS(_structname_) \
    BEGIN_BENTLEY_AEC_UNITS_NAMESPACE struct _structname_; DEFINE_REF_COUNTED_PTR(_structname_) END_BENTLEY_AEC_UNITS_NAMESPACE 


//-----------------------------------------------------------------------------------------
// ECSchema macros
//-----------------------------------------------------------------------------------------
#define BENTLEY_AEC_UNITS_SCHEMA_NAME                        "AecUnits"
#define BENTLEY_AEC_UNITS_SCHEMA_PATH                        L"ECSchemas/Domain/AecUnits.ecschema.xml"
#define BENTLEY_AEC_UNITS_SCHEMA(className)                  BENTLEY_AEC_UNITS_SCHEMA_NAME "." className
#define BENTLEY_AEC_UNITS_SCHEMA_CODE                        BENTLEY_AEC_UNITS_SCHEMA_NAME 
#define BENTLEY_AEC_UNITS_AUTHORITY                          BENTLEY_AEC_UNITS_SCHEMA_NAME



