/*--------------------------------------------------------------------------------------+
|
|     $Source: Grids/Elements/OrthogonalGridPortion.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#include <Grids/gridsApi.h>
#include <DgnPlatform/DgnDb.h>
#include <DgnPlatform/DgnCategory.h>
#include <DgnPlatform/ElementGeometry.h>
#include <DgnPlatform/ViewController.h>
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

    Dgn::DefinitionModelCR defModel = thisGrid->GetDgnDb().GetDictionaryModel ();

    GridAxisPtr horizontalAxis = GridAxis::CreateAndInsert (defModel, *thisGrid);
    GridAxisPtr verticalAxis = GridAxis::CreateAndInsert(defModel, *thisGrid);

    Dgn::SpatialLocationModelPtr subModel = thisGrid->GetSurfacesModel ();
    
    if (subModel.IsValid ())
        {
        GridElementVector horizontalElements = CreateGridElements (params, subModel.get(), true, horizontalAxis);
        GridElementVector verticalElements = CreateGridElements (params, subModel.get(), false, verticalAxis);

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
GridAxisPtr gridAxis,
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
            GridPlaneSurfacePtr gridPlane = GridPlaneSurface::Create (*subModel, gridAxis, gridPlaneGeom);
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


    Dgn::DefinitionModelCR defModel = thisGrid->GetDgnDb ().GetDictionaryModel ();

    GridAxisPtr horizontalAxis = GridAxis::CreateAndInsert(defModel, *thisGrid);
    GridAxisPtr verticalAxis = GridAxis::CreateAndInsert (defModel, *thisGrid);
    
    if (BentleyStatus::SUCCESS != thisGrid->CreateCoplanarGridPlanes (xSurfaces, horizontalAxis, params) ||
        BentleyStatus::SUCCESS != thisGrid->CreateCoplanarGridPlanes (ySurfaces, verticalAxis, params))
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
GridElementVector OrthogonalGridPortion::CreateGridElements (StandardCreateParams params, Dgn::SpatialLocationModelCPtr model, bool isHorizontal, GridAxisPtr gridAxis)
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
        DgnExtrusionDetail extDetail = GeometryUtils::CreatePlaneExtrusionDetail(params.m_length + 2 * extendTranslation.Magnitude(), height);
        extDetail.m_baseCurve->TransformInPlace(Transform::From(RotMatrix::FromAxisAndRotationAngle(2, rotAngle)));
        extDetail.m_baseCurve->TransformInPlace(Transform::From(extendTranslation));

        if (params.m_extendHeight)
            extDetail.m_baseCurve->TransformInPlace(Transform::From(DVec3d::From(0.0, 0.0, -BUILDING_TOLERANCE)));

        GridPlaneSurfacePtr baseGridPlane = GridPlaneSurface::Create(*model, gridAxis, extDetail);
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
// @bsimethod                                    Haroldas.Vitunskas                  10/17
//---------------------------------------------------------------------------------------
Dgn::RepositoryStatus OrthogonalGridPortion::RotateToAngleXY(double theta, bool updateDimensions)
    {
    DgnDbR db = GetDgnDb();
    Dgn::RepositoryStatus status = RepositoryStatus::Success;

    GridElementVector gridElements;
    for (DgnElementId gridSurfaceId : MakeIterator().BuildIdList<DgnElementId>())
        {
        GridSurfacePtr surface = db.Elements().GetForEdit<GridSurface>(gridSurfaceId);
        BeAssert(surface.IsValid() && "Grid surfaces related to grid portion must be valid");

        gridElements.push_back(surface);
        }

    if (gridElements.empty())
        return status;

    // Get rotation origin point
    DPoint3d rotationOrigin = gridElements.front()->GetPlacement().GetOrigin();

    for (GridSurfacePtr surface : gridElements)
        {
        surface->RotateXY(rotationOrigin, theta);
        if (RepositoryStatus::Success != (status = BuildingLocks_LockElementForOperation(*surface, BeSQLite::DbOpcode::Update, "update GridSurface")))
            return status;
        surface->Update();
        }

    if (!updateDimensions)
        return status;

    RotMatrix rotMatrix = RotMatrix::FromAxisAndRotationAngle(2, theta);

    for (GridSurfacePtr gridSurface : gridElements)
        {
        bvector<BeSQLite::EC::ECInstanceId> relationships = DimensionHandler::GetDimensioningRelationshipInstances(db, gridSurface->GetElementId());
        for (BeSQLite::EC::ECInstanceId instance : relationships)
            DimensionHandler::RotateDirection(db, instance, rotMatrix);
        }

    return status;
    }

END_GRIDS_NAMESPACE