/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__
#include <Bentley/Bentley.h>
#include <ECDb/ECDbTypes.h>
#include <ECDb/ECDbApi.h>
#include <ECUnits/Units.h>
#include <ECObjects/ECSchema.h>
#include <DgnPlatform/DgnPlatformApi.h>
#include <DgnPlatform/DgnDb.h>

#ifdef __PHYSICALREBAR_BUILD__
#define PHYSICALREBAR_EXPORT EXPORT_ATTRIBUTE
#else
#define PHYSICALREBAR_EXPORT IMPORT_ATTRIBUTE
#endif

#define BEGIN_BENTLEY_PHYSICALREBAR_NAMESPACE        BEGIN_BENTLEY_NAMESPACE namespace PhysicalRebar {
#define END_BENTLEY_PHYSICALREBAR_NAMESPACE          } END_BENTLEY_NAMESPACE
#define USING_NAMESPACE_BENTLEY_PHYSICALREBAR        using namespace BENTLEY_NAMESPACE_NAME::PhysicalRebar;

BEGIN_BENTLEY_PHYSICALREBAR_NAMESPACE
END_BENTLEY_PHYSICALREBAR_NAMESPACE

#define SPR_SCHEMA_NAME                             "PhysicalRebar"
#define SPR_SCHEMA_FILE                             L"PhysicalRebar.ecschema.xml"
#define SPR_SCHEMA_LOCATION                         L"PhysicalRebarECSchemas/Domain/"
#define SPR_SCHEMA_PATH                             SPR_SCHEMA_LOCATION SPR_SCHEMA_FILE
#define SPR_SCHEMA(name)                            SPR_SCHEMA_NAME "." name
#define SPR_SCHEMA_CODE(name)                       SPR_SCHEMA_NAME "_" name


//-----------------------------------------------------------------------------------------
// ECClass names
//-----------------------------------------------------------------------------------------

// Models


// Elements
#define SPR_CLASS_Rebar                                              "Rebar"
#define SPR_CLASS_RebarAccessory                                     "RebarAccessory"
#define SPR_CLASS_RebarAccessoryType                                 "RebarAccessoryType"
#define SPR_CLASS_RebarAssembly                                      "RebarAssembly"
#define SPR_CLASS_RebarEndDevice                                     "RebarEndDevice"
#define SPR_CLASS_RebarEndDeviceType                                 "RebarEndDeviceType"
#define SPR_CLASS_RebarEndTreatment                                  "RebarEndTreatment"
#define SPR_CLASS_RebarMaterial                                      "RebarMaterial"
#define SPR_CLASS_RebarMechanicalSplice                              "RebarMechanicalSplice"
#define SPR_CLASS_RebarMechanicalSpliceType                          "RebarMechanicalSpliceType"
#define SPR_CLASS_RebarSet                                           "RebarSet"
#define SPR_CLASS_RebarSize                                          "RebarSize"
#define SPR_CLASS_RebarSplicedEnd                                    "RebarSplicedEnd"
#define SPR_CLASS_RebarTerminator                                    "RebarTerminator"
#define SPR_CLASS_RebarTerminatorType                                "RebarTerminatorType"
#define SPR_CLASS_RebarType                                          "RebarType"


// Aspects


// Relationships


// Properties
#define SPR_PROP_RebarSize_Name                                                  "Name"
#define SPR_PROP_RebarSize_Diameter                                              "Diameter"
#define SPR_PROP_RebarSize_Area                                                  "Area"
#define SPR_PROP_RebarSize_Publisher                                             "Publisher"
#define SPR_PROP_RebarType_Shape                                                 "Shape"
#define SPR_PROP_RebarType_SimplifiedShape                                       "SimplifiedShape"


//-----------------------------------------------------------------------------------------
// Category names
//-----------------------------------------------------------------------------------------

//-----------------------------------------------------------------------------------------
// CodeSpec names
//-----------------------------------------------------------------------------------------

