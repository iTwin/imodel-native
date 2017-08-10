/*--------------------------------------------------------------------------------------+
|
|     $Source: StructuralPhysical/PublicAPI/StructuralPhysical/StructuralPhysicalDefinitions.h $
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
#define BENTLEY_STRUCTURAL_PHYSICAL_SCHEMA_NAME                        "StructuralPhysical"
#define BENTLEY_STRUCTURAL_PHYSICAL_SCHEMA_PATH                        L"ECSchemas/Domain/StructuralPhysical.ecschema.xml"
#define BENTLEY_STRUCTURAL_PHYSICAL_SCHEMA(className)                  BENTLEY_STRUCTURAL_PHYSICAL_SCHEMA_NAME "." className
#define BENTLEY_STRUCTURAL_PHYSICAL_SCHEMA_CODE                        BENTLEY_STRUCTURAL_PHYSICAL_SCHEMA_NAME
#define BENTLEY_STRUCTURAL_PHYSICAL_AUTHORITY                          BENTLEY_STRUCTURAL_PHYSICAL_SCHEMA_NAME

//-----------------------------------------------------------------------------------------
// ECClass names
//-----------------------------------------------------------------------------------------
#define STRUCTURAL_PHYSICAL_CLASS_StructuralElement "StructuralElement"
#define STRUCTURAL_PHYSICAL_CLASS_StructuralMember "StructuralMember"
#define STRUCTURAL_PHYSICAL_CLASS_SurfaceMember "SurfaceMember"
#define STRUCTURAL_PHYSICAL_CLASS_Slab "Slab"
#define STRUCTURAL_PHYSICAL_CLASS_Wall "Wall"
#define STRUCTURAL_PHYSICAL_CLASS_CurveMember "CurveMember"
#define STRUCTURAL_PHYSICAL_CLASS_Beam "Beam"
#define STRUCTURAL_PHYSICAL_CLASS_Column "Column"
#define STRUCTURAL_PHYSICAL_CLASS_Brace "Brace"
#define STRUCTURAL_PHYSICAL_CLASS_FoundationMember "FoundationMember"
#define STRUCTURAL_PHYSICAL_CLASS_StripFooting "StripFooting"
#define STRUCTURAL_PHYSICAL_CLASS_SpreadFooting "SpreadFooting"

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
STRUCTURAL_POINTER_TYPEDEFS(SurfaceMember)
STRUCTURAL_POINTER_TYPEDEFS(Slab)
STRUCTURAL_POINTER_TYPEDEFS(Wall)
STRUCTURAL_POINTER_TYPEDEFS(CurveMember)
STRUCTURAL_POINTER_TYPEDEFS(Beam)
STRUCTURAL_POINTER_TYPEDEFS(Column)
STRUCTURAL_POINTER_TYPEDEFS(Brace)
STRUCTURAL_POINTER_TYPEDEFS(FoundationMember)
STRUCTURAL_POINTER_TYPEDEFS(StripFooting)
STRUCTURAL_POINTER_TYPEDEFS(SpreadFooting)
