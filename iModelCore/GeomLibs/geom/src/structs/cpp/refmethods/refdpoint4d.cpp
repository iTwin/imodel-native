/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include <bsibasegeomPCH.h>
BEGIN_BENTLEY_GEOMETRY_NAMESPACE

//! Return point with direct initialization
DPoint4d DPoint4d::From
(
double          x,
double          y,
double          z,
double          w
)
    {
    DPoint4d result;
    result.x = x;
    result.y = y;
    result.z = z;
    result.w = w;
    return result;
    }


//! Return point with direct initialization
DPoint4d DPoint4d::From
(
DPoint3dCR xyz,
double     w
)
    {
    DPoint4d result;
    result.x = xyz.x;
    result.y = xyz.y;
    result.z = xyz.z;
    result.w = w;
    return result;
    }



//! Return product of 3d point with (possibly omitted!!) DMatrix4d
//! @param [in] matrix if missing, identity matrix is implied.
//! @param [in] point 3d point.
DPoint4d DPoint4d::FromMultiply
(
DMatrix4dCP matrix,
DPoint3dCR  point
)
    {
    DPoint4d result;
    if (matrix == NULL)
        return From (point, 1.0);
    result.x = matrix->coff[0][0] * point.x
             + matrix->coff[0][1] * point.y
             + matrix->coff[0][2] * point.z
             + matrix->coff[0][3]; 
    result.y = matrix->coff[1][0] * point.x
             + matrix->coff[1][1] * point.y
             + matrix->coff[1][2] * point.z
             + matrix->coff[1][3]; 
    result.z = matrix->coff[2][0] * point.x
             + matrix->coff[2][1] * point.y
             + matrix->coff[2][2] * point.z
             + matrix->coff[2][3]; 
    result.w = matrix->coff[3][0] * point.x
             + matrix->coff[3][1] * point.y
             + matrix->coff[3][2] * point.z
             + matrix->coff[3][3]; 
    return result;
    }



//! Return product of 4d point with (possibly omitted!!) DMatrix4d
//! @param [in] matrix if missing, identity matrix is implied.
//! @param [in] point 3d point.
DPoint4d DPoint4d::FromMultiply
(
DMatrix4dCP matrix,
DPoint4dCR  point
)
    {
    DPoint4d result;
    if (matrix == NULL)
        return point;
    result.x = matrix->coff[0][0] * point.x
             + matrix->coff[0][1] * point.y
             + matrix->coff[0][2] * point.z
             + matrix->coff[0][3] * point.w;
    result.y = matrix->coff[1][0] * point.x
             + matrix->coff[1][1] * point.y
             + matrix->coff[1][2] * point.z
             + matrix->coff[1][3] * point.w; 
    result.z = matrix->coff[2][0] * point.x
             + matrix->coff[2][1] * point.y
             + matrix->coff[2][2] * point.z
             + matrix->coff[2][3] * point.w; 
    result.w = matrix->coff[3][0] * point.x
             + matrix->coff[3][1] * point.y
             + matrix->coff[3][2] * point.z
             + matrix->coff[3][3] * point.w; 
    return result;
    }








/*-----------------------------------------------------------------*//**
* @indexVerb zero
* @bsimethod
+----------------------------------------------------------------------*/
void DPoint4d::Zero ()
    {
    x = y = z = w = 0.0;
    }


/*-----------------------------------------------------------------*//**
* @param [in] xComponent x component
* @param [in] yComponent y component
* @param [in] zComponent z component
* @param [in] wComponent w component
* @indexVerb set
* @bsimethod
+----------------------------------------------------------------------*/
void DPoint4d::SetComponents
(

double          xComponent,
double          yComponent,
double          zComponent,
double          wComponent

)
    {
    x = xComponent;
    y = yComponent;
    z = zComponent;
    w = wComponent;
    }


/*-----------------------------------------------------------------*//**
* Fill a DPoint4d, using given xyz components and weight.
* All components are copied in directly --
*  the xyz components are not multiplied by the weight.
*
* @param [in] source xyz components
* @param [in] w w component
* @indexVerb init
* @bsimethod
+----------------------------------------------------------------------*/
void DPoint4d::Init
(
DPoint3dCR source,
            double      wIn
)
    {
    x = source.x;
    y = source.y;
    z = source.z;
    w = wIn;
    }



/*--------------------------------------------------------------------------------**//**
* @bsimethod
+--------------------------------------------------------------------------------------*/
void DPoint4d::Init (double xx, double yy, double zz, double ww)
    {
    x = xx;
    y = yy;
    z = zz;
    w = ww;
    }




/*-----------------------------------------------------------------*//**
* Copy 4 components (xyzw) from a double array into this instance
*
* @param [in] pArray array of doubles
* @indexVerb init
* @bsimethod
+----------------------------------------------------------------------*/
void DPoint4d::InitFromArray
(
double      *pArray
)
    {
    x = pArray[0];
    y = pArray[1];
    z = pArray[2];
    w = pArray[3];
    }


/*-----------------------------------------------------------------*//**
* Copies component data out of this instance into
* doubles pXCoord, pYCoord, pZCoord and pWCoord.
*
* @param [out] xComponent x component
* @param [out] yComponent y component
* @param [out] zComponent z component
* @param [out] wComponent w component
* @indexVerb get
* @bsimethod
+----------------------------------------------------------------------*/
void DPoint4d::GetComponents
(

        double      &xComponent,
        double      &yComponent,
        double      &zComponent,
        double      &wComponent

) const
    {
    xComponent = x;
    yComponent = y;
    zComponent = z;
    wComponent = w;
    }


