/*--------------------------------------------------------------------------------------+
|
|     $Source: Profiles/Implementation/ProfilesGeometry.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ProfilesInternal.h"
#include <Profiles\ProfilesDefinitions.h>
#include <Profiles\ProfilesGeometry.h>

USING_NAMESPACE_BENTLEY_DGN

BEGIN_BENTLEY_PROFILES_NAMESPACE

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
IGeometryPtr ProfilesGeomApi::CreateIShape(double overallWidth, double overallDepth, double flangeThickness, double webThickness)
    {
    double halfWidth = overallWidth / 2.0;
    double halfDepth = overallDepth / 2.0;

    DPoint3d ishapePts[] =
        {
        /*00*/ DPoint3d::From(-halfWidth, -halfDepth),
        /*01*/ DPoint3d::From(-halfWidth, -(halfDepth - flangeThickness)),
        /*02*/ DPoint3d::From(-(webThickness / 2.0), -(halfDepth - flangeThickness)),
        /*03*/ DPoint3d::From(-(webThickness / 2.0), halfDepth - flangeThickness),
        /*04*/ DPoint3d::From(-halfWidth, halfDepth - flangeThickness),
        /*05*/ DPoint3d::From(-halfWidth, halfDepth),
        /*06*/ DPoint3d::From(halfWidth, halfDepth),
        /*07*/ DPoint3d::From(halfWidth, halfDepth - flangeThickness),
        /*08*/ DPoint3d::From( webThickness / 2.0, halfDepth - flangeThickness),
        /*09*/ DPoint3d::From( webThickness / 2.0, -(halfDepth - flangeThickness)),
        /*10*/ DPoint3d::From(halfWidth, -(halfDepth - flangeThickness)),
        /*11*/ DPoint3d::From(halfWidth, -halfDepth),
        /*12*/ DPoint3d::From(-halfWidth, -halfDepth),
        };

    ICurvePrimitivePtr ishapeGeometry = ICurvePrimitive::CreateLineString(ishapePts, (sizeof(ishapePts) / sizeof((ishapePts)[0])));

    return IGeometry::Create(CurveVector::Create(ishapeGeometry, CurveVector::BOUNDARY_TYPE_Outer));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     10/2018
+---------------+---------------+---------------+---------------+---------------+------*/

IGeometryPtr ProfilesGeomApi::CreateIShapeWithFillet(double overallWidth, double overallDepth, double flangeThickness, double webThickness, double filletRadius)
    {
    CurveVectorPtr ishapePath = CurveVector::Create(CurveVector::BOUNDARY_TYPE_Open);

    ishapePath->Add(ICurvePrimitive::CreateLine(DSegment3d::From(0, 0, 0, 100, 0, 0)));


    ICurvePrimitive::CreateLine(DSegment3d::From(-(overallWidth / 2.0), -(overallDepth / 2.0), 0.0,     
                                                 -(overallWidth / 2.0), -(overallDepth / 2.0 - flangeThickness), 0.0));

    ICurvePrimitive::CreateLine(DSegment3d::From(-(overallWidth / 2.0), -(overallDepth / 2.0 - flangeThickness), 0.0,
        -(webThickness / 2.0 + filletRadius), -(overallDepth / 2.0 - flangeThickness), 0.0));


    //ICurvePrimitive::CreateArc();

    return nullptr;
    }

