/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/RoadRailAlignment/RoadRailAlignment.h $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
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
#include <DgnPlatform/DgnCategory.h>
#include <LinearReferencing/LinearReferencingApi.h>


#ifdef __ROADRAILALIGNMENT_BUILD__
#define ROADRAILALIGNMENT_EXPORT EXPORT_ATTRIBUTE
#else
#define ROADRAILALIGNMENT_EXPORT IMPORT_ATTRIBUTE
#endif

/** @namespace BentleyApi::RoadRailAlignment %RoadRailAlignment data types */
#define BEGIN_BENTLEY_ROADRAILALIGNMENT_NAMESPACE        BEGIN_BENTLEY_NAMESPACE namespace RoadRailAlignment {
#define END_BENTLEY_ROADRAILALIGNMENT_NAMESPACE          } END_BENTLEY_NAMESPACE
#define USING_NAMESPACE_BENTLEY_ROADRAILALIGNMENT        using namespace BENTLEY_NAMESPACE_NAME::RoadRailAlignment;

// create the Bentley.ConceptCivil namespace
BEGIN_BENTLEY_ROADRAILALIGNMENT_NAMESPACE
END_BENTLEY_ROADRAILALIGNMENT_NAMESPACE

//-----------------------------------------------------------------------------------------
// ECSchema name and relative path
//-----------------------------------------------------------------------------------------
#define BRRA_SCHEMA_NAME                             "RoadRailAlignment"
#define BRRA_SCHEMA_FILE                             L"RoadRailAlignment.ecschema.xml"
#define BRRA_SCHEMA_LOCATION                         L"ECSchemas/Domain/"
#define BRRA_SCHEMA_PATH                             BRRA_SCHEMA_LOCATION BRRA_SCHEMA_FILE
#define BRRA_SCHEMA(name)                            BRRA_SCHEMA_NAME "." name
#define BRRA_SCHEMA_CODE(name)                       BRRA_SCHEMA_NAME "_" name


//-----------------------------------------------------------------------------------------
// ECClass names
//-----------------------------------------------------------------------------------------

// Elements
#define BRRA_CLASS_Alignment                                        "Alignment"
#define BRRA_CLASS_AlignmentCategoryModel                           "AlignmentCategoryModel"
#define BRRA_CLASS_AlignmentModel                                   "AlignmentModel"
#define BRRA_CLASS_AlignmentProfileViewDefinition                   "AlignmentProfileViewDefinition"
#define BRRA_CLASS_AlignmentReferentElement                         "AlignmentReferentElement"
#define BRRA_CLASS_AlignmentStation                                 "AlignmentStation"
#define BRRA_CLASS_AlignmentXSViewDefinition                        "AlignmentXSViewDefinition"
#define BRRA_CLASS_HorizontalAlignment                              "HorizontalAlignment"
#define BRRA_CLASS_HorizontalAlignmentsPortion                      "HorizontalAlignmentsPortion"
#define BRRA_CLASS_HorizontalAlignmentModel                         "HorizontalAlignmentModel"
#define BRRA_CLASS_VerticalAlignment                                "VerticalAlignment"
#define BRRA_CLASS_VerticalAlignmentModel                           "VerticalAlignmentModel"



// Relationships
#define BRRA_REL_AlignmentOwnsStations                              "AlignmentOwnsStations"
#define BRRA_REL_AlignmentRefersToHorizontal                        "AlignmentRefersToHorizontal"
#define BRRA_REL_AlignmentRefersToMainVertical                      "AlignmentRefersToMainVertical"
#define BRRA_REL_SpatialElementRefersToAlignment                    "SpatialElementRefersToAlignment"


// Properties
#define BRRA_PROP_Alignment_MainVerticalAlignment                   "MainVerticalAlignment"
#define BRRA_PROP_HorizontalAlignment_HorizontalGeometry            "HorizontalGeometry"
#define BRRA_PROP_VerticalAlignment_VerticalGeometry                "VerticalGeometry"


