/*--------------------------------------------------------------------------------------+
|
|     $Source: AlignmentPair.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <RoadRailAlignmentInternal.h>

#define STROKE_PERCENTAGE_OF_LENGTH 0.03
#define HIGHZ 1e+10
static double s_assertTol = 1.0e-4;
bool TestLength (double a, double b)
    {
#if !defined(NDEBUG)
    double d = fabs (b-a);
    if (d > s_assertTol)
        {
        //wchar_t message[1000];
        //swprintf (message, L"(Distance diff (%.17g)(%.17g)(%.6g)", a, b, d);
        //BentleyApi::BeAssertFunctions::PerformBeAssert(message, _CRT_WIDE(__FILE__), __LINE__);
        Utf8PrintfString message("Distance diff (%.17g)(%.17g)(%.6g)", a, b, d);
        ROADRAILALIGNMENT_LOGE(message.c_str());
        //BeAssert (DoubleOps::AlmostEqual (a, b, s_assertTol), message);
        return false;
        }
#endif
    return true;
    }

void TestVerticalOrder(CurveVectorCP verticalAlignment)
    {
#if !defined(NDEBUG)
    double lastX = -DBL_MAX;
    DPoint3d overallStart, overallEnd;
    verticalAlignment->GetStartEnd (overallStart, overallEnd);
    if (overallEnd.x > overallStart.x) // not a reverse alignment
        {
        for (ICurvePrimitivePtr primitive : *verticalAlignment)
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


void Dump (CurveVectorCR cv, const char *message)
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
    

//! Unit conversion
const double AlignmentPair::MetersToEnglishFeet       = 3.28083989501;
const double AlignmentPair::MetersToEnglishSurveyFeet = 3.2808333333465;


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Scott.Devoe                     09/2015
* factory method
+---------------+---------------+---------------+---------------+---------------+------*/
AlignmentPairPtr AlignmentPair::Create (CurveVectorCR horizontalAlignment, CurveVectorCP verticalAlignment)
    { 
#if !defined(NDEBUG)
    if (verticalAlignment != nullptr)
        {
        CurveVectorWithDistanceIndexPtr idx = CurveVectorWithDistanceIndex::Create ();
        CurveVectorPtr hzcurve = horizontalAlignment.Clone ();
            idx->SetPath (hzcurve);
        double hzlen = idx->TotalPathLength ();
        DPoint3d start, end;
        verticalAlignment->GetStartEnd (start, end);
        if (!TestLength (hzlen, fabs(end.x))
            || !TestLength (0.0, start.x))
            {
            GeometryDebug::Announce (*hzcurve, "Horizontal");
            GeometryDebug::Announce (*(const_cast<CurveVectorP> (verticalAlignment)), "Vertical");
            double hGap = hzcurve->MaxGapWithinPath ();
            double vGap = verticalAlignment->MaxGapWithinPath ();
            printf (" maxGap (%g,%g)\n", hGap, vGap);
            }
        }

#endif

    return new AlignmentPair (horizontalAlignment, verticalAlignment);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Scott.Devoe                     09/2015
* constructor
+---------------+---------------+---------------+---------------+---------------+------*/
AlignmentPair::AlignmentPair (CurveVectorCR horizontalAlignment, CurveVectorCP verticalAlignment) : m_hzIndex (nullptr), m_vtIndex (nullptr), m_vtXIndex(nullptr)
    { 
    m_horizontalCurveVector = horizontalAlignment.Clone ();
    m_verticalCurveVector = (verticalAlignment) ? verticalAlignment->Clone() : nullptr;

#if !defined(NDEBUG)
    if (verticalAlignment != nullptr && !horizontalAlignment.empty() && !verticalAlignment->empty())
        {
        DPoint3d start, end;
        m_verticalCurveVector->GetStartEnd(start, end);
        double horizLen = m_horizontalCurveVector->Length();
        double vertLen = fabs(end.x);
        TestLength (horizLen, vertLen);

        TestVerticalOrder(verticalAlignment);
        }
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
void AlignmentPair::UpdateCurveVectors (CurveVectorCP horizontalAlignment, CurveVectorCP verticalAlignment)
    {
#if !defined(NDEBUG)
    if (verticalAlignment != nullptr && horizontalAlignment != nullptr && !horizontalAlignment->empty() && !verticalAlignment->empty())
        {
        DPoint3d start, end;
        verticalAlignment->GetStartEnd(start, end);
        double horizLen = horizontalAlignment->Length();
        double vertLen = fabs(end.x);
        TestLength (horizLen, vertLen);

        TestVerticalOrder(verticalAlignment);
        }
#endif

    m_horizontalCurveVector = horizontalAlignment->Clone();
    m_verticalCurveVector = (verticalAlignment) ? verticalAlignment->Clone() : nullptr;
   
    m_hzIndex = nullptr;
    m_vtIndex = nullptr;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Scott.Devoe                     09/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void AlignmentPair::UpdateHorizontalCurveVector (CurveVectorCP horizontalAlignment)
    {
    m_horizontalCurveVector = horizontalAlignment->Clone ();
    m_hzIndex = nullptr;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Scott.Devoe                     09/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void AlignmentPair::UpdateVerticalCurveVector (CurveVectorCP verticalAlignment)
    {
    m_verticalCurveVector = (verticalAlignment) ? verticalAlignment->Clone() : nullptr;
    m_vtIndex = nullptr;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Scott.Devoe                     09/2015
+---------------+---------------+---------------+---------------+---------------+------*/
const CurveVectorPtr AlignmentPair::HorizontalCurveVector (Dgn::StandardUnit unit) const
    {
    if (unit == StandardUnit::MetricMeters)
        return m_horizontalCurveVector;
    else if (unit == StandardUnit::EnglishFeet)
        return _ConvertedHorizontalVector(MetersToEnglishFeet);
    else if (unit == StandardUnit::EnglishSurveyFeet)
        return _ConvertedHorizontalVector(MetersToEnglishSurveyFeet);
    return nullptr;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Scott.Devoe                     09/2015
+---------------+---------------+---------------+---------------+---------------+------*/
const CurveVectorPtr AlignmentPair::VerticalCurveVector (Dgn::StandardUnit unit) const
    {
    if (unit == StandardUnit::MetricMeters)
        return m_verticalCurveVector;
    else if (unit == StandardUnit::EnglishFeet)
        return _ConvertedVerticalVector(MetersToEnglishFeet);
    else if (unit == StandardUnit::EnglishSurveyFeet)
        return _ConvertedVerticalVector(MetersToEnglishSurveyFeet);
    return nullptr;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Scott.Devoe                     10/2015
+---------------+---------------+---------------+---------------+---------------+------*/
CurveVectorPtr AlignmentPair::_ConvertedHorizontalVector (double convFactor) const
    {
    CurveVectorPtr returnVector = m_horizontalCurveVector->Clone ();
    Transform trans = Transform::FromScaleFactors (convFactor, convFactor, convFactor);
    returnVector->TransformInPlace (trans);
    
    return returnVector;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Scott.Devoe                     10/2015
+---------------+---------------+---------------+---------------+---------------+------*/
CurveVectorPtr AlignmentPair::_ConvertedVerticalVector (double convFactor) const
    {
    CurveVectorPtr returnVector = m_verticalCurveVector->Clone ();
    Transform trans = Transform::FromScaleFactors (convFactor, convFactor, convFactor);
    returnVector->TransformInPlace (trans);

    return returnVector;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Scott.Devoe                     09/2015
+---------------+---------------+---------------+---------------+---------------+------*/
CurveVectorWithDistanceIndexPtr AlignmentPair::_HorizontalIndexVector () const
    {
    if (m_hzIndex == nullptr)
        {
        m_hzIndex = CurveVectorWithDistanceIndex::Create ();
        m_hzIndex->SetPath (const_cast<CurveVectorPtr&>(m_horizontalCurveVector));
        }
    return m_hzIndex;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Scott.Devoe                     09/2015
+---------------+---------------+---------------+---------------+---------------+------*/
CurveVectorWithDistanceIndexPtr AlignmentPair::_VerticalIndexVector () const
    {
    if (m_vtIndex == nullptr && m_verticalCurveVector.IsValid())
        {
        m_vtIndex = CurveVectorWithDistanceIndex::Create ();
        m_vtIndex->SetPath(const_cast<CurveVectorPtr&>(m_verticalCurveVector));
        }
    return m_vtIndex;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Scott.Devoe                     09/2015
+---------------+---------------+---------------+---------------+---------------+------*/
CurveVectorWithXIndexPtr AlignmentPair::_VerticalXIndexVector () const
    {
    if (m_vtIndex == nullptr && m_verticalCurveVector.IsValid())
        {
        m_vtXIndex = CurveVectorWithXIndex::Create (*m_verticalCurveVector);
        }
    return m_vtXIndex;
    }



/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Scott.Devoe                     09/2015
+---------------+---------------+---------------+---------------+---------------+------*/
bool AlignmentPair::ClosestPoint (DPoint3dR locationPoint, DPoint3dCR referencePoint) const
    {
    ICurvePrimitive::CurvePrimitiveType curveType;
    return ClosestPoint(locationPoint, referencePoint, curveType);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Ben.Bartholomew                 03/2016
+---------------+---------------+---------------+---------------+---------------+------*/
bool AlignmentPair::ClosestPoint (DPoint3dR locationPoint, DPoint3dCR referencePoint, ICurvePrimitive::CurvePrimitiveType& curveType) const
    {
    DMatrix4d localToView = DMatrix4d();
    localToView.InitIdentity();

    CurveLocationDetail location;
    DPoint3d atPoint = DPoint3d::From(referencePoint.x, referencePoint.y, 0.0);
    if (!HorizontalCurveVector()->ClosestPointBoundedXY(atPoint, &localToView, location))
        return false;
    locationPoint = location.point;
    double station = _DistanceFromStart(location);
    locationPoint.z = GetVerticalElevationAt(station);
    curveType = location.curve->GetCurvePrimitiveType();
    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Scott.Devoe                     03/2016
+---------------+---------------+---------------+---------------+---------------+------*/
bool AlignmentPair::ClosestPointAndTangentXY (DPoint3dR locationPoint, DPoint3dCR referencePoint, DVec3dR tangent) 
    {
    DMatrix4d localToView = DMatrix4d ();
    localToView.InitIdentity ();

    CurveLocationDetail location;
    DPoint3d atPoint = DPoint3d::From (referencePoint.x, referencePoint.y, 0.0);
    if (!HorizontalCurveVector ()->ClosestPointBoundedXY (atPoint, &localToView, location))
        return false;
    locationPoint = location.point;
    PathLocationDetail locDetail;
    double distanceAlong = HorizontalDistanceFromStart(locationPoint);
    if (_HorizontalIndexVector ()->SearchByDistanceFromPathStart (distanceAlong, locDetail) == true)
        {
        locDetail.PointAndUnitTangent (tangent);
        return true;
        }
    return false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Scott.Devoe                     09/2015
+---------------+---------------+---------------+---------------+---------------+------*/
bool AlignmentPair::ClosestPointXY (DPoint3dR locationPoint, DPoint3dCR referencePoint) const
    {
    ICurvePrimitive::CurvePrimitiveType curveType;
    return ClosestPointXY (locationPoint, referencePoint, curveType);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Scott.Devoe                     10/2015
+---------------+---------------+---------------+---------------+---------------+------*/
bool AlignmentPair::ClosestPointXY (DPoint3dR locationPoint, DPoint3dCR referencePoint, ICurvePrimitive::CurvePrimitiveType& curveType) const
    {
    DMatrix4d localToView = DMatrix4d ();
    localToView.InitIdentity ();

    CurveLocationDetail location;
    DPoint3d atPoint = DPoint3d::From (referencePoint.x, referencePoint.y, 0.0);
    if (!HorizontalCurveVector ()->ClosestPointBoundedXY (atPoint, &localToView, location))
        return false;
    locationPoint = location.point;
    curveType = location.curve->GetCurvePrimitiveType ();
    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Scott.Devoe                     09/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void AlignmentPair::GetStartEnd (DPoint3dR startPt, DPoint3dR endPt) const
    {
    HorizontalCurveVector ()->GetStartEnd (startPt, endPt);
    if (VerticalCurveVector().IsValid())
        {
        DPoint3d sv, ev;
        VerticalCurveVector()->GetStartEnd(sv, ev);
        startPt.z = sv.z;
        endPt.z = ev.z;
        }
    else
        {
        startPt.z = 0.0;
        endPt.z = 0.0;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Scott.Devoe                     09/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void AlignmentPair::GetStartAndEndDistancesAlong(double& startStation, double& endStation) const
    {
    DPoint3d start, end;
    if (VerticalCurveVector().IsValid())
        {
        VerticalCurveVector()->GetStartEnd(start, end);
        startStation = start.x;
        endStation = end.x;
        }
    else
        {
        startStation = 0.0;
        endStation = HorizontalCurveVector()->Length();
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Scott.Devoe                     09/2015
+---------------+---------------+---------------+---------------+---------------+------*/
double AlignmentPair::_DistanceFromStart (CurveLocationDetailCR location) const
    {
    size_t index = HorizontalCurveVector()->CurveLocationDetailIndex (location);
    size_t count = 0;
    double distance = 0.0;
    for (auto primitive : *HorizontalCurveVector())
        {
        double len;
        primitive->Length (len);
        if (index == count)
            {
            distance += location.fraction * len;
            break;
            }
        else
            {
            distance += len;
            }
        count++;
        }

    return distance;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Scott.Devoe                     09/2015
+---------------+---------------+---------------+---------------+---------------+------*/
double AlignmentPair::HorizontalDistanceFromStart (DPoint3dCR referencePoint, double * pOffset) const
    {
    PathLocationDetail location;
    location = _HorizontalIndexVector ()->SearchClosestPointBounded (referencePoint, true);
    double distance = location.DistanceFromPathStart ();
//    _HorizontalIndexVector ()->DistanceFromPathStart (location, distance);

    if (pOffset != nullptr)
        {
        DMatrix4d localToView = DMatrix4d ();
        localToView.InitIdentity ();

        CurveLocationDetail clocation;
        if (HorizontalCurveVector ()->ClosestPointBoundedXY (referencePoint, &localToView, clocation) == false)
            return distance;
        HorizontalCurveVector ()->CurveLocationDetailIndex (clocation);
        *pOffset = referencePoint.DistanceXY (clocation.point);

        DVec3d tangent;
        DPoint3d onPt = location.PointAndUnitTangent (tangent);
        DVec3d toPt = DVec3d::FromStartEnd (onPt, referencePoint);
        if (toPt.CrossProductXY (tangent) < 0.0) // left so negate
            {
            *pOffset = *pOffset * -1.0;
            }
        }
    return distance;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sandy.Bugai                     01/2016
+---------------+---------------+---------------+---------------+---------------+------*/
double AlignmentPair::HorizontalDistanceFromEnd (DPoint3dCR referencePoint, double * pOffset) const
    {
    // Get distance from start and then 'invert' the distance to get the end. Offset should be the same
    double distance = HorizontalDistanceFromStart(referencePoint, pOffset);
    return _HorizontalIndexVector()->TotalPathLength() - distance;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Scott.Devoe                     10/2015
+---------------+---------------+---------------+---------------+---------------+------*/
const ICurvePrimitivePtr AlignmentPair::GetPrimitiveAtPoint (DPoint3dCR referencePoint) const
    {
    PathLocationDetail location;
    location = _HorizontalIndexVector ()->SearchClosestPointBounded (referencePoint, true);

    CurveLocationDetail detail = location.GetCurveLocationDetail ();
    if (detail.curve == nullptr) return nullptr;
    return detail.curve->Clone ();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Scott.Devoe                     09/2015
+---------------+---------------+---------------+---------------+---------------+------*/
CurveVectorPtr AlignmentPair::GetPartialHorizontalAlignment (DPoint3dCR fromPt, DPoint3dCR toPt) const
    {
    DMatrix4d localToView = DMatrix4d ();
    localToView.InitIdentity ();

    CurveLocationDetail location;
    if (HorizontalCurveVector()->ClosestPointBoundedXY (fromPt, &localToView, location) == false)
        return nullptr;
    size_t index1 = HorizontalCurveVector()->CurveLocationDetailIndex (location);

    CurveLocationDetail location2;
    if (HorizontalCurveVector()->ClosestPointBoundedXY (toPt, &localToView, location2) == false)
        return nullptr;
    size_t index2 = HorizontalCurveVector ()->CurveLocationDetailIndex (location2);

    BeAssert(location.fraction < location2.fraction);
    CurveVectorPtr subGeom = HorizontalCurveVector()->CloneBetweenDirectedFractions (( int ) index1, location.fraction, ( int ) index2, location2.fraction, false);

    return subGeom;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Scott.Devoe                     09/2015
+---------------+---------------+---------------+---------------+---------------+------*/
CurveVectorPtr AlignmentPair::_GetPartialAlignment (CurveVectorType cvType, double startStation, double endStation) const
    {
    //! Prevents crashing in Clone() method when both stations are too close to each other
    if (1E-06 > fabs(startStation - endStation))
        return nullptr;

    bool needsReverse = false;
    if (endStation < startStation)
        {
        std::swap(startStation, endStation);
        needsReverse = true;
        }

    CurveVectorPtr result = nullptr;
    if (cvType == CURVE_VECTOR_Horizontal)
        {
        auto indexPtr = _HorizontalIndexVector ();
        if (indexPtr.IsValid())
            result = indexPtr->CloneBetweenDistances(startStation, endStation);
        }
    else
        {
        auto indexPtr = _VerticalXIndexVector ();
        if (indexPtr.IsValid())
            result = indexPtr->CloneDirectedXInterval(startStation, endStation);
        }

    if (result.IsValid() && needsReverse)
        result->ReverseCurvesInPlace();

    return result;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Scott.Devoe                     09/2015
+---------------+---------------+---------------+---------------+---------------+------*/
CurveVectorPtr AlignmentPair::GetPartialHorizontalAlignment (double startStation, double endStation) const
    {
    return _GetPartialAlignment (CurveVectorType::CURVE_VECTOR_Horizontal, startStation, endStation);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Scott.Devoe                     09/2015
+---------------+---------------+---------------+---------------+---------------+------*/
CurveVectorPtr AlignmentPair::GetPartialVerticalAlignment (double startStation, double endStation) const
    { 
#if !defined(NDEBUG)
    if (endStation > LengthXY ())
        TestLength (fabs (LengthXY () - endStation), 0.0);
#endif
    CurveVectorPtr retVal = _GetPartialAlignment (CurveVectorType::CURVE_VECTOR_Vertical, startStation, endStation);

    if (retVal.IsValid () && fabs (0.0 - startStation) > DBL_EPSILON)
        {
        Transform trans = Transform::From (0.0 - startStation, 0.0, 0.0);
        if (!retVal->TransformInPlace (trans))
            return nullptr;
        DPoint3d start, end;
        retVal->GetStartEnd (start, end);
        TestLength(start.x ,0.0);
        TestVerticalOrder(retVal.get());
        }

    return retVal;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Scott.Devoe                     09/2015
+---------------+---------------+---------------+---------------+---------------+------*/
AlignmentPairPtr AlignmentPair::GetPartialAlignment (DPoint3dCR fromPt, DPoint3dCR toPt) const
    {
    double startStation = HorizontalDistanceFromStart (fromPt);
    double endStation = HorizontalDistanceFromStart (toPt);
    return GetPartialAlignment (startStation, endStation);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Scott.Devoe                     09/2015
+---------------+---------------+---------------+---------------+---------------+------*/
AlignmentPairPtr AlignmentPair::GetPartialAlignment (double startStation, double endStation) const
    {
    CurveVectorPtr hz = GetPartialHorizontalAlignment (startStation, endStation);
    if (!hz.IsValid ()) return nullptr;
    CurveVectorPtr vt = GetPartialVerticalAlignment (startStation, endStation);
    return AlignmentPair::Create (*hz, vt.get());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Scott.Devoe                     09/2015
+---------------+---------------+---------------+---------------+---------------+------*/
double AlignmentPair::LengthXY () const
    {
    return _HorizontalIndexVector ()->TotalPathLength ();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Scott.Devoe                     09/2015
+---------------+---------------+---------------+---------------+---------------+------*/
DPoint3d AlignmentPair::GetPointAt (double sta) const
    {
    if (sta > _HorizontalIndexVector ()->TotalPathLength ())
        sta = sta - mgds_fc_epsilon;

    PathLocationDetail location;
    if (_HorizontalIndexVector ()->SearchByDistanceFromPathStart (sta, location) == true)
        return location.Point ();
    return DPoint3d::FromZero ();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Scott.Devoe                     10/2015
+---------------+---------------+---------------+---------------+---------------+------*/
DPoint3d AlignmentPair::GetPointAtAndOffset (double sta, double offset) const
    {
    if (sta > _HorizontalIndexVector ()->TotalPathLength ())
        sta = sta - mgds_fc_epsilon;

    PathLocationDetail location;
    if (_HorizontalIndexVector ()->SearchByDistanceFromPathStart (sta, location) == true)
        {
        DVec3d tangent;
        DPoint3d onPt = location.PointAndUnitTangent (tangent);
        DVec3d perpVec;
        perpVec.UnitPerpendicularXY (tangent);
        if (offset > 0.0)
            perpVec.Negate ();
        RotMatrix mat = RotMatrix ();
        mat.InitIdentity ();
        perpVec.Scale (fabs(offset));
        return DPoint3d::FromProduct (onPt, mat, perpVec);
        }
    return DPoint3d::FromZero ();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Scott.Devoe                     09/2015
+---------------+---------------+---------------+---------------+---------------+------*/
double AlignmentPair::GetVerticalElevationAt (double sta) const
    {
    auto indexVector = _VerticalXIndexVector ();
    if (indexVector.IsNull())
        return 0.0;
    auto xRange = indexVector->XRange ();
    double length = xRange.High ();
    if (sta > length)
        sta = sta - mgds_fc_epsilon; // some fuzz

    BeAssert (sta <= length);

    auto location = indexVector->XToPathLocationDetail (sta);
    if (location.IsValid())
        return location.Value ().Point ().z;

    return 0.0;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Scott.Devoe                     09/2015
+---------------+---------------+---------------+---------------+---------------+------*/
DPoint3d AlignmentPair::GetPointAtWithZ (double sta) const
    {
    DPoint3d returnPoint = GetPointAt (sta);
    returnPoint.z = GetVerticalElevationAt (sta);
    return returnPoint;
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Scott.Devoe                     09/2015
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus AlignmentPair::GetPointAndTangentAt (DPoint3dR point, DVec3dR tangent, double sta) const
    {
    if (sta > _HorizontalIndexVector()->TotalPathLength ())
        sta = sta - mgds_fc_epsilon;

    PathLocationDetail location;
    if (_HorizontalIndexVector()->SearchByDistanceFromPathStart (sta, location) == true)
        {
        point = location.PointAndUnitTangent (tangent);
        return BSISUCCESS;
        }

    return BSIERROR;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Scott.Devoe                     09/2015
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus AlignmentPair::GetPointAndTangentAtWithZ (DPoint3dR point, DVec3dR tangent, double sta) const
    {
    auto verticalIndex = _VerticalXIndexVector ();
    if (verticalIndex.IsNull())
        return BSIERROR;

    BentleyStatus status = BSISUCCESS;
    if (( status = GetPointAndTangentAt (point, tangent, sta) ) != BSISUCCESS)
        {
        return status;
        }

    auto location = verticalIndex->XToPathLocationDetail(sta);
    if (!location.IsValid ())
        return BSIERROR;

    DVec3d zTangent;
    DPoint3d zPoint = location.Value ().PointAndUnitTangent (zTangent);

    point.z = zPoint.z;

    if (!DoubleOps::AlmostEqual(zTangent.x, 0.0))
        {
        BeAssert(DoubleOps::AlmostEqual(tangent.MagnitudeXY(), 1.0));
        tangent.z = zTangent.z / zTangent.x;

        // validate that we have the same slope
        BeAssert(DoubleOps::AlmostEqual(zTangent.z / zTangent.x, tangent.z / tangent.MagnitudeXY()));
        }
    else
        {
        tangent.z = 0.0;
        }

    return BSISUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Scott.Devoe                     09/2015
+---------------+---------------+---------------+---------------+---------------+------*/
bvector<DPoint3d> AlignmentPair::GetStrokedAlignment () const
    {
    bvector <DPoint3d> returnPoints;
    if (VerticalCurveVector().IsNull())
        return returnPoints;

    double maxEdgeLength = STROKE_PERCENTAGE_OF_LENGTH * HorizontalCurveVector ()->Length ();
    IFacetOptionsPtr options = IFacetOptions::CreateForCurves ();
    options->SetMaxEdgeLength (maxEdgeLength);
    options->SetAngleTolerance(.05); //In radians
    options->SetChordTolerance(1);

    bvector<PathLocationDetailPair> locationPairs;
    CurveVectorWithXIndexPtr zIdx = CurveVectorWithXIndex::Create (*VerticalCurveVector());
    if (!zIdx.IsValid ()) return returnPoints;
    CurveVectorWithDistanceIndex::StrokeHorizontalAndVerticalCurves (*options, *options, *_HorizontalIndexVector(), *zIdx, locationPairs);
    bvector<double> stas;
    PathLocationDetailPair::Merge (locationPairs, &returnPoints, &stas);

    return returnPoints;
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
    
