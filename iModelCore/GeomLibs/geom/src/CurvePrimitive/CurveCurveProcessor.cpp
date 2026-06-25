/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include <bsibasegeomPCH.h>
#include "../DeprecatedFunctions.h"
#include "CurveCurveProcessor.h"
BEGIN_BENTLEY_GEOMETRY_NAMESPACE


/*--------------------------------------------------------------------------------**//**
* @bsimethod
+--------------------------------------------------------------------------------------*/
void CurveCurveProcessor::ProcessPrimitivePrimitive (ICurvePrimitiveP curveA, ICurvePrimitiveP curveB, bool bReverseOrder)
    {
    m_numProcessedByBaseClass++;
    }


/*--------------------------------------------------------------------------------**//**
* @bsimethod
+--------------------------------------------------------------------------------------*/
CurveCurveProcessor::CurveCurveProcessor(DMatrix4dCP pWorldToLocal)
    : m_pWorldToLocal (pWorldToLocal)
    {
    m_numProcessedByBaseClass = 0;
    SetExtend (false);
    }

#define IMPLEMENT_CCProcessor_ProcessAaBb(MethodName,TypeA,TypeB) \
void CurveCurveProcessor::MethodName (                             \
        ICurvePrimitiveP curveA, TypeA dataA,               \
        ICurvePrimitiveP curveB, TypeB dataB,               \
        bool bReverseOrder) {ProcessPrimitivePrimitive (curveA, curveB, bReverseOrder);}

IMPLEMENT_CCProcessor_ProcessAaBb (ProcessLineLine, DSegment3dCR, DSegment3dCR)
IMPLEMENT_CCProcessor_ProcessAaBb (ProcessLineLinestring, DSegment3dCR, bvector<DPoint3d> const &)
IMPLEMENT_CCProcessor_ProcessAaBb (ProcessLineArc, DSegment3dCR, DEllipse3dCR)

IMPLEMENT_CCProcessor_ProcessAaBb (ProcessLinestringLinestring, bvector<DPoint3d> const &, bvector<DPoint3d> const &)
IMPLEMENT_CCProcessor_ProcessAaBb (ProcessLinestringArc, bvector<DPoint3d> const &, DEllipse3dCR)

IMPLEMENT_CCProcessor_ProcessAaBb (ProcessArcArc, DEllipse3dCR, DEllipse3dCR)



/*--------------------------------------------------------------------------------**//**
* @bsimethod
+--------------------------------------------------------------------------------------*/
void CurveCurveProcessor::ProcessLineBspline (ICurvePrimitiveP curveA, DSegment3dCR segmentA, ICurvePrimitiveP curveB, bool bReverseOrder) {ProcessPrimitivePrimitive (curveA, curveB, bReverseOrder);}

/*--------------------------------------------------------------------------------**//**
* @bsimethod
+--------------------------------------------------------------------------------------*/
void CurveCurveProcessor::ProcessArcBspline (ICurvePrimitiveP curveA, DEllipse3dCR ellipseA, ICurvePrimitiveP curveB, bool bReverseOrder) {ProcessPrimitivePrimitive (curveA, curveB, bReverseOrder);}

/*--------------------------------------------------------------------------------**//**
* @bsimethod
+--------------------------------------------------------------------------------------*/
void CurveCurveProcessor::ProcessLinestringBspline (ICurvePrimitiveP curveA, bvector<DPoint3d> const &, ICurvePrimitiveP curveB, bool bReverseOrder) {ProcessPrimitivePrimitive (curveA, curveB, bReverseOrder);}

/*--------------------------------------------------------------------------------**//**
* @bsimethod
+--------------------------------------------------------------------------------------*/
void CurveCurveProcessor::ProcessBsplineBspline (ICurvePrimitiveP curveA, ICurvePrimitiveP curveB, bool bReverseOrder) {ProcessPrimitivePrimitive (curveA, curveB, bReverseOrder);}

void CurveCurveProcessor::Process (CurveVectorCP vectorA, ICurvePrimitiveP curveB)
    {
    for (size_t i = 0, n = vectorA->size (); i < n; i++)
        {
        CurveVectorCP child = vectorA->at (i)->GetChildCurveVectorCP ();
        if (NULL != child)
            Process (child, curveB);
        else
            Process (vectorA->at(i).get (), curveB);
        }
    }


