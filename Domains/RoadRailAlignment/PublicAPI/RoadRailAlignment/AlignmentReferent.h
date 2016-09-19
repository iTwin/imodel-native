/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/RoadRailAlignment/AlignmentReferent.h $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#pragma once

//__PUBLISH_SECTION_START__
#include "RoadRailAlignmentApi.h"

BEGIN_BENTLEY_ROADRAILALIGNMENT_NAMESPACE

//=======================================================================================
//! Known location along an Alignment.
//! @ingroup GROUP_RoadRailAlignment
//=======================================================================================
struct AlignmentReferentElement : Dgn::SpatialLocationElement, LinearReferencing::IReferent, LinearReferencing::ILinearlyLocatedElement
{
    DGNELEMENT_DECLARE_MEMBERS(BRRA_CLASS_AlignmentReferentElement, Dgn::SpatialLocationElement);
    friend struct AlignmentReferentElementHandler;

private:
    double m_restartValue;

protected:
    //! @private
    explicit AlignmentReferentElement(CreateParams const& params) : T_Super(params), LinearReferencing::ILinearlyLocatedElement() {}

    //! @private
    explicit AlignmentReferentElement(CreateParams const& params, LinearReferencing::DistanceExpressionCR distanceExpression, double restartValue);

    virtual double _GetRestartValue() const override { return m_restartValue; }
    virtual Dgn::DgnElementCR _ILinearlyLocatedToDgnElement() const override final { return *this; }
    virtual Dgn::DgnElementCR _IReferentToDgnElement() const override final { return *this; }

public:
    DECLARE_ROADRAILALIGNMENT_QUERYCLASS_METHODS(AlignmentReferentElement)    

}; // AlignmentReferentElement

//=======================================================================================
//! Well-known station along an alignment.
//! @ingroup GROUP_RoadRailAlignment
//=======================================================================================
struct AlignmentStation : AlignmentReferentElement
{
    DGNELEMENT_DECLARE_MEMBERS(BRRA_CLASS_AlignmentStation, AlignmentReferentElement);
    friend struct AlignmentStationHandler;

protected:
    //! @private
    explicit AlignmentStation(CreateParams const& params) : T_Super(params) {}

    //! @private
    explicit AlignmentStation(CreateParams const& params, LinearReferencing::DistanceExpressionCR distanceExpression, double restartValue) : 
        T_Super(params, distanceExpression, restartValue) {}

public:
    DECLARE_ROADRAILALIGNMENT_QUERYCLASS_METHODS(AlignmentStation)
    DECLARE_ROADRAILALIGNMENT_ELEMENT_BASE_METHODS(AlignmentStation)

    ROADRAILALIGNMENT_EXPORT static AlignmentStationPtr Create(AlignmentCR alignment, LinearReferencing::DistanceExpressionCR distanceExpression, double restartValue = 0.0);
}; // AlignmentStation

//=================================================================================
//! ElementHandler for AlignmentReferent Elements
//! @ingroup GROUP_RoadRailAlignment
//=================================================================================
struct EXPORT_VTABLE_ATTRIBUTE AlignmentReferentElementHandler : Dgn::dgn_ElementHandler::SpatialLocation
{
ELEMENTHANDLER_DECLARE_MEMBERS(BRRA_CLASS_AlignmentReferentElement, AlignmentReferentElement, AlignmentReferentElementHandler, Dgn::dgn_ElementHandler::SpatialLocation, ROADRAILALIGNMENT_EXPORT)
}; //AlignmentReferentElementHandler

//=================================================================================
//! ElementHandler for AlignmentStation Elements
//! @ingroup GROUP_RoadRailAlignment
//=================================================================================
struct EXPORT_VTABLE_ATTRIBUTE AlignmentStationHandler : AlignmentReferentElementHandler
{
ELEMENTHANDLER_DECLARE_MEMBERS(BRRA_CLASS_AlignmentStation, AlignmentStation, AlignmentStationHandler, AlignmentReferentElementHandler, ROADRAILALIGNMENT_EXPORT)
}; //AlignmentStationHandler

END_BENTLEY_ROADRAILALIGNMENT_NAMESPACE