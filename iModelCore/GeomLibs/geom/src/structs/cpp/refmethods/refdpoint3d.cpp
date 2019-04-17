/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#include <bsibasegeomPCH.h>
BEGIN_BENTLEY_NAMESPACE

/* VBSUB(Point3dDistanceSquared) */
/* CSVFUNC(distanceSquared) */

/*-----------------------------------------------------------------*//**
 @description Sets all components of a point or vector to zero.
 @param pPoint <= zeroed point or vector
@group "DPoint3d Initialization"
 @bsimethod                                                                     EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void bsiDPoint3d_zero


(
DPoint3dP pPoint
)
    {
    pPoint->x = 0.0;
    pPoint->y = 0.0;
    pPoint->z = 0.0;
    }

/*-----------------------------------------------------------------*//**
 @description Sets the x,y, and z components of a point
 @param pPoint <= initialized point or vector
 @param ax => x component
 @param ay => y component
 @param az => z component
@group "DPoint3d Initialization"
 @bsimethod                                                                     EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP void bsiDPoint3d_setXYZ


(
DPoint3dP pPoint,
double       ax,
double       ay,
double       az
)
    {
    pPoint->x = ax;
    pPoint->y = ay;
    pPoint->z = az;
    }

/*-----------------------------------------------------------------*//**
 @description Finds the largest absolute value among the components of a point or vector.
 @param pVector => point or vector
 @return largest absolute value among point coordinates
@group "DPoint3d Queries"
 @bsimethod                                                                     EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP double bsiDPoint3d_maxAbs


(
DPoint3dCP pVector
)
    {
    double maxVal = fabs (pVector->x);

    if (fabs (pVector->y) > maxVal)
        maxVal = fabs (pVector->y);

    if (fabs (pVector->z) > maxVal)
        maxVal = fabs (pVector->z);

    return maxVal;
    }


/*-----------------------------------------------------------------*//**
* @description Computes the magnitude of a vector.
* @instance pVector => The vector
* @return The length of the vector
* @bsihdr                                                                       EarlinLutz      12/97
+---------------+---------------+---------------+---------------+------*/
Public GEOMDLLIMPEXP double bsiDPoint3d_magnitude
(
DPoint3dCP pVector
)
    {
    return  sqrt ( pVector->x*pVector->x
                 + pVector->y*pVector->y
                 + pVector->z*pVector->z);
    }


