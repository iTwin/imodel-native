#include "PublicApi/GridSplineSurface.h"
#include <DgnPlatform/DgnDb.h>
#include <DgnPlatform/DgnCategory.h>
#include <DgnPlatform/ElementGeometry.h>
#include <DgnPlatform/ViewController.h>

BEGIN_GRIDS_NAMESPACE
USING_NAMESPACE_BENTLEY_DGN

DEFINE_GRIDS_ELEMENT_BASE_METHODS (GridSplineSurface)

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Jonas.Valiunas                  03/2017
+---------------+---------------+---------------+---------------+---------------+------*/
GridSplineSurface::GridSplineSurface
(
CreateParams const& params
) : T_Super(params) 
    {

    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Jonas.Valiunas                  03/2017
+---------------+---------------+---------------+---------------+---------------+------*/
GridSplineSurface::GridSplineSurface
(
CreateParams const& params,
ISolidPrimitivePtr  surface
) : T_Super(params, surface) 
    {
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Haroldas.Vitunskas                  10/17
//---------------------------------------------------------------------------------------
bool GridSplineSurface::_ValidateGeometry(ISolidPrimitivePtr surface)
    {
    DgnExtrusionDetail extrDetail;
    if (!surface->TryGetDgnExtrusionDetail(extrDetail))
        return false;

    ICurvePrimitivePtr curve = *(extrDetail.m_baseCurve)->begin();

    if (nullptr == curve->GetBsplineCurveCP() &&
        nullptr == curve->GetInterpolationCurveCP())
        return false;

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Jonas.Valiunas                  03/2017
+---------------+---------------+---------------+---------------+---------------+------*/
GridSplineSurfacePtr                 GridSplineSurface::Create 
(
Dgn::SpatialLocationModelCR model,
ISolidPrimitivePtr  surface
)
    {
    return new GridSplineSurface (CreateParamsFromModel(model, QueryClassId(model.GetDgnDb())), surface);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Haroldas.Vitunskas                  06/17
//---------------------------------------------------------------------------------------
GridSplineSurfacePtr GridSplineSurface::Create(Dgn::SpatialLocationModelCR model, DgnExtrusionDetail extDetail)
    {
    return GridSplineSurface::Create(model, ISolidPrimitive::CreateDgnExtrusion(extDetail));
    }
END_GRIDS_NAMESPACE