/*--------------------------------------------------------------------------------------+
|
|     $Source: SteelSchema/PublicAPI/Steel/SteelDefinitions.h $
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


#ifdef __STRUCTURAL_DOMAIN_BUILD__
    #define STEEL_EXPORT EXPORT_ATTRIBUTE
#else
    #define STEEL_EXPORT IMPORT_ATTRIBUTE
#endif

/** @namespace BentleyApi::STEEL %STEEL data types */
#define BEGIN_BENTLEY_STEEL_NAMESPACE        BEGIN_BENTLEY_NAMESPACE namespace Steel {
#define END_BENTLEY_STEEL_NAMESPACE          } END_BENTLEY_NAMESPACE
#define USING_NAMESPACE_BENTLEY_STEEL        using namespace BentleyApi::Steel;

#define STEEL_TYPEDEFS(_structname_) \
    BEGIN_BENTLEY_STEEL_NAMESPACE DEFINE_POINTER_SUFFIX_TYPEDEFS(_structname_) END_BENTLEY_STEEL_NAMESPACE

#define STEEL_REFCOUNTED_TYPEDEFS(_structname_) \
    BEGIN_BENTLEY_STEEL_NAMESPACE struct _structname_; DEFINE_REF_COUNTED_PTR(_structname_) END_BENTLEY_STEEL_NAMESPACE

#define PRIMARY_BASELINE_LABEL "__Primary"

//-----------------------------------------------------------------------------------------
// ECSchema macros
//-----------------------------------------------------------------------------------------
#define BENTLEY_STEEL_SCHEMA_NAME                        "Steel"
#define BENTLEY_STEEL_SCHEMA_PATH                        L"ECSchemas/Domain/Steel.ecschema.xml"
#define BENTLEY_STEEL_SCHEMA(className)                  BENTLEY_STEEL_SCHEMA_NAME "." className
#define BENTLEY_STEEL_SCHEMA_CODE                        BENTLEY_STEEL_SCHEMA_NAME
#define BENTLEY_STEEL_AUTHORITY                          BENTLEY_STEEL_SCHEMA_NAME

//#define STEEL_SCHEMA(name)                               BENTLEY_STEEL_SCHEMA_NAME "." name

//-----------------------------------------------------------------------------------------
// ECClass names (combine with STEEL_SCHEMA macro for use in ECSql)
//-----------------------------------------------------------------------------------------
#define STEEL_CLASS_SteelElement                      "SteelElement"

#define STEEL_CLASS_HomogeneousSteelElement           "HomogeneousSteelElement"
#define STEEL_CLASS_RolledSteelElement                "RolledSteelElement"
#define STEEL_CLASS_Beam                              "Beam"
#define STEEL_CLASS_Brace                             "Brace"
#define STEEL_CLASS_Column                            "Column"
#define STEEL_CLASS_Plate                             "Plate"
#define STEEL_CLASS_SteelShape                        "SteelShape"

//-----------------------------------------------------------------------------------------
// All R, CR, P, CP, Ptr, CPtr declarations
//-----------------------------------------------------------------------------------------

