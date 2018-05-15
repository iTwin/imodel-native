BEGIN_BENTLEY_GEOMETRY_NAMESPACE

/*-----------------------------------------------------------------*//**
* @description Store origin and unnormalized vector.
* @param pPlane <= initialized plane.
* @param pOrigin => origin point
* @param pNormal => normal vector
* @group "DPlane3d Initialization"
* @bsihdr                                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public  void bsiDPlane3d_initFromOriginAndNormal

(
DPlane3dP pPlane,
DPoint3dCP pOrigin,
DVec3dCP pNormal
);

/*-----------------------------------------------------------------*//**
* @description Return the plane as a DPoint4d.
* @param pPlane => plane structure with origin, normal
* @param pHPlane <= 4D plane coefficients
* @see bsiDPlane3d_initFromDPoint4d
* @group "DPlane3d Queries"
* @bsihdr                                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public  void      bsiDPlane3d_getDPoint4d

(
DPlane3dCP pPlane,
DPoint4dP pHPlane
);

/*-----------------------------------------------------------------*//**
* @description Convert the plane to implicit coeffcients ax+by+cz=d.
* @remarks WARNING: Check your usage.  It is about equally common to write the plane equation with
*       negated d, i.e. ax+by+cz+d=0.  If so, pass in (a,b,c,-d).
* @param pPlane => plane structure with origin, normal
* @param pA <= 4D plane x-coefficient
* @param pB <= 4D plane y-coefficient
* @param pC <= 4D plane z-coefficient
* @param pD <= 4D plane constant coefficient
* @see bsiDPlane3d_initFromImplicitPlaneCoefficients
* @group "DPlane3d Initialization"
* @bsihdr                                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public  void bsiDPlane3d_getImplicitPlaneCoefficients

(
DPlane3dCP pPlane,
double      *pA,
double      *pB,
double      *pC,
double      *pD
);

/*---------------------------------------------------------------------------------**//**
* Get the parameter range as start/sweep pairs.
* @indexVerb parameterRange
* @bsihdr                                                       EarlinLutz      03/99
+---------------+---------------+---------------+---------------+---------------+------*/
Public  void    bsiDDisk3d_getScalarNaturalParameterSweep

(
DDisk3dCP pInstance,
double          *pRadius0,
double          *pRadiusDelta,
double          *pAngle0,
double          *pAngleSweep
);

/*---------------------------------------------------------------------------------**//**
* @description Set all fields of the cone from arguments.
* @param pCone <= cone to initialize
* @param pFrame => Transformation whose translation, x, y, and z columns are the
*                   center, 0-degree vector, 90-degree vector, and axis direction.
*                   if <code>null</code>, an identity is used.
* @param radiusFraction => top circle radius divided by bottom circle radius.
* @param pRange     => parameter range in <code>(theta, z)</code> coordinates.
* @group "DCone3d Initialization"
* @bsihdr                                                       EarlinLutz      03/99
+---------------+---------------+---------------+---------------+---------------+------*/
Public  void    bsiDCone3d_setFrameAndFraction
(
DCone3dP pCone,
TransformCP pFrame,
double       radiusFraction,
DRange2dCP pRange
);

/*---------------------------------------------------------------------------------**//**
* @description Compute angles for silhouettes of the cone with respect to a (possibly perspective) view transformation.
* @param pCone  => cone to evaluate
* @param pTrigPoint <= array where x,y are cosine, sine of
*                      silhouette angles. z is zero -- maybe a convenient
*                      place for the angles if you need to stash them
* @param pMap       => view transformation
* @param pEyePoint => eyepoint, in same coordinates.
*                     For perspective, from xyz, set w=1
*                     For flat view in direction xyz, set w=0
* @return number of silhouette angles.
* @group "DCone3d Silhouette"
* @bsihdr                                                       EarlinLutz      03/99
+---------------+---------------+---------------+---------------+------*/
Public  int      bsiDCone3d_silhouetteAngles

