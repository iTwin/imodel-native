/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include <bsibasegeomPCH.h>

BEGIN_BENTLEY_GEOMETRY_NAMESPACE
/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
bool PushIfNonNull(CurveVectorR dest, ICurvePrimitivePtr const &curve)
    {
    if (curve.IsNull())
        return false;

    dest.push_back (curve);
    return true;
    }

bool CloneAndPush (CurveVectorR dest, ICurvePrimitivePtr const &parent, double fraction0, double fraction1, bool allowExtrapolation, bool usePartialCurves)
    {
    if (DoubleOps::AlmostEqualFraction (fraction0, fraction1))
        return false;
    ICurvePrimitivePtr curve;
    if (usePartialCurves &&
        !(   DoubleOps::IsExact01 (fraction0, fraction1)
          || DoubleOps::IsExact01 (fraction1, fraction0)
         )
      )
        {
        auto cloneOfParent = parent->Clone ();
        curve = ICurvePrimitive::CreatePartialCurve (cloneOfParent.get (), fraction0, fraction1);
        }
    else 
        curve = parent->CloneBetweenFractions (fraction0, fraction1, false);
    double a;
    if (curve.IsNull () || !curve->Length (a) || a <= DoubleOps::SmallMetricDistance ())
        return false;

    dest.push_back (curve);
    return true;
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
CurveVectorPtr CurveVector::Clone () const
    {
    CurveVectorPtr clone = Create (m_boundaryType);
    for (size_t i = 0, n = size (); i < n; i++)
        clone->push_back (at(i)->Clone ());
    return clone;
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
CurveVectorPtr CurveVector::Clone (TransformCR transform) const
    {
    CurveVectorPtr clone = Create (m_boundaryType);
    for (size_t i = 0, n = size (); i < n; i++)
        clone->push_back (at(i)->Clone (transform));
    return clone;
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
CurveVectorPtr CurveVector::Clone (DMatrix4dCR transform) const
    {
    CurveVectorPtr clone = Create (m_boundaryType);
    for (size_t i = 0, n = size (); i < n; i++)
        {
        ICurvePrimitivePtr cpClone;

        if (at(i).IsValid() &&
            (cpClone = at(i)->Clone(transform)).IsValid())
            clone->push_back (cpClone);
        }
    return clone->empty() ? nullptr : clone;
    }



/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      09/2012
+--------------------------------------------------------------------------------------*/
CurveVectorPtr CurveVector::CloneWithExplodedLinestrings () const
    {
    CurveVectorPtr dest = CurveVector::Create (GetBoundaryType ());
    for (size_t i = 0; i < size (); i++)
        {
        ICurvePrimitivePtr child = at(i);
        ICurvePrimitive::CurvePrimitiveType childType = child->GetCurvePrimitiveType ();
        if (childType == ICurvePrimitive::CURVE_PRIMITIVE_TYPE_LineString)
            {
            DSegment3d segment;
            for (size_t j = 0; child->TryGetSegmentInLineString (segment, j); j++)
                {
                dest->push_back (ICurvePrimitive::CreateLine (segment));
                }
            }
        else if (childType == ICurvePrimitive::CURVE_PRIMITIVE_TYPE_CurveVector)
            {
            CurveVectorPtr childClone =
                child->_GetChildCurveVectorCP ()->CloneWithExplodedLinestrings ();
            dest->push_back (
                ICurvePrimitive::CreateChildCurveVector_SwapFromSource (*childClone));
             }
        else
            dest->push_back (at(i)->Clone ());
        }
    return dest;
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
CurveVectorPtr CurveVector::CloneAsBsplines () const
    {
    CurveVectorPtr bsplineCV = Create (m_boundaryType);
    switch (GetBoundaryType ())
        {
        case CurveVector::BOUNDARY_TYPE_Open:
        case CurveVector::BOUNDARY_TYPE_Outer:
        case CurveVector::BOUNDARY_TYPE_Inner:
            {
            for (size_t i = 0, n = size (); i < n; i++)
                {
                MSBsplineCurve bcurve;
                if (at(i)->GetMSBsplineCurve (bcurve))
                    bsplineCV->push_back (ICurvePrimitive::CreateBsplineCurve (bcurve));
                bcurve.ReleaseMem();
                }

            break;
            }

        case CurveVector::BOUNDARY_TYPE_ParityRegion:
        case CurveVector::BOUNDARY_TYPE_UnionRegion:
            {
            for (size_t i = 0, n = size (); i < n; i++)
                bsplineCV->push_back (ICurvePrimitive::CreateChildCurveVector_SwapFromSource (
                                            *at(i)->GetChildCurveVectorCP ()->CloneAsBsplines ()));

            break;
            }

        default:
            return NULL;
        }

    return bsplineCV;
    }

 static double s_fractionTolerance = 1.0e-10;
 struct PlaceInCurveVector
 {
private:
 int m_index;
 double m_a;
public:
 PlaceInCurveVector (int index, double a)
    {
    m_index = index;
    m_a = a;
    }
    
 bool AlmostEqual (PlaceInCurveVector const &other) const
    {
    return m_index == other.m_index && fabs (m_a - other.m_a) <= s_fractionTolerance;
    }


/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
static PlaceInCurveVector AtOpenStart(size_t n){return PlaceInCurveVector (0,0.0);}

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
static PlaceInCurveVector AtOpenEnd(size_t n){return PlaceInCurveVector (n > 0 ? (int)n-1 : 0, 1.0);}


/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
PlaceInCurveVector MoveByCycles(int numCycle, size_t n)
    {
    return PlaceInCurveVector (m_index + numCycle * (int)n, m_a);
    }
 int Index () const { return m_index;};
 double Fraction () const { return m_a;}
 bool ClearlyLessThan (PlaceInCurveVector const &other) const
     {
     if (m_index < other.m_index)
         return true;
     if (m_index > other.m_index)
        return false;
     if (AlmostEqual (other))
        return false;
     return m_a < other.m_a;
     }
 void ClampForIndexLimit (size_t n)
    {
    if (n == 0)
        {
        m_index = 0;
        }
    else if (m_index < 0)
        {
        m_index = 0;
        m_a = 0.0;
        }
    else if (m_index >= (int)n)
        {
        m_index = (int)n - 1;
        m_a = 1.0;
        }
    }
 };

typedef PlaceInCurveVector const &PlaceInCurveVectorCR;


/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
static void AppendOpenFragment(CurveVectorR dest, CurveVectorCR source, PlaceInCurveVectorCR placeA, PlaceInCurveVectorCR placeB)
    {
    if (placeA.AlmostEqual (placeB))
        return;
    CurveVectorPtr fragment = source.CloneBetweenDirectedFractions (placeA.Index (), placeA.Fraction (), placeB.Index (), placeB.Fraction (), false);
    if (fragment.IsValid ()
        && fragment->size () > 0)
        dest.push_back (ICurvePrimitive::CreateChildCurveVector_SwapFromSource (*fragment));
    }


/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
static void AppendCyclicFragment(CurveVectorR dest, CurveVectorCR source, PlaceInCurveVectorCR placeA, PlaceInCurveVectorCR placeB)
    {
    if (placeA.AlmostEqual (placeB))
        return;
    CurveVectorPtr fragment = source.CloneBetweenCyclicIndexedFractions (placeA.Index (), placeA.Fraction (), placeB.Index (), placeB.Fraction ());
    if (fragment.IsValid ()
        && fragment->size () > 0)
        dest.push_back (ICurvePrimitive::CreateChildCurveVector_SwapFromSource (*fragment));
    }

//! Return a CurveVector (BOUNDARY_TYPE_None) which is a collection of open CurveVectors that collectively contain all parts of the input
//! The returned vector is null if the input is anything other than a "path" vector.
//! <ul>
//! <li>
CurveVectorPtr CurveVector::GenerateAllParts (int indexA, double fractionA, int indexB, double fractionB) const
    {
    BoundaryType bt = GetBoundaryType ();
    PlaceInCurveVector placeA (indexA, fractionA);
    PlaceInCurveVector placeB (indexB, fractionB);
    size_t n = size ();
    CurveVectorPtr fragments = CurveVector::Create (BOUNDARY_TYPE_None);
    if (n == 0)
        return CurveVectorPtr ();
    if (bt == BOUNDARY_TYPE_Open)
        {

        placeA.ClampForIndexLimit (n);
        placeB.ClampForIndexLimit (n);
        
        if (placeA.AlmostEqual (placeB))
            {
            AppendOpenFragment (*fragments, *this, placeA, PlaceInCurveVector::AtOpenEnd (n));
            AppendOpenFragment (*fragments, *this, PlaceInCurveVector::AtOpenStart (n), placeA);
            return fragments;
            }
        else if (placeA.ClearlyLessThan (placeB))
            {
            AppendOpenFragment (*fragments, *this, placeA, placeB);
            AppendOpenFragment (*fragments, *this, placeB, PlaceInCurveVector::AtOpenEnd (n));
            AppendOpenFragment (*fragments, *this, PlaceInCurveVector::AtOpenStart (n), placeA);
            return fragments;
            }
        else
            {
            AppendOpenFragment (*fragments, *this, placeA, placeB);
            AppendOpenFragment (*fragments, *this, placeB, PlaceInCurveVector::AtOpenEnd (n));
            AppendOpenFragment (*fragments, *this, PlaceInCurveVector::AtOpenStart (n), placeA);
            return fragments;
            }
        }
    else if (bt == BOUNDARY_TYPE_Outer || bt == BOUNDARY_TYPE_Inner)
        {
        if (placeA.AlmostEqual (placeB))
            {
            // Same cut points.   This just resets the start to A.
            AppendCyclicFragment (*fragments, *this, placeA, placeA.MoveByCycles (1, n));
            return fragments;
            }
        else if (placeA.ClearlyLessThan (placeB))
            {
            AppendCyclicFragment (*fragments, *this, placeA, placeB);
            AppendCyclicFragment (*fragments, *this, placeB, placeA.MoveByCycles (1, n));
            return fragments;
            }
        else
            {
            AppendCyclicFragment (*fragments, *this, placeA, placeB);
            AppendCyclicFragment (*fragments, *this, placeB, placeA.MoveByCycles (-1, n));
            return fragments;
            }
        }
    else
        return CurveVectorPtr ();

    }

    //! Return a new vector containing curves from index0,fraction0 to index1,fraction1 with the (signed int!!) indices interpretted cyclically.
CurveVectorPtr CurveVector::CloneBetweenCyclicIndexedFractions (int index0, double fraction0, int index1, double fraction1) const
    {
    CurveVectorPtr dest = CurveVector::Create (CurveVector::BOUNDARY_TYPE_Open);
    double period;
    if (index0 == index1)
        {
        PushIfNonNull (*dest, at(CyclicIndex (index0))->CloneBetweenFractions (fraction0, fraction1, false));
        }
    else if (index1 > index0)
        {
        if (size () == 1)
            {
            if (fraction1 < fraction0 && at(0)->IsPeriodicFractionSpace (period))
                fraction1 += period;
            PushIfNonNull (*dest, at(CyclicIndex (index0))->CloneBetweenFractions (fraction0, fraction1, true));
            }
        else
            {
            // move forward ..
            PushIfNonNull (*dest, at(CyclicIndex (index0))->CloneBetweenFractions (fraction0, 1.0, false));
            for (int i = index0 + 1; i < index1; i++)
                PushIfNonNull (*dest, at(CyclicIndex (i))->Clone ());
            PushIfNonNull (*dest, at(CyclicIndex (index1))->CloneBetweenFractions (0.0, fraction1, false));
            }
        }
    else // index0 > index1
        {
        if (size () == 1)
            {
            if (fraction0 < fraction1 && at(0)->IsPeriodicFractionSpace (period))
                fraction1 -= period;
            PushIfNonNull (*dest, at(CyclicIndex (index0))->CloneBetweenFractions (fraction0, fraction1, true));
            }
        else
            {
            // move backward ..
            PushIfNonNull (*dest, at(CyclicIndex (index0))->CloneBetweenFractions (fraction0, 0.0, false));
            for (int i = index0 - 1; i > index1; i--)
                PushIfNonNull (*dest, at(CyclicIndex (i))->CloneBetweenFractions (1.0, 0.0, false));
            PushIfNonNull (*dest, at(CyclicIndex (index1))->CloneBetweenFractions (1.0, fraction1, false));
            }
        }
    return dest;
    }


//! Return a new vector containing curves from index0,fraction0 to index1,fraction1 corresponding to the the CurveLocationDetails.
CurveVectorPtr CurveVector::CloneBetweenDirectedFractions (CurveLocationDetailCR location0, CurveLocationDetailCR location1, bool allowExtrapolation, bool usePartialCurves) const
    {
    size_t index0, index1;
    if (LeafToIndex (location0.curve, index0)
        && LeafToIndex (location1.curve, index1))
        {
        return CloneBetweenDirectedFractions
                (
                (int)index0, location0.fraction,
                (int)index1, location1.fraction,
                allowExtrapolation, usePartialCurves
                );
        }
    return nullptr;        
    }

//! Return a new vector containing curves from index0,fraction0 to index1,fraction1.
//! Indices are restricted to array bounds.
CurveVectorPtr CurveVector::CloneBetweenDirectedFractions (int index0, double fraction0, int index1, double fraction1, bool allowExtrapolation, bool usePartialCurves) const
    {
    CurveVectorPtr dest = CurveVector::Create (CurveVector::BOUNDARY_TYPE_Open);
    int n = (int) size ();
    // indices are ints.  Assume every step can be out of bounds -- don't try to preadjust, just test and skip if bad.
    if (index0 == index1)
        {
        if (index0 >= 0 && index0 < n)
            CloneAndPush (*dest, at(index1), fraction0, fraction1, allowExtrapolation, usePartialCurves);
        }
    else if (index1 > index0)
        {
        fraction0 = DoubleOps::Min (fraction0, 1.0);
        fraction1 = DoubleOps::Max (fraction1, 0.0);
        if (index0 >= 0 && index0 < n)
            CloneAndPush (*dest, at(index0), fraction0, 1.0, allowExtrapolation, usePartialCurves);
        for (int i = index0 + 1; i < index1; i++)
            if (i >= 0 && i < n)
                PushIfNonNull (*dest, at(i)->Clone ());
        if (index1 >= 0 && index1 < n)
            CloneAndPush (*dest, at(index1), 0.0, fraction1, allowExtrapolation, usePartialCurves);
        }
    else // index0 > index1
        {
        fraction0 = DoubleOps::Max (fraction0, 0.0);
        fraction1 = DoubleOps::Min (fraction1, 1.0);
        if (index0 < n)
            CloneAndPush (*dest, at(index0), fraction0, 0.0, allowExtrapolation, usePartialCurves);
        for (int i = index0 - 1; i > index1; i--)
            if (i >= 0 && i < n)
                CloneAndPush (*dest, at(CyclicIndex (i)), 1.0, 0.0, false, false);
        if (index1 >= 0 && index1 < n)
            CloneAndPush (*dest, at(CyclicIndex (index1)), 1.0, fraction1, allowExtrapolation, usePartialCurves);
        }
    return dest;
    }

enum ChildTerminusType
    {
    CTT_None,
    CTT_Linear,
    CTT_Curve
    };
// Return tangent vector in REVERSE direction from start.   Length is segment length.
static ChildTerminusType ChildStartInfo (CurveVectorCR parent, size_t childIndex, DRay3dR tangent)
    {
    if (childIndex >= parent.size ())
        return CTT_None;
    if (childIndex >= parent.size ())
        return CTT_None;
    ICurvePrimitive::CurvePrimitiveType childType = parent.at (childIndex)->GetCurvePrimitiveType ();
    DSegment3d segment;
    if (childType == ICurvePrimitive::CURVE_PRIMITIVE_TYPE_Line)
        {
        parent.at(childIndex)->TryGetLine (segment);
        tangent.origin      = segment.point[0];
        tangent.direction   = DVec3d::FromStartEnd (segment.point[1], segment.point[0]);
        return CTT_Linear;
        }
    else if (childType == ICurvePrimitive::CURVE_PRIMITIVE_TYPE_LineString)
        {
        bvector<DPoint3d> const * linestring = parent.at(childIndex)->GetLineStringCP ();
        size_t n = linestring->size ();
        if (n > 1 && parent.at(childIndex)->TryGetSegmentInLineString (segment, 0))
            {
            tangent.origin      = segment.point[0];
            tangent.direction   = DVec3d::FromStartEnd (segment.point[1], segment.point[0]);
            return CTT_Linear;
            }
        }
    if (parent.at (childIndex)->FractionToPoint (0.0, tangent.origin, tangent.direction))
        return CTT_Curve;
    return CTT_None;
    }
static ChildTerminusType ChildEndInfo (CurveVectorCR parent, size_t childIndex, DRay3dR tangent)
    {
    if (childIndex >= parent.size ())
        return CTT_None;
    ICurvePrimitive::CurvePrimitiveType childType = parent.at (childIndex)->GetCurvePrimitiveType ();
    DSegment3d segment;
    if (childType == ICurvePrimitive::CURVE_PRIMITIVE_TYPE_Line)
        {
        if (parent.at(childIndex)->TryGetLine (segment))
            {
            tangent.origin      = segment.point[1];
            tangent.direction   = DVec3d::FromStartEnd (segment.point[0], segment.point[1]);
            return CTT_Linear;
            }
        }
    else if (childType == ICurvePrimitive::CURVE_PRIMITIVE_TYPE_LineString)
        {
        bvector<DPoint3d> const * linestring = parent.at(childIndex)->GetLineStringCP ();
        size_t n = linestring->size ();
        if (n > 0 && parent.at(childIndex)->TryGetSegmentInLineString (segment, n - 2))
            {
            tangent.origin      = segment.point[1];
            tangent.direction   = DVec3d::FromStartEnd (segment.point[0], segment.point[1]);
            return CTT_Linear;
            }
        }
    if (parent.at (childIndex)->FractionToPoint (1.0, tangent.origin, tangent.direction))
        return CTT_Curve;
    return CTT_None;
    }


void KeepAlive (double a)
    {
    }

bool TrySmallAdjustmentTransform
(
TransformR transform,
DPoint3dCR fixedPoint,
DPoint3dCR targetA,
DPoint3dCR targetB,
double lengthMultiple   // only proceed if target vectors are larger than this multiple of delta...
)
    {
    transform.InitIdentity ();
    DVec3d vectorA = DVec3d::FromStartEnd (fixedPoint, targetA);
    DVec3d vectorB = DVec3d::FromStartEnd (fixedPoint, targetB);
    DVec3d cross = DVec3d::FromCrossProduct (vectorA, vectorB);
    double delta = targetA.Distance (targetB);
    double a = vectorA.Magnitude ();
    double b = vectorB.Magnitude ();
    double cc = cross.MagnitudeSquared ();
    double c = sqrt (cc);   // magA magB sin(theta)
    double d = vectorA.DotProduct (vectorB);  // magA magB cos(theta)
    if (delta * lengthMultiple > DoubleOps::Max (a, b))
        return false;
    double scale = b / a;
    double theta = atan2 (c, d);
    // what if parallel?
    DVec3d unitNormal;
    unitNormal.Normalize (cross);
    RotMatrix rotator = RotMatrix::FromVectorAndRotationAngle (unitNormal, theta);
    RotMatrix scaleAndRotate;
    scaleAndRotate.ScaleColumns (rotator, scale, scale, scale);
    transform.InitFromMatrixAndFixedPoint (scaleAndRotate, fixedPoint);
    DVec3d vectorB1;
    DPoint3d pointB2;
    scaleAndRotate.Multiply (vectorB1, vectorA);
    transform.Multiply (pointB2, targetA);
    double d1 = vectorB1.Distance (vectorB);
    double d2 = pointB2.Distance (targetB);
    KeepAlive (d1);
    KeepAlive (d2);
    return true;
    }
/*--------------------------------------------------------------------------------**//**
ASSUME input is a chain or loop, hence containing primitives immediately.
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
static CurveVectorPtr CloneChainWithGapsClosed (CurveVectorCR source, CurveGapOptionsCR options)
    {
    if (!(source.IsOpenPath () || source.IsClosedPath ()))
        return NULL;
    double gapTolerance = options.GetEqualPointTolerance ();
    double maxAdjust = options.GetMaxDirectAdjustTolerance ();
    double maxAdjustAlongCurve = options.GetMaxAdjustAlongCurve ();
    bool   removePriorGaps = options.GetRemovePriorGapPrimitives ();
    static double s_nearParallelTolerance = 0.001;
    size_t numGap = 0;
    double gapSum = 0.0;
    double a;
    CurveVectorPtr result = CurveVector::Create (source.GetBoundaryType ());
    for (size_t i = 0, n = source.size (); i < n; i++)
        {
        if (removePriorGaps && source[i]->GetMarkerBit (ICurvePrimitive::CURVE_PRIMITIVE_BIT_GapCurve))
            {
            // skip it!!!
            numGap++;
            source[i]->Length (a);
            gapSum += a;
            }
        else
            {
            result->push_back (source[i]->Clone ());
            }
        }
    size_t i0 = 0;
    size_t i1 = 1;
    size_t n = result->size ();

    if (source.IsClosedPath ())
        {
        i1 = 0;
        i0 = n - 1; 
        }
    static double s_fractionTrigger = 0.49;
    // Pass 1 -- direct adjustments.
    for (; i1 < n; i0 = i1++)
        {
        DRay3d tangentA, tangentB;
        ChildTerminusType ctt0 = ChildEndInfo (*result, i0, tangentA);
        ChildTerminusType ctt1 = ChildStartInfo (*result, i1, tangentB);
        double fractionA, fractionB;
        DPoint3d pointA, pointB, midPoint;
        double gapSize = tangentA.origin.Distance (tangentB.origin);
        double gapSize1;
        bool needGapSegment = ctt0 != CTT_None
                      && ctt1 != CTT_None
                      && gapSize > gapTolerance;

        if (ctt0 == CTT_None || ctt1 == CTT_None)
            {
            needGapSegment = false;
            }
        else if (gapSize <= gapTolerance)
            {
            needGapSegment = false;
            }
        else if (ctt0 == CTT_Linear && ctt1 == CTT_Linear)
            {
            // both fractions are positive "outbound"
            if (DRay3d::ClosestApproachUnboundedRayUnboundedRay (fractionA, fractionB, pointA, pointB, tangentA, tangentB))
                {
                double distanceAlongA = tangentA.origin.Distance (pointA);
                double distanceAlongB = tangentB.origin.Distance (pointB);
                double distanceAB = pointA.Distance (pointB);
                if (                
                       fractionA > -s_fractionTrigger
                    && fractionB > -s_fractionTrigger
                    && distanceAlongA < maxAdjustAlongCurve
                    && distanceAlongB < maxAdjustAlongCurve
                    && distanceAB <= maxAdjust)
                    {
                    midPoint = DPoint3d::FromInterpolate (pointA, 0.5, pointB);
                    if (   result->at (i0)->TrySetEnd (midPoint)
                        && result->at (i1)->TrySetStart (midPoint))
                    needGapSegment = false;
                    }
                }
            }
        else
            {
            fractionA = 1.0;
            fractionB = 0.0;
            if (CurveCurve::ClosestApproachNewton (*result->at(i0), *result->at(i1),
                            fractionA, fractionB, pointA, pointB))
                {
                gapSize1 = pointA.Distance (pointB);
                double distanceAlongA = tangentA.origin.Distance (pointA);
                double distanceAlongB = tangentB.origin.Distance (pointB);
                ICurvePrimitivePtr replacementA = result->at (i0)->CloneBetweenFractions (0.0, fractionA, true);
                ICurvePrimitivePtr replacementB = result->at (i1)->CloneBetweenFractions (fractionB, 1.0, true);
                // We would really like to take both trimmed or extended replacements directly ...
                if (gapSize1 <= gapTolerance        // i.e. we found a near intersection (rather than a near tangency)
                    && fractionA > 1.0 - s_fractionTrigger
                    && fractionB < s_fractionTrigger
                    && distanceAlongA < maxAdjustAlongCurve
                    && distanceAlongB < maxAdjustAlongCurve
                    && replacementA.IsValid ()
                    && replacementB.IsValid ()
                    )
                    {
                    midPoint = DPoint3d::FromInterpolate (pointA, 0.5, pointB);
                    if (   replacementA.IsValid ()
                        && replacementB.IsValid ())
                        {
                        result->at(i0) = replacementA;
                        result->at(i1) = replacementB;
                        needGapSegment = false;
                        }
                    }
                // If "take both" failed, try "take one" with an endpoint adjust on the other
                if (needGapSegment && gapSize1 <= maxAdjust)
                    {
                    DPoint3d pointX;
                    // If a line is flying by a tangency point,
                    //   trim/extend the curve to the tangency point.
                    //   move the line end to the tangency point.
                    if (ctt0 == CTT_Linear && ctt1 == CTT_Curve && replacementB.IsValid ())
                        {
                        replacementB->GetStartEnd (pointB, pointX);
                        if (result->at (i0)->TrySetEnd (pointB))
                            {
                            result->at (i1) = replacementB;
                            needGapSegment = false;
                            }
                        }
                    else if (ctt0 == CTT_Curve && ctt1 == CTT_Linear && replacementA.IsValid ())
                        {
                        replacementA->GetStartEnd (pointX, pointA);
                        if (result->at (i1)->TrySetStart (pointA))
                            {
                            result->at (i0) = replacementA;
                            needGapSegment = false;
                            }
                        }
                    else
                        {
                        static double s_smallAdjustmentFactor = 1000.0;
                        DPoint3d pointA0, pointA1, pointB0, pointB1, pointC0, pointC1, pointD0, pointD1;
                        result->at(i0)->GetStartEnd (pointA0, pointA1);
                        result->at(i1)->GetStartEnd (pointB0, pointB1);
                        double originalDelta = pointA1.Distance (pointB0);
                        double e0, e1;
                        Transform transform;
                        if (TrySmallAdjustmentTransform (transform, pointA0, pointA1, pointB0, s_smallAdjustmentFactor))
                            {
                            transform.Multiply (pointC0, pointA0);
                            transform.Multiply (pointC1, pointA1);
                            e0 = pointC0.Distance (pointA0);  // should be zero -- center of rotation.
                            e1 = pointC1.Distance (pointB0);  // should be zero -- this is the primary move.
                            result->at (i0)->TransformInPlace (transform);
                            result->at (i0)->GetStartEnd (pointD0, pointD1);
                            e0 = pointA0.Distance (pointD0);  // center of rotation
                            e1 = pointB0.Distance(pointD1);   // primary move
                            KeepAlive (e0);
                            KeepAlive (e1);
                            KeepAlive (originalDelta);
                            needGapSegment = false;
                            }
                        } 
                    }
                }
             else if (gapSize < maxAdjust
                        && tangentA.direction.SmallerUnorientedAngleTo (tangentB.direction) < s_nearParallelTolerance
                      )
                {
                // nearly parallel at ends -- try to move a linear end...
                if (ctt0 == CTT_Linear
                    && result->at(i0)->TrySetEnd (tangentB.origin))
                        needGapSegment = false;
                else if (ctt1 == CTT_Linear
                    && result->at(i1)->TrySetStart (tangentA.origin))
                        needGapSegment = false;
                else
                    {
                    Transform transform;
                    static double s_smallAdjustmentFactor = 1000.0;
                    DPoint3d pointA0, pointA1, pointB0, pointB1;
                    result->at(i0)->GetStartEnd (pointA0, pointA1);
                    result->at(i1)->GetStartEnd (pointB0, pointB1);
                    if (TrySmallAdjustmentTransform (transform, pointA0, pointA1, pointB0, s_smallAdjustmentFactor))
                        {
                        result->at (i0)->TransformInPlace (transform);
                        }
                    } 
                }
             }

        if (needGapSegment)
            {
            ICurvePrimitivePtr gapPrimitive = ICurvePrimitive::CreateLine (
                        DSegment3d::From (tangentA.origin, tangentB.origin));
            gapPrimitive->SetMarkerBit (ICurvePrimitive::CURVE_PRIMITIVE_BIT_GapCurve, true);
            result->insert (result->begin () + i1, gapPrimitive);
            i1++;
            n++;
            }
        }
    return result;
    }
/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
CurveVectorPtr CurveVector::CloneWithGapsClosed (CurveGapOptionsCR options) const
    {
    CurveVectorPtr result = Create (m_boundaryType);
    switch (GetBoundaryType ())
        {
        case CurveVector::BOUNDARY_TYPE_Open:
        case CurveVector::BOUNDARY_TYPE_Outer:
        case CurveVector::BOUNDARY_TYPE_Inner:
            {
            result = CloneChainWithGapsClosed (*this, options);
            break;
            }

        case CurveVector::BOUNDARY_TYPE_ParityRegion:
        case CurveVector::BOUNDARY_TYPE_UnionRegion:
            {
            for (size_t i = 0, n = size (); i < n; i++)
                result->push_back (ICurvePrimitive::CreateChildCurveVector_SwapFromSource (
                                            *at(i)->GetChildCurveVectorCP ()->CloneWithGapsClosed (options)));

            break;
            }
        case CurveVector::BOUNDARY_TYPE_None:
            {
            for (size_t i = 0, n = size (); i < n; i++)
                {
                if (at(i)->GetCurvePrimitiveType () == ICurvePrimitive::CURVE_PRIMITIVE_TYPE_CurveVector)
                    {
                    result->push_back (ICurvePrimitive::CreateChildCurveVector_SwapFromSource (
                                            *at(i)->GetChildCurveVectorCP ()->CloneWithGapsClosed (options)));
                    }
                else
                    {
                    // Simple curve primitive -- just clone in isolation.
                    result->push_back (at(i)->Clone ());
                    }
                }
            break;
            }

        default:
            return NULL;
        }

    return result;
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
double CurveVector::MaxGapWithinPath () const
    {
    double maxGap = 0.0;
    size_t i0 = 0;
    size_t i1 = 1;
    size_t n = size ();

    if (IsClosedPath ())
        {
        i1 = 0;
        i0 = n - 1; 
        }
        
    if (IsClosedPath () || IsOpenPath ())
        {
        DPoint3d start0, end0, start1, end1;
        bool ok0 = at(i0)->GetStartEnd (start0, end0);
        bool ok1;
        for (; i1 < n; i0 = i1++, ok1 = ok0, start0 = start1, end0 = end1)
            {
            ok1 = at (i1)->GetStartEnd (start1, end1);
            if (ok0 && ok1)
                DoubleOps::UpdateMax (maxGap, end0.Distance (start1));
            }
        }
    else
        {
        CurveVectorCP child;
        for (size_t i = 0; i < n; i++)
            {
            if (NULL != (child = at(i0)->_GetChildCurveVectorCP ()))
                DoubleOps::UpdateMax (maxGap, child->MaxGapWithinPath ());
            }
        }
    return maxGap;
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
CurveVectorPtr CurveVector::CloneReversed () const
    {
    CurveVectorPtr result = Create (m_boundaryType);
    for (size_t i = size (); i  > 0;)
        {
        i--;
        CurveVectorPtr child = at(i)->GetChildCurveVectorP ();
        if (child.IsValid ())
            {
            result->push_back (ICurvePrimitive::CreateChildCurveVector (child->CloneReversed ()));
            }
        else    // hmm.. does 1..0 work for "all" primitives?  would be better to have a dispatched clone reverse
            result->push_back (at(i)->CloneBetweenFractions (1.0, 0.0, false));
        }
    return result;    
    }





#define WIP1
#ifdef WIP1
static double s_absTol = 1.0e-12;
struct ExtendedCurvePrimitive
{
ICurvePrimitiveCR m_curve;
int m_numDerivative;
bool m_useNative;
ExtendedCurvePrimitive (ICurvePrimitiveCR curve, int numDerivative, bool useNativeIfExtensible) :
        m_curve (curve),
        m_useNative (useNativeIfExtensible && curve.IsExtensibleFractionSpace ()),
        m_numDerivative (numDerivative)
    {}
    
void FractionToPointWithExtrapolation (double fraction, DPoint3dR xyz, DVec3dR dX, DVec3dR ddX) const
    {
    if (m_useNative || (fraction >= 0.0 && fraction <= 1.0))
        {
        m_curve.FractionToPoint (fraction, xyz, dX, ddX);
        }
    else
        {
        double f = fraction;
        if (fraction > 1.0)
            f = 1.0;
        else if (fraction < 0.0)
            f = 0.0;
        m_curve.FractionToPoint (f, xyz, dX, ddX);
        double df = f - fraction;
        if (m_numDerivative > 1)
            {
            xyz.SumOf (xyz, dX, df, ddX, 0.5 * df * df);
            dX.SumOf (dX, ddX, df);
            }
        else
            {
            xyz.SumOf (xyz, dX, df);
            ddX.Zero ();
            }
        }
    }

  struct CloseApproachFunction : public FunctionRRToRRD
    {
    ExtendedCurvePrimitive const &m_curveA;
    ExtendedCurvePrimitive const &m_curveB;
    double m_uA, m_uB;
    DPoint3d m_XA, m_XB;
    DVec3d m_dXA, m_dXB;
    DVec3d m_ddXA, m_ddXB;
    CloseApproachFunction (ExtendedCurvePrimitive const & curveA, ExtendedCurvePrimitive const & curveB)
        : m_curveA (curveA), m_curveB (curveB)
        {
        }
    bool EvaluateRRToRRD (double uA, double uB,
        double &fA, double &fB,
        double &dfAduA, double &dfAduB,
        double &dfBduA, double &dfBduB
        ) override
        {
        m_uA = uA;
        m_uB = uB;
        m_curveA.FractionToPointWithExtrapolation (uA, m_XA, m_dXA, m_ddXA);
        m_curveB.FractionToPointWithExtrapolation (uB, m_XB, m_dXB, m_ddXB);
        DVec3d W = DVec3d::FromStartEnd (m_XA, m_XB);   // B-A
        fA = m_dXA.DotProduct (W);
            dfAduA = m_ddXA.DotProduct (W) - m_dXA.DotProduct (m_dXA);
            dfAduB = m_dXA.DotProduct (m_dXB);
        fB = m_dXB.DotProduct (W);
            dfBduA = -dfAduB;
            dfBduB = m_ddXB.DotProduct (W) + m_dXB.DotProduct (m_dXB);
        return true;
        }
    };
 };
 
 bool CurveCurve::ClosestApproachNewton (
    ICurvePrimitiveCR curveA,
    ICurvePrimitiveCR curveB,
    double &fractionA, double &fractionB,
    DPoint3dR xyzA, DPoint3dR xyzB
    )
    {
    ExtendedCurvePrimitive xcurveA (curveA, 2, true);
    ExtendedCurvePrimitive xcurveB (curveB, 2, true);
    ExtendedCurvePrimitive::CloseApproachFunction F (xcurveA, xcurveB);
    NewtonIterationsRRToRR newton (s_absTol);
    double uA = fractionA, uB = fractionB;
    if (newton.RunNewton (uA, uB, F))
        {
        // retouch final evaluation.
        fractionA = uA;
        fractionB = uB;
        curveA.FractionToPoint (fractionA, xyzA);
        curveB.FractionToPoint (fractionB, xyzB);
        return true;
        }
    return false;
    }

  
#endif
END_BENTLEY_GEOMETRY_NAMESPACE