/*-----------------------------------------------------------------*//**
* Set x,y,z or w component of a point.
*
* @param [in] a component value
* @param [in] index 0=x, 1=y, 2=z, 3=w, others cyclic
* @indexVerb set
* @bsimethod
+----------------------------------------------------------------------*/
void DPoint4d::SetComponent
(
double       a,
int         index
)
    {
    index = index & 0x03;   // last bits are what matters, even for negative !!!
    *((&x) + index) = a;
    }


/*-----------------------------------------------------------------*//**
* @param [in] index 0=x, 1=y, 2=z, 3=w, others cyclic
* @return specified component of the point or vector
* @indexVerb get
* @bsimethod
+----------------------------------------------------------------------*/
double DPoint4d::GetComponent
(

      int         index

) const
    {
    index = index & 0x03;   // last bits are what matters, even for negative !!!
    return *((&x) + index);
    }


/*-----------------------------------------------------------------*//**
* magnitude as pure 4d point -- sqrt sum of squares.
* @bsimethod
+----------------------------------------------------------------------*/
double DPoint4d::MagnitudeXYZW
(
) const
    {
    return sqrt (this->DotProduct (*this));
    }


/*-----------------------------------------------------------------*//**
*
* Return the full 4d (xyzw) dot product of two homogeneous points.
* @param [in] pVec2 second second
* @return dot product of two homogeneous points.
* @indexVerb dotProduct
* @bsimethod
+----------------------------------------------------------------------*/
double DPoint4d::DotProduct
(

DPoint4dCR point

) const
    {
    return x * point.x + y * point.y + z * point.z + w * point.w;
    }


/*-----------------------------------------------------------------*//**
* Return the dot product of only the xy parts of two homogeneous points.  Ignore z, ignore w.
* @param [in] point second point
* @return dot product of two homogeneous points.
* @indexVerb dotProduct
* @bsimethod
+----------------------------------------------------------------------*/
double DPoint4d::DotProductXY
(

DPoint4dCR point

) const
    {
    return x * point.x + y * point.y;
    }


/*-----------------------------------------------------------------*//**
*
* Return the xyz dot product of two homogeneous points, i.e. ignore w.
* @param [in] point second second
* @return dot product of two homogeneous points.
* @indexVerb dotProduct
* @bsimethod
+----------------------------------------------------------------------*/
double DPoint4d::DotProductXYZ
(
DPoint4dCR point
) const
    {
    return x * point.x + y * point.y + z * point.z;
    }


/*-----------------------------------------------------------------*//**
*
* Return the xyz dot product of two homogeneous points, i.e. ignore z.
* @param [in] point second second
* @return dot product of two homogeneous points.
* @indexVerb dotProduct
* @bsimethod
+----------------------------------------------------------------------*/
double DPoint4d::DotProductXYW
(

DPoint4dCR point

) const
    {
    return x * point.x + y * point.y + w * point.w;
    }


/*-----------------------------------------------------------------*//**
*
* @param [in] x x component of second point
* @param [in] y y component of second point
* @param [in] z z component of second point
* @param [in] w w component of second point
* @return dot product of two homogeneous points.
* @indexVerb dotProduct
* @bsimethod
+----------------------------------------------------------------------*/
double DPoint4d::DotProduct
(
double    ax,
double    ay,
double    az,
double    aw
) const
    {
    return x * ax + y * ay + z * az + w * aw;
    }


/*-----------------------------------------------------------------*//**
*
* @param [in] point second point
* @param [in] w w component of second point
* @return dot product of two homogeneous points.
* @indexVerb dotProduct
* @bsimethod
+----------------------------------------------------------------------*/
double DPoint4d::DotProduct
(

DPoint3dCR point,
      double    aw

) const
    {
    return x * point.x + y * point.y + z * point.z + w * aw;
    }


/*-----------------------------------------------------------------*//**
* Return the dot product of a plane normal and a vector 'to the
* eyepoint'.   The plane is given as cartesian origin and normal; the
* eye is given as homogeneous point, i.e. weight zero for flat view,
* nonzero for perspective.
* Eyepoints constucted 'by hand' usually look like this:
* Flat view "from infinity" looking in direction (xyz):
*       eyepoint = (x,y,z,0)
* i.e. a top view has eyepoint (0,0,1,0)
* Perspective from eyepoint at (x,y,z): eyepoint (x,y,z,1)
* When viewing is constructed by a sequence of homogeneous
* transformations, with the final (device) projection to the xy plane,
* the (pretransform) eyepoint is 'by definition'
*       Tinverse * (0,0,1,0)'
* i.e column 2 (zero based) of the composite viewing transform.
* (Note that the weight part can be nonzero.)
*
* @param [in] origin any cartesian point on plane
* @param [in] normal cartesian plane normal
* @return dot product of plane normal with vector towards eye.
* @indexVerb eyepoint
* @bsimethod
+----------------------------------------------------------------------*/
double DPoint4d::EyePlaneTest
(
DPoint3dCR origin,
DPoint3dCR normal
) const
    {
    if (w == 0.0)
        {
        /* this is just xyz vector components.  Simple dot with normal.*/
        /* Plane orign doesn't matter.*/
        return    this->x * normal.x
                + this->y * normal.y
                + this->z * normal.z;
        }
    else
        {
        /* this is weighted.*/
        double dot1 =
                  this->x * normal.x
                + this->y * normal.y
                + this->z * normal.z;

        double dot2 =
                  origin.x * normal.x
                + origin.y * normal.y
                + origin.z * normal.z;

        return dot1 - w * dot2;
        }
    }


/*-----------------------------------------------------------------*//**
* @param [in] vec2 second point
* @return distance between projections of two homnogeneous points.
* @indexVerb distance
* @bsimethod
+----------------------------------------------------------------------*/
double DPoint4d::RealDistance
(
DPoint4dCR vec2
) const
    {
    double distance2;
    if (RealDistanceSquared (&distance2, vec2))
        return sqrt (distance2);
    return 0.0;
    }


