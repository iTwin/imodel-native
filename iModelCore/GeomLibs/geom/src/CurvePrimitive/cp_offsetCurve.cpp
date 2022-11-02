/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include <bsibasegeomPCH.h>

BEGIN_BENTLEY_GEOMETRY_NAMESPACE

typedef struct TrackingCurveDetail const *TrackingCurveDetailCP;
// X(f) is base curve parameterization.
// Pseudo-offset curve is
//          Y(f) = X(f) + M * X'(f)
// where
//<ul>
//<li>X(f) is the base curve point.
//<li>X'(f) is the base curve derivative
//<li>M is a fixed matrix
//</ul>
// Derivative (non-normalized tangent vector) is
//         Y'(f) = X'(f) + M * X''(f)
// FACT:  If X(f) is parameterized "by distance" and magnitude(X'(f)) is a constant L (total length of X over 0..1 fraction interval),
//        and M = (a/L) * RotMatrix::FromCrossingVector (unitNormal)
//         Y(f) is the true offset of X(f) by distance a
// REMARK: The constant magnitude property is satisfied exactly by:
//            LineSegment
//            CircularArc
//            Any transition spiral
//   and approximately by certain bspline approximations to those
//    Specifically: For reasonable sized N, a cubic with
//             N interior intervals
//             (N+2) poles contructed to interpolate at the Greville ordinates
//          is quite good for spirals, circular arcs.
//          (and it is exact for lines)

struct TrackingCurveDetail
{
ICurvePrimitivePtr m_parentCurve;
RotMatrix m_matrix;

int64_t m_userData;

/*--------------------------------------------------------------------------------**//**
* @bsimethod
+--------------------------------------------------------------------------------------*/
TrackingCurveDetail (
ICurvePrimitiveP parentCurve,
RotMatrixCR matrix,
int64_t userData
) : m_parentCurve(parentCurve),
    m_matrix (matrix),
    m_userData (userData)
    {
    }


/*--------------------------------------------------------------------------------**//**
* @bsimethod
+--------------------------------------------------------------------------------------*/
TrackingCurveDetail() : m_parentCurve(NULL),  m_userData (0)
    {
    m_matrix.Zero ();
    }


RotMatrix GetMatrix() const { return m_matrix;}

ValidatedDPoint3d FractionToPoint (double fraction) const
    {
    DPoint3d point0;
    DVec3d   tangent0;
    if (   m_parentCurve.IsValid ()
        && m_parentCurve->FractionToPoint (fraction, point0, tangent0))
        {
        return ValidatedDPoint3d (point0 + m_matrix * tangent0, true);
        }
    return ValidatedDPoint3d (DPoint3d::From (0,0,0), false);
    }

ValidatedDRay3d FractionToPointAndDerivative (double fraction) const
    {
    DPoint3d point0;
    DVec3d   derivative1, derivative2;
    if (   m_parentCurve.IsValid ()
        && m_parentCurve->FractionToPoint (fraction, point0, derivative1, derivative2))
        {
        return ValidatedDRay3d (
                DRay3d::FromOriginAndVector
                    (
                    point0 + m_matrix * derivative1,
                    derivative1 + m_matrix * derivative2),
                    true);
        }
    return ValidatedDRay3d (DRay3d::FromOriginAndVector (DPoint3d::From (0,0,0), DVec3d::From (1,0,0)), false);
    }

ValidatedDPoint3dDVec3dDVec3d FractionToPointAndDerivatives (double fraction) const
    {
    DPoint3d point0;
    DVec3d   derivative1, derivative2, derivative3;
    if (   m_parentCurve.IsValid ()
        && m_parentCurve->FractionToPoint (fraction, point0, derivative1, derivative2, derivative3))
        {
        return ValidatedDPoint3dDVec3dDVec3d (
                DPoint3dDVec3dDVec3d (
                        point0 + m_matrix * derivative1,
                        derivative1 + m_matrix * derivative2,
                        derivative2 + m_matrix * derivative3
                        ), true);
        }
    return ValidatedDPoint3dDVec3dDVec3d (DPoint3dDVec3dDVec3d::FromXYPlane (), false);
    }
};

