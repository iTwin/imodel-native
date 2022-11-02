/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include <bsibasegeomPCH.h>
#include <float.h>
#include <math.h>
BEGIN_BENTLEY_GEOMETRY_NAMESPACE



/*-----------------------------------------------------------------*//**
* Constructor: retain tolerance
* @bsimethod
+---------------+---------------+---------------+---------------+------*/
InsertPointsInPolylineContext::InsertPointsInPolylineContext (double tolerance) : m_tolerance(tolerance) {}
/*-----------------------------------------------------------------*//**
*  Test for points within tolerance
* @bsimethod
+---------------+---------------+---------------+---------------+------*/
bool InsertPointsInPolylineContext::AlmostEqual (DPoint3dCR pointA, DPoint3dCR pointB) const
    {
    return pointA.Distance (pointB) <= m_tolerance;
    }

/*-----------------------------------------------------------------*//**
* Fill the m_interiorPoints array with <x,y,z,fraction> for all space points that are close to a
*   strict interior point of the segment.
* @bsimethod
+---------------+---------------+---------------+---------------+------*/
void InsertPointsInPolylineContext::CollectInteriorPointsAlongEdge (DSegment3dCR segment, bvector<DPoint3d> const &spacePoints)
    {
    m_interiorPoints.clear ();
    DPoint3d xyzB;
    double fraction;
    for (auto &xyzA : spacePoints)
        {
        segment.ProjectPoint (xyzB, fraction, xyzA);
        if (   fraction > 0.0
            && fraction < 1.0
            && AlmostEqual (xyzA, xyzB)
            && !AlmostEqual (xyzB, segment.point[0])
            && !AlmostEqual (xyzB, segment.point[1])
            )
            {
            m_interiorPoints.push_back (DPoint3dDouble (xyzA, fraction));
            }
        }
    }
/*-----------------------------------------------------------------*//**
* For each space point which is close to the interior of a segemnt of the polyline, insert the space point
* into the dest polyline.
* @bsimethod
+---------------+---------------+---------------+---------------+------*/
void InsertPointsInPolylineContext::InsertPointsInPolyline (bvector<DPoint3d> const &source, bvector<DPoint3d> &dest, bvector<DPoint3d> const &spacePoints)
    {
    dest.clear ();
    if (source.size () < 2)
        return;
    dest.push_back (source.front ());
    for (size_t i = 1; i < source.size (); i++)
        {
        CollectInteriorPointsAlongEdge (DSegment3d::From (source[i-1], source[i]), spacePoints);
        if (m_interiorPoints.size () > 0)
            {
            std::sort (m_interiorPoints.begin (), m_interiorPoints.end ());
            dest.push_back (m_interiorPoints.front ().xyz   );
            for (size_t k = 1; k < m_interiorPoints.size (); k++)
                {
                if (!AlmostEqual (m_interiorPoints[k].xyz, dest.back ()))
                    dest.push_back (m_interiorPoints[k].xyz);
                }
            }
        dest.push_back (source[i]);
        }
    }


END_BENTLEY_GEOMETRY_NAMESPACE
