/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/RoadRailPhysical/RoadRailPhysical.h $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

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
#define BRRP_CLASS_DesignSpeedDefinitionModel                       "DesignSpeedDefinitionModel"
#define BRRP_CLASS_OverallTypicalSectionBreakDownModel              "OverallTypicalSectionBreakDownModel"
#define BRRP_CLASS_RoadClassDefinitionModel                         "RoadClassDefinitionModel"
#define BRRP_CLASS_RoadRailCategoryModel                            "RoadRailCategoryModel"
#define BRRP_CLASS_RoadwayStandardsModel                            "RoadwayStandardsModel"
#define BRRP_CLASS_TypicalSectionPortionBreakDownModel              "TypicalSectionPortionBreakDownModel"


// Elements
#define BRRP_CLASS_AlignmentIntersectionElement                     "AlignmentIntersectionElement"
#define BRRP_CLASS_DesignSpeed                                      "DesignSpeed"
#define BRRP_CLASS_DesignSpeedDefinitionElement                     "DesignSpeedDefinitionElement"
#define BRRP_CLASS_DesignSpeedDefinitionTable                       "DesignSpeedDefinitionTable"
#define BRRP_CLASS_LinearlyLocatedStatus                            "LinearlyLocatedStatus"
#define BRRP_CLASS_OverallTypicalSection                            "OverallTypicalSection"
#define BRRP_CLASS_PathwayElement                                   "PathwayElement"
#define BRRP_CLASS_Railway                                          "Railway"
#define BRRP_CLASS_RegularTravelwaySegment                          "RegularTravelwaySegment"
#define BRRP_CLASS_RoadTypicalSection                               "RoadTypicalSection"
#define BRRP_CLASS_RoadClass                                        "RoadClass"
#define BRRP_CLASS_RoadClassDefinition                              "RoadClassDefinition"
#define BRRP_CLASS_RoadClassDefinitionTable                         "RoadClassDefinitionTable"
#define BRRP_CLASS_RoadDesignSpeedDefinition                        "RoadDesignSpeedDefinition"
#define BRRP_CLASS_RoadIntersectionElement                          "RoadIntersectionElement"
#define BRRP_CLASS_RoadIntersectionLegElement                       "RoadIntersectionLegElement"
#define BRRP_CLASS_Roadway                                          "Roadway"
#define BRRP_CLASS_RoadLaneComponent                                "RoadLaneComponent"
#define BRRP_CLASS_RoadTravelwayDefinition                          "RoadTravelwayDefinition"
#define BRRP_CLASS_TravelwayComponentElement                        "TravelwayComponentElement"
#define BRRP_CLASS_TravelwayDefinitionElement                       "TravelwayDefinitionElement"
#define BRRP_CLASS_TravelwayIntersectionSegmentElement              "TravelwayIntersectionSegmentElement"
#define BRRP_CLASS_TravelwaySegmentElement                          "TravelwaySegmentElement"
#define BRRP_CLASS_TravelwaySideComponent                           "TravelwaySideComponent"
#define BRRP_CLASS_TravelwaySideDefinition                          "TravelwaySideDefinition"
#define BRRP_CLASS_TravelwayStructureComponent                      "TravelwayStructureComponent"
#define BRRP_CLASS_TravelwayStructureDefinition                     "TravelwayStructureDefinition"
#define BRRP_CLASS_TravelwayTransition                              "TravelwayTransition"
#define BRRP_CLASS_TypicalSectionComponentElement                   "TypicalSectionComponentElement"
#define BRRP_CLASS_TypicalSectionConstraintConstantOffset           "TypicalSectionConstraintConstantOffset"
#define BRRP_CLASS_TypicalSectionConstraintOffset                   "TypicalSectionConstraintOffset"
#define BRRP_CLASS_TypicalSectionConstraintSource                   "TypicalSectionConstraintSource"
#define BRRP_CLASS_TypicalSectionConstraintWithOffset               "TypicalSectionConstraintWithOffset"
#define BRRP_CLASS_TypicalSectionElement                            "TypicalSectionElement"
#define BRRP_CLASS_TypicalSectionHorizontalConstraint               "TypicalSectionHorizontalConstraint"
#define BRRP_CLASS_TypicalSectionOffsetParameter                    "TypicalSectionOffsetParameter"
#define BRRP_CLASS_TypicalSectionParameter                          "TypicalSectionParameter"
#define BRRP_CLASS_TypicalSectionPoint                              "TypicalSectionPoint"
#define BRRP_CLASS_TypicalSectionPointName                          "TypicalSectionPointName"
#define BRRP_CLASS_TypicalSectionPortion                            "TypicalSectionPortion"
#define BRRP_CLASS_TypicalSectionPortionElement                     "TypicalSectionPortionElement"
#define BRRP_CLASS_TypicalSectionVerticalConstraint                 "TypicalSectionVerticalConstraint"


// Aspects
#define BRRP_CLASS_StatusAspect                                     "StatusAspect"


