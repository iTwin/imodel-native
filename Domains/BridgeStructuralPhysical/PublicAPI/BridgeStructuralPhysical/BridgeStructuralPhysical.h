/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once

#include <Bentley/Bentley.h>
#include <Bentley/BeAssert.h>
#include <Bentley/Nullable.h>

#include <ECDb/ECDbTypes.h>
#include <ECDb/ECDbApi.h>
#include <ECUnits/Units.h>
#include <ECObjects/ECSchema.h>

#include <DgnPlatform/DgnPlatformApi.h>
#include <DgnPlatform/DgnDb.h>
#include <DgnPlatform/DgnModel.h>

#include <LinearReferencing/LinearReferencingApi.h>
#include <RoadRailAlignment/RoadRailAlignmentApi.h>
#include <FormsDomain/FormsDomainApi.h>
// #include <ProfilesDomain/ProfilesDomainApi.h>
#include <StructuralDomain/StructuralPhysicalApi.h>
#include <CivilBaseGeometry/CivilBaseGeometryApi.h>

#ifdef __BRIDGESTRUCTURALPHYSICAL_BUILD__
#define BRIDGESTRUCTURALPHYSICAL_EXPORT EXPORT_ATTRIBUTE
#else
#define BRIDGESTRUCTURALPHYSICAL_EXPORT IMPORT_ATTRIBUTE
#endif

/** @namespace BentleyApi::BridgeStructuralPhysical %BridgeStructuralPhysical data types */
#define BEGIN_BENTLEY_BRIDGESTRUCTURALPHYSICAL_NAMESPACE        BEGIN_BENTLEY_NAMESPACE namespace BridgeStructuralPhysical {
#define END_BENTLEY_BRIDGESTRUCTURALPHYSICAL_NAMESPACE          } END_BENTLEY_NAMESPACE
#define USING_NAMESPACE_BENTLEY_BRIDGESTRUCTURALPHYSICAL        using namespace BENTLEY_NAMESPACE_NAME::BridgeStructuralPhysical;

// create the Bentley.ConceptCivil namespace
BEGIN_BENTLEY_BRIDGESTRUCTURALPHYSICAL_NAMESPACE
END_BENTLEY_BRIDGESTRUCTURALPHYSICAL_NAMESPACE

//-----------------------------------------------------------------------------------------
// ECSchema name and relative path
//-----------------------------------------------------------------------------------------
#define BBP_SCHEMA_NAME                             "BridgeStructuralPhysical"
#define BBP_SCHEMA_FILE                             L"BridgeStructuralPhysical.ecschema.xml"
#define BBP_SCHEMA_LOCATION                         L"ECSchemas/Domain/"
#define BBP_SCHEMA_PATH                             BBP_SCHEMA_LOCATION BBP_SCHEMA_FILE
#define BBP_SCHEMA(name)                            BBP_SCHEMA_NAME "." name
#define BBP_SCHEMA_CODE(name)                       BBP_SCHEMA_NAME "_" name
#define BENTLEY_BRIDGE_PHYSICAL_AUTHORITY       BBP_SCHEMA_NAME

//-----------------------------------------------------------------------------------------
// ECClass names
//-----------------------------------------------------------------------------------------

#define BBP_CLASS_Bridge                                            "Bridge"
#define BBP_CLASS_BridgeCategoryModel                               "BridgeCategoryModel"
#define BBP_CLASS_StructuralSystem									"StructuralSystem"
#define BBP_CLASS_SubstructureElement                               "SubstructureElement"
#define BBP_CLASS_SuperstructureElement                             "SuperstructureElement"
#define BBP_CLASS_GenericSubstructureElement                        "GenericSubstructureElement"
#define BBP_CLASS_GenericSuperstructureElement                      "GenericSuperstructureElement"


//-----------------------------------------------------------------------------------------
// ECCustomAttribute names
//-----------------------------------------------------------------------------------------

//-----------------------------------------------------------------------------------------
// ECRelationship names
//-----------------------------------------------------------------------------------------
#define BBP_REL_PhysicalModelBreaksDownBridgeElement                "PhysicalModelBreaksDownBridgeElement"
#define BBP_REL_SubstructureElementAssemblesStructuralMembers       "SubstructureElementAssemblesStructuralMembers"
#define BBP_REL_SuperstructureElementAssemblesStructuralMembers     "SuperstructureElementAssemblesStructuralMembers"
//-----------------------------------------------------------------------------------------
// ECProperty names
//-----------------------------------------------------------------------------------------
#define BBP_PROP_BridgeSuperstructure_FromSupport                   "FromSupport"
#define BBP_PROP_BridgeSuperstructure_ToSupport                     "ToSupport"

//-----------------------------------------------------------------------------------------
// Category names
//-----------------------------------------------------------------------------------------
#define BBP_CATEGORY_Bridge                                         "Bridge"

//-----------------------------------------------------------------------------------------
// CodeSpec names
//-----------------------------------------------------------------------------------------
#define BBP_CODESPEC_Bridge                                         "Bridge"
#define BBP_CODESPEC_StructuralSystem                               "StructuralSystem"


