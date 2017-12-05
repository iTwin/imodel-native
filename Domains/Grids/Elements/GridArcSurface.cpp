
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
ISolidPrimitivePtr  surface
) : T_Super(params, surface) 
    {
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Haroldas.Vitunskas                  10/17
//---------------------------------------------------------------------------------------
bool GridArcSurface::_ValidateGeometry(ISolidPrimitivePtr surface) const
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
GridAxisCR gridAxis,
ISolidPrimitivePtr  surface
)
    {
    GridArcSurfacePtr gridSurface =  new GridArcSurface (CreateParamsFromModelAxisClassId (model, gridAxis, QueryClassId(model.GetDgnDb())), surface);
    if (gridSurface.IsNull() || DgnDbStatus::Success != gridSurface->_Validate())
        return nullptr;

    return gridSurface;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Haroldas.Vitunskas                  06/17
//---------------------------------------------------------------------------------------
GridArcSurfacePtr GridArcSurface::Create
(
Dgn::SpatialLocationModelCR model, 
GridAxisCR gridAxis,
DgnExtrusionDetail extDetail
)
    {
    return GridArcSurface::Create(model, gridAxis, ISolidPrimitive::CreateDgnExtrusion(extDetail));
    }


//--------------------------------------------------------------------------------------
// @bsimethod                                    Jonas.Valiunas                  10/2017
//---------------+---------------+---------------+---------------+---------------+------
DgnDbStatus      GridArcSurface::_Validate
(
) const
    {
    DgnDbStatus status = T_Super::_Validate ();
    if (status == DgnDbStatus::Success)
        {
        GeometryCollection geomData = *ToGeometrySource ();

        if (geomData.begin () == geomData.end ())
            return DgnDbStatus::ValidationFailed;

        ISolidPrimitivePtr solidPrimitive = (*geomData.begin ()).GetGeometryPtr ()->GetAsISolidPrimitive ();
        if (solidPrimitive.IsValid () && _ValidateGeometry (solidPrimitive))
            return DgnDbStatus::Success;

        return DgnDbStatus::ValidationFailed;
        }
    return status;
    }

END_GRIDS_NAMESPACE