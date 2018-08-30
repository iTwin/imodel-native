/*--------------------------------------------------------------------------------------+
|
|     $Source: geom/src/CurvePrimitive/cp_directSpiral.h $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
//#include <bsibasegeomPCH.h>
// This code is to be included into CurvePrimitiveBsplineCurve.cpp  (INSIDE the namespace block)
//BEGIN_BENTLEY_GEOMETRY_NAMESPACE

#define DEFAULT_DIRECT_SPIRAL_STROKE_RADIANS (0.02)
#define DEFAULT_DIRECT_SPIRAL_MIN_STROKES (4)
#define DEFAULT_DIRECT_SPIRAL_MAX_STROKES (100)
// Vector integrands for testing
struct NominalLengthSpiralArcLengthIntegrands : BSIIncrementalVectorIntegrand
{
DSpiral2dDirectEvaluation &m_directSpiral;
NominalLengthSpiralArcLengthIntegrands (DSpiral2dDirectEvaluation &directSpiral)
    : m_directSpiral (directSpiral)
    {
    }

int GetVectorIntegrandCount () override { return 1;}
void EvaluateVectorIntegrand (double t, double *pF) override
    {
    DPoint2d uv;
    DVec2d d1uv;
    m_directSpiral.EvaluateAtFraction (t, uv, &d1uv, nullptr, nullptr);
    pF[0] = d1uv.Magnitude ();
    }
bvector<double> m_globalFraction;
bvector<double> m_localFraction;
bvector<double> m_summedDistance;
bvector<DPoint3d> m_xyz;
bvector<DPoint2d> m_uv;
bool AnnounceIntermediateIntegral (double t, double *pIntegrals) override
    {
    m_globalFraction.push_back (t);
    m_summedDistance.push_back (pIntegrals[0]);
    DPoint2d uv;
    m_directSpiral.EvaluateAtFraction (t, uv, nullptr, nullptr, nullptr);
    m_uv.push_back (uv);
    return true;
    }
void clear ()
    {
    m_globalFraction.clear ();
    m_localFraction.clear ();
    m_summedDistance.clear ();
    m_uv.clear ();
    m_xyz.clear ();
    }
double LastDistance () const { return m_summedDistance.empty () ? 0.0 : m_summedDistance.back ();}
void Integrate (double startFraction, double endFraction, TransformCR frame, uint32_t numInterval)
    {
    clear ();
    double localError;
    BSIQuadraturePoints quadraturePoints;
    quadraturePoints.InitGauss (5);
    // accumulate <globalFraction, uv, trueDistance> arrays ...
    if (numInterval < 1)
        numInterval = 1;
    quadraturePoints.IntegrateWithRombergExtrapolation (*this, startFraction, endFraction, numInterval, localError);
    // apply transform and fraction mapping
    double intervalDivisor;
    DoubleOps::SafeDivide (intervalDivisor, 1.0, (endFraction - startFraction), 0.0);
    DPoint3d xyz;
    for (size_t i = 0; i < m_uv.size (); i++)
        {
        frame.Multiply (xyz, m_uv[i].x, m_uv[i].y, 0);
        m_xyz.push_back (xyz);
        m_localFraction.push_back (startFraction + (m_globalFraction[i] - startFraction) * intervalDivisor);
        }
    }
};
/*=================================================================================**//**
* @bsiclass                                                      EarlinLutz   12/2015
!! The bspline approximation is not evaluated until requested as proxy !!!
!! The placement's raw pointer is DSpiral2dBase, but this class only constructs it with DSpiral2dDirectEvaluation.
+===============+===============+===============+===============+===============+======*/
struct CurvePrimitiveDirectSpiral : public ICurvePrimitive
{
typedef CurvePrimitiveBsplineCurve Super;

protected:
mutable MSBsplineCurvePtr m_curve;
DSpiral2dPlacement m_placement;    // for outside use
DSpiral2dDirectEvaluation *m_directSpiral;    // same pointer bits, but strongly typed.
mutable bvector<DPoint3d> m_strokes;
mutable bvector<double>   m_localFraction;
mutable bvector<double>   m_trueDistance;
mutable double m_cachedStrokeFraction;

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
uint32_t StrokeCountBetweenLocalFractions (double chordTol, double angleTol, double maxEdgeLength, double startFraction, double endFraction) const
    {
    double dBeta = fabs (m_directSpiral->mTheta1 - m_directSpiral->mTheta0) * fabs (endFraction - startFraction);
    double dNumStroke = ceil (dBeta / DEFAULT_DIRECT_SPIRAL_STROKE_RADIANS);
    uint32_t iNumStroke = (uint32_t)dNumStroke;
    if (iNumStroke < DEFAULT_DIRECT_SPIRAL_MIN_STROKES)
        iNumStroke = DEFAULT_DIRECT_SPIRAL_MIN_STROKES;
    if (iNumStroke > DEFAULT_DIRECT_SPIRAL_MAX_STROKES)
        iNumStroke = DEFAULT_DIRECT_SPIRAL_MAX_STROKES;
    return iNumStroke;
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
size_t _GetStrokeCount (IFacetOptionsCR options, double startFraction, double endFraction) const override
    {
    return StrokeCountBetweenLocalFractions (
        options.GetChordTolerance (), options.GetAngleTolerance (), options.GetMaxEdgeLength (),
        startFraction, endFraction);
    }



/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      12/2015
+--------------------------------------------------------------------------------------*/
void CreateCachedCurve() const
    {
    // hm.. user stroker to get point count ... the strokes are thrown away
    bvector<double> fractions;
    bvector<DPoint3d> points;
    uint32_t iNumStroke = StrokeCountBetweenLocalFractions (0,0,0, m_placement.fractionA, m_placement.fractionB);
    m_strokes.clear ();
    m_localFraction.clear ();
    m_trueDistance.clear ();
    m_cachedStrokeFraction = 1.0 / (double)iNumStroke;
    NominalLengthSpiralArcLengthIntegrands integrand (*m_directSpiral);
    integrand.Integrate (m_placement.fractionA, m_placement.fractionB, m_placement.frame, iNumStroke);
    m_trueDistance = integrand.m_summedDistance;
    m_strokes = integrand.m_xyz;
    m_localFraction = integrand.m_localFraction;
    m_curve = MSBsplineCurve::CreateFromInterpolationAtBasisFunctionPeaks (m_strokes, 4, 0);
    }

void EnsureCachedCurve () const
    {
    if (!m_curve.IsValid ())
        CreateCachedCurve ();
    }
public:

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      12/2015
+--------------------------------------------------------------------------------------*/
GEOMAPI_VIRTUAL DSpiral2dPlacementCP      _GetSpiralPlacementCP () const override {return &m_placement;}

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      12/2015
+--------------------------------------------------------------------------------------*/
void _Process(ICurvePrimitiveProcessor &processor, DSegment1dCP interval) const override 
    {
    (const_cast <CurvePrimitiveDirectSpiral *> (this))->EnsureCachedCurve ();
    processor._ProcessBsplineCurve(*this, *_GetProxyBsplineCurvePtr (), interval);
    // We are a spiral, yes? Call _ProcessSpiral?
    }


/*--------------------------------------------------------------------------------**//**
// CAPTURE placement
* @bsimethod                                                    EarlinLutz      12/2015
+--------------------------------------------------------------------------------------*/
explicit CurvePrimitiveDirectSpiral(
DSpiral2dDirectEvaluation const & directSpiral,
TransformCR frame,
double fractionA,
double fractionB
)
    {
    m_directSpiral = (DSpiral2dDirectEvaluation*)directSpiral.Clone ();
    m_placement.InitCapturePointer(m_directSpiral, frame, fractionA, fractionB);
    CreateCachedCurve ();
    }


~CurvePrimitiveDirectSpiral ()
    {
    // spiral pointer will be freed in destructor.
    }


/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      12/2015
+--------------------------------------------------------------------------------------*/
ICurvePrimitivePtr _Clone() const override
    {
    return new CurvePrimitiveDirectSpiral (*m_directSpiral, m_placement.frame, m_placement.fractionA, m_placement.fractionB);
    }


/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      12/2015
+--------------------------------------------------------------------------------------*/
CurvePrimitiveType _GetCurvePrimitiveType() const override {return CURVE_PRIMITIVE_TYPE_Spiral;}

bool _TransformInPlace (TransformCR transform) override
    {
    // If there is a simple scale, it should be extracted and applied to spiral parameters !!!!
    m_placement.frame = transform * m_placement.frame;
    CreateCachedCurve ();
    return true;
    }

size_t _NumComponent() const override { return 1; }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      12/2015
+--------------------------------------------------------------------------------------*/
bool _ReverseCurvesInPlace () override
    {
    std::swap (m_placement.fractionA, m_placement.fractionB);
    CreateCachedCurve ();
    return true;
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
bool _Length (double &length) const override
    {
    length = m_trueDistance.back ();
    return true;
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      05/15
+--------------------------------------------------------------------------------------*/
bool _Length(RotMatrixCP worldToLocal, double &length) const override
    {
    length = 0;
    double s;
    if (worldToLocal->IsUniformScale (s))
        {
        length = m_trueDistance.back () * s;
        return true;
        }
    return false;
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
bool _PointAtSignedDistanceFromFraction (double localStartFraction, double signedDistance, bool allowExtension, CurveLocationDetailR location) const override
    {
    double startFraction = m_placement.ActiveFractionToGlobalFraction (localStartFraction);
    double localDelta = signedDistance / m_trueDistance.back (); // first approximation !!!
    double localFractionDelta = m_placement.fractionB - m_placement.fractionA;
    static double s_relTol = 1.0e-10;
    double distanceTolerance = m_trueDistance.back () * s_relTol;   // um.. maybe it should be the full length?
    NominalLengthSpiralArcLengthIntegrands integrand (*m_directSpiral);
    DPoint2d uv;
    DVec2d d1uv;
    static uint32_t s_maxIteration = 10;
    uint32_t convergedCount = 0;
    uint32_t numStroke;
    double actualDistance = 0.0;
    double approximateDelta = localDelta * localFractionDelta;
    // dS / df for global fraction is nearly constant.   This should not happen many times . . .
    // TODO:  For internal points, use the cached fractions and distances to get within a single integration step
    for (uint32_t count = 0; count< s_maxIteration; count++)
        {
        double endFraction = startFraction + approximateDelta;
        if (fabs (startFraction - endFraction) < m_cachedStrokeFraction)
            numStroke = 1;
        else
            numStroke = StrokeCountBetweenLocalFractions (0, 0, 0, startFraction, endFraction);

        integrand.Integrate (startFraction, endFraction, m_placement.frame, numStroke);
        actualDistance += integrand.LastDistance ();
        // Get the final velocity ...
        m_directSpiral->EvaluateAtFraction (endFraction, uv, &d1uv, nullptr, nullptr);
        double dSdGlobal = d1uv.Magnitude ();   // This is GLOBAL fraction velocity
        double distanceError = actualDistance - signedDistance;
        if (fabs (distanceError) < distanceTolerance)
            {
            convergedCount++;
            if (convergedCount > 1)
                {
                location = CurveLocationDetail (this, 1);
                double localEndFraction = m_placement.GlobalFractionToActiveFraction (endFraction);
                _FractionToPoint (localEndFraction, location.point);
                location.SetSingleComponentFractionAndA (localEndFraction, actualDistance);
                return true;
                }
            // else -- fall through to confirm with a short step !!!
            }
        else
            {
            convergedCount = 0;
            }
        approximateDelta = -distanceError / dSdGlobal;
        startFraction = endFraction;
        }
    return false;
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
bool _SignedDistanceBetweenFractions (double startFraction, double endFraction, double &signedDistance) const override
    {
    NominalLengthSpiralArcLengthIntegrands integrand (*m_directSpiral);
    uint32_t numStroke = StrokeCountBetweenLocalFractions (0, 0, 0, startFraction, endFraction);
    double globalA = m_placement.ActiveFractionToGlobalFraction (startFraction);
    double globalB = m_placement.ActiveFractionToGlobalFraction (endFraction);
    integrand.Integrate (globalA, globalB, m_placement.frame, numStroke);
    double integral = integrand.LastDistance ();
    if (globalB < globalA)
        integral = - integral;
    signedDistance = integral;
    return true;
    }
bool _FractionToPoint (double f, DPoint3dR point) const override
    {
    DPoint2d xy;
    double globalFraction = m_placement.ActiveFractionToGlobalFraction (f);
    m_directSpiral->EvaluateAtFraction (globalFraction, xy, nullptr, nullptr, nullptr);
    point = m_placement.frame * DPoint3d::From (xy);
    return true;
    }

bool _FractionToPoint (double f, DPoint3dR point, DVec3dR tangent) const override
    {
    DPoint2d xy;
    DVec2d   dxy;
    double globalFraction = m_placement.ActiveFractionToGlobalFraction (f);
    double intervalScale = m_placement.fractionB - m_placement.fractionA;
    m_directSpiral->EvaluateAtFraction (globalFraction, xy, &dxy, nullptr, nullptr);
    dxy.Scale (intervalScale);

    point = m_placement.frame * DPoint3d::From (xy);
    m_placement.frame.MultiplyMatrixOnly (tangent, dxy.x, dxy.y, 0.0);
    return true;
    }

bool _FractionToPoint (double f, DPoint3dR point, DVec3dR tangent, DVec3dR derivative2) const override
    {
    DPoint2d xy;
    DVec2d   d1xy, d2xy;
    double globalFraction = m_placement.ActiveFractionToGlobalFraction (f);
    double intervalScale = m_placement.fractionB - m_placement.fractionA;
    m_directSpiral->EvaluateAtFraction (globalFraction, xy, &d1xy, &d2xy, nullptr);
    d1xy.Scale (intervalScale);
    d2xy.Scale (intervalScale * intervalScale);

    point = m_placement.frame * DPoint3d::From (xy);
    m_placement.frame.MultiplyMatrixOnly (tangent, d1xy.x, d1xy.y, 0.0);
    m_placement.frame.MultiplyMatrixOnly (tangent, d2xy.x, d2xy.y, 0.0);
    return true;
    }

bool _FractionToPoint (double f, DPoint3dR point, DVec3dR tangent, DVec3dR derivative2, DVec3dR derivative3) const override
    {
    DPoint2d xy;
    DVec2d   d1xy, d2xy, d3xy;
    double globalFraction = m_placement.ActiveFractionToGlobalFraction (f);
    double intervalScale = m_placement.fractionB - m_placement.fractionA;
    m_directSpiral->EvaluateAtFraction (globalFraction, xy, &d1xy, &d2xy, &d3xy);

    d1xy.Scale (intervalScale);
    d2xy.Scale (intervalScale * intervalScale);
    d3xy.Scale (intervalScale * intervalScale * intervalScale);
    point = m_placement.frame * DPoint3d::From (xy);
    m_placement.frame.MultiplyMatrixOnly (tangent, d1xy.x, d1xy.y, 0.0);
    m_placement.frame.MultiplyMatrixOnly (tangent, d2xy.x, d2xy.y, 0.0);
    m_placement.frame.MultiplyMatrixOnly (tangent, d3xy.x, d3xy.y, 0.0);
    return true;
    }


MSBsplineCurveCP _GetProxyBsplineCurveCP() const override 
    {
    EnsureCachedCurve ();
    return m_curve.get();
    }

MSBsplineCurvePtr _GetProxyBsplineCurvePtr() const override 
    {
    EnsureCachedCurve ();
    return m_curve;
    }

public:

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      12/2015
+--------------------------------------------------------------------------------------*/
MSBsplineCurveCP _GetBsplineCurveCP() const override {return NULL;}  // Users only see the bspline as proxy!!
MSBsplineCurvePtr _GetBsplineCurvePtr() const override {return NULL;}  // Users only see the bspline as proxy!!



bool _IsSameStructureAndGeometry (ICurvePrimitiveCR other, double tolerance) const override
    {
    if (other.GetCurvePrimitiveType () != CURVE_PRIMITIVE_TYPE_Spiral)
        return false;

    CurvePrimitiveDirectSpiral const * otherSpiral = dynamic_cast <CurvePrimitiveDirectSpiral const *>(&other);
    return   otherSpiral != nullptr && m_placement.AlmostEqual (otherSpiral->m_placement, tolerance);
    }

ICurvePrimitivePtr _CloneAsSingleOffsetPrimitiveXY (CurveOffsetOptionsCR options) const override 
    {
    double offsetDistance = options.GetOffsetDistance ();
    EnsureCachedCurve ();
    MSBsplineCurvePtr offsetBCurve = m_curve->CreateCopyOffsetXY (SetBCurveOffsetSign (offsetDistance), SetBCurveOffsetSign (offsetDistance), options);
    if (offsetBCurve.IsValid ())
        return ICurvePrimitive::CreateBsplineCurve(offsetBCurve);
    return nullptr;
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
ICurvePrimitivePtr _CloneBetweenFractions
(
double fractionAnew,
double fractionBnew,
bool allowExtrapolation
) const override
    {
    return new CurvePrimitiveDirectSpiral (
                *m_directSpiral,
                m_placement.frame,
                DoubleOps::Interpolate (m_placement.fractionA, fractionAnew, m_placement.fractionB),
                DoubleOps::Interpolate (m_placement.fractionA, fractionBnew, m_placement.fractionB));
    }

void _AppendCurvePlaneIntersections(DPlane3dCR plane, bvector<CurveLocationDetailPair> &intersections, double tol) const override
    {
    AppendTolerancedPlaneIntersections (plane, this, *m_curve, intersections, tol);
    ImprovePlaneCurveIntersections (plane, this, intersections);
    }

bool _ClosestPointBounded (DPoint3dCR spacePoint, double &fraction, DPoint3dR curvePoint, bool extend0, bool extend1) const override
    {
    m_curve->ClosestPoint(curvePoint, fraction, spacePoint);
    ImprovePerpendicularProjection(this, spacePoint, fraction, curvePoint);
    return true;
    }

uint32_t NumStrokeBetweenActiveFractions (double activeA, double activeB, IFacetOptionsCP options) const
    {
    double globalA = m_placement.ActiveFractionToGlobalFraction (activeA);
    double globalB = m_placement.ActiveFractionToGlobalFraction (activeB);
    double df = fabs (globalB - globalA);
    if (df < 1.5 * m_cachedStrokeFraction)
        return 2;
    return (uint32_t) ( 0.999 + df / m_cachedStrokeFraction);
}

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      03/2016
+--------------------------------------------------------------------------------------*/
bool _AddStrokes (bvector <PathLocationDetail> &points, IFacetOptionsCR options,
                double startFraction,
                double endFraction
                ) const override
    {
    bvector<double> fractions;      // These are global fractions !!!
    uint32_t numStrokes = NumStrokeBetweenActiveFractions (startFraction, endFraction, &options);
    double globalA = m_placement.ActiveFractionToGlobalFraction (startFraction);
    double globalB = m_placement.ActiveFractionToGlobalFraction (endFraction);
    DPoint2d uv;
    DPoint3d xyz;

    BSIQuadraturePoints quadraturePoints;
    quadraturePoints.InitGauss (5);

    NominalLengthSpiralArcLengthIntegrands integrand (*m_directSpiral);
    double localError;

    double currDistance = 0.0;      // NO!!! SHOULD BE DISTANCE FROM activeFration = 0 !

    for (uint32_t i = 0; i <= numStrokes; i++)
        {
        double f = i / (double)numStrokes;
        double g = DoubleOps::Interpolate (globalA, f, globalB);
        m_directSpiral->EvaluateAtFraction (g, uv, nullptr, nullptr, nullptr);
        m_placement.frame.Multiply (xyz, uv.x, uv.y, 0.0);
        if (i > 0)
            {
            integrand.clear ();
            // um.. these are "distance from startFraction"
            quadraturePoints.IntegrateWithRombergExtrapolation (integrand, points[i-1].CurveFraction (), points[i].CurveFraction (), 1, localError);
            currDistance += integrand.LastDistance ();
            }

        points.push_back (PathLocationDetail (
            CurveLocationDetail (this, f, xyz), 0, currDistance));
        }
    return true;
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      03/2016
+--------------------------------------------------------------------------------------*/
bool _AddStrokes (bvector <DPoint3d> &points, IFacetOptionsCR options,
                bool includeStartPoint,
                double startFraction,
                double endFraction
                ) const override
    {
    if (DoubleOps::AlmostEqualFraction (startFraction, 0.0)
        && DoubleOps::AlmostEqualFraction (endFraction, 1.0))
        {
        for (size_t i = includeStartPoint ? 0 : 1; i < m_strokes.size (); i++)
            points.push_back (m_strokes[i]);
        return true;
        }

    bvector<double> fractions;      // These are global fractions !!!
    uint32_t numStrokes = NumStrokeBetweenActiveFractions (startFraction, endFraction, &options);
    double globalA = m_placement.ActiveFractionToGlobalFraction (startFraction);
    double globalB = m_placement.ActiveFractionToGlobalFraction (endFraction);
    uint32_t i0 = includeStartPoint ? 1 : 0;
    DPoint2d uv;
    DPoint3d xyz;
    for (auto i = i0; i <= numStrokes; i++)
        {
        double f = i / (double)numStrokes;
        double g = DoubleOps::Interpolate (globalA, f, globalB);
        m_directSpiral->EvaluateAtFraction (g, uv, nullptr, nullptr, nullptr);
        m_placement.frame.Multiply (xyz, uv.x, uv.y, 0.0);
        points.push_back (xyz);
        }
    return true;
    }


/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
bool _GetRange (DRange3dR range) const override
    {
    range = DRange3d::From (m_strokes); // may be slightly small of curve goes tangent to x or y axis
    return true;
    }


/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
bool _GetRange (DRange3dR range, TransformCR transform) const override
    {
    range = DRange3d::From (transform, m_strokes); // may be slightly small of curve goes tangent to x or y axis
    return true;
    }

}; // CurvePrimitiveDirectSpiral
//END_BENTLEY_GEOMETRY_NAMESPACE
