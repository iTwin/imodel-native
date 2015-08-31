/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/DgnPlatform/GeomJsTypes/JsDPoint2d.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
//__BENTLEY_INTERNAL_ONLY__
// This file is included into GeomJsApi.h and relies on it for #include and various JsXxx classes.
#pragma once

#ifndef _JSDPOINT2D_H_
#define _JSDPOINT2D_H_

BEGIN_BENTLEY_DGNPLATFORM_NAMESPACE

//=======================================================================================
// @bsiclass                                                    Sam.Wilson      06/15
//=======================================================================================
struct JsDPoint2d : JsGeomWrapperBase<DPoint2d>
{

public:   
    JsDPoint2d() {m_data.Init(0,0);}
    JsDPoint2d(DPoint2dCR data) {m_data = data;}
    JsDPoint2d(double x, double y) {m_data.x=x; m_data.y=y;}
    JsDPoint2dP Clone () {return new JsDPoint2d (m_data);}
    double GetX() {return m_data.x;}
    double GetY() {return m_data.y;}
    void SetX(double v) {m_data.x = v;}
    void SetY(double v) {m_data.y = v;}
    
    VectorAdditionMethods(DPoint2d,JsDPoint2d,JsDPoint2dP,JsDVector2dP)
    DistanceMethods (JsDPoint2dP)

    
};
END_BENTLEY_DGNPLATFORM_NAMESPACE

#endif//ndef _JSDPOINT2D_H_

