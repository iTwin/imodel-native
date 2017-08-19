/*--------------------------------------------------------------------------------------+
|
|     $Source: StructuralMaterial/PublicAPI/StructuralMaterial/StructuralMaterialDefinitions.h $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

//__PUBLISH_SECTION_START__
#include <StructuralDomain\StructuralCommon\StructuralCommonDefinitions.h>

//-----------------------------------------------------------------------------------------
// ECSchema macros
//-----------------------------------------------------------------------------------------
#define BENTLEY_STRUCTURAL_MATERIAL_SCHEMA_NAME                        "StructuralMaterial"
#define BENTLEY_STRUCTURAL_MATERIAL_SCHEMA_PATH                        L"ECSchemas/Domain/StructuralMaterial.ecschema.xml"
#define BENTLEY_STRUCTURAL_MATERIAL_SCHEMA(className)                  BENTLEY_STRUCTURAL_MATERIAL_SCHEMA_NAME "." className
#define BENTLEY_STRUCTURAL_MATERIAL_SCHEMA_CODE                        BENTLEY_STRUCTURAL_MATERIAL_SCHEMA_NAME
#define BENTLEY_STRUCTURAL_MATERIAL_AUTHORITY                          BENTLEY_STRUCTURAL_MATERIAL_SCHEMA_NAME

//-----------------------------------------------------------------------------------------
// Categories
//-----------------------------------------------------------------------------------------
#define STRUCTURAL_MATERIAL_CATEGORY_StructuralCategory                "StructuralMaterialCategory"

//-----------------------------------------------------------------------------------------
// ECClass names
//-----------------------------------------------------------------------------------------
#define STRUCTURAL_MATERIAL_CLASS_MaterialProperties            "MaterialProperties"
#define STRUCTURAL_MATERIAL_CLASS_ConcreteMaterialProperties    "ConcreteMaterialProperties"
#define STRUCTURAL_MATERIAL_CLASS_SteelMaterialProperties       "SteelMaterialProperties"


//-----------------------------------------------------------------------------------------
// Define standard static QueryClass/QueryClassId methods on Elements and Aspects
//-----------------------------------------------------------------------------------------
#define DECLARE_STRUCTURAL_MATERIAL_QUERYCLASS_METHODS(__name__) \
    static Dgn::DgnClassId QueryClassId(Dgn::DgnDbCR db) { return Dgn::DgnClassId(db.Schemas().GetClassId(BENTLEY_STRUCTURAL_MATERIAL_SCHEMA_NAME, STRUCTURAL_MATERIAL_CLASS_##__name__)); } \
    static ECN::ECClassCP QueryClass(Dgn::DgnDbCR db) { return (db.Schemas().GetClass(BENTLEY_STRUCTURAL_MATERIAL_SCHEMA_NAME, STRUCTURAL_MATERIAL_CLASS_##__name__)); }

//-----------------------------------------------------------------------------------------
// Macro to declare Get, GetForEdit, Insert, Update methods on elements. Pointers (Ptr, CPtr) must be defined.
//-----------------------------------------------------------------------------------------
#define DECLARE_STRUCTURAL_MATERIAL_ELEMENT_BASE_GET_METHODS(__name__) \
    STRUCTURAL_DOMAIN_EXPORT static __name__##CPtr Get(Dgn::DgnDbR db, Dgn::DgnElementId id) { return db.Elements().Get< __name__ >(id); } \
    STRUCTURAL_DOMAIN_EXPORT static __name__##Ptr GetForEdit(Dgn::DgnDbR db, Dgn::DgnElementId id) { return db.Elements().GetForEdit< __name__ >(id); }

//-----------------------------------------------------------------------------------------
// All R, CR, P, CP, Ptr, CPtr declarations
//-----------------------------------------------------------------------------------------
STRUCTURAL_POINTER_TYPEDEFS(MaterialProperties)
STRUCTURAL_POINTER_TYPEDEFS(ConcreteMaterialProperties)
STRUCTURAL_POINTER_TYPEDEFS(SteelMaterialProperties)

