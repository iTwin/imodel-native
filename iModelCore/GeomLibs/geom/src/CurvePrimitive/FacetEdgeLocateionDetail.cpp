/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/

#include <bsibasegeomPCH.h>

BEGIN_BENTLEY_GEOMETRY_NAMESPACE
FacetEdgeLocationDetail::FacetEdgeLocationDetail (size_t readIndex, double fraction)
    : m_readIndex (readIndex), m_fraction(fraction)
    {
    }
    
FacetEdgeLocationDetail::FacetEdgeLocationDetail ()
    : m_readIndex (SIZE_MAX), m_fraction(0.0)
    {
    }

void FacetEdgeLocationDetailVector::Add (size_t readIndex, double fraction)
    {
    m_data.push_back (FacetEdgeLocationDetail (readIndex, fraction));
    }

void FacetEdgeLocationDetailVector::Add (FacetEdgeLocationDetailCR data)
    {
    m_data.push_back (data);
    }
    
    
size_t FacetEdgeLocationDetailVector::size () const
    {
    return m_data.size ();
    }
    
bool FacetEdgeLocationDetailVector::TryGet (size_t index, size_t &readIndex, double &fraction) const
    {
    if (index >= m_data.size ())
        {
        readIndex = SIZE_MAX;
        fraction = 0.0;
        return false;
        }
    readIndex = m_data[index].m_readIndex;
    fraction  = m_data[index].m_fraction;
    return true;
    }

bool FacetEdgeLocationDetailVector::TryGet (size_t index, FacetEdgeLocationDetailR data) const
    {
    if (index >= m_data.size ())
        {
        data = FacetEdgeLocationDetail (SIZE_MAX, 0.0);
        return false;
        }
    data = m_data[index];
    return true;
    }

FacetEdgeLocationDetailVector::FacetEdgeLocationDetailVector (){}

FacetEdgeLocationDetailVectorPtr FacetEdgeLocationDetailVector::Create ()
    {
    return new FacetEdgeLocationDetailVector ();
    }


END_BENTLEY_GEOMETRY_NAMESPACE
