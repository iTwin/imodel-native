
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

DEFINE_GRIDS_ELEMENT_BASE_METHODS (GridAxis)

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Jonas.Valiunas                  10/2017
+---------------+---------------+---------------+---------------+---------------+------*/
GridAxis::GridAxis
(
CreateParams const& params
) : T_Super(params) 
    {

    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Jonas.Valiunas                  10/2017
+---------------+---------------+---------------+---------------+---------------+------*/
GridAxis::CreateParams           GridAxis::CreateParamsFromModel
(
Dgn::DgnModelCR model,
DgnClassId classId
)
    {
    CreateParams createParams (model.GetDgnDb (), model.GetModelId (), classId);

    return createParams;
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Jonas.Valiunas                  10/2017
+---------------+---------------+---------------+---------------+---------------+------*/
GridAxisPtr                 GridAxis::CreateAndInsert
(
Dgn::DgnModelCR model,
GridCR grid
)
    {
    GridAxisPtr thisAxis = GridAxis::Create (model, grid);

    BuildingLocks_LockElementForOperation (*thisAxis, BeSQLite::DbOpcode::Insert, "Inserting grid axis");

    if (!thisAxis->Insert ().IsValid ())
        return nullptr;

    return thisAxis;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Jonas.Valiunas                  10/2017
+---------------+---------------+---------------+---------------+---------------+------*/
GridAxisPtr                 GridAxis::Create
(
Dgn::DgnModelCR model,
GridCR grid
)
    {
    GridAxisPtr thisAxis = new GridAxis (CreateParamsFromModel(model, QueryClassId(model.GetDgnDb())));

    thisAxis->SetGridId (grid.GetElementId ());

    return thisAxis;
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Jonas.Valiunas                  10/2017
//---------------+---------------+---------------+---------------+---------------+------
Dgn::ElementIterator GridAxis::MakeIterator () const
    {
    Dgn::ElementIterator iterator = GetDgnDb ().Elements ().MakeIterator (GRIDS_SCHEMA (GRIDS_CLASS_GridSurface), "WHERE Axis=?", "ORDER BY ECInstanceId ASC");
    ECN::ECClassId relClassId = GetDgnDb ().Schemas ().GetClassId (GRIDS_SCHEMA_NAME, GRIDS_REL_GridAxisContainsGridSurfaces);
    if (BeSQLite::EC::ECSqlStatement* pStmnt = iterator.GetStatement ())
        {
        pStmnt->BindNavigationValue (1, GetElementId (), relClassId);
        }
    return iterator;
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Jonas.Valiunas                  10/2017
//---------------+---------------+---------------+---------------+---------------+------
Dgn::DgnDbStatus      GridAxis::_Validate
(
) const
    {
    if (!GetGridId().IsValid()) //grid must be set
        return Dgn::DgnDbStatus::ValidationFailed;

    return Dgn::DgnDbStatus::Success;
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Jonas.Valiunas                  10/2017
//---------------+---------------+---------------+---------------+---------------+------
Dgn::DgnDbStatus      GridAxis::_OnInsert
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
Dgn::DgnDbStatus      GridAxis::_OnUpdate
(
Dgn::DgnElementCR original
)
    {
    Dgn::DgnDbStatus status = T_Super::_OnUpdate (original);
    if (status == Dgn::DgnDbStatus::Success)
        {
        return _Validate ();
        }
    return status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Jonas.Valiunas                  12/2017
+---------------+---------------+---------------+---------------+---------------+------*/
OrthogonalAxisX::OrthogonalAxisX
(
CreateParams const& params
) : T_Super(params) 
    {

    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Jonas.Valiunas                  12/2017
+---------------+---------------+---------------+---------------+---------------+------*/
OrthogonalAxisXPtr                 OrthogonalAxisX::CreateAndInsert
(
Dgn::DgnModelCR model,
OrthogonalGridCR grid
)
    {
    OrthogonalAxisXPtr thisAxis = OrthogonalAxisX::Create (model, grid);

    BuildingLocks_LockElementForOperation (*thisAxis, BeSQLite::DbOpcode::Insert, "Inserting grid axis");
    
    if (!thisAxis->InsertT<OrthogonalAxisX>().IsValid ())
        return nullptr;

    return thisAxis;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Jonas.Valiunas                  12/2017
+---------------+---------------+---------------+---------------+---------------+------*/
OrthogonalAxisXPtr                 OrthogonalAxisX::Create
(
Dgn::DgnModelCR model,
OrthogonalGridCR grid
)
    {
    OrthogonalAxisXPtr thisAxis = new OrthogonalAxisX (CreateParamsFromModel(model, QueryClassId(model.GetDgnDb())));

    thisAxis->SetGridId (grid.GetElementId ());

    return thisAxis;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Jonas.Valiunas                  12/2017
+---------------+---------------+---------------+---------------+---------------+------*/
OrthogonalAxisY::OrthogonalAxisY
(
CreateParams const& params
) : T_Super(params) 
    {

    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Jonas.Valiunas                  12/2017
+---------------+---------------+---------------+---------------+---------------+------*/
OrthogonalAxisYPtr                 OrthogonalAxisY::CreateAndInsert
(
Dgn::DgnModelCR model,
OrthogonalGridCR grid
)
    {
    OrthogonalAxisYPtr thisAxis = OrthogonalAxisY::Create (model, grid);

    BuildingLocks_LockElementForOperation (*thisAxis, BeSQLite::DbOpcode::Insert, "Inserting grid axis");

    if (!thisAxis->InsertT<OrthogonalAxisY>().IsValid())
        return nullptr;

    return thisAxis;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Jonas.Valiunas                  12/2017
+---------------+---------------+---------------+---------------+---------------+------*/
OrthogonalAxisYPtr                 OrthogonalAxisY::Create
(
Dgn::DgnModelCR model,
OrthogonalGridCR grid
)
    {
    OrthogonalAxisYPtr thisAxis = new OrthogonalAxisY (CreateParamsFromModel(model, QueryClassId(model.GetDgnDb())));

    thisAxis->SetGridId (grid.GetElementId ());

    return thisAxis;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Jonas.Valiunas                  12/2017
+---------------+---------------+---------------+---------------+---------------+------*/
CircularAxis::CircularAxis
(
CreateParams const& params
) : T_Super(params) 
    {

    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Jonas.Valiunas                  12/2017
+---------------+---------------+---------------+---------------+---------------+------*/
CircularAxisPtr                 CircularAxis::CreateAndInsert
(
Dgn::DgnModelCR model,
RadialGridCR grid
)
    {
    CircularAxisPtr thisAxis = CircularAxis::Create (model, grid);

    BuildingLocks_LockElementForOperation (*thisAxis, BeSQLite::DbOpcode::Insert, "Inserting grid axis");

    if (!thisAxis->InsertT<CircularAxis>().IsValid())
        return nullptr;

    return thisAxis;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Jonas.Valiunas                  12/2017
+---------------+---------------+---------------+---------------+---------------+------*/
CircularAxisPtr                 CircularAxis::Create
(
Dgn::DgnModelCR model,
RadialGridCR grid
)
    {
    CircularAxisPtr thisAxis = new CircularAxis (CreateParamsFromModel(model, QueryClassId(model.GetDgnDb())));

    thisAxis->SetGridId (grid.GetElementId ());

    return thisAxis;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Jonas.Valiunas                  12/2017
+---------------+---------------+---------------+---------------+---------------+------*/
RadialAxis::RadialAxis
(
CreateParams const& params
) : T_Super(params) 
    {

    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Jonas.Valiunas                  12/2017
+---------------+---------------+---------------+---------------+---------------+------*/
RadialAxisPtr                 RadialAxis::CreateAndInsert
(
Dgn::DgnModelCR model,
RadialGridCR grid
)
    {
    RadialAxisPtr thisAxis = RadialAxis::Create (model, grid);

    BuildingLocks_LockElementForOperation (*thisAxis, BeSQLite::DbOpcode::Insert, "Inserting grid axis");

    if (!thisAxis->InsertT<RadialAxis>().IsValid())
        return nullptr;

    return thisAxis;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Jonas.Valiunas                  12/2017
+---------------+---------------+---------------+---------------+---------------+------*/
RadialAxisPtr                 RadialAxis::Create
(
Dgn::DgnModelCR model,
RadialGridCR grid
)
    {
    RadialAxisPtr thisAxis = new RadialAxis (CreateParamsFromModel(model, QueryClassId(model.GetDgnDb())));

    thisAxis->SetGridId (grid.GetElementId ());

    return thisAxis;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Jonas.Valiunas                  12/2017
+---------------+---------------+---------------+---------------+---------------+------*/
GeneralGridAxis::GeneralGridAxis
(
CreateParams const& params
) : T_Super(params) 
    {

    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Jonas.Valiunas                  12/2017
+---------------+---------------+---------------+---------------+---------------+------*/
GeneralGridAxisPtr                 GeneralGridAxis::CreateAndInsert
(
Dgn::DgnModelCR model,
GridCR grid
)
    {
    GeneralGridAxisPtr thisAxis = GeneralGridAxis::Create (model, grid);

    BuildingLocks_LockElementForOperation (*thisAxis, BeSQLite::DbOpcode::Insert, "Inserting grid axis");

    if (!thisAxis->InsertT<GeneralGridAxis>().IsValid())
        return nullptr;

    return thisAxis;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Jonas.Valiunas                  12/2017
+---------------+---------------+---------------+---------------+---------------+------*/
GeneralGridAxisPtr                 GeneralGridAxis::Create
(
Dgn::DgnModelCR model,
GridCR grid
)
    {
    GeneralGridAxisPtr thisAxis = new GeneralGridAxis (CreateParamsFromModel(model, QueryClassId(model.GetDgnDb())));

    thisAxis->SetGridId (grid.GetElementId ());

    return thisAxis;
    }
END_GRIDS_NAMESPACE