(
DCone3dCP pCone,
DPoint3dP pTrigPoint,
DMap4dCP pMap,
DPoint4dCP pEyePoint
);

/*---------------------------------------------------------------------------------**//**
* @description Return a rule line at specified longitude (angle around cone).
* to the z range
* @param pCone  => cone to evaluate
* @param pSegment  <= ruling segment.
* @param theta => longitude angle (radians)
* @return true if theta is within the parameter range for the cone.
* @group "DCone3d Rule Lines"
* @bsihdr                                                       EarlinLutz      03/99
+---------------+---------------+---------------+---------------+---------------+------*/
Public  bool     bsiDCone3d_getRuleLine

(
DCone3dCP pCone,
DSegment3dP pSegment,
double          theta
);

/*---------------------------------------------------------------------------------**//**
* @description Return the cone's local radius at the specified local height.
* @param pCone  => cone to evaluate
* @param z => height in local coordinates
* @return the cone's radius in local coordinates
* @group "DCone3d Local Coordinates"
* @bsihdr                                                       EarlinLutz      03/99
+---------------+---------------+---------------+---------------+------*/
Public  double   bsiDCone3d_heightToRadius

(
DCone3dCP pCone,
double          z
);

/*---------------------------------------------------------------------------------**//**
* @description Get the parameter range as start/sweep pairs.
* @param pCone      => cone to evaluate
* @param pTheta0        <= start angle
* @param pThetaSweep    <= angle sweep
* @param pZ0            <= start altitude
* @param pZSweep        <= altitude sweep
* @group "DCone3d Parameter Range"
* @bsihdr                                                       EarlinLutz      03/99
+---------------+---------------+---------------+---------------+---------------+------*/
Public  void    bsiDCone3d_getScalarNaturalParameterSweep

(
DCone3dCP pCone,
double          *pTheta0,
double          *pThetaSweep,
double          *pZ0,
double          *pZSweep
);

/*---------------------------------------------------------------------------------**//**
* @description Return the range of the natural parameter for the active surface patch.
* @param pCone  => cone to evaluate
* @param pParam1Start => start value of natural parameter.
* @param pParam1End   => end value of natural parameter.
* @param pParam2Start => start value of natural parameter.
* @param pParam2End   => end value of natural parameter.
* @group "DCone3d Parameter Range"
* @bsihdr                                                       EarlinLutz      03/99
+---------------+---------------+---------------+---------------+---------------+------*/
Public  void     bsiDCone3d_getScalarNaturalParameterRange

(
DCone3dCP pCone,
double    *pParam1Start,
double    *pParam1End,
double    *pParam2Start,
double    *pParam2End
);

/*---------------------------------------------------------------------------------**//**
* Get the parameter range as start/sweep pairs.
* @indexVerb parameterRange
* @bsihdr                                                       EarlinLutz      03/99
+---------------+---------------+---------------+---------------+---------------+------*/
Public  void    bsiDEllipsoid3d_getScalarNaturalParameterSweep

(
DEllipsoid3dCP pInstance,
double          *pTheta0,
double          *pThetaSweep,
double          *pPhi0,
double          *pPhiSweep
);

/*---------------------------------------------------------------------------------**//**
* @description Set all fields of the cone from arguments.
* @param pCone <= cone to initialize
* @param pFrame => Transformation whose translation, x, y, and z columns are the
*                   center, 0-degree vector, 90-degree vector, and axis direction.
*                   if <code>null</code>, an identity is used.
* @param radiusFraction => top circle radius divided by bottom circle radius.
* @param pRange     => parameter range in <code>(theta, phi)</code> coordinates.
* @group "DCone3d Initialization"
* @bsihdr                                                       EarlinLutz      03/99
+---------------+---------------+---------------+---------------+---------------+------*/
Public  void    bsiDCone3d_set

