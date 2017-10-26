#include "PublicApi/GridAxis.h"
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
    GridAxisPtr thisAxis = new GridAxis (CreateParamsFromModel(model, QueryClassId(model.GetDgnDb())));

    thisAxis->SetGridId (grid.GetElementId ());

    BuildingLocks_LockElementForOperation (*thisAxis, BeSQLite::DbOpcode::Insert, "Inserting grid axis");

    if (!thisAxis->Insert ().IsValid ())
        return nullptr;

    return thisAxis;
    }


END_GRIDS_NAMESPACE