//-----------------------------------------------------------------------------------------
// Category names
//-----------------------------------------------------------------------------------------
#define BRRA_CATEGORY_Alignment                                     "Alignment"
#define BRRA_CATEGORY_HorizontalAlignment                           "Horizontal Alignment"
#define BRRA_CATEGORY_VerticalAlignment                             "Vertical Alignment"


//-----------------------------------------------------------------------------------------
// CodeSpec names
//-----------------------------------------------------------------------------------------
#define BRRA_CODESPEC_Alignment                                    "Alignment"

//-----------------------------------------------------------------------------------------
// Define standard static QueryClass/QueryClassId methods on Elements and Aspects
//-----------------------------------------------------------------------------------------
#define DECLARE_ROADRAILALIGNMENT_QUERYCLASS_METHODS(__name__) \
    static Dgn::DgnClassId QueryClassId(Dgn::DgnDbCR db) { return Dgn::DgnClassId(db.Schemas().GetClassId(BRRA_SCHEMA_NAME, BRRA_CLASS_##__name__)); } \
    static ECN::ECClassCP QueryClass(Dgn::DgnDbCR db) { return (db.Schemas().GetClass(BRRA_SCHEMA_NAME, BRRA_CLASS_##__name__)); }

//-----------------------------------------------------------------------------------------
// Macro to declare Get, GetForEdit, Insert, Update methods on elements. Pointers (Ptr, CPtr) must be defined.
//-----------------------------------------------------------------------------------------
#define DECLARE_ROADRAILALIGNMENT_ELEMENT_GET_METHODS(__name__) \
    ROADRAILALIGNMENT_EXPORT static __name__##CPtr Get       (Dgn::DgnDbR db, Dgn::DgnElementId id) { return db.Elements().Get< __name__ >(id); } \
    ROADRAILALIGNMENT_EXPORT static __name__##Ptr  GetForEdit(Dgn::DgnDbR db, Dgn::DgnElementId id) { return db.Elements().GetForEdit< __name__ >(id); }

//-----------------------------------------------------------------------------------------
// Macro to declare Get, GetForEdit, Insert, Update methods on elements. Pointers (Ptr, CPtr) must be defined.
//-----------------------------------------------------------------------------------------
#define DECLARE_ROADRAILALIGNMENT_ELEMENT_BASE_METHODS(__name__) \
    DECLARE_ROADRAILALIGNMENT_ELEMENT_GET_METHODS(__name__) \
    ROADRAILALIGNMENT_EXPORT        __name__##CPtr Insert(Dgn::DgnDbStatus* stat=nullptr) { return GetDgnDb().Elements().Insert< __name__ >(*this, stat); } \
    ROADRAILALIGNMENT_EXPORT        __name__##CPtr Update(Dgn::DgnDbStatus* stat=nullptr) { return GetDgnDb().Elements().Update< __name__ >(*this, stat); }   


//-----------------------------------------------------------------------------------------
// Define standard typedefs (P, CP, R, CR) in the RoadRailAlignment namespace
//-----------------------------------------------------------------------------------------
#define ROADRAILALIGNMENT_TYPEDEFS(_name_) \
    BEGIN_BENTLEY_ROADRAILALIGNMENT_NAMESPACE DEFINE_POINTER_SUFFIX_TYPEDEFS(_name_) END_BENTLEY_ROADRAILALIGNMENT_NAMESPACE

//-----------------------------------------------------------------------------------------
// Define RefCountedPtr and CPtr types
//-----------------------------------------------------------------------------------------
#define ROADRAILALIGNMENT_REFCOUNTED_PTR(_name_) \
    BEGIN_BENTLEY_ROADRAILALIGNMENT_NAMESPACE struct _name_; DEFINE_REF_COUNTED_PTR(_name_) END_BENTLEY_ROADRAILALIGNMENT_NAMESPACE 


//-----------------------------------------------------------------------------------------
// Define typedefs and Ptrs in the RoadRailAlignment namespace
//-----------------------------------------------------------------------------------------
ROADRAILALIGNMENT_TYPEDEFS(Alignment)
ROADRAILALIGNMENT_TYPEDEFS(AlignmentCategoryModel)
ROADRAILALIGNMENT_TYPEDEFS(AlignmentIntersectionInfo)
ROADRAILALIGNMENT_TYPEDEFS(AlignmentModel)
ROADRAILALIGNMENT_TYPEDEFS(AlignmentModelHandler)
ROADRAILALIGNMENT_TYPEDEFS(AlignmentPair)
ROADRAILALIGNMENT_TYPEDEFS(AlignmentPairEditor)
ROADRAILALIGNMENT_TYPEDEFS(AlignmentProfileViewDefinition)
ROADRAILALIGNMENT_TYPEDEFS(AlignmentReferentElement)
ROADRAILALIGNMENT_TYPEDEFS(AlignmentStation)
ROADRAILALIGNMENT_TYPEDEFS(AlignmentXSViewDefinition)
ROADRAILALIGNMENT_TYPEDEFS(HorizontalAlignment)
ROADRAILALIGNMENT_TYPEDEFS(HorizontalAlignmentsPortion)
ROADRAILALIGNMENT_TYPEDEFS(HorizontalAlignmentModel)
ROADRAILALIGNMENT_TYPEDEFS(VerticalAlignment)
ROADRAILALIGNMENT_TYPEDEFS(VerticalAlignmentModel)
ROADRAILALIGNMENT_TYPEDEFS(DividedRoadAlignmentPairEditor)
ROADRAILALIGNMENT_TYPEDEFS(StationRange)
ROADRAILALIGNMENT_TYPEDEFS(StationRangeEdit)

ROADRAILALIGNMENT_REFCOUNTED_PTR(Alignment)
ROADRAILALIGNMENT_REFCOUNTED_PTR(AlignmentCategoryModel)
ROADRAILALIGNMENT_REFCOUNTED_PTR(AlignmentIntersection)
ROADRAILALIGNMENT_REFCOUNTED_PTR(AlignmentModel)
ROADRAILALIGNMENT_REFCOUNTED_PTR(AlignmentPair)
ROADRAILALIGNMENT_REFCOUNTED_PTR(AlignmentPairEditor)
ROADRAILALIGNMENT_REFCOUNTED_PTR(AlignmentPairIntersection)
ROADRAILALIGNMENT_REFCOUNTED_PTR(AlignmentProfileViewDefinition)
ROADRAILALIGNMENT_REFCOUNTED_PTR(AlignmentStation)
ROADRAILALIGNMENT_REFCOUNTED_PTR(AlignmentStationingTranslator)
ROADRAILALIGNMENT_REFCOUNTED_PTR(AlignmentXSViewDefinition)
ROADRAILALIGNMENT_REFCOUNTED_PTR(HorizontalAlignment)
ROADRAILALIGNMENT_REFCOUNTED_PTR(HorizontalAlignmentsPortion)
ROADRAILALIGNMENT_REFCOUNTED_PTR(HorizontalAlignmentModel)
ROADRAILALIGNMENT_REFCOUNTED_PTR(VerticalAlignment)
ROADRAILALIGNMENT_REFCOUNTED_PTR(VerticalAlignmentModel)
ROADRAILALIGNMENT_REFCOUNTED_PTR(DividedRoadAlignmentPairEditor)
ROADRAILALIGNMENT_REFCOUNTED_PTR(RoadAlignmentPairEditor)



BEGIN_BENTLEY_ROADRAILALIGNMENT_NAMESPACE

enum class StationRangeOverlap
    {
    NoOverlap = 0,    //! self-explanatory
    LeftOverlap = 1,    //! rhs starts before and ends inside
    RightOverlap = 2,    //! rhs starts inside and ends after
    FullyEncapsulated = 3,    //! rhs starts before and ends after
    FullyEnvelops = 4     //! rhs starts and ends inside
    };

//=======================================================================================
//! A StationRange is a context object carrying the start and end stations
//! of a section of a RoadRange alignment
//=======================================================================================
struct StationRange
{
    double startStation;
    double endStation;

    StationRange() :startStation(NAN), endStation(NAN) {}
    StationRange(double start, double end) :startStation(start), endStation(end) {}

    bool operator==(StationRangeCR rhs) const { return (startStation == rhs.startStation) && (endStation == rhs.endStation); }

    double Distance() const { return fabs(endStation - startStation); }
    bool IsValid() const { return (!isnan(startStation) && !isnan(endStation) && startStation <= endStation); }

    bool ContainsInclusive(double val) const { return (val >= startStation && val <= endStation); }
    bool ContainsExclusive(double val) const { return (val > startStation && val < endStation); }

    void Extend(StationRangeCR rhs)
        {
        if (!rhs.IsValid())
            return;

        startStation = MIN(startStation, rhs.startStation);
        endStation = MAX(endStation, rhs.endStation);
        }

    //! Calculates the distance to the closest station of the range
    //! @remarks returns 0.0 if the value is inside the range
    double DistanceFromRange(double val) const
        {
        if (!IsValid())
            return -1.0;

        if (ContainsInclusive(val))
            return 0.0;

        if (startStation > val)
            return fabs(startStation - val);

        if (endStation < val)
            return fabs(endStation - val);

        BeAssert(0);
        return 0.0;
        }


    // Return the enumeration as this compares to the stationRange argument 'rhs'
    StationRangeOverlap Overlaps(StationRangeCR stationRange) const { return Overlaps(*this, stationRange); }
    static StationRangeOverlap Overlaps(StationRangeCR stationRange1, StationRangeCR stationRange2)
        {
        // start is in
        bool startin = false; bool endin = false;
        if (stationRange2.startStation >= stationRange1.startStation && stationRange2.startStation <= stationRange1.endStation)
            startin = true;
        if (stationRange2.endStation >= stationRange1.startStation && stationRange2.endStation <= stationRange1.endStation)
            endin = true;
        if (startin && endin)
            return StationRangeOverlap::FullyEnvelops;
        if (startin)
            return StationRangeOverlap::RightOverlap;
        if (endin)
            return StationRangeOverlap::LeftOverlap;

        startin = false; endin = false;
        if (stationRange1.startStation >= stationRange2.startStation && stationRange1.startStation <= stationRange2.endStation)
            startin = true;
        if (stationRange1.endStation >= stationRange2.startStation && stationRange1.endStation <= stationRange2.endStation)
            endin = true;
        if (startin && endin)
            return StationRangeOverlap::FullyEncapsulated;
        if (startin)
            return StationRangeOverlap::LeftOverlap;
        if (endin)
            return StationRangeOverlap::RightOverlap;

        return StationRangeOverlap::NoOverlap;
        }
}; // StationRange

//=======================================================================================
//! A StationRangeEdit is a context object carrying information about a 
//! station change in a section of a RoadRange alignment
//=======================================================================================
struct StationRangeEdit
{
    StationRange preEditRange;
    StationRange postEditRange;

    StationRangeEdit() {}
    StationRangeEdit(StationRangeCR stationRange) : preEditRange(stationRange), postEditRange(stationRange) {}
    StationRangeEdit(StationRangeCR preEdit, StationRangeCR postEdit) : preEditRange(preEdit), postEditRange(postEdit) {}

    double Delta() const { return postEditRange.Distance() - preEditRange.Distance(); }
    double Ratio() const
        {
        if (preEditRange.Distance() == 0.0) return 1.0;
        return postEditRange.Distance() / preEditRange.Distance();
        }
}; // StationRangeEdit

END_BENTLEY_ROADRAILALIGNMENT_NAMESPACE