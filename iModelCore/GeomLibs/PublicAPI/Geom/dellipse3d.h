/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#pragma once

/*__PUBLISH_SECTION_START__*/


BEGIN_BENTLEY_GEOMETRY_NAMESPACE
/**
Center, reference vectors, and angular range for elliptic arc.

@ingroup GROUP_Geometry
*/

struct GEOMDLLIMPEXP DEllipse3d
{
DPoint3d center;    //!< Center of ellipse.
DVec3d   vector0;   //!< Vector from center to parametric 0-degree point.
DVec3d   vector90;  //!< Vector from center to parametric 90-degree point.
double start;       //!< Start angle in parameterization
double sweep;       //!< Sweep angle in parameterization

/*__PUBLISH_SECTION_END__*/
//BEGIN_REFMETHODS
/*__PUBLISH_SECTION_START__*/

//BEGIN_FROM_METHODS
//!
//! @description Returns a DEllipse3d with given fields.
//!
//! @param [in] cx  center x coordinate
//! @param [in] cy  center y coordinate
//! @param [in] cz  center z coordinate
//! @param [in] ux  x part of 0 degree vector
//! @param [in] uy  y part of 0 degree vector
//! @param [in] uz  z part of 0 degree vector
//! @param [in] vx  x part of 90 degree vector
//! @param [in] vy  y part of 90 degree vector
//! @param [in] vz  z part of 90 degree vector
//! @param [in] theta0  start angle in parameter space
//! @param [in] sweep  sweep angle
//! @return DEllipse3d object.
//! 
//!
static DEllipse3d From (double cx, double cy, double cz, double ux, double uy, double uz, double vx, double vy, double vz,
double theta0 = 0.0,
double sweep = msGeomConst_2pi
);

//! Return an xy-plane full circle with center and radius.
static DEllipse3d FromCenterRadiusXY
(
DPoint3dCR center,  //!< [in] circle center
double radius       //!< [in] radius.
);

//!
//! @description Return an ellipse defined by fractional start and end on a parent ellipse
//! @param [in] parent existing ellipse.
//! @param [in] startFraction fractional coordiante of new ellipse start on parent.
//! @param [in] endFraction fractional coordiante of new ellipse end on parent.
static DEllipse3d FromFractionInterval
(
DEllipse3dCR parent,
double startFraction,
double endFraction
);

//!
//! @description Returns a DEllipse3d with fill in ellipse data from 2D major and minor axis lengths and the angle
//!   from the global to the local x-axis.
//!
//! @param [in] cx center x coordinate
//! @param [in] cy center y coordinate
//! @param [in] cz z coordinate of all points on the ellipse
//! @param [in] rx radius along local x axis
//! @param [in] ry radius along local y axis
//! @param [in] thetaX angle from global x to local x
//! @param [in] theta0 start angle in parameter space
//! @param [in] sweep sweep angle
//! @return DEllipse3d object.
//! 
//!
static DEllipse3d FromXYMajorMinor (double cx, double cy, double cz, double rx, double ry, double thetaX,
double theta0 = 0.0,
double sweep = msGeomConst_2pi
);
//!
//! @description Returns a DEllipse3d with fill in ellipse data from center, 0 degree, and 90 degree points.
//!
//! @param [in] center ellipse center
//! @param [in] point0 0 degree point
//! @param [in] point90 90 degree point
//! @param [in] theta0 start angle
//! @param [in] sweep sweep angle
//! @return DEllipse3d object.
//! 
//!
static DEllipse3d FromPoints (DPoint3dCR center, DPoint3dCR point0, DPoint3dCR point90, double theta0, double sweep);

//! Construct an arc from start point, start tangent, radius and sweep (and plane normal)
static ValidatedDEllipse3d FromStartTangentNormalRadiusSweep (
DPoint3dCR pointA,          //!< [in] start point of arc
DVec3dCR tangent,           //!< [in] tangent vector.
DVec3dCR planeNormal,       //!< [in] normal to plane of arc.   (The tangent vector is projected into this plane)
double radius,              //!< [in] arc radius
double sweepRadians         //!< [in] sweep angle.
);
//!
//! @description Initialize an elliptical arc from 3 points.
//!
//! @param [in] start start point
//! @param [in] middle mid point
//! @param [in] end end point
//! 
//!
static DEllipse3d FromPointsOnArc (DPoint3dCR start, DPoint3dCR middle, DPoint3dCR end);

//! Return a circular arc with gven center and start.  Endpoint is on the vector to given endTarget.
//! Sweep angle is the smaller of the two possible sweeps.
//! @param [in] center circle center
//! @param [in] startPoint arc start point.  This point determines the circle radius.
//! @param [in] endTarget Target point for end of circle.  If it is at a different radius, the actual end point will be
//!                 at the same radius as the start point.
static DEllipse3d FromArcCenterStartEnd (DPoint3dCR center, DPoint3dCR startPoint, DPoint3dCR endTarget);

//! Return an XY plane circular arc
static DEllipse3d FromCenter_StartPoint_EndTarget_StartTangentBias_CircularXY
(
DPoint3dCR center,      //!< [in] circle center
DPoint3d startPoint,    //!< [in] start point (arc point at 0 in angle space)
DPoint3dCR endTarget,    //!< [in] end target.  The ray from center to actual end point aims at this point.
DVec3dCR startTangentBias   //!< [in] preferred direction for start tangent.  The sweep angle is complemented as needed to make the startTangent DOTXY StartTangentBias positive
);

//! Return a (arc of an) ellipse with given 0 and 90 degree vectors.
static DEllipse3d FromVectors
(
DPoint3dCR center, //!< [in] ellipse center.
DVec3dCR vector0, //!< [in]  vector from center to parametric 0 degree point (Commonly but not necessarily the vector along the "major axis".)
DVec3dCR vector90, //!< [in] vector from center to paramatric 90 degree point.  (Commonly but not necessarily the vector along the "minor axis".
double theta0,  //!< [in] parametric angle of start point.
double sweep //!< [in] parametric sweep angle
);

//! Return a (arc of an) ellipse that is a fillet in the angle between two intersecting lines.
//! The fillet center is the (smaller) arc "between" the rays from the common point outwards.
//! @return The ellipse, or invalid carrier if the vectors are not independent.
static ValidatedDEllipse3d FromFilletCommonPointAndRaysToTangency
(
DPoint3dCR commonPoint, //!< [in] intersection point of the filleted rays.
DVec3dCR vectorA, //!< [in]  vector from intersection towards tanency point on first ray.
DVec3dCR vectorB,  //!< [in]  vector from intersection towards tanency point on second ray.
double radius      //! [in] circle radius.
);

//! Return a (arc of an) ellipse that is a fillet in the angle between two line segments with a common point.
//! The fillet center is the (smaller) arc "between" the rays from the common point outwards.
//! @return The ellipse, or invalid carrier if the vectors are not independent.
static ValidatedDEllipse3d FromFilletInCorner
(
DPoint3dCR pointA,  //!< [in] point "before" the filleted corner
DPoint3dCR pointB,  //!< [in] corner to be filleted away
DPoint3dCR pointC,  //!< [in] point "after" the filleted corner.
double radius       //!< [in] fillet radius
);

//! Return a (arc of an) ellipse that is a fillet in the angle between two line segments with a common point.
//! The fillet radius is chosen so that the tangency point closer of pointA and pointB
//! The fillet center is the (smaller) arc "between" the rays from the common point outwards.
//! The radius is determined
//! @return The ellipse, or invalid carrier if the vectors are not independent.
static ValidatedDEllipse3d FromFilletInBoundedCorner
(
DPoint3dCR pointA,  //!< [in] point "before" the filleted corner
DPoint3dCR pointB,  //!< [in] corner to be filleted away
DPoint3dCR pointC   //!< [in] point "after" the filleted corner.
);

//! Return a (arc of an) ellipse that is a fillet in the angle between two intersecting lines.
//! The fillet center is the (smaller) arc "between" the rays from the line-line intersection
//!     outbound in the direction of hte segments.
//! @return The ellipse, or invalid carrier if the vectors are not independent.
static ValidatedDEllipse3d FromFilletToOutboundTangencySegments
(
DSegment3dCR segmentA,  //!< [in] outward directed segment.
DSegment3dCR segmentB,  //!< [in] outward directed segment.
double radius      //! [in] circle radius.
);
//! Return an arc of an ellipse matches two points and tangents at those points, with caller-supplied sweep.
//!<ul>
//!<li>The start angle of the result will be 0.
//!<li>The start point (at that 0-radian start angle) will generally NOT be a major or minor axis point.
//!<li>To force it as a major/minor point, use newArc = DEllipse3d::FromMajorMinor (result.Value ())
//!</ul>
static ValidatedDEllipse3d FromStartTangentSweepEndTangentXY
(
DPoint3dCR pointA,      //!< [in] Constraint: Start at this point.
DVec3dCR tangentA,      //!< [in] Constraint: Start tangent is parallel to this
DPoint3dCR pointB,      //!< [in] Constraint: End at this point.
DVec3dCR tangentB,     //!< [in] Constratin: End tangent is parallel to this.
double sweepRadians    //!< [in] Constraint: sweep angle for output ellipse
);


//! Return an (arc of an) ellipse, with axes defined by a RotMatrix XY columns and scale factors.
static DEllipse3d FromScaledRotMatrix
(
DPoint3dCR center,  //!< [in] ellispse center
RotMatrixCR matrix, //!< [in] orientation matrix.
double r0,          //!< [in] 0 degree radius.  0 degree vector is r0 times the x column of the matrix.
double r90,          //!< [in]  90 degree radius.  90 degree vector is r90 times the y column of the matrix.
double theta0,      //!< [in]   parametric start angle
double sweep        //!< [in]   parametric sweep angle
);
//! Return an (arc of an) ellipse with axes defined directly by scaled vectors.
static DEllipse3d FromScaledVectors
(
DPoint3dCR center, //!< [in]    ellipse center
DVec3dCR vector0, //!< [in]     vector for 0 degree parametric direction
DVec3dCR vector90, //!< [in]    vector for 90 degree parametric direction
double r0,          //!< [in]   0 degree radius.  0 degree vector is r0 times vector0
double r90,          //!< [in]  90 degree radius.  90 degree vector is r90 times the vector90
double theta0,      //!< [in]   parametric start angle
double sweep        //!< [in]   parametric sweep angle
);

//! Return an (arc of an) ellipse with axes scaled from those of source
static DEllipse3d FromScaledVectors
(
DEllipse3dCR source, //!<[in] source ellipse
double factor        //!<[in] scale factor for vector0, vector90
);


//! Return an ellispe that sweeps the same points as source but has axes adjusted so vectors to parametric 0 and 90 degree points
//!    are perpendicular and the 0 degree vector is the customary major (larger) direction.
//! @param [in] source original ellipse, in which vectors might be non perpendicular.
static DEllipse3d FromMajorMinor (DEllipse3dCR source);

//! Return an (full 360 degree) circular arc with given center, plane normal, and radius.
//! @param [in] center ellipse center.
//! @param [in] normal plane normal.
//! @param [in] radius circle radius.
static DEllipse3d FromCenterNormalRadius (DPoint3dCR center, DVec3dCR normal, double radius);

//! Return an ellispe that sweeps the same points as source but has axes adjusted so vectors to parametric 0 and 90 degree points
//!    are perpendicular.   This may choose a "small" adjustment of the axes even if the 90 degree axis ends up longer. Use FromMajorMinor to force
//!    the 0 degree direction to be the larger axis.
//! @param [in] source original ellipse, in which vectors might be non perpendicular.
static DEllipse3d FromPerpendicularAxes (DEllipse3dCR source);
//! Return an ellipse that sweeps the same points in space but in the reversed direction.
//! @param [in] source original ellipse
static DEllipse3d FromReversed (DEllipse3dCR source);

//! Return an ellipse that sweeps the same points in space (with identical fraction-to-point results)
//! but has negated vector90 (and hence negated normal)
static DEllipse3d FromNegateVector90 (DEllipse3dCR source);


//! Return an ellipse that sweeps the same points in space (with identical fraction-to-point results)
//!      but is driven by a positive sweep angle.
//! @remark If the source ellipse has positive sweep it is simply copied back out.
//!   If it has a negative sweep, the start angle, sweep angle, and vector90 are negated.
//!   If a point is on the ellipse, its angular coordinate will be negated, but its fractional position is maintained.
//! @param [in] source original ellipse
static DEllipse3d FromCopyWithPositiveSweep (DEllipse3dCR source);


//! Return an ellipse that traverses the same points in space but
//! has its vector0 and vector90 axes shifted so that the parametric start takes on specified value.
//! @param [in] source original ellipse.
//! @param [in] newStart the angle value that should be the start in the new ellispe.
static DEllipse3d FromRotatedAxes (DEllipse3dCR source, double newStart);

//! return an ellipse that is the sweep of spaceEllipse onto a target plane, with the sweep direction
//! independent of the plane normal.
static ValidatedDEllipse3d FromEllipseSweptToPlane
(
DEllipse3dCR spaceEllipse,      //!< [in] known ellipse, in any position in space   
DPlane3dCR targetPlane,         //!< [in] target plane
DVec3dCR sweepDirection         //!< [in] direction of sweep
);
//END_FROM_METHODS

//!
//! @description Fill in ellipse data.
//!
//! @param [in] cx  center x coordinate
//! @param [in] cy  center y coordinate
//! @param [in] cz  center z coordinate
//! @param [in] ux  x part of 0 degree vector
//! @param [in] uy  y part of 0 degree vector
//! @param [in] uz  z part of 0 degree vector
//! @param [in] vx  x part of 90 degree vector
//! @param [in] vy  y part of 90 degree vector
//! @param [in] vz  z part of 90 degree vector
//! @param [in] theta0  start angle in parameter space
//! @param [in] sweep  sweep angle
//! 
//!
void Init
(
double          cx,
double          cy,
double          cz,
double          ux,
double          uy,
double          uz,
double          vx,
double          vy,
double          vz,
double          theta0,
double          sweep
);

//!
//! @description Fill in ellipse data from 2D major and minor axis lengths and the angle
//!   from the global to the local x-axis.
//!
//! @param [in] cx center x coordinate
//! @param [in] cy center y coordinate
//! @param [in] cz z coordinate of all points on the ellipse
//! @param [in] rx radius along local x axis
//! @param [in] ry radius along local y axis
//! @param [in] thetaX angle from global x to local x
//! @param [in] theta0 start angle in parameter space
//! @param [in] sweep sweep angle
//! 
//!
void InitFromXYMajorMinor
(
double          cx,
double          cy,
double          cz,
double          rx,
double          ry,
double          thetaX,
double          theta0,
double          sweep
);

//!
//! @description Fill in ellipse data from center, 0 degree, and 90 degree points.
//!
//! @param [in] center ellipse center
//! @param [in] point0 0 degree point
//! @param [in] point90 90 degree point
//! @param [in] theta0 start angle
//! @param [in] sweep sweep angle
//! 
//!
void InitFromPoints
(
DPoint3dCR      center,
DPoint3dCR      point0,
DPoint3dCR      point90,
double          theta0,
double          sweep
);

//!
//! @description Initialize an elliptical arc from 3 points.
//!
//! @param [in] start start point
//! @param [in] middle mid point
//! @param [in] end end point
//! @return true if the three points are valid, false if colinear.
//! 
//!
bool InitFromPointsOnArc
(
DPoint3dCR      start,
DPoint3dCR      middle,
DPoint3dCR      end
);

//!
//! @description Initialize a circlular arc from start point, end point, another vector which
//!  determines the plane, and the arc length.
//!
//! @param [in] startIN start point
//! @param [in] end end point
//! @param [in] arcLength required arc length
//! @param [in] planeVector vector to be used to determine the plane of the
//!                    arc.  The plane is chosen so that it contains both the
//!                    start-to-end vector and the plane vector, and the arc bulge
//!                    is in the direction of the plane vector (rather than opposite).
//! @return true if the arc length exceeds the chord length and the 2 points and plane vector
//!                determine a clear plane.
//! 
//!
bool InitArcFromPointPointArcLength
(
DPoint3dCR      startIN,
DPoint3dCR      end,
double          arcLength,
DVec3dCR      planeVector
);

//!
//! @description Initialize a circular arc from start point, start tangent, and end point.
//!
//! @param [in] startIN start point
//! @param [in] tangent start tangent
//! @param [in] end end point
//! @return true if circular arc computed.   false if start, end and tangent are colinear.
//! 
//!
bool InitArcFromPointTangentPoint
(
DPoint3dCR      startIN,
DVec3d      tangent,
DPoint3dCR      end
);

//!
//! @description Initialize a circular arc with given center and start, and with
//! sweep so that the end point is near the given end. (Note that the circle
//! will NOT pass directly through the endpoint itself if it is at a different
//! distance from the center.)  The arc is always the smaller of the two
//! possible parts of the full circle.
//!
//! @param [in] centerIN ellipse center
//! @param [in] startIN start point
//! @param [in] end nominal end point
//! @return false if the the three points are colinear.
//! 
//!
bool InitFromArcCenterStartEnd
(
DPoint3dCR      centerIN,
DPoint3dCR      startIN,
DPoint3dCR      end
);

//!
//! @description Fill in ellipse data from center and two basis vectors.
//!
//! @param [in] centerIN ellipse center
//! @param [in] vector0IN 0 degree vector
//! @param [in] vector90IN 90 degree vector
//! @param [in] theta0 start angle
//! @param [in] sweepIN sweep angle
//! 
//!
void InitFromVectors
(
DPoint3dCR      centerIN,
DVec3dCR        vector0IN,
DVec3dCR        vector90IN,
double          theta0,
double          sweepIN
);


//!
//! @description Set angular parameters to have given start and end points.
//! @remarks If the given points are really on the ellipse, this does the expected thing.
//! @remarks If the given points are not on the ellipse, here's exactly what happens.
//!    The start/end points are placed on the original ellipse at the point where the ellipse intersects
//!    the plane formed by the ellipse axis and the given point.  This leaves the problem that the ellipse
//!    defines two paths from the given start to end. This is resolved as follows.  The ellipse's existing
//!    0 and 90 degree vectors define a coordinate system.  In that system, the short sweep from the 0
//!    degree vector to the 90 degree vector is considered "counterclockwise".
//! @remarks Beware that the relation of supposed start/end points to the ellipse is ambiguous.
//!
//! @param [in] startPoint  new start point
//! @param [in] endPoint  new end point
//! @param [in] ccw true to force counterclockwise direction, false for clockwise.
//! @return true if the ellipse axes are independent.  false if the ellipse is degenerate.
//! 
//!
bool SetStartEnd
(
DPoint3dCR      startPoint,
DPoint3dCR      endPoint,
bool            ccw
);

//!
//! @description Fill in ellipse data from center, x and y directions from columns
//! 0 and 1 of a RotMatrix, and scale factors to apply to x and and y directions.
//!
//! @param [in] centerIN ellipse center
//! @param [in] matrix columns 0, 1 are ellipse directions (to be scaled by r0, r1)
//! @param [in] r0 scale factor for column 0
//! @param [in] r1 scale factor for column 1
//! @param [in] theta0 start angle
//! @param [in] sweepIN sweep angle
//! 
//!
void InitFromScaledRotMatrix
(
DPoint3dCR      centerIN,
RotMatrixCR     matrix,
double          r0,
double          r1,
double          theta0,
double          sweepIN
);



//!
//! @description Fill in ellipse data from center and x and y directions as vectors with scale factors.
//!
//! @param [in] centerIN ellipse center
//! @param [in] vector0IN 0 degree vector (e.g. major axis)
//! @param [in] vector90IN 90 degree vector (e.g. minor axis)
//! @param [in] r0 scale factor for vector 0
//! @param [in] r1 scale factor for vector 90
//! @param [in] theta0 start angle
//! @param [in] sweepIN sweep angle
//! 
//!
void InitFromScaledVectors
(
DPoint3dCR      centerIN,
DVec3dCR      vector0IN,
DVec3dCR      vector90IN,
double          r0,
double          r1,
double          theta0,
double          sweepIN
);

//!
//! @description Adjust axis vectors so 0-degree vector is along true major axis.
//! @param [in] source source ellipse.
//!
void InitMajorMinor
(
DEllipse3dCR source
);

//!
//! @description Extract major minor axis form of the ellipse.
//!
//! @param [out] center ellipse center
//! @param [out] matrix columns 0, 1 are normalized ellipse basis vectors, column 2 is their cross product
//! @param [out] r0 scale factor for column 0
//! @param [out] r1 scale factor for column 1
//! @param [out] theta0 start angle
//! @param [out] sweep sweep angle
//! 
//!
void GetScaledRotMatrix
(
DPoint3dR       center,
RotMatrixR      matrix,
double          &r0,
double          &r1,
double          &theta0,
double          &sweep
) const;


//!
//! @description Extract major minor axis form of the ellipse.
//!
//! @param [out] localToWorld orthogonal frame with origin at center, xy axes n major and minor axes.
//! @param [out] r0 scale factor for column 0
//! @param [out] r1 scale factor for column 1
//! @param [out] theta0 start angle
//! @param [out] sweep sweep angle
//! @param [out] worldToLocal inverse of localToWorld.
//! 
//!
void GetScaledTransforms
(
TransformR localToWorld,
double &r0,
double &r1,
double &theta0,
double &sweep,
TransformR worldToLocal
) const;
//!
//! @description Initialize a circle from center, normal and radius.
//! @param [in] centerIN circle center
//! @param [in] normal plane normal 
//! @param [in] radius circle radius
//! 
//!
void InitFromCenterNormalRadius
(
DPoint3dCR      centerIN,
DVec3dCR      normal,
double          radius
);

//!
//! @description Test whether the ellipse is complete (2pi range).
//! @return true if the ellipse is complete
//! 
//!
bool IsFullEllipse () const;

//! @description Test whether both vectors are near zero length.
//! @return true if both radii are near zero.
bool IsNearZeroRadius() const;

//! @return true if AlmostEqual center, vectors, and angles.
bool IsAlmostEqual (DEllipse3dCR other, double tolerance) const;
//!
//! @description Set the ellipse sweep to a full 360 degrees (2pi radians), preserving direction of sweep.
//! @remarks Start angle is left unchanged.
//! 
//!
void MakeFullSweep ();

//!
//! @description Set the ellipse sweep to the complement of its current angular range.
//! @remarks Full ellipse is left unchanged.
//! 
//!
void ComplementSweep ();

//!
//! @description Set the ellipse sweep to the smaller of its current or complement sweep
//! @remarks Full ellipse is left unchanged.
void SelectSmallerSweep ();


//!
//! @description Compute the ellipse xyz point at a given parametric (angular) coordinate.
//! @param [out] point evaluated point
//! @param [in] theta angle
//! 
//!
void Evaluate
(
DPoint3dR       point,
double          theta
) const;

//! @description return the point on the ellipse at parameteric angle.
DPoint3d RadiansToPoint (double theta) const;
//!
//! @description Compute the ellipse xyz point at a given parametric (xy) coordinate.
//! @param [out] point evaluated point
//! @param [in] xx local x coordinate: cos(theta)
//! @param [in] yy local y coordinate: sin(theta)
//! 
//!
void Evaluate
(
DPoint3dR       point,
double          xx,
double          yy
) const;

//!
//! @description Compute the ellipse xyz point at a given parametric (angular) coordinate.
//! @param [out] point evaluated point (unit weight)
//! @param [in] theta angle
//! 
//!
void Evaluate
(
DPoint4dR       point,
double          theta
) const;

//!
//! @description Compute the ellipse start and end points.
//! @param [out] startPoint start point of ellipse
//! @param [out] endPoint end point of ellipse
//! 
//!
void EvaluateEndPoints
(
DPoint3dR       startPoint,
DPoint3dR       endPoint
) const;

//!
//! @description  Compute the ellipse xyz point and derivatives at a given parametric (angular) coordinate.
//! @param [out] point3dX point on ellipse
//! @param [out] dX first derivative vector
//! @param [out] ddX second derivative vector
//! @param [in] theta angle for evaluation
//! 
//!
void Evaluate
(
DPoint3dR       point3dX,
DVec3dR       dX,
DVec3dR       ddX,
double          theta
) const;

//! Return a point at fractional position in the angle space.
DPoint3d FractionToPoint (double fraction) const;
//!
//! @description Compute the ellipse xyz point at a given fraction of the angular parametric range.
//! @param [out] point3dX point on ellipse
//! @param [in] fraction fractional parameter for evaluation
//! 
//!
void FractionParameterToPoint
(
DPoint3dR       point3dX,
double          fraction
) const;

//!
//! @description Compute the ellipse xyz point and derivatives at a given fraction of the angular parametric range.
//! @param [out] point3dX point on ellipse
//! @param [out] dX second derivative vector
//! @param [out] ddX second derivative vector
//! @param [in] fraction fractional parameter for evaluation
//! 
//!
void FractionParameterToDerivatives
(
DPoint3dR       point3dX,
DVec3dR       dX,
DVec3dR       ddX,
double          fraction
) const;

//!
//! @description Compute ellipse xyz point and derivatives, returned as an array.
//! @param [out] point3dX Array of ellipse point, first derivative, etc.  Must contain room for numDerivatives+1 points.  point3dX[i] = i_th derivative.
//! @param [in] numDerivative number of derivatives (0 to compute just the xyz point)
//! @param [in] theta angle for evaluation
//! 
//!
void Evaluate
(
DPoint3dP       point3dX,
int             numDerivative,
double          theta
) const;

//!
//! @description Convert a fractional parameter to ellipse parameterization angle.
//! @param [in] fraction fraction of angular range
//! @return angular parameter
//! 
//!
double FractionToAngle (double fraction) const;

//!
//! @description Compute the determinant of the Jacobian matrix for the transformation from local coordinates (cosine, sine) to global xy-coordinates.
//! @return determinant of Jacobian.
//! 
//!
double DeterminantJXY () const;

//! Return the (unnormalized) cross product of the basis vectors.
//!<ul>
//!<li>This vector is normal to the plane of the ellipse.
//!<li>The magnitude is the area of the parallelogram for a quadrant.
//!</ul>
DVec3d CrossProductOfBasisVectors () const;
//!
//! @description Get the coordinate frame for an ellipse.  X,Y axes are at 0 and 90 degrees.
//! Z axis is perpendicular with magnitude equal to the geometric mean of the other two.
//! @param [out] frame transformation from (cosine, sine, z) coordinates to global xyz.
//! @param [out] inverse inverse of frame.
//! @return true if the requested frames were returned.
//! 
//!
bool GetLocalFrame
(
TransformR      frame,
TransformR      inverse
) const;

//! Evaluate a frame with
//!<ul>translation is arc center.
//!<li>x direction is towards the fraction point.
//!<li>y direction is perpendicular and forward along arc.
//!<li>z is x cross y
//!</ul>
ValidatedTransform FractionToCenterFrame (double fraction) const;
//! Evaluate a frame with
//!<ul>translation is point on arc
//!<li>x direction is arc tangent
//!<li>y direction is towarcs center of curvature
//!<li>z is x cross y
//!</ul>
ValidatedTransform FractionToFrenetFrame (double fraction) const;

//!
//! @description Get the coordinate frame and inverse of an ellipse as viewed along the global z axis.
//! @param [out] frame transformation from (cosine, sine, z) coordinates to global xyz.
//! @param [out] inverse inverse of frame.
//! @return true if the requested frames were returned.
//! 
//!
bool GetXYLocalFrame
(
TransformR   frame,
TransformR   inverse
) const;


//!
//! @description Compute the local coordinates of a point in the skewed coordinates of the ellipse, using
//! only xy parts of both the ellipse and starting point.
//! @remarks This is equivalent to computing the intersection of the ellipse plane with a line through the point and
//! parallel to the z axis, and returning the coordinates of the intersection relative to the
//! skewed axes of the ellipse.
//! @param [out] localPoint evaluated point.  Coordinates x,y are multipliers for the ellipse axes.
//!                        Coordinate z is height of the initial point from the plane of the ellipse.
//! @param [in] point point to convert to local coordinates
//! @return true if ellipse axes are independent.
//! 
//!
bool PointToXYLocal
(
DPoint3dR       localPoint,
DPoint3dCR      point
) const;

//!
//! @description Compute the angular position of the point relative to the ellipse's local coordinates.
//! @remarks If the point is on the ellipse, this is the inverse of evaluating the ellipse at the angle.
//! @param [in] point point to evaluate
//! @return angle in ellipse parameterization
//! 
//!
double PointToAngle (DPoint3dCR point) const;

//!
//! @description Project a point onto the plane of the ellipse.
//!
//! @param [out] xYZNear projection of point onto ellipse plane
//! @param [out] coff0 coefficient on vector towards 0 degree point
//! @param [out] coff90 coefficient on vector towards 90 degree point
//! @param [in] xYZ point to project onto plane
//! @return true if the plane is well defined.
//! 
//!
bool ProjectPointToPlane
(
DPoint3dR       xYZNear,
double          &coff0,
double          &coff90,
DPoint3dCR      xYZ
) const;

//!
//! @description Compute an estimated number of points needed to stroke a full ellipse to within the given chord height tolerance.
//! @param [in] nDefault default number of points on full ellipse
//! @param [in] nMax max number of points on full ellipse
//! @param [in] chordTol distance tolerance
//! @param [in] angleTol turning angle tolerance
//! @return number of strokes required on the full ellipse
//! 
//!
int GetStrokeCount
(
int             nDefault = 12,
int             nMax = 180,
double          chordTol = 0.0,
double          angleTol = 0.0
) const;

//!
//! @description Evaluate an ellipse using given coefficients for the axes.
//! @remarks If the x,y components of the coefficients define a unit vector, the point is "on" the ellipse.
//! @param [out] point array of cartesian points
//! @param [in] trig array of local coords (e.g., (cos, sin)).
//! @param [in] numPoint number of pairs
//! 
//!
void EvaluateTrigPairs
(
DPoint3dP       point,
DPoint2dCP      trig,
int             numPoint
) const;

//!
//! @description Evaluate an ellipse at a number of (cosine, sine) pairs, removing
//! pairs whose corresponding angle is not in range.
//!
//! @param [out] point array of cartesian points
//! @param [in] trig array of local coords
//! @param [in] numPoint number of pairs
//! @return number of points found to be in the angular range of the ellipse.
//! 
//!
int TestAndEvaluateTrigPairs
(
DPoint3dP       point,
DPoint2dCP      trig,
int             numPoint
) const;

//!
//! @description Test if a specified angle is within the sweep of the ellipse.
//! @param [in] angle angle (radians) to test
//! @return true if angle is within the sweep angle of the elliptical arc.
//! 
//!
bool IsAngleInSweep (double angle) const;

//!
//! @description Convert an angular parameter to a fraction of bounded arc length.
//! @param [in] angle angle (radians) to convert
//! @return fractional parameter
//! 
//!
double AngleToFraction (double angle) const;

//!
//! @description Get the start and end angles of the ellipse.
//! @param [out] startAngle start angle
//! @param [out] endAngle end angle
//! 
//!
void GetLimits
(
double          &startAngle,
double          &endAngle
) const;

//!
//! @description Get the start and sweep angles of the ellipse.
//! @param [out] startAngle start angle
//! @param [out] sweepAngle sweep angle
//! 
//!
void GetSweep
(
double          &startAngle,
double          &sweepAngle
) const;

//!
//! @description Set the start and end angles of the ellipse.
//! @param [in] startAngle start angle
//! @param [in] endAngle end angle
//! 
//!
void SetLimits
(
double          startAngle,
double          endAngle
);

//!
//! @description Set the start and sweep angles of the ellipse.
//! @param [in] startAngle start angle
//! @param [in] sweepIN sweep angle
//! 
//!
void SetSweep
(
double          startAngle,
double          sweepIN
);

//!
//! @description Make a copy of the source ellipse, altering the axis vectors and angular limits so that
//! the revised ellipse has perpendicular axes in the conventional major/minor axis form.
//! @remarks Inputs may be the same.
//! @param [in] source ellipse with unconstrained axes
//! 
//!
void InitWithPerpendicularAxes (DEllipse3dCR source);

//!
//! @description Compute the range box of the ellipse in its major-minor axis coordinate system.
//! Compute line segments that are the horizontal and vertical midlines in that system.
//! Return those line segments ordered with the longest first, and return the shorter length.
//!
//! @remarks The typical use of this is that if the shorter length is less than some tolerance the
//! points swept out by the ellipse are the longer segment.  (But beware that the start and
//! end points of the segment can be other than the start and end points of the ellipse.)
//!
//! @param [out] longSegment longer axis of local conic range box
//! @param [out] shortSegment shorter axis of local conic range box
//! @return size of the shorter dimension
//! 
//!
double GetMajorMinorRangeMidlines
(
DSegment3dR     longSegment,
DSegment3dR     shortSegment
) const;

//!
//! @description Make a copy of the source ellipse, reversing the start and end angles.
//! @remarks Inputs may be the same.
//! @param [in] source source ellipse
//! 
//!
void InitReversed (DEllipse3dCR source);

//!
//! @description Compute the magnitude of the tangent vector to the ellipse at the specified angle.
//! @param [in] theta angular parameter
//! @return tangent magnitude
//! 
//!
double TangentMagnitude (double theta) const;

//!
//! @description Return arc length of ellipse.
//! @return arc length of ellipse.
//! 
//!
double ArcLength () const;

//!
//! @description Return the sweep angle corresponding to an arc length.
//! @remarks Negative returned sweep angle corresponds to arclength traversed in the opposite direction of the ellipse sweep.
//! @param [in] arcLength  arc length to invert
//! @return sweep angle
//! 
//!
double InverseArcLength (double arcLength) const;

//!
//! @description Compute the (signed) arc length between specified fractional parameters.
//! @remarks Fractions outside [0,1] return error.
//! @param [out] arcLength computed arc length.  Negative if fraction1 < fraction0.
//! @param [in] fraction0 start fraction for interval to measure
//! @param [in] fraction1 end fraction for interval to measure
//! @return true if the arc length was computed.
//! 
//!
bool FractionToLength
(
double          &arcLength,
double          fraction0,
double          fraction1
) const;

//!
//! @description Compute the xyz range limits of a 3D ellipse.
//! @param [out] range computed range
//! 
//!
void GetRange (DRange3dR range) const;

//!
//! @description Compute the range of the ellipse in its own coordinate system.
//! @remarks This depends on the start and sweep angles but not the center or axis coordinates.
//! @param [out] range computed range
//! 
//!
void GetLocalRange (DRange2dR range) const;


//! @return range when projected to fraction space of the ray
//! @param [in] ray
DRange1d ProjectedParameterRange (DRay3dCR ray) const;

//!
//! @description Find intersections of a (full) ellipse with a plane.
//! @remarks Return value n=1 is a single tangency point returned in trigPoints[0];
//!        n=2 is two simple intersections returned in trigPoints[0..1]
//! @remarks The three component values in trigPoints are:
//! <UL>
//! <LI>x == cosine of angle
//! <LI>y == sine of angle
//! <LI>z == angle in radians
//! </UL>
//! @param [out] trigPoints 2 points: cosine, sine, theta values of plane intersection
//! @param [in] plane homogeneous plane equation
//! @return The number of intersections, i.e. 0, 1, or 2
//! 
//!
int IntersectPlane
(
DPoint3dP       trigPoints,
DPoint4dCR      plane
) const;

//! @description Compute the intersection of the tangents from the endpoints of the arc.
//! @return false if arc is plus or minus 180 degrees (hence tangents intersect at infinity).
//! @param [out] xyz computed point.
bool IntersectionOfStartAndEndTangents (DPoint3dR xyz) const;


//! @description Find angles at which the ellipse tangent vector is perpendicular to given vector.
//! @param [out] angles 0,1, or 2 angles.   This is an array that must be allocated by the caller.
//! @param [in] vector perpendicular vector.
//! @return The number of solutions, i.e. 0, 1, or 2
//! 
int SolveTangentsPerpendicularToVector
(
double  *angles,   
DVec3dR vector
) const;
//!
//! @description Find the intersections of xy projections of an ellipse and line.
//! @remarks May return 0, 1, or 2 points.  Both ellipse and line are unbounded.
//! @param [out] cartesianPoints cartesian intersection points
//! @param [out] pLineParams array of line parameters (0=start, 1=end)
//! @param [out] ellipseCoffs array of coordinates relative to the ellipse.
//!                              For each point, (xy) are the cosine and sine of the
//!                              ellipse parameter, (z) is z distance from the plane of
//!                              of the ellipse.
//! @param [out] pEllipseAngle array of angles on the ellipse
//! @param [in] startPoint line start
//! @param [in] endPoint line end
//! @return the number of intersections.
//! 
//!
int IntersectXYLine
(
DPoint3dP       cartesianPoints,
double          *pLineParams,
DPoint3dP       ellipseCoffs,
double          *pEllipseAngle,
DPoint3dCR      startPoint,
DPoint3dCR      endPoint
) const;

//! @description Test if the ellipse is circular.
//! @return true if circular
//! 
bool IsCircular () const;

//! @description Test if the ellipse is circular.
//! @return true if circular
//! @param [out] radius circular radius
bool IsCircular (double &radius) const;

//! @description Test if the XY projection of the ellipse is circular.
//! @return true if circular
bool IsCircularXY () const;

//! @description Test if the XY projection of the ellipse is circular.
//! @return true if circular
bool IsCircularXY (double &radius) const;

//! @description Test if the XY projection of the ellipse is CCW when considering both the
//! sweep sign and the vector directions.
//! @return true if circular
bool IsCCWSweepXY () const;

//!
//! @description Find the intersections of xy projections of two ellipses.
//! @remarks May return 0, 1, 2, 3 or 4 points.  Both ellipses are unbounded.
//! @param [out] cartesianPoints cartesian intersection points.
//! @param [out] ellipse0Params array of coordinates relative to the first ellipse.
//!                            For each point, (xy) are the cosine and sine of the
//!                            ellipse parameter, (z) is z distance from the plane of
//!                            of the ellipse.
//! @param [out] ellipse1Params array of coordinates relative to the second ellipse.
//!                            For each point, (xy) are the cosine and sine of the
//!                            ellipse parameter, (z) is z distance from the plane of
//!                            of the ellipse.
//! @param [in] ellipse1 the other ellipse.
//! @return the number of intersections.
//! 
//!
int IntersectXYDEllipse3d
(
DPoint3dP       cartesianPoints,
DPoint3dP       ellipse0Params,
DPoint3dP      ellipse1Params,
DEllipse3dCR    ellipse1
) const;

//!
//! @description Find the intersections of xy projections of two ellipses, with bounds applied.
//! @remarks May return 0, 1, 2, 3 or 4 points.
//! @param [out] cartesianPoints cartesian intersection points.
//! @param [out] ellipse0Coffs array of coordinates relative to the first ellipse
//!                            For each point, (xy) are the cosine and sine of the
//!                            ellipse parameter, (z) is z distance from the plane of
//!                            of the ellipse.
//! @param [out] pEllipse0Angle array of angles on the first ellipse
//! @param [out] ellipse1Coffs array of coordinates relative to the second ellipse.
//!                            For each point, (xy) are the cosine and sine of the
//!                            ellipse parameter, (z) is z distance from the plane of
//!                            of the ellipse.
//! @param [out] pEllipse1Angle array of angles on the other ellipse
//! @param [in] ellipse1 the other ellipse.
//! @return the number of intersections.
//! 
//!
int IntersectXYDEllipse3dBounded
(
DPoint3dP       cartesianPoints,
DPoint3dP       ellipse0Coffs,
double          *pEllipse0Angle,
DPoint3dP       ellipse1Coffs,
double          *pEllipse1Angle,
DEllipse3dCR    ellipse1
) const;

//!
//! @description Find "intersections" of two DEllipse3d.
//! @remarks Ellipses in space can pass very close to
//! each other without intersecting, so some logic must be applied to define intersection
//! more cleanly. The logic applied is to choose the more circular ellipse and apply the
//! transformation which makes that one a unit circle, then intersect the xy projections of the
//! transformations.
//!
//! @param [out] cartesianPoints cartesian intersection points.
//! @param [out] ellipse0Params array of coordinates relative to the first ellipse
//!                                For each point, (xy) are the cosine and sine of the
//!                                ellipse parameter, (z) is z distance from the plane of
//!                                of the ellipse.
//! @param [out] ellipse1Params array of coordinates relative to the second ellipse.
//!                                For each point, (xy) are the cosine and sine of the
//!                                ellipse parameter, (z) is z distance from the plane of
//!                                of the ellipse.
//! @param [in] ellipse1 the other ellipse.
//! @return the number of intersections.
//! 
//!
int IntersectSweptDEllipse3d
(
DPoint3dP       cartesianPoints,
DPoint3dP       ellipse0Params,
DPoint3dP       ellipse1Params,
DEllipse3dCR    ellipse1
) const;

//!
//! @description Intersect two ellipses as described in ~mbsiDEllipse3d_intersectSweptDEllipse3d, and
//! filter out results not part of both ranges.
//!
//! @param [out] cartesianPoints cartesian intersection points.
//! @param [out] ellipse0Coffs array of coordinates relative to the first ellipse
//!                            For each point, (xy) are the cosine and sine of the
//!                            ellipse parameter, (z) is z distance from the plane of
//!                            of the ellipse.
//! @param [out] pEllipse0Angle array of angles on the first ellipse.
//! @param [out] ellipse1Coffs array of coordinates relative to the second ellipse.
//!                            For each point, (xy) are the cosine and sine of the
//!                            ellipse parameter, (z) is z distance from the plane of
//!                            of the ellipse.
//! @param [out] pEllipse1Angle array of angles on the other ellipse.
//! @param [in] ellipse1 the other ellipse.
//! @return the number of intersections.
//! 
//!
int IntersectSweptDEllipse3dBounded
(
DPoint3dP       cartesianPoints,
DPoint3dP       ellipse0Coffs,
double          *pEllipse0Angle,
DPoint3dP       ellipse1Coffs,
double          *pEllipse1Angle,
DEllipse3dCR    ellipse1
) const;

//!
//! @description Find "intersections" of a DSegment3d and a DEllipse3d.
//! @remarks Curves in space can pass very close to
//! each other without intersecting, so some logic must be applied to define intersection
//! more cleanly. The logic applied is to compute the intersection of the line with
//! the cylinder swept by the ellipse along its plane normal.
//!
//! @param [out] pointArray cartesian intersection points.
//! @param [out] ellipseParams array of coordinates relative to the instance
//!                            For each point, (xy) are the cosine and sine of the
//!                            ellipse parameter, (z) is z distance from the plane of
//!                            of the ellipse.
//! @param [out] pLineParams array of parametric coordinates on the line.
//! @param [in] segment the line segment
//! @return the number of intersections.
//!  
//!
int IntersectSweptDSegment3d
(
DPoint3dP       pointArray,
DPoint3dP       ellipseParams,
double          *pLineParams,
DSegment3dCR    segment
) const;

//!
//! @description Intersect an ellipse and a segment as described in ~mbsiDEllipse3d_intersectSweptDSegment3d, and
//! filter out results not part of both ranges.
//!
//! @param [out] pointArray cartesian intersection points.
//! @param [out] ellipseParams array of coordinates relative to the instance
//!                            For each point, (xy) are the cosine and sine of the
//!                            ellipse parameter, (z) is z distance from the plane of
//!                            of the ellipse.
//! @param [out] pLineParams array of parametric coordinates on the line.
//! @param [in] segment the line segment
//! @return the number of intersections.
//! 
//!
int IntersectSweptDSegment3dBounded
(
DPoint3dP       pointArray,
DPoint3dP       ellipseParams,
double          *pLineParams,
DSegment3dCR    segment
) const;

//!
//! @description Project a point onto the (unbounded) ellipse.
//! @remarks May return up to 4 points.
//! @param [out] cartesianPoints  array (allocated by caller) of points on the ellipse.
//! @param [out] pEllipseAngle  array (allocated by caller) of ellipse angles.
//! @param [in]  point  space point
//! @return the number of projection points
//! 
//!
int ProjectPoint
(
DPoint3dP       cartesianPoints,
double          *pEllipseAngle,
DPoint3dCR      point
) const;

//!
//! @description Project a point onto the xy projection of the (unbounded) ellipse.
//! @remarks May return up to 4 points.
//! @param [out] cartesianPoints  array (allocated by caller) of points on the ellipse.
//! @param [out] pEllipseAngle  array (allocated by caller) of ellipse angles.
//! @param [in] point  space point
//! @return the number of projection points
//! 
//!
int ProjectPointXY
(
DPoint3dP       cartesianPoints,
double          *pEllipseAngle,
DPoint3dCR      point
) const;

//!
//! @description Project a point to the xy projection of the ellipse, and apply sector bounds.
//! @remarks May return up to 4 points.
//! @param [out] cartesianPoints  array (allocated by caller) of points on the ellipse.
//! @param [out] pEllipseAngle  array (allocated by caller) of ellipse angles.
//! @param [in] point  space point
//! @return the number of projection points
//! 
//!
int ProjectPointXYBounded
(
DPoint3dP       cartesianPoints,
double          *pEllipseAngle,
DPoint3dCR      point
) const;

//!
//! @description Project a point to the xy projection of the ellipse, and apply sector bounds.
//! @remarks May return up to 4 points.
//! @param [out] cartesianPoints  array (allocated by caller) of points on the ellipse.
//! @param [out] pEllipseAngle  array (allocated by caller) of ellipse angles.
//! @param [in] point  space point
//! @return the number of projection points
//! 
//!
int ProjectPointBounded
(
DPoint3dP       cartesianPoints,
double          *pEllipseAngle,
DPoint3dCR      point
) const;

//!
//! @description Find the closest point on a bounded ellipse, considering both endpoints and perpendicular
//! projections, and ignoring z of both the ellipse and space point.
//! @param [out] minAngle  angular parameter at closest point
//! @param [out] minDistanceSquared  squared distance to closest point
//! @param [out] minPoint  closest point
//! @param [in] point  space point
//! @return always true
//! 
//!
bool ClosestPointXYBounded
(
double          &minAngle,
double          &minDistanceSquared,
DPoint3dR       minPoint,
DPoint3dCR      point
) const;

//!
//! @description Find the closest point on a bounded ellipse, considering both endpoints and perpendicular
//! projections.
//! @param [out] minAngle  angular parameter at closest point
//! @param [out] minDistanceSquared  squared distance to closest point
//! @param [out] minPoint  closest point
//! @param [in] point  space point
//! @return always true
//! 
//!
bool ClosestPointBounded
(
double          &minAngle,
double          &minDistanceSquared,
DPoint3dR       minPoint,
DPoint3dCR      point
) const;

//!
//! @description Find the intersections of xy projections of an ellipse and line, applying both ellipse and line parameter bounds.
//! @param [out] cartesianPoints cartesian intersection points.
//! @param [out] pLineParams array of line parameters (0=start, 1=end)
//! @param [out] ellipseCoffs array of intersection coordinates in ellipse
//!                                frame.   xy are cosine and sine of angles.
//!                                z is z distance from plane of ellipse.
//! @param [out] pEllipseAngle array of angles in ellipse parameter space.
//! @param [out] pIsTangency true if the returned intersection is a tangency.
//! @param [in] startPoint line start
//! @param [in] endPoint line end
//! @return the number of intersections after applying ellipse and line parameter limits.
//! 
//!
int IntersectXYLineBounded
(
DPoint3dP       cartesianPoints,
double          *pLineParams,
DPoint3dP       ellipseCoffs,
double          *pEllipseAngle,
bool            *pIsTangency,
DPoint3dCR      startPoint,
DPoint3dCR      endPoint
) const;

//!
//! @description Compute area and swept angle as seen from given point.
//! @param [out] area  swept area
//! @param [out] sweepOUT  swept angle (in radians)
//! @param [in] point  base point for sweep line.
//! 
//!
void XySweepProperties
(
double          &area,
double          &sweepOUT,
DPoint3dCR      point
) const;

//!
//! @description Return the (weighted) control points of quadratic beziers which
//!   combine to represent the full conic section.
//!
//! @param [out] poleArray array of poles of composite quadratic Bezier curve.  Array count depends on the angular extent.
//! @param [out] circlePoleArray array of corresponding poles which
//!            map the bezier polynomials back to the unit circle points (x,y,w)
//!            where x^2 + y^2 = w^2, but beware that w is not identically 1!  (Use this to map from bezier parameter back to angles.)
//! @param [out] angleArray array of corresponding angles.
//! @param [out] pNumPole number of poles returned
//! @param [out] pNumSpan number of spans returned.  The pole indices for the first span are (always) 0,1,2; for the second span (if present),
//!                    2,3,4, and so on.
//! @param [in] maxPole maximum number of poles desired.  maxPole must be at least
//!                5.  The circle is split into (maxPole - 1) / 2 spans.
//!                Beware that for 5 poles the circle is split into at most
//!                two spans, and there may be zero weights.   For 7 or more poles
//!                all weights can be positive.  The function may return fewer
//!                poles.
//! 
//!
void QuadricBezierPoles
(
DPoint4dP       poleArray,
DPoint3dP       circlePoleArray,
double          *angleArray,
int             *pNumPole,
int             *pNumSpan,
int             maxPole
) const;


void QuadricBezierPoles
(
DPoint4dP       poleArray,
DPoint3dP       circlePoleArray,
int             *pNumPole,
int             *pNumSpan,
int             maxPole
) const;



//!
//! @description Initialize an ellipse from center, primary axis point, and additional pass-though point.
//! @param [in] centerIN center point of ellipse.
//! @param [in] point0 point to appear at the zero degree point.   The ellipse must pass through
//!                this point as a major or minor axis point, i.e. its tangent must be perpendicular
//!                to the vector from the center to this point.
//! @param [in] point1 additional pass-through point.
//! @return false if center, point0 and point1 are not independent, or if
//!    point1 is too far away from center to allow ellipse constrution.
//! 
//!
bool InitFromCenterMajorAxisPointAndThirdPoint
(
DPoint3dCR      centerIN,
DPoint3dCR      point0,
DPoint3dCR      point1
);

//!
//! @description Return an array of up to 4 points where a ray has closest approach to an ellipse.
//! @remarks Both ellipse and ray are unbounded.
//! @param [out] pEllipseAngleBuffer  array (allocated by caller) to hold 4 angles on ellipse
//! @param [out] pRayFractionBuffer  array (allocated by caller) to hold 4 fractions on ray
//! @param [out] ellipsePointBuffer  array (allocated by caller) to hold 4 ellipse points
//! @param [out] rayPointBuffer  array (allocated by caller) to hold 4 ray points
//! @param [in] ray  ray to search
//! @return number of approach points computed.
//! 
//!
int ClosestApproach
(
double          *pEllipseAngleBuffer,
double          *pRayFractionBuffer,
DPoint3dP       ellipsePointBuffer,
DPoint3dP       rayPointBuffer,
DRay3dCR        ray
) const;

//!
//! @description Fill in ellipse data from data fields in DGN 3d ellipse element.
//! @param [in] centerIN  center of ellipse.
//! @param [in] directionX  vector in the x axis direction.  This is scaled by rX. (It is NOT normalized before
//!                scaling.  In common use, it will be a unit vector.)
//! @param [in] directionY  vector in the y axis direction.  This is scaled by rY. (It is NOT normalized before
//!                scaling.  In common use, it will be a unit vector.)
//! @param [in] rX  scale factor (usually a true distance) for x direction.
//! @param [in] rY  scale factor (usually a true distance) for y direction.
//! @param [in] startAngle start angle
//! @param [in] sweepAngle sweep angle
//! 
//!
void InitFromDGNFields3d
(
DPoint3dCR centerIN,
DVec3dCR directionX,
DVec3dCR directionY,
double          rX,
double          rY,
double          startAngle,
double          sweepAngle
);
//!
//! @description Fill in ellipse data from data fields in DGN 3d ellipse element.
//! @param [in] centerIN  center of ellipse.
//! @param [in] pQuatWXYZ  array of 4 doubles (ordered w,x,y,z) with quaternion for orthogonal frame.
//! @param [in] rX  scale factor (usually a true distance) for x direction.
//! @param [in] rY  scale factor (usually a true distance) for y direction.
//! @param [in] startAngle start angle
//! @param [in] sweepAngle sweep angle
//! 
//!
void InitFromDGNFields3d
(
DPoint3dCR centerIN,
double const*   pQuatWXYZ,
double          rX,
double          rY,
double          startAngle,
double          sweepAngle
);
//!
//! @description Fill in ellipse data from data fields in DGN 2d ellipse element.
//! @param [in] centerIN  center of ellipse.
//! @param [in] direction0  ellipse x axis direction.
//! @param [in] rX  scale factor for ellipse x direction.
//! @param [in] rY  scale factor for ellipse y direction.
//! @param [in] startAngle  start angle.
//! @param [in] sweepAngle  sweep angle.
//! @param [in] zDepth  z value for ellipse.
//! 
//!
void InitFromDGNFields2d
(

DPoint2dCR centerIN,
DVec2dCR direction0,
double          rX,
double          rY,
double          startAngle,
double          sweepAngle,
double          zDepth

);
//!
//! @description Fill in ellipse data from data fields in DGN 2d ellipse element.
//! @param [in] centerIN  center of ellipse.
//! @param [in] xAngle  angle from global x axis to local x axis.
//! @param [in] rX  scale factor for ellipse x direction.
//! @param [in] rY  scale factor for ellipse y direction.
//! @param [in] startAngle  start angle.
//! @param [in] sweepAngle  sweep angle.
//! @param [in] zDepth  z value for ellipse.
//! 
//!
void InitFromDGNFields2d
(
DPoint2dCR centerIN,
double          xAngle,
double          rX,
double          rY,
double          startAngle,
double          sweepAngle,
double          zDepth

);

//!
//! @description Fill in ellipse data from data fields in DGN 3d ellipse element.
//! @param [out] centerOUT  center of ellipse.
//! @param [out] pQuatWXYZ  quaternion for orthogonal frame.
//!            As per DGN convention, ordered WXYZ.
//!            If this is NULL,
//!           major and minor directions must be supplied as pDirection0 and pDirection90;
//! @param [out] directionX  unit vector in ellipse x direction.
//! @param [out] directionY  unit vector in ellipse y direction.
//! @param [out] rx  scale factor (usually a true distance) for x direction.
//! @param [out] ry  scale factor (usually a true distance) for y direction.
//! @param [out] startAngle  start angle.
//! @param [out] sweepAngle  sweep angle.
//! 
//!
void GetDGNFields3d
(
DPoint3dR       centerOUT,
double *        pQuatWXYZ,
DVec3dR         directionX,
DVec3dR         directionY,
double          &rx,
double          &ry,
double          &startAngle,
double          &sweepAngle
) const;

//!
//! 
//!
void GetDGNFields2d
(
DPoint2dR       centerIN,
double          &xAngle,
DVec2dR         direction0,
double          &rx,
double          &ry,
double          &startAngle,
double          &sweepAngle
) const;

//!
//! @param [out] localToGlobal  coordinate frame with origin at lower right of local range.
//! @param [out] globalToLocal  transformation from world to local
//! @param [out] range  ellipse range in the local coordinates.
//! 
//!
bool AlignedRange
(
TransformR      localToGlobal,
TransformR      globalToLocal,
DRange3dR       range
) const;

//!
//! @return largest (absolute) coordinate or vector component.
//!
double MaxAbs () const;

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
//! @param [in] extend0 true to allow extension before start.
//! @param [in] extend1 true to allow extension after end.
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
//! Note that an ellipse with zero sweephas zero length but the (single) coordinate is a well defined centroid.
//! @param [out] length length.
//! @param [out] centroid centroid point.
//! @param [in] fraction0 start fraction of active part of sweep.
//! @param [in] fraction1 end fraction of active part of sweep.
void WireCentroid
(
double     &length,
DPoint3dR   centroid,
double fraction0 = 0.0,
double fraction1 = 1.0
) const;

//! Construct (up to 4) ellipses defined by known primary radii,
//! primary axis point, and another edge point.
//! @param [out] ellipses computed ellipses.
//! @param [in] a x axis radius.
//! @param [in] b y axis radius.
//! @param [in] xPoint x axis point
//! @param [in] edgePoint any other point on the ellipse.
static void Construct_XRadius_YRadius_XPoint_EdgePoint
(
bvector<DEllipse3d>&ellipses,
double a,
double b,
DPoint3dCR xPoint,
DPoint3dCR edgePoint
);

//! Construct (up to 3) ellipses defined by known primary radius,
//! primary axis point, and two other edge points
//! @param [out] ellipses computed ellipses.
//! @param [in] xPoint x axis point
//! @param [in] edgePoint0 any other point on the ellipse.
//! @param [in] edgePoint1 any other point on the ellipse.
//! @param [in] a x axis radius.
static void Construct_XPoint_EdgePoint_EdgePoint_XRadius (
bvector<DEllipse3d> &ellipses,
DPoint3dCR xPoint,
DPoint3dCR edgePoint0,
DPoint3dCR edgePoint1,
double a
);



//! Construct an ellipse (it is unique if it exists) with given x axis, start point, x axis point, and end point.
//! @param [out] ellipse constructed ellipse.
//! @param [in] edgePoint0 start point
//! @param [in] xPoint x axis extrema
//! @param [in] edgePoint1 end point
//! @param [in] xAngle angle from global x axis to ellipse x axis
static bool TryConstruct_EdgePoint_XPoint_EdgePoint_XAngle
(
DEllipse3dR ellipse,
DPoint3dCR edgePoint0,
DPoint3dCR xPoint,
DPoint3dCR edgePoint1,
double xAngle
);

//! Construct circular arc(s) with given start point and tangent, tangent to a given ray.
//! @param [out] ellipse constructed ellipses.
//! @param [out] fractionB parameters on the ray.
//! @param [in] pointA start ponit
//! @param [in] directionA start tangent
//! @param [in] rayB ray for tangency.  The tangency can occur anywhere on the ray.
static void Construct_Point_Direction_TangentXY
(
bvector<DEllipse3d> &ellipse,
bvector<double> &fractionB,
DPoint3dCR pointA,
DVec3dCR   directionA,
DRay3d     rayB
);

//! Construct circular arc(s) with given start point and tangent, tangent to a given circle
//! @param [out] ellipse constructed ellipses.
//! @param [in] pointA start ponit
//! @param [in] directionA start tangent
//! @param [in] tangentCircleCenter center of circle for tangnecy
//! @param [in] tangentCircleRadius radius of circle for tangnecy
static void Construct_Point_Direction_TangentToCircleXY
(
bvector<DEllipse3d> &ellipse,
DPoint3dCR pointA,
DVec3dCR   directionA,
DPoint3dCR tangentCircleCenter,
double     tangentCircleRadius
);

//! Search an array of ellipses for the one whose point at specified faction is closest to the searchPoint
//! return (a copy of) the closest ellipse.
//! 
static ValidatedDEllipse3d ClosestEllipse
(
bvector<DEllipse3d> const &ellipses,        //!< [in] ellipses to search.
double fraction,                            //!< [in] fraction to evaluate ellipse for distance test
DPoint3dCR searchPoint                      //!< [in] search point.
);

//! Construct (if possible) two arcs that start at pointA with tangentA, join with tangent continuity, and end at pointB with tangentB.
static bool Construct_Biarcs
(
DEllipse3dR arcA,           //!< [out] First constructed arc
DEllipse3dR arcB,           //!< [out] Second constructed arc
DPoint3dCR pointA,          //!< [in] start point
DVec3dCR  tangentA,         //!< [in] INBOUND (forward) tangent at start
DPoint3dCR pointB,          //!< [in[ end point
DVec3dCR  tangentB          //!< [in] OUTBOUND (forward) tangent at end point
);

//! construct fractional coordinates of lines tangent to two circles.
static bool ConstructTangentLineRatios
(
double centerToCenterDistance,  //!< [in] distance between centers
double radiusA,                 //!< [in] radius of first circle.
double radiusB,                 //!< [in] radius of second circle
bool outerTangents,             //!< [in] true for tangents from outside to ouside, false for tangents that cross between centers
DPoint2dR uvA,                  //!< [in] fractional coordinates of tangency point on circle A, for use in DPoint3d::FromInterpolateAndPerpendicularXY 
DPoint2dR uvB                  //!< [in] fractional coordinates of tangency point on circle B, for use in DPoint3d::FromInterpolateAndPerpendicularXY 
);

//! Construct an arc, line and arc to depart from pointA with directionA and radiusA, and arrive at pointB with directionB and radiusB
static bool Construct_ArcLineArc_PointTangentRadius_PointTangentRadiusXY
(
DPoint3dCR pointA,          //!< [in] start point
DVec3dCR   directionA,      //!< [in] direction at pointA
double     radiusA,         //!< [in] radius at A.  Positive is left turn
DPoint3dCR pointB,          //!< [in] end point
DVec3dCR   directionB,      //!< [in] direction at pointB
double     radiusB,         //!< [in] radius at pointB.  Positive is left turn
DEllipse3dR arcA,           //!< [out] departure arc
DSegment3dR tangentSegment, //!< [out] joining segment, tangent at both ends
DEllipse3dR arcB            //!< [out] arrival arc
);

//! Construct an ellipse that is retains one end point, the tangent at that end, and the sweep angle
//! of an existing ellipse, but mvoes the other end by a vector.
static ValidatedDEllipse3d FromEndPointTranslation
(
DEllipse3dCR source,    //!< [in] existing arc
DVec3dCR translation,   //!< [in] translation to apply to one end
int movingEndIndex      //!< [in] 0 to move start, 1 to move end
);
};

/*----------------------------------------------------------------------+
|FUNC           bsiEllipse_componentRange       RK      06/96           |
| Conditional update range of one component of an ellipse.              |
|NORET                                                                  |
+----------------------------------------------------------------------*/
Public GEOMDLLIMPEXP void    bsiEllipse_componentRange
(
double*         minP,           /* IN OUT  min coordiante of range box */
double*         maxP,           /* IN OUT  max coordinate of range box */
double          centComponent,  /* IN      center component */
double          cosComponent,   /* IN      basis vector component multiplied by cos */
double          sinComponent,   /* IN      basis vector component multiplied by sin */
double          theta0,         /* IN      start angle */
double          sweep           /* IN      sweep angle. */
);

END_BENTLEY_GEOMETRY_NAMESPACE
