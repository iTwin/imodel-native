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


float FVec3d::FDotProduct (FVec3dCR other)    const {return GeometryTemplates::DotProduct <float,float> (x,y,z,other.x, other.y, other.z);}
FVec3d FVec3d::FCrossProduct (FVec3dCR other) const {return GeometryTemplates::CrossProduct <float, FVec3d> (x,y,z,other.x, other.y, other.z);}
double FVec3d::DotProduct (FVec3dCR other)    const {return GeometryTemplates::DotProduct <double, double> (x,y,z,other.x, other.y, other.z);}
FVec3d FVec3d::CrossProduct (FVec3dCR other) const {return GeometryTemplates::CrossProduct <double, FVec3d> (x,y,z,other.x, other.y, other.z);}

double FVec3d::Magnitude () const { return sqrt (GeometryTemplates::DotProduct <double, double> (x,y,z, x,y,z));}
double FVec3d::MagnitudeSquared () const { return GeometryTemplates::DotProduct <double, double> (x,y,z, x,y,z);}
double FVec3d::MaxAbs () const {return DoubleOps::MaxAbs (x,y,z);}
/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      05/2017
+--------------------------------------------------------------------------------------*/
void FVec3d::Swap (FVec3dR other)
    {
    FVec3d temp = *this;
    *this = other;
    other = temp;
    }

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


END_BENTLEY_NAMESPACE
