/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
// for inclusion in source files in polyface directory

BEGIN_BENTLEY_GEOMETRY_NAMESPACE

struct ClipPlaneSetPolygonClipContext
    {
    BVectorCache<DPoint3d> m_currentCandidates;
    BVectorCache<DPoint3d> m_nextCandidates;

    bvector<DPoint3d> m_currentCandidate;
    BVectorCache<DPoint3d> m_shards;

    bvector<DPoint3d> m_inside;
    bvector<DPoint3d> m_work1;
    bvector<DPoint3d> m_work2;
    double m_distanceTolerance;

    ClipPlaneSetCR m_clipSet;
    ClipPlaneSetCP m_maskSet;
    ClipPlaneSetPolygonClipContext(ClipPlaneSetCR clipSet, ClipPlaneSetCP maskSet) : m_clipSet(clipSet), m_maskSet(maskSet)
        {
        m_distanceTolerance = 0;
        }

    static bool HasSignificantArea(bvector<DPoint3d> const &xyz, double areaTolerance)
        {
        if (xyz.size() < 3)
            return false;
        DVec3d normal = PolygonOps::AreaNormal(xyz);
        double area = normal.Magnitude();
        return area > areaTolerance;
        }

    void ClipAndCollect(bvector<DPoint3d> &polygon, ClipPlaneSetCR clipset, BVectorCache<DPoint3d> &insideShards, BVectorCache<DPoint3d> &outsideShards)
        {
        m_currentCandidates.ClearToCache();
        m_currentCandidates.PushCopy(polygon);
        DVec3d normal0 = PolygonOps::AreaNormal(polygon);
        double area0 = normal0.Magnitude();
        static double s_areaTol = 1.0e-10;
        double areaTolerance = area0 * s_areaTol;
        // m_candidates contains polygon content not yet found to be IN a clip set . . 
        for (auto convexSet : clipset)
            {
            while (m_currentCandidates.SwapBackPop(m_currentCandidate))
                {
                convexSet.ConvexPolygonClipInsideOutside(m_currentCandidate, m_inside, m_shards, m_work1, m_work2, true, m_distanceTolerance);
                if (m_inside.size() > 0 && HasSignificantArea(m_inside, areaTolerance))
                    insideShards.PushCopy(m_inside);
                // shards become candidates for further clip plane sets
                m_nextCandidates.MoveAllFrom(m_shards);
                }
            m_currentCandidates.MoveAllFrom(m_nextCandidates);
            }
        for (auto &outside : m_currentCandidates)
            {
            if (HasSignificantArea(outside, areaTolerance))
                outsideShards.PushCopy(outside);
            }
        }

    };
END_BENTLEY_GEOMETRY_NAMESPACE
