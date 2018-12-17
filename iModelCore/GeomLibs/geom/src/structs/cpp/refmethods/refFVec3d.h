/*--------------------------------------------------------------------------------------+
|
|     $Source: geom/src/structs/cpp/refmethods/refFVec3d.h $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
// refFVec3d.h is #include'd into refDVec3d.cpp (for template sharing) -- do NOT include PCH.
BEGIN_BENTLEY_NAMESPACE



/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      05/2017
+--------------------------------------------------------------------------------------*/
FVec3d FVec3d::From (double xx, double yy, double zz) {return from (xx, yy, zz);}
FVec3d FVec3d::FromXY (FVec3dCR xy, double zz) {return from (xy.x, xy.y, zz);}
FVec3d FVec3d::FromXY (DVec3dCR xy, double zz) {return from (xy.x, xy.y, zz);}
FVec3d FVec3d::From (DVec3dCR xyz) {return from (xyz.x, xyz.y, xyz.z);}
FVec3d FVec3d::FromZero () { return from (0.0, 0.0, 0.0);}
FVec3d FVec3d::FromOne (){return from (1.0, 1.0, 1.0);}

FVec3d FVec3d::FromStartEnd (DPoint3dCR start, DPoint3dCR end){return GeometryTemplates::FromStartEnd <DPoint3d, FVec3d>(start, end);}
FVec3d FVec3d::FromStartEnd (FPoint3dCR start, FPoint3dCR end){return GeometryTemplates::FromStartEnd <FPoint3d, FVec3d>(start, end);}

ValidatedFVec3d FVec3d::FromStartEndNormalized (FPoint3dCR start, FPoint3dCR end){return GeometryTemplates::FromStartEndNormalized <FPoint3d, FVec3d>(start, end);}
ValidatedFVec3d FVec3d::FromStartEndNormalized (DPoint3dCR start, DPoint3dCR end){return GeometryTemplates::FromStartEndNormalized <DPoint3d, FVec3d>(start, end);}


FVec3d FVec3d::FromCrossProduct (FVec3dCR vector0, FVec3dCR vector1) {return GeometryTemplates::CrossProduct <double, FVec3d> (vector0.x, vector0.y, vector0.z, vector1.x, vector1.y, vector1.z);}
double FVec3d::DotProduct (FVec3dCR other)    const {return GeometryTemplates::DotProduct <double, double> (x,y,z,other.x, other.y, other.z);}

double FVec3d::DotProductXY (FVec3dCR other)    const {return GeometryTemplates::DotProductXY <double, double> (x,y,other.x, other.y);}
double FVec3d::CrossProductXY (FVec3dCR other) const {return GeometryTemplates::CrossProductXY <double, double> (x,y,other.x, other.y);}


double FVec3d::Magnitude () const { return sqrt (GeometryTemplates::DotProduct <double, double> (x,y,z, x,y,z));}
double FVec3d::MagnitudeSquared () const { return GeometryTemplates::DotProduct <double, double> (x,y,z, x,y,z);}
double FVec3d::MaxAbs () const {return DoubleOps::MaxAbs (x,y,z);}


bool FVec3d::IsEqual (FVec3dCR other) const
    {
    return x == other.x && y == other.y && z == other.z;
    }

bool FVec3d::IsEqual (FVec3dCR other, double tolerance) const
    {
    return fabs (x - other.x) <= tolerance
        && fabs (y - other.y) <= tolerance
        && fabs (z - other.z) <= tolerance;
   }

/*-----------------------------------------------------------------*//**
* @description Tests if two vectors are parallel.
*
* @param [in] vector2 The second vector
* @return true if the vectors are parallel within tolerance
* @bsimethod                            EarlinLutz      03/03
+----------------------------------------------------------------------*/
bool FVec3d::IsParallelTo
(
FVec3dCR vector2
) const
    {
    double      a2 = this->DotProduct (*this);
    double      b2 = vector2.DotProduct (vector2);
    double      cross;
    double      eps = Angle::SmallFloatRadians (); /* small angle tolerance (in radians) */
    auto vecC = DVec3d::FromCrossProduct (*this, vector2);
    cross = vecC.MagnitudeSquared ();

    /* a2,b2,c2 are squared lengths of respective vectors */
    /* c2 = sin^2(theta) * a2 * b2 */
    /* For small theta, sin^2(theta)~~theta^2 */
    /* Zero-length vector case falls through as equality */

    return  cross <= eps * eps * a2 * b2;
    }

/*-----------------------------------------------------------------*//**
* @description Tests if two vectors are parallel and positive dot product
*
* @param [in] vector2 The second vector
* @return true if the vectors are parallel within tolerance
* @bsimethod                            EarlinLutz      03/03
+----------------------------------------------------------------------*/
bool FVec3d::IsPositiveParallelTo
(
FVec3dCR vector2
) const
    {
    return IsParallelTo (vector2) && this->DotProduct (vector2) > 0.0;
    }

/*-----------------------------------------------------------------*//**
* @description Tests if two vectors are parallel and positive dot product
*
* @param [in] vector2 The second vector
* @return true if the vectors are parallel within tolerance
* @bsimethod                            EarlinLutz      03/03
+----------------------------------------------------------------------*/
bool FVec3d::IsPerpendicularTo
(
FVec3dCR vector2
) const
    {
    double e = Angle::SmallFloatRadians ();
    return GeometryTemplates::IsPerpendicularTo (*this, vector2, e);
    }


//! return true 3D angle between vectors.   Because there is no clear "up" direction, this is 0 or positive, and less than 180 degrees.
Angle FVec3d::AngleTo (FVec3dCR other) const
    {
    return Angle::FromRadians (GeometryTemplates::RadiansTo (*this, other));
    }
//! return (signed) angle from this vector to the other, as viewed in xy direction.
Angle FVec3d::AngleToXY (FVec3dCR other) const
    {
    return Angle::FromRadians (GeometryTemplates::RadiansToXY (*this, other));
    }
//! return (signed) angle from this vector to the other, as measured in the plane perpendicular to the planeUpVector
Angle FVec3d::PlanarAngleTo (FVec3dCR other, FVec3dCR planeUpVector) const
    {
    return Angle::FromRadians (GeometryTemplates::SignedRadiansTo (*this, other, planeUpVector));
    }
//! return the (signed) angle from this vector to the other.  This is a full 3D angle in the plane of the two vectors, with the
//! orientationVector selecting which side is positive.
Angle FVec3d::SignedAngleTo (FVec3dCR other, FVec3dCR orientationVector) const
    {
    return Angle::FromRadians (GeometryTemplates::PlanarRadiansTo (*this, other, orientationVector));
    }

//! return the smaller angle between the vectors extended in both directions.  This is no larger than 90 degrees.
Angle FVec3d::SmallerUnorientedAngleTo (FVec3dCR other) const
    {
    return Angle::FromRadians (GeometryTemplates::SmallerUnorientedRadiansTo (*this, other));
    }

//! return the smaller angle between the vectors extended in both directions.  This is no larger than 90 degrees.
Angle FVec3d::AngleFromPerpendicular (FVec3dCR other) const
    {
    return Angle::FromRadians (GeometryTemplates::RadiansFromPerpendicular (*this, other));
    }
END_BENTLEY_NAMESPACE
