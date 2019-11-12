/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
//__BENTLEY_INTERNAL_ONLY__
// This file is included into GeomJsApi.h and relies on it for #include and various JsXxx classes.
#pragma once

#ifndef _JsDgnXXXDetail_H_
#define _JsDgnXXXDetail_H_

BEGIN_BENTLEY_DGNPLATFORM_NAMESPACE


//=======================================================================================
// @bsiclass                                                    Eariln.Lutz     12/15
//=======================================================================================
struct JsSolidPrimitive : JsGeometry
{
private:
ISolidPrimitivePtr m_solidPrimitive;
// initialize with nullptr.   This should never be called -- maybe needed for compile/link?
public:
JsSolidPrimitive (ISolidPrimitivePtr const &solidPrimitive) : m_solidPrimitive (solidPrimitive){}
// hmm ...this class is not instantiable but typescript wrapperw want it to be so ...
JsSolidPrimitiveP Clone () override;
static JsSolidPrimitiveP StronglyTypedJsSolidPrimitive (ISolidPrimitivePtr primitive);

    JsSolidPrimitive (){}


    JsSolidPrimitiveP AsSolidPrimitive () override {return this;}
    IGeometryPtr GetIGeometryPtr () override {return IGeometry::Create (m_solidPrimitive);}

    virtual JsDgnConeP AsDgnCone () {return nullptr;}
    virtual JsDgnSphereP AsDgnSphere () {return nullptr;}
    virtual JsDgnBoxP AsDgnBox () {return nullptr;}
    virtual JsDgnTorusPipeP AsDgnTorusPipe () {return nullptr;}
    virtual JsDgnExtrusionP AsDgnExtrusion () {return nullptr;}
    virtual JsDgnRotationalSweepP AsDgnRotationalSweep () {return nullptr;}
    virtual JsDgnRuledSweepP AsDgnRuledSweepDgn () {return nullptr;}

    double SolidPrimitiveType (){return (double)m_solidPrimitive->GetSolidPrimitiveType ();}
    ISolidPrimitivePtr GetISolidPrimitivePtr() override {return m_solidPrimitive;}

    JsDRange3dP RangeAfterTransform (JsTransformP jsTransform) override
        {
        DRange3d range;
        Transform transform = jsTransform->Get ();
        m_solidPrimitive->GetRange (range, transform);
        return new JsDRange3d (range);
        }
    JsDRange3dP Range () override
        {
        DRange3d range;
        m_solidPrimitive->GetRange (range);
        return new JsDRange3d (range);
        }
     bool TryTransformInPlace (JsTransformP jsTransform) override
        {
        Transform transform = jsTransform->Get ();
        return m_solidPrimitive->TransformInPlace (transform);
        }
     bool IsSameStructureAndGeometry (JsGeometryP other) override
        {
        ISolidPrimitivePtr otherPrimitive;
        if (other != nullptr
            && (otherPrimitive = other->GetISolidPrimitivePtr (), otherPrimitive.IsValid ())       // COMMA
            )
            {
            return m_solidPrimitive->IsSameStructureAndGeometry (*otherPrimitive);
            }
        return false;
        }

     bool IsSameStructure (JsGeometryP other) override
        {
        ISolidPrimitivePtr otherPrimitive;
        if (other != nullptr
            && (otherPrimitive = other->GetISolidPrimitivePtr (), otherPrimitive.IsValid ())       // COMMA
            )
            {
            return m_solidPrimitive->IsSameStructure (*otherPrimitive);
            }
        return false;
        }


};

//=======================================================================================
// @bsiclass                                                    Eariln.Lutz     12/15
//=======================================================================================
struct JsDgnCone : JsSolidPrimitive
{
public:
JsDgnCone (ISolidPrimitivePtr const &solid) : JsSolidPrimitive (solid) {}
JsSolidPrimitiveP Clone () override {return new JsDgnCone (GetISolidPrimitivePtr ()->Clone ());}


JsDgnConeP AsDgnCone () override {return this;}
static JsDgnConeP CreateCircularCone (JsDPoint3dP pointA, JsDPoint3dP pointB, double radiusA, double radiusB, bool capped)
    {
    DgnConeDetail coneData (pointA->Get (), pointB->Get (), radiusA, radiusB, capped);
    auto solid = ISolidPrimitive::CreateDgnCone (coneData);
    return new JsDgnCone (solid);
    }

static JsDgnConeP CreateCircularConeXYZ (double ax, double ay, double az, double bx, double by, double bz, double radiusA, double radiusB, bool capped)
    {
    DgnConeDetail coneData (DPoint3d::From (ax, ay, az), DPoint3d::From (bx, by, bz), radiusA, radiusB, capped);
    auto solid = ISolidPrimitive::CreateDgnCone (coneData);
    return new JsDgnCone (solid);
    }


};

//=======================================================================================
// @bsiclass                                                    Eariln.Lutz     12/15
//=======================================================================================
struct JsDgnSphere: JsSolidPrimitive
{

public:
JsDgnSphere (ISolidPrimitivePtr const &solid) : JsSolidPrimitive (solid) {}
JsSolidPrimitiveP Clone () override {return new JsDgnSphere (GetISolidPrimitivePtr ()->Clone ());}
JsDgnSphereP AsDgnSphere () override {return this;}

static JsDgnSphereP CreateSphere (JsDPoint3dP center, double radius)
    {
    DgnSphereDetail data (center->Get (), radius);
    return new JsDgnSphere (ISolidPrimitive::CreateDgnSphere (data));
    }

};

