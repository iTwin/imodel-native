/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/RoadRailAlignment/AlignmentReferent.h $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
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
struct AlignmentReferentElement : LinearReferencing::LinearlyLocatedReferent
{
    DGNELEMENT_DECLARE_MEMBERS(BRRA_CLASS_AlignmentReferentElement, LinearReferencing::LinearlyLocatedReferent);
    friend struct AlignmentReferentElementHandler;

private:
    double m_restartValue;

protected:
    //! @private
    explicit AlignmentReferentElement(CreateParams const& params) : T_Super(params) {}

    //! @private
    explicit AlignmentReferentElement(CreateParams const& params, double restartValue) :
        T_Super(params), m_restartValue(restartValue) {}

    virtual double _GetRestartValue() const override { return m_restartValue; }

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

private:
    LinearReferencing::LinearlyReferencedLocationId m_atLocationAspectId;
    LinearReferencing::LinearlyReferencedAtLocationPtr m_unpersistedAtLocation;

protected:
    //! @private
    explicit AlignmentStation(CreateParams const& params);

    //! @private
    explicit AlignmentStation(CreateParams const& params, LinearReferencing::DistanceExpressionCR distanceExpression, double restartValue);

public:
    DECLARE_ROADRAILALIGNMENT_QUERYCLASS_METHODS(AlignmentStation)
    DECLARE_ROADRAILALIGNMENT_ELEMENT_BASE_METHODS(AlignmentStation)

    ROADRAILALIGNMENT_EXPORT LinearReferencing::DistanceExpressionCR GetAtPosition() const;
    ROADRAILALIGNMENT_EXPORT LinearReferencing::DistanceExpressionP GetAtPositionP();

    ROADRAILALIGNMENT_EXPORT static AlignmentStationPtr Create(AlignmentCR alignment, LinearReferencing::DistanceExpressionCR atPosition, double restartValue = 0.0);
}; // AlignmentStation

//=================================================================================
//! ElementHandler for AlignmentReferent Elements
//! @ingroup GROUP_RoadRailAlignment
//=================================================================================
struct EXPORT_VTABLE_ATTRIBUTE AlignmentReferentElementHandler : LinearReferencing::LinearlyLocatedReferentHandler
{
ELEMENTHANDLER_DECLARE_MEMBERS(BRRA_CLASS_AlignmentReferentElement, AlignmentReferentElement, AlignmentReferentElementHandler, LinearReferencing::LinearlyLocatedReferentHandler, ROADRAILALIGNMENT_EXPORT)
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