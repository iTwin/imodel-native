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
    //! Return the DgnCategoryId for the Alignment Category
    ROADRAILALIGNMENT_EXPORT static Dgn::DgnCategoryId GetAlignment(Dgn::SubjectCR);
    //! Return the DgnCategoryId for the Horizontal Alignment Category
    ROADRAILALIGNMENT_EXPORT static Dgn::DgnCategoryId GetHorizontal(Dgn::SubjectCR);
    //! Return the DgnCategoryId for the Vertical Alignment Category
    ROADRAILALIGNMENT_EXPORT static Dgn::DgnCategoryId GetVertical(Dgn::SubjectCR);

}; // BridgePhysicalDomain

    //! @private
    //! Create a new AlignmentCategoryModel
    //! @private
    
    //! @private

    //! Get the AlignmentCategoryModel from the DgnDb
    //! @param[in] The DgnDb to get the AlignmentCategoryModel from 
    //! @return The AlignmentCategoryModel from the DgnDb

    //! @private
//! @private
END_BENTLEY_ROADRAILALIGNMENT_NAMESPACE
