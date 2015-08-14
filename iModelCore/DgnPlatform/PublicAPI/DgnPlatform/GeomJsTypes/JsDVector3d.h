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
struct JsDVector3d : RefCountedBase
{
private:
    DVec3d m_vector;
public:
    JsDVector3d() {m_vector.Init(0,0,0);}
    JsDVector3d(DVec3dCR pt) : m_vector(pt) {;}
    JsDVector3d(double x, double y, double z) {m_vector.x=x; m_vector.y=y; m_vector.z=z;}
    DVec3d Get (){return m_vector;}
    double GetX() {return m_vector.x;}
    double GetY() {return m_vector.y;}
    double GetZ() {return m_vector.z;}
    void SetX(double v) {m_vector.x = v;}
    void SetY(double v) {m_vector.y = v;}
    void SetZ(double v) {m_vector.z = v;}
};




END_BENTLEY_DGNPLATFORM_NAMESPACE

#endif//ndef _JSDVECTOR3D_H_

