/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
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
#include <LinearReferencing/LinearReferencingApi.h>
#include <RoadRailAlignment/RoadRailAlignmentApi.h>

#ifdef __ROADRAILPHYSICAL_BUILD__
#define ROADRAILPHYSICAL_EXPORT EXPORT_ATTRIBUTE
#else
#define ROADRAILPHYSICAL_EXPORT IMPORT_ATTRIBUTE
#endif

/** @namespace BentleyApi::RoadRailPhysical %RoadRailPhysical data types */
#define BEGIN_BENTLEY_ROADRAILPHYSICAL_NAMESPACE        BEGIN_BENTLEY_NAMESPACE namespace RoadRailPhysical {
#define END_BENTLEY_ROADRAILPHYSICAL_NAMESPACE          } END_BENTLEY_NAMESPACE
#define USING_NAMESPACE_BENTLEY_ROADRAILPHYSICAL        using namespace BENTLEY_NAMESPACE_NAME::RoadRailPhysical;

#define BEGIN_BENTLEY_ROADPHYSICAL_NAMESPACE        BEGIN_BENTLEY_NAMESPACE namespace RoadPhysical {
#define END_BENTLEY_ROADPHYSICAL_NAMESPACE          } END_BENTLEY_NAMESPACE
#define USING_NAMESPACE_BENTLEY_ROADPHYSICAL        using namespace BENTLEY_NAMESPACE_NAME::RoadPhysical;

#define BEGIN_BENTLEY_RAILPHYSICAL_NAMESPACE        BEGIN_BENTLEY_NAMESPACE namespace RailPhysical {
#define END_BENTLEY_RAILPHYSICAL_NAMESPACE          } END_BENTLEY_NAMESPACE
#define USING_NAMESPACE_BENTLEY_RAILPHYSICAL        using namespace BENTLEY_NAMESPACE_NAME::RailPhysical;

// create the Bentley.RoadRailPhysical namespace
BEGIN_BENTLEY_ROADRAILPHYSICAL_NAMESPACE
END_BENTLEY_ROADRAILPHYSICAL_NAMESPACE

BEGIN_BENTLEY_ROADPHYSICAL_NAMESPACE
END_BENTLEY_ROADPHYSICAL_NAMESPACE

BEGIN_BENTLEY_RAILPHYSICAL_NAMESPACE
END_BENTLEY_RAILPHYSICAL_NAMESPACE

//-----------------------------------------------------------------------------------------
// ECSchema name and relative path
//-----------------------------------------------------------------------------------------
#define BRRP_SCHEMA_NAME                             "RoadRailPhysical"
#define BRRP_SCHEMA_FILE                             L"RoadRailPhysical.ecschema.xml"
#define BRRP_SCHEMA_LOCATION                         L"ECSchemas/Domain/"
#define BRRP_SCHEMA_PATH                             BRRP_SCHEMA_LOCATION BRRP_SCHEMA_FILE
#define BRRP_SCHEMA(name)                            BRRP_SCHEMA_NAME "." name
#define BRRP_SCHEMA_CODE(name)                       BRRP_SCHEMA_NAME "_" name

#define BRDP_SCHEMA_NAME                             "RoadPhysical"
#define BRDP_SCHEMA_FILE                             L"RoadPhysical.ecschema.xml"
#define BRDP_SCHEMA_LOCATION                         L"ECSchemas/Domain/"
#define BRDP_SCHEMA_PATH                             BRDP_SCHEMA_LOCATION BRDP_SCHEMA_FILE
#define BRDP_SCHEMA(name)                            BRDP_SCHEMA_NAME "." name
#define BRDP_SCHEMA_CODE(name)                       BRDP_SCHEMA_NAME "_" name

#define BRLP_SCHEMA_NAME                             "RailPhysical"
#define BRLP_SCHEMA_FILE                             L"RailPhysical.ecschema.xml"
#define BRLP_SCHEMA_LOCATION                         L"ECSchemas/Domain/"
#define BRLP_SCHEMA_PATH                             BRLP_SCHEMA_LOCATION BRLP_SCHEMA_FILE
#define BRLP_SCHEMA(name)                            BRLP_SCHEMA_NAME "." name
#define BRLP_SCHEMA_CODE(name)                       BRLP_SCHEMA_NAME "_" name


