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
struct AlignmentReferent : Dgn::SpatialLocationElement, LinearReferencing::IReferent, LinearReferencing::ILinearlyLocatedElement
{
    DGNELEMENT_DECLARE_MEMBERS(BRRA_CLASS_AlignmentReferent, Dgn::SpatialLocationElement);
    friend struct AlignmentReferentHandler;

private:
    double m_restartValue;

protected:
    //! @private
    explicit AlignmentReferent(CreateParams const& params) : T_Super(params), LinearReferencing::ILinearlyLocatedElement(Dgn::DgnElementId()) {}

    //! @private
    explicit AlignmentReferent(CreateParams const& params, AlignmentCR alignment, LinearReferencing::DistanceExpressionCR distanceExpression, double restartValue);

    virtual double _GetRestartValue() const override { return m_restartValue; }
    virtual Dgn::DgnElementCR _ToElementLRImpl() const override { return *this; }

public:
    DECLARE_ROADRAILALIGNMENT_QUERYCLASS_METHODS(AlignmentReferent)
    DECLARE_ROADRAILALIGNMENT_ELEMENT_BASE_METHODS(AlignmentReferent)
    ROADRAILALIGNMENT_EXPORT static AlignmentReferentPtr Create(AlignmentCR alignment, LinearReferencing::DistanceExpressionCR distanceExpression, double restartValue = 0.0);

}; // AlignmentReferent

//=================================================================================
//! ElementHandler for AlignmentReferent Elements
//! @ingroup GROUP_RoadRailAlignment
//=================================================================================
struct EXPORT_VTABLE_ATTRIBUTE AlignmentReferentHandler : Dgn::dgn_ElementHandler::SpatialLocation
{
ELEMENTHANDLER_DECLARE_MEMBERS(BRRA_CLASS_AlignmentReferent, AlignmentReferent, AlignmentReferentHandler, Dgn::dgn_ElementHandler::SpatialLocation, ROADRAILALIGNMENT_EXPORT)
}; //AlignmentReferentHandler

END_BENTLEY_ROADRAILALIGNMENT_NAMESPACE