#include "PublicApi/SketchGridPortion.h"
#include <DgnPlatform/DgnDb.h>
#include <DgnPlatform/DgnCategory.h>
#include <DgnPlatform/ElementGeometry.h>
#include <DgnPlatform/ViewController.h>
#include <GeometryUtils.h>

BEGIN_GRIDS_NAMESPACE
USING_NAMESPACE_BENTLEY_DGN
USING_NAMESPACE_BUILDING

DEFINE_GRIDS_ELEMENT_BASE_METHODS (SketchGridPortion)

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Jonas.Valiunas                  03/2017
+---------------+---------------+---------------+---------------+---------------+------*/
SketchGridPortion::SketchGridPortion
(
T_Super::CreateParams const& params
) : T_Super(params) 
    {

    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Jonas.Valiunas                  03/2017
+---------------+---------------+---------------+---------------+---------------+------*/
SketchGridPortionPtr        SketchGridPortion::Create
(
Dgn::DgnModelCR model
)
    {
    return new SketchGridPortion (CreateParamsFromModel(model, QueryClassId(model.GetDgnDb())));
    }

END_GRIDS_NAMESPACE