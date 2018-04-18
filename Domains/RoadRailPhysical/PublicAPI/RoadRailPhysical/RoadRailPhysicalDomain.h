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
    void _OnSchemaImported(Dgn::DgnDbR dgndb) const override;

public:
    RoadRailPhysicalDomain();

    ROADRAILPHYSICAL_EXPORT static Dgn::DgnDbStatus SetUpModelHierarchy(Dgn::SubjectCR);
    ROADRAILPHYSICAL_EXPORT static Dgn::DgnViewId SetUpDefaultViews(Dgn::SubjectCR, bvector<Dgn::DgnCategoryId> const* additionalCategoriesForSelector = nullptr);
    ROADRAILPHYSICAL_EXPORT static Dgn::PhysicalModelPtr QueryPhysicalModel(Dgn::SubjectCR parentSubject);
    ROADRAILPHYSICAL_EXPORT static Dgn::SubjectCPtr GetParentSubject(Dgn::PhysicalModelCR model);
    static Utf8CP GetDefaultPhysicalPartitionName() { return "Road/Rail Physical"; }
    static Utf8CP GetRailwayStandardsPartitionName() { return "Railway Standards"; }
    static Utf8CP GetRoadwayStandardsPartitionName() { return "Roadway Standards"; }    

    ROADRAILPHYSICAL_EXPORT static Dgn::DgnDbStatus SetGeometricElementAsBoundingContentForSheet(Dgn::GeometricElementCR boundingElm, Dgn::Sheet::ElementCR sheet);
    ROADRAILPHYSICAL_EXPORT static Dgn::DgnElementIdSet QueryElementIdsBoundingContentForSheets(Dgn::DgnDbCR dgnDb);
    ROADRAILPHYSICAL_EXPORT static Dgn::DgnElementIdSet QuerySheetIdsBoundedBy(Dgn::GeometricElementCR boundingElm);

private:
    WCharCP _GetSchemaRelativePath() const override { return BRRP_SCHEMA_PATH; }
}; // RoadRailPhysicalDomain

END_BENTLEY_ROADRAILPHYSICAL_NAMESPACE
