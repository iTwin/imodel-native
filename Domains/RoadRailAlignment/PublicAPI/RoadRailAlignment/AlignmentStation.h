/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/RoadRailAlignment/AlignmentStation.h $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#pragma once

//__PUBLISH_SECTION_START__
#include "RoadRailAlignmentApi.h"

BEGIN_BENTLEY_ROADRAILALIGNMENT_NAMESPACE

//=======================================================================================
//! Horizontal piece of an Alignment.
//! @ingroup GROUP_RoadRailAlignment
//=======================================================================================
struct AlignmentStation : Dgn::SpatialLocationElement, LinearReferencing::IReferent, LinearReferencing::ILinearlyLocatedElement
{
    DGNELEMENT_DECLARE_MEMBERS(BRRA_CLASS_AlignmentStation, Dgn::SpatialLocationElement);
    friend struct AlignmentStationHandler;

private:
    double m_restartValue;

protected:
    //! @private
    explicit AlignmentStation(CreateParams const& params) : T_Super(params), LinearReferencing::ILinearlyLocatedElement(Dgn::DgnElementId()) {}

    //! @private
    explicit AlignmentStation(CreateParams const& params, AlignmentCR alignment, LinearReferencing::DistanceExpressionCR distanceExpression, double restartValue);

    virtual double _GetRestartValue() const override { return m_restartValue; }
    virtual Dgn::DgnElementCR _ToElementLRImpl() const override { return *this; }

public:
    DECLARE_ROADRAILALIGNMENT_QUERYCLASS_METHODS(AlignmentStation)
    DECLARE_ROADRAILALIGNMENT_ELEMENT_BASE_METHODS(AlignmentStation)
    ROADRAILALIGNMENT_EXPORT static AlignmentStationPtr Create(AlignmentCR alignment, LinearReferencing::DistanceExpressionCR distanceExpression, double restartValue = 0.0);

}; // AlignmentStation

//=================================================================================
//! ElementHandler for AlignmentStation Elements
//! @ingroup GROUP_RoadRailAlignment
//=================================================================================
struct EXPORT_VTABLE_ATTRIBUTE AlignmentStationHandler : Dgn::dgn_ElementHandler::SpatialLocation
{
ELEMENTHANDLER_DECLARE_MEMBERS(BRRA_CLASS_AlignmentStation, AlignmentStation, AlignmentStationHandler, Dgn::dgn_ElementHandler::SpatialLocation, ROADRAILALIGNMENT_EXPORT)
}; //AlignmentStationHandler

END_BENTLEY_ROADRAILALIGNMENT_NAMESPACE