// Relationships
#define BRRP_REL_DesignSpeedRefersToDefinition                      "DesignSpeedRefersToDefinition"
#define BRRP_REL_PathwayAssemblesElements                           "PathwayAssemblesElements"
#define BRRP_REL_PathwayOwnsDesignSpeeds                            "PathwayOwnsDesignSpeeds"
#define BRRP_REL_PathwayRefersToMainAlignment                       "PathwayRefersToMainAlignment"
#define BRRP_REL_PhysicalElementOwnsLinearlyLocatedStatus           "PhysicalElementOwnsLinearlyLocatedStatus"
#define BRRP_REL_RegularSegmentRefersToTravelwayDefinition          "RegularSegmentRefersToTravelwayDefinition"
#define BRRP_REL_RoadClassRefersToDefinition                        "RoadClassRefersToDefinition"
#define BRRP_REL_RoadwayOwnsRoadClasses                             "RoadwayOwnsRoadClasses"
#define BRRP_REL_TypicalSectionComponentGroupsPoints                "TypicalSectionComponentGroupsPoints"
#define BRRP_REL_TypicalSectionConstraintOwnsOffset                 "TypicalSectionConstraintOwnsOffset"
#define BRRP_REL_TypicalSectionPointConstraint                      "TypicalSectionPointConstraint"
#define BRRP_REL_TypicalSectionPointOwnsConstraintSource            "TypicalSectionPointOwnsConstraintSource"
#define BRRP_REL_TypicalSectionPointRefersToName                    "TypicalSectionPointRefersToName"


// Properties
#define BRRP_PROP_StatusAspect_Status                               "Status"


//-----------------------------------------------------------------------------------------
// Category names
//-----------------------------------------------------------------------------------------
#define BRRP_CATEGORY_Track                                         "Track"
#define BRRP_CATEGORY_Road                                          "Road"
#define BRRP_CATEGORY_TravelwayDefComponent                         "TravelwayDefComponent"
#define BRRP_CATEGORY_TravelwaySideDefComponent                     "TravelwaySideDefComponent"
#define BRRP_CATEGORY_TravelwayStructureDefComponent                "TravelwayStructureDefComponent"
#define BRRP_CATEGORY_TypicalSectionPoint                           "TypicalSectionPoint"


