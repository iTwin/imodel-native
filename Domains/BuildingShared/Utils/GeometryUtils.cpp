/*--------------------------------------------------------------------------------------+
|
|     $Source: Utils/GeometryUtils.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#include "PublicApi/UtilsApi.h"
#include <dgnPlatform/ViewController.h>

USING_NAMESPACE_BENTLEY_DGN

#define CONTAINEMENT_TOLERANCE 0.001

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
    out = nullptr;

    if (!primitive.IsValid())
        {
        return ERROR;
        }

    BentleyStatus status = SUCCESS;
    uint32_t id = assignIds ? 1 : 0;
    switch (primitive->GetGeometryType())
        {
        case GeometricPrimitive::GeometryType::CurvePrimitive:
            status = BRepUtil::Create::BodyFromCurveVector(out, *CurveVector::Create(CurveVector::BoundaryType::BOUNDARY_TYPE_None, primitive->GetAsICurvePrimitive()), id);
            break;
        case GeometricPrimitive::GeometryType::CurveVector:
            status = BRepUtil::Create::BodyFromCurveVector(out, *primitive->GetAsCurveVector(), id);
            break;
        case GeometricPrimitive::GeometryType::SolidPrimitive:
            status = BRepUtil::Create::BodyFromSolidPrimitive(out, *primitive->GetAsISolidPrimitive(), id);
            break;
        case GeometricPrimitive::GeometryType::BsplineSurface:
            status = BRepUtil::Create::BodyFromBSurface(out, *primitive->GetAsMSBsplineSurface(), id);
            break;
        case GeometricPrimitive::GeometryType::Polyface:
            status = BRepUtil::Create::BodyFromPolyface(out, *primitive->GetAsPolyfaceHeader(), id);
            break;
        case GeometricPrimitive::GeometryType::BRepEntity:
            out = primitive->GetAsIBRepEntity();
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
// @bsimethod                                    Mindaugas Butkus                08/2017
//---------------+---------------+---------------+---------------+---------------+------
CurveVectorPtr GeometryUtils::GetXYCrossSection(IBRepEntityCR solid, double z)
    {
    DRange3d range = solid.GetEntityRange();

    if (range.low.z > z || range.high.z < z)
        return nullptr;

    bvector<DPoint3d> crossSectionPlanePoints;
    crossSectionPlanePoints.push_back({ range.low.x, range.low.y, z });
    crossSectionPlanePoints.push_back({ range.high.x, range.low.y , z });
    crossSectionPlanePoints.push_back({ range.high.x, range.high.y, z });
    crossSectionPlanePoints.push_back({ range.low.x, range.high.y, z });
    CurveVectorPtr crossSection = CurveVector::CreateLinear(crossSectionPlanePoints, CurveVector::BOUNDARY_TYPE_Outer);

    IBRepEntityPtr crossSectionSheetBody = nullptr;
    BRepUtil::Create::BodyFromCurveVector(crossSectionSheetBody, *crossSection);

    IBRepEntityPtr intersectionTool = solid.Clone();
    BRepUtil::Modify::BooleanOperation(crossSectionSheetBody, intersectionTool, BRepUtil::Modify::BooleanMode::Intersect);
    if (crossSectionSheetBody.IsValid()) // the given solid is intersecting XY plane on given Z coordinate
        {
        bvector<ISubEntityPtr> faces;
        BRepUtil::GetBodyFaces(&faces, *crossSectionSheetBody);

        IBRepEntityPtr crossSection;
        for (auto face : faces)
            {
            if (!crossSection.IsValid())
                crossSection = face->GetGeometry()->GetAsIBRepEntity();
            else
                {
                IBRepEntityPtr tool = face->GetGeometry()->GetAsIBRepEntity();
                BRepUtil::Modify::BooleanOperation(crossSection, tool, BRepUtil::Modify::BooleanMode::Unite);
                }
            }

        CurveVectorPtr crossSectionCV = BRepUtil::Create::BodyToCurveVector(*crossSection);
        crossSectionCV->ConsolidateAdjacentPrimitives();
        return crossSectionCV;
        }

    // the given solid does not intersect XY plane on given Z coordinate, just touches it
    bvector<DPoint3d> touchPoints;
    bvector<ISubEntityPtr> edgesAndVertices;
    BRepUtil::GetBodyVertices(&edgesAndVertices, solid);
    BRepUtil::GetBodyEdges(&edgesAndVertices, solid);
    for (ISubEntityPtr subEntity : edgesAndVertices)
        {
        ISubEntity::SubEntityType subEntityType = subEntity->GetSubEntityType();
        GeometricPrimitiveCPtr geom = subEntity->GetGeometry();

        if (ISubEntity::SubEntityType::Vertex == subEntityType)
            {
            EmbeddedDPoint3dArray vertexPoints = *geom->GetAsICurvePrimitive()->GetPointStringCP();
            for (DPoint3d point : vertexPoints)
                {
                if (DoubleOps::TolerancedComparison(point.z, z) == 0 && // check if AlmostEqual
                    std::find(touchPoints.begin(), touchPoints.end(), point) == touchPoints.end())
                    touchPoints.push_back(point);
                }
            }
        else if(ISubEntity::SubEntityType::Edge == subEntityType)
            {
            ICurvePrimitivePtr subEntityPrimitive = geom->GetAsICurvePrimitive();

            if (subEntityPrimitive->GetCurvePrimitiveType() == ICurvePrimitive::CurvePrimitiveType::CURVE_PRIMITIVE_TYPE_Arc)
                {
                DPlane3d xyPlane = DPlane3d::FromOriginAndNormal(DPoint3d::From(range.low.x, range.low.y, z), DVec3d::From(0, 0, 1));
                DPoint3d intersections[2];

                DEllipse3d arc = *subEntityPrimitive->GetArcCP();
                DPoint4d plane;
                xyPlane.GetDPoint4d(plane);
                int intersectionCount = arc.IntersectPlane(intersections, plane);
                BeAssert(intersectionCount < 2 && "The boolean operation should have returned a cross section");

                if (1 == intersectionCount)
                    {
                    DPoint3d intersectionPoint = arc.RadiansToPoint(intersections[0].z);
                    if (DoubleOps::TolerancedComparison(intersectionPoint.z, z) == 0 && // check if AlmostEqual
                        std::find(touchPoints.begin(), touchPoints.end(), intersectionPoint) == touchPoints.end())
                        touchPoints.push_back(intersectionPoint);
                    }
                }
            }
        }
    ICurvePrimitivePtr pointString = ICurvePrimitive::CreatePointString(touchPoints);
    CurveVectorPtr pointStringCV = CurveVector::Create(pointString);

    return pointStringCV;
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Nerijus.Jakeliunas              06/2017
//---------------+---------------+---------------+---------------+---------------+------
CurveVectorPtr GeometryUtils::ExtractXYProfileFromSolid(IBRepEntityCR solid, CurveVectorPtr* pTopProfile)
    {
    bvector<ISubEntityPtr> subEntities;
    BRepUtil::GetBodyFaces(&subEntities, solid);
    double lowestZValue = DBL_MAX;
    double highestZValue = DBL_MIN;
    CurveVectorPtr profilePtr = nullptr;

    for (ISubEntityPtr subEntityPtr : subEntities)
        {
        if (!subEntityPtr.IsValid() || !BRepUtil::IsPlanarFace(*subEntityPtr))
            {
            continue;
            }

        DPoint3d point;
        DVec3d normal, uDir, vDir;
        DPoint2d uvParam{ 0.0, 0.0 };
        BRepUtil::EvaluateFace(*subEntityPtr, point, normal, uDir, vDir, uvParam);
        if (!normal.IsParallelTo(DVec3d::From(0.0, 0.0, -1.0)))
            {
            continue;
            }

        GeometricPrimitiveCPtr geometry = subEntityPtr->GetGeometry();
        IBRepEntityPtr sheet = geometry->GetAsIBRepEntity();
        if (!sheet.IsValid() || sheet->GetEntityType() != IBRepEntity::EntityType::Sheet)
            {
            continue;
            }

        CurveVectorPtr curveVectorPtr = BRepUtil::Create::BodyToCurveVector(*sheet);
        if (!curveVectorPtr.IsValid())
            {
            continue;
            }

        DPoint3d startPoint;
        curveVectorPtr->GetStartPoint(startPoint);
        if (lowestZValue > startPoint.z && normal.IsPositiveParallelTo (DVec3d::From (0.0, 0.0, -1.0)))
            {
            lowestZValue = startPoint.z;
            profilePtr = curveVectorPtr;
            }
        if (highestZValue < startPoint.z && normal.IsPositiveParallelTo (DVec3d::From (0.0, 0.0, 1.0)))
            {
            highestZValue = startPoint.z;
            if (pTopProfile)
                *pTopProfile = curveVectorPtr;
            }
        }

    if (profilePtr.IsValid())
        {
        profilePtr->ConsolidateAdjacentPrimitives();
        profilePtr->ReverseCurvesInPlace(); // need to switch to CCW direction
        }
    else
        {
        DRange3d solidRange = solid.GetEntityRange();

        if (solidRange.low.z != solidRange.high.z)
            {
            profilePtr = GetXYCrossSection(solid, solidRange.low.z);
            if (profilePtr->GetBoundaryType () == CurveVector::BoundaryType::BOUNDARY_TYPE_Open)
                {
                profilePtr = GetXYCrossSection (solid, solidRange.low.z + 0.01);

                Transform transform = Transform::FromIdentity ();
                DPoint3d translation = { 0.0, 0.0, 0.0 };
                translation.z = -0.01;
                transform.SetTranslation (translation);

                profilePtr = profilePtr->Clone (transform);
                }
            }
        }


    if (pTopProfile)
        {
        if ((*pTopProfile).IsValid ())
            {
            (*pTopProfile)->ConsolidateAdjacentPrimitives ();
            (*pTopProfile) = CurveVector::ReduceToCCWAreas (*(*pTopProfile)); // need to switch to CCW direction
            }
        else
            {
            DRange3d solidRange = solid.GetEntityRange ();

            if (solidRange.low.z != solidRange.high.z)
                {
                *pTopProfile = GetXYCrossSection (solid, solidRange.high.z);
                if ((*pTopProfile)->GetBoundaryType () == CurveVector::BoundaryType::BOUNDARY_TYPE_Open)
                    {
                    *pTopProfile = GetXYCrossSection (solid, solidRange.high.z - 0.01);

                    Transform transform = Transform::FromIdentity ();
                    DPoint3d translation = { 0.0, 0.0, 0.0 };
                    translation.z = 0.01;
                    transform.SetTranslation (translation);

                    *pTopProfile = (*pTopProfile)->Clone (transform);
                    }
                }
            
            }
        }

    return profilePtr;
    }

//---------------------------------------------------------------------------------------
// @param         slicedGeometry      OUT solid slices paired with corresponding bottom planes of the cuts
// @param         geometryToSlice     IN geometry to slice
// @param         zElevationVector    IN vector of z elevations
// @bsimethod                                    Nerijus.Jakeliunas              06/2017
//---------------+---------------+---------------+---------------+---------------+-------
BentleyStatus GeometryUtils::SliceBodyByZElevations
(
bvector<bpair<Dgn::IBRepEntityPtr, Dgn::IBRepEntityPtr>>& slicedGeometry,
IBRepEntityCR geometryToSlice,
bvector<double>& zElevationVector
)
    {
    DRange3d range = geometryToSlice.GetEntityRange ();
    if (range.IsNull ())
        {
        return BSIERROR;
        }

    double rangeExtendValue = 0.5;
    range.Extend (rangeExtendValue);
    bvector<DPoint3d> cutProfilePoints;
    cutProfilePoints.push_back ({ range.low.x, range.low.y, 0.0 });
    cutProfilePoints.push_back ({ range.low.x + range.XLength (), range.low.y , 0.0 });
    cutProfilePoints.push_back ({ range.low.x + range.XLength (), range.low.y + range.YLength (), 0.0 });
    cutProfilePoints.push_back ({ range.low.x, range.low.y + range.YLength (), 0.0 });

    CurveVectorPtr cutProfilePtr = CurveVector::CreateLinear (cutProfilePoints, CurveVector::BOUNDARY_TYPE_Outer);
    IBRepEntityPtr sheetBody = nullptr;
    BRepUtil::Create::BodyFromCurveVector (sheetBody, *cutProfilePtr);

    bvector<IBRepEntityPtr> cutSheetBodies;

    Transform transform = Transform::FromIdentity ();
    DPoint3d translation = { 0.0, 0.0, 0.0 };

    for (bvector<double>::iterator pIter = zElevationVector.begin (); pIter != zElevationVector.end (); ++pIter)
        {
        translation.z = *pIter;
        transform.SetTranslation (translation);

        IBRepEntityPtr elevatedSheetBody = sheetBody->Clone ();
        elevatedSheetBody->ApplyTransform (transform);

        cutSheetBodies.push_back (elevatedSheetBody);
        }


    //TODO: move to a different method..
    //slice the body by sheets
    for (bvector<IBRepEntityPtr>::iterator pSheetIter = cutSheetBodies.begin (); (pSheetIter+1) != cutSheetBodies.end (); ++pSheetIter)
        {
        IBRepEntityPtr nextSheetPtr = *(pSheetIter + 1);
        IBRepEntityPtr slice = geometryToSlice.Clone ();
        BRepUtil::Modify::BooleanCut (slice, *(*pSheetIter), BRepUtil::Modify::CutDirectionMode::Backward, BRepUtil::Modify::CutDepthMode::All, 0.0, true);
        BRepUtil::Modify::BooleanCut (slice, *nextSheetPtr, BRepUtil::Modify::CutDirectionMode::Forward, BRepUtil::Modify::CutDepthMode::All, 0.0, true);
        slicedGeometry.push_back ({ slice, *pSheetIter });
        }

    return BSISUCCESS;
    }

//---------------------------------------------------------------------------------------
// @param         slicedGeometry      OUT solid slices paired with corresponding bottom planes of the cuts
// @param         geometryToSlice     IN
// @param         cuttingPlaneProfile IN
// @param         sliceHeight         IN
// @bsimethod                                    Nerijus.Jakeliunas              06/2017
//---------------+---------------+---------------+---------------+---------------+-------
BentleyStatus GeometryUtils::SliceBodyByPlanes
(
bvector<bpair<Dgn::IBRepEntityPtr, CurveVectorPtr>>& slicedGeometry,
IBRepEntityCR geometryToSlice,
CurveVectorCR cuttingPlaneProfile,
double sliceHeight
)
    {
    if (sliceHeight < 0.0001)
        {
        return BSIERROR;
        }

    DRange3d range = geometryToSlice.GetEntityRange();
    DPoint3d profilePoint;
    cuttingPlaneProfile.GetStartPoint(profilePoint);
    DPoint3d offset = DPoint3d::From(0.0, 0.0, range.low.z - profilePoint.z);
    Transform tranform = Transform::From(offset);
    CurveVectorPtr bottomPlanePtr = cuttingPlaneProfile.Clone();
    bottomPlanePtr->TransformInPlace(tranform);

    double remainingHeight = range.ZLength();
    while (remainingHeight > 0.0001)
        {
        IBRepEntityPtr bottomSheetBody = nullptr;
        BRepUtil::Create::BodyFromCurveVector(bottomSheetBody, *bottomPlanePtr);
        if (!bottomSheetBody.IsValid())
            {
            slicedGeometry = bvector<bpair<Dgn::IBRepEntityPtr, CurveVectorPtr>>();
            return BSIERROR;
            }

        // cut out material below bottom plane
        IBRepEntityPtr slice = geometryToSlice.Clone();
        BRepUtil::Modify::BooleanCut(slice, *bottomSheetBody, BRepUtil::Modify::CutDirectionMode::Backward, BRepUtil::Modify::CutDepthMode::All, sliceHeight, true);
        if (!slice.IsValid())
            {
            slicedGeometry = bvector<bpair<Dgn::IBRepEntityPtr, CurveVectorPtr>>();
            return BSIERROR;
            }

        offset = DPoint3d::From(0.0, 0.0, sliceHeight);
        tranform = Transform::From(offset);
        CurveVectorPtr topPlanePtr = bottomPlanePtr->Clone();
        topPlanePtr->TransformInPlace(tranform);

        IBRepEntityPtr topSheetBody = nullptr;
        BRepUtil::Create::BodyFromCurveVector(topSheetBody, *topPlanePtr);
        if (!topSheetBody.IsValid())
            {
            slicedGeometry = bvector<bpair<Dgn::IBRepEntityPtr, CurveVectorPtr>>();
            return BSIERROR;
            }

        // cut out material above top plane
        BRepUtil::Modify::BooleanCut(slice, *topSheetBody, BRepUtil::Modify::CutDirectionMode::Forward, BRepUtil::Modify::CutDepthMode::All, sliceHeight, true);
        if (!slice.IsValid())
            {
            slicedGeometry = bvector<bpair<Dgn::IBRepEntityPtr, CurveVectorPtr>>();
            return BSIERROR;
            }

        slicedGeometry.push_back({ slice, bottomPlanePtr });

        bottomPlanePtr = topPlanePtr;
        remainingHeight -= sliceHeight;
        }

    return BSISUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Haroldas.Vitunskas             03/2017
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus GeometryUtils::GetGeometricPrimitivesFromGeometricElement
(
bvector<Dgn::GeometricPrimitivePtr>& geometricPrimitives,
Dgn::GeometricElementCPtr geoElement
)
    {
    auto pGeometrySource = geoElement->ToGeometrySource();
    if (nullptr == pGeometrySource)
        return BentleyStatus::ERROR;

    GeometryCollection geomData(*pGeometrySource);
    Transform elemToWorld = (*geomData.begin()).GetGeometryToWorld();
    if (geomData.begin() == geomData.end())
        return BentleyStatus::ERROR;

    for (GeometryCollection::Iterator it = geomData.begin(); it != geomData.end(); ++it)
        {
        GeometricPrimitivePtr clone = (*it).GetGeometryPtr()->Clone();
        Transform elemToWorld = (*it).GetGeometryToWorld();
        clone->TransformInPlace(elemToWorld);
        geometricPrimitives.push_back(clone);
        }
    return BentleyStatus::SUCCESS;
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Jonas.Valiunas                  05/2017
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus GeometryUtils::GetIBRepEntitiesFromGeometricElement
(
bvector<Dgn::IBRepEntityPtr>& brepsOut,
Dgn::GeometricElementCPtr geoElement
)
    {
    BentleyStatus status = BentleyStatus::ERROR;
    bvector<Dgn::GeometricPrimitivePtr> primitives;
    GetGeometricPrimitivesFromGeometricElement(primitives, geoElement);
    for (bvector<Dgn::GeometricPrimitivePtr>::iterator iter = primitives.begin(); iter != primitives.end(); ++iter)
        {
        Dgn::IBRepEntityPtr brep;
        if (SUCCESS == (status = CreateBodyFromGeometricPrimitive(brep, *iter)))
            {
            brepsOut.push_back(brep);
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
// @bsimethod                                    Haroldas.Vitunskas                  07/17
//---------------------------------------------------------------------------------------
bool GeometryUtils::AlmostEqual(double a, double b, double tolerance)
    {
    return fabs(a - b) <= tolerance;
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
bvector<DPoint3d> GeometryUtils::ExtractSingleCurvePoints
(
    CurveVectorPtr curve
)
    {
    bvector<DPoint3d> points;

    curve->ConsolidateAdjacentPrimitives(true);
    if (!curve.IsValid())
        return points;

    if (curve->size() == 0)
        return points;

    ICurvePrimitivePtr boundaryCurve = curve->at(0);
    if (!boundaryCurve.IsValid())
        return points;

    if (boundaryCurve->GetCurvePrimitiveType() == ICurvePrimitive::CURVE_PRIMITIVE_TYPE_CurveVector)
        boundaryCurve = boundaryCurve->GetChildCurveVectorP()->at(0);
    if (!boundaryCurve.IsValid())
        return points;


    if (boundaryCurve->GetCurvePrimitiveType() == ICurvePrimitive::CURVE_PRIMITIVE_TYPE_LineString)
        points = *boundaryCurve->GetLineStringP();

    DPoint3d start, end;
    boundaryCurve->GetStartEnd(start, end);
    points.insert(points.begin(), start);
    points.push_back(end);
    auto pointCompare = [](DPoint3d const& p1, DPoint3d const& p2) { return p1.AlmostEqual(p2); };
    points.erase(std::unique(points.begin(), points.end(), pointCompare), points.end());

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
    DPoint3d center = DPoint3d::From((lineStart.x + lineEnd.x) / 2, (lineStart.y + lineEnd.y) / 2, (lineStart.z + lineEnd.z) / 2);

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
// @bsimethod                                    Haroldas.Vitunskas              07/2017
//---------------------------------------------------------------------------------------
CurveVectorPtr GeometryUtils::OffsetCurveInnerOuterChildren(CurveVectorCPtr originalCurve, double innerOffset, double outerOffset, bool mergeIntoDifference)
    {
    CurveVectorPtr offsetedCurve = CurveVector::Create(originalCurve->GetBoundaryType());

    if (CurveVector::BoundaryType::BOUNDARY_TYPE_Inner == originalCurve->GetBoundaryType())
        {
        // area offset uses area union/difference to get the offseted area shape, so we will need to convert the shape to outer boundary type first to get the right shape area
        originalCurve = CurveVector::ReduceToCCWAreas(*originalCurve);
        offsetedCurve = originalCurve->AreaOffset(innerOffset);
        if (!offsetedCurve.IsValid())
            return nullptr;

        offsetedCurve->SetBoundaryType(CurveVector::BoundaryType::BOUNDARY_TYPE_Inner);
        }
    else if (CurveVector::BoundaryType::BOUNDARY_TYPE_Outer == originalCurve->GetBoundaryType())
        {
        originalCurve = CurveVector::ReduceToCCWAreas(*originalCurve);
        offsetedCurve = originalCurve->AreaOffset(outerOffset);
        if (!offsetedCurve.IsValid())
            return nullptr;

        offsetedCurve->SetBoundaryType(CurveVector::BoundaryType::BOUNDARY_TYPE_Outer);
        }
    else
        {
        for (ICurvePrimitivePtr curvePrimitive : *originalCurve)
            {
            CurveVectorCPtr child = curvePrimitive->GetChildCurveVectorCP();
            CurveVectorPtr offsetedChild = OffsetCurveInnerOuterChildren(child, innerOffset, outerOffset)->Clone();

            if (offsetedChild.IsValid())
                offsetedCurve->Add(offsetedChild);
            }
        }

    if (!mergeIntoDifference)
        return offsetedCurve;

    // First merge all overlapping inner curve vectors. Since curve vectors are inner, (Acting like complement sets), to merge we will need to get union of their
    CurveVectorPtr innerUnion = CurveVector::Create(CurveVector::BoundaryType::BOUNDARY_TYPE_Inner);
    CurveVectorPtr outerUnion = CurveVector::Create(CurveVector::BoundaryType::BOUNDARY_TYPE_Outer);

    if (CurveVector::BOUNDARY_TYPE_Inner == offsetedCurve->GetBoundaryType())
        innerUnion = offsetedCurve;
    else if (CurveVector::BOUNDARY_TYPE_Outer == offsetedCurve->GetBoundaryType())
        outerUnion = offsetedCurve;
    else
        {
        for (ICurvePrimitivePtr curvePrimitive : *offsetedCurve)
            {
            CurveVectorCPtr child = curvePrimitive->GetChildCurveVectorCP();
            if (!child.IsValid())
                child = offsetedCurve; // if no children, use the actual curve

            if (CurveVector::BoundaryType::BOUNDARY_TYPE_Inner == child->GetBoundaryType())
                innerUnion = CurveVector::AreaUnion(*innerUnion, *child);
            else if (CurveVector::BoundaryType::BOUNDARY_TYPE_Outer == child->GetBoundaryType())
                outerUnion = CurveVector::AreaUnion(*outerUnion, *child);
            }
        }

    if (innerUnion.IsValid() && innerUnion->size() > 0 && outerUnion.IsValid() && outerUnion->size() > 0)
        {
        // Now to get the merged curve vector, we'll need to get difference between outer and inner unions
        CurveVectorPtr merged = CurveVector::AreaDifference(*outerUnion, *innerUnion);
        merged->ConsolidateAdjacentPrimitives(true);

        return merged;
        }
    
    if (outerUnion.IsValid())
        return outerUnion;

    return innerUnion;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Haroldas.Vitunskas              06/2017
//---------------------------------------------------------------------------------------
DPoint3d GeometryUtils::FindFurthestPoint(CurveVectorCPtr curveVector, DPoint3d point)
    {
    DPoint3d furthestPoint = point;

    // Project point on curve vector
    CurveLocationDetail hit;
    curveVector->ClosestPointBounded(point, hit);
    point = hit.point;

    DEllipse3d arc;
    bvector<DPoint3d> const* lineString;
    CurveVectorCP childCurveVector;

    for (ICurvePrimitivePtr curvePrimitive : *curveVector.get())
        {
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
        else if (nullptr != (lineString = curvePrimitive->GetLineStringCP()))
            {
            for (DPoint3d cvPoint : *lineString)
                if (furthestPoint.Distance(point) < cvPoint.Distance(point))
                    furthestPoint = cvPoint;
            }
        else if (nullptr != (childCurveVector = curvePrimitive->GetChildCurveVectorCP()))
            {
            if (CurveVector::INOUT_Out == childCurveVector->PointInOnOutXY(point))
                continue;

            DPoint3d furthestChildPoint = FindFurthestPoint(childCurveVector, point);
            if (furthestChildPoint.Distance(point) > furthestPoint.Distance(point))
                furthestPoint = furthestChildPoint;
            }
        }

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
// @bsimethod                                    Haroldas.Vitunskas              10/2017
//---------------------------------------------------------------------------------------
void GeometryUtils::RotatePlacementAroundPointXY(Dgn::Placement3dR placement, DPoint3d origin, double theta)
    {
    DVec3d translation = DVec3d::FromStartEnd(placement.GetOrigin(), origin);
    
    TranslatePlacementXY(placement, translation); // Move to new origin
    
    RotatePlacementXY(placement, theta); // Rotate
    translation.RotateXY(theta);

    translation.Negate();
    TranslatePlacementXY(placement, translation); // Move back
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
// @bsimethod                                    Haroldas.Vitunskas              06/2017
//---------------------------------------------------------------------------------------
double GeometryUtils::RotMatrixToAngleXY(RotMatrix rotMatrix)
    {
    DVec3d v = DVec3d::From(1, 0, 0);

    DPoint3d rV = v;
    rotMatrix.Multiply(rV);

    return - DVec3d::From(rV).AngleToXY(v);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Haroldas.Vitunskas              06/2017
//---------------------------------------------------------------------------------------
double GeometryUtils::PlacementToAngleXY(Placement3d placement)
    {
    YawPitchRollAngles angles = placement.GetAngles();
    RotMatrix rotMatrix = angles.ToRotMatrix();
    return RotMatrixToAngleXY(rotMatrix);
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

//---------------------------------------------------------------------------------------
// @bsimethod                                    Haroldas.Vitunskas                  06/17
//---------------------------------------------------------------------------------------
DgnExtrusionDetail GeometryUtils::CreatePlaneExtrusionDetail(double length, double height)
    {
    DVec3d gridNormal = DVec3d::From(1.0, 0.0, 0.0);

    DPoint3d startPoint = DPoint3d::FromZero();
    DPoint3d endPoint = DPoint3d::FromSumOf(startPoint, gridNormal, length);

    return CreatePlaneExtrusionDetail(startPoint, endPoint, height);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Haroldas.Vitunskas                  06/17
//---------------------------------------------------------------------------------------
DgnExtrusionDetail GeometryUtils::CreatePlaneExtrusionDetail(DPoint3d startPoint, DPoint3d endPoint, double height)
    {
    bvector<DPoint3d> points = { startPoint, endPoint };

    CurveVectorPtr base = CurveVector::CreateLinear(points, CurveVector::BOUNDARY_TYPE_None);
    DVec3d extrusionUp = DVec3d::From(0.0, 0.0, height);

    return DgnExtrusionDetail(base, extrusionUp, false);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Haroldas.Vitunskas                  06/17
//---------------------------------------------------------------------------------------
DgnExtrusionDetail GeometryUtils::CreateArcExtrusionDetail(double radius, double baseAngle, double height, double extendLength)
    {
    double extendAngle = extendLength / radius;
    if (baseAngle > 2 * (msGeomConst_pi - extendAngle))
        extendAngle = (2 * msGeomConst_pi - baseAngle) / 3; //Do not allow arc overlapping

    DPoint3d center = DPoint3d::FromZero();

    DPoint3d start = center;
    GeometryUtils::AddRotatedVectorToPoint(start, DVec3d::FromStartEnd(center, DPoint3d::From(radius, 0.0)), (baseAngle + extendAngle));

    DPoint3d end = center;
    GeometryUtils::AddRotatedVectorToPoint(end, DVec3d::FromStartEnd(center, DPoint3d::From(radius, 0.0)), -extendAngle);

    CurveVectorPtr base = CurveVector::Create(GeometryUtils::CreateArc(center, start, end, false));

    DVec3d extrusionUp = DVec3d::From(0.0, 0.0, height);

    return DgnExtrusionDetail(base, extrusionUp, false);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Haroldas.Vitunskas                  06/17
//---------------------------------------------------------------------------------------
DgnExtrusionDetail GeometryUtils::CreateSplineExtrusionDetail(bvector<DPoint3d> poles, double height, int baseOrder)
    {
    bvector<double> weights;
    for (DPoint3d pole : poles)
        weights.push_back(1.0);

    int order = poles.size() < baseOrder ? poles.size() : baseOrder;

    bvector<double> knots;
    for (int i = 0; i < order + poles.size(); ++i)
        knots.push_back(i);

    MSBsplineCurvePtr bspline = MSBsplineCurve::CreateFromPolesAndOrder(poles, &weights, &knots, order, false, false);
    ICurvePrimitivePtr curvePrimitive = ICurvePrimitive::CreateBsplineCurve(bspline);
    CurveVectorPtr base = CurveVector::Create(curvePrimitive);

    DVec3d extrusionUp = DVec3d::From(0.0, 0.0, height);

    return DgnExtrusionDetail(base, extrusionUp, false);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Haroldas.Vitunskas                  07/17
//---------------------------------------------------------------------------------------
bool GeometryUtils::CheckIfLineIntersectsCurveVector(bvector<DSegment3d>& intersections, DSegment3d line, CurveVectorCPtr curveVector)
    {
    intersections.clear();

    bool status = false;

    DEllipse3d arc;
    bvector<DPoint3d> const * lineString;
    CurveVectorCP childCurveVector;

    for (ICurvePrimitivePtr curvePrimitive : *curveVector)
        {
        if (curvePrimitive->TryGetArc(arc))
            {
            DPoint3dP intersectionPoints = nullptr;
            int intersectionCount = arc.IntersectSweptDSegment3dBounded(intersectionPoints, nullptr, nullptr, line);
            if (nullptr == intersectionPoints)
                continue;

            if (intersectionCount > 0)
                status = true;

            for (size_t i = 0; i < intersectionCount; ++i)
                {
                DSegment3d intersection = DSegment3d::From(intersectionPoints[i], intersectionPoints[i]);
                intersections.push_back(intersection);
                }
            }
        else if (nullptr != (lineString = curvePrimitive->GetLineStringCP()))
            {
            for (size_t i = 0; i < lineString->size() - 1; ++i)
                {
                DSegment3d intersection;
                if (!CheckIfTwoLinesIntersect(intersection, line, DSegment3d::From((*lineString)[i], (*lineString)[i + 1])))
                    continue;

                status = true;
                intersections.push_back(intersection);
                }
            }
        else if (nullptr != (childCurveVector = curvePrimitive->GetChildCurveVectorCP()))
            {
            bvector<DSegment3d> childIntersections;
            if (!CheckIfLineIntersectsCurveVector(childIntersections, line, childCurveVector))
                continue;

            status = true;
            intersections.insert(intersections.begin(), childIntersections.begin(), childIntersections.end());
            }
        }

    // Remove intersection points already covered by intersection segments
    for (DSegment3d segment : intersections)
        {
        DRange3d segmentRange;
        segment.GetRange(segmentRange);

        if (segment.IsAlmostSinglePoint())
            continue;

        bvector<DSegment3d>::iterator it;

        while (intersections.end() != (it = std::find_if(intersections.begin(), intersections.end(), [&](DSegment3d other) { return other.IsAlmostSinglePoint() && IsPointContainedInRangeToTolerance(segmentRange, other.point[0], BUILDING_TOLERANCE); })))
            {
            intersections.erase(it);
            }
        }

    status = status && (intersections.size() > 0);

    return status;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Haroldas.Vitunskas                  07/17
//---------------------------------------------------------------------------------------
bool GeometryUtils::CheckIfTwoLinesIntersect(DSegment3dR intersectionSegment, DSegment3d line1, DSegment3d line2)
    {
    intersectionSegment = DSegment3d::FromZero();

    // Convert lines to form [x, y, z] = v * dt
    DVec3d v1, v2, d1, d2;
    ToParametricForm(v1, d1, line1);
    ToParametricForm(v2, d2, line2);

    // Lines intersect if there are such a and b that v1 + a*d1 = v2 + b*d1. We get equasion system:
    //                                       +-          -+ +- -+   +-           -+
    // x0_1 + a * x1_1 = x0_2 + b * x1_2     | x1_1 -x1_2 | | a |   | x0_2 - x0_1 |
    // y0_1 + a * y1_1 = y0_2 + b * y1_2  => | y1_1 -y1_2 | | b | = | y0_2 - y0_1 |
    // z0_1 + a * z1_1 = z0_2 + b * z1_2     | z1_1 -z1_2 | |   |   | z0_2 - z0_1 |
    //                                       +-          -+ +- -+   +-           -+

    // Since we have 3 equasions and 2 unknown numbers let's solve the equasion on a 2d plane and check if it also works for the Z axis. So our equasion is:
    //                                       +-          -+ +- -+   +-           -+
    // x0_1 + a * x1_1 = x0_2 + b * x1_2     | x1_1 -x1_2 | | a |   | x0_2 - x0_1 |
    // y0_1 + a * y1_1 = y0_2 + b * y1_2  => | y1_1 -y1_2 | | b | = | y0_2 - y0_1 |
    //                                       +-          -+ +- -+   +-           -+

    // Using Cramer's rule (https://en.wikipedia.org/wiki/Cramer%27s_rule):
    double det = GetDeterminant2x2(d1.x, -d2.x,
                                   d1.y, -d2.y);
    if (0 == det) // Determinant is 0 => lines are either parallel and never intersect or they coincide
        return CheckIfParallelLineSegmentsCoincide(intersectionSegment, line1, line2);
        
    double detA = GetDeterminant2x2((v2.x - v1.x), -d2.x,
                                    (v2.y - v1.y), -d2.y);
    double a = detA / det;


    double detB = GetDeterminant2x2(d1.x, (v2.x - v1.x),
                                    d1.y, (v2.y - v1.y));
    double b = detB / det;

    // Let's check a and b for the z axis:
    if (v1.z + a * d1.z != v2.z + b * d2.z)
        return false; // Line segments do not intersect
       
    // Intersection point will be either v1 + a*d1 or v2 + b*d2:
    DPoint3d intersectionPoint = GetPointFromParamtericForm(v1, d1, a);

    // This only shows that line segment's extensions intersect. Let's check if the intersection point is within range of both lines
    DRange3d range1, range2;
    line1.GetRange(range1);
    line2.GetRange(range2);

    if (!IsPointContainedInRangeToTolerance(range1, intersectionPoint, BUILDING_TOLERANCE) || !IsPointContainedInRangeToTolerance(range2, intersectionPoint, BUILDING_TOLERANCE))
        return false;

    // Set intersection segment to be a single point segment of the intersection point
    intersectionSegment.point[0] = intersectionSegment.point[1] = intersectionPoint;
    return true;
    }
    
//---------------------------------------------------------------------------------------
// @bsimethod                                    Haroldas.Vitunskas                  07/17
//---------------------------------------------------------------------------------------
void GeometryUtils::ToParametricForm(DVec3dR v, DVec3dR d, DSegment3d line)
    {
    // Converts line to [x, y, z] = [x0, y0, z0] + t[x1, y1, z1]
    // Let's say that DVec3d v = [x0, y0, z0] and DVec3d d = [x1, y1, z1]

    // x0 and x1 are such numbers that x = x0 + x1*t, respectively, (y0, y1) : y = y0 + y1*t and (z0, z1) : z = z0 + z1*t
    // When t = 0, then x0 = x, y0 = y, z0 = z. Let's say that t = 0 is at first point of line segment
    v = DVec3d::From(line.point[0]);

    // When t = 1, then x1 = x - x0, y1 = y - y0, z1 = z - z0. Let's say that t = 1 is at second point of line segment
    d = DVec3d::From(line.point[1]);
    d.Subtract(v);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Haroldas.Vitunskas                  07/17
//---------------------------------------------------------------------------------------
DPoint3d GeometryUtils::GetPointFromParamtericForm(DVec3d v, DVec3d d, double t)
    {
    d.Scale(t); // d * t
    v.Add(d);   // v + d * t 
    return v;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Haroldas.Vitunskas                  07/17
//---------------------------------------------------------------------------------------
double GeometryUtils::GetDeterminant2x2(double a11, double a12, double a21, double a22)
    {
    //          |         |
    // det =    | a11 a12 | = (a11 * a22) - (a21 * a12)
    //          | a21 a22 |
    //          |         |
    return a11 * a22 - a21 * a12;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Haroldas.Vitunskas                  07/17
//---------------------------------------------------------------------------------------
bool GeometryUtils::CheckIfParallelLineSegmentsCoincide(DSegment3dR intersectionSegment, DSegment3d line1, DSegment3d line2)
    {
    // If lines coincide, then any point P(x_p, y_p, z_p) of line1 can be calculated using parametric coordinates of line2:
    // There exists such t that 
    // x_p = x0_2 + x1_2 * t; 
    // y_p = y0_2 + y1_2 * t;
    // z_p = z0_2 + z1_2 * t;

    intersectionSegment = DSegment3d::FromZero();

    if (!DVec3d::FromStartEnd(line1.point[0], line1.point[1]).IsParallelTo(DVec3d::FromStartEnd(line2.point[0], line2.point[1])))
        return false;

    DVec3d v1, v2, d1, d2;
    ToParametricForm(v1, d1, line1);
    ToParametricForm(v2, d2, line2);

    // Let's assume that x_p = x0_2 + x1_2 * t is true and therefore t = (x_p - x0_2) / x1_2. Then y_p = y0_2 + y1_2 * t; and z_p = z0_2 + z1_2 * t; must also be equal.
    // In case of x1_2 being 0 we switch the x and y axises. In case of y1_2 being 0 we switch y and z axises. 
    // If z1_2 is also 0 then we have x2 = x0_2; y2 = y0_2; z2 = z0_2 => line2 is a point

    DPoint3d p = line1.point[0];
    double t;

    if (0 != d2.x)
        t = (p.x - v2.x) / d2.x;
    else if (0 != d2.y)
        t = (p.y - v2.y) / d2.y;
    else if (0 != d2.z)
        t = (p.z - v2.z) / d2.z;
    else // line 2 is a point
        {
        DPoint3d intersectionPoint = line2.point[0];

        DRange3d range1;
        line1.GetRange(range1);

        if (!IsPointContainedInRangeToTolerance(range1, intersectionPoint, BUILDING_TOLERANCE))
            return false;

        // Set intersection segment to be a single point segment of the intersection point
        intersectionSegment.point[0] = intersectionSegment.point[1] = intersectionPoint;
        return true;
        }

    // Check if t is correct for all parametric equasions
    // x_p = x0_2 + x1_2 * t; 
    // y_p = y0_2 + y1_2 * t;
    // z_p = z0_2 + z1_2 * t;

    DPoint3d line2p = GetPointFromParamtericForm(v2, d2, t);

    if (!p.AlmostEqual(line2p)) // lines do not coincide
        return false;

    // Lines coincide. Intersection segment is intersection of both lines ranges
    DRange3d range1, range2;
    line1.GetRange(range1);
    line2.GetRange(range2);

    DRange3d intersection = DRange3d::FromIntersection(range1, range2, true);

    if (intersection.IsEmpty() || intersection.IsNull()) // segments do not intersect
        return false;

    intersectionSegment.point[0] = intersection.low;
    intersectionSegment.point[1] = intersection.high;
    return true;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Haroldas.Vitunskas                  07/17
//---------------------------------------------------------------------------------------
bool GeometryUtils::IsPointContainedInRangeToTolerance(DRange3d range, DPoint3d point, double tolerance)
    {
    DPoint3d low = range.low, high = range.high;

    return point.x + tolerance >= low.x
        && point.y + tolerance >= low.y
        && point.z + tolerance >= low.z
        && point.x - tolerance <= high.x
        && point.y - tolerance <= high.y
        && point.z - tolerance <= high.z;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Haroldas.Vitunskas                  07/17
//---------------------------------------------------------------------------------------
void GeometryUtils::ExtractInnerOuterCurves(bvector<CurveVectorCPtr>& innerCurves, bvector<CurveVectorCPtr>& outerCurves, CurveVectorCPtr source)
    {
    if (CurveVector::BoundaryType::BOUNDARY_TYPE_Outer == source->GetBoundaryType())
        outerCurves.push_back(source);
    else if (CurveVector::BoundaryType::BOUNDARY_TYPE_Inner == source->GetBoundaryType())
        innerCurves.push_back(source);
    else
        {
        for (ICurvePrimitivePtr curvePrimitive : *source.get())
            {
            CurveVectorCPtr child = curvePrimitive->GetChildCurveVectorCP();
            if (!child.IsValid())
                return;

            ExtractInnerOuterCurves(innerCurves, outerCurves, child);
            }
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Haroldas.Vitunskas                  07/17
//---------------------------------------------------------------------------------------
bvector<DPoint3d> GeometryUtils::ExtractLineString(CurveVectorCPtr curveVector)
    {
    bvector<DPoint3d> const * cpLineString = (*curveVector)[0]->GetLineStringCP();
    if (nullptr == cpLineString)
        return bvector<DPoint3d>();

    return *cpLineString;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Haroldas.Vitunskas                  07/17
//---------------------------------------------------------------------------------------
bvector<bvector<DPoint3d>>::iterator findShortestPath(bvector<bvector<DPoint3d>>::iterator begin, bvector<bvector<DPoint3d>>::iterator end, const std::function <bool(bvector<DPoint3d>)>& predicate = [](bvector<DPoint3d> path) {return true; })
    {
    double minDistance = DBL_MAX;
    bvector<bvector<DPoint3d>>::iterator shortestPath = end;
    for (bvector<bvector<DPoint3d>>::iterator it = begin; it != end; ++it)
        {
        bvector<DPoint3d> path = *it;

        if (!predicate(path))
            continue;

        double distance = GeometryUtils::GetPathLength(path);
        if (distance < minDistance)
            {
            minDistance = distance;
            shortestPath = it;
            }
        }
    return shortestPath;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Haroldas.Vitunskas                  07/17
//---------------------------------------------------------------------------------------
bmap<bvector<DPoint3d>::iterator, bvector<bvector<DPoint3d>>::iterator> findAllLineStringsThatContainPoint(bvector<bvector<DPoint3d>>& lineStrings,  DPoint3d pointToFind)
    {
    // If point is not a line string point, but is on any of the lines, put it between the line points
    for (auto it = lineStrings.begin(); it != lineStrings.end(); ++it)
        {
        size_t i = std::distance(lineStrings.begin(), it);
        if (it->end() == std::find_if(it->begin(), it->end(), [&](DPoint3d point) { return point.AlmostEqual(pointToFind); }))
            {
            for (auto itPoint = it->begin(); itPoint != it->end(); ++itPoint)
                {
                DPoint3d point = *itPoint, pointAfter;
                if (it->end() == itPoint + 1)
                    pointAfter = *(it->begin());
                else
                    pointAfter = *(itPoint + 1);

                DSegment3d line = DSegment3d::From(point, pointAfter);

                if (!GeometryUtils::IsPointOnLine(pointToFind, line))
                    continue;

                it->insert(itPoint + 1, pointToFind);
                break;
                }
            }
        
        it = lineStrings.begin() + i; // revalidate iterator
        }

    bmap<bvector<DPoint3d>::iterator, bvector<bvector<DPoint3d>>::iterator> result;
    for (auto it = lineStrings.begin(); it != lineStrings.end(); ++it)
        {
        auto itLineString = std::find_if(it->begin(), it->end(), [&](DPoint3d point) {return point.AlmostEqual(pointToFind); });
        if (it->end() != itLineString)
            result[itLineString] = it;
        }

    return result;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Haroldas.Vitunskas                  07/17
//---------------------------------------------------------------------------------------
double GeometryUtils::DistanceFromPointToLine(DSegment3d line, DPoint3d point)
    {
    if (line.IsAlmostSinglePoint())
        return line.point[0].Distance(point);

    DPoint3d projectedPoint;
    double closestParam;
    line.ProjectPointBounded(projectedPoint, closestParam, point, false, false);

    return projectedPoint.Distance(point);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Haroldas.Vitunskas                  07/17
//---------------------------------------------------------------------------------------
DSegment3d findClosestLineStringSegmentThatIntersectsWithPath(bvector<bvector<DPoint3d>> lineStrings, DSegment3d path)
    {
    double minDistance = DBL_MAX;
    DSegment3d closestSegment = DSegment3d::FromZero();

    for (bvector<DPoint3d> lineString : lineStrings)
        {
        if (!lineString.front().AlmostEqual(lineString.back()))
            lineString.push_back(lineString.front());

        for (size_t i = 0; i < lineString.size() - 1; ++i)
            {
            DSegment3d segment = DSegment3d::From(lineString[i], lineString[i + 1]), intersection;
            if (!GeometryUtils::CheckIfTwoLinesIntersect(intersection, path, segment) ||
                intersection.point[0].AlmostEqual(path.point[0]) || intersection.point[1].AlmostEqual(path.point[0]) ||
                intersection.point[0].AlmostEqual(path.point[1]) || intersection.point[1].AlmostEqual(path.point[1]))
                continue;

            double distance = GeometryUtils::DistanceFromPointToLine(segment, path.point[0]);
            if (distance < minDistance)
                {
                minDistance = distance;
                closestSegment = segment;
                }
            }
        }

    return closestSegment;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Haroldas.Vitunskas                  07/17
//---------------------------------------------------------------------------------------
int alternatePathsIfPointsNotContained(bvector<bvector<DPoint3d>>& paths, bvector<bvector<DPoint3d>>::iterator itCurrent, bvector<DPoint3d> pointsToAdd)
    {
    // Remove points already in some path
    auto removeBegin = std::remove_if(pointsToAdd.begin(), pointsToAdd.end(), [&](DPoint3d point) 
        {
        for (bvector<DPoint3d> path : paths)
            {
            if (path.end() != std::find_if(path.begin(), path.end(), [&](DPoint3d otherPoint) {return otherPoint.AlmostEqual(point); }))
                return true;
            }
        return false;
        });

    pointsToAdd.erase(removeBegin, pointsToAdd.end());

    int pointsAdded = 0;

    // Add one path to the current path, for every other point, create an alternative and insert it to the paths vector
    for (size_t i = 0; i < pointsToAdd.size(); ++i, ++pointsAdded)
        {
        if (pointsToAdd.size() - 1 == i) // Add the last point to the current path
            {
            itCurrent->push_back(pointsToAdd[i]);
            }
        else
            {
            bvector<DPoint3d> currentCopy = *itCurrent;
            currentCopy.push_back(pointsToAdd[i]);
            paths.push_back(currentCopy);
            }
        }

    return pointsAdded;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Haroldas.Vitunskas                  07/17
//---------------------------------------------------------------------------------------
bool GeometryUtils::CheckIfLineIntersectsWithLineString(bvector<DPoint3d> lineString, DSegment3d line)
    {
    if (!lineString.front().AlmostEqual(lineString.back()))
        lineString.push_back(lineString.front());

    for (size_t i = 0; i < lineString.size() - 1; ++i)
        {
        DSegment3d line2 = DSegment3d::From(lineString[i], lineString[i + 1]), intersection;
        if (!GeometryUtils::CheckIfTwoLinesIntersect(intersection, line, line2) ||
            intersection.point[0].AlmostEqual(line.point[0]) || intersection.point[1].AlmostEqual(line.point[0]) ||
            intersection.point[0].AlmostEqual(line.point[1]) || intersection.point[1].AlmostEqual(line.point[1]))
            continue;

        return true;
        }
    return false;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Haroldas.Vitunskas                  07/17
//---------------------------------------------------------------------------------------
void removeUnneededPoints(bvector<bvector<DPoint3d>>& lineStrings)
    {
    for (auto itLineString = lineStrings.begin(); itLineString != lineStrings.end(); ++itLineString)
        {
        size_t position = std::distance(lineStrings.begin(), itLineString);
        auto itRemove = std::remove_if(itLineString->begin(), itLineString->end(), [&](DPoint3d point)
            {
            auto itPoint = std::find_if(itLineString->begin(), itLineString->end(), [&](DPoint3d point2) {return point.AlmostEqual(point2); });
            if (itLineString->end() == itPoint)
                return false;

            auto itPointBefore = (itLineString->begin() == itPoint) ? itLineString->end() - 1 : itPoint - 1;
            auto itPointAfter = (itLineString->end() - 1 == itPoint) ? itLineString->begin() : itPoint + 1;

            return GeometryUtils::IsPointOnLine(*itPoint, DSegment3d::From(*itPointBefore, *itPointAfter));
            });

        itLineString->erase(itRemove, itLineString->end());

        itLineString = lineStrings.begin() + position; // revalidate iterator
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Haroldas.Vitunskas                  07/17
//---------------------------------------------------------------------------------------
void getCorrectionPointsByClosestIntersection(bvector<DPoint3d>& pointsToAdd, bvector<bvector<DPoint3d>> lineStrings, CurveVectorCPtr walkableShape, DPoint3d source, DPoint3d target)
    {
    DSegment3d closestIntersectingSegment = findClosestLineStringSegmentThatIntersectsWithPath(lineStrings, DSegment3d::From(source, target));

    if (!closestIntersectingSegment.IsAlmostSinglePoint())
        {
        if (GeometryUtils::CheckIfLineIsContainedInPolygonArea(walkableShape, DSegment3d::From(source, closestIntersectingSegment.point[0])))
            pointsToAdd.push_back(closestIntersectingSegment.point[0]);

        if (GeometryUtils::CheckIfLineIsContainedInPolygonArea(walkableShape, DSegment3d::From(source, closestIntersectingSegment.point[1])))
            pointsToAdd.push_back(closestIntersectingSegment.point[1]);
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Haroldas.Vitunskas                  07/17
//---------------------------------------------------------------------------------------
void getCorrectionPointsByPointPositionInShapes(bvector<DPoint3d>& pointsToAdd, bvector<bvector<DPoint3d>> lineStrings, DPoint3d source, DPoint3d target)
    {
    bmap<bvector<DPoint3d>::iterator, bvector<bvector<DPoint3d>>::iterator> containingLineStrings = findAllLineStringsThatContainPoint(lineStrings, source);

    for (auto pair : containingLineStrings)
        {
        bvector<DPoint3d> lineString = *pair.second;
        if (lineString.front().AlmostEqual(lineString.back()))
            pair.second->pop_back();

        if (!GeometryUtils::CheckIfLineIntersectsWithLineString(lineString, DSegment3d::From(source, target)))
            continue;

        DPoint3d pointBefore = *((pair.first == pair.second->begin()) ? pair.second->end() - 1 : pair.first - 1);
        DPoint3d pointAfter = *((pair.first + 1 == pair.second->end()) ? pair.second->begin() : pair.first + 1);

        pointsToAdd.push_back(pointBefore);
        pointsToAdd.push_back(pointAfter);
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Haroldas.Vitunskas                  07/17
//---------------------------------------------------------------------------------------
void findPathsNotIntersectingWithLineStringsFromPointToTarget(bvector<bvector<DPoint3d>>& paths, CurveVectorCPtr walkableShape, bvector<bvector<DPoint3d>> lineStrings, DPoint3d target)
    {
    // Find point with lowest distance
    auto itShortest = findShortestPath(paths.begin(), paths.end(), [&](bvector<DPoint3d> path) {return !path.back().AlmostEqual(target); });

    if (paths.end() == itShortest)
        return; // All paths are complete

    bvector<DPoint3d> path = *itShortest;
    DPoint3d lastPoint = path.back();

    int pointsAdded = 0;

    DSegment3d directPath = DSegment3d::From(lastPoint, target);
    if (GeometryUtils::CheckIfLineIsContainedInPolygonArea(walkableShape, directPath))
        {
        itShortest->push_back(target);
        pointsAdded = 1;
        }
    else
        {
        // Check if direct path from X to Y intersects with any line strings
        bvector<DPoint3d> pointsToAdd;
        getCorrectionPointsByClosestIntersection(pointsToAdd, lineStrings, walkableShape, lastPoint, target);

        if (0 == pointsToAdd.size())
            {
            // try adding neighbor points of the line string that point is on
            getCorrectionPointsByPointPositionInShapes(pointsToAdd, lineStrings, lastPoint, target);
            }

        pointsAdded = alternatePathsIfPointsNotContained(paths, itShortest, pointsToAdd);
        }

    if (0 == pointsAdded) // no points could have been added. This path can never reach target. 
        paths.erase(itShortest); // Will prevent infinite recursion

    findPathsNotIntersectingWithLineStringsFromPointToTarget(paths, walkableShape, lineStrings, target);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Haroldas.Vitunskas                  07/17
//---------------------------------------------------------------------------------------
void removeInnerPoints(bvector<bvector<DPoint3d>>& paths, CurveVectorCPtr walkableShape)
    {
    bool pointRemoved = true;
    while (pointRemoved)
        {
        pointRemoved = false;
        for (auto itCurrentPath = paths.begin(); itCurrentPath != paths.end(); ++itCurrentPath)
            {
            for (size_t i = 1; i < itCurrentPath->size() - 1; ++i)
                {
                DSegment3d line = DSegment3d::From((*itCurrentPath)[i - 1], (*itCurrentPath)[i + 1]);

                if (!GeometryUtils::CheckIfLineIsContainedInPolygonArea(walkableShape, line))
                    continue;
                    
                itCurrentPath->erase(itCurrentPath->begin() + i);
                --i; // prevent skipping points
                pointRemoved = true;
                }
            }
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Haroldas.Vitunskas                  07/17
//---------------------------------------------------------------------------------------
DPoint3d adjustToClosestLineString(bvector<bvector<DPoint3d>> lineStrings, DPoint3d point, CurveVector::InOutClassification positionToAdjustFrom)
    {
    double minDistance = DBL_MAX;
    DPoint3d adjustedPoint = point;
    for (bvector<DPoint3d> lineString : lineStrings)
        {
        CurveVectorPtr lineStringCurve = CurveVector::CreateLinear(lineString, CurveVector::BOUNDARY_TYPE_Outer);
        if (positionToAdjustFrom == lineStringCurve->PointInOnOutXY(point))
            {
            CurveLocationDetail hit;
            lineStringCurve->ClosestPointBounded(point, hit);
            if (point.Distance(hit.point) < minDistance)
                {
                adjustedPoint = hit.point;
                minDistance = adjustedPoint.Distance(point);
                }
            }
        else
            {
            adjustedPoint = point;
            minDistance = point.Distance(adjustedPoint);
            }
        }

    return adjustedPoint;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Haroldas.Vitunskas                  07/17
//---------------------------------------------------------------------------------------
DPoint3d adjustIfPositionNotCorrect(DPoint3d point, bvector<bvector<DPoint3d>> innerLineStrings, bvector<bvector<DPoint3d>> outerLineStrings)
    {
    DPoint3d adjusted = adjustToClosestLineString(outerLineStrings, point, CurveVector::INOUT_Out);
    if (adjusted.AlmostEqual(point))
        adjusted = adjustToClosestLineString(innerLineStrings, point, CurveVector::INOUT_In);

    return adjusted;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Haroldas.Vitunskas                  07/17
//---------------------------------------------------------------------------------------
bool checkIfPointsAreInTheSameSubarea(DPoint3d point1, DPoint3d point2, bvector<bvector<DPoint3d>> subareas)
    {
    for (bvector<DPoint3d> lineString : subareas)
        {
        CurveVectorPtr curve = CurveVector::CreateLinear(lineString, CurveVector::BOUNDARY_TYPE_Outer);
        CurveVector::InOutClassification pos1 = curve->PointInOnOutXY(point1), pos2 = curve->PointInOnOutXY(point2);

        if (pos1 != pos2 && (CurveVector::InOutClassification::INOUT_Out == pos1 || CurveVector::InOutClassification::INOUT_Out == pos2))
            return false; 
        }

    return true;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Haroldas.Vitunskas                  07/17
//---------------------------------------------------------------------------------------
Transform getTransformToSegmentAsXAxisXY(DSegment3d segment)
    {
    DPoint3d newOrigin = segment.point[0];
    Transform translation = Transform::FromRowValues(1, 0, 0, newOrigin.x,
                                                     0, 1, 0, newOrigin.y,
                                                     0, 0, 1, 0);

    double scaleFactor = segment.Length();
    Transform scale = Transform::FromRowValues(scaleFactor, 0, 0, 0,
                                               0, scaleFactor, 0, 0,
                                               0, 0, 1, 0);

    double rotAngle = DVec3d::From(1.0, 0.0, 0.0).AngleToXY(DVec3d::FromStartEnd(segment.point[0], segment.point[1]));
    double cosTheta = std::cos(rotAngle);
    double sinTheta = std::sin(rotAngle); 

    RotMatrix rotMatrix = RotMatrix::FromRowValues(cosTheta, -sinTheta, 0,
                                                   sinTheta, cosTheta, 0,
                                                   0, 0, 1);
    Transform rotation = Transform::From(rotMatrix);

    Transform transform = Transform::FromProduct(translation, scale, rotation), inverse;
    inverse.InverseOf(transform);
    return inverse;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Haroldas.Vitunskas                  07/17
//---------------------------------------------------------------------------------------
void GeometryUtils::ExtractInnerOuterLineStrings(bvector<bvector<DPoint3d>>& innerLineStrings, bvector<bvector<DPoint3d>>& outerLineStrings, CurveVectorCPtr shape)
    {
    bvector<CurveVectorCPtr> innerCurveVectors, outerCurveVectors;
    GeometryUtils::ExtractInnerOuterCurves(innerCurveVectors, outerCurveVectors, shape);

    innerLineStrings = bvector<bvector<DPoint3d>>(innerCurveVectors.size());
    outerLineStrings = bvector<bvector<DPoint3d>>(outerCurveVectors.size());

    auto itInnerCurves = innerCurveVectors.begin();
    auto itOuterCurves = outerCurveVectors.begin();

    std::generate(innerLineStrings.begin(), innerLineStrings.end(), [&]() {return GeometryUtils::ExtractLineString(*(itInnerCurves++)); });
    std::generate(outerLineStrings.begin(), outerLineStrings.end(), [&]() {return GeometryUtils::ExtractLineString(*(itOuterCurves++)); });

    // Remove failed linestrings
    auto itRemoveInner = std::remove_if(innerLineStrings.begin(), innerLineStrings.end(), [](bvector<DPoint3d> lineString) {return lineString.empty(); });
    auto itRemoveOuter = std::remove_if(outerLineStrings.begin(), outerLineStrings.end(), [](bvector<DPoint3d> lineString) {return lineString.empty(); });

    innerLineStrings.erase(itRemoveInner, innerLineStrings.end());
    outerLineStrings.erase(itRemoveOuter, outerLineStrings.end());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Haroldas.Vitunskas                  07/17
//---------------------------------------------------------------------------------------
bvector<DPoint3d> findIntersectionsWithXAxis(bvector<DPoint3d> lineString)
    {
    bvector<DPoint3d> intersectionPoints;

    if (lineString.front().AlmostEqual(lineString.back()))
        lineString.pop_back();

    // Intersection is true if for any two points P_i (x_i, y_i) and P_i+1 (x_i+1, y_i+1), (y_i > 0 != y_i+1 > 0).

    for (DPoint3d point : lineString)
        {
        if (!GeometryUtils::AlmostEqual(point.y, 0, CONTAINEMENT_TOLERANCE))
            continue;

        intersectionPoints.push_back(point);
        }

    lineString.push_back(lineString.front());
    for (size_t i = 0; i < lineString.size() - 1; ++i)
        {
        DPoint3d point = lineString[i], pointAfter = lineString[i + 1];
        if (GeometryUtils::AlmostEqual(point.y, 0, CONTAINEMENT_TOLERANCE) || GeometryUtils::AlmostEqual(pointAfter.y, 0, CONTAINEMENT_TOLERANCE))
            continue; // If either end point is 0, no more intersections for this edge are possible

        if (point.y > 0 != pointAfter.y > 0)
            {
            // Find intersection point
            // vec v(a, b, c) is such that p + v = pPlus
            // p + kv = (x, 0, z), k in (0, 1)
            // k is such that p.y + k * b = 0 => k = -p.y / b, if b is non-zero.

            DVec3d v = DVec3d::FromStartEnd(point, pointAfter);
            BeAssert(0 != v.y); //  b can't be 0, because the if line intersects X axis and no end point is 0, line can't be parallel to X axis

            DPoint3d intersectionPoint = DPoint3d::FromSumOf(point, v, -point.y / v.y);
            intersectionPoints.push_back(intersectionPoint);
            }
        }
    return intersectionPoints;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Haroldas.Vitunskas                  07/17
//---------------------------------------------------------------------------------------
DSegment3d shrinkFromBothEnds(DSegment3d line, double magnitude)
    {
    if (2 * magnitude > line.Length())
        return line; // no changes

    DPoint3d offseted0 = line.point[0], offseted1 = line.point[1];

    DVec3d insideLine = DVec3d::FromStartEnd(offseted0, offseted1);
    insideLine.ScaleToLength(magnitude);

    offseted0.Add(insideLine);
    offseted1.Subtract(insideLine);

    return DSegment3d::From(offseted0, offseted1);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Haroldas.Vitunskas                  07/17
//---------------------------------------------------------------------------------------
bool checkIfLinePositionInTransformedLineStringsIsCorrect(bvector<bvector<DPoint3d>> lineStrings, DSegment3d originalLine, Transform coordinateSystem, CurveVector::InOutClassification expectedPosition, bool forAll)
    {
    BeAssert(CurveVector::INOUT_On != expectedPosition); // position On is not supported for expected position

    CurveVector::InOutClassification badPosition = (CurveVector::INOUT_Out == expectedPosition) ? CurveVector::INOUT_In : CurveVector::INOUT_Out;

    bool position = (forAll) ? true : false;
    for (bvector<DPoint3d> lineString : lineStrings)
        {
        bool currentPositionCorrect = true;
        // Find all intersections of polygon edges with X axis.
        bvector<DPoint3d> intersectionPoints = findIntersectionsWithXAxis(lineString);

        // Analysis of intersection points:
        //  If intersection any point P(x, y) is such that 0 < x < 1, return false (line intersects with shape)
        // If any line end point is also an intersection, additional analysis is needed. We'll check if deviated end points to inside of the line are inside/outside the shape
        if (std::count_if(intersectionPoints.begin(), intersectionPoints.end(), [](DPoint3d point) {return point.x - CONTAINEMENT_TOLERANCE > 0 && point.x + CONTAINEMENT_TOLERANCE < 1; }) > 0)
            return false;

        if (intersectionPoints.end() != std::find_if(intersectionPoints.begin(), intersectionPoints.end(), [](DPoint3d point) {return GeometryUtils::AlmostEqual(point.x, 0, CONTAINEMENT_TOLERANCE) || GeometryUtils::AlmostEqual(point.x, 1, CONTAINEMENT_TOLERANCE); }))
            {
            DSegment3d shrinked = shrinkFromBothEnds(originalLine, CONTAINEMENT_TOLERANCE);
            coordinateSystem.Multiply(shrinked); // apply the coordinate system

            CurveVectorPtr outer = CurveVector::CreateLinear(lineString, CurveVector::BOUNDARY_TYPE_Outer);
            currentPositionCorrect = (badPosition != outer->PointInOnOutXY(shrinked.point[0]) && badPosition != outer->PointInOnOutXY(shrinked.point[1]));
            }

        position = (forAll) ? position && currentPositionCorrect : position || currentPositionCorrect;

        if (forAll && !position)
            return false;
        }

    return position;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Haroldas.Vitunskas                  07/17
//---------------------------------------------------------------------------------------
bool GeometryUtils::CheckIfLineIsContainedInPolygonArea(CurveVectorCPtr area, DSegment3d line)
    {
    if (CurveVector::INOUT_Out == area->PointInOnOutXY(line.point[0]) || CurveVector::INOUT_Out == area->PointInOnOutXY(line.point[1]))
        return false; // If either end point is outside the polygon, the line is not contained

     // 1. Transform line and curve vector in such way that line would be on X axis: {(x0, y0), (x1, y1)} = T{(0.0, 0.0), (1, 0)} and apply the transformation for the shape
    Transform newCoordinateSystem = getTransformToSegmentAsXAxisXY(line);

    CurveVectorPtr transformedArea = area->Clone();
    transformedArea->TransformInPlace(newCoordinateSystem);

    // Convert shape into inner/outer line strings
    bvector<bvector<DPoint3d>> innerLineStrings, outerLineStrings;
    ExtractInnerOuterLineStrings(innerLineStrings, outerLineStrings, transformedArea);

    // For outer line strings, line should be inside it and for all inner line strings, line should be out of it
    bool insideOuters = checkIfLinePositionInTransformedLineStringsIsCorrect(outerLineStrings, line, newCoordinateSystem, CurveVector::INOUT_In, false);
    bool outsideInners = checkIfLinePositionInTransformedLineStringsIsCorrect(innerLineStrings, line, newCoordinateSystem, CurveVector::INOUT_Out, true);

    return insideOuters && outsideInners;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Haroldas.Vitunskas                  07/17
//---------------------------------------------------------------------------------------
void restoreOriginalPathEnds(bvector<bvector<DPoint3d>>& paths, DPoint3d originalSource, DPoint3d originalDestination, bvector<bvector<DPoint3d>> originalLineStrings)
    {
    for (bvector<bvector<DPoint3d>>::iterator itPath = paths.begin(); itPath != paths.end(); ++itPath)
        {
        if (findClosestLineStringSegmentThatIntersectsWithPath(originalLineStrings, DSegment3d::From(originalSource, *(itPath->begin() + 1))).IsAlmostSinglePoint())
            (*itPath)[0] = originalSource; // if no intersection between direct path from source/destination to point before/after it and any original line string, replace adjusted point with point
        else if (findClosestLineStringSegmentThatIntersectsWithPath(originalLineStrings, DSegment3d::From(originalSource, itPath->front())).IsAlmostSinglePoint())
            itPath->insert(itPath->begin(), originalSource); // else if no intersections for path between point and adjusted point, keep the adjusted point for path
        else // path is invalid, it should be deleted
            {
            *itPath = bvector<DPoint3d>();
            continue;
            }

        if (findClosestLineStringSegmentThatIntersectsWithPath(originalLineStrings, DSegment3d::From(originalDestination, *(itPath->end() - 2))).IsAlmostSinglePoint())
            (*itPath)[itPath->size() - 1] = originalSource;
        else if (findClosestLineStringSegmentThatIntersectsWithPath(originalLineStrings, DSegment3d::From(originalDestination, itPath->back())).IsAlmostSinglePoint())
            itPath->push_back(originalSource);
        else
            *itPath = bvector<DPoint3d>();
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Haroldas.Vitunskas                  07/17
//---------------------------------------------------------------------------------------
void extractOriginalAndDeviationLineStrings(bvector<bvector<DPoint3d>>& originalLineStrings, bvector<bvector<DPoint3d>>& innerDeviationLineStrings, bvector<bvector<DPoint3d>>& outerDeviationLineStrings, CurveVectorCPtr originalCurve, CurveVectorCPtr deviatedCurve)
    {
    bvector<bvector<DPoint3d>> innerOriginal, outerOriginal;
    GeometryUtils::ExtractInnerOuterLineStrings(innerOriginal, outerOriginal, originalCurve);
    originalLineStrings = innerOriginal;
    originalLineStrings.insert(originalLineStrings.end(), outerOriginal.begin(), outerOriginal.end());

    GeometryUtils::ExtractInnerOuterLineStrings(innerDeviationLineStrings, outerDeviationLineStrings, deviatedCurve);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Haroldas.Vitunskas                  07/17
//---------------------------------------------------------------------------------------
BentleyStatus GeometryUtils::FindShortestPathBetweenPointsInCurveVector(bvector<DPoint3d>& pathLineString, CurveVectorPtr curveVector, DPoint3d source, DPoint3d destination)
    {
    CurveVectorCPtr walkableShape = GeometryUtils::OffsetCurveInnerOuterChildren(curveVector, EGRESS_CORRECTION, -EGRESS_CORRECTION, true);
    if (!walkableShape.IsValid())
        return BentleyStatus::ERROR;

    // Method:
    // 1.   Create deviated line strings by extracting them from walkable area
    bvector<bvector<DPoint3d>> innerDeviationLineStrings, outerDeviationLineStrings, originalLineStrings;
    extractOriginalAndDeviationLineStrings(originalLineStrings, innerDeviationLineStrings, outerDeviationLineStrings, curveVector, walkableShape);

    // If source and destination points are out of deviated outer line string, use their projections on the line string
    DPoint3d adjustedSource = adjustIfPositionNotCorrect(source, innerDeviationLineStrings, outerDeviationLineStrings);
    DPoint3d adjustedDestination = adjustIfPositionNotCorrect(destination, innerDeviationLineStrings, outerDeviationLineStrings);

    BeAssert(CurveVector::InOutClassification::INOUT_Out != curveVector->PointInOnOutXY(adjustedSource) && CurveVector::InOutClassification::INOUT_Out != curveVector->PointInOnOutXY(adjustedDestination));

    if (!checkIfPointsAreInTheSameSubarea(adjustedSource, adjustedDestination, outerDeviationLineStrings))
        return BentleyStatus::ERROR; // destination and source can't be in different areas

    // 2.   Try finding path from X to Y while checking for intersections
    bvector<bvector<DPoint3d>> allDeviationLineStrings;
    allDeviationLineStrings.insert(allDeviationLineStrings.end(), innerDeviationLineStrings.begin(), innerDeviationLineStrings.end());
    allDeviationLineStrings.insert(allDeviationLineStrings.end(), outerDeviationLineStrings.begin(), outerDeviationLineStrings.end());

    bvector<bvector<DPoint3d>> paths = { { adjustedSource } };
    findPathsNotIntersectingWithLineStringsFromPointToTarget(paths, walkableShape, allDeviationLineStrings, adjustedDestination);

    // 3.   Remove any inner points in paths. Inner point P is such that if you take points P- and P+ (where P- is point before P and P+ is point after P), 
    //      line {P-, P+} doesn't intersect any line strings
    removeInnerPoints(paths, walkableShape);

    // 4.   Find the path with shortest length

    // 4.1. Switch adjusted points to actual ones and check for any intersections with any original linestrings 
    //      for first and last lines of path
    restoreOriginalPathEnds(paths, source, destination, originalLineStrings);

    // 4.2. Remove invalid paths
    bvector<bvector<DPoint3d>>::iterator itRemove = std::remove_if(paths.begin(), paths.end(), [&](bvector<DPoint3d> path) { return 0 == path.size(); });
    paths.erase(itRemove, paths.end());

    // 4.3. Select the shortest path
    bvector<bvector<DPoint3d>>::iterator itShortest = findShortestPath(paths.begin(), paths.end());
    if (paths.end() != itShortest)
        {
        pathLineString = *itShortest;
        return BentleyStatus::SUCCESS;
        }

    return BentleyStatus::ERROR;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Haroldas.Vitunskas                  07/17
//---------------------------------------------------------------------------------------
double GeometryUtils::GetPathLength(bvector<DPoint3d> path)
    {
    double length = 0;
    for (size_t i = 0; i < path.size() - 1; ++i)
        {
        length += path[i].Distance(path[i + 1]);
        }

    return length;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Haroldas.Vitunskas                  07/17
//---------------------------------------------------------------------------------------
double GeometryUtils::GetCorrectedPathLength(DPoint3d source, DPoint3d target, bvector<DPoint3d> correctionPoints)
    {
    if (correctionPoints.size() > 0)
        {
        DPoint3d first = correctionPoints.front();
        DPoint3d last = correctionPoints.back();

        return source.Distance(first) + GetPathLength(correctionPoints) + last.Distance(target);
        }
    else
        return source.Distance(target);
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas Butkus                09/2017
//---------------+---------------+---------------+---------------+---------------+------
void GeometryUtils::AddVertex(CurveVectorR cv, DPoint3d const& vertex)
    {
    CurveLocationDetail locationOnCurveVector;
    cv.ClosestPointBounded(vertex, locationOnCurveVector);

    CurveVectorPtr parts = cv.GenerateAllParts(locationOnCurveVector.componentIndex, locationOnCurveVector.fraction, locationOnCurveVector.componentIndex, locationOnCurveVector.fraction);
    if (parts->size() == 2)
        {
        cv.clear();
        cv.AddPrimitives(*parts->at(1)->GetChildCurveVectorCP());
        cv.AddPrimitives(*parts->at(0)->GetChildCurveVectorCP());
        cv.ConsolidateAdjacentPrimitives(false);
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Haroldas.Vitunskas              09/2017
//--------------+---------------+---------------+---------------+---------------+--------
void GeometryUtils::LineStringAsWeightedCurve(CurveVectorPtr & curve, double weight)
    {
    bvector<DPoint3d> lineString = ExtractLineString(curve);
    if (lineString.size() < 2)
        return;

    for (auto itPoint = lineString.end() - 1; itPoint != lineString.begin() - 1; --itPoint)
        lineString.push_back(*itPoint); // {A, B, C, D } -> { A, B, C, D, D, C, B, A }
    
    bvector<DPoint3d> newLineString;
    for (auto itPoint = lineString.begin(); itPoint != lineString.end(); ++itPoint)
        {
        auto itBefore = itPoint != lineString.begin() ? itPoint - 1 : lineString.end() - 1;
        auto itAfter = itPoint != lineString.end() - 1 ? itPoint + 1 : lineString.begin();

        DVec3d dirBefore = DVec3d::FromStartEnd(*itPoint, *itBefore);
        DVec3d dirAfter = DVec3d::FromStartEnd(*itPoint, *itAfter);

        DVec3d offsetVec;
        if (0 == dirBefore.Magnitude() || 0 == dirAfter.Magnitude())
            {
            double rotationAngle = msGeomConst_pi / 2;
            if (0 == dirBefore.Magnitude())
                {
                offsetVec = dirAfter;
                rotationAngle *= -1;
                }
            else
                {
                offsetVec = dirBefore;
                }
            BeAssert(0 != offsetVec.Magnitude()); // In theory, this should be null only if the original curve vector has repeating points.

            offsetVec.RotateXY(rotationAngle);
            offsetVec.ScaleToLength(weight / 2);
            }
        else
            {
            double rotAngle = dirBefore.AngleToXY(dirAfter) / 2;
            offsetVec = DVec3d::FromRotateVectorAroundVector(dirBefore, DVec3d::From(0, 0, 1), Angle::FromRadians(rotAngle));
            offsetVec.ScaleToLength((weight / 2) / std::sin(rotAngle));
            }
        DPoint3d toInsert = *itPoint;
        toInsert.Add(offsetVec);
        newLineString.push_back(toInsert);
        }

    curve = CurveVector::CreateLinear(newLineString, CurveVector::BoundaryType::BOUNDARY_TYPE_Outer);
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                10/2017
//---------------+---------------+---------------+---------------+---------------+------
bool GeometryUtils::AllPointsInOrOnShape
(
    bvector<DPoint3d> const& shapePoints,
    bvector<DPoint3d> const& pointsToTest
)
    {
    CurveVectorPtr shape = CurveVector::CreateLinear(shapePoints, CurveVector::BoundaryType::BOUNDARY_TYPE_Outer);

    for (DPoint3d const& p : pointsToTest)
        {
        CurveVector::InOutClassification inout = shape->PointInOnOutXY(p);
        if (CurveVector::InOutClassification::INOUT_Unknown == inout)
            {
            return false;
            }
        else if(CurveVector::InOutClassification::INOUT_Out == inout)
            {
            DPoint3d curveOrRegionPoint;
            shape->ClosestCurveOrRegionPoint(p, curveOrRegionPoint);

            double distance = p.Distance(curveOrRegionPoint);
            if (!DoubleOps::AlmostEqual(distance, 0))
                {
                return false;
                }
            }
        }

    return true;
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                10/2017
//---------------+---------------+---------------+---------------+---------------+------
bvector<DPoint3d> GeometryUtils::GetConvexPolygonVertices
(
    size_t n, 
    double ccRadius, 
    DPoint3d ccCenter, 
    bool keepArea
)
    {
    BeAssert(n >= 3);
    
    if (keepArea)
        {
        // compensate for the lost area by increasing radius
        ccRadius = (sqrt(Angle::TwoPi())*ccRadius) / (sqrt(n * sin(Angle::TwoPi() / n)));
        }

    bvector<DPoint3d> vertices(n);
    double step = Angle::TwoPi() / n;
    for (int i = 0; i < n; i++)
        {
        vertices[i] = DPoint3d::From(ccCenter.x + ccRadius * cos(i * step), ccCenter.y + ccRadius * sin(i * step), 0.0);
        }

    return vertices;
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                10/2017
//---------------+---------------+---------------+---------------+---------------+------
bvector<DPoint3d> GeometryUtils::GetCircularArcPoints
(
    DEllipse3d arc,
    double maxEdgeLength,
    bool keepSectorArea
)
    {
    if (!arc.IsCircular())
        return bvector<DPoint3d>();

    double radius = arc.vector0.Magnitude();

    // maximum maxEdgeLength cannot be more than the diameter of the circle.
    maxEdgeLength = std::min(2 * radius, maxEdgeLength);

    double maxAngle = acos(1 - (maxEdgeLength * maxEdgeLength) / (2 * radius * radius));

    double sweep = abs(arc.sweep);

    size_t nEdges = floor(sweep / maxAngle);
    if (sweep - nEdges * maxAngle > 0)
        nEdges += 1;

    if (arc.IsFullEllipse())
        {
        if (nEdges < 3)
            nEdges = 3;

        // GetConvexPolygonVertices is a bit faster for a full ellipse
        bvector<DPoint3d> points = GetConvexPolygonVertices(nEdges, radius, arc.center, keepSectorArea);
        points.push_back(points.front());
        return points;
        }

    size_t nVertices = nEdges + 1;
    bvector<DPoint3d> points(nVertices);

    double step = sweep / nEdges;
    if (keepSectorArea)
        radius = radius * sqrt(step / sin(step));

    arc.vector0.ScaleToLength(radius);
    arc.vector90.ScaleToLength(radius);

    for (size_t i = 0; i < nVertices; i++)
        {
        points[i] = arc.FractionToPoint(step * i / sweep);
        }

    return points;
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                10/2017
//---------------+---------------+---------------+---------------+---------------+------
bvector<DPoint3d> GeometryUtils::GetCurvePrimitivePoints
(
    ICurvePrimitiveCPtr curvePrimitive,
    double maxEdgeLength,
    bool keepSectorArea
)
    {
    if (!curvePrimitive.IsValid())
        return bvector<DPoint3d>();

    BeAssert(maxEdgeLength >= 0);

    if (curvePrimitive->GetCurvePrimitiveType() == ICurvePrimitive::CurvePrimitiveType::CURVE_PRIMITIVE_TYPE_Arc)
        {
        DEllipse3d arc = *curvePrimitive->GetArcCP();
        if (arc.IsCircular())
            return GetCircularArcPoints(arc, maxEdgeLength, keepSectorArea);
        }

    if (curvePrimitive->GetCurvePrimitiveType() == ICurvePrimitive::CurvePrimitiveType::CURVE_PRIMITIVE_TYPE_Line)
        {
        DSegment3d line = *curvePrimitive->GetLineCP();
        DPoint3d start, end;
        line.GetEndPoints(start, end);
        return {start, end};
        }

    if (curvePrimitive->GetCurvePrimitiveType() == ICurvePrimitive::CurvePrimitiveType::CURVE_PRIMITIVE_TYPE_LineString)
        {
        bvector<DPoint3d> points = *curvePrimitive->GetLineStringCP();
        return points;
        }

    IFacetOptionsPtr  facetOptions = IFacetOptions::Create();
    facetOptions->SetEdgeChainsRequired(true);
    facetOptions->SetMaxEdgeLength(maxEdgeLength);

    bvector<DPoint3d> points;
    curvePrimitive->AddStrokes(points, *facetOptions);
    return points;
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                10/2017
//---------------+---------------+---------------+---------------+---------------+------
bvector<DPoint3d> GeometryUtils::GetCurveVectorPoints
(
    CurveVectorCPtr curveVectorPtr,
    double maxEdgeLength,
    bool keepSectorArea
)
    {
    if (!curveVectorPtr.IsValid())
        {
        return bvector<DPoint3d>();
        }

    CurveVectorCPtr target;
    switch (curveVectorPtr->GetBoundaryType())
        {
        case CurveVector::BoundaryType::BOUNDARY_TYPE_None:
            return bvector<DPoint3d>();
            break;
        case CurveVector::BoundaryType::BOUNDARY_TYPE_Outer:
        case CurveVector::BoundaryType::BOUNDARY_TYPE_Inner:
        case CurveVector::BoundaryType::BOUNDARY_TYPE_Open:
            target = curveVectorPtr;
            break;
        default:
            target = curveVectorPtr->at(0)->GetChildCurveVectorCP();
            break;
        }

    bvector<DPoint3d> curveVectorPoints;
    for (ICurvePrimitiveCPtr const& cp : *target)
        {
        bvector<DPoint3d> primitivePoints = GeometryUtils::GetCurvePrimitivePoints(cp, maxEdgeLength, keepSectorArea);
        curveVectorPoints.insert(curveVectorPoints.end(), primitivePoints.begin(), primitivePoints.end());
        }

    return curveVectorPoints;
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                11/2017
//---------------+---------------+---------------+---------------+---------------+------
bool GeometryUtils::IsSameSingleLoopGeometry
(
    CurveVectorCR geom1, 
    CurveVectorCR geom2, 
    double tolerance
)
    {
    BeAssert(CurveVector::BoundaryType::BOUNDARY_TYPE_ParityRegion != geom1.GetBoundaryType());
    BeAssert(CurveVector::BoundaryType::BOUNDARY_TYPE_UnionRegion != geom1.GetBoundaryType());
    BeAssert(CurveVector::BoundaryType::BOUNDARY_TYPE_ParityRegion != geom2.GetBoundaryType());
    BeAssert(CurveVector::BoundaryType::BOUNDARY_TYPE_UnionRegion != geom2.GetBoundaryType());

    if (!geom1.IsClosedPath() || !geom2.IsClosedPath())
        return false;

    CurveVectorPtr cv1 = geom1.Clone();
    CurveVectorPtr cv2 = geom2.Clone();

    cv1->ConsolidateAdjacentPrimitives(true);
    cv2->ConsolidateAdjacentPrimitives(true);

    if (cv1->size() != cv2->size())
        return false;

    if (cv1->size() == 1 && cv2->size() == 1
        && cv1->at(0)->GetCurvePrimitiveType() == ICurvePrimitive::CURVE_PRIMITIVE_TYPE_LineString
        && cv2->at(0)->GetCurvePrimitiveType() == ICurvePrimitive::CURVE_PRIMITIVE_TYPE_LineString)
        {
        // rotate linestring
        bvector<DPoint3d>* cv1LineString = cv1->at(0)->GetLineStringP();
        bvector<DPoint3d>* cv2LineString = cv2->at(0)->GetLineStringP();
        if (nullptr == cv1LineString || nullptr == cv2LineString)
            return false;

        if (cv1LineString->front().AlmostEqual(cv1LineString->back()))
            cv1LineString->pop_back();
        if (cv2LineString->front().AlmostEqual(cv2LineString->back()))
            cv2LineString->pop_back();

        if (cv1LineString->size() != cv2LineString->size())
            return false;

        DPoint3d cv1Start = cv1LineString->at(0);
        auto cv1StartInCv2 = std::find_if(cv2LineString->begin(), cv2LineString->end(),
                                          [cv1Start, tolerance] (DPoint3d const& p) { return cv1Start.AlmostEqual(p, tolerance); });
        if (cv2LineString->end() == cv1StartInCv2)
            return false;

        std::rotate(cv2LineString->begin(), cv1StartInCv2, cv2LineString->end());
        }
    else
        {
        ICurvePrimitivePtr cv1Start = cv1->at(0);
        auto cv1StartInCv2 = std::find_if(cv2->begin(), cv2->end(),
                                          [cv1Start, tolerance] (ICurvePrimitiveCPtr const& geom2)
            {
            return IsSameGeometry(*cv1Start, *geom2, tolerance);
            });
        if (cv2->end() == cv1StartInCv2)
            return false;

        std::rotate(cv2->begin(), cv1StartInCv2, cv2->end());
        }

    if (cv1->IsSameStructureAndGeometry(*cv2, tolerance))
        return true;

    auto cv1Iter = cv1->begin();
    auto cv2Iter = cv2->begin();
    for (; cv1->end() != cv1Iter && cv2->end() != cv2Iter; ++cv1Iter, ++cv2Iter)
        {
        if (!IsSameGeometry(**cv1Iter, **cv2Iter, tolerance))
            return false;
        }

    return true;
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                11/2017
//---------------+---------------+---------------+---------------+---------------+------
bool GeometryUtils::IsSameGeometry
(
    ICurvePrimitiveCR geom1,
    ICurvePrimitiveCR geom2,
    double tolerance
)
    {
    if (geom1.GetCurvePrimitiveType() != geom2.GetCurvePrimitiveType())
        return false;

    ICurvePrimitive::CurvePrimitiveType geomType = geom1.GetCurvePrimitiveType();
    switch (geomType)
        {
        case ICurvePrimitive::CurvePrimitiveType::CURVE_PRIMITIVE_TYPE_Arc:
            if (!geom1.GetArcCP()->IsAlmostEqual(*geom2.GetArcCP(), tolerance))
                return false;
            break;
        case ICurvePrimitive::CurvePrimitiveType::CURVE_PRIMITIVE_TYPE_CurveVector:
            if (!IsSameSingleLoopGeometry(*geom1.GetChildCurveVectorCP(), *geom2.GetChildCurveVectorCP(), tolerance))
                return false;
            break;
        case ICurvePrimitive::CurvePrimitiveType::CURVE_PRIMITIVE_TYPE_Line:
            if (!geom1.GetLineCP()->IsAlmostEqual(*geom2.GetLineCP(), tolerance))
                return false;
            break;
        case ICurvePrimitive::CurvePrimitiveType::CURVE_PRIMITIVE_TYPE_LineString:
            if (!DPoint3d::AlmostEqual(*geom1.GetLineStringCP(), *geom2.GetLineStringCP(), tolerance))
                return false;
            break;
        case ICurvePrimitive::CurvePrimitiveType::CURVE_PRIMITIVE_TYPE_PointString:
            if (!DPoint3d::AlmostEqual(*geom1.GetPointStringCP(), *geom2.GetPointStringCP(), tolerance))
                return false;
            break;
        case ICurvePrimitive::CurvePrimitiveType::CURVE_PRIMITIVE_TYPE_AkimaCurve:
            if (!DPoint3d::AlmostEqual(*geom1.GetAkimaCurveCP(), *geom2.GetAkimaCurveCP(), tolerance))
                return false;
            break;
        case ICurvePrimitive::CurvePrimitiveType::CURVE_PRIMITIVE_TYPE_BsplineCurve:
            if (!geom1.GetBsplineCurveCP()->AlmostEqual(*geom2.GetBsplineCurveCP(), tolerance))
                return false;
            break;
        case ICurvePrimitive::CurvePrimitiveType::CURVE_PRIMITIVE_TYPE_Catenary:
            {
            DCatenary3dPlacement catenary1, catenary2;
            geom1.TryGetCatenary(catenary1);
            geom2.TryGetCatenary(catenary2);
            if (!catenary1.AlmostEqual(catenary2, tolerance))
                return false;
            break;
            }
        case ICurvePrimitive::CurvePrimitiveType::CURVE_PRIMITIVE_TYPE_InterpolationCurve:
            if (!geom1.GetInterpolationCurveCP()->AlmostEqual(*geom2.GetInterpolationCurveCP(), tolerance))
                return false;
            break;
        default:
            BeAssert(false && "Not implemented");
        }

    return true;
    }

END_BUILDING_NAMESPACE

