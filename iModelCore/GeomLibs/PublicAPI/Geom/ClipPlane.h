/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once
/*__PUBLISH_SECTION_START__*/

BEGIN_BENTLEY_GEOMETRY_NAMESPACE

//!
//! @description A single plane described by normal and distance from origin.
//! Space point x,y,z is in the outer half space, plane, or inner half space the plane according to the sign of
//!    {x * normal.x + y * normal.y + z * normal.z - distance}
//! (i.e. the normal is INWARD)
//! (In customary use where the normal vector is unit length, the distance is true spatial distance from the 
//! origin to the closest point on the plane.
//!
struct ClipPlane
    {
#ifdef BENTLEYGEOMETRY_ClipPlaneCoordinatesPublic
public:
#else
private:
#endif
    
    //! outward normal to plane
    DVec3d          m_normal;
    //! distance from origin to (closest) point on plane.
    double          m_distance;
private:
    uint32_t        m_flags;
    
    enum 
        {
        PlaneMask_Interior      = 0x0001 << 0,
        PlaneMask_Invisible     = 0x0001 << 1,
        };

    
public:
    //! Default constructor -- z vector, everything else zero.
    GEOMDLLIMPEXP ClipPlane ();

    //! Construct plane with given normal and distance from origin.
    GEOMDLLIMPEXP ClipPlane (DVec3dCR normal, double distance, bool invisible = false, bool interior = false);

    //! Construct plane with given normal and point on plane.
    GEOMDLLIMPEXP ClipPlane (DVec3dCR normal, DPoint3dCR point, bool invisible = false, bool interior = false);

    //! Construct plane from DPlane3d.
    GEOMDLLIMPEXP ClipPlane (DPlane3dCR plane, bool invisible = false, bool interior = false);

    //!
    //!<ul>
    //!<li>For non-tilted case, (tiltAngle = 0),  the inward normal vector is upVector cross edge vector.
    //!<li>If tilt is nonzero, that plane is rotated around the (forward) edge vector.
    //!</ul>
    GEOMDLLIMPEXP static ValidatedClipPlane FromEdgeAndUpVector
        (
        DPoint3dCR point0,  //!< [in] start point of edge
        DPoint3dCR point1,  //!< [in] end point of edge
        DVec3dCR upVector,  //!< [in] vector towards eye.  0-tilt plane normal is the edge vector
        Angle tiltAngle     //!< [in] angle to tilt plane
        );
    //! Create a clip plane perpendicular to upVvector, positioned a distance forward or backward of given points.
    //!<ul>
    //!<li>
    GEOMDLLIMPEXP static ValidatedClipPlane FromPointsAndDistanceAlongPlaneNormal
        (
        bvector<DPoint3d> const &points,    //!< [in] polyline points
        DVec3d upVector,                    //!< [in] upward vector (e.g. towards eye at infinity)
        double  distance,                    //!< [in] distance to offset
        bool    pointsInside                //!< [in] true to orient so the points are inside.
        );

    //! Return the interior flag.
    GEOMDLLIMPEXP bool   GetIsInterior () const;

    //! Return the interior flag.
    GEOMDLLIMPEXP bool   GetIsInvisible () const;

    //! Return whether cut for this flag should be displayed (!invisible && !isInterior);
    GEOMDLLIMPEXP bool   IsVisible () const;

    //! Get invisible.
    GEOMDLLIMPEXP void SetInvisible (bool invisible);

    //! Get flags.
    GEOMDLLIMPEXP uint32_t GetFlags () const;

    //! Set flags.
    GEOMDLLIMPEXP void   SetFlags(uint32_t flags);

    //! Set flags.
    GEOMDLLIMPEXP void   SetFlags (bool invisible, bool interior);

    //! Return the plane distance.
    GEOMDLLIMPEXP double  GetDistance() const;

    //! Return the plane normal.
    GEOMDLLIMPEXP DVec3dCR GetNormal () const;

    //! Evaluate the plane equation at {point}.
    GEOMDLLIMPEXP double EvaluatePoint (DPoint3dCR point) const;

    //! Evaluate Dot Product with plane normal.
    GEOMDLLIMPEXP double DotProduct (DVec3dCR normal) const;

    //! Evaluate Dot Product with plane normal.
    GEOMDLLIMPEXP double DotProduct (DPoint3dCR point) const;

    //! Return if the point is on or inside plane.
    GEOMDLLIMPEXP bool IsPointOnOrInside (DPoint3dCR point) const;

    //! Return if the point is on or inside plane to tolerance.
    GEOMDLLIMPEXP bool IsPointOnOrInside (DPoint3dCR point, double tolerance) const;

    //! Return if the point is on plane within tolerance
    GEOMDLLIMPEXP bool IsPointOn (DPoint3dCR point, double tolerance) const;
    
    //! Return the plane as origin and normal.
    GEOMDLLIMPEXP DPlane3d GetDPlane3d () const;

    //! Return the plane as a transform with origin on plane, z vector perpendicular.
    GEOMDLLIMPEXP Transform GetLocalToWorldTransform (bool zPointsOut = false) const;

    //! Return the plane for use as h = [ax,ay,az,aw] DOT [x,y,z,1]
    GEOMDLLIMPEXP DPoint4d GetDPlane4d () const;
    GEOMDLLIMPEXP void     SetDPlane4d (DPoint4dCR plane);

    //! Return true if the through {pointA} and {pointB} crosses the plane at a fractional coordinate between 0 and 1.
    //! Note that if both points are ON the plane the return value is false -- the "on" case is not a simple intersection.
    GEOMDLLIMPEXP bool BoundedSegmentHasSimpleIntersection (DPoint3dCR pointA, DPoint3dCR pointB, double &fraction) const;

    //!  Transform plane.
    GEOMDLLIMPEXP void TransformInPlace (TransformCR transform);

        //! multiply plane coefficients times a DMatrix4d (such as the worldToNPC matrix)
        //! multiply [ax,ay,az,aw] * matrix
    GEOMDLLIMPEXP void MultiplyPlaneTimesMatrix(DMatrix4dCR matrix);

    //! GetRange.
    GEOMDLLIMPEXP bool      GetRange (DRange3dR range, TransformCP transform) const;

    //! Flip the normal direction.
    GEOMDLLIMPEXP void      Negate ();

    //! Clip a convex polygon.
    //! caller supplies work and altitude arrays as works space.
    //! result is written inplace to xyz.
    //! <ul>
    //! <li>onPlaneHandling=0 means no special handling for all-oin.
    //! <li>onPlaneHandling=1 means treat all-on as IN
    //! <li>onPlaneHandling= -1 means treat all-on as OUT
    //! </ul>
    GEOMDLLIMPEXP void ConvexPolygonClipInPlace (bvector<DPoint3d> &xyz, bvector<DPoint3d> &work, int onPlaneHandling) const;

    //! Clip a convex polygon.
    //! caller supplies work and altitude arrays as works space.
    //! result is written inplace to xyz.
    GEOMDLLIMPEXP void ConvexPolygonClipInPlace (bvector<DPoint3d> &xyz, bvector<DPoint3d> &work) const;

    //! Return crossings of all edges of a polygon (including final closure)
    //! This uses simple zero tests -- does not try to filter double data at vertex-on-plane case
    GEOMDLLIMPEXP void PolygonCrossings (bvector<DPoint3d> const &xyz, bvector<DPoint3d> &crossings) const;

    //! fill an array with the 0, 1, or 2 intersections of an arc with the plane.
    //! @return the number of intersections.
    GEOMDLLIMPEXP int SimpleIntersectionFractions (DEllipse3dCR arc, double intersectionFractions[2], bool bounded) const;

    GEOMDLLIMPEXP void ConvexPolygonSplitInsideOutside
    (
    bvector<DPoint3d> const &xyz,         //!< [in] original polygon
    bvector<DPoint3d> &xyzIn,             //!< [out] inside part
    bvector<DPoint3d> &xyzOut,             //!< [out] outside part
    DRange1d &altitudeRange                 //!< [out] min and max altitude values.
    ) const;

    void AppendCrossings (CurveVectorCR curves, bvector<CurveLocationDetailPair> &crossings) const;

    void AppendCrossings (ICurvePrimitiveCR curve, bvector<CurveLocationDetailPair> &crossings) const;

    //! Apply offset to plane.
    GEOMDLLIMPEXP   void    OffsetDistance (double distance);

    //! Search an array of points for the first index at which the point is on both of two planes.
    GEOMDLLIMPEXP static ValidatedSize FindPointOnBothPlanes
        (
        bvector<DPoint3d> const &data,  //!< [in] points to search
        ClipPlaneCR plane0, //!< [in] first plane
        ClipPlaneCR plane1, //!< [in] second plane
        double tolerance    //!< [in] tolerance for "on" decision
        );
    // Return the counter of the number of clip plane evaluations that
    // have been done.
    // optionally clear the count.
    GEOMDLLIMPEXP static size_t GetEvaluationCount (bool clear = false);

    //! Return the (possibly empty) polygon of intersection between a DRange3d and an (unbounded) plane.
    GEOMDLLIMPEXP static void ClipPlaneToRange
        (
        DRange3dCR range,       //!< [in] range
        DPlane3dCR plane,       //!< [in] unbounded plane
        bvector<DPoint3d> &clippedPoints,    //!< [out] (convex) intersection polygon.
        bvector<DPoint3d> *largeRectangle = nullptr   //!< [out] (optional) large rectangle which is in the plane and contains the projection of the range onto the plane.  Must be distinct from clippedPoints
        );
    };



END_BENTLEY_GEOMETRY_NAMESPACE
