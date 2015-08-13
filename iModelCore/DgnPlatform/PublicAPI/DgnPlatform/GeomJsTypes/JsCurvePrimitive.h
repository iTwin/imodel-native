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
    ICurvePrimitivePtr m_curvePrimitive;
    JsCurvePrimitive () {}

    JsCurvePrimitive (ICurvePrimitivePtr curvePrimitive) : m_curvePrimitive (curvePrimitive) {}

    static JsCurvePrimitiveP CreateLineSegment (JsDSegment3dP segment)
        {
        ICurvePrimitivePtr cp = ICurvePrimitive::CreateLine (segment->m_segment);
        return new JsCurvePrimitive (cp);
        }
    
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
};

END_BENTLEY_DGNPLATFORM_NAMESPACE

#endif//ndef _JSCURVEPRIMITIVE_H_

