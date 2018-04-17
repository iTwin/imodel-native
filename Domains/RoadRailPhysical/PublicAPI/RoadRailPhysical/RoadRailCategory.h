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
    static Dgn::SpatialCategoryCPtr InsertSpatialCategory(RoadRailAlignment::ConfigurationModelR, Utf8CP, Dgn::ColorDef const&);
    static Dgn::DrawingCategoryCPtr InsertDrawingCategory(RoadRailAlignment::ConfigurationModelR, Utf8CP, Dgn::ColorDef const&);
    static void InsertSubCategory(Dgn::DgnCategoryCR category, Utf8CP codeValue, Dgn::ColorDef const& color);
    static Dgn::DgnCategoryId QuerySpatialCategoryId(Dgn::SubjectCR, Utf8CP);
    static Dgn::DgnCategoryId QueryDrawingCategoryId(Dgn::SubjectCR, Utf8CP);
    static Dgn::DgnSubCategoryId QuerySubCategoryId(Dgn::DgnDbR, Dgn::DgnCategoryId, Utf8CP);

public:
    static void InsertDomainCategories(RoadRailAlignment::ConfigurationModelR);

//__PUBLISH_SECTION_START__
public:
    ROADRAILPHYSICAL_EXPORT static Dgn::DgnCategoryId GetRoadway(Dgn::SubjectCR);
    ROADRAILPHYSICAL_EXPORT static Dgn::DgnCategoryId GetRailway(Dgn::SubjectCR);
}; // RoadRailCategory

END_BENTLEY_ROADRAILPHYSICAL_NAMESPACE
