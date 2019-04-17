/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#include <bsibasegeomPCH.h>

BEGIN_BENTLEY_GEOMETRY_NAMESPACE


/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
PartialCurveDetail::PartialCurveDetail(
ICurvePrimitiveP _parentCurve,
double _fraction0,
double _fraction1,
int64_t _userData
) : parentCurve(_parentCurve),
    fraction0 (_fraction0),
    fraction1 (_fraction1),
    userData (_userData)
    {
    }


/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
PartialCurveDetail::PartialCurveDetail(
ICurvePrimitiveP _parentCurve,
DSegment1dCR interval,
int64_t _userData
) : parentCurve(_parentCurve),
    fraction0 (interval.GetStart ()),
    fraction1 (interval.GetEnd ()),
    userData (_userData)
    {
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
PartialCurveDetail::PartialCurveDetail(
PartialCurveDetailCR parent,
double f0,
double f1
) : parentCurve(parent.parentCurve),
    fraction0(DoubleOps::Interpolate (parent.fraction0, f0, parent.fraction1)),
    fraction1(DoubleOps::Interpolate (parent.fraction0, f1, parent.fraction1)),
    userData (parent.userData)
    {
    }


/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
PartialCurveDetail::PartialCurveDetail(
) : parentCurve(NULL),
    fraction0 (0.0),
    fraction1 (1.0),
    userData (0)
    {
    }

//! map a local fraction into the parent fraction.  (If the interval is a single fraction, all child fractions map to that same fraction.)
double PartialCurveDetail::ChildFractionToParentFraction (double f) const
    {
    return fraction0 + f * (fraction1 - fraction0);
    }

//! map a local fraction into the parent fraction.  (If the interval is a single fraction, all child fractions map to that same fraction.)
DSegment1d PartialCurveDetail::GetInterval () const
    {
    return DSegment1d (fraction0, fraction1);
    }


//! (attempt to) map a parent fraction back to the child interval.  This fails if the child is a single point.
bool PartialCurveDetail::ParentFractionToChildFraction (double parentFraction, double &childFraction) const
        {
        return DoubleOps::SafeDivideParameter (childFraction,
                    parentFraction - fraction0, fraction1 - fraction0, 0.0);
        }

//! Test if the partial curve fraction range is a single fraction.
bool PartialCurveDetail::IsSingleFraction () const {return fraction0 == fraction1;}


/*--------------------------------------------------------------------------------**//**
* @bsistruct                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
struct CurvePrimitivePartialCurve : public ICurvePrimitive
{
protected:
PartialCurveDetail m_detail;


/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
explicit CurvePrimitivePartialCurve(ICurvePrimitiveP parent, double fraction0, double fraction1, int64_t userData)
    :   m_detail(parent, fraction0, fraction1, userData)
    {
    }


/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
PartialCurveDetailCP _GetPartialCurveDetailCP() const override {return &m_detail;}
bool _TryGetPartialCurveData (double &fractionA, double &fractionB, ICurvePrimitivePtr &parent, int64_t &tag) const override
    {
    fractionA = m_detail.fraction0;
    fractionB = m_detail.fraction1;
    parent    = m_detail.parentCurve;
    tag = m_detail.userData;
    return true;
    }


/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
void _Process(ICurvePrimitiveProcessor &processor, DSegment1dCP interval) const override 
    {
    DSegment1d myInterval (m_detail.fraction0, m_detail.fraction1);
    if (NULL == interval)
        m_detail.parentCurve->_Process (processor, &myInterval);
    else
        {
        DSegment1d newInterval = myInterval.BetweenFractions (interval->GetStart (), interval->GetEnd ());
        m_detail.parentCurve->_Process (processor, &newInterval);
        }        
    }


/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
void ReplaceCurve(ICurvePrimitiveP parent)  {m_detail.parentCurve = ICurvePrimitivePtr (parent);}


/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
ICurvePrimitivePtr _Clone() const override
    {
    return Create (m_detail.parentCurve.get (), m_detail.fraction0, m_detail.fraction1, m_detail.userData);
    }


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
    return new CurvePrimitivePartialCurve (m_detail.parentCurve.get (),
                    m_detail.ChildFractionToParentFraction (fractionA),
                    m_detail.ChildFractionToParentFraction (fractionB),
                    m_detail.userData
                    );
    }

~CurvePrimitivePartialCurve () {m_detail.parentCurve = NULL;}


/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
bool IsReversedCurve() const {return m_detail.fraction0 > m_detail.fraction1;}


/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
CurvePrimitiveType _GetCurvePrimitiveType() const override {return CURVE_PRIMITIVE_TYPE_PartialCurve;}


/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
bool _IsPeriodicFractionSpace(double &period) const override
    {
    return !m_detail.parentCurve.IsValid () ? false : m_detail.parentCurve->IsPeriodicFractionSpace (period);
    }


/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
bool _FractionToPoint(double fraction, DPoint3dR point) const override
        {
        if (!m_detail.parentCurve.IsValid ())
            return false;
        return m_detail.parentCurve->FractionToPoint (m_detail.ChildFractionToParentFraction (fraction), point);
        }


/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
bool _FractionToPoint(double fraction, DPoint3dR point, DVec3dR tangent) const override
        {
        if (!m_detail.parentCurve.IsValid () || !m_detail.parentCurve->FractionToPoint (m_detail.ChildFractionToParentFraction (fraction), point, tangent))
            return false;
        tangent.Scale (m_detail.fraction1 - m_detail.fraction0);
        return true;
        }
    

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
bool _FractionToPoint(double fraction, DPoint3dR point, DVec3dR tangent, DVec3dR derivative2) const override
        {
        if (!m_detail.parentCurve.IsValid () || !m_detail.parentCurve->FractionToPoint (m_detail.ChildFractionToParentFraction (fraction), point, tangent, derivative2))
            return false;
        double a = m_detail.fraction1 - m_detail.fraction0;
        tangent.Scale (a);
        derivative2.Scale (a * a);
        return true;
        }


/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
bool _FractionToPoint(double fraction, DPoint3dR point, DVec3dR tangent, DVec3dR derivative2, DVec3dR derivative3) const override
        {
        if (!m_detail.parentCurve.IsValid () || !m_detail.parentCurve->FractionToPoint (m_detail.ChildFractionToParentFraction (fraction), point, tangent, derivative2, derivative3))
            return false;
        double a = m_detail.fraction1 - m_detail.fraction0;
        tangent.Scale (a);
        derivative2.Scale (a * a);
        derivative3.Scale (a * a * a);
        return true;
        }


/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
bool _FractionToFrenetFrame(double f, TransformR frame) const override
        {
        if (!m_detail.parentCurve.IsValid () || !m_detail.parentCurve->FractionToFrenetFrame (m_detail.ChildFractionToParentFraction (f), frame))
            return false;
        if (IsReversedCurve ())
            {
            // This should probably look at 2nd derivative rather than line.
            if (NULL != m_detail.parentCurve->GetLineCP ())
                frame.ScaleMatrixColumns (-1.0,-1.0,  1.0);
            else
                frame.ScaleMatrixColumns (-1.0, 1.0, -1.0);
            }
        return true;
        }


/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
bool _Length(double &length) const override
        {
        length = 0.0;
        if (m_detail.parentCurve.IsValid ())
            {
            bool stat = m_detail.parentCurve->SignedDistanceBetweenFractions (m_detail.fraction0, m_detail.fraction1, length);
            length = fabs (length);
            return stat;
            }
        return false;
        }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
bool _Length(RotMatrixCP worldToLocal, double &length) const override
        {
        length = 0.0;
        if (m_detail.parentCurve.IsValid ())
            {
            bool stat = m_detail.parentCurve->SignedDistanceBetweenFractions (worldToLocal, m_detail.fraction0, m_detail.fraction1, length);
            length = fabs (length);
            return stat;
            }
        return false;
        }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
virtual bool _GetRange(DRange3dR range) const override
        {
        MSBsplineCurve curve;
        if (GetMSBsplineCurve(curve, 0.0, 1.0))
            {
            range = curve.GetRange();
            curve.ReleaseMem();
            return true;
            }

        range.Init();
        return false;
        }


/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
virtual bool _GetRange(DRange3dR range, TransformCR transform) const override
        {
        MSBsplineCurve curve;
        if (GetMSBsplineCurve(curve, 0.0, 1.0))
            {
            curve.TransformCurve(transform);
            range = curve.GetRange();
            curve.ReleaseMem();
            return true;
            }

        range.Init();
        return false;
        }




/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
double _FastMaxAbs() const override
        {
        DRange3d poleRange;
        MSBsplineCurve curve;
        GetMSBsplineCurve (curve, 0.0, 1.0);
        curve.GetPoleRange (poleRange);
        curve.ReleaseMem ();
        return poleRange.LargestCoordinate ();
        }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      06/2014
+--------------------------------------------------------------------------------------*/
bool _IsSameStructureAndGeometry (ICurvePrimitiveCR other, double tolerance) const override
    {
    PartialCurveDetailCP otherDetail = GetPartialCurveDetailCP ();
    
    if (NULL == otherDetail)
        return false;
    return DoubleOps::AlmostEqual (m_detail.fraction0, otherDetail->fraction0)
        && DoubleOps::AlmostEqual (m_detail.fraction1, otherDetail->fraction1)
        && m_detail.parentCurve.IsValid ()
        && otherDetail->parentCurve.IsValid ()
        && m_detail.parentCurve->IsSameStructureAndGeometry (*otherDetail->parentCurve)
        ;
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      11/2013
+--------------------------------------------------------------------------------------*/
size_t _NumComponent () const override {return 1;}


#ifdef abc

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
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
* @bsimethod                                                    EarlinLutz      04/2012
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
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
bool _GetMSBsplineCurve(MSBsplineCurveR curve, double fractionA, double fractionB) const override
        {
        double fractionC = m_detail.ChildFractionToParentFraction (fractionA);
        double fractionD = m_detail.ChildFractionToParentFraction (fractionB);
        return m_detail.parentCurve.IsValid () && m_detail.parentCurve->GetMSBsplineCurve (curve, fractionC, fractionD);
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
        double fraction0 = m_detail.ChildFractionToParentFraction (startFraction);
        double fraction1 = m_detail.ChildFractionToParentFraction (endFraction);
        return m_detail.parentCurve.IsValid () && m_detail.parentCurve->AddStrokes (points, options, includeStartPoint, fraction0, fraction1);
        }


/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
size_t _GetStrokeCount(IFacetOptionsCR options, double startFraction, double endFraction) const override
        {
        double fraction0 = m_detail.ChildFractionToParentFraction (startFraction);
        double fraction1 = m_detail.ChildFractionToParentFraction (endFraction);
        if (m_detail.parentCurve.IsValid ())
            return m_detail.parentCurve->GetStrokeCount (options, fraction0, fraction1);
        else
            return 0;
        }


/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
bool _SignedDistanceBetweenFractions(double fractionA, double fractionB, double &signedDistance) const override
        {
        if (m_detail.parentCurve.IsValid ())
            {
            double fractionC = m_detail.ChildFractionToParentFraction (fractionA);
            double fractionD = m_detail.ChildFractionToParentFraction (fractionB);
            if (   m_detail.parentCurve.IsValid ()
                && m_detail.parentCurve->SignedDistanceBetweenFractions (fractionC, fractionD, signedDistance))
                {
                if ((fractionB - fractionA) * (fractionD - fractionC) < 0.0)
                    signedDistance = - signedDistance;
                return true;
                }
            }
        return false;
        }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
bool _SignedDistanceBetweenFractions(RotMatrixCP worldToLocal, double fractionA, double fractionB, double &signedDistance) const override
        {
        if (m_detail.parentCurve.IsValid ())
            {
            double fractionC = m_detail.ChildFractionToParentFraction (fractionA);
            double fractionD = m_detail.ChildFractionToParentFraction (fractionB);
            if (   m_detail.parentCurve.IsValid ()
                && m_detail.parentCurve->SignedDistanceBetweenFractions (worldToLocal, fractionC, fractionD, signedDistance))
                {
                if ((fractionB - fractionA) * (fractionD - fractionC) < 0.0)
                    signedDistance = - signedDistance;
                return true;
                }
            }
        return false;
        }

// (internal implementation for overloads)
bool PointAtSignedDistanceFromFraction_go (RotMatrixCP worldToLocal, double startFraction, double signedDistance, bool allowExtension, CurveLocationDetailR location) const
        {
        if (m_detail.parentCurve.IsValid () && !m_detail.IsSingleFraction ())
            {
            double parentStartFraction = m_detail.ChildFractionToParentFraction (startFraction);
            double endFraction;
            CurveLocationDetail parentLocation;
            if (nullptr == worldToLocal)
                m_detail.parentCurve->PointAtSignedDistanceFromFraction (parentStartFraction, signedDistance, allowExtension, parentLocation);
            else
                m_detail.parentCurve->PointAtSignedDistanceFromFraction (worldToLocal, parentStartFraction, signedDistance, allowExtension, parentLocation);
            if (m_detail.ParentFractionToChildFraction (parentLocation.fraction, endFraction))
                {
                location = CurveLocationDetail (this, 1);
                location.SetSingleComponentFractionAndA (endFraction, parentLocation.a);
                }
            return true;
            }
        return false;
        }
    
/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
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
    MSBsplineCurve curve;
    if (m_detail.IsSingleFraction ())
        {
        // fractional output is in the partial curve space, i.e. 0.0
        this->FractionToPoint (m_detail.fraction0, curvePoint);
        fraction = 0.0;
        return true;
        }
    if (GetMSBsplineCurve (curve, 0.0, 1.0))
        {
        curve.ClosestPoint (curvePoint, fraction, spacePoint);
        curve.ReleaseMem ();
        return true;
        }
    return false;
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
        if (m_detail.IsSingleFraction ())
            {
        // fractional output is in the partial curve space, i.e. 0.0
            DPoint3d xyzA = spacePoint;
            DPoint3d xyzB;
            double fractionOnPartialCurve = 0.0;
            this->FractionToPoint (fractionOnPartialCurve, xyzB);
            location = CurveLocationDetail (this, 0.0, xyzB);
            if (worldToLocal)
                {
                worldToLocal->MultiplyAndRenormalize (xyzA, xyzA);
                worldToLocal->MultiplyAndRenormalize (xyzB, xyzB);
                }
            location.a = xyzA.DistanceXY (xyzB);
            location.SetSingleComponentData ();
            return true;
            }
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
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
bool _TransformInPlace (TransformCR transform) override
        {
        return false;
        }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
bool _ReverseCurvesInPlace () override
    {
    std::swap (m_detail.fraction0, m_detail.fraction1);

    return false;
    }


public: 

static ICurvePrimitive* Create (ICurvePrimitiveP parentCurve, double fraction0, double fraction1, int64_t userData)
    {
    return new CurvePrimitivePartialCurve (parentCurve, fraction0, fraction1, userData);
    }


/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
void _AppendCurvePlaneIntersections(DPlane3dCR plane, bvector<CurveLocationDetailPair> &intersections, double tol) const override 
    {
    size_t numIntersectionsIn = intersections.size ();
    m_detail.parentCurve->AppendCurvePlaneIntersections (plane, intersections, tol);
    for (size_t i = numIntersectionsIn; i < intersections.size (); i++)
        {
        double childFraction;
        DPoint3d xyz = intersections[i].detailA.point;
        if (m_detail.ParentFractionToChildFraction (intersections[i].detailA.fraction, childFraction))
            {
            // replace the detail "pair" with reference to this ...
            intersections[i] = CurveLocationDetailPair (this, childFraction, xyz);
            }
        }
    }

}; // CreatePartialCurve


/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
ICurvePrimitivePtr ICurvePrimitive::CreatePartialCurve (ICurvePrimitiveP parentCurve, double fraction0, double fraction1, int64_t userData)
    {
    return CurvePrimitivePartialCurve::Create (parentCurve, fraction0, fraction1, userData);
    }



/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
ICurvePrimitivePtr ICurvePrimitive::CloneDereferenced (bool allowExtrapolation, bool maximumDeref) const
    {
    PartialCurveDetailCP detail = NULL, nextDetail = NULL;
    CurveVectorCP childVector = NULL;
    if (NULL != (detail = GetPartialCurveDetailCP ()))
        {
        DSegment1d interval (detail->fraction0, detail->fraction1);
        if (maximumDeref)
            {
            while (NULL != (nextDetail = detail->parentCurve->GetPartialCurveDetailCP ()))
                {
                DSegment1d nextInterval (nextDetail->fraction0, nextDetail->fraction1);
                interval = nextInterval.BetweenFractions (interval.GetStart (), interval.GetEnd ());
                detail = nextDetail;
                }
            }
        return detail->parentCurve->CloneBetweenFractions (interval.GetStart (), interval.GetEnd (), allowExtrapolation);
        }
    else if (NULL != (childVector = GetChildCurveVectorCP ()))
        {
        return CreateChildCurveVector_SwapFromSource (*childVector->CloneDereferenced (allowExtrapolation, maximumDeref));
        }
    else    // anything else is simple case ...
        return Clone ();
    }


/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
CurveVectorPtr CurveVector::CloneDereferenced (bool allowExtrapolation, bool maximumDeref) const
    {
    // child vector may contain more partial curves ...
    CurveVectorPtr newChildVector = CurveVector::Create (GetBoundaryType ());
    for (size_t i = 0, n = size (); i < n; i++)
        {
        newChildVector->push_back (at(i)->CloneDereferenced (allowExtrapolation, maximumDeref));
        }
    return newChildVector;
    }
END_BENTLEY_GEOMETRY_NAMESPACE