(
DCone3dP pCone,
TransformCP pFrame,
double       radiusFraction,
DRange2dCP pRange
);


/*-----------------------------------------------------------------*//**
* Search an array for the closest point, using only
* x and y components.  Useful for screenproximity
* tests between points at different depths.
*
* @param pPoint => fixed point for tests
* @param pArray => array of test points
* @param nPoint => number of points
* @see
* @return index of closest point
* @bsihdr                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public  int bsiGeom_closestXYDPoint4d


(
DPoint3dCP pPoint,
DPoint4dCP pArray,
int             nPoint
);

/*----------------------------------------------------------------------------+
| BARYCENTRIC COORDINATE FUNCTIONS:
|
| For a given triangle T with vertices v0, v1, v2, every point q in the plane
| of T is uniquely represented by its barycentric coordinates (b0, b1, b2)
| relative to T:
|
| q = b0 * v0 + b1 * v1 + b2 * v2,
| 1 = b0 + b1 + b2.
|
+----------------------------------------------------------------------------*/


Public bool    bsiDPoint3d_barycentricFromDPoint2dTriangleVectors

(
DPoint3dP pInstance,
double    *pArea,
DPoint2dCP pPoint,
DPoint2dCP pOrigin,
DPoint2dCP pVector1,
DPoint2dCP pVector2
);

/*-----------------------------------------------------------------*//**
* @description Sets this instance to the barycentric coordinates of pPoint
* relative to the triangle (pVertex0, pVertex1, pVertex2) in the xy-plane.
*
* @instance pInstance   <= barycentric coordinates of pPoint relative to T
* @param pPoint         => point in plane
* @param pVertex0       => vertex 0 of triangle T
* @param pVertex1       => vertex 1 of triangle T
* @param pVertex2       => vertex 2 of triangle T
* @see bsiDPoint3d_barycentricFromDPoint2dTriangleVectors
* @see bsiDPoint2d_fromBarycentricAndDPoint2dTriangle
* @group "DPoint3d Barycentric"
* @return true if and only if the area of T is sufficiently large.
* @bsihdr                                                                       DavidAssaf      10/98
+---------------+---------------+---------------+---------------+------*/
Public  bool    bsiDPoint3d_barycentricFromDPoint2dTriangle

(
DPoint3dP pInstance,
DPoint2dCP pPoint,
DPoint2dCP pVertex0,
DPoint2dCP pVertex1,
DPoint2dCP pVertex2
);

/*-----------------------------------------------------------------*//**
* @description Sets this instance to the barycentric coordinates of pPoint
* relative to the triangle (pVertex0, pVertex1, pVertex2) in the xy-plane.
*
* @instance pInstance   <= barycentric coordinates of pPoint relative to T
* @param pPoint         => point in plane
* @param pVertex0       => vertex 0 of triangle T
* @param pVertex1       => vertex 1 of triangle T
* @param pVertex2       => vertex 2 of triangle T
* @see bsiDPoint3d_barycentricFromDPoint2dTriangleVectors
* @see bsiDPoint2d_fromBarycentricAndDPoint2dTriangle
* @group "DPoint3d Barycentric"
* @return true if and only if the area of T is sufficiently large.
* @bsihdr                                                                       DavidAssaf      10/98
+---------------+---------------+---------------+---------------+------*/
Public  bool    bsiDPoint3d_barycentricFromDPoint2dTriangle
(
DPoint3dR uvw,
DPoint3dR dUVWdX,
DPoint3dR dUVWdY,
double   &area,
DPoint2dCR point,
DPoint2dCR vertex0,
DPoint2dCR vertex1,
DPoint2dCR vertex2
);

