/*--------------------------------------------------------------------------------------+
|
|     $Source: geom/src/CurvePrimitive/CCFillet.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <bsibasegeomPCH.h>

#include "CurveCurveProcessor.h"
BEGIN_BENTLEY_GEOMETRY_NAMESPACE

/*--------------------------------------------------------------------------------**//**
* @bsistruct                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
struct CCFilletProcessor : public CurveCurveProcessor
{
double m_radius;
bvector<CurveCurve::FilletDetail> m_arcs;
CCFilletProcessor (double radius, bool extend = false) :
    CurveCurveProcessor (NULL, 0.0),
    m_radius(radius)
    {
    SetExtend (extend);
    }

void GrabArcs (bvector<CurveCurve::FilletDetail> &arcs)
    {
    arcs.clear ();
    arcs.swap (m_arcs);
    }
    
CurveLocationDetail FinalizedDetail (CurveLocationDetailCR detail0, double fraction, DPoint3dCR xyz)
    {
    CurveLocationDetail detail = detail0;
    detail.fraction = fraction;
    detail.point = xyz;
    if (detail.numComponent > 0)
        {
        double f0 = detail.componentIndex / (double) detail.numComponent;
        double f1 = (detail.componentIndex + 1) / (double) detail.numComponent;
        detail.fraction = DoubleOps::Interpolate (f0, fraction, f1);
        }
    return detail;
    }

void GenerateFilletArc
(
DPoint3dCR filletCenter,
CurveLocationDetailCR detailA,  // contains start point!!
CurveLocationDetailCR detailB,  // contains end point !!
DVec3d     planeNormal, // ASSUMED NORMALIZED AND PERPENDICULAR to vectors to start, end.
bool       bReverseOrder
)
    {
    DPoint3d startPoint = detailA.point;
    DPoint3d endPoint = detailB.point;
    if (bReverseOrder)
        {
        GenerateFilletArc (filletCenter, detailB, detailA, planeNormal, false);
        }
    else
        {
        DEllipse3d ellipse;
        DVec3d endVector;
        ellipse.center = filletCenter;
        ellipse.vector0.DifferenceOf (startPoint, filletCenter);
        ellipse.vector90.CrossProduct (ellipse.vector0, planeNormal);
        ellipse.start = 0.0;
        endVector.DifferenceOf (endPoint, filletCenter);
        ellipse.sweep = ellipse.vector0.AngleTo (endVector);
        if (fabs (ellipse.sweep) < Angle::SmallAngle ())
            return;
        if (endVector.DotProduct (ellipse.vector90) < 0.0)
            ellipse.sweep = -ellipse.sweep;
        CurveCurve::FilletDetail detail;
        detail.detailA = detailA;
        detail.detailB = detailB;
        detail.arc = ellipse;
        m_arcs.push_back (detail);
        }
    }
bool validArcAngle (DEllipse3d ellipse, double theta)
    {
    if (m_extend)
        return true;
    return ellipse.IsAngleInSweep (theta);
    }


bool validFraction (CurveLocationDetailCR detail, double fraction)
    {
    bool extensible0 = true;
    bool extensible1 = true;
    if (detail.componentIndex != 0)
        extensible0 = false;
    if (detail.componentIndex + 1 != detail.numComponent)
        extensible1 = false;
    if (!m_extend)
        extensible0 = extensible1 = false;
    if (fraction < 0.0 && !extensible0)
        return false;
    if (fraction > 1.0 && !extensible1)
        return false;
    return true;
    }

/*--------------------------------------------------------------------------------**//**
On input, detailA0 and detailB0 identify curve and component index but have no point data.
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
void ProcessLineLine(
CurveLocationDetailCR detailA0, DSegment3dCR segmentA,
CurveLocationDetailCR detailB0, DSegment3dCR segmentB,
bool bReverseOrder
)
    {
#ifdef SpiralFillets
    if (ProcessSegmentSegment_withSpirals (segmentA, segmentB, bReverseOrder))
        return;
#endif
    DVec3d tangentA, tangentB, planeNormal;
    DVec3d perpA, perpB;
    CurveLocationDetail detailA, detailB;

    tangentA.DifferenceOf (segmentA.point[1], segmentA.point[0]);
    tangentB.DifferenceOf (segmentB.point[1], segmentB.point[0]);
    if (tangentA.IsParallelTo (tangentB))
        return;

    planeNormal.NormalizedCrossProduct (tangentA, tangentB);
    perpA.SizedCrossProduct (planeNormal, tangentA, m_radius);
    perpB.SizedCrossProduct (planeNormal, tangentB, m_radius);
    for (double signA = -1.0; signA < 3.0; signA += 2.0)
        {
        DRay3d offsetA;
        offsetA.origin.SumOf (segmentA.point[0], perpA, signA);
        offsetA.direction = tangentA;
        for (double signB = -1.0; signB < 3.0; signB += 2.0)
            {
            DRay3d offsetB;
            offsetB.origin.SumOf (segmentB.point[0], perpB, signB);
            offsetB.direction = tangentB;
            double paramA, paramB;
            DPoint3d centerPointA, centerPointB;
            DPoint3d tangentPointA, tangentPointB;
            if (DRay3d::ClosestApproachUnboundedRayUnboundedRay
                                (
                                paramA, paramB,
                                centerPointA, centerPointB,
                                offsetA, offsetB
                                )
                && validFraction (detailA0, paramA)
                && validFraction (detailB0, paramB)
               )
                {
                tangentPointA.SumOf (centerPointA, perpA, -signA);
                tangentPointB.SumOf (centerPointB, perpB, -signB);
                GenerateFilletArc (centerPointA, 
                        FinalizedDetail (detailA0, paramA, tangentPointA),
                        FinalizedDetail (detailB0, paramB, tangentPointB),
                        planeNormal, bReverseOrder);
                }
            }
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
    CurveLocationDetail detailA (curveA);
    CurveLocationDetail detailB (curveB);
    ProcessLineLine (detailA, segmentA, detailB, segmentB, bReverseOrder);
    }    

/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
void ProcessLineLinestring(
        ICurvePrimitiveP curveA, DSegment3dCR segmentA,
        ICurvePrimitiveP curveB, bvector<DPoint3d> const &linestringB,
        bool bReverseOrder) override
    {
    size_t numPointsB = linestringB.size ();
    CurveLocationDetail detailA (curveA);
    CurveLocationDetail detailB (curveB, numPointsB - 1);
    for (size_t i = 1; i < numPointsB; i++)
        {
        DSegment3d segmentB = DSegment3d::From (linestringB[i-1], linestringB[i]);
        detailB.componentIndex = i - 1;
        ProcessLineLine (
                    detailA, segmentA,
                    detailB, segmentB, bReverseOrder);
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
    size_t numPointsA = linestringA.size ();
    size_t numPointsB = linestringB.size ();
    CurveLocationDetail detailA (curveA, numPointsA - 1);
    CurveLocationDetail detailB (curveB, numPointsB - 1);

    for (size_t i = 1; i < numPointsA; i++)
        {
        DSegment3d segmentA = DSegment3d::From (linestringA[i-1], linestringA[i]);
        detailA.componentIndex = i - 1;
        for (size_t j = 1; j < numPointsB; j++)
            {
            DSegment3d segmentB = DSegment3d::From (linestringB[j-1], linestringB[j]);
            detailB.componentIndex = j - 1;
            ProcessLineLine (
                        detailA, segmentA,
                        detailB, segmentB,
                        bReverseOrder);
            }
        }
    }

void ProcessLinestringInternal(ICurvePrimitiveP curveA, bvector<DPoint3d> const &linestringA)
    {
    size_t numPointsA = linestringA.size ();
    CurveLocationDetail detailA (curveA, numPointsA - 1);
    CurveLocationDetail detailB (curveA, numPointsA - 1);
    
    for (size_t i = 1; i < numPointsA; i++)
        {
        DSegment3d segmentA = DSegment3d::From (linestringA[i-1], linestringA[i]);
        detailA.componentIndex = i - 1;
        for (size_t j = i + 1; j < numPointsA; j++)
            {
            DSegment3d segmentB = DSegment3d::From (linestringA[j-1], linestringA[j]);
            detailB.componentIndex = j - 1;
            ProcessLineLine (
                        detailA, segmentA,
                        detailB, segmentB, false);
            }
        }
    }



/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
void ProcessLineArc(
        CurveLocationDetailCR detailA, DSegment3dCR segmentA,
        CurveLocationDetailCR detailB, DEllipse3dCR ellipseB,
        bool bReverseOrder)
    {
    
    if (!ellipseB.IsCircular ())
        return;
    BentleyApi::Transform localToWorld, worldToLocal;
    double r0, r1, theta0, sweep;
    ellipseB.GetScaledTransforms (localToWorld, r0, r1, theta0, sweep, worldToLocal);

    DSegment3d localSegment;
    DVec3d planeNormal;
    localToWorld.GetMatrixColumn (planeNormal, 2);
    worldToLocal.Multiply (localSegment, segmentA);
    DVec3d localTangent = DVec3d::FromStartEnd (localSegment.point[0], localSegment.point[1]);
    DVec3d localPerp;
    if (!localPerp.UnitPerpendicularXY (localTangent))
        return;
    double signedOffsetDistance[2] = {m_radius, -m_radius};
    DEllipse3d localEllipse = DEllipse3d::From (0,0,0, r0, 0,0, 0, r0, 0, theta0, sweep);
    DSegment3d offsetSegment[2];
    DEllipse3d offsetEllipse[2];
    for (int segmentIndex = 0; segmentIndex < 2; segmentIndex++)
        for (int pointIndex = 0; pointIndex < 2; pointIndex++)
            {
            offsetSegment[segmentIndex].point[pointIndex].SumOf (
                            localSegment.point[pointIndex], localPerp, signedOffsetDistance[segmentIndex]);
            }
    for (int ellipseIndex = 0; ellipseIndex < 2; ellipseIndex++)
        {
        double r = r0 + signedOffsetDistance[ellipseIndex];
        offsetEllipse[ellipseIndex].Init (0,0,0, r, 0,0, 0,r,0, theta0, sweep);
        }

    for (int ellipseIndex = 0; ellipseIndex < 2; ellipseIndex++)
        {
        for (int segmentIndex = 0; segmentIndex < 2; segmentIndex++)
            {
            DPoint3d localCenter  [6];
            double   lineFraction [6];
            double   ellipseAngle [6];
            DPoint3d cosineSineZ  [6];
            int numIntersection = offsetEllipse[ellipseIndex].IntersectXYLine (localCenter, lineFraction, cosineSineZ, ellipseAngle,
                            offsetSegment[segmentIndex].point[0], offsetSegment[segmentIndex].point[1]);

            for (int i = 0; i < numIntersection; i++)
                {
                // Fraction on offset segment is identical to fraction on original.
                // Is angle on the offset ellipse the same as angle on original?  Probably, but the transform reduction
                //   could decide to change parameterization.   So map to fraction.
                double fractionB = Angle::NormalizeToSweep (ellipseAngle[i], theta0, sweep);
                double angleB = ellipseB.FractionToAngle (fractionB);
                if (validFraction (detailA, lineFraction[i])
                    && validArcAngle (ellipseB, angleB)
                    )
                    {
                    DPoint3d localPointA, localPointB;
                    localSegment.FractionParameterToPoint (localPointA, lineFraction[i]);
                    localEllipse.Evaluate (localPointB, ellipseAngle[i]);
                    localPointA.z = localPointB.z = localCenter[i].z;
                    DPoint3d worldPointA, worldPointB, worldCenter;
                    localToWorld.Multiply (worldPointA, localPointA);
                    localToWorld.Multiply (worldPointB, localPointB);
                    localToWorld.Multiply (worldCenter, localCenter[i]);
                    GenerateFilletArc (
                         worldCenter, 
                         FinalizedDetail (detailA, lineFraction[i], worldPointA),
                         FinalizedDetail (detailB, fractionB, worldPointB),
                         planeNormal, bReverseOrder);
                    }
                }            
            }
        }
    }

void ProcessLineArc(
        ICurvePrimitiveP curveA, DSegment3dCR segmentA,
        ICurvePrimitiveP curveB, DEllipse3dCR ellipseB,
        bool bReverseOrder) override
    {
    CurveLocationDetail detailA (curveA, 1);
    CurveLocationDetail detailB (curveB, 1);
    ProcessLineArc (detailA, segmentA, detailB, ellipseB, bReverseOrder);
    }


/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
void ProcessLinestringArc(
        ICurvePrimitiveP curveA, bvector<DPoint3d> const &linestringA,
        ICurvePrimitiveP curveB, DEllipse3dCR ellipseB,
        bool bReverseOrder) override
    {
    size_t nXYZA = linestringA.size ();
    CurveLocationDetail detailA (curveA, nXYZA - 1);
    CurveLocationDetail detailB (curveB, 1);
    for (size_t iA = 1;
            iA < nXYZA;
            iA++
        )
        {
        DSegment3d segment = DSegment3d::From (linestringA[iA-1], linestringA[iA]);
        detailA.componentIndex = iA - 1;
        ProcessLineArc (detailA, segment,
                        detailB, ellipseB, bReverseOrder);
        }
    }


/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
void ProcessArcArc(
        ICurvePrimitiveP curveA, DEllipse3dCR ellipseA,
        ICurvePrimitiveP curveB, DEllipse3dCR ellipseB,
        bool bReverseOrder) override
    {
    CurveLocationDetail detailA (curveA), detailB (curveB);
    double rA0, rA90, rB0, rB90;
    double thetaA0, thetaB0, sweepA, sweepB;
    BentleyApi::Transform localToWorldA, localToWorldB;
    BentleyApi::Transform worldToLocalA, worldToLocalB;
    ellipseA.GetScaledTransforms (localToWorldA, rA0, rA90, thetaA0, sweepA, worldToLocalA);
    ellipseB.GetScaledTransforms (localToWorldB, rB0, rB90, thetaB0, sweepB, worldToLocalB);
    if (   !DoubleOps::AlmostEqual (rA0, rA90)
       ||  !DoubleOps::AlmostEqual (rB0, rB90)
       )
       return;
       
    // hm... should choose larger ellipse as working system.
    DVec3d normalA, normalB;
    localToWorldA.GetMatrixColumn (normalA, 2);
    localToWorldB.GetMatrixColumn (normalB, 2);
    
    if (!normalA.IsParallelTo (normalB))
        return;
        
    // build both offset circles ...
    DEllipse3d ellipseBinA;
    worldToLocalA.Multiply (ellipseBinA, ellipseB);
    DVec3d unitB0, unitB90;
    unitB0.Normalize (ellipseBinA.vector0);
    unitB90.Normalize(ellipseBinA.vector90);
    double signedOffset[2] = {m_radius, -m_radius};
    DEllipse3d offsetA[2], offsetB[2];
    for (int i = 0; i < 2; i++)
        {
        double dr = signedOffset[i];
        offsetA[i].Init
            (
            0,0,0,
            rA0+dr, 0, 0,
            0, rA0+dr, 0,
            thetaA0, sweepA
            );

        offsetB[i] = ellipseBinA;
        offsetB[i].vector0.Scale (unitB0, rB0 + dr);
        offsetB[i].vector90.Scale (unitB90, rB0 + dr);
        }
    
    for (int i = 0; i < 2; i++)
        {
        for (int j = 0; j < 2; j++)
            {
            DPoint3d uvw[6];
            DPoint3d paramA[6];
            DPoint3d paramB[6];
            
            int numUVW = offsetA[i].IntersectXYDEllipse3d (uvw, paramA, paramB, offsetB[j]);
            for (int k = 0; k < numUVW; k++)
                {
                // fraction parameters of offset intersection are fraction parameters of the tangency points 
                //  on the original circles.
                double thetaA = atan2 (paramA[k].y, paramA[k].x);
                double thetaB = atan2 (paramB[k].y, paramB[k].x);
                double fractionA = ellipseA.AngleToFraction (thetaA);
                double fractionB = ellipseB.AngleToFraction (thetaB);
                if (   validArcAngle (ellipseA, thetaA)
                    && validArcAngle (ellipseB, thetaB)
                    )
                    {
                    DPoint3d center, tangencyA, tangencyB;
                    localToWorldA.Multiply (center, uvw[k]);
                    ellipseA.FractionParameterToPoint (tangencyA, fractionA);
                    ellipseB.FractionParameterToPoint (tangencyB, fractionB);
                    GenerateFilletArc (center,
                                FinalizedDetail (detailA, fractionA, tangencyA),
                                FinalizedDetail (detailB, fractionB, tangencyB),
                                normalA, bReverseOrder);
                    }
                }
            }
        }
    }

#ifdef abc
/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
void ProcessLineBspline(ICurvePrimitiveP curveA, DSegment3dCR segmentA, ICurvePrimitiveP curveB, bool bReverseOrder) override
    {
    MSBsplineCurveCP pBCurve = curveB->GetProxyBsplineCurveCP ();
    if (NULL == pBCurve)
        return;

    bvector<double> lineFraction, curveFraction;
    pBCurve->AddLineIntersectionsXY (NULL, &curveFraction, NULL, &lineFraction, segmentA, m_extend, m_pWorldToLocal);

    for (size_t i = 0, n = curveFraction.size (); i < n; i++)
        {
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
    pBCurve->AddLinestringIntersectionsXY (NULL, &curveFraction, NULL, &linestringFraction, linestringA, m_pWorldToLocal);
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
    pBCurveA->AddCurveIntersectionsXY (NULL, &fractionA, NULL, &fractionB, *pBCurveB, m_pWorldToLocal);
    CollectPairs (curveA, curveB, fractionA, fractionB, bReverseOrder);
    }
#endif
};


/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      04/2012
+--------------------------------------------------------------------------------------*/
void CurveCurve::CollectFilletArcs
(
CurveVectorCR chainA, 
CurveVectorCR chainB,
double radius, 
bool extend,
bvector<CurveCurve::FilletDetail> &arcs
)
    {
    CCFilletProcessor processor (radius, extend);
    bool sameSource = &chainA == &chainB;
    arcs.clear ();
    if (!sameSource)
        {
        for (size_t iA = 0, nA = chainA.size (); iA < nA; iA++)
            {
            for (size_t iB = 0, nB = chainB.size (); iB < nB; iB++)
                processor.Process (chainA[iA].get (), chainB[iB].get ());                        
            }
        }
    else
        {
        for (size_t iA = 0, nA = chainA.size (); iA < nA; iA++)
            {
            bvector<DPoint3d> const *linestring = chainA[iA]->GetLineStringCP ();
            // special case interactions within the linestring
            if (NULL != linestring)
                {
                processor.ProcessLinestringInternal (chainA[iA].get (), *linestring);
                }
            // all interactions with successors 
            for (size_t iB = iA + 1, nB = chainB.size (); iB < nB; iB++)
                processor.Process (chainA[iA].get (), chainB[iB].get ());                        
            }
        }
    processor.GrabArcs (arcs);        
    }
    
