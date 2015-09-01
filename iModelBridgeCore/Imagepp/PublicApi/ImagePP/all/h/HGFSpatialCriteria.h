//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HGFSpatialCriteria.h $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Class : HGFSpatialCriteria
//-----------------------------------------------------------------------------
// Selection criteria
//-----------------------------------------------------------------------------

#pragma once


#include "HIDXCriteria.h"
#include "HGF2DExtent.h"

BEGIN_IMAGEPP_NAMESPACE
/** -----------------------------------------------------------------------------
    @version 1.0
    @author Martin Roy


    This class is a search criteria based on an HGF2DExtent object. It is
    used to query objects inside indexes that work with spatial information.

    HIDXCriteria objects are normally added to an HIDXCearchCriteria
    object that is given as a parameter to Query methods.

    -----------------------------------------------------------------------------
*/
class HGFSpatialCriteria : public HIDXCriteria
    {
    HDECLARE_BASECLASS_ID(HGFSpatialCriteriaId_Base)

public:

    /** -----------------------------------------------------------------------------
        Constructor and destructor. The copy constructor and assignment
        operator are disabled.

        @param pi_rRegion The extent object that will be used when querying objects.

        Example:
        @code
        @end

        @see GetRegion()
        @see HIDX library
        -----------------------------------------------------------------------------
    */
    HGFSpatialCriteria(const HGF2DExtent& pi_rRegion);


    virtual         ~HGFSpatialCriteria();

    HGF2DExtent&    GetRegion();

private:

    // Copy ctor and assignment are disabled
    HGFSpatialCriteria(const HGFSpatialCriteria& pi_rObj);
    HGFSpatialCriteria& operator=(const HGFSpatialCriteria& pi_rObj);

    // The region to query
    HGF2DExtent     m_Region;
    };

END_IMAGEPP_NAMESPACE

#include "HGFSpatialCriteria.hpp"