/*-----------------------------------------------------------------*//**
* Given a space point spacePontP, finds the closest point on the plane
* containing the 3 points in pPlanePoint.  Stores the closest point
* coordinates in pClosePoint, and the s and t coordinates (as defined
* in bsiGeom_evaluateSkewedPlane) in sP and tP.
*
* @param pClosePoint <= point on plane.  May be null pointer
* @param sP <= parametric coordinate on s axis
* @param tP <= parametric coordinate on t axis
* @param pPlanePoint => origin, s=1, and t=1 points
* @param pSpacePoint => point to be projected
* @return true unless the plane points are collinear
* @bsihdr                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public  bool     bsiGeom_closestPointOnSkewedPlane

(
DPoint3dP pClosePoint,
double          *sP,
double          *tP,
DPoint3dCP pPlanePoint,
DPoint3dCP pSpacePoint
);

/*-----------------------------------------------------------------*//**
* @description Compute the minimum distance from a point to a triangle.
* @instance pSpacePoint   <= point in space
* @param pVertex0       => vertex of T
* @param pVertex1       => vertex of T
* @param pVertex2       => vertex of T
* @param pClosePoint    <= projection of space point onto plane of triangle
* @param pBCoords       <= barycentric coordinates of closest point
* @return minimum distance
* @group "DPoint3d Barycentric"
* @bsihdr                                       EarlinLutz      10/04
+---------------+---------------+---------------+---------------+------*/
Public  double bsiDPoint3d_minDistToTriangle

(
DPoint3dCP pSpacePoint,
DPoint3dCP pVertex0,
DPoint3dCP pVertex1,
DPoint3dCP pVertex2,
DPoint3dP pClosePoint,
DPoint3dP pBoundedUVW,
DPoint3dP pUnboundedUVW
);

/*-----------------------------------------------------------------*//**
* @description Applies transformation to simplify the problem of finding the barycentric
* coordinates of a 3d point relative to a triangle.  Returned are the
* components of the new 2d problem.
*
* @instance pInstance   => point to find barycentric coords of
* @param pNewPoint      <= point in plane of new triangle with same barycoords
* @param pNewVector1    <= side of new triangle
* @param pNewVector2    <= side of new triangle
* @param pVector1       => side of old triangle
* @param pVector2       => side of old triangle
* @group "DPoint3d Barycentric"
* @bsihdr                                                                       DavidAssaf      10/98
+---------------+---------------+---------------+---------------+------*/
Public void transformBarycentric3dSystemTo2d

(
DPoint3dCP pInstance,
DPoint2dP pNewPoint,
DPoint2dP pNewVector1,
DPoint2dP pNewVector2,
DPoint3dCP pVector1,
DPoint3dCP pVector2
);


/*-----------------------------------------------------------------*//**
* @description Sets this instance to the barycentric coordinates of pPoint
* relative to the triangle (pVertex0, pVertex1, pVertex2) in space.
* Points p and r in space have the same barycentric coordinates relative to
* T if and only if they project to the same point q in the plane of T;
* then their barycentric coordinates relative to T are those of q.
*
* @instance pInstance   <= barycentric coordinates of pPoint relative to T
* @param pPoint         => point in space
* @param pVertex0       => vertex of triangle T
* @param pVertex1       => vertex of triangle T
* @param pVertex2       => vertex of triangle T
* @see bsiDPoint3d_barycentricFromDPoint2dTriangle
* @see bsiDPoint3d_fromBarycentricAndDPoint3dTriangle
* @return true if and only if the area of T is sufficiently large.
* @group "DPoint3d Barycentric"
* @bsihdr                                                                       DavidAssaf      10/98
+---------------+---------------+---------------+---------------+------*/
Public  bool    bsiDPoint3d_barycentricFromDPoint3dTriangle

