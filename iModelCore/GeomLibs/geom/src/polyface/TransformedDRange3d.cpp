/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include <bsibasegeomPCH.h>
#include "./ClipPlaneSetPolygonClipContext.h"

BEGIN_BENTLEY_GEOMETRY_NAMESPACE
TransformedDRange3d::TransformedDRange3d(DRange3dCR range, TransformCR localToWorld, TransformCR worldToLocal) :
    m_localToWorld(localToWorld),
    m_worldToLocal(worldToLocal),
    m_localRange(range)
    {
    range.Get8Corners(m_worldCorners);
    m_localToWorld.Multiply(m_worldCorners, 8);
    }

TransformedDRange3dPtr TransformedDRange3d::Create(DRange3dCR range, TransformCR localToWorld)
    {
    auto worldToLocal = localToWorld.ValidatedInverse();
    if (!worldToLocal.IsValid())
        return nullptr;
    return new TransformedDRange3d(range, localToWorld, worldToLocal);
    }

InOutStates TransformedDRange3d::Classify(ClipPlaneCR plane) const
    {
    InOutStates state;
    // classify each corner.  Quick return as soon as both in and out are detected.
    // we know everything will shortly get classified, so mark that immediately . . .
    state.m_isFullyClassified = true;
    for (uint32_t i = 0; i < 8; i++)
        {
        if (plane.IsPointOnOrInside(m_worldCorners[i]))
            {
            state.m_hasInsidePoints = true;
            if (state.m_hasOutsidePoints)
                state.m_isFullyClassified = true;
            }
        else
            {
            state.m_hasOutsidePoints = true;
            if (state.m_hasInsidePoints)
                state.m_isFullyClassified = true;
            }
        }
    // drop through with everything in or everything out.
    return state;
    }

//! return true if this TransformedRange is entirely contained inside the single clip plane.
bool TransformedDRange3d::IsAllInside(ClipPlaneCR plane) const
    {
    // classify each corner.  Quick return as soon as a single outside is found
    for (uint32_t i = 0; i < 8; i++)
        {
        if (!plane.IsPointOnOrInside(m_worldCorners[i]))
            return false;
        }
    return true;
    }
bool TransformedDRange3d::IsAllInside(ConvexClipPlaneSetCR clipper) const
    {
    for (uint32_t clipperIndex = 0; clipperIndex < clipper.size(); clipperIndex++)
        {
        for (uint32_t i = 0; i < 8; i++)
            if (!clipper[clipperIndex].IsPointOnOrInside(m_worldCorners[i]))
                return false;
        }
    return true;
    }
//! get the 8 world corners
void TransformedDRange3d::GetWorldCorners(bvector<DPoint3d> &corners) const
    {
    corners.clear();
    for (uint32_t i = 0; i < 8; i++)
        corners.push_back(m_worldCorners[i]);
    }

InOutStates TransformedDRange3d::Classify(ConvexClipPlaneSetCR clipper,
    bool allowQuickAmbiguousReturn) const
    {
    InOutStates compositeState;
    for (auto &plane : clipper) {
        auto stateForPlane = Classify (plane);
        if (stateForPlane.IsAllOutside())
            return InOutStates (true, false, true);;
        compositeState.OrInOutPartsInPlace (stateForPlane);
        }
    // If all corners were in for all planes, it's in allIn hit .. ..
    if (!compositeState.m_hasOutsidePoints)
        return InOutStates(true, true, false);
    if (compositeState.m_hasInsidePoints)
        return InOutStates(true, true,true);
    if (allowQuickAmbiguousReturn)
        return InOutStates(false, false, true);
    // All corners are outside.  But we don't know if there are edges or faces
    // that make contact.
    // If any face point that contacts the clipper makes it mixed ...
    bvector<DPoint3d> facePoints;
    bvector<DPoint3d> clippedPoints;
    bvector<DPoint3d> work;
    for (int i = 0; i < 6; i++)
        {
        BoxFaces::Get5PointCCWFace (i, m_worldCorners, facePoints);
        clipper.ConvexPolygonClip (facePoints, clippedPoints, work, 1);
        if (clippedPoints.size() > 0)
            {
            return InOutStates(true, true, true);
            }
        facePoints.swap (clippedPoints);
        }
    // No contact at face points.
    // Two cases left:
    // (a) the range is completely outside the clipper.
    // (b) the clipper is completely inside the range.
    // Case (b) cannot happen for an unbounded range.
    // A bounded range must have corner points.
    // Get any corner and resolve it the question . . .
    auto worldXYZ = clipper.FindAnyVertex ();
    if (worldXYZ.IsValid())
        {
        DPoint3d localXYZ = m_worldToLocal * worldXYZ;
        if (m_localRange.IsContained(localXYZ))
            return InOutStates(true, true, true);
        return InOutStates(true, false, true);
        }
    return compositeState;
    }

