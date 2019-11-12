/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once

//__PUBLISH_SECTION_START__
#include <Bentley/Bentley.h>
#include <Bentley/WString.h>
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
#define USING_NAMESPACE_BENTLEY_STRUCTURAL using namespace BentleyApi::Structural;

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
#define BENTLEY_STRUCTURAL_PHYSICAL_SCHEMA_NAME                        "StructuralPhysical"
#define BENTLEY_STRUCTURAL_PHYSICAL_SCHEMA_PATH                        L"ECSchemas/Domain/StructuralPhysical.ecschema.xml"
#define BENTLEY_STRUCTURAL_PHYSICAL_SCHEMA(className)                  BENTLEY_STRUCTURAL_PHYSICAL_SCHEMA_NAME "." className
#define BENTLEY_STRUCTURAL_PHYSICAL_SCHEMA_CODE                        BENTLEY_STRUCTURAL_PHYSICAL_SCHEMA_NAME
#define BENTLEY_STRUCTURAL_PHYSICAL_AUTHORITY                          BENTLEY_STRUCTURAL_PHYSICAL_SCHEMA_NAME

//-----------------------------------------------------------------------------------------
// Categories
//-----------------------------------------------------------------------------------------
#define STRUCTURAL_PHYSICAL_CATEGORY_StructuralCategory              "StructuralCategory"
#define STRUCTURAL_COMMON_CLASS_StructuralPhysicalModel              "StructuralPhysicalModel"
//-----------------------------------------------------------------------------------------
// ECClass names
//-----------------------------------------------------------------------------------------
#define STRUCTURAL_PHYSICAL_CLASS_StructuralElement "StructuralElement"
#define STRUCTURAL_PHYSICAL_CLASS_StructuralMember "StructuralMember"
#define STRUCTURAL_PHYSICAL_CLASS_Slab "Slab"
#define STRUCTURAL_PHYSICAL_CLASS_Wall "Wall"
#define STRUCTURAL_PHYSICAL_CLASS_Beam "Beam"
#define STRUCTURAL_PHYSICAL_CLASS_Column "Column"
#define STRUCTURAL_PHYSICAL_CLASS_Brace "Brace"
#define STRUCTURAL_PHYSICAL_CLASS_FoundationMember "FoundationMember"
#define STRUCTURAL_PHYSICAL_CLASS_StripFooting "StripFooting"
#define STRUCTURAL_PHYSICAL_CLASS_SpreadFooting "SpreadFooting"
#ifdef _EXCLUDED_FROM_EAP_BUILD_
    #define STRUCTURAL_PHYSICAL_CLASS_StructuralSubtraction "StructuralSubtraction"
    #define STRUCTURAL_PHYSICAL_CLASS_StructuralAddition "StructuralAddition"
    #define STRUCTURAL_PHYSICAL_CLASS_MaterialProperties            "MaterialProperties"
    #define STRUCTURAL_PHYSICAL_CLASS_ConcreteMaterialProperties    "ConcreteMaterialProperties"
    #define STRUCTURAL_PHYSICAL_CLASS_SteelMaterialProperties       "SteelMaterialProperties"
    #define STRUCTURAL_PHYSICAL_CLASS_ICURVE_MEMBER "ICurveMember"
    #define STRUCTURAL_PHYSICAL_CLASS_ISURFACE_MEMBER "ISurfaceMember"
#endif
#define STRUCTURAL_PHYSICAL_CLASS_Pile "Pile"
#define STRUCTURAL_PHYSICAL_CLASS_PileCap "PileCap"
//-----------------------------------------------------------------------------------------
// Define standard static QueryClass/QueryClassId methods on Elements and Aspects
//-----------------------------------------------------------------------------------------
#define DECLARE_STRUCTURAL_PHYSICAL_QUERYCLASS_METHODS(__name__) \
    static Dgn::DgnClassId QueryClassId(Dgn::DgnDbCR db) { return Dgn::DgnClassId(db.Schemas().GetClassId(BENTLEY_STRUCTURAL_PHYSICAL_SCHEMA_NAME, STRUCTURAL_PHYSICAL_CLASS_##__name__)); } \
    static ECN::ECClassCP QueryClass(Dgn::DgnDbCR db) { return (db.Schemas().GetClass(BENTLEY_STRUCTURAL_PHYSICAL_SCHEMA_NAME, STRUCTURAL_PHYSICAL_CLASS_##__name__)); }

//-----------------------------------------------------------------------------------------
// Macro to declare Get, GetForEdit, Insert, Update methods on elements. Pointers (Ptr, CPtr) must be defined.
//-----------------------------------------------------------------------------------------
#define DECLARE_STRUCTURAL_PHYSICAL_ELEMENT_BASE_GET_METHODS(__name__) \
    STRUCTURAL_DOMAIN_EXPORT static __name__##CPtr Get(Dgn::DgnDbR db, Dgn::DgnElementId id) { return db.Elements().Get< __name__ >(id); } \
    STRUCTURAL_DOMAIN_EXPORT static __name__##Ptr GetForEdit(Dgn::DgnDbR db, Dgn::DgnElementId id) { return db.Elements().GetForEdit< __name__ >(id); }

//-----------------------------------------------------------------------------------------
// All R, CR, P, CP, Ptr, CPtr declarations
//-----------------------------------------------------------------------------------------
STRUCTURAL_POINTER_TYPEDEFS(StructuralPhysicalModel)
STRUCTURAL_POINTER_TYPEDEFS(StructuralElement)
STRUCTURAL_POINTER_TYPEDEFS(StructuralMember)
STRUCTURAL_POINTER_TYPEDEFS(Slab)
STRUCTURAL_POINTER_TYPEDEFS(Wall)
STRUCTURAL_POINTER_TYPEDEFS(Beam)
STRUCTURAL_POINTER_TYPEDEFS(Column)
STRUCTURAL_POINTER_TYPEDEFS(Brace)
STRUCTURAL_POINTER_TYPEDEFS(FoundationMember)
STRUCTURAL_POINTER_TYPEDEFS(StripFooting)
STRUCTURAL_POINTER_TYPEDEFS(SpreadFooting)
#ifdef _EXCLUDED_FROM_EAP_BUILD_
    STRUCTURAL_POINTER_TYPEDEFS(StructuralSubtraction)
    STRUCTURAL_POINTER_TYPEDEFS(StructuralAddition)
    STRUCTURAL_POINTER_TYPEDEFS(MaterialProperties)
    STRUCTURAL_POINTER_TYPEDEFS(ConcreteMaterialProperties)
    STRUCTURAL_POINTER_TYPEDEFS(SteelMaterialProperties)
#endif
STRUCTURAL_POINTER_TYPEDEFS(Pile)
STRUCTURAL_POINTER_TYPEDEFS(PileCap)