(
DPoint3dP pInstance,
DPoint3dCP pPoint,
DPoint3dCP pVertex0,
DPoint3dCP pVertex1,
DPoint3dCP pVertex2
);
/*-----------------------------------------------------------------*//**
* @description Sets this instance to the barycentric coordinates of pPoint
* relative to the triangle (pOrigin, pVector1-pOrigin, pVector2-pOrigin)
* in the xy-plane.
*
* @instance pInstance   <= barycentric coordinates of pPoint relative to T
* @param pArea          <= area of triangle.
* @param pPoint         => point in plane
* @param pOrigin        => vertex of triangle T (may be null for origin)
* @param pVector1       => side vector of T (emanating from pOrigin)
* @param pVector2       => side vector of T (emanating from pOrigin)
* @see bsiDPoint3d_barycentricFromDPoint2dTriangle
* @see bsiDPoint2d_fromBarycentricAndDPoint2dTriangleVectors
* @group "DPoint3d Barycentric"
* @return true if and only if the area of T is sufficiently large.
* @bsihdr                                      DavidAssaf      10/98
+---------------+---------------+---------------+---------------+------*/
Public  bool    bsiDPoint3d_barycentricFromDPoint2dTriangleVectors

(
DPoint3dP pInstance,
DPoint2dCP pPoint,
DPoint2dCP pOrigin,
DPoint2dCP pVector1,
DPoint2dCP pVector2
);

/*-----------------------------------------------------------------*//**
* @description Sets this instance to the point in the plane with the given barycentric
* coordinates relative to triangle T (pVertex0, pVertex1, pVertex2).
*
* @instance pInstance   <= point with given barycoords relative to T
* @param pBaryCoords    => barycentric coordinates relative to T
* @param pVertex0       => vertex 0 of triangle T
* @param pVertex1       => vertex 1 of triangle T
* @param pVertex2       => vertex 2 of triangle T
* @see bsiDPoint3d_barycentricFromDPoint2dTriangle
* @group "DPoint2d Barycentric"
* @bsihdr                                                               DavidAssaf      10/98
+---------------+---------------+---------------+---------------+------*/
Public  void bsiDPoint2d_fromBarycentricAndDPoint2dTriangle

(
DPoint2dP pInstance,
DPoint3dCP pBaryCoords,
DPoint2dCP pVertex0,
DPoint2dCP pVertex1,
DPoint2dCP pVertex2
);


/*-----------------------------------------------------------------*//**
* @description Dot the plane normal with the vector from the plane origin to the point.
* @remarks If the plane normal is a unit vector, this is the true distance from the
*       plane to the point.  If not, it is a scaled distance.
* @param pPlane => plane to evaluate
* @param pPoint => point for evaluation
* @return dot product
* @group "DPlane3d Projection"
* @bsihdr                                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public  double  bsiDPlane3d_evaluate

(
DPlane3dCP pPlane,
DPoint3dCP pPoint
);

/*---------------------------------------------------------------------------------**//**
*
* Add a point to the array.
*
* @param    pInstance <=> header to receive new point
* @param pPoint => point being added.
* @return true unless rubber array could not be extended.
* @bsihdr                                       EarlinLutz      11/98
+---------------+---------------+---------------+---------------+---------------+------*/
Public  bool     jmdlGraphicsPointArray_addDPoint3d
(
GraphicsPointArrayP pInstance,
const DPoint3d              *pPoint
);

/*---------------------------------------------------------------------------------**//**
* Mark the break between disconnected line segments.
* @bsihdr                                       EarlinLutz      11/98
+---------------+---------------+---------------+---------------+---------------+------*/
Public  void jmdlGraphicsPointArray_markBreak
(
GraphicsPointArrayP         pInstance
);