//-----------------------------------------------------------------------------------------
// Define standard static QueryClass/QueryClassId methods on Elements and Aspects
//-----------------------------------------------------------------------------------------
#define DECLARE_BRIDGESTRUCTURALPHYSICAL_QUERYCLASS_METHODS(__name__) \
    static Dgn::DgnClassId QueryClassId(Dgn::DgnDbCR db) { return Dgn::DgnClassId(db.Schemas().GetClassId(BBP_SCHEMA_NAME, BBP_CLASS_##__name__)); } \
    static ECN::ECClassCP QueryClass(Dgn::DgnDbCR db) { return (db.Schemas().GetClass(BBP_SCHEMA_NAME, BBP_CLASS_##__name__)); }


//-----------------------------------------------------------------------------------------
// Macro to declare Get, GetForEdit, Insert, Update methods on elements. Pointers (Ptr, CPtr) must be defined.
//-----------------------------------------------------------------------------------------
#define DECLARE_BRIDGESTRUCTURALPHYSICAL_ELEMENT_BASE_METHODS(__name__, __dgnelementname__) \
    BRIDGESTRUCTURALPHYSICAL_EXPORT static __name__##CPtr Get       (Dgn::DgnDbR db, Dgn::DgnElementId id) { auto cPtr = db.Elements().Get< __dgnelementname__ >(id); if (cPtr.IsNull()) return nullptr; return new __name__(*cPtr); } \
    BRIDGESTRUCTURALPHYSICAL_EXPORT static __name__##Ptr  GetForEdit(Dgn::DgnDbR db, Dgn::DgnElementId id) { auto ptr = db.Elements().GetForEdit< __dgnelementname__ >(id); if (ptr.IsNull()) return nullptr; return new __name__(*ptr); } \
    BRIDGESTRUCTURALPHYSICAL_EXPORT        __name__##CPtr Insert(Dgn::DgnDbStatus* stat=nullptr) { auto cPtr = GetDgnDb().Elements().Insert< __dgnelementname__ >(*getP(), stat); if (cPtr.IsNull()) return nullptr; return new __name__(*cPtr); } \
    BRIDGESTRUCTURALPHYSICAL_EXPORT        __name__##CPtr Update(Dgn::DgnDbStatus* stat=nullptr) { auto cPtr = GetDgnDb().Elements().Update< __dgnelementname__ >(*getP(), stat); if (cPtr.IsNull()) return nullptr; return new __name__(*cPtr); }


//-----------------------------------------------------------------------------------------
// Define standard typedefs (P, CP, R, CR) in the Conceptual namespace
//-----------------------------------------------------------------------------------------
#define BRIDGESTRUCTURALPHYSICAL_TYPEDEFS(_name_) \
    BEGIN_BENTLEY_BRIDGESTRUCTURALPHYSICAL_NAMESPACE DEFINE_POINTER_SUFFIX_TYPEDEFS(_name_) END_BENTLEY_BRIDGESTRUCTURALPHYSICAL_NAMESPACE


//-----------------------------------------------------------------------------------------
// Define RefCountedPtr and CPtr types
//-----------------------------------------------------------------------------------------
#define BRIDGESTRUCTURALPHYSICAL_REFCOUNTED_PTR(_name_) \
    BEGIN_BENTLEY_BRIDGESTRUCTURALPHYSICAL_NAMESPACE struct _name_; DEFINE_REF_COUNTED_PTR(_name_) END_BENTLEY_BRIDGESTRUCTURALPHYSICAL_NAMESPACE 


//-----------------------------------------------------------------------------------------
// Define typedefs and Ptrs in the BridgeStructuralPhysical namespace
//-----------------------------------------------------------------------------------------
BRIDGESTRUCTURALPHYSICAL_TYPEDEFS(Bridge)
BRIDGESTRUCTURALPHYSICAL_TYPEDEFS(StructuralSystem)
BRIDGESTRUCTURALPHYSICAL_TYPEDEFS(BridgeCategoryModel)
BRIDGESTRUCTURALPHYSICAL_TYPEDEFS(SubstructureElement)
BRIDGESTRUCTURALPHYSICAL_TYPEDEFS(SuperstructureElement)
BRIDGESTRUCTURALPHYSICAL_TYPEDEFS(GenericSubstructureElement)
BRIDGESTRUCTURALPHYSICAL_TYPEDEFS(GenericSuperstructureElement)

BRIDGESTRUCTURALPHYSICAL_REFCOUNTED_PTR(Bridge)
BRIDGESTRUCTURALPHYSICAL_REFCOUNTED_PTR(StructuralSystem)
BRIDGESTRUCTURALPHYSICAL_REFCOUNTED_PTR(BridgeCategoryModel)
BRIDGESTRUCTURALPHYSICAL_REFCOUNTED_PTR(SubstructureElement)
BRIDGESTRUCTURALPHYSICAL_REFCOUNTED_PTR(SuperstructureElement)
BRIDGESTRUCTURALPHYSICAL_REFCOUNTED_PTR(GenericSubstructureElement)
BRIDGESTRUCTURALPHYSICAL_REFCOUNTED_PTR(GenericSuperstructureElement)
