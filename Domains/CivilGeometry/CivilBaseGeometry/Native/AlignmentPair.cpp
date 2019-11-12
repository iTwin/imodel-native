/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "CivilBaseGeometryInternal.h"
#include <CivilBaseGeometry/AlignmentPair.h>

#define DEFAULT_FAILURE_DISTANCE_ALONG -1.0 // Distance returned from HorizontalDistanceFromStart when the hz alignment isn't valid

//---------------------------------------------------------------------------------------
// @debug
//---------------------------------------------------------------------------------------
bool TestLength (double a, double b)
    {
#if !defined(NDEBUG)
    static double s_assertTol = 1.0e-4;
    const double d = fabs (b-a);
    if (d > s_assertTol)
        {
        //wchar_t message[1000];
        //swprintf (message, L"(Distance diff (%.17g)(%.17g)(%.6g)", a, b, d);
        //BentleyApi::BeAssertFunctions::PerformBeAssert(message, _CRT_WIDE(__FILE__), __LINE__);
        Utf8PrintfString message("Distance diff (%.17g)(%.17g)(%.6g)", a, b, d);
        CIVILBASEGEOMETRY_LOGI(message.c_str());
        return false;
        }
#endif
    return true;
    }
//---------------------------------------------------------------------------------------
// @debug
//---------------------------------------------------------------------------------------
void TestHorizontalOrder(CurveVectorCR horizontalAlignment)
    {
#if !defined(NDEBUG)
    DPoint3d overallStart, overallEnd;
    horizontalAlignment.GetStartEnd (overallStart, overallEnd);

    DPoint3d lastPoint = overallStart;
    for (ICurvePrimitivePtr const& primitive : horizontalAlignment)
        {
        DPoint3d start, end;
        primitive->GetStartEnd (start, end);
        BeAssert(start.AlmostEqualXY(lastPoint, 0.08));     // FME and Rail tests currently have some point differences, so we can't go much tighter than this unless we change those test data
        lastPoint = end;
        }
#endif
    }
//---------------------------------------------------------------------------------------
// @debug
//---------------------------------------------------------------------------------------
void TestVerticalOrder(CurveVectorCR verticalAlignment)
    {
#if !defined(NDEBUG)
    double lastX = -DBL_MAX;
    DPoint3d overallStart, overallEnd;
    verticalAlignment.GetStartEnd (overallStart, overallEnd);

    if (overallEnd.x > overallStart.x) // not a reverse alignment
        {
        for (ICurvePrimitivePtr const& primitive : verticalAlignment)
            {
            DPoint3d start, end;
            primitive->GetStartEnd (start, end);

            if (fabs (start.x-lastX) > 0.0001)
                BeAssert (start.x >= lastX);
            if (fabs (end.x-start.x) > 0.0001)
                BeAssert (end.x >= start.x);

            lastX = end.x;
            }
        }
#endif
    }
//---------------------------------------------------------------------------------------
// @debug
//---------------------------------------------------------------------------------------
void TestAlignments(CurveVectorCR horizontalAlignment, CurveVectorCP pVerticalAlignment)
    {
#if !defined(NDEBUG)
//    TestHorizontalOrder(horizontalAlignment);
//
//    if (nullptr != pVerticalAlignment && !pVerticalAlignment->empty())
//        {
//        TestVerticalOrder(*pVerticalAlignment);
//
//        CurveVectorWithDistanceIndexPtr idx = CurveVectorWithDistanceIndex::Create();
//        CurveVectorPtr hzCurve = horizontalAlignment.Clone();
//        idx->SetPath(*hzCurve);
//        double hzLen = idx->TotalPathLength();
//
//        DPoint3d start, end;
//        pVerticalAlignment->GetStartEnd(start, end);
//
//        if (!TestLength(hzLen, fabs(end.x)) || !TestLength(0.0, start.x))
//            {
//#if 0 //&&AG needswork
//            GeometryDebug::Announce(*hzCurve, "Horizontal");
//            GeometryDebug::Announce(*pVerticalAlignment, "Vertical");
//#endif
//            double hGap = hzCurve->MaxGapWithinPath();
//            double vGap = pVerticalAlignment->MaxGapWithinPath();
//            printf(" maxGap (%g,%g)\n", hGap, vGap);
//            }
//        }
#endif
    }

#if 0 //&&AG not currently in use
//---------------------------------------------------------------------------------------
// @debug
//---------------------------------------------------------------------------------------
void Dump(CurveVectorCR cv, const char *message)
    {
#if !defined(NDEBUG)
    static int s_numToPrint = 0;
    if (s_numToPrint > 0)
        {
        Utf8String string;
        CurveVectorPtr cv1 ((CurveVectorP)&cv);
        IGeometryPtr g = IGeometry::Create (cv1);
        if (BentleyGeometryJson::TryGeometryToJsonString (string, *g))
            BentleyGeometryJson::DumpJson (string);
        s_numToPrint--;
        }
#endif
    }
