/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
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
    static Dgn::SpatialCategoryCPtr InsertSpatialCategory(Dgn::DefinitionModelR, Utf8CP, Dgn::ColorDef const&);
    static Dgn::DrawingCategoryCPtr InsertDrawingCategory(Dgn::DefinitionModelR, Utf8CP, Dgn::ColorDef const&);
    static void InsertSubCategory(Dgn::DgnCategoryCR category, Utf8CP codeValue, Dgn::ColorDef const& color);
    static Dgn::DgnCategoryId QuerySpatialCategoryId(Dgn::DgnDbR, Utf8CP);
    static Dgn::DgnCategoryId QueryDrawingCategoryId(Dgn::DgnDbR, Utf8CP);
    static Dgn::DgnSubCategoryId QuerySubCategoryId(Dgn::DgnDbR, Dgn::DgnCategoryId, Utf8CP);

public:
    static void InsertDomainCategories(Dgn::DgnDbR);

//__PUBLISH_SECTION_START__
public:
    //! Return the DgnCategoryId for the Network Category
    ROADRAILPHYSICAL_EXPORT static Dgn::DgnCategoryId GetNetwork(Dgn::DgnDbR);
    //! Return the DgnCategoryId for the Corridor Category
    ROADRAILPHYSICAL_EXPORT static Dgn::DgnCategoryId GetCorridor(Dgn::DgnDbR);
    //! Return the DgnCategoryId for the Roadway Category
    ROADRAILPHYSICAL_EXPORT static Dgn::DgnCategoryId GetRoadway(Dgn::DgnDbR);
    //! Return the DgnCategoryId for the Railway Category
    ROADRAILPHYSICAL_EXPORT static Dgn::DgnCategoryId GetRailway(Dgn::DgnDbR);
    //! Return the DgnCategoryId for the DesignSpeed Category
    ROADRAILPHYSICAL_EXPORT static Dgn::DgnCategoryId GetDesignSpeed(Dgn::DgnDbR);
}; // RoadRailCategory

END_BENTLEY_ROADRAILPHYSICAL_NAMESPACE
