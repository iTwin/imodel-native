/*--------------------------------------------------------------------------------------+
|
|     $Source: geom/src/CurvePrimitive/CurvePrimitiveBsplineCurve.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <bsibasegeomPCH.h>

BEGIN_BENTLEY_GEOMETRY_NAMESPACE
double SetBCurveOffsetSign (double offsetDistance)
    {
#ifdef BCurveNeedsReversedOffsetSign
    return -offsetDistance;
#else
    return offsetDistance;
#endif
    }
/*=================================================================================**//**
* @bsiclass                                                     Brien.Bastings  02/10
+===============+===============+===============+===============+===============+======*/
struct CurvePrimitiveBsplineCurve : public ICurvePrimitive
{
protected:

MSBsplineCurvePtr  m_curve;


/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
explicit CurvePrimitiveBsplineCurve(MSBsplineCurveCR curve) {m_curve = curve.CreateCopy ();}
explicit CurvePrimitiveBsplineCurve(MSBsplineCurvePtr curve) {m_curve = curve;}
// For use by derived classes that need to be deeper in their constructor before really initializing the curve.
explicit CurvePrimitiveBsplineCurve (){m_curve = MSBsplineCurve::CreatePtr ();}

void ReplaceCurve (MSBsplineCurvePtr curve)
    {
    m_curve = curve;
    }


/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
ICurvePrimitivePtr _Clone() const override {return Create (*m_curve);}


/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
bool _IsMappableFractionSpace() const override {return true;}

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
bool _IsFractionSpace() const override {return true;}

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
ICurvePrimitivePtr _CloneBetweenFractions
(
double fractionA,
double fractionB,
bool allowExtrapolation
) const override
    {
    CurvePrimitiveBsplineCurve *newCurve = new CurvePrimitiveBsplineCurve (
            m_curve->CreateCopyBetweenFractions (fractionA, fractionB)
            );
    return newCurve;
    }

ICurvePrimitivePtr _CloneAsSingleOffsetPrimitiveXY (CurveOffsetOptionsCR options) const override 
    {
    double offsetDistance = options.GetOffsetDistance ();
    MSBsplineCurvePtr offsetBCurve = m_curve->CreateCopyOffsetXY (SetBCurveOffsetSign (offsetDistance), SetBCurveOffsetSign (offsetDistance), options);
    if (offsetBCurve.IsValid ())
        return new CurvePrimitiveBsplineCurve (offsetBCurve);
    return NULL;
    }

~CurvePrimitiveBsplineCurve () {}//  Ptr frees on its own ..... m_curve->ReleaseMem ();}


/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
CurvePrimitiveType _GetCurvePrimitiveType() const override {return CURVE_PRIMITIVE_TYPE_BsplineCurve;}

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
MSBsplineCurveCP _GetBsplineCurveCP() const override {return m_curve.get ();}
MSBsplineCurvePtr _GetBsplineCurvePtr() const override {return m_curve;}

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
MSBsplineCurveCP _GetProxyBsplineCurveCP() const        override {return m_curve.get ();}
MSBsplineCurvePtr _GetProxyBsplineCurvePtr() const        override {return m_curve;}

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
void _Process(ICurvePrimitiveProcessor &processor, DSegment1dCP interval) const override 
    {processor._ProcessBsplineCurve (*this, *m_curve, interval);}


/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
bool _IsPeriodicFractionSpace(double &period) const override
    {
    if (m_curve->params.closed)
        {
        period = 1.0;
        return true;
        }
    period = 0.0;
    return false;
    }


/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
bool _FractionToPoint(double fraction, DPoint3dR point) const override
        {
        m_curve->FractionToPoint (point, fraction);
        return true;
        }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
bool _FractionToPoint(double fraction, DPoint3dR point, DVec3dR tangent) const override
        {
        m_curve->FractionToPoint (point, tangent, fraction);
        return true;
        }
    

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
bool _FractionToPoint(double fraction, DPoint3dR point, DVec3dR tangent, DVec3dR derivative2) const override
        {
        DVec3d dervs[3];
        if (SUCCESS == m_curve->ComputeDerivatives (dervs, 2, fraction))
            {
            point = dervs[0];
            tangent = dervs[1];
            derivative2 = dervs[2];
            return true;
            }

        return false;
        }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
bool _FractionToPoint(double fraction, DPoint3dR point, DVec3dR tangent, DVec3dR derivative2, DVec3dR derivative3) const override
        {
        DVec3d dervs[4];
        if (SUCCESS == m_curve->ComputeDerivatives (dervs, 3, fraction))
            {
            point = dervs[0];
            tangent = dervs[1];
            derivative2 = dervs[2];
            derivative3 = dervs[3];
            return true;
            }

        return false;
        }


/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
bool _FractionToFrenetFrame(double f, TransformR frame) const override
        {
        double curvature, torsion;
        DPoint3d point;
        DVec3d tmb[3];
        if (SUCCESS == m_curve->GetFrenetFrame (tmb, point, curvature, torsion, f))
            {
            frame.InitFromOriginAndVectors (point, tmb[0], tmb[1], tmb[2]);
            return true;
            }
        
        return false;
        }


/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
bool _Length(double &length) const override
        {
        length = m_curve->Length ();
        return true;
        }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
bool _Length(RotMatrixCP worldToLocal, double &length) const override
        {
        length = m_curve->Length (worldToLocal);
        return true;
        }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
bool _FastLength(double &length) const override
        {
        length = m_curve->PolygonLength ();
        return true;
        }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
bool _GetRange(DRange3dR range) const override
        {
        range = m_curve->GetRange ();
        return true;
        }


/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
bool _GetRange(DRange3dR range, TransformCR transform) const override
        {
        MSBsplineCurve localCurve;
        localCurve.CopyTransformed (*m_curve, transform);
        range = localCurve.GetRange ();
        localCurve.ReleaseMem ();
        return true;
        }



/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
double _FastMaxAbs() const override
        {
        DRange3d poleRange;
        m_curve->GetPoleRange (poleRange);
        return poleRange.LargestCoordinate ();
        }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      06/2014
+--------------------------------------------------------------------------------------*/
bool _IsSameStructureAndGeometry (ICurvePrimitiveCR other, double tolerance) const override
    {
    MSBsplineCurveCP otherBCurve = other.GetBsplineCurveCP ();
    return NULL != otherBCurve && m_curve->AlmostEqual (*otherBCurve, tolerance);
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      11/2013
+--------------------------------------------------------------------------------------*/
size_t _NumComponent () const override {return 1;}

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
bool _GetBreakFraction(size_t breakFractionIndex, double &fraction) const override
        {
        size_t numPoles = m_curve->NumberAllocatedPoles (), order = m_curve->GetOrder (), maxIndex;
        maxIndex = numPoles - order + 1;

        if (breakFractionIndex <= maxIndex)
            {
            fraction = m_curve->GetKnot (int (breakFractionIndex + order - 1));
            return true;
            }
        else
            return false;
        }
    

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
bool _AdjustFractionToBreakFraction(double fraction, Rounding::RoundingMode mode, size_t &breakIndex, double &adjustedFraction) const override
        {
        double lowerBound, upperBound;
        size_t numPoles = m_curve->NumberAllocatedPoles (), order = m_curve->GetOrder (), maxIndex;
        maxIndex = numPoles - order + 1;

        if (DoubleOps::BoundingValues (&m_curve->knots[order-1], maxIndex + 1, fraction, breakIndex, lowerBound, upperBound))
            {
            adjustedFraction = Rounding::Round (fraction, mode, lowerBound, upperBound);
            if (adjustedFraction > lowerBound)
                breakIndex ++;
            return true;
            }
        else
            return false;
        }


/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
bool _GetMSBsplineCurve(MSBsplineCurveR curve, double fractionA, double fractionB) const override
        {
        return SUCCESS == curve.CopySegment (*m_curve,
            m_curve->FractionToKnot (fractionA), m_curve->FractionToKnot (fractionB));
        }


/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
bool _AddStrokes(bvector <DPoint3d> &points, IFacetOptionsCR options,
                bool includeStartPoint,
                double startFraction,
                double endFraction
                ) const override
        {
        MSBsplineCurve curve;
        if (!_GetMSBsplineCurve (curve, startFraction, endFraction))
            {
            curve.ReleaseMem ();
            return false;
            }

        curve.AddStrokes (points, options.GetChordTolerance (), options.GetAngleTolerance (), options.GetMaxEdgeLength (), includeStartPoint);
        curve.ReleaseMem ();
        return true;
        }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
bool _AddStrokes(bvector <PathLocationDetail> &points, IFacetOptionsCR options,
                double startFraction,
                double endFraction
                ) const override
    {
    bvector<DPoint3d> xyz;
    bvector<double> params;
    DSegment1d segment (startFraction, endFraction);
    if (segment.Is01 ())
        {
        m_curve->AddStrokes (options, xyz, nullptr, &params, true);
        }
    else
        {
        // ugh.  stroke in a subset curve, map parameters back . ..
        MSBsplineCurve curve;
        if (!_GetMSBsplineCurve (curve, startFraction, endFraction))
            {
            curve.ReleaseMem ();
            return false;
            }

        curve.AddStrokes (options, xyz, nullptr, &params, true);
        for (size_t i = 0; i < xyz.size (); i++)
            {
            params[i] = segment.FractionToPoint (params[i]);
            }
        curve.ReleaseMem ();
        }
    if (xyz.size () > 0)
        {
        double distanceAlong = points.empty () ? 0.0 : points.back ().DistanceFromPathStart ();
        for (size_t i = 0; i < xyz.size (); i++)
            {
            if (i > 0)
                distanceAlong += m_curve->LengthBetweenFractions (params[i-1], params[i]);
            points.push_back (PathLocationDetail
                (
                CurveLocationDetail (this, params[i], xyz[i]),
                0, distanceAlong
                ));
            }
        }
    return true;
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
size_t _GetStrokeCount(IFacetOptionsCR options, double startFraction, double endFraction) const override
    {
    MSBsplineCurve curve;
    if (!_GetMSBsplineCurve (curve, startFraction, endFraction))
        {
        curve.ReleaseMem ();
        return 0;
        }

    size_t count = options.BsplineCurveStrokeCount (curve);
    curve.ReleaseMem ();
    return count;
    }


/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
bool _SignedDistanceBetweenFractions(double startFraction, double endFraction, double &signedDistance) const override
        {
        signedDistance = fabs (m_curve->LengthBetweenFractions (startFraction, endFraction));
        if (endFraction < startFraction)
            signedDistance = - signedDistance;
        return true;
        }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
bool _SignedDistanceBetweenFractions(RotMatrixCP worldToLocal, double startFraction, double endFraction, double &signedDistance) const override
        {
        signedDistance = fabs (m_curve->LengthBetweenFractions (worldToLocal, startFraction, endFraction));
        if (endFraction < startFraction)
            signedDistance = - signedDistance;
        return true;
        }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
bool _PointAtSignedDistanceFromFraction(double startFraction, double signedDistance, bool allowExtension, CurveLocationDetailR location) const override
        {
        double endFraction;
        double actualDistance;
        m_curve->FractionAtSignedDistance (startFraction, signedDistance, endFraction, actualDistance);
        location = CurveLocationDetail (this, 1);
        location.SetSingleComponentFractionAndA (endFraction, actualDistance);
        m_curve->FractionToPoint (location.point, endFraction);
        return true;
        }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
bool _PointAtSignedDistanceFromFraction(RotMatrixCP worldToView, double startFraction, double signedDistance, bool allowExtension, CurveLocationDetailR location) const override
        {
        double endFraction;
        double actualDistance;
        m_curve->FractionAtSignedDistance (worldToView, startFraction, signedDistance, endFraction, actualDistance);
        location = CurveLocationDetail (this, 1);
        location.SetSingleComponentFractionAndA (endFraction, actualDistance);
        m_curve->FractionToPoint (location.point, endFraction);
        return true;
        }

using ICurvePrimitive::_ClosestPointBounded;    // suppresses C4266
/*--------------------------------------------------------------------------------**//**
  bcurve -- no extension considered.
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
bool _ClosestPointBounded (DPoint3dCR spacePoint, double &fraction, DPoint3dR curvePoint, bool extend0, bool extend1) const override
        {
        m_curve->ClosestPoint (curvePoint, fraction, spacePoint);
        return true;
        }


/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
bool _ClosestPointBoundedXY(
        DPoint3dCR spacePoint,
        DMatrix4dCP worldToLocal,
        CurveLocationDetailR location,
        bool extend0,
        bool extend1
        ) const override
        {
        location = CurveLocationDetail (this, 1);
        m_curve->ClosestPointXY (location.point, location.fraction, location.a, spacePoint, worldToLocal);
        location.SetSingleComponentData ();
        return true;
        }


/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
bool _TransformInPlace (TransformCR transform) override
        {
        m_curve->TransformCurve (transform);
        return true;
        }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
bool _ReverseCurvesInPlace () override
        {
        m_curve->MakeReversed ();
        return true;
        }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
bool _WireCentroid(
        double &length,
        DPoint3dR centroid,
        double fraction0,
        double fraction1
        ) const override
        {
        m_curve->WireCentroid (length, centroid, fraction0, fraction1);
        return true;
        }

public: 

static ICurvePrimitive* Create (MSBsplineCurveCR curve) {return new CurvePrimitiveBsplineCurve (curve);}

static ICurvePrimitive* Create (MSBsplineCurvePtr curve) {return new CurvePrimitiveBsplineCurve (curve);}

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
void _AppendCurvePlaneIntersections(DPlane3dCR plane, bvector<CurveLocationDetailPair> &intersections, double tol) const override 
    {AppendTolerancedPlaneIntersections (plane, this, *m_curve, intersections, tol);}

}; // CurvePrimitiveBsplineCurve
//#define CREATE_SPIRAL_CURVE CurvePrimitiveSpiralCurve::Create
#define CREATE_SPIRAL_CURVE CurvePrimitiveSpiralCurve1::Create
#include "cp_directSpiral.h"
#include "cp_spiral.h"
#include "cp_catenary.h"

#ifndef CurvePrimitive_NO_INTERPOLATINGCURVE
/*=================================================================================**//**
* @bsiclass                                                     Brien.Bastings  02/10
+===============+===============+===============+===============+===============+======*/
struct CurvePrimitiveInterpolationCurve : CurvePrimitiveBsplineCurve
{
typedef CurvePrimitiveBsplineCurve Super;
protected:

MSInterpolationCurve    m_fitCurve;


void FinishConstructor ()
    {
    if (NULL == m_fitCurve.fitPoints)
        return;

    MSBsplineCurve curve;

    if (SUCCESS != mdlBspline_convertInterpolationToBspline (&curve, &m_fitCurve))
        return;

    ReplaceCurve (curve.CreateCapture ());
    }

// Always clone fitCurve.
explicit CurvePrimitiveInterpolationCurve(MSInterpolationCurveCR fitCurve)
    : CurvePrimitiveBsplineCurve ()
    {
    m_fitCurve.CopyFrom (fitCurve);
    FinishConstructor ();
    }

// optionally 
// swapWithSource true:  copy bits from source to m_fitCurve, then Zero the source.
// swapWithSource false: clone the source
explicit CurvePrimitiveInterpolationCurve(MSInterpolationCurveR fitCurve, bool swapFromSource)
    : CurvePrimitiveBsplineCurve ()
    {
    if (swapFromSource)
        {
        m_fitCurve = fitCurve;
        fitCurve.Zero ();
        }
    else
        {
        m_fitCurve.CopyFrom (fitCurve);
        }

    FinishConstructor ();
    }

~CurvePrimitiveInterpolationCurve () {m_fitCurve.ReleaseMem ();}

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
ICurvePrimitivePtr _Clone() const override {return Create (m_fitCurve);}

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
MSBsplineCurveCP _GetBsplineCurveCP() const override {return NULL;}  // Users do not see the bspline !!
MSBsplineCurvePtr _GetBsplineCurvePtr() const override {return NULL;}  // Users do not see the bspline !!
CurvePrimitiveType          _GetCurvePrimitiveType () const override {return CURVE_PRIMITIVE_TYPE_InterpolationCurve;}

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
MSInterpolationCurveCP _GetInterpolationCurveCP() const override {return &m_fitCurve;}

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
void _Process(ICurvePrimitiveProcessor &processor, DSegment1dCP interval) const override 
    {
    processor._ProcessInterpolationCurve (*this, m_fitCurve, interval);
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      06/2014
+--------------------------------------------------------------------------------------*/
bool _IsSameStructureAndGeometry (ICurvePrimitiveCR other, double tolerance) const override
    {
    MSInterpolationCurveCP otherInterpolationCurve = other.GetInterpolationCurveCP ();
    return NULL != otherInterpolationCurve && m_fitCurve.AlmostEqual (*otherInterpolationCurve, tolerance);
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
bool _TransformInPlace (TransformCR transform) override
    {
    // Hmmmm.. Do the primary and bspline transformations really apply in parallel? Should the bspline be regenerated fromr
    //  transformed primary?  Dunno.
    Super::_TransformInPlace (transform);
    bspcurv_transformInterpolationCurve (&m_fitCurve, &m_fitCurve, &transform);
    return true;
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
bool _ReverseCurvesInPlace () override
    {
    Super::_ReverseCurvesInPlace ();
    return SUCCESS == mdlBspline_reverseInterpolationCurve (&m_fitCurve, &m_fitCurve);
    }


public: 

static ICurvePrimitive* Create (MSInterpolationCurveCR fitCurve)
    {
    return new CurvePrimitiveInterpolationCurve (fitCurve);
    }
    
static ICurvePrimitive* CreateSwapFromSource (MSInterpolationCurveR fitCurve)
    {
    return new CurvePrimitiveInterpolationCurve (fitCurve, true);
    }


}; // CurvePrimitiveInterpolationCurve
ICurvePrimitivePtr  ICurvePrimitive::CreateInterpolationCurve (MSInterpolationCurveCR fitCurve)
    {return CurvePrimitiveInterpolationCurve::Create (fitCurve);}

ICurvePrimitivePtr  ICurvePrimitive::CreateInterpolationCurveSwapFromSource (MSInterpolationCurveR fitCurve)
    {return CurvePrimitiveInterpolationCurve::CreateSwapFromSource (fitCurve);}

#endif



#ifndef CurvePrimtive_NO_AKIMA
/*=================================================================================**//**
* @bsiclass                                                     Brien.Bastings  02/10
+===============+===============+===============+===============+===============+======*/
struct CurvePrimitiveAkimaCurve : public CurvePrimitiveBsplineCurve
{
typedef CurvePrimitiveBsplineCurve Super;

protected:

bvector<DPoint3d>   m_points;


/*--------------------------------------------------------------------------------**//**
Copy bits from curve, points into the Akima.
Caller is no longer resposible for freeing the curve.
Caller is responsible for correct relationship of curve and points.
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
explicit CurvePrimitiveAkimaCurve(MSBsplineCurvePtr &capturedCurve, DPoint3dCP points, size_t nPoints)
    : CurvePrimitiveBsplineCurve ()
    {
    m_points.resize (nPoints);
    memcpy (&m_points[0], points, nPoints * sizeof (*points));
    m_curve = capturedCurve;
    }

//~CurvePrimitiveAkimaCurve () {}   // base class deallocates the curve.
ICurvePrimitivePtr _Clone () const override     {return Create (&m_points[0], m_points.size ());}


/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
CurvePrimitiveType _GetCurvePrimitiveType() const override {return CURVE_PRIMITIVE_TYPE_AkimaCurve;}
bvector<DPoint3d> const*    _GetAkimaCurveCP () const override {return &m_points;}

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
void _Process(ICurvePrimitiveProcessor &processor, DSegment1dCP interval) const override 
    {
    processor._ProcessAkimaCurve (*this, m_points, interval);
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      06/2014
+--------------------------------------------------------------------------------------*/
bool _IsSameStructureAndGeometry (ICurvePrimitiveCR other, double tolerance) const override
    {
    bvector<DPoint3d> const *points = GetAkimaCurveCP ();
    bvector<DPoint3d> const *otherPoints = other.GetAkimaCurveCP ();
    return nullptr != points && nullptr != otherPoints
          && DPoint3d::AlmostEqual (*points, *otherPoints, tolerance);
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
bool _TransformInPlace (TransformCR transform) override
    {
    // Hmmmm.. Do the primary and bspline transformations really apply in parallel? Should the bspline be regenerated fromr
    //  transformed primary?  Dunno.
    Super::_TransformInPlace (transform);
    DPoint3dOps::Multiply (&m_points, transform);
    return true;
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
bool _ReverseCurvesInPlace () override
    {
    // Hmmmm.. Do the primary and bspline transformations really apply in parallel? Should the bspline be regenerated fromr
    //  transformed primary?  Dunno.
    Super::_ReverseCurvesInPlace ();
    DPoint3dOps::Reverse (m_points);
    return true;
    }


public:

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
MSBsplineCurveCP _GetBsplineCurveCP() const override {return NULL;}  // Users do not see the bspline !!
MSBsplineCurvePtr _GetBsplineCurvePtr() const override {return NULL;}  // Users do not see the bspline !!

static ICurvePrimitive* Create (DPoint3dCP points, size_t nPoints)
    {
    MSBsplineCurve curve;
    if (SUCCESS == curve.InitAkima (points, nPoints))
        {
        MSBsplineCurvePtr curvePtr = curve.CreateCapture ();
        return new CurvePrimitiveAkimaCurve (curvePtr, points, nPoints);
        }
    return nullptr;
    }
}; // CurvePrimitiveAkimaCurve
ICurvePrimitivePtr  ICurvePrimitive::CreateAkimaCurve (DPoint3dCP points, size_t nPoints)
    {return CurvePrimitiveAkimaCurve::Create (points, nPoints);}

#endif


/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
ICurvePrimitivePtr ICurvePrimitive::CreateBsplineCurve (MSBsplineCurveCR curve)
    {return CurvePrimitiveBsplineCurve::Create (curve);}

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
ICurvePrimitivePtr ICurvePrimitive::CreateBsplineCurveSwapFromSource (MSBsplineCurveR curve)
    {
    return CurvePrimitiveBsplineCurve::Create (curve.CreateCapture ());
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
ICurvePrimitivePtr ICurvePrimitive::CreateBsplineCurve (MSBsplineCurvePtr curve)
    {return CurvePrimitiveBsplineCurve::Create (curve);}


/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
ICurvePrimitivePtr ICurvePrimitive::CreateSpiral
(
DSpiral2dBaseCR spiral,
TransformCR frame,
double fractionA,
double fractionB,
double maxStrokeLength
)
    {
    int type = spiral.GetTransitionTypeCode ();
    if (type >= DSpiral2dBase::TransitionType_FirstDirectEvaluate)
        {
        // ASSUME specific primary data  . . 
        // DEMAND frame is xy plane except for origin and possible z rotation ...
        // DEMAND fractions are simple 0..1
        double radius0 = DoubleOps::ValidatedDivideDistance (1.0, spiral.mCurvature0);
        double radius1 = DoubleOps::ValidatedDivideDistance (1.0, spiral.mCurvature1);

        double radians;
        DPoint3d origin;
        double scale;
        RotMatrix axes;
        if (frame.IsTranslateScaleRotateAroundZ (origin, axes, scale, radians)
//            && DoubleOps::AlmostEqual (fractionA, 0.0)
//            && DoubleOps::AlmostEqual (fractionB, 1.0)
            )
            {
            // the given system has (big) axes.  Use unit axes, but scale all distances
            radius0 *= scale;
            radius1 *= scale;
            double length = spiral.mLength * scale;
            return ICurvePrimitive::CreatePseudoSpiralPointBearingRadiusLengthRadius (
                    type,
                    origin,
                    spiral.mTheta0 + radians,
                    radius0,
                    length,
                    radius1,
                    fractionA,
                    fractionB
                    );
            }

        Transform frame1 = frame;
        frame1.ScaleMatrixColumns (1, -1, -1);
        if (frame1.IsTranslateScaleRotateAroundZ (origin, axes, scale, radians)
//            && DoubleOps::AlmostEqual (fractionA, 0.0)
//            && DoubleOps::AlmostEqual (fractionB, 1.0)
            )
            {
            // the given system has (big) axes.  Use unit axes, but scale all distances
            radius0 *= -scale;
            radius1 *= -scale;
            double length = spiral.mLength * scale;
            return ICurvePrimitive::CreatePseudoSpiralPointBearingRadiusLengthRadius (
                    type,
                    origin,
                    radians - spiral.mTheta0,
                    radius0,
                    length,
                    radius1,
                    fractionA,
                    fractionB
                    );
            }
        
        return nullptr;
        }

    return CREATE_SPIRAL_CURVE (spiral, frame, fractionA, fractionB, maxStrokeLength);
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      12/2015
+--------------------------------------------------------------------------------------*/
ICurvePrimitivePtr ICurvePrimitive::CreateCatenary
(
double a,
DPoint3dDVec3dDVec3dCR basis,
double x0,
double x1
)
    {
    return new CurvePrimitiveCatenaryCurve (a, basis, x0, x1);
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
ICurvePrimitivePtr ICurvePrimitive::CreateSpiralBearingCurvatureBearingCurvature
(
int transitionType,
double startRadians,
double startCurvature,
double endRadians,
double endCurvature,
TransformCR frame,
double fractionA,
double fractionB
)
    {
    DSpiral2dBaseP xySpiral = DSpiral2dBase::CreateBearingCurvatureBearingCurvature
        (transitionType, startRadians, startCurvature, endRadians, endCurvature);
    if (xySpiral == NULL)
        return NULL;
    return CREATE_SPIRAL_CURVE (*xySpiral, frame, fractionA, fractionB);
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
ICurvePrimitivePtr ICurvePrimitive::CreateSpiralBearingRadiusBearingRadius
(
int transitionType,
double startRadians,
double startRadius,
double endRadians,
double endRadius,
TransformCR frame,
double fractionA,
double fractionB
)
    {
    DSpiral2dBaseP xySpiral = DSpiral2dBase::CreateBearingCurvatureBearingCurvature
        (transitionType,
        startRadians, 
        startRadius == 0.0 ? 0.0 : 1.0 / startRadius, 
        endRadians, 
        endRadius == 0.0 ? 0.0 : 1.0 / endRadius
        );
    if (xySpiral == NULL)
        return NULL;
    ICurvePrimitive* ptr = CREATE_SPIRAL_CURVE (*xySpiral, frame, fractionA, fractionB);
    delete xySpiral;
    return ptr;
    }


/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
ICurvePrimitivePtr ICurvePrimitive::CreateSpiralBearingCurvatureLengthCurvature
(
int transitionType,
double startRadians,
double startCurvature,
double length,
double endCurvature,
TransformCR frame,
double fractionA,
double fractionB
)
    {
    DSpiral2dBaseP xySpiral = DSpiral2dBase::CreateBearingCurvatureLengthCurvature
        (transitionType, startRadians, startCurvature, length, endCurvature);
    if (xySpiral == NULL)
        return NULL;
    return CREATE_SPIRAL_CURVE (*xySpiral, frame, fractionA, fractionB);
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
ICurvePrimitivePtr ICurvePrimitive::CreateSpiralBearingRadiusLengthRadius
(
int transitionType,
double startRadians,
double startRadius,
double length,
double endRadius,
TransformCR frame,
double fractionA,
double fractionB
)
    {
    DSpiral2dBaseP xySpiral = DSpiral2dBase::CreateBearingCurvatureLengthCurvature
        (transitionType,
        startRadians, 
        startRadius == 0.0 ? 0.0 : 1.0 / startRadius, 
        length, 
        endRadius == 0.0 ? 0.0 : 1.0 / endRadius
        );
    if (xySpiral == NULL)
        return NULL;
    ICurvePrimitive* ptr =  CREATE_SPIRAL_CURVE (*xySpiral, frame, fractionA, fractionB);
    delete xySpiral;
    return ptr;
    }



END_BENTLEY_GEOMETRY_NAMESPACE
