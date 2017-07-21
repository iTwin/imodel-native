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
struct AlignmentReferentElement : Dgn::SpatialLocationElement, LinearReferencing::ILinearlyLocatedElement, LinearReferencing::IReferent, LinearReferencing::ILinearlyLocatedSingleAt
{
    DGNELEMENT_DECLARE_MEMBERS(BRRA_CLASS_AlignmentReferentElement, Dgn::SpatialLocationElement);
    friend struct AlignmentReferentElementHandler;

protected:
    //! @private
    explicit AlignmentReferentElement(CreateParams const& params) : T_Super(params) {}

    //! @private
    explicit AlignmentReferentElement(CreateParams const& params, double distanceAlong);

    virtual LinearReferencing::ILinearlyLocatedElementCP _ToLinearlyLocatedElement() const { return this; }
    virtual Dgn::DgnElementCR _IReferentToDgnElement() const override { return *this; }
    virtual Dgn::DgnElementCR _ILinearlyLocatedToDgnElement() const override { return *this; }

    virtual LinearReferencing::NullableDouble _GetRestartValue() const override { return LinearReferencing::NullableDouble(); }

public:
    DECLARE_ROADRAILALIGNMENT_QUERYCLASS_METHODS(AlignmentReferentElement)

    ROADRAILALIGNMENT_EXPORT LinearReferencing::ILinearlyLocatedElementCP ToLinearlyLocatedElement() const { return _ToLinearlyLocatedElement(); }
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
    explicit AlignmentStation(CreateParams const& params);

    //! @private
    explicit AlignmentStation(CreateParams const& params, double distanceAlong, double station);

    virtual LinearReferencing::NullableDouble _GetRestartValue() const override { return GetPropertyValueDouble("Station"); }

public:
    DECLARE_ROADRAILALIGNMENT_QUERYCLASS_METHODS(AlignmentStation)
    DECLARE_ROADRAILALIGNMENT_ELEMENT_BASE_METHODS(AlignmentStation)

    double GetStation() const { return GetRestartValue().Value(); }
    void SetStation(double newStation) { SetPropertyValue("Station", newStation); }

    ROADRAILALIGNMENT_EXPORT static AlignmentStationPtr Create(AlignmentCR alignment, double distanceAlongFromStart, double restartValue = 0.0);
}; // AlignmentStation

//=======================================================================================
//! Well-known station along an alignment.
//! @ingroup GROUP_RoadRailAlignment
//=======================================================================================
struct AlignmentStationingTranslator : RefCountedBase
{
private:
    bvector<Alignment::DistanceAlongStationPair> m_stations;

    AlignmentStationingTranslator(AlignmentCR alignment);

public:
    ROADRAILALIGNMENT_EXPORT static AlignmentStationingTranslatorPtr Create(AlignmentCR alignment);

    ROADRAILALIGNMENT_EXPORT LinearReferencing::NullableDouble ToStation(double distanceAlongFromStart) const;
}; // AlignmentStationingTranslator

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