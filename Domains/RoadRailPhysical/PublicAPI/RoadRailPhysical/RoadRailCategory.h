/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/RoadRailPhysical/RoadRailCategory.h $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#pragma once

//__PUBLISH_SECTION_START__
BEGIN_BENTLEY_ROADRAILPHYSICAL_NAMESPACE

//=======================================================================================
//! Helper class to query/manage RoadRail-related categories.
//! @ingroup GROUP_RoadRailPhysical
//=======================================================================================
struct RoadRailCategory : NonCopyableClass
{
//__PUBLISH_SECTION_END__
private:
    static void InsertCategory(Dgn::DgnDbR, Dgn::ColorDef const&, Utf8CP);
    static Dgn::DgnCategoryId QueryCategoryId(Dgn::DgnDbR, Utf8CP);

public:
    static void InsertDomainCategories(Dgn::DgnDbR);

//__PUBLISH_SECTION_START__
public:
    //! Return the DgnCategoryId for road-rail elements
    ROADRAILPHYSICAL_EXPORT static Dgn::DgnCategoryId GetRoad(Dgn::DgnDbR);
    ROADRAILPHYSICAL_EXPORT static Dgn::DgnCategoryId GetTrack(Dgn::DgnDbR);
}; // RoadRailCategory

END_BENTLEY_ROADRAILPHYSICAL_NAMESPACE
