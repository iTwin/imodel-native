/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/Geom/dsegment3d.h $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
/*__PUBLISH_SECTION_START__*/

BEGIN_BENTLEY_GEOMETRY_NAMESPACE
/**
Start and end points of a line segment.
*/
struct GEOMDLLIMPEXP DSegment3d
{
//! Start and end points addressable as point[0] and point[1].
DPoint3d point[2];

#ifdef __cplusplus

//BEGIN_FROM_METHODS

//!
//! @description Return a segment defined by start point and extent vector.
//!
//!
static DSegment3d FromOriginAndDirection
(
DPoint3dCR      point0,
DPoint3dCR      tangent
);

//! @description Return a line segment with all coordinates zero.
static DSegment3d FromZero ();

//! @description Initialize with all coordinates zero.
void InitZero ();
//! @description initialize as transform of other segment.
void InitProduct (TransformCR transform, DSegment3dCR other);
//!
//! Initialize a segment from a ray.
//!
//!
static DSegment3d From (DRay3dCR ray);

//!
//! @description Returns a lines segment defined by its start and end coordinates.
//!
static DSegment3d From
(
double          x0,
double          y0,
double          z0,
double          x1,
double          y1,
double          z1
);

//!
//! @description Return a segment defined by fractional start and end on a parent segment.
//! @param [in] parent existing segment.
//! @param [in] startFraction fractional coordiante of new segment start on parent.
//! @param [in] endFraction fractional coordiante of new segment end on parent.
static DSegment3d FromFractionInterval
(
DSegment3dCR parent,
double startFraction,
double endFraction
);

//!
//! @description Return a segment defined by fractional start and end on a parent segment.
//! @param [in] parent existing segment.
//! @param [in] interval interval withs fractional start and end coordinates
static DSegment3d FromFractionInterval
(
DSegment3dCR parent,
DSegment1dCR interval
);

//!
//! Initialize a segment from endpoints.
//!
static DSegment3d From
(
DPoint3dCR      point0,
DPoint3dCR      point1
);

//! @description Return a segment defined by its endpoints.
static DSegment3d From (DPoint2dCR point0, DPoint2dCR point1);
//END_FROM_METHODS

/*__PUBLISH_SECTION_END__*/
//BEGIN_REFMETHODS
/*__PUBLISH_SECTION_START__*/
//!
//! @description Returns a lines segment defined by its start and end coordinates.
//!
void Init
(
double          x0,
double          y0,
double          z0,
double          x1,
double          y1,
double          z1
);

//!
//! Down-weight a DSegment4d back to DSegment3d.  Beware that this can fail
//! (if weights are zero), and changes parameterization when weights of the two
//! points are different.
//!
bool Init (DSegment4dCR source);

//!
//! Initialize a segment from endpoints.
//!
void Init
(
DPoint3dCR      point0,
DPoint3dCR      point1
);

//!
//! @description Return a segment defined by its endpoints.
//!
//!
void Init
(
DPoint2dCR      point0,
DPoint2dCR      point1
);

//!
//! @description Return a segment defined by start point and extent vector.
//!
//!
void InitFromOriginAndDirection
(
DPoint3dCR      point0,
DPoint3dCR      tangent
);

//!
//! Initialize a segment from a ray.
//!
//!
void InitFrom (DRay3dCR ray);

//!
//! Return the endpoints of the segment.
//!
//! @param [out] point0 start point
//! @param [out] point1 end point
//!
void GetEndPoints
(
DPoint3dR       point0,
DPoint3dR       point1
) const;

//!
//! @return squared length of the segment.
//!
//!
double LengthSquared () const;


//! @return true if AlmostEqual endpoints
bool IsAlmostEqual (DSegment3d other, double tolerance) const;

//! @return true if endpoint are identical (bitwise)
bool IsSinglePoint () const;

//! @return true if endpoint are AlmostEqual()
bool IsAlmostSinglePoint () const;

//!
//! @param [out] range range of segment.
//! @return always true
//!
bool GetRange (DRange3dR range) const;

//! @return range when projected to fraction space of the ray
//! @param [in] ray
DRange1d ProjectedParameterRange (DRay3dCR ray) const;

//!
//! Project a point onto the extended line in 3D.
//! @param [out] closestPoint point on extended line
//! @param [out] closestParam parameter for closest point
//! @param [in] point space point.
//!
bool ProjectPoint
(
DPoint3dR       closestPoint,
double          &closestParam,
DPoint3dCR      point
) const;

//!
//! Project a point onto the extended line using only xy parts.
//! @param [out] closestPoint point on extended line
//! @param [out] closestParam parameter for closest point
//! @param [in] point space point.
//!
bool ProjectPointXY
(
DPoint3dR       closestPoint,
double          &closestParam,
DPoint3dCR      point
) const;

//! Project a point onto the bounded line in 3D.  If nearest point of extended line
//! is outside the 0..1 parameter range, returned values are optinally restricted to nearest endpoint.
//! @param [out] closestPoint point on extended line
//! @param [out] closestParam parameter for closest point
//! @param [in] point space point.
//! @param [in] extend0 true to extend backwards before 0
//! @param [in] extend1 true to extend forwards from 1
//!
bool ProjectPointBounded
(
DPoint3dR       closestPoint,
double          &closestParam,
DPoint3dCR      point,
bool extend0,
bool extend1
) const;


//!
//! Project a point onto the bounded line in 3D.  If nearest point of extended line
//! is outside the 0..1 parameter range, returned values are for nearest endpoint.
//! @param [out] closestPoint point on extended line
//! @param [out] closestParam parameter for closest point
//! @param [in] point space point.
//!
bool ProjectPointBounded
(
DPoint3dR       closestPoint,
double          &closestParam,
DPoint3dCR      point
) const;

//!
//! Return the intersection of the (unbounded) segment with a plane.
//! @param [in] intPoint intersection point
//! @param [in] intParam parameter along the line
//! @param [in] plane plane (origin and normal)
//! @return false if line, plane are parallel.
//!
bool Intersect
(
DPoint3dR       intPoint,
double          &intParam,
DPlane3dCR      plane
) const;



//!
//! Return the intersection of the (unbounded) segment with a circle, using only
//! xy coordinates.
//! @param [out] intPoint 0, 1, or 2 intersection points.
//! @param [in] pIntParam parameter along the line
//! @param [in] center circle center.
//! @param [in] radius circle radius.
//! @return   number of intersections.
//!
int IntersectCircleXY
(
DPoint3dP       intPoint,
double          *pIntParam,
DPoint3dCR      center,
double          radius
) const;

//!
//! Get start point from the line segment.
//! @param [out] pt start point of object.
//!
void GetStartPoint (DPoint3dR pt) const;

//!
//! Get end point from the line segment.
//! @param [out] pt end point of object.
//!
void GetEndPoint (DPoint3dR pt) const;

//!
//! Set the "start" point for the line segment.
//! @param        point          new start point.
//!
void SetStartPoint (DPoint3dCR point);

//!
//! Set the "end" point for the line segment.
//! @param        point          new end point.
//!
void SetEndPoint (DPoint3dCR point);

//!
//! @param [out] point coordinates at fractional parameter.
//! @param [in] param fractional parameter
//! @remark This is a deprecated form of FractionToPoint.
bool FractionParameterToPoint
(
DPoint3dR       point,
double          param
) const;

//!
//! @return point coordinates at fractional parameter.
//! @param [in] fraction fractional parameter
//!
DPoint3d FractionToPoint (double fraction) const;

//!
//! @description Returns the parameter at which a point projects to the *unbounded)
//!   line containing the segment.  Parameters less than zero and greater than one
//!   mean the projection is outside the bounds of the line segment.
//! @param [out] param fraction where point projects to the line.
//! @param [in] point point to project to the line.
//!
bool PointToFractionParameter
(
double          &param,
DPoint3dCR      point
) const;

//!
//! @param [out] point point on line at fractional parameter.
//! @param [out] tangent tangent vector at fractional parameter.
//! @param [in] param fractional parameter.
//!
bool FractionParameterToTangent
(
DPoint3dR       point,
DVec3dR       tangent,
double          param
) const;

//! Return teh vector from start point to end point.
DVec3d VectorStartToEnd () const;
//!
//! @param [out] pt returned point.
//! @param [in] index index of point to return.
//!
bool GetPoint
(
DPoint3dR       pt,
int             index
) const;

//!
//! @param [out] pt returned point.
//! @param [in] index index of point to return.
//!
bool SetPoint
(
DPoint3dCR      pt,
int             index
);

//!
//! Computes equidistant points along the segment.
//!
//! @param [out] points array of computed points
//! @param [in] numPoints # points to compute.
//!
void InterpolateUniformArray
(
bvector<DPoint3d>&points,
size_t          numPoints
) const;

//!
//! @return line segment length.
//!
double Length () const;

//!
//! Compute the (signed) arc length between specified fractional parameters.
//! @param [out] arcLength computed arc length.  Negative if fraction1 < fraction0.
//! @param [in] fraction0 start fraction for interval to measure.
//! @param [in] fraction1 end fraction for interval to measure.
//! @return true if the arc length was computed.
//!
bool FractionToLength
(
double          &arcLength,
double          fraction0,
double          fraction1
) const;

//!
//! Compute the fraction parameter corresponding to a specified arc length away from
//!   a specified start fraction. (inverse of fractions to arcStep)
//! @param [out] fraction1 fraction at end of interval.
//! @param [in] fraction0 start fraction for interval to measure.
//! @param [in] arcStep arc length to move.  Negative arc length moves backwards.
//! @return true if the fractional step was computed.
//!
bool LengthToFraction
(
double          &fraction1,
double          fraction0,
double          arcStep
) const;

//! Find the closest point (projection or end), as viewed in xy plane, after applying optional transformation.
//! @param [out] closePoint closest point, in coordinates of the input segment.
//! @param [out] closeParam parameter at closest point
//! @param [out] distanceXY distance in transformed coordinates
//! @param [in] spacePoint world coordinates of test point.
//! @param [in] worldToLocal optional transformation.
bool ClosestPointBoundedXY
(
DPoint3dR   closePoint,
double&     closeParam,
double&     distanceXY,
DPoint3dCR  spacePoint,
DMatrix4dCP worldToLocal
) const;

//! Find the closest point (projection or end), as viewed in xy plane, after applying optional transformation.
//! @param [out] closePoint closest point, in coordinates of the input segment.
//! @param [out] closeParam parameter at closest point
//! @param [out] distanceXY distance in transformed coordinates
//! @param [in] spacePoint world coordinates of test point.
//! @param [in] worldToLocal optional transformation.
//! @param [in] extend0 true to allow the line to extend before start
//! @param [in] extend1 true to allow the line to extend after end
bool ClosestPointBoundedXY
(
DPoint3dR   closePoint,
double&     closeParam,
double&     distanceXY,
DPoint3dCR  spacePoint,
DMatrix4dCP worldToLocal,
bool extend0,
bool extend1
) const;


//! Compute the length (unit density) and wire centroid.
//! Note that a zero length line has zero length but the (single) coordinate is a well defined centroid.
//! @param [out] length line length.
//! @param [out] centroid centroid point.
//! @param [in] fraction0 start fraction for interval to measure.
//! @param [in] fraction1 end fraction for interval to measure.
void WireCentroid
(
double     &length,
DPoint3dR   centroid,
double      fraction0 = 0.0,
double      fraction1 = 1.0
) const;

//!
//! Find the closest approach of two bounded segments
//!
//! @param [out] fraction0 parameter on first segment.
//! @param [out] fraction1 parameter on second segment.
//! @param [out] point0 point on first ray.
//! @param [out] point1 point on second ray.
//! @param [out] segment0 first segment
//! @param [out] segment1 second segment
//!
static
void   ClosestApproachBounded
(
double      &fraction0,
double      &fraction1,
DPoint3dR    point0,
DPoint3dR    point1,
DSegment3dCR segment0,
DSegment3dCR segment1
);

//!
//! Find the closest approach of two unbounded segments
//!
//! @param [out] fraction0 parameter on first segment.
//! @param [out] fraction1 parameter on second segment.
//! @param [out] point0 point on first ray.
//! @param [out] point1 point on second ray.
//! @param [out] segment0 first segment
//! @param [out] segment1 second segment
//! @return false if parallel segments
//!
static
bool ClosestApproachUnbounded
(
double      &fraction0,
double      &fraction1,
DPoint3dR    point0,
DPoint3dR    point1,
DSegment3dCR segment0,
DSegment3dCR segment1
);


//!
//! Find the closest approach of two bounded segments, considering only xy parts of distance
//!
//! @param [out] fraction0 parameter on first segment
//! @param [out] fraction1 parameter on second segment
//! @param [out] point0 point on first ray.
//! @param [out] point1 point on second ray.
//! @param [out] segment0 first segment
//! @param [out] segment1 second segment
//!
static
void   ClosestApproachBoundedXY
(
double      &fraction0,
double      &fraction1,
DPoint3dR    point0,
DPoint3dR    point1,
DSegment3dCR segment0,
DSegment3dCR segment1
);

//!
//! Return the (simple, single point) intersection of two (unbounded) segments as viewed in XY.
//! (z coordinates of the outputs pointA and pointB may differ)
//! @param [out] fractionA fractional position on segmentA
//! @param [out] fractionB fractional position on segmentB
//! @param [out] pointA point on segmentA
//! @param [out] pointB point on segmentB
//! @param [in] segmentA first line segment.
//! @param [in] segmentB second line segment.
//! @return false if segments are parallel.
//!
static bool IntersectXY
(
double          &fractionA,
double          &fractionB,
DPoint3dR       pointA,
DPoint3dR       pointB,
DSegment3dCR    segmentA,
DSegment3dCR    segmentB
);

//! Construct one of the 4 lines tangent to two circles as viewed in xy.
//! Radius signs select which tangencies are selected
static ValidatedDSegment3d  ConstructTangent_CircleCircleXY
(
DPoint3dCR centerA,     //!< [in] center of first circle
double     radiusA,     //!< [in] radius of first circle.  Positive selects tangency to left of the line from centerA to centerB, negative to right.
DPoint3dCR centerB,     //!< [in] center of second circle
double     radiusB      //!< [in] radius of second circle.  Positive selects tangency to left of the line from centerA to centerB, negative to right.
);

#endif
};

