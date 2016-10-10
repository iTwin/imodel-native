#pragma once
/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/RoadRailPhysical/RoadDesignSpeed.h $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#pragma once

//__PUBLISH_SECTION_START__
#include "RoadRailPhysicalApi.h"

BEGIN_BENTLEY_ROADRAILPHYSICAL_NAMESPACE

//=======================================================================================
//! Linearly-located attribution on a RoadRange whose value is its design-speed.
//! @ingroup GROUP_RoadRailPhysical
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE RoadDesignSpeed : Dgn::InformationContentElement, LinearReferencing::ILinearlyLocatedAttribution, ILinearlyLocatedSingleFromTo
{
    DGNELEMENT_DECLARE_MEMBERS(BRRP_CLASS_RoadDesignSpeed, Dgn::InformationContentElement);
    friend struct RoadDesignSpeedHandler;

protected:
    //! @private
    explicit RoadDesignSpeed(CreateParams const& params);

    //! @private
    explicit RoadDesignSpeed(CreateParams const& params, double fromDistanceAlong, double toDistanceAlong);

    virtual Dgn::DgnElementCR _ILinearlyLocatedToDgnElement() const override { return *this; }

public:
    DECLARE_ROADRAILPHYSICAL_QUERYCLASS_METHODS(RoadDesignSpeed)
    DECLARE_ROADRAILPHYSICAL_ELEMENT_BASE_METHODS(RoadDesignSpeed)

    ROADRAILPHYSICAL_EXPORT static RoadDesignSpeedPtr Create(RoadRangeCR roadRange, double fromDistanceAlong, double toDistanceAlong);
}; // RoadDesignSpeed


//=================================================================================
//! ElementHandler for RoadDesignSpeed Elements
//! @ingroup GROUP_RoadRailPhysical
//=================================================================================
struct EXPORT_VTABLE_ATTRIBUTE RoadDesignSpeedHandler : Dgn::dgn_ElementHandler::InformationContent
{
ELEMENTHANDLER_DECLARE_MEMBERS(BRRP_CLASS_RoadDesignSpeed, RoadDesignSpeed, RoadDesignSpeedHandler, Dgn::dgn_ElementHandler::InformationContent, ROADRAILPHYSICAL_EXPORT)
}; // RoadDesignSpeedHandler

END_BENTLEY_ROADRAILPHYSICAL_NAMESPACE