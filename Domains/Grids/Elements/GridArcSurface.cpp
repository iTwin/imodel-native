
#include <Grids/gridsApi.h>
#include <DgnPlatform/DgnDb.h>
#include <DgnPlatform/DgnCategory.h>
#include <DgnPlatform/ElementGeometry.h>
#include <DgnPlatform/ViewController.h>

BEGIN_GRIDS_NAMESPACE
USING_NAMESPACE_BENTLEY_DGN

DEFINE_GRIDS_ELEMENT_BASE_METHODS (GridArcSurface)
DEFINE_GRIDS_ELEMENT_BASE_METHODS (PlanGridArcSurface)

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

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Jonas.Valiunas                  12/2017
+---------------+---------------+---------------+---------------+---------------+------*/
PlanGridArcSurface::PlanGridArcSurface
(
CreateParams const& params,
DgnExtrusionDetailCR extDetail
) : T_Super(params, ISolidPrimitive::CreateDgnExtrusion (extDetail)), IPlanGridSurface(*this, params, params.m_classId)
    {
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Jonas.Valiunas                  12/2017
+---------------+---------------+---------------+---------------+---------------+------*/
PlanGridArcSurface::PlanGridArcSurface
(
CreateParams const& params
) : T_Super(params), IPlanGridSurface(*this, params, params.m_classId)
    {
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Jonas.Valiunas                  12/2017
+---------------+---------------+---------------+---------------+---------------+------*/
SketchArcGridSurface::SketchArcGridSurface
(
CreateParams const& params
) : T_Super(params)
    {
    if (params.m_classId.IsValid ()) // elements created via handler have no classid.
        {
        SetBaseArc(params.m_arc);
        }
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Jonas.Valiunas                  12/2017
+---------------+---------------+---------------+---------------+---------------+------*/
SketchArcGridSurfacePtr             SketchArcGridSurface::Create
(
CreateParams const& params
)
    {
    SketchArcGridSurfacePtr surface = new SketchArcGridSurface (params);

    if (surface.IsNull() || DgnDbStatus::Success != surface->_Validate())
        return nullptr;
    
    return surface;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Jonas.Valiunas                  12/2017
+---------------+---------------+---------------+---------------+---------------+------*/
void                            SketchArcGridSurface::SetBaseArc
(
DEllipse3d arc
)
    {
    if (arc.IsNearZeroRadius()) //if the radius is zero - arc not valid
        return;

    ICurvePrimitivePtr arcPrimitive = ICurvePrimitive::CreateArc (arc);
    IGeometryPtr geometryPtr = IGeometry::Create (arcPrimitive);

    ECN::ECValue arc2dValue;
    arc2dValue.SetIGeometry (*geometryPtr);

    SetPropertyValue (prop_Arc2d (), arc2dValue);
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Jonas.Valiunas                  12/2017
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus                   SketchArcGridSurface::GetBaseArc
(
DEllipse3dR arc
) const
    {
    ECN::ECValue arc2dValue;

    GetPropertyValue (arc2dValue, prop_Arc2d());

    IGeometryPtr geometryPtr = arc2dValue.GetIGeometry ();
    if (!geometryPtr.IsValid ())
        return BentleyStatus::ERROR;

    ICurvePrimitivePtr curve = geometryPtr->GetAsICurvePrimitive ();
    if (!curve.IsValid ())
        return BentleyStatus::ERROR;
            
    DEllipse3dCP pArc = curve->GetArcCP ();
    if (pArc == NULL)
        return BentleyStatus::ERROR;

    arc = *pArc;

    return BentleyStatus::SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Jonas.Valiunas                  12/2017
+---------------+---------------+---------------+---------------+---------------+------*/
PlanCircumferentialGridSurface::PlanCircumferentialGridSurface
(
CreateParams const& params
) : T_Super(params)
    {
    if (params.m_classId.IsValid ()) // elements created via handler have no classid.
        {
        SetRadius(params.m_radius);
        SetStartAngle(params.m_startAngle);
        SetEndAngle(params.m_endAngle);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Jonas.Valiunas                  12/2017
+---------------+---------------+---------------+---------------+---------------+------*/
PlanCircumferentialGridSurfacePtr             PlanCircumferentialGridSurface::Create
(
CreateParams const& params
)
    {
    PlanCircumferentialGridSurfacePtr gridSurface = new PlanCircumferentialGridSurface (params);

    if (gridSurface.IsNull() || DgnDbStatus::Success != gridSurface->_Validate())
        return nullptr;

    return gridSurface;
    }
END_GRIDS_NAMESPACE