/*-----------------------------------------------------------------*//**
* @param [out] pDistSquared squared distance
* @param [in] vec2 second point
* @return true iff the homogeneous point was properly normalized.
* @indexVerb distance
* @bsimethod
+----------------------------------------------------------------------*/
bool DPoint4d::RealDistanceSquaredXY
(
double    *pDistanceSquared,
DPoint3dCR point2
) const
    {
    DPoint4d hpoint2;
    hpoint2.x = point2.x;
    hpoint2.y = point2.y;
    hpoint2.z = z;  // !!!
    hpoint2.w = 1.0;
    return RealDistanceSquared (pDistanceSquared, hpoint2);
    }


/*-----------------------------------------------------------------*//**
* @param [out] pDistSquared squared distance
* @param [in] vec2 second point
* @return true iff the homogeneous point was properly normalized.
* @indexVerb distance
* @bsimethod
+----------------------------------------------------------------------*/
bool DPoint4d::RealDistanceSquared
(
double    *pDistanceSquared,
DPoint3dCR point2

) const
    {
    DPoint4d hpoint2;
    hpoint2.x = point2.x;
    hpoint2.y = point2.y;
    hpoint2.z = point2.z;
    hpoint2.w = 1.0;
    return RealDistanceSquared (pDistanceSquared, hpoint2);
    }


/*-----------------------------------------------------------------*//**
* @param [out] pDistSquared squared distance
* @param [in] vec2 second point
* @return true iff the homogeneous points were properly normalized.
* @indexVerb distance
* @bsimethod
+----------------------------------------------------------------------*/
bool DPoint4d::RealDistanceSquared
(
double    *distanceSquared,
DPoint4dCR vec2
) const
    {
    double w1 = this->w;
    double w2 = vec2.w;
    double denom = w1 * w2;
    if (w1 == 1.0 && w2 == 1.0)     /* Nearly always, so save the weighting ... */
        {
        double dx = this->x - vec2.x;
        double dy = this->y - vec2.y;
        double dz = this->z - vec2.z;
        *distanceSquared = dx * dx + dy * dy + dz * dz;
        return true;
        }
    else if (denom != 0.0)
        {
        /* Brute force: normalize each x,y,z coordiante, i.e. 6 divides.
           Or: ( X1/w1 - X2/w2 ) = (w2*X1 - w1*X2)/(w1 * w2), i.e. 6 multiplies and one divide.
           If division is modestly slower, this is a win.
        */
        double dx = w2 * this->x - w1 * vec2.x;
        double dy = w2 * this->y - w1 * vec2.y;
        double dz = w2 * this->z - w1 * vec2.z;
        *distanceSquared = (dx * dx + dy * dy + dz * dz) / (denom * denom);
        return true;
        }
    *distanceSquared = 0.0;
    return false;
    }



//!
//! @param [out] distance distance between xy parts
//! @param [in] pointB other point.
//! @return true iff the homogeneous points could be normalized
bool DPoint4d::RealDistanceXY
(
double    &distance,
DPoint4dCR pointB
) const
    {
    double distanceSquared;
    DPoint4d myPointB = pointB;
    DPoint4d myPointA = *this;
    myPointB.z = myPointA.z = 0.0;
    if (myPointA.RealDistanceSquared (&distanceSquared, myPointB))
        {
        distance = sqrt (distanceSquared);
        return true;
        }
    distance = 0.0;
    return false;
    }




/*-----------------------------------------------------------------*//**
* Interpolates between two homogeneous vectors.                         |
*
* @param [in] point0 s=0 point
* @param [in] s interpolation parameter
* @param [in] point1 s=1 point
* @indexVerb interpolate
* @bsimethod
+----------------------------------------------------------------------*/
void DPoint4d::Interpolate
(

DPoint4dCR point0,
      double     s,
DPoint4dCR point1

)
    {
    if (s <= 0.5)
        {
        x = point0.x + s * (point1.x - point0.x);
        y = point0.y + s * (point1.y - point0.y);
        z = point0.z + s * (point1.z - point0.z);
        w = point0.w + s * (point1.w - point0.w);
        }
    else
        {
        double t = s - 1.0;
        x = point1.x + t * (point1.x - point0.x);
        y = point1.y + t * (point1.y - point0.y);
        z = point1.z + t * (point1.z - point0.z);
        w = point1.w + t * (point1.w - point0.w);
        }
    }

/*-----------------------------------------------------------------*//**
* Interpolates between two homogeneous vectors.                         |
*
* @param [in] point0 s=0 point
* @param [in] s interpolation parameter
* @param [in] point1 s=1 point
* @indexVerb interpolate
* @bsimethod
+----------------------------------------------------------------------*/
DPoint4d DPoint4d::FromInterpolate (DPoint4dCR point0, double s, DPoint4dCR point1)
    {
    DPoint4d xyzw;
    xyzw.x = point0.x + s * (point1.x - point0.x);
    xyzw.y = point0.y + s * (point1.y - point0.y);
    xyzw.z = point0.z + s * (point1.z - point0.z);
    xyzw.w = point0.w + s * (point1.w - point0.w);
    return xyzw;
    }

/*-----------------------------------------------------------------*//**
* Initializ a homogeneous point from a 3D point and separate weight.
* NOTE The xyz components copied unchanged, i.e. not multiplied by the
* weight.
*
* @param [in] point cartesian point
* @param [in] w weight component
* @indexVerb init
* @indexVerb copy
* @bsimethod
+----------------------------------------------------------------------*/
void DPoint4d::InitFrom
(
DPoint3dCR point,
      double    aw
)
    {
    x = point.x;
    y = point.y;
    z = point.z;
    w = aw;
    }