#endif
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Scott.Devoe                     09/2015
* constructor
+---------------+---------------+---------------+---------------+---------------+------*/
AlignmentPair::AlignmentPair(CurveVectorCP pHorizontalAlignment, CurveVectorCP pVerticalAlignment)
    { 
    UpdateCurveVectors(pHorizontalAlignment, pVerticalAlignment);
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      01/2016
+---------------+---------------+---------------+---------------+---------------+------*/
AlignmentPairPtr AlignmentPair::_Clone() const
    {
    return AlignmentPair::Create(m_horizontalCurveVector.get(), m_verticalCurveVector.get());
    }
//---------------------------------------------------------------------------------------
// @bsimethod                           Alexandre.Gagnon                        11/2017
//---------------------------------------------------------------------------------------
void AlignmentPair::_UpdateHorizontalCurveVector(CurveVectorCP pHorizontalAlignment)
    {
    m_horizontalCurveVector = (pHorizontalAlignment && !pHorizontalAlignment->empty()) ? pHorizontalAlignment->Clone() : nullptr;
    m_hzIndex = nullptr;

    if (m_horizontalCurveVector.IsValid() && CurveVector::BOUNDARY_TYPE_Open != m_horizontalCurveVector->GetBoundaryType())
        {
        CIVILBASEGEOMETRY_LOGW("AlignmentPair UpdateHorizontalCurveVector - Unexpected boundary type. Set to BOUNDARY_TYPE_OPEN");
        m_horizontalCurveVector->SetBoundaryType(CurveVector::BOUNDARY_TYPE_Open);
        }
    }
//---------------------------------------------------------------------------------------
// @bsimethod                           Alexandre.Gagnon                        11/2017
//---------------------------------------------------------------------------------------
void AlignmentPair::_UpdateVerticalCurveVector(CurveVectorCP pVerticalAlignment)
    {
    m_verticalCurveVector = (pVerticalAlignment && !pVerticalAlignment->empty()) ? pVerticalAlignment->Clone() : nullptr;
    m_vtXIndex = nullptr;

    if (m_verticalCurveVector.IsValid() && CurveVector::BOUNDARY_TYPE_Open != m_verticalCurveVector->GetBoundaryType())
        {
        CIVILBASEGEOMETRY_LOGW("AlignmentPair UpdateVerticalCurveVector - Unexpected boundary type. Set to BOUNDARY_TYPE_OPEN");
        m_verticalCurveVector->SetBoundaryType(CurveVector::BOUNDARY_TYPE_Open);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Scott.Devoe                     09/2015
+---------------+---------------+---------------+---------------+---------------+------*/
AlignmentPairPtr AlignmentPair::Create(CurveVectorCP pHorizontalAlignment, CurveVectorCP pVerticalAlignment)
    { 
    return new AlignmentPair(pHorizontalAlignment, pVerticalAlignment);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                           Alexandre.Gagnon                        11/2017
//---------------------------------------------------------------------------------------
CurveVectorCP AlignmentPair::GetHorizontalCurveVector() const
    {
    return m_horizontalCurveVector.get();
    }
//---------------------------------------------------------------------------------------
// @bsimethod                           Alexandre.Gagnon                        11/2017
//---------------------------------------------------------------------------------------
CurveVectorCP AlignmentPair::GetVerticalCurveVector() const
    {
    return m_verticalCurveVector.get();
    }
//---------------------------------------------------------------------------------------
// @bsimethod                           Alexandre.Gagnon                        11/2017
//---------------------------------------------------------------------------------------
AlignmentPairPtr AlignmentPair::Clone() const
    {
    return _Clone();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Scott.Devoe                     09/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void AlignmentPair::UpdateCurveVectors(CurveVectorCP pHorizontalAlignment, CurveVectorCP pVerticalAlignment)
    {
#if !defined(NDEBUG)
    if (nullptr != pHorizontalAlignment && !pHorizontalAlignment->empty())
        TestAlignments(*pHorizontalAlignment, pVerticalAlignment);
#endif

    UpdateHorizontalCurveVector(pHorizontalAlignment);
    UpdateVerticalCurveVector(pVerticalAlignment);
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Scott.Devoe                     09/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void AlignmentPair::UpdateHorizontalCurveVector(CurveVectorCP pHorizontalAlignment)
    {
    return _UpdateHorizontalCurveVector(pHorizontalAlignment);
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Scott.Devoe                     09/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void AlignmentPair::UpdateVerticalCurveVector(CurveVectorCP pVerticalAlignment)
    {
    return _UpdateVerticalCurveVector(pVerticalAlignment);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                           Alexandre.Gagnon                        12/2017
//---------------------------------------------------------------------------------------
bool AlignmentPair::HasSpirals() const
    {
    if (!m_horizontalCurveVector.IsValid())
        return false;

    for (auto const& primitive : *m_horizontalCurveVector)
        {
        if (primitive.IsValid() && ICurvePrimitive::CURVE_PRIMITIVE_TYPE_Spiral == primitive->GetCurvePrimitiveType())
            return true;
        }

    return false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Scott.Devoe                     09/2015
+---------------+---------------+---------------+---------------+---------------+------*/
double AlignmentPair::LengthXY() const
    {
    if (HorizontalIndexVector().IsValid())
        return HorizontalIndexVector()->TotalPathLength();

    return 0.0;
    }
//---------------------------------------------------------------------------------------
// @bsimethod                           Alexandre.Gagnon                        11/2017
//---------------------------------------------------------------------------------------
double AlignmentPair::GetVerticalElevationAt(double distanceAlongFromStart, bool extendVertical) const
    {
    const CurveVectorWithXIndexCPtr vtIdx = VerticalXIndexVector();
    if (!vtIdx.IsValid())
        return 0.0;

    const double vtLength = vtIdx->XRange().High();
    if (distanceAlongFromStart > vtLength)
        distanceAlongFromStart = distanceAlongFromStart - mgds_fc_epsilon; // some fuzz

    const ValidatedPathLocationDetail location = vtIdx->XToPathLocationDetail(distanceAlongFromStart);
    if (location.IsValid())
        return location.Value().Point().z;

    if (extendVertical)
        {
        DPoint3d vtStart, vtEnd;
        if (m_verticalCurveVector->GetStartEnd(vtStart, vtEnd))
            {
            if (distanceAlongFromStart < vtStart.x)
                return vtStart.z;
            if (distanceAlongFromStart > vtEnd.x)
                return vtEnd.z;
            }
        }

    return 0.0;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Scott.Devoe                     09/2015
+---------------+---------------+---------------+---------------+---------------+------*/
CurveVectorWithDistanceIndexPtr AlignmentPair::HorizontalIndexVector() const
    {
    if (!m_hzIndex.IsValid() && m_horizontalCurveVector.IsValid())
        {
        m_hzIndex = CurveVectorWithDistanceIndex::Create();
        m_hzIndex->SetPath(*m_horizontalCurveVector);
        }

    return m_hzIndex;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Scott.Devoe                     09/2015
+---------------+---------------+---------------+---------------+---------------+------*/
CurveVectorWithXIndexPtr AlignmentPair::VerticalXIndexVector() const
    {
    if (!m_vtXIndex.IsValid() && m_verticalCurveVector.IsValid())
        {
        m_vtXIndex = CurveVectorWithXIndex::Create(*m_verticalCurveVector);
        }

    return m_vtXIndex;
    }
//---------------------------------------------------------------------------------------
// @bsimethod                           Sandy.Bugai                        07/2017
//---------------------------------------------------------------------------------------
void AlignmentPair::InitLazyCaches() const
    {
    HorizontalIndexVector();
    VerticalXIndexVector();
    }
//---------------------------------------------------------------------------------------
// @bsimethod                           Alexandre.Gagnon                        10/2016
//---------------------------------------------------------------------------------------
CurveVectorPtr AlignmentPair::GetScaledCurveVector(CurveVectorCR cv, double scale) const
    {
    CurveVectorPtr returnVector = cv.Clone();
    Transform trans = Transform::FromScaleFactors(scale, scale, scale);
    TransformCurveWithPartialCurves(*returnVector, trans);
    return returnVector;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Scott.Devoe                     09/2015
+---------------+---------------+---------------+---------------+---------------+------*/
double AlignmentPair::DistanceAlongFromStart(CurveLocationDetailCR location) const
    {
    BeAssert(m_horizontalCurveVector.IsValid());

    const size_t index = m_horizontalCurveVector->CurveLocationDetailIndex(location);
    size_t count = 0;
    double distanceAlongFromStart = 0.0;
    for (auto const& primitive : *m_horizontalCurveVector)
        {
        double len;
        primitive->Length(len);

        if (index == count)
            {
            distanceAlongFromStart += location.fraction * len;
            break;
            }

        distanceAlongFromStart += len;
        count++;
        }

    return distanceAlongFromStart;
    }

BEGIN_UNNAMED_NAMESPACE
//---------------------------------------------------------------------------------------
// If the partial curve is a line, linestring or arc, returns the partial curve as a full ICurvePrimitive instead
// There are edge cases where a partial curve can't be resolved into a full curve because fractions are too close
// It might change the number of primitives the curve had
//---------------------------------------------------------------------------------------
void ExtractPrimitivesFromPartialCurves(CurveVectorR curve)
    {
    bvector<ICurvePrimitivePtr> temp;

    for (auto const& primitive : curve)
        {
        if (ICurvePrimitive::CURVE_PRIMITIVE_TYPE_PartialCurve != primitive->GetCurvePrimitiveType())
            {
            temp.push_back(primitive);
            continue;
            }

        //! WARNING: parentCurve will point to the primitive in the original CurveVector held by the RoadAlignment
        PartialCurveDetailCP pDetail = primitive->GetPartialCurveDetailCP();
        switch (pDetail->parentCurve->GetCurvePrimitiveType())
            {
            case ICurvePrimitive::CURVE_PRIMITIVE_TYPE_PartialCurve:
                {
                //! We have a two-level partial curve. Dereference the topmost curve and make it single-level
                ICurvePrimitivePtr newParent = pDetail->parentCurve->CloneDereferenced();
                BeAssert(newParent.IsValid() && (ICurvePrimitive::CURVE_PRIMITIVE_TYPE_PartialCurve != newParent->GetCurvePrimitiveType()));

                ICurvePrimitivePtr newPartial = ICurvePrimitive::CreatePartialCurve(newParent.get(), pDetail->fraction0, pDetail->fraction1);
                temp.push_back(newPartial);
                break;
                }
            case ICurvePrimitive::CURVE_PRIMITIVE_TYPE_BsplineCurve:
                {
                //! We want to keep parabolas as partial curve so we don't lose the poles
                ICurvePrimitivePtr newParent = pDetail->parentCurve->Clone();
                ICurvePrimitivePtr newPartial = ICurvePrimitive::CreatePartialCurve(newParent.get(), pDetail->fraction0, pDetail->fraction1);
                temp.push_back(newPartial);
                break;
                }
            case ICurvePrimitive::CURVE_PRIMITIVE_TYPE_Line:
            case ICurvePrimitive::CURVE_PRIMITIVE_TYPE_LineString:
            case ICurvePrimitive::CURVE_PRIMITIVE_TYPE_Arc:
                {
                const bool isFullClone = (DoubleOps::AlmostEqual(0.0, pDetail->fraction0) && DoubleOps::AlmostEqual(1.0, pDetail->fraction1));
                if (isFullClone)
                    {
                    //! Partial detail is irrelevant. Replace by a cloned copy of parentCurve
                    temp.push_back(pDetail->parentCurve->Clone());
                    }
                else
                    {
                    //! Create a full primitive out of the fractions. This might create an invalid primitive!
                    ICurvePrimitivePtr resolved = pDetail->parentCurve->CloneBetweenFractions(pDetail->fraction0, pDetail->fraction1, false);
                    if (resolved.IsValid())
                        temp.push_back(resolved);
                    }
                break;
                }
            default:
                {
                BeAssert(!"Must add case for");
                break;
                }
            }
        }

    //! Clear current curve and replace with result
    curve.clear();
    curve.insert(curve.end(), temp.begin(), temp.end());
    }

END_UNNAMED_NAMESPACE

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Scott.Devoe                     09/2015
+---------------+---------------+---------------+---------------+---------------+------*/
CurveVectorPtr AlignmentPair::GetPartialAlignment(CurveVectorType type, double startDistanceAlongFromStart, double endDistanceAlongFromStart) const
    {
    //! Prevents crashing in Clone() when both stations are too close to each other
    if (mgds_fc_epsilon > fabs(startDistanceAlongFromStart - endDistanceAlongFromStart))
        return nullptr;

    bool isReverse = false;
    if (startDistanceAlongFromStart > endDistanceAlongFromStart)
        {
        std::swap(startDistanceAlongFromStart, endDistanceAlongFromStart);
        isReverse = true;
        }

    CurveVectorPtr result = nullptr;
    if (CurveVectorType::Horizontal == type)
        {
        if (HorizontalIndexVector().IsValid())
            result = HorizontalIndexVector()->CloneBetweenDistances(startDistanceAlongFromStart, endDistanceAlongFromStart);
        }
    else if (VerticalXIndexVector().IsValid())
        {
        DPoint3d start, end;
        m_verticalCurveVector->GetStartEnd(start, end);

        //&&AG we might want to ask Earlin if he could expose the 'usePartialCurves' instead of copying all this code here !!
        ValidatedPathLocationDetail detailA = VerticalXIndexVector()->XToPathLocationDetail(startDistanceAlongFromStart);
        ValidatedPathLocationDetail detailB = VerticalXIndexVector()->XToPathLocationDetail((MIN(end.x, endDistanceAlongFromStart)));
        if (detailA.IsValid() && detailB.IsValid())
            {
            result = VerticalXIndexVector()->GetCurveVectorPtr()->CloneBetweenDirectedFractions((int)detailA.Value().PathIndex(), detailA.Value().CurveFraction(),
                                                                              (int)detailB.Value().PathIndex(), detailB.Value().CurveFraction(),
                                                                              false, true); // false: allowExtrapolation, true: usePartialCurves
            }

        //! If the end station was a bit greater than the vt length, it will resize the output properly
        // 0.6 is a sdev magic number
        if (endDistanceAlongFromStart > end.x && endDistanceAlongFromStart - end.x < 0.6 && result.IsValid())
            result->back()->TrySetEnd(DPoint3d::From(endDistanceAlongFromStart, 0.0, end.z));
        }

    //! We might have created partial curves over linestring, line or arc. In theses cases, we can extract the result curve without any side effects.
    if (result.IsValid())
        ExtractPrimitivesFromPartialCurves(*result);

    if (result.IsValid() && isReverse)
        result->ReverseCurvesInPlace();

    return result;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                           Alexandre.Gagnon                        11/2017
//---------------------------------------------------------------------------------------
CurveVectorPtr AlignmentPair::CloneHorizontalCurveVector(double scaleFactor) const
    {
    if (!m_horizontalCurveVector.IsValid() || 0 >= scaleFactor)
        return nullptr;

    if (DoubleOps::AlmostEqual(1.0, scaleFactor))
        return m_horizontalCurveVector->Clone();

    return GetScaledCurveVector(*m_horizontalCurveVector, scaleFactor);
    }
//---------------------------------------------------------------------------------------
// @bsimethod                           Alexandre.Gagnon                        11/2017
//---------------------------------------------------------------------------------------
CurveVectorPtr AlignmentPair::CloneVerticalCurveVector(double scaleFactor) const
    {
    if (!m_verticalCurveVector.IsValid() || 0 >= scaleFactor)
        return nullptr;

    if (DoubleOps::AlmostEqual(1.0, scaleFactor))
        return m_verticalCurveVector->Clone();

    return GetScaledCurveVector(*m_verticalCurveVector, scaleFactor);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Ben.Bartholomew                 03/2016
+---------------+---------------+---------------+---------------+---------------+------*/
bool AlignmentPair::ClosestPoint(DPoint3dR locationPoint, DPoint3dCR referencePoint, bool extendVertical, ICurvePrimitive::CurvePrimitiveType* pType) const
    {
    if (!m_horizontalCurveVector.IsValid())
        return false;

    CurveLocationDetail location;
    const DPoint3d atPoint = DPoint3d::From(referencePoint.x, referencePoint.y, 0.0);

    if (!m_horizontalCurveVector->ClosestPointBoundedXY(atPoint, nullptr, location))
        return false;

    locationPoint = location.point;
    const double distanceAlongFromStart = DistanceAlongFromStart(location);
    locationPoint.z = GetVerticalElevationAt(distanceAlongFromStart, extendVertical);
    
    if (nullptr != pType)
        *pType = location.curve->GetCurvePrimitiveType();

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Scott.Devoe                     10/2015
+---------------+---------------+---------------+---------------+---------------+------*/
bool AlignmentPair::ClosestPointXY(DPoint3dR locationPoint, DPoint3dCR referencePoint) const
    {
    if (!m_horizontalCurveVector.IsValid())
        return false;

    CurveLocationDetail location;
    const DPoint3d atPoint = DPoint3d::From(referencePoint.x, referencePoint.y, 0.0);

    if (!m_horizontalCurveVector->ClosestPointBoundedXY(atPoint, nullptr, location))
        return false;

    locationPoint = location.point;
    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Scott.Devoe                     03/2016
+---------------+---------------+---------------+---------------+---------------+------*/
bool AlignmentPair::ClosestPointAndTangentXY(DPoint3dR locationPoint, DVec3dR tangent, DPoint3dCR referencePoint) 
    {
    if (!m_horizontalCurveVector.IsValid())
        return false;

    CurveLocationDetail location;
    const DPoint3d atPoint = DPoint3d::From(referencePoint.x, referencePoint.y, 0.0);

    if (!m_horizontalCurveVector->ClosestPointBoundedXY(atPoint, nullptr, location))
        return false;

    const ValidatedDRay3d ray = location.PointAndUnitTangent();
    if (ray.IsValid())
        {
        locationPoint = ray.Value().origin;
        tangent = ray.Value().direction;
        }

    return ray.IsValid();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Scott.Devoe                     09/2015
+---------------+---------------+---------------+---------------+---------------+------*/
bool AlignmentPair::GetStartEnd(DPoint3dR startPt, DPoint3dR endPt, bool extendVertical) const
    {
    if (!m_horizontalCurveVector.IsValid() || !m_horizontalCurveVector->GetStartEnd(startPt, endPt))
        return false;

    const double len = LengthXY();
    startPt.z = GetVerticalElevationAt(0.0, extendVertical);
    endPt.z = GetVerticalElevationAt(len, extendVertical);
    return true;
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Scott.Devoe                     09/2015
+---------------+---------------+---------------+---------------+---------------+------*/
bool AlignmentPair::GetStartAndEndDistancesAlong(double& startStation, double& endStation) const
    {
    if (!m_horizontalCurveVector.IsValid() || m_horizontalCurveVector->empty())
        return false;

    startStation = 0.0;
    endStation = LengthXY();
    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Scott.Devoe                     09/2015
+---------------+---------------+---------------+---------------+---------------+------*/
double AlignmentPair::HorizontalDistanceAlongFromStart(DPoint3dCR referencePoint, double* pOffset) const
    {
    if (!HorizontalIndexVector().IsValid())
        return DEFAULT_FAILURE_DISTANCE_ALONG;

    const PathLocationDetail location = HorizontalIndexVector()->SearchClosestPointBounded(referencePoint, true);
    const double distance = location.DistanceFromPathStart();

    if (nullptr != pOffset)
        {
        DVec3d tangent;
        const DPoint3d onPt = location.PointAndUnitTangent(tangent);
        double offset = referencePoint.DistanceXY(onPt);

        const DVec3d toPt = DVec3d::FromStartEnd(onPt, referencePoint);
        if (0.0 > toPt.CrossProductXY(tangent)) // left so negate
            offset *= -1;

        *pOffset = offset;
        }

    return distance;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sandy.Bugai                     01/2016
+---------------+---------------+---------------+---------------+---------------+------*/
double AlignmentPair::HorizontalDistanceAlongFromEnd(DPoint3dCR referencePoint, double * pOffset) const
    {
    if (!HorizontalIndexVector().IsValid())
        return DEFAULT_FAILURE_DISTANCE_ALONG;

    // Get distance from start and then 'invert' the distance to get the end. Offset should be the same
    const double distance = HorizontalDistanceAlongFromStart(referencePoint, pOffset);
    return LengthXY() - distance;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Scott.Devoe                     10/2015
+---------------+---------------+---------------+---------------+---------------+------*/
ICurvePrimitiveCPtr AlignmentPair::GetPrimitiveAtPoint(DPoint3dCR referencePoint) const
    {
    if (!HorizontalIndexVector().IsValid())
        return nullptr;

    const PathLocationDetail location = HorizontalIndexVector()->SearchClosestPointBounded(referencePoint, true);
    const CurveLocationDetail detail = location.GetCurveLocationDetail();
    return detail.curve;
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Stephane.Poulin                 06/2017
+---------------+---------------+---------------+---------------+---------------+------*/
ICurvePrimitiveCPtr AlignmentPair::GetPrimitiveAtStation(double distanceAlongFromStart) const
    {
    if (!HorizontalIndexVector().IsValid())
        return nullptr;

    PathLocationDetail location;
    if (true == HorizontalIndexVector()->SearchByDistanceFromPathStart(distanceAlongFromStart, location))
        {
        const CurveLocationDetail detail = location.GetCurveLocationDetail();
        return detail.curve;
        }

    return nullptr;
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Stephane.Poulin                 06/2017
+---------------+---------------+---------------+---------------+---------------+------*/
ValidatedPathLocationDetail AlignmentPair::GetPathLocationDetailAtStation(double distanceAlongFromStart) const
    {
    if (!HorizontalIndexVector().IsValid())
        return ValidatedPathLocationDetail();

    PathLocationDetail location;
    const bool isValid = HorizontalIndexVector()->SearchByDistanceFromPathStart(distanceAlongFromStart, location);
    return ValidatedPathLocationDetail(location, isValid);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Scott.Devoe                     09/2015
+---------------+---------------+---------------+---------------+---------------+------*/
CurveVectorPtr AlignmentPair::GetPartialHorizontalAlignment(DPoint3dCR fromPt, DPoint3dCR toPt) const
    {
    if (!m_horizontalCurveVector.IsValid())
        return nullptr;

    const double startDistanceAlongFromStart = HorizontalDistanceAlongFromStart(fromPt, nullptr);
    const double endDistanceAlongFromStart = HorizontalDistanceAlongFromStart(toPt, nullptr);

    return GetPartialAlignment(CurveVectorType::Horizontal, startDistanceAlongFromStart, endDistanceAlongFromStart);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Scott.Devoe                     09/2015
+---------------+---------------+---------------+---------------+---------------+------*/
CurveVectorPtr AlignmentPair::GetPartialHorizontalAlignment(double startDistanceAlongFromStart, double endDistanceAlongFromStart) const
    {
    if (!m_horizontalCurveVector.IsValid())
        return nullptr;

    return GetPartialAlignment(CurveVectorType::Horizontal, startDistanceAlongFromStart, endDistanceAlongFromStart);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Scott.Devoe                     09/2015
+---------------+---------------+---------------+---------------+---------------+------*/
CurveVectorPtr AlignmentPair::GetPartialVerticalAlignment(double startDistanceAlongFromStart, double endDistanceAlongFromStart) const
    { 
#if !defined(NDEBUG)
    if (endDistanceAlongFromStart > LengthXY())
        TestLength(fabs(LengthXY() - endDistanceAlongFromStart), 0.0);
#endif

    if (nullptr == GetVerticalCurveVector() || !VerticalXIndexVector().IsValid())
        return nullptr;

    CurveVectorPtr retVal = GetPartialAlignment(CurveVectorType::Vertical, startDistanceAlongFromStart, endDistanceAlongFromStart);

    if (retVal.IsValid() && fabs(0.0 - startDistanceAlongFromStart) > DBL_EPSILON)
        {
        DPoint3d vStart, vEnd;
        GetVerticalCurveVector()->GetStartEnd(vStart, vEnd);

        const double xOffset = MIN(vEnd.x, startDistanceAlongFromStart);

        Transform trans = Transform::From(0.0 - xOffset, 0.0, 0.0);
        TransformCurveWithPartialCurves(*retVal, trans);

        DPoint3d start, end;
        retVal->GetStartEnd (start, end);
        TestLength(start.x ,0.0);
        TestVerticalOrder(*retVal);
        
        if (endDistanceAlongFromStart < startDistanceAlongFromStart)
            {
            BeAssert(end.x < start.x);
            Transform xAxisSignTransform = Transform::FromScaleFactors(-1, 1.0, 1.0);
            TransformCurveWithPartialCurves(*retVal, xAxisSignTransform);
            }
        }

    return retVal;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Scott.Devoe                     09/2015
+---------------+---------------+---------------+---------------+---------------+------*/
AlignmentPairPtr AlignmentPair::GetPartialAlignment (DPoint3dCR fromPt, DPoint3dCR toPt) const
    {
    const double startDistanceAlongFromStart = HorizontalDistanceAlongFromStart(fromPt);
    const double endDistanceAlongFromStart = HorizontalDistanceAlongFromStart(toPt);

    return GetPartialAlignment(startDistanceAlongFromStart, endDistanceAlongFromStart);
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Scott.Devoe                     09/2015
+---------------+---------------+---------------+---------------+---------------+------*/
AlignmentPairPtr AlignmentPair::GetPartialAlignment(double startDistanceAlongFromStart, double endDistanceAlongFromStart) const
    {
    const CurveVectorCPtr hz = GetPartialHorizontalAlignment(startDistanceAlongFromStart, endDistanceAlongFromStart);
    if (!hz.IsValid())
        return nullptr;

    const CurveVectorCPtr vt = GetPartialVerticalAlignment(startDistanceAlongFromStart, endDistanceAlongFromStart);
    return AlignmentPair::Create(hz.get(), vt.get());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Scott.Devoe                     09/2015
+---------------+---------------+---------------+---------------+---------------+------*/
ValidatedDPoint3d AlignmentPair::GetPointAt(double distanceAlongFromStart) const
    {
    if (!HorizontalIndexVector().IsValid())
        return ValidatedDPoint3d();

    if (distanceAlongFromStart > LengthXY())
        distanceAlongFromStart = distanceAlongFromStart - mgds_fc_epsilon;

    PathLocationDetail location;
    if (true == HorizontalIndexVector()->SearchByDistanceFromPathStart(distanceAlongFromStart, location))
        return ValidatedDPoint3d(location.Point(), true);

    return ValidatedDPoint3d(DPoint3d::FromZero(), false);
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Scott.Devoe                     10/2015
+---------------+---------------+---------------+---------------+---------------+------*/
ValidatedDPoint3d AlignmentPair::GetPointAtAndOffset(double distanceAlongFromStart, double offset) const
    {
    if (!HorizontalIndexVector().IsValid())
        return ValidatedDPoint3d();

    if (distanceAlongFromStart > LengthXY())
        distanceAlongFromStart = distanceAlongFromStart - mgds_fc_epsilon;

    PathLocationDetail location;
    if (true == HorizontalIndexVector()->SearchByDistanceFromPathStart(distanceAlongFromStart, location))
        {
        DVec3d tangent;
        const DPoint3d onPt = location.PointAndUnitTangent(tangent);

        const DVec3d perpVec = DVec3d::From(tangent.y, -tangent.x, 0.0);
        return ValidatedDPoint3d(DPoint3d::FromSumOf(onPt, perpVec, offset), true);
        }

    return ValidatedDPoint3d(DPoint3d::FromZero(), false);
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Scott.Devoe                     09/2015
+---------------+---------------+---------------+---------------+---------------+------*/
bool AlignmentPair::GetPointAndTangentAt(DPoint3dR hzPoint, DVec3dR hzTangent, double distanceAlongFromStart) const
    {
    if (!HorizontalIndexVector().IsValid())
        return false;

    if (distanceAlongFromStart > LengthXY())
        distanceAlongFromStart = distanceAlongFromStart - mgds_fc_epsilon;

    PathLocationDetail location;
    if (true == HorizontalIndexVector()->SearchByDistanceFromPathStart(distanceAlongFromStart, location))
        {
        hzPoint = location.PointAndUnitTangent(hzTangent);
        return true;
        }

    return false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Scott.Devoe                     09/2015
+---------------+---------------+---------------+---------------+---------------+------*/
ValidatedDPoint3d AlignmentPair::GetPointAtWithZ(double distanceAlongFromStart, bool extendVertical) const
    {
    ValidatedDPoint3d returnPoint = GetPointAt(distanceAlongFromStart);
    if (returnPoint.IsValid())
        returnPoint.Value().z = GetVerticalElevationAt(distanceAlongFromStart, extendVertical);

    return returnPoint;
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Scott.Devoe                     09/2015
+---------------+---------------+---------------+---------------+---------------+------*/
bool AlignmentPair::GetPointAndTangentAtWithZ(DPoint3dR locationPoint, DVec3dR hzTangent, double distanceAlongFromStart, bool extendVertical) const
    {
    if (!GetPointAndTangentAt(locationPoint, hzTangent, distanceAlongFromStart))
        return false;

    locationPoint.z = GetVerticalElevationAt(distanceAlongFromStart, extendVertical);
    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Scott.Devoe                     09/2015
+---------------+---------------+---------------+---------------+---------------+------*/
bvector<DPoint3d> AlignmentPair::GetStrokedAlignment(double maxEdgeLength) const
    {
    IFacetOptionsPtr options = IFacetOptions::CreateForCurves();
    options->SetMaxEdgeLength(maxEdgeLength);
    options->SetAngleTolerance(.05); //In radians
    options->SetChordTolerance(1);

    bvector<DPoint3d> strokes;

    if (!m_horizontalCurveVector.IsValid())
        return strokes;

    if (!VerticalXIndexVector().IsValid())
        {
        m_horizontalCurveVector->AddStrokePoints(strokes, *options);
        CIVILBASEGEOMETRY_LOGW("AlignmentPair - GetStrokedAlignment called on an alignment without vertical.");
        }
    else
        {
        bvector<PathLocationDetailPair> locationPairs;
        CurveVectorWithDistanceIndex::StrokeHorizontalAndVerticalCurves(*options, *options, *HorizontalIndexVector(), *VerticalXIndexVector(), locationPairs);
        PathLocationDetailPair::Merge(locationPairs, &strokes, nullptr);
        }

    return strokes;
    }

BEGIN_UNNAMED_NAMESPACE
//---------------------------------------------------------------------------------------
// @bsimethod                           Alexandre.Gagnon                        10/2017
//---------------------------------------------------------------------------------------
double computeMinimumRadius(CurveVectorCP pHorizontal, bool wantsCCW)
    {
    double minRadius = CS_SPI_INFINITY;

    if (nullptr == pHorizontal)
        return minRadius;

    for (auto const& primitive : *pHorizontal)
        {
        if (ICurvePrimitive::CURVE_PRIMITIVE_TYPE_Arc != primitive->GetCurvePrimitiveType())
            continue;

        const DEllipse3dCP pArc = primitive->GetArcCP();
        if (wantsCCW != pArc->IsCCWSweepXY())
            continue;

        double radius;
        if (pArc->IsCircularXY(radius) && radius < minRadius)
            minRadius = radius;
        }

    return minRadius;
    }
END_UNNAMED_NAMESPACE
//---------------------------------------------------------------------------------------
// @bsimethod                           Alexandre.Gagnon                        10/2017
//---------------------------------------------------------------------------------------
double AlignmentPair::ComputeLeftMinimumRadius() const
    {
    return computeMinimumRadius(GetHorizontalCurveVector(), true);
    }
//---------------------------------------------------------------------------------------
// @bsimethod                           Alexandre.Gagnon                        10/2017
//---------------------------------------------------------------------------------------
double AlignmentPair::ComputeRightMinimumRadius() const
    {
    return computeMinimumRadius(GetHorizontalCurveVector(), false);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sandy.Bugai                     03/2016
+---------------+---------------+---------------+---------------+---------------+------*/
bool AlignmentPair::ComputeIntersectionWith (DPoint3dR result, AlignmentPairCP second, DPoint3dCP nearestToReference, double * primaryStation, double * secondaryStation)
    {
    /*AlignmentIntersectionPtr alignmentIntersection = AlignmentIntersection::Create (this, second);
    if (nearestToReference != nullptr)
        {
        AlignmentIntersectionInfo info;
        if (alignmentIntersection->ComputeClosest (*nearestToReference, info))
            {
            *primaryStation = info.PrimaryStation ();
            *secondaryStation = info.SecondaryStation ();
            result = info.Point ();
            return true;
            }
        }
    else if (alignmentIntersection->ComputeIntersectionPoint (result, primaryStation, secondaryStation, false))
        {
        return true;
        }*/

    return false;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                           Alexandre.Gagnon                        09/2017
//---------------------------------------------------------------------------------------
void AlignmentPair::TransformCurveWithPartialCurves(CurveVectorR cv, TransformCR transform)
    {
    for (auto& primitive : cv)
        {
        if (ICurvePrimitive::CURVE_PRIMITIVE_TYPE_PartialCurve == primitive->GetCurvePrimitiveType())
            {
            PartialCurveDetailCP pDetail = primitive->GetPartialCurveDetailCP();
            if (!pDetail->parentCurve->TransformInPlace(transform))
                {
                CIVILBASEGEOMETRY_LOGI("AlignmentPair - Failed to transform partial curve with parent of type %i",
                    static_cast<int>(pDetail->parentCurve->GetCurvePrimitiveType()));
                }
            }
        else
            {
            if (!primitive->TransformInPlace(transform))
                {
                CIVILBASEGEOMETRY_LOGI("AlignmentPair - Failed to apply transform on primitive");
                }
            }
        }
    }


//---------------------------------------------------------------------------------------
// @bsimethod                           Arun.George                        06/2018
//---------------------------------------------------------------------------------------
double AlignmentPair::SlopeAtDistanceAlong(const double& distanceAlong) const
    {
    DPoint3d start, end;
    start = GetPointAtWithZ(distanceAlong - 0.01);
    end = GetPointAtWithZ(distanceAlong + 0.01);

    double startX = HorizontalDistanceAlongFromStart(start);
    double endX = HorizontalDistanceAlongFromStart(end);

    if (start.z == end.z)
        return 0.0;

    return (end.z - start.z) / (endX - startX);
    }