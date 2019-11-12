/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include <bsibasegeomPCH.h>
BEGIN_BENTLEY_NAMESPACE



/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
DVec2d DVec2d::From (double x, double y)
    {
    DVec2d vector;
    vector.x = x;
    vector.y = y;
    return vector;
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
DVec2d DVec2d::FromScale
(
DVec2dCR vector,
double   scale
)
    {
    return DVec2d::From (vector.x * scale, vector.y * scale);
    }


/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      11/15
+--------------------------------------------------------------------------------------*/
DVec2d DVec2d::FromRotate90CCW
(
DVec2dCR vector
)
    {
    return DVec2d::From (-vector.y, vector.x);
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      11/15
+--------------------------------------------------------------------------------------*/
DVec2d DVec2d::FromRotate90CCW
(
DVec2dCR vector,
int  numSteps
)
    {
    switch (numSteps & 0x3)
        {
        case 1: return DVec2d::From (-vector.y, vector.x);
        case 2: return DVec2d::From (-vector.x, -vector.y);
        case 3: return DVec2d::From ( vector.y, -vector.x);
        }
    return vector;
    }


/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      11/15
+--------------------------------------------------------------------------------------*/
DVec2d DVec2d::Rotate90CCW () const {return DVec2d::From (-y, x);}
DVec2d DVec2d::Rotate90CW() const { return DVec2d::From(y, -x); }



/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
DVec2d DVec2d::FromSumOf
(
DVec2dCR vector0,
double   scale0,
DVec2dCR vector1,
double   scale1
)
    {
    DVec2d result;
    result.x = vector0.x * scale0 + vector1.x * scale1;
    result.y = vector0.y * scale0 + vector1.y * scale1;
    return result;
    }




/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
DVec2d DVec2d::FromSumOf
(
DVec2dCR vector0,
double   scale0,
DVec2dCR vector1,
double   scale1,
DVec2dCR vector2,
double   scale2
)
    {
    DVec2d result;
    result.x = vector0.x * scale0 + vector1.x * scale1 + vector2.x * scale2;
    result.y = vector0.y * scale0 + vector1.y * scale1 + vector2.y * scale2;
    return result;
    }

// ====
/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
DVec2d DVec2d::FromSumOf
(
DVec2dCR vector0,
DVec2dCR vector1,
double   scale1
)
    {
    DVec2d result;
    result.x = vector0.x + vector1.x * scale1;
    result.y = vector0.y + vector1.y * scale1;
    return result;
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
DVec2d DVec2d::FromSumOf
(
DVec2dCR vector0,
DVec2dCR vector1,
double   scale1,
DVec2dCR vector2,
double   scale2
)
    {
    DVec2d result;
    result.x = vector0.x + vector1.x * scale1 + vector2.x * scale2;
    result.y = vector0.y + vector1.y * scale1 + vector2.y * scale2;
    return result;
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
DVec2d DVec2d::FromSumOf
(
DVec2dCR vector0,
DVec2dCR vector1,
double   scale1,
DVec2dCR vector2,
double   scale2,
DVec2dCR vector3,
double   scale3
)
    {
    DVec2d result;
    result.x = vector0.x + vector1.x * scale1 + vector2.x * scale2 + vector3.x * scale3;
    result.y = vector0.y + vector1.y * scale1 + vector2.y * scale2 + vector3.y * scale3;
    return result;
    }

//===

/*-----------------------------------------------------------------*//**
* @description Returns a DVec2d from a point (treating the point
            as a vector from its origin).
* @param [in] point the point.
+----------------------------------------------------------------------*/
DVec2d DVec2d::From
(
DPoint2dCR      point
)
    {
    DVec2d vector;
    vector.Init (point);
    return vector;
    }

//! Return a DVec2d from start point to end point.
//! @param [in] start start point
//! @param [in] end   end point
DVec2d DVec2d::FromStartEnd
(
DPoint2dCR start,
DPoint2dCR end
)
    {
    DVec2d vector;
    vector.x = end.x - start.x;
    vector.y = end.y - start.y;
    return vector;
    }

//! Return a DVec2d from start point to end point.
//! @param [in] start start point
//! @param [in] end   end point
DVec2d DVec2d::FromStartEnd
(
DPoint3dCR start,
DPoint3dCR end
)
    {
    DVec2d vector;
    vector.x = end.x - start.x;
    vector.y = end.y - start.y;
    return vector;
    }

/*-----------------------------------------------------------------*//**
* @description Returns the (vector) cross product of two vectors.
* @param [in] pVector1 The first vector
* @param [in] vector2 The second vector
* @bsihdr                               EarlinLutz      03/03
+----------------------------------------------------------------------*/
double DVec2d::CrossProduct
(

DVec2dCR vector2

) const
    {
    return this->x * vector2.y - this->y * vector2.x;
    }


/*-----------------------------------------------------------------*//**
* @description Returns the (vector) cross product of two vectors.
* @param [in] pVector1 The first vector
* @param [in] vector2 The second vector
* @bsihdr                               EarlinLutz      03/03
+----------------------------------------------------------------------*/
double DVec2d::CrossProductSquared
(

DVec2dCR vector2

) const
    {
		double cross = this->x * vector2.y - this->y * vector2.x;
    return cross * cross;
    }


/*-----------------------------------------------------------------*//**
* @description Returns the (scalar) dot product of two vectors.
* @param [in] pVector1 The first vector
* @param [in] pVector2 The second vector, given as point.
        The point's xyz are understood to be a vector from the origin.
* @return dot product of the two vectors.
* @bsihdr                               EarlinLutz      03/03
+----------------------------------------------------------------------*/
double DVec2d::DotProduct
(

DPoint2dCR point2

) const
    {
    return (this->x * point2.x + this->y * point2.y);
    }


/*-----------------------------------------------------------------*//**
* @description Returns the (scalar) dot product of two vectors.
* @param [in] vector2 The second vector
* @return The dot product of the two vectors
* @bsihdr                               EarlinLutz      03/03
+----------------------------------------------------------------------*/
double DVec2d::DotProduct
(

DVec2dCR vector2

) const
    {
    return (this->x * vector2.x + this->y * vector2.y);
    }


/*-----------------------------------------------------------------*//**
* @description Computes the dot product of one vector given as a vector structure and another given as
* xyz components.
* @param [in] ax The x component of second vector.
* @param [in] ay The y component of second vector.
* @param [in] az The z component of second vector.
* @return The dot product of the vector with a vector with the given components
* @bsihdr                               EarlinLutz      03/03
+----------------------------------------------------------------------*/
double DVec2d::DotProduct
(

double    ax,
double    ay

) const
    {
    return this->x * ax + this->y * ay;
    }


/*-----------------------------------------------------------------*//**
* @description Computes a unit vector  in the direction of the difference of the vectors
* or vectors (Second parameter vector is subtracted from the first parameter vector,
* exactly as in the subtract function.)
*
* @param [in] target The target point.
* @param [in] origin The origin point.
* @return The length of original difference vector.
* @bsimethod                                            EarlinLutz      12/97
+----------------------------------------------------------------------*/
double DVec2d::NormalizedDifference
(

DPoint2dCR target,
DPoint2dCR origin

)
    {
	this->x = target.x - origin.x;
    this->y = target.y - origin.y;
    return this->Normalize ();
    }


/*-----------------------------------------------------------------*//**
* @description Returns the angle between two vectors.  This angle is between 0 and
* pi.  Rotating the first vector by this angle around the cross product
* between the vectors aligns it with the second vector.
*
* @param [in] vector2 The second vector
* @return The angle between the vectors.
* @bsihdr                               EarlinLutz      03/03
+----------------------------------------------------------------------*/
double DVec2d::AngleTo
(

DVec2dCR vector2

) const
    {
	double cross, dot;
    cross   = this->CrossProduct (vector2);
    dot     = this->DotProduct (vector2);
    return  Angle::Atan2 (cross, dot);
    }


/*-----------------------------------------------------------------*//**
* @description Returns the angle between two vectors, choosing the smaller
*   of the two possible angles when both the vectors and their negations are considered.
*    This angle is between 0 and pi/2.
*
* @param [in] vector2 The second vector
* @return The angle between the vectors.
* @bsihdr                               EarlinLutz      03/03
+----------------------------------------------------------------------*/
double DVec2d::SmallerUnorientedAngleTo
(

DVec2dCR vector2

) const
    {
	double cross, dot;
    cross   = this->CrossProduct (vector2);
    dot     = this->DotProduct (vector2);
    return  Angle::Atan2 (fabs (cross), fabs (dot));
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
* @bsihdr                               EarlinLutz      03/03
+----------------------------------------------------------------------*/
bool DVec2d::IsVectorInSmallerSector
(

DVec2dCR vector0,
DVec2dCR vector1

) const
    {
	double c = vector0.CrossProduct (vector1);
    return		c * vector0.CrossProduct (*this) > 0.0
            &&  c * this->CrossProduct (vector1) > 0.0;
    }




/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
bool DVec2d::IsVectorInCCWSector
(

DVec2dCR vector0,
DVec2dCR vector1

) const
    {
	double cross = vector0.CrossProduct (vector1);

    if (cross == 0.0)
        {
        double dot   = vector0.DotProduct (vector1);
        if (dot > 0.0)
            return false;
        }

    if (cross > 0.0)
        return  vector0.CrossProduct (*this) >  0.0
            &&  this->CrossProduct (vector1) >  0.0;
    else
        return  vector0.CrossProduct (*this) > 0.0
            ||  this->CrossProduct (vector1) > 0.0;
    }


/*-----------------------------------------------------------------*//**
* @description Rotate a vector around the z axis.
* @param [in] theta The rotation angle.
* @bsihdr                               EarlinLutz      03/03
+----------------------------------------------------------------------*/
void DVec2d::RotateCCW
(

DVec2dCR vector,
double      theta

)
    {
    double c, s, xx, yy;
    s = sin (theta);
    c = cos (theta);

    xx = vector.x;
    yy = vector.y;

    this->x = xx * c - yy * s;
    this->y = xx * s + yy * c;
    }

DVec2d DVec2d::FromRotateCCW
(
DVec2dCR vector,
double      theta
)
    {
    double s = sin (theta);
    double c = cos (theta);
    return DVec2d::From
        (
        vector.x * c - vector.y * s,
        vector.x * s + vector.y * c
        );
    }

/*-----------------------------------------------------------------*//**
* @description Rotate a vector around the z axis.
* @param [in] theta The rotation angle.
* @bsihdr                               EarlinLutz      03/03
+----------------------------------------------------------------------*/
void DVec2d::RotateCCW
(

double      theta

)
    {
    double c, s, xx, yy;
    s = sin (theta);
    c = cos (theta);

    xx = this->x;
    yy = this->y;

    this->x = xx * c - yy * s;
    this->y = xx * s + yy * c;
    }


/*-----------------------------------------------------------------*//**
* @description Computes the squared magnitude of a vector.
*
* @return The squared magnitude of the vector.
* @bsihdr                               EarlinLutz      03/03
+----------------------------------------------------------------------*/
double DVec2d::MagnitudeSquared
(

) const
    {
    return this->x * this->x
           + this->y * this->y;
    }


/*---------------------------------------------------------------------------------**//**
* @description Compute a unit vector perpendicular given vector.
*
* @remarks Input may equal output.
*
* @param [out] pRotatedVector The rotated and scaled vector.  Z-coordinate is zero.
* @param [in] vector The source vector
* @return true if the input vector has nonzero length
* @bsimethod                                                    EarlinLutz      03/03
+--------------------------------------------------------------------------------------*/
bool DVec2d::UnitPerpendicular
(
DVec2dCR vector
)
    {
    double  a, d2, vx = vector.x, vy = vector.y;
    this->x = -vy;
    this->y =  vx;
    d2 = vx * vx + vy * vy;
    if (d2 == 0.0)
        return false;
    a = 1.0 / sqrt (d2);
    this->x *= a;
    this->y *= a;
    return true;
    }


/*-----------------------------------------------------------------*//**
* @description Computes the magnitude of a vector.
* @return The length of the vector
* @bsihdr                               EarlinLutz      03/03
+----------------------------------------------------------------------*/
double DVec2d::Magnitude
(

) const
    {
    return sqrt ( this->x * this->x
                 + this->y * this->y);
    }


/*-----------------------------------------------------------------*//**
* @description Multiplies a vector by a scale factor.
* @param [in] vector The vector to be scaled.
* @param [in] scale The scale factor.
* @bsihdr                               EarlinLutz      03/03
+----------------------------------------------------------------------*/
void DVec2d::Scale
(

DVec2dCR vector,
double       scale

)
    {
    this->x = vector.x * scale;
    this->y = vector.y * scale;
    }


/*-----------------------------------------------------------------*//**
* @description Multiplies a vector (in place) by a scale factor.
* @param [in] scale The scale
* @bsihdr                               EarlinLutz      03/03
+----------------------------------------------------------------------*/
void DVec2d::Scale
(

double       scale

)
    {
    this->x *= scale;
    this->y *= scale;
    }


/*-----------------------------------------------------------------*//**
* @description Computes a negated (opposite) vector.
*
* @param [in] vector The vector to be negated.
* @bsihdr                               EarlinLutz      03/03
+----------------------------------------------------------------------*/
void DVec2d::Negate
(

DVec2dCR vector

)
    {
    this->x = - vector.x;
    this->y = - vector.y;
    }


/*-----------------------------------------------------------------*//**
* @description Negate a vector in place.
*
* @bsihdr                               EarlinLutz      03/03
+----------------------------------------------------------------------*/
void DVec2d::Negate
(

)
    {
    this->x = - this->x;
    this->y = - this->y;
    }


/*-----------------------------------------------------------------*//**
* @description Normalizes (scales) a vector to length 1.
* If the input vector length is 0, the output vector is a zero vector
* and the returned length is 0.
*
* @param [in] vector The vector to be normalized.
* @return The length prior to normalization
* @bsihdr                               EarlinLutz      03/03
+----------------------------------------------------------------------*/
double DVec2d::Normalize
(
DVec2dCR vector
)
    {
    *this = vector;
    return Normalize ();
    }

/*-----------------------------------------------------------------*//**
* @bsimethod                            EarlinLutz      07/14
+----------------------------------------------------------------------*/
bool DVec2d::TryNormalize (DVec2dCR other, double &magnitude)
    {
    magnitude = other.Magnitude ();

    double f;
    if (DoubleOps::SafeDivide (f, 1.0, magnitude, 0.0))
        {
        Scale (other, f);
        return true;
        }
    else
        {
        this->x = 1.0;
        this->y = 0.0;
        magnitude = 0.0;
        return false;
        }
    }

/*-----------------------------------------------------------------*//**
* @bsihdr                               EarlinLutz      03/03
+----------------------------------------------------------------------*/
double DVec2d::ScaleToLength (DVec2dCR vector, double length)
    {
    double magnitude = Normalize (vector);
    Scale (length);
    return  magnitude;
    }


/*-----------------------------------------------------------------*//**
* @bsihdr                               EarlinLutz      03/03
+----------------------------------------------------------------------*/
double DVec2d::ScaleToLength (double length)
    {
    return this->ScaleToLength (*this, length);
    }


/*-----------------------------------------------------------------*//**
* @description Replaces a vector by a unit vector in the same direction, and returns
* the original length.
*
* @return The length prior to normalization
* @bsihdr                               EarlinLutz      03/03
+----------------------------------------------------------------------*/
double DVec2d::Normalize ()
    {
    double  magnitude =
            sqrt ( this->x * this->x
                 + this->y * this->y);


    if (magnitude > 0.0)
        {
        double f = 1.0 / magnitude;
        this->x = this->x * f;
        this->y = this->y * f;
        }
    else
        {
        this->x = 1.0;
        this->y = 0.0;
        magnitude = 0.0;
        }

    return  magnitude;
    }


/*-----------------------------------------------------------------*//**
* @description Tests if two vectors are parallel.
*
* @param [in] vector2 The second vector
* @return true if the vectors are parallel within tolerance
* @bsihdr                               EarlinLutz      03/03
+----------------------------------------------------------------------*/
bool DVec2d::IsParallelTo
(

DVec2dCR vector2

) const
    {
    double  a2 = this->DotProduct (*this);
    double  b2 = vector2.DotProduct (vector2);
    double  eps = Angle::SmallAngle(); /* small angle tolerance (in radians) */
    double  crossSquared = this->CrossProductSquared (vector2);

    /* a2,b2,c2 are squared lengths of respective vectors */
    /* c2 = sin^2(theta) * a2 * b2 */
    /* For small theta, sin^2(theta)~~theta^2 */
    /* Zero-length vector case falls through as equality */

    return  crossSquared <= eps * eps * a2 * b2;
    }

/*-----------------------------------------------------------------*//**
* @description Tests if two vectors are parallel and positive dot product
*
* @param [in] vector2 The second vector
* @return true if the vectors are parallel within tolerance
* @bsimethod                            EarlinLutz      03/03
+----------------------------------------------------------------------*/
bool DVec2d::IsPositiveParallelTo
(
DVec2dCR vector2
) const
    {
    return IsParallelTo (vector2) && this->DotProduct (vector2) > 0.0;
    }


/*-----------------------------------------------------------------*//**
* @description Tests if two vectors are perpendicular.
*
* @param [in] vector2 The second vector
* @return true if vectors are perpendicular within tolerance
* @bsihdr                               EarlinLutz      03/03
+----------------------------------------------------------------------*/
bool DVec2d::IsPerpendicularTo
(

DVec2dCR vector2

) const
    {
    double  aa = this->DotProduct (*this);
    double  bb = vector2.DotProduct (vector2);
    double  ab = this->DotProduct (vector2);
    double  eps = Angle::SmallAngle();

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
* @bsihdr                               EarlinLutz      03/03
+----------------------------------------------------------------------*/
bool DVec2d::SafeDivide
(

DVec2dCR vector,
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
        this->x = vector.x * a;
        this->y = vector.y * a;
        return true;
        }
    }


/*-----------------------------------------------------------------*//**
* @description Sets all components of a vector to zero.
* @bsihdr                               EarlinLutz      03/03
+----------------------------------------------------------------------*/
void DVec2d::Zero
(

)
    {
    this->x = 0.0;
    this->y = 0.0;
    }


/*-----------------------------------------------------------------*//**
* @description Returns a vector with all components 1.0.
* @bsihdr                               EarlinLutz      03/03
+----------------------------------------------------------------------*/
void DVec2d::One
(

)
    {
    this->x = 1.0;
    this->y = 1.0;
    }



/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
void DVec2d::Init
(
DPoint2dCR point
)
    {
    this->x = point.x;
    this->y = point.y;
    }


/*-----------------------------------------------------------------*//**
* @description Sets the x, and y components of a vector. Sets z to zero.
*
* @param [in] ax The x component.
* @param [in] ax The x component.
* @param [in] ay The y component
* @bsihdr                               EarlinLutz      03/03
+----------------------------------------------------------------------*/
void DVec2d::Init
(

double       ax,
double       ay

)
    {
    this->x = ax;
    this->y = ay;
    }


/*-----------------------------------------------------------------*//**
* @description Sets the x,y, and z components of a DVec2d structure from the
* corresponding parts of a DPoint4d.  Weight part of DPoint4d is not used.
*
* @param [in] hPoint The homogeneous point
* @bsihdr                               EarlinLutz      03/03
+----------------------------------------------------------------------*/
void DVec2d::XyOf
(

DPoint4dCR hPoint

)
    {
    this->x = hPoint.x;
    this->y = hPoint.y;
    }


/*-----------------------------------------------------------------*//**
* @description Set one of three components (x,y) of the vector.
*
* @param [in] a The component value.
* @param [in] index Selects the the axis: 0=x, 1=y
* @bsihdr                               EarlinLutz      03/03
+----------------------------------------------------------------------*/
void DVec2d::SetComponent
(

double       a,
int         index

)
    {
    index = index & 0x01;
    if (index == 0)
        {
        this->x = a;
        }
    else
        {
        this->y = a;
        }
    }


/*-----------------------------------------------------------------*//**
* @description Gets a single component of a vector.  If the index is out of
* range 0,1 it is interpretted cyclically.
*
* @param [in] index Indicates which component is accessed.  The values
*                       are 0=x, 1=y  Other values are treated cyclically.
* @return The specified component of the vector.
* @bsihdr                               EarlinLutz      03/03
+----------------------------------------------------------------------*/
double DVec2d::GetComponent
(

int         index

) const
    {
    index = index & 0x01;
    if (index == 0)
        {
        return this->x;
        }
    else
        {
        return this->y;
        }
    }


/*-----------------------------------------------------------------*//**
* @description Copies x,y components from a vector to individual variables.
*
* @param [out] &xCoord x component
* @param [out] &yCoord y component
* @bsihdr                               EarlinLutz      03/03
+----------------------------------------------------------------------*/
void DVec2d::GetComponents
(

        double      &xCoord,
        double      &yCoord

)
    {
    xCoord = this->x;
    yCoord = this->y;
    }


/*-----------------------------------------------------------------*//**
* @description Compute the sum of two vectors or vectors.
*
* @param [in] vector1 The the first vector
* @param [in] vector2 The second vector
* @bsihdr                               EarlinLutz      03/03
+----------------------------------------------------------------------*/
void DVec2d::SumOf
(

DVec2dCR vector1,
DVec2dCR vector2

)
    {
    this->x = vector1.x + vector2.x;
    this->y = vector1.y + vector2.y;
    }


/*-----------------------------------------------------------------*//**
* @description Adds a vector to a pointer or vector, returns the result in place.
*
* @param [in] vector The vector to add.
* @bsihdr                               EarlinLutz      03/03
+----------------------------------------------------------------------*/
void DVec2d::Add
(

DVec2dCR vector

)
    {
    this->x += vector.x;
    this->y += vector.y;
    }


/*-----------------------------------------------------------------*//**
* @description Subtract two vectors, and return the result in
*           place of the first.
*
* @param [in] vector2 The vector to subtract.
* @bsihdr                               EarlinLutz      03/03
+----------------------------------------------------------------------*/
void DVec2d::Subtract
(

DVec2dCR vector2

)
    {
    this->x -= vector2.x;
    this->y -= vector2.y;
    }


/*-----------------------------------------------------------------*//**
* @description Adds an origin and a scaled vector.
*
* @param [in] origin Origin for the sum.
* @param [in] vector The vector to be added.
* @param [in] scale The scale factor.
* @bsihdr                               EarlinLutz      03/03
+----------------------------------------------------------------------*/
void DVec2d::SumOf
(

DVec2dCR origin,
DVec2dCR vector,
double           scale

)
    {
    this->x = origin.x + vector.x * scale;
    this->y = origin.y + vector.y * scale;
    }


/*-----------------------------------------------------------------*//**
* @description Computes a vector whose position is given by a fractional
* argument and two vectors.
*
* @param [in] vector0 The vector corresponding to fractionParameter of 0.
* @param [in] fractionParameter The fractional parametric coordinate.
*               0.0 is the start of the segment, 1.0 is the end, 0.5 is middle
* @param [in] vector1 The vector corresponding to fractionParameter of 1.
* @bsihdr                               EarlinLutz      03/03
+----------------------------------------------------------------------*/
void DVec2d::Interpolate
(

DVec2dCR vector0,
double       fractionParameter,
DVec2dCR vector1

)
    {
    this->x = vector0.x + fractionParameter * (vector1.x - vector0.x);
    this->y = vector0.y + fractionParameter * (vector1.y - vector0.y);
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
DVec2d DVec2d::FromInterpolate
(
DVec2dCR vector0,
double   fractionParameter,
DVec2dCR vector1
)
    {
    DVec2d result;
    result.x = vector0.x + fractionParameter * (vector1.x - vector0.x);
    result.y = vector0.y + fractionParameter * (vector1.y - vector0.y);
    return result;
    }


/*-----------------------------------------------------------------*//**
* @description Adds an origin and two scaled vectors.
*
* @param [in] origin The origin.  May be null.
* @param [in] vector1 The first direction vector
* @param [in] scale1 The first scale factor
* @param [in] vector2 The second direction vector
* @param [in] scale2 The second scale factor
* @bsihdr                               EarlinLutz      03/03
+----------------------------------------------------------------------*/
void DVec2d::SumOf
(

DVec2dCR origin,
DVec2dCR vector1,
double           scale1,
DVec2dCR vector2,
double           scale2

)
    {
    this->x = origin.x + vector1.x * scale1 + vector2.x * scale2;
    this->y = origin.y + vector1.y * scale1 + vector2.y * scale2;
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
* @bsihdr                               EarlinLutz      03/03
+----------------------------------------------------------------------*/
void DVec2d::SumOf
(

DVec2dCR origin,
DVec2dCR vector1,
double          scale1,
DVec2dCR vector2,
double          scale2,
DVec2dCR vector3,
double          scale3

)
    {
    this->x = origin.x + vector1.x * scale1 + vector2.x * scale2 + vector3.x * scale3;
    this->y = origin.y + vector1.y * scale1 + vector2.y * scale2 + vector3.y * scale3;
    }


/*-----------------------------------------------------------------*//**
* @description Subtract coordinates of two vectors. (Compute Vector1 - Vector2)
*
* @param [in] vector1 The first vector
* @param [in] vector2 The second (subtracted) vector
* @bsihdr                               EarlinLutz      03/03
+----------------------------------------------------------------------*/
void DVec2d::DifferenceOf
(

DVec2dCR vector1,
DVec2dCR vector2

)
    {
    this->x = vector1.x - vector2.x;
    this->y = vector1.y - vector2.y;
    }


/*-----------------------------------------------------------------*//**
* @description Subtract coordinates of two points. (Compute Point1 - Point2)
*
* @param [in] target The target point
* @param [in] base The base point
* @bsihdr                               EarlinLutz      03/03
+----------------------------------------------------------------------*/
void DVec2d::DifferenceOf
(

DPoint2dCR target,
DPoint2dCR base

)
    {
    this->x = target.x - base.x;
    this->y = target.y - base.y;
    }


/*-----------------------------------------------------------------*//**
* @description Computes the (cartesian) distance between two vectors
*
* @param [in] vector2 The second vector
* @return The distance between vector.
* @bsihdr                               EarlinLutz      03/03
+----------------------------------------------------------------------*/
double DVec2d::Distance
(

DVec2dCR vector2

) const
    {
    double      xdist, ydist;

    xdist = vector2.x - this->x;
    ydist = vector2.y - this->y;

    return (sqrt (xdist * xdist + ydist * ydist));
    }


/*-----------------------------------------------------------------*//**
* @description Computes the squared distance between two vectors.
*
* @param [in] vector2 The second vector.
* @return The squared distance between the vectors.
* @bsihdr                               EarlinLutz      03/03
+----------------------------------------------------------------------*/
double DVec2d::DistanceSquared
(

DVec2dCR vector2

) const
    {
    double      xdist, ydist;

    xdist = (vector2.x - this->x);
    ydist = (vector2.y - this->y);

    return (xdist * xdist + ydist * ydist);
    }


/*-----------------------------------------------------------------*//**
* @description Finds the largest absolute value among the components of a vector.
* @return The largest absolute value among vector coordinates.
* @bsihdr                               EarlinLutz      03/03
+----------------------------------------------------------------------*/
double DVec2d::MaxAbs
(

) const
    {
    double maxVal = fabs (this->x);

    if (fabs (this->y) > maxVal)
        maxVal = fabs (this->y);

    return maxVal;
    }


/*-----------------------------------------------------------------*//**
* @description Test for exact equality between all components of two vectors.
* @param [in] vector2 The second vector
* @return true if the vectors are identical.
* @bsihdr                               EarlinLutz      03/03
+----------------------------------------------------------------------*/
bool DVec2d::IsEqual
(

DVec2dCR vector2

) const
    {
    bool                 result;
    result =    this->x == vector2.x
             && this->y == vector2.y;
    return  result;
    }


/*-----------------------------------------------------------------*//**
* @description Test if the x, y, and z components of two vectors are
*   equal within tolerance.
* Tests are done independently using the absolute value of each component differences
* (i.e. not the magnitude or sum of squared differences)
*
* @param [in] vector2 The second vector.
* @param [in] tolerance The tolerance.
* @return true if all components are within given tolerance of each other.
* @bsihdr                               EarlinLutz      03/03
+----------------------------------------------------------------------*/
bool DVec2d::IsEqual
(

DVec2dCR vector2,
double                  tolerance

) const
    {
    bool                 result;
    result = fabs(this->x - vector2.x) <= tolerance &&
     fabs(this->y - vector2.y) <= tolerance;
    return  result;
    }

//! Return the fractional coordinate where the other vector projects onto the instance. (0 if instance has no length)
ValidatedDouble DVec2d::ProjectionFraction (DVec2d const &other) const
    {
    double uv = DotProduct (other);
    double uu = MagnitudeSquared ();
    return DoubleOps::ValidatedDivideDistance (uv, uu);
    }

//! @return the fractional position along the instance when the other vector is projected perpendicular to the instance.
//! @remark 0.0 is returned if instance length is zero !!!
ValidatedDouble DVec2d::PerpendicularProjectionFraction (DVec2d const &other) const
    {
    double uv = CrossProduct (other);
    double uu = MagnitudeSquared ();
    return DoubleOps::ValidatedDivideDistance (uv, uu, 0.0);
    }

DVec2d DVec2d::OffsetBisector
(
DVec2d const &unitPerpA,
DVec2d const &unitPerpB,
double offset
)
    {
    DVec2d bisector = unitPerpA + unitPerpB;
    bisector.Normalize ();
    double c = offset * bisector.DotProduct (unitPerpA);
    return c * bisector;
    }
END_BENTLEY_NAMESPACE
