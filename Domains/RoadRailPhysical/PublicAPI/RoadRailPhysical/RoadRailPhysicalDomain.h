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

    ROADRAILPHYSICAL_EXPORT static Dgn::DgnDbStatus SetUpModelHierarchy(Dgn::SubjectCR, Utf8CP physicalPartitionName);
    ROADRAILPHYSICAL_EXPORT static Dgn::DgnViewId SetUpDefaultViews(Dgn::SubjectCR, Utf8CP alignmentPartitionName = nullptr, Utf8CP physicalPartitionName = nullptr,
        bvector<Dgn::DgnCategoryId> const* additionalCategoriesForSelector = nullptr);
    ROADRAILPHYSICAL_EXPORT static Dgn::PhysicalModelPtr QueryPhysicalModel(Dgn::SubjectCR parentSubject, Utf8CP modelName);
    static Utf8CP GetDefaultPhysicalPartitionName() { return "Roads/Rail Physical"; }
    static Utf8CP GetDefaultStandardsPartitionName() { return "Roadway Standards"; }

    ROADRAILPHYSICAL_EXPORT static Dgn::DgnDbStatus SetGeometricElementAsBoundingContentForSheet(Dgn::GeometricElementCR boundingElm, Dgn::Sheet::ElementCR sheet);
    ROADRAILPHYSICAL_EXPORT static Dgn::DgnElementIdSet QueryElementIdsBoundingContentForSheets(Dgn::DgnDbCR dgnDb);
    ROADRAILPHYSICAL_EXPORT static Dgn::DgnElementIdSet QuerySheetIdsBoundedBy(Dgn::GeometricElementCR boundingElm);

private:
    WCharCP _GetSchemaRelativePath() const override { return BRRP_SCHEMA_PATH; }
}; // RoadRailPhysicalDomain

END_BENTLEY_ROADRAILPHYSICAL_NAMESPACE
