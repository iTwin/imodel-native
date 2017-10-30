
#include <Grids/gridsApi.h>
#include <DgnPlatform/DgnDb.h>
#include <DgnPlatform/DgnCategory.h>
#include <DgnPlatform/ElementGeometry.h>
#include <DgnPlatform/ViewController.h>
#include <ConstraintSystem/ConstraintSystemApi.h>

BEGIN_GRIDS_NAMESPACE
USING_NAMESPACE_BENTLEY_DGN
USING_NAMESPACE_BUILDING

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
GridPortionCR grid
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
GridPortionCR grid
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
    Dgn::ElementIterator iterator = GetDgnDb ().Elements ().MakeIterator (GRIDS_SCHEMA (GRIDS_CLASS_GridSurface), "WHERE Axis=?");
    ECN::ECClassId relClassId = GetDgnDb ().Schemas ().GetClassId (GRIDS_SCHEMA_NAME, GRIDS_REL_GridAxisContainsGridSurfaces);
    if (BeSQLite::EC::ECSqlStatement* pStmnt = iterator.GetStatement ())
        {
        pStmnt->BindNavigationValue (1, GetElementId (), relClassId);
        }
    return iterator;
    }

END_GRIDS_NAMESPACE