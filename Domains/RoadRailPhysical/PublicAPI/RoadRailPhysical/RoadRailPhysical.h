/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/RoadRailPhysical/RoadRailPhysical.h $
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

// create the Bentley.ConceptCivil namespace
BEGIN_BENTLEY_ROADRAILPHYSICAL_NAMESPACE
END_BENTLEY_ROADRAILPHYSICAL_NAMESPACE

//-----------------------------------------------------------------------------------------
// ECSchema name and relative path
//-----------------------------------------------------------------------------------------
#define BRRP_SCHEMA_NAME                             "RoadRailPhysical"
#define BRRP_SCHEMA_FILE                             L"RoadRailPhysical.ecschema.xml"
#define BRRP_SCHEMA_LOCATION                         L"ECSchemas/Domain/"
#define BRRP_SCHEMA_PATH                             BRRP_SCHEMA_LOCATION BRRP_SCHEMA_FILE
#define BRRP_SCHEMA(name)                            BRRP_SCHEMA_NAME "." name
#define BRRP_SCHEMA_CODE(name)                       BRRP_SCHEMA_NAME "_" name


//-----------------------------------------------------------------------------------------
// ECClass names
//-----------------------------------------------------------------------------------------

// Models
#define BRRP_CLASS_RailwayStandardsModel                            "RailwayStandardsModel"
#define BRRP_CLASS_RoadRailPhysicalModel                            "RoadRailPhysicalModel"
#define BRRP_CLASS_RoadwayStandardsModel                            "RoadwayStandardsModel"


// Elements
#define BRRP_CLASS_Corridor                                         "Corridor"
#define BRRP_CLASS_CorridorPortionElement                           "CorridorPortionElement"
#define BRRP_CLASS_DesignSpeed                                      "DesignSpeed"
#define BRRP_CLASS_DesignSpeedDefinition                            "DesignSpeedDefinition"
#define BRRP_CLASS_DesignSpeedElement                               "DesignSpeedElement"
#define BRRP_CLASS_DesignSpeedTransition                            "DesignSpeedTransition"
#define BRRP_CLASS_GenericTypicalSectionPointDefinition             "GenericTypicalSectionPointDefinition"
#define BRRP_CLASS_PathwayElement                                   "PathwayElement"
#define BRRP_CLASS_Railway                                          "Railway"
#define BRRP_CLASS_RoadRailNetwork                                  "RoadRailNetwork"
#define BRRP_CLASS_Roadway                                          "Roadway"
#define BRRP_CLASS_TypicalSectionPointDefinition                    "TypicalSectionPointDefinition"


// Aspects
#define BRRP_CLASS_AssociatedFacetAspect                            "AssociatedFacetAspect"


// Relationships
#define BRRP_REL_CorridorAssemblesPortions                          "CorridorAssemblesPortions"
#define BRRP_REL_CorridorRefersToOrderedPathways                    "CorridorRefersToOrderedPathways"
#define BRRP_REL_DesignSpeedRefersToDefinition                      "DesignSpeedRefersToDefinition"
#define BRRP_REL_DrawingGraphicRepresentsCorridor                   "DrawingGraphicRepresentsCorridor"
#define BRRP_REL_GeometricElementBoundsContentForSheet              "GeometricElementBoundsContentForSheet"
#define BRRP_REL_GraphicalElement3dRepresentsCorridor               "GraphicalElement3dRepresentsCorridor"
#define BRRP_REL_ILinearElementRelatesToCorridorPortion             "ILinearElementRelatesToCorridorPortion"
#define BRRP_REL_ILinearElementSourceRefersToMainLinearElement      "ILinearElementSourceRefersToMainLinearElement"
#define BRRP_REL_PathwayAssemblesElements                           "PathwayAssemblesElements"
#define BRRP_REL_PathwayOwnsLinearlyLocatedAttribution              "PathwayOwnsLinearlyLocatedAttribution"


// Properties
#define BRRP_PROP_AssociatedFacetAspect_AssociatedFacet             "AssociatedFacet"


//-----------------------------------------------------------------------------------------
// Category names
//-----------------------------------------------------------------------------------------
#define BRRP_CATEGORY_Corridor                                      "Corridor"
#define BRRP_CATEGORY_Railway                                       "Railway"
#define BRRP_CATEGORY_Roadway                                       "Roadway"



//-----------------------------------------------------------------------------------------
// CodeSpec names
//-----------------------------------------------------------------------------------------
#define BRRP_CODESPEC_Corridor                                      "Corridor"
#define BRRP_CODESPEC_DesignSpeedDefinition                         "DesignSpeedDefinition"
#define BRRP_CODESPEC_Pathway                                       "Pathway"
#define BRRP_CODESPEC_RoadRailNetwork                               "RoadRailNetwork"
#define BRRP_CODESPEC_TypicalSectionPointDefinition                 "TypicalSectionPointDefinition"



