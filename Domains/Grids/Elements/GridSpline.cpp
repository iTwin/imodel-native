#include "PublicApi/GridSpline.h"
#include <DgnPlatform/DgnDb.h>
#include <DgnPlatform/DgnCategory.h>
#include <DgnPlatform/ElementGeometry.h>
#include <DgnPlatform/ViewController.h>

BEGIN_GRIDS_NAMESPACE
USING_NAMESPACE_BENTLEY_DGN

DEFINE_GRIDS_ELEMENT_BASE_METHODS (GridSpline)

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Jonas.Valiunas                  04/2017
+---------------+---------------+---------------+---------------+---------------+------*/
GridSpline::GridSpline
(
CreateParams const& params
) : T_Super(params) 
    {

    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Jonas.Valiunas                  04/2017
+---------------+---------------+---------------+---------------+---------------+------*/
GridSpline::GridSpline
(
CreateParams const& params,
ICurvePrimitivePtr  curve
) : T_Super(params, curve) 
    {
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Jonas.Valiunas                  04/2017
+---------------+---------------+---------------+---------------+---------------+------*/
GridSplinePtr                 GridSpline::Create 
(
Dgn::DgnModelCR model,
ICurvePrimitivePtr  curve
)
    {
    return new GridSpline (CreateParamsFromModel(model, QueryClassId(model.GetDgnDb())), curve);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Jonas.Valiunas                  04/2017
+---------------+---------------+---------------+---------------+---------------+------*/
void            GridSpline::_CopyFrom(Dgn::DgnElementCR source)
    {
    T_Super::_CopyFrom(source);
    }

END_GRIDS_NAMESPACE