/*-----------------------------------------------------------------*//**
* Get the i'th sector angular range
* @instance pEllipse => ellipse whose angular range is queried.
* @param pStartAngle <= start angle
* @param pEndAngle <= end angle
* @param i => sector to read
* @see
* @return true if sector index is valid.
* @indexVerb
* @bsihdr                                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public  bool    bsiDEllipse4d_getSector

(
DEllipse4dCP pEllipse,
double    *pStartAngle,
double    *pEndAngle,
int             i
);

/*-----------------------------------------------------------------*//**
* Compute the transfer matrix to normalize a weighted, uncentered
* ellipse into a centered cartesian ellipse.

* @param pMatrix <= transfer matrix
* @param pInverse <= its inverse.   Pass NULL if not needed.
* @param w0 => cosine weight
* @param w90 => sine weight
* @param wCenter => center weight
* @see
* @return true if weights define an angle change.
* @indexVerb
* @bsihdr                                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public  bool    bsiEllipse_angularTransferMatrix

(
RotMatrixP pMatrix,
RotMatrixP pInverse,
double          w0,
double          w90,
double          wCenter
);

/*-----------------------------------------------------------------*//**
* Let F = [cos(theta), sin(theta), 1+alpha*cos(theta)+beta*sin(theta)]
*     G = [cos(phi), sin(phi), 1]
* and G = M*F   (possibly scaled?)
* Return the phi corresponding to theta.

* @param theta => known angle prior to transform
* @param pMatrix => transfer matrix.
* @param  => matrix M
* @param alpha => cosine coefficient
* @param beta => sine coefficient
* @see
* @return modified angle
* @indexVerb
* @bsihdr                                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public  double bsiDEllipse4d_transferAngle

(
double          theta,
RotMatrixP pMatrix,

double          alpha,
double          beta
);

/*-----------------------------------------------------------------*//**
* Let F = [cos(theta), sin(theta), 1+alpha*cos(theta)+beta*sin(theta)]
*     G = [cos(phi), sin(phi), 1]
* and G = M*F   (possibly scaled?)
* Replace all angles (theta) in an ellispe's stroke intervals by
* corresponding phi angles.

* @param pDest <=> Ellipse whose angles are corrected.
* @param pSource => source of angle data
* @param pMatrix => matrix M
* @param alpha => cosine coefficient
* @param beta => sine coefficient
* @see
* @indexVerb
* @bsihdr                                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public  bool    bsiDEllipse4d_transferAngles

(
DEllipse4dP pDest,
DEllipse4dCP pSource,
RotMatrixP pMatrix,
double          alpha,
double          beta
);


/*-----------------------------------------------------------------*//**
*
* Find new basis vectors with 0 weights on the U and V vectors, and unit
* on the C vector.  This computation is not possible if the curve is
* a hyperbola or parabola when projected to 3D.
*
* @instance pNormalized <= normalized form
* @param pWeighted => unnormalized form
* @see
* @return true if the curve really is an ellipse (i.e. not hyperbola or parabola)
* @indexVerb
* @bsihdr                                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public  bool    bsiDEllipse4d_normalizeWeights

(
DEllipse4dP pNormalized,
DEllipse4dCP pWeighted
);
/*-----------------------------------------------------------------*//**
* Computes the silhouette ellipse of an ellipsoid under arbitrary
* DMap4d and viewpoint.
*
* @param pHEllipse <= silhouette ellipse/parabola/hyperbola
* @param pEllipsoidPoint => 4 defining points of the ellipsoid
* @param pHMap => further mapping
* @param pEyePoint => eyepoint
* @return false iff the eyeponit is inside the ellipsoid.
* @bsihdr                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public  bool    bsiGeom_ellipsoidSilhouette

(
DEllipse4dP pHEllipse,
DPoint3dCP pEllipsoidPoint,
DMap4dCP pHMap,
DPoint4dCP pEyePoint
);

/*-----------------------------------------------------------------*//**
@nodoc DEllipse4d
@description Convert a homogeneous ellipse to cartesian.  Callers should beware of the following
 significant points:
<UL>
<LI>A homogeneous "ellipse" may appear as a hyperbola or parabola in xyz space.
   Hence the conversion can fail.
<LI>When the conversion succeeds, it is still a Very Bad Thing To Do numerically
   because a homogeneous ellipse with "nice" numbers can have very large center and axis
   coordinates.   It is always preferable to do calculations directly on the homogeneous
   ellipse if possible.
<LI>When the conversion succeeds, the axis may be non-perpendicular.  A subsequent call
   may be made to initWithPerpendicularAxes to correct this.
</UL>
 @param pEllipse <= initialized ellipse
 @param pSource => homogeneous ellipse
 @param sector  => angular sector index.  If out of bounds, a full ellipse is created.
 @return true if homogeneous parts allow reduction to simple ellipse. (false if the homogeneous
    parts are a parabola or hyperbola.)
 @group "DEllipse3d Initialization"
 @bsimethod                                                     EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public  bool     bsiDEllipse3d_initFromDEllipse4d

(
DEllipse3dP pEllipse,
DEllipse4dCP pSource,
int           sector
);

/*-----------------------------------------------------------------*//**
* This routine will find the intersection between a general conic
* and a unit circle. The conic is in the form of:
* x = centerx + ux * cos(theta) + vx*sin(theta)
* y = centery + uy * cos(theta) + vy*sin(theta)
* w = centerw + uw * cos(theta) + vw*sin(theta)
*   where centerx, centery, centerw, ux, uy, uw, vx, vy, vw are constants
*   and    PI < = theta < = PI
* A unit circle is x^2 + Y^2 = 1
* Return values: number of solutions found.
*               0: no intersection
*               -1: input error or polynomial solver failed.
*
* @param pCosValue <= 0 to 4 cosine values
* @param pSinValue <= 0 to 4 sine values
* @param pThetaValue <= 0 to 4 angle values
* @param pNumInt <= number of intersections
* @param centerx
* @param ux
* @param vx
* @param centery
* @param uy
* @param vy
* @param cenerw
* @param uw
* @param vw
* @return -1 if the conic is (exactly) a unit circle,
*               else number of intersections.
* @bsihdr                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public  int bsiMath_conicIntersectUnitCircle

(
double          *pCosValue,
double          *pSinValue,
double          *pThetaValue,
int             *pNumInt,
double          centerx,
double          ux,
double          vx,
double          centery,
double          uy,
double          vy,
double          centerw,
double          uw,
double          vw
);

/*-----------------------------------------------------------------*//**
* Compute B so X'BX = X'AX and B is symmetric.

* @param pA <= symmetric coefficients
* @param pB => nonsymmetric coefficients
* @see
* @bsihdr                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public  void bsiQCoff_symmetrize

(
RotMatrixP pA,
RotMatrixCP pB
);

/*-----------------------------------------------------------------*//**
* Compute a matrix A such that
*   A*(c s 1)' = H * B where
*  H is the matrix
* [ 0 -1 -s][bx]        [c]
* [ 1  0  c][by] == A * [s]
* [ s -c  0][bz]        [1]

* @param pA <= coefficient matrix
* @param pVecB => vector
* @see
* @bsihdr                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public  void bsiQCoff_HOperator

(
RotMatrixP   pA,
DPoint3dCP pVecB
);

/*-----------------------------------------------------------------*//**
* Compute the matrix of a quadric section whose intersections with
* the unit circle are the cosine and sine of the angles where pPoint
* projects to the quadric.
* That is,
*   A = sym(D*B'* (I - QW') * B)
* Where sym is the symmetrizing operator and  B, D, Q, and W are things
* that need some explanation.

* @param pA <= matrix of new quadric section
* @param pB => matrix of existing quadric section
* @param pPoint => point being projected to matrix B.
* @param pPoint
* @see
* @bsihdr                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public  void bsiQCoff_projectToEllipse

(
RotMatrixP pA,
RotMatrixCP pB,
DPoint3dCP pPoint
);
/*-----------------------------------------------------------------*//**
* This routine finds the points of intersection between an implicit
* conic (specified by matrix A) X^AX = 0  and the unit circle
* x^2 + Y^2 = 1
* Returns  : number of intersections found.
*            -1: conic = circle or input error or polynomial solver failed.
*
* @param pCosValue <= x coordinates of intersections
* @param pSinValue <= y coordinates of intersections
* @param pThetaValue <= angular positions of intersections
* @param pNumInt <= number of intersections
* @param pCoefficientMatrix => matrix defining implicit conic
* @return 0 if success, nonzero if error
* @bsihdr                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public  StatusInt bsiMath_implicitConicIntersectUnitCircle

(
double          *pCosValue,
double    *pSinValue,
double    *pThetaValue,
int       *pNumInt,
RotMatrixCP pCoefficientMatrix
);

/*-----------------------------------------------------------------*//**
*
* Initializes a range cube with (inverted) large positive and negative
* values.
*
* @param
* @see
* @indexVerb init
* @bsihdr                                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public  void     bsiDRange2d_init

(
DRange2dP pRange        /* <= range to be initialized */
);

