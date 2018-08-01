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
// Vector integrands for testing
struct NominalLengthSpiralArcLengthIntegrands : BSIIncrementalVectorIntegrand
{
DSpiral2dFractionOfNominalLengthCurve &m_directSpiral;
NominalLengthSpiralArcLengthIntegrands (DSpiral2dFractionOfNominalLengthCurve &directSpiral)
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
bvector<double> fraction;
bvector<double> summedDistance;
bool AnnounceIntermediateIntegral (double t, double *pIntegrals) override
    {
    fraction.push_back (t);
    summedDistance.push_back (pIntegrals[0]);
    return true;
    }
void clear ()
    {
    fraction.clear ();
    summedDistance.clear ();
    }
double LastDistance () const { return summedDistance.empty () ? 0.0 : summedDistance.back ();}
};
/*=================================================================================**//**
* @bsiclass                                                      EarlinLutz   12/2015
!! The bspline approximation is not evaluated until requested as proxy !!!
!! The placement's raw pointer is DSpiral2dBase, but this class only constructs it with DSpiral2dFractionOfNominalLengthCurve.
+===============+===============+===============+===============+===============+======*/
struct CurvePrimitiveDirectSpiral : public ICurvePrimitive
{
typedef CurvePrimitiveBsplineCurve Super;

protected:
mutable MSBsplineCurvePtr m_curve;
DSpiral2dPlacement m_placement;    // for outside use
DSpiral2dFractionOfNominalLengthCurve *m_directSpiral;    // same pointer bits, but strongly typed.
mutable bvector<DPoint3d> m_strokes;
mutable bvector<double>   m_fraction;
mutable bvector<double>   m_trueDistance;
mutable double m_cachedStrokeFraction;

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      12/2015
+--------------------------------------------------------------------------------------*/
void CreateCachedCurve() const
    {
    // hm.. user stroker to get point count ... the strokes are thrown away
    bvector<double> fractions;
    bvector<DPoint3d> points;
    double dBeta = fabs (m_directSpiral->mTheta1 - m_directSpiral->mTheta0);
    double dNumStroke = ceil (dBeta / DEFAULT_DIRECT_SPIRAL_STROKE_RADIANS);
    uint32_t iNumStroke = (uint32_t)dNumStroke;
    if (iNumStroke < DEFAULT_DIRECT_SPIRAL_MIN_STROKES)
        iNumStroke = DEFAULT_DIRECT_SPIRAL_MIN_STROKES;
    m_strokes.clear ();
    m_fraction.clear ();
    m_trueDistance.clear ();
    m_cachedStrokeFraction = 1.0 / (double)iNumStroke;
    for (uint32_t i = 0; i <= iNumStroke; i++)
        {
        double fraction = ((double)i / (double)iNumStroke);
        m_fraction.push_back (fraction);
        DPoint2d xy;
        m_directSpiral->EvaluateAtFraction (fraction, xy, nullptr, nullptr, nullptr);
        DPoint3d xyz = DPoint3d::From (xy);
        m_placement.frame.Multiply (xyz);
        m_strokes.push_back (xyz);
        }

    BSIQuadraturePoints quadraturePoints;
    quadraturePoints.InitGauss (5);

    m_trueDistance.push_back (0.0);
    NominalLengthSpiralArcLengthIntegrands integrand (*m_directSpiral);
    double localError;
    double currentDistance = 0.0;
    for (uint32_t i = 1; i < m_fraction.size (); i++)
        {
        integrand.clear ();
        quadraturePoints.IntegrateWithRombergExtrapolation (integrand, m_fraction[i-1], m_fraction[i], 1, localError);
        currentDistance += integrand.LastDistance ();
        m_trueDistance.push_back (currentDistance);
        }
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
    // we wish .... processor._ProcessCatenary (*this, m_placement, interval);
    }


/*--------------------------------------------------------------------------------**//**
// CAPTURE placement
* @bsimethod                                                    EarlinLutz      12/2015
+--------------------------------------------------------------------------------------*/
explicit CurvePrimitiveDirectSpiral(
DSpiral2dFractionOfNominalLengthCurve const & directSpiral,
TransformCR frame,
double fractionA,
double fractionB
)
    {
    m_directSpiral = (DSpiral2dFractionOfNominalLengthCurve*)directSpiral.Clone ();
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
CurvePrimitiveType _GetCurvePrimitiveType() const override {return CURVE_PRIMITIVE_TYPE_Catenary;}

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
bool _PointAtSignedDistanceFromFraction (double startFraction, double signedDistance, bool allowExtension, CurveLocationDetailR location) const override
    {
    // NEEDS WORK -- integrate on true curve
        double endFraction;
        double actualDistance;
        m_curve->FractionAtSignedDistance (startFraction, signedDistance, endFraction, actualDistance);
        location = CurveLocationDetail (this, 1);
        location.SetSingleComponentFractionAndA (endFraction, actualDistance);
        m_curve->FractionToPoint (location.point, endFraction);
        return true;
    }

bool _FractionToPoint (double f, DPoint3dR point) const override
    {
    DPoint2d xy;
    m_directSpiral->EvaluateAtFraction (f, xy, nullptr, nullptr, nullptr);
    point = m_placement.frame * DPoint3d::From (xy);
    return true;
    }

bool _FractionToPoint (double f, DPoint3dR point, DVec3dR tangent) const override
    {
    DPoint2d xy;
    DVec2d   dxy;
    m_directSpiral->EvaluateAtFraction (f, xy, &dxy, nullptr, nullptr);
    point = m_placement.frame * DPoint3d::From (xy);
    m_placement.frame.MultiplyMatrixOnly (tangent, dxy.x, dxy.y, 0.0);
    return true;
    }

bool _FractionToPoint (double f, DPoint3dR point, DVec3dR tangent, DVec3dR derivative2) const override
    {
    DPoint2d xy;
    DVec2d   d1xy, d2xy;
    m_directSpiral->EvaluateAtFraction (f, xy, &d1xy, &d2xy, nullptr);
    point = m_placement.frame * DPoint3d::From (xy);
    m_placement.frame.MultiplyMatrixOnly (tangent, d1xy.x, d1xy.y, 0.0);
    m_placement.frame.MultiplyMatrixOnly (tangent, d2xy.x, d2xy.y, 0.0);
    return true;
    }

bool _FractionToPoint (double f, DPoint3dR point, DVec3dR tangent, DVec3dR derivative2, DVec3dR derivative3) const override
    {
    DPoint2d xy;
    DVec2d   d1xy, d2xy, d3xy;
    m_directSpiral->EvaluateAtFraction (f, xy, &d1xy, &d2xy, &d3xy);
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
    if (other.GetCurvePrimitiveType () != CURVE_PRIMITIVE_TYPE_Catenary)
        return false;

    CurvePrimitiveDirectSpiral const * otherCatenary = dynamic_cast <CurvePrimitiveDirectSpiral const *>(&other);
    return   otherCatenary != nullptr && m_placement.AlmostEqual (otherCatenary->m_placement, tolerance);
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
#ifdef CatenaryDerivesFromBspline
    if (Super::_ClosestPointBounded (spacePoint, fraction, curvePoint, extend0, extend1))
        {
        ImprovePerpendicularProjection (this, spacePoint, fraction, curvePoint);   
        return true;
        }
    return false;
#else
    m_curve->ClosestPoint(curvePoint, fraction, spacePoint);
    ImprovePerpendicularProjection(this, spacePoint, fraction, curvePoint);
    return true;
#endif
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
