/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/RoadRailAlignment/AlignmentCategory.h $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
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
    static Dgn::DgnCategoryId QueryDomainCategoryId(Dgn::SubjectCR, Utf8CP, bool isSpatial);

public:
    static void InsertDomainCategories(ConfigurationModelR);

//__PUBLISH_SECTION_START__
public:
    //! Return the DgnCategoryId for alignment elements
    ROADRAILALIGNMENT_EXPORT static Dgn::DgnCategoryId GetAlignment(Dgn::SubjectCR);
    ROADRAILALIGNMENT_EXPORT static Dgn::DgnCategoryId GetHorizontal(Dgn::SubjectCR);
    ROADRAILALIGNMENT_EXPORT static Dgn::DgnCategoryId GetVertical(Dgn::SubjectCR);
}; // BridgePhysicalDomain

END_BENTLEY_ROADRAILALIGNMENT_NAMESPACE
