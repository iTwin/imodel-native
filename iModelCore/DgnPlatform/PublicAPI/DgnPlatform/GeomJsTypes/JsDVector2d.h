/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/DgnPlatform/GeomJsTypes/JsDVector2d.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
//__BENTLEY_INTERNAL_ONLY__
// This file is included into GeomJsApi.h and relies on it for #include and various JsXxx classes.
#pragma once

#ifndef _JSDVECTOR2D_H_
#define _JSDVECTOR2D_H_

BEGIN_BENTLEY_DGNPLATFORM_NAMESPACE


//=======================================================================================
// @bsiclass                                                    Sam.Wilson      06/15
//=======================================================================================
struct JsDVector2d : JsGeomWrapperBase<DVec2d>
{
public:
    JsDVector2d() {m_data.Init(0,0);}
    JsDVector2d(DVec2dCR data) {m_data = data;}
    JsDVector2d(double x, double y) {m_data.x=x; m_data.y=y;}
    double GetX() {return m_data.x;}
    double GetY() {return m_data.y;}
    void SetX(double v) {m_data.x = v;}
    void SetY(double v) {m_data.y = v;}

    VectorAdditionMethods(DVec2d,JsDVector2d,JsDVector2dP,JsDVector2dP)
    DistanceMethods (JsDVector2dP)
    MagnitudeMethods

    JsDVector2dP Clone (){return new JsDVector2d (m_data);}

#ifdef TheseMethodsFromDVector2dHaveToBeReviewedFor2d
    JsDVector2dP Normalize()
        {
        DVec2d copy;
        copy.Normalize(m_data);
        return new JsDVector2d(copy);
        }
    
    JsDVector2dP Clone (){return new JsDVector2d (m_data);}

    //Scaled vector by -1
    JsDVector2dP Negate()
        {
        DVec2d uvw;
        uvw.Negate(m_data);
        return new JsDVector2d(uvw);
        }
        
JsDVector2dP VectorTo (JsDPoint2dP target)
    {
    return new JsDVector2d (DVec2d::FromStartEnd (m_data, target->Get ()));
    }
    JsDVector2dP UnitVectorTowards (JsDPoint2dP target)
        {
        return new JsDVector2d (DVec2d::FromStartEnd (m_data, target->Get ()));
        }
            
    //Returns new vector that begins at start, and ends at end
    static JsDVector2dP FromStartEnd (JsDPoint2dP start, JsDPoint2dP end)
        {
        return new JsDVector2d(DVec2d::FromStartEnd(start->Get(),end->Get()));
        }
    //Same as start end, but normalizes the result (makes magnitude 1)
    static JsDVector2dP FromStartEndNormalize (JsDPoint2dP start, JsDPoint2dP end)
        {
        return new JsDVector2d(DVec2d::FromStartEndNormalize(start->Get(),end->Get()));
        }
    //return a vector same length as source but rotate 90 degrees CCW
    JsDVector2dP FromCCWPerpendicularXY (JsDVector2dP source)
    {
    return new JsDVector2d(DVec2d::FromCCWPerpendicularXY(source->Get()));
    }  
JsDVector2dP FromRotate90Towards (JsDVector2dP source, JsDVector2dP target)
    {
    return new JsDVector2d(DVec2d::FromRotate90Towards(source->Get(),target->Get()));
    }
static JsDVector2dP FromRotate90Around (JsDVector2dP source, JsDVector2dP axis)
    {
    return new JsDVector2d(DVec2d::FromRotate90Around(source->Get(),axis->Get()));
    }
static JsDVector2dP FromXYAngleAndMagnitude (double angle, double magnitude)
    {
    return new JsDVector2d(DVec2d::FromXYAngleAndMagnitude(angle, magnitude));
    }

    VectorAdditionMethods(DVec2d,JsDVector2d,JsDVector2dP,JsDVector2dP)

