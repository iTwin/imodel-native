/*--------------------------------------------------------------------------------------+
|
|     $Source: geom/src/structs/DPoint3dDoubleArrays.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <bsibasegeomPCH.h>

BEGIN_BENTLEY_GEOMETRY_NAMESPACE


//! Append to each array
void DPoint3dDoubleArrays::AppendXF (DPoint3dCR xyz, double f)
    {
    m_xyz.push_back (xyz);
    m_f.push_back (f);
    }

//! Append xyz to its array, 
//! If there are prior fractions, add deltaF to the last.
//! If not, make the begin () fraction 0.0;
void DPoint3dDoubleArrays::AppendXdeltaF (DPoint3dCR xyz, double deltaF)
    {
    m_xyz.push_back (xyz);
    if (m_f.empty ())
        m_f.push_back (0.0);
    else
        m_f.push_back (m_f.back () + deltaF);
    }

//!Search the fraction array for an interval containing f.
//!<ul>
//!<li>This assumes the fractions are sorted
//!<li>When f is within the range of the fractions, f0 and f1 are surrounding values.
//!<li>When f is outside the range of the fractions, f0 and f1 are the appropriate boundary interval.
//!<li>i0 and i1 correspond to f0, f1
//!</ul>
bool DPoint3dDoubleArrays::SearchBracketPoints (double f, size_t &i0, double &f0, DPoint3dR xyz0, size_t &i1, double &f1, DPoint3dR xyz1) const
    {
    if (DoubleOps::BoundingValues (m_f, f, i0, f0, f1))
        {
        i1 = i0 + 1;
        xyz0 = m_xyz[i0];
        xyz1 = m_xyz[i1];
        return true;
        }
    return false;
    }

//! Return the range of the points.
DRange3d DPoint3dDoubleArrays::GetRange () const
    {
    return DPoint3dOps::Range (&m_xyz);
    }
//! Return the range of the points under a transform.
DRange3d DPoint3dDoubleArrays::GetRange (TransformCR transform) const
    {
    return DPoint3dOps::Range (&m_xyz, transform);
    }
//! Reverse the order of all arrays.
//! Optionally change each fraction value (f) to (1-f) so it remains sorted and 0 to 1.
//! This is virtual so derived classes can reverse additional arrays.
void DPoint3dDoubleArrays::ReverseXF (bool reverseAs01Fraction)
    {
    std::reverse (m_xyz.begin (), m_xyz.end ());
    std::reverse (m_f.begin (), m_f.end ());
    if (reverseAs01Fraction)
        for (auto &f : m_f)
            f = 1.0 - f;
    }

void DPoint3dDoubleArrays::Stroke(DEllipse3dCR arc, size_t numPoints)
    {
    m_xyz.clear ();
    m_f.clear ();
    if (numPoints > 0)
        {
        double df = numPoints == 1 ? 1.0 : 1.0 / (double)(numPoints - 1);
        for (uint32_t i = 0; i < numPoints; i++)
            {
            double f = i * df;
            if (i + 1 == numPoints)
                f = 1.0;
            m_f.push_back (f);
            m_xyz.push_back (arc.FractionToPoint (f));
            }
        }
    }

void DPoint3dDoubleArrays::Stroke(DSegment3dCR segment, size_t numPoints)
    {
    m_xyz.clear ();
    m_f.clear ();
    if (numPoints > 0)
        {
        double df = numPoints == 1 ? 1.0 : 1.0 / (double)(numPoints - 1);
        for (uint32_t i = 0; i < numPoints; i++)
            {
            double f = i * df;
            if (i + 1 == numPoints)
                f = 1.0;
            m_f.push_back (f);
            m_xyz.push_back (DPoint3d::FromInterpolate (segment.point[0], f, segment.point[1]));
            }
        }
    }

DPoint3dDoubleArrays::DPoint3dDoubleArrays(DEllipse3dCR arc, size_t numPoints) {Stroke (arc, numPoints);}
DPoint3dDoubleArrays::DPoint3dDoubleArrays(DSegment3dCR segment, size_t numPoints) {Stroke (segment, numPoints);}

void DPoint3dDoubleUVArrays::ReverseXFUV (bool reverseAs01Fraction, bool negateVectorU, bool negateVectorV)
    {
    ReverseXF (reverseAs01Fraction);
    // These will do nothing for empty arrays ..
    std::reverse (m_uv.begin (), m_uv.end ());
    std::reverse (m_vectorU.begin (), m_vectorU.end ());
    if (negateVectorU)
        {
        for (auto &uvw : m_vectorU)
            uvw.Negate ();
        }

    std::reverse (m_vectorV.begin (), m_vectorV.end ());
    if (negateVectorV)
        {
        for (auto &uvw : m_vectorV)
            uvw.Negate ();
        }
    }

void DPoint3dDoubleUVCurveArrays::ReverseXFUVC (size_t index0, size_t index1, bool reverseAs01Fraction, bool negateVectorU, bool negateVectorV)
    {
    if (m_xyz.size () >= index1)
        std::reverse (m_xyz.begin () + index0, m_xyz.begin () + index1);
    if (m_f.size () >= index1)
        {
        std::reverse (m_f.begin () + index0, m_f.begin () + index1);
        if (reverseAs01Fraction)
            for (size_t i = index0; i < index1; i++)
                m_f[i] = 1.0 - m_f[i];
        }
    if (m_vectorU.size () >= index1)
        {
        std::reverse (m_vectorU.begin () + index0, m_vectorU.begin () + index1);
        if (negateVectorU)
            for (size_t i = index0; i < index1; i++)
                m_vectorU[i].Negate ();
        }
    if (m_vectorV.size () >= index1)
        {
        std::reverse (m_vectorV.begin () + index0, m_vectorV.begin () + index1);
        if (negateVectorV)
            for (size_t i = index0; i < index1; i++)
                m_vectorV[i].Negate ();
        }
    if (m_curve.size () >= index1)
        std::reverse (m_curve.begin () + index0, m_curve.begin () + index1);
    }



void DPoint3dDoubleUVCurveArrays::ReverseXFUVC (bool reverseFAs01Fraction, bool negateVectorU, bool negateVectorV)
    {
    ReverseXFUV (reverseFAs01Fraction, negateVectorU, negateVectorV);
    std::reverse (m_curve.begin (), m_curve.end ());

    }

END_BENTLEY_GEOMETRY_NAMESPACE
