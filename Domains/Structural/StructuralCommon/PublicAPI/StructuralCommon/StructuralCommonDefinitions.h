/*--------------------------------------------------------------------------------------+
|
|     $Source: StructuralCommon/PublicAPI/StructuralCommon/StructuralCommonDefinitions.h $
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
#define STRUCTURAL_DOMAIN_EXPORT EXPORT_ATTRIBUTE
#else
#define STRUCTURAL_DOMAIN_EXPORT IMPORT_ATTRIBUTE
#endif

/** @namespace BentleyApi::StructuralPhysical %STRUCTURAL_PHYSICAL data types */
#define BEGIN_BENTLEY_STRUCTURAL_NAMESPACE BEGIN_BENTLEY_NAMESPACE namespace Structural {
#define END_BENTLEY_STRUCTURAL_NAMESPACE } END_BENTLEY_NAMESPACE
#define USING_NAMESPACE_BENTLEY_STRUCTURAL_PHYSICAL using namespace BentleyApi::Structural;

#define STRUCTURAL_POINTER_SUFFIX_TYPEDEFS(_structname_) \
    BEGIN_BENTLEY_STRUCTURAL_NAMESPACE DEFINE_POINTER_SUFFIX_TYPEDEFS(_structname_) END_BENTLEY_STRUCTURAL_NAMESPACE

#define STRUCTURAL_REFCOUNTED_TYPEDEFS(_structname_) \
    BEGIN_BENTLEY_STRUCTURAL_NAMESPACE struct _structname_; DEFINE_REF_COUNTED_PTR(_structname_) END_BENTLEY_STRUCTURAL_NAMESPACE

#define STRUCTURAL_POINTER_TYPEDEFS(_structname_) \
    STRUCTURAL_POINTER_SUFFIX_TYPEDEFS(_structname_) \
    STRUCTURAL_REFCOUNTED_TYPEDEFS(_structname_)

//-----------------------------------------------------------------------------------------
// ECSchema macros
//-----------------------------------------------------------------------------------------
#define BENTLEY_STRUCTURAL_COMMON_SCHEMA_NAME                        "StructuralCommon"
#define BENTLEY_STRUCTURAL_COMMON_SCHEMA_PATH                        L"ECSchemas/Domain/StructuralCommon.ecschema.xml"
#define BENTLEY_STRUCTURAL_COMMON_SCHEMA(className)                  BENTLEY_STRUCTURAL_COMMON_SCHEMA_NAME "." className
#define BENTLEY_STRUCTURAL_COMMON_SCHEMA_CODE                        BENTLEY_STRUCTURAL_COMMON_SCHEMA_NAME
#define BENTLEY_STRUCTURAL_COMMON_AUTHORITY                          BENTLEY_STRUCTURAL_COMMON_SCHEMA_NAME

//-----------------------------------------------------------------------------------------
// ECClass names (combine with ARCHITECTURAL_PHYSICAL_SCHEMA macro for use in ECSql)
//-----------------------------------------------------------------------------------------
#define STRUCTURAL_COMMON_CLASS_StructuralPhysicalModel              "StructuralPhysicalModel"
#define STRUCTURAL_COMMON_CLASS_StructuralTypeDefinitionModel        "StructuralTypeDefinitionModel"

//-----------------------------------------------------------------------------------------
// All R, CR, P, CP, Ptr, CPtr declarations
//-----------------------------------------------------------------------------------------
STRUCTURAL_POINTER_TYPEDEFS(StructuralPhysicalModel)
STRUCTURAL_POINTER_TYPEDEFS(StructuralTypeDefinitionModel)

