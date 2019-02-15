/*--------------------------------------------------------------------------------------+
|
|     $Source: Profiles/src/ProfilesGeometry.cpp $
|
|  $Copyright: (c) 2019 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ProfilesPch.h"
#include <ProfilesInternal\ProfilesGeometry.h>

BEGIN_BENTLEY_PROFILES_NAMESPACE

/*---------------------------------------------------------------------------------**//**
* Creates geometry of an arc between the two lines with the given radius. First line
* should end where the second line starts. First lines end point is adjusted to the
* start point of the arc, second lines start point is adjusted to the end of the arc.
* If one of resulting line lengths is zero, its set to nullptr.
* If arc radius is zero returns null, without adjusting start and end points for lines.
* @param firstLinePtr[in/out] first line segment.
* @param secondLinePtr[in/out] second line segment.
* @bsimethod                                                                     12/2018
+---------------+---------------+---------------+---------------+---------------+------*/
static ICurvePrimitivePtr createArcBetweenLines (ICurvePrimitivePtr& firstLinePtr, ICurvePrimitivePtr& secondLinePtr, double arcRadius)
    {
    if (BeNumerical::IsEqualToZero (arcRadius))
        return nullptr;

    if (firstLinePtr.IsNull() || secondLinePtr.IsNull())
        {
        BeAssert (false);
        return nullptr;
        }

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

    Angle const bisectorAngle = Angle::FromRadians (v1.SignedAngleTo (v2, bisector) / 2.0);
    double const bisectorLength = arcRadius / bisectorAngle.Sin();
    double const lineOffset = bisectorLength * bisectorAngle.Cos();

    DPoint3d const ellipseCenter = firstLineEndPoint + bisector * bisectorLength;
    DPoint3d const ellipseStart = firstLineEndPoint + v1 * lineOffset;
    DPoint3d const ellipseEnd = firstLineEndPoint + v2 * lineOffset;

    BeAssert (firstLinePtr->TrySetEnd (ellipseStart) && "Should be able to set end points of a single segment line");
    if (BeNumerical::IsEqualToZero (pFirstLineSegment->Length()))
        firstLinePtr = nullptr;

    BeAssert (secondLinePtr->TrySetStart (ellipseEnd) && "Should be able to set end points of a single segment line");
    if (BeNumerical::IsEqualToZero (pSecondLineSegment->Length()))
        secondLinePtr = nullptr;

    return ICurvePrimitive::CreateArc (DEllipse3d::FromArcCenterStartEnd (ellipseCenter, ellipseStart, ellipseEnd));
    }

/*---------------------------------------------------------------------------------**//*
* Returns intersection point of 2 non parallel lines. If lines are parallel, returns
* the end point of first line.
* @bsimethod                                                                     12/2018
+---------------+---------------+---------------+---------------+---------------+------*/
static DPoint3d getLineIntersectionPoint (DSegment3d const& firstLine, DSegment3d const& secondLine)
    {
    double fractionA, fractionB;
    DPoint3d pointA, pointB;

    bool intersects = DSegment3d::IntersectXY (fractionA, fractionB, pointA, pointB, firstLine, secondLine);
    if (!intersects)
        {
        BeAssert (false && "Lines shouldn't be parallel");
        firstLine.GetEndPoint (pointA);
        return pointA;
        }

    BeAssert (pointA.AlmostEqual (pointB) && "Intersection points should match (both z components should be 0.0)");
    return pointA;
    }