/*-----------------------------------------------------------------*//**
* Copy the xyz components out of a homogeneous point.  The weight is
* not referenced, i.e. the xyz components are NOT normalized.
*
* @param [out] point cartesian point
* @indexVerb get
* @bsimethod
+----------------------------------------------------------------------*/
void DPoint4d::GetXYZ
(

DPoint3dR point

) const
    {
    point.x = x;
    point.y = y;
    point.z = z;
    }


/*-----------------------------------------------------------------*//**
* Copy the xyw components out of a homogeneous point.  The z component
* not referenced. This is a copy, not a normalization.
*
* @param [out] point xyw parts copied to xyz
* @indexVerb get
* @bsimethod
+----------------------------------------------------------------------*/
void DPoint4d::GetXYW
(

DPoint3dR point

) const
    {
    point.x = x;
    point.y = y;
    point.z = w;
    }


/*-----------------------------------------------------------------*//**
* Set components of a 3d point from 3 indices into a homogeneous point.
* Indices are interpreted cyclically.
*
* @param [out] point output point
* @param [in] xIndex index for x component of output
* @param [in] yIndex index for y component of output
* @param [in] zIndex index for z component of output
* @indexVerb get
* @bsimethod
+----------------------------------------------------------------------*/
void DPoint4d::GetXYZ
(

DPoint3dR point,
      int       xIndex,
      int       yIndex,
      int       zIndex

) const
    {
    const double *pHArray = (const double *)this;

    /* Force cyclic ordering.  Low order bits even work for negative numbers!! */
    xIndex &= 0x3;
    yIndex &= 0x3;
    zIndex &= 0x3;

    point.x = pHArray[xIndex];
    point.y = pHArray[yIndex];
    point.z = pHArray[zIndex];
    }


/*-----------------------------------------------------------------*//**
* Computes the homogeneous vector for a plane defined by 3D origin
* and normal.
* NOTE If the normal vector is null, a 0000 vector is returned.
*
* @param [out] origin origin point
* @param [out] normal normal vector
* @return true unless normal is null
* @indexVerb init
* @bsimethod
+----------------------------------------------------------------------*/
bool DPoint4d::PlaneFromOriginAndNormal
(
DPoint3dCR origin,
DPoint3dCR normal
)
    {
    DVec3d unitNormal;
    bool    result;
    double d;

    if (unitNormal.Normalize (*((DVec3d const*)&normal)) > 0.0)
        {
        d = unitNormal.DotProduct (origin);
        Init (unitNormal, -d);
        result = true;
        }
    else
        {
        Zero ();
        result = false;
        }
    return result;
    }


/*-----------------------------------------------------------------*//**
* Computes the homogeneous coordinate vector for a plane defined by
* 3 3D points.
*
* @param [out] origin origin point
* @param [out] point1 another point on plane
* @param [out] point2 another point on plane
* @return true if normal is well defined.
* @indexVerb init
* @bsimethod
+----------------------------------------------------------------------*/
bool DPoint4d::PlaneFrom3Points
(

DPoint3dCR origin,
DPoint3dCR point1,
DPoint3dCR point2

)
    {
    DPoint3d normal;
    normal.CrossProductToPoints (origin, point1, point2);
    return PlaneFromOriginAndNormal (origin, normal);
    }


/*-----------------------------------------------------------------*//**
* Computes the homogeneous coordinate vector for a plane defined by
* a DPoint4d origin and a pair of 3D vectors.
*
* @param [out] origin a point on the plane.
* @param [out] vector1 a vector in the plane.
* @param [out] pVector2 another vector in the plane.
* @return false if origin, vectors are not independent.
* @indexVerb init
* @bsimethod
+----------------------------------------------------------------------*/
bool DPoint4d::PlaneFromOriginAndVectors
(

DPoint4dCR origin,
DPoint3dCR vector0,
DPoint3dCR vector1

)
    {
    if (origin.w == 0.0)
        {
        /* Really given three vectors. */
        SetComponents (0.0, 0.0, 0.0, 1.0);
        }
    else
        {
        DVec3d W;
        double a, b;
        a = origin.w;
        W.CrossProduct (*(DVec3d const*)&vector0, *(DVec3d const*)&vector1);
        b = origin.DotProduct (W, 0.0);
        SetComponents (a * W.x, a * W.y, a * W.z, -b);
        }
    /* UGH -- need to test zero.  No good tolerance around. */
    return x != 0.0 || y != 0.0 || z != 0.0;
    }


/*-----------------------------------------------------------------*//**
* @param [out] origin cartesian orign
* @param [out] normal cartesian normal
* @return true if
* @indexVerb get
* @bsimethod
+----------------------------------------------------------------------*/
bool DPoint4d::OriginAndNormalFromPlane
(
DPoint3dR origin,
DPoint3dR normal
) const
    {
    GetXYZ (normal);    // weight not used.
    origin.Scale (normal, -w);  // THIS WAS NEGATED IN bsiDPoint3d_originAndNormalFromPlane !!
    return normal.Normalize () > 0.0;
    }


/*-----------------------------------------------------------------*//**
* Adds two homogeneous points.
*
* @param [in] pt1 point 1
* @param [in] pt2 point 2
* @indexVerb add
* @bsimethod
+----------------------------------------------------------------------*/
void DPoint4d::SumOf
(

DPoint4dCR pt1,
DPoint4dCR pt2

)
    {
    x = pt1.x + pt2.x;
    y = pt1.y + pt2.y;
    z = pt1.z + pt2.z;
    w = pt1.w + pt2.w;
    }


/*-----------------------------------------------------------------*//**
* Scale each point by the other's weight and return the difference
*
* @indexVerb subtract
* @bsimethod
+----------------------------------------------------------------------*/
void DPoint4d::WeightedDifferenceOf
(
DPoint4dCR A,
DPoint4dCR B
)
    {
    x = A.x * B.w - B.x * A.w;
    y = A.y * B.w - B.y * A.w;
    z = A.z * B.w - B.z * A.w;
    w = 0.0;
    }


