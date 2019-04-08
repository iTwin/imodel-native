/*--------------------------------------------------------------------------------------+
|
|     $Source: Utils/PublicApi/GeometryUtils.h $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

#define SPLINE_ORDER 3
#define EGRESS_CORRECTION 0.3048
#define DEFAULT_MAX_EDGE_LENGTH 0.5
#define BUILDING_TOLERANCE 1.0e-8

BEGIN_BUILDING_SHARED_NAMESPACE

//GeometryUtils is a static class for doing various operations on geometry
class GeometryUtils
    {
public:    

    //! A copy of DVec3d::FromRotateVectorAroundVector.
    //! On bim0200dev calls the DVec3d::FromRotateVectorAroundVector and on older streams - uses the copied code.
    BUILDINGSHAREDUTILS_EXPORT static ValidatedDVec3d CreateVectorFromRotateVectorAroundVector(DVec3dCR vector, DVec3dCR axis, Angle angle);

    //! @param[in] curve The CurveVector to clone
    //! @param[in] transform The transform to apply to cloned curve
    //! @return a "deep copy" with transform applied
    BUILDINGSHAREDUTILS_EXPORT static CurveVectorPtr CloneTransformed(CurveVectorCR curve, TransformCR transform);

    BUILDINGSHAREDUTILS_EXPORT static bvector<DPoint3d>   ExtractSingleCurvePoints (CurveVectorCR curve);
    

    //! transforms a curvevector onto zero plane
    //! @param[in] planar curvevector
    //! @return profile transformed on zero plane
    BUILDINGSHAREDUTILS_EXPORT static CurveVectorPtr    GetProfileOnZeroPlane (CurveVectorCR profile);

    //! Forms points into a rectangle by a bounding point
    //! @params[out] point1, point2, point3, point4 points forming a rectangle
    //! @param[in] boundingPoint point in rectangle width line center that rectangle should adjust to
    //! @param[in] lengthVector weighted direction of rectangle length
    //! @param[in] widthVector weighted direction of rectangle width
    //! @param[in] ccw bool value showing if rectangle should be formed clockwise or counterclockwise from the bounding point
    BUILDINGSHAREDUTILS_EXPORT static void FormRectangle(DPoint3dR point1, DPoint3dR point2, DPoint3dR point3, DPoint3dR point4, DPoint3d boundingPoint, DVec3d lengthVector, DVec3d widthVector, bool ccw);
    
    //! Forms points into a rectangle by a center
    //! @params[out] point1, point2, point3, point4 points forming a rectangle
    //! @param[in] center rectangle center
    //! @param[in] lengthVecter weighted direction of rectangle length
    //! @param[in] widthVector weighted direction of rectangle width
    BUILDINGSHAREDUTILS_EXPORT static void FormRectangle(DPoint3dR point1, DPoint3dR point2, DPoint3dR point3, DPoint3dR point4, DPoint3d center, DVec3d lengthVector, DVec3d widthVector);
    
    //! Forms points into a rectangle by 2 bounding points
    //! @params[out] point1, point2, point3, point4 points forming a rectangle
    //! @param[in] boundingPoint1 point in rectangle width line center that rectangle should adjust to
    //! @param[in] boundingPoint2 point in rectangle width line center that rectangle should adjust to
    //! @param[in] widthVector weighter direction of rectangle width
    BUILDINGSHAREDUTILS_EXPORT static void FormRectangle(DPoint3dR point1, DPoint3dR point2, DPoint3dR point3, DPoint3dR point4, DPoint3d boundingPoint1, DPoint3d boundingPoint2, DVec3d widthVector);
    
    //! Forms rectangle points for rectangle to be on line. Formed rectangle does not go out of line bounds and is centered to it by width
    //! @param[out] pointsOut vector containing formed points
    //! @param[in] line line that rectangle should adjust to
    //! @param[in] rectangleCenter point where rectangle center should be. If rectangle goes out of line bounds with given center, the center is translated
    //! @param[in] rectangleLength rectangle length. If rectangleLength is higher than line that rectangle should adjust to, rectangleLength lowers to adjust
    //! @param[in] rectangleWidth rectangle width.
    BUILDINGSHAREDUTILS_EXPORT static void SetUpRectangleToAdjustToLine(bvector<DPoint3d>& pointsOut, DSegment3d line, DPoint3d rectangleCenter, double rectangleLength, double rectangleWidth);

    //! Checks if point is on given line
    //! @param[in] point point to check
    //! @param[in] line given line
    //! @return true if point is on given line
    BUILDINGSHAREDUTILS_EXPORT static bool IsPointOnLine(DPoint3d point, DSegment3d line);
    
    //! Calculates minimal translation that point needs to be multiplied of to be on given line
    //! @param[in] pointToFit point to translate on line
    //! @param[in] line given line
    //! @return translation to fit point to a line end point
    BUILDINGSHAREDUTILS_EXPORT static DVec3d FindTranslationToNearestEndPoint(DPoint3d pointToFit, DSegment3d line);

    //! Calculates transform (rotation) and translation for element to adjust to given line by given target point
    //! @param[out] translationOut calculated translation
    //! @param[in] toGlobal transform to global coordinates. Pass Identity transformation if given element direction is already in global coordinates
    //! @param[in] elementRay origin (center) point and direction of element
    //! @param[in] line line that element should be moved to
    //! @param[in] targetPoint point where new element origin should be
    //! @return new transform in global coordinates for element to be moved and rotated for direction to be parallel to line
    BUILDINGSHAREDUTILS_EXPORT static Transform GetTransformForMoveOnLine(DVec3dR translationOut, Transform toGlobal, DRay3d elementRay, DSegment3d line, DPoint3d targetPoint);
    
    //! Resizes given line to not exceed the bounding line points
    //! @param[in/out] lineOut resized line
    //! @param[in] boundingLine given bounding line
    //! @param[in] atEnd line point to change. false for start point, true for end point
    //! @param[in] target point where new resized line point should be. If bounding line point is closer to a line than the target point, resize is done only to the boundingLine end point.
    //! @return status if resize succeeded
    BUILDINGSHAREDUTILS_EXPORT static BentleyStatus ResizeLineToClosestPoint(DSegment3dR lineToResize, DSegment3d boundingLine, bool atEnd, DPoint3d target);

    //! Checks if two numbers are almost equal
    //! @param[in] a            first operand
    //! @param[in] b            second operand
    //! @param[in] tolerance    tolerance for numbers to be equal
    //! @return true if doubles are almost equal
    BUILDINGSHAREDUTILS_EXPORT static bool AlmostEqual(double a, double b, double tolerance = BUILDING_TOLERANCE);

    //! Finds the furthestPoint in a CurveVector. Supports curve vector composed of line strings and arcs
    //! @param[in] curveVector a curve vector to look for the furthest point in
    //! @param[in] point a point in/on curve vector
    //! @return a furthest point from given point that is on curveVector
    BUILDINGSHAREDUTILS_EXPORT static DPoint3d FindFurthestPoint(CurveVectorCR curveVector, DPoint3d point);

    //! Finds ellipse's tangent at given point
    //! @param[in] point a point on arc
    //! @param[in] ellipse
    //! @returns tangent of the ellipse
    BUILDINGSHAREDUTILS_EXPORT static DSegment3d FindEllipseTangent(DPoint3d point, DEllipse3d ellipse);

    //! Finds index of the closest point in vector
    //! @param[in] keyPoints    a vector of points
    //! @param[in] point        a point to find
    //! @return                 index of the closest point in given vector
    BUILDINGSHAREDUTILS_EXPORT static int FindClosestPointIndex(bvector<DPoint3d>& keyPoints, DPoint3d point);

    //! Finds angle in XY plane from rotation matrix
    //! @param[in] rotMatrix    rotation matrix around z axis
    //! @return                 rotation angle around z axis
    BUILDINGSHAREDUTILS_EXPORT static double RotMatrixToAngleXY(RotMatrix rotMatrix);

    //! Translates point by rotated vector
    //! @param[in/out]  point   point to translate
    //! @param[in]      axis    vector to rotate and add
    //! @param[in]      theta   angle to rotate vector by
    BUILDINGSHAREDUTILS_EXPORT static void AddRotatedVectorToPoint(DPoint3dR point, DVec3d axis, double theta);

    //! Creates an arc by center, start point, end point and rotation direction
    //! @param[in]  center  center point of arc
    //! @param[in]  start   start point of arc
    //! @param[in]  end     end point of arc
    //! @param[in]  ccw     direction of arc sweep. true for counter clock wise, false for clockwise
    BUILDINGSHAREDUTILS_EXPORT static DEllipse3d CreateDEllipse3dArc(DPoint3d center, DPoint3d start, DPoint3d end, bool ccw);

    //! Creates an arc by center, start point, end point and rotation direction
    //! @param[in]  center  center point of arc
    //! @param[in]  start   start point of arc
    //! @param[in]  end     end point of arc
    //! @param[in]  ccw     direction of arc sweep. true for counter clock wise, false for clockwise
    BUILDINGSHAREDUTILS_EXPORT static ICurvePrimitivePtr CreateArc(DPoint3d center, DPoint3d start, DPoint3d end, bool ccw);

    //! Creates extrusion detail for arc with center point (0,0,0)
    //! @param[in] radius   radius of arc from center
    //! @param[in] angle    arc angle
    //! @param[in] height   height of arc
    //! @return             formed arc extrusion detail
    BUILDINGSHAREDUTILS_EXPORT static DEllipse3d         CreateArc(double radius, double baseAngle, double extendLength = 0);

    //! Creates extrusion detail as a plane {(0,0,0), (length,0,0), (0,0,height)}
    //! @param[in] length   length of plane
    //! @param[in] height   height of plane
    //! @return             formed extrusion detail
    BUILDINGSHAREDUTILS_EXPORT static DgnExtrusionDetail CreatePlaneExtrusionDetail(double length, double height);

    //! Creates extrusion detail as a plane with base line {starPoint, endPoint} and given height
    //! @param[in] startPoint   starting point for plane
    //! @param[in] endPoint     ending point for plane
    //! @param[in] height       plane height
    //! @return                 formed extrusion detail
    BUILDINGSHAREDUTILS_EXPORT static DgnExtrusionDetail CreatePlaneExtrusionDetail(DPoint3d startPoint, DPoint3d endPoint, double height);

    //! Creates extrusion detail for arc with center point (0,0,0)
    //! @param[in] radius   radius of arc from center
    //! @param[in] angle    arc angle
    //! @param[in] height   height of arc
    //! @return             formed arc extrusion detail
    BUILDINGSHAREDUTILS_EXPORT static DgnExtrusionDetail CreateArcExtrusionDetail(double radius, double baseAngle, double height, double extendLength = 0);

    //! Creates curveprimitive for spline with given poles and order
    //! @param[in] poles    points for spline
    //! @param[in] order    order of spline
    //! @return             formed spline primitive
    BUILDINGSHAREDUTILS_EXPORT static ICurvePrimitivePtr CreateSplinePrimitive(bvector<DPoint3d> poles, int order = SPLINE_ORDER);

    //! Creates extrusion detail for spline with given poles and height
    //! @param[in] poles    points for spline
    //! @param[in] height   height of spline
    //! @param[in] order    order of spline
    //! @return             formed spline extrusion detail
    BUILDINGSHAREDUTILS_EXPORT static DgnExtrusionDetail CreateSplineExtrusionDetail(bvector<DPoint3d> poles, double height, int order = SPLINE_ORDER);

    //! Checks if a line intersects a curve vector
    //! @param[out] intersections       segments where line and curve vector intersects
    //! @param[in]  line                line by two points
    //! @param[in]  curveVector         curveVector. Supports curve vector composed of line strings and arcs
    //! @return                         true if line intersects curve vector
    BUILDINGSHAREDUTILS_EXPORT static bool CheckIfLineIntersectsCurveVector(bvector<DSegment3d>& intersections, DSegment3d line, CurveVectorCR curveVector);

    //! Checks if two lines intersect
    //! @param[out] intersectionSegment segment where lines intersect. 
    //!                                 If lines are not parallel it will be a single point segment, otherwise it will be a segment where both lines are intersecting.
    //!                                 For instance: in case of line1: {(0.0, 0.0), (1.0, 1.0)} and line2: {(0.0, 1.0), (1.0, 0.0)} the intersecting line will be {(0.5, 0.5), (0.5, 0.5)}
    //!                                           and in case of line1: {(0.0, 0,0), (5.0, 0.0)} and line2: {(3.0, 0.0), (8.0, 0.0)} the intersecting line will be {(3.0, 0.0), (5.0, 0.0)}
    //! @param[in]  line1               first line to check
    //! @param[in]  line2               second line to check
    //! @return                         true if lines intersect
    BUILDINGSHAREDUTILS_EXPORT static bool CheckIfTwoLinesIntersect(DSegment3dR intersectionSegment, DSegment3d line1, DSegment3d line2);
    
    //! Checks if two parallel line segments coincide
    //! @param[out] intersectioSegment  segment where lines intersect.
    //!                                 If any of lines are single point, it will be a single point segment, otherwise it will be a segment where both lines are intersecting.
    //!                                 Otherwise if lines coincide intersection segment will be an intersection of both lines ranges
    //! @param[in]  line1               first line to check
    //! @param[in]  line2               second line to check
    //! @return                         true if line segments coincide
    BUILDINGSHAREDUTILS_EXPORT static bool CheckIfParallelLineSegmentsCoincide(DSegment3dR intersectionSegment, DSegment3d line1, DSegment3d line2);
    
    //! Returns a point at t considering that [x, y, z] = v + d * t
    //! @param[in] v    v vector of parametric form
    //! @param[in] d    d vector of parametric form
    //! @param[in] t    t value
    //! @return         point at given t
    BUILDINGSHAREDUTILS_EXPORT static DPoint3d GetPointFromParamtericForm(DVec3d v, DVec3d d, double t);

    //! Returns determinant:
    //! | a11 a12 |
    //! | a21 a22 |
    //! @param[in] a11, a12, a21, a22   params in matrix
    //! @return determinant
    BUILDINGSHAREDUTILS_EXPORT static double GetDeterminant2x2(double a11, double a12, double a21, double a22);

    //! Converts line segment to a line in parametric form: [x, y, z] = v + d * t
    //! @param[out] v       v vector from the parametric form
    //! @param[out] d       d vector from the parametric form
    //! @param[in]  line    line to convert;
    BUILDINGSHAREDUTILS_EXPORT static void ToParametricForm(DVec3dR v, DVec3dR d, DSegment3d line);

    //! Checks if point is contained in range with given tolerance
    //! @param[in]  range       range to check point in
    //! @param[in]  point       point to check
    //! @param[in]  tolerance   compare tolerance
    BUILDINGSHAREDUTILS_EXPORT static bool IsPointContainedInRangeToTolerance(DRange3d range, DPoint3d point, double tolerance);

    //! Finds shortest path in given curve vector area as a linestring.
    //! @param[out]     pathLineString  shortest path as point sequence in given curve vector area
    //! @param[in]      curveVector     curve vector to find shortest path in. Supports only line strings as curve vector children
    //! @param[in]      source          starting point of path
    //! @param[in]      destination     end point of path
    //! @return         BentleyStatus::SUCCESS if path can be found in given curve vector
    BUILDINGSHAREDUTILS_EXPORT static BentleyStatus FindShortestPathBetweenPointsInCurveVector(bvector<DPoint3d>& pathLineString, CurveVectorCR curveVector, DPoint3d source, DPoint3d destination);

    //! Offsets children curves of curve vector by given amount and remerges the curves if needed
    //! @param[in]      originalCurve   curve to offset
    //! @param[in]      innerOffset     offset for inner children
    //! @param[in]      outerOffset     offset for outer children
    //! @param[in]      mergeIntoDifference     true if new children should be merged
    //! @return         offseted curve vector
    BUILDINGSHAREDUTILS_EXPORT static CurveVectorPtr OffsetCurveInnerOuterChildren(CurveVectorCR originalCurve, double innerOffset, double outerOffset, bool mergeIntoDifference = false);

    //! Checks if given segment is fully inside (or on) polygon area considering both inner and outer children
    //! @param[in]      area    polygon area to check. Supports only line strings ar curve vector children
    //! @param[in]      line    line to check
    //! @return         false the there exists a point out of outer polygon area or in inner polygon area
    BUILDINGSHAREDUTILS_EXPORT static bool CheckIfLineIsContainedInPolygonArea(CurveVectorCR area, DSegment3d line);

    //! Separates curve vector to inner and outer curves
    //! @param[out]     innerCurves     inner curve vector children
    //! @param[out]     outerCurves     outer curve vector children
    //! @param[in]      source          curve vector to separate
    BUILDINGSHAREDUTILS_EXPORT static void ExtractInnerOuterCurves(bvector<CurveVectorPtr>& innerCurves, bvector<CurveVectorPtr>& outerCurves, CurveVectorCR source);

    //! Returns line string inside curve vector's first curve primitive if possible
    //! @param[in]  curveVector     curve vector to extract line string from
    //! @return     line string if it exists, else an empty vector of points.
    BUILDINGSHAREDUTILS_EXPORT static bvector<DPoint3d> ExtractLineString(CurveVectorCR curveVector);

    //! Returns distance between line and a point. 
    //! Note:   If point can be projected directly on the line segment, distance is calculated from that projected point.
    //          Otherwise distance is calculated from closer end point of segment
    //! @param[in]  line    line to calculate distance from
    //! @param[in]  point   point to calculate distance to
    //! @return     distance between line and point
    BUILDINGSHAREDUTILS_EXPORT static double DistanceFromPointToLine(DSegment3d line, DPoint3d point);

    //! Checks if line intersects with any of the line string lines
    //! @param[in]  lineString  lineString to check for intersections with line
    //! @param[in]  line        line to check for intersections with lineString
    //! @return     true if line intersects with any of the line strings.
    //! Note:   line end being on any of the line string's lines does not count as intersection.
    BUILDINGSHAREDUTILS_EXPORT static bool CheckIfLineIntersectsWithLineString(bvector<DPoint3d> lineString, DSegment3d line);

    //! Extracts inner and outer line strings from the curve vector
    //! @param[out] innerLineStrings    inner line strings in curve vector
    //! @param[out] outerLineStrings    outer line strings in curve vector
    //! @param[in]  shape               curve vector to extract line strings from
    //! Note:   If any child of curve vector is not a line string, it will just be ignored.
    BUILDINGSHAREDUTILS_EXPORT static void ExtractInnerOuterLineStrings(bvector<bvector<DPoint3d>>& innerLineStrings, bvector<bvector<DPoint3d>>& outerLineStrings, CurveVectorCR shape);

    //! Returns sum of distances of given points sequence
    //! @param[in]  path    sequence of points
    //! @return     sum of distances of points sequence
    BUILDINGSHAREDUTILS_EXPORT static double GetPathLength(bvector<DPoint3d> path);

    //! Returns length of corrected path
    //! @param[in]  source              starting point of path
    //! @param[in]  target              end point of path
    //! @param[in]  correctionPoints    points between source and target
    //! @return     corrected path length
    BUILDINGSHAREDUTILS_EXPORT static double GetCorrectedPathLength(DPoint3d source, DPoint3d target, bvector<DPoint3d> correctionPoints);
    
    //! Adds vertex to a curve vector.
    //! @param[in/out] cv     CurveVector to add the vertex to
    //! @param[in]     vertex The vertex that should be added
    BUILDINGSHAREDUTILS_EXPORT static void AddVertex(CurveVectorR cv, DPoint3d const& vertex);

    BUILDINGSHAREDUTILS_EXPORT static void LineStringAsWeightedCurve(CurveVectorPtr&, double);

    //! @param[in] shapePoints  Points of a closed shape.
    //! @param[in] pointsToTest Points that are tested if they are in or on shape.
    //! @return false if any of the points inside pointsToTest are outside of shape defined by shapePoints.
    BUILDINGSHAREDUTILS_EXPORT static bool AllPointsInOrOnShape(bvector<DPoint3d> const& shapePoints, bvector<DPoint3d> const& pointsToTest);

    //! @param[in] n        Number of sides or vertices. n >= 3
    //! @param[in] ccRadius Circumscribed circle diameter
    //! @param[in] ccCenter Circumscribed circle center
    //! @param[in] keepArea When true, the polygon's area will be equal to the initial circumscribed circle's area and
    //!                     the resulting polygon's circumscribed circle will be bigger.
    //! @return             Convex polygon vertices
    BUILDINGSHAREDUTILS_EXPORT static bvector<DPoint3d> GetConvexPolygonVertices(size_t n, double ccRadius, DPoint3d ccCenter = {0,0,0}, bool keepArea = false);

    // Gets points from a circular arc.
    //! @param[in] arc            The arc from which points are taken. Arcs that are not circular return 0 points.
    //! @param[in] maxEdgeLength  Maximum distance between returned points.
    //! @param[in] keepSectorArea If true, the returned points + arc's center produces a polygon that has the same area as the given section (arc).
    BUILDINGSHAREDUTILS_EXPORT static bvector<DPoint3d> GetCircularArcPoints(DEllipse3d arc, double maxEdgeLength = DEFAULT_MAX_EDGE_LENGTH, bool keepSectorArea = false);

    //! @param[in] curvePrimitive The primitive to get the points from. Round primitives are faceted.
    //! @param[in] maxEdgeLength  Maximum distance between points if the curvePrimitive was faceted.
    //! @param[in] keepSectorArea If true and curvePrimitive is a circular arc - its radius is increased so the returned points + arc's center produces
    //!                           a polygon that has the same area as the given section (see GetCircularArcPoints).
    BUILDINGSHAREDUTILS_EXPORT static bvector<DPoint3d> GetCurvePrimitivePoints(ICurvePrimitiveCR curvePrimitive, double maxEdgeLength = DEFAULT_MAX_EDGE_LENGTH, bool keepSectorArea = false);

    //! @param[in] curveVectorPtr The curve from which to get the points.
    //! @param[in] maxEdgeLength  If the curveVectorPtr contains rounded shape - this is used for faceting.
    //! @param[in] keepSectorArea If true and curvevectorPtr contains circular arcs - their radius is increased so the returned points + arc's center produces
    //!                           a polygon that has the same area as the given section (see GetCircularArcPoints).
    //! @return                   Returns points of a curve vector. If it is a UnionRegion or a ParityRegion it returns points of a first contained CurveVector.
    BUILDINGSHAREDUTILS_EXPORT static bvector<DPoint3d> GetCurveVectorPoints(CurveVectorCR curveVectorPtr, double maxEdgeLength = DEFAULT_MAX_EDGE_LENGTH, bool keepSectorArea = false);
    
    //! Checks if given closed ICurvePrimitives have same geometry (within tolerance). They are rotated to have the same start point before comparing
    BUILDINGSHAREDUTILS_EXPORT static bool IsSameSingleLoopGeometry(ICurvePrimitiveCR geom1, ICurvePrimitiveCR geom2, double tolerance = DoubleOps::SmallCoordinateRelTol());

    //! Checks if given CurveVectors have the same geometry (within tolerance). They are rotated to have the same start point before comparing.
    BUILDINGSHAREDUTILS_EXPORT static bool IsSameSingleLoopGeometry(CurveVectorCR geom1, CurveVectorCR geom2, double tolerace = DoubleOps::SmallCoordinateRelTol());

    //! Checks if given ICurvePrimitives have the same geometry (within tolerance).
    BUILDINGSHAREDUTILS_EXPORT static bool IsSameGeometry(ICurvePrimitiveCR geom1, ICurvePrimitiveCR geom2, double tolerance = DoubleOps::SmallCoordinateRelTol());


    //! Finds transform from one plane to another
    BUILDINGSHAREDUTILS_EXPORT static Transform FindTransformBetweenPlanes(DPlane3d const& source, DPlane3d const& target);

    //! Finds rotation transform from one plane to another
    BUILDINGSHAREDUTILS_EXPORT static Transform FindRotationTransformBetweenPlanes(DPlane3d const & source, DPlane3d const & target);

    //! Transforms vector from XY plane onto a given one
    //! @param[out] transformed transformed vector
    //! @param[in]  vector      vector to transform
    //! @param[in]  plane       plane to transform vector on
    BUILDINGSHAREDUTILS_EXPORT static void TransformVectorOnPlane(DVec3dR transformed, DVec3d vector, DPlane3d plane);

    //! Transforms vector from XY plane onto a given one using only rotation
    //! @param[out] transformed transformed vector
    //! @param[in]  vector      vector to transform
    //! @param[in]  plane       plane to transform vector on
    BUILDINGSHAREDUTILS_EXPORT static void RotationTransformVectorOnPlane(DVec3dR transformed, DVec3d vector, DPlane3d plane);

    //! Rotates line point around plane normal by given angle
    BUILDINGSHAREDUTILS_EXPORT static void RotateLineEndPointToAngleOnPlane(DPoint3dR rotatedEndPoint, DPoint3dCR fixedEndPoint, double angle, DPlane3d plane);

    //! Joins all connected linestrings (opposite of CurveVector::CloneWithExplodedLinestrings)
    //! @return CurveVector with all adjacent linestrings connected into a single linestring.
    BUILDINGSHAREDUTILS_EXPORT static CurveVectorPtr ConsolidateAdjacentLinestrings(CurveVectorCR);
};

END_BUILDING_SHARED_NAMESPACE