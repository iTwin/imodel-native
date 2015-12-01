/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/DgnPlatform/GeomJsTypes/JsDgnXXXDetail.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
//__BENTLEY_INTERNAL_ONLY__
// This file is included into GeomJsApi.h and relies on it for #include and various JsXxx classes.
#pragma once

#ifndef _JsDgnXXXDetail_H_
#define _JsDgnXXXDetail_H_

BEGIN_BENTLEY_DGNPLATFORM_NAMESPACE

//=======================================================================================
// @bsiclass                                                    Eariln.Lutz     11/15
//=======================================================================================
struct JsDgnConeDetail: JsGeomWrapperBase <DgnConeDetail>
{
public:
    JsDgnConeDetail () {}

    JsDgnConeDetail (DgnConeDetailCR data)   { m_data = data;}

    static JsDgnConeDetailP CreateCircularCone (JsDPoint3dP pointA, JsDPoint3dP pointB, double radiusA, double radiusB, bool capped)
        {
        DgnConeDetail data (pointA->Get (), pointB->Get (), radiusA, radiusB, capped);
        return new JsDgnConeDetail (data);
        }

};


//=======================================================================================
// @bsiclass                                                    Eariln.Lutz     11/15
//=======================================================================================
struct JsDgnSphereDetail: JsGeomWrapperBase <DgnSphereDetail>
{
public:
    JsDgnSphereDetail () {}

    JsDgnSphereDetail (DgnSphereDetailCR data)   { m_data = data;}

    static JsDgnSphereDetailP CreateSphere (JsDPoint3dP center, double radius)
        {
        DgnSphereDetail data (center->Get (), radius);
        return new JsDgnSphereDetail (data);
        }

};

//=======================================================================================
// @bsiclass                                                    Eariln.Lutz     11/15
//=======================================================================================
struct JsDgnTorusPipeDetail: JsGeomWrapperBase <DgnTorusPipeDetail>
{
public:
    JsDgnTorusPipeDetail () {}

    JsDgnTorusPipeDetail (DgnTorusPipeDetailCR data)   { m_data = data;}

    static JsDgnTorusPipeDetailP CreateTorusPipe (
            JsDPoint3dP center,
            JsDVector3dP unitX,
            JsDVector3dP unitY,
            double majorRadius,
            double minorRadius,
            JsAngleP sweep,
            bool capped
            )
        {
        DgnTorusPipeDetail data (center->Get (),
                unitX->Get (), unitY->Get (),
                majorRadius, minorRadius,
                sweep->GetRadians (),
                capped
                );
        return new JsDgnTorusPipeDetail (data);
        }

};

//=======================================================================================
// @bsiclass                                                    Eariln.Lutz     11/15
//=======================================================================================
struct JsDgnBoxDetail: JsGeomWrapperBase <DgnBoxDetail>
{
public:
    JsDgnBoxDetail () {}

    JsDgnBoxDetail (DgnBoxDetailCR data)   { m_data = data;}

    static JsDgnBoxDetailP CreateBox (
            JsDPoint3dP baseOrigin,
            JsDPoint3dP topOrigin,
            JsDVector3dP unitX,
            JsDVector3dP unitY,
            double baseX,
            double baseY,
            double topX,
            double topY,
            bool capped
            )
        {
        DgnBoxDetail data (baseOrigin->Get (), topOrigin->Get (),
                unitX->Get (), unitY->Get (),
                baseX, baseY,
                topX, topY,
                capped
                );
        return new JsDgnBoxDetail (data);
        }
};

struct JsSolidPrimitive : RefCountedBase
{
private:
ISolidPrimitivePtr m_solidPrimitive;
// initialize with nullptr.   This should never be called -- maybe needed for compile/link?

public:
    JsSolidPrimitive (){}
    JsSolidPrimitive (ISolidPrimitivePtr const &solidPrimitive) : m_solidPrimitive (solidPrimitive){}

    static JsSolidPrimitiveP CreateDgnCone (JsDgnConeDetailP detail)
        {return new JsSolidPrimitive (ISolidPrimitive::CreateDgnCone (detail->GetCR ()));}

    static JsSolidPrimitiveP CreateDgnSphere (JsDgnSphereDetailP detail)
        {return new JsSolidPrimitive (ISolidPrimitive::CreateDgnSphere (detail->GetCR ()));}

    static JsSolidPrimitiveP CreateDgnBox (JsDgnBoxDetailP detail)
        {return new JsSolidPrimitive (ISolidPrimitive::CreateDgnBox (detail->GetCR ()));}

    static JsSolidPrimitiveP CreateDgnTorusPipe (JsDgnTorusPipeDetailP detail)
        {return new JsSolidPrimitive (ISolidPrimitive::CreateDgnTorusPipe (detail->GetCR ()));}

    JsSolidPrimitiveP Clone ()
        {
        auto clone = m_solidPrimitive->Clone ();
        return new JsSolidPrimitive (clone);
        }

    double SolidPrimitiveType (){return (double)m_solidPrimitive->GetSolidPrimitiveType ();}
};



END_BENTLEY_DGNPLATFORM_NAMESPACE

#endif//ndef _JsDgnXXXDetail_H_

#ifdef abc





#endif