/*-----------------------------------------------------------------*//**
* Scale each point by the other's weight and return the difference.
* (Note that the w component of the result is always zero)
*
* @indexVerb subtract
* @bsimethod
+----------------------------------------------------------------------*/
void DPoint4d::WeightedDifferenceOf
(

DPoint4dCR A,
DPoint3dCR B,
      double       wB

)
    {
    x = A.x * wB - B.x * A.w;
    y = A.y * wB - B.y * A.w;
    z = A.z * wB - B.z * A.w;
    w = 0.0;
    }


/*-----------------------------------------------------------------*//**
* Scale each point by the other's weight and return the difference.
* (Note that the w component of the result is always zero)
*
* @indexVerb subtract
* @bsimethod
+----------------------------------------------------------------------*/
void DPoint4d::WeightedDifferenceOf
(
DPoint3dCR A,
      double         wA,
DPoint4dCR B
)
    {
    x = A.x * B.w - B.x * wA;
    y = A.y * B.w - B.y * wA;
    z = A.z * B.w - B.z * wA;
    w = 0.0;
    }


/*-----------------------------------------------------------------*//**
* Add a vector to the instance.
*
* @param [in] vector vector to add
* @indexVerb add
* @bsimethod
+----------------------------------------------------------------------*/
void DPoint4d::Add
(

DPoint4dCR vector

)
    {
    x += vector.x;
    y += vector.y;
    z += vector.z;
    w += vector.w;
    }


/*-----------------------------------------------------------------*//**
* Subtract a vector from the instance.
*
* @param [in] pPoint vector to subtract
* @indexVerb subtract
* @bsimethod
+----------------------------------------------------------------------*/
void DPoint4d::Subtract (DPoint4dCR vector)
    {
    x -= vector.x;
    y -= vector.y;
    z -= vector.z;
    w -= vector.w;
    }


/*-----------------------------------------------------------------*//**
* Subtract second point from first.
*
* @param [in] point1 first point
* @param [in] point2 second point
* @indexVerb subtract
* @bsimethod
+----------------------------------------------------------------------*/
void DPoint4d::DifferenceOf
(

DPoint4dCR point1,
DPoint4dCR point2

)
    {
    x = point1.x - point2.x;
    y = point1.y - point2.y;
    z = point1.z - point2.z;
    w = point1.w - point2.w;
    }


/*-----------------------------------------------------------------*//**
* Adds two homogeneous points to a base point.
*
* @param [in] point0 base point
* @param [in] point1 point 1
* @param [in] scale1 scale factor for point 1
* @param [in] point2 point 2
* @param [in] scale2 scale factor for point 2
* @indexVerb add
* @bsimethod
+----------------------------------------------------------------------*/
void DPoint4d::SumOf
(

DPoint4dCR point0,
DPoint4dCR point1,
      double      scale1,
DPoint4dCR point2,
      double      scale2

)
    {
    x = point0.x + point1.x * scale1 + point2.x * scale2;
    y = point0.y + point1.y * scale1 + point2.y * scale2;
    z = point0.z + point1.z * scale1 + point2.z * scale2;
    w = point0.w + point1.w * scale1 + point2.w * scale2;
    }




/*--------------------------------------------------------------------------------**//**
* @bsimethod
+--------------------------------------------------------------------------------------*/
void DPoint4d::SumOf
(
DPoint4dCR point1,
      double      scale1,
DPoint4dCR point2,
      double      scale2

)
    {
    x = point1.x * scale1 + point2.x * scale2;
    y = point1.y * scale1 + point2.y * scale2;
    z = point1.z * scale1 + point2.z * scale2;
    w = point1.w * scale1 + point2.w * scale2;
    }






/*-----------------------------------------------------------------*//**
* Adds three homogeneous points to a base point.
*
* @param [in] point0 base point
* @param [in] point1 point 1
* @param [in] scale1 scale factor for point 1
* @param [in] point2 point 2
* @param [in] scale2 scale factor for point 2
* @param [in] point3 point 3
* @param [in] scale3 scale factor for point 3
* @indexVerb add
* @bsimethod
+----------------------------------------------------------------------*/
void DPoint4d::SumOf
(

DPoint4dCR point0,
DPoint4dCR point1,
      double      scale1,
DPoint4dCR point2,
      double      scale2,
DPoint4dCR point3,
      double      scale3

)
    {
    x = point0.x + point1.x * scale1 + point2.x * scale2 + point3.x * scale3;
    y = point0.y + point1.y * scale1 + point2.y * scale2 + point3.y * scale3;
    z = point0.z + point1.z * scale1 + point2.z * scale2 + point3.z * scale3;
    w = point0.w + point1.w * scale1 + point2.w * scale2 + point3.w * scale3;
    }

/*-----------------------------------------------------------------*//**
* Adds three homogeneous points.
*
* @param [in] point1 point 1
* @param [in] scale1 scale factor for point 1
* @param [in] point2 point 2
* @param [in] scale2 scale factor for point 2
* @param [in] point3 point 3
* @param [in] scale3 scale factor for point 3
* @indexVerb add
* @bsimethod
+----------------------------------------------------------------------*/
void DPoint4d::SumOf
(
DPoint4dCR point1,
      double      scale1,
DPoint4dCR point2,
      double      scale2,
DPoint4dCR point3,
      double      scale3

)
    {
    x = point1.x * scale1 + point2.x * scale2 + point3.x * scale3;
    y = point1.y * scale1 + point2.y * scale2 + point3.y * scale3;
    z = point1.z * scale1 + point2.z * scale2 + point3.z * scale3;
    w = point1.w * scale1 + point2.w * scale2 + point3.w * scale3;
    }

