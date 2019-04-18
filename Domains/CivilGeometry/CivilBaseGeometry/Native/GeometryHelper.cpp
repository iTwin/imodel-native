/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#include "CivilBaseGeometryInternal.h"
#include <CivilBaseGeometry/GeometryHelper.h>

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Scott.Devoe                     07/2015
+---------------+---------------+---------------+---------------+---------------+------*/
bool GeometryHelper::ClosestPoint(CurveVectorPtr const& hz, DPoint3dCR pickpt, DPoint3dR retPt, DMatrix4dCR localToView)
    {
    CurveLocationDetail location;
    if (hz->ClosestPointBoundedXY (pickpt, &localToView, location) == false)
        return false;

    retPt = location.point;
    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Scott.Devoe                     07/2015
+---------------+---------------+---------------+---------------+---------------+------*/
CurveVectorPtr GeometryHelper::GetPartialAlignment(CurveVectorPtr const& hz, DPoint3dCR fromPt, DPoint3dCR toPt, DMatrix4dCR localToView)
    {
    CurveLocationDetail location;
    if (hz->ClosestPointBoundedXY (fromPt, &localToView, location) == false)
        return nullptr;
    size_t index1 = hz->CurveLocationDetailIndex (location);

    CurveLocationDetail location2;
    if (hz->ClosestPointBoundedXY (toPt, &localToView, location2) == false)
        return nullptr;
    size_t index2 = hz->CurveLocationDetailIndex (location2);

    BeAssert(location.fraction < location2.fraction);
    CurveVectorPtr subGeom = hz->CloneBetweenDirectedFractions ((int)index1, location.fraction, (int)index2, location2.fraction, false);
    return subGeom;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      07/2015
+---------------+---------------+---------------+---------------+---------------+------*/
CurveVectorPtr GeometryHelper::GetPartialAlignment(CurveVectorWithDistanceIndex const& hzIdx, double startStation, double endStation)
    {
    bool needsReverse = false;
    if (startStation > endStation)
        {
        needsReverse = true;
        std::swap(startStation, endStation);
        }

    CurveVectorPtr result = hzIdx.CloneBetweenDistancesXY(startStation, endStation);
    if (needsReverse && result.IsValid())
        result->ReverseCurvesInPlace();

    return result;    
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      07/2015
+---------------+---------------+---------------+---------------+---------------+------*/
CurveVectorPtr GeometryHelper::GetPartialVerticalAlignment(CurveVectorWithDistanceIndex const& vtIdx, double startStation, double endStation, double startStationOnResult)
    {
    CurveVectorPtr retVal = GetPartialAlignment(vtIdx, startStation, endStation);
 
    if (retVal.IsValid() && fabs(startStationOnResult - startStation) > DBL_EPSILON)
        {
        Transform trans = Transform::From(startStationOnResult - startStation, 0.0, 0.0);
        if (!retVal->TransformInPlace(trans))
            return nullptr;
        }
    
    return retVal;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Scott.Devoe                     07/2015
+---------------+---------------+---------------+---------------+---------------+------*/
double GeometryHelper::DistanceFromStart(CurveVectorCR hz, DPoint3dCR atPt, DMatrix4dCP localToView, double * pOffset)
    {
    CurveLocationDetail location;
    if (hz.ClosestPointBoundedXY (atPt, localToView, location) == false)
        return -1.0;
    size_t index = hz.CurveLocationDetailIndex (location);
    if (pOffset != nullptr)
        {
        *pOffset = atPt.DistanceXY (location.point);
        }
    size_t count = 0;
    double distance = 0.0;
    for (auto primitive : hz)
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
double GeometryHelper::DistanceFromStart (CurveVectorWithDistanceIndexPtr const& hzIdx, DPoint3dCR atPt)
    {
    PathLocationDetail location = hzIdx->SearchClosestPointBounded (atPt, true);
    return location.DistanceFromPathStart ();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Scott.Devoe                     07/2015
*
* Given a vertical alignment and a station return the z value
*
+---------------+---------------+---------------+---------------+---------------+------*/
double GeometryHelper::GetVerticalElevationAtStation (CurveVectorCR vtAlign, double sta)
    {
    CurveVectorWithDistanceIndexPtr vtIdx = CurveVectorWithDistanceIndex::Create ();
    CurveVectorPtr vtPtr = vtAlign.Clone();
    vtIdx->SetPath (vtPtr);

    return GetVerticalElevationAtStation(vtIdx, sta);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Stephane.Poulin                 08/2015
+---------------+---------------+---------------+---------------+---------------+------*/
double GeometryHelper::GetVerticalElevationAtStation(CurveVectorWithDistanceIndexPtr const& vtIdx, double sta)
    {
    BeAssert(vtIdx.IsValid());
    BeAssert(sta >= 0.0);

    double length = vtIdx->TotalPathLength ();
    if (sta > length)
        sta = sta - mgds_fc_epsilon; // some fuzz

    BeAssert(sta <= length);

    PathLocationDetail location;
    if (vtIdx->SearchByDistanceFromPathStartXY (sta, location) == true)
        return location.Point ().z;

    return 0.0;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Scott.Devoe                     07/2015
*
* given a horizontal alignment and a station, return the point on the alignment
+---------------+---------------+---------------+---------------+---------------+------*/
DPoint3d GeometryHelper::GetPointFromStation (CurveVectorCR hzAlign, double sta)
    {
    CurveVectorWithDistanceIndexPtr hzIdx = CurveVectorWithDistanceIndex::Create ();
    CurveVectorPtr hzPtr = hzAlign.Clone ();
    hzIdx->SetPath (hzPtr);

    return GetPointFromStation(hzIdx, sta);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Stephane.Poulin                 08/2015
* given a horizontal alignment and a station, return the point on the alignment
+---------------+---------------+---------------+---------------+---------------+------*/
DPoint3d GeometryHelper::GetPointFromStation(CurveVectorWithDistanceIndexPtr const& hzIdx, double sta)
    {
    BeAssert(hzIdx.IsValid());
    BeAssert(sta >= 0.0);

    if (sta > hzIdx->TotalPathLength())
        sta = sta - mgds_fc_epsilon;

    PathLocationDetail location;
    if (hzIdx->SearchByDistanceFromPathStart (sta, location) == true)
        return location.Point ();

    return DPoint3d::FromZero();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Stephane.Poulin                 08/2015
* given a horizontal alignment and a station, return the point and the tangent on the alignment
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus GeometryHelper::GetPointAndTangentFromStation(DPoint3dR point, DVec3dR tangent, CurveVectorWithDistanceIndexPtr const& hzIdx, double sta)
    {
    BeAssert(hzIdx.IsValid());
    BeAssert(sta >= 0.0);

    if (sta > hzIdx->TotalPathLength())
        sta = hzIdx->TotalPathLength ();

    PathLocationDetail location;
    if (hzIdx->SearchByDistanceFromPathStart(sta, location) == true)
        {
        point = location.PointAndUnitTangent(tangent);
        return BSISUCCESS;
        }

    return BSIERROR;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Scott.Devoe                     09/2015
* given horizontal and vertical return the 3d features
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus GeometryHelper::GetPointAndTangentFromStationWithZ(DPoint3dR point, DVec3dR tangent,
        CurveVectorWithDistanceIndexPtr const& hzIdx, CurveVectorWithDistanceIndexPtr const& vtIdx, double sta)
    {
    BeAssert(sta >= 0.0);
    BeAssert((vtIdx->TotalPathLength () + mgds_fc_epsilon) >= sta); // station should not be significantly past the end of the vertical align.

    if (SUCCESS != GeometryHelper::GetPointAndTangentFromStation (point, tangent, hzIdx, sta))
        return ERROR;

    double projectedLength = vtIdx->TotalPathLength ();
    sta = MIN(sta, projectedLength); // make sure we are on the vertical align!

    PathLocationDetail location;
    if (!vtIdx->SearchByDistanceFromPathStartXY (sta, location))
        return ERROR;

    DVec3d zTangent;
    DPoint3d zPoint = location.PointAndUnitTangent (zTangent);

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

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Scott.Devoe                     07/2015
* return the actual length (in x) of the vertical curve vector
+---------------+---------------+---------------+---------------+---------------+------*/
double GeometryHelper::GetVerticalAlignementLength (CurveVectorCR vtAlign)
    { 
    CurveVectorPtr vtPtr = vtAlign.Clone();
    if (vtPtr.IsValid ())
        {
        DPoint3d start, end;
        if (true == vtPtr->GetStartEnd (start, end))
            {
            return fabs (end.x - start.x);
            }
        }
    return 0.0;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Scott.Devoe                     09/2015
+---------------+---------------+---------------+---------------+---------------+------*/
bool GeometryHelper::IsOdd (size_t n)
    {
    return n % 2 == 1;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Scott.Devoe                     09/2015
+---------------+---------------+---------------+---------------+---------------+------*/
bvector<DPoint3d> GeometryHelper::GetStrokedAlignment (CurveVectorPtr hzAlign, CurveVectorPtr vtAlign)
    {
    BeAssert(hzAlign.IsValid());
    BeAssert(vtAlign.IsValid());
    
    IFacetOptionsPtr options = CreateDefaultFacetOptionsForHzAlign(*hzAlign);
    bvector<PathLocationDetailPair> locationPairs = GetStrokedAlignmentLocationPairs(*hzAlign, *vtAlign, *options);

    bvector <DPoint3d> returnPoints;
    bvector<double> stas;
    PathLocationDetailPair::Merge (locationPairs, &returnPoints, &stas);

    return returnPoints;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Stephane.Poulin                 5/2016
+---------------+---------------+---------------+---------------+---------------+------*/
bvector<PathLocationDetailPair> GeometryHelper::GetStrokedAlignmentLocationPairs (CurveVectorR hzAlign, CurveVectorR vtAlign, IFacetOptionsCR options)
    {
    auto searcher = CurveVectorWithDistanceIndex::Create ();
    searcher->SetPath (hzAlign);
    CurveVectorWithXIndexPtr zIdx = CurveVectorWithXIndex::Create (vtAlign);

    bvector<PathLocationDetailPair> locationPairs;
    CurveVectorWithDistanceIndex::StrokeHorizontalAndVerticalCurves (options, options, *searcher, *zIdx, locationPairs);

    return locationPairs;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Stephane.Poulin                 5/2016
+---------------+---------------+---------------+---------------+---------------+------*/
IFacetOptionsPtr GeometryHelper::CreateDefaultFacetOptionsForHzAlign(CurveVectorCR hzAlign)
    {
    IFacetOptionsPtr options = IFacetOptions::CreateForCurves();
    options->SetMaxEdgeLength(0.1 * hzAlign.Length());

    return options;
    }
//---------------------------------------------------------------------------------------
// @bsimethod                            Alexandre.Gagnon                        02/2016
//---------------------------------------------------------------------------------------
bool GeometryHelper::IntersectCurvesXY(bvector<DPoint3d>& intersections, CurveVectorR curve1, CurveVectorR curve2)
    {
    intersections.clear();
    DMatrix4d localToView;
    localToView.InitIdentity();

    CurveVectorPtr intersectionA = CurveVector::Create(CurveVector::BOUNDARY_TYPE_None);
    CurveVectorPtr intersectionB = CurveVector::Create(CurveVector::BOUNDARY_TYPE_None);
    CurveCurve::IntersectionsXY(*intersectionA, *intersectionB, curve1, curve2, &localToView);

    for (ICurvePrimitivePtr primitive : *intersectionA)
        {
        PartialCurveDetailCP partialDetail = primitive->GetPartialCurveDetailCP();

        DPoint3d intersection;
        if (partialDetail->parentCurve->FractionToPoint(partialDetail->fraction0, intersection))
            intersections.push_back(intersection);
        }

    return !intersections.empty();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                            Alexandre.Gagnon                        03/2016
//---------------------------------------------------------------------------------------
CurveVectorPtr GeometryHelper::GetInterpolateOffsetCurve(CurveVectorPtr curve, double startOffset, double endOffset, double interpolateStepDistance)
    {
    CurveVectorWithDistanceIndexPtr curveIdxed = CurveVectorWithDistanceIndex::Create();
    curveIdxed->SetPath(curve);

    const double curveLength = curveIdxed->TotalPathLength();
    size_t numberOfSteps = static_cast<size_t>(curveLength / interpolateStepDistance);
    if (numberOfSteps <= 0)
        numberOfSteps = 1;

    const double effectiveStepDistance = curveLength / numberOfSteps;
    const double offsetStep = (endOffset - startOffset) / numberOfSteps;

    bvector<DPoint3d> rPoints;
    rPoints.reserve(numberOfSteps + 1);

    for (size_t i = 0; i <= numberOfSteps; ++i)
        {
        DPoint3d point;
        DVec3d tangent;
        GetPointAndTangentFromStation(point, tangent, curveIdxed, i * effectiveStepDistance);
        DVec3d normal = DVec3d::From(tangent.y, -tangent.x, 0.0);

        rPoints.push_back(DPoint3d::FromSumOf(point, normal, startOffset + i * offsetStep));
        }

    CurveVectorPtr rCurve = CurveVector::CreateLinear(rPoints);
    return rCurve;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                            Alexandre.Gagnon                        08/2016
//---------------------------------------------------------------------------------------
/*CurveVectorPtr GeometryHelper::CreateVerticalAlignmentFromHorizontalAndDrape(CurveVectorCR hzAlign, ConceptualDrapeCR draper)
    {
    IFacetOptionsPtr options = IFacetOptions::CreateForCurves();
    options->SetMaxEdgeLength(10);

    bvector<DPoint3d> strokePoints;
    hzAlign.AddStrokePoints(strokePoints, *options);

    strokePoints = draper.DrapePointsOnIncludedElements(strokePoints, true);

    bvector<DPoint3d> lineString;
    lineString.reserve(strokePoints.size());

    double distance = 0.0;
    lineString.push_back(DPoint3d::From(distance, 0.0, strokePoints.front().z));

    for (size_t i = 1; i < strokePoints.size(); ++i)
        {
        distance += strokePoints[i - 1].Distance(strokePoints[i]);
        lineString.push_back(DPoint3d::From(distance, 0.0, strokePoints[i].z));
        }
        
    return CurveVector::CreateLinear(lineString);
    }*/

//---------------------------------------------------------------------------------------
// @bsimethod                            Sandy.Bugai                        03/2016
//---------------------------------------------------------------------------------------
Utf8String GeometryHelper::SerializeCurveVector(CurveVectorPtr curveVector)
    {
    Utf8String s;
    BentleyGeometryJson::TryGeometryToJsonString(s, *IGeometry::Create(curveVector));
    return s;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                            Sandy.Bugai                        05/2016
//---------------------------------------------------------------------------------------
CurveVectorPtr GeometryHelper::DeserializeCurveVector(Utf8CP serializedJson)
    {
    bvector<IGeometryPtr> curves;
    BentleyGeometryJson::TryJsonStringToGeometry(serializedJson, curves);

    // Lazy. I only handle one curve right now
    BeAssert(curves.size() == 1);
    if (curves.size() <= 0)
        return nullptr;

    return curves.front()->GetAsCurveVector();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                   Earlin.Lutz                     05/2016
+---------------+---------------+---------------+---------------+---------------+------*/
static void AddOffsetPoint (bvector<DPoint3d> &points, DPoint3dCR xyz, DVec3dCR tangent, double offsetDistance)
    {
    DVec3d perpVector = DVec3d::FromUnitPerpendicularXY (tangent);
    DPoint3d xyzB = xyz - offsetDistance * perpVector;
    if (points.size () > 0 && xyzB.AlmostEqual (points.back ()))
        return;
    points.push_back (xyzB);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Scott.Devoe                     03/2016
+---------------+---------------+---------------+---------------+---------------+------*/
CurveVectorPtr GeometryHelper::CloneOffsetCurvesXYNoBSpline (CurveVectorPtr originalCurve, CurveOffsetOptions options)
    {
    if (originalCurve.IsNull ())
        return nullptr;

    // We've been chaning this boolean a few times and we need to lock down our use cases for it.
    // Set to false due to defect 435578 & 423852 (as examples) which create notches in offset alignments of spirals if we have this set to 'true'. 
    // If we find a case where we need to set it back to true then we need to figure out a way to handle both cases somehow.
    static int s_method = 3;
    if (s_method == 1)
        {
        // Evaluate point and tangent on each original curve with simple distance-along criteria.
        // From point and tangent, compute the offset point.
        // Combine all the offset points into a single linestring.
        static double s_chordLength = 1.0;

        bvector<DPoint3d> points;
        double offsetDistance = options.GetOffsetDistance ();
        //auto result = CurveVector::Create (CurveVector::BOUNDARY_TYPE_Open);
        for (auto &prim : *originalCurve)
            {
            points.clear ();
            DPoint3d xyz0, xyz1, xyz;
            DVec3d tangent0, tangent1, tangent;
            double a;
            if (prim->Length (a))
                {
                prim->GetStartEnd (xyz0, xyz1, tangent0, tangent1);
                int numChord = ( int ) ( 0.9999 + a / s_chordLength );
                if (prim->GetCurvePrimitiveType () == ICurvePrimitive::CURVE_PRIMITIVE_TYPE_Line)
                    numChord = 1;
                if (numChord < 1)
                    numChord = 1;
                AddOffsetPoint (points, xyz0, tangent0, offsetDistance);
                double da = a / ( double ) numChord;
                //double df = 1.0 / (double)numChord; 
                double fraction0 = 0.0;
                for (int i = 1; i < numChord; i++)
                    {
                    CurveLocationDetail location;
                    if (!prim->PointAtSignedDistanceFromFraction (fraction0, da, false, location))
                        break;
                    prim->FractionToPoint (location.fraction, xyz, tangent);
                    AddOffsetPoint (points, xyz, tangent, offsetDistance);
                    fraction0 = location.fraction;
                    }
                AddOffsetPoint (points, xyz1, tangent1, offsetDistance);
                }
            }
        return CurveVector::CreateLinear (points);
        }
    else if (s_method == 3)
        {
        // Offset spirals and bsplines at fixed step size (as in method 1)  Do others through virtuals.
        static double s_chordLength = 1.0;

        bvector<DPoint3d> points;
        double offsetDistance = options.GetOffsetDistance ();
        auto result = CurveVector::Create (CurveVector::BOUNDARY_TYPE_Open);
        for (auto &prim : *originalCurve)
            {
            if (prim->GetCurvePrimitiveType () == ICurvePrimitive::CURVE_PRIMITIVE_TYPE_BsplineCurve)
                {
                points.clear ();
                DPoint3d xyz0, xyz1, xyz;
                DVec3d tangent0, tangent1, tangent;
                double a;
                if (prim->Length (a))
                    {
                    prim->GetStartEnd (xyz0, xyz1, tangent0, tangent1);
                    int numChord = ( int ) ( 0.9999 + a / s_chordLength );
                    if (prim->GetCurvePrimitiveType () == ICurvePrimitive::CURVE_PRIMITIVE_TYPE_Line)
                        numChord = 1;
                    if (numChord < 1)
                        numChord = 1;
                    AddOffsetPoint (points, xyz0, tangent0, offsetDistance);
                    double da = a / ( double ) numChord;
                    //double df = 1.0 / (double)numChord; 
                    double fraction0 = 0.0;
                    for (int i = 1; i < numChord; i++)
                        {
                        CurveLocationDetail location;
                        if (!prim->PointAtSignedDistanceFromFraction (fraction0, da, false, location))
                            break;
                        prim->FractionToPoint (location.fraction, xyz, tangent);
                        AddOffsetPoint (points, xyz, tangent, offsetDistance);
                        fraction0 = location.fraction;
                        }
                    AddOffsetPoint (points, xyz1, tangent1, offsetDistance);
                    }
                result->push_back (ICurvePrimitive::CreateLineString (points));
                }
            else if (prim->GetCurvePrimitiveType () == ICurvePrimitive::CURVE_PRIMITIVE_TYPE_LineString)
                {
                auto points = prim->GetLineStringCP ();
                // uh oh -- linestring does not support CloneAsSingle ....
                // promote to curve vector.
                // do the heavy offset.
                // pull each primitive out.
                if (nullptr != points && points->size () > 1)
                    {
                    auto cv = CurveVector::CreateLinear (*points);
                    auto offset = cv->CloneOffsetCurvesXY (options);
                    if (offset.IsValid ())
                        {
                        for (auto &prim : *offset)
                            result->push_back (prim);
                        }
                    }
                }
            else
                {
                result->push_back (prim->CloneAsSingleOffsetPrimitiveXY (options));
                }
            }
        return result;
        }
    else if (s_method == 2)
        {
        // Scott/Sandy experiment -- stroke spirals back to linestrings
        static bool s_strokeSpirals = true;
        CurveVectorPtr newClone = originalCurve->Clone ();
        CurveVectorPtr cleanCurve = CurveVector::Create (CurveVector::BOUNDARY_TYPE_Open);
        for (auto prim : *newClone)
            {
            if (prim->GetCurvePrimitiveType () == ICurvePrimitive::CURVE_PRIMITIVE_TYPE_Spiral && s_strokeSpirals)
                {
                bvector<DPoint3d> pts;
                IFacetOptionsPtr curveOptions = IFacetOptions::CreateForCurves ();
                curveOptions->SetAngleTolerance (1.0);
                curveOptions->SetChordTolerance (1.0);
                prim->AddStrokes (pts, *curveOptions);
                ICurvePrimitivePtr newPrimitive = ICurvePrimitive::CreateLineString (pts);
                cleanCurve->Add (newPrimitive);
                }
            else
                cleanCurve->Add (prim);
            }
        return cleanCurve->CloneOffsetCurvesXY (options);
        }
    else
        {
        // Use the heavy full-curve offseter.  This tries to make fancy joints between primitives -- probably bad.
        return originalCurve->CloneOffsetCurvesXY (options);
        }
    }

    //=========================================================================================
    // @bsimethod                                              Nicholas.Woodfield    7/2017
    //=========================================================================================
    ClipPlaneContainment GeometryHelper::WhichSide(DPlane3dCR plane, DPoint3dCR pt)
        {
        const double tolerance = 1.0E-8;

        if (plane.Evaluate(pt) < tolerance)
            {
            return ClipPlaneContainment::ClipPlaneContainment_StronglyOutside;
            }
        else
            {
            return ClipPlaneContainment::ClipPlaneContainment_StronglyInside;
            }
        }

    //=========================================================================================
    // @bsimethod                                              Nicholas.Woodfield    7/2017
    //=========================================================================================
    bool GeometryHelper::IntersectRangePlane(DRange3dCR range, DPlane3dCR plane)
        {
        DPoint3d n = range.high;
        DPoint3d p = range.low;

        DVec3d planeNormal = plane.normal;
        planeNormal.Normalize();

        if (planeNormal.x >= 0.0)
            {
            p.x = range.high.x;
            n.x = range.low.x;
            }

        if (planeNormal.y >= 0.0)
            {
            p.y = range.high.y;
            n.y = range.low.y;
            }

        if (planeNormal.y >= 0.0)
            {
            p.z = range.high.z;
            n.z = range.low.z;
            }

        if (WhichSide(plane, n) == ClipPlaneContainment::ClipPlaneContainment_StronglyInside)
            return false;

        if (WhichSide(plane, p) == ClipPlaneContainment::ClipPlaneContainment_StronglyOutside)
            return false;

        return true;
        }

    //=========================================================================================
    // @bsimethod                                              Nicholas.Woodfield    7/2017
    //=========================================================================================
    void GeometryHelper::InitXYRange(DRange3dR range, bvector<DPoint3d> const& points)
        {
        range.InitFrom(points);
        range.low.z = 0.0;
        range.high.z = 0.0;
        }