InOutStates TransformedDRange3d::Classify(ClipPlaneSetCR clipper) const
    {
    double tolerance = DoubleOps::SmallMetricDistance ();
    InOutStates compositeState;
    // Successively test corners, edges, and faces of the range.
    // At each step, if definite in and definite out points have appeared exit immediately with 
    //  the mixed state.
    // After falling through that, each and every one of the convex clippers has avoided any face contact.
    // Hence single point tests for each clipper can imply state of the entire clipper.
    // (And any clipper that has no vertices must be all out -- it's unbounded, so if any point is in
    //    the unbounded planes would have to cross the range faces)
    // The face testing could subsume the vertex and edge stages.
    // But vertex is much faster than edge, and edge is much faster than face,
    //   so do them first hoping for quick exit.
    // Vertices ...
    for (uint32_t i = 0; i < 8; i++)
        {
        compositeState.AnnouncePartialClassification (clipper.IsPointOnOrInside(m_worldCorners[i], tolerance));
        if (compositeState.IsMixed ())
            return InOutStates::DefiniteMixed();
        }
    // edges ...
    bvector<DSegment1d> edgeIntervals;
    static double s_almostOne = 1.0 - 1.e-8;
    for (uint32_t axis = 0; axis < 3; axis++)
        {
        for (uint32_t edgeIndex = 0; edgeIndex < 4; edgeIndex++)
            {
            DSegment3d segment = BoxFaces::GetEdgeSegment (m_worldCorners, axis, edgeIndex);
            edgeIntervals.clear ();
            clipper.AppendIntervals (segment, edgeIntervals);
            if (edgeIntervals.size () == 0)
                compositeState.m_hasOutsidePoints = true;
            else
                {
                compositeState.m_hasInsidePoints = true;
                double sum = 0.0;
                for(auto &interval : edgeIntervals)
                    sum += interval.Length ();
                if (sum < s_almostOne)
                    compositeState.m_hasOutsidePoints = true;
                }
            if (compositeState.IsMixed ())
                return InOutStates::DefiniteMixed ();
            }
        }
    // faces ... this triggers memory allocation to accomodate outside parts . . .
    ClipPlaneSetPolygonClipContext context(clipper, nullptr);
    BVectorCache<DPoint3d> inside;
    BVectorCache<DPoint3d> outside;
    bvector<DPoint3d> facePoints;
    for (int i = 0; i < 6; i++)
        {
        BoxFaces::Get5PointCCWFace(i, m_worldCorners, facePoints);
        outside.PopToCache();
        inside.PopToCache();
        context.ClipAndCollect(facePoints, clipper, inside, outside);
        if (inside.size () > 0)
            compositeState.m_hasInsidePoints = true;
        if (outside.size () > 0)
            compositeState.m_hasOutsidePoints = true;
        if (compositeState.IsMixed())
            return InOutStates::DefiniteMixed();
        }
    // ugh .. the faces are entirely in or entirely out.
    // because this union of convex sets can be very complicated, we cannot
    // extrapolate this to the interior.
    if (compositeState.m_hasOutsidePoints)
        {
        // we can treat each convex set as a single point to complete classification .
        for (auto &convexClipper : clipper)
            {
            auto worldXYZ = convexClipper.FindAnyVertex();
            if (worldXYZ.IsValid())
                {
                DPoint3d localXYZ = m_worldToLocal * worldXYZ.Value ();
                if (m_localRange.IsContained(localXYZ))
                    {
                    compositeState.m_hasInsidePoints = true;
                    if (compositeState.IsMixed())
                        return InOutStates::DefiniteMixed();
                    }
                }
           }
        return InOutStates::DefiniteAllOut ();
        }
    // The entire boundary (all 6 faces) is "inside" the clipper.
    // Surely the whole range is inside, yes?
    // NO -- its possible that the clipper has a void that perversely fits
    //   completely in the range.
    // So we have to search clipper planes for (just) one that has
    //   * a real intersection with the range
    //   * And that real intersection is a live part of that clipper volume.
    bvector<DPoint3d> clippedPoints, work;
    for (auto &convexClipper : clipper)
        {
        for (auto &clipPlane : convexClipper)
            {
            // don't consider a plane marked as interior to its volume . .
            if (clipPlane.GetIsInvisible())
                continue;
            auto localPlane = clipPlane.GetDPlane3d ();
            m_worldToLocal.Multiply (localPlane, localPlane);
            ClipPlane::ClipPlaneToRange (m_localRange, localPlane, facePoints);
            m_localToWorld.Multiply (facePoints);
            convexClipper.ConvexPolygonClip(facePoints, clippedPoints, work, 1);
            // points returned as "inside" the clipper are on the boundary of the clipper
            // and also are "inside" the range.
            // Hence an adjacent point "just outside" the clipper plane must be
            // "inside" the range, and the range is MIXED !!!!
            if (clippedPoints.size() > 0)
                return InOutStates::DefiniteMixed ();
            }
        }
    return InOutStates::DefiniteAllIn();
    }