/*--------------------------------------------------------------------------------**//**
* @bsimethod
+--------------------------------------------------------------------------------------*/
void CurveCurveProcessor::Process (ICurvePrimitiveP curveA, ICurvePrimitiveP curveB)
    {
    ICurvePrimitive::CurvePrimitiveType typeA = curveA->GetCurvePrimitiveType ();
    ICurvePrimitive::CurvePrimitiveType typeB = curveB->GetCurvePrimitiveType ();

    if (typeA == ICurvePrimitive::CURVE_PRIMITIVE_TYPE_CurveVector)
        {
        CurveVectorCP vectorA = curveA->GetChildCurveVectorCP ();
        for (size_t i = 0; i < vectorA->size (); i++)
            {
            ICurvePrimitivePtr childA = vectorA->at(i);
            Process (childA.get (), curveB);
            }
        return;
        }

    if (typeB == ICurvePrimitive::CURVE_PRIMITIVE_TYPE_CurveVector)
        {
        CurveVectorCP vectorB = curveB->GetChildCurveVectorCP ();
        for (size_t i = 0; i < vectorB->size (); i++)
            {
            ICurvePrimitivePtr childB = vectorB->at(i);
            Process (curveA, childB.get ());
            }
        return;
        }


    switch (typeA)
        {

        case ICurvePrimitive::CURVE_PRIMITIVE_TYPE_Line:
            switch (typeB)
                {
                case ICurvePrimitive::CURVE_PRIMITIVE_TYPE_Line:
                    ProcessLineLine (curveA, *curveA->GetLineCP (), curveB, *curveB->GetLineCP (), false);
                    break;
                case ICurvePrimitive::CURVE_PRIMITIVE_TYPE_LineString:
                    ProcessLineLinestring (curveA, *curveA->GetLineCP (), curveB, *curveB->GetLineStringCP (), false);
                    break;
                case ICurvePrimitive::CURVE_PRIMITIVE_TYPE_Arc:
                    ProcessLineArc (curveA, *curveA->GetLineCP (), curveB, *curveB->GetArcCP (), false);
                    break;
                case ICurvePrimitive::CURVE_PRIMITIVE_TYPE_BsplineCurve:
                    ProcessLineBspline (curveA, *curveA->GetLineCP (),curveB, false);
                    break;
                default:
                    {
                    if (NULL != curveB->GetProxyBsplineCurveCP ())
                        ProcessLineBspline (curveA, *curveA->GetLineCP (),curveB, false);
                    break;
                    }
                }
            break;

        case ICurvePrimitive::CURVE_PRIMITIVE_TYPE_LineString:
            switch (typeB)
                {
                case ICurvePrimitive::CURVE_PRIMITIVE_TYPE_Line:
                    ProcessLineLinestring (curveB, *curveB->GetLineCP (),curveA, *curveA->GetLineStringCP (), true);
                    break;
                case ICurvePrimitive::CURVE_PRIMITIVE_TYPE_LineString:
                    ProcessLinestringLinestring (curveA, *curveA->GetLineStringCP (),curveB, *curveB->GetLineStringCP (), false);
                    break;
                case ICurvePrimitive::CURVE_PRIMITIVE_TYPE_Arc:
                    ProcessLinestringArc (curveA, *curveA->GetLineStringCP (), curveB, *curveB->GetArcCP (), false);
                    break;
                case ICurvePrimitive::CURVE_PRIMITIVE_TYPE_BsplineCurve:
                    ProcessLinestringBspline (curveA, *curveA->GetLineStringCP (),curveB, false);
                    break;
                default:
                    {
                    if (NULL != curveB->GetProxyBsplineCurveCP ())
                        ProcessLinestringBspline (curveA, *curveA->GetLineStringCP (),curveB, false);
                    break;
                    }
                }
            break;

        case ICurvePrimitive::CURVE_PRIMITIVE_TYPE_Arc:
            switch (typeB)
                {
                case ICurvePrimitive::CURVE_PRIMITIVE_TYPE_Line:
                    ProcessLineArc (curveB, *curveB->GetLineCP (),curveA, *curveA->GetArcCP (), true);
                    break;
                case ICurvePrimitive::CURVE_PRIMITIVE_TYPE_LineString:
                    ProcessLinestringArc (curveB, *curveB->GetLineStringCP (),curveA, *curveA->GetArcCP (), true);
                    break;
                case ICurvePrimitive::CURVE_PRIMITIVE_TYPE_Arc:
                    ProcessArcArc (curveA, *curveA->GetArcCP (), curveB, *curveB->GetArcCP (), false);
                    break;
                case ICurvePrimitive::CURVE_PRIMITIVE_TYPE_BsplineCurve:
                    ProcessArcBspline (curveA, *curveA->GetArcCP (),curveB, false);
                    break;
                default:
                    {
                    if (NULL != curveB->GetProxyBsplineCurveCP ())
                        ProcessArcBspline (curveA, *curveA->GetArcCP (),curveB, false);
                    break;
                    }
                }
            break;

        default:
            {
            if (NULL != curveA->GetProxyBsplineCurveCP ())
                {
                switch (typeB)
                    {
                    case ICurvePrimitive::CURVE_PRIMITIVE_TYPE_Line:
                        ProcessLineBspline (curveB, *curveB->GetLineCP (),curveA, true);
                        break;
                    case ICurvePrimitive::CURVE_PRIMITIVE_TYPE_LineString:
                        ProcessLinestringBspline (curveB, *curveB->GetLineStringCP (),curveA, true);
                        break;
                    case ICurvePrimitive::CURVE_PRIMITIVE_TYPE_Arc:
                        ProcessArcBspline (curveB, *curveB->GetArcCP (), curveA, true);
                        break;
                    case ICurvePrimitive::CURVE_PRIMITIVE_TYPE_BsplineCurve:
                        ProcessBsplineBspline (curveA, curveB, false);
                        break;
                    default:
                        {
                        if (NULL != curveB->GetProxyBsplineCurveCP ())
                            ProcessBsplineBspline (curveA, curveB, false);
                        break;
                        }
                    }
                }
            break;
            }
        }
    }

