/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/RoadRailAlignment/AlignmentCategory.h $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#pragma once

//__PUBLISH_SECTION_START__
BEGIN_BENTLEY_ROADRAILALIGNMENT_NAMESPACE

//=======================================================================================
//! Helper class to query/manage Bridge-related categories.
//! @ingroup GROUP_BridgePhysical
//=======================================================================================
struct AlignmentCategory : NonCopyableClass
{
//__PUBLISH_SECTION_END__
private:
    static void InsertCategory(Dgn::DgnDbR, Dgn::ColorDef const&, Utf8CP);
    static Dgn::DgnCategoryId QueryCategoryId(Dgn::DgnDbR, Utf8CP);

public:
    static void InsertDomainCategories(Dgn::DgnDbR);

//__PUBLISH_SECTION_START__
public:
    //! Return the DgnCategoryId for alignment elements
    ROADRAILALIGNMENT_EXPORT static Dgn::DgnCategoryId Get(Dgn::DgnDbR);
}; // BridgePhysicalDomain

END_BENTLEY_ROADRAILALIGNMENT_NAMESPACE
