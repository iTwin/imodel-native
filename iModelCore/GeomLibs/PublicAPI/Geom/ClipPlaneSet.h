/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/Geom/ClipPlaneSet.h $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
/*__PUBLISH_SECTION_START__*/

BEGIN_BENTLEY_GEOMETRY_NAMESPACE

typedef bvector <ClipPlane>     T_ClipPlanes;
typedef bvector <ClipPlaneSet>    T_ClipPlaneSets;


enum    ClipPlaneContainment
    {
    ClipPlaneContainment_StronglyInside   =  1,
    ClipPlaneContainment_Ambiguous        =  2,
    ClipPlaneContainment_StronglyOutside  =  3,
    };

// An array of arrays of T, with the point arrays to be used like a stack.
// When the stack is popped, the top array is swapped to a holding area instead of 
// going to destructor, and then reused on later push
template<typename T>
struct BVectorCache : bvector<bvector<T>>
{
bvector<bvector<T>> m_cache;
// pope the top bvector<T>, and save it in the cache.
// return false if nothing to pop
bool PopToCache ()
    {
    if (this->size () > 0)
        {
        m_cache.push_back (bvector<T>  ());
        this->back ().swap (m_cache.back ());     // This preserves the heap allocation
        m_cache.back ().clear ();    
        this->pop_back (); // This is an empty bvector, so destructor does not free anything.    
        return true;
        }
    return false;
    }
// push an (empty) bvector.
// (If possible, it is taken from the cache, and will have pre-allocated heap memory)
void PushFromCache ()
    {
    this->push_back (bvector<T>());
    if (m_cache.size () > 0)
        {
        this->back ().swap (m_cache.back ());     // This preserves the heap allocation
        this->back ().clear();
        }
    }

// push 
// push data, using sequence that reuses capacity if possible
void PushCopy (bvector<T> const &data)
    {
    PushFromCache ();
    this->back() = data;
    }
// clear all bvectors to the cache. (preserving heap allocations)
void ClearToCache ()
    {
    while (PopToCache ())
        {
        }
    }


bool SwapBackPop (bvector<T> &data)
    {
    data.clear ();
    if (!this->empty ())
        {
        data.swap (this->back());
        PopToCache ();
        return true;
        }
    return false;
    }
void MoveAllFrom (BVectorCache<T> &other)
    {
    while (other.size () > 0)
        {
        this->push_back (bvector<T> ());
        this->back ().swap (other.back ());
        other.pop_back ();
        }
    }
};
/*=================================================================================**//**
//! A ConvexClipPlaneSet is an array of planes oriented so the intersection of their inside halfspaces is a convex volume.
//! @bsiclass  
+===============+===============+===============+===============+===============+======*/
struct  ConvexClipPlaneSet : T_ClipPlanes
    {
    //! Create emplty convex plane set.
    ConvexClipPlaneSet () {}

    //! Create convex plane set of specified size.
    ConvexClipPlaneSet (size_t n) : T_ClipPlanes (n) { }

    //! Create convex plane set from planes
    GEOMDLLIMPEXP  ConvexClipPlaneSet (ClipPlaneCP planes, size_t nPlanes);

    //! Test if point is inside all planes.
    GEOMDLLIMPEXP bool IsPointInside (DPoint3dCR point) const;

    //! Test if point is inside all planes to supplied tolerance.
    GEOMDLLIMPEXP bool IsPointOnOrInside (DPoint3dCR point, double tolerance) const;

    //! Test if sphere is inside all planes.
    GEOMDLLIMPEXP bool IsSphereInside (DPoint3dCR point, double tolerance) const;

    //! Clip a (bounded) line segment to this set of planes.
    GEOMDLLIMPEXP bool ClipBoundedSegment (DPoint3dCR pointA, DPoint3dCR pointB, double &fraction0, double &fraction1, double planeSign = 1.0) const;

    //! Clip an unbounded line to this set of planes.
    GEOMDLLIMPEXP bool ClipUnBoundedSegment (DPoint3dCR pointA, DPoint3dCR pointB, double &fraction0, double &fraction1, double planeSign = 1.0) const;

    //! Create clip plane set from XY box.
    GEOMDLLIMPEXP static ConvexClipPlaneSet  FromXYBox (double x0, double y0, double x1, double y1);

    //! Create clip plane set for regiosn to one side of a polyline.
    //! If hiddenEdge is an empty array, all clips are marked as regular clippers
    GEOMDLLIMPEXP static ConvexClipPlaneSet  FromXYPolyLine
    (
    bvector<DPoint3d> const &points,
    bvector<bool> const &hiddenEdge,
    bool leftIsInside
    );

    //! Add space "to the left of a polyline", with left determined by edges and an upvector.
    //!<ul>
    //!<li>If teh polyline is closed, an orientation test is done and the point order is reveresed if needed to be sure
    //!     the planes are oriented to enclose the space.
    //!<li>For each edge of the polyline, the primary plane has its primary outward normal as edge vector cross up vector.
    //!<li>The normal is then rotated by the tilt angle towards the up vector.
    //!<li>to close around a polygon, repeat the first point at end.
    //!<li>When closed, this correspondes to a CCW polygon with the upVector pointing at the eye, and the volume expands behind the polygon
    //!<li>
    //!</ul>
    GEOMDLLIMPEXP void AddSweptPolyline (
        bvector<DPoint3d> const &points,  //!< [in] polyline points
        DVec3d upVector,          //!< [in] upward vector (e.g. towards eye at infinity)
        Angle  tiltAngle            //!< [in] angle for tilt of planes.
        );
    //! Add the plane if it is valid.
    GEOMDLLIMPEXP bool Add (ValidatedClipPlane const &plane);

    //! @description return a summary classification
    //! <ul>
    //! <li>ClipPlaneContainment_StronglyInside All points are inside the plane set
    //! <li>ClipPlaneContainment_Ambiguous 
    //! <li>ClipPlaneContainment_StronglyOut Strongly out. All points are out, and
    //!     are all on one side of one of planes, so
    //!     edges joining pairs of points will always be out also.
    //! </ul>
    GEOMDLLIMPEXP ClipPlaneContainment ClassifyPointContainment (DPoint3dCP points, size_t nPoints, bool onIsOutside = false) const;

    //! Clip points.
    GEOMDLLIMPEXP void ClipPointsOnOrInside
        (
        bvector<DPoint3d> const &points,   //!< [in] input points
        bvector<DPoint3d> *inOrOn,      //!< [out] points that are in or outside.
        bvector<DPoint3d> *out      //!< [out] points that are outside.
        ) const;

    //!  Transform plane set.
    GEOMDLLIMPEXP void TransformInPlace (TransformCR transform);

   //! multiply plane coefficients times a DMatrix4d (such as the worldToNPC matrix)
   //! multiply [ax,ay,az,aw] * matrix
    GEOMDLLIMPEXP void MultiplyPlanesTimesMatrix(DMatrix4dCR matrix);

    //! Get range of this ConvexClipPlaneSet.
    GEOMDLLIMPEXP bool      GetRange (DRange3dR range, TransformCP transform) const;

    //! reinitialize to clip to a swept polygon.
    //!<ul>
    //!<li> 1 -- success, and the sweep vector and polygon area normal have positive dot product
    //!<li> -1 -- success, and the sweep vector and polygon area normal have negative dot product
    //!</ul> 0 -- failure - polygon normal is perpendicular to sweep.
    GEOMDLLIMPEXP int ReloadSweptConvexPolygon
        (
        bvector<DPoint3d> const &points, //!< [in] polygon points
        DVec3dCR sweepDirection,        //!< [in] direction for sweep
        int sideSelect                  //!< [in] 0 to just clip at polygon sides.   Positive to keep only above the polygon plane, negative to keep only below.
        );

    //! Return the (polygon) of intersection
    GEOMDLLIMPEXP void ConvexPolygonClip
        (
        bvector<DPoint3d> const &input, //!< [in] points of a convex polygon
        bvector<DPoint3d> &output,      //!< [out] clipped polygon
        bvector<DPoint3d> &work         //!< [inout] extra polygon
        ) const;

    //! Enumerate the "in" intervals .. the array is NOT cleared
    //! If the intervals array is nullptr, returns true immediately when any interior interval is found.
    GEOMDLLIMPEXP bool AppendIntervals(DEllipse3dCR arc, bvector<DSegment1d> *intervals, double planeSign = 1.0) const;
    GEOMDLLIMPEXP bool AppendIntervals(MSBsplineCurveCR curve, bvector<DSegment1d> *intervals) const;
    //! Return the (0 or 1) intersection polygon and (0 or to numClipPlane) outside pieces.
    GEOMDLLIMPEXP void ConvexPolygonClipInsideOutside
    (
    bvector<DPoint3d> const &input, //!< [in] points of a convex polygon
    bvector<DPoint3d> &inside,      //!< [out] clipped polygon (inside the convex set)
    BVectorCache<DPoint3d> &outside,      //!< [out] clipped polygons (outside the convex set)
    bvector<DPoint3d> &work1,       //!< [in,out] work vector for efficient reuse over multiple calls
    bvector<DPoint3d> &work2,       //!< [in,out] work vector for efficient reuse over multiple calls
    bool clearOutside = true,        //!< [in] true to clear the outside data.
    double distanceTolerance = 0.0  //!< [in] if nonzero, polygons within this distance of interior planes are classified as entirely "in"
    ) const;

    //! Compute crossings of this set's planes with curve primitives within a CurveVector.
    //! @param [in] curves candidate curves
    //! @param [out] crossings detailed crossing data.
    GEOMDLLIMPEXP void AppendCrossings (CurveVectorCR curves, bvector<CurveLocationDetailPair> &crossings) const;

    //! Compute crossings of this set's planes a curve primitive
    //! @param [in] curve candidate curve
    //! @param [out] crossings detailed crossing data.
    GEOMDLLIMPEXP void AppendCrossings (ICurvePrimitiveCR curve, bvector<CurveLocationDetailPair> &crossings) const;
}; // ConvexClipPlaneSet


