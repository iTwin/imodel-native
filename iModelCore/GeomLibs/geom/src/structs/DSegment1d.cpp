/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#include <bsibasegeomPCH.h>

BEGIN_BENTLEY_GEOMETRY_NAMESPACE

DSegment1d::DSegment1d (double x0, double x1) : m_x0(x0), m_x1(x1) {}
DSegment1d::DSegment1d (double x) : m_x0(x), m_x1(x) {}

DSegment1d DSegment1d::CopyTranslated (double delta) const
    {
    return DSegment1d (m_x0 + delta, m_x1 + delta);
    }

ValidatedDSegment1d DSegment1d::CopyAsFractionOf (DSegment1dCR parent) const
    {
    double divL;
    bool stat = DoubleOps::SafeDivide (divL, 1.0, parent.m_x1 - parent.m_x0, 0.0);
    return ValidatedDSegment1d (
            DSegment1d((m_x0 - parent.m_x0) * divL,
                       (m_x1 - parent.m_x0) * divL
                      ), stat);
    }

double DSegment1d::GetStart () const { return m_x0;}

double DSegment1d::GetEnd () const {return m_x1;}

//! modify start coordinate.
void DSegment1d::SetStart (double x) { m_x0 = x;}
//! modify end coordinate.
void DSegment1d::SetEnd (double x) { m_x1 = x;}

bool DSegment1d::IsStrictInterior (double x) const {return EndPointProduct (x) < 0.0;}

bool DSegment1d::Is01 () const {return m_x0 == 0.0 && m_x1 == 1.0;}

bool DSegment1d::AlmostEqual (DSegment1d const &other, double tolerance) const
    {
    return DoubleOps::AlmostEqual (m_x0, other.m_x0, tolerance)
        &&  DoubleOps::AlmostEqual (m_x1, other.m_x1, tolerance);
    }


bool DSegment1d::IsInteriorOrEnd (double x) const {return EndPointProduct (x) <= 0.0;}

double DSegment1d::EndPointProduct (double x) const {return (x - m_x0) * (x - m_x1);}

double DSegment1d::FractionToPoint (double fraction) const
    {
    return m_x0 + fraction * (m_x1 - m_x0);
    }

bool DSegment1d::PointToFraction (double x, double &fraction) const
    {
    return DoubleOps::SafeDivide (fraction, x - m_x0, m_x1 - m_x0, 0.0);
    }

DSegment1d DSegment1d::BetweenFractions (double f0, double f1) const
    {
    return DSegment1d (FractionToPoint (f0), FractionToPoint (f1));
    }

DSegment1d DSegment1d::Reverse () const
    {
    return DSegment1d (m_x1, m_x0);
    }

DSegment1d DSegment1d::Mirror01 () const
    {
    return DSegment1d (1.0 - m_x0, 1.0 - m_x1);
    }


void DSegment1d::ReverseInPlace ()
    {
    std::swap (m_x0, m_x1);
    }

double DSegment1d::Length () const
    {
    return fabs (m_x1 - m_x0);
    }

double DSegment1d::Delta () const
    {
    return m_x1 - m_x0;
    }

double DSegment1d::EndToStartDelta (DSegment1dCR other) const
    {
    return other.m_x0 - m_x1;
    }

double DSegment1d::EndToStartDistance (DSegment1dCR other) const
    {
    return fabs (EndToStartDelta (other));
    }


bool DSegment1d::IsEqual (DSegment1dCR other) const
    {
    return m_x0 == other.m_x0 && m_x1 == other.m_x1;
    }

bool DSegment1d::IsReversed (DSegment1dCR other) const
    {
    return m_x0 == other.m_x1 && m_x1 == other.m_x0;
    }

ValidatedDSegment1d DSegment1d::DirectedOverlap (DSegment1dCR other) const
    {
    DSegment1d result = *this;
    double clip0 = other.m_x0;
    double clip1 = other.m_x1;
    if (clip0 > clip1)
        {
        clip1 = other.m_x0;
        clip0 = other.m_x1;
        }
    if (m_x1 >= m_x0)
        {
        if (result.m_x0 < clip0)
            result.m_x0 = clip0;
        if (result.m_x1 > clip1)
            result.m_x1 = clip1;
        if (result.m_x1 >= result.m_x0)
            return ValidatedDSegment1d (result, true);
        }
    else
        {
        if (result.m_x1 < clip0)
            result.m_x1 = clip0;
        if (result.m_x0 > clip1)
            result.m_x0 = clip1;
        if (result.m_x1 <= result.m_x0)
            return ValidatedDSegment1d (result, true);
        }
    return ValidatedDSegment1d (result, false);
    }

ValidatedDSegment1d DSegment1d::NonZeroDirectedOverlap (DSegment1dCR other) const
    {
    ValidatedDSegment1d result = DirectedOverlap (other);
    if (result.IsValid () && result.Value ().Length () <= 0.0)
        result.SetIsValid (false);
    return result;  
    }
    
bool DSegment1d::NonZeroFractionalDirectedOverlap (DSegment1dCR segmentA, DSegment1dCR segmentB, DSegment1dR fractionA, DSegment1dR fractionB)
    {
    auto overlap0 = segmentA.DirectedOverlap (segmentB);
    DSegment1d overlap = overlap0.Value ();
    if (overlap0.IsValid () && overlap.Length () > 0.0)
        {
        // nonzero length ==> safe normalization (division by lengths that are equal or larger)
        fractionA = overlap.CopyAsFractionOf (segmentA);
        fractionB = overlap.CopyAsFractionOf (segmentB);
        return true;
        }
    fractionA = DSegment1d (0,0);
    fractionB = DSegment1d (0,0);
    return false;
    }
    
END_BENTLEY_GEOMETRY_NAMESPACE