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

DEFINE_GRIDS_ELEMENT_BASE_METHODS (GridPortion)


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Jonas.Valiunas                  09/2017
+---------------+---------------+---------------+---------------+---------------+------*/
GridPortion::GridPortion
(
T_Super::CreateParams const& params,
DVec3d          normal
) : T_Super(params) 
    {
    if (!m_categoryId.IsValid () && m_classId.IsValid()) //really odd tests in platform.. attempts to create elements with 0 class id and 0 categoryId. NEEDS WORK: PLATFORM
        {
        Dgn::DgnCategoryId catId = SpatialCategory::QueryCategoryId (params.m_dgndb.GetDictionaryModel (), GRIDS_CATEGORY_CODE_Uncategorized);
        if (catId.IsValid ())
            DoSetCategoryId (catId);
        }
    SetPropertyValue (prop_Normal (), normal);
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Jonas.Valiunas                  03/2017
+---------------+---------------+---------------+---------------+---------------+------*/
GridPortion::CreateParams           GridPortion::CreateParamsFromModel
(
Dgn::DgnModelCR model,
DgnClassId classId
)
    {
    DgnElement::CreateParams createParams (model.GetDgnDb (), model.GetModelId (), classId);

    return CreateParams(createParams);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Jonas.Valiunas                  03/2017
+---------------+---------------+---------------+---------------+---------------+------*/
GridPortionPtr                 GridPortion::Create 
(
Dgn::DgnModelCR model,
DVec3d          normal
)
    {
    return new GridPortion (CreateParamsFromModel(model, QueryClassId(model.GetDgnDb())), normal);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Jonas.Valiunas                     09/17
//---------------------------------------------------------------------------------------
DPlane3d GridPortion::GetPlane 
(
) const
    {
    DPlane3d plane;
    plane.Zero ();
    plane.normal = DVec3d::From (GetPropertyValueDPoint3d (prop_Normal ()));
    return plane;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Haroldas.Vitunskas                  06/17
//---------------------------------------------------------------------------------------
RepositoryStatus GridPortion::RotateToAngleXY (double theta)
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
RepositoryStatus GridPortion::TranslateToPoint (DPoint3d point)
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
BentleyStatus GridPortion::GetGridRotationAngleXY(double& angle) const
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
Dgn::ElementIterator GridPortion::MakeIterator () const
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
Dgn::SpatialLocationModelPtr    GridPortion::GetSurfacesModel
(
) const
    {
    Dgn::DgnModelPtr subModel = GetSubModel ();

    if (subModel.IsValid ())
        {
        Dgn::SpatialLocationModelPtr surfacesModel = dynamic_cast<Dgn::SpatialLocationModel*>(subModel.get ());
        BeAssert (surfacesModel.IsValid () && "GridPortion submodel is not spatialLocationModel!");
        return surfacesModel;
        }
    return CreateSubModel ();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Jonas.Valiunas                  10/2017
+---------------+---------------+---------------+---------------+---------------+------*/
Dgn::SpatialLocationModelPtr    GridPortion::CreateSubModel
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
Dgn::ElementIterator GridPortion::MakeAxesIterator () const
    {
    Dgn::ElementIterator iterator = GetDgnDb ().Elements ().MakeIterator (GRIDS_SCHEMA (GRIDS_CLASS_GridAxis), "WHERE Grid=?", "ORDER BY ECInstanceId ASC");
    ECN::ECClassId relClassId = GetDgnDb ().Schemas ().GetClassId (GRIDS_SCHEMA_NAME, GRIDS_REL_GridPortionHasAxes);
    if (BeSQLite::EC::ECSqlStatement* pStmnt = iterator.GetStatement ())
        {
        pStmnt->BindNavigationValue (1, GetElementId (), relClassId);
        }
    return iterator;
    }
//---------------------------------------------------------------------------------------
// @bsimethod                                    Haroldas.Vitunskas                  10/17
//---------------------------------------------------------------------------------------
GridPortionPtr GridPortion::TryGet(Dgn::DgnDbR db, Dgn::DgnElementId parentId, Utf8CP gridName)
    {
    return db.Elements().GetForEdit<Grids::GridPortion>(BuildingElementsUtils::GetElementIdByParentElementAuthorityAndName(db,
                                                                                                                    GRIDS_AUTHORITY_GridPortion,
                                                                                                                    parentId,
                                                                                                                    gridName));
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Haroldas.Vitunskas                  10/17
//---------------------------------------------------------------------------------------
Utf8CP  GridPortion::GetName() const
    {
    return GetCode().GetValueUtf8CP();
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Jonas.Valiunas                  10/2017
//---------------+---------------+---------------+---------------+---------------+------
Dgn::DgnDbStatus      GridPortion::Validate
(
) const
    {
    DPlane3d plane = GetPlane ();
    if (bsiDPlane3d_isZero(&plane)) //plane must be set
        return Dgn::DgnDbStatus::ValidationFailed;

    return Dgn::DgnDbStatus::Success;
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Jonas.Valiunas                  10/2017
//---------------+---------------+---------------+---------------+---------------+------
Dgn::DgnDbStatus      GridPortion::_OnInsert
(
)
    {
    Dgn::DgnDbStatus status = T_Super::_OnInsert ();
    if (status == Dgn::DgnDbStatus::Success)
        {
        return Validate ();
        }
    return status;
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Jonas.Valiunas                  10/2017
//---------------+---------------+---------------+---------------+---------------+------
Dgn::DgnDbStatus      GridPortion::_OnUpdate
(
    Dgn::DgnElementCR original
)
    {
    Dgn::DgnDbStatus status = T_Super::_OnUpdate(original);
    if (status == Dgn::DgnDbStatus::Success)
        {
        return Validate();
        }
    return status;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Haroldas.Vitunskas                  11/17
//---------------------------------------------------------------------------------------
Dgn::DgnDbStatus GridPortion::_OnDelete() const
    {
    for (DgnElementId axisId : MakeAxesIterator().BuildIdList<DgnElementId>())
        GetDgnDb().Elements().Delete(axisId);

    GetSurfacesModel()->Delete();

    return T_Super::_OnDelete();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Jonas.Valiunas                  05/17
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   GridPortion::IntersectGridSurface 
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

END_GRIDS_NAMESPACE

