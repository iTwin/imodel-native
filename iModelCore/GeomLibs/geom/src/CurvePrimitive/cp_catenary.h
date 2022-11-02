/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
//#include <bsibasegeomPCH.h>
// This code is to be included into CurvePrimitiveBsplineCurve.cpp  (INSIDE the namespace block)
//BEGIN_BENTLEY_GEOMETRY_NAMESPACE

#define DEFAULT_CATENARY_STROKE_RADIANS (0.05)

/*=================================================================================**//**
* @bsiclass
!! The bspline approximation is not evaluated until requested as proxy !!!
+===============+===============+===============+===============+===============+======*/
struct CurvePrimitiveCatenaryCurve : public ICurvePrimitive
{
typedef CurvePrimitiveBsplineCurve Super;

protected:
DCatenary3dPlacement m_placement;
MSBsplineCurvePtr m_curve;

/*--------------------------------------------------------------------------------**//**
* @bsimethod
+--------------------------------------------------------------------------------------*/
void CreateCachedCurve()
    {
#define CatenaryGrevilleFit
#ifdef CatenaryGrevilleFit
    // hm.. user stroker to get point count ... the strokes are thrown away
    bvector<double> fractions;
    bvector<DPoint3d> points;
    m_placement.Stroke (points, fractions, 0.0, 1.0, 0.0, DEFAULT_CATENARY_STROKE_RADIANS, 0.0);
    m_curve = MSBsplineCurve::CreateFromInterpolationAtGrevilleKnots (*this, fractions.size (), 3, true, 0);
#else
    bvector<double> fractions;
    bvector<DPoint3d> points;
    m_placement.Stroke (points, fractions, DEFAULT_STROKE_RADIANS);
#ifdef LimFit
    auto bcurve = MSBsplineCurve::CreateFromInterpolationAtBasisFunctionPeaks (points, 4, 0);
    ReplaceCurve (bcurve);
#else
    MSBsplineCurve bcurve;
    bcurve.InitFromInterpolatePoints (&points[0], (int)points.size (), 3, false, nullptr, nullptr, false, 4);
    m_curve = bcurve.CreateCapture ();
#endif
#endif
    }

void EnsureCachedCurve () const
    {
    if (!m_curve.IsValid ())
        const_cast <CurvePrimitiveCatenaryCurve *> (this) ->CreateCachedCurve ();
    }
public:

/*--------------------------------------------------------------------------------**//**
* @bsimethod
+--------------------------------------------------------------------------------------*/
bool _TryGetCatenary (DCatenary3dPlacementR data) const override {data = m_placement; return true;}

/*--------------------------------------------------------------------------------**//**
* @bsimethod
+--------------------------------------------------------------------------------------*/
void _Process(ICurvePrimitiveProcessor &processor, DSegment1dCP interval) const override 
    {
    EnsureCachedCurve ();
    processor._ProcessBsplineCurve(*this, *_GetProxyBsplineCurvePtr (), interval);
    // we wish .... processor._ProcessCatenary (*this, m_placement, interval);
    }


/*--------------------------------------------------------------------------------**//**
* @bsimethod
+--------------------------------------------------------------------------------------*/
explicit CurvePrimitiveCatenaryCurve(
double a,
DPoint3dDVec3dDVec3dCR basis,
double distance0,
double distance1
) : m_placement (a, basis, distance0, distance1)
    {
    }
explicit CurvePrimitiveCatenaryCurve(DCatenary3dPlacement const &placement)
    : m_placement (placement)
    {
    }


~CurvePrimitiveCatenaryCurve ()
    {
    }


/*--------------------------------------------------------------------------------**//**
* @bsimethod
+--------------------------------------------------------------------------------------*/
ICurvePrimitivePtr _Clone() const override
    {
    return new CurvePrimitiveCatenaryCurve (m_placement);
    }


/*--------------------------------------------------------------------------------**//**
* @bsimethod
+--------------------------------------------------------------------------------------*/
CurvePrimitiveType _GetCurvePrimitiveType() const override {return CURVE_PRIMITIVE_TYPE_Catenary;}

// Need query method for Catenary access??
bool _TransformInPlace (TransformCR transform) override
    {
    //Super::_TransformInPlace (transform);
    m_placement.MultiplyInPlace (transform);
    return true;
    }

size_t _NumComponent() const override { return 1; }

/*--------------------------------------------------------------------------------**//**
* @bsimethod
+--------------------------------------------------------------------------------------*/
bool _ReverseCurvesInPlace () override
    {
    m_placement.ReverseInPlace ();
    //Super::_ReverseCurvesInPlace ();
    return true;
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod
+--------------------------------------------------------------------------------------*/
bool _Length (double &length) const override
    {
    length = m_placement.m_distanceLimits.Length();
    return true;
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod
+--------------------------------------------------------------------------------------*/
bool _Length(RotMatrixCP worldToLocal, double &length) const override
    {
    length = 0.0;
    return false;
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod
+--------------------------------------------------------------------------------------*/
bool _PointAtSignedDistanceFromFraction (double startFraction, double signedLength, bool allowExtension, CurveLocationDetailR location) const override
    {
    // ignore allowExtension -- the base class non-virtual is handling that before/after calling this virtual
    double startDistance = m_placement.m_distanceLimits.FractionToPoint (startFraction);
    double endDistance = startDistance + signedLength;
    double endFraction;
    if (m_placement.m_distanceLimits.PointToFraction (endDistance, endFraction))
        {
        return FractionToPoint (endFraction, location);
        }
    location = CurveLocationDetail ();
    return false;
    }

bool _FractionToPoint (double f, DPoint3dR point) const override
    {
    point = m_placement.FractionToPoint (f);
    return true;
    }

bool _FractionToPoint (double f, DPoint3dR point, DVec3dR tangent) const override
    {
    auto ray = m_placement.FractionToPointAndTangent (f);
    point = ray.origin;
    tangent = ray.direction;
    return true;
    }

bool _FractionToPoint (double f, DPoint3dR point, DVec3dR tangent, DVec3dR derivative2) const override
    {
    auto basis  = m_placement.FractionToPointAndDerivatives (f);
    DVec3d derivative3;
    basis.GetOriginAndVectors (point, tangent, derivative2, derivative3);
    return true;
    }

bool _FractionToPoint (double f, DPoint3dR point, DVec3dR tangent, DVec3dR derivative2, DVec3dR derivative3) const override
    {
    auto basis  = m_placement.FractionToPointAndDerivatives (f);
    basis.GetOriginAndVectors (point, tangent, derivative2, derivative3);
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
* @bsimethod
+--------------------------------------------------------------------------------------*/
MSBsplineCurveCP _GetBsplineCurveCP() const override {return NULL;}  // Users only see the bspline as proxy!!
MSBsplineCurvePtr _GetBsplineCurvePtr() const override {return NULL;}  // Users only see the bspline as proxy!!



bool _IsSameStructureAndGeometry (ICurvePrimitiveCR other, double tolerance) const override
    {
    if (other.GetCurvePrimitiveType () != CURVE_PRIMITIVE_TYPE_Catenary)
        return false;

    CurvePrimitiveCatenaryCurve const * otherCatenary = dynamic_cast <CurvePrimitiveCatenaryCurve const *>(&other);
    return   otherCatenary != nullptr && m_placement.AlmostEqual (otherCatenary->m_placement, tolerance);
    }
/*--------------------------------------------------------------------------------**//**
* @bsimethod
+--------------------------------------------------------------------------------------*/
bool _IsValidGeometry(GeometryValidatorPtr &validator) const override
    {
    return true;
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
* @bsimethod
+--------------------------------------------------------------------------------------*/
ICurvePrimitivePtr _CloneBetweenFractions
(
double fractionA,
double fractionB,
bool allowExtrapolation
) const override
    {
    DCatenary3dPlacement placement = m_placement.CloneBetweenFractions (fractionA, fractionB);
    return new CurvePrimitiveCatenaryCurve (placement);
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

/*--------------------------------------------------------------------------------**//**
* @bsimethod
+--------------------------------------------------------------------------------------*/
bool _AddStrokes (bvector <PathLocationDetail> &points, IFacetOptionsCR options,
                double startFraction,
                double endFraction
                ) const override
    {
    bvector<DPoint3d> xyz;
    bvector<double> fractions;      // These are global fractions !!!
    m_placement.Stroke (xyz, fractions,
            startFraction, endFraction,
            options.GetChordTolerance (), options.GetAngleTolerance (), options.GetMaxEdgeLength ()
            );
    double s0 = m_placement.m_distanceLimits.GetStart ();
    double s1 = m_placement.m_distanceLimits.GetEnd ();

    for (size_t i = 0; i < xyz.size (); i++)
        {
        double f = fractions[i];
        double distanceAlong = DoubleOps::Interpolate (s0, f, s1);
        points.push_back (PathLocationDetail
            (CurveLocationDetail (this, f, xyz[i]), 0, distanceAlong));
        }
    return true;
    } 

/*--------------------------------------------------------------------------------**//**
* @bsimethod
+--------------------------------------------------------------------------------------*/
bool _AddStrokes (bvector <DPoint3d> &points, IFacetOptionsCR options,
                bool includeStartPoint,
                double startFraction,
                double endFraction
                ) const override
    {
    bvector<double> fractions;      // These are global fractions !!!
    m_placement.Stroke(points, fractions,
        startFraction, endFraction,
        options.GetChordTolerance(), options.GetAngleTolerance(), options.GetMaxEdgeLength()
        );
    return true;

    }




}; // CurvePrimitiveCatenaryCurve
//END_BENTLEY_GEOMETRY_NAMESPACE
