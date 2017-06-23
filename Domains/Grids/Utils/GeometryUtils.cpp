/*--------------------------------------------------------------------------------------+
|
|     $Source: Utils/GeometryUtils.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#include "PublicApi/GeometryUtils.h"
#include <dgnPlatform/ViewController.h>

USING_NAMESPACE_BENTLEY_DGN

BEGIN_BUILDING_NAMESPACE
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Jonas.Valiunas                  02/2017
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   GeometryUtils::CreateBodyFromGeometricPrimitive
(
IBRepEntityPtr& out, 
GeometricPrimitiveCPtr primitive, 
bool assignIds
)
    {
    BentleyStatus status = SUCCESS;
    uint32_t id = assignIds ? 1 : 0;
    switch (primitive->GetGeometryType ())
        {
    case GeometricPrimitive::GeometryType::CurvePrimitive:
        status = BRepUtil::Create::BodyFromCurveVector (out, *CurveVector::Create (CurveVector::BoundaryType::BOUNDARY_TYPE_None, primitive->GetAsICurvePrimitive ()), id);
        break;
    case GeometricPrimitive::GeometryType::CurveVector:
        status = BRepUtil::Create::BodyFromCurveVector (out, *primitive->GetAsCurveVector (), id);
        break;
    case GeometricPrimitive::GeometryType::SolidPrimitive:
        status = BRepUtil::Create::BodyFromSolidPrimitive (out, *primitive->GetAsISolidPrimitive (), id);
        break;
    case GeometricPrimitive::GeometryType::BsplineSurface:
        status = BRepUtil::Create::BodyFromBSurface (out, *primitive->GetAsMSBsplineSurface (), id);
        break;
    case GeometricPrimitive::GeometryType::Polyface:
        status = BRepUtil::Create::BodyFromPolyface (out, *primitive->GetAsPolyfaceHeader (), id);
        break;
    case GeometricPrimitive::GeometryType::BRepEntity:
        out = primitive->GetAsIBRepEntity ();
        status = SUCCESS;
        break;
    case GeometricPrimitive::GeometryType::TextString:
    default:
        status = ERROR; //we're not converting textstrings into breps.
        break;
        }
    return status;
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Nerijus.Jakeliunas              06/2017
//---------------+---------------+---------------+---------------+---------------+------
CurveVectorPtr GeometryUtils::ExtractXYProfileFromSolid (IBRepEntityCR solid)
    {
    bvector<ISubEntityPtr> subEntities;
    BRepUtil::GetBodyFaces (&subEntities, solid);
    for (ISubEntityPtr subEntityPtr : subEntities)
        {
        if (!subEntityPtr.IsValid () || !BRepUtil::IsPlanarFace (*subEntityPtr))
            {
            continue;
            }

        DPoint3d point;
        DVec3d normal, uDir, vDir;
        DPoint2d uvParam{ 0.0, 0.0 };
        BRepUtil::EvaluateFace (*subEntityPtr, point, normal, uDir, vDir, uvParam);
        if (!normal.IsPositiveParallelTo (DVec3d::From (0.0, 0.0, -1.0)))
            {
            continue;
            }

        GeometricPrimitiveCPtr geometry = subEntityPtr->GetGeometry ();
        IBRepEntityPtr sheet = geometry->GetAsIBRepEntity ();
        if (!sheet.IsValid () || sheet->GetEntityType () != IBRepEntity::EntityType::Sheet)
            {
            continue;
            }

        CurveVectorPtr curveVectorPtr = BRepUtil::Create::BodyToCurveVector (*sheet);
        if (curveVectorPtr.IsValid ())
            {
            curveVectorPtr->ConsolidateAdjacentPrimitives ();
            curveVectorPtr->ReverseCurvesInPlace (); // need to switch to CCW direction
            }

        return curveVectorPtr;
        }

    return nullptr;
    }


//---------------------------------------------------------------------------------------
// @param         slicedGeometry      OUT solid slices paired with corresponding bottom planes of the cuts
// @param         geometryToSlice     IN
// @param         cuttingPlaneProfile IN
// @param         sliceHeight         IN
// @bsimethod                                    Nerijus.Jakeliunas              06/2017
//---------------+---------------+---------------+---------------+---------------+-------
BentleyStatus   GeometryUtils::SliceBodyByPlanes 
(
bvector<bpair<Dgn::IBRepEntityPtr, CurveVectorPtr>>& slicedGeometry,
IBRepEntityCR geometryToSlice,
CurveVectorCR cuttingPlaneProfile,
double sliceHeight
)
    {
    if (sliceHeight < 0.0001 )
        {
        return BSIERROR;
        }

    DRange3d range = geometryToSlice.GetEntityRange ();
    DPoint3d profilePoint;
    cuttingPlaneProfile.GetStartPoint (profilePoint);
    DPoint3d offset = DPoint3d::From (0.0, 0.0, range.low.z - profilePoint.z);
    Transform tranform = Transform::From (offset);
    CurveVectorPtr bottomPlanePtr = cuttingPlaneProfile.Clone ();
    bottomPlanePtr->TransformInPlace (tranform);

    double remainingHeight = range.ZLength ();
    while (remainingHeight > 0.0001)
        {
        IBRepEntityPtr bottomSheetBody = nullptr;
        BRepUtil::Create::BodyFromCurveVector (bottomSheetBody, *bottomPlanePtr);
        if (!bottomSheetBody.IsValid ())
            {
            slicedGeometry = bvector<bpair<Dgn::IBRepEntityPtr, CurveVectorPtr>> ();
            return BSIERROR;
            }

        // cut out material below bottom plane
        IBRepEntityPtr slice = geometryToSlice.Clone ();
        BRepUtil::Modify::BooleanCut (slice, *bottomSheetBody, BRepUtil::Modify::CutDirectionMode::Backward, BRepUtil::Modify::CutDepthMode::All, sliceHeight, true);
        if (!slice.IsValid ())
            {
            slicedGeometry = bvector<bpair<Dgn::IBRepEntityPtr, CurveVectorPtr>> ();
            return BSIERROR;
            }

        offset = DPoint3d::From (0.0, 0.0, sliceHeight);
        tranform = Transform::From (offset);
        CurveVectorPtr topPlanePtr = bottomPlanePtr->Clone ();
        topPlanePtr->TransformInPlace (tranform);

        IBRepEntityPtr topSheetBody = nullptr;
        BRepUtil::Create::BodyFromCurveVector (topSheetBody, *topPlanePtr);
        if (!topSheetBody.IsValid ())
            {
            slicedGeometry = bvector<bpair<Dgn::IBRepEntityPtr, CurveVectorPtr>> ();
            return BSIERROR;
            }

        // cut out material above top plane
        BRepUtil::Modify::BooleanCut (slice, *topSheetBody, BRepUtil::Modify::CutDirectionMode::Forward, BRepUtil::Modify::CutDepthMode::All, sliceHeight, true);
        if (!slice.IsValid ())
            {
            slicedGeometry = bvector<bpair<Dgn::IBRepEntityPtr, CurveVectorPtr>> ();
            return BSIERROR;
            }

        slicedGeometry.push_back ({ slice, bottomPlanePtr });

        bottomPlanePtr = topPlanePtr;
        remainingHeight -= sliceHeight;
        }

    return BSISUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Haroldas.Vitunskas             03/2017
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus    GeometryUtils::GetGeometricPrimitivesFromGeometricElement
(
bvector<Dgn::GeometricPrimitivePtr>& geometricPrimitives, 
Dgn::GeometricElementCPtr geoElement
)
    {
    auto pGeometrySource = geoElement->ToGeometrySource();
    if (nullptr == pGeometrySource)
        return BentleyStatus::ERROR;

    GeometryCollection geomData(*pGeometrySource);
    Transform elemToWorld = (*geomData.begin ()).GetGeometryToWorld ();
    if (geomData.begin() == geomData.end())
        return BentleyStatus::ERROR;

    for (GeometryCollection::Iterator it = geomData.begin (); it != geomData.end (); ++it)
        {
        GeometricPrimitivePtr clone = (*it).GetGeometryPtr ()->Clone ();
        Transform elemToWorld = (*it).GetGeometryToWorld ();
        clone->TransformInPlace (elemToWorld);
        geometricPrimitives.push_back (clone);
        }
    return BentleyStatus::SUCCESS;
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Jonas.Valiunas                  05/2017
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus    GeometryUtils::GetIBRepEntitiesFromGeometricElement
(
bvector<Dgn::IBRepEntityPtr>& brepsOut,
Dgn::GeometricElementCPtr geoElement
)
    {
    BentleyStatus status = BentleyStatus::ERROR;
    bvector<Dgn::GeometricPrimitivePtr> primitives;
    GetGeometricPrimitivesFromGeometricElement (primitives, geoElement);
    for (bvector<Dgn::GeometricPrimitivePtr>::iterator iter = primitives.begin (); iter != primitives.end (); ++iter)
        {
        Dgn::IBRepEntityPtr brep;
        if (SUCCESS == (status = CreateBodyFromGeometricPrimitive (brep, *iter)))
            {
            brepsOut.push_back (brep);
            }
        }

    return status;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Haroldas.Vitunskas              04/2017
//---------------------------------------------------------------------------------------
void GeometryUtils::FormRectangle
(
    DPoint3dR point1,
    DPoint3dR point2,
    DPoint3dR point3,
    DPoint3dR point4,
    DPoint3d center,
    DVec3d lengthVector,
    DVec3d widthVector
)
    {
    DPoint3d bound1 = center;
    DPoint3d bound2 = center;
    lengthVector.Scale(0.5);

    bound1.Subtract(lengthVector);
    bound2.Add(lengthVector);

    FormRectangle(point1, point2, point3, point4, bound1, bound2, widthVector);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Haroldas.Vitunskas              04/2017
//---------------------------------------------------------------------------------------
void GeometryUtils::FormRectangle
(
    DPoint3dR point1,
    DPoint3dR point2,
    DPoint3dR point3,
    DPoint3dR point4,
    DPoint3d boundingPoint,
    DVec3d lengthVector,
    DVec3d widthVector,
    bool ccw
)
    {
    DPoint3d boundingPoint2 = boundingPoint;

    if (ccw)
        {
        boundingPoint2.Add(lengthVector);
        FormRectangle(point1, point2, point3, point4, boundingPoint, boundingPoint2, widthVector);
        }
    else
        {
        boundingPoint2.Subtract(lengthVector);
        FormRectangle(point1, point2, point3, point4, boundingPoint2, boundingPoint, widthVector);
        }

    }
//---------------------------------------------------------------------------------------
// @bsimethod                                    Haroldas.Vitunskas              04/2017
//---------------------------------------------------------------------------------------
void GeometryUtils::FormRectangle
(
    DPoint3dR point1,
    DPoint3dR point2,
    DPoint3dR point3,
    DPoint3dR point4,
    DPoint3d boundingPoint1,
    DPoint3d boundingPoint2,
    DVec3d widthVector
)
    {
    widthVector.Scale(0.5);

    point1 = boundingPoint1;
    point1.Add(widthVector);

    point2 = boundingPoint2;
    point2.Add(widthVector);

    point3 = boundingPoint2;
    point3.Subtract(widthVector);

    point4 = boundingPoint1;
    point4.Subtract(widthVector);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Haroldas.Vitunskas              04/2017
//---------------------------------------------------------------------------------------
bool GeometryUtils::AlmostEqual(double a, double b)
    {
    return fabs(a - b) <= BUILDING_TOLERANCE;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Haroldas.Vitunskas              05/2017
//---------------------------------------------------------------------------------------
bool GeometryUtils::IsPointOnLine(DPoint3d pointToCheck, DSegment3d line)
    {
    DPoint3d point1, point2;
    line.GetStartPoint(point1);
    line.GetEndPoint(point2);

    return AlmostEqual(pointToCheck.Distance(point1) + pointToCheck.Distance(point2), line.Length());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Haroldas.Vitunskas              05/2017
//---------------------------------------------------------------------------------------
DVec3d GeometryUtils::FindTranslationToNearestEndPoint(DPoint3d pointToFit, DSegment3d line)
    {
    DPoint3d bound1, bound2;
    line.GetStartPoint(bound1);
    line.GetEndPoint(bound2);

    if (pointToFit.Distance(bound1) < pointToFit.Distance(bound2))
        return DVec3d::FromStartEnd(pointToFit, bound1);
    else
        return DVec3d::FromStartEnd(pointToFit, bound2);
    }

/*---------------------------------------------------------------------------------**//**
*@bsimethod                                     Mindaugas.Butkus                 04/2017
+---------------+---------------+---------------+---------------+---------------+------*/
bvector<DPoint3d>               GeometryUtils::ExtractSingleCurvePoints
(
CurveVectorPtr curve
)
    {
    bvector<DPoint3d> points;

    curve->ConsolidateAdjacentPrimitives (true);
    if (!curve.IsValid ())
        return points;

    if (curve->size () == 0)
        return points;

    ICurvePrimitivePtr boundaryCurve = curve->at (0);
    if (!boundaryCurve.IsValid ())
        return points;

    if (boundaryCurve->GetCurvePrimitiveType () == ICurvePrimitive::CURVE_PRIMITIVE_TYPE_CurveVector)
        boundaryCurve = boundaryCurve->GetChildCurveVectorP ()->at (0);
    if (!boundaryCurve.IsValid ())
        return points;


    if (boundaryCurve->GetCurvePrimitiveType () == ICurvePrimitive::CURVE_PRIMITIVE_TYPE_LineString)
        points = *boundaryCurve->GetLineStringP ();

    DPoint3d start, end;
    boundaryCurve->GetStartEnd (start, end);
    points.insert (points.begin (), start);
    points.push_back (end);
    auto pointCompare = [](DPoint3d const& p1, DPoint3d const& p2) { return p1.AlmostEqual (p2); };
    points.erase (std::unique (points.begin (), points.end (), pointCompare), points.end ());

    return points;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Haroldas.Vitunskas              05/2017
//---------------------------------------------------------------------------------------
void GeometryUtils::SetUpRectangleToAdjustToLine(bvector<DPoint3d>& points, DSegment3d spaceLine, DPoint3d rectangleCenter, double rectangleLength, double rectangleWidth)
    {
    DPoint3d boundingPoint1, boundingPoint2;
    spaceLine.GetStartPoint(boundingPoint1);
    spaceLine.GetEndPoint(boundingPoint2);

    DVec3d lengthVector = DVec3d::FromStartEnd(boundingPoint1, boundingPoint2);
    lengthVector.ScaleToLength(rectangleLength);

    DVec3d widthVector = lengthVector;
    widthVector.RotateXY(msGeomConst_pi / 2);
    widthVector.ScaleToLength(rectangleWidth);

    DPoint3d point1, point2, point3, point4;

    if (boundingPoint1.Distance(boundingPoint2) < rectangleLength)
        GeometryUtils::FormRectangle(point1, point2, point3, point4, boundingPoint1, boundingPoint2, widthVector);
    else
        {
        if (boundingPoint1.Distance(rectangleCenter) < rectangleLength / 2)
            GeometryUtils::FormRectangle(point1, point2, point3, point4, boundingPoint1, lengthVector, widthVector, true);
        else if (boundingPoint2.Distance(rectangleCenter) < rectangleLength / 2)
            GeometryUtils::FormRectangle(point1, point2, point3, point4, boundingPoint2, lengthVector, widthVector, false);
        else
            GeometryUtils::FormRectangle(point1, point2, point3, point4, rectangleCenter, lengthVector, widthVector);
        }

    points = {
        point1,
        point2,
        point3,
        point4
        };
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Haroldas.Vitunskas              05/2017
//---------------------------------------------------------------------------------------
Transform GeometryUtils::GetTransformForMoveOnLine(DVec3dR translation, Transform toGlobal, DRay3d elementRay, DSegment3d line, DPoint3d targetPoint)
    {
    DPoint3d bound1, bound2;
    line.GetStartPoint(bound1);
    line.GetEndPoint(bound2);

    translation = DVec3d::From(0, 0);

    DPoint3d newPosition = targetPoint;
    double closestParam;
    line.ProjectPoint(newPosition, closestParam, targetPoint); //make sure new element center is on the line
    
    DPoint3d originPoint = elementRay.origin;

    translation = DVec3d::FromStartEnd(originPoint, newPosition);

    DVec3d elementDirection = elementRay.direction;
    elementDirection.Multiply(toGlobal.Matrix(), elementDirection); //transfer element to global coordinates

    DVec3d boundingLineVector = DVec3d::FromStartEnd(bound1, bound2);

    double rotationAngle = elementDirection.AngleToXY(boundingLineVector);
    RotMatrix rotationMatrix = RotMatrix::FromAxisAndRotationAngle(2, rotationAngle); //rotation around Z axis

    Transform localTransformation = Transform::FromIdentity();
    localTransformation.SetMatrix(rotationMatrix);

    localTransformation.MultiplyTranslationTransform(translation, 1, localTransformation); //Rotating around center so translation should come before rotation

    return Transform::FromProduct(toGlobal, localTransformation);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Haroldas.Vitunskas              05/2017
//---------------------------------------------------------------------------------------
BentleyStatus GeometryUtils::ResizeLineToClosestPoint(DSegment3dR line, DSegment3d boundingLine, bool atEnd, DPoint3d target)
    {
    //Project line points and target on boundingLine
    DPoint3d lineStart, lineEnd;
    line.GetStartPoint(lineStart);
    line.GetEndPoint(lineEnd);

    double closestParam;
    boundingLine.ProjectPoint(lineStart, closestParam, lineStart);
    boundingLine.ProjectPoint(lineEnd, closestParam, lineEnd);

    DPoint3d projectedTarget;
    boundingLine.ProjectPoint(projectedTarget, closestParam, target);
    
    //Select linePoint to extend
    DPoint3d center = DPoint3d::From((lineStart.x + lineEnd.x) / 2, (lineStart.y + lineEnd.y) / 2, (lineStart.z + lineEnd.z) / 2 );
    
    DPoint3d bound1, bound2;
    boundingLine.GetStartPoint(bound1);
    boundingLine.GetEndPoint(bound2);

    DPoint3d bound;
    if (DVec3d::FromStartEnd(center, bound1).IsPositiveParallelTo(DVec3d::FromStartEnd(center, projectedTarget)))
        bound = bound1;
    else if (DVec3d::FromStartEnd(center, bound2).IsPositiveParallelTo(DVec3d::FromStartEnd(center, projectedTarget)))
        bound = bound2;
    else
        return BentleyStatus::ERROR;

    //Get Point to extend to
    DPoint3d extendTarget;
    if (center.Distance(bound) < center.Distance(projectedTarget))
        extendTarget = bound;
    else
        extendTarget = projectedTarget;

    // Get Point to extend from
    DPoint3d newLineStart, resizeOrigin;
    if (atEnd)
        {
        newLineStart = lineStart;
        resizeOrigin = lineEnd;
        }
    else
        {
        newLineStart = lineEnd;
        resizeOrigin = lineStart;
        }

    //check if new line direction doesn't negate the old one
    if (!DVec3d::FromStartEnd(newLineStart, extendTarget).IsPositiveParallelTo(DVec3d::FromStartEnd(newLineStart, resizeOrigin)))
        return BentleyStatus::ERROR;

    //readjust new line
    line = DSegment3d::From(newLineStart, extendTarget);
    return BentleyStatus::SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Haroldas.Vitunskas              06/2017
//---------------------------------------------------------------------------------------
DPoint3d GeometryUtils::FindFurthestPoint(CurveVectorPtr curveVector, DPoint3d point)
    {
    if (CurveVector::INOUT_Out == curveVector->PointInOnOutXY(point))
        return point;

    DPoint3d furthestPoint = point;

    DEllipse3d arc;
    bvector<DPoint3d> const* lineString;
    for (ICurvePrimitivePtr curvePrimitive : *curveVector.get())
        if (curvePrimitive->TryGetArc(arc))
            {
            //Check extreme points of arc
            bvector<DPoint3d> vertices = { arc.center, arc.center, arc.center, arc.center };
            vertices[0].Add(arc.vector0);
            vertices[1].Subtract(arc.vector0);
            vertices[2].Add(arc.vector90);
            vertices[3].Subtract(arc.vector90);
            vertices.push_back(arc.RadiansToPoint(arc.start)); //Add arc start
            vertices.push_back(arc.RadiansToPoint(arc.start + arc.sweep)); //Add arc end

            DPoint3d furthestVertex = point;
            for (DPoint3d vertex : vertices)
                if (furthestVertex.Distance(point) < vertex.Distance(point) && CurveVector::InOutClassification::INOUT_On == curveVector->PointInOnOutXY(vertex))
                    furthestVertex = vertex;

            DVec3d centerToPoint = DVec3d::FromStartEnd(arc.center, point);
            DPoint3d arcStart = arc.RadiansToPoint(arc.start);
            DVec3d centerToArcStart = DVec3d::FromStartEnd(arc.center, arcStart);
            double anglePointToStart = centerToPoint.AngleToXY(centerToArcStart);
            double anglePointToDiameterProjection = msGeomConst_pi;
            double angleStartToDiameterProjection = anglePointToDiameterProjection - anglePointToStart;
            DPoint3d diameterProjection = arc.RadiansToPoint(arc.start + angleStartToDiameterProjection); //Point that is on the other side of center in arc

            if (diameterProjection.Distance(point) < furthestVertex.Distance(point))
                {
                if (furthestPoint.Distance(point) < furthestVertex.Distance(point))
                    furthestPoint = furthestVertex;
                }
            else
                if (furthestPoint.Distance(point) < diameterProjection.Distance(point) && CurveVector::InOutClassification::INOUT_On == curveVector->PointInOnOutXY(diameterProjection))
                    furthestPoint = diameterProjection;
            }
        else
            if (nullptr != (lineString = curvePrimitive->GetLineStringCP()))
                for (DPoint3d cvPoint : *lineString)
                    if (furthestPoint.Distance(point) < cvPoint.Distance(point))
                        furthestPoint = cvPoint;

    return furthestPoint;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Haroldas.Vitunskas              06/2017
//---------------------------------------------------------------------------------------
DSegment3d GeometryUtils::FindEllipseTangent(DPoint3d point, DEllipse3d ellipse)
    {
    DPoint3d center = ellipse.center;

    DVec3d radiusVec = DVec3d::FromStartEnd(point, center);
    radiusVec.RotateXY(msGeomConst_pi / 2);

    DPoint3d tangentStart = point, tangentEnd = point;
    
    tangentStart.Subtract(radiusVec);
    tangentEnd.Add(radiusVec);

    return DSegment3d::From(tangentStart, tangentEnd);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Nerijus.Jakeliunas              08/2016
+---------------+---------------+---------------+---------------+---------------+------*/
int GeometryUtils::FindClosestPointIndex(bvector<DPoint3d>& keyPoints, DPoint3d point)
    {
    int closestPointIndex = -1;
    double minDistance = DBL_MAX;
    int index = 0;

    for (auto&& keyPoint : keyPoints)
        {
        double distance = point.Distance(keyPoint);
        if (distance < minDistance)
            {
            closestPointIndex = index;
            minDistance = distance;
            }
        index++;
        }

    return closestPointIndex;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Haroldas.Vitunskas              06/2017
//---------------------------------------------------------------------------------------
void GeometryUtils::RotatePlacementXY(Placement3dR placement, double theta)
    {
    //Create rotation matrix
    RotMatrix rotationMatrix = RotMatrix::FromAxisAndRotationAngle(2, theta);

    Transform localTransformation = Transform::FromIdentity();
    localTransformation.SetMatrix(rotationMatrix);

    Transform toGlobal = placement.GetTransform();

    Transform rotationTransform = Transform::FromProduct(toGlobal, localTransformation);

    //Transform placement
    YawPitchRollAngles yprAngles;
    DPoint3d origin;
    YawPitchRollAngles::TryFromTransform(origin, yprAngles, rotationTransform);

    placement.SetAngles(yprAngles);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Haroldas.Vitunskas              06/2017
//---------------------------------------------------------------------------------------
void GeometryUtils::TranslatePlacementXY(Dgn::Placement3dR placement, DVec3d translation)
    {
    translation.z = 0;
    TranslatePlacement(placement, translation);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Haroldas.Vitunskas              06/2017
//---------------------------------------------------------------------------------------
void GeometryUtils::TranslatePlacement(Dgn::Placement3dR placement, DVec3d translation)
    {
    Transform translationTransform = Transform::From(translation);
    translationTransform.Multiply(placement.GetOriginR());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Haroldas.Vitunskas                  03/17
//---------------------------------------------------------------------------------------
void GeometryUtils::AddRotatedVectorToPoint(DPoint3dR point, DVec3d axis, double theta)
    {
    axis.RotateXY(theta);
    point.Add(axis);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Haroldas.Vitunskas                  03/17
//---------------------------------------------------------------------------------------
ICurvePrimitivePtr GeometryUtils::CreateArc(DPoint3d center, DPoint3d start, DPoint3d end, bool ccw)
    {
    DEllipse3d arc = DEllipse3d::FromArcCenterStartEnd(center, start, end);

    if (ccw)
        {
        if (!arc.IsCCWSweepXY())
            arc.SetStartEnd(start, end, false);
        }
    else
        if (arc.IsCCWSweepXY())
            arc.SetStartEnd(start, end, false);

    return ICurvePrimitive::CreateArc(arc);
    }
END_BUILDING_NAMESPACE