/*---------------------------------------------------------------------------------**//**
* Creates geometry of an arc between the two lines with the given radius. First line
* should end where the second line starts. First lines end point is adjusted to the 
* start point of the arc, second lines start point is adjusted to the end of the arc.
* @bsimethod                                                                     12/2018
+---------------+---------------+---------------+---------------+---------------+------*/
static ICurvePrimitivePtr createArcBetweenLines (ICurvePrimitivePtr& firstLinePtr, ICurvePrimitivePtr& secondLinePtr, double arcRadius)
    {
    DSegment3d const* pFirstLineSegment = firstLinePtr->GetLineCP();
    DSegment3d const* pSecondLineSegment = secondLinePtr->GetLineCP();
    if (pFirstLineSegment == nullptr || pSecondLineSegment == nullptr)
        {
        BeAssert (false && "ICurvePrimitive should be a single segment line");
        return nullptr;
        }

    DPoint3d firstLineEndPoint, secondLineStartPoint;
    pFirstLineSegment->GetEndPoint (firstLineEndPoint);
    pSecondLineSegment->GetStartPoint (secondLineStartPoint);
    if (!firstLineEndPoint.IsEqual (secondLineStartPoint))
        {
        BeAssert (false && "first line should end where the second one starts");
        return nullptr;
        }

    DVec3d v1 = pFirstLineSegment->VectorStartToEnd();
    v1.Normalize();
    v1.Negate();

    DVec3d v2 = pSecondLineSegment->VectorStartToEnd();
    v2.Normalize();

    DVec3d bisector = DVec3d::FromSumOf (v1, v2);
    bisector.Normalize();

    double const bisectorAngle = v1.SignedAngleTo (v2, bisector) / 2.0;
    double const bisectorLength = arcRadius / std::sin (bisectorAngle);
    double const lineOffset = bisectorLength * std::cos (bisectorAngle);

    DPoint3d const ellipseCenter = firstLineEndPoint + bisector * bisectorLength;
    DPoint3d const ellipseStart = firstLineEndPoint + v1 * lineOffset;
    DPoint3d const ellipseEnd = firstLineEndPoint + v2 * lineOffset;

    BeAssert (firstLinePtr->TrySetEnd (ellipseStart) && "Should be able to set end points of a single segment line");
    BeAssert (secondLinePtr->TrySetStart (ellipseEnd) && "Should be able to set end points of a single segment line");

    return ICurvePrimitive::CreateArc (DEllipse3d::FromArcCenterStartEnd (ellipseCenter, ellipseStart, ellipseEnd));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     12/2018
+---------------+---------------+---------------+---------------+---------------+------*/
IGeometryPtr ProfilesGeomApi::CreateCShape (CShapeProfileCPtr profile)
    {
    double const flangeThickness = profile->GetFlangeThickness();
    double const webThickness = profile->GetWebThickness();
    double const filletRadius = profile->GetFilletRadius();
    double const flangeEdgeRadius = profile->GetFlangeEdgeRadius();
    double const halfWidth = profile->GetFlangeWidth() / 2.0;
    double const halfDepth = profile->GetDepth() / 2.0;
    double const slopeHeight = profile->GetSlopeHeight();

    DPoint3d const topLeft = { -halfWidth,  halfDepth, 0.0 };
    DPoint3d const topRight = {  halfWidth,  halfDepth, 0.0 };
    DPoint3d const bottomRight = {  halfWidth, -halfDepth, 0.0 };
    DPoint3d const bottomLeft = { -halfWidth, -halfDepth, 0.0 };
    DPoint3d const topFlangeEdge = topRight - DPoint3d { 0.0, flangeThickness };
    DPoint3d const topInnerCorner = topLeft - DPoint3d { -webThickness, flangeThickness + slopeHeight };
    DPoint3d const bottomInnerCorner = bottomLeft - DPoint3d { -webThickness, -flangeThickness - slopeHeight };
    DPoint3d const bottomFlangEdge = bottomRight - DPoint3d { 0.0, -flangeThickness };

    ICurvePrimitivePtr topLine = ICurvePrimitive::CreateLine (topLeft, topRight);
    ICurvePrimitivePtr topFlangeEdgeLine = ICurvePrimitive::CreateLine (topRight, topFlangeEdge);
    ICurvePrimitivePtr topFlangeSlopeLine = ICurvePrimitive::CreateLine (topFlangeEdge, topInnerCorner);
    ICurvePrimitivePtr innerWebLine = ICurvePrimitive::CreateLine (topInnerCorner, bottomInnerCorner);
    ICurvePrimitivePtr bottomFlangeSlopeLine = ICurvePrimitive::CreateLine (bottomInnerCorner, bottomFlangEdge);
    ICurvePrimitivePtr bottomFlangeEdgeLine = ICurvePrimitive::CreateLine (bottomFlangEdge, bottomRight);
    ICurvePrimitivePtr bottomLine = ICurvePrimitive::CreateLine (bottomRight, bottomLeft);
    ICurvePrimitivePtr leftLine = ICurvePrimitive::CreateLine (bottomLeft, topLeft);
    ICurvePrimitivePtr topFlangeEdgeArc = nullptr;
    ICurvePrimitivePtr bottomFlangeEdgeArc = nullptr;
    ICurvePrimitivePtr bottomInnerCornerArc = nullptr;
    ICurvePrimitivePtr topInnerCornerArc = nullptr;

    if (profile->GetFlangeEdgeRadius() > 0.0)
        {
        topFlangeEdgeArc = createArcBetweenLines (topFlangeEdgeLine, topFlangeSlopeLine, flangeEdgeRadius);
        bottomFlangeEdgeArc = createArcBetweenLines (bottomFlangeSlopeLine, bottomFlangeEdgeLine, flangeEdgeRadius);
        }

    if (profile->GetFilletRadius() > 0.0)
        {
        topInnerCornerArc = createArcBetweenLines (topFlangeSlopeLine, innerWebLine, filletRadius);
        bottomInnerCornerArc = createArcBetweenLines (innerWebLine, bottomFlangeSlopeLine, filletRadius);
        }

    ICurvePrimitivePtr orderedCurves[] =
        {
        topLine, topFlangeEdgeLine, topFlangeEdgeArc, topFlangeSlopeLine, topInnerCornerArc, innerWebLine, bottomInnerCornerArc,
        bottomFlangeSlopeLine, bottomFlangeEdgeArc, bottomFlangeEdgeLine, bottomLine, leftLine
        };

    CurveVectorPtr curveVector = CurveVector::Create (CurveVector::BOUNDARY_TYPE_Outer);
    for (auto const& curve : orderedCurves)
        {
        if (curve.IsValid())
            curveVector->Add (curve);
        }

    return IGeometry::Create (curveVector);
    }

END_BENTLEY_PROFILES_NAMESPACE