bool TransformedDRange3d::IsAnyPointInsideClipper(ClipPlaneSetCR clipper) const
    {
    double tolerance = DoubleOps::SmallMetricDistance();
    // Successively test corners, edges, and faces of the range.
    // At each step, if definite in and definite out points have appeared exit immediately with 
    //  the mixed state.
    // After falling through that, each and every one of the convex clippers has avoided any face contact.
    // Hence single point tests for each clipper can imply state of the entire clipper.
    // (And any clipper that has no vertices must be all out -- it's unbounded, so if any point is in
    //    the unbounded planes would have to cross the range faces)
    // The face testing could subsume the vertex and edge stages.
    // But vertex is much faster than edge, and edge is much faster than face,
    //   so do them first hoping for quick exit.
    // Vertices ...
    for (uint32_t i = 0; i < 8; i++)
        {
        if (clipper.IsPointOnOrInside(m_worldCorners[i], tolerance))
            return true;
        }
    // edges ...
    bvector<DSegment1d> edgeIntervals;

    for (uint32_t axis = 0; axis < 3; axis++)
        {
        for (uint32_t edgeIndex = 0; edgeIndex < 4; edgeIndex++)
            {
            DSegment3d segment = BoxFaces::GetEdgeSegment(m_worldCorners, axis, edgeIndex);
            edgeIntervals.clear();
            clipper.AppendIntervals(segment, edgeIntervals);
            if (edgeIntervals.size () > 0)
                return true;
            }
        }

    // faces ... this triggers memory allocation to accomodate outside parts . . .
    ClipPlaneSetPolygonClipContext context(clipper, nullptr);
    BVectorCache<DPoint3d> inside;
    BVectorCache<DPoint3d> outside;
    bvector<DPoint3d> facePoints;
    for (int i = 0; i < 6; i++)
        {
        BoxFaces::Get5PointCCWFace(i, m_worldCorners, facePoints);
        outside.PopToCache();
        inside.PopToCache();
        context.ClipAndCollect(facePoints, clipper, inside, outside);
        if (inside.size() > 0)
            return true;
        }
    // ugh .. the faces are entirely in or entirely out.
    // because this union of convex sets can be very complicated, we cannot infer whether the clippers are inside.
    // But we can conclude that each convex clipper is entirely inside or entirely outside.
    // Look for any representative point on any clipper.
    // See if it is inside.
    for (auto &convexClipper : clipper)
        {
        auto worldXYZ = convexClipper.FindAnyVertex();
        if (worldXYZ.IsValid())
            {
            DPoint3d localXYZ = m_worldToLocal * worldXYZ.Value();
            if (m_localRange.IsContained(localXYZ))
                return true;
            }
        }
    return false;
    }

END_BENTLEY_GEOMETRY_NAMESPACE
