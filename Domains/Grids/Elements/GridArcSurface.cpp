
#include <Grids/gridsApi.h>
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
GridAxisCPtr gridAxis,
ISolidPrimitivePtr  surface
) : T_Super(params, gridAxis, surface) 
    {
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Haroldas.Vitunskas                  10/17
//---------------------------------------------------------------------------------------
bool GridArcSurface::_ValidateGeometry(ISolidPrimitivePtr surface)
    {
    DgnExtrusionDetail extrDetail;
    if (!surface->TryGetDgnExtrusionDetail(extrDetail))
        return false;

    ICurvePrimitivePtr curve = *(extrDetail.m_baseCurve)->begin();
    if (nullptr == curve->GetArcCP())
        return false;

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Jonas.Valiunas                  03/2017
+---------------+---------------+---------------+---------------+---------------+------*/
GridArcSurfacePtr                 GridArcSurface::Create 
(
Dgn::SpatialLocationModelCR model,
GridAxisCPtr gridAxis,
ISolidPrimitivePtr  surface
)
    {
    return new GridArcSurface (CreateParamsFromModel(model, QueryClassId(model.GetDgnDb())), gridAxis, surface);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Haroldas.Vitunskas                  06/17
//---------------------------------------------------------------------------------------
GridArcSurfacePtr GridArcSurface::Create
(
Dgn::SpatialLocationModelCR model, 
GridAxisCPtr gridAxis,
DgnExtrusionDetail extDetail
)
    {
    return GridArcSurface::Create(model, gridAxis, ISolidPrimitive::CreateDgnExtrusion(extDetail));
    }
END_GRIDS_NAMESPACE