/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include <bsibasegeomPCH.h>

BEGIN_BENTLEY_GEOMETRY_NAMESPACE

CurveVectorWithXIndex::CurveVectorWithXIndex (CurveVectorCR curves)
    : m_curves (&curves)
    {
    }

CurveVectorCPtr CurveVectorWithXIndex::GetCurveVectorPtr () const
    {
    return m_curves;
    }
bool CurveVectorWithXIndex::AddIndexEntry (size_t index, double fraction)
    {
    CurveLocationDetail cd;
    if (index > m_curves->size ())
        return false;
    if (!m_curves->at (index)->FractionToPoint (fraction, cd))
        return false;
    double x = cd.point.x;
    if (   m_locations.size () > 0
        && DoubleOps::TolerancedComparison (m_locations.back ().m_pathDistance, x) == 1)
        return false;
    m_locations.push_back (PathLocationDetail (cd, index, x));
    return true;
    }

bool CurveVectorWithXIndex::BuildIndex ()
    {
    m_locations.clear ();
    if (m_curves->size () == 0)
        return false;
    m_locations.clear ();
    bool ok = AddIndexEntry (0, 0.0);
    for (size_t i = 0; i < m_curves->size (); i++)
        ok &= AddIndexEntry (i, 1.0);
    return ok;
    }

CurveVectorWithXIndexPtr CurveVectorWithXIndex::Create (CurveVectorR curves)
    {
    CurveVectorWithXIndexPtr alignment = new CurveVectorWithXIndex (curves);
    if (alignment->BuildIndex ())
        return alignment;
    return nullptr;
    }

DRange1d CurveVectorWithXIndex::XRange () const
    {
    if (m_locations.size () == 0)
        return DRange1d (); // null range
    return DRange1d (m_locations.front ().m_pathDistance, m_locations.back ().m_pathDistance);
    }
ValidatedPathLocationDetail CurveVectorWithXIndex::AtStart (bool valid) const
    {
    if (!m_locations.empty ())
        return ValidatedPathLocationDetail(m_locations.front (), valid);
    return ValidatedPathLocationDetail (CurveLocationDetail (), false);
    }

ValidatedPathLocationDetail CurveVectorWithXIndex::AtEnd (bool valid) const
    {
    if (!m_locations.empty ())
        return ValidatedPathLocationDetail(m_locations.back (), valid);
    return ValidatedPathLocationDetail (CurveLocationDetail (), false);
    }

ValidatedPathLocationDetail CurveVectorWithXIndex::XToPathLocationDetail (double x) const
    {
    if (m_locations.empty ())
        return AtStart (false);
    int startPosition = DoubleOps::TolerancedComparison (x, m_locations.front ().m_pathDistance);
    if (startPosition < 0)
        return AtStart (false);
    if (startPosition == 0)
        return AtStart (true);

    int endPosition = DoubleOps::TolerancedComparison (x, m_locations.back ().m_pathDistance);
    if (endPosition > 0)
        return AtEnd (false);
    if (endPosition == 0)
        return AtEnd (true);

    // strictly after start and before end ...
    PathLocationDetail key (x);

    // Use end ()-1 as upper bound -- hence the "too far" result is a valid list member.
    auto memberB = std::upper_bound (m_locations.begin (), m_locations.end () - 1,
            key,
            PathLocationDetail::IsLessThan_ByPathDistance
            );
            
    if (memberB == m_locations.begin())
        {
        return AtStart (true);
        }
    DPlane3d plane = DPlane3d::FromOriginAndNormal (x, 0,0, 1,0,0);
    bvector<CurveLocationDetailPair> intersections;
    memberB->m_curveDetail.curve->AppendCurvePlaneIntersections (plane, intersections);
    //  hm.. if it is an elevation curve, there should be only one intersection, and
    // it should be a single point (not an "on" interval).
    // Just take the first result.
    if (intersections.size () >= 1)
        {
        return ValidatedPathLocationDetail (PathLocationDetail (intersections[0].detailA, memberB->m_pathIndex, x), true);
        }
    // Plane intersection failed.  This should not happen.  take memberB.
    return  ValidatedPathLocationDetail (*memberB, false);
    }

CurveVectorPtr CurveVectorWithXIndex::CloneDirectedInterval (PathLocationDetailCR detailA, PathLocationDetailCR detailB) const
    {
    ValidatedPathLocationDetail validatedA = detailA.CorrectIndex  (*m_curves);
    ValidatedPathLocationDetail validatedB = detailB.CorrectIndex  (*m_curves);
    if (validatedA.IsValid () && validatedB.IsValid ())
        return m_curves->CloneBetweenDirectedFractions (detailA.m_pathIndex, detailA.m_curveDetail.fraction, detailB.m_pathIndex, detailB.m_curveDetail.fraction, false);
    return nullptr;
    }

CurveVectorPtr CurveVectorWithXIndex::CloneDirectedXInterval (double xA, double xB) const
    {
    ValidatedPathLocationDetail validatedA = XToPathLocationDetail (xA);
    ValidatedPathLocationDetail validatedB = XToPathLocationDetail (xB);
    if (validatedA.IsValid () && validatedB.IsValid ())
        return CloneDirectedInterval (validatedA.Value (), validatedB.Value ());
    return nullptr;
    }

double CurveVectorWithXIndex::GetStartX () const
    {
    if (m_locations.size () > 0)
        return m_locations.front ().m_pathDistance;
    return 0.0;
    }

double CurveVectorWithXIndex::GetEndX () const
    {
    if (m_locations.size () > 0)
        return m_locations.back ().m_pathDistance;
    return 0.0;
    }

double CurveVectorWithXIndex::XToFraction (double x) const
    {
    return DoubleOps::InverseInterpolate (GetStartX (), x, GetEndX ()).Value ();
    }

double CurveVectorWithXIndex::FractionToX (double fraction) const
    {
    return DoubleOps::Interpolate (GetStartX (), fraction, GetEndX ());
    }

END_BENTLEY_GEOMETRY_NAMESPACE
