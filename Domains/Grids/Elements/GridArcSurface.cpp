#include "PublicApi/GridArcSurface.h"
#include <DgnPlatform/DgnDb.h>
#include <DgnPlatform/DgnCategory.h>
#include <DgnPlatform/ElementGeometry.h>
#include <DgnPlatform/ViewController.h>

BEGIN_GRIDS_NAMESPACE
USING_NAMESPACE_BENTLEY_DGN

DEFINE_GRIDS_ELEMENT_BASE_METHODS (GridArcSurface)

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Jonas.Valiunas                  03/2017
+---------------+---------------+---------------+---------------+---------------+------*/
GridArcSurface::GridArcSurface
(
CreateParams const& params
) : T_Super(params) 
    {

    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Jonas.Valiunas                  03/2017
+---------------+---------------+---------------+---------------+---------------+------*/
GridArcSurface::GridArcSurface
(
CreateParams const& params,
CurveVectorPtr  surfaceVector
) : T_Super(params, surfaceVector) 
    {
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Jonas.Valiunas                  03/2017
+---------------+---------------+---------------+---------------+---------------+------*/
GridArcSurface::GridArcSurface
(
CreateParams const& params,
ISolidPrimitivePtr  surface
) : T_Super(params, surface) 
    {
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Jonas.Valiunas                  03/2017
+---------------+---------------+---------------+---------------+---------------+------*/
GridArcSurfacePtr                 GridArcSurface::Create 
(
Dgn::DgnModelCR model,
CurveVectorPtr  surfaceVector
)
    {
    return new GridArcSurface (CreateParamsFromModel(model, QueryClassId(model.GetDgnDb())), surfaceVector);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Jonas.Valiunas                  03/2017
+---------------+---------------+---------------+---------------+---------------+------*/
GridArcSurfacePtr                 GridArcSurface::Create 
(
Dgn::DgnModelCR model,
ISolidPrimitivePtr  surface
)
    {
    return new GridArcSurface (CreateParamsFromModel(model, QueryClassId(model.GetDgnDb())), surface);
    }
END_GRIDS_NAMESPACE