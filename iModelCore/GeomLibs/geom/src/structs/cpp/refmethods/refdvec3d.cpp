/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include <bsibasegeomPCH.h>
BEGIN_BENTLEY_NAMESPACE

template<typename T>
struct VectorProducts
{
DVec3d crossVector;
double crossMag;
double dot;
VectorProducts (T const &vector0, T const &vector1)
    {
    crossVector = DVec3d::FromCrossProduct (vector0, vector1);
    dot = vector0.DotProduct (vector1);
    crossMag = crossVector.Magnitude ();
    }
void ForcePositive ()
    {
    crossMag = fabs (crossMag);
    dot      = fabs (dot);
    }
// return dot product of vector the crossVector. (And inlining will handle float/double matchup)
template <typename T1>
double DotWithVectorCross (T1 const &vector)
    {
    return vector.x * crossVector.x + vector.y * crossVector.y + vector.z * crossVector.z;
    }
};

template<typename T>
struct VectorProductsXY
{
double cross;
double dot;
VectorProductsXY (T const &vector0, T const &vector1)
    {
    cross = vector0.CrossProductXY (vector1);
    dot = vector0.DotProductXY (vector1);
    }
void ForcePositive ()
    {
    cross    = fabs (cross);
    dot      = fabs (dot);
    }
};

// Goal: do all internal computations double (i.e intermediate cross products are always DVec3d)
struct GeometryTemplates
{
template <typename VectorType>
static DVec3d SumOf (VectorType const & vector0, VectorType const &vector1, double s1)
    {
    DVec3d result;
    result.x = vector0.x + vector1.x *s1;
    result.y = vector0.y + vector1.y *s1;
    result.z = vector0.z + vector1.z *s1;
    return result;
    }


template<typename VectorType>
static bool IsPerpendicularTo (VectorType const &vector0, VectorType const &vector1, double eps)
    {
    double      aa = vector0.DotProduct (vector0);
    double      bb = vector1.DotProduct (vector1);
    double      ab = vector0.DotProduct (vector1);
    return  ab * ab <= eps * eps * aa * bb;
    }
template<typename ArgType, typename ResultType>
static inline ResultType FromStartEnd (ArgType const &start, ArgType const &end)
    {
    return ResultType::From (end.x - start.x, end.y - start.y, end.z - start.z);
    }

template<typename ArgType, typename ResultType>
static inline ValidatedValue <ResultType> FromStartEndNormalized (ArgType const &start, ArgType const &end)
    {
    ResultType rawVector = FromStartEnd<ArgType, ResultType> (start, end);
    double a = rawVector.Magnitude ();
    if (a >= DoubleOps::SmallMetricDistance ())
        return ValidatedValue<ResultType> (ResultType::From (rawVector.x / a, rawVector.y / a, rawVector.z / a), true);
    return ValidatedValue<ResultType> (rawVector, false);
    }

template<typename ArgType, typename ResultType>
static inline ResultType DotProduct(ArgType x0, ArgType y0, ArgType z0, ArgType x1, ArgType y1, ArgType z1)
    {
    return x0 * x1 + y0 * y1 + z0 * z1;
    }

template<typename ArgType, typename ResultType>
static inline ResultType CrossProduct(ArgType x0, ArgType y0, ArgType z0, ArgType x1, ArgType y1, ArgType z1)
    {
    return ResultType::From (
                y0 * z1 - z0 * y1,
                z0 * x1 - x0 * z1,
                x0 * y1 - y0 * x1
                );
    }

template<typename VectorType>
static double RadiansTo (VectorType const &vector0, VectorType const &vector1)
    {
    VectorProducts<VectorType> p (vector0, vector1);
    return  atan2 (p.crossMag, p.dot);
    }

template<typename ArgType, typename ResultType>
static inline ResultType DotProductXY (ArgType x0, ArgType y0, ArgType x1, ArgType y1)
    {
    return x0 * x1 + y0 * y1;
    }

template<typename ArgType, typename ResultType>
static inline ResultType CrossProductXY (ArgType x0, ArgType y0, ArgType x1, ArgType y1)
    {
    return x0 * y1 - y0 * x1;
    }

template<typename VectorType, typename VectorType1>
static double SignedRadiansTo (VectorType const &vector0, VectorType const &vector1, VectorType1 const &vectorW)
    {
    VectorProducts<VectorType> p (vector0, vector1);
    double theta = atan2 (p.crossMag, p.dot);

    if (p.template DotWithVectorCross<VectorType1> (vectorW) < 0.0)
        return  -theta;
    else
        return  theta;
    }

template<typename VectorType>
static double PlanarRadiansTo (VectorType const &vector0, VectorType const &vector1, VectorType const &planeNormal)
    {
    double      square = planeNormal.DotProduct (planeNormal);
    if (square == 0.0)
        return 0.0;

    double factor = 1.0 / square;

    auto projection0 = SumOf<VectorType> (vector0, planeNormal, - vector0.DotProduct (planeNormal) * factor);
    auto projection1 = SumOf<VectorType> (vector1, planeNormal, - vector1.DotProduct (planeNormal) * factor);
    return  SignedRadiansTo<DVec3d, VectorType> (projection0, projection1, planeNormal);
    }


template<typename VectorType>
static double SmallerUnorientedRadiansTo (VectorType const &vector0, VectorType const &vector1)
    {
    VectorProducts<VectorType> p (vector0, vector1);
    p.ForcePositive ();
    return atan2 (p.crossMag,p.dot);
    }

template<typename VectorType>
static double SmallerUnorientedRadiansToXY (VectorType const &vector0, VectorType const &vector1)
    {
    VectorProductsXY<VectorType> p (vector0, vector1);
    p.ForcePositive ();
    return atan2 (p.cross,p.dot);
    }


//! return (signed) angle from this vector to the other, as viewed in xy direction.
template<typename VectorType>
static double RadiansToXY (VectorType const &vector0, VectorType const &vector1)
    {
    VectorProductsXY<VectorType> p (vector0, vector1);
    return atan2 (p.cross,p.dot);
    }

template<typename VectorType>
static double RadiansFromPerpendicular (VectorType const &vector0, VectorType const &vector1)
    {
    VectorProducts<VectorType> p (vector0, vector1);
    return  atan2 (fabs (p.dot), p.crossMag);
    }

};
/*-----------------------------------------------------------------*//**
* @description Returns a DVec3d with 3 component array of double.
*
* @param [in] pXyz x, y, z components
+----------------------------------------------------------------------*/
DVec3d DVec3d::FromArray
(
const   double  *pXyz
)
    {
    DVec3d vector;
    vector.x = pXyz[0];
    vector.y = pXyz[1];
    vector.z = pXyz[2];
    return vector;
    }



/*-----------------------------------------------------------------*//**
* @description Returns a DVec3d with given  x,y, and z components of a vector
*
* @param [in] ax The x component.
* @param [in] ay The y component.
* @param [in] az The z component.
+----------------------------------------------------------------------*/
DVec3d DVec3d::From
(
double          ax,
double          ay,
double          az
)
    {
    DVec3d vector;
    vector.x = ax;
    vector.y = ay;
    vector.z = az;
    return vector;
    }






/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
DVec3d DVec3d::FromScale
(
DVec3dCR vector,
double   scale
)
    {
    return DVec3d::From (vector.x * scale, vector.y * scale, vector.z * scale);
    }



/*-----------------------------------------------------------------*//**
* @description Returns a DVec3d from a point (treating the point
            as a vector from its origin).
* @param [in] point the point.
+----------------------------------------------------------------------*/
DVec3d DVec3d::From
(
DPoint3dCR      point
)
    {
    DVec3d vector;
    vector.x = point.x;
    vector.y = point.y;
    vector.z = point.z;
    return vector;
    }

DVec3d DVec3d::From
(
DVec2dCR uv
)
    {
    DVec3d vector;
    vector.x = uv.x;
    vector.y = uv.y;
    vector.z = 0.0;
    return vector;
    }



/*-----------------------------------------------------------------*//**
* @description Returns a DVec3d between start and end.
* @param [in] start start point
* @param [in] end   end point
+----------------------------------------------------------------------*/
DVec3d DVec3d::FromStartEnd
(
DPoint3dCR start,
DPoint3dCR end
)
    {
    DVec3d vector;
    vector.x = end.x - start.x;
    vector.y = end.y - start.y;
    vector.z = end.z - start.z;
    return vector;
    }

/*-----------------------------------------------------------------*//**
* @description Returns a DVec3d between start and end.
* @param [in] start start point
* @param [in] end   end point
+----------------------------------------------------------------------*/
DVec3d DVec3d::FromStartEnd
(
FPoint3dCR start,
FPoint3dCR end
)
    {
    DVec3d vector;
    vector.x = end.x - start.x;
    vector.y = end.y - start.y;
    vector.z = end.z - start.z;
    return vector;
    }

DVec3d DVec3d::From (FVec3dCR f)
    {
    return From (f.x, f.y, f.z);
    }

/*-----------------------------------------------------------------*//**
* @description Returns a DVec3d between start and end, normalized if nonzero.
* @param [in] start start point
* @param [in] end   end point
+----------------------------------------------------------------------*/
DVec3d DVec3d::FromStartEndNormalize
(
DPoint3dCR start,
DPoint3dCR end
)
    {
    DVec3d vector;
    vector.x = end.x - start.x;
    vector.y = end.y - start.y;
    vector.z = end.z - start.z;
    vector.Normalize ();
    return vector;
    }

