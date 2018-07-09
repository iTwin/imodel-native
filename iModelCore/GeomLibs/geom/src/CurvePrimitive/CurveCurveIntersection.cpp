/*--------------------------------------------------------------------------------------+
|
|     $Source: geom/src/CurvePrimitive/CurveCurveIntersection.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <bsibasegeomPCH.h>

#include "CurveCurveProcessor.h"
BEGIN_BENTLEY_GEOMETRY_NAMESPACE

static double s_lineFractionTol = 1.0e-8;

bool TolerancedFractionLE (double fa, double fb)
  {
  if (fabs (fa - fb) < s_lineFractionTol)
    return true;
  return fa < fb;
  }

bool TolerancedFractionIn01 (double f)
    {
    return f > -s_lineFractionTol && f < 1.0 + s_lineFractionTol;
    }
/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
bool AlmostEqualXY(DPoint4dCR pointA, DPoint4dCR pointB)
    {
    if (pointA.w != 0.0 && pointB.w != 0.0)
        {
        double xA = pointA.x / pointA.w;
        double yA = pointA.y / pointA.w;
        double xB = pointB.x / pointB.w;
        double yB = pointB.y / pointB.w;
        double q = 1.0 + fabs (xA) + fabs (xB) + fabs (yA) + fabs (yB);
        double tol = DoubleOps::SmallCoordinateRelTol () * q;
        return fabs (xA - xB) < tol && fabs (yA - yB) < tol;
        }
    return false;
    }


/*--------------------------------------------------------------------------------**//**
* @bsistruct                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
struct CCIProcessor : public CurveCurveProcessor
{
CurveVectorR m_intersectionA;
CurveVectorR m_intersectionB;

CCIProcessor (CurveVectorR intersectionA, CurveVectorR intersectionB, DMatrix4dCP pWorldToLocal, double tol, bool extend = false) :
    CurveCurveProcessor (pWorldToLocal, tol),
    m_intersectionA (intersectionA),
    m_intersectionB (intersectionB)
    {
    if (extend)
        SetExtend (extend);
    }

void announce (char const *messages)
    {
    }
/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
void CollectPair(
        ICurvePrimitiveP curve0,
        ICurvePrimitiveP curve1,
        double fraction0,
        double fraction1,
        bool bReverseCurveOrder
        )
    {
    if (!m_extend &&
        !(TolerancedFractionIn01 (fraction0) && TolerancedFractionIn01(fraction1))
        )
        {
        announce ("fractions out of range");
        // ignore out of range
        }
    else if (!bReverseCurveOrder)
        {
        m_intersectionA.push_back(ICurvePrimitive::CreatePartialCurve (curve0, fraction0, fraction0, 0));
        m_intersectionB.push_back(ICurvePrimitive::CreatePartialCurve (curve1, fraction1, fraction1, 0));
        }
    else
        {
        m_intersectionB.push_back(ICurvePrimitive::CreatePartialCurve (curve0, fraction0, fraction0, 0));
        m_intersectionA.push_back(ICurvePrimitive::CreatePartialCurve (curve1, fraction1, fraction1, 0));
        }
    }


/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
void CollectPair(
        ICurvePrimitiveP curveA,
        ICurvePrimitiveP curveB,
        double fractionA0,
        double fractionA1,
        double fractionB0,
        double fractionB1,
        bool bReverseCurveOrder
        )
    {
    if (!bReverseCurveOrder)
        {
        m_intersectionA.push_back(ICurvePrimitive::CreatePartialCurve (curveA, fractionA0, fractionA1, 0));
        m_intersectionB.push_back(ICurvePrimitive::CreatePartialCurve (curveB, fractionB0, fractionB1, 0));
        }
    else
        {
        m_intersectionB.push_back(ICurvePrimitive::CreatePartialCurve (curveA, fractionA0, fractionA1, 0));
        m_intersectionA.push_back(ICurvePrimitive::CreatePartialCurve (curveB, fractionB0, fractionB1, 0));
        }
    }


/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
void CollectPairs(
        ICurvePrimitiveP curve0,
        ICurvePrimitiveP curve1,
        bvector<double> const & fraction0,
        bvector<double> const & fraction1,
        bool bReverseCurveOrder
        )
    {
    for (size_t i = 0; i < fraction0.size (); i++)
        {
        CollectPair (curve0, curve1, fraction0[i], fraction1[i], bReverseCurveOrder);
        }
    }
// search for index where if (f0,f1) is a member of both indexed segments

// ASSUME segment0, segment1 are same size.
// return SIZE_MAX if not found.
size_t FindContainingSegmentPair
(
double f0,
double f1,
bvector<DSegment1d> const &segment0,
bvector<DSegment1d> const &segment1
)
    {
    for (size_t i = 0; i < segment0.size (); i++)
        {
        if (segment0[i].IsInteriorOrEnd (f0) && segment1[i].IsInteriorOrEnd (f1))
            return i;
        }
    return SIZE_MAX;
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
void CollectPairs(
        ICurvePrimitiveP curve0,
        ICurvePrimitiveP curve1,
        bvector<double> const & fraction0,
        bvector<double> const & fraction1,
        bvector<DSegment1d> const &overlap0,
        bvector<DSegment1d> const &overlap1,
        bool bReverseCurveOrder
        )
    {
    for (size_t i = 0; i < fraction0.size (); i++)
        {
        if (SIZE_MAX == FindContainingSegmentPair (fraction0[i], fraction1[i], overlap0, overlap1))
            CollectPair (curve0, curve1, fraction0[i], fraction1[i], bReverseCurveOrder);
        }
    double fractionTol = Angle::SmallAngle ();
    size_t n = overlap0.size ();
    for (size_t i = 0; i < overlap0.size (); i++)
        {
        size_t j = i;
        // Advance over adjoining intervals ...
        for (;
                   j + 1 < n
            && overlap0[j].EndToStartDistance (overlap0[j + 1]) < fractionTol
            && overlap1[j].EndToStartDistance (overlap1[j + 1]) < fractionTol
            ;)
            {
            j++;
            }

        CollectPair (curve0, curve1,
                overlap0[i].GetStart (), overlap0[j].GetEnd (),
                overlap1[i].GetStart (), overlap1[j].GetEnd (),
                bReverseCurveOrder
                );
        i = j;
        }
    }

// Return true if the a fractional position within an edge is internal to the linestring.
//    (false for extensions of internal edges)
bool validEdgeFractionWithinLinestring (double f, size_t edgeIndex, size_t numPoint)
    {
    // interior fractions are always ok ...
    if (f >= -s_lineFractionTol && f <= 1.0 + s_lineFractionTol)
        return true;

    // We are outside the immediate edge...
    if (m_extend)
        {
        if (numPoint <= 2)
            return true;
        if (edgeIndex <= 0)
            return f <= 1.0;
        else if (edgeIndex == numPoint - 2)
            return f >= 0.0;
        }
    
    return false;
    }

double EdgeFractionToLinestringFraction (double f, size_t index, size_t numPoint)
  {
  if (numPoint == 2)
    return f;
  return (index + f) / (numPoint - 1);
  }

// Return true if the a fractional position within an edge is internal to the linestring.
//    (false for extensions of internal edges)
bool validArcAngle (double theta, DEllipse3d const &ellipse)
    {
    if (m_extend)
        return true;
    if (ellipse.IsAngleInSweep (theta))
        return true;
    return false;
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
void ProcessLineLine(
        ICurvePrimitiveP curveA, DSegment3dCR segmentA, size_t indexA, size_t numA,
        ICurvePrimitiveP curveB, DSegment3dCR segmentB, size_t indexB, size_t numB,
        bool bReverseOrder)
    {
    DSegment4d hSegA, hSegB;
    DPoint4d hPointA, hPointB;
    double   fractionA, fractionB;
    Transform (hSegA, segmentA);
    Transform (hSegB, segmentB);

    // Parallel? Coincident? Distinct?
    DSegment4d segmentAOnB, segmentBOnA;
    double a0, a1, b0, b1;  // fractional positions on respective segments.
    if (   hSegA.ProjectDPoint4dCartesianXYW (segmentBOnA.point[0], a0, hSegB.point[0])
        && AlmostEqualXY (segmentBOnA.point[0], hSegB.point[0])
        && hSegA.ProjectDPoint4dCartesianXYW (segmentBOnA.point[1], a1, hSegB.point[1])
        && AlmostEqualXY (segmentBOnA.point[1], hSegB.point[1])
        && hSegB.ProjectDPoint4dCartesianXYW (segmentAOnB.point[0], b0, hSegA.point[0])
        && AlmostEqualXY (segmentAOnB.point[0], hSegA.point[0])
        && hSegB.ProjectDPoint4dCartesianXYW (segmentAOnB.point[1], b1, hSegA.point[1])
        && AlmostEqualXY (segmentAOnB.point[1], hSegA.point[1])
        )
        {
        ValidatedDSegment1d validatedBonA;
        DSegment1d fullBonA (b0, b1);
        if (a0 <= a1)
            validatedBonA = DSegment1d (0,1).DirectedOverlap (DSegment1d (a0, a1));
        else
            {
            validatedBonA = DSegment1d (0,1).DirectedOverlap (DSegment1d (a1, a0));
            validatedBonA.Value ().ReverseInPlace ();
            }
            
        if (validatedBonA.IsValid ())
            {
            DSegment1d intervalBonA = validatedBonA;
            DSegment1d intervalAonB (
                fullBonA.FractionToPoint (intervalBonA.GetStart ()),
                fullBonA.FractionToPoint (intervalBonA.GetEnd ())
                );
            // Whichever interval is first after reversal must have forward motion
            if (bReverseOrder)
                {
                if (intervalAonB.Delta () < 0.0)
                    {
                    intervalBonA.ReverseInPlace ();
                    intervalAonB.ReverseInPlace ();
                    }
                }
            else
                {
                if (intervalBonA.Delta () < 0.0)
                    {
                    intervalBonA.ReverseInPlace ();
                    intervalAonB.ReverseInPlace ();
                    }
                }
            CollectPair (curveA, curveB,
                        EdgeFractionToLinestringFraction (intervalBonA.GetStart (), indexA, numA),
                        EdgeFractionToLinestringFraction (intervalBonA.GetEnd (), indexA, numA),
                        EdgeFractionToLinestringFraction (intervalAonB.GetStart (), indexB, numB),
                        EdgeFractionToLinestringFraction (intervalAonB.GetEnd (), indexB, numB),
                        bReverseOrder
                        );
            }
        }
    else  if (DSegment4d::IntersectXY(hPointA, fractionA, hPointB, fractionB, hSegA, hSegB)
        && validEdgeFractionWithinLinestring (fractionA, indexA, numA)
        && validEdgeFractionWithinLinestring (fractionB, indexB, numB)
        )
        {
        CollectPair (curveA, curveB,
                EdgeFractionToLinestringFraction (fractionA, indexA, numA),
                EdgeFractionToLinestringFraction (fractionB, indexB, numB),
                bReverseOrder);
        }
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
void ProcessLineLine(
        ICurvePrimitiveP curveA, DSegment3dCR segmentA,
        ICurvePrimitiveP curveB, DSegment3dCR segmentB,
        bool bReverseOrder) override
    {
    ProcessLineLine (curveA, segmentA, 0, 2, curveB, segmentB, 0, 2, bReverseOrder);
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
void ProcessLineLinestring(
        ICurvePrimitiveP curveA, DSegment3dCR segmentA,
        ICurvePrimitiveP curveB, bvector<DPoint3d> const &linestringB,
        bool bReverseOrder) override
    {
    DSegment3d segmentB;
    size_t numPointsB = linestringB.size ();
    if (numPointsB < 2)
        return;

    for (size_t i = 1; i < numPointsB; i++)
        {
        segmentB.Init (linestringB[i-1], linestringB[i]);
        ProcessLineLine (curveA, segmentA, 0, 2, curveB, segmentB, i - 1, numPointsB, bReverseOrder);
        }
    }


/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
void ProcessLinestringLinestring(
        ICurvePrimitiveP curveA, bvector<DPoint3d> const &linestringA,
        ICurvePrimitiveP curveB, bvector<DPoint3d> const &linestringB,
        bool bReverseOrder) override
    {
    DSegment3d segmentA, segmentB;
    size_t nXYZA = linestringA.size ();
    size_t nXYZB = linestringB.size ();    
    for (size_t iA = 1; iA < nXYZA; iA++)
        {
        segmentA.Init (linestringA[iA-1],linestringA[iA]);
        for (size_t iB = 1; iB < nXYZB; iB++)
            {
            segmentB.Init (linestringB[iB-1], linestringB[iB]);
            ProcessLineLine (curveA, segmentA, iA-1, nXYZA, curveB, segmentB, iB - 1, nXYZB, bReverseOrder);
            }
        }
    }




/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
void ProcessLineArc(
        ICurvePrimitiveP curveA, DSegment3dCR segmentA,
        ICurvePrimitiveP curveB, DEllipse3dCR ellipseB,
        bool bReverseOrder) override
    {
    DSegment4d hSegA;
    DConic4d hConicB;
    DPoint4d hPointA[2], hPointB[2];
    double   fractionA[2], thetaB[2];
    Transform (hSegA, segmentA);
    Transform (hConicB, ellipseB);
    int numIntersection = bsiDConic4d_intersectDSegment4dXYW
                (
                &hConicB,
                hPointB, thetaB,
                hPointA, fractionA,
                &hSegA
                );
    for (int i = 0; i < numIntersection; i++)
        {
        if (validEdgeFractionWithinLinestring (fractionA[i], 0, 2)
            && validArcAngle (thetaB[i], ellipseB)
            )
            CollectPair (curveA, curveB,
                fractionA[i],
                ellipseB.AngleToFraction (thetaB[i]),
                bReverseOrder);
        }
    }


/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
void ProcessLinestringArc(
        ICurvePrimitiveP curveA, bvector<DPoint3d> const &linestringA,
        ICurvePrimitiveP curveB, DEllipse3dCR ellipseB,
        bool bReverseOrder) override
    {
    DSegment4d hSegA;
    DConic4d hConicB;
    DPoint4d hPointA[2], hPointB[2];
    double   fractionA[2], thetaB[2];
    Transform (hConicB, ellipseB);
    size_t nXYZA = linestringA.size ();
    TransformWeightedPoint (hSegA.point[0], linestringA[0], 1.0);
    for (size_t iA = 1; iA < nXYZA; iA++, hSegA.point[0] = hSegA.point[1])
        {
        TransformWeightedPoint (hSegA.point[1], linestringA[iA], 1.0);

        int numIntersection = bsiDConic4d_intersectDSegment4dXYW
                    (
                    &hConicB,
                    hPointB, thetaB,
                    hPointA, fractionA,
                    &hSegA
                    );
        for (int i = 0; i < numIntersection; i++)
            {
            if (validEdgeFractionWithinLinestring (fractionA[i], iA - 1, nXYZA)
                && validArcAngle (thetaB[i], ellipseB)
                )
                {
                CollectPair (curveA, curveB,
                        PolylineOps::SegmentFractionToPolylineFraction (iA -1, nXYZA - 1, fractionA[i]),
                        ellipseB.AngleToFraction (thetaB[i]),
                        bReverseOrder);
                }
            }
        }
    }

DConic4d Flatten (DConic4dCR conic)
    {
    DConic4d result = conic;
    result.center.x = result.vector0.x = result.vector90.x;
    return result;
    }
// fast test for identical basis.
// 
bool SameBasisXY (DConic4dCR conicAxyz, DConic4dCR conicBxyz)
    {
    DConic4d conicA = Flatten (conicAxyz);
    DConic4d conicB = Flatten (conicBxyz);
    double bigX = DoubleOps::Max (bsiDConic4d_maxAbsUnnormalizedXYZ (conicA), bsiDConic4d_maxAbsUnnormalizedXYZ (conicB));
    double bigW = DoubleOps::Max (bsiDConic4d_maxAbsWeight (conicA), bsiDConic4d_maxAbsWeight (conicB));
    double dW = bsiDConic4d_maxWeightDiff (conicA, conicB);
    double dX = bsiDConic4d_maxUnnormalizedXYZDiff (conicA, conicB);
    double tolW = Angle::SmallAngle () * bigW;
    double tolX = Angle::SmallAngle () * bigX;
    return dX < tolX && dW < tolW;
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
void ProcessArcArc(
        ICurvePrimitiveP curveA, DEllipse3dCR ellipseA,
        ICurvePrimitiveP curveB, DEllipse3dCR ellipseB,
        bool bReverseOrder) override
    {
    DConic4d hConicA, hConicB;
    DPoint4d hPointA[4], hPointB[4];
    double   thetaA[4],  thetaB[4];
    Transform (hConicA,  ellipseA);
    Transform (hConicB,  ellipseB);
    if (SameBasisXY (hConicA, hConicB))
        {
        bvector<DSegment1d> fractionA, fractionB;
        Angle::OverlapWrapableIntervals (hConicA.start, hConicA.sweep, hConicB.start, hConicB.sweep, fractionA, fractionB);
        BeAssert (fractionA.size () == fractionB.size () && fractionA.size () <= 2);
        for (size_t i = 0; i < fractionA.size (); i++)
            {
            CollectPair (curveA, curveB,
                        fractionA[i].GetStart (), fractionA[i].GetEnd (),
                        fractionB[i].GetStart (), fractionB[i].GetEnd (),
                        bReverseOrder
                        );
            }
        return;
        }

    int numIntersection = bsiDConic4d_intersectDConic4dXYW
            (&hConicA, hPointA, thetaA, hPointB, thetaB, &hConicB);
    for (int i = 0; i < numIntersection; i++)
        {
        if (   validArcAngle (thetaA[i], ellipseA)
            && validArcAngle (thetaB[i], ellipseB))
            {
            CollectPair (curveA, curveB,
                    ellipseA.AngleToFraction (thetaA[i]),
                    ellipseB.AngleToFraction (thetaB[i]),
                    bReverseOrder);
            }
        }
    }


/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
void ProcessLineBspline(ICurvePrimitiveP curveA, DSegment3dCR segmentA, ICurvePrimitiveP curveB, bool bReverseOrder) override
    {
    MSBsplineCurveCP pBCurve = curveB->GetProxyBsplineCurveCP ();
    if (NULL == pBCurve)
        return;

    bvector<double> lineFraction, curveFraction;
    // always allow the line to extend, in case just-beyond-the-end retracts towards the end in the refinement step.
    pBCurve->AddLineIntersectionsXY (NULL, &curveFraction, NULL, &lineFraction, segmentA, true, m_pWorldToLocal);
    if (nullptr == curveA->GetBsplineCurveCP () || nullptr == curveB->GetBsplineCurveCP ())
        {
        for (size_t i = 0; i < lineFraction.size (); i++)
            {
            ImprovePlaneCurveCurveTransverseIntersectionXY (*curveA, *curveB, m_pWorldToLocal, lineFraction[i], curveFraction[i]);
            }
        }

    for (size_t i = 0, n = curveFraction.size (); i < n; i++)
        {
        if (!m_extend)
            {
            if (!DoubleOps::IsAlmostIn01 (lineFraction[i]))
                continue;
            if (!DoubleOps::IsAlmostIn01 (curveFraction[i]))
                continue;
            }
        CollectPair (curveA, curveB, lineFraction[i], curveFraction[i], bReverseOrder);
        }
    }


/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
void ProcessArcBspline(ICurvePrimitiveP curveA, DEllipse3dCR ellipseA, ICurvePrimitiveP curveB, bool bReverseOrder) override
    {
    MSBsplineCurveCP pBCurve = curveB->GetProxyBsplineCurveCP ();
    if (NULL == pBCurve)
        return;

    bvector<double> fractionA, fractionB;
    pBCurve->AddArcIntersectionsXY (NULL, &fractionB, NULL, &fractionA, ellipseA, m_extend, m_pWorldToLocal);
    if (nullptr == curveA->GetBsplineCurveCP () || nullptr == curveB->GetBsplineCurveCP ())
        {
        for (size_t i = 0; i < fractionA.size (); i++)
            {
            ImprovePlaneCurveCurveTransverseIntersectionXY (*curveA, *curveB, m_pWorldToLocal, fractionA[i], fractionB[i]);
            }
        }


    for (size_t i = 0, n = fractionA.size (); i < n; i++)
        {
        CollectPair (curveA, curveB, fractionA[i], fractionB[i], bReverseOrder);
        }
    }


/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
void ProcessLinestringBspline(ICurvePrimitiveP curveA, bvector<DPoint3d> const &linestringA,
            ICurvePrimitiveP curveB, bool bReverseOrder) override
    {
    MSBsplineCurveCP pBCurve = curveB->GetProxyBsplineCurveCP ();
    if (NULL == pBCurve)
        return;

    bvector<double> linestringFraction, curveFraction;
    pBCurve->AddLinestringIntersectionsXY (NULL, &curveFraction, NULL, &linestringFraction, linestringA,
          m_extend, m_pWorldToLocal);
    // If there is a proxy, improve the result using the real curve . .
    if (nullptr == curveB->GetBsplineCurveCP ())
        {
        for (size_t i = 0; i < linestringFraction.size (); i++)
            {
            ImprovePlaneCurveCurveTransverseIntersectionXY (*curveA, *curveB, m_pWorldToLocal, linestringFraction[i], curveFraction[i]);
            }
        }
    CollectPairs (curveA, curveB, linestringFraction, curveFraction, bReverseOrder);
    }


/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
void ProcessBsplineBspline(ICurvePrimitiveP curveA, ICurvePrimitiveP curveB, bool bReverseOrder) override
    {
    MSBsplineCurveCP pBCurveA = curveA->GetProxyBsplineCurveCP ();
    MSBsplineCurveCP pBCurveB = curveB->GetProxyBsplineCurveCP ();
    if (NULL == pBCurveA || NULL == pBCurveB)
        return;


    bvector<double> fractionA, fractionB;
    bvector<DSegment1d> overlapA, overlapB;
    pBCurveA->AddCurveIntersectionsXY (NULL, &fractionA, &overlapA, NULL, &fractionB, &overlapB, *pBCurveB, m_pWorldToLocal);
    // If either is a proxy, improve the result using the real curve . .
    if (nullptr == curveA->GetBsplineCurveCP () || nullptr == curveB->GetBsplineCurveCP ())
        {
        for (size_t i = 0; i < fractionA.size (); i++)
            {
            ImprovePlaneCurveCurveTransverseIntersectionXY (*curveA, *curveB, m_pWorldToLocal, fractionA[i], fractionB[i]);
            }
        }
    CollectPairs (curveA, curveB, fractionA, fractionB, overlapA, overlapB, bReverseOrder);
    }

};

/*--------------------------------------------------------------------------------**//**
* ASSUME inputs are partial curves.
* Find single points that are duplicated by other single points or subsumed in adjacent overlaps.
* @bsimethod                                                    EarlinLutz      04/2013
+--------------------------------------------------------------------------------------*/
void PurgeRedundantIntersections (
CurveVectorR intersectionA,
CurveVectorR intersectionB
)
    {
    size_t numA = intersectionA.size ();

// Since use of this is within assert, we must guard the allocation of 'e' the same way to avoid unused variable warnings.
#if !defined (NDEBUG)
    size_t numB = intersectionB.size ();
#endif 

    double fA, fB;
    DPoint3d pointA, pointB;
    assert (numA == numB);
    size_t numAccept = 0;
    for (size_t i = 0; i < numA; i++)
        {
        bool accept = true;
        if (CurveCurve::IsSinglePointPair (intersectionA, intersectionB, i, fA, pointA, fB, pointB))
            {
            for (size_t j = 0; j < numA && accept; j++)
                {
                if (j < numAccept)
                    {
                    if (CurveCurve::IsContained (intersectionA, intersectionB, i, j, true, true))
                        accept = false;
                    }
                else if (j > i)
                    {
                    if (CurveCurve::IsContained (intersectionA, intersectionB, i, j, false, true))
                        accept = false;
                    }
                }
            }
        if (accept)
            {
            if (numAccept < i)
                {
                intersectionA[numAccept] = intersectionA[i];
                intersectionB[numAccept] = intersectionB[i];
                }
            numAccept++;
            }
        }
    if (numAccept < numA)
        {
        intersectionA.resize (numAccept);
        intersectionB.resize (numAccept);
        }
    }

