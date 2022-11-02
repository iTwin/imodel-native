/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include <bsibasegeomPCH.h>

BEGIN_BENTLEY_GEOMETRY_NAMESPACE

/*--------------------------------------------------------------------------------**//**
* @bsimethod
+--------------------------------------------------------------------------------------*/
PathLocationDetail::PathLocationDetail (CurveLocationDetailCR curveDetail, int pathIndex, double pathDistance)
    : m_curveDetail (curveDetail), m_pathIndex (pathIndex), m_pathDistance (pathDistance)
    {
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod
+--------------------------------------------------------------------------------------*/
PathLocationDetail::PathLocationDetail (CurveLocationDetailCR curveDetail, size_t pathIndex, double pathDistance)
    : m_curveDetail (curveDetail), m_pathIndex ((int)pathIndex), m_pathDistance (pathDistance)
    {
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod
+--------------------------------------------------------------------------------------*/
PathLocationDetail::PathLocationDetail (double pathDistance)
    : m_pathDistance (pathDistance)
    {
    }


bool PathLocationDetail::AlmostEquallPoint (PathLocationDetailCR dataA, PathLocationDetailCR dataB)
    {
    return dataA.m_curveDetail.point.AlmostEqual (dataB.m_curveDetail.point);
    }

bool PathLocationDetail::AlmostEquallPointDistance (PathLocationDetailCR dataA, PathLocationDetailCR dataB)
    {
    return dataA.m_curveDetail.point.AlmostEqual (dataB.m_curveDetail.point)
            && DoubleOps::AlmostEqual (dataA.m_pathDistance, dataB.m_pathDistance);
    }

bool PathLocationDetail::AlmostEquallPointDistanceCurve (PathLocationDetailCR dataA, PathLocationDetailCR dataB)
    {
    return dataA.m_curveDetail.point.AlmostEqual (dataB.m_curveDetail.point)
            && DoubleOps::AlmostEqual (dataA.m_pathDistance, dataB.m_pathDistance)
            && dataA.m_curveDetail.curve == dataB.m_curveDetail.curve;
    }

void PathLocationDetail::GetDistances (bvector<PathLocationDetail> const &locations, bvector<double> &distances)
    {
    distances.clear ();
    distances.reserve (locations.size());
    for (PathLocationDetail const &location : locations)
        distances.push_back (location.m_pathDistance);
    }


/*--------------------------------------------------------------------------------**//**
* @bsimethod
+--------------------------------------------------------------------------------------*/
DPoint3d PathLocationDetail::Point () const { return m_curveDetail.point;}
CurveLocationDetail PathLocationDetail::GetCurveLocationDetail () const { return m_curveDetail;}
bool PathLocationDetail::HasCurve () const { return m_curveDetail.curve != nullptr;}

bool PathLocationDetail::IsSameCurve (PathLocationDetail const &other) const
    {
    return m_curveDetail.curve == other.m_curveDetail.curve;
    }

bool PathLocationDetail::IsSameCurve (PathLocationDetail const &other, DSegment1dR fractionInterval, ICurvePrimitive::CurvePrimitiveType &type) const
    {

    type = m_curveDetail.curve->GetCurvePrimitiveType ();
    fractionInterval = DSegment1d (m_curveDetail.fraction, other.m_curveDetail.fraction);
    if (m_curveDetail.curve == other.m_curveDetail.curve)
        return true;
    fractionInterval = DSegment1d (m_curveDetail.fraction);
    return false;
    }


DPoint3d PathLocationDetail::PointAndUnitTangent (DVec3dR tangent) const
  {
  DPoint3d xyz;
  m_curveDetail.curve->FractionToPoint (m_curveDetail.fraction, xyz, tangent);
  tangent.Normalize ();
  return m_curveDetail.point;
  }

ValidatedPathLocationDetail PathLocationDetail::CorrectIndex (CurveVectorCR path) const
    {
    if (m_curveDetail.curve == nullptr)
        return ValidatedPathLocationDetail (*this, false);        
    if ((size_t)m_pathIndex < path.size () && path[m_pathIndex].get () == m_curveDetail.curve)
        return ValidatedPathLocationDetail (*this, true);
    // Search for the curve.
    for (size_t i = 0; i < path.size (); i++)
        if (path[i].get () == m_curveDetail.curve)
            return ValidatedPathLocationDetail (*this, true);
    return ValidatedPathLocationDetail (*this, false);        
    }  

//============================================================================================
//============================================================================================
//============================================================================================

PathLocationDetail  PathLocationDetailPair::DetailA () const {return m_detailA;}
PathLocationDetail  PathLocationDetailPair::DetailB () const {return m_detailB;}
PathLocationDetail  PathLocationDetailPair::Detail (size_t select01) const {return select01 == 0 ? m_detailA : m_detailB;}

void PathLocationDetailPair::SetDetailA (PathLocationDetail const &value) {m_detailA = value;}
void PathLocationDetailPair::SetDetailB (PathLocationDetail const &value) {m_detailB = value;}


ptrdiff_t PathLocationDetailPair::GetTagA () const {return m_tagA;}
ptrdiff_t PathLocationDetailPair::GetTagB () const {return m_tagB;}
ptrdiff_t PathLocationDetailPair::GetTag (size_t index) const {return index == 0 ? m_tagA : m_tagB;}
void PathLocationDetailPair::SetTagA (ptrdiff_t tag) {m_tagA = tag;}
void PathLocationDetailPair::SetTagB (ptrdiff_t tag) {m_tagB = tag;}

PathLocationDetailPair::PathLocationDetailPair (PathLocationDetail const &detailA, PathLocationDetail const &detailB, ptrdiff_t tagA, ptrdiff_t tagB)
    : m_detailA (detailA), m_detailB (detailB), m_tagA (tagA), m_tagB (tagB)
    {}

PathLocationDetailPair::PathLocationDetailPair ()
    : m_tagA (0), m_tagB (0) {}


GEOMDLLIMPEXP bool PathLocationDetailPair::IsLessThanByPathDistance_lexicalAB (PathLocationDetailPair const &dataA, PathLocationDetailPair const &dataB)
    {
    if (PathLocationDetail::IsLessThan_ByPathDistance (dataA.m_detailA, dataB.m_detailA))
        return true;
    if (PathLocationDetail::IsLessThan_ByPathDistance (dataB.m_detailA, dataA.m_detailA))
        return false;
    if (PathLocationDetail::IsLessThan_ByPathDistance (dataA.m_detailB, dataB.m_detailB))
        return true;
    return false;
    }

GEOMDLLIMPEXP bool PathLocationDetailPair::IsLessThanByPathDistance_lexicalBA (PathLocationDetailPair const &dataA, PathLocationDetailPair const &dataB)
    {
    if (PathLocationDetail::IsLessThan_ByPathDistance (dataA.m_detailB, dataB.m_detailB))
        return true;
    if (PathLocationDetail::IsLessThan_ByPathDistance (dataB.m_detailB, dataA.m_detailB))
        return false;
    if (PathLocationDetail::IsLessThan_ByPathDistance (dataA.m_detailA, dataB.m_detailA))
        return true;
    return false;
    }


END_BENTLEY_GEOMETRY_NAMESPACE