/*=================================================================================**//**
//! A ClipPlaneSet is an array of ConvexClipPlaneSet representing the union of all of these sets.
//! @bsiclass  
+===============+===============+===============+===============+===============+======*/
struct  ClipPlaneSet :  bvector <ConvexClipPlaneSet>
    {
    //! Create empty clip plane set
    ClipPlaneSet () { }

    //! Create clip plane set from a single set of convex planes.
    GEOMDLLIMPEXP ClipPlaneSet(ClipPlaneCP planes, size_t nPlanes);

    //! Create clip plane set from a given convex clip plane set
    GEOMDLLIMPEXP ClipPlaneSet (ConvexClipPlaneSetCR convexSet);

    //! Create a (chain of) convex clippers for an (unbounded) polygon sweep in given direction.
    //! polygon may have disconnects.
    //! default sweep direction is Z
    GEOMDLLIMPEXP static ClipPlaneSet FromSweptPolygon (DPoint3dCP points, size_t n, DVec3dCP direction = NULL);
    GEOMDLLIMPEXP static ClipPlaneSet FromSweptPolygon (DPoint3dCP points, size_t n, DVec3dCP direction,
                bvector<bvector<DPoint3d>> *shapes
                );

    //! Test if point is inside.
    GEOMDLLIMPEXP bool IsPointInside (DPoint3dCR point) const;

    //! Test if point is inside to supplied tolerance.
    GEOMDLLIMPEXP bool IsPointOnOrInside (DPoint3dCR point, double tolerance) const;

    //! Test if point is inside to supplied tolerance.
    GEOMDLLIMPEXP bool IsSphereInside (DPoint3dCR point, double radius) const;

    //!  Transform each plane set.
    GEOMDLLIMPEXP void TransformInPlace (TransformCR transform);
   //! multiply plane coefficients times a DMatrix4d (such as the worldToNPC matrix)
   //! multiply [ax,ay,az,aw] * matrix
    GEOMDLLIMPEXP void MultiplyPlanesTimesMatrix(DMatrix4dCR matrix);

    //! Create clip plane set from XY box.
    GEOMDLLIMPEXP static ClipPlaneSet  FromXYBox (double x0, double y0, double x1, double y1);

    //! Test for intersection with ray.
    GEOMDLLIMPEXP bool    TestRayIntersect (DPoint3dCR point, DVec3dCR direction) const;

    //! Get nearest intersect distance along ray.
    GEOMDLLIMPEXP bool    GetRayIntersection (double& tNear, DPoint3dCR point, DVec3dCR direction) const;

    //! @description return a summary classification
    //! <ul>
    //! <li>ClipPlaneContainment_StronglyInside All points are inside the plane set
    //! <li>ClipPlaneContainment_Ambiguous 
    //! <li>ClipPlaneContainment_StronglyOut Strongly out. All points are out, and
    //!     are all on one side of one of planes, so
    //!     edges joining pairs of points will always be out also.
    //! </ul>
    GEOMDLLIMPEXP ClipPlaneContainment ClassifyPointContainment (DPoint3dCP points, size_t nPoints, bool onIsOutside = false) const;
    
    //! Get range of this ClipPlaneSet.
    GEOMDLLIMPEXP bool      GetRange (DRange3dR range, TransformCP transform) const;

    //! Clip a bounded line segment
    GEOMDLLIMPEXP bool IsAnyPointInOrOn (DSegment3dCR segment) const;
    //! Enumerate the "in" intervals .. the array is NOT cleared
    GEOMDLLIMPEXP void AppendIntervals (DSegment3dCR segment, bvector<DSegment1d> &intervals) const;

    //! Test a bounded arc
    GEOMDLLIMPEXP bool IsAnyPointInOrOn(DEllipse3dCR arc) const;
    //! Enumerate the "in" intervals .. the array is NOT cleared
    GEOMDLLIMPEXP void AppendIntervals(DEllipse3dCR arc, bvector<DSegment1d> &intervals) const;
    GEOMDLLIMPEXP bool IsAnyPointInOrOn(MSBsplineCurveCR curve) const;
    GEOMDLLIMPEXP void AppendIntervals(MSBsplineCurveCR curve, bvector<DSegment1d> &intervals) const;

    //! Compute crossings of this ClipPlaneSet with curve primitives within a CurveVector.
    //! @param [in] curves candidate curves
    //! @param [out] crossings detailed crossing data.
    GEOMDLLIMPEXP void AppendCrossings (CurveVectorCR curves, bvector<CurveLocationDetailPair> &crossings) const;

    //! Compute crossings of this ClipPlaneSet a curve primitive
    //! @param [in] curve candidate curve
    //! @param [out] crossings detailed crossing data.
    GEOMDLLIMPEXP void AppendCrossings (ICurvePrimitiveCR curve, bvector<CurveLocationDetailPair> &crossings) const;

    //! Determine if a CurveVector is completely in, completely out, or mixed with respect
    //! to a postive ClipPlaneSet and a mask (hole) ClipPlaneSet.
    //! @param curves [in] curves or region to clip.
    //! @param clipSet [in] the positive clip set
    //! @param maskSet [in] the negative (holes) clip set
    //! @param considerRegions [in] if true, treat Outer, Inner, Parity, and Union regions as areas.
    //!        If false, only consider boundaries.
    GEOMDLLIMPEXP static ClipPlaneContainment ClassifyCurveVectorInSetDifference
    (
    CurveVectorCR curves,
    ClipPlaneSetCR clipSet,
    ClipPlaneSetCP maskSet,
    bool considerRegions
    );

    //! Determine if a CurveVector is completely in, completely out, or mixed with respect
    //! to a postive ClipPlaneSet and a mask (hole) ClipPlaneSet.
    //! @param curve [in] curve to test
    //! @param clipSet [in] the positive clip set
    //! @param maskSet [in] the negative (holes) clip set
    GEOMDLLIMPEXP static ClipPlaneContainment ClassifyCurvePrimitiveInSetDifference
    (
    ICurvePrimitiveCR curve,
    ClipPlaneSetCR clipSet,
    ClipPlaneSetCP maskSet
    );

    //! Determine if a Polyface is completely in, completely out, or mixed with respect
    //! to a postive ClipPlaneSet and a mask (hole) ClipPlaneSet.
    //! @param polyface [in] polyface to test
    //! @param clipSet [in] the positive clip set
    //! @param maskSet [in] the negative (holes) clip set
    GEOMDLLIMPEXP static ClipPlaneContainment ClassifyPolyfaceInSetDifference
    (
    PolyfaceQueryCR polyface,
    ClipPlaneSetCR clipSet,
    ClipPlaneSetCP maskSet
    );

    //! Clip a polyface to a a postive ClipPlaneSet and a mask (hole) ClipPlaneSet.
    //! @param polyface [in] polyface to test
    //! @param clipSet [in] the positive clip set
    //! @param maskSet [in] the negative (holes) clip set
    //! @param inside [out] (optional) "inside" parts
    //! @param outside [out] (optional) "outside" parts
    GEOMDLLIMPEXP void static ClipToSetDifference
    (
    PolyfaceQueryCR polyface,
    ClipPlaneSetCR clipSet,
    ClipPlaneSetCP maskSet,
    PolyfaceHeaderPtr *inside,
    PolyfaceHeaderPtr *outside
    );
};

END_BENTLEY_GEOMETRY_NAMESPACE