static bool CollectRanges (CurveVectorCR data, bvector<DRange3d> &ranges)
    {
    bool ok = true;
    for(ICurvePrimitivePtr const &curve : data)
        {
        DRange3d range;
        ok &= curve->GetRange (range);
        ranges.push_back (range);
        }
    return ok;
    }
/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
void CurveCurve::IntersectionsXY
(
CurveVectorR intersectionA,
CurveVectorR intersectionB,
CurveVectorR chainA, 
CurveVectorR chainB,
DMatrix4dCP    pWorldToLocal
)
    {
    double tol = 0.0;
    intersectionA.clear ();
    intersectionB.clear ();
    CCIProcessor processor (intersectionA, intersectionB, pWorldToLocal, tol);
    bvector<DRange3d> rangeA, rangeB;
    CollectRanges (chainA, rangeA);
    CollectRanges (chainB, rangeB);
    for (size_t iA = 0, nA = chainA.size (); iA < nA; iA++)
        for (size_t iB = 0, nB = chainB.size (); iB < nB; iB++)
            {
            if (rangeA[iA].IntersectsWith (rangeB[iB], 2))
                processor.Process (chainA[iA].get (), chainB[iB].get ());                        
            }
    PurgeRedundantIntersections (intersectionA, intersectionB);
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
void CurveCurve::SelfIntersectionsXY
(
CurveVectorR intersectionA,
CurveVectorR intersectionB,
CurveVectorR chain, 
DMatrix4dCP    pWorldToLocal
)
    {
    double tol = 0.0;
    intersectionA.clear ();
    intersectionB.clear ();
    CCIProcessor processor (intersectionA, intersectionB, pWorldToLocal, tol);
    bvector<DRange3d> ranges;
    CollectRanges (chain, ranges);
    for (size_t iA = 0, n = chain.size (); iA < n; iA++)
        for (size_t iB = iA + 1; iB < n; iB++)
            {
            if (ranges[iA].IntersectsWith (ranges[iB], 2))
                {
                size_t k0 = intersectionA.size ();
                processor.Process (chain[iA].get (), chain[iB].get ());
                if (iA + 1 == iB)
                    {
                    // purge "head to tail" intersection from consecutive curves
                    for (size_t k = k0; k < intersectionA.size (); k++)
                        {
                        double fractionA, fractionB;
                        DPoint3d pointA, pointB;
                        if (CurveCurve::IsSinglePointPair (intersectionA, intersectionB, k, fractionA, pointA, fractionB, pointB)
                            && DoubleOps::AlmostEqual (fractionA, 1.0)
                            && DoubleOps::AlmostEqual (fractionB, 0.0)
                            )
                            {
                            intersectionA.erase (intersectionA.begin () + k);
                            intersectionB.erase (intersectionB.begin () + k);
                            }
                        }
                    }
                }
            }
    PurgeRedundantIntersections (intersectionA, intersectionB);
    }
/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
void CurveCurve::IntersectionsXY
(
CurveVectorR intersectionA,
CurveVectorR intersectionB,
ICurvePrimitiveR curveA, 
CurveVectorR chainB,
DMatrix4dCP    pWorldToLocal
)
    {
    double tol = 0.0;
    CCIProcessor processor (intersectionA, intersectionB, pWorldToLocal, tol);
    for(ICurvePrimitivePtr &curveB : chainB)
        processor.Process (&curveA, curveB.get ());                        
    PurgeRedundantIntersections (intersectionA, intersectionB);
    }
	
/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
void CurveCurve::IntersectionsXY
(
CurveVectorR intersectionA,
CurveVectorR intersectionB,
ICurvePrimitiveP curveA, 
ICurvePrimitiveP curveB,
DMatrix4dCP    pWorldToLocal,
bool           extend
)
    {
    double tol = 0.0;
    intersectionA.clear ();
    intersectionB.clear ();
    CCIProcessor processor (intersectionA, intersectionB, pWorldToLocal, tol, extend);
    processor.Process (curveA, curveB);
    PurgeRedundantIntersections (intersectionA, intersectionB);
    }

#ifdef CompileCompareStartFraction
static bool cb_comparePartialCurveStartFraction
(
ICurvePrimitivePtr const&dataA,
ICurvePrimitivePtr const&dataB
)
    {
    PartialCurveDetailCP detailA = dataA->GetPartialCurveDetailCP ();
    PartialCurveDetailCP detailB = dataB->GetPartialCurveDetailCP ();
    // should only be called with partial curves, but .....
    if (detailA == NULL && detailB == NULL)
        return false;
    if (detailA == NULL)
        return false;
    if (detailB == NULL)
        return true;
    return detailA->fraction0 < detailB->fraction0;
    }
#endif
struct CCSplitProcessor
{
CurveVectorCR m_splitter;
CurveVectorPtr m_intersectionsA;
CurveVectorPtr m_intersectionsB;
CurveVectorPtr m_results;
bool m_primitivesOnly;
CCIProcessor *m_CCIProcessor;
CurveVector::InOutClassification m_targetClassification;
CCSplitProcessor (
    CurveVectorCR splitter,
    double tol,
    bool primitivesOnly
    )
    : m_splitter (splitter),
      m_intersectionsA (CurveVector::Create(CurveVector::BOUNDARY_TYPE_None)),
      m_intersectionsB (CurveVector::Create(CurveVector::BOUNDARY_TYPE_None)),
      m_results (CurveVector::Create (CurveVector::BOUNDARY_TYPE_None)),
      m_CCIProcessor (new CCIProcessor (*m_intersectionsA, *m_intersectionsB, NULL, tol, false)),
      m_primitivesOnly (primitivesOnly)
    {
    }

~CCSplitProcessor ()
    {
    delete m_CCIProcessor;
    }

void AnnounceResult (CurveVectorR dest, ICurvePrimitiveCR primitive)
    {
    dest.push_back (primitive.Clone ());
    }

void AnnounceResult
    (
    CurveVectorR dest,
    ICurvePrimitiveCR primitive,
    double fractionA, 
    double fractionB
    )
    {
    if (fractionB > fractionA)
        {
        ICurvePrimitivePtr partial = primitive.CloneBetweenFractions (fractionA, fractionB, false);
        if (partial.IsValid ())
          dest.push_back (partial);
        }
    }

// Return deep clone ...
CurveVectorPtr CloneWithSplits (CurveVectorCR source);


void AppendSplits (CurveVectorR dest, ICurvePrimitiveR primitive)
    {
    CurveVectorPtr child = primitive.GetChildCurveVectorP ();
    if (child.IsValid ())
        {
        if (m_primitivesOnly)
            {
            for (size_t i = 0, n = child->size (); i < n; i++)
                AppendSplits (dest, *child->at(i));
            }
        else
            {
            dest.push_back (ICurvePrimitive::CreateChildCurveVector (CloneWithSplits (*child)));
            }
        }
    else
        {
        // The processor knows about the results arrays ...
        m_intersectionsA->clear ();
        m_intersectionsB->clear ();
        m_CCIProcessor->Process (&m_splitter, const_cast <ICurvePrimitiveP>(&primitive));
        DPoint3d testPoint;
        // NOW ... m_intersectionsB has all the intersections along this primitive.
        if (m_intersectionsB->size () == 0)
            {
            // any interior point classifies the whole primitive ...
            if (primitive.FractionToPoint (0.5, testPoint))
                AnnounceResult (dest, primitive);
            }
        else
            {
            bvector<double> splitParams;
            splitParams.push_back (0.0);
            for (size_t i = 0, n = m_intersectionsB->size (); i < n; i++)
                {
                PartialCurveDetailCP detail = m_intersectionsB->at (i).get()->GetPartialCurveDetailCP ();
                if (detail != NULL)
                    {
                    splitParams.push_back (detail->fraction0);
                    splitParams.push_back (detail->fraction1);
                    }
                }
            splitParams.push_back (1.0);
            std::sort (splitParams.begin (), splitParams.end ());
            for (size_t i = 1; i < splitParams.size (); i++)
                AnnounceResult (dest, primitive, splitParams[i-1], splitParams[i]);
            }
        }
    }

};

CurveVectorPtr CCSplitProcessor::CloneWithSplits (CurveVectorCR source)
    {
    CurveVectorPtr result = CurveVector::Create (m_primitivesOnly ? CurveVector::BOUNDARY_TYPE_None : source.GetBoundaryType ());
    for (size_t i = 0; i < source.size (); i++)
        AppendSplits (*result, *source[i]);
    return result;
    }


/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
CurveVectorPtr CurveVector::CloneWithSplits (CurveVectorCR splittersA, bool primitivesOnly)
    {
    double tol = 0.0;
    CCSplitProcessor processor (splittersA, tol, primitivesOnly);
    return processor.CloneWithSplits (*this);
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
void CurveVector::AppendSplitCurvesByRegion (CurveVectorCR region, CurveVectorP insideCollector, CurveVectorP outsideCollector, CurveVectorP onCollector)
    {
    CurveVectorPtr splitCurves = CloneWithSplits (region, true);
    if (!splitCurves.IsValid ())
        return;
    for (size_t i = 0, n = splitCurves->size (); i < n; i++)
        {
        DPoint3d testPoint;
        ICurvePrimitivePtr curve = splitCurves->at(i);
        if (curve->FractionToPoint (0.5, testPoint))
            {
            CurveVector::InOutClassification c = region.PointInOnOutXY (testPoint);
            if (c == CurveVector::INOUT_In)
                {
                if (NULL != insideCollector)
                    insideCollector->push_back (curve);
                }
            else if (c == CurveVector::INOUT_Out)
                {
                if (NULL != outsideCollector)
                    outsideCollector->push_back (curve);
                }
            else if (c == CurveVector::INOUT_On)
                {
                if (NULL != onCollector)
                    onCollector->push_back (curve);                
                }
            }
        }
    }

struct SplitCurvesByPlaneCollector
{
CurveVectorP m_aboveCollector;
CurveVectorP m_belowCollector;
CurveVectorP m_onCollector;
double m_tolerance;
DPlane3d m_plane;
DRay3d   m_ray;

void AddAbove (ICurvePrimitiveCR prim)
  {
  if (NULL != m_aboveCollector)
    m_aboveCollector->push_back (prim.Clone ());
  }

void AddBelow (ICurvePrimitiveCR prim)
  {
  if (NULL != m_belowCollector)
    m_belowCollector->push_back (prim.Clone ());
  }

void AddOn (ICurvePrimitiveCR prim)
  {
  if (NULL != m_onCollector)
    m_onCollector->push_back (prim.Clone ());
  }


void TestAndAdd (ICurvePrimitiveCR prim, double fractionA, double fractionB)
  {
  DPoint3d xyz;
  if (DoubleOps::ClearlyIncreasingFraction (fractionA, fractionB)
    && prim.FractionToPoint (0.5 * (fractionA + fractionB), xyz))
    {
    double h = m_plane.Evaluate (xyz);

    ICurvePrimitivePtr partial = prim.CloneBetweenFractions (fractionA, fractionB, false);
    if (fabs (h) <= m_tolerance)
      AddOn (*partial);
    else if (h > m_tolerance)
      AddAbove (*partial);
    else
      AddBelow (*partial);
    }
  }

bvector<CurveLocationDetailPair> m_cuts;
SplitCurvesByPlaneCollector
    (
    DPlane3d plane,
     CurveVectorP belowCollector, CurveVectorP aboveCollector, CurveVectorP onCollector,
    double tolerance
    )
    : m_aboveCollector (aboveCollector),
    m_belowCollector (belowCollector),
    m_onCollector (onCollector),
    m_tolerance(tolerance),
    m_plane(plane)
    {
    m_ray.origin = m_plane.origin;
    m_plane.normal.Normalize ();
    m_ray.direction = m_plane.normal;
    }

void Process (CurveVectorCR candidates);
void Process (ICurvePrimitiveR candidate)
    {
    CurveVectorPtr child = candidate.GetChildCurveVectorP ();
    if (child.IsValid ())
        {
        Process (*child);
        }
    else
        {
        DRange1d range = candidate.ProjectedParameterRange (m_ray);
        if (range.IsNull ())
            {
            }
        else if (range.low > m_tolerance)
            {
            AddAbove (candidate);
            }
        else if (range.high < -m_tolerance)
            {
            AddBelow (candidate);
            }
        else if (range.low > -m_tolerance && range.high < m_tolerance)
            {
            AddOn (candidate);
            }
        else
            {
            m_cuts.clear ();
            candidate.AppendCurvePlaneIntersections (m_plane, m_cuts);
            // hm.. are they sorted??  I think so....
            double fractionA = 0.0;
            for (size_t i = 0, n = m_cuts.size (); i < n; i++)
                {
                double fractionB0 = m_cuts[i].detailA.fraction;
                double fractionB1 = m_cuts[i].detailB.fraction;
                if (DoubleOps::ClearlyIncreasingFraction (fractionA, fractionB0))
                    TestAndAdd (candidate, fractionA, fractionB0);
                if (DoubleOps::ClearlyIncreasingFraction (fractionB0, fractionB1))
                    AddOn (*candidate.CloneBetweenFractions (fractionA, fractionB0, false));
                fractionA = fractionB1;
                }
            TestAndAdd (candidate, fractionA, 1.0);
            }
        }
    }
};

void SplitCurvesByPlaneCollector::Process (CurveVectorCR candidates)
    {
    for (size_t i = 0, n = candidates.size (); i < n; i++)
        Process (*candidates[i]);
    }


/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
void CurveVector::AppendSplitCurvesByPlane (DPlane3dCR plane, CurveVectorP belowCollector, CurveVectorP aboveCollector, CurveVectorP onCollector)
    {
    SplitCurvesByPlaneCollector collector (plane, belowCollector, aboveCollector, onCollector,
            ResolveTolerance (0.0));
    collector.Process (*this);    
    }


/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
bool CurveCurve::GetPartialCurveDetailPair
(
CurveVectorR intersectionA,
CurveVectorR intersectionB,
size_t i,
PartialCurveDetailR detailA,
PartialCurveDetailR detailB
)
    {
    PartialCurveDetailCP pDetailA;
    PartialCurveDetailCP pDetailB;
    if (i < intersectionA.size ()
        && NULL != (pDetailA = intersectionA.at(i)->GetPartialCurveDetailCP ())
        && i < intersectionB.size ()
        && NULL != (pDetailB = intersectionB.at(i)->GetPartialCurveDetailCP ())
        )
        {
        detailA = *pDetailA;
        detailB = *pDetailB;
        return true;
        }
    return false;
    }


/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
bool CurveCurve::IsSinglePointPair
(
CurveVectorR intersectionA,
CurveVectorR intersectionB,
size_t i,
double &fractionA,
DPoint3dR pointA,
double &fractionB,
DPoint3dR pointB
)
    {
    PartialCurveDetail detailA, detailB;
    if (!GetPartialCurveDetailPair (intersectionA, intersectionB, i, detailA, detailB))
        return false;
    if (detailA.fraction0 != detailA.fraction1)
        return false;
    if (detailB.fraction0 != detailB.fraction1)
        return false;
    if (detailA.parentCurve.IsNull() || detailB.parentCurve.IsNull())
        return false;
    fractionA = detailA.fraction0;
    fractionB = detailB.fraction0;
    
    return detailA.parentCurve->FractionToPoint (fractionA, pointA)
            && detailB.parentCurve->FractionToPoint (fractionB, pointB);
    }

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
bool CurveCurve::IsSinglePointPair
(
CurveVectorR intersectionA,
CurveVectorR intersectionB,
size_t i,
DRay3dR unitTangentA,
DRay3dR unitTangentB
)
    {
    PartialCurveDetail detailA, detailB;
    if (!GetPartialCurveDetailPair(intersectionA, intersectionB, i, detailA, detailB))
        return false;
    if (detailA.fraction0 != detailA.fraction1)
        return false;
    if (detailB.fraction0 != detailB.fraction1)
        return false;
    if (detailA.parentCurve.IsNull() || detailB.parentCurve.IsNull())
        return false;

    auto validatedTangentA = detailA.parentCurve->FractionToPointAndUnitTangent(detailA.fraction0);
    auto validatedTangentB = detailB.parentCurve->FractionToPointAndUnitTangent(detailB.fraction0);

    if (validatedTangentA.IsValid() && validatedTangentB.IsValid())
        {
        unitTangentA = validatedTangentA;
        unitTangentB = validatedTangentB;
        return true;
        }
    return false;
    }
/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
bool CurveCurve::IsSinglePointPair
(
CurveVectorR intersectionA,
CurveVectorR intersectionB,
size_t i,
CurveLocationDetailR detailA,
CurveLocationDetailR detailB
)
    {
    PartialCurveDetail partialCurveDetailA, partialCurveDetailB;
    if (!GetPartialCurveDetailPair (intersectionA, intersectionB, i, partialCurveDetailA, partialCurveDetailB))
        return false;
    if (partialCurveDetailA.fraction0 != partialCurveDetailA.fraction1)
        return false;
    if (partialCurveDetailB.fraction0 != partialCurveDetailB.fraction1)
        return false;
    if (partialCurveDetailA.parentCurve.IsNull() || partialCurveDetailB.parentCurve.IsNull())
        return false;
    double fractionA = partialCurveDetailA.fraction0;
    double fractionB = partialCurveDetailB.fraction0;
    
    return partialCurveDetailA.parentCurve->FractionToPoint (fractionA, detailA)
            && partialCurveDetailB.parentCurve->FractionToPoint (fractionB, detailB);
    }
	
bool FractionsContained (PartialCurveDetailCR detailA, PartialCurveDetailCR detailB)
  {
  double a0 = detailA.fraction0;
  double a1 = detailA.fraction1;
  double b0 = detailB.fraction0;
  double b1 = detailB.fraction1;
  if (b0 <= b1)
    return TolerancedFractionLE (b0, a0)
        && TolerancedFractionLE (a0, b1)
        && TolerancedFractionLE (b0, a1)
        && TolerancedFractionLE (a1, b1);
  else
    return TolerancedFractionLE (b1, a0)
        && TolerancedFractionLE (a0, b0)
        && TolerancedFractionLE (b1, a1)
        && TolerancedFractionLE (a1, b0);
  }
/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
bool CurveCurve::IsContained
(
CurveVectorR intersectionA,
CurveVectorR intersectionB,
size_t i,
size_t j,
bool considerSinglePoints,
bool considerIntervals
)
    {
    PartialCurveDetail detailAi, detailBi;
    PartialCurveDetail detailAj, detailBj;
    if (!GetPartialCurveDetailPair (intersectionA, intersectionB, i, detailAi, detailBi))
        return false;
    if (!GetPartialCurveDetailPair (intersectionA, intersectionB, j, detailAj, detailBj))
        return false;
    if (detailAi.parentCurve.IsNull() || detailBi.parentCurve.IsNull())
        return false;
    if (detailAj.parentCurve.IsNull() || detailBj.parentCurve.IsNull())
        return false;
    bool jIsPoint = detailBj.IsSingleFraction () && detailBi.IsSingleFraction ();
    if (jIsPoint)
        {
        if (!considerSinglePoints)
            return false;
        }
    else
        {
        if (!considerIntervals)
            return false;
        }
    // same curve ...
    if (detailAi.parentCurve.get () == detailAj.parentCurve.get ()
        && detailBi.parentCurve.get () == detailBj.parentCurve.get ()
        )
        {
        return FractionsContained (detailAi, detailAj) && FractionsContained (detailBi, detailBj);
        }
    // Hmmm  different curves.   Should look for end-start matching?
    return false;
    }



END_BENTLEY_GEOMETRY_NAMESPACE
