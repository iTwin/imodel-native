/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/RoadRailPhysical/RoadRailPhysicalDomain.h $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

#include "RoadRailPhysical.h"

//__PUBLISH_SECTION_START__
BEGIN_BENTLEY_ROADRAILPHYSICAL_NAMESPACE

//=======================================================================================
//! The DgnDomain for the RoadRailPhysical schema.
//! @ingroup GROUP_RoadRailPhysical
//=======================================================================================
struct RoadRailPhysicalDomain : Dgn::DgnDomain
{
DOMAIN_DECLARE_MEMBERS(RoadRailPhysicalDomain, ROADRAILPHYSICAL_EXPORT)

protected:
    //! @private
    void _OnSchemaImported(Dgn::DgnDbR dgndb) const override;

public:
    //! @private
    RoadRailPhysicalDomain();

    //! @private
    ROADRAILPHYSICAL_EXPORT static Dgn::DgnDbStatus SetUpModelHierarchy(Dgn::SubjectCR);

    //! @private
    ROADRAILPHYSICAL_EXPORT static Dgn::DgnViewId SetUpDefaultViews(Dgn::SubjectCR, bvector<Dgn::DgnCategoryId> const* additionalCategoriesForSelector = nullptr);

    //! Query for the physical model
    //! @param[in] parentSubject The parent subject of the physical model with \p modelName
    //! @return The PhysicalModel belonging to the \p parentSubject
    ROADRAILPHYSICAL_EXPORT static Dgn::PhysicalModelPtr QueryPhysicalModel(Dgn::SubjectCR parentSubject);

    //! Queryt for the Parent Subject
    //! @param[in] model The PhysicalModel who's parent is being queried for.
    //! @return The Subject of the \p model
    ROADRAILPHYSICAL_EXPORT static Dgn::SubjectCPtr GetParentSubject(Dgn::PhysicalModelCR model);
    //! The name of the default Physical partition
    static Utf8CP GetDefaultPhysicalPartitionName() { return "Roads/Rail Physical"; }

    //! The name of the default Standards Partition
    static Utf8CP GetDefaultStandardsPartitionName() { return "Roadway Standards"; }

    //! @private
    ROADRAILPHYSICAL_EXPORT static Dgn::DgnDbStatus SetGeometricElementAsBoundingContentForSheet(Dgn::GeometricElementCR boundingElm, Dgn::Sheet::ElementCR sheet);
    
    //! Get the DgnElementIdSet containing the DgnElementIds of all of the NamedBoundaries that are the bounding elements for drawings in SheetModels
    //! @param[in] dgnDb The DgnDb to search for bounding elements
    //! @return DgnElementSet containing boundary DgnElementIds
    ROADRAILPHYSICAL_EXPORT static Dgn::DgnElementIdSet QueryElementIdsBoundingContentForSheets(Dgn::DgnDbCR dgnDb);

    //! Get the DgnElementIdSet of any SheetModles that are bounded by the \p boundingElm.  
    //! @param[in] A geometric element that bounds a SheetModel.  This can be obtained by getting the GeometricElement from the DgnElementIds returned by QueryElementIdsBoundingContentForSheets()
    //! @return ElementIdSet containing all of the sheets that are physically located within the area defined by \p boundingElm
    ROADRAILPHYSICAL_EXPORT static Dgn::DgnElementIdSet QuerySheetIdsBoundedBy(Dgn::GeometricElementCR boundingElm);

private:
    WCharCP _GetSchemaRelativePath() const override { return BRRP_SCHEMA_PATH; }
}; // RoadRailPhysicalDomain

END_BENTLEY_ROADRAILPHYSICAL_NAMESPACE