/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
DPoint3d DPoint3d::From (double xx, double yy, double zz)
    {
    DPoint3d xyz;
    xyz.x = xx;
    xyz.y = yy;
    xyz.z = zz;
    return xyz;
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
DPoint3d DPoint3d::From (FPoint3dCR in)
    {
    DPoint3d xyz;
    xyz.x = in.x;
    xyz.y = in.y;
    xyz.z = in.z;
    return xyz;
    }


/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
DPoint3d DPoint3d::FromShift
(
DPoint3dCR xyz0,    //!< [in] reference point
double dx,      //!< [in] shift to apply to x direction
double dy,      //!< [in] shift to apply to y direction
double dz       //!< [in] shift to apply to z direction
)
    {
    DPoint3d xyz;
    xyz.x = xyz0.x + dx;
    xyz.y = xyz0.y + dy;
    xyz.z = xyz0.z + dz;
    return xyz;
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
DPoint3d DPoint3d::FromXY (DPoint3dCR xy, double zz)
    {
    DPoint3d xyz;
    xyz.x = xy.x;
    xyz.y = xy.y;
    xyz.z = zz;
    return xyz;
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
DPoint3d DPoint3d::From (DPoint2dCR xy, double zz)
    {
    DPoint3d xyz;
    xyz.x = xy.x;
    xyz.y = xy.y;
    xyz.z = zz;
    return xyz;
    }


//---------------------------------------------------------------------------------------
// @bsimethod                                                   MattGooding     08/13
//---------------------------------------------------------------------------------------
DPoint3d DPoint3d::FromZero ()
    {
    DPoint3d xyz;
    xyz.Zero();
    return xyz;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   MattGooding     08/13
//---------------------------------------------------------------------------------------
DPoint3d DPoint3d::FromOne ()
    {
    DPoint3d xyz;
    xyz.One();
    return xyz;
    }

DPoint3d DPoint3d::From (DPoint2dCR xy)
    {
    DPoint3d xyz;
    xyz.x = xy.x;
    xyz.y = xy.y;
    xyz.z = 0.0;
    return xyz;
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
DPoint3d DPoint3d::FromXYZ (double xx, double yy, double zz)
    {
    DPoint3d xyz;
    xyz.x = xx;
    xyz.y = yy;
    xyz.z = zz;
    return xyz;
    }

/*-----------------------------------------------------------------*//**
* @description Returns a DPoint3d with doubles from a 3 component array to the x,y, and z components
* of a DPoint3d
*
* @param [in] pXyz x, y, z components
+----------------------------------------------------------------------*/
DPoint3d DPoint3d::FromArray
(
const   double  *pXyz
)
    {
    DPoint3d xyz;
    xyz.x = pXyz[0];
    xyz.y = pXyz[1];
    xyz.z = pXyz[2];
    return xyz;
    }




/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
void DPoint3d::Swap (DPoint3dR other)
    {
    DPoint3d temp = *this;
    *this = other;
    other = temp;
    }




/*-----------------------------------------------------------------*//**
* @description Returns the (scalar) cross product of the xy parts of two vectors.
*   The vectors are computed from the Origin to Target1 and Target2.
* @param [in] target1 The target point for the first vector.
* @param [in] target2 The target point for the second vector.
* @bsimethod                                                                    EarlinLutz      12/97
+----------------------------------------------------------------------*/
double DPoint3d::CrossProductToPointsXY
(

DPoint3dCR target1,
DPoint3dCR target2

) const
    {
    double x1 = target1.x - x;
    double y1 = target1.y - y;

    double x2 = target2.x - x;
    double y2 = target2.y - y;

    return x1 * y2 - y1 * x2;
    }


/*-----------------------------------------------------------------*//**
* @description Returns the (scalar) dot product of two vectors.
*   The vectors are computed from the Origin to Target1 and Target2.
* @param [in] target1 The target point for the first vector.
* @param [in] target2 The target point for the second vector.
* @bsimethod                                                                    EarlinLutz      12/97
+----------------------------------------------------------------------*/
double DPoint3d::DotProductToPoints
(

DPoint3dCR target1,
DPoint3dCR target2

) const
    {
    double x1 = target1.x - x;
    double y1 = target1.y - y;
    double z1 = target1.z - z;

    double x2 = target2.x - x;
    double y2 = target2.y - y;
    double z2 = target2.z - z;

    return  x1 * x2 + y1 * y2 + z1 * z2;
    }


/*-----------------------------------------------------------------*//**
* @description Returns the (scalar) dot product of xy parts of two vectors.
*   The vectors are computed from the BasePoint to Target1 and Target2.
* @param [in] target1 The target point for the first vector.
* @param [in] target2 The target point for the second vector.
* @bsimethod                                                                    EarlinLutz      12/97
+----------------------------------------------------------------------*/
double DPoint3d::DotProductToPointsXY
(

DPoint3dCR target1,
DPoint3dCR target2

) const
    {
    double x1 = target1.x - x;
    double y1 = target1.y - y;

    double x2 = target2.x - x;
    double y2 = target2.y - y;

    return  x1 * x2 + y1 * y2;
    }


/*-----------------------------------------------------------------*//**
* @description Returns the (scalar) dot product of a two vectors.
*  One vector is computed internally as the difference of the TargetPoint
*   and Origin. (TargetPoint-Origin)
*  The other is given directly as a single argument.
*
* @param [in] origin  The start (orign) point of the first vector of the cross product.
* @param [in] vector  The second
* @bsimethod                                                                    EarlinLutz      12/97
+----------------------------------------------------------------------*/
double DPoint3d::DotDifference
(

DPoint3dCR origin,
DVec3dCR vector

) const
    {
    return
            (x - origin.x) * vector.x
        +   (y - origin.y) * vector.y
        +   (z - origin.z) * vector.z;
    }


/*-----------------------------------------------------------------*//**
* @description Computes the triple product of vectors from a base point three target points.
* @param [in] target1 The target point for the first vector.
* @param [in] target2 The target point for the second vector.
* @param [in] target3 The target point for the third vector.
* @return The triple product
* @bsimethod                                                                    EarlinLutz      12/97
+----------------------------------------------------------------------*/
double DPoint3d::TripleProductToPoints
(

DPoint3dCR target1,
DPoint3dCR target2,
DPoint3dCR target3

) const
    {
    DVec3d vector1, vector2, vector3;
    vector1.DifferenceOf (target1, *this);
    vector2.DifferenceOf (target2, *this);
    vector3.DifferenceOf (target3, *this);
    return vector1.TripleProduct (vector2, vector3);
    }


/*-----------------------------------------------------------------*//**
* @description Sets all components of a point or vector to zero.
* @bsimethod                                                                    EarlinLutz      12/97
+----------------------------------------------------------------------*/
void DPoint3d::Zero
(

)
    {
    x = 0.0;
    y = 0.0;
    z = 0.0;
    }


/*-----------------------------------------------------------------*//**
* @description Returns a point or vector with all components 1.0.
* @bsimethod                                                                    EarlinLutz      12/97
+----------------------------------------------------------------------*/
void DPoint3d::One
(

)
    {
    x = 1.0;
    y = 1.0;
    z = 1.0;
    }


/*-----------------------------------------------------------------*//**
* @description Copies doubles from a 3 component array to the x,y, and z components
* of a DPoint3d
*
* @param [in] pXyz x, y, z components
* @bsimethod                                                                    EarlinLutz      12/97
+----------------------------------------------------------------------*/
void DPoint3d::InitFromArray
(

const   double      *pXyz

)
    {
    x = pXyz[0];
    y = pXyz[1];
    z = pXyz[2];
    }



/*-----------------------------------------------------------------*//**
* @description Copy from a 2d point setting z to zero.
*
* @param [out] fPoint source point
* @bsimethod                                                                    EarlinLutz      12/97
+----------------------------------------------------------------------*/
void DPoint3d::Init (DPoint2dCR source)
    {
    x = source.x;
    y = source.y;
    z = 0.0;
    }



/*-----------------------------------------------------------------*//**
* @description Sets the x,y, and z components of a point
*
* @param [in] ax The x component.
* @param [in] ay The y component.
* @param [in] az The z component.
* @bsimethod                                                                    EarlinLutz      12/97
+----------------------------------------------------------------------*/
void DPoint3d::Init
(

double       ax,
double       ay,
double       az

)
    {
    x = ax;
    y = ay;
    z = az;
    }


/*-----------------------------------------------------------------*//**
* @description Sets the x, and y components of a point. Sets z to zero.
*
* @param [in] ax The x component.
* @param [in] ax The x component.
* @param [in] ay The y component
* @bsimethod                                                                    EarlinLutz      12/97
+----------------------------------------------------------------------*/
void DPoint3d::Init
(

double       ax,
double       ay

)
    {
    x = ax;
    y = ay;
    z = 0.0;
    }


/*-----------------------------------------------------------------*//**
* @description Sets the x,y, and z components of a DPoint3d structure from the
* corresponding parts of a DPoint4d.  Weight part of DPoint4d is not used.
*
* @param [in] hPoint The homogeneous point
* @bsimethod                                                                    EarlinLutz      12/97
+----------------------------------------------------------------------*/
void DPoint3d::XyzOf
(

DPoint4dCR hPoint

)
    {
    x = hPoint.x;
    y = hPoint.y;
    z = hPoint.z;
    }


/*-----------------------------------------------------------------*//**
* @description Set one of three components (x,y,z) of the point.
*
* @param [in] a The component value.
* @param [in] index Selects the the axis: 0=x, 1=y, 2=z, others cyclic.
* @bsimethod                                                                    EarlinLutz      12/97
+----------------------------------------------------------------------*/
void DPoint3d::SetComponent
(

double       a,
int         index

)
    {
    if (index > 2)
        index = index % 3;
    if (index == 0)
        {
        x = a;
        }
    else if (index == 1)
        {
        y = a;
        }
    else if (index == 2)
        {
        z = a;
        }
    else /* index < 0.*/
        {
        this->SetComponent (a, 3 - ( (-index) % 3));
        }
    }


/*-----------------------------------------------------------------*//**
* @description Gets a single component of a point.  If the index is out of
* range 0,1,2, it is interpreted cyclically.
*
* @param [in] index Indicates which component is accessed.  The values
*                       are 0=x, 1=y, 2=z.  Other values are treated cyclically.
* @return The specified component of the point or vector.
* @bsimethod                                                                    EarlinLutz      12/97
+----------------------------------------------------------------------*/
double DPoint3d::GetComponent
(

int index

) const
    {
    if (index > 2)
        index = index % 3;
    if (index == 0)
        {
        return x;
        }
    else if (index == 1)
        {
        return y;
        }
    else if (index == 2)
        {
        return z;
        }
    else /* index < 0.*/
        {
        return this->GetComponent (3 - ( (-index) % 3));
        }
    }


/*-----------------------------------------------------------------*//**
* @description Copies x,y,z components from a point to individual variables.
*
* @param [out] xCoord x component
* @param [out] yCoord y component
* @param [out] zCoord z component
* @bsimethod                                                                    EarlinLutz      12/97
+----------------------------------------------------------------------*/
void DPoint3d::GetComponents
(

double      &xCoord,
double      &yCoord,
double      &zCoord

) const
    {
    xCoord = x;
    yCoord = y;
    zCoord = z;
    }


/*-----------------------------------------------------------------*//**
* @description Computes a point whose position is given by a fractional
* argument and two endpoints.
*
* @param [in] point0 The point corresponding to fractionParameter of 0.
* @param [in] fractionParameter The fractional parametric coordinate.
*               0.0 is the start of the segment, 1.0 is the end, 0.5 is midpoint.
* @param [in] point1 The point corresponding to fractionParameter of 1.
* @bsimethod                                                                    EarlinLutz      12/97
+----------------------------------------------------------------------*/
void DPoint3d::Interpolate
(

DPoint3dCR point0,
double fractionParameter,
DPoint3dCR point1

)
    {
    if (fractionParameter <= 0.5)
        {
        x = point0.x + fractionParameter * (point1.x - point0.x);
        y = point0.y + fractionParameter * (point1.y - point0.y);
        z = point0.z + fractionParameter * (point1.z - point0.z);
        }
    else
        {
        double t = fractionParameter - 1.0;
        x = point1.x + t * (point1.x - point0.x);
        y = point1.y + t * (point1.y - point0.y);
        z = point1.z + t * (point1.z - point0.z);
        }
    }


/*-----------------------------------------------------------------*//**
* @description Form vectors from the origin to the test point and the two boundary vectors.
* Test if the test vector is within the smaller angle between the other two vectors.
* @param [in] pTestPoint The point to test.
* @param [in] target1 The first target point.
* @param [in] target2 The second target point.
* @return true if the test point is within the angle.
* @bsimethod                                                                    EarlinLutz      12/97
+----------------------------------------------------------------------*/
bool DPoint3d::IsPointInSmallerSector
(

DPoint3dCR origin,
DPoint3dCR target1,
DPoint3dCR target2

) const
    {
    DVec3d vector0, vector1, vector;
    vector0.DifferenceOf (target1, origin);
    vector1.DifferenceOf (target2, origin);
    vector.DifferenceOf (*this, origin);
    return vector.IsVectorInSmallerSector (vector0, vector1);
    }


/*-----------------------------------------------------------------*//**
* @description Test if a point is within the counter-clockwise sector defined by
* an origin and two boundary points, with an up vector to determine which
* direction is counter clockwise.
* @param [in] pTestPoint The point to test.
* @param [in] target0 The first target point.
* @param [in] target1 The second target point.
* @return true if the test point is within the angle.
* @bsimethod                                                                    EarlinLutz      12/97
+----------------------------------------------------------------------*/
bool DPoint3d::IsPointInCCWector
(

DPoint3dCR origin,
DPoint3dCR target0,
DPoint3dCR target1,
DVec3dCR upVector

) const
    {
    DVec3d vector0, vector1, vector;
    vector0.DifferenceOf (target0, origin);
    vector1.DifferenceOf (target1, origin);
    vector.DifferenceOf (*this, origin);
    return vector.IsVectorInCCWSector (vector0, vector1, upVector);
    }


/*-----------------------------------------------------------------*//**
* @description Computes the (cartesian) distance between two points
*
* @param [in] point2 The second point
* @return The distance between points.
* @bsimethod                                                                    EarlinLutz      12/97
+----------------------------------------------------------------------*/
double DPoint3d::Distance
(

DPoint3dCR point2

) const
    {
    double xdist, ydist, zdist;

    xdist = point2.x - x;
    ydist = point2.y - y;
    zdist = point2.z - z;

    return (sqrt (xdist * xdist + ydist * ydist + zdist * zdist));
    }


/*-----------------------------------------------------------------*//**
* @description Computes the squared distance between two points.
*
* @param [in] point2 The second point.
* @return The squared distance between the points.
* @bsimethod                                                                    EarlinLutz      12/97
+----------------------------------------------------------------------*/
double DPoint3d::DistanceSquared
(

DPoint3dCR point2

) const
    {
    double xdist, ydist, zdist;

    xdist = point2.x - x;
    ydist = point2.y - y;
    zdist = point2.z - z;

    return (xdist * xdist + ydist * ydist + zdist * zdist);
    }


/*-----------------------------------------------------------------*//**
* @description Computes the squared distance between two points, using only the
*       xy parts.
*
* @param [in] point2 The second point
* @return The squared distance between the XY projections of the two points.
*               (i.e. any z difference is ignored)
* @bsimethod                                                                    EarlinLutz      12/97
+----------------------------------------------------------------------*/
double DPoint3d::DistanceSquaredXY
(

DPoint3dCR point2

) const
    {
    double xdist, ydist;

    xdist = point2.x - x;
    ydist = point2.y - y;

    return (xdist * xdist + ydist * ydist);
    }


/*-----------------------------------------------------------------*//**
* @description Computes the distance between two points, using
*   only x and y components.
*
* @param [in] point2 The second point
* @return The distance between the XY projections of the two points.
*               (i.e. any z difference is ignored)
* @bsimethod                                                                    EarlinLutz      12/97
+----------------------------------------------------------------------*/
double DPoint3d::DistanceXY
(

DPoint3dCR point2

) const
    {
    double xdist, ydist;

    xdist = point2.x - x;
    ydist = point2.y - y;

    return sqrt (xdist * xdist + ydist * ydist);
    }

//! @description Computes the distance between two points, using
//!   only x and y components, optionally applying a transform into view space.
//!
//! @param [in] otherPoint The second point
//! @param [in] matrix optional transfromation
//! @param [out] distance computed distance.
//! @return true if both points normalized properly after the transform.
bool DPoint3d::DistanceXY
(
DPoint3dCR otherPoint,
DMatrix4dCP matrix,
double &distance
) const
    {
    if (matrix == NULL)
        {
        distance = DistanceXY (otherPoint);
        return true;
        }
    DPoint3d xyzA, xyzB;
    DPoint4d xyzwA = DPoint4d::FromMultiply (matrix, *this);
    DPoint4d xyzwB = DPoint4d::FromMultiply (matrix, otherPoint);
    if (xyzwA.GetProjectedXYZ (xyzA) && xyzwB.GetProjectedXYZ (xyzB))
        {
        distance = xyzA.DistanceXY (xyzB);
        return true;
        }
    distance = DBL_MAX;
    return false;
    }

/*-----------------------------------------------------------------*//**
* @bsimethod                                EarlinLutz      12/97
+----------------------------------------------------------------------*/
double DPoint3d::MaxAbs () const
    {
    double maxVal = fabs (x);

    if (fabs (y) > maxVal)
        maxVal = fabs (y);

    if (fabs (z) > maxVal)
        maxVal = fabs (z);

    return maxVal;
    }

/*-----------------------------------------------------------------*//**
* @bsimethod                                EarlinLutz      08/15
+----------------------------------------------------------------------*/
double DPoint3d::MaxDiff (DPoint3dCR other) const
    {
    double maxVal = fabs (x - other.x);

    double dy = fabs (y - other.y);
    if (fabs (dy) > maxVal)
        maxVal = fabs (dy);

    double dz = fabs (z - other.z);
    if (fabs (dz) > maxVal)
        maxVal = fabs (dz);

    return maxVal;
    }



/*-----------------------------------------------------------------*//**
* @bsimethod                                EarlinLutz      01/14
+----------------------------------------------------------------------*/
double DPoint3d::MinAbs () const
    {
    double maxVal = fabs (x);

    if (fabs (y) < maxVal)
        maxVal = fabs (y);

    if (fabs (z) < maxVal)
        maxVal = fabs (z);

    return maxVal;
    }

/*-----------------------------------------------------------------*//**
* @bsimethod                                EarlinLutz      01/14
+------------------------------------b----------------------------------*/
int DPoint3d::MaxAbsIndex () const
    {
    double maxVal = fabs (x);
    int index = 0;
    if (fabs (y) > maxVal)
        {
        maxVal = fabs (y);
        index = 1;
        }

    if (fabs (z) > maxVal)
        {
        maxVal = fabs (z);
        index = 2;
        }

    return index;
    }

/*-----------------------------------------------------------------*//**
* @bsimethod                                EarlinLutz      01/14
+----------------------------------------------------------------------*/
int DPoint3d::MinAbsIndex () const
    {
    double maxVal = fabs (x);
    int index = 0;
    if (fabs (y) < maxVal)
        {
        maxVal = fabs (y);
        index = 1;
        }

    if (fabs (z) < maxVal)
        {
        maxVal = fabs (z);
        index = 2;
        }

    return index;
    }

//! @description Returns the range of components.
DRange1d DPoint3d::ComponentRange () const {return DRange1d::From (x,y,z);}


/*-----------------------------------------------------------------*//**
* @description Test for exact equality between all components of two points or vectors.
* @param [in] vector2 The second point or vector
* @return true if the points are identical.
* @bsimethod                                                                    EarlinLutz      12/97
+----------------------------------------------------------------------*/
bool DPoint3d::IsEqual
(

DPoint3dCR vector2

) const
    {
    bool result;

    result =   x == vector2.x
            && y == vector2.y
            && z == vector2.z;

    return  result;
    }


/*-----------------------------------------------------------------*//**
* @description Test if the x, y, and z components of two points or vectors are
*   equal within tolerance.
* Tests are done independently using the absolute value of each component differences
* (i.e. not the magnitude or sum of squared differences)
*
* @param [in] vector2 The second point or vector.
* @param [in] tolerance The tolerance.
* @return true if all components are within given tolerance of each other.
* @bsimethod                                                                    EarlinLutz      12/97
+----------------------------------------------------------------------*/
bool DPoint3d::IsEqual
(

DPoint3dCR vector2,
double                  tolerance

) const
    {
    bool result;
    
    result = fabs (x - vector2.x) <= tolerance &&
             fabs (y - vector2.y) <= tolerance &&
             fabs (z - vector2.z) <= tolerance;

    return  result;
    }


/*-----------------------------------------------------------------*//**
* @description  Computes the coordinates of point under the translation
* and scaling that puts 000 at cube>low and 111 at cube>high.
*
* @param [in] point Point whose NPC coordinates are to be computed
* @param [in] cube Cube whose corners map to 000 and 111
* @bsimethod                                                                    EarlinLutz      12/97
+----------------------------------------------------------------------*/
void DPoint3d::NpcCoordinatesOf
(

DPoint3dCR point,
DRange3dCR cube

)
    {
    //int     result = 0;
    double  a;

    a = cube.high.x - cube.low.x;
    x = (a != 0.0) ? (point.x - cube.low.x) / a : 0.0;

    a = cube.high.y - cube.low.y;
    y = (a != 0.0) ? (point.y - cube.low.y) / a : 0.0;

    a = cube.high.z - cube.low.z;
    z = (a != 0.0) ? (point.z - cube.low.z) / a : 0.0;
    }


/*-----------------------------------------------------------------*//**
* @return true if the point has coordinates which indicate it is
*   a disconnect (separator) ponit.
* @bsimethod                                    EarlinLutz      12/97
+----------------------------------------------------------------------*/
bool DPoint3d::IsDisconnect
(

) const
    {
    return x == DISCONNECT
        || y == DISCONNECT
        || z == DISCONNECT;
    }


/*-----------------------------------------------------------------*//**
* Initialize a point with all coordinates as the disconnect value.
* @bsimethod                                    EarlinLutz      12/97
+----------------------------------------------------------------------*/
void DPoint3d::InitDisconnect
(

)
    {
    x = y = z = DISCONNECT;
    }


/*-----------------------------------------------------------------*//**
* @description Initialize a point by copying x,y,z from a vector.
* @param [out]pPoint  The point
* @param [in] vector  The vecotr
* @bsimethod                                                                    EarlinLutz      12/97
+----------------------------------------------------------------------*/
void DPoint3d::Init
(

DVec3dCR vector

)
    {
    x = vector.x;
    y = vector.y;
    z = vector.z;
    }



/*-----------------------------------------------------------------*//**
* @description Subtract a vector from a point.
*
* @param [in] base  The the first point or vector
* @param [in] vector  The second point or vector
* @bsimethod                    EarlinLutz      12/97
+----------------------------------------------------------------------*/
void DPoint3d::Subtract
(

DPoint3dCR base,
DVec3dCR vector

)
    {
    x = base.x - vector.x;
    y = base.y - vector.y;
    z = base.z - vector.z;
    }


/*-----------------------------------------------------------------*//**
* @description Adds a vector to a pointer or vector, returns the result in place.
*
* @param [in] vector  The vector to add.
* @bsimethod                                                                    EarlinLutz      12/97
+----------------------------------------------------------------------*/
void DPoint3d::Add
(

DVec3dCR vector

)
    {
    x += vector.x;
    y += vector.y;
    z += vector.z;
    }

/*-----------------------------------------------------------------*//**
* @description Adds a vector to a pointer or vector, returns the result in place.
*
* @param [in] delta  The vector to add.
* @bsimethod                                                                    EarlinLutz      12/97
+----------------------------------------------------------------------*/
void DPoint3d::AddToArray
(
DPoint3dP points,
int n,
DPoint3dCR delta
)
    {
    for (int i = 0; i < n; i++)
        {
        points[i].x += delta.x;
        points[i].y += delta.y;
        points[i].z += delta.z;
        }
    }


/*-----------------------------------------------------------------*//**
* @description Adds an origin and a scaled vector.
*
* @param [in] origin  Origin for the sum.
* @param [in] vector  The vector to be added.
* @param [in] scale  The scale factor.
* @bsimethod                                                                    EarlinLutz      12/97
+----------------------------------------------------------------------*/
void DPoint3d::SumOf
(

DPoint3dCR origin,
DVec3dCR vector,
double   scale

)
    {
    x = origin.x + vector.x * scale;
    y = origin.y + vector.y * scale;
    z = origin.z + vector.z * scale;
    }


/*-----------------------------------------------------------------*//**
* @description Adds an origin and two scaled vectors.
*
* @param [in] origin  The origin.  May be null.
* @param [in] vector1  The first direction vector
* @param [in] scale1  The first scale factor
* @param [in] vector2  The second direction vector
* @param [in] scale2  The second scale factor
* @bsimethod                                                                    EarlinLutz      12/97
+----------------------------------------------------------------------*/
void DPoint3d::SumOf
(

DPoint3dCR origin,
DVec3dCR vector1,
        double           scale1,
DVec3dCR vector2,
        double           scale2

)
    {
    x = origin.x + vector1.x * scale1 + vector2.x * scale2;
    y = origin.y + vector1.y * scale1 + vector2.y * scale2;
    z = origin.z + vector1.z * scale1 + vector2.z * scale2;
    }


/*-----------------------------------------------------------------*//**
* @description Adds an origin and three scaled vectors.
*
* @param [in] origin  The origin. May be null
* @param [in] vector1  The first direction vector
* @param [in] scale1  The first scale factor
* @param [in] vector2  The second direction vector
* @param [in] scale2  The second scale factor
* @param [in] vector3  The third direction vector
* @param [in] scale3  The third scale factor
* @bsimethod                                                                    EarlinLutz      12/97
+----------------------------------------------------------------------*/
void DPoint3d::SumOf
(

DPoint3dCR origin,
DVec3dCR vector1,
        double          scale1,
DVec3dCR vector2,
        double          scale2,
DVec3dCR vector3,
        double          scale3

)
    {
    x = origin.x + vector1.x * scale1 + vector2.x * scale2 + vector3.x * scale3;
    y = origin.y + vector1.y * scale1 + vector2.y * scale2 + vector3.y * scale3;
    z = origin.z + vector1.z * scale1 + vector2.z * scale2 + vector3.z * scale3;
    }


/*-----------------------------------------------------------------*//**
* @vbdescription Returns the cross (vector) cross product of two vectors.
* @param [in] vector1 The first vector
* @param [in] vector2 The second vector
* @bsimethod                                    EarlinLutz      12/97
+----------------------------------------------------------------------*/
void DPoint3d::CrossProduct
(

DPoint3dCR vector1,
DPoint3dCR vector2

)
    {
    double xx = vector1.y * vector2.z - vector1.z * vector2.y;
    double yy = vector1.z * vector2.x - vector1.x * vector2.z;
    double zz = vector1.x * vector2.y - vector1.y * vector2.x;
    x = xx;
    y = yy;
    z = zz;
    }


/*-----------------------------------------------------------------*//**
* @description Returns the (vector) cross product of two vectors.
*   The vectors are computed from the Origin to Target1 and Target2.
* @param [in] origin The base point for computing vectors.
* @param [in] target1 The target point for the first vector.
* @param [in] target2 The target point for the second vector.
* @bsimethod                                                                    EarlinLutz      12/97
+----------------------------------------------------------------------*/
void DPoint3d::CrossProductToPoints
(

DPoint3dCR origin,
DPoint3dCR target1,
DPoint3dCR target2

)
    {
    double x1 = target1.x - origin.x;
    double y1 = target1.y - origin.y;
    double z1 = target1.z - origin.z;

    double x2 = target2.x - origin.x;
    double y2 = target2.y - origin.y;
    double z2 = target2.z - origin.z;

    x = y1 * z2 - z1 * y2;
    y = z1 * x2 - x1 * z2;
    z = x1 * y2 - y1 * x2;
    }


/*-----------------------------------------------------------------*//**
* @description Return the (scalar) cross product of the xy parts of two vectors.
* @param [in] pVector1 The first vector
* @param [in] vector2 The second vector
* @return The 2d cross product.
* @bsimethod                                                                    EarlinLutz      12/97
+----------------------------------------------------------------------*/
double DPoint3d::CrossProductXY
(

DPoint3dCR vector2

) const
    {
    return  x * vector2.y - y * vector2.x;
    }


/*-----------------------------------------------------------------*//**
* @description Compute the normalized cross product of two vectors
* and return the length of the unnormalized cross product.
*
* @param [in] vector1 The first vector
* @param [in] vector2 The second vector
* @return The length of the original (prenormalization) cross product vector
* @bsimethod                                                                    EarlinLutz      12/97
+----------------------------------------------------------------------*/
double DPoint3d::NormalizedCrossProduct
(

DPoint3dCR vector1,
DPoint3dCR vector2

)
    {
    this->CrossProduct(vector1, vector2);
    return this->Normalize();
    }


/*-----------------------------------------------------------------*//**
* @description Computes the cross product of the two parameter vectors and scales it to a given
* length.  The scaled vector is stored as the product vector, and the length of the original
* cross product vector is returned.
*
* @param [in] vector1 The first vector
* @param [in] vector2 The second vector
* @param [in] productLength The Desired length
* @return The length of original vector.
* @bsimethod                                                                    EarlinLutz      12/97
+----------------------------------------------------------------------*/
double DPoint3d::SizedCrossProduct
(

DPoint3dCR vector1,
DPoint3dCR vector2,
double productLength

)
    {
    double length;
    this->CrossProduct(vector1, vector2);
    length = this->Magnitude ();
    if (length != 0)
        {
        this->Scale (*this, productLength / length);
        }
    return length;
    }


/*-----------------------------------------------------------------*//**
* @description Computes the cross product of two vectors and scales it to the
* geometric mean of the lengths of the two vectors.  This is useful
* because it has the direction of the cross product (i.e. normal to the plane
* of the two vectors) and a size in between the two vectors.
*
* @param [in] vector1 The first vector
* @param [in] vector2 The second vector
* @return The length of original vector.
* @bsimethod                                                                    EarlinLutz      12/97
+----------------------------------------------------------------------*/
double DPoint3d::GeometricMeanCrossProduct
(

DPoint3dCR vector1,
DPoint3dCR vector2

)
    {
    double length;
    double aa, bb, c;
    this->CrossProduct(vector1, vector2);
    length = this->Magnitude ();
    aa = vector1.MagnitudeSquared ();
    bb = vector2.MagnitudeSquared ();
    if (length != 0)
        {
        c = sqrt (sqrt (aa * bb));
        this->Scale (*this, c / length);
        }
    return length;
    }


/*-----------------------------------------------------------------*//**
* @description Returns the (scalar) dot product of two vectors.
* @param [in] vector2 The second vector
* @return The dot product of the two vectors
* @bsimethod                                                                    EarlinLutz      12/97
+----------------------------------------------------------------------*/
double DPoint3d::DotProduct
(

DPoint3dCR vector2

) const
    {
    return (x * vector2.x + y * vector2.y + z * vector2.z);
    }


/*-----------------------------------------------------------------*//**
* @description Returns the (scalar) dot product of xy parts of two vectors.
* @param [in] vector2 The second vector
* @return The dot product of the xy parts of the two vectors
* @bsimethod                                                                    EarlinLutz      12/97
+----------------------------------------------------------------------*/
double DPoint3d::DotProductXY
(

DPoint3dCR vector2

) const
    {
    return (x * vector2.x + y * vector2.y);
    }


/*-----------------------------------------------------------------*//**
* @description Computes the dot product of one vector given as a point structure and another given as
* xyz components.
* @param [in] ax The x component of second vector.
* @param [in] ay The y component of second vector.
* @param [in] az The z component of second vector.
* @return The dot product of the vector with a vector with the given components
* @bsimethod                                                                    EarlinLutz      12/97
+----------------------------------------------------------------------*/
double DPoint3d::DotProduct
(

        double    ax,
        double    ay,
        double    az

) const
    {
    return x * ax + y * ay + z * az;
    }



/*-----------------------------------------------------------------*//**
* @description Computes a unit vector  in the direction of the difference of the points
* or vectors (Second parameter vector is subtracted from the first parameter vector,
* exactly as in the subtract function.)
*
* @param [in] target The target point.
* @param [in] origin The origin point.
* @return The length of original difference vector.
* @bsimethod                                            EarlinLutz      12/97
+----------------------------------------------------------------------*/
double DPoint3d::NormalizedDifference
(

DPoint3dCR target,
DPoint3dCR origin

)
    {
    this->DifferenceOf (target, origin);
    return this->Normalize ();
    }


/*-----------------------------------------------------------------*//**
* @description Returns the angle between two vectors.  This angle is between 0 and
* pi.  Rotating the first vector by this angle around the cross product
* between the vectors aligns it with the second vector.
*
* @param [in] vector2 The second vector
* @return The angle between the vectors.
* @bsimethod                                                                    EarlinLutz      12/97
+----------------------------------------------------------------------*/
double DPoint3d::AngleTo
(

DPoint3dCR vector2

) const
    {
    DPoint3d   crossProd;
    double cross, dot;
    crossProd.CrossProduct (*this, vector2);
    cross   = crossProd.Magnitude ();
    dot     = this->DotProduct (vector2);
    return  Angle::Atan2 (cross, dot);
    }
/*-----------------------------------------------------------------*//**
* @bsimethod                            EarlinLutz      09/12
+----------------------------------------------------------------------*/
double DPoint3d::AngleXY () const
    {
    return  Angle::Atan2 (y, x);
    }



/*-----------------------------------------------------------------*//**
* @description Returns the angle between two vectors, choosing the smaller
*   of the two possible angles when both the vectors and their negations are considered.
*    This angle is between 0 and pi/2.
*
* @param [in] vector2 The second vector
* @return The angle between the vectors.
* @bsimethod                              EarlinLutz    12/97
+----------------------------------------------------------------------*/
double DPoint3d::SmallerUnorientedAngleTo
(

DPoint3dCR vector2

) const
    {
    DPoint3d   crossProd;
    double cross, dot;
    crossProd.CrossProduct (*this, vector2);
    cross   = crossProd.Magnitude ();
    dot     = this->DotProduct (vector2);
    return  Angle::Atan2 (cross, fabs (dot));
    }


/*-----------------------------------------------------------------*//**
* @description Test a vector is "between" vector0 and vector1.
* If the vectors are coplanar and vector0 is neither parallel nor antiparallel
* to vector1, betweenness has the expected meaning: there are two angles between
* vector0 and vector1; one is less than 180; the test vector is tested to
* see if it is in the smaller angle.
* If the vectors are not coplanar, the test is based on the projection of
* the test vector into the plane of the other two vectors.
*
* Zero testing is untoleranced, and is biased to all parallel conditions "false".
* That is, if any pair of the input vectors is parallel or antiparallel,
* the mathematical answer is false.  Floating point tolerances
* will cause "nearby" cases to be unpredictable.  It is assumed that if
* the caller considers the "parallel" cases important they will be
* checked explicitly.
*
* @param [in] vector0 The first boundary vector.
* @param [in] vector1 The second boundary vector.
* @return true if the test vector is within the angle.
* @bsimethod                                                                    EarlinLutz      12/97
+----------------------------------------------------------------------*/
bool DPoint3d::IsVectorInSmallerSector
(

DPoint3dCR vector0,
DPoint3dCR vector1

) const
    {
    DPoint3d   cross01;
    cross01.CrossProduct (vector0, vector1);
    return      vector0.TripleProduct (*this, cross01) > 0.0
            &&  this->TripleProduct (vector1, cross01) > 0.0;
    }


/*-----------------------------------------------------------------*//**
* @description Test if the test vector vector is "between" vector0 and vector1, with CCW direction
* resolved by an up vector.  The cross product of vector0 and vector1 is
* considered the positive plane normal if its dot product with the up vector
* is positive.
*
* @param [in] vector0 The boundary vector.
* @param [in] vector1 The boundary vector.
* @param [in] upVector The out of plane vector.  If null, the z vector is used.
* @return true if test vector is within the angle.
* @bsimethod                                                                    EarlinLutz      12/97
+----------------------------------------------------------------------*/
bool DPoint3d::IsVectorInCCWSector
(

DPoint3dCR vector0,
DPoint3dCR vector1,
DPoint3dCR upVector

) const
    {
    DPoint3d    cross01;
    double      dot;

    cross01.CrossProduct (vector0, vector1);

    if (cross01.MagnitudeSquared () == 0.0)
        {
        dot = vector0.DotProduct (vector1);
        if (dot > 0.0)
            return false;
        }

    dot = cross01.DotProduct (upVector);

    if (dot > 0.0)
        return  vector0.TripleProduct (*this, cross01) > 0.0
            &&  this->TripleProduct (vector1, cross01) > 0.0;
    else
        return  vector0.TripleProduct (*this, cross01) < 0.0
            ||  this->TripleProduct (vector1, cross01) < 0.0;
    }


/*-----------------------------------------------------------------*//**
* @description Returns the angle from Vector1 to Vector2 using only xy parts.
*  This angle is between -pi and +pi.
*
* @param [in] vector2 The second vector
* @return The angle between vectors.
* @bsimethod                                                                    EarlinLutz      12/97
+----------------------------------------------------------------------*/
double DPoint3d::AngleToXY
(

DPoint3dCR vector2

) const
    {
    double cross, dot;
    cross = this->CrossProductXY (vector2);
    dot   = this->DotProductXY (vector2);
    return  Angle::Atan2 (cross, dot);
    }


/*-----------------------------------------------------------------*//**
* @description Returns the angle between two vectors, considering both
*   the vectors and their negations and choosing the smaller.
*   This angle is between 0 and pi/2.
*
* @param [in] vector2 The second vector
* @return The angle between vectors.
* @bsimethod                                    EarlinLutz      12/97
+----------------------------------------------------------------------*/
double DPoint3d::SmallerUnorientedAngleToXY
(

DPoint3dCR vector2

) const
    {
    double cross, dot;
    cross = this->CrossProductXY (vector2);
    dot   = this->DotProductXY (vector2);
    return  Angle::Atan2 (fabs (cross), fabs(dot));
    }


/*-----------------------------------------------------------------*//**
* @description Rotate a vector around the z axis.
* @param [in] theta The rotation angle.
* @bsimethod                            DavidAssaf      6/99
+----------------------------------------------------------------------*/
void DPoint3d::RotateXY
(

DPoint3dCR vector,
        double      theta

)
    {
    double c, s, xx, yy;
    s = sin (theta);
    c = cos (theta);

    xx = vector.x;
    yy = vector.y;

    x = xx * c - yy * s;
    y = xx * s + yy * c;
    z = vector.z;
    }


/*-----------------------------------------------------------------*//**
* @description Rotate a vector around the z axis.
* @param [in] theta The rotation angle.
* @bsimethod                                                                    DavidAssaf      6/99
+----------------------------------------------------------------------*/
void DPoint3d::RotateXY
(

        double      theta

)
    {
    double c, s, xx, yy;
    s = sin (theta);
    c = cos (theta);

    xx = x;
    yy = y;

    x = xx * c - yy * s;
    y = xx * s + yy * c;
    }


/*-----------------------------------------------------------------*//**
* @description Computes the signed from one vector to another, in the plane
*       of the two vectors.   Initial computation using only the two vectors
*       yields two possible angles depending on which side of the plane of the
*       vectors is viewed.  To choose which side to view, go on the side whose
*       normal has a positive dot product with the orientation vector.
* This angle can be between -pi and +pi.
*
* @param [in] vector2 The second vector
* @param [in] orientationVector The vector used to determine orientation.
* @return The signed angle
* @bsimethod                                                                    EarlinLutz      12/97
+----------------------------------------------------------------------*/
double DPoint3d::SignedAngleTo
(

DPoint3dCR vector2,
DPoint3dCR orientationVector

) const
    {
    DPoint3d   crossProd;
    double cross, dot, theta;
    crossProd.CrossProduct (*this, vector2);
    cross   = crossProd.Magnitude ();
    dot     = this->DotProduct (vector2);
    theta   = Angle::Atan2 (cross, dot);

    if (crossProd.DotProduct (orientationVector) < 0.0)
        return  -theta;
    else
        return  theta;
    }


/*-----------------------------------------------------------------*//**
* @description Computes the signed angle between the projection of two vectors
*       onto a plane with given normal.
*
* @param [in] vector2 The second vector
* @param [in] pNormal The plane normal vector
* @return The angle in plane
* @bsimethod                                                                    EarlinLutz      12/97
+----------------------------------------------------------------------*/
double DPoint3d::PlanarAngleTo
(

DPoint3dCR vector2,
DPoint3dCR planeNormal

) const
    {
    DPoint3d  projection1, projection2;
    double    square = planeNormal.DotProduct (planeNormal);
    double    projectionFactor1, projectionFactor2;
    double    factor;

    if (square == 0.0)
        return 0.0;

    factor = 1.0 / square;

    projectionFactor1 = - this->DotProduct (planeNormal) * factor;
    projectionFactor2 = - vector2.DotProduct (planeNormal) * factor;

    projection1.SumOf (*this, planeNormal, projectionFactor1);
    projection2.SumOf (vector2, planeNormal, projectionFactor2);

    return  projection1.SignedAngleTo (projection2, planeNormal);
    }


/*-----------------------------------------------------------------*//**
* @description Scale each point by the other's weight and subtract, i.e. form
* (point1 * point2.w - point2 * point1.w).  The weight term
* vanishes.   Copy the xyz parts back as a vector.
*
* @param [in] point1 The first point
* @param [in] pTarget2 The second pont.
* @bsimethod                                                                    EarlinLutz      12/97
+----------------------------------------------------------------------*/
void DPoint3d::WeightedDifferenceOf
(

DPoint4dCR point1,
DPoint4dCR point2

)
    {
    double w2 = point2.w;
    double w1 = point1.w;
    x = point1.x * w2 - point2.x * w1;
    y = point1.y * w2 - point2.y * w1;
    z = point1.z * w2 - point2.z * w1;
    }


/*-----------------------------------------------------------------*//**
* @description Form the cross product of the weighted differences from point0 to point1 and point2.
*
* @param [in] basePoint The common base point (second point for differences)
* @param [in] target1 The first target point.
* @param [in] target2 The second target point.
* @bsimethod                                                                    EarlinLutz      12/97
+----------------------------------------------------------------------*/
void DPoint3d::WeightedDifferenceCrossProduct
(

DPoint4dCR basePoint,
DPoint4dCR target1,
DPoint4dCR target2

)
    {
    DPoint3d U, V;
    U.WeightedDifferenceOf (target1, basePoint);
    V.WeightedDifferenceOf (target2, basePoint);
    this->CrossProduct (U, V);
    }


/*-----------------------------------------------------------------*//**
* @description Computes the squared magnitude of a vector.
*
* @return The squared magnitude of the vector.
* @bsimethod                                                                    EarlinLutz      12/97
+----------------------------------------------------------------------*/
double DPoint3d::MagnitudeSquared
(

) const
    {
    return   x * x + y * y + z * z;
    }


/*-----------------------------------------------------------------*//**
* @description Computes the magnitude of the xy part of a vector.
* @return The magnitude of the xy parts of the given vector.
* @bsimethod                                                                    EarlinLutz      12/97
+----------------------------------------------------------------------*/
double DPoint3d::MagnitudeXY
(

) const
    {
    return sqrt (x * x + y * y);
    }


/*-----------------------------------------------------------------*//**
* @description Computes the squared magnitude of the xy part of a vector.
* @return The squared magnitude of the xy parts of the given vector.
* @bsimethod                                                                    EarlinLutz      12/97
+----------------------------------------------------------------------*/
double DPoint3d::MagnitudeSquaredXY
(

) const
    {
    return x * x + y * y;
    }


/*-----------------------------------------------------------------*//**
* @description Compute a unit vector perpendicular to the xy parts of given vector.
* @param [in] vector The source vector
* @return true if the input vector has nonzero length
* @bsimethod                                                                    EarlinLutz      12/97
+----------------------------------------------------------------------*/
bool DPoint3d::UnitPerpendicularXY
(

DPoint3dCR vector

)
    {
    double  a, d2, xx = vector.x, yy = vector.y;
    x = -yy;
    y =  xx;
    z = 0.0;
    d2 = xx * xx + yy * yy;
    if (d2 == 0.0)
        return false;
    a = 1.0 / sqrt (d2);
    x *= a;
    y *= a;
    return true;
    }


/*-----------------------------------------------------------------*//**
* @description Computes the magnitude of a vector.
* @return The length of the vector
* @bsimethod                                                                    EarlinLutz      12/97
+----------------------------------------------------------------------*/
double DPoint3d::Magnitude
(

) const
    {
    return  sqrt (x * x + y * y + z * z);
    }


/*-----------------------------------------------------------------*//**
* @description Multiplies a vector by a scale factor.
* @param [in] vector The vector to be scaled.
* @param [in] scale The scale factor.
* @bsimethod                                                                    EarlinLutz      12/97
+----------------------------------------------------------------------*/
void DPoint3d::Scale
(

DPoint3dCR vector,
double     scale

)
    {
    x = vector.x * scale;
    y = vector.y * scale;
    z = vector.z * scale;
    }


/*-----------------------------------------------------------------*//**
* @description Multiplies a vector (in place) by a scale factor.
* @param [in] scale The scale
* @bsimethod                                                                    EarlinLutz      12/97
+----------------------------------------------------------------------*/
void DPoint3d::Scale
(

double scale

)
    {
    x *= scale;
    y *= scale;
    z *= scale;
    }


/*-----------------------------------------------------------------*//**
* @description Computes a negated (opposite) vector.
*
* @param [in] vector The vector to be negated.
* @bsimethod                                                                    EarlinLutz      12/97
+----------------------------------------------------------------------*/
void DPoint3d::Negate
(

DPoint3dCR vector

)
    {
    x = - vector.x;
    y = - vector.y;
    z = - vector.z;
    }


/*-----------------------------------------------------------------*//**
* @description Negate a vector in place.
*
* @bsimethod                                                                    EarlinLutz      12/97
+----------------------------------------------------------------------*/
void DPoint3d::Negate
(

)
    {
    x = - x;
    y = - y;
    z = - z;
    }


/*-----------------------------------------------------------------*//**
* @description Normalizes (scales) a vector to length 1.
* If the input vector length is 0, the output vector is a zero vector
* and the returned length is 0.
*
* @param [in] vector The vector to be normalized.
* @return The length prior to normalization
* @bsimethod                                                                    EarlinLutz      12/97
+----------------------------------------------------------------------*/
double DPoint3d::Normalize
(

DPoint3dCR vector

)
    {
    double  magnitude =
            sqrt  (vector.x * vector.x
                 + vector.y * vector.y
                 + vector.z * vector.z);


    if (magnitude > 0.0)
        {
        double f = 1.0 / magnitude;
        x = vector.x * f;
        y = vector.y * f;
        z = vector.z * f;
        }
    else
        {
        x = vector.x;
        y = vector.y;
        z = vector.z;
        }

    return  magnitude;
    }


/*-----------------------------------------------------------------*//**
* @description Scales a vector to specified length.
* If the input vector length is 0, the output vector is a zero vector
* and the returned length is 0.
*
* @param [in] vector The original vector.
* @param [in] length The requested length.
* @return The length prior to scaling.
* @bsimethod                                                                    EarlinLutz      01/99
+----------------------------------------------------------------------*/
double DPoint3d::ScaleToLength
(

DPoint3dCR vector,
      double    length

)
    {
    double  magnitude =
            sqrt  (vector.x * vector.x
                 + vector.y * vector.y
                 + vector.z * vector.z);

    if (magnitude > 0.0)
        {
        double f = length / magnitude;
        x = vector.x * f;
        y = vector.y * f;
        z = vector.z * f;
        }
    else
        {
        x = length;
        y = 0.0;
        z = 0.0;
        }

    return  magnitude;
    }


/*-----------------------------------------------------------------*//**
* @description Scales a vector to a specified length, and returns
* the prior length.
* If the input vector length is 0, the output vector is a zero vector
* and the returned length is 0.
*
* @param [in] length The requested length
* @return The length prior to scaling.
* @bsimethod                                                                    EarlinLutz      01/99
+----------------------------------------------------------------------*/
double DPoint3d::ScaleToLength
(

double length

)
    {
    return this->ScaleToLength (*this, length);
    }


/*-----------------------------------------------------------------*//**
* @description Replaces a vector by a unit vector in the same direction, and returns
* the original length.
* If the input vector length is 0, the output vector is a zero vector
* and the returned length is 0.
*
* @return The length prior to normalization
* @bsimethod                                                                    EarlinLutz      12/97
+----------------------------------------------------------------------*/
double DPoint3d::Normalize
(

)
    {
    double  magnitude = sqrt (x * x + y * y + z * z);

    if (magnitude > 0.0)
        {
        double f = 1.0 / magnitude;
        x *= f;
        y *= f;
        z *= f;
        }

    return  magnitude;
    }


/*-----------------------------------------------------------------*//**
* @description Tests if two vectors are parallel.
*
* @param [in] vector2 The second vector
* @return true if the vectors are parallel within tolerance
* @bsimethod                                                                    EarlinLutz      12/97
+----------------------------------------------------------------------*/
bool DPoint3d::IsParallelTo
(

DPoint3dCR vector2

) const
    {
    DPoint3d    vecC;
    double      a2 = this->DotProduct (*this);
    double      b2 = vector2.DotProduct (vector2);
    double      cross;
    double      eps = Angle::SmallAngle (); /* small angle tolerance (in radians) */
    vecC.CrossProduct (*this, vector2);
    cross = vecC.MagnitudeSquared ();

    /* a2,b2,c2 are squared lengths of respective vectors */
    /* c2 = sin^2(theta) * a2 * b2 */
    /* For small theta, sin^2(theta)~~theta^2 */
    /* Zero-length vector case falls through as equality */

    return  cross <= eps * eps * a2 * b2;
    }


/*-----------------------------------------------------------------*//**
* @description Tests if two vectors are perpendicular.
*
* @param [in] vector2 The second vector
* @return true if vectors are perpendicular within tolerance
* @bsimethod                                                                    EarlinLutz      12/97
+----------------------------------------------------------------------*/
bool DPoint3d::IsPerpendicularTo
(

DPoint3dCR vector2

) const
    {
    double      aa = this->DotProduct (*this);
    double      bb = vector2.DotProduct (vector2);
    double      ab = this->DotProduct (vector2);
    double      eps = Angle::SmallAngle ();

    return  ab * ab <= eps * eps * aa * bb;
    }


/*-----------------------------------------------------------------*//**
* @description Try to divide each component of a vector by a scalar.  If the denominator
* near zero compared to any numerator component, return the original
* vector.
* @param [out] pScaledVector The vector after scaling.
* @param [in] vector The initial vector.
* @param [in] denominator The divisor.
* @return true if division is numerically safe.
* @bsimethod                                    EarlinLutz      12/97
+----------------------------------------------------------------------*/
bool DPoint3d::SafeDivide
(

DPoint3dCR vector,
double  denominator

)
    {
    static double s_relTol = 1.0e-12;
    double absD = fabs (denominator);
    double tol = s_relTol * vector.MaxAbs ();

    if (absD <= tol)
        {
        *this = vector;
        return false;
        }
    else
        {
        double a = 1.0 / denominator;
        x = vector.x * a;
        y = vector.y * a;
        z = vector.z * a;
        return true;
        }
    }


/*-----------------------------------------------------------------*//**
* @description Computes the triple product of three vectors.
* The following are equivalent definitions of the triple product of three
* vectors V1, V2, and V3:
*
*<UL>
*<LI> (V1 cross V2) dot V3
*<LI> V1 dot (V2 cross V3)
*<LI>The determinant of the 3x3 matrix with the three vectors as its
*               columns.
*<LI>The determinant of the 3x3 matrix with the three vectors as its
*               rows.
*<LI>The (signed)volume of the parallelepiped whose 4 vertices are at the
*               origin and at the ends of the 3 vectors placed at the
*               origin.
*</UL>
*
* @param [in] vector2 The second vector.
* @param [in] vector3 The third vector.
* @return The triple product
* @bsimethod                                                                    EarlinLutz      12/97
+----------------------------------------------------------------------*/
double DPoint3d::TripleProduct
(

DPoint3dCR vector2,
DPoint3dCR vector3

) const
    {
    return
          x * (vector2.y * vector3.z - vector2.z * vector3.y)
        + y * (vector2.z * vector3.x - vector2.x * vector3.z)
        + z * (vector2.x * vector3.y - vector2.y * vector3.x);
    }


/*-----------------------------------------------------------------*//**
* @description Subtract two points or vectors, and return the result in
*           place of the first.
*
* @param [in] vector2 The vector to subtract.
* @bsimethod                                                                    EarlinLutz      12/97
+----------------------------------------------------------------------*/
void DPoint3d::Subtract
(

DPoint3dCR vector2

)
    {
    x -= vector2.x;
    y -= vector2.y;
    z -= vector2.z;
    }


/*-----------------------------------------------------------------*//**
* @description Subtract coordinates of two vectors or points. (Compute Point1 - Point2)
*
* @param [in] point1 The first point
* @param [in] point2 The second (subtracted) point.
* @bsimethod                                                                    EarlinLutz      12/97
+----------------------------------------------------------------------*/
void DPoint3d::DifferenceOf
(

DPoint3dCR point1,
DPoint3dCR point2

)
    {
    x = point1.x - point2.x;
    y = point1.y - point2.y;
    z = point1.z - point2.z;
    }


/*-----------------------------------------------------------------*//**
* @description Adds an origin and a scaled vector.
*
* @param [in] origin Origin for the sum.
* @param [in] vector The vector to be added.
* @param [in] scale The scale factor.
* @bsimethod                                                                    EarlinLutz      12/97
+----------------------------------------------------------------------*/
void DPoint3d::SumOf
(

DPoint3dCR origin,
DPoint3dCR vector,
double     scale

)
    {
    x = origin.x + vector.x * scale;
    y = origin.y + vector.y * scale;
    z = origin.z + vector.z * scale;
    }


/*-----------------------------------------------------------------*//**
* @description Adds an origin and two scaled vectors.
*
* @param [in] origin The origin.  May be null.
* @param [in] vector1 The first direction vector
* @param [in] scale1 The first scale factor
* @param [in] vector2 The second direction vector
* @param [in] scale2 The second scale factor
* @bsimethod                                                                    EarlinLutz      12/97
+----------------------------------------------------------------------*/
void DPoint3d::SumOf
(

DPoint3dCR origin,
DPoint3dCR vector1,
        double           scale1,
DPoint3dCR vector2,
        double           scale2

)
    {
    x = origin.x + vector1.x * scale1 + vector2.x * scale2;
    y = origin.y + vector1.y * scale1 + vector2.y * scale2;
    z = origin.z + vector1.z * scale1 + vector2.z * scale2;
    }


/*-----------------------------------------------------------------*//**
* @description Adds an origin and three scaled vectors.
*
* @param [in] origin The origin. May be null
* @param [in] vector1 The first direction vector
* @param [in] scale1 The first scale factor
* @param [in] vector2 The second direction vector
* @param [in] scale2 The second scale factor
* @param [in] vector3 The third direction vector
* @param [in] scale3 The third scale factor
* @bsimethod                                                                    EarlinLutz      12/97
+----------------------------------------------------------------------*/
void DPoint3d::SumOf
(

DPoint3dCR origin,
DPoint3dCR vector1,
        double          scale1,
DPoint3dCR vector2,
        double          scale2,
DPoint3dCR vector3,
        double          scale3

)
    {
    x = origin.x + vector1.x * scale1 + vector2.x * scale2 + vector3.x * scale3;
    y = origin.y + vector1.y * scale1 + vector2.y * scale2 + vector3.y * scale3;
    z = origin.z + vector1.z * scale1 + vector2.z * scale2 + vector3.z * scale3;
    }


/*-----------------------------------------------------------------*//**
* @description Adds a vector to a pointer or vector, returns the result in place.
*
* @param [in] vector The vector to add.
* @bsimethod                                                                    EarlinLutz      12/97
+----------------------------------------------------------------------*/
void DPoint3d::Add
(

DPoint3dCR vector

)
    {
    x += vector.x;
    y += vector.y;
    z += vector.z;
    }



/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
void DPoint3d::SumOf (DPoint3dCR point1, DPoint3dCR point2)
    {
    x = point1.x + point2.x;
    y = point1.y + point2.y;
    z = point1.z + point2.z;
    }




/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
void DPoint3d::SumOf (DPoint3dCR point1, double a1, DPoint3dCR point2, double a2)
    {
    x = point1.x * a1 + point2.x * a2;
    y = point1.y * a1 + point2.y * a2;
    z = point1.z * a1 + point2.z * a2;
    }




/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
void DPoint3d::SumOf (DPoint3dCR point1, double a1, DPoint3dCR point2, double a2, DPoint3dCR point3, double a3)
    {
    x = point1.x * a1 + point2.x * a2 + point3.x * a3;
    y = point1.y * a1 + point2.y * a2 + point3.y * a3;
    z = point1.z * a1 + point2.z * a2 + point3.z * a3;
    }



/*-----------------------------------------------------------------*//**
* @description Multiply each point in an array by its corresponding scale factor.
* @param pDest <= destination array.
* @param pSource => source array.
* @param pScales => scale factors
* @param n => number of points.
* @bsimethod                                    EarlinLutz      08/07
+----------------------------------------------------------------------*/
void DPoint3d::MultiplyArrayByScales
(
DPoint3dP pDest,
DPoint3dCP pSource,
double     *pScales,
int        n
)
    {
    for (int i = 0; i < n; i++)
        {
        double a = pScales[i];
        pDest[i].x = pSource[i].x * a;
        pDest[i].y = pSource[i].y * a;
        pDest[i].z = pSource[i].z * a;
        }
    }


/*-----------------------------------------------------------------*//**
* @description Divide each point in an array by its corresponding scale factor.
    Leave any point with near zero weight unchanged.
* @param pDest <= destination array.
* @param pSource => source array.
* @param pScales => scale factors
* @param n => number of points.
* @bsimethod                                     EarlinLutz      08/07
+----------------------------------------------------------------------*/
void DPoint3d::DivideArrayByScales
(
DPoint3dP pDest,
DPoint3dCP pSource,
double     *pScales,
int        n
)
    {
    double tol = Angle::SmallAngle ();
    for (int i = 0; i < n; i++)
        {
        double a = pScales[i];
        if (fabs (a) >= tol)
            {
            a = 1.0 / a;
            pDest[i].x = pSource[i].x * a;
            pDest[i].y = pSource[i].y * a;
            pDest[i].z = pSource[i].z * a;
            }
        }
    }


//! return product of transform times point given as components
//! @param [in] transform affine transform.
//! @param [in] point point to transform.
DPoint3d DPoint3d::FromProduct
(
TransformCR transform,
double x,
double y,
double z
)
    {
    DPoint3d result;
    result.x = transform.form3d[0][3]
                + transform.form3d[0][0] * x
                + transform.form3d[0][1] * y
                + transform.form3d[0][2] * z;
    result.y = transform.form3d[1][3]
                + transform.form3d[1][0] * x
                + transform.form3d[1][1] * y
                + transform.form3d[1][2] * z;
    result.z = transform.form3d[2][3]
                + transform.form3d[2][0] * x
                + transform.form3d[2][1] * y
                + transform.form3d[2][2] * z;
    return result;
    }




/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
DPoint3d DPoint3d::FromProduct
(
DPoint3dCR  origin,
RotMatrixCR matrix,
double x,
double y,
double z
)
    {
    DPoint3d result;
    result.x = origin.x
                + matrix.form3d[0][0] * x
                + matrix.form3d[0][1] * y
                + matrix.form3d[0][2] * z;
    result.y = origin.y
                + matrix.form3d[1][0] * x
                + matrix.form3d[1][1] * y
                + matrix.form3d[1][2] * z;
    result.z = origin.z
                + matrix.form3d[2][0] * x
                + matrix.form3d[2][1] * y
                + matrix.form3d[2][2] * z;
    return result;
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
DPoint3d DPoint3d::FromProduct
(
DPoint3dCR  origin,
RotMatrixCR matrix,
DVec3dCR vector
)
    {
    DPoint3d result;
    result.x = origin.x
                + matrix.form3d[0][0] * vector.x
                + matrix.form3d[0][1] * vector.y
                + matrix.form3d[0][2] * vector.z;
    result.y = origin.y
                + matrix.form3d[1][0] * vector.x
                + matrix.form3d[1][1] * vector.y
                + matrix.form3d[1][2] * vector.z;
    result.z = origin.z
                + matrix.form3d[2][0] * vector.x
                + matrix.form3d[2][1] * vector.y
                + matrix.form3d[2][2] * vector.z;
    return result;
    }



//! interpolate between points.
//! @param [in] pointA start point
//! @param [in] fraction fractional parameter
//! @param [in] pointB end point
DPoint3d DPoint3d::FromInterpolate
(
DPoint3dCR pointA,
double     fraction,
DPoint3dCR pointB
)
    {
    DPoint3d result;
    result.Interpolate (pointA, fraction, pointB);
    return result;
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      11/2016
+--------------------------------------------------------------------------------------*/
DPoint3d DPoint3d::FromInterpolateAndPerpendicularXY
(
DPoint3dCR pointA,
double     fraction,
DPoint3dCR pointB,
double fractionXYPerp
)
    {
    DVec3d vector = pointB - pointA;
    DPoint3d result;
    result.Interpolate (pointA, fraction, pointB);
    result.x -= fractionXYPerp * vector.y;
    result.y += fractionXYPerp * vector.x;
    return result;
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      11/2016
+--------------------------------------------------------------------------------------*/
DPoint3d DPoint3d::FromWeightedAverage
(
DPoint3dCR pointA,  //!< [in] first point
double weightA,     //!< [in] weight of first point.
DPoint3dCR pointB,  //!< [in] second point
double weightB      //!< [in] weight of second point
)
    {
    double f;
    DoubleOps::SafeDivide (f, weightB, weightA + weightB, 0.5);
    DPoint3d result;
    result.Interpolate (pointA, f, pointB);
    return result;
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      11/2016
+--------------------------------------------------------------------------------------*/
DPoint3d DPoint3d::FromWeightedAverage
(
DPoint3dCR pointA,  //!< [in] first point
double weightA,     //!< [in] weight of first point.
DPoint3dCR pointB,  //!< [in] second point
double weightB,     //!< [in] weight of second point.
DPoint3dCR pointC,  //!< [in] third point
double weightC      //!< [in] weight of third point
)
    {
    double fA, fB, fC;
    double w = weightA + weightB + weightC;

    if (   DoubleOps::SafeDivide (fA, weightA, w, 0.0)
        && DoubleOps::SafeDivide (fB, weightB, w, 0.0)
        && DoubleOps::SafeDivide (fC, weightC, w, 0.0)
        )
        {
        }
    else
        {
        fA = fB = fC = 1.0/3.0;
        }
    return DPoint3d::FromSumOf (pointA, fA, pointB, fB, pointC, fC);
    }



/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
DPoint3d DPoint3d::FromSumOf (DPoint3dCR pointA, DPoint3dCR vector)
    {
    DPoint3d result;
    result.x = pointA.x + vector.x;
    result.y = pointA.y + vector.y;
    result.z = pointA.z + vector.z;
    return result;
    }


/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
DPoint3d DPoint3d::FromSumOf (DPoint3dCR pointA, DPoint3dCR vector, double scaleFactor)
    {
    DPoint3d result;
    result.x = pointA.x + vector.x * scaleFactor;
    result.y = pointA.y + vector.y * scaleFactor;
    result.z = pointA.z + vector.z * scaleFactor;
    return result;
    }




/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
DPoint3d DPoint3d::FromSumOf (DPoint3dCR pointA, DPoint3dCR vector0, double scaleFactor0, DPoint3dCR vector1, double scaleFactor1)
    {
    DPoint3d result;
    result.x = pointA.x + vector0.x * scaleFactor0 + vector1.x * scaleFactor1;
    result.y = pointA.y + vector0.y * scaleFactor0 + vector1.y * scaleFactor1;
    result.z = pointA.z + vector0.z * scaleFactor0 + vector1.z * scaleFactor1;
    return result;
    }




/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
DPoint3d DPoint3d::FromSumOf (DPoint3dCR pointA, DPoint3dCR vector0, double scaleFactor0, DPoint3dCR vector1, double scaleFactor1, DPoint3dCR vector2, double scaleFactor2)
    {
    DPoint3d result;
    result.x = pointA.x + vector0.x * scaleFactor0 + vector1.x * scaleFactor1 + vector2.x * scaleFactor2;
    result.y = pointA.y + vector0.y * scaleFactor0 + vector1.y * scaleFactor1 + vector2.y * scaleFactor2;
    result.z = pointA.z + vector0.z * scaleFactor0 + vector1.z * scaleFactor1 + vector2.z * scaleFactor2;
    return result;
    }




/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
DPoint3d DPoint3d::FromScale (DPoint3d point, double scale)
    {
    DPoint3d result;
    result.x = point.x * scale;
    result.y = point.y * scale;
    result.z = point.z * scale;
    return result;
    }


/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
DPoint3d DPoint3d::FromSumOf (DPoint3dCR point0, double scaleFactor0, DPoint3dCR point1, double scaleFactor1)
    {
    DPoint3d result;
    result.x = point0.x * scaleFactor0 + point1.x * scaleFactor1;
    result.y = point0.y * scaleFactor0 + point1.y * scaleFactor1;
    result.z = point0.z * scaleFactor0 + point1.z * scaleFactor1;
    return result;
    }


/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
DPoint3d DPoint3d::FromSumOf (DPoint3dCR point0, double scaleFactor0, DPoint3dCR point1, double scaleFactor1, DPoint3dCR point2, double scaleFactor2)
    {
    DPoint3d result;
    result.x = point0.x * scaleFactor0 + point1.x * scaleFactor1 + point2.x * scaleFactor2;
    result.y = point0.y * scaleFactor0 + point1.y * scaleFactor1 + point2.y * scaleFactor2;
    result.z = point0.z * scaleFactor0 + point1.z * scaleFactor1 + point2.z * scaleFactor2;
    return result;
    }





/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
DPoint3d DPoint3d::FromInterpolateBilinear (DPoint3dCR data00, DPoint3dCR data10, DPoint3dCR data01, DPoint3dCR data11, double u, double v)
    {
    double a00 = (1.0 - u) * (1.0 - v);
    double a10 = u * (1.0 - v);
    double a01 = (1.0 - u) * v;
    double a11 = u * v;
    DPoint3d result;
    result.x = a00 * data00.x + a10 * data10.x + a01 * data01.x + a11 * data11.x;
    result.y = a00 * data00.y + a10 * data10.y + a01 * data01.y + a11 * data11.y;
    result.z = a00 * data00.z + a10 * data10.z + a01 * data01.z + a11 * data11.z;
    return result;
    }



//! return product of transform times point
//! @param [in] transform affine transform.
//! @param [in] x x component
//! @param [in] y y component
//! @param [in] z z component
DPoint3d DPoint3d::FromProduct
(
TransformCR transform,
DPoint3dCR point
)
    {
    DPoint3d result;
    result.x = transform.form3d[0][3]
                + transform.form3d[0][0] * point.x
                + transform.form3d[0][1] * point.y
                + transform.form3d[0][2] * point.z;
    result.y = transform.form3d[1][3]
                + transform.form3d[1][0] * point.x
                + transform.form3d[1][1] * point.y
                + transform.form3d[1][2] * point.z;
    result.z = transform.form3d[2][3]
                + transform.form3d[2][0] * point.x
                + transform.form3d[2][1] * point.y
                + transform.form3d[2][2] * point.z;
    return result;
    }



/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
static bool almostEqual(double ax, double ay, double az, double bx, double by, double bz)
    {
    double dx = ax - bx;
    double dy = ay - by;
    double dz = az - bz;
    double aa = ax * ax + ay * ay + az * az
             + bx * bx + by * by + bz * bz;
    double e = DoubleOps::SmallCoordinateRelTol ();
    double ee = e * e;
    double bb = dx * dx + dy * dy + dz * dz;
    return bb < ee * (1.0 + aa);
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
static bool almostEqual(double ax, double ay, double bx, double by)
    {
    double dx = ax - bx;
    double dy = ay - by;
    double aa = ax * ax + ay * ay
             + bx * bx + by * by;
    double e = DoubleOps::SmallCoordinateRelTol ();
    double ee = e * e;
    double bb = dx * dx + dy * dy;
    return bb < ee * (1.0 + aa);
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
static bool almostEqual(double ax, double ay, double bx, double by, double abstol)
    {
    double dx = ax - bx;
    double dy = ay - by;
    return fabs (dx) <= abstol && fabs (dy) < abstol;
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
static bool almostEqual(double ax, double ay, double az, double bx, double by, double bz, double abstol)
    {
    double dx = ax - bx;
    double dy = ay - by;
    double dz = az - bz;
    return fabs (dx) <= abstol && fabs (dy) < abstol && fabs (dz) < abstol;
    }



/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
bool DPoint3d::AlmostEqual (DPoint3d const & dataB) const
    {return almostEqual (x, y, z, dataB.x, dataB.y, dataB.z);}

bool DPoint3d::AlmostEqualXY (DPoint3d const & dataB) const
    {return almostEqual (x, y, dataB.x, dataB.y);}

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
bool DPoint3d::AlmostEqual (DPoint3d const & dataB, double tolerance) const
    {
    return tolerance > 0.0 ? almostEqual (x, y, z, dataB.x, dataB.y, dataB.z, tolerance)
                            : almostEqual (x, y, z, dataB.x, dataB.y, dataB.z);
    }

bool DPoint3d::AlmostEqualXY (DPoint3d const & dataB, double tolerance) const
    {
    return tolerance > 0.0 ? almostEqual (x, y, dataB.x, dataB.y, tolerance)
                            : almostEqual (x, y, dataB.x, dataB.y);
    }


/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
bool DVec3d::AlmostEqual (DVec3d const & dataB) const
    {return almostEqual (x, y, z, dataB.x, dataB.y, dataB.z);}


/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
bool DPoint2d::AlmostEqual (DPoint2d const & dataB) const
    {return almostEqual (x, y, dataB.x, dataB.y);}


/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
bool DVec2d::AlmostEqual (DVec2d const & dataB) const
    {return almostEqual (x, y, dataB.x, dataB.y);}

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      01/2015
+--------------------------------------------------------------------------------------*/
bool DPoint3d::AlmostEqual (bvector<DPoint3d> const &left, bvector<DPoint3d> const &right, double tolerance)
    {
    size_t n = left.size ();
    if (n != right.size ())
        return false;

    if (tolerance <= 0.0)
        {
        for (size_t i = 0; i < n; i++)
            if (!left[i].AlmostEqual (right[i]))
                return false;
        }
    else
        {
        for (size_t i = 0; i < n; i++)
            if (!left[i].AlmostEqual (right[i], tolerance))
                return false;
        }
    
    return true;
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      01/2015
+--------------------------------------------------------------------------------------*/
bool DPoint3d::AlmostEqualXY (bvector<DPoint3d> const &left, bvector<DPoint3d> const &right, double tolerance)
    {
    size_t n = left.size ();
    if (n != right.size ())
        return false;

    if (tolerance <= 0.0)
        {
        for (size_t i = 0; i < n; i++)
            if (!left[i].AlmostEqualXY (right[i]))
                return false;
        }
    else
        {
        for (size_t i = 0; i < n; i++)
            if (!left[i].AlmostEqualXY (right[i], tolerance))
                return false;
        }
    
    return true;
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      03/2017
+--------------------------------------------------------------------------------------*/
ValidatedDPoint3d DPoint3d::FromIntersectPerpendicularsXY
(
DPoint3dCR basePoint,   //!< [in] common point of rays
DPoint3dCR targetA,     //!< [in] target point of first ray.
double fractionA,       //!< [in] fractional position for perpendicular to first ray
DPoint3dCR targetB,     //!< [in] target point of second ray
double fractionB        //!< [in] fractional position for perpenedicular to second ray
)
    {
    DVec3d U = targetA - basePoint;
    DVec3d V = targetB - basePoint;
    double dx, dy;
    
    if (bsiSVD_solve2x2 (&dx, &dy,
            U.x, U.y,
            V.x, V.y,
            fractionA * U.DotProductXY (U), fractionB * V.DotProductXY (V)
            ))
        return ValidatedDPoint3d (DPoint3d::From (basePoint.x + dx, basePoint.y + dy, basePoint.z));
    return ValidatedDPoint3d (basePoint, false);
    }

END_BENTLEY_NAMESPACE
#include "refFPoint3d.h"