/*---------------------------------------------------------------------------------**//**
* Create IGeometry from an ordered array of ICurvePrimitive objects.
* @bsimethod                                                                     12/2018
+---------------+---------------+---------------+---------------+---------------+------*/
static IGeometryPtr createGeometryFromPrimitiveArray (bvector<ICurvePrimitivePtr>& orderedCurves, CurveVector::BoundaryType boundaryType = CurveVector::BOUNDARY_TYPE_Outer)
    {
    CurveVectorPtr curveVector = CurveVector::Create (boundaryType);
    for (auto const& curve : orderedCurves)
        {
        if (curve.IsValid())
            curveVector->Add (curve);
        }

    return IGeometry::Create (curveVector);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     12/2018
+---------------+---------------+---------------+---------------+---------------+------*/
IGeometryPtr ProfilesGeometry::CreateCShape (CShapeProfile const& profile)
    {
    double const flangeThickness = profile.GetFlangeThickness();
    double const webThickness = profile.GetWebThickness();
    double const filletRadius = profile.GetFilletRadius();
    double const flangeEdgeRadius = profile.GetFlangeEdgeRadius();
    double const halfWidth = profile.GetFlangeWidth() / 2.0;
    double const halfDepth = profile.GetDepth() / 2.0;
    double const slopeHeight = profile.GetFlangeSlopeHeight();

    DPoint3d const topLeft = { -halfWidth, halfDepth, 0.0 };
    DPoint3d const topRight = { halfWidth, halfDepth, 0.0 };
    DPoint3d const bottomRight = { halfWidth, -halfDepth, 0.0 };
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

    ICurvePrimitivePtr topFlangeEdgeArc = createArcBetweenLines (topFlangeEdgeLine, topFlangeSlopeLine, flangeEdgeRadius);
    ICurvePrimitivePtr bottomFlangeEdgeArc = createArcBetweenLines (bottomFlangeSlopeLine, bottomFlangeEdgeLine, flangeEdgeRadius);
    ICurvePrimitivePtr bottomInnerCornerArc = createArcBetweenLines (topFlangeSlopeLine, innerWebLine, filletRadius);
    ICurvePrimitivePtr topInnerCornerArc = createArcBetweenLines (innerWebLine, bottomFlangeSlopeLine, filletRadius);

    bvector<ICurvePrimitivePtr> orderedCurves =
        {
        topLine, topFlangeEdgeLine, topFlangeEdgeArc, topFlangeSlopeLine, topInnerCornerArc, innerWebLine, bottomInnerCornerArc,
        bottomFlangeSlopeLine, bottomFlangeEdgeArc, bottomFlangeEdgeLine, bottomLine, leftLine
        };
    return createGeometryFromPrimitiveArray (orderedCurves);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     12/2018
+---------------+---------------+---------------+---------------+---------------+------*/
IGeometryPtr ProfilesGeometry::CreateIShape (IShapeProfile const& profile)
    {
    double const flangeThickness = profile.GetFlangeThickness();
    double const halfWebThickness = profile.GetWebThickness() / 2.0;
    double const filletRadius = profile.GetFilletRadius();
    double const flangeEdgeRadius = profile.GetFlangeEdgeRadius();
    double const halfWidth = profile.GetFlangeWidth() / 2.0;
    double const halfDepth = profile.GetDepth() / 2.0;
    double const slopeHeight = profile.GetFlangeSlopeHeight();

    DPoint3d const topLeft = { -halfWidth, halfDepth, 0.0 };
    DPoint3d const topMiddle = { 0.0, halfDepth, 0.0 };
    DPoint3d const topRight = { halfWidth, halfDepth, 0.0 };
    DPoint3d const bottomRight = { halfWidth, -halfDepth, 0.0 };
    DPoint3d const bottomMiddle = { 0.0, -halfDepth, 0.0 };
    DPoint3d const bottomLeft = { -halfWidth, -halfDepth, 0.0 };

    DPoint3d const topRightFlangeEdge = topRight - DPoint3d { 0.0, flangeThickness };
    DPoint3d const topLeftFlangeEdge = topLeft - DPoint3d { 0.0, flangeThickness };
    DPoint3d const topRightInnerCorner = topMiddle - DPoint3d { -halfWebThickness, flangeThickness + slopeHeight };
    DPoint3d const topLeftInnerCorner = topMiddle - DPoint3d { halfWebThickness, flangeThickness + slopeHeight };
    DPoint3d const bottomRightInnerCorner = bottomMiddle - DPoint3d { -halfWebThickness, -flangeThickness - slopeHeight };
    DPoint3d const bototmLeftInnerCorner = bottomMiddle - DPoint3d { halfWebThickness, -flangeThickness - slopeHeight };
    DPoint3d const bottomRightFlangEdge = bottomRight - DPoint3d { 0.0, -flangeThickness };
    DPoint3d const bottomLeftFlangeEdge = bottomLeft - DPoint3d { 0.0, -flangeThickness };

    ICurvePrimitivePtr topLine = ICurvePrimitive::CreateLine (topLeft, topRight);
    ICurvePrimitivePtr topRightFlangeEdgeLine = ICurvePrimitive::CreateLine (topRight, topRightFlangeEdge);
    ICurvePrimitivePtr topRightFlangeEdgeArc = nullptr;
    ICurvePrimitivePtr topRightFlangeSlopeLine = ICurvePrimitive::CreateLine (topRightFlangeEdge, topRightInnerCorner);
    ICurvePrimitivePtr topRightInnerCornerArc = nullptr;
    ICurvePrimitivePtr innerRightWebLine = ICurvePrimitive::CreateLine (topRightInnerCorner, bottomRightInnerCorner);
    ICurvePrimitivePtr bottomRightInnerCornerArc = nullptr;
    ICurvePrimitivePtr bottomRightFlangeSlopeLine = ICurvePrimitive::CreateLine (bottomRightInnerCorner, bottomRightFlangEdge);
    ICurvePrimitivePtr bottomRightFlangeEdgeArc = nullptr;
    ICurvePrimitivePtr bottomRightFlangeEdgeLine = ICurvePrimitive::CreateLine (bottomRightFlangEdge, bottomRight);
    ICurvePrimitivePtr bottomLine = ICurvePrimitive::CreateLine (bottomRight, bottomLeft);
    ICurvePrimitivePtr bottomLeftFlangeEdgeLine = ICurvePrimitive::CreateLine (bottomLeft, bottomLeftFlangeEdge);
    ICurvePrimitivePtr bottomLeftFlangeEdgeArc = nullptr;
    ICurvePrimitivePtr bottomLeftFlangeSlopeLine = ICurvePrimitive::CreateLine (bottomLeftFlangeEdge, bototmLeftInnerCorner);
    ICurvePrimitivePtr bottomLeftInnerCornerArc = nullptr;
    ICurvePrimitivePtr innerLeftWebLine = ICurvePrimitive::CreateLine (bototmLeftInnerCorner, topLeftInnerCorner);
    ICurvePrimitivePtr topLeftInnerCornerArc = nullptr;
    ICurvePrimitivePtr topLeftFlangeSlopeLine = ICurvePrimitive::CreateLine (topLeftInnerCorner, topLeftFlangeEdge);
    ICurvePrimitivePtr topLeftFlangeEdgeArc = nullptr;
    ICurvePrimitivePtr topLeftFlangeEdgeLine = ICurvePrimitive::CreateLine (topLeftFlangeEdge, topLeft);

    topRightFlangeEdgeArc = createArcBetweenLines (topRightFlangeEdgeLine, topRightFlangeSlopeLine, flangeEdgeRadius);
    bottomRightFlangeEdgeArc = createArcBetweenLines (bottomRightFlangeSlopeLine, bottomRightFlangeEdgeLine, flangeEdgeRadius);
    bottomLeftFlangeEdgeArc = createArcBetweenLines (bottomLeftFlangeEdgeLine, bottomLeftFlangeSlopeLine, flangeEdgeRadius);
    topLeftFlangeEdgeArc = createArcBetweenLines (topLeftFlangeSlopeLine, topLeftFlangeEdgeLine, flangeEdgeRadius);

    topRightInnerCornerArc = createArcBetweenLines (topRightFlangeSlopeLine, innerRightWebLine, filletRadius);
    bottomRightInnerCornerArc = createArcBetweenLines (innerRightWebLine, bottomRightFlangeSlopeLine, filletRadius);
    bottomLeftInnerCornerArc = createArcBetweenLines (bottomLeftFlangeSlopeLine, innerLeftWebLine, filletRadius);
    topLeftInnerCornerArc = createArcBetweenLines (innerLeftWebLine, topLeftFlangeSlopeLine, filletRadius);

    bvector<ICurvePrimitivePtr> orderedCurves =
        {
        topLine, topRightFlangeEdgeLine, topRightFlangeEdgeArc, topRightFlangeSlopeLine, topRightInnerCornerArc, innerRightWebLine,
        bottomRightInnerCornerArc, bottomRightFlangeSlopeLine, bottomRightFlangeEdgeArc, bottomRightFlangeEdgeLine,
        bottomLine, bottomLeftFlangeEdgeLine, bottomLeftFlangeEdgeArc, bottomLeftFlangeSlopeLine, bottomLeftInnerCornerArc,
        innerLeftWebLine, topLeftInnerCornerArc, topLeftFlangeSlopeLine, topLeftFlangeEdgeArc, topLeftFlangeEdgeLine
        };
    return createGeometryFromPrimitiveArray (orderedCurves);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     12/2018
+---------------+---------------+---------------+---------------+---------------+------*/
IGeometryPtr ProfilesGeometry::CreateAsymmetricIShape (AsymmetricIShapeProfile const& profile)
    {
    double const topFlangeThickness = profile.GetTopFlangeThickness();
    double const bottomFlangeThickness = profile.GetBottomFlangeThickness();
    double const topFlangeFilletRadius = profile.GetTopFlangeFilletRadius();
    double const bottomFlangeFilletRadius = profile.GetBottomFlangeFilletRadius();
    double const topFlangeEdgeRadius = profile.GetTopFlangeEdgeRadius();
    double const bottomFlangeEdgeRadius = profile.GetBottomFlangeEdgeRadius();
    double const halfWebThickness = profile.GetWebThickness() / 2.0;
    double const halfTopWidth = profile.GetTopFlangeWidth() / 2.0;
    double const halfBottomWidth = profile.GetBottomFlangeWidth() / 2.0;
    double const halfDepth = profile.GetDepth() / 2.0;
    double const topFlangeSlopeHeight = profile.GetTopFlangeSlopeHeight();
    double const bottomFlangeSlopeHeight = profile.GetTopFlangeSlopeHeight();

    DPoint3d const topLeft = { -halfTopWidth, halfDepth, 0.0 };
    DPoint3d const topMiddle = { 0.0, halfDepth, 0.0 };
    DPoint3d const topRight = { halfTopWidth, halfDepth, 0.0 };
    DPoint3d const bottomRight = { halfBottomWidth, -halfDepth, 0.0 };
    DPoint3d const bottomMiddle = { 0.0, -halfDepth, 0.0 };
    DPoint3d const bottomLeft = { -halfBottomWidth, -halfDepth, 0.0 };

    DPoint3d const topRightFlangeEdge = topRight - DPoint3d { 0.0, topFlangeThickness };
    DPoint3d const topLeftFlangeEdge = topLeft - DPoint3d { 0.0, topFlangeThickness };
    DPoint3d const topRightInnerCorner = topMiddle - DPoint3d { -halfWebThickness, topFlangeThickness + topFlangeSlopeHeight };
    DPoint3d const topLeftInnerCorner = topMiddle - DPoint3d { halfWebThickness, topFlangeThickness + topFlangeSlopeHeight };
    DPoint3d const bottomRightInnerCorner = bottomMiddle - DPoint3d { -halfWebThickness, -bottomFlangeThickness - bottomFlangeSlopeHeight };
    DPoint3d const bototmLeftInnerCorner = bottomMiddle - DPoint3d { halfWebThickness, -bottomFlangeThickness - bottomFlangeSlopeHeight };
    DPoint3d const bottomRightFlangEdge = bottomRight - DPoint3d { 0.0, -bottomFlangeThickness };
    DPoint3d const bottomLeftFlangeEdge = bottomLeft - DPoint3d { 0.0, -bottomFlangeThickness };

    ICurvePrimitivePtr topLine = ICurvePrimitive::CreateLine (topLeft, topRight);
    ICurvePrimitivePtr topRightFlangeEdgeLine = ICurvePrimitive::CreateLine (topRight, topRightFlangeEdge);
    ICurvePrimitivePtr topRightFlangeEdgeArc = nullptr;
    ICurvePrimitivePtr topRightFlangeSlopeLine = ICurvePrimitive::CreateLine (topRightFlangeEdge, topRightInnerCorner);
    ICurvePrimitivePtr topRightInnerCornerArc = nullptr;
    ICurvePrimitivePtr innerRightWebLine = ICurvePrimitive::CreateLine (topRightInnerCorner, bottomRightInnerCorner);
    ICurvePrimitivePtr bottomRightInnerCornerArc = nullptr;
    ICurvePrimitivePtr bottomRightFlangeSlopeLine = ICurvePrimitive::CreateLine (bottomRightInnerCorner, bottomRightFlangEdge);
    ICurvePrimitivePtr bottomRightFlangeEdgeArc = nullptr;
    ICurvePrimitivePtr bottomRightFlangeEdgeLine = ICurvePrimitive::CreateLine (bottomRightFlangEdge, bottomRight);
    ICurvePrimitivePtr bottomLine = ICurvePrimitive::CreateLine (bottomRight, bottomLeft);
    ICurvePrimitivePtr bottomLeftFlangeEdgeLine = ICurvePrimitive::CreateLine (bottomLeft, bottomLeftFlangeEdge);
    ICurvePrimitivePtr bottomLeftFlangeEdgeArc = nullptr;
    ICurvePrimitivePtr bottomLeftFlangeSlopeLine = ICurvePrimitive::CreateLine (bottomLeftFlangeEdge, bototmLeftInnerCorner);
    ICurvePrimitivePtr bottomLeftInnerCornerArc = nullptr;
    ICurvePrimitivePtr innerLeftWebLine = ICurvePrimitive::CreateLine (bototmLeftInnerCorner, topLeftInnerCorner);
    ICurvePrimitivePtr topLeftInnerCornerArc = nullptr;
    ICurvePrimitivePtr topLeftFlangeSlopeLine = ICurvePrimitive::CreateLine (topLeftInnerCorner, topLeftFlangeEdge);
    ICurvePrimitivePtr topLeftFlangeEdgeArc = nullptr;
    ICurvePrimitivePtr topLeftFlangeEdgeLine = ICurvePrimitive::CreateLine (topLeftFlangeEdge, topLeft);

    topRightFlangeEdgeArc = createArcBetweenLines (topRightFlangeEdgeLine, topRightFlangeSlopeLine, topFlangeEdgeRadius);
    topLeftFlangeEdgeArc = createArcBetweenLines (topLeftFlangeSlopeLine, topLeftFlangeEdgeLine, topFlangeEdgeRadius);

    bottomRightFlangeEdgeArc = createArcBetweenLines (bottomRightFlangeSlopeLine, bottomRightFlangeEdgeLine, bottomFlangeEdgeRadius);
    bottomLeftFlangeEdgeArc = createArcBetweenLines (bottomLeftFlangeEdgeLine, bottomLeftFlangeSlopeLine, bottomFlangeEdgeRadius);

    topRightInnerCornerArc = createArcBetweenLines (topRightFlangeSlopeLine, innerRightWebLine, topFlangeFilletRadius);
    topLeftInnerCornerArc = createArcBetweenLines (innerLeftWebLine, topLeftFlangeSlopeLine, topFlangeFilletRadius);

    bottomRightInnerCornerArc = createArcBetweenLines (innerRightWebLine, bottomRightFlangeSlopeLine, bottomFlangeFilletRadius);
    bottomLeftInnerCornerArc = createArcBetweenLines (bottomLeftFlangeSlopeLine, innerLeftWebLine, bottomFlangeFilletRadius);

    bvector<ICurvePrimitivePtr> orderedCurves =
        {
        topLine, topRightFlangeEdgeLine, topRightFlangeEdgeArc, topRightFlangeSlopeLine, topRightInnerCornerArc, innerRightWebLine,
        bottomRightInnerCornerArc, bottomRightFlangeSlopeLine, bottomRightFlangeEdgeArc, bottomRightFlangeEdgeLine,
        bottomLine, bottomLeftFlangeEdgeLine, bottomLeftFlangeEdgeArc, bottomLeftFlangeSlopeLine, bottomLeftInnerCornerArc,
        innerLeftWebLine, topLeftInnerCornerArc, topLeftFlangeSlopeLine, topLeftFlangeEdgeArc, topLeftFlangeEdgeLine
        };
    return createGeometryFromPrimitiveArray (orderedCurves);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     12/2018
+---------------+---------------+---------------+---------------+---------------+------*/
IGeometryPtr ProfilesGeometry::CreateLShape (LShapeProfile const& profile)
    {
    double const halfWidth = profile.GetWidth() / 2.0;
    double const halfDepth = profile.GetDepth() / 2.0;
    double const thickness = profile.GetThickness();
    double const filletRadius = profile.GetFilletRadius();
    double const edgeRadius = profile.GetEdgeRadius();
    double const flangeSlopeHeight = profile.GetHorizontalLegSlopeHeight();
    double const webSlopeHeight = profile.GetVerticalLegSlopeHeight();

    DPoint3d const topLeft = { -halfWidth, halfDepth, 0.0 };
    DPoint3d const topRight = { -halfWidth + thickness, halfDepth, 0.0 };
    DPoint3d const bottomRight = { halfWidth, -halfDepth, 0.0 };
    DPoint3d const bottomLeft = { -halfWidth, -halfDepth, 0.0 };
    DPoint3d const bottomRightEdge = bottomRight - DPoint3d { 0.0, -thickness };

    // Flange and web slopes not included
    DPoint3d bottomInnerCorner = bottomLeft - DPoint3d { -thickness, -thickness };

    DSegment3d webSlopeSegment = DSegment3d::From (topRight, bottomInnerCorner - DPoint3d { -webSlopeHeight, 0.0 });
    DSegment3d flangeSlopeSegment = DSegment3d::From (bottomRightEdge, bottomInnerCorner - DPoint3d { 0.0, -flangeSlopeHeight });
    bottomInnerCorner = getLineIntersectionPoint (flangeSlopeSegment, webSlopeSegment);

    ICurvePrimitivePtr topLine = ICurvePrimitive::CreateLine (topLeft, topRight);
    ICurvePrimitivePtr webEdgeArc = nullptr;
    ICurvePrimitivePtr webSlopeLine = ICurvePrimitive::CreateLine (topRight, bottomInnerCorner);
    ICurvePrimitivePtr bottomInnerCornerArc = nullptr;
    ICurvePrimitivePtr flangeSlopeLine = ICurvePrimitive::CreateLine (bottomInnerCorner, bottomRightEdge);
    ICurvePrimitivePtr bottomRigthEdgeArc = nullptr;
    ICurvePrimitivePtr rightLine = ICurvePrimitive::CreateLine (bottomRightEdge, bottomRight);
    ICurvePrimitivePtr bottomLine = ICurvePrimitive::CreateLine (bottomRight, bottomLeft);
    ICurvePrimitivePtr leftLine = ICurvePrimitive::CreateLine (bottomLeft, topLeft);

    webEdgeArc = createArcBetweenLines (topLine, webSlopeLine, edgeRadius);
    bottomRigthEdgeArc = createArcBetweenLines (flangeSlopeLine, rightLine, edgeRadius);

    bottomInnerCornerArc = createArcBetweenLines (webSlopeLine, flangeSlopeLine, filletRadius);

    bvector<ICurvePrimitivePtr> orderedCurves =
        {
        topLine, webEdgeArc, webSlopeLine, bottomInnerCornerArc, flangeSlopeLine, bottomRigthEdgeArc, rightLine, bottomLine, leftLine
        };
    return createGeometryFromPrimitiveArray (orderedCurves);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     02/2019
+---------------+---------------+---------------+---------------+---------------+------*/
IGeometryPtr ProfilesGeometry::CreateSchifflerizedLShape (SchifflerizedLShapeProfile const& profile)
    {
    double const legLength = profile.GetLegLength();
    double const thickness = profile.GetThickness();
    double const halfWidth = legLength / 2.0;
    double const halfDepth = legLength / 2.0;
    double const legBendOffset = profile.GetLegBendOffset();
    double const innerEdgeOffset = thickness / Angle::FromDegrees (15.0).Cos() * Angle::FromDegrees (15.0).Sin();
    double const bentLegOuterLength = legLength - legBendOffset;
    double const bentLegInnerLength = bentLegOuterLength - innerEdgeOffset;
    Angle const horizontalLegBendAngle = Angle::FromDegrees (15.0);
    Angle const verticalLegBendAngle = Angle::FromDegrees (75.0);

    DPoint3d const outerLegJoint { -halfWidth, -halfDepth };
    DPoint3d const innerLegJoint { -halfWidth + thickness, -halfDepth + thickness };
    DPoint3d const verticalLegOuterBendPoint { -halfWidth, -halfDepth + legBendOffset };
    DPoint3d const verticalLegInnerBendPoint { -halfWidth + thickness, -halfDepth + legBendOffset };
    DPoint3d const verticalLegOuterEdge = verticalLegOuterBendPoint - DPoint3d { -bentLegOuterLength * verticalLegBendAngle.Cos(), -bentLegOuterLength * verticalLegBendAngle.Sin() };
    DPoint3d const verticalLegInnerEdge = verticalLegInnerBendPoint - DPoint3d { -bentLegInnerLength * verticalLegBendAngle.Cos(), -bentLegInnerLength * verticalLegBendAngle.Sin() };
    DPoint3d const horizontalLegOuterBendPoint { -halfWidth + legBendOffset, -halfDepth };
    DPoint3d const horizontalLegInnerBendPoint { -halfWidth + legBendOffset, -halfDepth + thickness };
    DPoint3d const horizontalLegOuterEdge = horizontalLegOuterBendPoint - DPoint3d { -bentLegOuterLength * horizontalLegBendAngle.Cos(), -bentLegOuterLength * horizontalLegBendAngle.Sin() };
    DPoint3d const horizontalLegInnerEdge = horizontalLegInnerBendPoint - DPoint3d { -bentLegInnerLength * horizontalLegBendAngle.Cos(), -bentLegInnerLength * horizontalLegBendAngle.Sin() };

    ICurvePrimitivePtr verticalLegOuterUnbentLine = ICurvePrimitive::CreateLine (outerLegJoint, verticalLegOuterBendPoint);
    ICurvePrimitivePtr verticalLegOuterBentLine = ICurvePrimitive::CreateLine (verticalLegOuterBendPoint, verticalLegOuterEdge);
    ICurvePrimitivePtr verticalLegEdgeLine = ICurvePrimitive::CreateLine (verticalLegOuterEdge, verticalLegInnerEdge);
    ICurvePrimitivePtr verticalLegInnerBentLine = ICurvePrimitive::CreateLine (verticalLegInnerEdge, verticalLegInnerBendPoint);
    ICurvePrimitivePtr verticalLegInnerUnbentLine = ICurvePrimitive::CreateLine (verticalLegInnerBendPoint, innerLegJoint);
    ICurvePrimitivePtr horizontalLegInnerUnbentLine = ICurvePrimitive::CreateLine (innerLegJoint, horizontalLegInnerBendPoint);
    ICurvePrimitivePtr horizontalLegInnerBentLine = ICurvePrimitive::CreateLine (horizontalLegInnerBendPoint, horizontalLegInnerEdge);
    ICurvePrimitivePtr horizontalLegEdgeLine = ICurvePrimitive::CreateLine (horizontalLegInnerEdge, horizontalLegOuterEdge);
    ICurvePrimitivePtr horizontalLegOuterBentLine = ICurvePrimitive::CreateLine (horizontalLegOuterEdge, horizontalLegOuterBendPoint);
    ICurvePrimitivePtr horizontalLegOuterUnbentLine = ICurvePrimitive::CreateLine (horizontalLegOuterBendPoint, outerLegJoint);

    ICurvePrimitivePtr verticalLegEdgeArc = createArcBetweenLines (verticalLegEdgeLine, verticalLegInnerBentLine, profile.GetEdgeRadius());
    ICurvePrimitivePtr innerLegJointArc = createArcBetweenLines (verticalLegInnerUnbentLine, horizontalLegInnerUnbentLine, profile.GetFilletRadius());
    ICurvePrimitivePtr horizontalLegEdgeArc = createArcBetweenLines (horizontalLegInnerBentLine, horizontalLegEdgeLine, profile.GetEdgeRadius());

    bvector<ICurvePrimitivePtr> orderedCurves =
        {
        verticalLegOuterUnbentLine, verticalLegOuterBentLine, verticalLegEdgeLine, verticalLegEdgeArc, verticalLegInnerBentLine,
        verticalLegInnerUnbentLine, innerLegJointArc, horizontalLegInnerUnbentLine, horizontalLegInnerBentLine, horizontalLegEdgeArc,
        horizontalLegEdgeLine, horizontalLegOuterBentLine, horizontalLegOuterUnbentLine
        };
    IGeometryPtr geometryPtr = createGeometryFromPrimitiveArray (orderedCurves);

    DMatrix4d rotation = DMatrix4d::From (RotMatrix::FromVectorAndRotationAngle (DVec3d::From (0.0, 0.0, 1.0), -PI / 4.0));
    Transform transform;
    transform.InitFrom (rotation);
    geometryPtr->TryTransformInPlace (transform);

    return geometryPtr;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     12/2018
+---------------+---------------+---------------+---------------+---------------+------*/
IGeometryPtr ProfilesGeometry::CreateTShape (TShapeProfile const& profile)
    {
    double const flangeThickness = profile.GetFlangeThickness();
    double const halfWebThickness = profile.GetWebThickness() / 2.0;
    double const filletRadius = profile.GetFilletRadius();
    double const flangeEdgeRadius = profile.GetFlangeEdgeRadius();
    double const webEdgeRadius = profile.GetWebEdgeRadius();
    double const halfWidth = profile.GetFlangeWidth() / 2.0;
    double const halfDepth = profile.GetDepth() / 2.0;
    double const flangeSlopeHeight = profile.GetFlangeSlopeHeight();
    double const webSlopeHeight = profile.GetWebSlopeHeight();

    DPoint3d const topLeft = { -halfWidth, halfDepth, 0.0 };
    DPoint3d const topMiddle = { 0.0, halfDepth, 0.0 };
    DPoint3d const topRight = { halfWidth, halfDepth, 0.0 };
    DPoint3d const bottomRight = { halfWebThickness, -halfDepth, 0.0 };
    DPoint3d const bottomLeft = { -halfWebThickness, -halfDepth, 0.0 };
    DPoint3d const topRightFlangeEdge = topRight - DPoint3d { 0.0, flangeThickness };
    DPoint3d const topLeftFlangeEdge = topLeft - DPoint3d { 0.0, flangeThickness };

    // Flange and web slopes not included
    DPoint3d topRightInnerCorner = topMiddle - DPoint3d { -halfWebThickness, flangeThickness };
    DPoint3d topLeftInnerCorner = topMiddle - DPoint3d { halfWebThickness, flangeThickness };

    DSegment3d rightWebSlopeSegment = DSegment3d::From (bottomRight, topRightInnerCorner - DPoint3d { -webSlopeHeight, 0.0 });
    DSegment3d rightFlangeSlopeSegment = DSegment3d::From (topRightFlangeEdge, topRightInnerCorner - DPoint3d { 0.0, flangeSlopeHeight });
    topRightInnerCorner = getLineIntersectionPoint (rightFlangeSlopeSegment, rightWebSlopeSegment);

    DSegment3d leftWebSlopeSegment = DSegment3d::From (bottomLeft, topLeftInnerCorner - DPoint3d { webSlopeHeight, 0.0 });
    DSegment3d leftFlangeSlopeSegment = DSegment3d::From (topLeftFlangeEdge, topLeftInnerCorner - DPoint3d { 0.0, flangeSlopeHeight });
    topLeftInnerCorner = getLineIntersectionPoint (leftWebSlopeSegment, leftFlangeSlopeSegment);

    ICurvePrimitivePtr topLine = ICurvePrimitive::CreateLine (topLeft, topRight);
    ICurvePrimitivePtr topRightFlangeEdgeLine = ICurvePrimitive::CreateLine (topRight, topRightFlangeEdge);
    ICurvePrimitivePtr topRightFlangeEdgeArc = nullptr;
    ICurvePrimitivePtr topRightFlangeSlopeLine = ICurvePrimitive::CreateLine (topRightFlangeEdge, topRightInnerCorner);
    ICurvePrimitivePtr topRightInnerCornerArc = nullptr;
    ICurvePrimitivePtr innerRightWebLine = ICurvePrimitive::CreateLine (topRightInnerCorner, bottomRight);
    ICurvePrimitivePtr bottomRightWebEdgeArc = nullptr;
    ICurvePrimitivePtr bottomLine = ICurvePrimitive::CreateLine (bottomRight, bottomLeft);
    ICurvePrimitivePtr bottomLeftWebEdgeArc = nullptr;
    ICurvePrimitivePtr innerLeftWebLine = ICurvePrimitive::CreateLine (bottomLeft, topLeftInnerCorner);
    ICurvePrimitivePtr topLeftInnerCornerArc = nullptr;
    ICurvePrimitivePtr topLeftFlangeSlopeLine = ICurvePrimitive::CreateLine (topLeftInnerCorner, topLeftFlangeEdge);
    ICurvePrimitivePtr topLeftFlangeEdgeArc = nullptr;
    ICurvePrimitivePtr topLeftFlangeEdgeLine = ICurvePrimitive::CreateLine (topLeftFlangeEdge, topLeft);

    topRightFlangeEdgeArc = createArcBetweenLines (topRightFlangeEdgeLine, topRightFlangeSlopeLine, flangeEdgeRadius);
    topLeftFlangeEdgeArc = createArcBetweenLines (topLeftFlangeSlopeLine, topLeftFlangeEdgeLine, flangeEdgeRadius);

    bottomRightWebEdgeArc = createArcBetweenLines (innerRightWebLine, bottomLine, webEdgeRadius);
    bottomLeftWebEdgeArc = createArcBetweenLines (bottomLine, innerLeftWebLine, webEdgeRadius);

    topRightInnerCornerArc = createArcBetweenLines (topRightFlangeSlopeLine, innerRightWebLine, filletRadius);
    topLeftInnerCornerArc = createArcBetweenLines (innerLeftWebLine, topLeftFlangeSlopeLine, filletRadius);

    bvector<ICurvePrimitivePtr> orderedCurves =
        {
        topLine, topRightFlangeEdgeLine, topRightFlangeEdgeArc, topRightFlangeSlopeLine, topRightInnerCornerArc, innerRightWebLine, bottomRightWebEdgeArc,
        bottomLine, bottomLeftWebEdgeArc, innerLeftWebLine, topLeftInnerCornerArc, topLeftFlangeSlopeLine, topLeftFlangeEdgeArc, topLeftFlangeEdgeLine
        };
    return createGeometryFromPrimitiveArray (orderedCurves);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     01/2019
+---------------+---------------+---------------+---------------+---------------+------*/
IGeometryPtr ProfilesGeometry::CreateTTShape (TTShapeProfile const& profile)
    {
    double const halfWidth = profile.GetFlangeWidth() / 2.0;
    double const halfDepth = profile.GetDepth() / 2.0;
    double const flangeThickness = profile.GetFlangeThickness();
    double const webThickness = profile.GetWebThickness();
    double const halfWebSpacing = profile.GetWebSpacing() / 2.0;
    double const webLength = profile.GetWebOuterFaceLength();
    double const flangeSlopeHeight = profile.GetFlangeSlopeHeight();
    double const webOuterSlopeHeight = profile.GetWebOuterSlopeHeight();
    double const webInnerSlopeHeight = profile.GetWebInnerSlopeHeight();

    DPoint3d const flangeTopLeft = { -halfWidth, halfDepth, 0.0 };
    DPoint3d const flangeBottomLeft = flangeTopLeft - DPoint3d { 0.0, flangeThickness };
    DPoint3d const flangeTopRight = { halfWidth, halfDepth, 0.0 };
    DPoint3d const flangeBottomRight = flangeTopRight - DPoint3d { 0.0, flangeThickness };
    DPoint3d const flangeBottomMiddle = { 0.0, halfDepth - flangeThickness, 0.0 };

    DPoint3d const rightWebTopLeft = flangeBottomMiddle - DPoint3d { -halfWebSpacing + webInnerSlopeHeight, flangeSlopeHeight };
    DPoint3d rightWebTopRight = flangeBottomMiddle - DPoint3d { -halfWebSpacing - webThickness, 0.0 };
    DPoint3d const rightWebBottomRight = flangeBottomMiddle - DPoint3d { -halfWebSpacing - webThickness, webLength };
    DPoint3d const rightWebBottomLeft = flangeBottomMiddle - DPoint3d { -halfWebSpacing, webLength };

    DPoint3d leftWebTopLeft = flangeBottomMiddle - DPoint3d { halfWebSpacing + webThickness, 0.0 };
    DPoint3d const leftWebTopRight = flangeBottomMiddle - DPoint3d { halfWebSpacing - webInnerSlopeHeight, flangeSlopeHeight };
    DPoint3d const leftWebBottomRight = flangeBottomMiddle - DPoint3d { halfWebSpacing, webLength };
    DPoint3d const leftWebBottomLeft = flangeBottomMiddle - DPoint3d { halfWebSpacing + webThickness, webLength };

    // Adjust fillet points where two slopes meet
    DSegment3d rightWebRightSlopeSegment = DSegment3d::From (rightWebBottomRight, rightWebTopRight - DPoint3d { -webOuterSlopeHeight, 0.0 });
    DSegment3d flangeRightSlopeSegment = DSegment3d::From (flangeBottomRight, rightWebTopRight - DPoint3d { 0.0, flangeSlopeHeight });
    rightWebTopRight = getLineIntersectionPoint (rightWebRightSlopeSegment, flangeRightSlopeSegment);

    DSegment3d leftWebLeftSlopeSegment = DSegment3d::From (leftWebBottomLeft, leftWebTopLeft - DPoint3d { webOuterSlopeHeight, 0.0 });
    DSegment3d flangeLeftSlopeSegment = DSegment3d::From (flangeBottomLeft, leftWebTopLeft - DPoint3d { 0.0, flangeSlopeHeight });
    leftWebTopLeft = getLineIntersectionPoint (leftWebLeftSlopeSegment, flangeLeftSlopeSegment);

    ICurvePrimitivePtr flangeTopLine = ICurvePrimitive::CreateLine (flangeTopLeft, flangeTopRight);
    ICurvePrimitivePtr flangeRightLine = ICurvePrimitive::CreateLine (flangeTopRight, flangeBottomRight);
    ICurvePrimitivePtr flangeRightEdgeArc = nullptr;
    ICurvePrimitivePtr flangeRightSlopeLine = ICurvePrimitive::CreateLine (flangeBottomRight, rightWebTopRight);
    ICurvePrimitivePtr rightWebRightFilletArc = nullptr;
    ICurvePrimitivePtr rightWebRightSlopeLine = ICurvePrimitive::CreateLine (rightWebTopRight, rightWebBottomRight);
    ICurvePrimitivePtr rightWebRightEdgeArc = nullptr;
    ICurvePrimitivePtr rightWebBottomLine = ICurvePrimitive::CreateLine (rightWebBottomRight, rightWebBottomLeft);
    ICurvePrimitivePtr rightWebLeftEdgeArc = nullptr;
    ICurvePrimitivePtr rightWebLeftSlopeLine = ICurvePrimitive::CreateLine (rightWebBottomLeft, rightWebTopLeft);
    ICurvePrimitivePtr rightWebLeftFilletArc = nullptr;
    ICurvePrimitivePtr webSpacingLine = ICurvePrimitive::CreateLine (rightWebTopLeft, leftWebTopRight);
    ICurvePrimitivePtr leftWebRightFilletArc = nullptr;
    ICurvePrimitivePtr leftWebRightSlopeLine = ICurvePrimitive::CreateLine (leftWebTopRight, leftWebBottomRight);
    ICurvePrimitivePtr leftWebRightEdgeArc = nullptr;
    ICurvePrimitivePtr leftWebBottomLine = ICurvePrimitive::CreateLine (leftWebBottomRight, leftWebBottomLeft);
    ICurvePrimitivePtr leftWebLeftEdgeArc = nullptr;
    ICurvePrimitivePtr leftWebLeftSlopeLine = ICurvePrimitive::CreateLine (leftWebBottomLeft, leftWebTopLeft);
    ICurvePrimitivePtr leftWebLeftFilletArc = nullptr;
    ICurvePrimitivePtr flangeLeftSlopeLine = ICurvePrimitive::CreateLine (leftWebTopLeft, flangeBottomLeft);
    ICurvePrimitivePtr flangeLeftEdgeArc = nullptr;
    ICurvePrimitivePtr flangeLeftLine = ICurvePrimitive::CreateLine (flangeBottomLeft, flangeTopLeft);

    double const filletRadius = profile.GetFilletRadius();
    double const flangeEdgeRadius = profile.GetFlangeEdgeRadius();
    double const webEdgeRadius = profile.GetWebEdgeRadius();

    // Clip fillet radius for the fillets between the webs (in the WebSpacing)
    double spacingFilletRadius = 0.0;
    if (BeNumerical::IsGreaterThanZero (filletRadius))
        spacingFilletRadius = std::min (filletRadius, halfWebSpacing - webInnerSlopeHeight);

    flangeRightEdgeArc = createArcBetweenLines (flangeRightLine, flangeRightSlopeLine, flangeEdgeRadius);
    rightWebRightFilletArc = createArcBetweenLines (flangeRightSlopeLine, rightWebRightSlopeLine, filletRadius);
    rightWebRightEdgeArc = createArcBetweenLines (rightWebRightSlopeLine, rightWebBottomLine, webEdgeRadius);
    rightWebLeftEdgeArc = createArcBetweenLines (rightWebBottomLine, rightWebLeftSlopeLine, webEdgeRadius);
    rightWebLeftFilletArc = createArcBetweenLines (rightWebLeftSlopeLine, webSpacingLine, spacingFilletRadius);
    leftWebRightFilletArc = createArcBetweenLines (webSpacingLine, leftWebRightSlopeLine, spacingFilletRadius);
    leftWebRightEdgeArc = createArcBetweenLines (leftWebRightSlopeLine, leftWebBottomLine, webEdgeRadius);
    leftWebLeftEdgeArc = createArcBetweenLines (leftWebBottomLine, leftWebLeftSlopeLine, webEdgeRadius);
    leftWebLeftFilletArc = createArcBetweenLines (leftWebLeftSlopeLine, flangeLeftSlopeLine, filletRadius);
    flangeLeftEdgeArc = createArcBetweenLines (flangeLeftSlopeLine, flangeLeftLine, flangeEdgeRadius);

    bvector<ICurvePrimitivePtr> orderedCurves =
        {
        flangeTopLine, flangeRightLine, flangeRightEdgeArc, flangeRightSlopeLine, rightWebRightFilletArc, rightWebRightSlopeLine,
        rightWebRightEdgeArc, rightWebBottomLine, rightWebLeftEdgeArc, rightWebLeftSlopeLine, rightWebLeftFilletArc, webSpacingLine,
        leftWebRightFilletArc, leftWebRightSlopeLine, leftWebRightEdgeArc, leftWebBottomLine, leftWebLeftEdgeArc,
        leftWebLeftSlopeLine, leftWebLeftFilletArc, flangeLeftSlopeLine, flangeLeftEdgeArc, flangeLeftLine
        };
    return createGeometryFromPrimitiveArray (orderedCurves);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     12/2018
+---------------+---------------+---------------+---------------+---------------+------*/
IGeometryPtr ProfilesGeometry::CreateZShape (ZShapeProfile const& profile)
    {
    double const flangeWidth = profile.GetFlangeWidth();
    double const flangeThickness = profile.GetFlangeThickness();
    double const filletRadius = profile.GetFilletRadius();
    double const flangeEdgeRadius = profile.GetFlangeEdgeRadius();
    double const halfDepth = profile.GetDepth() / 2.0;
    double const webThickness = profile.GetWebThickness();
    double const halfWebThickness = webThickness / 2.0;
    double const slopeHeight = profile.GetFlangeSlopeHeight();

    DPoint3d const topLeft = { -flangeWidth + halfWebThickness, halfDepth, 0.0 };
    DPoint3d const topRight = { 0.0 + halfWebThickness, halfDepth, 0.0 };
    DPoint3d const bottomRight = { flangeWidth - halfWebThickness, -halfDepth, 0.0 };
    DPoint3d const bottomLeft = { 0.0 - halfWebThickness, -halfDepth, 0.0 };

    DPoint3d const topInnerCorner = topRight - DPoint3d { webThickness, flangeThickness + slopeHeight };
    DPoint3d const bottomInnerCorner = bottomLeft - DPoint3d { -webThickness, -flangeThickness - slopeHeight };
    DPoint3d const topFlangeEdge = topLeft - DPoint3d { 0.0, flangeThickness };
    DPoint3d const bottomFlangEdge = bottomRight - DPoint3d { 0.0, -flangeThickness };

    ICurvePrimitivePtr topLine = ICurvePrimitive::CreateLine (topLeft, topRight);
    ICurvePrimitivePtr rightWebLine = ICurvePrimitive::CreateLine (topRight, bottomInnerCorner);
    ICurvePrimitivePtr bottomInnerCornerArc = nullptr;
    ICurvePrimitivePtr bottomFlangeSlopeLine = ICurvePrimitive::CreateLine (bottomInnerCorner, bottomFlangEdge);
    ICurvePrimitivePtr bottomFlangeEdgeArc = nullptr;
    ICurvePrimitivePtr bottomFlangeEdgeLine = ICurvePrimitive::CreateLine (bottomFlangEdge, bottomRight);
    ICurvePrimitivePtr bottomLine = ICurvePrimitive::CreateLine (bottomRight, bottomLeft);
    ICurvePrimitivePtr leftWebLine = ICurvePrimitive::CreateLine (bottomLeft, topInnerCorner);
    ICurvePrimitivePtr topInnerCornerArc = nullptr;
    ICurvePrimitivePtr topFlangeSlopeLine = ICurvePrimitive::CreateLine (topInnerCorner, topFlangeEdge);
    ICurvePrimitivePtr topFlangeEdgeArc = nullptr;
    ICurvePrimitivePtr topLeftFlangeEdgeLine = ICurvePrimitive::CreateLine (topFlangeEdge, topLeft);

    bottomFlangeEdgeArc = createArcBetweenLines (bottomFlangeSlopeLine, bottomFlangeEdgeLine, flangeEdgeRadius);
    topFlangeEdgeArc = createArcBetweenLines (topFlangeSlopeLine, topLeftFlangeEdgeLine, flangeEdgeRadius);

    bottomInnerCornerArc = createArcBetweenLines (rightWebLine, bottomFlangeSlopeLine, filletRadius);
    topInnerCornerArc = createArcBetweenLines (leftWebLine, topFlangeSlopeLine, filletRadius);

    bvector<ICurvePrimitivePtr> orderedCurves =
        {
        topLine, rightWebLine, bottomInnerCornerArc, bottomFlangeSlopeLine, bottomFlangeEdgeArc, bottomFlangeEdgeLine,
        bottomLine, leftWebLine, topInnerCornerArc, topFlangeSlopeLine, topFlangeEdgeArc, topLeftFlangeEdgeLine
        };
    return createGeometryFromPrimitiveArray (orderedCurves);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     01/2019
+---------------+---------------+---------------+---------------+---------------+------*/
static IGeometryPtr createCenterLineLShape(double halfWidth, double halfDepth, double wallThickness, double innerFilletRadius, double outerFilletRadius, double girth)
    {
    DPoint3d const tl_outerApex = { -halfWidth, halfDepth, 0.0 };
    DPoint3d const bl_outerApex = { -halfWidth, -halfDepth, 0.0 };
    DPoint3d const br_outerApex = { halfWidth, -halfDepth, 0.0 };

    DPoint3d const tl_outerGirth = { -(halfWidth - girth), halfDepth , 0.0 };
    DPoint3d const tl_innerGirth = { -(halfWidth - girth), halfDepth - wallThickness , 0.0 };

    DPoint3d const tl_innerApex = { -(halfWidth - wallThickness), halfDepth - wallThickness, 0.0 };
    DPoint3d const bl_innerApex = { -(halfWidth - wallThickness), -(halfDepth - wallThickness), 0.0 };
    DPoint3d const br_innerApex = { halfWidth - wallThickness, -(halfDepth - wallThickness), 0.0 };

    DPoint3d const br_outerGirth = { halfWidth, -(halfDepth - girth), 0.0 };
    DPoint3d const br_innerGirth = { (halfWidth - wallThickness), -(halfDepth - girth) , 0.0 };

    ICurvePrimitivePtr outerTopLine = ICurvePrimitive::CreateLine(tl_outerGirth, tl_outerApex);

    ICurvePrimitivePtr outerLeftLine = ICurvePrimitive::CreateLine(tl_outerApex, bl_outerApex);
    ICurvePrimitivePtr outerTopLeftArcLine = createArcBetweenLines(outerTopLine, outerLeftLine, outerFilletRadius);

    ICurvePrimitivePtr outerBottomLine = ICurvePrimitive::CreateLine(bl_outerApex, br_outerApex);
    ICurvePrimitivePtr outerBottomLeftArcLine = createArcBetweenLines(outerLeftLine, outerBottomLine, outerFilletRadius);

    ICurvePrimitivePtr outerRightGirthLine = ICurvePrimitive::CreateLine(br_outerApex, br_outerGirth);
    ICurvePrimitivePtr outerBottomRightArcLine = createArcBetweenLines(outerBottomLine, outerRightGirthLine, outerFilletRadius);

    ICurvePrimitivePtr rightGirthConnectLine = ICurvePrimitive::CreateLine(br_outerGirth, br_innerGirth);
    ICurvePrimitivePtr innerRightGirthLine = ICurvePrimitive::CreateLine(br_innerGirth, br_innerApex);

    ICurvePrimitivePtr innerBottomLine = ICurvePrimitive::CreateLine(br_innerApex, bl_innerApex);
    ICurvePrimitivePtr innerBottomRightArcLine = createArcBetweenLines(innerRightGirthLine, innerBottomLine, innerFilletRadius);

    ICurvePrimitivePtr innerLeftLine = ICurvePrimitive::CreateLine(bl_innerApex, tl_innerApex);
    ICurvePrimitivePtr innerBottomLeftArcLine = createArcBetweenLines(innerBottomLine, innerLeftLine, innerFilletRadius);

    ICurvePrimitivePtr innerTopLine = ICurvePrimitive::CreateLine(tl_innerApex, tl_innerGirth);
    ICurvePrimitivePtr innerTopArcLine = createArcBetweenLines(innerLeftLine, innerTopLine, innerFilletRadius);
    ICurvePrimitivePtr innerGirthConnectLine = ICurvePrimitive::CreateLine(tl_innerGirth, tl_outerGirth);

    bvector<ICurvePrimitivePtr> curves =
        {
        outerTopLine, outerTopLeftArcLine, outerLeftLine,
        outerBottomLeftArcLine, outerBottomLine, outerBottomRightArcLine,
        outerRightGirthLine, rightGirthConnectLine, innerRightGirthLine,
        innerBottomRightArcLine, innerBottomLine, innerBottomLeftArcLine,
        innerLeftLine, innerTopArcLine, innerTopLine, innerGirthConnectLine,
        };

    return createGeometryFromPrimitiveArray(curves);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     01/2019
+---------------+---------------+---------------+---------------+---------------+------*/
static IGeometryPtr createCenterLineLShape(double halfWidth, double halfDepth, double wallThickness, double innerFilletRadius, double outerFilletRadius)
    {
    DPoint3d const tl_outerApex = { -halfWidth, halfDepth, 0.0 };
    DPoint3d const bl_outerApex = { -halfWidth, -halfDepth, 0.0 };
    DPoint3d const br_outerApex = { halfWidth, -halfDepth, 0.0 };

    DPoint3d const tl_innerApex = { -(halfWidth - wallThickness), halfDepth, 0.0 };
    DPoint3d const bl_innerApex = { -(halfWidth - wallThickness), -(halfDepth - wallThickness), 0.0 };
    DPoint3d const br_innerApex = { halfWidth, -(halfDepth - wallThickness), 0.0 };

    ICurvePrimitivePtr outerLeftLine     = ICurvePrimitive::CreateLine(tl_outerApex, bl_outerApex);
    ICurvePrimitivePtr outerBottomLine   = ICurvePrimitive::CreateLine(bl_outerApex, br_outerApex);
    ICurvePrimitivePtr outerLeftArcLine  = createArcBetweenLines(outerLeftLine, outerBottomLine, outerFilletRadius);
    ICurvePrimitivePtr bottomConnectLine = ICurvePrimitive::CreateLine(br_outerApex, br_innerApex);
    ICurvePrimitivePtr innerBottomLine   = ICurvePrimitive::CreateLine(br_innerApex, bl_innerApex);
    ICurvePrimitivePtr innerLeftLine     = ICurvePrimitive::CreateLine(bl_innerApex, tl_innerApex);
    ICurvePrimitivePtr innerLeftArcLine  = createArcBetweenLines(innerBottomLine, innerLeftLine, innerFilletRadius);
    ICurvePrimitivePtr topConnectLine    = ICurvePrimitive::CreateLine(tl_innerApex, tl_outerApex);

    bvector<ICurvePrimitivePtr> curves =
        {
        outerLeftLine,  outerLeftArcLine, outerBottomLine,
        bottomConnectLine,  innerBottomLine, innerLeftArcLine,
        innerLeftLine, topConnectLine,
        };

    return createGeometryFromPrimitiveArray(curves);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     12/2018
+---------------+---------------+---------------+---------------+---------------+------*/
IGeometryPtr ProfilesGeometry::CreateCenterLineLShape(CenterLineLShapeProfile const& profile)
    {
    double const halfWidth = profile.GetWidth() / 2.0;
    double const halfDepth = profile.GetDepth() / 2.0;
    double const wallThickness = profile.GetWallThickness();
    double const innerFilletRadius = profile.GetFilletRadius();
    double const girth = profile.GetGirth();

    if (BeNumerical::IsEqualToZero (girth))
        return createCenterLineLShape (halfWidth, halfDepth, wallThickness, innerFilletRadius, innerFilletRadius + wallThickness);
    else
        return createCenterLineLShape (halfWidth, halfDepth, wallThickness, innerFilletRadius, innerFilletRadius + wallThickness, girth);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     01/2019
+---------------+---------------+---------------+---------------+---------------+------*/
static IGeometryPtr createCenterLineCShape(double halfWidth, double halfDepth, double wallThickness, double innerFilletRadius, double outerFilletRadius, double girth)
    {
    DPoint3d const tl_outerApex = { -halfWidth, halfDepth, 0.0 };
    DPoint3d const tl_innerApex = { -(halfWidth - wallThickness), halfDepth - wallThickness, 0.0 };
    DPoint3d const bl_outerApex = { -halfWidth, -halfDepth, 0.0 };
    DPoint3d const bl_innerApex = { -(halfWidth - wallThickness), -(halfDepth - wallThickness), 0.0 };
    DPoint3d const br_outerApex = { halfWidth, -halfDepth, 0.0 };
    DPoint3d const br_innerApex = { halfWidth - wallThickness, -(halfDepth - wallThickness), 0.0 };
    DPoint3d const tr_outerApex = { halfWidth, halfDepth, 0.0 };
    DPoint3d const tr_innerApex = { halfWidth - wallThickness, (halfDepth - wallThickness), 0.0 };
    DPoint3d const br_outerGirth = { halfWidth, -(halfDepth - girth), 0.0 };
    DPoint3d const br_innerGirth = { halfWidth - wallThickness, -(halfDepth - girth), 0.0 };
    DPoint3d const tr_innerGirth = { halfWidth - wallThickness, (halfDepth - girth), 0.0 };
    DPoint3d const tr_outerGirth = { halfWidth, (halfDepth - girth), 0.0 };

    ICurvePrimitivePtr topOuterGirthLine  = ICurvePrimitive::CreateLine (tr_outerGirth, tr_outerApex);
    ICurvePrimitivePtr topOuterLine  = ICurvePrimitive::CreateLine (tr_outerApex, tl_outerApex);
    ICurvePrimitivePtr topRightOuterArc = createArcBetweenLines (topOuterGirthLine, topOuterLine, outerFilletRadius);
    ICurvePrimitivePtr leftOuterLine  = ICurvePrimitive::CreateLine (tl_outerApex, bl_outerApex);
    ICurvePrimitivePtr topLeftOuterArc = createArcBetweenLines (topOuterLine, leftOuterLine, outerFilletRadius);
    ICurvePrimitivePtr bottomOuterLine  = ICurvePrimitive::CreateLine (bl_outerApex, br_outerApex);
    ICurvePrimitivePtr bottomLeftOuterArc = createArcBetweenLines (leftOuterLine, bottomOuterLine, outerFilletRadius);
    ICurvePrimitivePtr bottomGirthOuterLine  = ICurvePrimitive::CreateLine (br_outerApex, br_outerGirth);
    ICurvePrimitivePtr bottomGirthConnectLine = ICurvePrimitive::CreateLine (br_outerGirth, br_innerGirth);
    ICurvePrimitivePtr bottomGirthOuterArc = createArcBetweenLines(bottomOuterLine, bottomGirthOuterLine, outerFilletRadius);
    ICurvePrimitivePtr bottomGirthInnerLine = ICurvePrimitive::CreateLine (br_innerGirth, br_innerApex);
    ICurvePrimitivePtr bottomInnerLine = ICurvePrimitive::CreateLine (br_innerApex, bl_innerApex);
    ICurvePrimitivePtr bottomGirthInnerArc = createArcBetweenLines(bottomGirthInnerLine, bottomInnerLine, innerFilletRadius);
    ICurvePrimitivePtr leftInnerLine = ICurvePrimitive::CreateLine (bl_innerApex, tl_innerApex);
    ICurvePrimitivePtr bottomLeftInnerArc = createArcBetweenLines(bottomInnerLine, leftInnerLine, innerFilletRadius);
    ICurvePrimitivePtr topInnerLine = ICurvePrimitive::CreateLine (tl_innerApex, tr_innerApex);
    ICurvePrimitivePtr topLeftInnerArc = createArcBetweenLines(leftInnerLine, topInnerLine, innerFilletRadius);
    ICurvePrimitivePtr topGirthInnerLine = ICurvePrimitive::CreateLine (tr_innerApex, tr_innerGirth);
    ICurvePrimitivePtr topGirthInnerArc = createArcBetweenLines(topInnerLine, topGirthInnerLine, innerFilletRadius);
    ICurvePrimitivePtr topGirtConnectLine = ICurvePrimitive::CreateLine (tr_innerGirth, tr_outerGirth);

    bvector<ICurvePrimitivePtr> curves =
        {
        topOuterGirthLine, topRightOuterArc, topOuterLine,
        topLeftOuterArc, leftOuterLine, bottomLeftOuterArc,
        bottomOuterLine, bottomGirthOuterArc, bottomGirthOuterLine,
        bottomGirthConnectLine, bottomGirthInnerLine, bottomGirthInnerArc,
        bottomInnerLine, bottomLeftInnerArc, leftInnerLine,
        topLeftInnerArc, topInnerLine, topGirthInnerArc,
        topGirthInnerLine, topGirtConnectLine,
        };

    return createGeometryFromPrimitiveArray(curves);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     01/2019
+---------------+---------------+---------------+---------------+---------------+------*/
static IGeometryPtr createCenterLineCShape (double halfWidth, double halfDepth, double wallThickness, double innerFilletRadius, double outerFilletRadius)
    {
    DPoint3d const tl_outerApex = { -halfWidth, halfDepth, 0.0 };
    DPoint3d const tl_innerApex = { -(halfWidth - wallThickness), halfDepth - wallThickness, 0.0 };
    DPoint3d const bl_outerApex = { -halfWidth, -halfDepth, 0.0 };
    DPoint3d const bl_innerApex = { -(halfWidth - wallThickness), -(halfDepth - wallThickness), 0.0 };
    DPoint3d const br_outerApex = { halfWidth, -halfDepth, 0.0 };
    DPoint3d const br_innerApex = { halfWidth, -(halfDepth - wallThickness), 0.0 };
    DPoint3d const tr_outerApex = { halfWidth, halfDepth, 0.0 };
    DPoint3d const tr_innerApex = { halfWidth, (halfDepth - wallThickness), 0.0 };

    ICurvePrimitivePtr topOuterLine  = ICurvePrimitive::CreateLine (tr_outerApex, tl_outerApex);
    ICurvePrimitivePtr leftOuterLine = ICurvePrimitive::CreateLine (tl_outerApex, bl_outerApex);
    ICurvePrimitivePtr leftOuterTopArc = createArcBetweenLines (topOuterLine, leftOuterLine, outerFilletRadius);
    ICurvePrimitivePtr bottomOuterLine = ICurvePrimitive::CreateLine (bl_outerApex, br_outerApex);
    ICurvePrimitivePtr leftOuterBottomArc = createArcBetweenLines (leftOuterLine, bottomOuterLine, outerFilletRadius);
    ICurvePrimitivePtr bottomLinesConnector = ICurvePrimitive::CreateLine (br_outerApex, br_innerApex);
    ICurvePrimitivePtr bottomInnerLine = ICurvePrimitive::CreateLine (br_innerApex, bl_innerApex);
    ICurvePrimitivePtr leftInnerLine = ICurvePrimitive::CreateLine (bl_innerApex, tl_innerApex);
    ICurvePrimitivePtr leftInnerBottomArc = createArcBetweenLines (bottomInnerLine, leftInnerLine, innerFilletRadius);
    ICurvePrimitivePtr topInnerLine = ICurvePrimitive::CreateLine (tl_innerApex, tr_innerApex);
    ICurvePrimitivePtr leftInnerTopArc = createArcBetweenLines (leftInnerLine, topInnerLine, innerFilletRadius);
    ICurvePrimitivePtr topLinesConnector = ICurvePrimitive::CreateLine (tr_innerApex, tr_outerApex);

    bvector<ICurvePrimitivePtr> curves =
        {
        topOuterLine, leftOuterTopArc, leftOuterLine, leftOuterBottomArc, bottomOuterLine,
        bottomLinesConnector, bottomInnerLine, leftInnerBottomArc, leftInnerLine, leftInnerTopArc,
        topInnerLine, topLinesConnector,
        };

    return createGeometryFromPrimitiveArray(curves);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     12/2018
+---------------+---------------+---------------+---------------+---------------+------*/
IGeometryPtr ProfilesGeometry::CreateCenterLineCShape(CenterLineCShapeProfile const& profile)
    {
    double const halfWidth = profile.GetFlangeWidth() / 2.0;
    double const halfDepth = profile.GetDepth() / 2.0;
    double const wallThickness = profile.GetWallThickness();
    double const filletRadius = profile.GetFilletRadius();
    double const girth = profile.GetGirth();

    if (BeNumerical::IsEqualToZero (girth))
        return createCenterLineCShape (halfWidth, halfDepth, wallThickness, filletRadius, filletRadius + wallThickness);
    else
        return createCenterLineCShape (halfWidth, halfDepth, wallThickness, filletRadius, filletRadius + wallThickness, girth);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     02/2019
+---------------+---------------+---------------+---------------+---------------+------*/
static IGeometryPtr createArbitraryCenterLineShape (CurveVectorPtr const& curvesPtr, double wallThickness)
    {
    if (curvesPtr.IsNull())
        return nullptr;

    CurveOffsetOptions curveOffsetOptions (wallThickness / 2.0);

    CurveVectorCPtr rightOffsetedCurvesPtr = curvesPtr->CloneOffsetCurvesXY (curveOffsetOptions);
    CurveVectorCPtr leftOffsetedCurvesPtr = curvesPtr->CloneReversed()->CloneOffsetCurvesXY (curveOffsetOptions);

    DPoint3d rightStart, rightEnd, leftStart, leftEnd;
    rightOffsetedCurvesPtr->GetStartEnd (rightStart, rightEnd);
    leftOffsetedCurvesPtr->GetStartEnd (leftStart, leftEnd);

    ICurvePrimitivePtr firstCapPtr = ICurvePrimitive::CreateLine (rightEnd, leftStart);
    ICurvePrimitivePtr secondCapPtr = ICurvePrimitive::CreateLine (leftEnd, rightStart);

    // Note: adding a whole curveVector to combinedCurves creates an invalid geometry,
    // adding primitives one by one seems to work fine ..
    CurveVectorPtr combinedCurves = CurveVector::Create (CurveVector::BOUNDARY_TYPE_Outer);
    for (ICurvePrimitivePtr const& curvePtr : *rightOffsetedCurvesPtr)
        combinedCurves->Add (curvePtr);
    combinedCurves->Add (firstCapPtr);
    for (ICurvePrimitivePtr const& curvePtr : *leftOffsetedCurvesPtr)
        combinedCurves->Add (curvePtr);
    combinedCurves->Add (secondCapPtr);

    return IGeometry::Create (combinedCurves);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     02/2019
+---------------+---------------+---------------+---------------+---------------+------*/
static IGeometryPtr createArbitraryCenterLineShape (ICurvePrimitivePtr const& singleCurvePtr, double wallThickness)
    {
    if (singleCurvePtr.IsNull())
        return nullptr;

    CurveVectorPtr curvesPtr = CurveVector::Create (singleCurvePtr->Clone());
    return createArbitraryCenterLineShape (curvesPtr, wallThickness);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     02/2019
+---------------+---------------+---------------+---------------+---------------+------*/
IGeometryPtr ProfilesGeometry::CreateArbitraryCenterLineShape (IGeometry const& centerLine, double wallThickness)
    {
    switch (centerLine.GetGeometryType())
        {
        case IGeometry::GeometryType::CurvePrimitive:
            return createArbitraryCenterLineShape (centerLine.GetAsICurvePrimitive(), wallThickness);
        case IGeometry::GeometryType::CurveVector:
            return createArbitraryCenterLineShape (centerLine.GetAsCurveVector(), wallThickness);
        default:
            return nullptr;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     01/2019
+---------------+---------------+---------------+---------------+---------------+------*/
static IGeometryPtr createCenterLineForCShape (double halfWidth, double halfDepth, double wallThickness, double filletRadius)
    {
    double halfWallThickness = wallThickness / 2.0;

    DPoint3d const tr_Apex = { halfWidth, halfDepth - halfWallThickness, 0.0 };
    DPoint3d const tl_Apex = { -(halfWidth - halfWallThickness), halfDepth - halfWallThickness, 0.0 };
    DPoint3d const bl_Apex = { -(halfWidth - halfWallThickness), -(halfDepth - halfWallThickness), 0.0 };
    DPoint3d const br_Apex = { halfWidth, -(halfDepth - halfWallThickness), 0.0 };

    ICurvePrimitivePtr topLine = ICurvePrimitive::CreateLine (tr_Apex, tl_Apex);
    ICurvePrimitivePtr leftLine = ICurvePrimitive::CreateLine (tl_Apex, bl_Apex);
    ICurvePrimitivePtr topleftArc = createArcBetweenLines(topLine, leftLine, filletRadius);
    ICurvePrimitivePtr bottomLine = ICurvePrimitive::CreateLine (bl_Apex, br_Apex);
    ICurvePrimitivePtr bottomleftArc = createArcBetweenLines(leftLine, bottomLine, filletRadius);

    bvector<ICurvePrimitivePtr> curves =
        {
        topLine, topleftArc, leftLine, bottomleftArc, bottomLine,
        };

    return createGeometryFromPrimitiveArray(curves, CurveVector::BOUNDARY_TYPE_Open);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     01/2019
+---------------+---------------+---------------+---------------+---------------+------*/
static IGeometryPtr createCenterLineForCShape (double halfWidth, double halfDepth, double wallThickness, double filletRadius, double girth)
    {
    double halfWallThickness = wallThickness / 2.0;

    DPoint3d const tr_Apex = { halfWidth - halfWallThickness, halfDepth - halfWallThickness, 0.0 };
    DPoint3d const tl_Apex = { -(halfWidth - halfWallThickness), halfDepth - halfWallThickness, 0.0 };
    DPoint3d const bl_Apex = { -(halfWidth - halfWallThickness), -(halfDepth - halfWallThickness), 0.0 };
    DPoint3d const br_Apex = { halfWidth - halfWallThickness, -(halfDepth - halfWallThickness), 0.0 };

    DPoint3d const tr_Girth = { halfWidth - halfWallThickness, halfDepth - girth, 0.0 };
    DPoint3d const br_Girth = { halfWidth - halfWallThickness, -(halfDepth - girth), 0.0 };

    ICurvePrimitivePtr topGirthLine = ICurvePrimitive::CreateLine (tr_Girth, tr_Apex);
    ICurvePrimitivePtr topLine = ICurvePrimitive::CreateLine (tr_Apex, tl_Apex);
    ICurvePrimitivePtr topGirthArc = createArcBetweenLines(topGirthLine, topLine, filletRadius);
    ICurvePrimitivePtr leftLine = ICurvePrimitive::CreateLine (tl_Apex, bl_Apex);
    ICurvePrimitivePtr topleftArc = createArcBetweenLines(topLine, leftLine, filletRadius);
    ICurvePrimitivePtr bottomLine = ICurvePrimitive::CreateLine (bl_Apex, br_Apex);
    ICurvePrimitivePtr bottomLeftArc = createArcBetweenLines(leftLine, bottomLine, filletRadius);
    ICurvePrimitivePtr bottomGirthLine = ICurvePrimitive::CreateLine (br_Apex, br_Girth);
    ICurvePrimitivePtr bottomGirthArc = createArcBetweenLines(bottomLine, bottomGirthLine, filletRadius);

    bvector<ICurvePrimitivePtr> curves =
        {
        topGirthLine, topGirthArc, topLine,
        topleftArc, leftLine, bottomLeftArc, bottomLine,
        bottomGirthArc, bottomGirthLine,
        };

    return createGeometryFromPrimitiveArray(curves, CurveVector::BOUNDARY_TYPE_Open);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     01/2019
+---------------+---------------+---------------+---------------+---------------+------*/
IGeometryPtr ProfilesGeometry::CreateCenterLineForCShape (CenterLineCShapeProfile const& profile)
    {
    double const halfWidth = profile.GetFlangeWidth() / 2.0;
    double const halfDepth = profile.GetDepth() / 2.0;
    double const wallThickness = profile.GetWallThickness();
    double const filletRadius = profile.GetFilletRadius();
    double const girth = profile.GetGirth();

    if (BeNumerical::IsEqualToZero (girth))
        return createCenterLineForCShape (halfWidth, halfDepth, wallThickness, filletRadius + wallThickness / 2.0);
    else
        return createCenterLineForCShape (halfWidth, halfDepth, wallThickness, filletRadius + wallThickness / 2.0, girth);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     01/2019
+---------------+---------------+---------------+---------------+---------------+------*/
static IGeometryPtr createCenterLineForLShape (double halfWidth, double halfDepth, double wallThickness, double filletRadius)
    {
    double halfWallThickness = wallThickness / 2.0;

    DPoint3d const tl_Apex = { -(halfWidth - halfWallThickness), halfDepth, 0.0 };
    DPoint3d const bl_Apex = { -(halfWidth - halfWallThickness), -(halfDepth - halfWallThickness), 0.0 };
    DPoint3d const br_Apex = { halfWidth, -(halfDepth - halfWallThickness), 0.0 };

    ICurvePrimitivePtr leftLine = ICurvePrimitive::CreateLine(tl_Apex, bl_Apex);
    ICurvePrimitivePtr bottomLine = ICurvePrimitive::CreateLine(bl_Apex, br_Apex);
    ICurvePrimitivePtr bottomleftArc = createArcBetweenLines(leftLine, bottomLine, filletRadius);

    bvector<ICurvePrimitivePtr> curves = { leftLine, bottomleftArc, bottomLine };
    return createGeometryFromPrimitiveArray (curves, CurveVector::BOUNDARY_TYPE_Open);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     01/2019
+---------------+---------------+---------------+---------------+---------------+------*/
static IGeometryPtr createCenterLineForLShape(double halfWidth, double halfDepth, double wallThickness, double filletRadius, double girth)
    {
    double halfWallThickness = wallThickness / 2.0;

    DPoint3d const tr_Girth = { -(halfWidth - girth), halfDepth - halfWallThickness, 0.0 };
    DPoint3d const tl_Apex = { -(halfWidth - halfWallThickness), halfDepth - halfWallThickness, 0.0 };
    DPoint3d const bl_Apex = { -(halfWidth - halfWallThickness), -(halfDepth - halfWallThickness), 0.0 };
    DPoint3d const br_Apex = { halfWidth - halfWallThickness, -(halfDepth - halfWallThickness), 0.0 };
    DPoint3d const br_Girth = { (halfWidth - halfWallThickness), -(halfDepth - girth), 0.0 };

    ICurvePrimitivePtr topGirthLine = ICurvePrimitive::CreateLine (tr_Girth, tl_Apex);
    ICurvePrimitivePtr leftLine = ICurvePrimitive::CreateLine (tl_Apex, bl_Apex);
    ICurvePrimitivePtr topLeftArc = createArcBetweenLines (topGirthLine, leftLine, filletRadius);
    ICurvePrimitivePtr bottomLine = ICurvePrimitive::CreateLine (bl_Apex, br_Apex);
    ICurvePrimitivePtr bottomLeftArc = createArcBetweenLines (leftLine, bottomLine, filletRadius);
    ICurvePrimitivePtr rightGirthLine = ICurvePrimitive::CreateLine (br_Apex, br_Girth);
    ICurvePrimitivePtr bottomRightArc = createArcBetweenLines (bottomLine, rightGirthLine, filletRadius);

    bvector<ICurvePrimitivePtr> curves =
        {
        topGirthLine, topLeftArc, leftLine, bottomLeftArc, bottomLine, bottomRightArc, rightGirthLine,
        };

    return createGeometryFromPrimitiveArray (curves, CurveVector::BOUNDARY_TYPE_Open);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     01/2019
+---------------+---------------+---------------+---------------+---------------+------*/
IGeometryPtr ProfilesGeometry::CreateCenterLineForLShape(CenterLineLShapeProfile const& profile)
    {
    double const halfWidth = profile.GetWidth() / 2.0;
    double const halfDepth = profile.GetDepth() / 2.0;
    double const wallThickness = profile.GetWallThickness();
    double const filletRadius = profile.GetFilletRadius();
    double const girth = profile.GetGirth();

    if (BeNumerical::IsEqualToZero (girth))
        return createCenterLineForLShape (halfWidth, halfDepth, wallThickness, filletRadius + wallThickness / 2.0);
    else
        return createCenterLineForLShape (halfWidth, halfDepth, wallThickness, filletRadius + wallThickness / 2.0, girth);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     12/2018
+---------------+---------------+---------------+---------------+---------------+------*/
IGeometryPtr ProfilesGeometry::CreateCircle (CircleProfile const& profile)
    {
    DEllipse3d ellipse = DEllipse3d::FromCenterRadiusXY (DPoint3d::From (0.0, 0.0), profile.GetRadius());

    CurveVectorPtr curveVector = CurveVector::Create (CurveVector::BOUNDARY_TYPE_Inner);
    curveVector->Add (ICurvePrimitive::CreateArc (ellipse));

    return IGeometry::Create (curveVector);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     12/2018
+---------------+---------------+---------------+---------------+---------------+------*/
IGeometryPtr ProfilesGeometry::CreateHollowCircle (HollowCircleProfile const& profile)
    {
    DEllipse3d outerEllipse = DEllipse3d::FromCenterRadiusXY (DPoint3d::From (0.0, 0.0), profile.GetRadius());
    DEllipse3d innerEllipse = DEllipse3d::FromCenterRadiusXY (DPoint3d::From (0.0, 0.0), profile.GetRadius() - profile.GetWallThickness());

    CurveVectorPtr outerCurveVector = CurveVector::Create (CurveVector::BOUNDARY_TYPE_Outer);
    outerCurveVector->Add (ICurvePrimitive::CreateArc (outerEllipse));

    CurveVectorPtr innerCurveVector = CurveVector::Create (CurveVector::BOUNDARY_TYPE_Inner);
    innerCurveVector->Add (ICurvePrimitive::CreateArc (innerEllipse));

    CurveVectorPtr curveVector = CurveVector::Create (CurveVector::BOUNDARY_TYPE_ParityRegion);
    curveVector->Add (outerCurveVector);
    curveVector->Add (innerCurveVector);

    return IGeometry::Create (curveVector);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     12/2018
+---------------+---------------+---------------+---------------+---------------+------*/
IGeometryPtr ProfilesGeometry::CreateEllipse (EllipseProfile const& profile)
    {
    DEllipse3d ellipse = DEllipse3d::FromPoints (DPoint3d::From (0.0, 0.0), DPoint3d::From (profile.GetXRadius(), 0.0),
        DPoint3d::From (0.0, profile.GetYRadius()), 0.0, PI * 2.0);

    CurveVectorPtr curveVector = CurveVector::Create (CurveVector::BOUNDARY_TYPE_Inner);
    curveVector->Add (ICurvePrimitive::CreateArc (ellipse));

    return IGeometry::Create (curveVector);
    }

/*---------------------------------------------------------------------------------**//**
* Appends an array of ICurvePrimitives forming a rectangle shape to the given
* CurveVector. If roundingRadius is greater than zero - rectangle shape with rounded
* corners is produced.
* @bsimethod                                                                     12/2018
+---------------+---------------+---------------+---------------+---------------+------*/
static void appendRectangleToCurveVector (CurveVectorPtr& curveVector, double width, double depth, double roundingRadius)
    {
    BeAssert (curveVector->empty());

    double const halfWidth = width / 2.0;
    double const halfDepth = depth / 2.0;

    DPoint3d const topLeft = { -halfWidth, halfDepth, 0.0 };
    DPoint3d const topRight = { halfWidth, halfDepth, 0.0 };
    DPoint3d const bottomRight = { halfWidth, -halfDepth, 0.0 };
    DPoint3d const bottomLeft = { -halfWidth, -halfDepth, 0.0 };

    ICurvePrimitivePtr topLine = ICurvePrimitive::CreateLine (topLeft, topRight);
    ICurvePrimitivePtr rightLine = ICurvePrimitive::CreateLine (topRight, bottomRight);
    ICurvePrimitivePtr bottomLine = ICurvePrimitive::CreateLine (bottomRight, bottomLeft);
    ICurvePrimitivePtr leftLine = ICurvePrimitive::CreateLine (bottomLeft, topLeft);

    ICurvePrimitivePtr topRightArc = createArcBetweenLines (topLine, rightLine, roundingRadius);
    ICurvePrimitivePtr bottomRightArc = createArcBetweenLines (rightLine, bottomLine, roundingRadius);
    ICurvePrimitivePtr bottomLeftArc = createArcBetweenLines (bottomLine, leftLine, roundingRadius);
    ICurvePrimitivePtr topLeftArc = createArcBetweenLines (leftLine, topLine, roundingRadius);

    ICurvePrimitivePtr orderedCurves[] = { topLine, topRightArc, rightLine, bottomRightArc, bottomLine, bottomLeftArc, leftLine, topLeftArc };
    for (auto const& curvePtr : orderedCurves)
        {
        if (curvePtr.IsValid())
            curveVector->Add (curvePtr);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     12/2018
+---------------+---------------+---------------+---------------+---------------+------*/
IGeometryPtr ProfilesGeometry::CreateRectangle (RectangleProfile const& profile)
    {
    CurveVectorPtr curveVector = CurveVector::Create (CurveVector::BOUNDARY_TYPE_Outer);
    appendRectangleToCurveVector (curveVector, profile.GetWidth(), profile.GetDepth(), 0.0);

    return IGeometry::Create (curveVector);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     12/2018
+---------------+---------------+---------------+---------------+---------------+------*/
IGeometryPtr ProfilesGeometry::CreateRoundedRectangle (RoundedRectangleProfile const& profile)
    {
    CurveVectorPtr curveVector = CurveVector::Create (CurveVector::BOUNDARY_TYPE_Outer);
    appendRectangleToCurveVector (curveVector, profile.GetWidth(), profile.GetDepth(), profile.GetRoundingRadius());

    return IGeometry::Create (curveVector);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     12/2018
+---------------+---------------+---------------+---------------+---------------+------*/
IGeometryPtr ProfilesGeometry::CreateHollowRectangle (HollowRectangleProfile const& profile)
    {
    double const width = profile.GetWidth();
    double const depth = profile.GetDepth();
    double const doubleThickness = profile.GetWallThickness() * 2.0;
    double const outerRadius = profile.GetOuterFilletRadius();
    double const innerRadius = profile.GetInnerFilletRadius();

    CurveVectorPtr outerCurveVector = CurveVector::Create (CurveVector::BOUNDARY_TYPE_Outer);
    appendRectangleToCurveVector (outerCurveVector, width, depth, outerRadius);

    CurveVectorPtr innerCurveVector = CurveVector::Create (CurveVector::BOUNDARY_TYPE_Inner);
    appendRectangleToCurveVector (innerCurveVector, width - doubleThickness, depth - doubleThickness, innerRadius);

    CurveVectorPtr curveVector = CurveVector::Create (CurveVector::BOUNDARY_TYPE_ParityRegion);
    curveVector->Add (outerCurveVector);
    curveVector->Add (innerCurveVector);

    return IGeometry::Create (curveVector);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     12/2018
+---------------+---------------+---------------+---------------+---------------+------*/
IGeometryPtr ProfilesGeometry::CreateTrapezium (TrapeziumProfile const& profile)
    {
    double const topWidth = profile.GetTopWidth();
    double const halfBottomWidth = profile.GetBottomWidth() / 2.0;
    double const halfDepth = profile.GetDepth() / 2.0;
    double const topOffset = profile.GetTopOffset();

    DPoint3d const topLeft = { -halfBottomWidth + topOffset, halfDepth, 0.0 };
    DPoint3d const topRight = { -halfBottomWidth + topOffset + topWidth, halfDepth, 0.0 };
    DPoint3d const bottomRight = { halfBottomWidth, -halfDepth, 0.0 };
    DPoint3d const bottomLeft = { -halfBottomWidth, -halfDepth, 0.0 };

    ICurvePrimitivePtr topLine = ICurvePrimitive::CreateLine (topLeft, topRight);
    ICurvePrimitivePtr rightLine = ICurvePrimitive::CreateLine (topRight, bottomRight);
    ICurvePrimitivePtr bottomLine = ICurvePrimitive::CreateLine (bottomRight, bottomLeft);
    ICurvePrimitivePtr leftLine = ICurvePrimitive::CreateLine (bottomLeft, topLeft);

    bvector<ICurvePrimitivePtr> orderedCurves = { topLine, rightLine, bottomLine, leftLine };
    return createGeometryFromPrimitiveArray (orderedCurves);
    }

/*---------------------------------------------------------------------------------**//**
* If capsules depth is greater than width - construct horizontal capsule and simply
* rotate the geometry by 90 degrees.
* NOTE: By doing so, size of the geometry doesn't change - the transformation matrix
* is stored in the DB anyway.
* @bsimethod                                                                     01/2019
+---------------+---------------+---------------+---------------+---------------+------*/
IGeometryPtr ProfilesGeometry::CreateCapsule (CapsuleProfile const& profile)
    {
    double halfWidth = profile.GetWidth() / 2.0;
    double halfDepth = profile.GetDepth() / 2.0;

    bool const needsRotation = BeNumerical::IsLess (halfWidth, halfDepth);
    if (needsRotation)
        std::swap (halfWidth, halfDepth);

    DPoint3d const topLeft = { -halfWidth + halfDepth, halfDepth, 0.0 };
    DPoint3d const topRight = { halfWidth - halfDepth, halfDepth, 0.0 };

    DPoint3d const bottomRight = { halfWidth - halfDepth, -halfDepth, 0.0 };
    DPoint3d const bottomLeft = { -halfWidth + halfDepth, -halfDepth, 0.0 };

    DPoint3d const middleLeft = { -halfWidth, 0.0, 0.0 };
    DPoint3d const middleRight = { halfWidth, 0.0, 0.0 };

    ICurvePrimitivePtr topLine = ICurvePrimitive::CreateLine (topLeft, topRight);
    ICurvePrimitivePtr rightArc = ICurvePrimitive::CreateArc (DEllipse3d::FromPointsOnArc (topRight, middleRight, bottomRight));
    ICurvePrimitivePtr bottomLine = ICurvePrimitive::CreateLine (bottomRight, bottomLeft);
    ICurvePrimitivePtr leftArc = ICurvePrimitive::CreateArc (DEllipse3d::FromPointsOnArc (bottomLeft, middleLeft, topLeft));

    bvector<ICurvePrimitivePtr> orderedCurves = { topLine, rightArc, bottomLine, leftArc };
    IGeometryPtr geometryPtr = createGeometryFromPrimitiveArray (orderedCurves);

    if (needsRotation)
        {
        DMatrix4d rotation = DMatrix4d::From (RotMatrix::FromVectorAndRotationAngle (DVec3d::From (0.0, 0.0, 1.0), PI / 2.0));
        Transform transform;
        transform.InitFrom (rotation);
        geometryPtr->TryTransformInPlace (transform);
        }

    return geometryPtr;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                                     01/2019
+---------------+---------------+---------------+---------------+---------------+------*/
IGeometryPtr ProfilesGeometry::CreateRegularPolygon (RegularPolygonProfile const& profile)
    {
    uint64_t const sideCount = profile.GetSideCount();
    double const sideLength = profile.GetSideLength();
    Angle const theta = Angle::FromRadians ((PI * 2.0) / (double)sideCount);

    // Position the first corner at the very top (along YAxis)
    Angle currentAngle = Angle::FromRadians (PI / 2.0);

    bvector<ICurvePrimitivePtr> curves ((size_t)sideCount);
    for (uint64_t i = 0; i < sideCount; ++i)
        {
        double const x1 = sideLength * currentAngle.Cos();
        double const y1 = sideLength * currentAngle.Sin();

        currentAngle = currentAngle + theta;

        double const x2 = sideLength * currentAngle.Cos();
        double const y2 = sideLength * currentAngle.Sin();

        ICurvePrimitivePtr linePtr = ICurvePrimitive::CreateLine (DPoint3d::From (x1, y1), DPoint3d::From (x2, y2));
        curves.push_back (linePtr);
        }

    return createGeometryFromPrimitiveArray (curves);
    }

/*---------------------------------------------------------------------------------**//**
* 'singleProfile' passed by reference and not directly retrieved from 'doubleProfile'
* because of support for geometry update case (see Profile::UpdateGeometry()).
* @bsimethod                                                                     01/2019
+---------------+---------------+---------------+---------------+---------------+------*/
IGeometryPtr ProfilesGeometry::CreateDoubleLShape (DoubleLShapeProfile const& doubleProfile, LShapeProfile const& singleProfile)
    {
    IGeometryPtr singleProfileGeometryPtr = singleProfile.GetShape();
    if (singleProfileGeometryPtr.IsNull())
        return nullptr;

    double const width = singleProfile.GetWidth();
    double const depth = singleProfile.GetDepth();

    bool const needsRotation = depth > width && doubleProfile.GetType() == DoubleLShapeProfileType::SLBB ||
                               width > depth && doubleProfile.GetType() == DoubleLShapeProfileType::LLBB;
    double const singleSideOffset = doubleProfile.GetSpacing() / 2.0 + (needsRotation ? depth : width) / 2.0;

    DMatrix4d rightSideMatrix, leftSideMatrix;
    if (needsRotation)
        {
        DMatrix4d rotation = DMatrix4d::From (RotMatrix::FromVectorAndRotationAngle (DVec3d::From (0.0, 0.0, 1.0), -PI / 2.0));
        DMatrix4d rightSideTranslation = DMatrix4d::FromScaleAndTranslation (DPoint3d::From (1.0, 1.0, 1.0), DPoint3d::From (singleSideOffset, 0.0, 0.0));
        DMatrix4d leftSideTranslation = DMatrix4d::FromScaleAndTranslation (DPoint3d::From (-1.0, 1.0, 1.0), DPoint3d::From (-singleSideOffset, 0.0, 0.0));

        rightSideMatrix = rightSideTranslation * rotation;
        leftSideMatrix = leftSideTranslation * rotation;
        }
    else
        {
        rightSideMatrix = DMatrix4d::FromScaleAndTranslation (DPoint3d::From (1.0, -1.0, 1.0), DPoint3d::From (singleSideOffset, 0.0, 0.0));
        leftSideMatrix = DMatrix4d::FromScaleAndTranslation (DPoint3d::From (-1.0, -1.0, 1.0), DPoint3d::From (-singleSideOffset, 0.0, 0.0));
        }

    Transform rightSideTransform, leftSideTransform;
    BeAssert (rightSideTransform.InitFrom (rightSideMatrix));
    BeAssert (leftSideTransform.InitFrom (leftSideMatrix));

    CurveVectorPtr curveVector = CurveVector::Create (CurveVector::BOUNDARY_TYPE_UnionRegion);
    curveVector->Add (singleProfileGeometryPtr->Clone (rightSideTransform)->GetAsCurveVector());
    curveVector->Add (singleProfileGeometryPtr->Clone (leftSideTransform)->GetAsCurveVector());

    return IGeometry::Create (curveVector);
    }

/*---------------------------------------------------------------------------------**//**
* 'singleProfile' passed by reference and not directly retrieved from 'doubleProfile'
* because of support for geometry update case (see Profile::UpdateGeometry()).
* @bsimethod                                                                     01/2019
+---------------+---------------+---------------+---------------+---------------+------*/
IGeometryPtr ProfilesGeometry::CreateDoubleCShape (DoubleCShapeProfile const& doubleProfile, CShapeProfile const& singleProfile)
    {
    IGeometryPtr singleProfileGeometryPtr = singleProfile.GetShape();
    if (singleProfileGeometryPtr.IsNull())
        return nullptr;

    double const singleSideOffset = doubleProfile.GetSpacing() / 2.0 + singleProfile.GetFlangeWidth() / 2.0;

    DMatrix4d rightSideMatrix = DMatrix4d::FromTranslation (singleSideOffset, 0.0, 0.0);
    DMatrix4d leftSideMatrix = DMatrix4d::FromScaleAndTranslation (DPoint3d::From (-1.0, 1.0, 1.0), DPoint3d::From (-singleSideOffset, 0.0, 0.0));

    Transform rightSideTransform, leftSideTransform;
    BeAssert (rightSideTransform.InitFrom (rightSideMatrix));
    BeAssert (leftSideTransform.InitFrom (leftSideMatrix));

    CurveVectorPtr curveVector = CurveVector::Create (CurveVector::BOUNDARY_TYPE_UnionRegion);
    curveVector->Add (singleProfileGeometryPtr->Clone (rightSideTransform)->GetAsCurveVector());
    curveVector->Add (singleProfileGeometryPtr->Clone (leftSideTransform)->GetAsCurveVector());

    return IGeometry::Create (curveVector);
    }

/*---------------------------------------------------------------------------------**//**
* 'updatedProfilePtr' is used to update composite geometry when one of the referenced
* profiles was updated (from _UpdateShapeGeometry()). 'updatedProfilePtr' might be null if
* the geometry is being created for the first time (from _CreateShapeGeometry()).
* @bsimethod                                                                     01/2019
+---------------+---------------+---------------+---------------+---------------+------*/
IGeometryPtr ProfilesGeometry::CreateArbitraryCompositeShape (ArbitraryCompositeProfile const& profile, SinglePerimeterProfileCPtr updatedProfilePtr)
    {
    CurveVectorPtr curveVector = CurveVector::Create (CurveVector::BOUNDARY_TYPE_UnionRegion);

    bvector<ArbitraryCompositeProfileComponent> components = profile.GetComponents();
    for (auto const& component : components)
        {
        ProfileCPtr singleProfilePtr = Profile::Get (profile.GetDgnDb(), component.singleProfileId);
        if (singleProfilePtr.IsNull())
            return nullptr;

        DPoint3d scale = DPoint3d::From (1.0, component.mirrorAboutYAxis ? -1.0 : 1.0, 1.0);
        DPoint3d translation = DPoint3d::From (component.offset);

        DMatrix4d transformMatrix = DMatrix4d::FromScaleAndTranslation (scale, translation);
        if (!BeNumerical::IsEqualToZero (component.rotation.Radians()))
            {
            DMatrix4d rotationMatrix = DMatrix4d::From (RotMatrix::FromVectorAndRotationAngle (DVec3d::From (0.0, 0.0, 1.0), component.rotation.Radians()));
            transformMatrix = transformMatrix * rotationMatrix;
            }

        Transform transform;
        BeAssert (transform.InitFrom (transformMatrix));

        IGeometryPtr profileGeometryPtr = nullptr;
        if (updatedProfilePtr.IsValid() && updatedProfilePtr->GetElementId() == singleProfilePtr->GetElementId())
            profileGeometryPtr = updatedProfilePtr->GetShape();
        else
            profileGeometryPtr = singleProfilePtr->GetShape();

        curveVector->Add (profileGeometryPtr->Clone (transform)->GetAsCurveVector());
        }

    return IGeometry::Create (curveVector);
    }

/*---------------------------------------------------------------------------------**//**
* 'baseProfile' passed by reference and not directly retrieved from 'profile'
* because of support for geometry update case (see Profile::UpdateGeometry()).
* @bsimethod                                                                     01/2019
+---------------+---------------+---------------+---------------+---------------+------*/
IGeometryPtr ProfilesGeometry::CreateDerivedShape (DerivedProfile const& profile, SinglePerimeterProfile const& baseProfile)
    {
    DPoint3d scale = DPoint3d::From (profile.GetScale());
    DPoint3d const translation = DPoint3d::From (profile.GetOffset());
    Angle const rotation = profile.GetRotation();

    if (profile.GetMirrorAboutYAxis())
        scale.x *= -1.0;

    DMatrix4d transformMatrix = DMatrix4d::FromScaleAndTranslation (scale, translation);
    if (!BeNumerical::IsEqualToZero (rotation.Radians()))
        {
        DMatrix4d const rotationMatrix = DMatrix4d::From (RotMatrix::FromVectorAndRotationAngle (DVec3d::From (0.0, 0.0, 1.0), rotation.Radians()));
        transformMatrix = transformMatrix * rotationMatrix;
        }

    Transform transform;
    BeAssert (transform.InitFrom (transformMatrix));

    return baseProfile.GetShape()->Clone (transform);
    }

END_BENTLEY_PROFILES_NAMESPACE
