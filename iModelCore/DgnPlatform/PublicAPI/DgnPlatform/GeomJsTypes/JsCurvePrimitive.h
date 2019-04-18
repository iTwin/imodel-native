/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
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
    JsCurvePrimitiveP Clone () override {return new JsCurvePrimitive (m_curvePrimitive->Clone ());} 

    ICurvePrimitivePtr GetICurvePrimitivePtr () override {return m_curvePrimitive;}
    JsCurvePrimitiveP AsCurvePrimitive () override {return this;}
    IGeometryPtr GetIGeometryPtr () override {return IGeometry::Create (m_curvePrimitive);}

    // Return the native ICurvePrimitive wrapped as the strongest Js type possible.
    // optionally let child CurveVector return as (true,false)==>(nullptr, JsCurvePrimitive)
    static JsCurvePrimitiveP StronglyTypedJsCurvePrimitive (ICurvePrimitivePtr const &data, bool nullifyCurveVector);


    double CurvePrimitiveType (){return (double)(int)m_curvePrimitive->GetCurvePrimitiveType ();}
    JsTransformP FrenetFrameAtFraction (double f)
        {
        Transform transform;
        if (m_curvePrimitive->FractionToFrenetFrame (f, transform))
            {
            return new JsTransform (transform);
            }
        return nullptr;
        }

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

    JsDRay3dP PointAndUnitTangentAtFraction (double f)
        {
        auto ray = m_curvePrimitive->FractionToPointAndUnitTangent (f);
        return ray.IsValid () ? new JsDRay3d (ray.Value ()) : nullptr;
        }

    JsDPoint3dP GetStartPoint () { return PointAtFraction (0.0);}
    JsDPoint3dP GetEndPoint () { return PointAtFraction (1.0);}


    double Length ()
        {
        double a;
        if (m_curvePrimitive->Length (a))
            return a;
        return 0.0;
        }
    JsDRange3dP Range () override
        {
        DRange3d range;
        m_curvePrimitive->GetRange (range);
        return new JsDRange3d (range);
        }
    JsDRange3dP RangeAfterTransform (JsTransformP transformP) override
        {
        Transform transform = transformP->Get();
        DRange3d range;
        m_curvePrimitive->GetRange (range, transform);
        return new JsDRange3d (range);
        }

     bool TryTransformInPlace (JsTransformP jsTransform) override
        {
        Transform transform = jsTransform->Get ();
        return m_curvePrimitive->TransformInPlace (transform);
        }

     bool IsSameStructureAndGeometry (JsGeometryP other) override
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

     bool IsSameStructure (JsGeometryP other) override
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
    // FORWARD DECL -- CurveLocationDetail needed.
    virtual JsCurveLocationDetailP ClosestPointBounded (JsDPoint3dP spacePoint);
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

    static JsLineSegmentP CreateXYZ (double ax, double ay, double az, double bx, double by, double bz)
        {
        return new JsLineSegment (
            ICurvePrimitive::CreateLine (DSegment3d::From (ax, ay, az, bx, by, bz)));
        }
    JsLineSegment * Clone () override {return new JsLineSegment (m_curvePrimitive->Clone ());}

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
    JsDPoint3dArrayP GetPoints () const {return JsDPoint3dArray::Create (m_curvePrimitive->GetLineStringP ());}

    JsLineString * Clone () override {return new JsLineString (m_curvePrimitive->Clone ());}

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

    DEllipse3d GetDEllipse3d () const
        {
        DEllipse3d arc;
        m_curvePrimitive->TryGetArc (arc);
        return arc;
        }
    JsEllipticArc * Clone () override {return new JsEllipticArc (m_curvePrimitive->Clone ());}

    static JsEllipticArc *CreateCircleXY (JsDPoint3dP center, double radius)
        {
        return new JsEllipticArc (DEllipse3d::FromCenterRadiusXY (center->Get (), radius));
        }

    static JsEllipticArc *CreateCircleStartMidEnd (JsDPoint3dP pointA, JsDPoint3dP pointB, JsDPoint3dP pointC)
        {
        return new JsEllipticArc (DEllipse3d::FromPointsOnArc (pointA->Get (), pointB->Get (), pointC->Get ()));
        }
    static JsEllipticArc *CreateFilletAtMiddlePoint (JsDPoint3dP pointA, JsDPoint3dP pointB, JsDPoint3dP pointC, double radius)
        {
        return new JsEllipticArc (DEllipse3d::FromFilletInCorner (pointA->Get (), pointB->Get (), pointC->Get (), radius));
        }
    static JsEllipticArc *CreateStartTangentNormalRadiusSweep (JsDPoint3dP startPoint, JsDVector3dP startTangent, JsDVector3dP planeNormal, double radius, JsAngleP sweepAngle)
        {
        return new JsEllipticArc (DEllipse3d::FromStartTangentNormalRadiusSweep
                    (
                    startPoint->Get (), startTangent->Get (), planeNormal->Get (), radius, sweepAngle->GetRadians ()
                    ));
        }

    static JsEllipticArc *CreateLargestFilletAtMiddlePoint (JsDPoint3dP pointA, JsDPoint3dP pointB, JsDPoint3dP pointC)
        {
        return new JsEllipticArc (DEllipse3d::FromFilletInBoundedCorner (pointA->Get (), pointB->Get (), pointC->Get ()));
        }


    JsTransformP CenterFrameAtFraction (double fraction) const
        {
        DEllipse3d arc;
        if (m_curvePrimitive->TryGetArc (arc))
            {
            auto frame = arc.FractionToCenterFrame (fraction);
            if (frame.IsValid ())
                return new JsTransform (frame.Value ());
            }
        return nullptr;
        }
    //! return the complement of this arc.  Returns null if this arc is a full 360 degree sweep.
    JsEllipticArc *Complement () const;

    // Construct (possbily many) tangent arcs as viewed in XY
    // Returned arcs are always the smaller of two possible arcs.
    static JsUnstructuredCurveVector *CreateXYTangentArcs (JsCurvePrimitiveP curveA, bool extendA, JsCurvePrimitiveP curveB, bool extendB);

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