// Apply the worldToLocal transform to a point with weight
void CurveCurveProcessor::TransformWeightedPoint
(
DPoint4d &hPoint,
DPoint3d const &cPoint,
double   w
)
    {
    if (m_pWorldToLocal)
        {
        bsiDMatrix4d_multiplyWeightedDPoint3dArray (m_pWorldToLocal, &hPoint, &cPoint, &w, 1);
        }
    else
        hPoint.InitFrom (cPoint, w);
    }

// Apply the worldToLocal transform to a segment.
void CurveCurveProcessor::TransformSegment
(
DSegment4d &hSeg,
DSegment3d const &cSeg
)
    {
    TransformWeightedPoint (hSeg.point[0], cSeg.point[0], 1.0);
    TransformWeightedPoint (hSeg.point[1], cSeg.point[1], 1.0);
    }

// Apply the worldToLocal transform to an ellipse
void CurveCurveProcessor::TransformEllipse
(
DConic4d &hConic,
DEllipse3d const &cEllipse
)
    {
    bsiDConic4d_initFromDEllipse3d (&hConic, &cEllipse);
    if (m_pWorldToLocal)
        bsiDConic4d_applyDMatrix4d (&hConic, m_pWorldToLocal, &hConic);
    }

// lexicographical point comparison: x first, then y, then z
int CompareDSegment3d::compareXYZ(DPoint3dCR p0, DPoint3dCR p1) const
    {
    auto xCompare = DoubleOps::TolerancedComparison(p0.x, p1.x, m_coordTol);
    auto yCompare = DoubleOps::TolerancedComparison(p0.y, p1.y, m_coordTol);
    auto zCompare = DoubleOps::TolerancedComparison(p0.z, p1.z, m_coordTol);
    if (!xCompare && !yCompare && !zCompare)
        return 0;
    if (xCompare)
        return xCompare;
    if (yCompare)
        return yCompare;
    return zCompare;
    }
// lexicographical segment "less" operator: compare first points of each segment, then the second points
bool CompareDSegment3d::operator()(DSegment3dCR a, DSegment3dCR b) const
    {
    int compareA = compareXYZ(a.point[0], b.point[0]);
    int compareB = compareXYZ(a.point[1], b.point[1]);
    if (compareA != 0)
        return compareA < 0;
    return compareB < 0;
    }

void CurveCurveProcessAndCollectCloseApproaches::CollectPair(ICurvePrimitiveCP curve0, ICurvePrimitiveCP curve1, double fraction0, double fraction1, bool bReverse)
    {
    return CollectPair(curve0, nullptr, fraction0, curve1, nullptr, fraction1, bReverse);
    }

void CurveCurveProcessAndCollectCloseApproaches::CollectPair(ICurvePrimitiveCP curve0, DPoint3dCP point0, double fraction0, ICurvePrimitiveCP curve1, DPoint3dCP point1, double fraction1, bool bReverse)
    {
    DSegment3d seg;
    if (!point0)
        curve0->FractionToPoint(fraction0, seg.point[0]);
    else
        seg.SetStartPoint(*point0);
    if (!point1)
        curve1->FractionToPoint(fraction1, seg.point[1]);
    else
        seg.SetEndPoint(*point1);

    CurveLocationDetailPair pair(curve0, fraction0, seg.point[0], curve1, fraction1, seg.point[1]);
    pair.detailA.a = pair.detailB.a = seg.Length();
    if (bReverse)
        pair.SwapDetails();

    if (ClosestOnly())
        {
        if (!m_pairs.empty())
            {
            if (m_pairs.begin()->second.detailA.a < pair.detailA.a)
                return; // we already have a closer approach
            m_pairs.erase(m_pairs.begin());
            }
        }
    else if (pair.detailA.a > m_maxDistance)
        {
        return;
        }

    m_pairs.emplace(seg, pair);
    }

bool CurveCurveProcessAndCollectCloseApproaches::GetResults(CurveCurve::ICloseApproachAnnouncer& announce) const
    {
    // for each range of equivalent close approaches, announce the closest
    bool announcedAtLeastOne = false;
    for (auto rangeBegin = m_pairs.begin(), rangeEnd = m_pairs.end(); rangeBegin != m_pairs.end(); rangeBegin = rangeEnd)
        {
        rangeEnd = m_pairs.upper_bound(rangeBegin->first);
        auto least = std::min_element(rangeBegin, rangeEnd, s_segmentPairLess);
        if (least != rangeEnd)
            {
            announce(least->second);
            announcedAtLeastOne = true;
            }
        }
    return announcedAtLeastOne;
    }
 bool CurveCurveProcessAndCollectCloseApproaches::GetResult(CurveLocationDetailPairR result) const
    {
    auto least = std::min_element(m_pairs.begin(), m_pairs.end(), s_segmentPairLess);
    if (least == m_pairs.end())
        return false;
    result = least->second;
    return true;
    }

END_BENTLEY_GEOMETRY_NAMESPACE