//-----------------------------------------------------------------------------------------
// ECClass names
//-----------------------------------------------------------------------------------------

// Elements
#define BRRP_CLASS_Corridor                                         "Corridor"
#define BRRP_CLASS_CorridorPortionElement                           "CorridorPortionElement"
#define BRRP_CLASS_DesignSpeed                                      "DesignSpeed"
#define BRRP_CLASS_DesignSpeedDefinition                            "DesignSpeedDefinition"
#define BRRP_CLASS_PathwayDesignCriteria                            "PathwayDesignCriteria"
#define BRRP_CLASS_PathwayElement                                   "PathwayElement"
#define BRRP_CLASS_TransportationNetwork                            "TransportationNetwork"
#define BRRP_CLASS_TransportationSystem                             "TransportationSystem"
#define BRRP_CLASS_UndeterminedCorridorPortion                      "UndeterminedCorridorPortion"

#define BRLP_CLASS_RailNetwork                                      "RailNetwork"
#define BRLP_CLASS_Railway                                          "Railway"

#define BRDP_CLASS_RoadNetwork                                      "RoadNetwork"
#define BRDP_CLASS_Roadway                                          "Roadway"


// Aspects


// Relationships
#define BRRP_REL_CorridorPortionOwnsAlignments                      "CorridorPortionOwnsAlignments"
#define BRRP_REL_DesignSpeedRefersToEndDefinition                   "DesignSpeedRefersToEndDefinition"
#define BRRP_REL_DesignSpeedRefersToStartDefinition                 "DesignSpeedRefersToStartDefinition"
#define BRRP_REL_PathwayOwnsDesignCriteria                          "PathwayOwnsDesignCriteria"


// Properties
#define BRRP_PROP_DesignSpeed_EndDefinition                         "EndDefinition"
#define BRRP_PROP_DesignSpeed_StartDefinition                       "StartDefinition"
#define BRRP_PROP_DesignSpeedDefinition_DesignSpeed                 "DesignSpeed"
#define BRRP_PROP_DesignSpeedDefinition_UnitSystem                  "UnitSystem"
#define BRRP_PROP_CorridorPortionElement_MainAlignment              "MainAlignment"


//-----------------------------------------------------------------------------------------
// Category names
//-----------------------------------------------------------------------------------------
#define BRRP_CATEGORY_Corridor                                      "Corridor"
#define BRRP_CATEGORY_DesignSpeed                                   "DesignSpeed"
#define BRLP_CATEGORY_Railway                                       "Railway"
#define BRDP_CATEGORY_Roadway                                       "Roadway"
#define BRRP_CATEGORY_TransportationNetwork                         "Transportation Network"



//-----------------------------------------------------------------------------------------
// CodeSpec names
//-----------------------------------------------------------------------------------------
#define BRRP_CODESPEC_Corridor                                      "Corridor"
#define BRRP_CODESPEC_DesignSpeedDefinition                         "DesignSpeedDefinition"
#define BRRP_CODESPEC_Pathway                                       "Pathway"
#define BRRP_CODESPEC_TransportationSystem                          "TransportationSystem"

#define BRLP_CODESPEC_RailNetwork                                   "RailNetwork"

#define BRDP_CODESPEC_RoadNetwork                                   "RoadNetwork"



