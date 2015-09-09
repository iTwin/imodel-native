/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/DgnPlatform/GeomJsTypes/JsBsplineCurve.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
//__BENTLEY_INTERNAL_ONLY__
// This file is included into GeomJsApi.h and relies on it for #include and various JsXxx classes.
#pragma once

#ifndef _JSBSPLINECURVE_H_
#define _JSBSPLINECURVE_H_

BEGIN_BENTLEY_DGNPLATFORM_NAMESPACE

//=======================================================================================
// @bsiclass                                                    Eariln.Lutz     08/15
//=======================================================================================
struct JsBsplineCurve: JsGeomWrapperBase<MSBsplineCurvePtr>
{
private:
public:
    JsBsplineCurve (MSBsplineCurvePtr const &header) { m_data = header;}
    JsBsplineCurveP Clone () {return new JsBsplineCurve (m_data->CreateCopy ());}
    static JsBsplineCurveP CreateFromPoles (JsDPoint3dArrayP xyz, 
        JsDoubleArrayP weights, JsDoubleArrayP knots,
        double order, bool closed, bool preWeighted)
        {
        MSBsplineCurvePtr bcurve = MSBsplineCurve::CreateFromPolesAndOrder (
                            xyz->GetRef (),
                            weights == nullptr ? nullptr : &weights->GetRef (),
                            knots   == nullptr ? nullptr : &knots->GetRef (),
                            (int)order,
                            closed,
                            preWeighted
                            );
        if (bcurve.IsValid ())
                return new JsBsplineCurve (bcurve);

        return nullptr;
        }

    bool IsPeriodic (){return false;}

};

END_BENTLEY_DGNPLATFORM_NAMESPACE

#endif//ndef _JSPOLYFACEMESH_H_