/*-----------------------------------------------------------------*//**
* Adds two homogeneous points to a base point.
*
* @param [in] point0 base point
* @param [in] point1 point 1
* @param [in] scale1 scale factor for point 1
* @indexVerb add
* @bsimethod
+----------------------------------------------------------------------*/
void DPoint4d::SumOf
(

DPoint4dCR point0,
DPoint4dCR point1,
      double      scale1

)
    {
    x = point0.x + point1.x * scale1;
    y = point0.y + point1.y * scale1;
    z = point0.z + point1.z * scale1;
    w = point0.w + point1.w * scale1;
    }


/*-----------------------------------------------------------------*//**
* Normalizes a homogeneous point (by dividing by w part.)
*
* @param [out] rPoint normalized point
* @return true if normalization succeeded
* @indexVerb normalize
* @bsimethod
+----------------------------------------------------------------------*/
bool DPoint4d::GetProjectedXYZ (DPoint3dR rPoint) const
    {
    bool    result = true;
    if (this->w == 1.0)
        {
        rPoint.x = this->x;
        rPoint.y = this->y;
        rPoint.z = this->z;
        }
    else if (this->w == 0.0)
        {
        rPoint.x = rPoint.y = rPoint.z = 0.0;
        result = false;
        }
    else
        {
        double a = 1.0 / this->w;
        rPoint.x = this->x * a;
        rPoint.y = this->y * a;
        rPoint.z = this->z * a;
        }
    return result;
    }

/*-----------------------------------------------------------------*//**
* return scaled x,y,z
+----------------------------------------------------------------------*/
DPoint3d DPoint4d::GetScaledXYZ (double scale) const
    {
    return DPoint3d::From (scale * x, scale * y, scale * z);
    }


/*-----------------------------------------------------------------*//**
* Initializes the instance by normalizing the weight of the source.
*
* @return true if normalization succeeded
* @indexVerb init
* @bsimethod
+----------------------------------------------------------------------*/
bool DPoint4d::InitWithNormalizedWeight (DPoint4dCR source)
    {
    bool    result = true;

    if (source.w == 0.0)
        {
        *this = source;
        result = false;
        }
    else
        {
        double a = 1.0 / source.w;
        x = source.x * a;
        y = source.y * a;
        z = source.z * a;
        w = 1.0;
        }

    return result;
    }



/*-----------------------------------------------------------------*//**
* Divide through by weight component.
*
* @return true if normalization succeeded
* @indexVerb normalize
* @bsimethod
+----------------------------------------------------------------------*/
bool DPoint4d::NormalizeWeightInPlace ()
    {
    return InitWithNormalizedWeight (*this);
    }

/*-----------------------------------------------------------------*//**
* Normalize weights, return as array of DPoint3d.
* @return true if normalization succeeded for all points.
+----------------------------------------------------------------------*/
bool DPoint4d::NormalizeArrayWeights (DPoint3dP xyz, DPoint4dCP xyzw, int n)
    {
    bool allOK = true;
    for (int i = 0; i < n; i++)
        if (!xyzw[i].GetProjectedXYZ (xyz[i]))
            allOK = false;
    return allOK;
    }

/*-----------------------------------------------------------------*//**
* Normalizes a homogeneous plane (by dividing through by the vector
* magnitude).
*
* @param [in] plane0 homogeneous plane
* @return true unless normal is zero vector.
* @indexVerb normalize
* @bsimethod
+----------------------------------------------------------------------*/
bool DPoint4d::NormalizePlaneOf (DPoint4dCR plane0)
    {
    bool    result = true;
    double mag = sqrt  (plane0.x * plane0.x
                      + plane0.y * plane0.y
                      + plane0.z * plane0.z);

    if (mag == 0.0)
        {
        *this = plane0;
        result = false;
        }
    else
        {
        Scale (1.0 / mag);
        result = true;
        }
    return result;
    }


/*-----------------------------------------------------------------*//**
* sets pOutVec to pInVec*scale.
*
* @param [in] pInVec input vector
* @param [in] scale scale
* @indexVerb scale
* @bsimethod
+----------------------------------------------------------------------*/
void DPoint4d::Scale
(

DPoint4dCR point,
        double       scale

)
    {
    x = scale * point.x;
    y = scale * point.y;
    z = scale * point.z;
    w = scale * point.w;
    }



/*--------------------------------------------------------------------------------**//**
* @bsimethod
+--------------------------------------------------------------------------------------*/
void DPoint4d::Scale (double       scale)
    {
    x *= scale;
    y *= scale;
    z *= scale;
    w *= scale;
    }


/*-----------------------------------------------------------------*//**
* Negate a point.
*
* @param [in] point input point
* @indexVerb scale
* @bsimethod
+----------------------------------------------------------------------*/
void DPoint4d::Negate (DPoint4dCR point)
    {
    x = - point.x;
    y = - point.y;
    z = - point.z;
    w = - point.w;
    }


/*-----------------------------------------------------------------*//**
* Negate all components of a point in place.
* @indexVerb scale
* @bsimethod
+----------------------------------------------------------------------*/
void DPoint4d::Negate ()
    {
    x = - x;
    y = - y;
    z = - z;
    w = - w;
    }


/*-----------------------------------------------------------------*//**
* Exact equality test between points.  (Also see method with same name
* but added tolerance argument.)
*
* @param [in] other vector
* @return true if the points are identical.
* @see #isEqual
* @indexVerb equal
* @bsimethod
+----------------------------------------------------------------------*/
bool DPoint4d::IsEqual (DPoint4dCR other) const
    {
    return     x == other.x
            && y == other.y
            && z == other.z
            && w == other.w     ;
    }


/*-----------------------------------------------------------------*//**
* @param [in] other vector
* @param [in] tolerance tolerance
* @return true if all components are within given tolerance of each other.
* @indexVerb equal
* @bsimethod
+----------------------------------------------------------------------*/
bool DPoint4d::IsEqual (DPoint4dCR other, double tolerance) const
    {
    return fabs(x - other.x) <= tolerance &&
           fabs(y - other.y) <= tolerance &&
           fabs(z - other.z) <= tolerance &&
           fabs(w - other.w) <= tolerance;
    }


