/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
// refFPoint3d.h is #include'd into refDPoint3d.cpp (for template sharing) -- do NOT include PCH.
BEGIN_BENTLEY_NAMESPACE

/*--------------------------------------------------------------------------------**//**
* @bsimethod
+--------------------------------------------------------------------------------------*/
FPoint3d FPoint3d::From (double xx, double yy, double zz)
    {
    return from (xx, yy, zz);        // inlined -- collapses to float
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod
+--------------------------------------------------------------------------------------*/
FPoint3d FPoint3d::FromRoundLeft (DPoint3dCR xyz)
    {
    return from (
        DoubleOps::DoubleToFloatRoundLeft (xyz.x),
        DoubleOps::DoubleToFloatRoundLeft (xyz.y),
        DoubleOps::DoubleToFloatRoundLeft (xyz.z)
        );
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod
+--------------------------------------------------------------------------------------*/
FPoint3d FPoint3d::FromRoundRight (DPoint3dCR xyz)
    {
    return from (
        DoubleOps::DoubleToFloatRoundRight (xyz.x),
        DoubleOps::DoubleToFloatRoundRight (xyz.y),
        DoubleOps::DoubleToFloatRoundRight (xyz.z)
        );
    }


/*--------------------------------------------------------------------------------**//**
* @bsimethod
+--------------------------------------------------------------------------------------*/
FPoint3d FPoint3d::FromShift
(
FPoint3dCR xyz0,    //!< [in] reference point
double dx,      //!< [in] shift to apply to x direction
double dy,      //!< [in] shift to apply to y direction
double dz       //!< [in] shift to apply to z direction
)
    {
    return from (xyz0.x + dx, xyz0.y + dy, xyz0.z + dz);
    }
/*--------------------------------------------------------------------------------**//**
* @bsimethod
+--------------------------------------------------------------------------------------*/
FPoint3d FPoint3d::FromXY (FPoint3dCR xy, double zz) {return from (xy.x, xy.y, zz);}
FPoint3d FPoint3d::FromXY (DPoint3dCR xy, double zz) {return from (xy.x, xy.y, zz);}

/*--------------------------------------------------------------------------------**//**
* @bsimethod
+--------------------------------------------------------------------------------------*/
FPoint3d FPoint3d::From (DPoint2dCR xy, double zz) {return from (xy.x, xy.y, zz);}

FPoint3d FPoint3d::From (DPoint3dCR xyz) {return from (xyz.x, xyz.y, xyz.z);}

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
FPoint3d FPoint3d::FromZero () { return from (0.0, 0.0, 0.0);}

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
FPoint3d FPoint3d::FromOne (){return from (1.0, 1.0, 1.0);}


/*--------------------------------------------------------------------------------**//**
* @bsimethod
+--------------------------------------------------------------------------------------*/
void FPoint3d::Swap (FPoint3dR other)
    {
    FPoint3d temp = *this;
    *this = other;
    other = temp;
    }




/*-----------------------------------------------------------------*//**
* @description Returns the (scalar) cross product of the xy parts of two vectors.
*   The vectors are computed from the Origin to Target1 and Target2.
* @param [in] target1 The target point for the first vector.
* @param [in] target2 The target point for the second vector.
* @bsimethod
+----------------------------------------------------------------------*/
double FPoint3d::CrossProductToPointsXY
(
FPoint3dCR target1,
FPoint3dCR target2
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
* @bsimethod
+----------------------------------------------------------------------*/
double FPoint3d::DotProductToPoints
(

FPoint3dCR target1,
FPoint3dCR target2

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
* @bsimethod
+----------------------------------------------------------------------*/
double FPoint3d::DotProductToPointsXY
(

FPoint3dCR target1,
FPoint3dCR target2

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
* @bsimethod
+----------------------------------------------------------------------*/
double FPoint3d::DotDifference
(

FPoint3dCR origin,
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
* @bsimethod
+----------------------------------------------------------------------*/
double FPoint3d::TripleProductToPoints
(

FPoint3dCR target1,
FPoint3dCR target2,
FPoint3dCR target3

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
* @bsimethod
+----------------------------------------------------------------------*/
void FPoint3d::Zero () {x = y = z = 0.0;}


/*-----------------------------------------------------------------*//**
* @description Returns a point or vector with all components 1.0.
* @bsimethod
+----------------------------------------------------------------------*/
void FPoint3d::One () {x = y = z = 1.0;}



/*-----------------------------------------------------------------*//**
* @description Copy from a 2d point setting z to zero.
*
* @param [out] fPoint source point
* @bsimethod
+----------------------------------------------------------------------*/
void FPoint3d::Init (DPoint2dCR source, double zz)
    {
    init (source.x, source.y, zz);
    }


/*-----------------------------------------------------------------*//**
* @description Sets the x,y, and z components of a point
*
* @param [in] ax The x component.
* @param [in] ay The y component.
* @param [in] az The z component.
* @bsimethod
+----------------------------------------------------------------------*/
void FPoint3d::Init
(

double       ax,
double       ay,
double       az

)
    {
    init (ax, ay, az);
    }


/*-----------------------------------------------------------------*//**
* @description Sets the x,y, and z components of a FPoint3d structure from the
* corresponding parts of a DPoint4d.  Weight part of DPoint4d is not used.
*
* @param [in] hPoint The homogeneous point
* @bsimethod
+----------------------------------------------------------------------*/
void FPoint3d::XyzOf
(

DPoint4dCR hPoint

)
    {
    init (hPoint.x, hPoint.y, hPoint.z);
    }


/*-----------------------------------------------------------------*//**
* @description Set one of three components (x,y,z) of the point.
*
* @param [in] a The component value.
* @param [in] index Selects the the axis: 0=x, 1=y, 2=z, others cyclic.
* @bsimethod
+----------------------------------------------------------------------*/
void FPoint3d::SetComponent
(

double       a,
int         index

)
    {
    if (index > 2)
        index = index % 3;
    if (index == 0)
        {
        x = (float)a;
        }
    else if (index == 1)
        {
        y = (float)a;
        }
    else if (index == 2)
        {
        z = (float)a;
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
* @bsimethod
+----------------------------------------------------------------------*/
double FPoint3d::GetComponent
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
* @bsimethod
+----------------------------------------------------------------------*/
void FPoint3d::GetComponents
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
* @bsimethod
+----------------------------------------------------------------------*/
void FPoint3d::Interpolate
(

FPoint3dCR point0,
double fractionParameter,
FPoint3dCR point1

)
    {
    if (fractionParameter <= 0.5)
        {
        init (
            point0.x + fractionParameter * (point1.x - point0.x),
            point0.y + fractionParameter * (point1.y - point0.y),
            point0.z + fractionParameter * (point1.z - point0.z)
            );
        }
    else
        {
        double t = fractionParameter - 1.0;
        init (
            point1.x + t * (point1.x - point0.x),
            point1.y + t * (point1.y - point0.y),
            point1.z + t * (point1.z - point0.z)
            );
        }
    }


/*-----------------------------------------------------------------*//**
* @description Form vectors from the origin to the test point and the two boundary vectors.
* Test if the test vector is within the smaller angle between the other two vectors.
* @param [in] pTestPoint The point to test.
* @param [in] target1 The first target point.
* @param [in] target2 The second target point.
* @return true if the test point is within the angle.
* @bsimethod
+----------------------------------------------------------------------*/
bool FPoint3d::IsPointInSmallerSector
(

FPoint3dCR origin,
FPoint3dCR target1,
FPoint3dCR target2

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
* @bsimethod
+----------------------------------------------------------------------*/
bool FPoint3d::IsPointInCCWector
(

FPoint3dCR origin,
FPoint3dCR target0,
FPoint3dCR target1,
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
* @bsimethod
+----------------------------------------------------------------------*/
double FPoint3d::Distance
(

FPoint3dCR point2

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
* @bsimethod
+----------------------------------------------------------------------*/
double FPoint3d::DistanceSquared
(

FPoint3dCR point2

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
* @bsimethod
+----------------------------------------------------------------------*/
double FPoint3d::DistanceSquaredXY
(

FPoint3dCR point2

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
* @bsimethod
+----------------------------------------------------------------------*/
double FPoint3d::DistanceXY
(

FPoint3dCR point2

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
bool FPoint3d::DistanceXY
(
FPoint3dCR otherPoint,
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
    DPoint4d xyzwA = DPoint4d::FromMultiply (matrix, DPoint3d::From(*this));
    DPoint4d xyzwB = DPoint4d::FromMultiply (matrix, DPoint3d::From (otherPoint));
    if (xyzwA.GetProjectedXYZ (xyzA) && xyzwB.GetProjectedXYZ (xyzB))
        {
        distance = xyzA.DistanceXY (xyzB);
        return true;
        }
    distance = DBL_MAX;
    return false;
    }

/*-----------------------------------------------------------------*//**
* @bsimethod
+----------------------------------------------------------------------*/
double FPoint3d::MaxAbs () const
    {
    double maxVal = fabs (x);

    if (fabs (y) > maxVal)
        maxVal = fabs (y);

    if (fabs (z) > maxVal)
        maxVal = fabs (z);

    return maxVal;
    }

/*-----------------------------------------------------------------*//**
* @bsimethod
+----------------------------------------------------------------------*/
double FPoint3d::MaxDiff (FPoint3dCR other) const
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
* @bsimethod
+----------------------------------------------------------------------*/
double FPoint3d::MinAbs () const
    {
    double maxVal = fabs (x);

    if (fabs (y) < maxVal)
        maxVal = fabs (y);

    if (fabs (z) < maxVal)
        maxVal = fabs (z);

    return maxVal;
    }

/*-----------------------------------------------------------------*//**
* @bsimethod
+----------------------------------------------------------------------*/
int FPoint3d::MaxAbsIndex () const
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
* @bsimethod
+----------------------------------------------------------------------*/
int FPoint3d::MinAbsIndex () const
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
DRange1d FPoint3d::ComponentRange () const {return DRange1d::From (x,y,z);}


/*-----------------------------------------------------------------*//**
* @description Test for exact equality between all components of two points or vectors.
* @param [in] vector2 The second point or vector
* @return true if the points are identical.
* @bsimethod
+----------------------------------------------------------------------*/
bool FPoint3d::IsEqual
(

FPoint3dCR vector2

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
* @bsimethod
+----------------------------------------------------------------------*/
bool FPoint3d::IsEqual
(

FPoint3dCR vector2,
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
* @description Initialize a point by copying x,y,z from a vector.
* @param [out]pPoint  The point
* @param [in] vector  The vecotr
* @bsimethod
+----------------------------------------------------------------------*/
void FPoint3d::Init
(

DVec3dCR vector

)
    {
    init (vector.x, vector.y, vector.z);
    }



/*-----------------------------------------------------------------*//**
* @description Subtract a vector from a point.
*
* @param [in] base  The the first point or vector
* @param [in] vector  The second point or vector
* @bsimethod
+----------------------------------------------------------------------*/
void FPoint3d::Subtract
(

FPoint3dCR base,
DVec3dCR vector

)
    {
    init (base.x - vector.x, base.y - vector.y, base.z - vector.z);
    }


/*-----------------------------------------------------------------*//**
* @description Adds a vector to a pointer or vector, returns the result in place.
*
* @param [in] vector  The vector to add.
* @bsimethod
+----------------------------------------------------------------------*/
void FPoint3d::Add
(

DVec3dCR vector

)
    {
    init(x + vector.x, y + vector.y, z + vector.z);
    }


/*-----------------------------------------------------------------*//**
* @description Adds an origin and a scaled vector.
*
* @param [in] origin  Origin for the sum.
* @param [in] vector  The vector to be added.
* @param [in] scale  The scale factor.
* @bsimethod
+----------------------------------------------------------------------*/
void FPoint3d::SumOf
(

FPoint3dCR origin,
DVec3dCR vector,
double   scale

)
    {
    init (origin.x + vector.x * scale, origin.y + vector.y * scale, origin.z + vector.z * scale);
    }


/*-----------------------------------------------------------------*//**
* @description Adds an origin and two scaled vectors.
*
* @param [in] origin  The origin.  May be null.
* @param [in] vector1  The first direction vector
* @param [in] scale1  The first scale factor
* @param [in] vector2  The second direction vector
* @param [in] scale2  The second scale factor
* @bsimethod
+----------------------------------------------------------------------*/
void FPoint3d::SumOf
(

FPoint3dCR origin,
DVec3dCR vector1,
        double           scale1,
DVec3dCR vector2,
        double           scale2

)
    {
    init (
        origin.x + vector1.x * scale1 + vector2.x * scale2,
        origin.y + vector1.y * scale1 + vector2.y * scale2,
        origin.z + vector1.z * scale1 + vector2.z * scale2
        );
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
* @bsimethod
+----------------------------------------------------------------------*/
void FPoint3d::SumOf
(

FPoint3dCR origin,
DVec3dCR vector1,
        double          scale1,
DVec3dCR vector2,
        double          scale2,
DVec3dCR vector3,
        double          scale3

)
    {
    init (
        origin.x + vector1.x * scale1 + vector2.x * scale2 + vector3.x * scale3,
        origin.y + vector1.y * scale1 + vector2.y * scale2 + vector3.y * scale3,
        origin.z + vector1.z * scale1 + vector2.z * scale2 + vector3.z * scale3
        );
    }

#ifdef SKIP_IN_FPOINT3D

/*-----------------------------------------------------------------*//**
* @vbdescription Returns the cross (vector) cross product of two vectors.
* @param [in] vector1 The first vector
* @param [in] vector2 The second vector
* @bsimethod
+----------------------------------------------------------------------*/
void FPoint3d::CrossProduct
(

FPoint3dCR vector1,
FPoint3dCR vector2

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
* @bsimethod
+----------------------------------------------------------------------*/
void FPoint3d::CrossProductToPoints
(

FPoint3dCR origin,
FPoint3dCR target1,
FPoint3dCR target2

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
* @bsimethod
+----------------------------------------------------------------------*/
double FPoint3d::CrossProductXY
(

FPoint3dCR vector2

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
* @bsimethod
+----------------------------------------------------------------------*/
double FPoint3d::NormalizedCrossProduct
(

FPoint3dCR vector1,
FPoint3dCR vector2

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
* @bsimethod
+----------------------------------------------------------------------*/
double FPoint3d::SizedCrossProduct
(

FPoint3dCR vector1,
FPoint3dCR vector2,
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
* @bsimethod
+----------------------------------------------------------------------*/
double FPoint3d::GeometricMeanCrossProduct
(

FPoint3dCR vector1,
FPoint3dCR vector2

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
* @bsimethod
+----------------------------------------------------------------------*/
double FPoint3d::DotProduct
(

FPoint3dCR vector2

) const
    {
    return (x * vector2.x + y * vector2.y + z * vector2.z);
    }


/*-----------------------------------------------------------------*//**
* @description Returns the (scalar) dot product of xy parts of two vectors.
* @param [in] vector2 The second vector
* @return The dot product of the xy parts of the two vectors
* @bsimethod
+----------------------------------------------------------------------*/
double FPoint3d::DotProductXY
(

FPoint3dCR vector2

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
* @bsimethod
+----------------------------------------------------------------------*/
double FPoint3d::DotProduct
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
* @bsimethod
+----------------------------------------------------------------------*/
double FPoint3d::NormalizedDifference
(

FPoint3dCR target,
FPoint3dCR origin

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
* @bsimethod
+----------------------------------------------------------------------*/
double FPoint3d::AngleTo
(

FPoint3dCR vector2

) const
    {
    FPoint3d   crossProd;
    double cross, dot;
    crossProd.CrossProduct (*this, vector2);
    cross   = crossProd.Magnitude ();
    dot     = this->DotProduct (vector2);
    return  Angle::Atan2 (cross, dot);
    }
/*-----------------------------------------------------------------*//**
* @bsimethod
+----------------------------------------------------------------------*/
double FPoint3d::AngleXY () const
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
* @bsimethod
+----------------------------------------------------------------------*/
double FPoint3d::SmallerUnorientedAngleTo
(

FPoint3dCR vector2

) const
    {
    FPoint3d   crossProd;
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
* @bsimethod
+----------------------------------------------------------------------*/
bool FPoint3d::IsVectorInSmallerSector
(

FPoint3dCR vector0,
FPoint3dCR vector1

) const
    {
    FPoint3d   cross01;
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
* @bsimethod
+----------------------------------------------------------------------*/
bool FPoint3d::IsVectorInCCWSector
(

FPoint3dCR vector0,
FPoint3dCR vector1,
FPoint3dCR upVector

) const
    {
    FPoint3d    cross01;
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
* @bsimethod
+----------------------------------------------------------------------*/
double FPoint3d::AngleToXY
(

FPoint3dCR vector2

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
* @bsimethod
+----------------------------------------------------------------------*/
double FPoint3d::SmallerUnorientedAngleToXY
(

FPoint3dCR vector2

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
* @bsimethod
+----------------------------------------------------------------------*/
void FPoint3d::RotateXY
(

FPoint3dCR vector,
        double      theta

)
    {
    double c, s, xx, yy;
    s = sin (theta);
    c = cos (theta);

    xx = vector.x;
    yy = vector.y;

    init (xx * c - yy * s, xx * s + yy * c, vector.z);
    }


/*-----------------------------------------------------------------*//**
* @description Rotate a vector around the z axis.
* @param [in] theta The rotation angle.
* @bsimethod
+----------------------------------------------------------------------*/
void FPoint3d::RotateXY
(

        double      theta

)
    {
    double c, s, xx, yy;
    s = sin (theta);
    c = cos (theta);

    xx = x;
    yy = y;

    init (xx * c - yy * s, xx * s + yy * c, z);
    }
#endif

#ifdef SKIP_IN_FPOINT3D
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
* @bsimethod
+----------------------------------------------------------------------*/
double FPoint3d::SignedAngleTo
(

FPoint3dCR vector2,
FPoint3dCR orientationVector

) const
    {
    FPoint3d   crossProd;
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
* @bsimethod
+----------------------------------------------------------------------*/
double FPoint3d::PlanarAngleTo
(

FPoint3dCR vector2,
FPoint3dCR planeNormal

) const
    {
    FPoint3d  projection1, projection2;
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
* @bsimethod
+----------------------------------------------------------------------*/
void FPoint3d::WeightedDifferenceOf
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
* @bsimethod
+----------------------------------------------------------------------*/
void FPoint3d::WeightedDifferenceCrossProduct
(

DPoint4dCR basePoint,
DPoint4dCR target1,
DPoint4dCR target2

)
    {
    FPoint3d U, V;
    U.WeightedDifferenceOf (target1, basePoint);
    V.WeightedDifferenceOf (target2, basePoint);
    this->CrossProduct (U, V);
    }

#endif

/*-----------------------------------------------------------------*//**
* @description Computes the magnitude of a vector.
* @return The length of the vector
* @bsimethod
+----------------------------------------------------------------------*/
double FPoint3d::Magnitude
(

) const
    {
    return  sqrt (x * x + y * y + z * z);
    }


/*-----------------------------------------------------------------*//**
* @description Computes the squared magnitude of a vector.
*
* @return The squared magnitude of the vector.
* @bsimethod
+----------------------------------------------------------------------*/
double FPoint3d::MagnitudeSquared
(

) const
    {
    return   x * x + y * y + z * z;
    }


/*-----------------------------------------------------------------*//**
* @description Computes the magnitude of the xy part of a vector.
* @return The magnitude of the xy parts of the given vector.
* @bsimethod
+----------------------------------------------------------------------*/
double FPoint3d::MagnitudeXY
(

) const
    {
    return sqrt (x * x + y * y);
    }


/*-----------------------------------------------------------------*//**
* @description Computes the squared magnitude of the xy part of a vector.
* @return The squared magnitude of the xy parts of the given vector.
* @bsimethod
+----------------------------------------------------------------------*/
double FPoint3d::MagnitudeSquaredXY
(

) const
    {
    return x * x + y * y;
    }
#ifdef SKIP_IN_FPOINT3D
/*-----------------------------------------------------------------*//**
* @description Compute a unit vector perpendicular to the xy parts of given vector.
* @param [in] vector The source vector
* @return true if the input vector has nonzero length
* @bsimethod
+----------------------------------------------------------------------*/
bool FPoint3d::UnitPerpendicularXY
(

FPoint3dCR vector

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
* @description Multiplies a vector by a scale factor.
* @param [in] vector The vector to be scaled.
* @param [in] scale The scale factor.
* @bsimethod
+----------------------------------------------------------------------*/
void FPoint3d::Scale
(

FPoint3dCR vector,
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
* @bsimethod
+----------------------------------------------------------------------*/
void FPoint3d::Scale
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
* @bsimethod
+----------------------------------------------------------------------*/
void FPoint3d::Negate
(

FPoint3dCR vector

)
    {
    x = - vector.x;
    y = - vector.y;
    z = - vector.z;
    }


/*-----------------------------------------------------------------*//**
* @description Negate a vector in place.
*
* @bsimethod
+----------------------------------------------------------------------*/
void FPoint3d::Negate
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
* @bsimethod
+----------------------------------------------------------------------*/
double FPoint3d::Normalize
(

FPoint3dCR vector

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
* @bsimethod
+----------------------------------------------------------------------*/
double FPoint3d::ScaleToLength
(

FPoint3dCR vector,
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
* @bsimethod
+----------------------------------------------------------------------*/
double FPoint3d::ScaleToLength
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
* @bsimethod
+----------------------------------------------------------------------*/
double FPoint3d::Normalize
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
* @bsimethod
+----------------------------------------------------------------------*/
bool FPoint3d::IsParallelTo
(

FPoint3dCR vector2

) const
    {
    FPoint3d    vecC;
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
* @bsimethod
+----------------------------------------------------------------------*/
bool FPoint3d::IsPerpendicularTo
(

FPoint3dCR vector2

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
* @bsimethod
+----------------------------------------------------------------------*/
bool FPoint3d::SafeDivide
(

FPoint3dCR vector,
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
* @bsimethod
+----------------------------------------------------------------------*/
double FPoint3d::TripleProduct
(

FPoint3dCR vector2,
FPoint3dCR vector3

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
* @bsimethod
+----------------------------------------------------------------------*/
void FPoint3d::Subtract
(

FPoint3dCR vector2

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
* @bsimethod
+----------------------------------------------------------------------*/
void FPoint3d::DifferenceOf
(

FPoint3dCR point1,
FPoint3dCR point2

)
    {
    x = point1.x - point2.x;
    y = point1.y - point2.y;
    z = point1.z - point2.z;
    }
#endif


/*--------------------------------------------------------------------------------**//**
* @bsimethod
+--------------------------------------------------------------------------------------*/
void FPoint3d::SumOf (FPoint3dCR point1, double a1, FPoint3dCR point2, double a2)
    {
    init (
        point1.x * a1 + point2.x * a2,
        point1.y * a1 + point2.y * a2,
        point1.z * a1 + point2.z * a2
        );
    }




/*--------------------------------------------------------------------------------**//**
* @bsimethod
+--------------------------------------------------------------------------------------*/
void FPoint3d::SumOf (FPoint3dCR point1, double a1, FPoint3dCR point2, double a2, FPoint3dCR point3, double a3)
    {
    init (
        point1.x * a1 + point2.x * a2 + point3.x * a3,
        point1.y * a1 + point2.y * a2 + point3.y * a3,
        point1.z * a1 + point2.z * a2 + point3.z * a3
        );
    }


#ifdef SKIP_IN_FPOINT3D
/*--------------------------------------------------------------------------------**//**
* @bsimethod
+--------------------------------------------------------------------------------------*/
void FPoint3d::SumOf (FPoint3dCR point1, FPoint3dCR point2)
    {
    x = point1.x + point2.x;
    y = point1.y + point2.y;
    z = point1.z + point2.z;
    }



/*-----------------------------------------------------------------*//**
* @description Multiply each point in an array by its corresponding scale factor.
* @param pDest <= destination array.
* @param pSource => source array.
* @param pScales => scale factors
* @param n => number of points.
* @bsimethod
+----------------------------------------------------------------------*/
void FPoint3d::MultiplyArrayByScales
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
* @bsimethod
+----------------------------------------------------------------------*/
void FPoint3d::DivideArrayByScales
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

#endif
//! return product of transform times point given as components
//! @param [in] transform affine transform.
//! @param [in] point point to transform.
FPoint3d FPoint3d::FromMultiply
(
TransformCR transform,
double x,
double y,
double z
)
    {
    return from
            (
            transform.form3d[0][3]
                + transform.form3d[0][0] * x
                + transform.form3d[0][1] * y
                + transform.form3d[0][2] * z,
            transform.form3d[1][3]
                + transform.form3d[1][0] * x
                + transform.form3d[1][1] * y
                + transform.form3d[1][2] * z,
            transform.form3d[2][3]
                + transform.form3d[2][0] * x
                + transform.form3d[2][1] * y
                + transform.form3d[2][2] * z
            );
    }




/*--------------------------------------------------------------------------------**//**
* @bsimethod
+--------------------------------------------------------------------------------------*/
FPoint3d FPoint3d::FromMultiply
(
FPoint3dCR  origin,
RotMatrixCR matrix,
double x,
double y,
double z
)
    {
    return from (
            origin.x
                + matrix.form3d[0][0] * x
                + matrix.form3d[0][1] * y
                + matrix.form3d[0][2] * z,
            origin.y
                + matrix.form3d[1][0] * x
                + matrix.form3d[1][1] * y
                + matrix.form3d[1][2] * z,
            origin.z
                + matrix.form3d[2][0] * x
                + matrix.form3d[2][1] * y
                + matrix.form3d[2][2] * z
                );
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod
+--------------------------------------------------------------------------------------*/
FPoint3d FPoint3d::FromMultiply
(
FPoint3dCR  origin,
RotMatrixCR matrix,
DVec3dCR vector
)
    {
    return from (
            origin.x
                + matrix.form3d[0][0] * vector.x
                + matrix.form3d[0][1] * vector.y
                + matrix.form3d[0][2] * vector.z,
            origin.y
                + matrix.form3d[1][0] * vector.x
                + matrix.form3d[1][1] * vector.y
                + matrix.form3d[1][2] * vector.z,
            origin.z
                + matrix.form3d[2][0] * vector.x
                + matrix.form3d[2][1] * vector.y
                + matrix.form3d[2][2] * vector.z
            );
    }



//! interpolate between points.
//! @param [in] pointA start point
//! @param [in] fraction fractional parameter
//! @param [in] pointB end point
FPoint3d FPoint3d::FromInterpolate
(
FPoint3dCR pointA,
double     fraction,
FPoint3dCR pointB
)
    {
    FPoint3d result;
    result.Interpolate (pointA, fraction, pointB);
    return result;
    }
#ifdef SKIP_IN_FPOINT3D
/*--------------------------------------------------------------------------------**//**
* @bsimethod
+--------------------------------------------------------------------------------------*/
FPoint3d FPoint3d::FromInterpolateAndPerpendicularXY
(
FPoint3dCR pointA,
double     fraction,
FPoint3dCR pointB,
double fractionXYPerp
)
    {
    DVec3d vector = pointB - pointA;
    FPoint3d result;
    result.Interpolate (pointA, fraction, pointB);
    result.x -= fractionXYPerp * vector.y;
    result.y += fractionXYPerp * vector.x;
    return result;
    }
/*--------------------------------------------------------------------------------**//**
* @bsimethod
+--------------------------------------------------------------------------------------*/
FPoint3d FPoint3d::FromWeightedAverage
(
FPoint3dCR pointA,  //!< [in] first point
double weightA,     //!< [in] weight of first point.
FPoint3dCR pointB,  //!< [in] second point
double weightB      //!< [in] weight of second point
)
    {
    double f;
    DoubleOps::SafeDivide (f, weightB, weightA + weightB, 0.5);
    FPoint3d result;
    result.Interpolate (pointA, f, pointB);
    return result;
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod
+--------------------------------------------------------------------------------------*/
FPoint3d FPoint3d::FromWeightedAverage
(
FPoint3dCR pointA,  //!< [in] first point
double weightA,     //!< [in] weight of first point.
FPoint3dCR pointB,  //!< [in] second point
double weightB,     //!< [in] weight of second point.
FPoint3dCR pointC,  //!< [in] third point
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
    return FPoint3d::FromSumOf (pointA, fA, pointB, fB, pointC, fC);
    }

#endif

/*--------------------------------------------------------------------------------**//**
* @bsimethod
+--------------------------------------------------------------------------------------*/
FPoint3d FPoint3d::FromSumOf (FPoint3dCR pointA, DVec3dCR vector)
    {
    return from (
        pointA.x + vector.x,
        pointA.y + vector.y,
        pointA.z + vector.z
        );
    }


/*--------------------------------------------------------------------------------**//**
* @bsimethod
+--------------------------------------------------------------------------------------*/
FPoint3d FPoint3d::FromSumOf (FPoint3dCR pointA, DVec3dCR vector, double scaleFactor)
    {
    return from (
        pointA.x + vector.x * scaleFactor,
        pointA.y + vector.y * scaleFactor,
        pointA.z + vector.z * scaleFactor
        );
    }




/*--------------------------------------------------------------------------------**//**
* @bsimethod
+--------------------------------------------------------------------------------------*/
FPoint3d FPoint3d::FromSumOf (FPoint3dCR pointA, DVec3dCR vector0, double scaleFactor0, DVec3dCR vector1, double scaleFactor1)
    {
    return from (
        pointA.x + vector0.x * scaleFactor0 + vector1.x * scaleFactor1,
        pointA.y + vector0.y * scaleFactor0 + vector1.y * scaleFactor1,
        pointA.z + vector0.z * scaleFactor0 + vector1.z * scaleFactor1
        );
    }




/*--------------------------------------------------------------------------------**//**
* @bsimethod
+--------------------------------------------------------------------------------------*/
FPoint3d FPoint3d::FromSumOf (FPoint3dCR pointA, DVec3dCR  vector0, double scaleFactor0, DVec3dCR vector1, double scaleFactor1, DVec3dCR vector2, double scaleFactor2)
    {
    return from (
        pointA.x + vector0.x * scaleFactor0 + vector1.x * scaleFactor1 + vector2.x * scaleFactor2,
        pointA.y + vector0.y * scaleFactor0 + vector1.y * scaleFactor1 + vector2.y * scaleFactor2,
        pointA.z + vector0.z * scaleFactor0 + vector1.z * scaleFactor1 + vector2.z * scaleFactor2
        );
    }



#ifdef SKIP_IN_DPOINT3D

/*--------------------------------------------------------------------------------**//**
* @bsimethod
+--------------------------------------------------------------------------------------*/
FPoint3d FPoint3d::FromScale (FPoint3d point, double scale)
    {
    FPoint3d result;
    result.x = point.x * scale;
    result.y = point.y * scale;
    result.z = point.z * scale;
    return result;
    }
#endif


/*--------------------------------------------------------------------------------**//**
* @bsimethod
+--------------------------------------------------------------------------------------*/
FPoint3d FPoint3d::FromSumOf (FPoint3dCR point0, double scaleFactor0, FPoint3dCR point1, double scaleFactor1)
    {
    return from (
        point0.x * scaleFactor0 + point1.x * scaleFactor1,
        point0.y * scaleFactor0 + point1.y * scaleFactor1,
        point0.z * scaleFactor0 + point1.z * scaleFactor1
        );
    }


/*--------------------------------------------------------------------------------**//**
* @bsimethod
+--------------------------------------------------------------------------------------*/
FPoint3d FPoint3d::FromSumOf (FPoint3dCR point0, double scaleFactor0, FPoint3dCR point1, double scaleFactor1, FPoint3dCR point2, double scaleFactor2)
    {
    return from (
        point0.x * scaleFactor0 + point1.x * scaleFactor1 + point2.x * scaleFactor2,
        point0.y * scaleFactor0 + point1.y * scaleFactor1 + point2.y * scaleFactor2,
        point0.z * scaleFactor0 + point1.z * scaleFactor1 + point2.z * scaleFactor2
        );
    }





/*--------------------------------------------------------------------------------**//**
* @bsimethod
+--------------------------------------------------------------------------------------*/
FPoint3d FPoint3d::FromInterpolateBilinear (FPoint3dCR data00, FPoint3dCR data10, FPoint3dCR data01, FPoint3dCR data11, double u, double v)
    {
    double a00 = (1.0 - u) * (1.0 - v);
    double a10 = u * (1.0 - v);
    double a01 = (1.0 - u) * v;
    double a11 = u * v;
    return from (
        a00 * data00.x + a10 * data10.x + a01 * data01.x + a11 * data11.x,
        a00 * data00.y + a10 * data10.y + a01 * data01.y + a11 * data11.y,
        a00 * data00.z + a10 * data10.z + a01 * data01.z + a11 * data11.z
        );
    }



//! return product of transform times point
//! @param [in] transform affine transform.
//! @param [in] x x component
//! @param [in] y y component
//! @param [in] z z component
FPoint3d FPoint3d::FromMultiply
(
TransformCR transform,
FPoint3dCR point
)
    {
    return from (
        transform.form3d[0][3]
                + transform.form3d[0][0] * point.x
                + transform.form3d[0][1] * point.y
                + transform.form3d[0][2] * point.z,
        transform.form3d[1][3]
                + transform.form3d[1][0] * point.x
                + transform.form3d[1][1] * point.y
                + transform.form3d[1][2] * point.z,
        transform.form3d[2][3]
                + transform.form3d[2][0] * point.x
                + transform.form3d[2][1] * point.y
                + transform.form3d[2][2] * point.z
                );
    }

//! return product of transform times point
//! @param [in] transform affine transform.
//! @param [in] x x component
//! @param [in] y y component
//! @param [in] z z component
FPoint3d FPoint3d::FromMultiply
(
TransformCR transform,
DPoint3dCR point
)
    {
    return from (transform * point);
    }


/*--------------------------------------------------------------------------------**//**
* @bsimethod
+--------------------------------------------------------------------------------------*/
bool FPoint3d::AlmostEqual (FPoint3d const & dataB) const
    {return almostEqual (x, y, z, dataB.x, dataB.y, dataB.z);}

bool FPoint3d::AlmostEqualXY (FPoint3d const & dataB) const
    {return almostEqual (x, y, dataB.x, dataB.y);}

/*--------------------------------------------------------------------------------**//**
* @bsimethod
+--------------------------------------------------------------------------------------*/
bool FPoint3d::AlmostEqual (FPoint3d const & dataB, double tolerance) const
    {
    return tolerance > 0.0 ? almostEqual (x, y, z, dataB.x, dataB.y, dataB.z, tolerance)
                            : almostEqual (x, y, z, dataB.x, dataB.y, dataB.z);
    }

bool FPoint3d::AlmostEqualXY (FPoint3d const & dataB, double tolerance) const
    {
    return tolerance > 0.0 ? almostEqual (x, y, dataB.x, dataB.y, tolerance)
                            : almostEqual (x, y, dataB.x, dataB.y);
    }



/*--------------------------------------------------------------------------------**//**
* @bsimethod
+--------------------------------------------------------------------------------------*/
bool FPoint3d::AlmostEqual (bvector<FPoint3d> const &left, bvector<FPoint3d> const &right, double tolerance)
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
* @bsimethod
+--------------------------------------------------------------------------------------*/
bool FPoint3d::AlmostEqualXY (bvector<FPoint3d> const &left, bvector<FPoint3d> const &right, double tolerance)
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
#ifdef SKIP_IN_FPOINT3D
/*--------------------------------------------------------------------------------**//**
* @bsimethod
+--------------------------------------------------------------------------------------*/
ValidatedDPoint3d FPoint3d::FromIntersectPerpendicularsXY
(
FPoint3dCR basePoint,   //!< [in] common point of rays
FPoint3dCR targetA,     //!< [in] target point of first ray.
double fractionA,       //!< [in] fractional position for perpendicular to first ray
FPoint3dCR targetB,     //!< [in] target point of second ray
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
        return ValidatedDPoint3d (from (basePoint.x + dx, basePoint.y + dy, basePoint.z));
    return ValidatedDPoint3d (basePoint, false);
    }
#endif
END_BENTLEY_NAMESPACE