/*-----------------------------------------------------------------*//**
*
* Initizlizes the range to contain the range of the given array of points.
* If there are no points in the array, the range is initialized by
* DRange2d.init()
*
* @param pPoint => array of points to search
* @param n => number of points in array
* @see
* @indexVerb init
* @bsihdr                                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public  void     bsiDRange2d_initFromArray

(
DRange2dP pRange,
DPoint2dCP pPoint,
int             n
);

/*-----------------------------------------------------------------*//**
*
* @return the largest individual coordinate value among (a) range min point,
* (b) range max point, and (c) range diagonal vector.
* @see
* @indexVerb extrema
* @bsihdr                                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public  double bsiDRange2d_getLargestCoordinate

(
DRange2dCP pRange
);

/*-----------------------------------------------------------------*//**
*
* Compute the intersection of a range cube and a ray.
*
* If there is not a finite intersection, both params are set to 0 and
* and both points to pPoint0.
*
* @param pParam0 <= ray parameter where cube is entered
* @param pParam1 <= ray parameter where cube is left
* @param pPoint0 <= entry point
* @param pPoint1 <= exit point
* @param pStart => start point of ray
* @param pDirection => direction of ray
* @return true if non-empty intersection.
* @see
* @indexVerb intersect
* @bsihdr                                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public  bool        bsiDRange2d_intersectRay

(
DRange2dCP pRange,
double      *pParam0,
double      *pParam1,
DPoint2dP pPoint0,
DPoint2dP pPoint1,
DPoint2dCP pStart,
DPoint2dCP pDirection
);


/*-----------------------------------------------------------------*//**
* Project a point to a plane defined by origin and (not necessarily unit)
* normal vector.
*
* @param pOutPoint <= projected point (or NULL)
* @param pInPoint  => point to project to plane
* @param pNormal   => plane normal
* @param pOrigin   => plane origin
* @return signed distance from point to plane.  If the plane normal has zero length,
*           distance to plane origin.
* @bsihdr                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public  double bsiDPoint3d_distancePointToPlane

(
DPoint3dP pOutPoint,
DPoint3dCP pInPoint,
DPoint3dCP pNormal,
DPoint3dCP pOrigin
);
/*-----------------------------------------------------------------*//**
* @description Compute the intersection point of a line and a plane.
*
* @param pParam <= intersection parameter within line
* @param pPoint <= intersection point
* @param pLineStart => point on line at parameter 0.0
* @param pLineEnd => point on line at parameter 1.0
* @param pOrigin => any point on plane
* @param pNormal => normal vector for plane
* @return true unless the line is parallel to the plane.
* @bsihdr                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public  bool    bsiGeom_linePlaneIntersection

(
double      *pParam,
DPoint3dP pPoint,
DPoint3dCP pLineStart,
DPoint3dCP pLineEnd,
DPoint3dCP pOrigin,
DPoint3dCP pNormal
);



END_BENTLEY_GEOMETRY_NAMESPACE