ValidatedDVec3d DVec3d::ValidatedNormalize() const
    {
    DVec3d result;
    double magnitude;
    bool stat = result.TryNormalize(*this, magnitude);
    return ValidatedDVec3d(result, stat);
    }

DVec3d DVec3d::FromCCWPerpendicularXY (DVec3d source)
    {
    DVec3d vector;
    vector.x = -source.y;
    vector.y = source.x;
    vector.z = source.z;
    return vector;    
    }

DVec3d DVec3d::FromRotate90Towards (DVec3dCR source, DVec3dCR target)
    {
    DVec3d normal = FromNormalizedCrossProduct (source, target);
    return FromCrossProduct (normal, source);
    }

DVec3d DVec3d::FromRotate90Around (DVec3dCR source, DVec3dCR axis)
    {
    DVec3d unitNormal = axis;
    unitNormal.Normalize ();
    return FromCrossProduct (unitNormal, source) + unitNormal.DotProduct (source) * unitNormal;
    }

ValidatedDVec3d DVec3d::FromRotateVectorAroundVector (DVec3dCR vector, DVec3dCR axis, Angle angle)
    {
    // Rodriguez formulat, https://en.wikipedia.org/wiki/Rodrigues'_rotation_formula
    auto unitAxis = axis.ValidatedNormalize ();
    if (unitAxis.IsValid ())
        {
        double c = angle.Cos ();
        double s = angle.Sin ();
        DVec3d unit = unitAxis.Value ();
        return ValidatedDVec3d
            (
            c * vector + s * DVec3d::FromCrossProduct (unit, vector) + (unit.DotProduct (vector) * (1.0 - c)) * unit,
            true
            );
        }
    // unchanged vector if axis is null
    return ValidatedDVec3d (vector, false);
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
DVec3d DVec3d::FromStartEnd
(
DPoint4dCR start,
DPoint4dCR end
)
    {
    DVec3d vector;
    vector.x = end.x * start.w -  start.x * end.w;
    vector.y = end.y * start.w  - start.y * end.w;
    vector.z = end.z * start.w  - start.z * end.w;
    return vector;
    }



/*-----------------------------------------------------------------*//**
* @description Returns a DVec3d from given angle and distance in xy plane.
*       Z part is set to zero.
*
* @param [in] theta Angle from X axis to the vector, in the xy plane.
* @param [in] magnitude Vector magnitude
+----------------------------------------------------------------------*/
DVec3d DVec3d::FromXYAngleAndMagnitude
(
double          theta,
double          magnitude
)
    {
    DVec3d vector;
    vector.x = magnitude * cos (theta);
    vector.y = magnitude * sin (theta);
    vector.z = 0.0;
    return vector;
    }




/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
DVec3d DVec3d::FromMatrixColumn
(
TransformCR transform,
int i
) 
    {
    DVec3d result;
    i = Angle::Cyclic3dAxis (i);
    result.x = transform.form3d[0][i];
    result.y = transform.form3d[1][i];
    result.z = transform.form3d[2][i];
    return result;
    }




/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
DVec3d DVec3d::FromTranslation
(
TransformCR source
)
    {
    DVec3d result;
    result.x = source.form3d[0][3];
    result.y = source.form3d[1][3];
    result.z = source.form3d[2][3];
    return result;
    }


DVec3d DVec3d::FromStartEnd (TransformCR start, DPoint3dCR target)
    {
    return DVec3d::From
        (
        target.x - start.form3d[0][3],
        target.y - start.form3d[1][3],
        target.z - start.form3d[2][3]
        );
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
DVec3d DVec3d::FromColumn
(
RotMatrixCR matrix,
int i
)
    {
    DVec3d result;
    i = Angle::Cyclic3dAxis (i);
    result.x = matrix.form3d[0][i];
    result.y = matrix.form3d[1][i];
    result.z = matrix.form3d[2][i];
    return result;
    }




/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
DVec3d DVec3d::FromMatrixRow
(
TransformCR transform,
int i
)
    {
    DVec3d result;
    i = Angle::Cyclic3dAxis (i);
    result.x = transform.form3d[i][0];
    result.y = transform.form3d[i][1];
    result.z = transform.form3d[i][2];
    return result;
    }




/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
DVec3d DVec3d::FromRow
(
RotMatrixCR matrix,
int i
)
    {
    DVec3d result;
    i = Angle::Cyclic3dAxis (i);
    result.x = matrix.form3d[i][0];
    result.y = matrix.form3d[i][1];
    result.z = matrix.form3d[i][2];
    return result;
    }




/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
DVec3d DVec3d::FromMatrixColumnCrossProduct
(
TransformCR transform,
int i,
int j
)
    {
    i = Angle::Cyclic3dAxis (i);
    j = Angle::Cyclic3dAxis (j);
    return FromCrossProduct (transform.form3d[0][i], transform.form3d[1][i], transform.form3d[2][i],
                transform.form3d[0][j], transform.form3d[1][j], transform.form3d[2][j]);
    }




/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
DVec3d DVec3d::FromColumnCrossProduct
(
RotMatrixCR matrix,
int i,
int j
)
    {
    i = Angle::Cyclic3dAxis (i);
    j = Angle::Cyclic3dAxis (j);
    return FromCrossProduct (matrix.form3d[0][i], matrix.form3d[1][i], matrix.form3d[2][i],
                matrix.form3d[0][j], matrix.form3d[1][j], matrix.form3d[2][j]);
    }




/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
DVec3d DVec3d::FromCrossProduct
(
double x0,
double y0,
double z0,
double x1,
double y1,
double z1
)
    {
    DVec3d result;
    result.x = y0 * z1 - z0 * y1;
    result.y = z0 * x1 - x0 * z1;
    result.z = x0 * y1 - y0 * x1;
    return result;
    }





/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
DVec3d DVec3d::FromCrossProduct
(
DVec3dCR vector0,
DVec3dCR vector1
)
    {
    return DVec3d::FromCrossProduct (vector0.x, vector0.y, vector0.z, vector1.x, vector1.y, vector1.z);
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
DVec3d DVec3d::FromCrossProduct
(
FVec3dCR vector0,
FVec3dCR vector1
)
    {
    return DVec3d::FromCrossProduct (vector0.x, vector0.y, vector0.z, vector1.x, vector1.y, vector1.z);
    }



/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
DVec3d DVec3d::FromNormalizedCrossProduct
(
DVec3dCR vector0,
DVec3dCR vector1
)
    {
    DVec3d vector = DVec3d::FromCrossProduct (vector0.x, vector0.y, vector0.z, vector1.x, vector1.y, vector1.z);
    vector.Normalize ();
    return vector;
    }




/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
void DVec3d::CrossProduct
(
DVec3dCR vector0,
DVec3dCR vector1
)
    {
    double xx = vector0.y * vector1.z - vector0.z * vector1.y;
    double yy = vector0.z * vector1.x - vector0.x * vector1.z;
    double zz = vector0.x * vector1.y - vector0.y * vector1.x;
    this->x = xx;
    this->y = yy;
    this->z = zz;
    }
/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
double DVec3d::CrossProductMagnitude
(
DVec3dCR other
) const
    {
    double xx = y * other.z - z * other.y;
    double yy = z * other.x - x * other.z;
    double zz = x * other.y - y * other.x;
    return sqrt (xx * xx + yy * yy + zz * zz);
    }



/*-----------------------------------------------------------------*//**
* @description Returns the (vector) cross product of two vectors.
* @param [in] vector1 The first vector
* @param [in] point2 The second vector, given as a point.
        The point's xyz are understood to be a vector from the origin.
* @bsimethod                            EarlinLutz      03/03
+----------------------------------------------------------------------*/
void DVec3d::CrossProduct
(

DVec3dCR vector1,
DPoint3dCR point2

)
    {
    double xx = vector1.y * point2.z - vector1.z * point2.y;
    double yy = vector1.z * point2.x - vector1.x * point2.z;
    double zz = vector1.x * point2.y - vector1.y * point2.x;
    this->x = xx;
    this->y = yy;
    this->z = zz;
    }


/*-----------------------------------------------------------------*//**
* @description Returns the (vector) cross product of two vectors.
* @param [in] point1 The first vector, givenn as a point.
        The point's xyz are understood to be a vector from the origin.
* @param [in] vector2 The second vector.
* @bsimethod                            EarlinLutz      03/03
+----------------------------------------------------------------------*/
void DVec3d::CrossProduct
(

DPoint3dCR point1,
DVec3dCR vector2

)
    {
    double xx = point1.y * vector2.z - point1.z * vector2.y;
    double yy = point1.z * vector2.x - point1.x * vector2.z;
    double zz = point1.x * vector2.y - point1.y * vector2.x;
    this->x = xx;
    this->y = yy;
    this->z = zz;
    }


/*-----------------------------------------------------------------*//**
* @description Returns the (scalar) dot product of two vectors.
* @param [in] pVector1 The first vector
* @param [in] pVector2 The second vector, given as point.
        The point's xyz are understood to be a vector from the origin.
* @return dot product of the two vectors.
* @bsimethod                            EarlinLutz      03/03
+----------------------------------------------------------------------*/
double DVec3d::DotProduct (DPoint3dCR other) const
    {
    return GeometryTemplates::DotProduct <double, double> (x, y, z, other.x, other.y, other.z);
    }





/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
bool DVec3d::GetPerpendicularParts
(
DVec3dCR hypotenuse, 
double &fraction, 
DVec3dR parallelPart, 
DVec3dR perpendicularPart
) const
    {
    double UdotV = DotProduct (hypotenuse);
    double UdotU = MagnitudeSquared ();
    bool stat =  DoubleOps::SafeDivide (fraction, UdotV, UdotU, 0.0);
    parallelPart.Scale (*this, fraction);
    perpendicularPart.DifferenceOf (hypotenuse, parallelPart);
    return stat;
    }








/*-----------------------------------------------------------------*//**
* @description Returns the (vector) cross product of two vectors.
*   The vectors are computed from the Origin to Target1 and Target2.
* @param [in] origin The base point for computing vectors.
* @param [in] target1 The target point for the first vector.
* @param [in] target2 The target point for the second vector.
* @bsimethod                            EarlinLutz      03/03
+----------------------------------------------------------------------*/
void DVec3d::CrossProductToPoints
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

    this->x = y1 * z2 - z1 * y2;
    this->y = z1 * x2 - x1 * z2;
    this->z = x1 * y2 - y1 * x2;
    }

/*-----------------------------------------------------------------*//**
* @description Returns the (vector) cross product of two vectors.
*   The vectors are computed from the Origin to Target1 and Target2.
* @param [in] origin The base point for computing vectors.
* @param [in] target1 The target point for the first vector.
* @param [in] target2 The target point for the second vector.
* @bsimethod                            EarlinLutz      03/03
+----------------------------------------------------------------------*/
DVec3d DVec3d::FromCrossProductToPoints
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

    DVec3d result;
    result.x = y1 * z2 - z1 * y2;
    result.y = z1 * x2 - x1 * z2;
    result.z = x1 * y2 - y1 * x2;
    return result;
    }

/*-----------------------------------------------------------------*//**
* @description Returns the (vector) cross product of two vectors.
*   The vectors are computed from the Origin to Target1 and Target2.
* @param [in] origin The base point for computing vectors.
* @param [in] target1 The target point for the first vector.
* @param [in] target2 The target point for the second vector.
* @bsimethod                            EarlinLutz      03/03
+----------------------------------------------------------------------*/
DVec3d DVec3d::FromNormalizedCrossProductToPoints
(
DPoint3dCR origin,
DPoint3dCR target1,
DPoint3dCR target2
)
    {
    DVec3d result = FromCrossProductToPoints (origin, target1, target2);
    result.Normalize ();
    return result;
    }

/*-----------------------------------------------------------------*//**
* @description Return the (scalar) cross product of the xy parts of two vectors.
* @param [in] pVector1 The first vector
* @param [in] vector2 The second vector
* @return The 2d cross product.
* @bsimethod                            EarlinLutz      03/03
+----------------------------------------------------------------------*/
double DVec3d::CrossProductXY
(

DVec3dCR vector2

) const
    {
    return  this->x * vector2.y - this->y * vector2.x;
    }


/*-----------------------------------------------------------------*//**
* @description Compute the normalized cross product of two vectors
* and return the length of the unnormalized cross product.
*
* @param [in] vector1 The first vector
* @param [in] vector2 The second vector
* @return The length of the original (prenormalization) cross product vector
* @bsimethod                            EarlinLutz      03/03
+----------------------------------------------------------------------*/
double DVec3d::NormalizedCrossProduct
(

DVec3dCR vector1,
DVec3dCR vector2

)
    {
    this->CrossProduct(vector1, vector2);
    return this->Normalize ();
    }


/*-----------------------------------------------------------------*//**
* @description Computes the cross product of the two parameter vectors and scales it to a given
* length.  The scaled vector is stored as the product vector, and the length of the original
* cross product vector is returned.
*
* @param [in] vector1 The first vector
* @param [in] vector2 The second vector
* @param [in] productLength The Desired length
* @return The length of unscaled cross product.
* @bsimethod                            EarlinLutz      03/03
+----------------------------------------------------------------------*/
double DVec3d::SizedCrossProduct
(

DVec3dCR vector1,
DVec3dCR vector2,
        double      productLength

)
    {
    double length;
    this->CrossProduct(vector1, vector2);
    length = this->Magnitude ();
    if (length != 0)
        this->Scale (*this, productLength / length);
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
* @return The length of unscaled cross product.
* @bsimethod                            EarlinLutz      03/03
+----------------------------------------------------------------------*/
double DVec3d::GeometricMeanCrossProduct
(

DVec3dCR vector1,
DVec3dCR vector2

)
    {
    double aa, bb, c;
    aa = vector1.MagnitudeSquared ();
    bb = vector2.MagnitudeSquared ();
    c = sqrt (sqrt (aa * bb));
    return SizedCrossProduct (vector1, vector2, c);
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
* @bsimethod                            EarlinLutz      03/03
+----------------------------------------------------------------------*/
double DVec3d::TripleProduct
(

DVec3dCR vector2,
DVec3dCR vector3

) const
    {
    return
          this->x * ( vector2.y * vector3.z - vector2.z * vector3.y )
        + this->y * ( vector2.z * vector3.x - vector2.x * vector3.z )
        + this->z * ( vector2.x * vector3.y - vector2.y * vector3.x )
        ;
    }


/*-----------------------------------------------------------------*//**
* @description Returns the (scalar) dot product of two vectors.
* @param [in] vector2 The second vector
* @return The dot product of the two vectors
* @bsimethod                            EarlinLutz      03/03
+----------------------------------------------------------------------*/
double DVec3d::DotProduct
(

DVec3dCR vector2

) const
    {
    return (this->x * vector2.x + this->y * vector2.y + this->z * vector2.z);
    }




/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
double DVec3d::DotProductRow
(
RotMatrixCR matrix,
int         index
) const
    {
    index = Angle::Cyclic3dAxis (index);
    return  x * matrix.form3d[index][0]
          + y * matrix.form3d[index][1]
          + z * matrix.form3d[index][2];
    }




/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
double DVec3d::DotProductColumn
(
RotMatrixCR matrix,
int         index
) const
    {
    index = Angle::Cyclic3dAxis (index);
    return  x * matrix.form3d[0][index]
          + y * matrix.form3d[1][index]
          + z * matrix.form3d[2][index];
    }




/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
double DVec3d::DotProductMatrixRow
(
TransformCR matrix,
int         index
) const
    {
    index = Angle::Cyclic3dAxis (index);
    return  x * matrix.form3d[index][0]
          + y * matrix.form3d[index][1]
          + z * matrix.form3d[index][2];
    }




/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
double DVec3d::DotProductMatrixColumn
(
TransformCR matrix,
int         index
) const
    {
    index = Angle::Cyclic3dAxis (index);
    return  x * matrix.form3d[0][index]
          + y * matrix.form3d[1][index]
          + z * matrix.form3d[2][index];
    }





/*-----------------------------------------------------------------*//**
* @description Returns the (scalar) dot product of xy parts of two vectors.
* @param [in] vector2 The second vector
* @return The dot product of the xy parts of the two vectors
* @bsimethod                            EarlinLutz      03/03
+----------------------------------------------------------------------*/
double DVec3d::DotProductXY
(

DVec3dCR vector2

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
* @bsimethod                            EarlinLutz      03/03
+----------------------------------------------------------------------*/
double DVec3d::DotProduct
(

        double    ax,
        double    ay,
        double    az

) const
    {
    return this->x * ax + this->y * ay + this->z * az;
    }



/*-----------------------------------------------------------------*//**
* @description Sets three vectors so that they are mutually
* perpendicular, the third (Z) vector is identical to the
* given axis vector, and all have the same length.
* If the given axis vector contains only zeros, a (0,0,1) vector
*   is used instead.
*
* @param [out] xAxis x direction of the coordinate system
* @param [out] yAxis y direction of the coordinate system
* @param [out] zAxis z direction of the coordinate system
* @return true unless given vector is z zero vector.
* @bsimethod                            EarlinLutz      03/03
+----------------------------------------------------------------------*/
bool DVec3d::GetTriad
(

DVec3dR xAxis,
DVec3dR yAxis,
DVec3dR zAxis

) const
    {
    DVec3d      vector;
    double      length = this->Magnitude ();
    double      zTest = length / 64.0;
    bool   boolStat = true;
    zAxis = *this;

    if ( length == 0.0 )
        {
        zAxis.z = 1.0;
        length = 1.0;
        boolStat = false;
        }

    /* Pick a principle axis not too parallel to pZAxis.
       This is autocad's rule from DXF interchange book.
    */
    vector.x = vector.y = vector.z = 0.0;
    if (fabs (zAxis.x) < zTest && fabs (zAxis.y) < zTest)
        {
        vector.y = 1.0;
        }
    else
        {
        vector.z = 1.0;
        }

    xAxis.CrossProduct (vector, zAxis);
    yAxis.CrossProduct (zAxis, xAxis);
    xAxis.Normalize ();
    yAxis.Normalize ();
    zAxis.Normalize ();
    xAxis.Scale (xAxis, length);
    yAxis.Scale (yAxis, length);
    zAxis.Scale (zAxis, length);

    return  boolStat;
    }


/*-----------------------------------------------------------------*//**
* @description Sets three vectors so that they are mutually
* perpendicular unit vectors with the  third (Z) vector in the
* direction of the given axis vector.
* If the given axis vector contains only zeros, a (0,0,1) vector
*   is used instead.
*
* @param [out] xAxis unit x direction vector
* @param [out] yAxis unit y direction vector
* @param [out] zAxis unit z direction vector
* @return true unless given vector has zero length.
* @bsimethod                            EarlinLutz      03/03
+----------------------------------------------------------------------*/
bool DVec3d::GetNormalizedTriad
(

DVec3dR xAxis,
DVec3dR yAxis,
DVec3dR zAxis

) const
    {
    DVec3d      vector;
    double      length = this->Magnitude();
    double      zTest = length / 64.0;
    bool   boolStat = true;
    zAxis = *this;

    if (length == 0.0)
        {
        xAxis = DVec3d::UnitX ();
        yAxis = DVec3d::UnitY ();
        zAxis = DVec3d::UnitZ ();
        return false;
        }

    /* Pick a principle axis not too parallel to pZAxis.
       This is autocad's rule from DXF interchange book.
    */
    vector.x = vector.y = vector.z = 0.0;
    if (fabs (zAxis.x) < zTest && fabs (zAxis.y) < zTest)
        {
        vector.y = 1.0;
        }
    else
        {
        vector.z = 1.0;
        }

    xAxis.CrossProduct (vector, zAxis);
    yAxis.CrossProduct (zAxis, xAxis);
    xAxis.Normalize ();
    yAxis.Normalize ();
    zAxis.Normalize ();

    return  boolStat;
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
double DVec3d::NormalizedDifference
(

DPoint3dCR target,
DPoint3dCR origin

)
    {
    this->x = target.x - origin.x;
    this->y = target.y - origin.y;
    this->z = target.z - origin.z;
    return this->Normalize ();
    }




/*-----------------------------------------------------------------*//**
* @description Returns the angle between two vectors.  This angle is between 0 and
* pi.  Rotating the first vector by this angle around the cross product
* between the vectors aligns it with the second vector.
*
* @param [in] vector2 The second vector
* @return The angle between the vectors.
* @bsimethod                            EarlinLutz      03/03
+----------------------------------------------------------------------*/
double DVec3d::AngleTo
(
DVec3dCR vector2
) const
    {
    return GeometryTemplates::RadiansTo (*this, vector2);
    }

/*-----------------------------------------------------------------*//**
* @description Returns the angle between two vectors, choosing the smaller
*   of the two possible angles when both the vectors and their negations are considered.
*    This angle is between 0 and pi/2.
*
* @param [in] vector2 The second vector
* @return The angle between the vectors.
* @bsimethod                            EarlinLutz      03/03
+----------------------------------------------------------------------*/
double DVec3d::SmallerUnorientedAngleTo
(
DVec3dCR vector2
) const
    {
    return GeometryTemplates::SmallerUnorientedRadiansTo (*this, vector2);
    }

/*-----------------------------------------------------------------*//**
* @description Returns the angle that 2 vectors deviate from being perpendicular.
*
* @param [in] vector2 The second vector
* @return The angle away from perpendicular.
* @bsimethod                            EarlinLutz      12/14
+----------------------------------------------------------------------*/
double DVec3d::AngleFromPerpendicular
(
DVec3dCR vector2
) const
    {
    return GeometryTemplates::RadiansFromPerpendicular (*this, vector2);
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
* @bsimethod                            EarlinLutz      03/03
+----------------------------------------------------------------------*/
bool DVec3d::IsVectorInSmallerSector
(
DVec3dCR vector0,
DVec3dCR vector1
) const
    {
    DVec3d   cross01;
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
* @bsimethod                            EarlinLutz      03/03
+----------------------------------------------------------------------*/
bool DVec3d::IsVectorInCCWSector
(
DVec3dCR vector0,
DVec3dCR vector1,
DVec3dCR upVector
) const
    {
    DVec3d  cross01;
    double  dot;

    cross01.CrossProduct (vector0, vector1);

    if (cross01.IsZero ())
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
* @description Test if the test vector vector is "between" vector0 and vector1, with CCW direction
* @param [in] vector0 The boundary vector.
* @param [in] vector1 The boundary vector.
* @bsimethod                            EarlinLutz      03/03
+----------------------------------------------------------------------*/
bool DVec3d::IsVectorInCCWXYSector
(
DVec3dCR vector0,
DVec3dCR vector1
) const
    {
    double cross = vector0.CrossProductXY (vector1);
    
    if (cross == 0.0)
        {
        double dot   = vector0.DotProductXY (vector1);
        if (dot > 0.0)
            return false;
        cross = -1.0;   // treat as opposing vectors.
        }

    if (cross > 0.0)
        return  vector0.CrossProductXY (*this) > 0.0
            &&  this->CrossProductXY (vector1) > 0.0;
    else
        return  vector0.CrossProductXY (*this) > 0.0
            ||  this->CrossProductXY (vector1) > 0.0;

    }



/*-----------------------------------------------------------------*//**
* @description Returns the angle from Vector1 to Vector2 using only xy parts.
*  This angle is between -pi and +pi.
*
* @param [in] vector2 The second vector
* @return The angle between vectors.
* @bsimethod                            EarlinLutz      03/03
+----------------------------------------------------------------------*/
double DVec3d::AngleToXY (DVec3dCR vector2) const
    {
    return GeometryTemplates::RadiansToXY (*this, vector2);
    }


/*-----------------------------------------------------------------*//**
* @description Returns the angle between two vectors, considering both
*   the vectors and their negations and choosing the smaller.
*   This angle is between 0 and pi/2.
*
* @param [in] vector2 The second vector
* @return The angle between vectors.
* @bsimethod                            EarlinLutz      03/03
+----------------------------------------------------------------------*/
double DVec3d::SmallerUnorientedAngleToXY (DVec3dCR vector2) const
    {
    return GeometryTemplates::SmallerUnorientedRadiansToXY (*this, vector2);
    }


/*-----------------------------------------------------------------*//**
* @description Rotate a vector around the z axis.
* @param [in] theta The rotation angle.
* @bsimethod                            EarlinLutz      03/03
+----------------------------------------------------------------------*/
void DVec3d::RotateXY
(
DVec3dCR vector,
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
    this->z = vector.z;
    }


/*-----------------------------------------------------------------*//**
* @description Rotate a vector around the z axis.
* @param [in] theta The rotation angle.
* @bsimethod                            EarlinLutz      03/03
+----------------------------------------------------------------------*/
void DVec3d::RotateXY
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
* @bsimethod                            EarlinLutz      03/03
+----------------------------------------------------------------------*/
double DVec3d::SignedAngleTo
(
DVec3dCR vector2,
DVec3dCR orientationVector
) const
    {
    return GeometryTemplates::SignedRadiansTo (*this, vector2, orientationVector);
    }


/*-----------------------------------------------------------------*//**
* @description Computes the signed angle between the projection of two vectors
*       onto a plane with given normal.
*
* @param [in] vector2 The second vector
* @param [in] planeNormal The plane normal vector
* @return The angle in plane
* @bsimethod                            EarlinLutz      03/03
+----------------------------------------------------------------------*/
double DVec3d::PlanarAngleTo
(
DVec3dCR vector2,
DVec3dCR planeNormal
) const
    {
    return GeometryTemplates::PlanarRadiansTo (*this, vector2, planeNormal);
    }


/*-----------------------------------------------------------------*//**
* @description Scale each (homogeneous) point by the other's weight and subtract, i.e. form
* (point1 * point2.w - point2 * point1.w).  The weight term
* vanishes.   Copy the xyz parts back as a vector.
*
* @param [in] hPoint1 The first homogeneous point
* @param [in] hPoint2 The second homogeneous point.
* @bsimethod                            EarlinLutz      03/03
+----------------------------------------------------------------------*/
void DVec3d::WeightedDifferenceOf
(
DPoint4dCR hPoint1,
DPoint4dCR hPoint2
)
    {
    double w2 = hPoint2.w;
    double w1 = hPoint1.w;
    this->x = hPoint1.x * w2 - hPoint2.x * w1;
    this->y = hPoint1.y * w2 - hPoint2.y * w1;
    this->z = hPoint1.z * w2 - hPoint2.z * w1;
    }

/*-----------------------------------------------------------------*//**
* @description Scale each (homogeneous) point by the other's weight and subtract, i.e. form
* (point1 * point2.w - point2 * point1.w).  The weight term
* vanishes.   Copy the xyz parts back as a vector.
*
* @param [in] hPoint1 The first homogeneous point
* @param [in] hPoint2 The second homogeneous point.
* @bsimethod                            EarlinLutz      03/03
+----------------------------------------------------------------------*/
DVec3d DVec3d::FromWeightedDifferenceOf
(
DPoint4dCR hPoint1,
DPoint3dCR point2
)
    {
    double w1 = hPoint1.w;
    return DVec3d::From (
          hPoint1.x - point2.x * w1,
          hPoint1.y - point2.y * w1,
          hPoint1.z - point2.z * w1
          );
    }


/*-----------------------------------------------------------------*//**
* @description Form the cross product of the weighted differences from base poitn
    to two targets
*
* @param [in] basePoint The common base point (second point for differences)
* @param [in] target1 The first target point.
* @param [in] target2 The second target point.
* @bsimethod                            EarlinLutz      03/03
+----------------------------------------------------------------------*/
void DVec3d::WeightedDifferenceCrossProduct
(
DPoint4dCR basePoint,
DPoint4dCR target1,
DPoint4dCR target2
)
    {
    DVec3d   U, V;
    U.WeightedDifferenceOf (target1, basePoint);
    V.WeightedDifferenceOf (target2, basePoint);
    this->CrossProduct (U, V);
    }


/*-----------------------------------------------------------------*//**
* @description Computes the squared magnitude of a vector.
*
* @return The squared magnitude of the vector.
* @bsimethod                            EarlinLutz      03/03
+----------------------------------------------------------------------*/
double DVec3d::MagnitudeSquared () const
    {
    return   this->x * this->x
           + this->y * this->y
           + this->z * this->z;
    }

/*-----------------------------------------------------------------*//**
* @bsimethod                            EarlinLutz      11/12
+----------------------------------------------------------------------*/
double DVec3d::SafeOneOverMagnitudeSquared (double defaultValue) const
    {
    double a;
    DoubleOps::SafeDivideDistanceSquared (a, 1.0, MagnitudeSquared (), defaultValue);
    return a;
    }

ValidatedDouble DVec3d::ValidatedFractionOfProjection (DVec3dCR vectorToProject, double defaultValue) const
    {
    return DoubleOps::ValidatedDivideParameter(this->DotProduct(vectorToProject), this->DotProduct(*this), defaultValue);
    }


/*-----------------------------------------------------------------*//**
* @description Computes the magnitude of the xy part of a vector.
* @return The magnitude of the xy parts of the given vector.
* @bsimethod                            EarlinLutz      03/03
+----------------------------------------------------------------------*/
double DVec3d::MagnitudeXY () const
    {
    return sqrt (this->x * this->x + this->y * this->y);
    }


/*-----------------------------------------------------------------*//**
* @description Computes the squared magnitude of the xy part of a vector.
* @return The squared magnitude of the xy parts of the given vector.
* @bsimethod                            EarlinLutz      03/03
+----------------------------------------------------------------------*/
double DVec3d::MagnitudeSquaredXY () const
    {
    return this->x * this->x + this->y * this->y;
    }


/*-----------------------------------------------------------------*//**
* @description Compute a unit vector perpendicular to the xy parts of given vector.
* @param [in] vector The source vector
* @return true if the input vector has nonzero length
* @bsimethod                            EarlinLutz      03/03
+----------------------------------------------------------------------*/
bool DVec3d::UnitPerpendicularXY
(
DVec3dCR vector
)
    {
    double  a, d2, xx = vector.x, yy = vector.y;
    this->x = -yy;
    this->y =  xx;
    this->z = 0.0;
    d2 = xx * xx + yy * yy;
    if (d2 == 0.0)
        return false;
    a = 1.0 / sqrt (d2);
    this->x *= a;
    this->y *= a;
    return true;
    }

/*-----------------------------------------------------------------*//**
* @bsimethod                            EarlinLutz      11/15
+----------------------------------------------------------------------*/
ValidatedDVec3d DVec3d::FromUnitPerpendicularXY (DVec3dCR vector)
    {
    DVec3d unit;
    bool stat = unit.UnitPerpendicularXY (vector);
    return ValidatedDVec3d (unit, stat);
    }

/*-----------------------------------------------------------------*//**
* @description Computes the magnitude of a vector.
* @return The length of the vector
* @bsimethod                            EarlinLutz      03/03
+----------------------------------------------------------------------*/
double DVec3d::Magnitude () const
    {
    return  sqrt ( this->x * this->x
                 + this->y * this->y
                 + this->z * this->z);
    }


/*-----------------------------------------------------------------*//**
* @description Multiplies a vector by a scale factor.
* @param [in] vector The vector to be scaled.
* @param [in] scale The scale factor.
* @bsimethod                            EarlinLutz      03/03
+----------------------------------------------------------------------*/
void DVec3d::Scale
(
DVec3dCR vector,
        double       scale
)
    {
    this->x = vector.x * scale;
    this->y = vector.y * scale;
    this->z = vector.z * scale;
    }


/*-----------------------------------------------------------------*//**
* @description Multiplies a vector (in place) by a scale factor.
* @param [in] scale The scale
* @bsimethod                            EarlinLutz      03/03
+----------------------------------------------------------------------*/
void DVec3d::Scale (double scale)
    {
    this->x *= scale;
    this->y *= scale;
    this->z *= scale;
    }


/*-----------------------------------------------------------------*//**
* @description Computes a negated (opposite) vector.
*
* @param [in] vector The vector to be negated.
* @bsimethod                            EarlinLutz      03/03
+----------------------------------------------------------------------*/
void DVec3d::Negate (DVec3dCR vector)
    {
    this->x = - vector.x;
    this->y = - vector.y;
    this->z = - vector.z;
    }


/*-----------------------------------------------------------------*//**
* @description Negate a vector in place.
*
* @bsimethod                            EarlinLutz      03/03
+----------------------------------------------------------------------*/
void DVec3d::Negate ()
    {
    this->x = - this->x;
    this->y = - this->y;
    this->z = - this->z;
    }


/*-----------------------------------------------------------------*//**
* @description Normalizes (scales) a vector to length 1.
* If the input vector length is 0, the output vector is a zero vector
* and the returned length is 0.
*
* @param [in] vector The vector to be normalized.
* @return The length prior to normalization
* @bsimethod                            EarlinLutz      03/03
+----------------------------------------------------------------------*/
double DVec3d::Normalize (DVec3dCR vector)
    {
    *this = vector;
    return Normalize ();
    }


/*-----------------------------------------------------------------*//**
* @description Scales a vector to specified length.
* If the input vector length is 0, the output vector is a zero vector
* and the returned length is 0.
*
* @param [in] vector The original vector.
* @param [in] length The requested length.
* @return The length prior to scaling.
* @bsimethod                            EarlinLutz      03/03
+----------------------------------------------------------------------*/
double DVec3d::ScaleToLength (DVec3dCR vector, double    length)
    {
    double magnitude = Normalize (vector);
    Scale (length);
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
* @bsimethod                            EarlinLutz      03/03
+----------------------------------------------------------------------*/
double DVec3d::ScaleToLength (double length)
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
* @bsimethod                            EarlinLutz      03/03
+----------------------------------------------------------------------*/
double DVec3d::Normalize ()
    {
    double  magnitude =
            sqrt ( this->x * this->x
                 + this->y * this->y
                 + this->z * this->z);

    if (magnitude > 0.0)
        {
        double f = 1.0 / magnitude;
        this->x *= f;
        this->y *= f;
        this->z *= f;
        }
    else
        {
        this->x = 1.0;
        this->y = 0.0;
        this->z = 0.0;
        magnitude = 0.0;
        }
    return  magnitude;
    }


/*-----------------------------------------------------------------*//**
* @bsimethod                            EarlinLutz      07/14
+----------------------------------------------------------------------*/
bool DVec3d::TryNormalize (DVec3dCR other, double &magnitude)
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
        this->z = 0.0;
        magnitude = 0.0;
        return false;
        }
    }


/*-----------------------------------------------------------------*//**
* @description Tests if two vectors are parallel.
*
* @param [in] vector2 The second vector
* @return true if the vectors are parallel within tolerance
* @bsimethod                            EarlinLutz      03/03
+----------------------------------------------------------------------*/
bool DVec3d::IsParallelTo
(
DVec3dCR vector2
) const
    {
    DVec3d      vecC;
    double      a2 = this->DotProduct (*this);
    double      b2 = vector2.DotProduct (vector2);
    double      cross;
    double      eps = Angle::SmallAngle(); /* small angle tolerance (in radians) */
    vecC.CrossProduct (*this, vector2);
    cross = vecC.MagnitudeSquared ();

    /* a2,b2,c2 are squared lengths of respective vectors */
    /* c2 = sin^2(theta) * a2 * b2 */
    /* For small theta, sin^2(theta)~~theta^2 */
    /* Zero-length vector case falls through as equality */

    return  cross <= eps * eps * a2 * b2;
    }
/*-----------------------------------------------------------------*//**
* @description Tests if two vectors are parallel.
*
* @param [in] vector2 The second vector
* @return true if the vectors are parallel within tolerance
* @bsimethod                            EarlinLutz      03/03
+----------------------------------------------------------------------*/
bool DVec3d::IsParallelTo
(
DVec3dCR vector2,
double radians
) const
    {
    DVec3d      vecC;
    double      a2 = this->DotProduct (*this);
    double      b2 = vector2.DotProduct (vector2);
    double      cross;
    vecC.CrossProduct (*this, vector2);
    cross = vecC.MagnitudeSquared ();

    /* a2,b2,c2 are squared lengths of respective vectors */
    /* c2 = sin^2(theta) * a2 * b2 */
    /* For small theta, sin^2(theta)~~theta^2 */
    /* Zero-length vector case falls through as equality */

    return  cross <= radians * radians * a2 * b2;
    }

/*-----------------------------------------------------------------*//**
* @description Tests if two vectors are parallel and positive dot product
*
* @param [in] vector2 The second vector
* @return true if the vectors are parallel within tolerance
* @bsimethod                            EarlinLutz      03/03
+----------------------------------------------------------------------*/
bool DVec3d::IsPositiveParallelTo
(
DVec3dCR vector2
) const
    {
    return IsParallelTo (vector2) && this->DotProduct (vector2) > 0.0;
    }


/*-----------------------------------------------------------------*//**
* @description Tests if two vectors are perpendicular.
*
* @param [in] vector2 The second vector
* @return true if vectors are perpendicular within tolerance
* @bsimethod                            EarlinLutz      03/03
+----------------------------------------------------------------------*/
bool DVec3d::IsPerpendicularTo
(

DVec3dCR vector2

) const
    {
    double e = Angle::SmallAngle ();
    return GeometryTemplates::IsPerpendicularTo (*this, vector2, e);
    }


/*-----------------------------------------------------------------*//**
* @description Try to divide each component of a vector by a scalar.  If the denominator
* near zero compared to any numerator component, return the original
* vector.
* @param [out] pScaledVector The vector after scaling.
* @param [in] vector The initial vector.
* @param [in] denominator The divisor.
* @return true if division is numerically safe.
* @bsimethod                            EarlinLutz      03/03
+----------------------------------------------------------------------*/
bool DVec3d::SafeDivide
(
DVec3dCR vector,
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
        this->z = vector.z * a;
        return true;
        }
    }


/*-----------------------------------------------------------------*//**
* @description Copies doubles from a 3 component array to the x,y, and z components
* of a DVec3d
*
* @param [in] pXyz x, y, z components
* @bsimethod                            EarlinLutz      03/03
+----------------------------------------------------------------------*/
void DVec3d::InitFromArray
(
const   double      *pXyz
)
    {
    this->x = pXyz[0];
    this->y = pXyz[1];
    this->z = pXyz[2];
    }


/*-----------------------------------------------------------------*//**
* @description Sets the x,y, and z components of a vector
*
* @param [in] ax The x component.
* @param [in] ay The y component.
* @param [in] az The z component.
* @bsimethod                            EarlinLutz      03/03
+----------------------------------------------------------------------*/
void DVec3d::Init
(
double       ax,
double       ay,
double       az
)
    {
    this->x = ax;
    this->y = ay;
    this->z = az;
    }


/*-----------------------------------------------------------------*//**
* @description Initialize a vector from a point (treating the point
            as a vector from its origin).
* @param [in] point the point.
* @bsimethod                            EarlinLutz      03/03
+----------------------------------------------------------------------*/
void DVec3d::Init
(
DPoint3dCR point
)
    {
    this->x = point.x;
    this->y = point.y;
    this->z = point.z;
    }


/*-----------------------------------------------------------------*//**
* @description Sets the x, and y components of a vector. Sets z to zero.
*
* @param [in] ax The x component.
* @param [in] ax The x component.
* @param [in] ay The y component
* @bsimethod                            EarlinLutz      03/03
+----------------------------------------------------------------------*/
void DVec3d::Init
(
double       ax,
double       ay
)
    {
    this->x = ax;
    this->y = ay;
    this->z = 0.0;
    }


/*-----------------------------------------------------------------*//**
* @description Sets a vector from given angle and distance in xy plane.
*       Z part is set to zero.
*
* @param [in] theta Angle from X axis to the vector, in the xy plane.
* @param [in] magnitude Vector magnitude
* @bsimethod                            EarlinLutz      03/03
+----------------------------------------------------------------------*/
void DVec3d::InitFromXYAngleAndMagnitude
(
double       theta,
double       magnitude
)
    {
    this->x = magnitude * cos (theta);
    this->y = magnitude * sin (theta);
    this->z = 0.0;
    }


/*-----------------------------------------------------------------*//**
* @description Sets the x,y, and z components of a DVec3d structure from the
* corresponding parts of a DPoint4d.  Weight part of DPoint4d is not used.
*
* @param [in] hPoint The homogeneous point
* @bsimethod                            EarlinLutz      03/03
+----------------------------------------------------------------------*/
void DVec3d::XyzOf (DPoint4dCR hPoint)
    {
    this->x = hPoint.x;
    this->y = hPoint.y;
    this->z = hPoint.z;
    }


/*-----------------------------------------------------------------*//**
* @description Set one of three components (x,y,z) of the vector.
*
* @param [in] a The component value.
* @param [in] index Selects the the axis: 0=x, 1=y, 2=z, others cyclic.
* @bsimethod                            EarlinLutz      03/03
+----------------------------------------------------------------------*/
void DVec3d::SetComponent
(
double       a,
int         index
)
    {
    index = Angle::Cyclic3dAxis (index);
    if (index == 0)
        this->x = a;
    else if (index == 1)
        this->y = a;
    else if (index == 2)
        this->z = a;
    }


/*-----------------------------------------------------------------*//**
* @description Gets a single component of a vector.  If the index is out of
* range 0,1,2, it is interpreted cyclically.
*
* @param [in] index Indicates which component is accessed.  The values
*                       are 0=x, 1=y, 2=z.  Other values are treated cyclically.
* @return The specified component of the vector.
* @bsimethod                            EarlinLutz      03/03
+----------------------------------------------------------------------*/
double DVec3d::GetComponent
(

      int         index

) const
    {
    index = Angle::Cyclic3dAxis (index);
    if (index == 0)
        return this->x;
    else if (index == 1)
        return this->y;
    else
        return this->z;
    }


/*-----------------------------------------------------------------*//**
* @description Copies x,y,z components from a vector to individual variables.
*
* @param [out] xCoord x component
* @param [out] yCoord y component
* @param [out] zCoord z component
* @bsimethod                            EarlinLutz      03/03
+----------------------------------------------------------------------*/
void DVec3d::GetComponents
(
double      &xCoord,
double      &yCoord,
double      &zCoord
) const
    {
    xCoord = this->x;
    yCoord = this->y;
    zCoord = this->z;
    }


/*-----------------------------------------------------------------*//**
* @description Compute the sum of two vectors or vectors.
*
* @param [in] vector1 The the first vector
* @param [in] vector2 The second vector
* @bsimethod                            EarlinLutz      03/03
+----------------------------------------------------------------------*/
void DVec3d::SumOf
(
DVec3dCR vector1,
DVec3dCR vector2
)
    {
    this->x = vector1.x + vector2.x;
    this->y = vector1.y + vector2.y;
    this->z = vector1.z + vector2.z;
    }



/*-----------------------------------------------------------------*//**
* @description two scaled vectors.
*
* @param [in] vector1 The first vector
* @param [in] scale1 The first scale factor
* @param [in] vector2 The second vector
* @param [in] scale2 The second scale factor
* @bsimethod                            EarlinLutz      03/03
+----------------------------------------------------------------------*/
void DVec3d::SumOf
(
DVec3dCR vector1,
double   scale1,
DVec3dCR vector2,
double   scale2
)
    {
    this->x = vector1.x * scale1 + vector2.x * scale2;
    this->y = vector1.y * scale1 + vector2.y * scale2;
    this->z = vector1.z * scale1 + vector2.z * scale2;
    }



/*-----------------------------------------------------------------*//**
* @description Adds a vector to a pointer or vector, returns the result in place.
*
* @param [in] vector The vector to add.
* @bsimethod                            EarlinLutz      03/03
+----------------------------------------------------------------------*/
void DVec3d::Add
(

DVec3dCR vector

)
    {
    this->x += vector.x;
    this->y += vector.y;
    this->z += vector.z;
    }


/*-----------------------------------------------------------------*//**
* @description Subtract two vectors, and return the result in
*           place of the first.
*
* @param [in] vector2 The vector to subtract.
* @bsimethod                            EarlinLutz      03/03
+----------------------------------------------------------------------*/
void DVec3d::Subtract
(

DVec3dCR vector2

)
    {
    this->x -= vector2.x;
    this->y -= vector2.y;
    this->z -= vector2.z;
    }


/*-----------------------------------------------------------------*//**
* @description Adds an origin and a scaled vector.
*
* @param [in] origin Origin for the sum.
* @param [in] vector The vector to be added.
* @param [in] scale The scale factor.
* @bsimethod                            EarlinLutz      03/03
+----------------------------------------------------------------------*/
void DVec3d::SumOf
(

DVec3dCR origin,
DVec3dCR vector,
        double           scale

)
    {
    this->x = origin.x + vector.x * scale;
    this->y = origin.y + vector.y * scale;
    this->z = origin.z + vector.z * scale;
    }


/*-----------------------------------------------------------------*//**
* @description Computes a vector whose position is given by a fractional
* argument and two vectors.
*
* @param [in] vector0 The vector corresponding to fractionParameter of 0.
* @param [in] fractionParameter The fractional parametric coordinate.
*               0.0 is the start of the segment, 1.0 is the end, 0.5 is middle
* @param [in] vector1 The vector corresponding to fractionParameter of 1.
* @bsimethod                            EarlinLutz      03/03
+----------------------------------------------------------------------*/
void DVec3d::Interpolate
(

DVec3dCR vector0,
        double       fractionParameter,
DVec3dCR vector1

)
    {
    this->x = vector0.x + fractionParameter * (vector1.x - vector0.x);
    this->y = vector0.y + fractionParameter * (vector1.y - vector0.y);
    this->z = vector0.z + fractionParameter * (vector1.z - vector0.z);
    }





/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
DVec3d DVec3d::FromInterpolate
(
DVec3dCR vector0,
double   fractionParameter,
DVec3dCR vector1
)
    {
    DVec3d result;
    result.x = vector0.x + fractionParameter * (vector1.x - vector0.x);
    result.y = vector0.y + fractionParameter * (vector1.y - vector0.y);
    result.z = vector0.z + fractionParameter * (vector1.z - vector0.z);
    return result;
    }




/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
DVec3d DVec3d::FromSumOf
(
DVec3dCR vector0,
double   scale0,
DVec3dCR vector1,
double   scale1
)
    {
    DVec3d result;
    result.x = vector0.x * scale0 + vector1.x * scale1;
    result.y = vector0.y * scale0 + vector1.y * scale1;
    result.z = vector0.z * scale0 + vector1.z * scale1;
    return result;
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
DVec3d DVec3d::FromSumOf
(
DVec3dCR vector0,
DVec3dCR vector1,
double   scale1
)
    {
    DVec3d result;
    result.x = vector0.x + vector1.x * scale1;
    result.y = vector0.y + vector1.y * scale1;
    result.z = vector0.z + vector1.z * scale1;
    return result;
    }



/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
DVec3d DVec3d::FromSumOf
(
DVec3dCR vector0,
DVec3dCR vector1
)
    {
    DVec3d result;
    result.x = vector0.x + vector1.x;
    result.y = vector0.y + vector1.y;
    result.z = vector0.z + vector1.z;
    return result;
    }



/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
DVec3d DVec3d::FromSumOf
(
DVec3dCR vector0,
double   scale0,
DVec3dCR vector1,
double   scale1,
DVec3dCR vector2,
double   scale2
)
    {
    DVec3d result;
    result.x = vector0.x * scale0 + vector1.x * scale1 + vector2.x * scale2;
    result.y = vector0.y * scale0 + vector1.y * scale1 + vector2.y * scale2;
    result.z = vector0.z * scale0 + vector1.z * scale1 + vector2.z * scale2;
    return result;
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
DVec3d DVec3d::FromSumOf
(
DVec3dCR vector0,
DVec3dCR vector1,
double   scale1,
DVec3dCR vector2,
double   scale2
)
    {
    DVec3d result;
    result.x = vector0.x + vector1.x * scale1 + vector2.x * scale2;
    result.y = vector0.y + vector1.y * scale1 + vector2.y * scale2;
    result.z = vector0.z + vector1.z * scale1 + vector2.z * scale2;
    return result;
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
DVec3d DVec3d::FromSumOf
(
DVec3dCR vector0,
DVec3dCR vector1,
double   scale1,
DVec3dCR vector2,
double   scale2,
DVec3dCR vector3,
double   scale3
)
    {
    DVec3d result;
    result.x = vector0.x + vector1.x * scale1 + vector2.x * scale2 + vector3.x * scale3;
    result.y = vector0.y + vector1.y * scale1 + vector2.y * scale2 + vector3.y * scale3;
    result.z = vector0.z + vector1.z * scale1 + vector2.z * scale2 + vector3.z * scale3;
    return result;
    }



/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
DVec3d DVec3d::FromInterpolateBilinear (DVec3dCR data00, DVec3dCR data10, DVec3dCR data01, DVec3dCR data11, double u, double v)
    {
    double a00 = (1.0 - u) * (1.0 - v);
    double a10 = u * (1.0 - v);
    double a01 = (1.0 - u) * v;
    double a11 = u * v;
    DVec3d result;
    result.x = a00 * data00.x + a10 * data10.x + a01 * data01.x + a11 * data11.x;
    result.y = a00 * data00.y + a10 * data10.y + a01 * data01.y + a11 * data11.y;
    result.z = a00 * data00.z + a10 * data10.z + a01 * data01.z + a11 * data11.z;
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
* @bsimethod                            EarlinLutz      03/03
+----------------------------------------------------------------------*/
void DVec3d::SumOf
(

DVec3dCR origin,
DVec3dCR vector1,
        double           scale1,
DVec3dCR vector2,
        double           scale2

)
    {
    this->x = origin.x + vector1.x * scale1 + vector2.x * scale2;
    this->y = origin.y + vector1.y * scale1 + vector2.y * scale2;
    this->z = origin.z + vector1.z * scale1 + vector2.z * scale2;
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
* @bsimethod                            EarlinLutz      03/03
+----------------------------------------------------------------------*/
void DVec3d::SumOf
(

DVec3dCR origin,
DVec3dCR vector1,
        double          scale1,
DVec3dCR vector2,
        double          scale2,
DVec3dCR vector3,
        double          scale3

)
    {
    this->x = origin.x + vector1.x * scale1 + vector2.x * scale2 + vector3.x * scale3;
    this->y = origin.y + vector1.y * scale1 + vector2.y * scale2 + vector3.y * scale3;
    this->z = origin.z + vector1.z * scale1 + vector2.z * scale2 + vector3.z * scale3;
    }




/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
void DVec3d::SumOf
(
DVec3dCR vector1,
        double          scale1,
DVec3dCR vector2,
        double          scale2,
DVec3dCR vector3,
        double          scale3
)
    {
    x = vector1.x * scale1 + vector2.x * scale2 + vector3.x * scale3;
    y = vector1.y * scale1 + vector2.y * scale2 + vector3.y * scale3;
    z = vector1.z * scale1 + vector2.z * scale2 + vector3.z * scale3;
    }




/*-----------------------------------------------------------------*//**
* @description Subtract coordinates of two vectors. (Compute Vector1 - Vector2)
*
* @param [in] vector1 The first vector
* @param [in] vector2 The second (subtracted) vector
* @bsimethod                            EarlinLutz      03/03
+----------------------------------------------------------------------*/
void DVec3d::DifferenceOf
(

DVec3dCR vector1,
DVec3dCR vector2

)
    {
    this->x = vector1.x - vector2.x;
    this->y = vector1.y - vector2.y;
    this->z = vector1.z - vector2.z;
    }


/*-----------------------------------------------------------------*//**
* @description Subtract coordinates of two points. (Compute Point1 - Point2)
*
* @param [in] target The target point
* @param [in] base The base point
* @bsimethod                            EarlinLutz      03/03
+----------------------------------------------------------------------*/
void DVec3d::DifferenceOf
(

DPoint3dCR target,
DPoint3dCR base

)
    {
    this->x = target.x - base.x;
    this->y = target.y - base.y;
    this->z = target.z - base.z;
    }

/*-----------------------------------------------------------------*//**
* @description Subtract coordinates of two points. (Compute Point1 - Point2)
*
* @param [in] target The target point
* @param [in] base The base point
* @bsimethod                            EarlinLutz      03/03
+----------------------------------------------------------------------*/
void DVec3d::DifferenceOf
(

FPoint3dCR target,
FPoint3dCR base

)
    {
    this->x = target.x - base.x;
    this->y = target.y - base.y;
    this->z = target.z - base.z;
    }

/*-----------------------------------------------------------------*//**
* @description Computes the (cartesian) distance between two vectors
*
* @param [in] vector2 The second vector
* @return The distance between vector.
* @bsimethod                            EarlinLutz      03/03
+----------------------------------------------------------------------*/
double DVec3d::Distance
(

DVec3dCR vector2

) const
    {
    double      xdist, ydist, zdist;

    xdist = vector2.x - this->x;
    ydist = vector2.y - this->y;
    zdist = vector2.z - this->z;

    return (sqrt (xdist * xdist + ydist * ydist + zdist * zdist));
    }


/*-----------------------------------------------------------------*//**
* @description Computes the squared distance between two vectors.
*
* @param [in] vector2 The second vector.
* @return The squared distance between the vectors.
* @bsimethod                            EarlinLutz      03/03
+----------------------------------------------------------------------*/
double DVec3d::DistanceSquared
(

DVec3dCR vector2

) const
    {
    double      xdist, ydist, zdist;

    xdist = (vector2.x - this->x);
    ydist = (vector2.y - this->y);
    zdist = (vector2.z - this->z);

    return (xdist * xdist + ydist * ydist + zdist * zdist);
    }


/*-----------------------------------------------------------------*//**
* @description Computes the squared distance between two vectors, using only the
*       xy parts.
*
* @param [in] vector2 The second vector
* @return The squared distance between the XY projections of the two vectors.
*               (i.e. any z difference is ignored)
* @bsimethod                            EarlinLutz      03/03
+----------------------------------------------------------------------*/
double DVec3d::DistanceSquaredXY
(

DVec3dCR vector2

) const
    {
    double      xdist, ydist;

    xdist = vector2.x - this->x;
    ydist = vector2.y - this->y;

    return (xdist * xdist + ydist * ydist);
    }


/*-----------------------------------------------------------------*//**
* @description Computes the distance between two vectors, using
*   only x and y components.
*
* @param [in] vector2 The second vector
* @return The distance between the XY projections of the two vectors.
*               (i.e. any z difference is ignored)
* @bsimethod                            EarlinLutz      03/03
+----------------------------------------------------------------------*/
double DVec3d::DistanceXY
(

DVec3dCR vector2

) const
    {
    double      xdist, ydist;

    xdist = vector2.x - this->x;
    ydist = vector2.y - this->y;

    return sqrt (xdist * xdist + ydist * ydist);
    }


/*-----------------------------------------------------------------*//**
* @description Finds the largest absolute value among the components of a vector.
* @return The largest absolute value among vector coordinates.
* @bsimethod                            EarlinLutz      03/03
+----------------------------------------------------------------------*/
double DVec3d::MaxAbs
(

) const
    {
    double maxVal = fabs (this->x);

    if (fabs (this->y) > maxVal)
        maxVal = fabs (this->y);

    if (fabs (this->z) > maxVal)
        maxVal = fabs (this->z);

    return maxVal;
    }


/*-----------------------------------------------------------------*//**
* @description Test for exact equality between all components of two vectors.
* @param [in] vector2 The second vector
* @return true if the vectors are identical.
* @bsimethod                            EarlinLutz      03/03
+----------------------------------------------------------------------*/
bool DVec3d::IsEqual
(

DVec3dCR vector2

) const
    {
    bool result;

    result =     this->x == vector2.x
              && this->y == vector2.y
              && this->z == vector2.z;
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
* @bsimethod                            EarlinLutz      03/03
+----------------------------------------------------------------------*/
bool DVec3d::IsEqual
(

DVec3dCR vector2,
double                  tolerance

) const
    {
    bool result;

    result =     fabs (this->x - vector2.x) <= tolerance 
              && fabs (this->y - vector2.y) <= tolerance 
              && fabs (this->z - vector2.z) <= tolerance;
    
    return  result;
    }


/*-----------------------------------------------------------------*//**
* @description Returns the product of a matrix times a vector.
* @param [out] pResult result of the multiplication.
* @param [in] matrix The matrix.
* @param [in] vector The known vector.
* @indexVerb multiply
* @bsimethod                                                    EarlinLutz      12/97
+----------------------------------------------------------------------*/
void DVec3d::Multiply
(

RotMatrixCR matrix,
DVec3dCR vector

)
    {
    DVec3d    inVec;

    inVec = vector;

    this->x =     matrix.form3d[0][0] * inVec.x
                + matrix.form3d[0][1] * inVec.y
                + matrix.form3d[0][2] * inVec.z;

    this->y =     matrix.form3d[1][0] * inVec.x
                + matrix.form3d[1][1] * inVec.y
                + matrix.form3d[1][2] * inVec.z;

    this->z =     matrix.form3d[2][0] * inVec.x
                + matrix.form3d[2][1] * inVec.y
                + matrix.form3d[2][2] * inVec.z;
    }


/*-----------------------------------------------------------------*//**
* @description Returns the product of a matrix transpose times a vector.
* @param [out] pResult result of the multiplication.
* @param [in] matrix The the matrix.
* @param [in] vector The known vector.
* @indexVerb multiply
* @bsimethod                                                    EarlinLutz      12/97
+----------------------------------------------------------------------*/
void DVec3d::MultiplyTranspose
(

RotMatrixCR matrix,
DVec3dCR vector

)
    {
    DVec3d    inVec;

    inVec = vector;

    this->x =     matrix.form3d[0][0] * inVec.x
                + matrix.form3d[1][0] * inVec.y
                + matrix.form3d[2][0] * inVec.z;

    this->y =     matrix.form3d[0][1] * inVec.x
                + matrix.form3d[1][1] * inVec.y
                + matrix.form3d[2][1] * inVec.z;

    this->z =     matrix.form3d[0][2] * inVec.x
                + matrix.form3d[1][2] * inVec.y
                + matrix.form3d[2][2] * inVec.z;
    }


/*-----------------------------------------------------------------*//**
* @description Returns the product of a matrix times a vector,
*           with the vector given as separate components.
*
* @param [out] pResult result of multiplication
* @param [in] matrix The matrix to apply
* @param [in] x The x component of input vector
* @param [in] y The y component of input vector
* @param [in] z The z component of input vector
* @indexVerb multiply
* @bsimethod                                                    EarlinLutz      12/97
+----------------------------------------------------------------------*/
void DVec3d::Multiply
(

RotMatrixCR matrix,
      double         x,
      double         y,
      double         z

)
    {
    this->x =     matrix.form3d[0][0] * x
                + matrix.form3d[0][1] * y
                + matrix.form3d[0][2] * z;

    this->y =     matrix.form3d[1][0] * x
                + matrix.form3d[1][1] * y
                + matrix.form3d[1][2] * z;

    this->z =     matrix.form3d[2][0] * x
                + matrix.form3d[2][1] * y
                + matrix.form3d[2][2] * z;
    }


/*-----------------------------------------------------------------*//**
* @description Returns the product P = [x,y,z]*M where M is the input matrix and P is
*           the product vector.
* @param [out] pVector product vector
* @param [in] matrix The matrix to apply
* @param [in] x The x component
* @param [in] y The y component
* @param [in] z The z component
* @indexVerb multiply
* @bsimethod                                                    EarlinLutz      12/97
+----------------------------------------------------------------------*/
void DVec3d::MultiplyTranspose
(

RotMatrixCR matrix,
      double        x,
      double        y,
      double        z

)
    {
    this->x =     matrix.form3d[0][0] * x
                + matrix.form3d[1][0] * y
                + matrix.form3d[2][0] * z;

    this->y =     matrix.form3d[0][1] * x
                + matrix.form3d[1][1] * y
                + matrix.form3d[2][1] * z;

    this->z =     matrix.form3d[0][2] * x
                + matrix.form3d[1][2] * y
                + matrix.form3d[2][2] * z;
    }


/*-----------------------------------------------------------------*//**
* @description Extracts a column of a matrix.
* @param [out] pVector the vector
* @param [in] matrix The matrix.
* @param [in] col column index. Columns are numbered 0,1,2.  Others
        indices are reduced cyclically.
* @indexVerb multiply
* @bsimethod                                    EarlinLutz      04/03
+----------------------------------------------------------------------*/
void DVec3d::InitFromColumn
(

RotMatrixCR matrix,
      int         col

)
    {
    col = Angle::Cyclic3dAxis (col);
    this->x = matrix.form3d[0][col];
    this->y = matrix.form3d[1][col];
    this->z = matrix.form3d[2][col];
    }


/*-----------------------------------------------------------------*//**
* @description Extracts a row of a matrix.
* @param [out] pVector the vector
* @param [in] matrix The matrix.
* @param [in] row row index. Rows are numbered 0,1,2.  Others
        indices are reduced cyclically.
* @indexVerb multiply
* @bsimethod                                       EarlinLutz   04/03
+----------------------------------------------------------------------*/
void DVec3d::InitFromRow
(
RotMatrixCR matrix,
      int            row
)
    {
    row = Angle::Cyclic3dAxis (row);

    this->x = matrix.form3d[row][0];
    this->y = matrix.form3d[row][1];
    this->z = matrix.form3d[row][2];
    }


//! Compute an axis and angle to rotate from the instance vector to a target.
//! @param [out] target direction that the instance is to rotate towards.
//! @param [out] axis   returned axis of rotation
//! @param [out] radians returned rotation angle
bool DVec3d::AngleAndAxisOfRotationFromVectorToVector
(
DVec3dCR target,
DVec3dR axis,
double &radians
) const
    {
    double a0, a1;
    DVec3d unitStartVector, unitEndVector;
    if (!unitStartVector.TryNormalize (*this, a0)
        || !unitEndVector.TryNormalize (target, a1)
        )
        {
        radians = 0.0;
        axis.Init (0,0,0);        
        return false;
        }
            
    DVec3d crossProduct;
    double cosine = unitStartVector.DotProduct (unitEndVector);
    crossProduct.CrossProduct (unitStartVector, unitEndVector);

    double sine;
    if (axis.TryNormalize (crossProduct, sine))
        {
        /* Usual case -- non parallel, nonzero vectors.*/
        radians = Angle::Atan2 (sine, cosine);
        }
    else
        {
        // Directly opposing vectors.
        // Rotate 180 degrees around any perpendicular direction.
        DVec3d xVec, yVec, zVec;
        unitStartVector.GetNormalizedTriad (xVec, yVec, zVec);
        axis = xVec;
        if (cosine < 0.0)
            radians = msGeomConst_pi;
        else
            radians = 0.0;
        }
    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Earlin.Lutz     03/15
+--------------------------------------------------------------------------------------*/
bool DVec3d::ProjectToVector (DVec3dCR vectorU, double &fraction) const
    {
    double dotUU = vectorU.MagnitudeSquared ();
    double dotUQ = DotProduct (vectorU);
    return DoubleOps::SafeDivide (fraction, dotUQ, dotUU, 0.0);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Earlin.Lutz     03/15
+--------------------------------------------------------------------------------------*/
bool DVec3d::ProjectToPlane (DVec3dCR vectorU, DVec3dCR vectorV, DPoint2dR uv) const
    {
    double dotUU = vectorU.MagnitudeSquared ();
    double dotVV = vectorV.MagnitudeSquared ();    
    double dotUV = vectorU.DotProduct (vectorV);
    double dotUQ = DotProduct (vectorU);
    double dotVQ = DotProduct (vectorV);

    if (bsiSVD_solve2x2 (&uv.x, &uv.y, dotUU, dotUV, dotUV, dotVV, dotUQ, dotVQ))
        return true;
        
    // The vectors are parallel.  Project to the longer one.
    // (If that fails, the SafeDivide 0 default makes the uv return stay at 00.)
    uv.Zero ();
    if (dotUU >= dotVV)
        DoubleOps::SafeDivide (uv.x, dotUQ, dotUU, 0.0);
    else
        DoubleOps::SafeDivide (uv.y, dotVQ, dotVV, 0.0);
    return false;
    }


END_BENTLEY_NAMESPACE

#include "refFVec3d.h"
