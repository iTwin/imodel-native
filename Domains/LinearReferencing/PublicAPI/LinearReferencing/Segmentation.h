/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/LinearReferencing/Segmentation.h $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#pragma once

//__PUBLISH_SECTION_START__
#include "LinearReferencingApi.h"

BEGIN_BENTLEY_LINEARREFERENCING_NAMESPACE

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
    ILinearlyLocatedCP m_linearlyLocatedCP;

    LinearSegment() :  m_startDistanceAlong(0), m_stopDistanceAlong(0), m_linearlyLocatedCP(nullptr) {}

public:
    LinearSegment(ILinearlyLocatedCR linearlyLocated, double startDistanceAlong, double stopDistanceAlong):
        m_linearlyLocatedCP(&linearlyLocated), m_startDistanceAlong(startDistanceAlong), m_stopDistanceAlong(stopDistanceAlong)
        {}

    LINEARREFERENCING_EXPORT double GetStartDistanceAlong() const { return m_startDistanceAlong; }
    LINEARREFERENCING_EXPORT double GetStopDistanceAlong() const { return m_stopDistanceAlong; }
    LINEARREFERENCING_EXPORT ILinearlyLocatedCR GetILinearlyLocated() const { return *m_linearlyLocatedCP; }
}; // LinearSegment

//=======================================================================================
//! Interface implemented by linear elements supporting operations for segmentation 
//! operation and output.
//! @ingroup GROUP_LinearReferencing
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE ISegmentableLinearElement : ILinearElement
{
protected:
    LINEARREFERENCING_EXPORT virtual bvector<LinearSegment> _QuerySegments(bset<Dgn::DgnClassId> const& iLinearlyLocatedClassIds, NullableDouble fromDistanceAlong, NullableDouble toDistanceAlong) const;

public:
    LINEARREFERENCING_EXPORT bvector<LinearSegment> QuerySegments(bset<Dgn::DgnClassId> const& iLinearlyLocatedClassIds = bset<Dgn::DgnClassId>(),
        NullableDouble fromDistanceAlong = NullableDouble(), NullableDouble toDistanceAlong = NullableDouble()) const { return _QuerySegments(iLinearlyLocatedClassIds, fromDistanceAlong, toDistanceAlong); }
}; // ISegmentableLinearElement

END_BENTLEY_LINEARREFERENCING_NAMESPACE