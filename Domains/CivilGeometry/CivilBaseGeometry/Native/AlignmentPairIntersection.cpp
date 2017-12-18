/*--------------------------------------------------------------------------------------+
|
|     $Source: CivilBaseGeometry/Native/AlignmentPairIntersection.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "CivilBaseGeometryInternal.h"
#include <CivilBaseGeometry/AlignmentPairIntersection.h>

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Earlin.Lutz                     01/2016
+---------------+---------------+---------------+---------------+---------------+------*/
ValidatedDEllipse3d AlignmentPairIntersection::_ArcFromStartShoulderTarget
(
DPoint3dCR pointA, // Start point of arc
DPoint3dCR pointB, // shoulder point
DPoint3dCR pointC  // target point for outbound tangent
)
    {
    DVec3d vectorAB = DVec3d::FromStartEnd (pointA, pointB);
    DVec3d vectorBC = DVec3d::FromStartEnd (pointB, pointC);
    double dab = pointA.Distance (pointB);
    auto unitNormal = DVec3d::FromCrossProduct (vectorAB, vectorBC).ValidatedNormalize ();
    if (!unitNormal.IsValid ())
        return ValidatedDEllipse3d ();  // That will be invalid 
    auto unitPerp = DVec3d::FromCrossProduct (unitNormal, vectorAB).ValidatedNormalize ();
    if (!unitPerp.IsValid ())
        return ValidatedDEllipse3d ();  // That will be invalid 

    double beta = vectorAB.AngleTo (vectorBC);      // positive angle as viewed from cross product upwards.
    double alpha = 0.5 * beta;
    double radius = dab / tan (alpha);
    DPoint3d center = pointA + unitPerp * radius;
    DVec3d vector0 = pointA - center;
    DVec3d vector90 = DVec3d::FromCrossProduct (unitNormal, vector0);   // cross product with unit normal preserves length
    return ValidatedDEllipse3d (DEllipse3d::FromVectors (center, vector0, vector90, 0.0, beta), true);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Earlin.Lutz                     01/2016
+---------------+---------------+---------------+---------------+---------------+------*/
CurveVectorPtr AlignmentPairIntersection::ConstructDoubleFillet (
    DPoint3dCR pointA,
    DVec3dCR directionA,
    double distanceA,
    DPoint3dCR pointB,
    DVec3dCR directionB,
    double distanceB
    )
    {
    auto unitA = directionA.ValidatedNormalize ();
    auto unitB = directionB.ValidatedNormalize ();
    DPoint3d shoulderA = pointA + distanceA * unitA;
    DPoint3d shoulderB = pointB + distanceB * unitB;
    auto arcA = AlignmentPairIntersection::_ArcFromStartShoulderTarget (pointA, shoulderA, shoulderB);
    auto arcB = AlignmentPairIntersection::_ArcFromStartShoulderTarget (pointB, shoulderB, shoulderA);
    DPoint3d pointA2 = pointA;
    DPoint3d pointB2 = pointB;
    if (arcA.IsValid ())
        arcA.Value ().FractionParameterToPoint (pointA2, 1.0);
    if (arcB.IsValid ())
        arcB.Value ().FractionParameterToPoint (pointB2, 1.0);

    auto curves = CurveVector::Create (CurveVector::BOUNDARY_TYPE_Open);
    if (arcA.IsValid ())
        curves->push_back (ICurvePrimitive::CreateArc (arcA.Value ()));
    if (!pointA2.AlmostEqual (pointB2))
        curves->push_back (ICurvePrimitive::CreateLine (DSegment3d::From (pointA2, pointB2)));
    if (arcB.IsValid ())
        curves->push_back (ICurvePrimitive::CreateArc (DEllipse3d::FromReversed (arcB.Value ())));
    return curves;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Scott.Devoe                     10/2015
+---------------+---------------+---------------+---------------+---------------+------*/
bool AlignmentPairIntersection::ComputePotentialFillets (const double& primaryOffset, const double & secondaryOffset, const double& radius, bvector<CurveCurve::FilletDetail> &arcs)
    {
    CurveCurve::CollectFilletArcs (m_primaryRoad->GetHorizontalCurveVector(), m_secondaryRoad->GetHorizontalCurveVector(), radius, false, arcs);
    if (arcs.empty())
        return false;

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Scott.Devoe                     02/2016
+---------------+---------------+---------------+---------------+---------------+------*/
void AlignmentPairIntersection::SetPrimaryOffsets (const double& right, const double& left)
    {
    m_primaryLeftOffset = left;
    m_primaryRightOffset = right;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Scott.Devoe                     02/2016
+---------------+---------------+---------------+---------------+---------------+------*/
void AlignmentPairIntersection::SetSecondaryOffsets (const double& right, const double& left)
    {
    m_secondaryLeftOffset = left;
    m_secondaryRightOffset = right;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Scott.Devoe                     12/2015
+---------------+---------------+---------------+---------------+---------------+------*/
bool AlignmentPairIntersection::ComputeIntersectionPointImpl (DPoint3dR pt, DPoint3dCP checkPoint, double * primaryStation, double * secondaryStation, bool allowExtension)
    {
    AlignmentIntersectionPtr alignmentIntersection = AlignmentIntersection::Create (m_primaryRoad.get (), m_secondaryRoad.get ());
    if (allowExtension) // this is now vague
        {
        alignmentIntersection->SetAllowExtendPrimaryEnd (true);
        alignmentIntersection->SetAllowExtendPrimaryStart ( true );
        alignmentIntersection->SetAllowExtendSecondaryEnd (true);
        alignmentIntersection->SetAllowExtendSecondaryStart (true);
        }
    AlignmentIntersectionInfo info;
    if (checkPoint != nullptr)
        {
        if (!alignmentIntersection->ComputeClosest (*checkPoint, info))
            return false;

        *primaryStation = info.PrimaryStation ();
        *secondaryStation = info.SecondaryStation ();
        pt = info.Point ();
        return true;
        }

    // just grab the first intersection (if available) if caller didn't specify a checkPoint
    bvector<AlignmentIntersectionInfo> potentials;
    if (!alignmentIntersection->Compute(potentials) || potentials.empty())
        return false;

    *primaryStation = potentials.front().PrimaryStation();
    *secondaryStation = potentials.front().SecondaryStation();
    pt = potentials.front().Point();
    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Scott.Devoe                     03/2016
+---------------+---------------+---------------+---------------+---------------+------*/
void AlignmentPairIntersection::UpdateSecondaryRoad (CurveVectorCR hz, CurveVectorCP vt)
    {
    if (m_secondaryRoad.IsValid ())
        m_secondaryRoad->UpdateCurveVectors (hz, vt);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sandy.Bugai                     02/2016
+---------------+---------------+---------------+---------------+---------------+------*/
bool AlignmentPairIntersection::ComputeIntersectionPoint(DPoint3dR pt, double * primaryStation, double * secondaryStation, bool allowExtension)
    {
    return ComputeIntersectionPointImpl(pt, nullptr, primaryStation, secondaryStation, allowExtension);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Scott.Devoe                     12/2015
+---------------+---------------+---------------+---------------+---------------+------*/
bool AlignmentPairIntersection::ComputeOffsetFillet (const double& offsetDist, const double& radius, bvector<CurveCurve::FilletDetail> &arcs)
    {
    CurveVectorCR primaryHZ = m_primaryRoad->GetHorizontalCurveVector();
    CurveVectorCR secondaryHZ = m_secondaryRoad->GetHorizontalCurveVector();

    CurveOffsetOptions offsetOptions (offsetDist);
    CurveVectorPtr offsetCurve = primaryHZ.CloneOffsetCurvesXY(offsetOptions); // need to figure out left and right

    if (offsetCurve.IsValid())
        {
        CurveCurve::CollectFilletArcs(*offsetCurve, secondaryHZ, radius, false, arcs);
        if (!arcs.empty())
            return true;
        }

    return false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Scott.Devoe                     10/2015
+---------------+---------------+---------------+---------------+---------------+------*/
AlignmentPairIntersectionPtr AlignmentPairIntersection::Create (AlignmentPairCP primary, AlignmentPairCP secondary)
    {
    return new AlignmentPairIntersection (const_cast<AlignmentPairP>( primary ), const_cast<AlignmentPairP>( secondary ));
    }

/////////////////////////////////////////////////////////////////////////////////////
// Alignment Intersection

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Scott.Devoe                     02/2016
+---------------+---------------+---------------+---------------+---------------+------*/
size_t AlignmentIntersection::_ComputeExplicitIntersections (bvector<AlignmentIntersectionInfo>& potentialIntersections)
    { 
    CurveVectorCR primaryHZ = m_primaryRoad->GetHorizontalCurveVector();
    CurveVectorCR secondaryHZ = m_secondaryRoad->GetHorizontalCurveVector();

    CurveVectorPtr intersectionA = CurveVector::Create (CurveVector::BOUNDARY_TYPE_None);
    CurveVectorPtr intersectionB = CurveVector::Create (CurveVector::BOUNDARY_TYPE_None);
    CurveCurve::IntersectionsXY(*intersectionA, *intersectionB, const_cast<CurveVectorR>(primaryHZ), const_cast<CurveVectorR>(secondaryHZ), nullptr);
    for (int i = 0; i < intersectionA->size (); i++)
        {
        DPoint3d pointA, pointB;
        double fractionA, fractionB;
        CurveLocationDetail detailA, detailB;
        if (CurveCurve::IsSinglePointPair (*intersectionA, *intersectionB, i, fractionA, pointA, fractionB, pointB))
            {
            AlignmentIntersectionInfo info;
            ICurvePrimitivePtr primitive = intersectionA->at (i);
            if (primitive.IsNull ()) 
                continue;

            primitive->GetStartPoint (info.m_point);
            if (_ComputeStations ())
                {
                info.m_primaryStation = m_primaryRoad->HorizontalDistanceAlongFromStart (info.m_point);
                info.m_secondaryStation = m_secondaryRoad->HorizontalDistanceAlongFromStart(info.m_point);

                DPoint3d dummy;
                DVec3d tangent;
                m_secondaryRoad->GetPointAndTangentAt(dummy, tangent, info.m_secondaryStation);
                // tangent is already normalized in this case
                info.m_secondaryTangentAtIntersect = tangent;
                }
            if (_ComputeZDeltas ())
                {
                // SDEVTODO
                }
            potentialIntersections.push_back (info);
            }
        }

    //&&AG checks for intersections between curve endpoints
    if (m_computeIntersectionsAtAlignmentEndPoints && potentialIntersections.empty())
        {
        DPoint3d primStart, primEnd, secStart, secEnd;
        primaryHZ.GetStartEnd(primStart, primEnd);
        secondaryHZ.GetStartEnd(secStart, secEnd);

        bool hit = true;
        bool isPrimStart = false;
        bool isSecStart = false;

        if (primStart.AlmostEqualXY(secStart))
            {
            isPrimStart = true;
            isSecStart = true;
            }
        else if (primStart.AlmostEqualXY(secEnd))
            {
            isPrimStart = true;
            isSecStart = false;
            }
        else if (primEnd.AlmostEqualXY(secStart))
            {
            isPrimStart = false;
            isSecStart = true;
            }
        else if (primEnd.AlmostEqualXY(secEnd))
            {
            isPrimStart = false;
            isSecStart = false;
            }
        else
            hit = false;

        if (hit)
            {
            AlignmentIntersectionInfo info;
            info.m_point = (isPrimStart) ? primStart : primEnd;
            info.m_primaryStation = isPrimStart ? 0.0 : m_primaryRoad->LengthXY();
            info.m_secondaryStation = isSecStart ? 0.0 : m_secondaryRoad->LengthXY();

            DPoint3d dummy;
            m_secondaryRoad->GetPointAndTangentAt(dummy, info.m_secondaryTangentAtIntersect, info.m_secondaryStation);
            potentialIntersections.push_back(info);
            }
        }


    return potentialIntersections.size ();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Scott.Devoe                     02/2016
+---------------+---------------+---------------+---------------+---------------+------*/
size_t AlignmentIntersection::_ComputeExtendedIntersections (bvector<AlignmentIntersectionInfo>& potentialExtendedIntersections)
    {
    CurveVectorCR primaryHz = m_primaryRoad->GetHorizontalCurveVector();
    CurveVectorCR secondaryHz = m_secondaryRoad->GetHorizontalCurveVector();
    if (primaryHz.empty() || secondaryHz.empty())
        return 0;

    ICurvePrimitivePtr primitivePStart = primaryHz.front();
    ICurvePrimitivePtr primitivePEnd = primaryHz.back();
    ICurvePrimitivePtr primitiveSStart = secondaryHz.front();
    ICurvePrimitivePtr primitiveSEnd = secondaryHz.back();
    if (primitivePStart.IsNull () || primitivePEnd.IsNull () || primitiveSStart.IsNull () || primitiveSEnd.IsNull ())
        return 0;

    if (_ExtendPrimaryStart ())
        _ComputePrimaryStartProjection (*primitivePStart, potentialExtendedIntersections);
    if (_ExtendSecondaryStart ())
        _ComputeSecondaryStartProjection (*primitiveSStart, potentialExtendedIntersections);
    if (_ExtendPrimaryEnd ())
        _ComputePrimaryEndProjection (*primitivePEnd, potentialExtendedIntersections);
    if (_ExtendSecondaryEnd ())
        _ComputeSecondaryEndProjection (*primitiveSEnd, potentialExtendedIntersections);

    return potentialExtendedIntersections.size();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Scott.Devoe                     02/2016
+---------------+---------------+---------------+---------------+---------------+------*/
size_t AlignmentIntersection::_ComputePrimaryEndProjection (ICurvePrimitiveR primitiveToProject, bvector<AlignmentIntersectionInfo>& potentialExtendedIntersections)
    {
    for (auto testPrimitive : m_secondaryRoad->GetHorizontalCurveVector())
        {
        CurveVectorPtr intersectionA = CurveVector::Create (CurveVector::BOUNDARY_TYPE_None);
        CurveVectorPtr intersectionB = CurveVector::Create (CurveVector::BOUNDARY_TYPE_None);
        CurveCurve::IntersectionsXY (*intersectionA, *intersectionB, &primitiveToProject, testPrimitive.get (), nullptr, true);
        for (size_t i = 0; i < intersectionA->size (); i++)
            {
            DPoint3d pt;
            double fractionOnProjected, fractionOnTest;
            DPoint3d intersectionPt;
            if (CurveCurve::IsSinglePointPair (*intersectionA, *intersectionB, i, fractionOnProjected, pt,
                fractionOnTest, intersectionPt))
                {
                if (fractionOnTest > 1.0 || fractionOnTest < 0.0) // not on
                    continue;
                if (DoubleOps::ClearlyIncreasingFraction(fractionOnProjected, 0.99))     // only testing end
                    continue;
                AlignmentIntersectionInfo info;
                info.m_point = pt;
                info.m_extensionType = AlignmentIntersectionInfo::ExtensionType::PrimaryEnd;
                if (_ComputeStations ())
                    {
                    double initLength;
                    primitiveToProject.Length (initLength);
                    double extension = (initLength * fractionOnProjected) - initLength;
                    info.m_primaryStation = m_primaryRoad->LengthXY () + extension;
                    info.m_secondaryStation = m_secondaryRoad->HorizontalDistanceAlongFromStart(pt);
                    info.m_extendedAmount = extension;

                    DPoint3d dummy;
                    DVec3d tangent;
                    testPrimitive->ComponentFractionToPoint(0, fractionOnTest, dummy, tangent);
                    tangent.Normalize();
                    info.m_secondaryTangentAtIntersect = tangent;
                    }
                potentialExtendedIntersections.push_back (info);
                }
            }
        }
    return potentialExtendedIntersections.size ();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Scott.Devoe                     02/2016
+---------------+---------------+---------------+---------------+---------------+------*/
size_t AlignmentIntersection::_ComputePrimaryStartProjection (ICurvePrimitiveR primitiveToProject, bvector<AlignmentIntersectionInfo>& potentialExtendedIntersections)
    {
    for (auto testPrimitive : m_secondaryRoad->GetHorizontalCurveVector())
        {
        CurveVectorPtr intersectionA = CurveVector::Create (CurveVector::BOUNDARY_TYPE_None);
        CurveVectorPtr intersectionB = CurveVector::Create (CurveVector::BOUNDARY_TYPE_None);
        CurveCurve::IntersectionsXY (*intersectionA, *intersectionB, &primitiveToProject, testPrimitive.get (), nullptr, true);
        for (size_t i = 0; i < intersectionA->size (); i++)
            {
            DPoint3d pt;
            double fractionOnProjected, fractionOnTest;
            DPoint3d intersectionPt;
            if (CurveCurve::IsSinglePointPair (*intersectionA, *intersectionB, i, fractionOnProjected, pt,
                fractionOnTest, intersectionPt))
                {
                if (fractionOnTest > 1.0 || fractionOnTest < 0.0) // not on
                    continue;
                if (DoubleOps::ClearlyIncreasingFraction(0.01, fractionOnProjected))     // only testing start
                    continue;

                AlignmentIntersectionInfo info;
                info.m_point = pt;
                info.m_extensionType = AlignmentIntersectionInfo::ExtensionType::PrimaryStart;
                if (_ComputeStations ())
                    {
                    double initLength;
                    primitiveToProject.Length (initLength);
                    double extension = initLength * fractionOnProjected;
                    info.m_primaryStation = 0.0 + extension;
                    info.m_secondaryStation = m_secondaryRoad->HorizontalDistanceAlongFromStart(pt);
                    info.m_extendedAmount = extension;

                    DPoint3d dummy;
                    DVec3d tangent;
                    testPrimitive->ComponentFractionToPoint(0, fractionOnTest, dummy, tangent);
                    tangent.Normalize();
                    info.m_secondaryTangentAtIntersect = tangent;
                    }
                potentialExtendedIntersections.push_back (info);
                }
            }
        }
    return potentialExtendedIntersections.size ();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Scott.Devoe                     02/2016
+---------------+---------------+---------------+---------------+---------------+------*/
size_t AlignmentIntersection::_ComputeSecondaryStartProjection (ICurvePrimitiveR primitiveToProject, bvector<AlignmentIntersectionInfo>& potentialExtendedIntersections)
    {
    for (auto testPrimitive : m_primaryRoad->GetHorizontalCurveVector())
        {
        CurveVectorPtr intersectionA = CurveVector::Create (CurveVector::BOUNDARY_TYPE_None);
        CurveVectorPtr intersectionB = CurveVector::Create (CurveVector::BOUNDARY_TYPE_None);
        CurveCurve::IntersectionsXY(*intersectionA, *intersectionB, &primitiveToProject, testPrimitive.get(), nullptr, true);
        for (size_t i = 0; i < intersectionA->size (); i++)
            {
            DPoint3d pt;
            double fractionOnProjected, fractionOnTest;
            DPoint3d intersectionPt;
            if (CurveCurve::IsSinglePointPair (*intersectionA, *intersectionB, i, fractionOnProjected, pt,
                fractionOnTest, intersectionPt))
                {
                if (fractionOnTest > 1.0 || fractionOnTest < 0.0) // not on
                    continue;
                if (DoubleOps::ClearlyIncreasingFraction(0.01, fractionOnProjected))     // only testing start
                    continue;

                AlignmentIntersectionInfo info;
                info.m_point = pt;
                info.m_extensionType = AlignmentIntersectionInfo::ExtensionType::SecondaryStart;
                if (_ComputeStations ())
                    {
                    double initLength;
                    primitiveToProject.Length (initLength);
                    double extension = initLength * fractionOnProjected;
                    info.m_secondaryStation = 0.0 + extension;
                    info.m_primaryStation = m_primaryRoad->HorizontalDistanceAlongFromStart(pt);
                    info.m_extendedAmount = extension;

                    DPoint3d dummy;
                    DVec3d tangent;
                    primitiveToProject.ComponentFractionToPoint(0, fractionOnProjected, dummy, tangent);
                    tangent.Normalize();
                    info.m_secondaryTangentAtIntersect = tangent;
                    }
                potentialExtendedIntersections.push_back (info);
                }
            }
        }
    return potentialExtendedIntersections.size ();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Scott.Devoe                     02/2016
+---------------+---------------+---------------+---------------+---------------+------*/
size_t AlignmentIntersection::_ComputeSecondaryEndProjection (ICurvePrimitiveR primitiveToProject, bvector<AlignmentIntersectionInfo>& potentialExtendedIntersections)
    {
    for (auto testPrimitive : m_primaryRoad->GetHorizontalCurveVector())
        {
        CurveVectorPtr intersectionA = CurveVector::Create (CurveVector::BOUNDARY_TYPE_None);
        CurveVectorPtr intersectionB = CurveVector::Create (CurveVector::BOUNDARY_TYPE_None);
        CurveCurve::IntersectionsXY (*intersectionA, *intersectionB, &primitiveToProject, testPrimitive.get (), nullptr, true);
        for (size_t i = 0; i < intersectionA->size (); i++)
            {
            DPoint3d pt;
            double fractionOnProjected, fractionOnTest;
            DPoint3d intersectionPt;
            if (CurveCurve::IsSinglePointPair (*intersectionA, *intersectionB, i, fractionOnProjected, pt,
                fractionOnTest, intersectionPt))
                {
                if (fractionOnTest > 1.0 || fractionOnTest < 0.0) // not on
                    continue;
                if (DoubleOps::ClearlyIncreasingFraction(fractionOnProjected, 0.99))     // only testing end
                    continue;

                AlignmentIntersectionInfo info;
                info.m_point = pt;
                info.m_extensionType = AlignmentIntersectionInfo::ExtensionType::SecondaryEnd;
                if (_ComputeStations ())
                    {
                    double initLength;
                    primitiveToProject.Length (initLength);
                    double extension = ( initLength * fractionOnProjected ) - initLength;
                    info.m_secondaryStation = m_secondaryRoad->LengthXY () + extension;
                    info.m_primaryStation = m_primaryRoad->HorizontalDistanceAlongFromStart(pt);
                    info.m_extendedAmount = extension;

                    DPoint3d dummy;
                    DVec3d tangent;
                    primitiveToProject.ComponentFractionToPoint(0, fractionOnProjected, dummy, tangent);
                    tangent.Normalize();
                    info.m_secondaryTangentAtIntersect = tangent;
                    }
                potentialExtendedIntersections.push_back (info);
                }
            }
        }
    return potentialExtendedIntersections.size ();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Scott.Devoe                     02/2016
+---------------+---------------+---------------+---------------+---------------+------*/
size_t AlignmentIntersection::Compute (bvector<AlignmentIntersectionInfo>& potentialIntersections)
    {
    _ComputeExplicitIntersections (potentialIntersections);
    bvector<AlignmentIntersectionInfo> extended;
    if (_ComputeExtendedIntersections (extended) > 0)
        {
        for (auto info : extended)
            potentialIntersections.push_back (info);
        }

    return potentialIntersections.size ();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Scott.Devoe                     02/2016
+---------------+---------------+---------------+---------------+---------------+------*/
bool AlignmentIntersection::ComputeClosest(DPoint3dCR referencePt, AlignmentIntersectionInfoR info)
    {
    return ComputeClosestImpl(referencePt, nullptr, info);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Scott.Devoe                     02/2016
+---------------+---------------+---------------+---------------+---------------+------*/
bool AlignmentIntersection::ComputeClosest(DPoint3dCR referencePt, double maxDistance, AlignmentIntersectionInfoR info)
    {
    return ComputeClosestImpl(referencePt, &maxDistance, info);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Scott.Devoe                     02/2016
+---------------+---------------+---------------+---------------+---------------+------*/
bool AlignmentIntersection::ComputeClosestImpl(DPoint3dCR referencePt, const double* maxDistance, AlignmentIntersectionInfoR info)
    {
    bvector<AlignmentIntersectionInfo> potentialIntersections;
    if (Compute (potentialIntersections) > 0)
        {
        int index = -1;
        double minFoundDistance = 0.0;
        int i = 0;
        bool bSet = false;
        for (auto info : potentialIntersections)
            {
            double checkDist = referencePt.DistanceXY (info.m_point);
            if (maxDistance == nullptr || checkDist <= *maxDistance)
                {
                if (bSet == false)
                    {
                    index = i;
                    minFoundDistance = checkDist;
                    bSet = true;
                    }
                else if (checkDist < minFoundDistance)
                    {
                    index = i;
                    minFoundDistance = checkDist;
                    }
                }
            i++;
            }
        if (index >= 0)
            {
            info = potentialIntersections.at (index);
            return true;
            }
        }
    return false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sandy.Bugai                     07/2016
+---------------+---------------+---------------+---------------+---------------+------*/
bool AlignmentIntersection::ComputeClosestToStation(double referenceStation, bool alongPrimary, AlignmentIntersectionInfoR info)
    {
    bvector<AlignmentIntersectionInfo> potentialIntersections;
    if (Compute(potentialIntersections) > 0)
        {
        int index = -1;
        double minFoundStationDistance = 0.0;
        int i = 0;
        bool bSet = false;
        for (auto info : potentialIntersections)
            {
            double checkDist = fabs(referenceStation - (alongPrimary ? info.PrimaryStation() : info.SecondaryStation()));
            //if (maxDistance == nullptr || checkDist <= *maxDistance)
                {
                if (bSet == false)
                    {
                    index = i;
                    minFoundStationDistance = checkDist;
                    bSet = true;
                    }
                else if (checkDist < minFoundStationDistance)
                    {
                    index = i;
                    minFoundStationDistance = checkDist;
                    }
                }
            i++;
            }
        if (index >= 0)
            {
            info = potentialIntersections.at(index);
            return true;
            }
        }
    return false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Scott.Devoe                     02/2016
+---------------+---------------+---------------+---------------+---------------+------*/
AlignmentIntersectionPtr AlignmentIntersection::Create (AlignmentPairCP primary, AlignmentPairCP secondary)
    {
    return new AlignmentIntersection (const_cast< AlignmentPairP >( primary ), const_cast< AlignmentPairP >( secondary ));
    }