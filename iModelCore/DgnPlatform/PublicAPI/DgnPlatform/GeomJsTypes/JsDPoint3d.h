/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/DgnPlatform/GeomJsTypes/JsDPoint3d.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
//__BENTLEY_INTERNAL_ONLY__
// This file is included into GeomJsApi.h and relies on it for #include and various JsXxx classes.
#pragma once

#ifndef _JSDPOINT3D_H_
#define _JSDPOINT3D_H_

BEGIN_BENTLEY_DGNPLATFORM_NAMESPACE

DVec3d GetDVec3d (JsDVector3dP);
//=======================================================================================
// @bsiclass                                                    Sam.Wilson      06/15
//=======================================================================================
struct JsDPoint3d : JsGeomWrapperBase<DPoint3d>
{

public:   

    JsDPoint3d() {m_data.Init(0,0,0);}
    JsDPoint3d(DPoint3dCR data) {m_data = data;}
    JsDPoint3d(double x, double y, double z) {m_data.x=x; m_data.y=y; m_data.z=z;}

    double GetX() {return m_data.x;}
    double GetY() {return m_data.y;}
    double GetZ() {return m_data.z;}
    void SetX(double v) {m_data.x = v;}
    void SetY(double v) {m_data.y = v;}
    void SetZ(double v) {m_data.z = v;}

    JsDPoint3dP Clone () {return new JsDPoint3d (m_data);}
    
    JsDPoint3dP Interpolate (double fraction, JsDPoint3dP right)
        {
        return new JsDPoint3d(DPoint3d::FromInterpolate(m_data, fraction,right->m_data));
        }
    JsDPoint3dP PlusVector (JsDVector3dP vector)
        {
        return new JsDPoint3d(DPoint3d::FromSumOf(m_data, GetDVec3d (vector))); 
        }
    JsDPoint3dP MinusVector (JsDVector3dP vector)
        {
        return new JsDPoint3d (DPoint3d::FromSumOf (m_data, GetDVec3d (vector), -1.0));
        }
    JsDPoint3dP PlusScaledVector(JsDVector3dP vector,double scalar)
        {
        return new JsDPoint3d(DPoint3d::FromSumOf(m_data, GetDVec3d (vector), scalar)); 
        }
     JsDPoint3dP Plus2ScaledVectors (JsDVector3dP vectorA, double scalarA,  JsDVector3dP vectorB, double scalarB)
        {
        return new JsDPoint3d(DPoint3d::FromSumOf(m_data, GetDVec3d (vectorA), scalarA, GetDVec3d(vectorB),scalarB)); 
        }
    
     JsDPoint3dP Plus3ScaledVectors(JsDVector3dP vectorA, double scalarA,  JsDVector3dP vectorB, double scalarB, JsDVector3dP vectorC, double scalarC)
        {
        return new JsDPoint3d(DPoint3d::FromSumOf(m_data, GetDVec3d (vectorA), scalarA, GetDVec3d(vectorB),scalarB,  GetDVec3d(vectorC),scalarC)); 
        }
        
    double Distance (JsDPoint3dP other){return m_data.Distance (other->m_data);}
    double DistanceSquared (JsDPoint3dP other){return m_data.DistanceSquared (other->m_data);}


};
END_BENTLEY_DGNPLATFORM_NAMESPACE

#endif//ndef _JSDPOINT3D_H_