//=======================================================================================
// @bsiclass                                                    Eariln.Lutz     12/15
//=======================================================================================
struct JsDgnTorusPipe: JsSolidPrimitive
{
public:
JsDgnTorusPipe (ISolidPrimitivePtr const &solid) : JsSolidPrimitive (solid) {}
JsSolidPrimitiveP Clone () override {return new JsDgnTorusPipe (GetISolidPrimitivePtr ()->Clone ());}
JsDgnTorusPipeP AsDgnTorusPipe () override {return this;}

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

static JsDgnTorusPipeP CreateFromArc (
        JsEllipticArcP arc,
        double minorRadius,
        bool capped
        )
    {
    DgnTorusPipeDetail data (arc->GetDEllipse3d (),
            minorRadius,
            capped
            );
    return new JsDgnTorusPipe (ISolidPrimitive::CreateDgnTorusPipe (data));
    }


};


//=======================================================================================
// @bsiclass                                                    Eariln.Lutz     12/15
//=======================================================================================
struct JsDgnBox: JsSolidPrimitive
{
public:
    JsDgnBox () {}

    JsDgnBox (ISolidPrimitivePtr const &solid) : JsSolidPrimitive (solid) {}
    JsSolidPrimitiveP Clone () override {return new JsDgnBox (GetISolidPrimitivePtr ()->Clone ());}
    JsDgnBoxP AsDgnBox () override {return this;}



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

    static JsDgnBoxP CreateCenteredBox (JsDPoint3dP center, JsDVector3dP diagonalSize, bool capped)
        {
        auto data =  DgnBoxDetail::InitFromCenterAndSize(center->Get(), diagonalSize->Get(), capped);
        return new JsDgnBox (ISolidPrimitive::CreateDgnBox (data));
        }

};

//=======================================================================================
// @bsiclass                                                    Eariln.Lutz     12/15
//=======================================================================================
struct JsDgnExtrusion: JsSolidPrimitive
{
public:
    JsDgnExtrusion () {}

    JsDgnExtrusion (ISolidPrimitivePtr const &solid) : JsSolidPrimitive (solid) {}
    JsSolidPrimitiveP Clone () override {return new JsDgnExtrusion (GetISolidPrimitivePtr ()->Clone ());}
    JsDgnExtrusionP AsDgnExtrusion () override {return this;}

    static JsDgnExtrusionP Create (
            JsCurveVectorP profile,
            JsDVector3dP   vector,
            bool capped
            )
        {
        auto cv = profile->GetCurveVectorPtr ();
        DgnExtrusionDetail data (
                profile->GetCurveVectorPtr (),
                vector->Get (),
                capped
                );
        return new JsDgnExtrusion (ISolidPrimitive::CreateDgnExtrusion (data));
        }

};

//=======================================================================================
// @bsiclass                                                    Eariln.Lutz     12/15
//=======================================================================================
struct JsDgnRotationalSweep: JsSolidPrimitive
{
public:
    JsDgnRotationalSweep () {}

    JsDgnRotationalSweep (ISolidPrimitivePtr const &solid) : JsSolidPrimitive (solid) {}
    JsSolidPrimitiveP Clone () override {return new JsDgnRotationalSweep (GetISolidPrimitivePtr ()->Clone ());}
    JsDgnRotationalSweepP AsDgnRotationalSweep () override {return this;}


    static JsDgnRotationalSweepP Create (
            JsCurveVectorP profile,
            JsDPoint3dP    center,
            JsDVector3dP   axis,
            JsAngleP       sweep,
            bool capped
            )
        {
        auto cv = profile->GetCurveVectorPtr ();
        DgnRotationalSweepDetail data (
                profile->GetCurveVectorPtr (),
                center->Get (), axis->Get (), sweep->GetRadians (),
                capped
                );
        return new JsDgnRotationalSweep (ISolidPrimitive::CreateDgnRotationalSweep (data));
        }

};

//=======================================================================================
// @bsiclass                                                    Eariln.Lutz     12/15
//=======================================================================================
struct JsDgnRuledSweep: JsSolidPrimitive
{
public:
    JsDgnRuledSweep () {}

    JsDgnRuledSweep (ISolidPrimitivePtr const &solid) : JsSolidPrimitive (solid) {}
    JsSolidPrimitiveP Clone () override {return new JsDgnRuledSweep (GetISolidPrimitivePtr ()->Clone ());}
    JsDgnRuledSweepP AsDgnRuledSweepDgn () override {return this;}

    static JsDgnRuledSweepP Create (
            JsUnstructuredCurveVectorP profiles,
            bool capped
            )
        {
        bvector<CurveVectorPtr> profileA;
        auto cv = profiles->GetCurveVectorPtr ();
        for (auto &child: *cv)
            {
            CurveVectorPtr childCV = child->GetChildCurveVectorP ();
            if (!childCV.IsValid ())
                return nullptr;
            profileA.push_back (childCV);
            }

        DgnRuledSweepDetail data (profileA, capped);
        return new JsDgnRuledSweep (ISolidPrimitive::CreateDgnRuledSweep (data));
        }

};

END_BENTLEY_DGNPLATFORM_NAMESPACE

#endif//ndef _JsDgnXXXDetail_H_

