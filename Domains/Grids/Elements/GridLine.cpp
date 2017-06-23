#include "PublicApi/GridLine.h"
#include <DgnPlatform/DgnDb.h>
#include <DgnPlatform/DgnCategory.h>
#include <DgnPlatform/ElementGeometry.h>
#include <DgnPlatform/ViewController.h>

BEGIN_GRIDS_NAMESPACE
USING_NAMESPACE_BENTLEY_DGN

DEFINE_GRIDS_ELEMENT_BASE_METHODS (GridLine)

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Jonas.Valiunas                  04/2017
+---------------+---------------+---------------+---------------+---------------+------*/
GridLine::GridLine
(
CreateParams const& params
) : T_Super(params) 
    {

    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Jonas.Valiunas                  04/2017
+---------------+---------------+---------------+---------------+---------------+------*/
GridLine::GridLine
(
CreateParams const& params,
ICurvePrimitivePtr  curve
) : T_Super(params, curve) 
    {
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Jonas.Valiunas                  04/2017
+---------------+---------------+---------------+---------------+---------------+------*/
GridLinePtr                 GridLine::Create 
(
Dgn::DgnModelCR model,
ICurvePrimitivePtr  curve
)
    {
    return new GridLine (CreateParamsFromModel(model, QueryClassId(model.GetDgnDb())), curve);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Jonas.Valiunas                  04/2017
+---------------+---------------+---------------+---------------+---------------+------*/
void            GridLine::_CopyFrom(Dgn::DgnElementCR source)
    {
    T_Super::_CopyFrom(source);
    }

END_GRIDS_NAMESPACE