/*--------------------------------------------------------------------------------**//**
* @bsimethod                                                    EarlinLutz      08/2012
+--------------------------------------------------------------------------------------*/
void CurveCurve::CollectFilletArcs
(
ICurvePrimitiveR curveA, 
ICurvePrimitiveR curveB,
double radius,
bool extend,
bvector<CurveCurve::FilletDetail> &arcs
)
    {
    arcs.clear ();
    CCFilletProcessor processor (radius, extend);
    bvector<DPoint3d> const *linestring = curveA.GetLineStringCP ();
    // special case interactions within the linestring
    if (&curveA == &curveB && NULL != linestring)
        {
        processor.ProcessLinestringInternal (&curveA, *linestring);
        }
    else
        processor.Process (&curveA, &curveB);
    processor.GrabArcs (arcs);
    }


CurveVectorPtr CurveVector::CloneWithFillets (double radius) const
    {
    CurveVector::BoundaryType boundaryType = GetBoundaryType ();
    if (size () < 1)
        return NULL;
    if (boundaryType == CurveVector::BOUNDARY_TYPE_Outer
        || boundaryType == CurveVector::BOUNDARY_TYPE_Inner
        || boundaryType == CurveVector::BOUNDARY_TYPE_Open
        )
        {
        if (CountPrimitivesOfType (ICurvePrimitive::CURVE_PRIMITIVE_TYPE_LineString) > 0)
            {
            CurveVectorPtr source1 = CloneWithExplodedLinestrings ();
            return source1->CloneWithFillets (radius);
            }
        else
            {
            CurveVectorPtr result = CurveVector::Create (GetBoundaryType ());
            CCFilletProcessor processor (radius, false);
            ICurvePrimitivePtr primitiveA =  at(0);
            ICurvePrimitivePtr primitiveB;
            bvector<CurveCurve::FilletDetail> arcs;
            ICurvePrimitivePtr wrappedPrimitive = NULL;
            bool doWrap = IsClosedPath ();
            size_t n = size ();
            size_t nLoopStep = doWrap ? n + 1 : n;
            for (size_t i = 1; i < nLoopStep; i++)
                {
                if (!primitiveA.IsValid ())
                    break;
                if (i == n)
                    primitiveB = wrappedPrimitive;
                else
                    primitiveB = at(i);
                if (!primitiveB.IsValid ())
                    break;
                processor.Process (primitiveA.get (), primitiveB.get ());
                processor.GrabArcs (arcs);
                if (arcs.size () == 1)
                    {
                    CurveCurve::FilletDetail detail = arcs[0];
                    ICurvePrimitivePtr trimA = primitiveA->CloneBetweenFractions (0.0, detail.detailA.fraction, true);
                    ICurvePrimitivePtr trimB = primitiveB->CloneBetweenFractions (detail.detailB.fraction, 1.0, true);
                    if (doWrap && i == 1)
                        wrappedPrimitive = trimA;   // This will come back to get trimmed from the other end !!
                    else
                        result->push_back (trimA);
                    result->push_back (ICurvePrimitive::CreateArc (detail.arc));
                    primitiveA = trimB;
                    }
                else
                    {
                    if (doWrap && i == 1)
                        wrappedPrimitive = primitiveA;   // This will come back to get trimmed from the other end !!
                    else
                        result->push_back (primitiveA->Clone ());
                    if (doWrap && i == 1)
                        wrappedPrimitive = primitiveA;
                    primitiveA = primitiveB;
                    }
                }

            if (primitiveA.IsValid ())
                result->push_back (primitiveA->Clone ());

            return result;
            }
        }
    return CurveVectorPtr (NULL);
    }

