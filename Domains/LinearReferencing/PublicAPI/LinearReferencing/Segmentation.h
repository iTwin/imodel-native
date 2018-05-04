/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/LinearReferencing/Segmentation.h $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

//__BENTLEY_INTERNAL_ONLY__
#include "LinearReferencing.h"
#include "ILinearElement.h"

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
    LINEARREFERENCING_EXPORT virtual bvector<LinearSegment> _QueryLinearSegments(QueryParams const& params) const;

public:    
    LINEARREFERENCING_EXPORT bvector<LinearSegment> QueryLinearSegments(QueryParams const& params) const { return _QueryLinearSegments(params); }
}; // ISegmentableLinearElement

END_BENTLEY_LINEARREFERENCING_NAMESPACE