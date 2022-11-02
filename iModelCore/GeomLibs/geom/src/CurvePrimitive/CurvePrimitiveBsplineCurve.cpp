/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
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
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct CurvePrimitiveBsplineCurve : public ICurvePrimitive
{
protected:

MSBsplineCurvePtr  m_curve;


/*--------------------------------------------------------------------------------**//**
* @bsimethod
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
* @bsimethod
+--------------------------------------------------------------------------------------*/
ICurvePrimitivePtr _Clone() const override {return Create (*m_curve);}


/*--------------------------------------------------------------------------------**//**
* @bsimethod
+--------------------------------------------------------------------------------------*/
bool _IsMappableFractionSpace() const override {return true;}

/*--------------------------------------------------------------------------------**//**
* @bsimethod
+--------------------------------------------------------------------------------------*/
bool _IsFractionSpace() const override {return true;}

/*--------------------------------------------------------------------------------**//**
* @bsimethod
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
* @bsimethod
+--------------------------------------------------------------------------------------*/
CurvePrimitiveType _GetCurvePrimitiveType() const override {return CURVE_PRIMITIVE_TYPE_BsplineCurve;}

/*--------------------------------------------------------------------------------**//**
* @bsimethod
+--------------------------------------------------------------------------------------*/
MSBsplineCurveCP _GetBsplineCurveCP() const override {return m_curve.get ();}
MSBsplineCurvePtr _GetBsplineCurvePtr() const override {return m_curve;}

/*--------------------------------------------------------------------------------**//**
* @bsimethod
+--------------------------------------------------------------------------------------*/
MSBsplineCurveCP _GetProxyBsplineCurveCP() const        override {return m_curve.get ();}
MSBsplineCurvePtr _GetProxyBsplineCurvePtr() const        override {return m_curve;}

/*--------------------------------------------------------------------------------**//**
* @bsimethod
+--------------------------------------------------------------------------------------*/
void _Process(ICurvePrimitiveProcessor &processor, DSegment1dCP interval) const override 
    {processor._ProcessBsplineCurve (*this, *m_curve, interval);}


/*--------------------------------------------------------------------------------**//**
* @bsimethod
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
* @bsimethod
+--------------------------------------------------------------------------------------*/
bool _FractionToPoint(double fraction, DPoint3dR point) const override
        {
        m_curve->FractionToPoint (point, fraction);
        return true;
        }

/*--------------------------------------------------------------------------------**//**
* @bsimethod
+--------------------------------------------------------------------------------------*/
bool _FractionToPoint(double fraction, DPoint3dR point, DVec3dR tangent) const override
        {
        m_curve->FractionToPoint (point, tangent, fraction);
        return true;
        }
    

/*--------------------------------------------------------------------------------**//**
* @bsimethod
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
* @bsimethod
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
* @bsimethod
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
* @bsimethod
+--------------------------------------------------------------------------------------*/
bool _Length(double &length) const override
        {
        length = m_curve->Length ();
        return true;
        }

/*--------------------------------------------------------------------------------**//**
* @bsimethod
+--------------------------------------------------------------------------------------*/
bool _Length(RotMatrixCP worldToLocal, double &length) const override
        {
        length = m_curve->Length (worldToLocal);
        return true;
        }

/*--------------------------------------------------------------------------------**//**
* @bsimethod
+--------------------------------------------------------------------------------------*/
bool _FastLength(double &length) const override
        {
        length = m_curve->PolygonLength ();
        return true;
        }

/*--------------------------------------------------------------------------------**//**
* @bsimethod
+--------------------------------------------------------------------------------------*/
bool _GetRange(DRange3dR range) const override
        {
        range = m_curve->GetRange ();
        return true;
        }


/*--------------------------------------------------------------------------------**//**
* @bsimethod
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
* @bsimethod
+--------------------------------------------------------------------------------------*/
double _FastMaxAbs() const override
        {
        DRange3d poleRange;
        m_curve->GetPoleRange (poleRange);
        return poleRange.LargestCoordinate ();
        }

/*--------------------------------------------------------------------------------**//**
* @bsimethod
+--------------------------------------------------------------------------------------*/
bool _IsSameStructureAndGeometry (ICurvePrimitiveCR other, double tolerance) const override
    {
    MSBsplineCurveCP otherBCurve = other.GetBsplineCurveCP ();
    return NULL != otherBCurve && m_curve->AlmostEqual (*otherBCurve, tolerance);
    }
/*--------------------------------------------------------------------------------**//**
* @bsimethod
+--------------------------------------------------------------------------------------*/
bool _IsValidGeometry(GeometryValidatorPtr &validator) const override
    {
    return m_curve->IsValidGeometry (validator);
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod
+--------------------------------------------------------------------------------------*/
size_t _NumComponent () const override {return 1;}

/*--------------------------------------------------------------------------------**//**
* @bsimethod
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
* @bsimethod
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
* @bsimethod
+--------------------------------------------------------------------------------------*/
bool _GetMSBsplineCurve(MSBsplineCurveR curve, double fractionA, double fractionB) const override
        {
        return SUCCESS == curve.CopySegment (*m_curve,
            m_curve->FractionToKnot (fractionA), m_curve->FractionToKnot (fractionB));
        }


/*--------------------------------------------------------------------------------**//**
* @bsimethod
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
* @bsimethod
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
* @bsimethod
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
* @bsimethod
+--------------------------------------------------------------------------------------*/
bool _SignedDistanceBetweenFractions(double startFraction, double endFraction, double &signedDistance) const override
        {
        signedDistance = fabs (m_curve->LengthBetweenFractions (startFraction, endFraction));
        if (endFraction < startFraction)
            signedDistance = - signedDistance;
        return true;
        }

/*--------------------------------------------------------------------------------**//**
* @bsimethod
+--------------------------------------------------------------------------------------*/
bool _SignedDistanceBetweenFractions(RotMatrixCP worldToLocal, double startFraction, double endFraction, double &signedDistance) const override
        {
        signedDistance = fabs (m_curve->LengthBetweenFractions (worldToLocal, startFraction, endFraction));
        if (endFraction < startFraction)
            signedDistance = - signedDistance;
        return true;
        }

/*--------------------------------------------------------------------------------**//**
* @bsimethod
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
* @bsimethod
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
* @bsimethod
+--------------------------------------------------------------------------------------*/
bool _ClosestPointBounded (DPoint3dCR spacePoint, double &fraction, DPoint3dR curvePoint, bool extend0, bool extend1) const override
        {
        m_curve->ClosestPoint (curvePoint, fraction, spacePoint);
        return true;
        }


/*--------------------------------------------------------------------------------**//**
* @bsimethod
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
* @bsimethod
+--------------------------------------------------------------------------------------*/
bool _TransformInPlace (TransformCR transform) override
        {
        m_curve->TransformCurve (transform);
        return true;
        }

/*--------------------------------------------------------------------------------**//**
* @bsimethod
+--------------------------------------------------------------------------------------*/
bool _ReverseCurvesInPlace () override
        {
        m_curve->MakeReversed ();
        return true;
        }

/*--------------------------------------------------------------------------------**//**
* @bsimethod
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
* @bsimethod
+--------------------------------------------------------------------------------------*/
void _AppendCurvePlaneIntersections(DPlane3dCR plane, bvector<CurveLocationDetailPair> &intersections, double tol) const override 
    {AppendTolerancedPlaneIntersections (plane, this, *m_curve, intersections, tol);}

}; // CurvePrimitiveBsplineCurve
//#define CREATE_SPIRAL_CURVE CurvePrimitiveSpiralCurve::Create
//#define CREATE_SPIRAL_CURVE CurvePrimitiveSpiralCurve1::Create
#include "cp_directSpiral.h"
#include "cp_spiral.h"
#include "cp_catenary.h"
static int s_spiralSelector = 0;
int ICurvePrimitive::SelectSpiralImplementation(int selector)
    {
    auto oldSelector = s_spiralSelector;
    s_spiralSelector = selector;
    return oldSelector;
    }
static ICurvePrimitivePtr CreateSpiralFromPlacement (
    DSpiral2dBaseCR spiral,
    TransformCR frame,
    double fractionA,
    double fractionB,
    double maxStrokeLength = DEFAULT_SPIRAL_MAX_STROKE_LENGTH
    );

#ifndef CurvePrimitive_NO_INTERPOLATINGCURVE
/*=================================================================================**//**
* @bsiclass
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
* @bsimethod
+--------------------------------------------------------------------------------------*/
ICurvePrimitivePtr _Clone() const override {return Create (m_fitCurve);}

/*--------------------------------------------------------------------------------**//**
* @bsimethod
+--------------------------------------------------------------------------------------*/
MSBsplineCurveCP _GetBsplineCurveCP() const override {return NULL;}  // Users do not see the bspline !!
MSBsplineCurvePtr _GetBsplineCurvePtr() const override {return NULL;}  // Users do not see the bspline !!
CurvePrimitiveType          _GetCurvePrimitiveType () const override {return CURVE_PRIMITIVE_TYPE_InterpolationCurve;}

/*--------------------------------------------------------------------------------**//**
* @bsimethod
+--------------------------------------------------------------------------------------*/
MSInterpolationCurveCP _GetInterpolationCurveCP() const override {return &m_fitCurve;}

/*--------------------------------------------------------------------------------**//**
* @bsimethod
+--------------------------------------------------------------------------------------*/
void _Process(ICurvePrimitiveProcessor &processor, DSegment1dCP interval) const override 
    {
    processor._ProcessInterpolationCurve (*this, m_fitCurve, interval);
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod
+--------------------------------------------------------------------------------------*/
bool _IsSameStructureAndGeometry (ICurvePrimitiveCR other, double tolerance) const override
    {
    MSInterpolationCurveCP otherInterpolationCurve = other.GetInterpolationCurveCP ();
    return NULL != otherInterpolationCurve && m_fitCurve.AlmostEqual (*otherInterpolationCurve, tolerance);
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod
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
* @bsimethod
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
* @bsiclass
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
* @bsimethod
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
* @bsimethod
+--------------------------------------------------------------------------------------*/
CurvePrimitiveType _GetCurvePrimitiveType() const override {return CURVE_PRIMITIVE_TYPE_AkimaCurve;}
bvector<DPoint3d> const*    _GetAkimaCurveCP () const override {return &m_points;}

/*--------------------------------------------------------------------------------**//**
* @bsimethod
+--------------------------------------------------------------------------------------*/
void _Process(ICurvePrimitiveProcessor &processor, DSegment1dCP interval) const override 
    {
    processor._ProcessAkimaCurve (*this, m_points, interval);
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod
+--------------------------------------------------------------------------------------*/
bool _IsSameStructureAndGeometry (ICurvePrimitiveCR other, double tolerance) const override
    {
    bvector<DPoint3d> const *points = GetAkimaCurveCP ();
    bvector<DPoint3d> const *otherPoints = other.GetAkimaCurveCP ();
    return nullptr != points && nullptr != otherPoints
          && DPoint3d::AlmostEqual (*points, *otherPoints, tolerance);
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod
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
* @bsimethod
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
* @bsimethod
+--------------------------------------------------------------------------------------*/
MSBsplineCurveCP _GetBsplineCurveCP() const override {return NULL;}  // Users do not see the bspline !!
MSBsplineCurvePtr _GetBsplineCurvePtr() const override {return NULL;}  // Users do not see the bspline !!

static ICurvePrimitivePtr Create (DPoint3dCP points, size_t nPoints)
    {
    MSBsplineCurve curve;
    if (SUCCESS == curve.InitAkima (points, nPoints))
        {
        MSBsplineCurvePtr curvePtr = curve.CreateCapture ();
        return new CurvePrimitiveAkimaCurve (curvePtr, points, nPoints);
        }
    if (nPoints > 0)
        return ICurvePrimitive::CreateLineString (points, nPoints);
    return nullptr;
    }
}; // CurvePrimitiveAkimaCurve
ICurvePrimitivePtr  ICurvePrimitive::CreateAkimaCurve (DPoint3dCP points, size_t nPoints)
    {return CurvePrimitiveAkimaCurve::Create (points, nPoints);}

#endif

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct CurvePrimitiveSpiralCurve : public CurvePrimitiveBsplineCurve
{
typedef CurvePrimitiveBsplineCurve Super;

protected:
DSpiral2dPlacement m_placement;


/*--------------------------------------------------------------------------------**//**
* @bsimethod
+--------------------------------------------------------------------------------------*/
void UpdateCurve()
    {
    bvector<DPoint3d> poles;
    DSpiral2dPolish *polishSpiral = dynamic_cast <DSpiral2dPolish*>(m_placement.spiral);
    if (nullptr != polishSpiral && polishSpiral->GetBezierPoles (poles, m_placement.fractionA, m_placement.fractionB))
        {
        m_placement.frame.Multiply (poles);
        ReplaceCurve (MSBsplineCurve::CreateFromPolesAndOrder(poles, nullptr, nullptr, (int) poles.size (), false));
        return; // connect needs this return?
        }
    MSBsplineCurve curve;
    DPoint3d origin;
    RotMatrix matrix;
    m_placement.frame.GetTranslation (origin);
    m_placement.frame.GetMatrix (matrix);
    if (SUCCESS == bspcurv_curveFromDSpiral2dBaseInterval (&curve, m_placement.spiral,
                m_placement.fractionA, m_placement.fractionB, &origin, &matrix))
        {
        ReplaceCurve (curve.CreateCapture ());
        }
    }
public:

/*--------------------------------------------------------------------------------**//**
* @bsimethod
+--------------------------------------------------------------------------------------*/
virtual DSpiral2dPlacementCP _GetSpiralPlacementCP() const override {return &m_placement;}

/*--------------------------------------------------------------------------------**//**
* @bsimethod
+--------------------------------------------------------------------------------------*/
virtual void _Process(ICurvePrimitiveProcessor &processor, DSegment1dCP interval) const override
    {
    processor._ProcessSpiral (*this, m_placement, interval);
    }



/*--------------------------------------------------------------------------------**//**
* @bsimethod
+--------------------------------------------------------------------------------------*/
explicit CurvePrimitiveSpiralCurve(
DSpiral2dBaseCR spiral,
TransformCR frame,
double fractionA,
double fractionB
) : CurvePrimitiveBsplineCurve (),
   m_placement (spiral, frame, fractionA, fractionB)
    {
    UpdateCurve ();
    }
~CurvePrimitiveSpiralCurve ()
    {
    m_placement.ReplaceSpiral (NULL);
    }


/*--------------------------------------------------------------------------------**//**
* @bsimethod
+--------------------------------------------------------------------------------------*/
virtual ICurvePrimitivePtr _Clone() const override
    {
    return Create (*m_placement.spiral, m_placement.frame, m_placement.fractionA, m_placement.fractionB);
    }


/*--------------------------------------------------------------------------------**//**
* @bsimethod
+--------------------------------------------------------------------------------------*/
virtual CurvePrimitiveType _GetCurvePrimitiveType() const override {return CURVE_PRIMITIVE_TYPE_Spiral;}
// Need query method for spiral access??
virtual bool _TransformInPlace (TransformCR transform) override
    {
    // Hmmmm.. If it's a non-rigid transform it changes the distance parameterization.
    Super::_TransformInPlace (transform);
    m_placement.frame.InitProduct (transform, m_placement.frame);
    return true;
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod
+--------------------------------------------------------------------------------------*/
virtual bool _ReverseCurvesInPlace () override
    {
    if (m_placement.ReverseInPlace ())
        {
        Super::_ReverseCurvesInPlace ();
        return true;
        }
    return false;
    }
#define ImposeNominalProportionalLength 1
#ifdef ImposeNominalProportionalLength
/*--------------------------------------------------------------------------------**//**
* @bsimethod
+--------------------------------------------------------------------------------------*/
bool _SignedDistanceBetweenFractions (double startFraction, double endFraction, double &signedDistance) const override
    {
    signedDistance = (endFraction - startFraction) * m_placement.SpiralLengthActiveInterval ();
    return true;
    }
#endif

public:

/*--------------------------------------------------------------------------------**//**
* @bsimethod
+--------------------------------------------------------------------------------------*/
virtual MSBsplineCurveCP _GetBsplineCurveCP() const override {return NULL;}  // Users do not see the bspline !!
virtual MSBsplineCurvePtr _GetBsplineCurvePtr() const override {return NULL;}  // Users do not see the bspline !!

static ICurvePrimitive* Create
(
DSpiral2dBaseCR spiral,
TransformCR frame,
double fractionA,
double fractionB,
double maxEdgeLength = 0
)
    {
    DSpiral2dDirectEvaluation const *nominalLengthSpiral = dynamic_cast <DSpiral2dDirectEvaluation const *> (&spiral);
    if (nominalLengthSpiral != nullptr)
        {
        return new CurvePrimitiveDirectSpiral (*nominalLengthSpiral, frame, fractionA, fractionB);
        }
    return new CurvePrimitiveSpiralCurve (spiral, frame, fractionA, fractionB);
    }

bool _IsSameStructureAndGeometry (ICurvePrimitiveCR other, double tolerance) const override
    {
    DSpiral2dPlacementCP placementA = GetSpiralPlacementCP ();
    DSpiral2dPlacementCP placementB = other.GetSpiralPlacementCP ();
    return   nullptr != placementA
          && nullptr != placementB
          && placementA->AlmostEqual (*placementB, tolerance);
    }

}; // CurvePrimitiveSpiralCurve




/*--------------------------------------------------------------------------------**//**
* @bsimethod
+--------------------------------------------------------------------------------------*/
ICurvePrimitivePtr ICurvePrimitive::CreateBsplineCurve (MSBsplineCurveCR curve)
    {return CurvePrimitiveBsplineCurve::Create (curve);}

/*--------------------------------------------------------------------------------**//**
* @bsimethod
+--------------------------------------------------------------------------------------*/
ICurvePrimitivePtr ICurvePrimitive::CreateBsplineCurveSwapFromSource (MSBsplineCurveR curve)
    {
    return CurvePrimitiveBsplineCurve::Create (curve.CreateCapture ());
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod
+--------------------------------------------------------------------------------------*/
ICurvePrimitivePtr ICurvePrimitive::CreateBsplineCurve (MSBsplineCurvePtr curve)
    {return CurvePrimitiveBsplineCurve::Create (curve);}


// Carry out spiral construction (see main method) with confirmed good curvatures
ICurvePrimitivePtr CreatePseudoSpiralCurvatureLengthCurvature_go
(
int typeCode,               //!< [in] transition type.  This method is intended to work with "cubic" approximations (New South Wales, Australian etc)
DPoint3dCR startPoint,      //!< [in] start point of spiral.
double startRadians,        //!< [in] start bearing angle in xy plane.double curvatureA,          //!< [in] radiusA (signed) radius (or 0 for line) at start.
double curvatureA,          //!< [in] (signed) curvature at start
double lengthAB,            //!< [in] length of spiral between radiusA and radiusB.
double curvatureB,          //!< [in] (signed) curvature at end.
bool reverseInterval,
double startFraction,
double endFraction
)
    {

    ICurvePrimitivePtr result;
    // extrapolate to inflection, assuming clothoid (linear) curvature function
    //   (curvatureB - curvatureA) / lengthAB = curvatureB / length0B
    double length0B = curvatureB * lengthAB / (curvatureB - curvatureA);
    // double length0A = length0B - lengthAB;
    double fractionA = curvatureA / curvatureB;
    double fractionB = 1.0;
    // create a complete spiral (from inflection to curvatureB)
    if (1.0 < fabs (curvatureB * length0B * 0.5))
        return nullptr;
    auto referenceSpiral = ICurvePrimitive::CreateSpiralBearingCurvatureLengthCurvature (typeCode, 0.0, 0.0, length0B, curvatureB,
            Transform::FromIdentity (), 0.0, 1.0);
    if (referenceSpiral != nullptr)
        {
        DPoint3d pointC;
        DVec3d vectorC;
        referenceSpiral->FractionToPoint (reverseInterval ? fractionB : fractionA, pointC, vectorC);
        if (reverseInterval)
            vectorC.Scale (-1.0);
        double bearingC = vectorC.AngleXY ();
        auto frameC = Transform::FromOriginAndBearingXY (pointC, bearingC);
        auto inverseC = frameC.ValidatedInverse ();
        auto frameA = Transform::FromOriginAndBearingXY (startPoint, startRadians);
        if (inverseC.IsValid ())
            {
            double fractionA1 = reverseInterval ? fractionB : fractionA;
            double fractionB1 = reverseInterval ? fractionA : fractionB;
            double fractionA2 = DoubleOps::Interpolate (fractionA1, startFraction, fractionB1);
            double fractionB2 = DoubleOps::Interpolate (fractionA1, endFraction, fractionB1);
            result = ICurvePrimitive::CreateSpiralBearingCurvatureLengthCurvature (typeCode, 0.0, 0.0, length0B, curvatureB,
                frameA * inverseC,
                fractionA2, fractionB2);
            }
        }
    return result;
    }


//! Construct a spiral with start radius, spiral length, and end radius.
//! This is a special construction for "cubic" approximations.
//! The constructed spiral is a fractional subset of another spiral that includes its inflection point (which may be outside
//! the active fractional subset).
ICurvePrimitivePtr ICurvePrimitive::CreatePseudoSpiralPointBearingRadiusLengthRadius
(
int typeCode,               //!< [in] transition type.  This method is intended to work with "cubic" approximations (New South Wales, Australian etc)
DPoint3dCR startPoint,             //!< [in] start point of spiral.
double startRadians,        //!< [in] start bearing angle in xy plane.
double radiusA,             //!< [in] radiusA (signed) radius (or 0 for line) at start.
double lengthAB,            //!< [in] length of spiral between radiusA and radiusB.
double radiusB             //!< [in] radiusB (signed) radius (or 0 for line) at end.
)
    {
    return CreatePseudoSpiralPointBearingRadiusLengthRadius (typeCode, startPoint, startRadians, radiusA, lengthAB, radiusB, 0.0, 1.0);
    }
//! Construct a spiral with start radius, spiral length, and end radius.
//! This is a special construction for "cubic" approximations.
//! The constructed spiral is a fractional subset of another spiral that includes its inflection point (which may be outside
//! the active fractional subset).
ICurvePrimitivePtr ICurvePrimitive::CreatePseudoSpiralPointBearingRadiusLengthRadius
(
int typeCode,               //!< [in] transition type.  This method is intended to work with "cubic" approximations (New South Wales, Australian etc)
DPoint3dCR startPoint,             //!< [in] start point of spiral.
double startRadians,        //!< [in] start bearing angle in xy plane.
double radiusA,             //!< [in] radiusA (signed) radius (or 0 for line) at start.
double lengthAB,            //!< [in] length of spiral between radiusA and radiusB.
double radiusB,              //!< [in] radiusB (signed) radius (or 0 for line) at end.
double startFraction,       //!< [in] start fraction for active interval.
double endFraction          //!< [in] end fraction for active interval.
)
    {
    if (DoubleOps::AlmostEqual (0.0, radiusA))
        radiusA = 0.0;
    if (DoubleOps::AlmostEqual (0.0, radiusB))
        radiusB = 0.0;
    if (DoubleOps::AlmostEqual (0.0, lengthAB))
        return nullptr;

    if (radiusA == 0.0 && radiusB == 0.0)
        {
        return ICurvePrimitive::CreateLine (
            DSegment3d::From (startPoint, startPoint + DVec3d::FromXYAngleAndMagnitude (startRadians, lengthAB)));
        }

    if (DoubleOps::AlmostEqual (radiusA, radiusB))
        {
        // Create a circular arc ...
        double theta = lengthAB / radiusA;      // if radiusA is negative, all the vector and angle signs coordinate to move in positive x direction !!!
        DEllipse3d arc = DEllipse3d::FromStartTangentNormalRadiusSweep (
                    startPoint, 
                    DVec3d::From (cos (startRadians), sin (startRadians)),
                    DVec3d::From (0,0,1),
                    radiusA,
                    fabs (theta));
        return ICurvePrimitive::CreateArc (arc);
        }

    double curvatureA = DoubleOps::ValidatedDivideDistance (1.0, radiusA);
    double curvatureB = DoubleOps::ValidatedDivideDistance (1.0, radiusB);
    if (fabs (curvatureB) > fabs (curvatureA) || DoubleOps::AlmostEqual (fabs (radiusA), fabs (radiusB)))
        return CreatePseudoSpiralCurvatureLengthCurvature_go (typeCode, startPoint, startRadians, curvatureA, lengthAB, curvatureB, false, startFraction, endFraction);
    else
        {
        static double signA = -1.0;
        static double signB = -1.0;
        return CreatePseudoSpiralCurvatureLengthCurvature_go (typeCode, startPoint, startRadians, signB * curvatureB, lengthAB, signA * curvatureA, true, startFraction, endFraction);
        }
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod
+--------------------------------------------------------------------------------------*/
ICurvePrimitivePtr ICurvePrimitive::CreateSpiral
(
DSpiral2dBaseCR spiral,
TransformCR frame,
double fractionA,
double fractionB,
double maxEdgeLength
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

    return CreateSpiralFromPlacement (spiral, frame, fractionA, fractionB);
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod
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
* @bsimethod
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
    auto cp = CreateSpiralFromPlacement(*xySpiral, frame, fractionA, fractionB);
    delete xySpiral;
    return cp;
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod
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
double fractionB,
bvector<double> const &extraData
)
    {
    DSpiral2dBaseP xySpiral = DSpiral2dBase::CreateBearingCurvatureBearingCurvature
        (transitionType,
        startRadians, 
        startRadius == 0.0 ? 0.0 : 1.0 / startRadius, 
        endRadians, 
        endRadius == 0.0 ? 0.0 : 1.0 / endRadius, extraData
        );
    if (xySpiral == NULL)
        return NULL;
    auto cp = CreateSpiralFromPlacement(*xySpiral, frame, fractionA, fractionB);
	delete xySpiral;
	return cp;
    }


/*--------------------------------------------------------------------------------**//**
* @bsimethod
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
double fractionB,
bvector<double> const &extraData
)
    {
    DSpiral2dBaseP xySpiral = DSpiral2dBase::CreateBearingCurvatureLengthCurvature
        (transitionType, startRadians, startCurvature, length, endCurvature, extraData);
    if (xySpiral == NULL)
        return NULL;
    auto cp = CreateSpiralFromPlacement (*xySpiral, frame, fractionA, fractionB);
    delete xySpiral;
    return cp;
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod
+--------------------------------------------------------------------------------------*/
ICurvePrimitivePtr ICurvePrimitive::CreateSpiralBearingRadiusLengthRadius
(
int transitionType,
double startRadians,
double radius0,
double length,
double radius1,
TransformCR frame,
double fractionA,
double fractionB,
bvector<double> const  &extraData
)
    {
    auto startCurvature = radius0 == 0.0 ? 0.0 : 1.0 / radius0;
    auto endCurvature = radius1 == 0.0 ? 0.0 : 1.0 / radius1;
    DSpiral2dBaseP xySpiral = DSpiral2dBase::CreateBearingCurvatureLengthCurvature
        (transitionType,
        startRadians, 
        startCurvature, 
        length, 
        endCurvature, extraData
        );
    if (xySpiral == NULL)
        return NULL;
    ICurvePrimitivePtr ptr = CreateSpiralFromPlacement (*xySpiral, frame, fractionA, fractionB);
    delete xySpiral;
    return ptr;
    }

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
    return CreateSpiralBearingRadiusLengthRadius (transitionType, startRadians, startRadius, length, endRadius, frame, fractionA, fractionB, bvector<double> ());
    }
//! Same as CreateSpiralBearingCurvatureLengthCurvature, but no extraData.
ICurvePrimitivePtr ICurvePrimitive::CreateSpiralBearingCurvatureLengthCurvature
(int transitionType, double startRadians, double startCurvature, double length, double endCurvature, TransformCR frame, double fractionA, double fractionB)
    {
    return CreateSpiralBearingCurvatureLengthCurvature(transitionType, startRadians, startCurvature, length, endCurvature, frame, fractionA, fractionB, bvector<double>());
    }
//! DEPRECATED: Calls CreateSpiralBearingRadiusBearingRadius with empty extraData
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
    return CreateSpiralBearingRadiusBearingRadius (transitionType, startRadians, startRadius, endRadians, endRadius, frame, fractionA, fractionB,
        bvector<double> ());
    }

static ICurvePrimitivePtr CreateSpiralFromPlacement
(
    DSpiral2dBaseCR spiral,
    TransformCR frame,
    double fractionA,
    double fractionB,
    double maxStrokeLength
)
    {
    if (s_spiralSelector == 1)
        return CurvePrimitiveSpiralCurve::Create(spiral, frame, fractionA, fractionB, maxStrokeLength);
    else
        return CurvePrimitiveSpiralCurve1::Create(spiral, frame, fractionA, fractionB, maxStrokeLength);
    }


END_BENTLEY_GEOMETRY_NAMESPACE
