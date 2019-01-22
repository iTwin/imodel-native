/*--------------------------------------------------------------------------------------+
|
|     $Source: Profiles/src/ProfilesGeometry.cpp $
|
|  $Copyright: (c) 2019 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ProfilesInternal.h"
#include <ProfilesInternal\ProfilesGeometry.h>

USING_NAMESPACE_BENTLEY_DGN

BEGIN_BENTLEY_PROFILES_NAMESPACE

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
static IGeometryPtr createGeometryFromPrimitiveArray (bvector<ICurvePrimitivePtr>& orderedCurves)
    {
    CurveVectorPtr curveVector = CurveVector::Create (CurveVector::BOUNDARY_TYPE_Outer);
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
    ICurvePrimitivePtr topFlangeEdgeArc = nullptr;
    ICurvePrimitivePtr bottomFlangeEdgeArc = nullptr;
    ICurvePrimitivePtr bottomInnerCornerArc = nullptr;
    ICurvePrimitivePtr topInnerCornerArc = nullptr;

    if (profile.GetFlangeEdgeRadius() > 0.0)
        {
        topFlangeEdgeArc = createArcBetweenLines (topFlangeEdgeLine, topFlangeSlopeLine, flangeEdgeRadius);
        bottomFlangeEdgeArc = createArcBetweenLines (bottomFlangeSlopeLine, bottomFlangeEdgeLine, flangeEdgeRadius);
        }

    if (profile.GetFilletRadius() > 0.0)
        {
        topInnerCornerArc = createArcBetweenLines (topFlangeSlopeLine, innerWebLine, filletRadius);
        bottomInnerCornerArc = createArcBetweenLines (innerWebLine, bottomFlangeSlopeLine, filletRadius);
        }

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

    if (profile.GetFlangeEdgeRadius() > 0.0)
        {
        topRightFlangeEdgeArc = createArcBetweenLines (topRightFlangeEdgeLine, topRightFlangeSlopeLine, flangeEdgeRadius);
        bottomRightFlangeEdgeArc = createArcBetweenLines (bottomRightFlangeSlopeLine, bottomRightFlangeEdgeLine, flangeEdgeRadius);
        bottomLeftFlangeEdgeArc = createArcBetweenLines (bottomLeftFlangeEdgeLine, bottomLeftFlangeSlopeLine, flangeEdgeRadius);
        topLeftFlangeEdgeArc = createArcBetweenLines (topLeftFlangeSlopeLine, topLeftFlangeEdgeLine, flangeEdgeRadius);
        }

    if (profile.GetFilletRadius() > 0.0)
        {
        topRightInnerCornerArc = createArcBetweenLines (topRightFlangeSlopeLine, innerRightWebLine, filletRadius);
        bottomRightInnerCornerArc = createArcBetweenLines (innerRightWebLine, bottomRightFlangeSlopeLine, filletRadius);
        bottomLeftInnerCornerArc = createArcBetweenLines (bottomLeftFlangeSlopeLine, innerLeftWebLine, filletRadius);
        topLeftInnerCornerArc = createArcBetweenLines (innerLeftWebLine, topLeftFlangeSlopeLine, filletRadius);
        }

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

    if (topFlangeEdgeRadius > 0.0)
        {
        topRightFlangeEdgeArc = createArcBetweenLines (topRightFlangeEdgeLine, topRightFlangeSlopeLine, topFlangeEdgeRadius);
        topLeftFlangeEdgeArc = createArcBetweenLines (topLeftFlangeSlopeLine, topLeftFlangeEdgeLine, topFlangeEdgeRadius);
        }

    if (bottomFlangeEdgeRadius > 0.0)
        {
        bottomRightFlangeEdgeArc = createArcBetweenLines (bottomRightFlangeSlopeLine, bottomRightFlangeEdgeLine, bottomFlangeEdgeRadius);
        bottomLeftFlangeEdgeArc = createArcBetweenLines (bottomLeftFlangeEdgeLine, bottomLeftFlangeSlopeLine, bottomFlangeEdgeRadius);
        }

    if (topFlangeFilletRadius > 0.0)
        {
        topRightInnerCornerArc = createArcBetweenLines (topRightFlangeSlopeLine, innerRightWebLine, topFlangeFilletRadius);
        topLeftInnerCornerArc = createArcBetweenLines (innerLeftWebLine, topLeftFlangeSlopeLine, topFlangeFilletRadius);
        }

    if (bottomFlangeFilletRadius > 0.0)
        {
        bottomRightInnerCornerArc = createArcBetweenLines (innerRightWebLine, bottomRightFlangeSlopeLine, bottomFlangeFilletRadius);
        bottomLeftInnerCornerArc = createArcBetweenLines (bottomLeftFlangeSlopeLine, innerLeftWebLine, bottomFlangeFilletRadius);
        }

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

    if (edgeRadius > 0.0)
        {
        webEdgeArc = createArcBetweenLines (topLine, webSlopeLine, edgeRadius);
        bottomRigthEdgeArc = createArcBetweenLines (flangeSlopeLine, rightLine, edgeRadius);
        }

    if (filletRadius > 0.0)
        bottomInnerCornerArc = createArcBetweenLines (webSlopeLine, flangeSlopeLine, filletRadius);

    bvector<ICurvePrimitivePtr> orderedCurves =
        {
        topLine, webEdgeArc, webSlopeLine, bottomInnerCornerArc, flangeSlopeLine, bottomRigthEdgeArc, rightLine, bottomLine, leftLine
        };
    return createGeometryFromPrimitiveArray (orderedCurves);
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
    DPoint3d const bottomMiddle = { 0.0, -halfDepth, 0.0 };
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

    if (flangeEdgeRadius > 0.0)
        {
        topRightFlangeEdgeArc = createArcBetweenLines (topRightFlangeEdgeLine, topRightFlangeSlopeLine, flangeEdgeRadius);
        topLeftFlangeEdgeArc = createArcBetweenLines (topLeftFlangeSlopeLine, topLeftFlangeEdgeLine, flangeEdgeRadius);
        }

    if (webEdgeRadius > 0.0)
        {
        bottomRightWebEdgeArc = createArcBetweenLines (innerRightWebLine, bottomLine, webEdgeRadius);
        bottomLeftWebEdgeArc = createArcBetweenLines (bottomLine, innerLeftWebLine, webEdgeRadius);

        if (bottomLine->GetLineCP()->Length() == 0.0)
            bottomLine = nullptr;
        }

    if (filletRadius > 0.0)
        {
        topRightInnerCornerArc = createArcBetweenLines (topRightFlangeSlopeLine, innerRightWebLine, filletRadius);
        topLeftInnerCornerArc = createArcBetweenLines (innerLeftWebLine, topLeftFlangeSlopeLine, filletRadius);
        }

    bvector<ICurvePrimitivePtr> orderedCurves =
        {
        topLine, topRightFlangeEdgeLine, topRightFlangeEdgeArc, topRightFlangeSlopeLine, topRightInnerCornerArc, innerRightWebLine, bottomRightWebEdgeArc,
        bottomLine, bottomLeftWebEdgeArc, innerLeftWebLine, topLeftInnerCornerArc, topLeftFlangeSlopeLine, topLeftFlangeEdgeArc, topLeftFlangeEdgeLine
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

    if (profile.GetFlangeEdgeRadius() > 0.0)
        {
        bottomFlangeEdgeArc = createArcBetweenLines (bottomFlangeSlopeLine, bottomFlangeEdgeLine, flangeEdgeRadius);
        topFlangeEdgeArc = createArcBetweenLines (topFlangeSlopeLine, topLeftFlangeEdgeLine, flangeEdgeRadius);
        }

    if (profile.GetFilletRadius() > 0.0)
        {
        bottomInnerCornerArc = createArcBetweenLines (rightWebLine, bottomFlangeSlopeLine, filletRadius);
        topInnerCornerArc = createArcBetweenLines (leftWebLine, topFlangeSlopeLine, filletRadius);
        }

    bvector<ICurvePrimitivePtr> orderedCurves =
        {
        topLine, rightWebLine, bottomInnerCornerArc, bottomFlangeSlopeLine, bottomFlangeEdgeArc, bottomFlangeEdgeLine,
        bottomLine, leftWebLine, topInnerCornerArc, topFlangeSlopeLine, topFlangeEdgeArc, topLeftFlangeEdgeLine
        };
    return createGeometryFromPrimitiveArray (orderedCurves);
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
    double const outerFilletRadius = profile.GetFilletRadius() + wallThickness;
    double const girth = profile.GetGirth();

    DPoint3d tl_outerGirthEnd   = { -(halfWidth - girth), halfDepth, 0.0 };
    DPoint3d tl_outerGirthStart = { -(halfWidth - wallThickness - innerFilletRadius), halfDepth, 0.0 };
    DPoint3d const tl_outerRangeApex = { -halfWidth, halfDepth, 0.0};
    DPoint3d tl_outerLeft = { -halfWidth, halfDepth - wallThickness - innerFilletRadius, 0.0 };
    DPoint3d const bl_outerLeft = { -halfWidth, -(halfDepth - wallThickness - innerFilletRadius), 0.0 };
    DPoint3d const bl_outerRangeApex = { -halfWidth, -halfDepth, 0.0 };
    DPoint3d const bl_outerBottom = { -(halfWidth - wallThickness - innerFilletRadius), -halfDepth, 0.0 };
    DPoint3d br_outerBottom = { (halfWidth - wallThickness - innerFilletRadius), -halfDepth, 0.0 };
    DPoint3d const br_outerRangeApex = { halfWidth, -halfDepth, 0.0 };
    DPoint3d br_outerGirthStart = { halfWidth, -(halfDepth - wallThickness - innerFilletRadius), 0.0 };
    DPoint3d br_outerGirthEnd = { halfWidth, -(halfDepth - girth) };
    DPoint3d br_innerGirthEnd = { halfWidth - wallThickness, -(halfDepth - girth) };
    DPoint3d br_innerGirthStart = { halfWidth - wallThickness, -(halfDepth - wallThickness - innerFilletRadius), 0.0 };
    DPoint3d br_innerRangeApex = { halfWidth - wallThickness, -(halfDepth - wallThickness), 0.0 };
    DPoint3d br_innerBottom = { (halfWidth - wallThickness - innerFilletRadius), -(halfDepth - wallThickness), 0.0 };
    DPoint3d const bl_innerBottom = { -(halfWidth - wallThickness - innerFilletRadius), -(halfDepth - wallThickness), 0.0 };
    DPoint3d const bl_innerRangeApex = { -(halfWidth - wallThickness), -(halfDepth - wallThickness), 0.0 };
    DPoint3d const bl_innerLeft = { -(halfWidth - wallThickness), -(halfDepth - wallThickness - innerFilletRadius), 0.0 };
    DPoint3d tl_innerLeft = { -(halfWidth - wallThickness), halfDepth - wallThickness - innerFilletRadius, 0.0 };
    DPoint3d tl_innerRangeApex = { -(halfWidth - wallThickness), halfDepth - wallThickness, 0.0};
    DPoint3d tl_innerGirthStart = { -(halfWidth - wallThickness - innerFilletRadius), halfDepth - wallThickness, 0.0 };
    DPoint3d tl_innerGirthEnd = { -(halfWidth - girth), halfDepth - wallThickness, 0.0 };

    if (BeNumerical::IsEqualToZero(girth))
        {
        tl_innerRangeApex  = { -(halfWidth - wallThickness), halfDepth, 0.0 };
        tl_outerGirthStart = tl_innerRangeApex;
        tl_outerGirthEnd   = tl_innerRangeApex;
        tl_innerGirthEnd   = tl_innerRangeApex;
        tl_innerGirthStart = tl_innerRangeApex;
        tl_innerLeft = tl_innerRangeApex;
        tl_outerLeft = tl_outerRangeApex;

        br_innerRangeApex = {halfWidth, -(halfDepth - wallThickness), 0.0};
        br_outerGirthStart = br_innerRangeApex;
        br_outerGirthEnd   = br_innerRangeApex;
        br_innerGirthStart = br_innerRangeApex;
        br_innerGirthEnd   = br_innerRangeApex;
        br_innerBottom = br_innerRangeApex;
        br_outerBottom = br_outerRangeApex;
        }

    ICurvePrimitivePtr line1  = ICurvePrimitive::CreateLine (tl_outerGirthEnd, tl_outerGirthStart);
    ICurvePrimitivePtr line2  = ICurvePrimitive::CreateLine (tl_outerGirthStart, tl_outerRangeApex);
    ICurvePrimitivePtr line3  = ICurvePrimitive::CreateLine (tl_outerRangeApex, tl_outerLeft);

    if (BeNumerical::IsGreaterThanZero (outerFilletRadius) && BeNumerical::IsGreaterThanZero(girth))
        {
        line2 = createArcBetweenLines (line2, line3, outerFilletRadius);
        line3 = nullptr;
        }

    ICurvePrimitivePtr line4  = ICurvePrimitive::CreateLine (tl_outerLeft, bl_outerLeft);
    ICurvePrimitivePtr line5  = ICurvePrimitive::CreateLine (bl_outerLeft, bl_outerRangeApex);
    ICurvePrimitivePtr line6  = ICurvePrimitive::CreateLine (bl_outerRangeApex, bl_outerBottom);

    if (BeNumerical::IsGreaterThanZero (outerFilletRadius))
        {
        line5 = createArcBetweenLines (line5, line6, outerFilletRadius);
        line6 = nullptr;
        }

    ICurvePrimitivePtr line7  = ICurvePrimitive::CreateLine (bl_outerBottom, br_outerBottom);
    ICurvePrimitivePtr line8  = ICurvePrimitive::CreateLine (br_outerBottom, br_outerRangeApex);
    ICurvePrimitivePtr line9  = ICurvePrimitive::CreateLine (br_outerRangeApex, br_outerGirthStart);

    if (BeNumerical::IsGreaterThanZero (outerFilletRadius) && BeNumerical::IsGreaterThanZero (girth))
        {
        line8 = createArcBetweenLines(line8, line9, outerFilletRadius);
        line9 = nullptr;
        }

    ICurvePrimitivePtr line10 = ICurvePrimitive::CreateLine (br_outerGirthStart, br_outerGirthEnd);
    ICurvePrimitivePtr line11 = ICurvePrimitive::CreateLine (br_outerGirthEnd, br_innerGirthEnd);
    ICurvePrimitivePtr line12 = ICurvePrimitive::CreateLine (br_innerGirthEnd, br_innerGirthStart);
    ICurvePrimitivePtr line13 = ICurvePrimitive::CreateLine (br_innerGirthStart, br_innerRangeApex);
    ICurvePrimitivePtr line14 = ICurvePrimitive::CreateLine (br_innerRangeApex, br_innerBottom);

    if (BeNumerical::IsGreaterThanZero (innerFilletRadius))
        {
        line13 = createArcBetweenLines (line13, line14, innerFilletRadius);
        line14 = nullptr;
        }
    
    ICurvePrimitivePtr line15 = ICurvePrimitive::CreateLine (br_innerBottom, bl_innerBottom);//
    ICurvePrimitivePtr line16 = ICurvePrimitive::CreateLine (bl_innerBottom, bl_innerRangeApex);
    ICurvePrimitivePtr line17 = ICurvePrimitive::CreateLine (bl_innerRangeApex, bl_innerLeft);

    if (BeNumerical::IsGreaterThanZero (innerFilletRadius))
        {
        line16 = createArcBetweenLines (line16, line17, innerFilletRadius);
        line17 = nullptr;
        }
   
    ICurvePrimitivePtr line18 = ICurvePrimitive::CreateLine (bl_innerLeft, tl_innerLeft);
    ICurvePrimitivePtr line19 = ICurvePrimitive::CreateLine (tl_innerLeft, tl_innerRangeApex);
    ICurvePrimitivePtr line20 = ICurvePrimitive::CreateLine (tl_innerRangeApex, tl_innerGirthStart);

    if (BeNumerical::IsGreaterThanZero (innerFilletRadius) && BeNumerical::IsGreaterThanZero (girth))
        {
        line19 = createArcBetweenLines(line19, line20, innerFilletRadius);
        line20 = nullptr;
        }

    ICurvePrimitivePtr line21 = ICurvePrimitive::CreateLine (tl_innerGirthStart, tl_innerGirthEnd);
    ICurvePrimitivePtr line22 = ICurvePrimitive::CreateLine (tl_innerGirthEnd, tl_outerGirthEnd);

    bvector<ICurvePrimitivePtr> curves =
        {
        line1,  line2, line3,
        line4,  line5, line6,
        line7,  line8, line9,
        line10, line11, line12,
        line13, line14, line15,
        line16, line17, line18,
        line19, line20, line21,
        line22,
        };

    return createGeometryFromPrimitiveArray (curves);
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
    double const outerFilletRadius = filletRadius + wallThickness;
    double const girth = profile.GetGirth();

    DPoint3d const tl_outerRangeApex  = { -halfWidth, halfDepth, 0.0 };
    DPoint3d const tl_outerLeft       = { -halfWidth, halfDepth - outerFilletRadius, 0.0 };
    DPoint3d const bl_outerLeft       = { -halfWidth, -(halfDepth - outerFilletRadius), 0.0 };
    DPoint3d const bl_outerRangeApex  = { -halfWidth, -halfDepth, 0.0 };
    DPoint3d const bl_outerBottom     = { -(halfWidth - outerFilletRadius), -halfDepth, 0.0 };
    DPoint3d br_outerBottom     = { halfWidth - outerFilletRadius, -halfDepth, 0.0 };
    DPoint3d const br_outerRangeApex  = { halfWidth, -halfDepth, 0.0 };
    DPoint3d br_outerRight      = { halfWidth, -(halfDepth - outerFilletRadius), 0.0 };
    DPoint3d br_outerRightGirth = { halfWidth, -(halfDepth - girth), 0.0 };
    DPoint3d br_innerRightGirth = { halfWidth - wallThickness, -(halfDepth - girth), 0.0 };
    DPoint3d br_innerRight      = { halfWidth - wallThickness, -(halfDepth - wallThickness - filletRadius), 0.0 };
    DPoint3d br_innerRangeApex  = { halfWidth - wallThickness, -(halfDepth - wallThickness), 0.0 };
    DPoint3d br_innerBottom     = { (halfWidth - wallThickness - filletRadius), -(halfDepth - wallThickness), 0.0 };
    DPoint3d const bl_innerBottom     = { -(halfWidth - wallThickness - filletRadius), -(halfDepth - wallThickness), 0.0 };
    DPoint3d const bl_innerRangeApex  = { -(halfWidth - wallThickness), -(halfDepth - wallThickness), 0.0 };
    DPoint3d const bl_innerLeft       = { -(halfWidth - wallThickness), -(halfDepth - wallThickness - filletRadius), 0.0 };
    DPoint3d const tl_innerLeft       = { -(halfWidth - wallThickness), halfDepth - wallThickness - filletRadius, 0.0 };
    DPoint3d const tl_innerRangeApex  = { -(halfWidth - wallThickness), halfDepth - wallThickness, 0.0 };
    DPoint3d const tl_innerTop        = { -(halfWidth - wallThickness - filletRadius), (halfDepth - wallThickness), 0.0 };
    DPoint3d tr_innerTop        = { (halfWidth - wallThickness - filletRadius), (halfDepth - wallThickness), 0.0 };
    DPoint3d tr_innerRangeApex  = { halfWidth - wallThickness, (halfDepth - wallThickness), 0.0 };
    DPoint3d tr_innerRight      = { halfWidth - wallThickness, (halfDepth - wallThickness - filletRadius), 0.0 };
    DPoint3d tr_innerRightGirth = { halfWidth - wallThickness, (halfDepth - girth), 0.0 };
    DPoint3d tr_outerRightGirth = { halfWidth, (halfDepth - girth), 0.0 };
    DPoint3d tr_outerRight      = { halfWidth, (halfDepth - outerFilletRadius), 0.0 };
    DPoint3d const tr_outerRangeApex  = { halfWidth, halfDepth, 0.0 };
    DPoint3d tr_outerTop        = { (halfWidth - outerFilletRadius), halfDepth, 0.0 };
    DPoint3d const tl_outerTop        = { -(halfWidth - outerFilletRadius), halfDepth, 0.0 };

    if (BeNumerical::IsEqualToZero (girth))
        {
        br_outerRight = { halfWidth, -(halfDepth - wallThickness), 0.0 };
        br_innerRangeApex  = br_outerRight;
        br_innerRightGirth = br_innerRangeApex;
        br_innerRight = br_innerRangeApex;
        br_innerBottom = br_innerRangeApex;
        br_outerRightGirth = br_innerRangeApex;
        br_outerBottom = br_outerRangeApex;


        tr_outerRight = { halfWidth,  halfDepth - wallThickness, 0.0 };
        tr_innerRangeApex = tr_outerRight;
        tr_innerRightGirth = tr_innerRangeApex;
        tr_innerRight = tr_innerRangeApex;
        tr_innerTop = tr_innerRangeApex;

        tr_outerRightGirth = tr_innerRangeApex;
        tr_outerTop = tr_outerRangeApex;
        }

    ICurvePrimitivePtr line2 = ICurvePrimitive::CreateLine (tl_outerLeft, bl_outerLeft);
    ICurvePrimitivePtr line3 = ICurvePrimitive::CreateLine (bl_outerLeft, bl_outerRangeApex);
    ICurvePrimitivePtr line4 = ICurvePrimitive::CreateLine (bl_outerRangeApex, bl_outerBottom);

    if (BeNumerical::IsGreaterThanZero (outerFilletRadius))
        {
        line3 = createArcBetweenLines (line3, line4, outerFilletRadius);
        line4 = nullptr;
        }

    ICurvePrimitivePtr  line5 = ICurvePrimitive::CreateLine (bl_outerBottom, br_outerBottom);
    ICurvePrimitivePtr  line6 = ICurvePrimitive::CreateLine (br_outerBottom, br_outerRangeApex);
    ICurvePrimitivePtr  line7 = ICurvePrimitive::CreateLine (br_outerRangeApex, br_outerRight);

    if (BeNumerical::IsGreaterThanZero (outerFilletRadius) && BeNumerical::IsGreaterThanZero (girth))
        {
        line6 = createArcBetweenLines (line6, line7, outerFilletRadius);
        line7 = nullptr;
        }
   
    ICurvePrimitivePtr  line8 = ICurvePrimitive::CreateLine (br_outerRight, br_outerRightGirth);
    ICurvePrimitivePtr  line9 = ICurvePrimitive::CreateLine (br_outerRightGirth, br_innerRightGirth);
    ICurvePrimitivePtr line10 = ICurvePrimitive::CreateLine (br_innerRightGirth, br_innerRight);
    ICurvePrimitivePtr line11 = ICurvePrimitive::CreateLine (br_innerRight, br_innerRangeApex);
    ICurvePrimitivePtr line12 = ICurvePrimitive::CreateLine (br_innerRangeApex, br_innerBottom);

    if (BeNumerical::IsGreaterThanZero (filletRadius))
        {
        line11 = createArcBetweenLines (line11, line12, filletRadius);
        line12 = nullptr;
        }

    ICurvePrimitivePtr line13 = ICurvePrimitive::CreateLine (br_innerBottom, bl_innerBottom);
    ICurvePrimitivePtr line14 = ICurvePrimitive::CreateLine (bl_innerBottom, bl_innerRangeApex);
    ICurvePrimitivePtr line15 = ICurvePrimitive::CreateLine (bl_innerRangeApex, bl_innerLeft);

    if (BeNumerical::IsGreaterThanZero (filletRadius))
        {
        line14 = createArcBetweenLines(line14, line15, filletRadius);
        line15 = nullptr;
        }

    ICurvePrimitivePtr line16 = ICurvePrimitive::CreateLine (bl_innerLeft, tl_innerLeft);
    ICurvePrimitivePtr line17 = ICurvePrimitive::CreateLine (tl_innerLeft, tl_innerRangeApex);
    ICurvePrimitivePtr line18 = ICurvePrimitive::CreateLine (tl_innerRangeApex, tl_innerTop);

    if (BeNumerical::IsGreaterThanZero (filletRadius))
        {
        line17 = createArcBetweenLines (line17, line18, filletRadius);
        line18 = nullptr;
        }

    ICurvePrimitivePtr line19 = ICurvePrimitive::CreateLine (tl_innerTop, tr_innerTop);
    ICurvePrimitivePtr line20 = ICurvePrimitive::CreateLine (tr_innerTop, tr_innerRangeApex);
    ICurvePrimitivePtr line21 = ICurvePrimitive::CreateLine (tr_innerRangeApex, tr_innerRight);

    if (BeNumerical::IsGreaterThanZero (filletRadius) && BeNumerical::IsGreaterThanZero (girth))
        {
        line20 = createArcBetweenLines (line20, line21, filletRadius);
        line21 = nullptr;
        }

    ICurvePrimitivePtr line22 = ICurvePrimitive::CreateLine (tr_innerRight, tr_innerRightGirth);
    ICurvePrimitivePtr line23 = ICurvePrimitive::CreateLine (tr_innerRightGirth, tr_outerRightGirth);
    ICurvePrimitivePtr line24 = ICurvePrimitive::CreateLine (tr_outerRightGirth, tr_outerRight);

    ICurvePrimitivePtr line25 = ICurvePrimitive::CreateLine (tr_outerRight, tr_outerRangeApex);
    ICurvePrimitivePtr line26 = ICurvePrimitive::CreateLine (tr_outerRangeApex, tr_outerTop);

    if (BeNumerical::IsGreaterThanZero (outerFilletRadius) && BeNumerical::IsGreaterThanZero (girth))
        {
        line25 = createArcBetweenLines (line25, line26, outerFilletRadius);
        line26 = nullptr;
        }

    ICurvePrimitivePtr line27 = ICurvePrimitive::CreateLine (tr_outerTop, tl_outerTop);
    ICurvePrimitivePtr line28 = ICurvePrimitive::CreateLine (tl_outerTop, tl_outerRangeApex);
    ICurvePrimitivePtr  line1 = ICurvePrimitive::CreateLine (tl_outerRangeApex, tl_outerLeft);

    if (BeNumerical::IsGreaterThanZero (outerFilletRadius))
        {
        line28 = createArcBetweenLines (line28, line1, outerFilletRadius);
        line1 = nullptr;
        }

    bvector<ICurvePrimitivePtr> curves =
        {
        line1,  line2, line3,
        line4,  line5, line6,
        line7,  line8, line9,
        line10, line11, line12,
        line13, line14, line15,
        line16, line17, line18, 
        line19, line20, line21, 
        line22, line23, line24,
        line25, line26, line27, 
        line28,
        };

    return createGeometryFromPrimitiveArray (curves);
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
    BeAssert (curveVector->size() == 0);

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

    ICurvePrimitivePtr topRightArc, bottomRightArc, bottomLeftArc, topLeftArc;
    if (roundingRadius > DBL_EPSILON)
        {
        topRightArc = createArcBetweenLines (topLine, rightLine, roundingRadius);
        bottomRightArc = createArcBetweenLines (rightLine, bottomLine, roundingRadius);
        bottomLeftArc = createArcBetweenLines (bottomLine, leftLine, roundingRadius);
        topLeftArc = createArcBetweenLines (leftLine, topLine, roundingRadius);

        if (topLine->GetLineCP()->Length() <= DBL_EPSILON)
            topLine = nullptr;
        if (rightLine->GetLineCP()->Length() <= DBL_EPSILON)
            rightLine = nullptr;
        if (bottomLine->GetLineCP()->Length() <= DBL_EPSILON)
            bottomLine = nullptr;
        if (leftLine->GetLineCP()->Length() <= DBL_EPSILON)
            leftLine = nullptr;
        }

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
* profiles was updated (from _UpdateGeometry()). 'updatedProfilePtr' might be null if
* the geometry is being created for the first time (from _CreateGeometry()).
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