//-----------------------------------------------------------------------------------------
// Define standard static QueryClass/QueryClassId methods on Elements and Aspects
//-----------------------------------------------------------------------------------------
#define DECLARE_PHYSICALREBAR_QUERYCLASS_METHODS(__name__) \
static Dgn::DgnClassId QueryClassId(Dgn::DgnDbCR db) { return Dgn::DgnClassId(db.Schemas().GetClassId(SPR_SCHEMA_NAME, SPR_CLASS_##__name__)); } \
static ECN::ECClassCP QueryClass(Dgn::DgnDbCR db) { return (db.Schemas().GetClass(SPR_SCHEMA_NAME, SPR_CLASS_##__name__)); }


//-----------------------------------------------------------------------------------------
// Macro to declare Get, GetForEdit, Insert, Update methods on elements. Pointers (Ptr, CPtr) must be defined.
//-----------------------------------------------------------------------------------------
#define DECLARE_PHYSICALREBAR_ELEMENT_BASE_GET_METHODS(__name__) \
PHYSICALREBAR_EXPORT static __name__##CPtr Get       (Dgn::DgnDbR db, Dgn::DgnElementId id) { return db.Elements().Get< __name__ >(id); } \
PHYSICALREBAR_EXPORT static __name__##Ptr  GetForEdit(Dgn::DgnDbR db, Dgn::DgnElementId id) { return db.Elements().GetForEdit< __name__ >(id); }

#define DECLARE_PHYSICALREBAR_ELEMENT_BASE_GET_UPDATE_METHODS(__name__) \
DECLARE_PHYSICALREBAR_ELEMENT_BASE_GET_METHODS(__name__) \
PHYSICALREBAR_EXPORT        __name__##CPtr Update(Dgn::DgnDbStatus* stat=nullptr) { return GetDgnDb().Elements().Update< __name__ >(*this, stat); }

#define DECLARE_PHYSICALREBAR_ELEMENT_BASE_METHODS(__name__) \
DECLARE_PHYSICALREBAR_ELEMENT_BASE_GET_UPDATE_METHODS(__name__) \
PHYSICALREBAR_EXPORT        __name__##CPtr Insert(Dgn::DgnDbStatus* stat=nullptr) { return GetDgnDb().Elements().Insert< __name__ >(*this, stat); }


//-----------------------------------------------------------------------------------------
// Define standard typedefs (P, CP, R, CR) in the PhysicalRebar namespace
//-----------------------------------------------------------------------------------------
#define PHYSICALREBAR_TYPEDEFS(_name_) \
BEGIN_BENTLEY_PHYSICALREBAR_NAMESPACE DEFINE_POINTER_SUFFIX_TYPEDEFS(_name_) END_BENTLEY_PHYSICALREBAR_NAMESPACE

//-----------------------------------------------------------------------------------------
// Define RefCountedPtr and CPtr types
//-----------------------------------------------------------------------------------------
#define PHYSICALREBAR_REFCOUNTED_PTR(_name_) \
BEGIN_BENTLEY_PHYSICALREBAR_NAMESPACE struct _name_; DEFINE_REF_COUNTED_PTR(_name_) END_BENTLEY_PHYSICALREBAR_NAMESPACE

//-----------------------------------------------------------------------------------------
// Define both RefCounterPtr/CPtr and (P, CP, R, CR) types
//-----------------------------------------------------------------------------------------
#define PHYSICALREBAR_REFCOUNTED_PTR_AND_TYPEDEFS(_name_) \
BEGIN_BENTLEY_PHYSICALREBAR_NAMESPACE \
    DEFINE_POINTER_SUFFIX_TYPEDEFS(_name_) \
    DEFINE_REF_COUNTED_PTR(_name_) \
END_BENTLEY_PHYSICALREBAR_NAMESPACE


//-----------------------------------------------------------------------------------------
// Define typedefs and Ptrs in the PhysicalRebar namespace
//-----------------------------------------------------------------------------------------
PHYSICALREBAR_TYPEDEFS(IReinforcable)
PHYSICALREBAR_REFCOUNTED_PTR_AND_TYPEDEFS(Rebar)
PHYSICALREBAR_REFCOUNTED_PTR_AND_TYPEDEFS(RebarAccessory)
PHYSICALREBAR_REFCOUNTED_PTR_AND_TYPEDEFS(RebarAccessoryType)
PHYSICALREBAR_REFCOUNTED_PTR_AND_TYPEDEFS(RebarAssembly)
PHYSICALREBAR_REFCOUNTED_PTR_AND_TYPEDEFS(RebarEndDevice)
PHYSICALREBAR_REFCOUNTED_PTR_AND_TYPEDEFS(RebarEndDeviceType)
PHYSICALREBAR_REFCOUNTED_PTR_AND_TYPEDEFS(RebarEndTreatment)
PHYSICALREBAR_TYPEDEFS(RebarLayoutType)
PHYSICALREBAR_REFCOUNTED_PTR_AND_TYPEDEFS(RebarMaterial)
PHYSICALREBAR_REFCOUNTED_PTR_AND_TYPEDEFS(RebarMechanicalSplice)
PHYSICALREBAR_REFCOUNTED_PTR_AND_TYPEDEFS(RebarMechanicalSpliceType)
PHYSICALREBAR_REFCOUNTED_PTR_AND_TYPEDEFS(RebarSet)
PHYSICALREBAR_REFCOUNTED_PTR_AND_TYPEDEFS(RebarSize)
PHYSICALREBAR_REFCOUNTED_PTR_AND_TYPEDEFS(RebarSplicedEnd)
PHYSICALREBAR_REFCOUNTED_PTR_AND_TYPEDEFS(RebarTerminator)
PHYSICALREBAR_REFCOUNTED_PTR_AND_TYPEDEFS(RebarTerminatorType)
PHYSICALREBAR_REFCOUNTED_PTR_AND_TYPEDEFS(RebarType)


//-----------------------------------------------------------------------------------------
// Define enums in the PhysicalRebar namespace
//-----------------------------------------------------------------------------------------
