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

#ifdef abc
//=======================================================================================
// @bsiclass                                                    Eariln.Lutz     11/15
//=======================================================================================
struct JsDgnExtrusionDetail: JsGeomWrapperBase <DgnExtrusionDetail>
{
public:
    JsDgnExtrusionDetail () {}

    JsDgnExtrusionDetail (DgnExtrusionDetailCR data)   { m_data = data;}

    static JsDgnExtrusionDetailP Create (JsCurveVectorP contour, JsDVector3dP vector, bool capped)
        {
        DgnExtrusionDetail data (contour->Get (), vector->Get (), capped);
        return new JsDgnExtrusionDetail (data);
        }

};
#endif

struct JsSolidPrimitive : JsGeometry
{
private:
ISolidPrimitivePtr m_solidPrimitive;
// initialize with nullptr.   This should never be called -- maybe needed for compile/link?

public:
    JsSolidPrimitive (){}
    JsSolidPrimitive (ISolidPrimitivePtr const &solidPrimitive) : m_solidPrimitive (solidPrimitive){}

    JsSolidPrimitiveP Clone ()
        {
        auto clone = m_solidPrimitive->Clone ();
        return new JsSolidPrimitive (clone);
        }

    double SolidPrimitiveType (){return (double)m_solidPrimitive->GetSolidPrimitiveType ();}
    ISolidPrimitivePtr GetISolidPrimitivePtr() {return m_solidPrimitive;}
};

struct JsDgnCone : JsSolidPrimitive
{
public:
JsDgnCone (ISolidPrimitivePtr const &solid) : JsSolidPrimitive (solid) {}

static JsDgnConeP CreateCircularCone (JsDPoint3dP pointA, JsDPoint3dP pointB, double radiusA, double radiusB, bool capped)
    {
    DgnConeDetail coneData (pointA->Get (), pointB->Get (), radiusA, radiusB, capped);
    auto solid = ISolidPrimitive::CreateDgnCone (coneData);
    return new JsDgnCone (solid);
    }

};

//=======================================================================================
// @bsiclass                                                    Eariln.Lutz     11/15
//=======================================================================================
struct JsDgnSphere: JsSolidPrimitive
{
public:
JsDgnSphere (ISolidPrimitivePtr const &solid) : JsSolidPrimitive (solid) {}

static JsDgnSphereP CreateSphere (JsDPoint3dP center, double radius)
    {
    DgnSphereDetail data (center->Get (), radius);
    return new JsDgnSphere (ISolidPrimitive::CreateDgnSphere (data));
    }

};

//=======================================================================================
// @bsiclass                                                    Eariln.Lutz     11/15
//=======================================================================================
struct JsDgnTorusPipe: JsSolidPrimitive
{
public:
JsDgnTorusPipe (ISolidPrimitivePtr const &solid) : JsSolidPrimitive (solid) {}

static JsDgnTorusPipeP CreateTorusPipe (
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
    return new JsDgnTorusPipe (ISolidPrimitive::CreateDgnTorusPipe (data));
    }

};


//=======================================================================================
// @bsiclass                                                    Eariln.Lutz     11/15
//=======================================================================================
struct JsDgnBox: JsSolidPrimitive
{
public:
    JsDgnBox () {}

    JsDgnBox (ISolidPrimitivePtr const &solid) : JsSolidPrimitive (solid) {}

    static JsDgnBoxP CreateBox (
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
        return new JsDgnBox (ISolidPrimitive::CreateDgnBox (data));
        }

    static JsDgnBoxP CreateBoxCentered (JsDPoint3dP center, JsDVector3dP diagonalSize, bool capped)
        {
        auto data =  DgnBoxDetail::InitFromCenterAndSize(center->Get(), diagonalSize->Get(), capped);
        return new JsDgnBox (ISolidPrimitive::CreateDgnBox (data));
        }

};
END_BENTLEY_DGNPLATFORM_NAMESPACE

#endif//ndef _JsDgnXXXDetail_H_

#ifdef abc





#endif