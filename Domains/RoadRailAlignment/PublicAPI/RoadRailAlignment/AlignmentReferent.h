/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once

//__PUBLISH_SECTION_START__
#include "RoadRailAlignment.h"
#include "Alignment.h"

BEGIN_BENTLEY_ROADRAILALIGNMENT_NAMESPACE

//=======================================================================================
//! Well-known station along an alignment.
//! @ingroup GROUP_RoadRailAlignment
//=======================================================================================
struct AlignmentStation : LinearReferencing::ReferentElement
{
    DGNELEMENTWRAPPER_DECLARE_MEMBERS(LinearReferencing::ReferentElement, Dgn::SpatialLocationElement)

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
    explicit AlignmentStation(Dgn::SpatialLocationElement const& element) : T_Super(element) {}
    explicit AlignmentStation(Dgn::SpatialLocationElement& element) : T_Super(element) {}

    //! @private
    explicit AlignmentStation(Dgn::SpatialLocationElement& element, CreateAtParams const& atParams);

public:
    DECLARE_ROADRAILALIGNMENT_QUERYCLASS_METHODS(AlignmentStation)
    DECLARE_ROADRAILALIGNMENT_ELEMENT_GET_METHODS(AlignmentStation, Dgn::SpatialLocationElement)
    DECLARE_LINEARREFERENCING_LINEARLYLOCATED_SET_METHODS(AlignmentStation, Dgn::SpatialLocationElement)

    //! Returns the station, accounting for any StationEquations
    double GetStation() const { return get()->GetPropertyValueDouble("Station"); }

    //! @private
    void SetStation(double newStation) { getP()->SetPropertyValue("Station", newStation); }

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

END_BENTLEY_ROADRAILALIGNMENT_NAMESPACE