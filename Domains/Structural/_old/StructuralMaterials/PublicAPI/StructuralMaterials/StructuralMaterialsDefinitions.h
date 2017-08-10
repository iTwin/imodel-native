/*--------------------------------------------------------------------------------------+
|
|     $Source: _old/StructuralMaterials/PublicAPI/StructuralMaterials/StructuralMaterialsDefinitions.h $
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
    #define STRUCTURAL_MATERIALS_EXPORT EXPORT_ATTRIBUTE
#else
    #define STRUCTURAL_MATERIALS_EXPORT IMPORT_ATTRIBUTE
#endif

/** @namespace BentleyApi::STRUCTURAL_MATERIAL %STRUCTURAL_MATERIAL data types */
#define BEGIN_BENTLEY_STRUCTURAL_MATERIALS_NAMESPACE        BEGIN_BENTLEY_NAMESPACE namespace StructuralMaterials {
#define END_BENTLEY_STRUCTURAL_MATERIALS_NAMESPACE          } END_BENTLEY_NAMESPACE
#define USING_NAMESPACE_BENTLEY_STRUCTURAL_MATERIALS        using namespace BentleyApi::StructuralMaterials;

#define STRUCTURAL_MATERIALS_TYPEDEFS(_structname_) \
    BEGIN_BENTLEY_STRUCTURAL_MATERIALS_NAMESPACE DEFINE_POINTER_SUFFIX_TYPEDEFS(_structname_) END_BENTLEY_STRUCTURAL_MATERIALS_NAMESPACE

#define STRUCTURAL_MATERIALS_REFCOUNTED_TYPEDEFS(_structname_) \
    BEGIN_BENTLEY_STRUCTURAL_MATERIALS_NAMESPACE struct _structname_; DEFINE_REF_COUNTED_PTR(_structname_) END_BENTLEY_STRUCTURAL_MATERIALS_NAMESPACE

#define PRIMARY_BASELINE_LABEL "__Primary"

//-----------------------------------------------------------------------------------------
// ECSchema macros
//-----------------------------------------------------------------------------------------
#define BENTLEY_STRUCTURAL_MATERIALS_SCHEMA_NAME                        "StructuralMaterial"
#define BENTLEY_STRUCTURAL_MATERIALS_SCHEMA_PATH                        L"ECSchemas/Domain/StructuralMaterials.ecschema.xml"
#define BENTLEY_STRUCTURAL_MATERIALS_SCHEMA(className)                  BENTLEY_STRUCTURAL_MATERIALS_SCHEMA_NAME "." className
#define BENTLEY_STRUCTURAL_MATERIALS_SCHEMA_CODE                        BENTLEY_STRUCTURAL_MATERIALS_SCHEMA_NAME
#define BENTLEY_STRUCTURAL_MATERIALS_AUTHORITY                          BENTLEY_STRUCTURAL_MATERIALS_SCHEMA_NAME

//#define STRUCTURAL_MATERIAL_SCHEMA(name)                               BENTLEY_STRUCTURAL_MATERIALS_SCHEMA_NAME "." name

//-----------------------------------------------------------------------------------------
// ECClass names (combine with STRUCTURAL_MATERIAL_SCHEMA macro for use in ECSql)
//-----------------------------------------------------------------------------------------
//#define SM_CLASS_Door                                       "Door"


//-----------------------------------------------------------------------------------------
// Authority names
//-----------------------------------------------------------------------------------------
//#define BP_AUTHORITY_Planning                               "Planning"

//-----------------------------------------------------------------------------------------
// All R, CR, P, CP, Ptr, CPtr declarations
//-----------------------------------------------------------------------------------------
//STRUCTURAL_MATERIAL_TYPEDEFS(ArchitecturalBaseElement)
//STRUCTURAL_MATERIAL_REFCOUNTED_TYPEDEFS(ArchitecturalBaseElement)



