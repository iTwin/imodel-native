/*--------------------------------------------------------------------------------------+
|
|     $Source: Profiles/Implementation/ProfilesGeometry.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ProfilesInternal.h"
#include <Profiles\ProfilesGeometry.h>

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
IGeometryPtr ProfilesGeomApi::CreateCShape (CShapeProfileCPtr profile)
    {
    double const flangeThickness = profile->GetFlangeThickness();
    double const webThickness = profile->GetWebThickness();
    double const filletRadius = profile->GetFilletRadius();
    double const flangeEdgeRadius = profile->GetFlangeEdgeRadius();
    double const halfWidth = profile->GetFlangeWidth() / 2.0;
    double const halfDepth = profile->GetDepth() / 2.0;
    double const slopeHeight = profile->GetFlangeSlopeHeight();

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
IGeometryPtr ProfilesGeomApi::CreateIShape (IShapeProfileCPtr profile)
    {
    double const flangeThickness = profile->GetFlangeThickness();
    double const halfWebThickness = profile->GetWebThickness() / 2.0;
    double const filletRadius = profile->GetFilletRadius();
    double const flangeEdgeRadius = profile->GetFlangeEdgeRadius();
    double const halfWidth = profile->GetFlangeWidth() / 2.0;
    double const halfDepth = profile->GetDepth() / 2.0;
    double const slopeHeight = profile->GetFlangeSlopeHeight();

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

    if (profile->GetFlangeEdgeRadius() > 0.0)
        {
        topRightFlangeEdgeArc = createArcBetweenLines (topRightFlangeEdgeLine, topRightFlangeSlopeLine, flangeEdgeRadius);
        bottomRightFlangeEdgeArc = createArcBetweenLines (bottomRightFlangeSlopeLine, bottomRightFlangeEdgeLine, flangeEdgeRadius);
        bottomLeftFlangeEdgeArc = createArcBetweenLines (bottomLeftFlangeEdgeLine, bottomLeftFlangeSlopeLine, flangeEdgeRadius);
        topLeftFlangeEdgeArc = createArcBetweenLines (topLeftFlangeSlopeLine, topLeftFlangeEdgeLine, flangeEdgeRadius);
        }

    if (profile->GetFilletRadius() > 0.0)
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
IGeometryPtr ProfilesGeomApi::CreateAsymmetricIShape (AsymmetricIShapeProfileCPtr profile)
    {
    double const topFlangeThickness = profile->GetTopFlangeThickness();
    double const bottomFlangeThickness = profile->GetBottomFlangeThickness();
    double const topFlangeFilletRadius = profile->GetTopFlangeFilletRadius();
    double const bottomFlangeFilletRadius = profile->GetBottomFlangeFilletRadius();
    double const topFlangeEdgeRadius = profile->GetTopFlangeEdgeRadius();
    double const bottomFlangeEdgeRadius = profile->GetBottomFlangeEdgeRadius();
    double const halfWebThickness = profile->GetWebThickness() / 2.0;
    double const halfTopWidth = profile->GetTopFlangeWidth() / 2.0;
    double const halfBottomWidth = profile->GetBottomFlangeWidth() / 2.0;
    double const halfDepth = profile->GetDepth() / 2.0;
    double const topFlangeSlopeHeight = profile->GetTopFlangeSlopeHeight();
    double const bottomFlangeSlopeHeight = profile->GetTopFlangeSlopeHeight();

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
IGeometryPtr ProfilesGeomApi::CreateLShape (LShapeProfileCPtr profile)
    {
    double const halfWidth = profile->GetWidth() / 2.0;
    double const halfDepth = profile->GetDepth() / 2.0;
    double const thickness = profile->GetThickness();
    double const filletRadius = profile->GetFilletRadius();
    double const edgeRadius = profile->GetEdgeRadius();
    double const flangeSlopeHeight = profile->GetHorizontalLegSlopeHeight();
    double const webSlopeHeight = profile->GetVerticalLegSlopeHeight();

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
IGeometryPtr ProfilesGeomApi::CreateTShape (TShapeProfileCPtr profile)
    {
    double const flangeThickness = profile->GetFlangeThickness();
    double const halfWebThickness = profile->GetWebThickness() / 2.0;
    double const filletRadius = profile->GetFilletRadius();
    double const flangeEdgeRadius = profile->GetFlangeEdgeRadius();
    double const webEdgeRadius = profile->GetWebEdgeRadius();
    double const halfWidth = profile->GetFlangeWidth() / 2.0;
    double const halfDepth = profile->GetDepth() / 2.0;
    double const flangeSlopeHeight = profile->GetFlangeSlopeHeight();
    double const webSlopeHeight = profile->GetWebSlopeHeight();

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

        DPoint3d bottomLineStartPoint, bottomLineEndPoint;
        BeAssert (bottomLine->GetStartEnd (bottomLineStartPoint, bottomLineEndPoint));
        if (bottomLineStartPoint.AlmostEqual (bottomLineEndPoint))
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
IGeometryPtr ProfilesGeomApi::CreateZShape (ZShapeProfileCPtr profile)
    {
    double const flangeWidth = profile->GetFlangeWidth();
    double const flangeThickness = profile->GetFlangeThickness();
    double const filletRadius = profile->GetFilletRadius();
    double const flangeEdgeRadius = profile->GetFlangeEdgeRadius();
    double const halfDepth = profile->GetDepth() / 2.0;
    double const webThickness = profile->GetWebThickness();
    double const halfWebThickness = webThickness / 2.0;
    double const slopeHeight = profile->GetFlangeSlopeHeight();

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

    if (profile->GetFlangeEdgeRadius() > 0.0)
        {
        bottomFlangeEdgeArc = createArcBetweenLines (bottomFlangeSlopeLine, bottomFlangeEdgeLine, flangeEdgeRadius);
        topFlangeEdgeArc = createArcBetweenLines (topFlangeSlopeLine, topLeftFlangeEdgeLine, flangeEdgeRadius);
        }

    if (profile->GetFilletRadius() > 0.0)
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
IGeometryPtr ProfilesGeomApi::CreateCircle (CircleProfileCPtr profile)
    {
    DEllipse3d ellipse = DEllipse3d::FromCenterRadiusXY (DPoint3d::From (0.0, 0.0), profile->GetRadius());
    ICurvePrimitivePtr circleCurve = ICurvePrimitive::CreateArc (ellipse);

    CurveVectorPtr curveVector = CurveVector::Create (CurveVector::BOUNDARY_TYPE_Inner);
    curveVector->Add (circleCurve);

    return IGeometry::Create (curveVector);
    }

END_BENTLEY_PROFILES_NAMESPACE