//-----------------------------------------------------------------------------------------
// CodeSpec names
//-----------------------------------------------------------------------------------------
#define BRRP_CODESPEC_DesignSpeedDefinition                        "DesignSpeedDefinition"
#define BRRP_CODESPEC_DesignSpeedDefinitionTable                   "DesignSpeedDefinitionTable"
#define BRRP_CODESPEC_OverallTypicalSection                        "OverallTypicalSection"
#define BRRP_CODESPEC_RoadClassDefinition                          "RoadClassDefinition"
#define BRRP_CODESPEC_RoadClassDefinitionTable                     "RoadClassDefinitionTable"
#define BRRP_CODESPEC_RoadTravelway                                "RoadTravelway"
#define BRRP_CODESPEC_TypicalSectionParameter                      "TypicalSectionParameter"
#define BRRP_CODESPEC_TypicalSectionPoint                          "TypicalSectionPoint"
#define BRRP_CODESPEC_TypicalSectionPointName                      "TypicalSectionPointName"


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
ROADRAILPHYSICAL_REFCOUNTED_PTR_AND_TYPEDEFS(AlignmentIntersectionElement)
ROADRAILPHYSICAL_REFCOUNTED_PTR_AND_TYPEDEFS(DesignSpeed)
ROADRAILPHYSICAL_REFCOUNTED_PTR_AND_TYPEDEFS(DesignSpeedDefinitionElement)
ROADRAILPHYSICAL_REFCOUNTED_PTR_AND_TYPEDEFS(DesignSpeedDefinitionModel)
ROADRAILPHYSICAL_REFCOUNTED_PTR_AND_TYPEDEFS(DesignSpeedDefinitionTable)
ROADRAILPHYSICAL_TYPEDEFS(ITypicalSectionConstraintPoint)
ROADRAILPHYSICAL_REFCOUNTED_PTR_AND_TYPEDEFS(LinearlyLocatedStatus)
ROADRAILPHYSICAL_REFCOUNTED_PTR_AND_TYPEDEFS(OverallTypicalSection)
ROADRAILPHYSICAL_REFCOUNTED_PTR_AND_TYPEDEFS(OverallTypicalSectionBreakDownModel)
ROADRAILPHYSICAL_REFCOUNTED_PTR_AND_TYPEDEFS(PathwayElement)
ROADRAILPHYSICAL_REFCOUNTED_PTR_AND_TYPEDEFS(RegularTravelwaySegment)
ROADRAILPHYSICAL_REFCOUNTED_PTR_AND_TYPEDEFS(RoadRailCategoryModel)
ROADRAILPHYSICAL_REFCOUNTED_PTR_AND_TYPEDEFS(StatusAspect)
ROADRAILPHYSICAL_REFCOUNTED_PTR_AND_TYPEDEFS(TravelwayComponentElement)
ROADRAILPHYSICAL_REFCOUNTED_PTR_AND_TYPEDEFS(TravelwayDefinitionElement)
ROADRAILPHYSICAL_REFCOUNTED_PTR_AND_TYPEDEFS(TravelwayIntersectionSegmentElement)
ROADRAILPHYSICAL_REFCOUNTED_PTR_AND_TYPEDEFS(TravelwaySegmentElement)
ROADRAILPHYSICAL_REFCOUNTED_PTR_AND_TYPEDEFS(TravelwaySideComponent)
ROADRAILPHYSICAL_REFCOUNTED_PTR_AND_TYPEDEFS(TravelwaySideDefinition)
ROADRAILPHYSICAL_REFCOUNTED_PTR_AND_TYPEDEFS(TravelwayStructureComponent)
ROADRAILPHYSICAL_REFCOUNTED_PTR_AND_TYPEDEFS(TravelwayStructureDefinition)
ROADRAILPHYSICAL_REFCOUNTED_PTR_AND_TYPEDEFS(TravelwayTransition)
ROADRAILPHYSICAL_REFCOUNTED_PTR_AND_TYPEDEFS(TypicalSectionComponentElement)
ROADRAILPHYSICAL_REFCOUNTED_PTR_AND_TYPEDEFS(TypicalSectionConstraintConstantOffset)
ROADRAILPHYSICAL_REFCOUNTED_PTR_AND_TYPEDEFS(TypicalSectionConstraintOffset)
ROADRAILPHYSICAL_REFCOUNTED_PTR_AND_TYPEDEFS(TypicalSectionConstraintSource)
ROADRAILPHYSICAL_REFCOUNTED_PTR_AND_TYPEDEFS(TypicalSectionConstraintWithOffset)
ROADRAILPHYSICAL_REFCOUNTED_PTR_AND_TYPEDEFS(TypicalSectionHorizontalConstraint)
ROADRAILPHYSICAL_REFCOUNTED_PTR_AND_TYPEDEFS(TypicalSectionOffsetParameter)
ROADRAILPHYSICAL_REFCOUNTED_PTR_AND_TYPEDEFS(TypicalSectionParameter)
ROADRAILPHYSICAL_REFCOUNTED_PTR_AND_TYPEDEFS(TypicalSectionPoint)
ROADRAILPHYSICAL_REFCOUNTED_PTR_AND_TYPEDEFS(TypicalSectionPointName)
ROADRAILPHYSICAL_REFCOUNTED_PTR_AND_TYPEDEFS(TypicalSectionPortionBreakDownModel)
ROADRAILPHYSICAL_REFCOUNTED_PTR_AND_TYPEDEFS(TypicalSectionPortion)
ROADRAILPHYSICAL_REFCOUNTED_PTR_AND_TYPEDEFS(TypicalSectionPortionElement)
ROADRAILPHYSICAL_REFCOUNTED_PTR_AND_TYPEDEFS(TypicalSectionVerticalConstraint)

// Road-specific
ROADRAILPHYSICAL_REFCOUNTED_PTR_AND_TYPEDEFS(RoadClassDefinition)
ROADRAILPHYSICAL_REFCOUNTED_PTR_AND_TYPEDEFS(RoadClassDefinitionModel)
ROADRAILPHYSICAL_REFCOUNTED_PTR_AND_TYPEDEFS(RoadClassDefinitionTable)
ROADRAILPHYSICAL_REFCOUNTED_PTR_AND_TYPEDEFS(RoadDesignSpeedDefinition)
ROADRAILPHYSICAL_REFCOUNTED_PTR_AND_TYPEDEFS(RoadIntersectionElement)
ROADRAILPHYSICAL_REFCOUNTED_PTR_AND_TYPEDEFS(RoadIntersectionLegElement)
ROADRAILPHYSICAL_REFCOUNTED_PTR_AND_TYPEDEFS(RoadClass)
ROADRAILPHYSICAL_REFCOUNTED_PTR_AND_TYPEDEFS(RoadDesignSpeed)
ROADRAILPHYSICAL_REFCOUNTED_PTR_AND_TYPEDEFS(RoadRailCategoryModel)
ROADRAILPHYSICAL_REFCOUNTED_PTR_AND_TYPEDEFS(RoadLaneComponent)
ROADRAILPHYSICAL_REFCOUNTED_PTR_AND_TYPEDEFS(RoadTravelwayDefinition)
ROADRAILPHYSICAL_REFCOUNTED_PTR_AND_TYPEDEFS(Roadway)
ROADRAILPHYSICAL_REFCOUNTED_PTR_AND_TYPEDEFS(RoadwayStandardsModel)

// Rail-specific
ROADRAILPHYSICAL_REFCOUNTED_PTR_AND_TYPEDEFS(Railway)