struct CircleConstructionFunction_PointTangentPointXY;
typedef ValidatedValue<CircleConstructionFunction_PointTangentPointXY> ValidatedCircleConstructionFunction_PointTangentPointXY;

struct CircleConstructionFunction_PointTangentPointXY
    {
    // Circle construction for three conditions A, B, C:
    // pointA = passthrough point
    // tangentB = tangent at point A
    // pointC = passthrough point.
    //
    // put pointA at the origin, tangentB in Y direction.
    // now the circle center is at x position (r,0), where r is a signed radius.
    private:
        Transform m_worldToLocal;
        Transform m_localToWorld;
public:
        CircleConstructionFunction_PointTangentPointXY (TransformCR localToWorld, TransformCR worldToLocal)
            : m_worldToLocal (worldToLocal), m_localToWorld (localToWorld)
            {
            }

        CircleConstructionFunction_PointTangentPointXY ()
            : m_worldToLocal (Transform::FromIdentity ()),
              m_localToWorld (Transform::FromIdentity ())
            {
            }


        ValidatedDouble LocalCircleSignedRadius (double cx, double cy)
            {
            // In local coordinates ...
            // pass through origin.
            // tangent to y axis at origin -- center is on x axis at (r,0)
            // cx,cy = passthrough point
            // (r-cx)^2 + cy^2 = r^2
            //  -2 cx r + cx^2 + cy ^2 = 0
            double rrC = cx * cx + cy * cy;
            return DoubleOps::ValidatedDivide (rrC, 2.0 * cx);
            }
        // return (x,y) = (r, dr)       with both signed
        // i.e. local center is at (r,0)
        //      local derivative wrt the parameter that controls pointC is (dr, 0)
        ValidatedDVec2d LocalCircleSignedRadiusAndDerivative (double cx, double cy, double tx, double ty)
            {
            // In local coordinates ...
            // pass through origin.
            // tangent to y axis at origin -- center is on x axis at (r,0)
            // cx,cy = passthrough point
            // (r-cx)^2 + cy^2 = r^2
            //  -2 cx r + cx^2 + cy ^2 = 0
            double f = cx * cx + cy * cy;
            double df = 2.0 * (cx * tx + cy * ty);
            double g = 2.0 * cx;
            double dg = 2.0 * tx;
            return DoubleOps::ValidatedDivideAndDifferentiate (f, df, g, dg);
            }

        ValidatedDPoint3d WorldCircleCenter (DPoint3dCR worldPoint)
            {
            DPoint3d localPointC = m_worldToLocal * worldPoint;
            auto radius = LocalCircleSignedRadius (localPointC.x, localPointC.y);
            if (radius.IsValid ())
                return m_localToWorld * DPoint3d::From (radius.Value (), 0.0, 0.0);
            return ValidatedDPoint3d (worldPoint, false);
            }

        ValidatedDRay3d WorldCircleCenterAndDerivative (DPoint3dCR worldPoint, DVec3dCR worldTangent)
            {
            DPoint3d localPointC = m_worldToLocal * worldPoint;
            DVec3d   localTangentC = m_worldToLocal * worldTangent;
            auto rDr = LocalCircleSignedRadiusAndDerivative (localPointC.x, localPointC.y, localTangentC.x, localTangentC.y);
            if (rDr.IsValid ())
                {
                DPoint3d worldCenter = m_localToWorld * DPoint3d::From (rDr.Value ().x, 0.0, 0.0);
                DVec3d worldDerivative = m_localToWorld * DVec3d::From (rDr.Value ().y, 0.0, 0.0);
                return ValidatedDRay3d (DRay3d::FromOriginAndVector (worldCenter, worldDerivative), true);
                }
            return ValidatedDRay3d ();
            }


        static ValidatedCircleConstructionFunction_PointTangentPointXY Create (DPoint3dCR pointA, DVec3dCR tangentA)
            {
            auto vectorY = DVec3d::From (tangentA.x, tangentA.y, 0.0).ValidatedNormalize ();
            if (vectorY.IsValid ())
                {
                DVec3d vectorX = DVec3d::From (vectorY.Value ().y, -vectorY.Value().x, 0.0);
                Transform localToWorld = Transform::FromOriginAndVectors
                    (
                    pointA,
                    vectorX,
                    vectorY,
                    DVec3d::From (1, 0, 0)
                    );
                Transform worldToLocal;
                worldToLocal.InvertRigidBodyTransformation (localToWorld);
                return ValidatedCircleConstructionFunction_PointTangentPointXY (
                        CircleConstructionFunction_PointTangentPointXY (localToWorld, worldToLocal));
                }
            return ValidatedCircleConstructionFunction_PointTangentPointXY ();
            }
    };

