#include "PublicApi/GridArc.h"
#include <DgnPlatform/DgnDb.h>
#include <DgnPlatform/DgnCategory.h>
#include <DgnPlatform/ElementGeometry.h>
#include <DgnPlatform/ViewController.h>

BEGIN_GRIDS_NAMESPACE
USING_NAMESPACE_BENTLEY_DGN

DEFINE_GRIDS_ELEMENT_BASE_METHODS (GridArc)

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Jonas.Valiunas                  04/2017
+---------------+---------------+---------------+---------------+---------------+------*/
GridArc::GridArc
(
CreateParams const& params
) : T_Super(params) 
    {

    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Jonas.Valiunas                  04/2017
+---------------+---------------+---------------+---------------+---------------+------*/
GridArc::GridArc
(
CreateParams const& params,
ICurvePrimitivePtr  curve
) : T_Super(params, curve) 
    {
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Jonas.Valiunas                  04/2017
+---------------+---------------+---------------+---------------+---------------+------*/
GridArcPtr                 GridArc::Create 
(
Dgn::DgnModelCR model,
ICurvePrimitivePtr  curve
)
    {
    return new GridArc (CreateParamsFromModel(model, QueryClassId(model.GetDgnDb())), curve);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Jonas.Valiunas                  04/2017
+---------------+---------------+---------------+---------------+---------------+------*/
void            GridArc::_CopyFrom(Dgn::DgnElementCR source)
    {
    T_Super::_CopyFrom(source);
    }

END_GRIDS_NAMESPACE