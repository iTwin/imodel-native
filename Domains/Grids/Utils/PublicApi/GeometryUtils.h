/*--------------------------------------------------------------------------------------+
|
|     $Source: Utils/PublicApi/GeometryUtils.h $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

#include <BuildingMacros.h>
#include <DgnPlatform/ElementGeometry.h>
#include <DgnPlatform/SolidKernel.h>

BEGIN_BUILDING_NAMESPACE

//GeometryUtils is a static class for doing various operations on geometry (this is also a wrapper for all parasolid related activity)
class GeometryUtils
    {
public:
    BUILDINGUTILS_EXPORT static BentleyStatus    CreateBodyFromGeometricPrimitive (Dgn::IBRepEntityPtr& out, Dgn::GeometricPrimitiveCPtr primitive, bool assignIds = false);
    BUILDINGUTILS_EXPORT static BentleyStatus    SliceBodyByPlanes (bvector<bpair<Dgn::IBRepEntityPtr, CurveVectorPtr>>& slicedGeometry, Dgn::IBRepEntityCR geometryToSlice,
                                                                    CurveVectorCR cuttingPlaneProfile, double sliceHeight);
    BUILDINGUTILS_EXPORT static CurveVectorPtr   ExtractXYProfileFromSolid (Dgn::IBRepEntityCR solid);
    BUILDINGUTILS_EXPORT static BentleyStatus    GetGeometricPrimitivesFromGeometricElement(bvector<Dgn::GeometricPrimitivePtr>& geometricPrimitivesOut, Dgn::GeometricElementCPtr geoElement);
    BUILDINGUTILS_EXPORT static BentleyStatus    GetIBRepEntitiesFromGeometricElement (bvector<Dgn::IBRepEntityPtr>& brepsOut, Dgn::GeometricElementCPtr geoElement);
    
    BUILDINGUTILS_EXPORT static bvector<DPoint3d>   ExtractSingleCurvePoints (CurveVectorPtr curve);
    
    //! Forms points into a rectangle by a bounding point
    //! @params[out] point1, point2, point3, point4 points forming a rectangle
    //! @param[in] boundingPoint point in rectangle width line center that rectangle should adjust to
    //! @param[in] lengthVector weighted direction of rectangle length
    //! @param[in] widthVector weighted direction of rectangle width
    //! @param[in] ccw bool value showing if rectangle should be formed clockwise or counterclockwise from the bounding point
    BUILDINGUTILS_EXPORT static void FormRectangle(DPoint3dR point1, DPoint3dR point2, DPoint3dR point3, DPoint3dR point4, DPoint3d boundingPoint, DVec3d lengthVector, DVec3d widthVector, bool ccw);
    
    //! Forms points into a rectangle by a center
    //! @params[out] point1, point2, point3, point4 points forming a rectangle
    //! @param[in] center rectangle center
    //! @param[in] lengthVecter weighted direction of rectangle length
    //! @param[in] widthVector weighted direction of rectangle width
    BUILDINGUTILS_EXPORT static void FormRectangle(DPoint3dR point1, DPoint3dR point2, DPoint3dR point3, DPoint3dR point4, DPoint3d center, DVec3d lengthVector, DVec3d widthVector);
    
    //! Forms points into a rectangle by 2 bounding points
    //! @params[out] point1, point2, point3, point4 points forming a rectangle
    //! @param[in] boundingPoint1 point in rectangle width line center that rectangle should adjust to
    //! @param[in] boundingPoint2 point in rectangle width line center that rectangle should adjust to
    //! @param[in] widthVector weighter direction of rectangle width
    BUILDINGUTILS_EXPORT static void FormRectangle(DPoint3dR point1, DPoint3dR point2, DPoint3dR point3, DPoint3dR point4, DPoint3d boundingPoint1, DPoint3d boundingPoint2, DVec3d widthVector);
    
    //! Forms rectangle points for rectangle to be on line. Formed rectangle does not go out of line bounds and is centered to it by width
    //! @param[out] pointsOut vector containing formed points
    //! @param[in] line line that rectangle should adjust to
    //! @param[in] rectangleCenter point where rectangle center should be. If rectangle goes out of line bounds with given center, the center is translated
    //! @param[in] rectangleLength rectangle length. If rectangleLength is higher than line that rectangle should adjust to, rectangleLength lowers to adjust
    //! @param[in] rectangleWidth rectangle width.
    BUILDINGUTILS_EXPORT static void SetUpRectangleToAdjustToLine(bvector<DPoint3d>& pointsOut, DSegment3d line, DPoint3d rectangleCenter, double rectangleLength, double rectangleWidth);

    //! Checks if point is on given line
    //! @param[in] point point to check
    //! @param[in] line given line
    //! @return true if point is on given line
    BUILDINGUTILS_EXPORT static bool IsPointOnLine(DPoint3d point, DSegment3d line);
    
    //! Calculates minimal translation that point needs to be multiplied of to be on given line
    //! @param[in] pointToFit point to translate on line
    //! @param[in] line given line
    //! @return translation to fit point to a line end point
    BUILDINGUTILS_EXPORT static DVec3d FindTranslationToNearestEndPoint(DPoint3d pointToFit, DSegment3d line);

    //! Calculates transform (rotation) and translation for element to adjust to given line by given target point
    //! @param[out] translationOut calculated translation
    //! @param[in] toGlobal transform to global coordinates. Pass Identity transformation if given element direction is already in global coordinates
    //! @param[in] elementRay origin (center) point and direction of element
    //! @param[in] line line that element should be moved to
    //! @param[in] targetPoint point where new element origin should be
    //! @return new transform in global coordinates for element to be moved and rotated for direction to be parallel to line
    BUILDINGUTILS_EXPORT static Transform GetTransformForMoveOnLine(DVec3dR translationOut, Transform toGlobal, DRay3d elementRay, DSegment3d line, DPoint3d targetPoint);
    
    //! Resizes given line to not exceed the bounding line points
    //! @param[in/out] lineOut resized line
    //! @param[in] boundingLine given bounding line
    //! @param[in] atEnd line point to change. false for start point, true for end point
    //! @param[in] target point where new resized line point should be. If bounding line point is closer to a line than the target point, resize is done only to the boundingLine end point.
    //! @return status if resize succeeded
    BUILDINGUTILS_EXPORT static BentleyStatus ResizeLineToClosestPoint(DSegment3dR lineToResize, DSegment3d boundingLine, bool atEnd, DPoint3d target);

    //! Checks if two numbers are almost equal
    //! @param[in] a first operand
    //! @param[in] b second operand
    //! @return true if doubles are almost equal
    BUILDINGUTILS_EXPORT static bool AlmostEqual(double a, double b);

    //! Finds the furthestPoint in a CurveVector. Supports curve vector composed of line strings and arcs
    //! @param[in] curveVector a curve vector to look for the furthest point in
    //! @param[in] point a point in/on curve vector
    //! @return a furthest point from given point that is on curveVector
    BUILDINGUTILS_EXPORT static DPoint3d FindFurthestPoint(CurveVectorPtr curveVector, DPoint3d point);

    //! Finds ellipse's tangent at given point
    //! @param[in] point a point on arc
    //! @param[in] ellipse
    //! @returns tangent of the ellipse
    BUILDINGUTILS_EXPORT static DSegment3d FindEllipseTangent(DPoint3d point, DEllipse3d ellipse);

    //! Finds index of the closest point in vector
    //! @param[in] keyPoints    a vector of points
    //! @param[in] point        a point to find
    //! @return                 index of the closest point in given vector
    BUILDINGUTILS_EXPORT static int FindClosestPointIndex(bvector<DPoint3d>& keyPoints, DPoint3d point);
    
    //! Rotates placement by given angle in radians in XY plane
    //! @param[in/out] placement    placement to rotate
    //! @param[in] theta            angle in radians
    BUILDINGUTILS_EXPORT static void RotatePlacementXY(Dgn::Placement3dR placement, double theta);

    //! Translates placement by given vector in XY plane
    //! @param[in/out] placement    placement to translate
    //! @param[in] translation      vector to translate by
    BUILDINGUTILS_EXPORT static void TranslatePlacementXY(Dgn::Placement3dR placement, DVec3d translation);

    //! Translates placement by given vector
    //! @param[in/out] placement    placement to translate
    //! @param[in] translation      vector to translate by
    BUILDINGUTILS_EXPORT static void TranslatePlacement(Dgn::Placement3dR placement, DVec3d translation);

    //! Translates point by rotated vector
    //! @param[in/out]  point   point to translate
    //! @param[in]      axis    vector to rotate and add
    //! @param[in]      theta   angle to rotate vector by
    BUILDINGUTILS_EXPORT static void AddRotatedVectorToPoint(DPoint3dR point, DVec3d axis, double theta);

    //! Creates an arc by center, start point, end point and rotation direction
    //! @param[in]  center  center point of arc
    //! @param[in]  start   start point of arc
    //! @param[in]  end     end point of arc
    //! @param[in]  ccw     direction of arc sweep. true for counter clock wise, false for clockwise
    BUILDINGUTILS_EXPORT static ICurvePrimitivePtr CreateArc(DPoint3d center, DPoint3d start, DPoint3d end, bool ccw);
    };

END_BUILDING_NAMESPACE