/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/DgnPlatform/GeomJsTypes/JsDEllipse3d.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
//__BENTLEY_INTERNAL_ONLY__
// This file is included into GeomJsApi.h and relies on it for #include and various JsXxx classes.
#pragma once

#ifndef _JSDELLIPSE3D_H_
#define _JSDELLIPSE3D_H_

BEGIN_BENTLEY_DGNPLATFORM_NAMESPACE
//=======================================================================================
// @bsiclass                                                    Sam.Wilson      06/15
//=======================================================================================
struct JsDEllipse3d : RefCountedBase
{
    friend struct JsCurvePrimitive;
    private:
    DEllipse3d m_ellipse;

    JsDEllipse3d (DEllipse3dCR ellipse) : m_ellipse (ellipse)
        {
        }
    public:
    JsDEllipse3d() {}
    JsDEllipse3d (JsDPoint3dP center, JsDVector3dP vector0, JsDVector3dP vector90,
                JsAngleP startAngle, JsAngleP sweepAngle)
        : m_ellipse(DEllipse3d::FromVectors (
                    center->m_point, vector0->m_vector, vector90->m_vector,
                    startAngle->m_angle.Radians (),
                    sweepAngle->m_angle.Radians ()
                    ))
         {}
       
    static JsDEllipse3dP FromCoordinates (
            double cx, double cy, double cz,
            double v0x, double v0y, double v0z,
            double v90x, double v90y, double v90z,
            JsAngleP startAngle,
            JsAngleP sweepAngle)
            {
            return new JsDEllipse3d
                (DEllipse3d::From (
                    cx, cy, cz,
                    v0x, v0y, v0z,
                    v90x, v90y, v90z,
                    startAngle->m_angle.Radians (),
                    sweepAngle->m_angle.Radians ()
                ));            
            }

    JsDPoint3d * GetCenter() {return new JsDPoint3d (m_ellipse.center);}
    JsDVector3d * GetVector0() {return new JsDVector3d (m_ellipse.vector0);}
    JsDVector3d * GetVector90() {return new JsDVector3d (m_ellipse.vector90);}
    JsAngle * GetStartAngle () {return new JsAngle (AngleInDegrees::FromRadians (m_ellipse.start));}
    JsAngle * GetSweepAngle () {return new JsAngle (AngleInDegrees::FromRadians (m_ellipse.sweep));}

    void SetCenter     (JsDPoint3dP center) {m_ellipse.center = center->m_point;}
    void SetVector0    (JsDVector3dP vector0) {m_ellipse.vector0 = vector0->m_vector;}
    void SetVector90   (JsDVector3dP vector90) {m_ellipse.vector90 = vector90->m_vector;}
    void SetStartAngle (JsAngleP angle) {m_ellipse.start = angle->m_angle.Radians ();}
    void SetSweepAngle (JsAngleP angle) {m_ellipse.sweep = angle->m_angle.Radians ();}
    
    JsDPoint3dP PointAtFraction (double f)
        {
        DPoint3d xyz;
        m_ellipse.FractionParameterToPoint (xyz, f);
        return new JsDPoint3d (xyz);
        }
    JsDRay3dP PointAndDerivativeAtFraction (double f)
        {
        DPoint3d xyz;
        DVec3d derivative1;
        DVec3d derivative2;
        m_ellipse.FractionParameterToDerivatives (xyz, derivative1, derivative2, f);
        return new JsDRay3d (xyz, derivative1);
        }
    
};


END_BENTLEY_DGNPLATFORM_NAMESPACE

#endif//ndef _JSDELLIPSE3D_H_