//-----------------------------------------------------------------------------------------
// Define standard static QueryClass/QueryClassId methods on Elements and Aspects
//-----------------------------------------------------------------------------------------
#define DECLARE_ROADRAILPHYSICAL_QUERYCLASS_METHODS(__name__) \
    static Dgn::DgnClassId QueryClassId(Dgn::DgnDbCR db) { return Dgn::DgnClassId(db.Schemas().GetClassId(BRRP_SCHEMA_NAME, BRRP_CLASS_##__name__)); } \
    static ECN::ECClassCP QueryClass(Dgn::DgnDbCR db) { return (db.Schemas().GetClass(BRRP_SCHEMA_NAME, BRRP_CLASS_##__name__)); }

#define DECLARE_ROADPHYSICAL_QUERYCLASS_METHODS(__name__) \
    static Dgn::DgnClassId QueryClassId(Dgn::DgnDbCR db) { return Dgn::DgnClassId(db.Schemas().GetClassId(BRDP_SCHEMA_NAME, BRDP_CLASS_##__name__)); } \
    static ECN::ECClassCP QueryClass(Dgn::DgnDbCR db) { return (db.Schemas().GetClass(BRDP_SCHEMA_NAME, BRDP_CLASS_##__name__)); }

#define DECLARE_RAILPHYSICAL_QUERYCLASS_METHODS(__name__) \
    static Dgn::DgnClassId QueryClassId(Dgn::DgnDbCR db) { return Dgn::DgnClassId(db.Schemas().GetClassId(BRLP_SCHEMA_NAME, BRLP_CLASS_##__name__)); } \
    static ECN::ECClassCP QueryClass(Dgn::DgnDbCR db) { return (db.Schemas().GetClass(BRLP_SCHEMA_NAME, BRLP_CLASS_##__name__)); }


//-----------------------------------------------------------------------------------------
// Macro to declare Get, GetForEdit, Insert, Update methods on elements. Pointers (Ptr, CPtr) must be defined.
//-----------------------------------------------------------------------------------------
#define DECLARE_ROADRAILPHYSICAL_ELEMENT_BASE_GET_METHODS(__name__, __dgnelementname__) \
    ROADRAILPHYSICAL_EXPORT static __name__##CPtr Get       (Dgn::DgnDbR db, Dgn::DgnElementId id) { auto cPtr = db.Elements().Get< __dgnelementname__ >(id); if (cPtr.IsNull()) return nullptr; return new __name__(*cPtr); } \
    ROADRAILPHYSICAL_EXPORT static __name__##Ptr  GetForEdit(Dgn::DgnDbR db, Dgn::DgnElementId id) { auto ptr = db.Elements().GetForEdit< __dgnelementname__ >(id); if (ptr.IsNull()) return nullptr; return new __name__(*ptr); }

#define DECLARE_ROADRAILPHYSICAL_ELEMENT_BASE_GET_UPDATE_METHODS(__name__, __dgnelementname__) \
    DECLARE_ROADRAILPHYSICAL_ELEMENT_BASE_GET_METHODS(__name__, __dgnelementname__) \
    ROADRAILPHYSICAL_EXPORT        __name__##CPtr Update(Dgn::DgnDbStatus* stat=nullptr) { auto cPtr = GetDgnDb().Elements().Update< __dgnelementname__ >(*getP(), stat); if (cPtr.IsNull()) return nullptr; return new __name__(*cPtr); }

#define DECLARE_ROADRAILPHYSICAL_ELEMENT_BASE_METHODS(__name__, __dgnelementname__) \
    DECLARE_ROADRAILPHYSICAL_ELEMENT_BASE_GET_UPDATE_METHODS(__name__, __dgnelementname__) \
    ROADRAILPHYSICAL_EXPORT        __name__##CPtr Insert(Dgn::DgnDbStatus* stat=nullptr) { auto cPtr = GetDgnDb().Elements().Insert< __dgnelementname__ >(*getP(), stat); if (cPtr.IsNull()) return nullptr; return new __name__(*cPtr); }
    

//-----------------------------------------------------------------------------------------
// Define standard typedefs (P, CP, R, CR) in the RoadRailPhysical namespace
//-----------------------------------------------------------------------------------------
#define ROADRAILPHYSICAL_TYPEDEFS(_name_) \
    BEGIN_BENTLEY_ROADRAILPHYSICAL_NAMESPACE DEFINE_POINTER_SUFFIX_TYPEDEFS(_name_) END_BENTLEY_ROADRAILPHYSICAL_NAMESPACE

#define ROADPHYSICAL_TYPEDEFS(_name_) \
    BEGIN_BENTLEY_ROADPHYSICAL_NAMESPACE DEFINE_POINTER_SUFFIX_TYPEDEFS(_name_) END_BENTLEY_ROADPHYSICAL_NAMESPACE

#define RAILPHYSICAL_TYPEDEFS(_name_) \
    BEGIN_BENTLEY_RAILPHYSICAL_NAMESPACE DEFINE_POINTER_SUFFIX_TYPEDEFS(_name_) END_BENTLEY_RAILPHYSICAL_NAMESPACE

//-----------------------------------------------------------------------------------------
// Define RefCountedPtr and CPtr types
//-----------------------------------------------------------------------------------------
#define ROADRAILPHYSICAL_REFCOUNTED_PTR(_name_) \
    BEGIN_BENTLEY_ROADRAILPHYSICAL_NAMESPACE struct _name_; DEFINE_REF_COUNTED_PTR(_name_) END_BENTLEY_ROADRAILPHYSICAL_NAMESPACE 

//-----------------------------------------------------------------------------------------
// Define both RefCounterPtr/CPtr and (P, CP, R, CR) types
//-----------------------------------------------------------------------------------------
#define ROADRAILPHYSICAL_REFCOUNTED_PTR_AND_TYPEDEFS(_name_) \
    BEGIN_BENTLEY_ROADRAILPHYSICAL_NAMESPACE \
        DEFINE_POINTER_SUFFIX_TYPEDEFS(_name_) \
        DEFINE_REF_COUNTED_PTR(_name_) \
    END_BENTLEY_ROADRAILPHYSICAL_NAMESPACE

#define ROADPHYSICAL_REFCOUNTED_PTR_AND_TYPEDEFS(_name_) \
    BEGIN_BENTLEY_ROADPHYSICAL_NAMESPACE \
        DEFINE_POINTER_SUFFIX_TYPEDEFS(_name_) \
        DEFINE_REF_COUNTED_PTR(_name_) \
    END_BENTLEY_ROADPHYSICAL_NAMESPACE

#define RAILPHYSICAL_REFCOUNTED_PTR_AND_TYPEDEFS(_name_) \
    BEGIN_BENTLEY_RAILPHYSICAL_NAMESPACE \
        DEFINE_POINTER_SUFFIX_TYPEDEFS(_name_) \
        DEFINE_REF_COUNTED_PTR(_name_) \
    END_BENTLEY_RAILPHYSICAL_NAMESPACE


//-----------------------------------------------------------------------------------------
// Define typedefs and Ptrs in the RoadRailPhysical namespace
//-----------------------------------------------------------------------------------------

// Road & Rail shared
ROADRAILPHYSICAL_REFCOUNTED_PTR_AND_TYPEDEFS(Corridor)
ROADRAILPHYSICAL_REFCOUNTED_PTR_AND_TYPEDEFS(CorridorPortionElement)
ROADRAILPHYSICAL_REFCOUNTED_PTR_AND_TYPEDEFS(DesignSpeed)
ROADRAILPHYSICAL_REFCOUNTED_PTR_AND_TYPEDEFS(DesignSpeedDefinition)
ROADRAILPHYSICAL_REFCOUNTED_PTR_AND_TYPEDEFS(PathwayDesignCriteria)
ROADRAILPHYSICAL_REFCOUNTED_PTR_AND_TYPEDEFS(PathwayElement)
ROADRAILPHYSICAL_REFCOUNTED_PTR_AND_TYPEDEFS(TransportationNetwork)
ROADRAILPHYSICAL_REFCOUNTED_PTR_AND_TYPEDEFS(TransportationSystem)
ROADRAILPHYSICAL_REFCOUNTED_PTR_AND_TYPEDEFS(UndeterminedCorridorPortion)

// Road-specific
ROADPHYSICAL_REFCOUNTED_PTR_AND_TYPEDEFS(RoadNetwork)
ROADPHYSICAL_REFCOUNTED_PTR_AND_TYPEDEFS(Roadway)

// Rail-specific
RAILPHYSICAL_REFCOUNTED_PTR_AND_TYPEDEFS(RailNetwork)
RAILPHYSICAL_REFCOUNTED_PTR_AND_TYPEDEFS(Railway)