/*-----------------------------------------------------------------*//**
* @param [in] other vector
* @param [in] tolerance tolerance
* @param [in] xyzTol tolerance for absolute difference between x,y,z components.
* @param [in] wTol tolerance for absolute difference between w components.
* @return true if all components are within given tolerance of each other,
*       using different tolerances for xyz and w data.
* @indexVerb equal
* @bsimethod
+----------------------------------------------------------------------*/
bool DPoint4d::IsEqual
(
DPoint4dCR other,
double     xyzTol,
double     wTol
) const
    {
    return   fabs(x - other.x) <= xyzTol &&
             fabs(y - other.y) <= xyzTol &&
             fabs(z - other.z) <= xyzTol &&
             fabs(w - other.w) <= wTol;
    }


/*-----------------------------------------------------------------*//**
* @return largest absoluted value among point coordinates.
* @indexVerb magnitude
* @bsimethod
+----------------------------------------------------------------------*/
double DPoint4d::MaxAbs () const
    {
    return DoubleOps::MaxAbs (x, y, z, w);
    }

double DPoint4d::MaxUnnormalizedXYZDiff (DPoint4dCR other) const
    {
    return DoubleOps::MaxAbs (x - other.x, y - other.y, z - other.z);
    }

double DPoint4d::MaxAbsUnnormalizedXYZ () const
    {
    return DoubleOps::MaxAbs (x, y, z);
    }

//! test for nearly equal points in an array
bool DPoint4d::AlmostEqual
(
bvector<DPoint4d> const &dataA,     //!< [in] first array
bvector<DPoint4d> const &dataB,     //!< [in] second array
double xyzTol,                      //!< [in] tolerance for xyz parts
double wTol                         //!< [in] tolerance for weights
)
    {
    size_t n = dataA.size ();
    return dataB.size () == n
        && n > 0
        && AlmostEqual (&dataA[0], &dataB[0], dataA.size (), xyzTol, wTol);
    }

    //! test for nearly equal points in an array
bool DPoint4d::AlmostEqualReversed
(
bvector<DPoint4d> const &dataA,     //!< [in] first array
bvector<DPoint4d> const &dataB,     //!< [in] second array
double xyzTol,                      //!< [in] tolerance for xyz parts
double wTol                         //!< [in] tolerance for weights
)
    {
    size_t n = dataA.size ();
    return dataB.size () == n
        && n > 0
        && AlmostEqualReversed (&dataA[0], &dataB[0], dataA.size (), xyzTol, wTol);
    }

//! test for nearly equal points in an array
bool DPoint4d::AlmostEqual
(
DPoint4dCP dataA,     //!< [in] first array
DPoint4dCP dataB,     //!< [in] second array
size_t n,             //!< [in] number of points
double xyzTol,        //!< [in] tolerance for xyz parts
double wTol           //!< [in] tolerance for weights
)
    {
    for (size_t i = 0; i < n; i++)
        {
        if (fabs (dataA[i].x - dataB[i].x) > xyzTol)
            return false;
        if (fabs (dataA[i].y - dataB[i].y) > xyzTol)
            return false;
        if (fabs (dataA[i].z - dataB[i].z) > xyzTol)
            return false;
        if (fabs (dataA[i].w - dataB[i].w) > wTol)
            return false;
        }
    return true;
    }

//! test for nearly equal points in an array
bool DPoint4d::AlmostEqualReversed
(
DPoint4dCP dataA,     //!< [in] first array
DPoint4dCP dataB,     //!< [in] second array
size_t n,             //!< [in] number of points
double xyzTol,        //!< [in] tolerance for xyz parts
double wTol           //!< [in] tolerance for weights
)
    {
    size_t j = n;
    for (size_t i = 0; i < n; i++)
        {
        j--;
        if (fabs (dataA[i].x - dataB[j].x) > xyzTol)
            return false;
        if (fabs (dataA[i].y - dataB[j].y) > xyzTol)
            return false;
        if (fabs (dataA[i].z - dataB[j].z) > xyzTol)
            return false;
        if (fabs (dataA[i].w - dataB[j].w) > wTol)
            return false;
        }
    return true;
    }

/*-----------------------------------------------------------------*//**
* Returns the angle of rotation represented by this instance quaternion and
* sets axis to be the normalized vector about which this instance rotates.
* The instance is assumed to be a normalized quaternion, i.e. of the form
* (x,y,z,w) where
*<pre>
*               x*x + y*y + z*z + w*w = 1.
*</pre>
* The angle is returned within the closed interval [0,Pi].
*
* @param [out] axis normalized axis of rotation
* @return rotation angle (in radians) between 0 and Pi, inclusive
* @bsimethod
+----------------------------------------------------------------------*/
double DPoint4d::GetRotationAngleAndVectorFromQuaternion (DPoint3dR axis) const
    {
    /* Source : Hoschek & Lasser, Fundamentals of CAGD, 1993, p 10. */
    double          sinHalfAngle, angle;

    axis.x = this->x;
    axis.y = this->y;
    axis.z = this->z;

    sinHalfAngle = axis.Normalize ();

    /* pQuat->w = cosHalfAngle */
    angle = 2.0 * Angle::Atan2 (sinHalfAngle, this->w); /* range [0,2PI] */

    /* shift angle range to [0,Pi] by flipping axis and using smaller angle */
    if (angle > msGeomConst_pi)
        {
        angle = msGeomConst_2pi - angle;
        axis.x = - axis.x;
        axis.y = - axis.y;
        axis.z = - axis.z;
        }

    // no rotation => any vector will do: use 001
    if (angle == 0.0)
        {
        axis.x = axis.y = 0.0;
        axis.z = 1.0;
        }

    return angle;
    }


