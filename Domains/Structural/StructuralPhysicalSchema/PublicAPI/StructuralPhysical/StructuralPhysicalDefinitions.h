/*--------------------------------------------------------------------------------------+
|
|     $Source: StructuralPhysicalSchema/PublicAPI/StructuralPhysical/StructuralPhysicalDefinitions.h $
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
#define STRUCTURAL_PHYSICAL_EXPORT EXPORT_ATTRIBUTE
#else
#define STRUCTURAL_PHYSICAL_EXPORT IMPORT_ATTRIBUTE
#endif

/** @namespace BentleyApi::StructuralPhysical %STRUCTURAL_PHYSICAL data types */
#define BEGIN_BENTLEY_STRUCTURAL_PHYSICAL_NAMESPACE        BEGIN_BENTLEY_NAMESPACE namespace StructuralPhysical {
#define END_BENTLEY_STRUCTURAL_PHYSICAL_NAMESPACE          } END_BENTLEY_NAMESPACE
#define USING_NAMESPACE_BENTLEY_STRUCTURAL_PHYSICAL        using namespace BentleyApi::StructuralPhysical;

#define STRUCTURAL_PHYSICAL_TYPEDEFS(_structname_) \
    BEGIN_BENTLEY_STRUCTURAL_PHYSICAL_NAMESPACE DEFINE_POINTER_SUFFIX_TYPEDEFS(_structname_) END_BENTLEY_STRUCTURAL_PHYSICAL_NAMESPACE

#define STRUCTURAL_PHYSICAL_REFCOUNTED_TYPEDEFS(_structname_) \
    BEGIN_BENTLEY_STRUCTURAL_PHYSICAL_NAMESPACE struct _structname_; DEFINE_REF_COUNTED_PTR(_structname_) END_BENTLEY_STRUCTURAL_PHYSICAL_NAMESPACE


//-----------------------------------------------------------------------------------------
// ECSchema macros
//-----------------------------------------------------------------------------------------
#define BENTLEY_STRUCTURAL_PHYSICAL_SCHEMA_NAME                        "StructuralPhysical"
#define BENTLEY_STRUCTURAL_PHYSICAL_SCHEMA_PATH                        L"ECSchemas/Domain/StructuralPhysical.ecschema.xml"
#define BENTLEY_STRUCTURAL_PHYSICAL_SCHEMA(className)                  BENTLEY_STRUCTURAL_PHYSICAL_SCHEMA_NAME "." className
#define BENTLEY_STRUCTURAL_PHYSICAL_SCHEMA_CODE                        BENTLEY_STRUCTURAL_PHYSICAL_SCHEMA_NAME
#define BENTLEY_STRUCTURAL_PHYSICAL_AUTHORITY                          BENTLEY_STRUCTURAL_PHYSICAL_SCHEMA_NAME

//-----------------------------------------------------------------------------------------
// ECClass names (combine with ARCHITECTURAL_PHYSICAL_SCHEMA macro for use in ECSql)
//-----------------------------------------------------------------------------------------
#define BP_CLASS_StructuralPhysicalModel                               "StructuralPhysicalModel"
#define BP_CLASS_StructuralTypeDefinitionModel                         "StructuralTypeDefinitionModel"

//-----------------------------------------------------------------------------------------
// All R, CR, P, CP, Ptr, CPtr declarations
//-----------------------------------------------------------------------------------------
STRUCTURAL_PHYSICAL_TYPEDEFS(StructuralPhysicalModel)
STRUCTURAL_PHYSICAL_REFCOUNTED_TYPEDEFS(StructuralPhysicalModel)
STRUCTURAL_PHYSICAL_TYPEDEFS(StructuralTypeDefinitionModel)
STRUCTURAL_PHYSICAL_REFCOUNTED_TYPEDEFS(StructuralTypeDefinitionModel)

