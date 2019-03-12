/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnUtils/DgnGeometryUtils.cpp $
|
|  $Copyright: (c) 2019 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "PublicApi/BuildingDgnUtilsApi.h"
#include <Vu\VuApi.h>
#include <BuildingShared/Units/UnitConverter.h>
#include <math.h>

USING_NAMESPACE_BUILDING_SHARED

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Jonas.Valiunas                  02/2017
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus DgnGeometryUtils::CreateBodyFromGeometricPrimitive
(
Dgn::IBRepEntityPtr& out,
Dgn::GeometricPrimitiveCPtr primitive,
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
        case Dgn::GeometricPrimitive::GeometryType::CurvePrimitive:
            status = Dgn::BRepUtil::Create::BodyFromCurveVector(out, *CurveVector::Create(CurveVector::BoundaryType::BOUNDARY_TYPE_None, primitive->GetAsICurvePrimitive()), id);
            break;
        case Dgn::GeometricPrimitive::GeometryType::CurveVector:
            status = Dgn::BRepUtil::Create::BodyFromCurveVector(out, *primitive->GetAsCurveVector(), id);
            break;
        case Dgn::GeometricPrimitive::GeometryType::SolidPrimitive:
            status = Dgn::BRepUtil::Create::BodyFromSolidPrimitive(out, *primitive->GetAsISolidPrimitive(), id);
            break;
        case Dgn::GeometricPrimitive::GeometryType::BsplineSurface:
            status = Dgn::BRepUtil::Create::BodyFromBSurface(out, *primitive->GetAsMSBsplineSurface(), id);
            break;
        case Dgn::GeometricPrimitive::GeometryType::Polyface:
            status = Dgn::BRepUtil::Create::BodyFromPolyface(out, *primitive->GetAsPolyfaceHeader(), id);
            break;
        case Dgn::GeometricPrimitive::GeometryType::BRepEntity:
            out = primitive->GetAsIBRepEntity();
            status = SUCCESS;
            break;
        case Dgn::GeometricPrimitive::GeometryType::TextString:
        default:
            status = ERROR; //we're not converting textstrings into breps.
            break;
        }
    return status;
    }

