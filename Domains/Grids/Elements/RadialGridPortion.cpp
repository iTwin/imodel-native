#include "PublicApi/RadialGridPortion.h"
#include <DgnPlatform/DgnDb.h>
#include <DgnPlatform/DgnCategory.h>
#include <DgnPlatform/ElementGeometry.h>
#include <DgnPlatform/ViewController.h>

BEGIN_GRIDS_NAMESPACE
USING_NAMESPACE_BENTLEY_DGN

DEFINE_GRIDS_ELEMENT_BASE_METHODS (RadialGridPortion)

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Jonas.Valiunas                  03/2017
+---------------+---------------+---------------+---------------+---------------+------*/
RadialGridPortion::RadialGridPortion
(
CreateParams const& params
) : T_Super(params) 
    {

    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Jonas.Valiunas                  03/2017
+---------------+---------------+---------------+---------------+---------------+------*/
RadialGridPortionPtr        RadialGridPortion::Create
(
Dgn::DgnModelCR model
)
    {
    return new RadialGridPortion (CreateParamsFromModel(model, QueryClassId(model.GetDgnDb())));
    }


END_GRIDS_NAMESPACE