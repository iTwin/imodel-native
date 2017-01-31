/*--------------------------------------------------------------------------------------+
|
|     $Source: geom/src/CurvePrimitive/cp_spiral.h $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
//#include <bsibasegeomPCH.h>
// This code is to be included into CurvePrimitiveBsplineCurve.cpp  (INSIDE the namespace block)
//BEGIN_BENTLEY_GEOMETRY_NAMESPACE
extern double SetBCurveOffsetSign (double offsetDistance);

static double s_defaultStrokeRadians = 0.04;
static int s_defaultMinIntervals = 8;



struct CaptureSpiralStrokes : AnnounceDoubleDPoint2d
{
DPoint3dDoubleArrays &m_data;
DSpiral2dPlacement   &m_placement;
DPoint2d m_startUV;

CaptureSpiralStrokes (DPoint3dDoubleArrays &data, DSpiral2dPlacement &placement, DPoint2dCR startUV)
    : m_placement(placement),
    m_data(data),
    m_startUV (startUV)
    {
    }
void Announce (double f, DVec2dCR uv)
    {
    m_data.m_f.push_back (f);
    DPoint2d uvB = DPoint2d::FromSumOf (m_startUV, uv, 1.0);
    m_data.m_xyz.push_back (m_placement.frame * DPoint3d::From (uvB));
    }
};

struct CaptureSpiralPathLocationDetail : AnnounceDoubleDPoint2d
{
bvector<PathLocationDetail> &m_data;
DSpiral2dPlacement   const &m_placement;
ICurvePrimitiveCP      m_curve;
double m_baseDistance;
double m_baseFraction;
double m_referenceLength;
CaptureSpiralPathLocationDetail
(
bvector<PathLocationDetail> &data,
DSpiral2dPlacement const &placement,
ICurvePrimitiveCP curve,
double baseDistance,    // distance to store when announced fraction equals base fraction
double baseFraction,    // base fraction for distance referece
double referenceLength  // length for use in (f - baseFraction) * referenceLength
)
    : m_placement(placement),
    m_data(data),
    m_curve(curve),
    m_baseDistance (baseDistance),
    m_baseFraction (baseFraction),
    m_referenceLength (referenceLength)

    {
    }
void Announce (double f, DVec2dCR uv)
    {
    double distanceAlong = m_baseDistance + (f - m_baseFraction) * m_referenceLength;
    DPoint3d xyz = m_placement.frame * DPoint3d::From (uv);
    m_data.push_back (PathLocationDetail (CurveLocationDetail (m_curve, f, xyz), 0, distanceAlong));
    }
};

/*=================================================================================**//**
* @bsiclass                                                     EarlinLutz      03/2016
+===============+===============+===============+===============+===============+======*/
struct CurvePrimitiveSpiralCurve1 : public ICurvePrimitive
{

protected:
DSpiral2dPlacement m_placement;
MSBsplineCurvePtr m_curve;
DPoint3dDoubleArrays m_strokes;

void ReplaceCurve (MSBsplineCurvePtr source)
    {
    m_curve = source;
    }
/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      03/2016
+--------------------------------------------------------------------------------------*/
void UpdateCurve(double maxStrokeLength)
    {
    double errorEstimate;
    DVec2d uvA = DVec2d::From (0,0);
    if (m_placement.fractionA != 0.0)
        DSpiral2dBase::Stroke (*m_placement.spiral, 0.0, m_placement.fractionA, s_defaultStrokeRadians, uvA, errorEstimate, maxStrokeLength);

    MSBsplineCurve curve;
    DPoint3d origin;
    RotMatrix matrix;
    m_placement.frame.GetTranslation (origin);
    m_placement.frame.GetMatrix (matrix);
    if (SUCCESS == bspcurv_curveFromDSpiral2dBaseInterval (&curve, m_placement.spiral,
                m_placement.fractionA, m_placement.fractionB, &origin, &matrix, maxStrokeLength))
        {
        ReplaceCurve (curve.CreateCapture ());
        }
    // build stroked approximation . . .
    double error;
    m_strokes.m_xyz.clear ();
    m_strokes.m_f.clear ();
    CaptureSpiralStrokes capture(m_strokes, m_placement, uvA);
    DSpiral2dBase::StrokeToAnnouncer (*m_placement.spiral, m_placement.fractionA, m_placement.fractionB, s_defaultStrokeRadians, capture, error, s_defaultMinIntervals);
    // fractions are relative to the parent spiral. Normalize to the actual ...
    DoubleOps::Normalize (m_strokes.m_f, m_placement.fractionA, m_placement.fractionB);
#ifndef NDEBUG
    static SmallIntegerHistogram s_spiralStrokeCounter (100);
    s_spiralStrokeCounter.Record (m_strokes.m_xyz.size ());
        double dSpiral = m_placement.spiral->mLength;
        double dStrokes = PolylineOps::Length (m_strokes.m_xyz, false);
        BeAssert (dStrokes <= fabs (dSpiral));
#endif

    }
public:

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      03/2016
+--------------------------------------------------------------------------------------*/
DSpiral2dPlacementCP _GetSpiralPlacementCP() const override {return &m_placement;}

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      03/2016
+--------------------------------------------------------------------------------------*/
void _Process(ICurvePrimitiveProcessor &processor, DSegment1dCP interval) const override 
    {
    processor._ProcessSpiral (*this, m_placement, interval);
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      03/2016
+--------------------------------------------------------------------------------------*/
bool _FractionToPoint (double f, DPoint3dR point) const override
    {
    double f0, f1;
    DPoint3d xyz0, xyz1;
    size_t i0, i1;
    if (m_strokes.SearchBracketPoints (f, i0, f0, xyz0, i1, f1, xyz1))
        {
        if (fabs (f - f0) < fabs (f - f1))
            {
            auto delta = m_placement.DisplacementBetweenActiveFractions (f0, f);
            point = xyz0 + delta;
            }
        else
            {
            auto delta = m_placement.DisplacementBetweenActiveFractions (f1, f);
            point = xyz1 + delta;
            }
        return true;
        }
    return false;
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      03/2016
+--------------------------------------------------------------------------------------*/
bool _FractionToPoint (double f, DPoint3dR point, DVec3dR derivative1) const override
    {
    DVec3d derivative2, derivative3;
    return _FractionToPoint (f, point, derivative1, derivative2, derivative3);
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      03/2016
+--------------------------------------------------------------------------------------*/
bool _FractionToPoint (double f, DPoint3dR point, DVec3dR derivative1, DVec3dR derivative2) const override
    {
    DVec3d derivative3;
    return _FractionToPoint (f, point, derivative1, derivative2, derivative3);
    }


/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      03/2016
+--------------------------------------------------------------------------------------*/
bool _FractionToPoint (double f, DPoint3dR point, DVec3dR derivative1, DVec3dR derivative2, DVec3dR derivative3) const override
    {
    double f0, f1;
    DPoint3d xyz0, xyz1;
    size_t i0, i1;
    if (m_strokes.SearchBracketPoints (f, i0, f0, xyz0, i1, f1, xyz1))
        {
        RotMatrix derivatives = m_placement.ActiveFractionToDerivatives (f);
        derivatives.GetColumns (derivative1, derivative2, derivative3);
        if (fabs (f - f0) < fabs (f - f1))
            {
            auto delta = m_placement.DisplacementBetweenActiveFractions (f0, f);
            point = xyz0 + delta;
            }
        else
            {
            auto delta = m_placement.DisplacementBetweenActiveFractions (f1, f);
            point = xyz1 + delta;
            }
        return true;
        }
    return false;
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      03/2016
+--------------------------------------------------------------------------------------*/
bool _GetRange (DRange3dR range) const override
    {
    range = m_strokes.GetRange ();
    return true;
    }


/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      03/2016
+--------------------------------------------------------------------------------------*/
bool _GetRange (DRange3dR range, TransformCR transform) const override
    {
    range = m_strokes.GetRange (transform);
    return false;
    }
/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      03/2016
+--------------------------------------------------------------------------------------*/
explicit CurvePrimitiveSpiralCurve1(
DSpiral2dBaseCR spiral,
TransformCR frame,
double fractionA,
double fractionB,
double maxStrokeLength = 10.0
) : m_placement (spiral, frame, fractionA, fractionB)
    {
    UpdateCurve (maxStrokeLength);
    }
~CurvePrimitiveSpiralCurve1 ()
    {
    m_placement.ReplaceSpiral (NULL);
    }


/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      03/2016
+--------------------------------------------------------------------------------------*/
ICurvePrimitivePtr _Clone() const override
    {
    return Create (*m_placement.spiral, m_placement.frame, m_placement.fractionA, m_placement.fractionB);
    }


/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      03/2016
+--------------------------------------------------------------------------------------*/
CurvePrimitiveType _GetCurvePrimitiveType() const override {return CURVE_PRIMITIVE_TYPE_Spiral;}
// Need query method for spiral access??
bool _TransformInPlace (TransformCR transform) override
    {
    // Hmmmm.. If it's a non-rigid transform it changes the distance parameterization.
    if (m_curve.IsValid())
        m_curve->TransformCurve (transform);

    transform.Multiply (m_strokes.m_xyz, m_strokes.m_xyz);
    double scale;
    if (transform.IsRigidScale (scale))
        {
        RotMatrix matrixA, matrixA1, matrixB;
        DPoint3d  originA;
        transform.GetTranslation (originA);
        transform.GetMatrix (matrixA);
        matrixA1 = matrixA;
        DVec3d originB = DVec3d::FromTranslation (m_placement.frame);
        m_placement.frame.GetMatrix (matrixB);
        // [sQ a][R b] = [sQ*R sQb+a]
        // but we save it as [Q*R sQb+a]
        double a = 1.0 / scale;
        matrixA1.ScaleColumns (a,a,a);  // the unit-scale copy of matrixA !!
        RotMatrix matrixC = matrixA1 * matrixB;
        DPoint3d originC =  originA + matrixA * originB;

        m_placement.frame = Transform::From (matrixC, originC);
        m_placement.spiral->ScaleInPlace (scale);
        }
    else        
        m_placement.frame.InitProduct (transform, m_placement.frame);
    return true;
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      03/2016
+--------------------------------------------------------------------------------------*/
bool _ReverseCurvesInPlace () override
    {
    if (m_placement.ReverseInPlace ())
        {
        m_curve->MakeReversed ();
        m_strokes.ReverseXF (true);
        return true;
        }
    return false;
    }

size_t _NumComponent () const override {return 1;}
bool _IsExtensibleFractionSpace() const override {return true;}
bool _IsMappableFractionSpace() const override {return true;}
bool _IsFractionSpace() const override {return true;}
bool _IsPeriodicFractionSpace(double &period) const override { return false;}

ICurvePrimitivePtr _CloneBetweenFractions
(
double fractionA,
double fractionB,
bool allowExtrapolation
) const override
    {
    double f0 = DoubleOps::Interpolate (m_placement.fractionA, fractionA, m_placement.fractionB);
    double f1 = DoubleOps::Interpolate (m_placement.fractionA, fractionB, m_placement.fractionB);
    return Create (*m_placement.spiral, m_placement.frame, f0, f1);
    }
public:

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      03/2016
+--------------------------------------------------------------------------------------*/
MSBsplineCurveCP _GetBsplineCurveCP() const override {return NULL;}  // Users do not see the bspline !!
MSBsplineCurvePtr _GetBsplineCurvePtr() const override {return NULL;}  // Users do not see the bspline !!

static ICurvePrimitive* Create
(
DSpiral2dBaseCR spiral,
TransformCR frame,
double fractionA,
double fractionB,
double maxStrokeLength = DEFAULT_SPIRAL_MAX_STROKE_LENGTH
)
    {
    return new CurvePrimitiveSpiralCurve1 (spiral, frame, fractionA, fractionB, maxStrokeLength);
    }

bool _IsSameStructureAndGeometry (ICurvePrimitiveCR other, double tolerance) const override
    {
    DSpiral2dPlacementCP placementA = GetSpiralPlacementCP ();
    DSpiral2dPlacementCP placementB = other.GetSpiralPlacementCP ();
    return   nullptr != placementA
          && nullptr != placementB
          && placementA->AlmostEqual (*placementB, tolerance);
    }

ICurvePrimitivePtr _CloneAsSingleOffsetPrimitiveXY (CurveOffsetOptionsCR options) const override 
    {
    // ASSUME m_stroke is a reasonable number of base points.
    bvector<DPoint3d> fitPoints;
    bvector<double>fitFractions;
    int order = 4;
    size_t numFitPoints = m_strokes.m_xyz.size () + 2;  // Greville knots will hit the uniform spacing, plus extra in first/last intervals.
    DoubleOps::MovingAverages (fitFractions, 0.0, 1.0, numFitPoints, order - 1, order - 1, order - 1);
    double offsetDistance = options.GetOffsetDistance ();   // positive is to the RIGHT of the tangent in xy view
    DPoint3d xyzA, xyzB;
    DVec3d tangent;
    for (auto f : fitFractions)
        {
        FractionToPoint (f, xyzA, tangent);
        auto offsetDirection = DVec3d::FromUnitPerpendicularXY (tangent);   // this rotates to left -- negate the offset ...
        if (offsetDirection.IsValid ())
            {
            xyzB = xyzA - offsetDistance * offsetDirection.Value ();
            }
        else
            {
            Transform frame;
            FractionToFrenetFrame (f, frame);
            xyzB = xyzA - offsetDistance * DVec3d::FromMatrixColumn (frame, 2);
            }
        fitPoints.push_back (xyzB);
        }
    auto offsetBCurve = MSBsplineCurve::CreateFromInterpolationAtBasisFunctionPeaks (fitPoints, order, 1);
    if (offsetBCurve.IsValid ())
        return ICurvePrimitive::CreateBsplineCurve(offsetBCurve);
    return NULL;
    }

    struct CaptureXYZ : AnnounceDoubleDPoint2d
    {
    bvector<DPoint3d> &m_xyz;
    DSpiral2dPlacement const &m_placement;
    bool m_includeNextPoint;
    CaptureXYZ (DSpiral2dPlacement const &placement, bvector<DPoint3d> &xyz, bool includeStart)
        : m_xyz (xyz), m_includeNextPoint (includeStart), m_placement (placement)
        {
        }
    void Announce (double fraction, DVec2dCR xy)
        {
        DPoint3d xyz = m_placement.frame * DPoint3d::From (xy);
        if (m_includeNextPoint
            || m_xyz.size () == 0
            || (m_xyz.size () > 0 && !xyz.AlmostEqual (m_xyz.back ()))
            )
            {
            m_xyz.push_back (xyz);
            }
        m_includeNextPoint = true;
        }
    };

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      03/2016
+--------------------------------------------------------------------------------------*/
bool _AddStrokes (bvector <DPoint3d> &points, IFacetOptionsCR options,
                bool includeStartPoint,
                double startFraction,
                double endFraction
                ) const override
    {
    CaptureXYZ capture (m_placement, points, includeStartPoint);
    double error;
    if (options.GetAngleTolerance () < s_defaultStrokeRadians
        || !DoubleOps::IsExact01 (startFraction, endFraction)
        )
        return DSpiral2dBase::StrokeToAnnouncer (*m_placement.spiral,
                    startFraction, endFraction, options.GetAngleTolerance (), capture, error);
    else
        {
        size_t i0 = 0;
        if (!includeStartPoint
            && points.size () > 0
            && m_strokes.m_xyz[0].AlmostEqual (points.back ())
            )
            i0 = 1;
#ifndef NDEBUG
        double dSpiral = m_placement.spiral->mLength;
        double dStrokes = PolylineOps::Length (m_strokes.m_xyz, false);
        BeAssert (dStrokes <= dSpiral);
#endif
        for (size_t i = i0; i < m_strokes.m_xyz.size (); i++)
            points.push_back (m_strokes.m_xyz[i]);
        return true;
        }    
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      03/2016
+--------------------------------------------------------------------------------------*/
bool _Length (double &length) const override
    {
    // needs work -- placement scale?
    length = m_placement.SpiralLengthActiveInterval ();
    return true;
    }
// NOTE: Do not implement _FastLength () -- the default comes here, and because it's a spiral it's quick.
// NOTE: Do not implement _FastMaxAbs () -- the default uses the stroked range.

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      05/15
+--------------------------------------------------------------------------------------*/
bool _Length(RotMatrixCP worldToLocal, double &length) const  override
    {
    RotMatrix matrix = RotMatrix::FromIdentity ();
    if (nullptr != worldToLocal)
        matrix = *worldToLocal;
    length = m_placement.MappedSpiralLengthActiveInterval (matrix);
    return true;
    }

MSBsplineCurveCP _GetProxyBsplineCurveCP () const override {return m_curve.get ();}
MSBsplineCurvePtr _GetProxyBsplineCurvePtr() const override { return m_curve; }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      03/2016
+--------------------------------------------------------------------------------------*/
bool _SignedDistanceBetweenFractions (double startFraction, double endFraction, double &signedDistance) const override
    {
    signedDistance = (endFraction - startFraction) * m_placement.SpiralLengthActiveInterval ();
    return true;
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      03/2016
+--------------------------------------------------------------------------------------*/
bool _ClosestPointBounded (DPoint3dCR spacePoint, double &fraction, DPoint3dR point, bool extend0, bool extend1) const override
    {
    size_t componentIndex, numComponent;
    double componentFraction;
    DPoint3d strokePoint;
    if (PolylineOps::ClosestPoint (m_strokes.m_xyz, false, spacePoint, fraction, strokePoint,
                componentIndex, numComponent, componentFraction,
                false, false))
        {
        // ASSUME -- spiral is a small flat arc.
        //   Anything beyond the end is the end.
        point = strokePoint;
        if (fraction <= 0.0 || fraction >= 1.0)
            return true;
        return ImprovePerpendicularProjection (this, spacePoint, fraction, point);
        }
    fraction = 0.0;
    point.Zero ();
    return false;
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      03/2016
+--------------------------------------------------------------------------------------*/
bool _ClosestPointBoundedXY
(
DPoint3dCR spacePoint,
DMatrix4dCP worldToLocal,
CurveLocationDetailR detail,
bool extend0,
bool extend1
) const override
    {
    detail          = CurveLocationDetail (this, 1);
    PolylineOps::ClosestPointXY (m_strokes.m_xyz, false, spacePoint, worldToLocal,
                detail.fraction, detail.point, detail.componentIndex, detail.numComponent, detail.componentFraction,
                detail.a, extend0, extend1);
    // ASSUME -- spiral is a small flat arc.
    //   Anything beyond the end is the end.
    if (detail.fraction <= 0.0 || detail.fraction >= 1.0)
        return true;
    return ImprovePerpendicularProjectionXY (this, spacePoint, detail.fraction, detail.point, worldToLocal);
    }
/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      03/2016
+--------------------------------------------------------------------------------------*/
bool _SignedDistanceBetweenFractions (RotMatrixCP worldToLocal, double startFraction, double endFraction, double &signedDistance) const override
    {
    // NEEDS WORK -- scaled or skewed world to local !!!
    signedDistance = (endFraction - startFraction) * m_placement.SpiralLengthActiveInterval ();
    return true;
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      05/2016
+--------------------------------------------------------------------------------------*/
bool _PointAtSignedDistanceFromFraction (double startFraction, double signedLength, bool allowExtension, CurveLocationDetailR location) const override
    {
    double totalLength = m_placement.SpiralLengthActiveInterval ();
    double fractionDelta = signedLength / totalLength;
    double fraction = startFraction + fractionDelta;
    if (!allowExtension)
        fraction = DoubleOps::ClampFraction (fraction);
    bool stat = FractionToPoint (fraction, location);
    location.a = (fraction - startFraction) * totalLength;
    return stat;
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      03/2016
+--------------------------------------------------------------------------------------*/
bool _AddStrokes (bvector <PathLocationDetail> &points, IFacetOptionsCR options,
                double startFraction,
                double endFraction
                ) const override
    {
    DSegment1d segment (startFraction, endFraction);
    double f0 = m_strokes.m_f.front ();
    double baseDistance = points.empty () ? 0.0 : points.back ().DistanceFromPathStart ();
    double totalLength;
    totalLength = m_placement.SpiralLength01 ();
    if (options.GetAngleTolerance () < s_defaultStrokeRadians
        || !segment.Is01 ()
        )
        {
        double error;
        CaptureSpiralPathLocationDetail capture (points, m_placement, this, baseDistance, f0, totalLength);
        return DSpiral2dBase::StrokeToAnnouncer (*m_placement.spiral,
                    startFraction, endFraction, options.GetAngleTolerance (), capture, error);
        }
    else
        {
        for (size_t i = 0; i < m_strokes.m_xyz.size (); i++)
            {
            double f = m_strokes.m_f[i];
            double distanceAlong = baseDistance + (f - f0) * totalLength;
            points.push_back (PathLocationDetail
                (CurveLocationDetail (this, f, m_strokes.m_xyz[i]), 0, distanceAlong));
            }
        }
    return true;
    }    
// TODO??
// _AnnounceKeyPoints?
// _PointAtSignedDistanceFromFraction (RotMatrix
// 
void  _AppendCurvePlaneIntersections (DPlane3dCR plane, bvector<CurveLocationDetailPair> &intersections, double tolerance) const override
    {
    size_t i0 = intersections.size ();
    // Intersection with strokes is first approximation ...
    AppendTolerancedPlaneIntersections (plane, this, m_strokes.m_xyz, intersections, tolerance);
    // and revise with standard iterator ...
    for (size_t i = i0; i < intersections.size (); i++)
        {
        ImprovePlaneCurveIntersection (plane, this, intersections[i]);
        }
    }
// virtual bool _WireCentroid (double &length, DPoint3dR centroid, double fraction0, double fraction1) const;

size_t _GetStrokeCount (IFacetOptionsCR options, double startFraction, double endFraction) const override
    {
    double startDistance = m_placement.spiral->FractionToDistance (startFraction);
    double endDistance   = m_placement.spiral->FractionToDistance (endFraction);
    double distanceChange = endDistance - startDistance;
    double angleChange = m_placement.spiral->DistanceToLocalAngle (endDistance) - m_placement.spiral->DistanceToLocalAngle (startDistance);
    return options.DistanceAndTurnStrokeCount (distanceChange, angleChange);
    }

}; // CurvePrimitiveSpiralCurve


bool ICurvePrimitive::CreateSpiralsStartShoulderTarget
(
int transitionType,
DPoint3dCR pointA,
DPoint3dCR pointB,
DPoint3dCR pointC,
ICurvePrimitivePtr &primitiveA,
ICurvePrimitivePtr &primitiveB
)
    {
    Transform localToWorld, worldToLocal;
    primitiveA = primitiveB = nullptr;
    if (localToWorld.InitNormalizedFrameFromOriginXPointYPoint (pointA, pointB, pointC))
        {
        worldToLocal.InvertRigidBodyTransformation (localToWorld);
        DPoint3d localA = DPoint3d::From (0, 0, 0);
        DPoint3d localB = worldToLocal * pointB;
        DPoint3d localC = worldToLocal * pointC;
        DPoint2d localP, localQ;
        DSpiral2dBase *spiralA = DSpiral2dBase::Create (transitionType);
        DSpiral2dBase *spiralB = DSpiral2dBase::Create (transitionType);
        if (DSpiral2dBase::SymmetricPointShoulderTargetTransition (
            DPoint2d::From (localA), DPoint2d::From (localB), DPoint2d::From (localC),
            *spiralA,
            *spiralB,
            localP, localQ))
            {
            DVec2d xVec = DVec2d::From (1.0, 0.0);

            Transform frameA = localToWorld * Transform::FromOriginAndXVector (DPoint2d::From (0, 0), xVec);
            Transform frameB = localToWorld * Transform::FromOriginAndXVector (localP, xVec);

            primitiveA = ICurvePrimitive::CreateSpiral (*spiralA, frameA, 0.0, 1.0);
            primitiveB = ICurvePrimitive::CreateSpiral (*spiralB, frameB, 0.0, 1.0);
            return true;
            }
        }
    return false;
    }

CurveVectorPtr CurveVector::CreateSpiralLineToLineShift
(
int transitionType,
DPoint3dCR pointA,
DPoint3dCR shoulderB,
DPoint3dCR shoulderC,
DPoint3dCR pointD
)
    {
    ICurvePrimitivePtr spiralA, spiralB, spiralC, spiralD;
    ICurvePrimitive::CreateSpiralsStartShoulderTarget (DSpiral2dBase::TransitionType_Clothoid, pointA, shoulderB, shoulderC, spiralA, spiralB);

    ICurvePrimitive::CreateSpiralsStartShoulderTarget (DSpiral2dBase::TransitionType_Clothoid, pointD, shoulderC, shoulderB, spiralD, spiralC);

    auto cv = CurveVector::Create (CurveVector::BOUNDARY_TYPE_Open);
    DPoint3d xyzB1 = pointA;
    if (spiralA.IsValid () && spiralB.IsValid ())
        {
        cv->push_back (spiralA);
        cv->push_back (spiralB);
        spiralB->FractionToPoint (1.0, xyzB1);
        }


    DPoint3d xyzC0 = pointD;
    if (spiralC.IsValid () && spiralD.IsValid ())
        {
        spiralC->ReverseCurvesInPlace ();
        spiralD->ReverseCurvesInPlace ();
        spiralC->FractionToPoint (0.0, xyzC0);
        }
    
    if (!xyzB1.AlmostEqual (xyzC0))
        cv->push_back (ICurvePrimitive::CreateLine (DSegment3d::From (xyzB1, xyzC0)));

    if (spiralC.IsValid () && spiralD.IsValid ())
        {
        cv->push_back (spiralC);
        cv->push_back (spiralD);
        }

    return cv;
    }



//END_BENTLEY_GEOMETRY_NAMESPACE
