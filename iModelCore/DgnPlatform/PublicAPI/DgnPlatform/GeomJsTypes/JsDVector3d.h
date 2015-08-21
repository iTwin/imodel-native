/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/DgnPlatform/GeomJsTypes/JsDVector3d.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
//__BENTLEY_INTERNAL_ONLY__
// This file is included into GeomJsApi.h and relies on it for #include and various JsXxx classes.
#pragma once

#ifndef _JSDVECTOR3D_H_
#define _JSDVECTOR3D_H_

BEGIN_BENTLEY_DGNPLATFORM_NAMESPACE


//=======================================================================================
// @bsiclass                                                    Sam.Wilson      06/15
//=======================================================================================
struct JsDVector3d : JsGeomWrapperBase<DVec3d>
{
public:
    JsDVector3d() {m_data.Init(0,0,0);}
    JsDVector3d(DVec3dCR data) {m_data = data;}
    JsDVector3d(double x, double y, double z) {m_data.x=x; m_data.y=y; m_data.z=z;}
    double GetX() {return m_data.x;}
    double GetY() {return m_data.y;}
    double GetZ() {return m_data.z;}
    void SetX(double v) {m_data.x = v;}
    void SetY(double v) {m_data.y = v;}
    void SetZ(double v) {m_data.z = v;}
    JsDVector3dP Normalize()
    {
    DVec3d copy;
    copy.Normalize(m_data);
    return new JsDVector3d(copy);
    }
    static JsDVector3dP CreateClone(JsDVector3dP original)
        {
        return new JsDVector3d(original->GetX(),original->GetY(),original->GetZ());
        }
    //Scaled vector by -1
    JsDVector3dP Negate()
        {
        DVec3d uvw;
        uvw.Negate(m_data);
        return new JsDVector3d(uvw);
        }
    //Returns new vector that begins at start, and ends at end
    static JsDVector3dP FromStartEnd (JsDPoint3dP start, JsDPoint3dP end)
        {
        return new JsDVector3d(DVec3d::FromStartEnd(start->Get(),end->Get()));
        }
    //Same as start end, but normalizes the result (makes magnitude 1)
    static JsDVector3dP FromStartEndNormalize (JsDPoint3dP start, JsDPoint3dP end)
        {
        return new JsDVector3d(DVec3d::FromStartEndNormalize(start->Get(),end->Get()));
        }
    //return a vector same length as source but rotate 90 degrees CCW
    JsDVector3dP FromCCWPerpendicularXY (JsDVector3dP source)
    {
    return new JsDVector3d(DVec3d::FromCCWPerpendicularXY(source->Get()));
    }  
JsDVector3dP FromRotate90Towards (JsDVector3dP source, JsDVector3dP target)
    {
    return new JsDVector3d(DVec3d::FromRotate90Towards(source->Get(),target->Get()));
    }
static JsDVector3dP FromRotate90Around (JsDVector3dP source, JsDVector3dP axis)
    {
    return new JsDVector3d(DVec3d::FromRotate90Around(source->Get(),axis->Get()));
    }
static JsDVector3dP FromXYAngleAndMagnitude (double angle, double magnitude)
    {
    return new JsDVector3d(DVec3d::FromXYAngleAndMagnitude(angle, magnitude));
    }
static JsDVector3dP FromInterpolate (JsDVector3dP vectorA, double fraction, JsDVector3dP vectorB)
    {
    return new JsDVector3d(DVec3d::FromInterpolate(vectorA->Get(),fraction, vectorB->Get()));
    }
static JsDVector3dP FromInterpolateBilinear (JsDVector3dP vector00, JsDVector3dP vector10, JsDVector3dP vector01, JsDVector3dP vector11, double u, double v)
    {
    return new JsDVector3d(DVec3d::FromInterpolateBilinear(vector00->Get(),vector10->Get(),vector01->Get(),vector11->Get(),u,v));
    }
static JsDVector3dP FromAdd2Scaled (JsDVector3dP vectorA, double scaleA,JsDVector3dP vectorB, double scaleB)
    {
    return new JsDVector3d(DVec3d::FromSumOf(vectorA->Get(),scaleA, vectorB->Get(),scaleB));
    }
static JsDVector3dP FromAdd3Scaled (JsDVector3dP vectorA, double scaleA, JsDVector3dP vectorB, double scaleB, JsDVector3dP vectorC, double scaleC)
    {
    return new JsDVector3d(DVec3d::FromSumOf(vectorA->Get(),scaleA, vectorB->Get(),scaleB, vectorC->Get(), scaleC));
    }
 JsDVector3dP Add(JsDVector3dP vectorB)
    {
    return new JsDVector3d(DVec3d::FromSumOf(m_data,vectorB->Get()));
    }
 JsDVector3dP AddScaled (JsDVector3dP vectorB, double scalar)
    {
    return new JsDVector3d(DVec3d::FromSumOf(m_data,1.0,vectorB->Get(),scalar));
    }
 JsDVector3dP Subtract (JsDVector3dP vectorB)
    {
    return Add(vectorB->Negate());
    }
 JsDVector3dP Scale (double scale)
    {
    return new JsDVector3d(DVec3d::FromScale(m_data,scale));
    }
 JsDVector3dP ScaleToLength (double scale)
    {
    DVec3d copy;
    copy = m_data;
    copy.ScaleToLength(copy,scale);
    return new JsDVector3d(copy);
    }
  JsDVector3dP CrossProduct(JsDVector3dP vectorB)
    {
    return new JsDVector3d(DVec3d::FromCrossProduct(m_data,vectorB->Get()));
    }
JsDVector3dP NormalizedCrossProduct(JsDVector3dP vectorB)
    {
    return new JsDVector3d(DVec3d::FromNormalizedCrossProduct(m_data,vectorB->Get()));
    }
  JsDVector3dP SizedCrossProduct (JsDVector3dP vectorA,JsDVector3dP vectorB, double productLength)
    {
    DVec3d copy;
    copy = m_data;
    copy.SizedCrossProduct(vectorA->Get(),vectorB->Get(),productLength);
    return new JsDVector3d(copy);
    }
JsDVector3dP RotateXY (double angle)
    {
    DVec3d copy;
    copy = m_data;
    copy.RotateXY(angle);
    return new JsDVector3d(copy);
    }
 JsDVector3dP UnitPerpendicularXY (JsDVector3dP vector)
    {
    DVec3d copy;
    copy = m_data;
    copy.UnitPerpendicularXY(vector->Get());
    return new JsDVector3d(copy);
    }
double CrossProductMagnitude(JsDVector3dP vectorB)
    {
    DVec3d copy;
    copy = DVec3d::FromCrossProduct(m_data,vectorB->Get());
    return copy.Magnitude();
    }
double DotProduct(JsDVector3dP vectorB)
    {
    return m_data.DotProduct(vectorB->Get());
    }
double DotProductXY (JsDVector3dP vectorB)
    {
    return m_data.DotProductXY( vectorB->Get());
    }
double CrossProductXY (JsDVector3dP vectorB)
    {
    return m_data.CrossProductXY(vectorB->Get());
    }
double TripleProduct(JsDVector3dP vectorB, JsDVector3dP vectorC)
    {
    return m_data.TripleProduct(vectorB->Get(), vectorC->Get());
    }
double Magnitude ()
    {
    return m_data.Magnitude();
    }
double MagnitudeSquared ()
    {
    return m_data.MagnitudeSquared();
    }
double Distance (JsDVector3dP vectorB)
    {
    return m_data.Distance(vectorB->Get());
    }
double DistanceSquared (JsDVector3dP vectorB)
    {
    return m_data.DistanceSquared(vectorB->Get());
    }
double MaxAbs ()
    {
    return m_data.MaxAbs();
    }
JsDVector3dP CreatePerpendicularTo(JsDVector3dP original)
    {

    }
JsAngleP AngleTo (JsDVector3dP vectorB)
    {
    return JsAngle::CreateRadians(m_data.AngleTo(vectorB->Get()));
    }
JsAngleP AngleToXY (JsDVector3dP vectorB)
    {
    return JsAngle::CreateRadians(m_data.AngleToXY(vectorB->Get()));
    }
JsAngleP SmallerUnorientedAngleTo (JsDVector3dP vectorB)
    {
    return JsAngle::CreateRadians(m_data.SmallerUnorientedAngleTo(vectorB->Get()));
    }
JsAngleP SignedAngleTo (JsDVector3dP vectorB, JsDVector3dP upVector)
    {
    return JsAngle::CreateRadians(m_data.SignedAngleTo( vectorB->Get(),upVector->Get()));
    }
JsAngleP PlanarAngleTo (JsDVector3dP vectorB, JsDVector3dP planeNormal)
    {
    return JsAngle::CreateRadians(m_data.PlanarAngleTo( vectorB->Get(), planeNormal->Get()));
    }
bool IsInSmallerSector (JsDVector3dP vectorA, JsDVector3dP vectorB)
    {
    return m_data.IsVectorInSmallerSector(vectorA->Get(), vectorB->Get());
    }
bool IsInCCWSector (JsDVector3dP vectorA, JsDVector3dP vectorB, JsDVector3dP upVector)
    {
    return m_data.IsVectorInCCWSector(vectorA->Get(),vectorB->Get(),upVector->Get());
    }
bool IsParallelTo (JsDVector3dP vectorB)
    {
    return m_data.IsParallelTo(vectorB->Get());
    }
bool IsPerpendicularTo (JsDVector3dP vectorB)
    {
    return m_data.IsPerpendicularTo(vectorB->Get());
    }


};




END_BENTLEY_DGNPLATFORM_NAMESPACE

#endif//ndef _JSDVECTOR3D_H_

