/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/DgnV8OpenRoadsDesigner/DgnV8OpenRoadsDesigner.h $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__
#include <Bentley/Bentley.h>
#include <ECDb/ECDbTypes.h>
#include <ECDb/ECDbApi.h>
#include <ECUnits/Units.h>
#include <ECObjects/ECSchema.h>
#include <DgnPlatform/DgnPlatformApi.h>
#include <DgnPlatform/DgnDb.h>

#ifdef __DGNV8OPENROADSDESIGNER_BUILD__
#define DGNV8OPENROADSDESIGNER_EXPORT EXPORT_ATTRIBUTE
#else
#define DGNV8OPENROADSDESIGNER_EXPORT IMPORT_ATTRIBUTE
#endif

#define BEGIN_BENTLEY_DGNV8OPENROADSDESIGNER_NAMESPACE        BEGIN_BENTLEY_NAMESPACE namespace DgnV8OpenRoadsDesigner {
#define END_BENTLEY_DGNV8OPENROADSDESIGNER_NAMESPACE          } END_BENTLEY_NAMESPACE
#define USING_NAMESPACE_BENTLEY_DGNV8OPENROADSDESIGNER        using namespace BENTLEY_NAMESPACE_NAME::DgnV8OpenRoadsDesigner;

BEGIN_BENTLEY_DGNV8OPENROADSDESIGNER_NAMESPACE
END_BENTLEY_DGNV8OPENROADSDESIGNER_NAMESPACE

#define V8ORD_SCHEMA_NAME                             "DgnV8OpenRoadsDesigner"
#define V8ORD_SCHEMA_FILE                             L"DgnV8OpenRoadsDesigner.ecschema.xml"
#define V8ORD_SCHEMA_LOCATION                         L"ECSchemas/Domain/"
#define V8ORD_SCHEMA_PATH                             V8ORD_SCHEMA_LOCATION V8ORD_SCHEMA_FILE
#define V8ORD_SCHEMA(name)                            V8ORD_SCHEMA_NAME "." name
#define V8ORD_SCHEMA_CODE(name)                       V8ORD_SCHEMA_NAME "_" name


//-----------------------------------------------------------------------------------------
// ECClass names
//-----------------------------------------------------------------------------------------

// Models


// Elements


// Aspects
#define V8ORD_CLASS_CorridorSurfaceAspect                            "CorridorSurfaceAspect"
#define V8ORD_CLASS_FeatureAspect                                    "FeatureAspect"


// Relationships


// Properties
#define V8ORD_PROP_CorridorSurfaceAspect_IsTopMesh                               "IsTopMesh"
#define V8ORD_PROP_CorridorSurfaceAspect_IsBottomMesh                            "IsBottomMesh"
#define V8ORD_PROP_FeatureAspect_Name                                            "Name"
#define V8ORD_PROP_FeatureAspect_DefinitionName                                  "DefinitionName"


//-----------------------------------------------------------------------------------------
// Category names
//-----------------------------------------------------------------------------------------

//-----------------------------------------------------------------------------------------
// CodeSpec names
//-----------------------------------------------------------------------------------------