 JsDVector2dP Scale (double scale)
    {
    return new JsDVector2d(DVec2d::FromScale(m_data,scale));
    }
 JsDVector2dP ScaleToLength (double scale)
    {
    DVec2d copy;
    copy = m_data;
    copy.ScaleToLength(copy,scale);
    return new JsDVector2d(copy);
    }
  JsDVector2dP CrossProduct(JsDVector2dP vectorB)
    {
    return new JsDVector2d(DVec2d::FromCrossProduct(m_data,vectorB->Get()));
    }
JsDVector2dP NormalizedCrossProduct(JsDVector2dP vectorB)
    {
    return new JsDVector2d(DVec2d::FromNormalizedCrossProduct(m_data,vectorB->Get()));
    }
JsDVector2dP SizedCrossProduct (JsDVector2dP vectorA,JsDVector2dP vectorB, double productLength)
    {
    DVec2d copy;
    copy = m_data;
    copy.SizedCrossProduct(vectorA->Get(),vectorB->Get(),productLength);
    return new JsDVector2d(copy);
    }
JsDVector2dP RotateXY (double angle)
    {
    DVec2d copy;
    copy = m_data;
    copy.RotateXY(angle);
    return new JsDVector2d(copy);
    }
JsDVector2dP UnitPerpendicularXY (JsDVector2dP vector)
    {
    DVec2d copy;
    copy = m_data;
    copy.UnitPerpendicularXY(vector->Get());
    return new JsDVector2d(copy);
    }
double CrossProductMagnitude(JsDVector2dP vectorB)
    {
    DVec2d copy;
    copy = DVec2d::FromCrossProduct(m_data,vectorB->Get());
    return copy.Magnitude();
    }
double DotProduct(JsDVector2dP vectorB)
    {
    return m_data.DotProduct(vectorB->Get());
    }
double DotProductXY (JsDVector2dP vectorB)
    {
    return m_data.DotProductXY( vectorB->Get());
    }
double CrossProductXY (JsDVector2dP vectorB)
    {
    return m_data.CrossProductXY(vectorB->Get());
    }
double TripleProduct(JsDVector2dP vectorB, JsDVector2dP vectorC)
    {
    return m_data.TripleProduct(vectorB->Get(), vectorC->Get());
    }
double MaxAbs ()
    {
    return m_data.MaxAbs();
    }
    
JsDVector2dP UnitPerpendicular ()
    {
    DVec2d xVector, yVector, zVector;
    m_data.GetNormalizedTriad (xVector, yVector, zVector);
    return new JsDVector2d (xVector);
    }
JsAngleP AngleTo (JsDVector2dP vectorB)
    {
    return JsAngle::CreateRadians(m_data.AngleTo(vectorB->Get()));
    }
JsAngleP AngleToXY (JsDVector2dP vectorB)
    {
    return JsAngle::CreateRadians(m_data.AngleToXY(vectorB->Get()));
    }
JsAngleP SmallerUnorientedAngleTo (JsDVector2dP vectorB)
    {
    return JsAngle::CreateRadians(m_data.SmallerUnorientedAngleTo(vectorB->Get()));
    }
JsAngleP SignedAngleTo (JsDVector2dP vectorB, JsDVector2dP upVector)
    {
    return JsAngle::CreateRadians(m_data.SignedAngleTo( vectorB->Get(),upVector->Get()));
    }
JsAngleP PlanarAngleTo (JsDVector2dP vectorB, JsDVector2dP planeNormal)
    {
    return JsAngle::CreateRadians(m_data.PlanarAngleTo( vectorB->Get(), planeNormal->Get()));
    }
bool IsInSmallerSector (JsDVector2dP vectorA, JsDVector2dP vectorB)
    {
    return m_data.IsVectorInSmallerSector(vectorA->Get(), vectorB->Get());
    }
bool IsInCCWSector (JsDVector2dP vectorA, JsDVector2dP vectorB, JsDVector2dP upVector)
    {
    return m_data.IsVectorInCCWSector(vectorA->Get(),vectorB->Get(),upVector->Get());
    }
bool IsParallelTo (JsDVector2dP vectorB)
    {
    return m_data.IsParallelTo(vectorB->Get());
    }
bool IsPerpendicularTo (JsDVector2dP vectorB)
    {
    return m_data.IsPerpendicularTo(vectorB->Get());
    }
#endif

};




END_BENTLEY_DGNPLATFORM_NAMESPACE

#endif//ndef _JSDVECTOR2D_H_

