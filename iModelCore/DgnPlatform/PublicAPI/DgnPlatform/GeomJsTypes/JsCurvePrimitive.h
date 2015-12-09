/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/DgnPlatform/GeomJsTypes/JsCurvePrimitive.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
//__BENTLEY_INTERNAL_ONLY__
// This file is included into GeomJsApi.h and relies on it for #include and various JsXxx classes.
#pragma once

#ifndef _JSCURVEPRIMITIVE_H_
#define _JSCURVEPRIMITIVE_H_

BEGIN_BENTLEY_DGNPLATFORM_NAMESPACE

//=======================================================================================
// @bsiclass                                                    Eariln.Lutz     08/15
//=======================================================================================
struct JsCurvePrimitive: RefCountedBase
{
protected :
    ICurvePrimitivePtr m_curvePrimitive;

    void Set (ICurvePrimitivePtr const &curvePrimitive)
        {
        m_curvePrimitive = curvePrimitive;
        }
public:
    JsCurvePrimitive () {}

    JsCurvePrimitive (ICurvePrimitivePtr curvePrimitive) : m_curvePrimitive (curvePrimitive) {}

    static JsCurvePrimitiveP CreateLineString (JsDPoint3dArrayP data)
        {
        ICurvePrimitivePtr cp = ICurvePrimitive::CreateLineString (data->GetRef ());
        return new JsCurvePrimitive (cp);
        }

    static JsCurvePrimitiveP CreateBsplineCurve (JsBsplineCurveP data)
        {
        ICurvePrimitivePtr cp = ICurvePrimitive::CreateBsplineCurve (*data->Get ());
        if (cp.IsValid ())
            return new JsCurvePrimitive (cp);
        return nullptr;
        }

    JsCurvePrimitiveP Clone () {return new JsCurvePrimitive (m_curvePrimitive->Clone ());} 

    double CurvePrimitiveType (){return (double)(int)m_curvePrimitive->GetCurvePrimitiveType ();}
    JsDPoint3dP PointAtFraction (double f)
        {
        DPoint3d xyz;
        if (m_curvePrimitive->FractionToPoint (f, xyz))
            {
            return new JsDPoint3d (xyz);
            }
        return nullptr;
        }

    JsDRay3dP PointAndDerivativeAtFraction (double f)
        {
        DPoint3d xyz;
        DVec3d derivative1;
        if (m_curvePrimitive->FractionToPoint (f, xyz, derivative1))
            {
            return new JsDRay3d (xyz, derivative1);
            }
        return nullptr;
        }

    JsDRange3dP Range ()
        {
        DRange3d range;
        m_curvePrimitive->GetRange (range);
        return new JsDRange3d (range);
        }
};

struct JsLineSegment : JsCurvePrimitive
{
public:
    JsLineSegment () {}
    JsLineSegment (ICurvePrimitivePtr const &data) {Set (data);}
    JsLineSegment (JsDPoint3dP pointA, JsDPoint3dP pointB)
        {
        ICurvePrimitivePtr cp = ICurvePrimitive::CreateLine (DSegment3d::From (pointA->Get (), pointB->Get ()));
        Set (cp);
        }
    JsLineSegment * Clone (){return new JsLineSegment (m_curvePrimitive->Clone ());}

};

struct JsEllipticArc: JsCurvePrimitive
{
public:
    JsEllipticArc () {}
    JsEllipticArc (ICurvePrimitivePtr const &data) {Set (data);}
    JsEllipticArc (JsDPoint3dP center, JsDVector3dP vector0, JsDVector3dP vector90, JsAngleP startAngle, JsAngleP sweepAngle)
        {
        ICurvePrimitivePtr cp = ICurvePrimitive::CreateArc 
            (DEllipse3d::FromVectors (center->Get (), vector0->Get (), vector90->Get (), startAngle->GetRadians (), sweepAngle->GetRadians ()));
        Set(cp);
        }

    JsEllipticArc * Clone (){return new JsEllipticArc (m_curvePrimitive->Clone ());}

JsDPoint3d * GetCenter()
    {
    DEllipse3d data;
    return m_curvePrimitive->TryGetArc (data) ? new JsDPoint3d (data.center) : nullptr;
    }

JsDVector3d * GetVector0()
    {
    DEllipse3d data;
    return m_curvePrimitive->TryGetArc (data) ? new JsDVector3d (data.vector0) : nullptr;
    }
JsDVector3d * GetVector90()
    {
    DEllipse3d data;
    return m_curvePrimitive->TryGetArc (data) ? new JsDVector3d (data.vector90) : nullptr;
    }
JsAngle * GetStartAngle ()
    {
    DEllipse3d data;
    return m_curvePrimitive->TryGetArc (data) ? new JsAngle (AngleInDegrees::FromRadians (data.start)) : nullptr;
    }
JsAngle * GetSweepAngle ()
    {
    DEllipse3d data;
    return m_curvePrimitive->TryGetArc (data) ? new JsAngle (AngleInDegrees::FromRadians (data.sweep)) : nullptr;
    }
};

END_BENTLEY_DGNPLATFORM_NAMESPACE

#endif//ndef _JSCURVEPRIMITIVE_H_

