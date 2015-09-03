/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/DgnPlatform/GeomJsTypes/JsAngle.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
//__BENTLEY_INTERNAL_ONLY__
// This file is included into GeomJsApi.h and relies on it for #include and various JsXxx classes.
#pragma once

#ifndef JSANGLE_H_
#define JSANGLE_H_

BEGIN_BENTLEY_DGNPLATFORM_NAMESPACE

//=======================================================================================
// @bsiclass                                                    Sam.Wilson      06/15
//=======================================================================================
struct JsAngle : RefCountedBase
{
private:
AngleInDegrees m_angle;
public:
JsAngle (AngleInDegrees a) : m_angle (a){}
JsAngle (){m_angle = AngleInDegrees::FromDegrees (0.0);}
AngleInDegrees Get (){return m_angle;}

JsAngleP Clone () { return new JsAngle (m_angle);}

static JsAngleP CreateDegrees (double degrees){return new JsAngle (AngleInDegrees::FromDegrees (degrees));}
static JsAngleP CreateRadians (double radians){return new JsAngle (AngleInDegrees::FromRadians (radians));}
double GetRadians (){return m_angle.Radians ();}
double GetDegrees (){return m_angle.Degrees ();}
void SetRadians (double radians){m_angle = AngleInDegrees::FromRadians (radians);}
void SetDegrees (double degrees){m_angle = AngleInDegrees::FromDegrees (degrees);}

};

END_BENTLEY_DGNPLATFORM_NAMESPACE

#endif//ndef JSANGLE_H_

