/*--------------------------------------------------------------------------------------+
|
|     $Source: AlignmentPair.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "RoadRailAlignmentInternal.h"
#include <RoadRailAlignment/AlignmentPair.h>

#define STROKE_PERCENTAGE_OF_LENGTH 0.03
static double s_assertTol = 1.0e-4;


//---------------------------------------------------------------------------------------
// @debug
//---------------------------------------------------------------------------------------
bool TestLength (double a, double b)
    {
#if !defined(NDEBUG)
    const double d = fabs (b-a);
    if (d > s_assertTol)
        {
        //wchar_t message[1000];
        //swprintf (message, L"(Distance diff (%.17g)(%.17g)(%.6g)", a, b, d);
        //BentleyApi::BeAssertFunctions::PerformBeAssert(message, _CRT_WIDE(__FILE__), __LINE__);
        Utf8PrintfString message("Distance diff (%.17g)(%.17g)(%.6g)", a, b, d);
        ROADRAILALIGNMENT_LOGE(message.c_str());
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
    TestHorizontalOrder(horizontalAlignment);

    if (nullptr != pVerticalAlignment && !pVerticalAlignment->empty())
        {
        TestVerticalOrder(*pVerticalAlignment);

        CurveVectorWithDistanceIndexPtr idx = CurveVectorWithDistanceIndex::Create();
        CurveVectorPtr hzCurve = horizontalAlignment.Clone();
        idx->SetPath(*hzCurve);
        double hzLen = idx->TotalPathLength();

        DPoint3d start, end;
        pVerticalAlignment->GetStartEnd(start, end);

        if (!TestLength(hzLen, fabs(end.x)) || !TestLength(0.0, start.x))
            {
            GeometryDebug::Announce(*hzCurve, "Horizontal");
            GeometryDebug::Announce(*pVerticalAlignment, "Vertical");
            double hGap = hzCurve->MaxGapWithinPath();
            double vGap = pVerticalAlignment->MaxGapWithinPath();
            printf(" maxGap (%g,%g)\n", hGap, vGap);
            }
        }
#endif
    }

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
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Scott.Devoe                     09/2015
* factory method
+---------------+---------------+---------------+---------------+---------------+------*/
AlignmentPairPtr AlignmentPair::Create (CurveVectorCR horizontalAlignment, CurveVectorCP pVerticalAlignment)
    { 
    return new AlignmentPair(horizontalAlignment, pVerticalAlignment);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Scott.Devoe                     09/2015
* constructor
+---------------+---------------+---------------+---------------+---------------+------*/
AlignmentPair::AlignmentPair(CurveVectorCR horizontalAlignment, CurveVectorCP pVerticalAlignment) :
    m_hzIndex(nullptr),
    m_vtIndex(nullptr),
    m_vtXIndex(nullptr)
    { 
    m_horizontalCurveVector = horizontalAlignment.Clone ();
    m_verticalCurveVector = (pVerticalAlignment && !pVerticalAlignment->empty()) ? pVerticalAlignment->Clone() : nullptr;

    if (CurveVector::BOUNDARY_TYPE_Open != m_horizontalCurveVector->GetBoundaryType())
        {
        ROADRAILALIGNMENT_LOGW("AlignmentPair constructor - Unexpected boundary type for hz vector. Set to BOUNDARY_TYPE_OPEN");
        m_horizontalCurveVector->SetBoundaryType(CurveVector::BOUNDARY_TYPE_Open);
        }

    if (m_verticalCurveVector.IsValid() && CurveVector::BOUNDARY_TYPE_Open != m_verticalCurveVector->GetBoundaryType())
        {
        ROADRAILALIGNMENT_LOGW("AlignmentPair constructor - Unexpected boundary type for vt vector. Set to BOUNDARY_TYPE_OPEN");
        m_verticalCurveVector->SetBoundaryType(CurveVector::BOUNDARY_TYPE_Open);
        }

#if !defined(NDEBUG)
    TestAlignments(horizontalAlignment, pVerticalAlignment);
#endif
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      01/2016
+---------------+---------------+---------------+---------------+---------------+------*/
AlignmentPairPtr AlignmentPair::Clone() const
    {
    return AlignmentPair::Create(*m_horizontalCurveVector, m_verticalCurveVector.get());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Scott.Devoe                     09/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void AlignmentPair::UpdateCurveVectors(CurveVectorCR horizontalAlignment, CurveVectorCP pVerticalAlignment)
    {
#if !defined(NDEBUG)
    if (!horizontalAlignment.empty())
        TestAlignments(horizontalAlignment, pVerticalAlignment);
#endif

    UpdateHorizontalCurveVector(horizontalAlignment);
    UpdateVerticalCurveVector(pVerticalAlignment);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Scott.Devoe                     09/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void AlignmentPair::UpdateHorizontalCurveVector(CurveVectorCR horizontalAlignment)
    {
    m_horizontalCurveVector = horizontalAlignment.Clone ();
    m_hzIndex = nullptr;

    if (CurveVector::BOUNDARY_TYPE_Open != m_horizontalCurveVector->GetBoundaryType())
        {
        ROADRAILALIGNMENT_LOGW("AlignmentPair UpdateHorizontalCurveVector - Unexpected boundary type. Set to BOUNDARY_TYPE_OPEN");
        m_horizontalCurveVector->SetBoundaryType(CurveVector::BOUNDARY_TYPE_Open);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Scott.Devoe                     09/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void AlignmentPair::UpdateVerticalCurveVector(CurveVectorCP pVerticalAlignment)
    {
    m_verticalCurveVector = (pVerticalAlignment && !pVerticalAlignment->empty()) ? pVerticalAlignment->Clone() : nullptr;
    m_vtIndex = nullptr;
    m_vtXIndex = nullptr;

    if (m_verticalCurveVector.IsValid() && CurveVector::BOUNDARY_TYPE_Open != m_verticalCurveVector->GetBoundaryType())
        {
        ROADRAILALIGNMENT_LOGW("AlignmentPair UpdateVerticalCurveVector - Unexpected boundary type. Set to BOUNDARY_TYPE_OPEN");
        m_verticalCurveVector->SetBoundaryType(CurveVector::BOUNDARY_TYPE_Open);
        }
    }

#if 1 //&&AG NEEDSWORK
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Scott.Devoe                     09/2015
+---------------+---------------+---------------+---------------+---------------+------*/
CurveVectorPtr AlignmentPair::HorizontalCurveVector(Dgn::StandardUnit unit) const
    {
    switch (unit)
        {
        case StandardUnit::MetricMeters:
            return m_horizontalCurveVector;

        case StandardUnit::EnglishFeet:
            return GetConvertedCurveVector(*m_horizontalCurveVector, MetersToEnglishFeet);

        case StandardUnit::EnglishSurveyFeet:
            return GetConvertedCurveVector(*m_horizontalCurveVector, MetersToEnglishSurveyFeet);

        default:
            return nullptr;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Scott.Devoe                     09/2015
+---------------+---------------+---------------+---------------+---------------+------*/
CurveVectorPtr AlignmentPair::VerticalCurveVector(Dgn::StandardUnit unit) const
    {
    if (!m_verticalCurveVector.IsValid())
        return nullptr;

    switch (unit)
        {
        case StandardUnit::MetricMeters:
            return m_verticalCurveVector;

        case StandardUnit::EnglishFeet:
            return GetConvertedCurveVector(*m_verticalCurveVector, MetersToEnglishFeet);

        case StandardUnit::EnglishSurveyFeet:
            return GetConvertedCurveVector(*m_verticalCurveVector, MetersToEnglishSurveyFeet);

        default:
            return nullptr;
        }
    }
#endif


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Scott.Devoe                     09/2015
+---------------+---------------+---------------+---------------+---------------+------*/
CurveVectorWithDistanceIndexPtr AlignmentPair::HorizontalIndexVector() const
    {
    if (!m_hzIndex.IsValid())
        {
        m_hzIndex = CurveVectorWithDistanceIndex::Create();
        m_hzIndex->SetPath(*m_horizontalCurveVector);
        }

    return m_hzIndex;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Scott.Devoe                     09/2015
+---------------+---------------+---------------+---------------+---------------+------*/
CurveVectorWithDistanceIndexPtr AlignmentPair::VerticalIndexVector() const
    {
    if (!m_vtIndex.IsValid() && m_verticalCurveVector.IsValid())
        {
        m_vtIndex = CurveVectorWithDistanceIndex::Create();
        m_vtIndex->SetPath(*m_verticalCurveVector);
        }

    return m_vtIndex;
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
// @bsimethod                           Alexandre.Gagnon                        10/2016
//---------------------------------------------------------------------------------------
CurveVectorPtr AlignmentPair::GetConvertedCurveVector(CurveVectorCR cv, double convFactor) const
    {
    CurveVectorPtr returnVector = cv.Clone();
    Transform trans = Transform::FromScaleFactors(convFactor, convFactor, convFactor);
    TransformCurveWithPartialAlignments(*returnVector, trans);
    return returnVector;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Scott.Devoe                     09/2015
+---------------+---------------+---------------+---------------+---------------+------*/
double AlignmentPair::DistanceAlongFromStart(CurveLocationDetailCR location) const
    {
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
CurveVectorPtr AlignmentPair::GetPartialAlignment(CurveVectorType cvType, double startDistanceAlongFromStart, double endDistanceAlongFromStart) const
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
    if (CurveVectorType::CURVE_VECTOR_Horizontal == cvType)
        {
        result = HorizontalIndexVector()->CloneBetweenDistances(startDistanceAlongFromStart, endDistanceAlongFromStart);
        }
    else
        {
        CurveVectorWithXIndexPtr indexPtr = VerticalXIndexVector();
        if (!indexPtr.IsValid())
            return nullptr;

        DPoint3d start, end;
        m_verticalCurveVector->GetStartEnd(start, end);

        //&&AG we might want to ask Earlin if he could expose the 'usePartialCurves' instead of copying all this code here !!
        ValidatedPathLocationDetail detailA = indexPtr->XToPathLocationDetail(startDistanceAlongFromStart);
        ValidatedPathLocationDetail detailB = indexPtr->XToPathLocationDetail((MIN(end.x, endDistanceAlongFromStart)));
        if (!detailA.IsValid() || !detailB.IsValid())
            return nullptr;

        result = indexPtr->GetCurveVectorPtr()->CloneBetweenDirectedFractions((int)detailA.Value().PathIndex(), detailA.Value().CurveFraction(),
                                                                              (int)detailB.Value().PathIndex(), detailB.Value().CurveFraction(),
                                                                              false, true); // false: allowExtrapolation, true: usePartialCurves

        //! If the end station was a bit greater than the vt length, it will resize the output properly
        // 0.6 is a sdev magic number
        if (endDistanceAlongFromStart > end.x && endDistanceAlongFromStart - end.x < 0.6)
            result->back()->TrySetEnd(DPoint3d::From(endDistanceAlongFromStart, 0.0, end.z));
        }

    //! We might have created partial curves over linestring, line or arc. In theses cases, we can extract the result curve without any side effects.
    if (result.IsValid())
        ExtractPrimitivesFromPartialCurves(*result);

    if (result.IsValid() && isReverse)
        result->ReverseCurvesInPlace();

    return result;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Ben.Bartholomew                 03/2016
+---------------+---------------+---------------+---------------+---------------+------*/
bool AlignmentPair::ClosestPoint(DPoint3dR locationPoint, DPoint3dCR referencePoint, ICurvePrimitive::CurvePrimitiveType* pType) const
    {
    CurveLocationDetail location;
    const DPoint3d atPoint = DPoint3d::From(referencePoint.x, referencePoint.y, 0.0);

    if (!m_horizontalCurveVector->ClosestPointBoundedXY(atPoint, nullptr, location))
        return false;

    locationPoint = location.point;
    const double distanceAlongFromStart = DistanceAlongFromStart(location);
    locationPoint.z = GetVerticalElevationAt(distanceAlongFromStart);
    
    if (nullptr != pType)
        *pType = location.curve->GetCurvePrimitiveType();

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Scott.Devoe                     10/2015
+---------------+---------------+---------------+---------------+---------------+------*/
bool AlignmentPair::ClosestPointXY(DPoint3dR locationPoint, DPoint3dCR referencePoint) const
    {
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
    CurveLocationDetail location;
    const DPoint3d atPoint = DPoint3d::From(referencePoint.x, referencePoint.y, 0.0);

    if (!m_horizontalCurveVector->ClosestPointBoundedXY(atPoint, nullptr, location))
        return false;

    ValidatedDRay3d ray = location.PointAndUnitTangent();
    if (!ray.IsValid())
        return false;

    locationPoint = ray.Value().origin;
    tangent = ray.Value().direction;
    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Scott.Devoe                     09/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void AlignmentPair::GetStartEnd(DPoint3dR startPt, DPoint3dR endPt) const
    {
    // Horizontal is empty
    if (!m_horizontalCurveVector->GetStartEnd(startPt, endPt))
        return;

    startPt.z = 0.0;
    endPt.z = 0.0;

    CurveVectorWithXIndexPtr vtIdx = VerticalXIndexVector();
    if (vtIdx.IsValid())
        {
        const ValidatedPathLocationDetail sDetail = vtIdx->XToPathLocationDetail(0.0);
        const double len = LengthXY();
        const ValidatedPathLocationDetail eDetail = vtIdx->XToPathLocationDetail(len);

        if (sDetail.IsValid())
            startPt.z = sDetail.Value().Point().z;

        if (eDetail.IsValid())
            endPt.z = eDetail.Value().Point().z;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Scott.Devoe                     09/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void AlignmentPair::GetStartAndEndDistancesAlong(double& startStation, double& endStation) const
    {
    startStation = 0.0;
    endStation = LengthXY();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Scott.Devoe                     09/2015
+---------------+---------------+---------------+---------------+---------------+------*/
double AlignmentPair::HorizontalDistanceAlongFromStart(DPoint3dCR referencePoint, double* pOffset) const
    {
    const PathLocationDetail location = HorizontalIndexVector()->SearchClosestPointBounded(referencePoint, true);
    const double distance = location.DistanceFromPathStart();

    if (nullptr != pOffset)
        {
        DVec3d tangent;
        const DPoint3d onPt = location.PointAndUnitTangent(tangent);
        *pOffset = referencePoint.DistanceXY(onPt);

        const DVec3d toPt = DVec3d::FromStartEnd(onPt, referencePoint);
        if (0.0 > toPt.CrossProductXY(tangent)) // left so negate
            *pOffset = *pOffset * -1.0;
        }

    return distance;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sandy.Bugai                     01/2016
+---------------+---------------+---------------+---------------+---------------+------*/
double AlignmentPair::HorizontalDistanceAlongFromEnd(DPoint3dCR referencePoint, double * pOffset) const
    {
    // Get distance from start and then 'invert' the distance to get the end. Offset should be the same
    const double distance = HorizontalDistanceAlongFromStart(referencePoint, pOffset);
    return LengthXY() - distance;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Scott.Devoe                     10/2015
+---------------+---------------+---------------+---------------+---------------+------*/
ICurvePrimitiveCPtr AlignmentPair::GetPrimitiveAtPoint(DPoint3dCR referencePoint) const
    {
    const PathLocationDetail location = HorizontalIndexVector()->SearchClosestPointBounded(referencePoint, true);

    const CurveLocationDetail detail = location.GetCurveLocationDetail();
    if (nullptr == detail.curve)
        return nullptr;

    return detail.curve;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Scott.Devoe                     09/2015
+---------------+---------------+---------------+---------------+---------------+------*/
CurveVectorPtr AlignmentPair::GetPartialHorizontalAlignment(DPoint3dCR fromPt, DPoint3dCR toPt) const
    {
    const double startDistanceAlongFromStart = HorizontalDistanceAlongFromStart(fromPt, nullptr);
    const double endDistanceAlongFromStart = HorizontalDistanceAlongFromStart(toPt, nullptr);

    return GetPartialAlignment(CurveVectorType::CURVE_VECTOR_Horizontal, startDistanceAlongFromStart, endDistanceAlongFromStart);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Scott.Devoe                     09/2015
+---------------+---------------+---------------+---------------+---------------+------*/
CurveVectorPtr AlignmentPair::GetPartialHorizontalAlignment(double startDistanceAlongFromStart, double endDistanceAlongFromStart) const
    {
    return GetPartialAlignment(CurveVectorType::CURVE_VECTOR_Horizontal, startDistanceAlongFromStart, endDistanceAlongFromStart);
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

    if (!m_verticalCurveVector.IsValid())
        return nullptr;

    CurveVectorPtr retVal = GetPartialAlignment(CurveVectorType::CURVE_VECTOR_Vertical, startDistanceAlongFromStart, endDistanceAlongFromStart);

    if (retVal.IsValid() && fabs(0.0 - startDistanceAlongFromStart) > DBL_EPSILON)
        {
        Transform trans = Transform::From(0.0 - startDistanceAlongFromStart, 0.0, 0.0);
        TransformCurveWithPartialAlignments(*retVal, trans);

        DPoint3d start, end;
        retVal->GetStartEnd (start, end);
        TestLength(start.x ,0.0);
        TestVerticalOrder(*retVal);
        
        if (endDistanceAlongFromStart < startDistanceAlongFromStart)
            {
            BeAssert(end.x < start.x);
            Transform xAxisSignTransform = Transform::FromScaleFactors(-1, 1.0, 1.0);
            TransformCurveWithPartialAlignments(*retVal, xAxisSignTransform);
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
    return AlignmentPair::Create(*hz, vt.get());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Scott.Devoe                     09/2015
+---------------+---------------+---------------+---------------+---------------+------*/
double AlignmentPair::LengthXY() const
    {
    return HorizontalIndexVector()->TotalPathLength();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Scott.Devoe                     09/2015
+---------------+---------------+---------------+---------------+---------------+------*/
ValidatedDPoint3d AlignmentPair::GetPointAt(double sta) const
    {
    if (sta > LengthXY())
        sta = sta - mgds_fc_epsilon;

    PathLocationDetail location;
    if (true == HorizontalIndexVector()->SearchByDistanceFromPathStart(sta, location))
        return ValidatedDPoint3d(location.Point(), true);

    return ValidatedDPoint3d(DPoint3d::FromZero(), false);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Scott.Devoe                     10/2015
+---------------+---------------+---------------+---------------+---------------+------*/
ValidatedDPoint3d AlignmentPair::GetPointAtAndOffset(double distanceAlongFromStart, double offset) const
    {
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
ValidatedDPoint3d AlignmentPair::GetPointAtWithZ(double distanceAlongFromStart) const
    {
    ValidatedDPoint3d returnPoint = GetPointAt(distanceAlongFromStart);
    if (returnPoint.IsValid())
        returnPoint.Value().z = GetVerticalElevationAt(distanceAlongFromStart);

    return returnPoint;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Scott.Devoe                     09/2015
+---------------+---------------+---------------+---------------+---------------+------*/
double AlignmentPair::GetVerticalElevationAt(double distanceAlongFromStart) const
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

    // temporary workaround - devoe
	//&&TODO
	//&&AG NEEDSWORK
	//! Make sure this is consistent with the elevation returned by GetStartEnd()
	// (currently, it is NOT)
    DPoint3d start, end;
    m_verticalCurveVector->GetStartEnd(start, end);

    if (distanceAlongFromStart > end.x)
        return end.z;

    if (distanceAlongFromStart < start.x)
        return start.z;

    return 0.0;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Scott.Devoe                     09/2015
+---------------+---------------+---------------+---------------+---------------+------*/
bool AlignmentPair::GetPointAndTangentAt(DPoint3dR locationPoint, DVec3dR hzTangent, double distanceAlongFromStart) const
    {
    if (distanceAlongFromStart > LengthXY())
        distanceAlongFromStart = distanceAlongFromStart - mgds_fc_epsilon;

    PathLocationDetail location;
    if (true == HorizontalIndexVector()->SearchByDistanceFromPathStart(distanceAlongFromStart, location))
        {
        locationPoint = location.PointAndUnitTangent(hzTangent);
        return true;
        }

    return false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Scott.Devoe                     09/2015
+---------------+---------------+---------------+---------------+---------------+------*/
bool AlignmentPair::GetPointAndTangentAtWithZ(DPoint3dR locationPoint, DVec3dR hzTangent, double distanceAlongFromStart) const
    {
    if (!GetPointAndTangentAt(locationPoint, hzTangent, distanceAlongFromStart))
        return false;

    const CurveVectorWithXIndexCPtr vtIdx = VerticalXIndexVector();
    if (vtIdx.IsValid())
        {
        if (distanceAlongFromStart > vtIdx->XRange().high)
            distanceAlongFromStart = distanceAlongFromStart - mgds_fc_epsilon;

        const ValidatedPathLocationDetail location = vtIdx->XToPathLocationDetail(distanceAlongFromStart);
        if (location.IsValid())
            {
            DPoint3d zPoint = location.Value().Point();
            locationPoint.z = zPoint.z;
            }
        }

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

    const CurveVectorWithXIndexPtr zIdx = VerticalXIndexVector();
    if (!zIdx.IsValid())
        {
        m_horizontalCurveVector->AddStrokePoints(strokes, *options);
        ROADRAILALIGNMENT_LOGD("AlignmentPair - GetStrokedAlignment called on an alignment without vertical.");
        }
    else
        {
        bvector<PathLocationDetailPair> locationPairs;
        CurveVectorWithDistanceIndex::StrokeHorizontalAndVerticalCurves(*options, *options, *HorizontalIndexVector(), *zIdx, locationPairs);
        PathLocationDetailPair::Merge(locationPairs, &strokes, nullptr);
        }

    return strokes;
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
void AlignmentPair::TransformCurveWithPartialAlignments(CurveVectorR cv, TransformCR transform)
    {
    for (auto& primitive : cv)
        {
        if (ICurvePrimitive::CURVE_PRIMITIVE_TYPE_PartialCurve == primitive->GetCurvePrimitiveType())
            {
            PartialCurveDetailCP pDetail = primitive->GetPartialCurveDetailCP();
            if (!pDetail->parentCurve->TransformInPlace(transform))
                {
                ROADRAILALIGNMENT_LOGE("AlignmentPair - Failed to transform partial curve with parent of type %i",
                    static_cast<int>(pDetail->parentCurve->GetCurvePrimitiveType()));
                }
            }
        else
            {
            if (!primitive->TransformInPlace(transform))
                {
                ROADRAILALIGNMENT_LOGE("AlignmentPair - Failed to apply transform on primitive");
                }
            }
        }
    }