//-----------------------------------------------------------------------------------------
// Define standard static QueryClass/QueryClassId methods on Elements and Aspects
//-----------------------------------------------------------------------------------------
#define DECLARE_ROADRAILPHYSICAL_QUERYCLASS_METHODS(__name__) \
    static Dgn::DgnClassId QueryClassId(Dgn::DgnDbCR db) { return Dgn::DgnClassId(db.Schemas().GetClassId(BRRP_SCHEMA_NAME, BRRP_CLASS_##__name__)); } \
    static ECN::ECClassCP QueryClass(Dgn::DgnDbCR db) { return (db.Schemas().GetClass(BRRP_SCHEMA_NAME, BRRP_CLASS_##__name__)); }


//-----------------------------------------------------------------------------------------
// Macro to declare Get, GetForEdit, Insert, Update methods on elements. Pointers (Ptr, CPtr) must be defined.
//-----------------------------------------------------------------------------------------
#define DECLARE_ROADRAILPHYSICAL_ELEMENT_BASE_GET_METHODS(__name__) \
    ROADRAILPHYSICAL_EXPORT static __name__##CPtr Get       (Dgn::DgnDbR db, Dgn::DgnElementId id) { return db.Elements().Get< __name__ >(id); } \
    ROADRAILPHYSICAL_EXPORT static __name__##Ptr  GetForEdit(Dgn::DgnDbR db, Dgn::DgnElementId id) { return db.Elements().GetForEdit< __name__ >(id); }

#define DECLARE_ROADRAILPHYSICAL_ELEMENT_BASE_GET_UPDATE_METHODS(__name__) \
    DECLARE_ROADRAILPHYSICAL_ELEMENT_BASE_GET_METHODS(__name__) \
    ROADRAILPHYSICAL_EXPORT        __name__##CPtr Update(Dgn::DgnDbStatus* stat=nullptr) { return GetDgnDb().Elements().Update< __name__ >(*this, stat); }   

#define DECLARE_ROADRAILPHYSICAL_ELEMENT_BASE_METHODS(__name__) \
    DECLARE_ROADRAILPHYSICAL_ELEMENT_BASE_GET_UPDATE_METHODS(__name__) \
    ROADRAILPHYSICAL_EXPORT        __name__##CPtr Insert(Dgn::DgnDbStatus* stat=nullptr) { return GetDgnDb().Elements().Insert< __name__ >(*this, stat); }
    

//-----------------------------------------------------------------------------------------
// Define standard typedefs (P, CP, R, CR) in the RoadRailPhysical namespace
//-----------------------------------------------------------------------------------------
#define ROADRAILPHYSICAL_TYPEDEFS(_name_) \
    BEGIN_BENTLEY_ROADRAILPHYSICAL_NAMESPACE DEFINE_POINTER_SUFFIX_TYPEDEFS(_name_) END_BENTLEY_ROADRAILPHYSICAL_NAMESPACE

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


//-----------------------------------------------------------------------------------------
// Define typedefs and Ptrs in the RoadRailPhysical namespace
//-----------------------------------------------------------------------------------------

// Road & Rail shared
ROADRAILPHYSICAL_REFCOUNTED_PTR_AND_TYPEDEFS(AssociatedFacetAspect)
ROADRAILPHYSICAL_REFCOUNTED_PTR_AND_TYPEDEFS(Corridor)
ROADRAILPHYSICAL_REFCOUNTED_PTR_AND_TYPEDEFS(CorridorPortionElement)
ROADRAILPHYSICAL_REFCOUNTED_PTR_AND_TYPEDEFS(DesignSpeed)
ROADRAILPHYSICAL_REFCOUNTED_PTR_AND_TYPEDEFS(DesignSpeedDefinition)
ROADRAILPHYSICAL_REFCOUNTED_PTR_AND_TYPEDEFS(DesignSpeedElement)
ROADRAILPHYSICAL_REFCOUNTED_PTR_AND_TYPEDEFS(DesignSpeedTransition)
ROADRAILPHYSICAL_REFCOUNTED_PTR_AND_TYPEDEFS(GenericTypicalSectionPointDefinition)
ROADRAILPHYSICAL_REFCOUNTED_PTR_AND_TYPEDEFS(PathwayElement)
ROADRAILPHYSICAL_REFCOUNTED_PTR_AND_TYPEDEFS(RoadRailNetwork)
ROADRAILPHYSICAL_REFCOUNTED_PTR_AND_TYPEDEFS(RoadRailPhysicalModel)
ROADRAILPHYSICAL_REFCOUNTED_PTR_AND_TYPEDEFS(TypicalSectionPointDefinition)

// Road-specific
ROADRAILPHYSICAL_REFCOUNTED_PTR_AND_TYPEDEFS(Roadway)
ROADRAILPHYSICAL_REFCOUNTED_PTR_AND_TYPEDEFS(RoadwayStandardsModel)

// Rail-specific
ROADRAILPHYSICAL_REFCOUNTED_PTR_AND_TYPEDEFS(Railway)
ROADRAILPHYSICAL_REFCOUNTED_PTR_AND_TYPEDEFS(RailwayStandardsModel)
