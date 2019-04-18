/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
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
    explicit LinearlyLocatedReferentElement(CreateParams const& params, CreateAtParams const& atParams);

    //! @private
    virtual LinearReferencing::ILinearlyLocatedElementCP _ToLinearlyLocatedElement() const { return this; }

    //! @private
    virtual Dgn::DgnElementCR _IReferentToDgnElement() const override { return *this; }

    //! @private
    virtual Dgn::DgnElementCR _ILinearlyLocatedToDgnElement() const override { return *this; }

    //! @private
    virtual LinearReferencing::NullableDouble _GetRestartValue() const override { return LinearReferencing::NullableDouble(); }

public:
    DECLARE_ROADRAILALIGNMENT_QUERYCLASS_METHODS(LinearlyLocatedReferentElement)

    //! Convert this to a LinearlyLocatedElement.
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

public:
    struct CreateAtParams : LinearReferencing::ILinearlyLocatedSingleAt::CreateAtParams
    {
        DEFINE_T_SUPER(LinearReferencing::ILinearlyLocatedSingleAt::CreateAtParams)
        
        double m_station;

        CreateAtParams(AlignmentCR alignment, double distanceAlong, double station = 0.0) : 
            T_Super(alignment, LinearReferencing::DistanceExpression(distanceAlong)), m_station(station) {}
    }; // CreateAtParams

protected:
    //! @private
    explicit AlignmentStation(CreateParams const& params);

    //! @private
    explicit AlignmentStation(CreateParams const& params, CreateAtParams const& atParams);

    //! @private
    virtual LinearReferencing::NullableDouble _GetRestartValue() const override { return GetPropertyValueDouble("Station"); }

public:
    DECLARE_ROADRAILALIGNMENT_QUERYCLASS_METHODS(AlignmentStation)
    DECLARE_ROADRAILALIGNMENT_ELEMENT_BASE_METHODS(AlignmentStation)

    //! Returns the station, accounting for any StationEquations
    double GetStation() const { return GetRestartValue().Value(); }

    //! @private
    void SetStation(double newStation) { SetPropertyValue("Station", newStation); }

    ROADRAILALIGNMENT_EXPORT static AlignmentStationPtr Create(CreateAtParams const& params);
}; // AlignmentStation

//=======================================================================================
//! Handles the translation between a station, which may start at a non-zero value and accounts for any station equations,
//! and a linear distance along an Alignment
//! @ingroup GROUP_RoadRailAlignment
//=======================================================================================
struct AlignmentStationingTranslator : RefCountedBase
{
private:
    bvector<Alignment::DistanceAlongStationPair> m_stations;
    double m_length;

    AlignmentStationingTranslator(AlignmentCR alignment);

public:
    //! Create a new AlignmentStationingTranslator
    //! @param[in] alignment The Alignment that will be used for translation
    ROADRAILALIGNMENT_EXPORT static AlignmentStationingTranslatorPtr Create(AlignmentCR alignment);

    //! Translate a linear distance along the Alignment to a station value
    //! @param[in] distanceAlongFromStart The linear distance along the Alignment, in meters, from the start of the Alignment
    //! @return The station value corresponding to the \p distanceAlongFromStart
    ROADRAILALIGNMENT_EXPORT LinearReferencing::NullableDouble ToStation(double distanceAlongFromStart) const;

    //! Translate a station along the Alignment to a linear distance along
    //! @param[in] station The station, in meters, along the Alignment.
    //! @return The linear distance along the alignment, in meters.
    ROADRAILALIGNMENT_EXPORT LinearReferencing::NullableDouble ToDistanceAlongFromStart(double station) const;
}; // AlignmentStationingTranslator


//__PUBLISH_SECTION_END__
//=================================================================================
//! ElementHandler for AlignmentReferent Elements
//! @ingroup GROUP_RoadRailAlignment
//! @private
//=================================================================================
struct EXPORT_VTABLE_ATTRIBUTE LinearlyLocatedReferentElementHandler : Dgn::dgn_ElementHandler::SpatialLocation
{
ELEMENTHANDLER_DECLARE_MEMBERS(BRRA_CLASS_LinearlyLocatedReferentElement, LinearlyLocatedReferentElement, LinearlyLocatedReferentElementHandler, Dgn::dgn_ElementHandler::SpatialLocation, ROADRAILALIGNMENT_EXPORT)
}; //LinearlyLocatedReferentElementHandler

//=================================================================================
//! ElementHandler for AlignmentStation Elements
//! @ingroup GROUP_RoadRailAlignment
//! @private
//=================================================================================
struct EXPORT_VTABLE_ATTRIBUTE AlignmentStationHandler : LinearlyLocatedReferentElementHandler
{
ELEMENTHANDLER_DECLARE_MEMBERS(BRRA_CLASS_AlignmentStation, AlignmentStation, AlignmentStationHandler, LinearlyLocatedReferentElementHandler, ROADRAILALIGNMENT_EXPORT)
}; //AlignmentStationHandler

//__PUBLISH_SECTION_START__
END_BENTLEY_ROADRAILALIGNMENT_NAMESPACE