/*__PUBLISH_SECTION_END__*/
/**
Start and end points of a homogeneous line segment.
*/
struct GEOMDLLIMPEXP DSegment4d
{
//! Start and end points addressable as point[0] and point[1].
DPoint4d point[2];

void Init (DPoint4dCR pointA, DPoint4dCR pointB);
void Init (DPoint3dCR pointA, DPoint3dCR pointB);
void Init (double xA, double yA, double zA, double wA, double xB, double yB, double zB, double wB);
void Init (double xA, double yA, double zA, double xB, double yB, double zB);
void Init (DSegment3dCR segment);

bool ProjectDPoint4dCartesianXYW
(
DPoint4dR closestPoint,
double      &closestParam,
DPoint4dCR spacePoint
) const;

bool FractionParameterToPoint (DPoint4dR point, double fraction) const;

// New methods - 5/2012

bool GetEndPoints (DPoint3dR point0, DPoint3dR point1) const;
void InitProduct (TransformCR transform, DSegment4dCR source);
void InitProduct (DMatrix4dCR mat, DSegment4dCR source);
void GetStartPoint (DPoint3dR point) const;
void GetEndPoint (DPoint3dR pt) const;
DSegment4d FromFractionInterval (DSegment4d parent, double startFraction, double endFraction);
DPoint4d FractionParameterToPoint(double fraction) const;
bool FractionParameterToPoint(DPoint3d &pnt, double fraction) const;
bool PointToFractionParameter (double &param, DPoint3d pt) const;
bool FractionParameterToTangent (DPoint3d &point, DVec3dR tangent, double param) const;
DPoint4d FractionParameterToTangent  (DPoint4d spacepoint, DPoint4d &tangent, double param) const;
bool FractionToLength (double &arcLength, double fraction0, double fraction1) const;
bool LengthToFraction (double &fraction1, double fraction0, double arcStep) const;
bool ClosestPointBoundedXY (DPoint3d &closePoint, double &closeParam, double &distanceXY, DPoint3d spacePoint, DMatrix4dCP worldToLocal, bool extend0, bool extend1) const;
bool ClosestPointBoundedXY (DPoint3d &closePoint, double &closeParam, double &distanceXY, DPoint3d spacePoint, DMatrix4dCP worldToLocal) const;
bool ProjectPoint (DPoint3d &closestPoint, double &closestParam, DPoint3d spacePoint) const;
bool ProjectPointBounded (DPoint3dR closestPoint, double &closestParam, DPoint3d spacePoint) const;
// project spacePoint onto the unbounded line, measuring in projected x/w and y/w.
bool ProjectPointUnboundedCartesianXYW (DPoint4dR closestPoint, double &closeParam, DPoint4dCR spacePoint) const;
bool GetRange (DRange3dR range3d) const;

//bool GetRange (DRange4d range4d) const;

//! Return line segment from full xyzw data
//! @param [in] pointA full xyzw start point.
//! @param [in] pointB full xyzw end point
static DSegment4d From (DPoint4dCR pointA, DPoint4dCR pointB);

//! Return line segment from xyz data
//! @param [in] pointA xyz start point.
//! @param [in] pointB xyz end point.
static DSegment4d From (DPoint3dCR pointA, DPoint3dCR pointB);

//! Return line segment from full xyzw data
static DSegment4d From
(
double xA, //! [in] (weighted) start coordinate
double yA, //! [in] (weighted) start coordinate
double zA, //! [in] (weighted) start coordinate
double wA, //! [in] (weighted) start coordinate
double xB, //! [in] (weighted) end coordinate
double yB, //! [in] (weighted) end coordinate
double zB, //! [in] (weighted) end coordinate
double wB  //! [in] (weighted) end coordinate
);

//! Return a line segment from xyz data.
static DSegment4d From
(
double xA, //! [in] (weighted) start coordinate
double yA, //! [in] (weighted) start coordinate
double zA, //! [in] (weighted) start coordinate
double xB, //! [in] (weighted) end coordinate
double yB, //! [in] (weighted) end coordinate
double zB  //! [in] (weighted) end coordinate
);

//! Promote a 3d segment to 4d (with weight 1)
//! @param [in] segment source line.
static DSegment4d From (DSegment3dCR segment);

/*-----------------------------------------------------------------*//**
* Compute the parameters and points where the xy projections of two rays intersect.
*
* @param point01 <= intersection point on line 0.
* @param param01 <= parametric coordinate on segment 0
* @param point23 <= intesection point on line 1.
* @param param23 <= parametric coordinate on segment 1
* @param segment01 => first segment
* @param segment23 => second segment
* @return true when there is a single-point intersection within both segments.
* @return true unless lines are parallel
+---------------+---------------+---------------+---------------+------*/
static bool IntersectXY
(
DPoint4dR   point01,
double      &param01,
DPoint4dR   point23,
double      &param23,
DSegment4dCR segment01,
DSegment4dCR segment23
);

/*-----------------------------------------------------------------*//**
* Compute the parameters and points where the xy projections of two rays intersect.
* Only return points in 0..1 parameter range.  Output parameters are
* untouched (undefined) if no intersections occur in range.
*
* @param point01 <= intersection point on line 0.
* @param param01 <= parametric coordinate on segment 0
* @param point23 <= intesection point on line 1.
* @param param23 <= parametric coordinate on segment 1
* @param segment01 => first segment
* @param segment23 => second segment
* @return true when there is a single-point intersection within both segments.
+---------------+---------------+---------------+---------------+------*/
static bool IntersectXYBounded
(
DPoint4dR   point01,
double      &param01,
DPoint4dR   point23,
double      &param23,
DSegment4dCR segment01,
DSegment4dCR segment23
);


};
/*__PUBLISH_SECTION_START__*/
END_BENTLEY_GEOMETRY_NAMESPACE
