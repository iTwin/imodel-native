/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/DgnPlatform/GeomJsTypes/JsCurvePrimitive.h $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
//__BENTLEY_INTERNAL_ONLY__
// This file is included into GeomJsApi.h and relies on it for #include and various JsXxx classes.
#pragma once

#ifndef _JSCURVEPRIMITIVE_H_
#define _JSCURVEPRIMITIVE_H_

BEGIN_BENTLEY_DGNPLATFORM_NAMESPACE
// CloneBetewwenFractions -- can implement in base class, but needs upcast to specialized type.
//   REMARK -- upcast needs to be implemented recursively !!!
// Type signature methods . ..
//      GetCurvePrimitiveType
//      GetLine,
//      GetLineString
//      GetLineStringP
//      GetArc
//      GetBsplineCurve
//      GetBsplineCurvePtr
//      getProxyBSplineCurve
//      GetProxyBsplineCurvePtr
//      GetInterpolationCurveCP
//      GetAkimaCurve
//
//

//=======================================================================================
// @bsiclass                                                    Eariln.Lutz     08/15
//=======================================================================================
struct JsCurvePrimitive: JsGeometry
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
    // This should never get called -- derived classes will get the virtual dispatch . . 
    virtual JsCurvePrimitiveP Clone () override {return new JsCurvePrimitive (m_curvePrimitive->Clone ());} 

    virtual ICurvePrimitivePtr GetICurvePrimitivePtr () override {return m_curvePrimitive;}
    virtual JsCurvePrimitiveP AsCurvePrimitive () override {return this;}

    // Return the native ICurvePrimitive wrapped as the strongest Js type possible.
    // optionally let child CurveVector return as (true,false)==>(nullptr, JsCurvePrimitive)
    static JsCurvePrimitiveP StronglyTypedJsCurvePrimitive (ICurvePrimitivePtr &data, bool nullifyCurveVector);


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

    double Length ()
        {
        double a;
        if (m_curvePrimitive->Length (a))
            return a;
        return 0.0;
        }
    virtual JsDRange3dP Range () override
        {
        DRange3d range;
        m_curvePrimitive->GetRange (range);
        return new JsDRange3d (range);
        }
    virtual JsDRange3dP RangeAfterTransform (JsTransformP transformP) override
        {
        Transform transform = transformP->Get();
        DRange3d range;
        m_curvePrimitive->GetRange (range, transform);
        return new JsDRange3d (range);
        }

     virtual bool TryTransformInPlace (JsTransformP jsTransform) override
        {
        Transform transform = jsTransform->Get ();
        return m_curvePrimitive->TransformInPlace (transform);
        }

     virtual bool IsSameStructureAndGeometry (JsGeometryP other) override
        {
        ICurvePrimitivePtr otherPrimitive;
        if (other != nullptr
            && (otherPrimitive = other->GetICurvePrimitivePtr (), otherPrimitive.IsValid ())       // COMMA
            )
            {
            return m_curvePrimitive->IsSameStructureAndGeometry (*otherPrimitive);
            }
        return false;
        }

     virtual bool IsSameStructure (JsGeometryP other) override
        {
        ICurvePrimitivePtr otherPrimitive;
        if (other != nullptr
            && (otherPrimitive = other->GetICurvePrimitivePtr (), otherPrimitive.IsValid ())       // COMMA
            )
            {
            return m_curvePrimitive->IsSameStructure (*otherPrimitive);
            }
        return false;
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
    virtual JsLineSegment * Clone () override {return new JsLineSegment (m_curvePrimitive->Clone ());}

};

struct JsLineString : JsCurvePrimitive
{
public:
    JsLineString () {}
    JsLineString (ICurvePrimitivePtr const &data) {Set (data);}
    JsLineString (JsDPoint3dArrayP points)
        {
        ICurvePrimitivePtr cp = ICurvePrimitive::CreateLineString (points->GetRef ());
        Set (cp);
        }
    virtual JsLineString * Clone () override {return new JsLineString (m_curvePrimitive->Clone ());}

};

struct JsEllipticArc: JsCurvePrimitive
{
public:
    JsEllipticArc () {}
    JsEllipticArc (ICurvePrimitivePtr const &data) {Set (data);}
    JsEllipticArc (DEllipse3dCR source)
        {
        ICurvePrimitivePtr cp = ICurvePrimitive::CreateArc (source);
        Set(cp);
        }

    JsEllipticArc (JsDPoint3dP center, JsDVector3dP vector0, JsDVector3dP vector90, JsAngleP startAngle, JsAngleP sweepAngle)
        {
        ICurvePrimitivePtr cp = ICurvePrimitive::CreateArc 
            (DEllipse3d::FromVectors (center->Get (), vector0->Get (), vector90->Get (), startAngle->GetRadians (), sweepAngle->GetRadians ()));
        Set(cp);
        }