//-----------------------------------------------------------------------------------------
// Define standard static QueryClass/QueryClassId methods on Elements and Aspects
//-----------------------------------------------------------------------------------------
#define DECLARE_DGNV8OPENROADSDESIGNER_QUERYCLASS_METHODS(__name__) \
static Dgn::DgnClassId QueryClassId(Dgn::DgnDbCR db) { return Dgn::DgnClassId(db.Schemas().GetClassId(V8ORD_SCHEMA_NAME, V8ORD_CLASS_##__name__)); } \
static ECN::ECClassCP QueryClass(Dgn::DgnDbCR db) { return (db.Schemas().GetClass(V8ORD_SCHEMA_NAME, V8ORD_CLASS_##__name__)); }


//-----------------------------------------------------------------------------------------
// Macro to declare Get, GetForEdit, Insert, Update methods on elements. Pointers (Ptr, CPtr) must be defined.
//-----------------------------------------------------------------------------------------
#define DECLARE_DGNV8OPENROADSDESIGNER_ELEMENT_BASE_GET_METHODS(__name__) \
DGNV8OPENROADSDESIGNER_EXPORT static __name__##CPtr Get       (Dgn::DgnDbR db, Dgn::DgnElementId id) { return db.Elements().Get< __name__ >(id); } \
DGNV8OPENROADSDESIGNER_EXPORT static __name__##Ptr  GetForEdit(Dgn::DgnDbR db, Dgn::DgnElementId id) { return db.Elements().GetForEdit< __name__ >(id); }

#define DECLARE_DGNV8OPENROADSDESIGNER_ELEMENT_BASE_GET_UPDATE_METHODS(__name__) \
DECLARE_DGNV8OPENROADSDESIGNER_ELEMENT_BASE_GET_METHODS(__name__) \
DGNV8OPENROADSDESIGNER_EXPORT        __name__##CPtr Update(Dgn::DgnDbStatus* stat=nullptr) { return GetDgnDb().Elements().Update< __name__ >(*this, stat); }

#define DECLARE_DGNV8OPENROADSDESIGNER_ELEMENT_BASE_METHODS(__name__) \
DECLARE_DGNV8OPENROADSDESIGNER_ELEMENT_BASE_GET_UPDATE_METHODS(__name__) \
DGNV8OPENROADSDESIGNER_EXPORT        __name__##CPtr Insert(Dgn::DgnDbStatus* stat=nullptr) { return GetDgnDb().Elements().Insert< __name__ >(*this, stat); }


//-----------------------------------------------------------------------------------------
// Define standard typedefs (P, CP, R, CR) in the DgnV8OpenRoadsDesigner namespace
//-----------------------------------------------------------------------------------------
#define DGNV8OPENROADSDESIGNER_TYPEDEFS(_name_) \
BEGIN_BENTLEY_DGNV8OPENROADSDESIGNER_NAMESPACE DEFINE_POINTER_SUFFIX_TYPEDEFS(_name_) END_BENTLEY_DGNV8OPENROADSDESIGNER_NAMESPACE

//-----------------------------------------------------------------------------------------
// Define RefCountedPtr and CPtr types
//-----------------------------------------------------------------------------------------
#define DGNV8OPENROADSDESIGNER_REFCOUNTED_PTR(_name_) \
BEGIN_BENTLEY_DGNV8OPENROADSDESIGNER_NAMESPACE struct _name_; DEFINE_REF_COUNTED_PTR(_name_) END_BENTLEY_DGNV8OPENROADSDESIGNER_NAMESPACE

//-----------------------------------------------------------------------------------------
// Define both RefCounterPtr/CPtr and (P, CP, R, CR) types
//-----------------------------------------------------------------------------------------
#define DGNV8OPENROADSDESIGNER_REFCOUNTED_PTR_AND_TYPEDEFS(_name_) \
BEGIN_BENTLEY_DGNV8OPENROADSDESIGNER_NAMESPACE \
    DEFINE_POINTER_SUFFIX_TYPEDEFS(_name_) \
    DEFINE_REF_COUNTED_PTR(_name_) \
END_BENTLEY_DGNV8OPENROADSDESIGNER_NAMESPACE


//-----------------------------------------------------------------------------------------
// Define typedefs and Ptrs in the DgnV8OpenRoadsDesigner namespace
//-----------------------------------------------------------------------------------------
DGNV8OPENROADSDESIGNER_REFCOUNTED_PTR_AND_TYPEDEFS(CorridorSurfaceAspect)
DGNV8OPENROADSDESIGNER_REFCOUNTED_PTR_AND_TYPEDEFS(FeatureAspect)


//-----------------------------------------------------------------------------------------
// Define enums in the DgnV8OpenRoadsDesigner namespace
//-----------------------------------------------------------------------------------------
