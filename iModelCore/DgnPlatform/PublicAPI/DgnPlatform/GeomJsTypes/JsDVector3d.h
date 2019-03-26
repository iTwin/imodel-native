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
    
    JsDVector3dP Clone (){return new JsDVector3d (m_data);}

    //Scaled vector by -1
    JsDVector3dP Negate()
        {
        DVec3d uvw;
        uvw.Negate(m_data);
        return new JsDVector3d(uvw);
        }
        
           
    //return a vector same length as source but rotate 90 degrees CCW
    JsDVector3dP FromCCWPerpendicularXY (JsDVector3dP source)
    {
    return new JsDVector3d(DVec3d::FromCCWPerpendicularXY(source->Get()));
    }  
JsDVector3dP Rotate90Towards (JsDVector3dP target)
    {
    return new JsDVector3d(DVec3d::FromRotate90Towards(Get(),target->Get()));
    }
JsDVector3dP Rotate90Around (JsDVector3dP axis)
    {
    return new JsDVector3d(DVec3d::FromRotate90Around(Get(),axis->Get()));
    }
static JsDVector3dP CreateXYAngleAndMagnitude (JsAngleP angle, double magnitude)
    {
    return new JsDVector3d(DVec3d::FromXYAngleAndMagnitude(angle->GetRadians (), magnitude));
    }

    DeclareAndImplementMethods_AddVector(DVec3d,JsDVector3d,JsDVector3dP,DVec3d,JsDVector3d,JsDVector3dP)
    DeclareAndImplementMethods_Distance (JsDVector3dP)
    DeclareAndImplementMethods_VectorMagnitudeAndScaling(JsDVector3dP)

 JsDVector3dP Scale (double scale)
    {
    return new JsDVector3d(DVec3d::FromScale(m_data,scale));
    }

  JsDVector3dP CrossProduct(JsDVector3dP vectorB)
    {
    return new JsDVector3d(DVec3d::FromCrossProduct(m_data,vectorB->Get()));
    }
JsDVector3dP UnitCrossProduct(JsDVector3dP vectorB)
    {
    return CreateJsVector (DVec3d::FromCrossProduct(m_data,vectorB->Get()), 1.0);
    }
JsDVector3dP SizedCrossProduct (JsDVector3dP vectorA,JsDVector3dP vectorB, double productLength)
    {
    DVec3d copy;
    copy = m_data;
    copy.SizedCrossProduct(vectorA->Get(),vectorB->Get(),productLength);
    return CreateJsVector (DVec3d::FromCrossProduct(m_data,vectorB->Get()), productLength);
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
    
JsDVector3dP UnitPerpendicular ()
    {
    DVec3d xVector, yVector, zVector;
    m_data.GetNormalizedTriad (xVector, yVector, zVector);
    return new JsDVector3d (xVector);
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