/*--------------------------------------------------------------------------------**//**
* @bsistruct
+--------------------------------------------------------------------------------------*/
struct CurvePrimitiveTrackingCurve : public ICurvePrimitive
{
protected:
TrackingCurveDetail m_detail;


MSBsplineCurvePtr m_proxyCurve;

size_t DefaultStrokeCount () const
    {
    auto options = IFacetOptions::CreateForCurves ();
    static double s_chordFraction = 1.0e-3;
    double a;
    m_detail.m_parentCurve->FastLength (a);
    options->SetChordTolerance (s_chordFraction * a);
    return m_detail.m_parentCurve->GetStrokeCount (*options);
    }
void ComputeProxy ()
    {
    size_t numPoles = DefaultStrokeCount ();
    if (numPoles < 2)
        numPoles = 2;
    int order = 4;
    if (m_detail.m_parentCurve->GetCurvePrimitiveType () == ICurvePrimitive::CURVE_PRIMITIVE_TYPE_Line)
        order = 2;
    m_proxyCurve = MSBsplineCurve::CreateFromInterpolationAtGrevilleKnots (*this, numPoles, order, true, 100);
    }



/*--------------------------------------------------------------------------------**//**
* @bsimethod
+--------------------------------------------------------------------------------------*/
explicit CurvePrimitiveTrackingCurve(ICurvePrimitiveP parent, RotMatrixCR matrix, int64_t m_userData)
    :   m_detail(parent, matrix, m_userData)
    {
    ComputeProxy ();
    }


/*--------------------------------------------------------------------------------**//**
* @bsimethod
+--------------------------------------------------------------------------------------*/
bool _TryGetTrackingCurveData (ICurvePrimitivePtr &parent, RotMatrixR matrix, int64_t &tag) const override
    {
    matrix = m_detail.m_matrix;
    parent    = m_detail.m_parentCurve;
    tag = m_detail.m_userData;
    return true;
    }



/*--------------------------------------------------------------------------------**//**
* @bsimethod
+--------------------------------------------------------------------------------------*/
void _Process(ICurvePrimitiveProcessor &processor, DSegment1dCP interval) const override 
    {
#ifdef abc
    if (m_detail.m_parentCurve.IsValid ())
        processor._ProcessTrackingCurve (*m_parentCurve, m_detail.m_matrix, interval);
#endif
    }


/*--------------------------------------------------------------------------------**//**
* @bsimethod
+--------------------------------------------------------------------------------------*/
void ReplaceCurve(ICurvePrimitiveP parent)  {m_detail.m_parentCurve = ICurvePrimitivePtr (parent);}


/*--------------------------------------------------------------------------------**//**
* @bsimethod
+--------------------------------------------------------------------------------------*/
ICurvePrimitivePtr _Clone() const override
    {
    return new CurvePrimitiveTrackingCurve (m_detail.m_parentCurve->Clone ().get (), m_detail.m_matrix, m_detail.m_userData);
    }


/*--------------------------------------------------------------------------------**//**
An interval of the Tracking is a pseudoOffset of the parent curve interval !!!
* @bsimethod
+--------------------------------------------------------------------------------------*/
ICurvePrimitivePtr _CloneBetweenFractions
(
double fractionA,
double fractionB,
bool allowExtrapolation
) const override
    {
    return new CurvePrimitiveTrackingCurve (m_detail.m_parentCurve->CloneBetweenFractions (fractionA, fractionB, allowExtrapolation).get (),
                    m_detail.m_matrix,
                    m_detail.m_userData
                    );
    }

~CurvePrimitiveTrackingCurve () {m_detail.m_parentCurve = NULL;}


/*--------------------------------------------------------------------------------**//**
* @bsimethod
+--------------------------------------------------------------------------------------*/
CurvePrimitiveType _GetCurvePrimitiveType() const override {return CURVE_PRIMITIVE_TYPE_TrackingCurve;}


/*--------------------------------------------------------------------------------**//**
* @bsimethod
+--------------------------------------------------------------------------------------*/
bool _IsPeriodicFractionSpace(double &period) const override
    {
    return !m_detail.m_parentCurve.IsValid () ? false : m_detail.m_parentCurve->IsPeriodicFractionSpace (period);
    }


/*--------------------------------------------------------------------------------**//**
* @bsimethod
+--------------------------------------------------------------------------------------*/
bool _FractionToPoint(double fraction, DPoint3dR point) const override
    {
    auto value = m_detail.FractionToPoint (fraction);
    point = value.Value ();
    return value.IsValid (); 
    }


/*--------------------------------------------------------------------------------**//**
* @bsimethod
+--------------------------------------------------------------------------------------*/
bool _FractionToPoint(double fraction, DPoint3dR point, DVec3dR tangent) const override
    {
    auto ray = m_detail.FractionToPointAndDerivative (fraction);
    point = ray.Value ().origin;
    tangent = ray.Value ().direction;
    return ray.IsValid ();
    }
    

/*--------------------------------------------------------------------------------**//**
* @bsimethod
+--------------------------------------------------------------------------------------*/
bool _FractionToPoint(double fraction, DPoint3dR point, DVec3dR tangent, DVec3dR derivative2) const override
    {
    auto basis = m_detail.FractionToPointAndDerivatives (fraction);
    point = basis.Value().origin;
    tangent = basis.Value().vectorU;
    derivative2 = basis.Value ().vectorV;
    return basis.IsValid ();
    }


/*--------------------------------------------------------------------------------**//**
* @bsimethod
+--------------------------------------------------------------------------------------*/
bool _FractionToPoint(double fraction, DPoint3dR point, DVec3dR tangent, DVec3dR derivative2, DVec3dR derivative3) const override
    {
    derivative3.Zero ();    // This is not true.
    return _FractionToPoint (fraction, point, tangent, derivative2);
    }

struct ArcLengthIntegrand : BSIVectorIntegrand
    {
    ValidatedRotMatrix m_matrix;
    CurvePrimitiveTrackingCurve const &m_curve;
    ArcLengthIntegrand (CurvePrimitiveTrackingCurve const &curve, RotMatrixCP matrix)
        :
        m_curve(curve)
        {
        m_matrix = nullptr != matrix
                 ? ValidatedRotMatrix (*matrix, true)
                 : ValidatedRotMatrix (RotMatrix::FromIdentity (), false);
        }

