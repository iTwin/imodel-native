/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include <bsibasegeomPCH.h>
BEGIN_BENTLEY_NAMESPACE


/*-----------------------------------------------------------------*//**
* @description Returns a DPoint2d with 2 components (xy) from a double array
*
* @param [in] pXy x, y components
+----------------------------------------------------------------------*/
DPoint2d DPoint2d::FromArray
(
const   double  *pXy
)
    {
    DPoint2d point;
    point.InitFromArray (pXy);
    return point;
    }

/*-----------------------------------------------------------------*//**
* @description Returns a DPoint2d with 2 components (xy) from given components
* @param [in] ax x coordinate
* @param [in] ay y coordinate
+----------------------------------------------------------------------*/
DPoint2d DPoint2d::From
(
double ax,
double ay
)
    {
    DPoint2d point;
    point.x = ax;
    point.y = ay;
    return point;
    }

/*-----------------------------------------------------------------*//**
+----------------------------------------------------------------------*/
DPoint2d DPoint2d::From (DPoint3dCR source)
    {
    DPoint2d point;
    point.x = source.x;
    point.y = source.y;
    return point;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DPoint2d DPoint2d::From (FPoint2dCR source)
    {
    return From(source.x, source.y);
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
DPoint2d DPoint2d::FromZero ()
    {
    DPoint2d xy;
    xy.Zero();
    return xy;
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
DPoint2d DPoint2d::FromOne ()
    {
    DPoint2d xy;
    xy.One();
    return xy;
    }


/*-----------------------------------------------------------------*//**
* @return squared magnitude of the vector
* @indexVerb magnitude
* @bsimethod
+----------------------------------------------------------------------*/
double DPoint2d::MagnitudeSquared
(

) const
    {
    return this->x * this->x + this->y * this->y;
    }




/*--------------------------------------------------------------------------------**//**
* @bsimethod
+--------------------------------------------------------------------------------------*/
void DPoint2d::Swap (DPoint2dR other)
    {
    DPoint2d temp = *this;
    *this = other;
    other = temp;
    }



/*-----------------------------------------------------------------*//**
* @description Returns the (scalar) cross product of two vectors.
* @param [in] vector1 first vector
* @param [in] pVector2 second vector
* @indexVerb crossProduct
* @bsimethod
+----------------------------------------------------------------------*/
double DPoint2d::CrossProduct
(

DPoint2dCR vector1

) const
    {
    return this->x * vector1.y - this->y * vector1.x;
    }


/*-----------------------------------------------------------------*//**
* @description Returns the (scalar) cross product of two vectors.
*   The vectors are computed from the Origin to Target1 and Target2.
* @param [in] pOrigin base point
* @param [in] target1 target of first vector
* @param [in] target2 target of second vector
* @bsimethod
+----------------------------------------------------------------------*/
double DPoint2d::CrossProductToPoints
(

DPoint2dCR target1,
DPoint2dCR target2

) const
    {
    DPoint2d tmpVector1, tmpVector2;

    tmpVector1.DifferenceOf (target1, *this);
    tmpVector2.DifferenceOf (target2, *this);
    return  tmpVector1.x * tmpVector2.y - tmpVector1.y * tmpVector2.x;
    }


/*-----------------------------------------------------------------*//**
* @description Returns the (scalar) dot product of two vectors.
* @param [in] vector2 second vector.
* @return dot product of the two vectors
* @indexVerb dotProduct
* @bsimethod
+----------------------------------------------------------------------*/
double DPoint2d::DotProduct
(

DPoint2dCR vector2

) const
    {
    return this->x * vector2.x + this->y * vector2.y;
    }


/*-----------------------------------------------------------------*//**
* @description Returns the (scalar) dot product of two vectors.
*   The vectors are computed from the Origin to Target1 and Target2.
* @param [in] pPoint1 target of first vector
* @param [in] target2 target of second vector
* @indexVerb dotProduct
* @bsimethod
+----------------------------------------------------------------------*/
double DPoint2d::DotProductToPoints
(

DPoint2dCR target1,
DPoint2dCR target2

) const
    {
    DPoint2d tmpVector1, tmpVector2;

    tmpVector1.DifferenceOf (target1, *this);
    tmpVector2.DifferenceOf (target2, *this);
    return  tmpVector1.x * tmpVector2.x + tmpVector1.y * tmpVector2.y;
    }


/*-----------------------------------------------------------------*//**
* @description Return the squared distance between two points or vectors.
* @param [in] point2 end point
* @return squared distance between points
* @indexVerb distance squared
* @bsimethod
+----------------------------------------------------------------------*/
double DPoint2d::DistanceSquared
(

DPoint2dCR point2

) const
    {
    double dx = point2.x - this->x;
    double dy = point2.y - this->y;
    return dx * dx + dy * dy;
    }


/*-----------------------------------------------------------------*//**
* @description Adds an origin and a scaled vector.
* @param [in] pOrigin origin .  May be null.
* @param [in] vector direction vector
* @param [in] scale scale factor
* @indexVerb add
* @bsimethod
+----------------------------------------------------------------------*/
void DPoint2d::SumOf
(

DPoint2dCR point,
DPoint2dCR vector,
      double    s

)
    {
    this->x = point.x + vector.x * s;
    this->y = point.y + vector.y * s;
    }


/*-----------------------------------------------------------------*//**
* Compute the point at an interpolated (fractional) position
*   between a start and end point.

* @param [in] point0 start point (at parameter s=0)
* @param const double    s => interpolation parameter
* @param [in] point1 end point (at parameter s=1)
* @indexVerb interpolate
* @bsimethod
+----------------------------------------------------------------------*/
void DPoint2d::Interpolate
(
DPoint2dCR point0,
      double    s,
DPoint2dCR point1
)
    {
    if (s <= 0.5)
        {
        x = point0.x + s * (point1.x - point0.x);
        y = point0.y + s * (point1.y - point0.y);
        }
    else
        {
        double t = s - 1.0;
        x = point1.x + t * (point1.x - point0.x);
        y = point1.y + t * (point1.y - point0.y);
        }
    }


/*-----------------------------------------------------------------*//**
* @description Compute the sum of two points or vectors.
* @param [in] point1 First point or vector
* @param [in] point2 Second point or vector
* @indexVerb add
* @bsimethod
+----------------------------------------------------------------------*/
void DPoint2d::SumOf
(

DPoint2dCR point1,
DPoint2dCR point2

)
    {
    x = point1.x + point2.x;
    y = point1.y + point2.y;
    }




/*--------------------------------------------------------------------------------**//**
* @bsimethod
+--------------------------------------------------------------------------------------*/
DPoint2d DPoint2d::FromSumOf (DPoint2dCR pointA, DVec2dCR vector, double scaleFactor)
    {
    DPoint2d result;
    result.x = pointA.x + vector.x * scaleFactor;
    result.y = pointA.y + vector.y * scaleFactor;
    return result;
    }




/*--------------------------------------------------------------------------------**//**
* @bsimethod
+--------------------------------------------------------------------------------------*/
DPoint2d DPoint2d::FromSumOf (DPoint2dCR pointA, DVec2dCR vector0, double scaleFactor0, DVec2dCR vector1, double scaleFactor1)
    {
    DPoint2d result;
    result.x = pointA.x + vector0.x * scaleFactor0 + vector1.x * scaleFactor1;
    result.y = pointA.y + vector0.y * scaleFactor0 + vector1.y * scaleFactor1;
    return result;
    }




/*--------------------------------------------------------------------------------**//**
* @bsimethod
+--------------------------------------------------------------------------------------*/
DPoint2d DPoint2d::FromSumOf (DPoint2dCR pointA, DVec2dCR vector0, double scaleFactor0, DVec2dCR vector1, double scaleFactor1, DVec2dCR vector2, double scaleFactor2)
    {
    DPoint2d result;
    result.x = pointA.x + vector0.x * scaleFactor0 + vector1.x * scaleFactor1 + vector2.x * scaleFactor2;
    result.y = pointA.y + vector0.y * scaleFactor0 + vector1.y * scaleFactor1 + vector2.y * scaleFactor2;
    return result;
    }





/*--------------------------------------------------------------------------------**//**
* @bsimethod
+--------------------------------------------------------------------------------------*/
DPoint2d DPoint2d::FromScale (DPoint2d point, double scale)
    {
    DPoint2d result;
    result.x = point.x * scale;
    result.y = point.y * scale;
    return result;
    }


/*--------------------------------------------------------------------------------**//**
* @bsimethod
+--------------------------------------------------------------------------------------*/
DPoint2d DPoint2d::FromSumOf (DPoint2dCR point0, double scaleFactor0, DPoint2dCR point1, double scaleFactor1)
    {
    DPoint2d result;
    result.x = point0.x * scaleFactor0 + point1.x * scaleFactor1;
    result.y = point0.y * scaleFactor0 + point1.y * scaleFactor1;
    return result;
    }


/*--------------------------------------------------------------------------------**//**
* @bsimethod
+--------------------------------------------------------------------------------------*/
DPoint2d DPoint2d::FromSumOf (DPoint2dCR point0, double scaleFactor0, DPoint2dCR point1, double scaleFactor1, DPoint2dCR point2, double scaleFactor2)
    {
    DPoint2d result;
    result.x = point0.x * scaleFactor0 + point1.x * scaleFactor1 + point2.x * scaleFactor2;
    result.y = point0.y * scaleFactor0 + point1.y * scaleFactor1 + point2.y * scaleFactor2;
    return result;
    }


/*--------------------------------------------------------------------------------**//**
* @bsimethod
+--------------------------------------------------------------------------------------*/
DPoint2d DPoint2d::FromInterpolate (DPoint2dCR pointA, double fraction, DPoint2dCR pointB)
    {
    DPoint2d result;
    result.Interpolate (pointA, fraction, pointB);
    return result;
    }

DPoint2d DPoint2d::FromForwardLeftInterpolate (DPoint2dCR point0, double tangentFraction, double leftFraction, DPoint2dCR point1)
    {
    double dx = point1.x - point0.x;
    double dy = point1.y - point0.y;
    return DPoint2d::From
        (
        point0.x + tangentFraction * dx - leftFraction * dy,
        point0.y + tangentFraction * dy + leftFraction * dx
        );
    }

DPoint2d DPoint2d::AddForwardLeft (DVec2dCR vector, double tangentFraction, double leftFraction) const
    {
    double dx = vector.x;
    double dy = vector.y;
    return DPoint2d::From
        (
            x + tangentFraction * dx - leftFraction * dy,
            y + tangentFraction * dy + leftFraction * dx
            );
    }


/*--------------------------------------------------------------------------------**//**
* @bsimethod
+--------------------------------------------------------------------------------------*/
DPoint2d DPoint2d::FromInterpolateBilinear (DPoint2dCR data00, DPoint2dCR data10, DPoint2dCR data01, DPoint2dCR data11, double u, double v)
    {
    double a00 = (1.0 - u) * (1.0 - v);
    double a10 = u * (1.0 - v);
    double a01 = (1.0 - u) * v;
    double a11 = u * v;
    DPoint2d result;
    result.x = a00 * data00.x + a10 * data10.x + a01 * data01.x + a11 * data11.x;
    result.y = a00 * data00.y + a10 * data10.y + a01 * data01.y + a11 * data11.y;
    return result;
    }


/*-----------------------------------------------------------------*//**
* Add a vector to the instance.
*
* @param [in] vector vector to add
* @indexVerb add
* @bsimethod
+----------------------------------------------------------------------*/
void DPoint2d::Add
(

DPoint2dCR vector

)
    {
    x += vector.x;
    y += vector.y;
    }


/*-----------------------------------------------------------------*//**
* @description Subtract one vector from another in place.
*
* @param [in] pPoint vector to subtract
* @indexVerb subtract
* @bsimethod
+----------------------------------------------------------------------*/
void DPoint2d::Subtract
(

DPoint2dCR vector

)
    {
    x -= vector.x;
    y -= vector.y;
    }


/*-----------------------------------------------------------------*//**
* @description Return the difference of two points or vectors.
* @param [in] point1 First point or vector.
* @param [in] point2 Second (subtracted) point or vector.
* @indexVerb subtract
* @bsimethod
+----------------------------------------------------------------------*/
void DPoint2d::DifferenceOf
(

DPoint2dCR point1,
DPoint2dCR point2

)
    {
    x = point1.x - point2.x;
    y = point1.y - point2.y;
    }



/*--------------------------------------------------------------------------------**//**
* @bsimethod
+--------------------------------------------------------------------------------------*/
void DPoint2d::Scale (DPoint2dCR point, double s)
    {
    x = point.x * s;
    y = point.y * s;
    }




/*--------------------------------------------------------------------------------**//**
* @bsimethod
+--------------------------------------------------------------------------------------*/
void DPoint2d::Scale (double s)
    {
    x *= s;
    y *= s;
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
double DPoint2d::ScaleToLength
(
DPoint2dCR vector,
double    length
)
    {
    double  magnitude =
            sqrt  (vector.x * vector.x
                 + vector.y * vector.y);

    if (magnitude > 0.0)
        {
        double f = length / magnitude;
        x = vector.x * f;
        y = vector.y * f;
        }
    else
        {
        x = length;
        y = 0.0;
        }

    return  magnitude;
    }




/*--------------------------------------------------------------------------------**//**
* @bsimethod
+--------------------------------------------------------------------------------------*/
double DPoint2d::ScaleToLength
(
double length
)
    {
    return this->ScaleToLength (*this, length);
    }




/*-----------------------------------------------------------------*//**
*
* normalizes pVector1 in place, and returns the original magnitude.  If the
* original magnitude is 0 the vector is left unchanged.
*
* @return original length
* @indexVerb normalize
* @bsimethod
+----------------------------------------------------------------------*/
double DPoint2d::Normalize
(

)
    {
    double magnitude = sqrt (x * x + y * y);

    if (magnitude > 0.0)
        {
        double f = 1.0 / magnitude;
        x *= f;
        y *= f;
        }

    return  magnitude;
    }


/*-----------------------------------------------------------------*//**
*
* normalizes vector1, stores as instance, and returns the original magnitude.  If the
* original magnitude is 0 the vector is copied.
*
* @return original length
* @indexVerb normalize
* @bsimethod
+----------------------------------------------------------------------*/
double DPoint2d::Normalize
(

DPoint2dCR vector1

)
    {
    double magnitude = sqrt (vector1.x * vector1.x + vector1.y * vector1.y);

    *this = vector1;
    if (magnitude > 0.0)
        {
        double f = 1.0 / magnitude;
        x *= f;
        y *= f;
        }

    return magnitude;
    }


/*-----------------------------------------------------------------*//**
* @param [in] vec original vector
* @indexVerb rotate
* @bsimethod
+----------------------------------------------------------------------*/
void DPoint2d::Rotate90
(

DPoint2dCR vec

)
    {
    double xx = -vec.y;        /* Local copies to allow in-place udpate */
    double yy =  vec.x;
    x = xx;
    y = yy;
    }


/*-----------------------------------------------------------------*//**
* @param [in] vec original vector
* @indexVerb rotate
* @bsimethod
+----------------------------------------------------------------------*/
void DPoint2d::RotateCCW
(

DPoint2dCR vec,
double  radians

)
    {
    double c = cos (radians);
    double s = sin (radians);

    double xx = vec.x;        /* Local copies to allow in-place udpate */
    double yy = vec.y;
    x = c * xx - s * yy;
    y = s * xx + c * yy;
    }


/*-----------------------------------------------------------------*//**
* @description Sets all components of a point or vector to zero.
* @indexVerb zero
* @indexVerb init
* @bsimethod
+----------------------------------------------------------------------*/
void DPoint2d::Zero
(

)
    {
    x = 0.0;
    y = 0.0;
    }


/*-----------------------------------------------------------------*//**
* @description Returns a point or vector with all components 1.0.
* @bsimethod
+----------------------------------------------------------------------*/
void DPoint2d::One
(

)
    {
    x = 1.0;
    y = 1.0;
    }


/*-----------------------------------------------------------------*//**
* copy 2 components (xy) from a double array to the DPoint2d
* @param [in] pXy x, y components
* @indexVerb init
* @bsimethod
+----------------------------------------------------------------------*/
void DPoint2d::InitFromArray
(

const   double      *pXy

)
    {
    x = pXy[0];
    y = pXy[1];
    }


/*-----------------------------------------------------------------*//**
* @param [in] source x, y components
* @indexVerb init
* @bsimethod
+----------------------------------------------------------------------*/
void DPoint2d::Init
(

DPoint3dCR source

)
    {
    x = source.x;
    y = source.y;
    }




/*--------------------------------------------------------------------------------**//**
* @bsimethod
+--------------------------------------------------------------------------------------*/
void DPoint2d::Init (double ax, double ay)
    {
    x = ax;
    y = ay;
    }



/*-----------------------------------------------------------------*//**
* @description Sets a single component of a point.  If the index is out of
* range 0,1, it is interpreted cyclically.
*
* @param [in] index 0=x, 1=y, others cyclic
* @indexVerb set
* @bsimethod
+----------------------------------------------------------------------*/
void DPoint2d::SetComponent
(

double       a,
int         index

)
    {
    index = index & 0x01;
    if (index == 0)
        {
        x = a;
        }
    else /* if (index == 1) */
        {
        y = a;
        }
    }


/*-----------------------------------------------------------------*//**
* @description Gets a single component of a point.  If the index is out of
* range 0,1, it is interpreted cyclically.
*
* @param [in] index 0=x, 1=y, others cyclic
* @return specified component of the point or vector.
* @indexVerb get
* @bsimethod
+----------------------------------------------------------------------*/
double DPoint2d::GetComponent
(

      int         index

) const
    {
    index = index & 0x01;
    if (index == 0)
        {
        return x;
        }
    else /* (index == 1) */
        {
        return y;
        }
    }


/*-----------------------------------------------------------------*//**
* @param [out] &xCoord x component
* @param [out] &yCoord y component
* @indexVerb get
* @bsimethod
+----------------------------------------------------------------------*/
void DPoint2d::GetComponents
(

double &xCoord,
double &yCoord

) const
    {
    xCoord = x;
    yCoord = y;
    }


/*-----------------------------------------------------------------*//**
* @description Adds an origin and two scaled vectors.
*
* @param [in] origin origin .  May be null.
* @param [in] vector1 direction vector
* @param [in] scale1 scale factor
* @param [in] vector2 direction vector
* @param [in] scale2 scale factor
* @indexVerb add
* @bsimethod
+----------------------------------------------------------------------*/
void DPoint2d::SumOf
(

DPoint2dCR origin,
DPoint2dCR vector1,
        double           scale1,
DPoint2dCR vector2,
        double           scale2

)
    {
    x = origin.x + vector1.x * scale1 + vector2.x * scale2;
    y = origin.y + vector1.y * scale1 + vector2.y * scale2;
    }


/*-----------------------------------------------------------------*//**
* @description Adds an origin and two scaled vectors.
*
* @param [in] origin origin .  May be null.
* @param [in] vector1 direction vector
* @param [in] scale1 scale factor
* @param [in] vector2 direction vector
* @param [in] scale2 scale factor
* @param [in] vector3 direction vector
* @param [in] scale3 scale factor
* @indexVerb add
* @bsimethod
+----------------------------------------------------------------------*/
void DPoint2d::SumOf
(

DPoint2dCR origin,
DPoint2dCR vector1,
        double          scale1,
DPoint2dCR vector2,
        double          scale2,
DPoint2dCR vector3,
        double          scale3

)
    {
    x = origin.x + vector1.x * scale1 + vector2.x * scale2 + vector3.x * scale3;
    y = origin.y + vector1.y * scale1 + vector2.y * scale2 + vector3.y * scale3;
    }




/*--------------------------------------------------------------------------------**//**
* @bsimethod
+--------------------------------------------------------------------------------------*/
void DPoint2d::SumOf
(
DPoint2dCR vector1,
double          scale1,
DPoint2dCR vector2,
double          scale2,
DPoint2dCR vector3,
double          scale3
)
    {
    x = vector1.x * scale1 + vector2.x * scale2 + vector3.x * scale3;
    y = vector1.y * scale1 + vector2.y * scale2 + vector3.y * scale3;
    }


/*--------------------------------------------------------------------------------**//**
* @bsimethod
+--------------------------------------------------------------------------------------*/
void DPoint2d::SumOf
(
DPoint2dCR vector1,
double          scale1,
DPoint2dCR vector2,
double          scale2
)
    {
    x = vector1.x * scale1 + vector2.x * scale2;
    y = vector1.y * scale1 + vector2.y * scale2;
    }




/*-----------------------------------------------------------------*//**
* Sets pNormal to the unit vector in the direction of point1  point2

* @param [in] point1 point 1
* @param [in] point2 point 2
* @return double
* @indexVerb normalize
* @indexVerb subtract
* @bsimethod
+----------------------------------------------------------------------*/
double DPoint2d::NormalizedDifferenceOf
(

DPoint2dCR point1,
DPoint2dCR point2

)
    {
    this->DifferenceOf (point1, point2);
    return this->Normalize ();
    }


/*-----------------------------------------------------------------*//**
* @description Returns (signed, counterclockwise) angle between two vectors.
*   The angle is in radians. The angle range is from -pi to +pi; positive
*   angles are counterclockwise, negative angles are clockwise.
* @param [in] vector2 second vector
* @return angle in radians
* @indexVerb angle
* @bsimethod
+----------------------------------------------------------------------*/
double DPoint2d::AngleTo
(

DPoint2dCR vector2

) const
    {
    double cross, dot;
    cross = this->CrossProduct (vector2);
    dot   = this->DotProduct (vector2);
    return  Angle::Atan2 (cross, dot);
    }


/*-----------------------------------------------------------------*//**
* @description Returns the distance between 2 points
* @param [in] point1 second point
* @return distance from point 0 to point 1
* @indexVerb distance
* @bsimethod
+----------------------------------------------------------------------*/
double DPoint2d::Distance
(

DPoint2dCR point1

) const
    {
    double xdist, ydist;
    xdist = (point1.x - x);
    ydist = (point1.y - y);

    return (sqrt (xdist * xdist + ydist * ydist));
    }


/*-----------------------------------------------------------------*//**
* @description Returns the magnitude (length) of a vector.
* @return Length of the vector.
* @indexVerb magnitude
* @bsimethod
+----------------------------------------------------------------------*/
double DPoint2d::Magnitude
(

) const
    {
    return (sqrt (x * x + y * y));
    }


/*-----------------------------------------------------------------*//**
* @description returns the negative of a vector.
* @param [in] vector input
* @indexVerb scale
* @bsimethod
+----------------------------------------------------------------------*/
void DPoint2d::Negate
(

DPoint2dCR vector

)
    {
    x = - vector.x;
    y = - vector.y;
    }

//!
//! @return true if the point has a nan in any component
//!
bool DPoint2d::IsNan() const
    {
    return isnan(x) || isnan(y);
    }

/*-----------------------------------------------------------------*//**
* @description Test if two vectors are parallel.
* @param [in] vector2 Second vector
* @return true if vectors are (nearly) parallel.
* @indexVerb angle
* @bsimethod
+----------------------------------------------------------------------*/
bool DPoint2d::IsParallelTo
(

DPoint2dCR vector2

) const
    {
    double a2 = this->DotProduct (*this);
    double b2 = vector2.DotProduct (vector2);
    double cross;
    double eps = Angle::SmallAngle (); /*  a small angle tolerance
                                            (in radians) */
    cross = this->CrossProduct (vector2);

    return  cross * cross <= eps * eps * a2 * b2;
    }


/*-----------------------------------------------------------------*//**
* @description Test if two vectors are perpendicular.
* @param [in] vector2 Second vector
* @return true if vectors are (nearly) parallel.
* @indexVerb angle
* @bsimethod
+----------------------------------------------------------------------*/
bool DPoint2d::IsPerpendicularTo
(

DPoint2dCR vector2

) const
    {
    double a2 = this->DotProduct (*this);
    double b2 = vector2.DotProduct (vector2);
    double dot;
    double eps = Angle::SmallAngle (); /*  a small angle tolerance
                                            (in radians) */
    dot = this->DotProduct (vector2);

    return  dot * dot <= eps * eps * a2 * b2;
    }


/*-----------------------------------------------------------------*//**
* @description Test if two points or vectors are exactly equal.
* @param [in] vector2 Second point or vector
* @return true if the points are identical.
* @see #isEqual
* @indexVerb equal
* @bsimethod
+----------------------------------------------------------------------*/
bool DPoint2d::IsEqual
(

DPoint2dCR vector2

) const
    {
    bool result;

    result = x == vector2.x && y == vector2.y;

    return  result;
    }


/*-----------------------------------------------------------------*//**
* @description Test if the x and y components of two points or vectors are
*   equal within tolerance.
* Tests are done independently using the absolute value of each component differences
* (i.e. not the magnitude or sum of squared differences)
*
* @param [in] vector2 second point or vector
* @param [in] tolerance tolerance
* @return true if all components are within given tolerance of each other.
* @indexVerb equal
* @bsimethod
+----------------------------------------------------------------------*/
bool DPoint2d::IsEqual
(

DPoint2dCR vector2,
double                  tolerance

) const
    {
    bool result;

    result = fabs (x - vector2.x) <= tolerance &&
             fabs (y - vector2.y) <= tolerance;
    
    return result;
    }


/*-----------------------------------------------------------------*//**
* @description Finds the largest absolute value among the components of a point or vector.
* @return largest absolute value among point coordinates.
* @indexVerb magnitude
* @bsimethod
+----------------------------------------------------------------------*/
double DPoint2d::MaxAbs
(

) const
    {
    double maxVal = fabs (x);
    if (fabs (y) > maxVal)
        maxVal = fabs (y);
    return maxVal;
    }

/*-----------------------------------------------------------------*//**
* @bsimethod
+----------------------------------------------------------------------*/
double DPoint2d::MaxDiff (DPoint2dCR other) const
    {
    double maxVal = fabs (x - other.x);

    double dy = fabs (y - other.y);
    if (fabs (dy) > maxVal)
        maxVal = fabs (dy);

    return maxVal;
    }

/*-----------------------------------------------------------------*//**
* @return true if the point has coordinates which indicate it is
*   a disconnect (separator) ponit.
* @bsimethod
+----------------------------------------------------------------------*/
bool DPoint2d::IsDisconnect
(

) const
    {
    return x == DISCONNECT
        || y == DISCONNECT;
    }


/*-----------------------------------------------------------------*//**
* Initialize a point with all coordinates as the disconnect value.
* @bsimethod
+----------------------------------------------------------------------*/
void DPoint2d::InitDisconnect ()
    {
    x = y = DISCONNECT;
    }

/*-----------------------------------------------------------------*//**
* @bsimethod
+----------------------------------------------------------------------*/
bool DPoint2d::IsConvexPair (bool allowExtrapolation) const
    {
    double tol = Angle::SmallAngle ();
    if (fabs (1.0 - x - y) > tol)
        return false;
    if (allowExtrapolation)
        return true;
    return x >= -tol && y >= -tol;
    }

//! @return true if a is "to the left of, or below with equal x"
bool DPoint2d::LexicalXYLessThan (DPoint2d const &a, DPoint2d const &b)
    {
    if (a.x < b.x)
        return true;
    if (a.x > b.x)
        return false;
    return a.y < b.y;
    }
END_BENTLEY_NAMESPACE
