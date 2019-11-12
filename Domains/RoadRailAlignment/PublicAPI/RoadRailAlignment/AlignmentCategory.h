/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once

#include "RoadRailAlignment.h"
#include "RoadRailAlignmentDomain.h"

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
    static Dgn::DgnCategoryId QueryDomainCategoryId(Dgn::DgnDbR, Utf8CP, bool isSpatial);

public:
    static void InsertDomainCategories(Dgn::DgnDbR db);

//__PUBLISH_SECTION_START__
public:
    //! Return the DgnCategoryId for the Alignment Category
    ROADRAILALIGNMENT_EXPORT static Dgn::DgnCategoryId GetAlignment(Dgn::DgnDbR);
    //! Return the DgnCategoryId for the Linear Category
    ROADRAILALIGNMENT_EXPORT static Dgn::DgnCategoryId GetLinear(Dgn::DgnDbR);
    //! Return the DgnCategoryId for the Vertical Alignment Category
    ROADRAILALIGNMENT_EXPORT static Dgn::DgnCategoryId GetVertical(Dgn::DgnDbR);
}; // AlignmentCategory

//__PUBLISH_SECTION_START__
END_BENTLEY_ROADRAILALIGNMENT_NAMESPACE