struct PointTangentCurveTangentNewtonFunction : FunctionRToR, FunctionRToRD
{
ICurvePrimitiveCP m_curve;
CircleConstructionFunction_PointTangentPointXY &m_functions;
PointTangentCurveTangentNewtonFunction (CircleConstructionFunction_PointTangentPointXY &functions)
    : m_functions (functions)
    {
    }
void SetCurve (ICurvePrimitiveCP curve) {m_curve = curve;}
ValidatedDouble EvaluateRToR (double fraction) override 
    {
    DPoint3d xyz;
    DVec3d tangent;
    if (m_curve->FractionToPoint (fraction, xyz, tangent))
        {
        ValidatedDPoint3d center = m_functions.WorldCircleCenter (xyz);
        if (center.IsValid ())
            return ValidatedDouble (tangent.DotProduct (xyz - center));
        }
    return ValidatedDouble (0.0, false);
    }

bool EvaluateRToRD (double fraction, double &f, double &dfdu) override 
    {
    DPoint3d xyz;
    DVec3d tangent, skew;
    if (m_curve->FractionToPoint (fraction, xyz, tangent, skew))
        {

        ValidatedDRay3d center = m_functions.WorldCircleCenterAndDerivative (xyz, tangent);
        if (center.IsValid ())
            {
            DVec3d radialVector = xyz - center.Value ().origin;
            f = tangent.DotProduct (radialVector);
            dfdu = skew.DotProduct (radialVector) + tangent.DotProduct (tangent - center.Value ().direction);
            return true;
            }
        }
    return false;
    }


};

