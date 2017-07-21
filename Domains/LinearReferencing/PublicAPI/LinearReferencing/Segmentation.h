/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/LinearReferencing/Segmentation.h $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#pragma once

//__PUBLISH_SECTION_START__
#include "LinearReferencingApi.h"

BEGIN_BENTLEY_LINEARREFERENCING_NAMESPACE

//=======================================================================================
//! Reference to a LinearlyReferencedLocation used to return the result of
//! querying for locations along an ILinearElement.
//! @ingroup GROUP_LinearReferencing
//=======================================================================================
struct LinearLocation
{
friend struct ISegmentableLinearElement;

private:
    double m_startDistanceAlong, m_stopDistanceAlong;
    Dgn::DgnElementId m_linearlyLocatedId;
    Dgn::DgnClassId m_linearlyLocatedClassId;
    LinearlyReferencedLocationId m_locationId;

    LinearLocation() :  m_startDistanceAlong(0), m_stopDistanceAlong(0),
        m_linearlyLocatedId(Dgn::DgnElementId()), m_linearlyLocatedClassId(Dgn::DgnClassId()), m_locationId(LinearlyReferencedLocationId()) {}

public:
    LinearLocation(Dgn::DgnElementId linearlyLocatedId, Dgn::DgnClassId linearlyLocatedClassId, double startDistanceAlong, double stopDistanceAlong,
        LinearlyReferencedLocationId locationId) :
        m_linearlyLocatedId(linearlyLocatedId), m_linearlyLocatedClassId(linearlyLocatedClassId),
        m_startDistanceAlong(startDistanceAlong), m_stopDistanceAlong(stopDistanceAlong), m_locationId(locationId)
        {}

    LinearLocation(ILinearlyLocatedCR linearlyLocated, double startDistanceAlong, double stopDistanceAlong):
        m_linearlyLocatedId(linearlyLocated.ToElement().GetElementId()), m_linearlyLocatedClassId(linearlyLocated.ToElement().GetElementClassId()),
        m_startDistanceAlong(startDistanceAlong), m_stopDistanceAlong(stopDistanceAlong), m_locationId(LinearlyReferencedLocationId())
        {}

    double GetStartDistanceAlong() const { return m_startDistanceAlong; }
    double GetStopDistanceAlong() const { return m_stopDistanceAlong; }
    Dgn::DgnElementId GetILinearlyLocatedId() const { return m_linearlyLocatedId; }
    Dgn::DgnClassId GetILinearlyLocatedClassId() const { return m_linearlyLocatedClassId; }
    LinearlyReferencedLocationId GetLinearlyReferencedLocationId() const { return m_locationId; }
}; // LinearLocation

//=======================================================================================
//! Part of a Linear-Element-Source that is distinguished from the remainder of itself 
//! by a subset of elements linearly-located along it, or attributes, each having a 
//! single value for the entire part.
//! @ingroup GROUP_LinearReferencing
//=======================================================================================
struct LinearSegment
{
private:
    double m_startDistanceAlong, m_stopDistanceAlong;
    bvector<LinearLocation> m_referencedLocations;

public:
    LinearSegment(double startDistanceAlong, double stopDistanceAlong, bvector<LinearLocation> const& referencedLocations) :
        m_startDistanceAlong(startDistanceAlong), m_stopDistanceAlong(stopDistanceAlong),  m_referencedLocations(referencedLocations)
        {}

    double GetStartDistanceAlong() const { return m_startDistanceAlong; }
    double GetStopDistanceAlong() const { return m_stopDistanceAlong; }
    bvector<LinearLocation> GetReferencedLocations() const { return m_referencedLocations; }
}; // LinearSegment

//=======================================================================================
//! Interface implemented by linear elements supporting operations for segmentation 
//! operation and output.
//! @ingroup GROUP_LinearReferencing
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE ISegmentableLinearElement : virtual ILinearElement
{
protected:
    LINEARREFERENCING_EXPORT virtual bvector<LinearLocation> _QueryLinearLocations(bset<Dgn::DgnClassId> const& iLinearlyLocatedClassIds, NullableDouble fromDistanceAlong, NullableDouble toDistanceAlong) const;
    LINEARREFERENCING_EXPORT virtual bvector<LinearSegment> _QueryLinearSegments(bset<Dgn::DgnClassId> const& iLinearlyLocatedClassIds, NullableDouble fromDistanceAlong, NullableDouble toDistanceAlong) const;

public:
    LINEARREFERENCING_EXPORT bvector<LinearLocation> QueryLinearLocations(bset<Dgn::DgnClassId> const& iLinearlyLocatedClassIds = bset<Dgn::DgnClassId>(),
        NullableDouble fromDistanceAlong = NullableDouble(), NullableDouble toDistanceAlong = NullableDouble()) const { return _QueryLinearLocations(iLinearlyLocatedClassIds, fromDistanceAlong, toDistanceAlong); }
    LINEARREFERENCING_EXPORT bvector<LinearSegment> QueryLinearSegments(bset<Dgn::DgnClassId> const& iLinearlyLocatedClassIds = bset<Dgn::DgnClassId>(),
        NullableDouble fromDistanceAlong = NullableDouble(), NullableDouble toDistanceAlong = NullableDouble()) const { return _QueryLinearSegments(iLinearlyLocatedClassIds, fromDistanceAlong, toDistanceAlong); }
}; // ISegmentableLinearElement

//=======================================================================================
//! Base class for algorithms cascading changes to FromTo linearly referenced locations.
//! These algorithms are expected to keep linearly located elements together. Neither
//! gaps nor overlaps allowed.
//! @ingroup GROUP_LinearReferencing
//=======================================================================================
struct CascadeFromToLocationChangesAlgorithm : ICascadeLinearLocationChangesAlgorithm
{
DEFINE_T_SUPER(ICascadeLinearLocationChangesAlgorithm)

protected:
    CascadeFromToLocationChangesAlgorithm(ILinearlyLocatedCR original, ILinearlyLocatedCR replacement, CascadeLocationChangesAction action):
        T_Super(original, replacement, action) {}

    LINEARREFERENCING_EXPORT void _FindFromToLocationChanges(bvector<LinearlyReferencedFromToLocationCP>& fromToLocationsChanged);
    LINEARREFERENCING_EXPORT virtual Dgn::DgnDbStatus _Prepare(ILinearElementSourceCR source) override final;
    LINEARREFERENCING_EXPORT virtual Dgn::DgnDbStatus _Commit(ILinearElementSourceCR source) override;

    LINEARREFERENCING_EXPORT virtual Dgn::DgnDbStatus _Prepare(ILinearElementSourceCR source,
        bvector<LinearLocation> const& existingLinearSegments, bvector<LinearlyReferencedFromToLocationCP> const& fromToLocationsChanged);
};

END_BENTLEY_LINEARREFERENCING_NAMESPACE