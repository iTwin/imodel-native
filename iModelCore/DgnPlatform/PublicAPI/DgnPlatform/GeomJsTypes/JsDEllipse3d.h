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
struct JsDEllipse3d : JsGeomWrapperBase<DEllipse3d>
{

    JsDEllipse3d (DEllipse3dCR ellipse)
        {
        m_data = ellipse;
        }
    public:
    JsDEllipse3d() {}
    
    JsDEllipse3d (JsDPoint3dP center, JsDVector3dP vector0, JsDVector3dP vector90,
                JsAngleP startAngle, JsAngleP sweepAngle)
        {
        m_data = DEllipse3d::FromVectors (
                    center->Get (), vector0->Get (), vector90->Get (),
                    startAngle->GetRadians (),
                    sweepAngle->GetRadians ()
                    );
         }
    
    JsDEllipse3dP Clone () {return new JsDEllipse3d (m_data);}
    
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
                    startAngle->GetRadians (),
                    sweepAngle->GetRadians ()
                ));            
            }

    JsDPoint3d * GetCenter() {return new JsDPoint3d (m_data.center);}
    JsDVector3d * GetVector0() {return new JsDVector3d (m_data.vector0);}
    JsDVector3d * GetVector90() {return new JsDVector3d (m_data.vector90);}
    JsAngle * GetStartAngle () {return new JsAngle (AngleInDegrees::FromRadians (m_data.start));}
    JsAngle * GetSweepAngle () {return new JsAngle (AngleInDegrees::FromRadians (m_data.sweep));}

    void SetCenter     (JsDPoint3dP center) {m_data.center = center->Get ();}
    void SetVector0    (JsDVector3dP vector0) {m_data.vector0 = vector0->Get ();}
    void SetVector90   (JsDVector3dP vector90) {m_data.vector90 = vector90->Get ();}
    void SetStartAngle (JsAngleP angle) {m_data.start = angle->GetRadians ();}
    void SetSweepAngle (JsAngleP angle) {m_data.sweep = angle->GetRadians ();}
    
    JsDPoint3dP PointAtFraction (double f)
        {
        DPoint3d xyz;
        m_data.FractionParameterToPoint (xyz, f);
        return new JsDPoint3d (xyz);
        }
    JsDRay3dP PointAndDerivativeAtFraction (double f)
        {
        DPoint3d xyz;
        DVec3d derivative1;
        DVec3d derivative2;
        m_data.FractionParameterToDerivatives (xyz, derivative1, derivative2, f);
        return new JsDRay3d (xyz, derivative1);
        }
    
};


END_BENTLEY_DGNPLATFORM_NAMESPACE

#endif//ndef _JSDELLIPSE3D_H_