void InsertProjectionPoints (DPoint3dDoubleUVCurveArrays &strokes, DPoint3dCR spacePoint)
    {
    for (size_t i = 0; i + 1 < strokes.m_xyz.size (); i++)    // Note -- use index, not iterator -- the array can grow during the processing.
        {
        if (strokes.m_curve [i] == strokes.m_curve [i+1])
            {
            DRay3d ray = DRay3d::FromOriginAndTarget  (strokes.m_xyz[i], strokes.m_xyz[i+1]);
            double strokeFraction, curveFraction;
            DPoint3d xyz;
            if (ray.ProjectPointBounded (xyz, strokeFraction, spacePoint))
                {
                DVec3d vector = DVec3d::FromInterpolate (strokes.m_vectorU[i], strokeFraction, strokes.m_vectorU[i+1]);
                curveFraction = DoubleOps::Interpolate (strokes.m_f[i], strokeFraction, strokes.m_f[i+1]);
                strokes.Insert (i+1, xyz, curveFraction, vector, strokes.m_curve[i]);
                i++;
                }
            }
        }
    }

void InsertProjectionPoints (bvector< DPoint3dDoubleUVCurveArrays> &allStrokes, DPoint3dCR spacePoint)
    {
    for (auto &strokes : allStrokes)
        InsertProjectionPoints (strokes, spacePoint);
    }

