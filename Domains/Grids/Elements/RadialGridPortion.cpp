/*--------------------------------------------------------------------------------------+
|
|     $Source: Grids/Elements/RadialGridPortion.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#include <Grids/gridsApi.h>
#include <DgnPlatform/DgnDb.h>
#include <DgnPlatform/DgnCategory.h>
#include <DgnPlatform/ElementGeometry.h>
#include <DgnPlatform/ViewController.h>
#include <BuildingShared/BuildingSharedApi.h>

BEGIN_GRIDS_NAMESPACE
USING_NAMESPACE_BENTLEY_DGN
USING_NAMESPACE_BUILDING
USING_NAMESPACE_BUILDING_SHARED

DEFINE_GRIDS_ELEMENT_BASE_METHODS (RadialGrid)

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Jonas.Valiunas                  03/2017
+---------------+---------------+---------------+---------------+---------------+------*/
RadialGrid::RadialGrid
(
CreateParams const& params
) : T_Super(params) 
    {
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Jonas.Valiunas                  03/2017
+---------------+---------------+---------------+---------------+---------------+------*/
RadialGridPtr        RadialGrid::Create
(
CreateParams const& params
)
    {
    return new RadialGrid (params);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Haroldas.Vitunskas                  06/17
//---------------------------------------------------------------------------------------
BentleyStatus RadialGrid::CreateAndInsertGridSurfaces(CreateParams params, Dgn::SpatialLocationModelCPtr model, GridAxisCR planeAxis, GridAxisCR arcAxis)
    {
    bvector<GridSurfacePtr> surfaces;

    double height = params.m_height;

    if (params.m_extendHeight)
        height += 2 * BUILDING_TOLERANCE;

    DgnExtrusionDetail extDetail = GeometryUtils::CreatePlaneExtrusionDetail(params.m_length, height);

    if (params.m_extendHeight)
        extDetail.m_baseCurve->TransformInPlace(Transform::From(DVec3d::From(0.0, 0.0, -BUILDING_TOLERANCE)));

    GridPlanarSurfacePtr baseGridPlane = GridPlanarSurface::Create(*model.get(), planeAxis, extDetail);
    if (!baseGridPlane.IsValid())
        return BentleyStatus::ERROR;

    if (params.m_planeCount > 0)
        surfaces.push_back (baseGridPlane);

    // Create plane grids
    for (int i = 1; i < params.m_planeCount; ++i)
        {
        GridPlanarSurfacePtr planeSurface = dynamic_cast<GridPlanarSurface *>(baseGridPlane->Clone().get());
        if (!planeSurface.IsValid())
            return BentleyStatus::ERROR;

        planeSurface->RotateXY(i * params.m_planeIterationAngle);
        surfaces.push_back(planeSurface);
        }

    // Create arc grids
    for (int i = 0; i < params.m_circularCount; ++i)
        {
        DgnExtrusionDetail extDetail = GeometryUtils::CreateArcExtrusionDetail((i + 1) * params.m_circularInterval, params.m_planeIterationAngle * params.m_planeCount, height, UnitConverter::FromFeet(CIRCULAR_GRID_EXTEND_LENGTH));
        
        if (params.m_extendHeight)
            extDetail.m_baseCurve->TransformInPlace(Transform::From(DVec3d::From(0.0, 0.0, -BUILDING_TOLERANCE)));
        
        GridArcSurfacePtr arcSurface = GridArcSurface::Create(*model.get(), arcAxis, extDetail);
        if (!arcSurface.IsValid())
            return BentleyStatus::ERROR;

        surfaces.push_back(arcSurface);
        }

    // insert elements
    for (GridSurfacePtr gridSurface : surfaces)
        {
        Dgn::RepositoryStatus lockStatus = BuildingLocks_LockElementForOperation(*gridSurface, BeSQLite::DbOpcode::Insert, "Inserting gridSurface");
        if (Dgn::RepositoryStatus::Success != lockStatus)
            return BentleyStatus::ERROR;

        if (gridSurface->Insert().IsNull())
            return BentleyStatus::ERROR;
        }

    return BentleyStatus::SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Jonas.Valiunas                     10/17
//---------------------------------------------------------------------------------------
RadialGridPtr RadialGrid::CreateAndInsert (CreateParams const& params)
    {
    RadialGridPtr thisGrid = new RadialGrid (params);

    BuildingLocks_LockElementForOperation (*thisGrid, BeSQLite::DbOpcode::Insert, "Inserting Radial grid");

    if (!thisGrid->Insert ().IsValid ())
        return nullptr;

    Dgn::DefinitionModelCR defModel = thisGrid->GetDgnDb ().GetDictionaryModel ();

    RadialAxisPtr planeAxis = RadialAxis::CreateAndInsert (defModel, *thisGrid);
    CircularAxisPtr arcAxis = CircularAxis::CreateAndInsert (defModel, *thisGrid);

    Dgn::SpatialLocationModelPtr subModel = thisGrid->GetSurfacesModel();

    if (subModel.IsValid())
        {
        if (BentleyStatus::ERROR == CreateAndInsertGridSurfaces(params, subModel.get(), *planeAxis, *arcAxis))
            return nullptr;
        }
    return thisGrid;
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Jonas.Valiunas                  01/2018
//---------------+---------------+---------------+---------------+---------------+------
Dgn::DgnDbStatus      RadialGrid::_OnUpdate
(
Dgn::DgnElementCR original
)
    {
    Dgn::DgnDbStatus status = T_Super::_OnUpdate(original);
    Transform originalTrans = original.ToGeometrySource3d()->GetPlacement().GetTransform();
    Transform newTrans = GetPlacement().GetTransform();
    if (_NeedsUpdateSurfacesOnPlacementChange() &&
    !newTrans.IsEqual(originalTrans, PLACEMENT_TOLERANCE, PLACEMENT_TOLERANCE))
        {
        //figure out the delta(diff)
        Transform inverted, diff;
        bsiTransform_invertTransform(&inverted, &originalTrans);
        bsiTransform_multiplyTransformTransform(&diff, &inverted, &newTrans);
        Dgn::DgnDbR db = GetDgnDb();
        //apply delta to each surface, just a workaround for now...
        for (DgnElementId gridSurfaceId : MakeIterator().BuildIdList<DgnElementId>())
            {
            GridSurfacePtr surface = db.Elements().GetForEdit<GridSurface>(gridSurfaceId);
            Placement3d newPlacement(surface->GetPlacement());
            newPlacement.TryApplyTransform(diff);
            surface->SetPlacement(newPlacement);
            if (RepositoryStatus::Success != BuildingLocks_LockElementForOperation(*surface, BeSQLite::DbOpcode::Update, "update GridSurface"))
                BeAssert(!"failed to acquire lock for element update");
            surface->Update();
            }
        }

    return status;
    }

END_GRIDS_NAMESPACE