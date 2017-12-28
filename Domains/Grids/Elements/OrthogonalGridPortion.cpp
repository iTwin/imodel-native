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
#include <BuildingShared/BuildingSharedApi.h>
#include <BeSQLite/BeSQLite.h>

BEGIN_GRIDS_NAMESPACE
USING_NAMESPACE_BENTLEY_DGN
USING_NAMESPACE_CONSTRAINTMODEL
USING_NAMESPACE_BUILDING
USING_NAMESPACE_BUILDING_SHARED

DEFINE_GRIDS_ELEMENT_BASE_METHODS (OrthogonalGrid)


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Jonas.Valiunas                  03/2017
+---------------+---------------+---------------+---------------+---------------+------*/
OrthogonalGrid::OrthogonalGrid
(
T_Super::CreateParams const& params
) : T_Super(params)
    {

    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Jonas.Valiunas                  10/2017
+---------------+---------------+---------------+---------------+---------------+------*/
OrthogonalGridPtr        OrthogonalGrid::CreateAndInsert
(
StandardCreateParams const& params
)
    {
    OrthogonalGridPtr thisGrid = new OrthogonalGrid (params);

    BuildingLocks_LockElementForOperation (*thisGrid, BeSQLite::DbOpcode::Insert, "Inserting orthogonal grid");

    if (!thisGrid->Insert ().IsValid())
        return nullptr;

    Dgn::DefinitionModelCR defModel = thisGrid->GetDgnDb().GetDictionaryModel ();

    OrthogonalAxisXPtr horizontalAxis = OrthogonalAxisX::CreateAndInsert (defModel, *thisGrid);
    OrthogonalAxisYPtr verticalAxis = OrthogonalAxisY::CreateAndInsert(defModel, *thisGrid);

    Dgn::SpatialLocationModelPtr subModel = thisGrid->GetSurfacesModel ();
    
    if (subModel.IsValid ())
        {
        if (BentleyStatus::ERROR == CreateAndInsertSurfaces(params, subModel.get(), *horizontalAxis, *verticalAxis))
            return nullptr;
        }
    return thisGrid;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Haroldas.Vitunskas                  04/17
//---------------------------------------------------------------------------------------
DVec3d OrthogonalGrid::FindOrthogonalFormTranslation(int elementIndex, double interval, double rotationAngle, bool isHorizontal)
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
void OrthogonalGrid::AddDimensionsToOrthogonalGrid
(
GridSurfacePtr lastElement,
GridSurfacePtr element,
double distance
)
    {
    DPlane3d elementPlane = (dynamic_cast<GridPlanarSurface *>(element.get()))->GetPlane();
    DVec3d planeNormal = elementPlane.normal;

    if (lastElement.IsValid())
        {
        DPoint3d elementOrigin = elementPlane.origin;
        DPoint3d lastPlaneOrigin;
        DPlane3d lastPlane = (dynamic_cast<GridPlanarSurface *>(lastElement.get()))->GetPlane();
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
// @bsimethod                                    Haroldas.Vitunskas                  11/17
//---------------------------------------------------------------------------------------
BentleyStatus OrthogonalGrid::CreateSurfaces(bvector<GridSurfacePtr> & allSurfaces, Dgn::SpatialLocationModelCPtr model, int count, double interval, double rotAngle, double length, double height, bool extendHeight, DVec3d extendTranslation, GridAxisCR gridAxis, bool isHorizontal)
    {
    for (int i = 0; i < count; ++i)
        {
        DgnExtrusionDetail extDetail = GeometryUtils::CreatePlaneExtrusionDetail(length + 2 * extendTranslation.Magnitude(), height);
        extDetail.m_baseCurve->TransformInPlace(Transform::From(RotMatrix::FromAxisAndRotationAngle(2, rotAngle)));
        extDetail.m_baseCurve->TransformInPlace(Transform::From(extendTranslation));

        if (extendHeight)
            extDetail.m_baseCurve->TransformInPlace(Transform::From(DVec3d::From(0.0, 0.0, -BUILDING_TOLERANCE)));

        GridPlanarSurfacePtr baseGridPlane = GridPlanarSurface::Create(*model, gridAxis, extDetail);
        if (!baseGridPlane.IsValid())
            return BentleyStatus::ERROR;

        baseGridPlane->Translate(FindOrthogonalFormTranslation(i, interval, rotAngle, isHorizontal));
        allSurfaces.push_back(baseGridPlane);
        }

    return BentleyStatus::SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Haroldas.Vitunskas                  11/17
//---------------------------------------------------------------------------------------
BentleyStatus OrthogonalGrid::CreateAndInsertSurfaces(StandardCreateParams params, Dgn::SpatialLocationModelCPtr model, GridAxisCR horizontalGridAxis, GridAxisCR verticalGridAxis)
    {
    // Adjust grid height
    double height = params.m_height;
    if (params.m_extendHeight)
        height += 2 * BUILDING_TOLERANCE;

    bvector<GridSurfacePtr> surfaces;

    BentleyStatus status = BentleyStatus::SUCCESS;

    // Create horizontal elements
    if (BentleyStatus::SUCCESS != CreateSurfaces(surfaces,
                                                 model,
                                                 params.m_horizontalCount,
                                                 params.m_horizontalInterval,
                                                 0,
                                                 params.m_length,
                                                 height,
                                                 params.m_extendHeight,
                                                 params.m_horizontalExtendTranslation,
                                                 horizontalGridAxis,
                                                 true))
        return status;

    // Create vertical elements
    if (BentleyStatus::SUCCESS != CreateSurfaces(surfaces,
                                                 model,
                                                 params.m_verticalCount,
                                                 params.m_verticalInterval,
                                                 msGeomConst_pi / 2,
                                                 params.m_length,
                                                 height,
                                                 params.m_extendHeight,
                                                 params.m_verticalExtendTranslation,
                                                 verticalGridAxis,
                                                 false))
        return status;

    // insert elements
    for (GridSurfacePtr gridSurface : surfaces)
        {
        Dgn::RepositoryStatus lockStatus = BuildingLocks_LockElementForOperation(*gridSurface, BeSQLite::DbOpcode::Insert, "Inserting gridSurface");
        if (Dgn::RepositoryStatus::Success != lockStatus)
            return BentleyStatus::ERROR;

        if (gridSurface->Insert().IsNull())
            return BentleyStatus::ERROR;
        }

    return status;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Haroldas.Vitunskas                  10/17
//---------------------------------------------------------------------------------------
Dgn::RepositoryStatus OrthogonalGrid::RotateToAngleXY(double theta, bool updateDimensions)
    {
    DgnDbR db = GetDgnDb();
    Dgn::RepositoryStatus status = RepositoryStatus::Success;

    bvector<GridSurfacePtr> gridElements;
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