bool IsCircular () const
    {
    DEllipse3d data;
    double radius;
    return m_curvePrimitive->TryGetArc (data) ? data.IsCircular (radius): false;
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
    JsBsplineCurve * Clone () override {return new JsBsplineCurve (m_curvePrimitive->Clone ());}

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
    JsCatenaryCurve * Clone () override {return new JsCatenaryCurve (m_curvePrimitive->Clone ());}

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

//=======================================================================================
// @bsiclass                                                    Eariln.Lutz     08/15
//=======================================================================================
struct JsSpiralCurve: JsCurvePrimitive
{
private:
public:
    JsSpiralCurve () {}
    JsSpiralCurve (ICurvePrimitivePtr const &data) {Set (data);}
    JsSpiralCurve * Clone () override {return new JsSpiralCurve (m_curvePrimitive->Clone ());}

    //! Create a spiral with radius and curvature at start, length along the spiral, and radius at end.
    //! @param spiralType [in] transition selector -- see SpiralCurve summary
    //! @param startBearing [in] direction (in xy plane) at the start of the curve.
    //! @param startRadius [in] radius of curvature at start.  (0 radius is straight line)
    //! @param length [in] length of full spiral curve
    //! @param endRadius [in] radius at end of the full spiral
    //! @param frame [in] coordinate frame with origin at start of full spiral, bearing angle 0 along x axis.
    //! @param fractionA [in] fractional position (distance along) at start of active subset of the spiral.  This is nearly always 0.0.
    //! @param fractionB [in] fractional position (distance along) at end of active subset of the spiral.  This is nearly always 1.0.
    static JsSpiralCurve * CreateSpiralBearingRadiusLengthRadius (
        double spiralType,
        JsAngleP startBearing,
        double   startRadius,
        double length,
        double endRadius,
        JsTransformP frame,
        double fractionA,
        double fractionB
        )
        {
        auto nativePrimitive = ICurvePrimitive::CreateSpiralBearingRadiusLengthRadius (
                            (int)spiralType,
                            startBearing->GetRadians (),
                            startRadius,
                            length,
                            endRadius,
                            frame->Get (),
                            fractionA, fractionB);
        if (nativePrimitive.IsValid ())
            {
            return new JsSpiralCurve (nativePrimitive);
            }
        return nullptr;
        }
};



END_BENTLEY_DGNPLATFORM_NAMESPACE

#endif//ndef _JSCURVEPRIMITIVE_H_