/*-----------------------------------------------------------------*//**
* @return true if the point has coordinates which indicate it is
*   a disconnect (separator) ponit.
* @bsimethod
+----------------------------------------------------------------------*/
bool DPoint4d::IsDisconnect () const
    {
    return x == DISCONNECT
        || y == DISCONNECT
        || z == DISCONNECT
        || w == DISCONNECT;
    }


/*-----------------------------------------------------------------*//**
* Initialize a point with all coordinates as the disconnect value.
* @bsimethod
+----------------------------------------------------------------------*/
void DPoint4d::InitDisconnect ()
    {
    SetComponents (DISCONNECT, DISCONNECT, DISCONNECT, DISCONNECT);
    }

// INDICES ARE NOT CHECKED !!!!  INTENDED FOR USE BY INTERNAL TRUSTED CODE
static double DPoint4d__DeterminantOf3Components (DPoint4dCR pointA, DPoint4dCR pointB, DPoint4dCR pointC, int i, int j, int k)
    {
    double const *a = (double const*)&pointA;
    double const *b = (double const*)&pointB;
    double const *c = (double const*)&pointC;
    return
         a[i] * (b[j] * c[k] - b[k] * c[j])
        +a[j] * (b[k] * c[i] - b[i] * c[k])
        +a[k] * (b[i] * c[j] - b[j] * c[i]);
    }
/*-----------------------------------------------------------------*//**
* Initialize a point perpendicular to all 3
* @bsimethod
+----------------------------------------------------------------------*/
DPoint4d DPoint4d::FromCrossProduct (DPoint4dCR pointA, DPoint4dCR pointB, DPoint4dCR pointC)
    {
    return DPoint4d::From 
        (
        DPoint4d__DeterminantOf3Components (pointA, pointB, pointC, 1, 2, 3),
        -DPoint4d__DeterminantOf3Components (pointA, pointB, pointC, 2, 3, 0),
        DPoint4d__DeterminantOf3Components (pointA, pointB, pointC, 3, 0, 1),
        -DPoint4d__DeterminantOf3Components (pointA, pointB, pointC, 0, 1, 2)
        );
    }

ValidatedDPoint3dDVec3dDVec3d DPoint4d::TryNormalizePointAndDerivatives
(
DPoint4dCR homogeneousPoint,
DPoint4dCR homogeneousDerivative1,
DPoint4dCR homogeneousDerivative2
)
    {
    double w = homogeneousPoint.w;
    double dw = homogeneousDerivative1.w;
    double ddw = homogeneousDerivative2.w;

    DPoint3d  X;
    DVec3d dX, ddX;

    if (w == 1.0 && dw == 0.0 && ddw == 0.0)
        {
        X   = DPoint3d::From (homogeneousPoint.x, homogeneousPoint.y, homogeneousPoint.z);
        dX = DVec3d::From (homogeneousDerivative1.x, homogeneousDerivative1.y, homogeneousDerivative1.z);
        ddX = DVec3d::From (homogeneousDerivative2.x, homogeneousDerivative2.y, homogeneousDerivative2.z);
        return ValidatedDPoint3dDVec3dDVec3d (DPoint3dDVec3dDVec3d (X, dX, ddX), true);
        }
    else
        {
        bool stat;
        double a, da, dda;
        if (w == 0.0)
            {
            a = 1.0;
            stat = false;
            }
        else
            {
            a = 1.0 / w;
            stat = true;
            }

        da = -dw *a * a;
        dda = - a * ( 2.0 * da * dw + a * a * ddw);
        X.x = a * homogeneousPoint.x;
        X.y = a * homogeneousPoint.y;
        X.z = a * homogeneousPoint.z;

        dX.x = a * homogeneousDerivative1.x + da * homogeneousPoint.x;
        dX.y = a * homogeneousDerivative1.y + da * homogeneousPoint.y;
        dX.z = a * homogeneousDerivative1.z + da * homogeneousPoint.z;

        double b = 2.0 * da;
        ddX.x = a * homogeneousDerivative2.x + b * homogeneousDerivative1.x + dda * homogeneousPoint.x;
        ddX.y = a * homogeneousDerivative2.y + b * homogeneousDerivative1.y + dda * homogeneousPoint.y;
        ddX.z = a * homogeneousDerivative2.z + b * homogeneousDerivative1.z + dda * homogeneousPoint.z;

        return ValidatedDPoint3dDVec3dDVec3d (DPoint3dDVec3dDVec3d (X, dX, ddX), stat);
        }
    }

/*-----------------------------------------------------------------*//**
* Computes the homogeneous coordinate vector for a plane defined by
* a DPoint4d origin and a pair of 3D vectors.
*
* @instance pInstance => homogeneous vector for plane
* @param pOrigin <= a point on the plane.
* @param pVector1 <= a vector in the plane.
* @param pVector2 <= another vector in the plane.
* @return false if origin, vectors are not independent.
* @indexVerb init
* @bsimethod
+---------------+---------------+---------------+---------------+------*/
bool    DPoint4d::InitPlaneFromDPoint4dDVec3dDVec3d
(
DPoint4dCR origin,
DVec3dCR vector0,
DVec3dCR vector1
)
    {
    if (origin.w == 0.0)
        {
        this->Init (0,0,0,1);
        }
    else
        {
        DPoint3d W;
        double a, b;
        a = origin.w;
        W.CrossProduct (vector0, vector1);
        b = origin.DotProduct (W, 0.0);
        this->Init (a * W.x, a * W.y, a * W.z, -b);
        }
    /* UGH -- need to test zero.  No good tolerance around. */
    return this->x != 0.0 || this->y != 0.0 || this->z != 0.0;
    }
END_BENTLEY_GEOMETRY_NAMESPACE
