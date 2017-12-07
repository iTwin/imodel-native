/*--------------------------------------------------------------------------------------+
|
|     $Source: Grids/Elements/GridPortion.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <Grids/gridsApi.h>
#include <DgnPlatform/DgnDb.h>
#include <DgnPlatform/DgnCategory.h>
#include <DgnPlatform/ElementGeometry.h>
#include <DgnPlatform/ViewController.h>
#include <ConstraintSystem/ConstraintSystemApi.h>
//#include <DimensionHandler.h>

BEGIN_GRIDS_NAMESPACE
USING_NAMESPACE_BENTLEY_DGN
USING_NAMESPACE_CONSTRAINTMODEL
USING_NAMESPACE_BUILDING

DEFINE_GRIDS_ELEMENT_BASE_METHODS (Grid)
DEFINE_GRIDS_ELEMENT_BASE_METHODS (ElevationGrid)
DEFINE_GRIDS_ELEMENT_BASE_METHODS (PlanGrid)


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Jonas.Valiunas                  09/2017
+---------------+---------------+---------------+---------------+---------------+------*/
Grid::Grid
(
T_Super::CreateParams const& params
) : T_Super(params) 
    {
    if (!m_categoryId.IsValid () && m_classId.IsValid()) //really odd tests in platform.. attempts to create elements with 0 class id and 0 categoryId. NEEDS WORK: PLATFORM
        {
        Dgn::DgnCategoryId catId = SpatialCategory::QueryCategoryId (params.m_dgndb.GetDictionaryModel (), GRIDS_CATEGORY_CODE_Uncategorized);
        if (catId.IsValid ())
            DoSetCategoryId (catId);
        }
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Jonas.Valiunas                  03/2017
+---------------+---------------+---------------+---------------+---------------+------*/
Grid::CreateParams           Grid::CreateParamsFromModel
(
Dgn::DgnModelCR model,
DgnClassId classId
)
    {
    DgnElement::CreateParams createParams (model.GetDgnDb (), model.GetModelId (), classId);

    return CreateParams(createParams);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Jonas.Valiunas                     09/17
//---------------------------------------------------------------------------------------
DPlane3d PlanGrid::GetPlane 
(
) const
    {
    DPlane3d plane;
    bsiDPlane3d_initFromOriginAndNormalXYZXYZ (&plane, 0.0, 0.0, 0.0, 0.0, 0.0, 1.0);
    YawPitchRollAngles angles = GetPlacement().GetAngles ();
    RotMatrix rotMatrix = angles.ToRotMatrix ();
    rotMatrix.Multiply (plane.normal);
    return plane;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Haroldas.Vitunskas                  06/17
//---------------------------------------------------------------------------------------
RepositoryStatus Grid::RotateToAngleXY (double theta)
    {
    RepositoryStatus status = RepositoryStatus::Success;
    for (Dgn::ElementIteratorEntry pIterEntry : MakeIterator ())
        {
        GridSurfacePtr gridSurface = GetDgnDb ().Elements ().GetForEdit<GridSurface> (pIterEntry.GetElementId ());
        gridSurface->RotateXY (theta);
        if (RepositoryStatus::Success != (status = BuildingLocks_LockElementForOperation (*gridSurface, BeSQLite::DbOpcode::Update, "update GridSurface")))
            return status;
        gridSurface->Update ();
        }
    return status;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Haroldas.Vitunskas                  06/17
//---------------------------------------------------------------------------------------
RepositoryStatus Grid::TranslateToPoint (DPoint3d point)
    {
    RepositoryStatus status = RepositoryStatus::Success;
    Dgn::ElementIterator gridElements = MakeIterator ();

    GridSurfaceCPtr firstGridElem = GetDgnDb ().Elements ().Get<GridSurface> ((*gridElements.begin()).GetElementId ());

    DVec3d translation = DVec3d::FromStartEnd (firstGridElem->GetPlacement ().GetOrigin (), point);

    for (Dgn::ElementIteratorEntry pIterEntry : gridElements)
        {
        GridSurfacePtr gridSurface = GetDgnDb ().Elements ().GetForEdit<GridSurface> (pIterEntry.GetElementId());
        gridSurface->Translate (translation);
        if (RepositoryStatus::Success != (status = BuildingLocks_LockElementForOperation (*gridSurface, BeSQLite::DbOpcode::Update, "update GridSurface")))
            return status;
        gridSurface->Update ();
        }
    return status;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Haroldas.Vitunskas                  10/17
//---------------------------------------------------------------------------------------
BentleyStatus Grid::GetGridRotationAngleXY(double& angle) const
    {
    bvector<DgnElementId> gridElementIds = MakeIterator().BuildIdList<DgnElementId>();
    if (gridElementIds.empty() || !gridElementIds.front().IsValid())
        return BentleyStatus::ERROR;

    GridSurfaceCPtr firstElem = GetDgnDb().Elements().Get<GridSurface>(gridElementIds.front());
    angle = GeometryUtils::PlacementToAngleXY(firstElem->GetPlacement());

    return BentleyStatus::SUCCESS;
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Jonas.Valiunas                  10/17
//---------------+---------------+---------------+---------------+---------------+------
Dgn::ElementIterator Grid::MakeIterator () const
    {
    Dgn::ElementIterator iterator;
    if (GetSubModelId ().IsValid ())
        {
        iterator = GetDgnDb ().Elements ().MakeIterator (GRIDS_SCHEMA (GRIDS_CLASS_GridSurface), "WHERE Model.Id=?", "ORDER BY ECInstanceId ASC");
        iterator.GetStatement ()->BindId (1, GetSubModelId ());
        }
    return iterator;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Jonas.Valiunas                  10/2017
+---------------+---------------+---------------+---------------+---------------+------*/
Dgn::SpatialLocationModelPtr    Grid::GetSurfacesModel
(
) const
    {
    Dgn::DgnModelPtr subModel = GetSubModel ();

    if (subModel.IsValid ())
        {
        Dgn::SpatialLocationModelPtr surfacesModel = dynamic_cast<Dgn::SpatialLocationModel*>(subModel.get ());
        BeAssert (surfacesModel.IsValid () && "Grid submodel is not spatialLocationModel!");
        return surfacesModel;
        }
    return CreateSubModel ();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Jonas.Valiunas                  10/2017
+---------------+---------------+---------------+---------------+---------------+------*/
Dgn::SpatialLocationModelPtr    Grid::CreateSubModel
(
) const
    {
    Dgn::SpatialLocationModelPtr model = SpatialLocationModel::Create (*this);
    if (!model.IsValid ())
        return nullptr;

    Dgn::IBriefcaseManager::Request req;
    GetDgnDb().BriefcaseManager ().PrepareForModelInsert (req, *model, Dgn::IBriefcaseManager::PrepareAction::Acquire);

    if (Dgn::DgnDbStatus::Success != model->Insert ())
        return nullptr;

    return model;
    }


//--------------------------------------------------------------------------------------
// @bsimethod                                    Jonas.Valiunas                  10/2017
//---------------+---------------+---------------+---------------+---------------+------
Dgn::ElementIterator Grid::MakeAxesIterator () const
    {
    Dgn::ElementIterator iterator = GetDgnDb ().Elements ().MakeIterator (GRIDS_SCHEMA (GRIDS_CLASS_GridAxis), "WHERE Grid=?", "ORDER BY ECInstanceId ASC");
    ECN::ECClassId relClassId = GetDgnDb ().Schemas ().GetClassId (GRIDS_SCHEMA_NAME, GRIDS_REL_GridHasAxes);
    if (BeSQLite::EC::ECSqlStatement* pStmnt = iterator.GetStatement ())
        {
        pStmnt->BindNavigationValue (1, GetElementId (), relClassId);
        }
    return iterator;
    }
//---------------------------------------------------------------------------------------
// @bsimethod                                    Haroldas.Vitunskas                  10/17
//---------------------------------------------------------------------------------------
GridPtr Grid::TryGet(Dgn::DgnDbR db, Dgn::DgnElementId parentId, Utf8CP gridName)
    {
    return db.Elements().GetForEdit<Grids::Grid>(BuildingElementsUtils::GetElementIdByParentElementAuthorityAndName(db,
                                                                                                                    GRIDS_AUTHORITY_Grid,
                                                                                                                    parentId,
                                                                                                                    gridName));
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Haroldas.Vitunskas                  10/17
//---------------------------------------------------------------------------------------
Utf8CP  Grid::GetName() const
    {
    return GetCode().GetValueUtf8CP();
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Jonas.Valiunas                  10/2017
//---------------+---------------+---------------+---------------+---------------+------
Dgn::DgnDbStatus      Grid::_Validate
(
) const
    {
    return Dgn::DgnDbStatus::Success;
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Jonas.Valiunas                  10/2017
//---------------+---------------+---------------+---------------+---------------+------
Dgn::DgnDbStatus      Grid::_OnInsert
(
)
    {
    Dgn::DgnDbStatus status = T_Super::_OnInsert ();
    if (status == Dgn::DgnDbStatus::Success)
        {
        return _Validate ();
        }
    return status;
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Jonas.Valiunas                  10/2017
//---------------+---------------+---------------+---------------+---------------+------
Dgn::DgnDbStatus      Grid::_OnUpdate
(
    Dgn::DgnElementCR original
)
    {
    Dgn::DgnDbStatus status = T_Super::_OnUpdate(original);
    if (status == Dgn::DgnDbStatus::Success)
        {
        return _Validate ();
        }
    return status;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Haroldas.Vitunskas                  11/17
//---------------------------------------------------------------------------------------
Dgn::DgnDbStatus Grid::_OnDelete() const
    {
    for (DgnElementId axisId : MakeAxesIterator().BuildIdList<DgnElementId>())
        GetDgnDb().Elements().Delete(axisId);

    GetSurfacesModel()->Delete();

    return T_Super::_OnDelete();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Jonas.Valiunas                  05/17
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   Grid::IntersectGridSurface 
(
GridSurfaceCPtr surface, 
Dgn::DgnModelCR targetModel
) const
    {
    Dgn::DgnModelPtr model = GetSubModel ();
    if (!model.IsValid ())
        {
        return ERROR;
        }

    Dgn::DgnDbR db = model->GetDgnDb ();

    for (Dgn::ElementIteratorEntry elementEntry : model->MakeIterator ())
        {
        GridSurfaceCPtr innerSurface = db.Elements ().Get<GridSurface> (elementEntry.GetElementId ());
        if (innerSurface.IsValid())
            {
            GridSurfaceCreatesGridCurveHandler::Insert (db, innerSurface, surface, targetModel);
            }
        }

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Jonas.Valiunas                  12/2017
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus                   ElevationGrid::ValidateSurfaces
(
bvector<CurveVectorPtr> const& surfaces
)
    {
    DPlane3d zPlane;
    zPlane.origin = DPoint3d::FromZero ();
    zPlane.normal = DVec3d::From (0.0, 0.0, 1.0);
    for (CurveVectorPtr gridPlaneGeom : surfaces)
        {
        DPlane3d planeThis;
        Transform localToWorld, worldToLocal;
        DRange3d range;

        if (!gridPlaneGeom->IsPlanar (localToWorld, worldToLocal, range))
            return ERROR;

        bsiTransform_getOriginAndVectors (&localToWorld, &planeThis.origin, NULL, NULL, &planeThis.normal);

        //if planes are not parallel to Z, then fail
        if (!bsiDVec3d_areParallelTolerance (&planeThis.normal, &zPlane.normal, BUILDING_TOLERANCE))
            {
            return ERROR;
            }
        }
    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Jonas.Valiunas                  12/2017
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus                   ElevationGrid::CreateElevationGridPlanes
(
bvector<CurveVectorPtr> const& surfaces,
GridAxisCR gridAxis,
bool createDimensions
)
    {
    BentleyStatus status = BentleyStatus::ERROR;
    Dgn::SpatialLocationModelPtr subModel = GetSurfacesModel ();

    if (subModel.IsValid ())
        {
        status = BentleyStatus::SUCCESS;

        //not putting into same method yet.. will do as GridAxis element is introduced.
        GridPlanarSurfacePtr lastGridPlane = nullptr;
        DPlane3d planeLast;
        for (CurveVectorPtr gridPlaneGeom : surfaces)
            {
            GridPlanarSurfacePtr gridPlane = GridPlanarSurface::Create (*subModel, gridAxis, gridPlaneGeom);
            BuildingLocks_LockElementForOperation (*gridPlane, BeSQLite::DbOpcode::Insert, "Inserting gridSurface");
            gridPlane->Insert ();
            DPlane3d planeThis;
            planeThis = gridPlane->GetPlane ();
            if (lastGridPlane.IsValid () && createDimensions)
                {
                DimensionHandler::Insert (GetDgnDb(), lastGridPlane->GetElementId (), gridPlane->GetElementId (), 0, 0, planeThis.normal, bsiDPlane3d_evaluate (&planeLast, &planeThis.origin));
                }
            planeLast = planeThis;
            lastGridPlane = gridPlane;
            }
        }
    return status;
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Jonas.Valiunas                  12/2017
+---------------+---------------+---------------+---------------+---------------+------*/
ElevationGridPtr        ElevationGrid::CreateAndInsertWithSurfaces
(
CreateParams const& params,
bvector<CurveVectorPtr> const& surfaces,
bool createDimensions
)
    {
    if (BentleyStatus::SUCCESS != ValidateSurfaces (surfaces))
        return nullptr;

    ElevationGridPtr thisGrid = new ElevationGrid (params);

    BuildingLocks_LockElementForOperation (*thisGrid, BeSQLite::DbOpcode::Insert, "Inserting elevation grid");
    if (!thisGrid->Insert ().IsValid ())
        return nullptr;

    Dgn::DefinitionModelCR defModel = thisGrid->GetDgnDb ().GetDictionaryModel ();

    GridAxisPtr horizontalAxis = GridAxis::CreateAndInsert(defModel, *thisGrid);
    
    if (BentleyStatus::SUCCESS != thisGrid->CreateElevationGridPlanes (surfaces, *horizontalAxis, createDimensions))
        BeAssert (!"error inserting gridSurfaces into elevation grid.. shouldn't get here..");
    return thisGrid;
    }



END_GRIDS_NAMESPACE

