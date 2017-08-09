/*--------------------------------------------------------------------------------------+
|
|     $Source: StructuralPhysical/PublicAPI/StructuralPhysical/StructuralPhysicalDefinitions.h $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

//__PUBLISH_SECTION_START__
#include <StructuralCommon/StructuralCommonDefinitions.h>

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
#define STRUCTURAL_PHYSICAL_StructuralElement = "StructuralElement"
#define STRUCTURAL_PHYSICAL_StructuralMember = "StructuralMember"
#define STRUCTURAL_PHYSICAL_SurfaceMember = "SurfaceMember"
#define STRUCTURAL_PHYSICAL_Slab = "Slab"
#define STRUCTURAL_PHYSICAL_Wall = "Wall"
#define STRUCTURAL_PHYSICAL_CurveMember = "CurveMember"
#define STRUCTURAL_PHYSICAL_Beam = "Beam"
#define STRUCTURAL_PHYSICAL_Column = "Column"
#define STRUCTURAL_PHYSICAL_Brace = "Brace"
#define STRUCTURAL_PHYSICAL_FoundationMember = "FoundationMember"
#define STRUCTURAL_PHYSICAL_StripFooting = "StripFooting"
#define STRUCTURAL_PHYSICAL_SpreadFooting = "SpreadFooting"

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