    void EvaluateVectorIntegrand (double t, double *pF) override 
        {
        DRay3d point = m_curve.m_detail.FractionToPointAndDerivative (t);
        if (m_matrix.IsValid ())
            {
            DVec3d tangent = m_matrix.Value () * point.direction;
            pF[0] = tangent.Magnitude ();
            }
        else
            {
            pF[0] = point.direction.Magnitude ();
            }
        }
    int  GetVectorIntegrandCount () override {return 1;}
    };

bool IntegrateLength (double &length, double f0, double f1, RotMatrixCP matrix) const
    {
    size_t numStroke = m_proxyCurve.IsValid () ? m_proxyCurve->GetNumKnots () : DefaultStrokeCount ();
    BSIQuadraturePoints quadrature;
    quadrature.InitGauss (7);
    ArcLengthIntegrand integrand(*this, matrix);
    double sums[10];        // really only use 1.
    double df = 1.0 / numStroke;
    sums[0] = 0.0;
    for (size_t i = 0; i < numStroke; i++)
        {
        double f0a = i * df;
        double f1a = (i+1) * df;
        if (!DoubleOps::AlmostEqual (f0a, f1a))
            quadrature.AccumulateWeightedSums (integrand, f0a, f1a, sums, 1);
        }
    length = sums[0];
    return true;
    }
/*--------------------------------------------------------------------------------**//**
* @bsimethod
+--------------------------------------------------------------------------------------*/
bool _Length(double &length) const override
        {
        return IntegrateLength (length, 0.0, 1.0, nullptr);
        }

/*--------------------------------------------------------------------------------**//**
* @bsimethod
+--------------------------------------------------------------------------------------*/
bool _Length(RotMatrixCP worldToLocal, double &length) const override
        {
        return IntegrateLength (length, 0.0, 1.0, worldToLocal);
        }

/*--------------------------------------------------------------------------------**//**
* @bsimethod
+--------------------------------------------------------------------------------------*/
bool _GetRange(DRange3dR range) const override
        {
        range = m_proxyCurve->GetRange ();
        return true;
        }


/*--------------------------------------------------------------------------------**//**
* @bsimethod
+--------------------------------------------------------------------------------------*/
bool _GetRange(DRange3dR range, TransformCR transform) const override
        {
        auto curve = m_proxyCurve->CreateCopyTransformed (transform);
        range = curve->GetRange ();
        return true;
        }


/*--------------------------------------------------------------------------------**//**
* @bsimethod
+--------------------------------------------------------------------------------------*/
double _FastMaxAbs() const override
        {
        DRange3d poleRange;
        m_proxyCurve->GetPoleRange (poleRange);
        return poleRange.LargestCoordinate ();
        }

/*--------------------------------------------------------------------------------**//**
* @bsimethod
+--------------------------------------------------------------------------------------*/
bool _IsSameStructureAndGeometry (ICurvePrimitiveCR other, double tolerance) const override
    {
    RotMatrix otherMatrix;
    ICurvePrimitivePtr otherParent;
    int64_t otherTag;
    if (!other.TryGetTrackingCurveData (otherParent, otherMatrix, otherTag))
        return false;
    return m_detail.m_matrix.IsEqual (otherMatrix, DoubleOps::SmallCoordinateRelTol () * m_detail.m_matrix.MaxAbs ())
        && m_detail.m_parentCurve.IsValid ()
        && otherParent.IsValid ()
        && m_detail.m_parentCurve->IsSameStructureAndGeometry (*otherParent)
        ;
    }
/*--------------------------------------------------------------------------------**//**
* @bsimethod
+--------------------------------------------------------------------------------------*/
bool _IsValidGeometry(GeometryValidatorPtr &validator) const override
    {
    if (!validator.IsValid ())
        return true;
    return m_detail.m_parentCurve.IsValid()
        && m_detail.m_parentCurve->IsValidGeometry (validator)
        && validator->IsValidGeometry (m_detail.m_matrix);
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod
+--------------------------------------------------------------------------------------*/
size_t _NumComponent () const override {return 1;}


#ifdef abc

/*--------------------------------------------------------------------------------**//**
* @bsimethod
+--------------------------------------------------------------------------------------*/
bool _GetBreakFraction(size_t breakFractionIndex, double &fraction) const override
        {
        size_t numPoles = m_curve.NumberAllocatedPoles (), order = m_curve.GetOrder (), maxIndex;
        maxIndex = numPoles - order + 1;

        if (breakFractionIndex >= 0 && breakFractionIndex <= maxIndex)
            {
            fraction = m_curve.GetKnot (int (breakFractionIndex + order - 1));
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
        size_t numPoles = m_curve.NumberAllocatedPoles (), order = m_curve.GetOrder (), maxIndex;
        maxIndex = numPoles - order + 1;

        if (DoubleOps::BoundingValues (&m_curve.knots[order-1], maxIndex + 1, fraction, breakIndex, lowerBound, upperBound))
            {
            adjustedFraction = Rounding::Round (fraction, mode, lowerBound, upperBound);
            if (adjustedFraction > lowerBound)
                breakIndex ++;
            return true;
            }
        else
            return false;
        }
#endif    

/*--------------------------------------------------------------------------------**//**
* @bsimethod
+--------------------------------------------------------------------------------------*/
bool _GetMSBsplineCurve(MSBsplineCurveR curve, double fractionA, double fractionB) const override
        {
        return SUCCESS == curve.CopyFrom (*m_proxyCurve);
        }
MSBsplineCurveCP _GetProxyBsplineCurveCP () const override {return m_proxyCurve.get ();}
MSBsplineCurvePtr _GetProxyBsplineCurvePtr() const override { return m_proxyCurve; }


/*--------------------------------------------------------------------------------**//**
* @bsimethod
+--------------------------------------------------------------------------------------*/
bool _AddStrokes(bvector <DPoint3d> &points, IFacetOptionsCR options,
                bool includeStartPoint,
                double startFraction,
                double endFraction
                ) const override
        {
        size_t numPoint = GetStrokeCount (options, startFraction, endFraction);
        if (numPoint < 2)
            numPoint = 2;
        double df = 1.0 / (numPoint - 1);
        for (
            size_t i = includeStartPoint || points.size () == 0 ? 0 : 1;
            i < numPoint;
            i++
            )
            {
            points.push_back (m_detail.FractionToPoint (i * df));
            }
        return false;
        }

/*--------------------------------------------------------------------------------**//**
* @bsimethod
+--------------------------------------------------------------------------------------*/
size_t _GetStrokeCount(IFacetOptionsCR options, double startFraction, double endFraction) const override
        {
        if (m_detail.m_parentCurve.IsValid ())
            return m_detail.m_parentCurve->GetStrokeCount (options, startFraction, endFraction);
        else
            return 0;
        }


/*--------------------------------------------------------------------------------**//**
* @bsimethod
+--------------------------------------------------------------------------------------*/
bool _SignedDistanceBetweenFractions(double fractionA, double fractionB, double &signedDistance) const override
        {
        return false;
        }

/*--------------------------------------------------------------------------------**//**
* @bsimethod
+--------------------------------------------------------------------------------------*/
bool _SignedDistanceBetweenFractions(RotMatrixCP worldToLocal, double fractionA, double fractionB, double &signedDistance) const override
        {
        return false;
        }

// (internal implementation for overloads)
bool PointAtSignedDistanceFromFraction_go (RotMatrixCP worldToLocal, double startFraction, double signedDistance, bool allowExtension, CurveLocationDetailR location) const
        {
#ifdef abc
        if (m_detail.m_parentCurve.IsValid () && !m_detail.IsSingleFraction ())
            {
            double parentStartFraction = m_detail.ChildFractionToParentFraction (startFraction);
            double endFraction;
            CurveLocationDetail parentLocation;
            if (nullptr == worldToLocal)
                m_detail.m_parentCurve->PointAtSignedDistanceFromFraction (parentStartFraction, signedDistance, allowExtension, parentLocation);
            else
                m_detail.m_parentCurve->PointAtSignedDistanceFromFraction (worldToLocal, parentStartFraction, signedDistance, allowExtension, parentLocation);
            if (m_detail.ParentFractionToChildFraction (parentLocation.fraction, endFraction))
                {
                location = CurveLocationDetail (this, 1);
                location.SetSingleComponentFractionAndA (endFraction, parentLocation.a);
                }
            return true;
            }
#endif
        return false;
        }
    
/*--------------------------------------------------------------------------------**//**
* @bsimethod
+--------------------------------------------------------------------------------------*/
bool _PointAtSignedDistanceFromFraction(double startFraction, double signedDistance, bool allowExtension, CurveLocationDetailR location) const override
    {
    return PointAtSignedDistanceFromFraction_go (nullptr, startFraction, signedDistance, allowExtension, location);
    }

bool _PointAtSignedDistanceFromFraction(RotMatrixCP worldToLocal, double startFraction, double signedDistance, bool allowExtension, CurveLocationDetailR location) const override
    {
    return PointAtSignedDistanceFromFraction_go (worldToLocal, startFraction, signedDistance, allowExtension, location);
    }

using ICurvePrimitive::_ClosestPointBounded;    // suppresses C4266
bool _ClosestPointBounded (DPoint3dCR spacePoint, double &fraction, DPoint3dR curvePoint, bool extend0, bool extend1) const override
    {
    // Ugh.. convert to curve is expensive.  ALSO JUST PLAIN WRONG -- PARAMETERIZATION MIGHT NOT MATCH
    MSBsplineCurve curve;
    if (GetMSBsplineCurve (curve, 0.0, 1.0))
        {
        curve.ClosestPoint (curvePoint, fraction, spacePoint);
        curve.ReleaseMem ();
        return true;
        }
    return false;
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
        // Ugh.. convert to curve is expensive.  ALSO JUST PLAIN WRONG -- PARAMETERIZATION MIGHT NOT MATCH
        MSBsplineCurve curve;
        if (GetMSBsplineCurve (curve, 0.0, 1.0))
            {
            location = CurveLocationDetail (this, 1);
            curve.ClosestPointXY (location.point, location.fraction, location.a, spacePoint, worldToLocal);
            location.SetSingleComponentData ();
            curve.ReleaseMem ();
            return true;
            }
        return false;
        }


/*--------------------------------------------------------------------------------**//**
* @bsimethod
+--------------------------------------------------------------------------------------*/
bool _TransformInPlace (TransformCR transform) override
        {
        return false;
        }

/*--------------------------------------------------------------------------------**//**
* @bsimethod
+--------------------------------------------------------------------------------------*/
bool _ReverseCurvesInPlace () override
    {
    return false;
    }


public: 

static ICurvePrimitive* Create (ICurvePrimitiveP m_parentCurve, RotMatrixCR matrix, int64_t m_userData)
    {
    return new CurvePrimitiveTrackingCurve (m_parentCurve, matrix, m_userData);
    }


/*--------------------------------------------------------------------------------**//**
* @bsimethod
+--------------------------------------------------------------------------------------*/
void _AppendCurvePlaneIntersections(DPlane3dCR plane, bvector<CurveLocationDetailPair> &intersections, double tol) const override 
    {
    //AppendTolerancedPlaneIntersections (plane, this, m_curve, intersections, tol);
    }

}; // CreateTrackingCurve


/*--------------------------------------------------------------------------------**//**
* @bsimethod
+--------------------------------------------------------------------------------------*/
ICurvePrimitivePtr ICurvePrimitive::CreateTrackingCurve (ICurvePrimitiveP parentCurve, RotMatrixCR matrix, int64_t m_userData)
    {
    return CurvePrimitiveTrackingCurve::Create (parentCurve, matrix, m_userData);
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod
+--------------------------------------------------------------------------------------*/
ICurvePrimitivePtr ICurvePrimitive::CreateTrackingCurveXY (ICurvePrimitiveP parentCurve, double signedDistance)
    {
    DPoint3d point;
    DVec3d tangent;
    parentCurve->FractionToPoint (0.0, point, tangent);
    auto a = DoubleOps::ValidatedDivide (signedDistance, tangent.Magnitude ());
    if (a.IsValid ())
        {
        double b = a.Value ();
        auto matrix = RotMatrix::FromRowValues
            (
            0, b, 0,
            -b,0, 0,
            0,0, 0
            );
        return CurvePrimitiveTrackingCurve::Create (parentCurve, matrix, 0);
        }
    return nullptr;
    }

bool ICurvePrimitive::_TryGetTrackingCurveData (ICurvePrimitivePtr &parent, RotMatrixR matrix, int64_t &tag) const
    {
    matrix.Zero ();
    parent = nullptr;
    tag = 0;
    return false;
    }

bool ICurvePrimitive::TryGetTrackingCurveData (ICurvePrimitivePtr &parent, RotMatrixR matrix, int64_t &tag) const
    {
    return _TryGetTrackingCurveData (parent, matrix, tag);
    }
END_BENTLEY_GEOMETRY_NAMESPACE
