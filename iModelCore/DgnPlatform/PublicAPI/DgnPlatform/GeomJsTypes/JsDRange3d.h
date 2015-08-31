/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/DgnPlatform/GeomJsTypes/JsDRange3d.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
//__BENTLEY_INTERNAL_ONLY__
// This file is included into GeomJsApi.h and relies on it for #include and various JsXxx classes.
#pragma once

#ifndef _JSDRANGE3D_H_
#define _JSDRANGE3D_H_

BEGIN_BENTLEY_DGNPLATFORM_NAMESPACE

//=======================================================================================
// @bsiclass                                                    Sam.Wilson      06/15
//=======================================================================================
struct JsDRange3d : JsGeomWrapperBase<DRange3d>
{
public:
    JsDRange3d() {m_data.Init ();}
    JsDRange3d(DRange3dCR data) { m_data = data;}

    JsDRange3dP Clone (){return new JsDRange3d (m_data);}

    JsDPoint3dP GetLow() {return new JsDPoint3d (m_data.low);}
    JsDPoint3dP GetHigh() {return new JsDPoint3d (m_data.high);}
    void SetLow (JsDPoint3dP point) {m_data.low = point->Get ();}
    void SetHigh (JsDPoint3dP point) {m_data.high = point->Get ();}

    bool IsNull (){return m_data.IsNull ();}
    bool IsSinglePoint (){return m_data.IsPoint ();}

    double XLength (){return m_data.XLength ();}
    double YLength (){return m_data.XLength ();}
    double ZLength (){return m_data.XLength ();}

    double MaxAbs (){return m_data.MaxAbs ();}
    bool IsAlmostZeroX (){return m_data.IsAlmostZeroX ();}
    bool IsAlmostZeroY (){return m_data.IsAlmostZeroY ();}
    bool IsAlmostZeroZ (){return m_data.IsAlmostZeroZ ();}

    bool ContainsXYZ (double x, double y, double z){return m_data.IsContained (x,y,z);}
    bool ContainsPoint (JsDPoint3dP point){return m_data.IsContained (point->Get ());}
    bool ContainsPointPointXY (JsDPoint3dP point){return m_data.IsContainedXY (point->Get ());}
    bool ContainsRange (JsDRange3dP other){return other->m_data.IsContained (m_data);}

    bool IntersectsRange (JsDRange3dP other){return m_data.IntersectsWith (other->m_data);}

    // policy: only return simple distances (no squared -- sqrt time will not show up in ts)
    double DistanceToPoint (JsDPoint3dP point){return m_data.DistanceOutside (point->Get ());}
    double DistanceToRange (JsDRange3dP other){return sqrt (m_data.DistanceSquaredTo (other->m_data));}


    void ExtendAllDirections (double a){m_data.Extend (a);}
    void ExtendXYZ (double x, double y, double z){m_data.Extend (x,y,z);}
    void ExtendPoint (JsDPoint3dP point){m_data.Extend (point->Get ());}
    void ExtendRange (JsDRange3dP range) {m_data.Extend (range->m_data);}

    JsDRange3dP Intersect (JsDRange3dP other)
        {
        DRange3d result;
        result.IntersectionOf (m_data, other->m_data);
        return new JsDRange3d (result);
        }
    JsDRange3dP Union (JsDRange3dP other)
        {
        DRange3d result;
        result.UnionOf (m_data, other->m_data);
        return new JsDRange3d (result);
        }

    void Init (){m_data.Init ();}



};
END_BENTLEY_DGNPLATFORM_NAMESPACE

#endif//ndef _JSDRANGE3D_H_

