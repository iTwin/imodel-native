/*--------------------------------------------------------------------------------------+
|
|     $Source: ConcreteSchema/PublicAPI/Concrete/ConcreteDefinitions.h $
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
    #define CONCRETE_EXPORT EXPORT_ATTRIBUTE
#else
    #define CONCRETE_EXPORT IMPORT_ATTRIBUTE
#endif

/** @namespace BentleyApi::CONCRETE %CONCRETE data types */
#define BEGIN_BENTLEY_CONCRETE_NAMESPACE        BEGIN_BENTLEY_NAMESPACE namespace Concrete {
#define END_BENTLEY_CONCRETE_NAMESPACE          } END_BENTLEY_NAMESPACE
#define USING_NAMESPACE_BENTLEY_CONCRETE        using namespace BentleyApi::Concrete;

#define CONCRETE_TYPEDEFS(_structname_) \
    BEGIN_BENTLEY_CONCRETE_NAMESPACE DEFINE_POINTER_SUFFIX_TYPEDEFS(_structname_) END_BENTLEY_CONCRETE_NAMESPACE

#define CONCRETE_REFCOUNTED_TYPEDEFS(_structname_) \
    BEGIN_BENTLEY_CONCRETE_NAMESPACE struct _structname_; DEFINE_REF_COUNTED_PTR(_structname_) END_BENTLEY_CONCRETE_NAMESPACE

#define PRIMARY_BASELINE_LABEL "__Primary"

//-----------------------------------------------------------------------------------------
// ECSchema macros
//-----------------------------------------------------------------------------------------
#define BENTLEY_CONCRETE_SCHEMA_NAME                        "Concrete"
#define BENTLEY_CONCRETE_SCHEMA_PATH                        L"ECSchemas/Domain/Concrete.ecschema.xml"
#define BENTLEY_CONCRETE_SCHEMA(className)                  BENTLEY_CONCRETE_SCHEMA_NAME "." className
#define BENTLEY_CONCRETE_SCHEMA_CODE                        BENTLEY_CONCRETE_SCHEMA_NAME
#define BENTLEY_CONCRETE_AUTHORITY                          BENTLEY_CONCRETE_SCHEMA_NAME

//#define CONCRETE_SCHEMA(name)                               BENTLEY_CONCRETE_SCHEMA_NAME "." name

//-----------------------------------------------------------------------------------------
// ECClass names (combine with CONCRETE_SCHEMA macro for use in ECSql)
//-----------------------------------------------------------------------------------------
#define CONCRETE_CLASS_ConcreteElement                      "ConcreteElement"

#define CONCRETE_CLASS_FrameElement                         "FrameElement"
#define CONCRETE_CLASS_Beam                                 "Beam"
#define CONCRETE_CLASS_Column                               "Column"

#define CONCRETE_CLASS_SubStructureElement                  "SubStructureElement"
#define CONCRETE_CLASS_ShallowFoundationElement             "ShallowFoundationElement"
#define CONCRETE_CLASS_SpreadFooting                        "SpreadFooting"
#define CONCRETE_CLASS_StripFooting                         "StripFooting"

#define CONCRETE_CLASS_SurfaceElement                       "SurfaceElement"
#define CONCRETE_CLASS_Slab                                 "Slab"
#define CONCRETE_CLASS_Wall                                 "Wall"

//-----------------------------------------------------------------------------------------
// Category names
//-----------------------------------------------------------------------------------------
#define CONCRETE_CATEGORY_Beams                      BENTLEY_CONCRETE_SCHEMA_NAME "Beams"
#define CONCRETE_CATEGORY_Columns                    BENTLEY_CONCRETE_SCHEMA_NAME "Columns"
#define CONCRETE_CATEGORY_Slabs                      BENTLEY_CONCRETE_SCHEMA_NAME "Slabs"
#define CONCRETE_CATEGORY_Walls                      BENTLEY_CONCRETE_SCHEMA_NAME "Walls"

//-----------------------------------------------------------------------------------------
// All R, CR, P, CP, Ptr, CPtr declarations
//-----------------------------------------------------------------------------------------
CONCRETE_TYPEDEFS(ConcreteElement)
CONCRETE_REFCOUNTED_TYPEDEFS(ConcreteElement)

CONCRETE_TYPEDEFS(FrameElement)
CONCRETE_REFCOUNTED_TYPEDEFS(FrameElement)
CONCRETE_TYPEDEFS(Beam)
CONCRETE_REFCOUNTED_TYPEDEFS(Beam)
CONCRETE_TYPEDEFS(Column)
CONCRETE_REFCOUNTED_TYPEDEFS(Column)

CONCRETE_TYPEDEFS(SurfaceElement)
CONCRETE_REFCOUNTED_TYPEDEFS(SurfaceElement)
CONCRETE_TYPEDEFS(Slab)
CONCRETE_REFCOUNTED_TYPEDEFS(Slab)
CONCRETE_TYPEDEFS(Wall)
CONCRETE_REFCOUNTED_TYPEDEFS(Wall)