    virtual JsEllipticArc * Clone () override {return new JsEllipticArc (m_curvePrimitive->Clone ());}

    static JsEllipticArc *CreateCircleXY (JsDPoint3dP center, double radius)
        {
        return new JsEllipticArc (DEllipse3d::FromCenterRadiusXY (center->Get (), radius));
        }

    static JsEllipticArc *CreateCircleStartMidEnd (JsDPoint3dP pointA, JsDPoint3dP pointB, JsDPoint3dP pointC)
        {
        return new JsEllipticArc (DEllipse3d::FromPointsOnArc (pointA->Get (), pointB->Get (), pointC->Get ()));
        }


JsDPoint3dDVector3dDVector3d * GetBasisPlane () const
    {
    DEllipse3d data;
    return m_curvePrimitive->TryGetArc (data)
            ? new JsDPoint3dDVector3dDVector3d (data.center, data.vector0, data.vector90)
            : nullptr;
    }

JsDPoint3d * GetCenter() const
    {
    DEllipse3d data;
    return m_curvePrimitive->TryGetArc (data) ? new JsDPoint3d (data.center) : nullptr;
    }

JsDVector3d * GetVector0() const
    {
    DEllipse3d data;
    return m_curvePrimitive->TryGetArc (data) ? new JsDVector3d (data.vector0) : nullptr;
    }
JsDVector3d * GetVector90() const
    {
    DEllipse3d data;
    return m_curvePrimitive->TryGetArc (data) ? new JsDVector3d (data.vector90) : nullptr;
    }
JsAngle * GetStartAngle () const
    {
    DEllipse3d data;
    return m_curvePrimitive->TryGetArc (data) ? new JsAngle (AngleInDegrees::FromRadians (data.start)) : nullptr;
    }
JsAngle * GetSweepAngle () const
    {
    DEllipse3d data;
    return m_curvePrimitive->TryGetArc (data) ? new JsAngle (AngleInDegrees::FromRadians (data.sweep)) : nullptr;
    }
JsAngle * GetEndAngle () const
    {
    DEllipse3d data;
    return m_curvePrimitive->TryGetArc (data) ? new JsAngle (AngleInDegrees::FromRadians (data.start + data.sweep)) : nullptr;
    }

JsEllipticArcP CloneWithPerpendicularAxes () const
    {
    DEllipse3d data;
    return m_curvePrimitive->TryGetArc (data) ? new JsEllipticArc (DEllipse3d::FromPerpendicularAxes (data)) : nullptr;
    }
};



//=======================================================================================
// @bsiclass                                                    Eariln.Lutz     08/15
//=======================================================================================
struct JsBsplineCurve: JsCurvePrimitive
{
private:
public:
    JsBsplineCurve () {}
    JsBsplineCurve (ICurvePrimitivePtr const &data) {Set (data);}
    virtual JsBsplineCurve * Clone () override {return new JsBsplineCurve (m_curvePrimitive->Clone ());}

    static JsBsplineCurve * Create (JsDPoint3dArrayP xyz, 
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
            {
            auto bcurvePrimitive = ICurvePrimitive::CreateBsplineCurve (bcurve);
            return new JsBsplineCurve (bcurvePrimitive);
            }
        return nullptr;
        }

    static JsBsplineCurve * CreateFromPoles (JsDPoint3dArrayP xyz, double order)
        {
        return Create (xyz, nullptr, nullptr, order, false, true);
        }

    bool IsPeriodic ()
        {
        auto data = m_curvePrimitive->GetBsplineCurvePtr ();
        return data.IsValid () ? data->IsClosed () : false;
        }
};


//=======================================================================================
// @bsiclass                                                    Eariln.Lutz     08/15
//=======================================================================================
struct JsCatenaryCurve: JsCurvePrimitive
{
private:
public:
    JsCatenaryCurve () {}
    JsCatenaryCurve (ICurvePrimitivePtr const &data) {Set (data);}
    virtual JsCatenaryCurve * Clone () override {return new JsCatenaryCurve (m_curvePrimitive->Clone ());}

    // Create a catenary curve:   xyz = origin + x * xVector + yVector * a * cosh(x/a)
    static JsCatenaryCurve * CreateFromCoefficientAndXLimits (JsDPoint3dP origin, JsDVector3dP xVector, JsDVector3dP yVector, double a, double xStart, double xEnd)
        {
        auto nativePrimitive = ICurvePrimitive::CreateCatenary (
                            a,
                            DPoint3dDVec3dDVec3d (origin->Get (), xVector->Get (), yVector->Get ()),
                            xStart,
                            xEnd);
        if (nativePrimitive.IsValid ())
            {
            return new JsCatenaryCurve (nativePrimitive);
            }
        return nullptr;
        }
};



END_BENTLEY_DGNPLATFORM_NAMESPACE

#endif//ndef _JSCURVEPRIMITIVE_H_

