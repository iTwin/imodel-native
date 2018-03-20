/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/RoadRailAlignment/AlignmentReferent.h $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

//__PUBLISH_SECTION_START__
#include "RoadRailAlignment.h"
#include "Alignment.h"

BEGIN_BENTLEY_ROADRAILALIGNMENT_NAMESPACE

//=======================================================================================
//! Known location along an Alignment.
//! @ingroup GROUP_RoadRailAlignment
//=======================================================================================
struct LinearlyLocatedReferentElement : Dgn::SpatialLocationElement, LinearReferencing::ILinearlyLocatedElement, LinearReferencing::IReferent, LinearReferencing::ILinearlyLocatedSingleAt
{
    DGNELEMENT_DECLARE_MEMBERS(BRRA_CLASS_LinearlyLocatedReferentElement, Dgn::SpatialLocationElement);
    friend struct LinearlyLocatedReferentElementHandler;

protected:
    //! @private
    explicit LinearlyLocatedReferentElement(CreateParams const& params) : T_Super(params) {}

    //! @private
    explicit LinearlyLocatedReferentElement(CreateParams const& params, double distanceAlong);

    virtual LinearReferencing::ILinearlyLocatedElementCP _ToLinearlyLocatedElement() const { return this; }
    virtual Dgn::DgnElementCR _IReferentToDgnElement() const override { return *this; }
    virtual Dgn::DgnElementCR _ILinearlyLocatedToDgnElement() const override { return *this; }

    virtual LinearReferencing::NullableDouble _GetRestartValue() const override { return LinearReferencing::NullableDouble(); }

public:
    DECLARE_ROADRAILALIGNMENT_QUERYCLASS_METHODS(LinearlyLocatedReferentElement)

    ROADRAILALIGNMENT_EXPORT LinearReferencing::ILinearlyLocatedElementCP ToLinearlyLocatedElement() const { return _ToLinearlyLocatedElement(); }
}; // LinearlyLocatedReferentElement

//=======================================================================================
//! Well-known station along an alignment.
//! @ingroup GROUP_RoadRailAlignment
//=======================================================================================
struct AlignmentStation : LinearlyLocatedReferentElement
{
    DGNELEMENT_DECLARE_MEMBERS(BRRA_CLASS_AlignmentStation, LinearlyLocatedReferentElement);
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

    ROADRAILALIGNMENT_EXPORT static AlignmentStationPtr Create(AlignmentCR alignment, double distanceAlongFromStart, double station = 0.0);
}; // AlignmentStation

//=======================================================================================
//! Well-known station along an alignment.
//! @ingroup GROUP_RoadRailAlignment
//=======================================================================================
struct AlignmentStationingTranslator : RefCountedBase
{
private:
    bvector<Alignment::DistanceAlongStationPair> m_stations;
    double m_length;

    AlignmentStationingTranslator(AlignmentCR alignment);

public:
    ROADRAILALIGNMENT_EXPORT static AlignmentStationingTranslatorPtr Create(AlignmentCR alignment);

    ROADRAILALIGNMENT_EXPORT LinearReferencing::NullableDouble ToStation(double distanceAlongFromStart) const;
    ROADRAILALIGNMENT_EXPORT LinearReferencing::NullableDouble ToDistanceAlongFromStart(double station) const;
}; // AlignmentStationingTranslator

//=================================================================================
//! ElementHandler for AlignmentReferent Elements
//! @ingroup GROUP_RoadRailAlignment
//=================================================================================
struct EXPORT_VTABLE_ATTRIBUTE LinearlyLocatedReferentElementHandler : Dgn::dgn_ElementHandler::SpatialLocation
{
ELEMENTHANDLER_DECLARE_MEMBERS(BRRA_CLASS_LinearlyLocatedReferentElement, LinearlyLocatedReferentElement, LinearlyLocatedReferentElementHandler, Dgn::dgn_ElementHandler::SpatialLocation, ROADRAILALIGNMENT_EXPORT)
}; //LinearlyLocatedReferentElementHandler

//=================================================================================
//! ElementHandler for AlignmentStation Elements
//! @ingroup GROUP_RoadRailAlignment
//=================================================================================
struct EXPORT_VTABLE_ATTRIBUTE AlignmentStationHandler : LinearlyLocatedReferentElementHandler
{
ELEMENTHANDLER_DECLARE_MEMBERS(BRRA_CLASS_AlignmentStation, AlignmentStation, AlignmentStationHandler, LinearlyLocatedReferentElementHandler, ROADRAILALIGNMENT_EXPORT)
}; //AlignmentStationHandler

END_BENTLEY_ROADRAILALIGNMENT_NAMESPACE