void CurveCurve::ConstructArcs_PointTangentCurveTangent
(
bvector<FilletDetail> &arcs,
DPoint3dCR pointA,
DVec3dCR tangentA,
CurveVectorCR curves
)
    {
    arcs.clear ();
    bvector< DPoint3dDoubleUVCurveArrays> strokes;
    curves.AddStrokePoints (strokes);        // default curve stroking.
    InsertProjectionPoints (strokes, pointA);
    for (auto &path : strokes)
        ConstructArcs_Add_PointTangentCurveTangent (arcs, pointA, tangentA, path);
    }

void CurveCurve::ConstructArcs_PointTangentCurveTangent
(
bvector<FilletDetail> &arcs,
DPoint3dCR pointA,
DVec3dCR tangentA,
ICurvePrimitiveCR curve
)
    {
    arcs.clear ();
    DPoint3dDoubleUVCurveArrays strokes;
    auto options = IFacetOptions::CreateForCurves ();
    InsertProjectionPoints (strokes, pointA);
    curve.AddStrokes (strokes, *options);        // default curve stroking.
    ConstructArcs_Add_PointTangentCurveTangent (arcs, pointA, tangentA, strokes);
    }


void CurveCurve::ConstructArcs_Add_PointTangentCurveTangent
(
bvector<FilletDetail> &arcs,
DPoint3dCR pointA,
DVec3dCR tangentA,
DPoint3dDoubleUVCurveArrays &curveStrokes
)
    {
    auto detailFunctions = CircleConstructionFunction_PointTangentPointXY::Create (pointA, tangentA);
    BeAssert (curveStrokes.m_curve.size () == curveStrokes.m_f.size ());
    if (detailFunctions.IsValid ())  // This fails if the tangent direction is null
        {
        PointTangentCurveTangentNewtonFunction curveFunctions (detailFunctions.Value());
        double a = DoubleOps::SmallCoordinateRelTol ();
        static double toleranceFactor = 1000.0;
        NewtonIterationsRRToRR newton (toleranceFactor * a);
        for (size_t i = 0; i + 1 < curveStrokes.m_f.size (); i++)
            {
            if (curveStrokes.m_curve[i] == curveStrokes.m_curve[i + 1])
                {
                curveFunctions.SetCurve (curveStrokes.m_curve[i]);
                double fraction0 = curveStrokes.m_f[i];
                double fraction1 = curveStrokes.m_f[i + 1];
                double function0 = curveFunctions.EvaluateRToR (fraction0);
                double function1 = curveFunctions.EvaluateRToR (fraction1);
                if (function0 * function1 <= 0.0)
                    {
                    ValidatedDouble fractionInInterval = DoubleOps::InverseInterpolate (function0, 0.0, function1);
                    if (fractionInInterval.IsValid ())
                        {
                        double curveFraction = DoubleOps::Interpolate (fraction0, fractionInInterval.Value (), fraction1);
                        if (newton.RunNewton (curveFraction, curveFunctions))
                            {
                            if (DoubleOps::IsAlmostIn01 (curveFraction))
                                {
                                CurveLocationDetail xyzC;
                                curveStrokes.m_curve[i]->FractionToPoint (curveFraction, xyzC);
                                ValidatedDPoint3d center = detailFunctions.Value ().WorldCircleCenter (xyzC.point);
                                if (center.IsValid ())
                                    {
                                    FilletDetail detail;
                                    detail.arc = DEllipse3d::FromCenter_StartPoint_EndTarget_StartTangentBias_CircularXY
                                        (
                                        center,
                                        pointA,
                                        xyzC.point,
                                        tangentA
                                        );
                                    detail.detailA = CurveLocationDetail (nullptr, 0.0, pointA);
                                    detail.detailB = xyzC;
                                    arcs.push_back (detail);
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }

END_BENTLEY_GEOMETRY_NAMESPACE