//---------------------------------------------------------------------------------------
// @param         slicedGeometry      OUT solid slices paired with corresponding bottom planes of the cuts
// @param         geometryToSlice     IN
// @param         cuttingPlaneProfile IN
// @param         sliceHeight         IN
// @bsimethod                                    Nerijus.Jakeliunas              06/2017
//---------------+---------------+---------------+---------------+---------------+-------
BentleyStatus DgnGeometryUtils::SliceBodyByPlanes
(
bvector<bpair<Dgn::IBRepEntityPtr, CurveVectorPtr>>& slicedGeometry,
Dgn::IBRepEntityCR geometryToSlice,
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
        Dgn::IBRepEntityPtr bottomSheetBody = nullptr;
        Dgn::BRepUtil::Create::BodyFromCurveVector(bottomSheetBody, *bottomPlanePtr);
        if (!bottomSheetBody.IsValid())
            {
            slicedGeometry = bvector<bpair<Dgn::IBRepEntityPtr, CurveVectorPtr>>();
            return BSIERROR;
            }

        // cut out material below bottom plane
        Dgn::IBRepEntityPtr slice = geometryToSlice.Clone();
        Dgn::BRepUtil::Modify::BooleanCut(slice, *bottomSheetBody, Dgn::BRepUtil::Modify::CutDirectionMode::Backward, Dgn::BRepUtil::Modify::CutDepthMode::All, sliceHeight, true);
        if (!slice.IsValid())
            {
            slicedGeometry = bvector<bpair<Dgn::IBRepEntityPtr, CurveVectorPtr>>();
            return BSIERROR;
            }

        offset = DPoint3d::From(0.0, 0.0, sliceHeight);
        tranform = Transform::From(offset);
        CurveVectorPtr topPlanePtr = bottomPlanePtr->Clone();
        topPlanePtr->TransformInPlace(tranform);

        Dgn::IBRepEntityPtr topSheetBody = nullptr;
        Dgn::BRepUtil::Create::BodyFromCurveVector(topSheetBody, *topPlanePtr);
        if (!topSheetBody.IsValid())
            {
            slicedGeometry = bvector<bpair<Dgn::IBRepEntityPtr, CurveVectorPtr>>();
            return BSIERROR;
            }

        // cut out material above top plane
        Dgn::BRepUtil::Modify::BooleanCut(slice, *topSheetBody, Dgn::BRepUtil::Modify::CutDirectionMode::Forward, Dgn::BRepUtil::Modify::CutDepthMode::All, sliceHeight, true);
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

//--------------------------------------------------------------------------------------
// @bsimethod                                    Nerijus.Jakeliunas              06/2017
//---------------+---------------+---------------+---------------+---------------+------
CurveVectorPtr DgnGeometryUtils::ExtractXYProfileFromSolid(Dgn::IBRepEntityCR solid, CurveVectorPtr* pTopProfile)
    {
    bvector<Dgn::ISubEntityPtr> subEntities;
    Dgn::BRepUtil::GetBodyFaces(&subEntities, solid);
    double lowestZValue = DBL_MAX;
    double highestZValue = DBL_MIN;
    CurveVectorPtr profilePtr = nullptr;

    for (Dgn::ISubEntityPtr subEntityPtr : subEntities)
        {
        if (!subEntityPtr.IsValid() || !Dgn::BRepUtil::IsPlanarFace(*subEntityPtr))
            {
            continue;
            }

        DPoint3d point;
        DVec3d normal, uDir, vDir;
        DPoint2d uvParam{ 0.0, 0.0 };
        Dgn::BRepUtil::EvaluateFace(*subEntityPtr, point, normal, uDir, vDir, uvParam);
        if (!normal.IsParallelTo(DVec3d::From(0.0, 0.0, -1.0)))
            {
            continue;
            }

        Dgn::GeometricPrimitiveCPtr geometry = subEntityPtr->GetGeometry();
        Dgn::IBRepEntityPtr sheet = geometry->GetAsIBRepEntity();
        if (!sheet.IsValid() || sheet->GetEntityType() != Dgn::IBRepEntity::EntityType::Sheet)
            {
            continue;
            }

        CurveVectorPtr curveVectorPtr = Dgn::BRepUtil::Create::BodyToCurveVector(*sheet);
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
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Haroldas.Vitunskas             03/2017
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus DgnGeometryUtils::GetGeometricPrimitivesFromGeometricElement
(
bvector<Dgn::GeometricPrimitivePtr>& geometricPrimitives,
Dgn::GeometricElementCPtr geoElement
)
    {
    auto pGeometrySource = geoElement->ToGeometrySource();
    if (nullptr == pGeometrySource)
        return BentleyStatus::ERROR;

    Dgn::GeometryCollection geomData(*pGeometrySource);
    Transform elemToWorld = (*geomData.begin()).GetGeometryToWorld();
    if (geomData.begin() == geomData.end())
        return BentleyStatus::ERROR;

    for (Dgn::GeometryCollection::Iterator it = geomData.begin(); it != geomData.end(); ++it)
        {
        Dgn::GeometricPrimitivePtr clone = (*it).GetGeometryPtr()->Clone();
        Transform elemToWorld = (*it).GetGeometryToWorld();
        clone->TransformInPlace(elemToWorld);
        geometricPrimitives.push_back(clone);
        }
    return BentleyStatus::SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Jonas.Valiunas                  05/2017
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus DgnGeometryUtils::GetIBRepEntitiesFromGeometricElement
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

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jonas.Valiunas                 03/2018
//--------------+---------------+---------------+---------------+---------------+------
BentleyStatus   DgnGeometryUtils::GetTopBottomProfilesOnZeroPlane
(
Dgn::IBRepEntityCR solid,
CurveVectorPtr& bottomProfile,
CurveVectorPtr& topProfile
)
    {
    bottomProfile = ExtractXYProfileFromSolid(solid, &topProfile);
    bottomProfile = GeometryUtils::GetProfileOnZeroPlane(*bottomProfile);
    topProfile = GeometryUtils::GetProfileOnZeroPlane(*topProfile);
    return BSISUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Haroldas.Vitunskas              06/2017
//---------------------------------------------------------------------------------------
void DgnGeometryUtils::RotatePlacementXY(Placement3dR placement, double theta)
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
void DgnGeometryUtils::RotatePlacementAroundPointXY(Placement3dR placement, DPoint3d origin, double theta)
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
void DgnGeometryUtils::TranslatePlacementXY(Placement3dR placement, DVec3d translation)
    {
    translation.z = 0;
    TranslatePlacement(placement, translation);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Haroldas.Vitunskas              06/2017
//---------------------------------------------------------------------------------------
void DgnGeometryUtils::TranslatePlacement(Placement3dR placement, DVec3d translation)
    {
    Transform translationTransform = Transform::From(translation);
    translationTransform.Multiply(placement.GetOriginR());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Haroldas.Vitunskas              06/2017
//---------------------------------------------------------------------------------------
double DgnGeometryUtils::PlacementToAngleXY(Placement3d placement)
    {
    YawPitchRollAngles angles = placement.GetAngles();
    RotMatrix rotMatrix = angles.ToRotMatrix();
    return GeometryUtils::RotMatrixToAngleXY(rotMatrix);
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                07/2018
//---------------+---------------+---------------+---------------+---------------+------
Dgn::IBRepEntityPtr DgnGeometryUtils::GetXYCrossSectionSheetBody
(
    Dgn::IBRepEntityCR solid, 
    double z
)
    {
    CurveVectorPtr crossSection = GetXYCrossSection(solid, z);
    if (crossSection.IsNull())
        return nullptr;

    Dgn::IBRepEntityPtr crossSectionSheet = nullptr;
    Dgn::BRepUtil::Create::BodyFromCurveVector(crossSectionSheet, *crossSection);
    return crossSectionSheet;
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas Butkus                08/2017
//---------------+---------------+---------------+---------------+---------------+------
CurveVectorPtr DgnGeometryUtils::GetXYCrossSection(Dgn::IBRepEntityCR solid, double z)
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

    Dgn::IBRepEntityPtr crossSectionSheetBody = nullptr;
    Dgn::BRepUtil::Create::BodyFromCurveVector(crossSectionSheetBody, *crossSection);

    Dgn::IBRepEntityPtr intersectionTool = solid.Clone();
    Dgn::BRepUtil::Modify::BooleanOperation(crossSectionSheetBody, intersectionTool, Dgn::BRepUtil::Modify::BooleanMode::Intersect);
    if (crossSectionSheetBody.IsValid()) // the given solid is intersecting XY plane on given Z coordinate
        {
        bvector<Dgn::ISubEntityPtr> faces;
        Dgn::BRepUtil::GetBodyFaces(&faces, *crossSectionSheetBody);

        Dgn::IBRepEntityPtr crossSection;
        for (auto face : faces)
            {
            if (!crossSection.IsValid())
                crossSection = face->GetGeometry()->GetAsIBRepEntity();
            else
                {
                Dgn::IBRepEntityPtr tool = face->GetGeometry()->GetAsIBRepEntity();
                Dgn::BRepUtil::Modify::BooleanOperation(crossSection, tool, Dgn::BRepUtil::Modify::BooleanMode::Unite);
                }
            }

        CurveVectorPtr crossSectionCV = Dgn::BRepUtil::Create::BodyToCurveVector(*crossSection);
        if (crossSectionCV.IsNull()) // probably the body has multiple non intersecting faces
            {
            Dgn::BRepUtil::GetBodyFaces(&faces, *crossSectionSheetBody);

            CurveVectorPtr cv = CurveVector::Create(CurveVector::BOUNDARY_TYPE_UnionRegion);
            for (Dgn::ISubEntityPtr const& face : faces)
                {
                CurveVectorPtr faceCV = Dgn::BRepUtil::Create::PlanarFaceToCurveVector(*face);
                if (faceCV.IsNull()) // face was not planar
                    {
                    continue;
                    }

                cv->Add(faceCV);
                }

            crossSectionCV = cv;
            }

        if (crossSectionCV.IsNull())
            return nullptr;

        crossSectionCV->ConsolidateAdjacentPrimitives();
        return crossSectionCV;
        }

    // the given solid does not intersect XY plane on given Z coordinate, just touches it
    bvector<DPoint3d> touchPoints;
    bvector<Dgn::ISubEntityPtr> edgesAndVertices;
    Dgn::BRepUtil::GetBodyVertices(&edgesAndVertices, solid);
    Dgn::BRepUtil::GetBodyEdges(&edgesAndVertices, solid);
    for (Dgn::ISubEntityPtr subEntity : edgesAndVertices)
        {
        Dgn::ISubEntity::SubEntityType subEntityType = subEntity->GetSubEntityType();
        Dgn::GeometricPrimitiveCPtr geom = subEntity->GetGeometry();

        if (Dgn::ISubEntity::SubEntityType::Vertex == subEntityType)
            {
            EmbeddedDPoint3dArray vertexPoints = *geom->GetAsICurvePrimitive()->GetPointStringCP();
            for (DPoint3d point : vertexPoints)
                {
                if (DoubleOps::TolerancedComparison(point.z, z) == 0 && // check if AlmostEqual
                    std::find(touchPoints.begin(), touchPoints.end(), point) == touchPoints.end())
                    touchPoints.push_back(point);
                }
            }
        else if(Dgn::ISubEntity::SubEntityType::Edge == subEntityType)
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

//---------------------------------------------------------------------------------------
// @param         slicedGeometry      OUT solid slices paired with corresponding bottom planes of the cuts
// @param         geometryToSlice     IN geometry to slice
// @param         zElevationVector    IN vector of z elevations
// @bsimethod                                    Nerijus.Jakeliunas              06/2017
//---------------+---------------+---------------+---------------+---------------+-------
BentleyStatus DgnGeometryUtils::SliceBodyByZElevations
(
bvector<bpair<Dgn::IBRepEntityPtr, double>>& slicedGeometry,
Dgn::IBRepEntityCR geometryToSlice,
bvector<double>& zElevationVector
)
    {
    DRange3d range = geometryToSlice.GetEntityRange ();
    if (range.IsNull ())
        {
        return BSIERROR;
        }

    for (bvector<double>::iterator pIter = zElevationVector.begin(); pIter != zElevationVector.end(); ++pIter)
        {
        if ((pIter + 1) == zElevationVector.end())
            break;

        double bottomElevation = *pIter;
        double topElevation = *(pIter + 1);

        Dgn::IBRepEntityPtr slice = GetBodySlice(geometryToSlice, bottomElevation, topElevation);
        slicedGeometry.push_back({slice, bottomElevation});
        }

    return BSISUCCESS;
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                07/2018
//---------------+---------------+---------------+---------------+---------------+------
Dgn::IBRepEntityPtr DgnGeometryUtils::GetBodySlice
(
    Dgn::IBRepEntityCR geometryToSlice, 
    double bottomElevation, 
    double topElevation
)
    {
    BeAssert(bottomElevation <= topElevation);

    if (DoubleOps::AlmostEqual(bottomElevation, topElevation))
        {
        return GetXYCrossSectionSheetBody(geometryToSlice, bottomElevation);
        }

    Dgn::IBRepEntityPtr slice = GetUpwardSlice(geometryToSlice, bottomElevation);
    if (slice.IsNull())
        return nullptr;

    return GetDownwardSlice(*slice, topElevation);
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                07/2018
//---------------+---------------+---------------+---------------+---------------+------
Dgn::IBRepEntityPtr DgnGeometryUtils::GetCutSheetBody
(
    Dgn::IBRepEntityCR geometryToSlice, 
    double elevation
)
    {
    DRange3d range = geometryToSlice.GetEntityRange();
    if (range.IsNull())
        return nullptr;

    Transform transformBottom = Transform::From(DPoint3d::From(0, 0, elevation));

    double rangeExtendValue = 0.5;
    range.Extend(rangeExtendValue);
    bvector<DPoint3d> cutProfilePoints;
    cutProfilePoints.push_back({range.low.x, range.low.y, 0.0});
    cutProfilePoints.push_back({range.low.x + range.XLength(), range.low.y , 0.0});
    cutProfilePoints.push_back({range.low.x + range.XLength(), range.low.y + range.YLength(), 0.0});
    cutProfilePoints.push_back({range.low.x, range.low.y + range.YLength(), 0.0});

    CurveVectorPtr cutProfilePtr = CurveVector::CreateLinear(cutProfilePoints, CurveVector::BOUNDARY_TYPE_Outer);
    Dgn::IBRepEntityPtr cutSheetBody = nullptr;
    Dgn::BRepUtil::Create::BodyFromCurveVector(cutSheetBody, *cutProfilePtr);
    if (cutSheetBody.IsNull())
        return nullptr;

    cutSheetBody->ApplyTransform(transformBottom);
    return cutSheetBody;
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                07/2018
//---------------+---------------+---------------+---------------+---------------+------
Dgn::IBRepEntityPtr DgnGeometryUtils::GetUpwardSlice
(
    Dgn::IBRepEntityCR geometryToSlice, 
    double elevation
)
    {
    DRange3d range = geometryToSlice.GetEntityRange();
    if (!range.IsNull())
        {
        if (range.high.z < elevation)
            {
            return nullptr;
            }
        else if (DoubleOps::AlmostEqual(range.high.z, elevation))
            {
            return GetXYCrossSectionSheetBody(geometryToSlice, elevation);
            }
        }

    Dgn::IBRepEntityPtr slice = geometryToSlice.Clone();
    Dgn::IBRepEntityPtr cutSheetBody = GetCutSheetBody(geometryToSlice, elevation);
    if (cutSheetBody.IsNull())
        return nullptr;

    Dgn::BRepUtil::Modify::BooleanCut(slice, *cutSheetBody, Dgn::BRepUtil::Modify::CutDirectionMode::Backward, Dgn::BRepUtil::Modify::CutDepthMode::All, 0.0, true);
    if (slice.IsNull())
        return nullptr;

    return slice;
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                07/2018
//---------------+---------------+---------------+---------------+---------------+------
Dgn::IBRepEntityPtr DgnGeometryUtils::GetDownwardSlice
(
    Dgn::IBRepEntityCR geometryToSlice, 
    double elevation
)
    {
    DRange3d range = geometryToSlice.GetEntityRange();
    if (!range.IsNull())
        {
        if (range.low.z > elevation)
            {
            return nullptr;
            }
        else if (DoubleOps::AlmostEqual(range.low.z, elevation))
            {
            return GetXYCrossSectionSheetBody(geometryToSlice, elevation);
            }
        }

    Dgn::IBRepEntityPtr slice = geometryToSlice.Clone();
    Dgn::IBRepEntityPtr cutSheetBody = GetCutSheetBody(geometryToSlice, elevation);
    if (cutSheetBody.IsNull())
        return nullptr;

    Dgn::BRepUtil::Modify::BooleanCut(slice, *cutSheetBody, Dgn::BRepUtil::Modify::CutDirectionMode::Forward, Dgn::BRepUtil::Modify::CutDepthMode::All, 0.0, true);
    if (slice.IsNull())
        return nullptr;

    return slice;
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Nerijus.Jakeliunas              06/2017
//---------------+---------------+---------------+---------------+---------------+------
CurveVectorPtr DgnGeometryUtils::ExtractBottomFaceShape(Dgn::GeometricElement3dCR extrusionThatIsSolid)
    {
    auto pGeometrySource = extrusionThatIsSolid.ToGeometrySource();
    if (nullptr == pGeometrySource)
        {
        return nullptr;
        }

    Dgn::GeometryCollection geomDatas(*pGeometrySource);
    for (auto geomData : geomDatas)
        {
        switch (geomData.GetEntryType())
            {
            case (Dgn::GeometryCollection::Iterator::EntryType::BRepEntity):
            {
            Dgn::IBRepEntityPtr solidPtr = (*geomData).GetGeometryPtr()->GetAsIBRepEntity();
            if (solidPtr.IsValid() && (solidPtr->GetEntityType() == Dgn::IBRepEntity::EntityType::Solid))
                {
                return DgnGeometryUtils::ExtractXYProfileFromSolid(*solidPtr);
                }
            break;
            }
            case (Dgn::GeometryCollection::Iterator::EntryType::CurveVector):
            {
            CurveVectorPtr cvPtr = (*geomData).GetGeometryPtr()->GetAsCurveVector();
            if (cvPtr.IsValid())
                {
                if (cvPtr->IsClosedPath())
                    {
                    return cvPtr; //data from ABD bridge has text artifacts and geometry.
                    }

                DPoint3d centroid;
                DVec3d   normal;
                double   area;
                if (cvPtr->CentroidNormalArea(centroid, normal, area))
                    {
                    if (area > 0.0)
                        {
                        return cvPtr; //data from ABD bridge has text artifacts and geometry.
                        }
                    }
                }

            break;
            }
            }
        }

    return nullptr;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Jonas.Valiunas                  07/2016
+---------------+---------------+---------------+---------------+---------------+------*/
bool DgnGeometryUtils::GetDgnExtrusionDetail
(
    Dgn::GeometricElement3dCR extrusionThatIsSolid,
    DgnExtrusionDetail& extDetail       //<= extrusion detail if success(true)
)
    {
    auto pGeometrySource = extrusionThatIsSolid.ToGeometrySource ();
    if (nullptr == pGeometrySource)
        {
        return false;
        }

    Dgn::GeometryCollection geomData (*pGeometrySource);
    if (geomData.begin () == geomData.end ())
        {
        return false;
        }

    Transform elemToWorld = (*geomData.begin ()).GetGeometryToWorld ();

    bool status = false;

    Dgn::GeometricPrimitivePtr geomPtr = (*geomData.begin()).GetGeometryPtr();
    if (geomPtr.IsValid())
        {
        ISolidPrimitivePtr solidPrimitive = geomPtr->GetAsISolidPrimitive();
        if (solidPrimitive.IsValid())
            {
            solidPrimitive->TransformInPlace(elemToWorld);
            status = solidPrimitive->TryGetDgnExtrusionDetail(extDetail);
            }
        }
    return status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Nerijus.Jakeliunas               09/2016
+---------------+---------------+---------------+---------------+---------------+------*/
CurveVectorPtr DgnGeometryUtils::GetBaseShape(Dgn::GeometricElement3dCR extrusionThatIsSolid)
    {
    DgnExtrusionDetail extDetail;
    if (DgnGeometryUtils::GetDgnExtrusionDetail(extrusionThatIsSolid, extDetail))
        {
        return extDetail.FractionToProfile(0.0);
        }

    CurveVectorPtr planePtr = DgnGeometryUtils::ExtractBottomFaceShape(extrusionThatIsSolid);
    if (!planePtr.IsValid())
        {
        BeAssert(false && "Failed to extract base shape");
        BentleyApi::NativeLogging::LoggingManager::GetLogger(L"BuildingSpacePlanning")->error("DgnGeometryUtils::Failed to extract base shape.\n");

        DRange3d range = extrusionThatIsSolid.GetPlacement().CalculateRange();
        bvector<DPoint3d> points =
            {
            range.low,
            { range.high.x, range.low.y, range.low.z },
            { range.high.x, range.high.y, range.low.z },
            { range.low.x, range.high.y, range.low.z },
            range.low,
            };

        planePtr = CurveVector::CreateLinear(points, CurveVector::BOUNDARY_TYPE_Outer);
        }
    else
        {
        planePtr->TransformInPlace(extrusionThatIsSolid.GetPlacementTransform());
        }

    return planePtr;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Elonas.Seviakovas               03/2019
+---------------+---------------+---------------+---------------+---------------+------*/
bool DgnGeometryUtils::InitiateExtrusionGeometry
(
    Dgn::GeometricElement3dR extrusion,
    Dgn::Render::GeometryParamsCP pGeometryParameters,
    Dgn::DgnSubCategoryId subCategoryId,
    DgnExtrusionDetailCR detail,
    IGeometryPtr const &otherGeometry,
    bool updatePlacementOrigin
)
    {
    Dgn::DgnCategoryId catId = extrusion.GetCategoryId();

    CurveVectorPtr baseShape = detail.FractionToProfile(0.0);
    double actualArea;
    DVec3d normal;
    DPoint3d centroid;
    baseShape->CentroidNormalArea(centroid, normal, actualArea);

    DPoint3d origin = updatePlacementOrigin ? centroid : extrusion.GetPlacement().GetOrigin();

    Dgn::GeometryBuilderPtr builder = Dgn::GeometryBuilder::Create(*extrusion.GetModel(), catId, origin);
    if (pGeometryParameters != nullptr)
    {
        builder->Append(*pGeometryParameters);
    }

    ISolidPrimitivePtr solidPrimitive = ISolidPrimitive::CreateDgnExtrusion(detail);
    Transform worldToLocal = Transform::From(origin).ValidatedInverse();
    solidPrimitive->TransformInPlace(worldToLocal);

    if (!builder->Append(*solidPrimitive))
        return false;

    if (otherGeometry.IsValid() && otherGeometry->TryTransformInPlace(worldToLocal))
        {
        builder->Append(*otherGeometry);
        }

    baseShape->TransformInPlace(worldToLocal);
    Utf8CP userLabel = extrusion.ToElement()->GetUserLabel();

    AppendExtrusionLabel(builder, *baseShape, userLabel, subCategoryId);

    if (SUCCESS != builder->Finish(extrusion))
        return false;

    extrusion.SetPropertyValue("FootprintArea", actualArea);

    return false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Elonas.Seviakovas               03/2019
+---------------+---------------+---------------+---------------+---------------+------*/
bool DgnGeometryUtils::InitiateExtrusionGeometry
(
    Dgn::GeometricElement3dR extrusion,
    Dgn::Render::GeometryParamsCP pGeometryParameters,
    Dgn::DgnSubCategoryId subCategoryId,
    CurveVectorCPtr base,
    double height,
    IGeometryPtr const &otherGeometry,
    bool updatePlacementOrigin
)
{
    if (!base.IsValid())
    {
        return false;
    }

    CurveVectorPtr selectedShape = base->Clone();
    selectedShape->ConsolidateAdjacentPrimitives(true);
    DVec3d zvector;
    zvector.Zero();
    zvector.z = 1.0;
    zvector.Scale(height); //we build an upwards looking sweep direction 
    DgnExtrusionDetail extrusionDetail(selectedShape, zvector, true);

    return InitiateExtrusionGeometry(extrusion, pGeometryParameters, subCategoryId, extrusionDetail, otherGeometry, updatePlacementOrigin);
}

void DgnGeometryUtils::CalculateProperties(Dgn::GeometricElement3dR extrusion)
    {
    CurveVectorPtr curveVec = GetBaseShape(extrusion);
    if (!curveVec.IsValid())
        {
        return;
        }

    double area;
    DVec3d normal;
    DPoint3d centroid;
    curveVec->CentroidNormalArea(centroid, normal, area);

    extrusion.SetPropertyValue("FootprintArea", area);
    }

void DgnGeometryUtils::ClearElementGeometry(Dgn::GeometricElement3dCR extrusion)
    {
    Dgn::GeometrySourceCP geometrySource = extrusion.ToGeometrySource();
    Dgn::GeometryStreamR geometryStream = const_cast<Dgn::GeometryStreamR>(geometrySource->GetGeometryStream());
    geometryStream.Clear();
    }

struct Extrusion : Dgn::GeometricElement3d {
    Extrusion(Dgn::GeometricElement3d::CreateParams params) : Dgn::GeometricElement3d(params) { }
};

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Elonas.Seviakovas               03/2019
+---------------+---------------+---------------+---------------+---------------+------*/
bool findLabelOriginPoint(CurveVectorCR base, DPoint3d& labelOrigin)
    {
    bvector<bvector<DPoint3d>> regionsPoints;
    if (!base.CollectLinearGeometry(regionsPoints) || regionsPoints.size() < 1)
        {
        return false;
        }

    EmbeddedIntArray indices;
    EmbeddedDPoint3dArray XYZOut;
    if (BSISUCCESS != vu_triangulateXYPolygon(&indices, &XYZOut, regionsPoints[0].data(), (int)regionsPoints[0].size(), 0.01, 3, false, true))
        {
        return false;
        }

    auto trianglePoints = bvector <DPoint3d>();
    double maxRadius = -1.0;
    for (int index : indices)
        {
        if (index > -1)
            {
            trianglePoints.push_back(XYZOut[index]);
            }

        if (-1 == index)
            {
            if (trianglePoints.size() != 3)
                {
                trianglePoints = bvector <DPoint3d>();
                continue;
                }

            double c = trianglePoints[0].Distance(trianglePoints[1]);
            double a = trianglePoints[1].Distance(trianglePoints[2]);
            double b = trianglePoints[2].Distance(trianglePoints[0]);

            // http://mathworld.wolfram.com/Incenter.html
            double incenterX = (a * trianglePoints[0].x + b * trianglePoints[1].x + c * trianglePoints[2].x) / (a + b + c);
            double incenterY = (a * trianglePoints[0].y + b * trianglePoints[1].y + c * trianglePoints[2].y) / (a + b + c);
            DPoint3d incenter = DPoint3d::From(incenterX, incenterY, trianglePoints[0].z);

            // http://mathworld.wolfram.com/Inradius.html
            double halfPerimeter = (a + b + c) / 2.0;
            double radius = sqrt((halfPerimeter - a) * (halfPerimeter - b) * (halfPerimeter - c) / halfPerimeter);

            if (CurveVector::INOUT_In == base.PointInOnOutXY(incenter) && maxRadius < radius)
                {
                maxRadius = radius;
                labelOrigin = incenter;
                }

            trianglePoints = bvector <DPoint3d>();
            }
        }

    return maxRadius > 0.0;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Elonas.Seviakovas               03/2019
+---------------+---------------+---------------+---------------+---------------+------*/
Dgn::TextStringPtr createElementText
(
    CurveVectorCR base,
    Utf8CP text,
    double zElevation = 0.0,
    double verticalOffset = 0.0
)
{
    double pixelDist = 1.0 / 120.0;
    double tScale = 30.0 * pixelDist;

    DPoint3d userOrigin;
    userOrigin.Zero();

    DPoint2d textScale;
    textScale.x = textScale.y = tScale;

    Dgn::TextStringStylePtr style = Dgn::TextStringStyle::Create();
    style->SetFont(Dgn::DgnFontManager::GetDecoratorFont());
    style->SetSize(textScale);
    style->SetIsUnderlined(true);

    RotMatrix textMatrix;
    textMatrix.InitFromScaleFactors(1.0, 1.0, 1.0);
    Dgn::TextStringPtr textStrPtr = Dgn::TextString::Create();
    textStrPtr->SetText(text);
    textStrPtr->SetOrientation(textMatrix);
    textStrPtr->SetStyle(*style);

    DRange3d range;
    base.GetRange(range);

    double dx = (range.low.x + range.high.x) / 2.0;
    double dy = (range.low.y + range.high.y) / 2.0;
    DPoint3d labelOrigin;
    Transform localToWorld, worldToLocal;
    if (!base.IsRectangle(localToWorld, worldToLocal) && findLabelOriginPoint(base, labelOrigin))
    {
        dx = labelOrigin.x;
        dy = labelOrigin.y;
    }
    userOrigin = DPoint3d::From(dx, dy - tScale * verticalOffset, zElevation);

    textStrPtr->SetOriginFromJustificationOrigin(userOrigin, Dgn::TextString::HorizontalJustification::Center, Dgn::TextString::VerticalJustification::Middle);

    return textStrPtr;
}


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Elonas.Seviakovas               03/2019
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnGeometryUtils::AppendExtrusionLabel
(
    Dgn::GeometryBuilderPtr& builder, 
    CurveVectorCR profile, Utf8CP name, 
    Dgn::DgnSubCategoryId subCategoryId /*= DgnSubCategoryId ()*/
)
    {
    if (nullptr == name || strlen(name) < 1)
        {
        return;
        }

    double area;
    DVec3d normal;
    DPoint3d centroid;
    profile.CentroidNormalArea(centroid, normal, area);

    //SetArea (area);
    if (area > 0.01)
        {
        if (subCategoryId.IsValid())
            {
            builder->Append(subCategoryId);
            }

        Dgn::TextStringPtr textStrPtr = createElementText(profile, name, centroid.z, -1.0);
        builder->Append(*textStrPtr);

        area = UnitConverter::ToSquareFeet(area);

        char areaText[100];
        BeStringUtilities::Snprintf(areaText, 100, "Area %.1f Sq.Ft.", area);

        Dgn::TextStringPtr areaStrPtr = createElementText(profile, areaText, centroid.z, 1.0);
        builder->Append(*areaStrPtr);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Elonas.Seviakovas               03/2019
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnGeometryUtils::SetExtrusionHeight
(
    Dgn::GeometricElement3dR extrusion,
    Dgn::Render::GeometryParamsCP pGeometryParameters,
    Dgn::DgnSubCategoryId subCategoryId,
    double newHeight
)
    {
    Dgn::GeometryCollection geometryData(extrusion);
    if (geometryData.begin() == geometryData.end())
        {
        return;
        }

    //add new geometry
    Dgn::GeometryBuilderPtr builder = Dgn::GeometryBuilder::Create(extrusion);
    if (pGeometryParameters != nullptr)
    {
        builder->Append(*pGeometryParameters);
    }

    CurveVectorPtr baseProfile = nullptr;

    Dgn::GeometricPrimitivePtr geometryPtr = (*geometryData.begin()).GetGeometryPtr();
    if (geometryPtr.IsValid())
        {
        ISolidPrimitivePtr solidPrimitive = geometryPtr->GetAsISolidPrimitive();
        DgnExtrusionDetail extDetail;
        if (solidPrimitive.IsValid() && solidPrimitive->TryGetDgnExtrusionDetail(extDetail))
            {
            //clean the existing geometry
            ClearElementGeometry(extrusion);

            extDetail.m_extrusionVector.Normalize();
            extDetail.m_extrusionVector.Scale(newHeight);
            solidPrimitive->TrySetDgnExtrusionDetail(extDetail);

            builder->Append(*solidPrimitive);

            baseProfile = extDetail.m_baseCurve;
            }
        else
            {
            Dgn::IBRepEntityPtr solid = (*geometryData.begin()).GetGeometryPtr()->GetAsIBRepEntity();
            if (solid.IsValid() && solid->GetEntityType() == Dgn::IBRepEntity::EntityType::Solid)
                {
                //clean the existing geometry
                ClearElementGeometry(extrusion);

                // TODO:  can/should we change hight of Solid ??? Do nothing for now
                builder->Append(*solid);
                baseProfile = DgnGeometryUtils::ExtractXYProfileFromSolid(*solid);
                }
            }
        }


    if (baseProfile.IsValid())
        {
        Utf8CP userLabel = extrusion.ToElement()->GetUserLabel();

        AppendExtrusionLabel(builder, *baseProfile, userLabel, subCategoryId);
        }
    builder->Finish(extrusion);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Elonas.Seviakovas               03/2019
+---------------+---------------+---------------+---------------+---------------+------*/
double DgnGeometryUtils::GetExtrusionHeight
(
    Dgn::GeometricElement3dCR extrusion
)
    {
    DgnExtrusionDetail extrusionDetail;
    if (DgnGeometryUtils::GetDgnExtrusionDetail(extrusion, extrusionDetail))
        {
        return extrusionDetail.m_extrusionVector.Magnitude();
        }
    else
        {
        return extrusion.ToGeometrySource3d()->GetPlacement().CalculateRange().ZLength();
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Elonas.Seviakovas               03/2019
+---------------+---------------+---------------+---------------+---------------+------*/
DPlane3d DgnGeometryUtils::GetExtrusionBasePlane
(
    Dgn::GeometricElement3dCR extrusion
)
    {
    DPlane3d plane;
    plane.Zero();
    Transform localToWorld, worldToLocal;
    DRange3d range;
    CurveVectorPtr baseShape = GetBaseShape(extrusion);
    if (baseShape->IsPlanar(localToWorld, worldToLocal, range))
        {
        plane.origin = localToWorld.Origin();
        plane.normal = localToWorld.ColumnZ();
        }
    else if (baseShape->size() == 1 &&
        baseShape->at(0)->GetCurvePrimitiveType() == ICurvePrimitive::CurvePrimitiveType::CURVE_PRIMITIVE_TYPE_PointString)
        {
        ICurvePrimitivePtr pointString = baseShape->at(0);
        bvector<DPoint3d> points = *pointString->GetPointStringCP();
        plane = DPlane3d::FromOriginAndNormal(points[0], DVec3d::From(0, 0, 1));
        }
    return plane;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Elonas.Seviakovas               03/2019
+---------------+---------------+---------------+---------------+---------------+------*/
DPlane3d DgnGeometryUtils::GetExtrusionTopPlane
(
    Dgn::GeometricElement3dCR extrusion
)
    {
    DPlane3d plane;
    plane.Zero();
    plane.normal.z = 1.0;
    Transform localToWorld, worldToLocal;
    DRange3d range;
    CurveVectorPtr baseShape = GetBaseShape(extrusion);
    if (baseShape->IsPlanar(localToWorld, worldToLocal, range))
        {
        plane.origin = localToWorld.Origin();
        plane.normal = localToWorld.ColumnZ();
        DPoint3d offset{ 0.0, 0.0, 1.0 };
        offset.ScaleToLength(GetExtrusionHeight(extrusion));
        plane.origin.Add(offset);
        }
    return plane;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Elonas.Seviakovas               03/2019
+---------------+---------------+---------------+---------------+---------------+------*/
Dgn::GeometryBuilderPtr DgnGeometryUtils::GetExtrusionGeometryBuilder
(
    Dgn::GeometricElement3dCR extrusion,
    Dgn::Render::GeometryParamsCP pGeometryParams,
    DPoint3dP pOrigin
)
    {
    DPoint3d zeroPoint;
    zeroPoint.Zero();

    if (pOrigin == nullptr)
        {
        pOrigin = &zeroPoint;
        }

    Dgn::GeometryBuilderPtr builder = Dgn::GeometryBuilder::Create(*(extrusion.GetModel()), extrusion.GetCategoryId(), *pOrigin);

    if (pGeometryParams != nullptr)
        {
        builder->Append(*pGeometryParams);
        }

    return builder;
    }

void DgnGeometryUtils::SetExtrusionName
(
    Dgn::GeometricElement3dR element,
    Dgn::Render::GeometryParamsCP pGeometryParameters,
    Dgn::DgnSubCategoryId subCategoryId,
    Utf8CP name
) 
    {
    element.SetUserLabel(name);

    // need to trigger update of geometry so that we would update label element
    SetExtrusionHeight(element, pGeometryParameters, subCategoryId, GetExtrusionHeight(element));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Elonas.Seviakovas               03/2019
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnGeometryUtils::SetDgnExtrusionDetail
(
    Dgn::GeometricElement3dR element,
    Dgn::Render::GeometryParamsCP pGeometryParameters,
    Dgn::DgnSubCategoryId subCategoryId,
    DgnExtrusionDetail const &extrusionDetail //=> extrusion detail to set
)
    {
    //clean the existing geometry
    ClearElementGeometry(element);

    // Create geometry
    InitiateExtrusionGeometry(element, pGeometryParameters, subCategoryId, extrusionDetail, nullptr, true);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Elonas.Seviakovas               03/2019
+---------------+---------------+---------------+---------------+---------------+------*/
bool DgnGeometryUtils::UpdateExtrusionGeometry
(
    Dgn::GeometricElement3dR extrusion,
    Dgn::Render::GeometryParamsCP pGeometryParameters,
    Dgn::DgnSubCategoryId subCategoryId,
    CurveVectorCPtr base,
    bool updatePlacementOrigin
)
    {
    return InitiateExtrusionGeometry(extrusion, pGeometryParameters, subCategoryId, base, GetExtrusionHeight(extrusion), nullptr, updatePlacementOrigin);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Elonas.Seviakovas               03/2019
+---------------+---------------+---------------+---------------+---------------+------*/
bool DgnGeometryUtils::UpdateExtrusionGeometry
(
    Dgn::GeometricElement3dR extrusion,
    Dgn::Render::GeometryParamsCP pGeometryParameters,
    Dgn::DgnSubCategoryId subCategoryId,
    DgnExtrusionDetailCR extrusionDetail,
    IGeometryPtr const &otherGeometry,
    bool updatePlacementOrigin
)
    {
    return InitiateExtrusionGeometry(extrusion, pGeometryParameters, subCategoryId, extrusionDetail, otherGeometry, updatePlacementOrigin);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Elonas.Seviakovas               03/2019
+---------------+---------------+---------------+---------------+---------------+------*/
bool DgnGeometryUtils::GetExtrusionPlane
(
    Dgn::GeometricElement3dCR extrusion,
    int geometryId,
    DPlane3dR planeOut
)
    {
    if (geometryId != 0 && geometryId != 1) //currently only able to handle extrusion top/bottom face
        BeAssert(!"Not Yet implemented");
    switch (geometryId)
        {
    case 0:
        planeOut = GetExtrusionBasePlane(extrusion);
        break;
    case 1:
        planeOut = GetExtrusionTopPlane(extrusion);
        break;
    default:
        return false;
        }

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Elonas.Seviakovas               03/2019
+---------------+---------------+---------------+---------------+---------------+------*/
bool DgnGeometryUtils::StretchExtrusionToPlane
(
    Dgn::GeometricElement3dR extrusion,
    Dgn::Render::GeometryParamsCP pGeometryParameters,
    Dgn::DgnSubCategoryId subCategoryId,
    DPlane3dR targetPlane
)
    {
    //if (planeId != 1)
    //    BeAssert(!"Not yet implemented");  //NEEDS WORK: currently can only stretch the top face?

    DPlane3d bottomPlane = GetExtrusionBasePlane(extrusion);
    double distance = targetPlane.Evaluate(bottomPlane.origin);
    SetExtrusionHeight(extrusion, pGeometryParameters, subCategoryId, fabs(distance));

    return false;
    }