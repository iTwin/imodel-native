/*--------------------------------------------------------------------------------------+
|
|     $Source: Grids/Elements/OrthogonalGridPortion.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#include "PublicApi/OrthogonalGridPortion.h"
#include <DgnPlatform/DgnDb.h>
#include <DgnPlatform/DgnCategory.h>
#include <DgnPlatform/ElementGeometry.h>
#include <DgnPlatform/ViewController.h>
#include "PublicApi\GridPlaneSurface.h"
//#include <DimensionHandler.h>
#include <ConstraintSystem/ConstraintSystemApi.h>
#include <BeSQLite/BeSQLite.h>

BEGIN_GRIDS_NAMESPACE
USING_NAMESPACE_BENTLEY_DGN
USING_NAMESPACE_CONSTRAINTMODEL
USING_NAMESPACE_BUILDING

DEFINE_GRIDS_ELEMENT_BASE_METHODS (OrthogonalGridPortion)


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Jonas.Valiunas                  03/2017
+---------------+---------------+---------------+---------------+---------------+------*/
OrthogonalGridPortion::OrthogonalGridPortion
(
T_Super::CreateParams const& params
) : T_Super(params)
    {

    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Jonas.Valiunas                  03/2017
+---------------+---------------+---------------+---------------+---------------+------*/
OrthogonalGridPortion::OrthogonalGridPortion
(
T_Super::CreateParams const& params,
DVec3d normal
) : T_Super(params, normal)
    {

    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Jonas.Valiunas                  10/2017
+---------------+---------------+---------------+---------------+---------------+------*/
OrthogonalGridPortionPtr        OrthogonalGridPortion::CreateAndInsert
(
StandardCreateParams const& params
)
    {
    OrthogonalGridPortionPtr thisGrid = new OrthogonalGridPortion (params, params.m_normal);

    BuildingLocks_LockElementForOperation (*thisGrid, BeSQLite::DbOpcode::Insert, "Inserting orthogonal grid");

    if (!thisGrid->Insert ().IsValid())
        return nullptr;

    Dgn::SpatialLocationModelPtr subModel = thisGrid->GetSurfacesModel ();
    
    if (subModel.IsValid ())
        {
        GridElementVector horizontalElements = CreateGridElements (params, subModel.get(), true);
        GridElementVector verticalElements = CreateGridElements (params, subModel.get(), false);

        for (GridSurfacePtr gridSurface : horizontalElements)
            {
            BuildingLocks_LockElementForOperation (*gridSurface, BeSQLite::DbOpcode::Insert, "Inserting gridSurface");
            gridSurface->Insert ();
            }

        for (GridSurfacePtr gridSurface : verticalElements)
            {
            BuildingLocks_LockElementForOperation (*gridSurface, BeSQLite::DbOpcode::Insert, "Inserting gridSurface");
            gridSurface->Insert ();
            }
        }
    return thisGrid;
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Jonas.Valiunas                  10/2017
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus                   OrthogonalGridPortion::CreateCoplanarGridPlanes
(
bvector<CurveVectorPtr> const& surfaces,
CreateParams const& params
)
    {
    BentleyStatus status = BentleyStatus::ERROR;
    Dgn::SpatialLocationModelPtr subModel = GetSurfacesModel ();

    if (subModel.IsValid ())
        {
        status = BentleyStatus::SUCCESS;
        //not putting into same method yet.. will do as GridAxis element is introduced.
        GridPlaneSurfacePtr lastGridPlane = nullptr;
        DPlane3d planeLast;
        for (CurveVectorPtr gridPlaneGeom : surfaces)
            {
            GridPlaneSurfacePtr gridPlane = GridPlaneSurface::Create (*subModel, gridPlaneGeom);
            BuildingLocks_LockElementForOperation (*gridPlane, BeSQLite::DbOpcode::Insert, "Inserting gridSurface");
            gridPlane->Insert ();
            DPlane3d planeThis;
            planeThis = gridPlane->GetPlane ();
            if (lastGridPlane.IsValid ())
                {
                //if planes are not parallel, then fail
                if (!bsiDVec3d_areParallelTolerance (&planeThis.normal, &planeLast.normal, BUILDING_TOLERANCE))
                    {
                    return BentleyStatus::ERROR;
                    }

                if (params.m_createDimensions)
                    DimensionHandler::Insert (params.m_dgndb, lastGridPlane->GetElementId (), gridPlane->GetElementId (), 0, 0, planeLast.normal, bsiDPlane3d_evaluate (&planeLast, &planeThis.origin));
                }
            planeLast = planeThis;
            lastGridPlane = gridPlane;
            }
        }
    return status;
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Jonas.Valiunas                  10/2017
+---------------+---------------+---------------+---------------+---------------+------*/
OrthogonalGridPortionPtr        OrthogonalGridPortion::CreateAndInsertBySurfaces
(
bvector<CurveVectorPtr> const& xSurfaces,
bvector<CurveVectorPtr> const& ySurfaces,
CreateParams const& params
)
    {
    if (BentleyStatus::SUCCESS != ValidateBySurfacesParams (xSurfaces, ySurfaces, params))
        return nullptr;

    OrthogonalGridPortionPtr thisGrid = new OrthogonalGridPortion (params, params.m_normal);

    BuildingLocks_LockElementForOperation (*thisGrid, BeSQLite::DbOpcode::Insert, "Inserting orthogonal grid");
    if (!thisGrid->Insert ().IsValid ())
        return nullptr;

    if (BentleyStatus::SUCCESS != thisGrid->CreateCoplanarGridPlanes (xSurfaces, params) ||
        BentleyStatus::SUCCESS != thisGrid->CreateCoplanarGridPlanes (ySurfaces, params))
        BeAssert (!"error inserting gridSurfaces into orthogonal grid.. shouldn't get here..");
    return thisGrid;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Jonas.Valiunas                  10/2017
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus                   OrthogonalGridPortion::ValidateBySurfacesParams
(
bvector<CurveVectorPtr> const& xSurfaces,
bvector<CurveVectorPtr> const& ySurfaces,
CreateParams const& params
)
    {
    if (AreSurfacesCoplanar (xSurfaces) && AreSurfacesCoplanar (ySurfaces))
        {
        return BentleyStatus::SUCCESS;
        }
    return BentleyStatus::ERROR;
    /*
    BeSQLite::Savepoint savePoint (params.m_dgndb, "creating gridPortion"); //do not need to commit - will get commited on destructor

    OrthogonalGridPortionPtr thisGrid = new OrthogonalGridPortion (params, params.m_normal);

    BuildingLocks_LockElementForOperation (*thisGrid, BeSQLite::DbOpcode::Insert, "Inserting orthogonal grid");
    if (!thisGrid->Insert ().IsValid ())
        return nullptr;

    if (BentleyStatus::SUCCESS != thisGrid->CreateCoplanarGridPlanes (xSurfaces, params) ||
        BentleyStatus::SUCCESS != thisGrid->CreateCoplanarGridPlanes (ySurfaces, params))
        savePoint.Cancel ();
    return thisGrid;*/
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Jonas.Valiunas                  10/2017
+---------------+---------------+---------------+---------------+---------------+------*/
bool                            OrthogonalGridPortion::AreSurfacesCoplanar
(
bvector<CurveVectorPtr> const& surfaces
)
    {
    DPlane3d planeLast;
    for (CurveVectorPtr gridPlaneGeom : surfaces)
        {
        DPlane3d planeThis;
        Transform localToWorld, worldToLocal;
        DRange3d range;

        gridPlaneGeom->IsPlanar (localToWorld, worldToLocal, range);
        bsiTransform_getOriginAndVectors (&localToWorld, &planeThis.origin, NULL, NULL, &planeThis.normal);

        //if planes are not parallel, then fail
        if (gridPlaneGeom != (*surfaces.begin()) && !bsiDVec3d_areParallelTolerance (&planeThis.normal, &planeLast.normal, BUILDING_TOLERANCE))
            {
            return false;
            }
        planeLast = planeThis;
        }
    return true;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Haroldas.Vitunskas                  04/17
//---------------------------------------------------------------------------------------
DVec3d OrthogonalGridPortion::FindOrthogonalFormTranslation(int elementIndex, double interval, double rotationAngle, bool isHorizontal)
    {
    double offset;
    if (isHorizontal)
        offset = elementIndex * interval;
    else
        offset = -elementIndex * interval;

    DPoint3d startingPoint = DPoint3d::FromZero();
    startingPoint.y += offset;
    DVec3d translation = DVec3d::FromStartEnd(DPoint3d::FromZero(), startingPoint);
    translation.RotateXY(rotationAngle);
    return translation;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Haroldas.Vitunskas                  04/17
//---------------------------------------------------------------------------------------
void OrthogonalGridPortion::AddDimensionsToOrthogonalGrid
(
GridSurfacePtr lastElement,
GridSurfacePtr element,
double distance
)
    {
    DPlane3d elementPlane = (dynamic_cast<GridPlaneSurface *>(element.get()))->GetPlane();
    DVec3d planeNormal = elementPlane.normal;

    if (lastElement.IsValid())
        {
        DPoint3d elementOrigin = elementPlane.origin;
        DPoint3d lastPlaneOrigin;
        DPlane3d lastPlane = (dynamic_cast<GridPlaneSurface *>(lastElement.get()))->GetPlane();
        bsiDPlane3d_projectPoint(&lastPlane, &lastPlaneOrigin, &elementOrigin);

        DVec3d direction = DVec3d::FromStartEnd(lastPlaneOrigin, elementOrigin);
        bsiDVec3d_normalizeInPlace(&direction);

        DimensionHandler::Insert(lastElement->GetDgnDb(),
                                 lastElement->GetElementId(),
                                 element->GetElementId(),
                                 0, 0,
                                 direction,
                                 distance);
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Haroldas.Vitunskas                  06/17
//---------------------------------------------------------------------------------------
GridElementVector OrthogonalGridPortion::CreateGridElements (StandardCreateParams params, Dgn::SpatialLocationModelCPtr model, bool isHorizontal)
    {
    GridElementVector orthogonalGrid;

    DVec3d extendTranslation;
    double interval;
    int count;
    double rotAngle = 0;
    double height = params.m_height;

    if (params.m_extendHeight)
        height += 2 * BUILDING_TOLERANCE;

    if (isHorizontal)
        {
        extendTranslation = params.m_horizontalExtendTranslation;
        interval = params.m_horizontalInterval;
        count = params.m_horizontalCount;
        }
    else
        {
        extendTranslation = params.m_verticalExtendTranslation;
        interval = params.m_verticalInterval;
        count = params.m_verticalCount;
        rotAngle += msGeomConst_pi / 2;
        }

    for (int i = 0; i < count; ++i)
        {
        DgnExtrusionDetail extDetail = GeometryUtils::CreatePlaneExtrusionDetail(params.m_length, height);
        extDetail.m_baseCurve->TransformInPlace(Transform::From(RotMatrix::FromAxisAndRotationAngle(2, rotAngle)));
        extDetail.m_baseCurve->TransformInPlace(Transform::From(extendTranslation));

        if (params.m_extendHeight)
            extDetail.m_baseCurve->TransformInPlace(Transform::From(DVec3d::From(0.0, 0.0, -BUILDING_TOLERANCE)));

        GridPlaneSurfacePtr baseGridPlane = GridPlaneSurface::Create(*model, extDetail);
        if (!baseGridPlane.IsValid())
            return GridElementVector();

        baseGridPlane->Translate(FindOrthogonalFormTranslation(i, interval, rotAngle, isHorizontal));
        orthogonalGrid.push_back(baseGridPlane);
        }

    return orthogonalGrid;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Haroldas.Vitunskas                  06/17
//---------------------------------------------------------------------------------------
GridElementVector OrthogonalGridPortion::CreateGridPreview(StandardCreateParams const& params)
    {
    GridElementVector horizontalElements = CreateGridElements(params, params.m_model, true);
    GridElementVector verticalElements = CreateGridElements(params, params.m_model, false);

    GridElementVector orthogonalGrid = horizontalElements;
    orthogonalGrid.insert(orthogonalGrid.end(), verticalElements.begin(), verticalElements.end());

    return orthogonalGrid;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Haroldas.Vitunskas                  06/17
//---------------------------------------------------------------------------------------
BentleyStatus OrthogonalGridPortion::CreateAndInsert (GridAxisMap& grid, StandardCreateParams const& params, DVec3d normal)
    {
    GridElementVector horizontalElements = CreateGridElements(params, params.m_model, true);
    GridElementVector verticalElements = CreateGridElements(params, params.m_model, false);

    grid[HORIZONTAL_AXIS] = horizontalElements;
    grid[VERTICAL_AXIS] = verticalElements;

    for (bpair<Utf8String, GridElementVector> pair : grid)
        {
        GridSurfacePtr lastElement;
        double distance = (0 == strcmp(pair.first.c_str(), HORIZONTAL_AXIS) ? params.m_horizontalInterval : params.m_verticalInterval);

        for (GridSurfacePtr gridSurface : pair.second)
            {
            if (BentleyStatus::ERROR == BuildingUtils::InsertElement(gridSurface))
                return BentleyStatus::ERROR;

            AddDimensionsToOrthogonalGrid(lastElement, gridSurface, distance);
            lastElement = gridSurface;
            }
        }

    return BentleyStatus::SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Haroldas.Vitunskas                  06/17
//---------------------------------------------------------------------------------------
double getIntervalBetweenElements(GridElementVector elements)
    {
    if (elements.size() >= 2)
        {
        GridSurfacePtr plane1 = elements[0];
        GridSurfacePtr plane2 = elements[1];

        return plane1->GetPlacement().GetOrigin().DistanceXY(plane2->GetPlacement().GetOrigin());
        }
    else
        return 0;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Haroldas.Vitunskas                  06/17
//---------------------------------------------------------------------------------------
void rotateAxisToAngleXY(GridElementVector& axis, double theta)
    {
    double axisInterval = getIntervalBetweenElements(axis);

    if (axis.size() > 0)
        {
        double existingRotation = GeometryUtils::PlacementToAngleXY(axis.front()->GetPlacement());
        DVec3d axisTranslation = DVec3d::From(0.0, 0.0, 0.0);

        if (axis.size() > 1)
            {
            axisTranslation = DVec3d::FromStartEnd(axis[0]->GetPlacement().GetOrigin(), axis[1]->GetPlacement().GetOrigin());
            axisTranslation.RotateXY(theta);
            }

        axis[0]->RotateXY(theta);
        Placement3d basePlacement = axis[0]->GetPlacement();

        for (int i = 1; i < axis.size(); ++i)
            {
            axis[i]->SetPlacement(basePlacement);

            axisTranslation.ScaleToLength(i * axisInterval);
            DPoint3d newOrigin = basePlacement.GetOrigin();
            newOrigin.Add(axisTranslation);
            axis[i]->MoveToPoint(newOrigin);
            }
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Haroldas.Vitunskas                  06/17
//---------------------------------------------------------------------------------------
void OrthogonalGridPortion::RotateToAngleXY(GridAxisMap& grid, double theta, bool updateDimensions)
    {
    GridElementVector newHorizontalAxis = grid[HORIZONTAL_AXIS];
    rotateAxisToAngleXY(newHorizontalAxis, theta);
    grid[HORIZONTAL_AXIS] = newHorizontalAxis;

    GridElementVector newVerticalAxis = grid[VERTICAL_AXIS];
    rotateAxisToAngleXY(newVerticalAxis, theta);
    grid[VERTICAL_AXIS] = newVerticalAxis;

    RotMatrix rotMatrix = RotMatrix::FromAxisAndRotationAngle(2, theta);

    if (updateDimensions)
        for (bpair<Utf8String, GridElementVector> axis : grid)
            {
            if (grid[axis.first].empty())
                continue;

            DgnDbR db = axis.second.front()->GetDgnDb();

            for (GridSurfacePtr gridSurface : axis.second)
                {
                bvector<BeSQLite::EC::ECInstanceId> relationships = DimensionHandler::GetDimensioningRelationshipInstances(db, gridSurface->GetElementId());
                for (BeSQLite::EC::ECInstanceId instance : relationships)
                    DimensionHandler::RotateDirection(db, instance, rotMatrix);
                }
            }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Haroldas.Vitunskas                  06/17
//---------------------------------------------------------------------------------------
void OrthogonalGridPortion::RotateToAngleXY(GridElementVector& grid, double theta, bool updateDimensions)
    {
    GridAxisMap axisMap;
    if (BentleyStatus::ERROR == ElementVectorToAxisMap(axisMap, grid))
        return;

    RotateToAngleXY(axisMap, theta, updateDimensions);

    grid = axisMap[HORIZONTAL_AXIS];
    grid.insert(grid.end(), axisMap[VERTICAL_AXIS].begin(), axisMap[VERTICAL_AXIS].end());


    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Haroldas.Vitunskas                  06/17
//---------------------------------------------------------------------------------------
BentleyStatus OrthogonalGridPortion::ElementVectorToAxisMap(GridAxisMap& axisMap, GridElementVector elements)
    {
    axisMap = GridAxisMap();

    DVec3d horizontalNormal = DVec3d::From(0, 0, 0);
    DVec3d verticalNormal = DVec3d::From(0, 0, 0);

    // Cast all elements to grid planes
    bvector<GridPlaneSurfacePtr> planes;
    for (GridSurfacePtr surface : elements)
        {
        GridPlaneSurfacePtr plane = dynamic_cast<GridPlaneSurface *>(surface.get());
        if (!plane.IsValid())
            return BentleyStatus::ERROR;

        planes.push_back(plane);
        }

    // Find such normals that if you rotate horizontal normal by msGeomConst_pi / 2 you get vertical normal
    for (GridPlaneSurfacePtr plane : planes)
        {
        DVec3d normal = plane->GetPlane().normal;

        if (normal.IsZero()) // no plane normal can be zero
            return BentleyStatus::ERROR;

        if (horizontalNormal.IsZero()) // assign first normal as horizontal
            horizontalNormal = normal;
        else
            {
            if (normal.IsPositiveParallelTo(horizontalNormal))
                continue;
            
            if (!normal.IsPerpendicularTo(horizontalNormal)) // all normals must be either perpendicular or parallel to each other
                return BentleyStatus::ERROR;

            verticalNormal = normal;
            break;
            }
        }

    if (!verticalNormal.IsZero()) // If no normal perpendicular to horizontal normal has been found all planes will be under horizontal axis
        {
        // Check if horizontal and vertical normals are correct
        DVec3d testHorizontalNormal = horizontalNormal;
        testHorizontalNormal.RotateXY(msGeomConst_pi / 2);
        if (!testHorizontalNormal.IsPositiveParallelTo(verticalNormal)) // If rotated horizontal normal is not positive parallel to vertical normal, they probably need to be swapped
            {
            DVec3d testVerticalNormal = verticalNormal; // Check rotating current vertical normal by msGeomConst_pi / 2 we get horizontal normal
            testVerticalNormal.RotateXY(msGeomConst_pi / 2);

            if (!testVerticalNormal.IsPositiveParallelTo(horizontalNormal))
                return BentleyStatus::ERROR;

            // Swap normals
            DVec3d temp = horizontalNormal;
            horizontalNormal = verticalNormal;
            verticalNormal = temp;
            }
        }

    // Check every plane normal and put it under corresponding axis
    for (GridPlaneSurfacePtr plane : planes)
        {
        DVec3d normal = plane->GetPlane().normal;

        if (normal.IsPositiveParallelTo(horizontalNormal))
            axisMap[HORIZONTAL_AXIS].push_back(plane);
        else if (normal.IsPositiveParallelTo(verticalNormal))
            axisMap[VERTICAL_AXIS].push_back(plane);
        else
            return BentleyStatus::ERROR;
        }

    return BentleyStatus::SUCCESS;
    }

END_GRIDS_NAMESPACE