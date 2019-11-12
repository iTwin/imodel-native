/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once
/*__PUBLISH_SECTION_START__*/


BEGIN_BENTLEY_GEOMETRY_NAMESPACE
/**
4d point coordinates.
@ingroup GROUP_Geometry
*/
struct GEOMDLLIMPEXP DPoint4d
{
//! x coordinate
double  x;
//! y coordinate
double  y;
//! z coordinate
double  z;
//! w coordinate.  1 is a simple point. 0 is a vector.   For other values, dividing through by w gives the simple point.
double  w;

#ifdef __cplusplus


/*__PUBLISH_SECTION_END__*/
//BEGIN_REFMETHODS
/*__PUBLISH_SECTION_START__*/

//! Return point with direct initialization
//! @param [in] x x coordinate
//! @param [in] y y coordinate
//! @param [in] z z coordinate
//! @param [in] w z coordinate
static DPoint4d From
(
double x,
double y,
double z,
double w
);

//! Return point with direct initialization
//! @param [in] xyz x,y,z coordinates
//! @param [in] w coordinate
static DPoint4d From
(
DPoint3dCR xyz,
double w
);

//! Return product of 3d point with (possibly omitted) DMatrix4d
//! @param [in] matrix if missing, identity matrix is implied.
//! @param [in] point 3d point.
static DPoint4d FromMultiply
(
DMatrix4dCP matrix,
DPoint3dCR  point
);

//! Return product of 4d point with (possibly omitted!!) DMatrix4d
//! @param [in] matrix if missing, identity matrix is implied.
//! @param [in] point 3d point.
static DPoint4d FromMultiply
(
DMatrix4dCP matrix,
DPoint4dCR  point
);

//! zero out this point.
void Zero ();

//!
//! @param [in] xComponent x component
//! @param [in] yComponent y component
//! @param [in] zComponent z component
//! @param [in] wComponent w component
//!
void SetComponents
(
double          xComponent,
double          yComponent,
double          zComponent,
double          wComponent
);

//!
//! Fill a DPoint4d, using given xyz components and weight.
//! All components are copied in directly --
//!  the xyz components are not multiplied by the weight.
//!
//! @param [in] source xyz components
//! @param [in] w w component
//!
void Init
(
DPoint3dCR      source,
double          w
);

//!
//! Fill a DPoint4d, using given xyz components and weight.
//! All components are copied in directly --
//!  the xyz components are not multiplied by the weight.
//!
//! @param [in] x x component
//! @param [in] y y component
//! @param [in] z z component
//! @param [in] x x component
//! @param [in] w z coordinate
//!
void Init (double x, double y, double z, double w);



//!
//! Copy 4 components (xyzw) from a double array into this instance
//!
//! @param [in] pArray array of doubles
//!
void InitFromArray
(
double          *pArray
);

//!
//! Copies component data out of this instance into
//! doubles pXCoord, pYCoord, pZCoord and pWCoord.
//!
//! @param [out] xCoord x component
//! @param [out] yCoord y component
//! @param [out] zCoord z component
//! @param [out] wCoord w component
//!
void GetComponents
(
double          &xCoord,
double          &yCoord,
double          &zCoord,
double          &wCoord
) const;

//!
//! Set x,y,z or w component of a point.
//!
//! @param [in] a component value
//! @param [in] index 0=x, 1=y, 2=z, 3=w, others cyclic
//!
void SetComponent
(
double          a,
int             index
);

//!
//! @param [in] index 0=x, 1=y, 2=z, 3=w, others cyclic
//! @return specified component of the point or vector
//!
double GetComponent (int index) const;

//!
//! magnitude as pure 4d point -- sqrt sum of squares.
//!
double MagnitudeXYZW () const;

//!
//!
//! Return the full 4d (xyzw) dot product of two homogeneous points.
//! @param [in] point second point of dot product.
//! @return dot product of two homogeneous points.
//!
double DotProduct (DPoint4dCR point) const;

//!
//! Return the dot product of only the xy parts of two homogeneous points.  Ignore z, ignore w.
//! @param [in] point second point
//! @return dot product of two homogeneous points.
//!
double DotProductXY (DPoint4dCR point) const;

//!
//!
//! Return the xyz dot product of two homogeneous points, i.e. ignore w.
//! @param [in] point2 second point of dot product.
//! @return dot product of two homogeneous points.
//!
double DotProductXYZ (DPoint4dCR point2) const;

//!
//!
//! Return the xyz dot product of two homogeneous points, i.e. ignore z.
//! @param [in] point second second
//! @return dot product of two homogeneous points.
//!
double DotProductXYW (DPoint4dCR point) const;

//!
//!
//! @param [in] x x component of second point
//! @param [in] y y component of second point
//! @param [in] z z component of second point
//! @param [in] w w component of second point
//! @return dot product of two homogeneous points.
//!
double DotProduct
(
double          x,
double          y,
double          z,
double          w
) const;

//!
//!
//! @param [in] point2 second point
//! @param [in] w w component of second point
//! @return dot product of two homogeneous points.
//!
double DotProduct
(
DPoint3dCR      point2,
double          w
) const;

//!
//! Return the dot product of a plane normal and a vector 'to the
//! eyepoint'.   The plane is given as cartesian origin and normal; the
//! eye is given as homogeneous point, i.e. weight zero for flat view,
//! nonzero for perspective.
//! Eyepoints constucted 'by hand' usually look like this:
//! Flat view "from infinity" looking in direction (xyz):
//!       eyepoint = (x,y,z,0)
//! i.e. a top view has eyepoint (0,0,1,0)
//! Perspective from eyepoint at (x,y,z): eyepoint (x,y,z,1)
//! When viewing is constructed by a sequence of homogeneous
//! transformations, with the final (device) projection to the xy plane,
//! the (pretransform) eyepoint is 'by definition'
//!       Tinverse * (0,0,1,0)'
//! i.e column 2 (zero based) of the composite viewing transform.
//! (Note that the weight part can be nonzero.)
//!
//! @param [in] origin any cartesian point on plane
//! @param [in] normal cartesian plane normal
//! @return dot product of plane normal with vector towards eye.
//!
double EyePlaneTest
(
DPoint3dCR      origin,
DPoint3dCR      normal
) const;

//!
//! @param [in] vec2 second point
//! @return distance between projections of two homnogeneous points.
//!
double RealDistance (DPoint4dCR vec2) const;

//!
//! @param [out] pDistanceSquared squared distance
//! @param [in] vec2 second point
//! @return true iff the homogeneous point was properly normalized.
//!
bool RealDistanceSquaredXY
(
double          *pDistanceSquared,
DPoint3dCR      vec2
) const;

//!
//! @param [out] distance distance between xy parts
//! @param [in] pointB other point.
//! @return true iff the homogeneous points could be normalized
bool RealDistanceXY
(
double    &distance,
DPoint4dCR pointB
) const;

//!
//! @param [out] pDistanceSquared squared distance
//! @param [in] vec2 second point
//! @return true iff the homogeneous point was properly normalized.
//!
bool RealDistanceSquared
(
double          *pDistanceSquared,
DPoint3dCR      vec2
) const;

//!
//! @param [out] pDistanceSquared squared distance
//! @param [in] vec2 second point
//! @return true iff the homogeneous points were properly normalized.
//!
bool RealDistanceSquared
(
double          *pDistanceSquared,
DPoint4dCR      vec2
) const;

//!
//! Interpolates between two homogeneous vectors.                         |
//!
//! @param [in] point0 s=0 point
//! @param [in] s interpolation parameter
//! @param [in] point1 s=1 point
//!
void Interpolate
(
DPoint4dCR      point0,
double          s,
DPoint4dCR      point1
);

//!
//! Interpolates between two homogeneous vectors.                         |
//!
//! @param [in] point0 s=0 point
//! @param [in] s interpolation parameter
//! @param [in] point1 s=1 point
//! @return interpolated point
static DPoint4d FromInterpolate
(
DPoint4dCR      point0,
double          s,
DPoint4dCR      point1
);


//!
//! Initializ a homogeneous point from a 3D point and separate weight.
//! NOTE The xyz components copied unchanged, i.e. not multiplied by the
//! weight.
//!
//! @param [in] point cartesian point
//! @param [in] w weight component
//!
void InitFrom
(
DPoint3dCR      point,
double          w
);

//!
//! Copy the xyz components out of a homogeneous point.  The weight is
//! not referenced, i.e. the xyz components are NOT normalized.
//!
//! @param [out] point cartesian point
//!
void GetXYZ (DPoint3dR point) const;

//!
//! Copy the xyw components out of a homogeneous point.  The z component
//! not referenced. This is a copy, not a normalization.
//!
//! @param [out] point xyw parts copied to xyz
//!
void GetXYW (DPoint3dR point) const;

//!
//! Set components of a 3d point from 3 indices into a homogeneous point.
//! Indices are interpreted cyclically.
//!
//! @param [out] point output point
//! @param [in] xIndex index for x component of output
//! @param [in] yIndex index for y component of output
//! @param [in] zIndex index for z component of output
//!
void GetXYZ
(
DPoint3dR       point,
int             xIndex,
int             yIndex,
int             zIndex
) const;

//!
//! Computes the homogeneous vector for a plane defined by 3D origin
//! and normal.
//! NOTE If the normal vector is null, a 0000 vector is returned.
//!
//! @param [out] origin origin point
//! @param [out] normal normal vector
//! @return true unless normal is null
//!
bool PlaneFromOriginAndNormal
(
DPoint3dCR      origin,
DPoint3dCR      normal
);

//!
//! Computes the homogeneous coordinate vector for a plane defined by
//! 3 3D points.
//!
//! @param [out] origin origin point
//! @param [out] point1 another point on plane
//! @param [out] point2 another point on plane
//! @return true if normal is well defined.
//!
bool PlaneFrom3Points
(
DPoint3dCR      origin,
DPoint3dCR      point1,
DPoint3dCR      point2
);

//!
//! Computes the homogeneous coordinate vector for a plane defined by
//! a DPoint4d origin and a pair of 3D vectors.
//!
//! @param [out] origin a point on the plane.
//! @param [out] vector0 a vector in the plane.
//! @param [out] vector1 another vector in the plane.
//! @return false if origin, vectors are not independent.
//!
bool PlaneFromOriginAndVectors
(
DPoint4dCR      origin,
DPoint3dCR      vector0,
DPoint3dCR      vector1
);

//!
//! @param [out] origin cartesian orign
//! @param [out] normal cartesian normal
//! @return true if
//!
bool OriginAndNormalFromPlane
(
DPoint3dR       origin,
DPoint3dR       normal
) const;

//!
//! Adds two homogeneous points.
//!
//! @param [in] pt1 point 1
//! @param [in] pt2 point 2
//!
void SumOf
(
DPoint4dCR      pt1,
DPoint4dCR      pt2
);

//!
//! Scale each point by the other's weight and return the difference
//!
//!
void WeightedDifferenceOf
(
DPoint4dCR      A,
DPoint4dCR      B
);

//!
//! Scale each point by the other's weight and return the difference.
//! (Note that the w component of the result is always zero)
//!
//!
void WeightedDifferenceOf
(
DPoint4dCR      A,
DPoint3dCR      B,
double          wB
);

//!
//! Scale each point by the other's weight and return the difference.
//! (Note that the w component of the result is always zero)
//!
//!
void WeightedDifferenceOf
(
DPoint3dCR      A,
double          wA,
DPoint4dCR      B
);

//!
//! Add a vector to the instance.
//!
//! @param [in] vector vector to add
//!
void Add (DPoint4dCR vector);

//!
//! Subtract a vector from the instance.
//!
//! @param [in] vector vector to subtract
//!
void Subtract (DPoint4dCR vector);

//!
//! Subtract second point from first.
//!
//! @param [in] point1 first point
//! @param [in] point2 second point
//!
void DifferenceOf
(
DPoint4dCR      point1,
DPoint4dCR      point2
);

//!
//! Adds two homogeneous points to a base point.
//!
//! @param [in] point0 base point
//! @param [in] point1 point 1
//! @param [in] scale1 scale factor for point 1
//! @param [in] point2 point 2
//! @param [in] scale2 scale factor for point 2
//!
void SumOf
(
DPoint4dCR      point0,
DPoint4dCR      point1,
double          scale1,
DPoint4dCR      point2,
double          scale2
);

//!
//! Adds two homogeneous points with scales
//!
//! @param [in] point1 point 1
//! @param [in] scale1 scale factor for point 1
//! @param [in] point2 point 2
//! @param [in] scale2 scale factor for point 2
//!
void SumOf
(
DPoint4dCR      point1,
double          scale1,
DPoint4dCR      point2,
double          scale2
);

//!
//! Adds three homogeneous points to a base point.
//!
//! @param [in] point0 base point
//! @param [in] point1 point 1
//! @param [in] scale1 scale factor for point 1
//! @param [in] point2 point 2
//! @param [in] scale2 scale factor for point 2
//! @param [in] point3 point 3
//! @param [in] scale3 scale factor for point 3
//!
void SumOf
(
DPoint4dCR      point0,
DPoint4dCR      point1,
double          scale1,
DPoint4dCR      point2,
double          scale2,
DPoint4dCR      point3,
double          scale3
);

//!
//! Adds three homogeneous points.
//!
//! @param [in] point1 point 1
//! @param [in] scale1 scale factor for point 1
//! @param [in] point2 point 2
//! @param [in] scale2 scale factor for point 2
//! @param [in] point3 point 3
//! @param [in] scale3 scale factor for point 3
//!
void SumOf
(
DPoint4dCR      point1,
double          scale1,
DPoint4dCR      point2,
double          scale2,
DPoint4dCR      point3,
double          scale3
);


//!
//! Adds two homogeneous points to a base point.
//!
//! @param [in] point0 base point
//! @param [in] point1 point 1
//! @param [in] scale1 scale factor for point 1
//!
void SumOf
(
DPoint4dCR      point0,
DPoint4dCR      point1,
double          scale1
);

//!
//! Normalizes a homogeneous point (by dividing by w part.)
//!
//! @param [out] rPoint normalized point
//! @return true if normalization succeeded
//!
bool GetProjectedXYZ (DPoint3dR rPoint) const;

//!
//! Return the x,y,z parts scaled.   w part is not used.
//!
DPoint3d GetScaledXYZ (double scale) const;

//!
//! Initializes the instance by normalizing the weight of the source.
//!
//! @return true if normalization succeeded
//!
bool InitWithNormalizedWeight (DPoint4dCR source);

//!
//! Divide through by weight component.
//!
//! @return true if normalization succeeded
//!
bool NormalizeWeightInPlace ();

//!
//! Normalizes a homogeneous plane (by dividing through by the vector
//! magnitude).
//!
//! @param [in] plane0 homogeneous plane
//! @return true unless normal is zero vector.
//!
bool NormalizePlaneOf (DPoint4dCR plane0);

//!
//! sets pOutVec to pInVec*scale.
//!
//! @param [in] point input vector
//! @param [in] scale scale
//!
void Scale
(
DPoint4dCR      point,
double          scale
);

//!
//! Scale a point in place.
//!
//! @param [in] scale scale factor
//!
void Scale (double scale);

//!
//! Negate a point.
//!
//! @param [in] point input point
//!
void Negate (DPoint4dCR point);

//!
//! Negate all components of a point in place.
//!
void Negate ();

//!
//! Exact equality test between points.  (Also see method with same name
//! but added tolerance argument.)
//!
//! @param [in] vec2 vector
//! @return true if the points are identical.
//!  (DPoint4dCR, double)
//!
bool IsEqual (DPoint4dCR vec2) const;

//!
//! @param [in] vec2 vector
//! @param [in] tolerance tolerance
//! @return true if all components are within given tolerance of each other.
//!
bool IsEqual
(
DPoint4dCR      vec2,
double          tolerance
) const;

//!
//! @param [in] vec2 vector
//! @param [in] xyzTol tolerance for absolute difference between x,y,z components.
//! @param [in] wTol tolerance for absolute difference between w components.
//! @return true if all components are within given tolerance of each other,
//!       using different tolerances for xyz and w data.
//!
bool IsEqual
(
DPoint4dCR      vec2,
double          xyzTol,
double          wTol
) const;

//! test for nearly equal points in two arrays
static bool AlmostEqual
(
bvector<DPoint4d> const &dataA,     //!< [in] first array
bvector<DPoint4d> const &dataB,     //!< [in] second array
double xyzTol,                      //!< [in] tolerance for xyz parts
double wTol                         //!< [in] tolerance for weights
);

//! test for nearly equal points in two arrays, reversing the second
static bool AlmostEqualReversed
(
bvector<DPoint4d> const &dataA,     //!< [in] first array
bvector<DPoint4d> const &dataB,     //!< [in] second array
double xyzTol,                      //!< [in] tolerance for xyz parts
double wTol                         //!< [in] tolerance for weights
);

//! test for nearly equal points in two arrays
static bool AlmostEqual
(
DPoint4dCP dataA,     //!< [in] first array
DPoint4dCP dataB,     //!< [in] second array
size_t n,             //!< [in] number of points
double xyzTol,        //!< [in] tolerance for xyz parts
double wTol           //!< [in] tolerance for weights
);

//! test for nearly equal points in two arrays, reversing the second
static bool AlmostEqualReversed
(
DPoint4dCP dataA,     //!< [in] first array
DPoint4dCP dataB,     //!< [in] second array
size_t n,             //!< [in] number of points
double xyzTol,        //!< [in] tolerance for xyz parts
double wTol           //!< [in] tolerance for weights
);


//!
//! @return largest absoluted value among point coordinates.
//!
double MaxAbs () const;

//! @return largest absoluted value among xyz coordinates, ignoring weight.
double MaxAbsUnnormalizedXYZ () const;

//! @return largest absoluted value among xyz coordinates coordinates, ignoring weight.
double MaxUnnormalizedXYZDiff (DPoint4dCR other) const;

//!
//! Returns the angle of rotation represented by this instance quaternion and
//! sets axis to be the normalized vector about which this instance rotates.
//! The instance is assumed to be a normalized quaternion, i.e. of the form
//! (x,y,z,w) where
//! <pre>
//!               x*x + y*y + z*z + w*w = 1.
//! </pre>
//! The angle is returned within the closed interval [0,Pi].
//!
//! @param [out] axis normalized axis of rotation
//! @return rotation angle (in radians) between 0 and Pi, inclusive
//!
double GetRotationAngleAndVectorFromQuaternion (DPoint3dR axis) const;

//!
//! @return true if the point has coordinates which indicate it is
//!   a disconnect (separator) ponit.
//!
bool IsDisconnect () const;

//!
//! Initialize a point with all coordinates as the disconnect value.
//!
void InitDisconnect ();

//! Return a point "perpendicular" to all 3 inputs.
static DPoint4d FromCrossProduct (DPoint4dCR pointA, DPoint4dCR pointB, DPoint4dCR pointC);

//! Return point and vectors that are the cartesian image of a homoegneous point and derivatives
//!<ul>
//!<li>The return is marked invalid if weight is zero.
//!</ul>
//! @return origin and vectors after normalization.
static ValidatedDPoint3dDVec3dDVec3d TryNormalizePointAndDerivatives
(
DPoint4dCR homogeneousPoint,        //!< [in] weighted point
DPoint4dCR homogeneousDerivative1,  //!< [in] weighted first derivative 
DPoint4dCR homogeneousDerivative2   //!< [in] weighted second derivative
);
//! Initialize as coefficient of a plane with origin and 2 in-plane vectors.
//! @param [in] origin homogeneous origin
//! @param [in] vector0 first in-plane vector
//! @param [in] vector1 second in-plane vector
bool    InitPlaneFromDPoint4dDVec3dDVec3d
(
DPoint4dCR origin,
DVec3dCR vector0,
DVec3dCR vector1
);

//! Normalize weights, return as array of DPoint3d.
//! @return true if normalization succeeded for all points.
static bool NormalizeArrayWeights (DPoint3dP xyz, DPoint4dCP xyzw, int n);
};

#endif

END_BENTLEY_GEOMETRY_NAMESPACE

