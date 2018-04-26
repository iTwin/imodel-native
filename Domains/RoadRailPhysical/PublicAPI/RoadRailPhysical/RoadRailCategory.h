/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/RoadRailPhysical/RoadRailCategory.h $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

//__PUBLISH_SECTION_START__
#include "RoadRailPhysical.h"

BEGIN_BENTLEY_ROADRAILPHYSICAL_NAMESPACE

//=======================================================================================
//! Helper class to query/manage RoadRail-related categories.
//! @ingroup GROUP_RoadRailPhysical
//=======================================================================================
struct RoadRailCategory : NonCopyableClass
{
//__PUBLISH_SECTION_END__
private:
    static Dgn::SpatialCategoryCPtr InsertSpatialCategory(RoadRailAlignment::RoadRailCategoryModelR, Utf8CP, Dgn::ColorDef const&);
    static Dgn::DrawingCategoryCPtr InsertDrawingCategory(RoadRailAlignment::RoadRailCategoryModelR, Utf8CP, Dgn::ColorDef const&);
    static void InsertSubCategory(Dgn::DgnCategoryCR category, Utf8CP codeValue, Dgn::ColorDef const& color);
    static Dgn::DgnCategoryId QuerySpatialCategoryId(Dgn::DgnDbR, Utf8CP);
    static Dgn::DgnCategoryId QueryDrawingCategoryId(Dgn::DgnDbR, Utf8CP);
    static Dgn::DgnSubCategoryId QuerySubCategoryId(Dgn::DgnDbR, Dgn::DgnCategoryId, Utf8CP);

public:
    static void InsertDomainCategories(Dgn::DgnDbR);

//__PUBLISH_SECTION_START__
public:
    ROADRAILPHYSICAL_EXPORT static Dgn::DgnCategoryId GetCorridor(Dgn::DgnDbR);
    ROADRAILPHYSICAL_EXPORT static Dgn::DgnCategoryId GetRoadway(Dgn::DgnDbR);
    ROADRAILPHYSICAL_EXPORT static Dgn::DgnCategoryId GetRailway(Dgn::DgnDbR);
}; // RoadRailCategory

END_BENTLEY_ROADRAILPHYSICAL_NAMESPACE
