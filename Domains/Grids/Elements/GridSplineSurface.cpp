
#include <Grids/gridsApi.h>
#include <DgnPlatform/DgnDb.h>
#include <DgnPlatform/DgnCategory.h>
#include <DgnPlatform/ElementGeometry.h>
#include <DgnPlatform/ViewController.h>

BEGIN_GRIDS_NAMESPACE
USING_NAMESPACE_BENTLEY_DGN

DEFINE_GRIDS_ELEMENT_BASE_METHODS (GridSplineSurface)
DEFINE_GRIDS_ELEMENT_BASE_METHODS (PlanGridSplineSurface)

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
bool GridSplineSurface::_ValidateGeometry(ISolidPrimitivePtr surface) const
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
GridAxisCR gridAxis,
ISolidPrimitivePtr  surface
)
    {
    GridSplineSurfacePtr gridSurface = new GridSplineSurface (CreateParamsFromModelAxisClassId (model, gridAxis, QueryClassId(model.GetDgnDb())), surface);
    if (gridSurface.IsNull() || DgnDbStatus::Success != gridSurface->_Validate())
        return nullptr;

    return gridSurface;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Haroldas.Vitunskas                  06/17
//---------------------------------------------------------------------------------------
GridSplineSurfacePtr GridSplineSurface::Create
(
Dgn::SpatialLocationModelCR model, 
GridAxisCR gridAxis,
DgnExtrusionDetail extDetail
)
    {
    return GridSplineSurface::Create(model, gridAxis, ISolidPrimitive::CreateDgnExtrusion(extDetail));
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Jonas.Valiunas                  10/2017
//---------------+---------------+---------------+---------------+---------------+------
DgnDbStatus      GridSplineSurface::_Validate
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
PlanGridSplineSurface::PlanGridSplineSurface
(
CreateParams const& params,
DgnExtrusionDetailCR extDetail
) : T_Super(params, ISolidPrimitive::CreateDgnExtrusion (extDetail)), IPlanGridSurface(*this, params, params.m_classId)
    {
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Jonas.Valiunas                  12/2017
+---------------+---------------+---------------+---------------+---------------+------*/
PlanGridSplineSurface::PlanGridSplineSurface
(
CreateParams const& params
) : T_Super(params), IPlanGridSurface(*this, params, params.m_classId)
    {
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Jonas.Valiunas                  12/2017
+---------------+---------------+---------------+---------------+---------------+------*/
SketchSplineGridSurface::SketchSplineGridSurface
(
CreateParams const& params
) : T_Super(params)
    {
    if (params.m_classId.IsValid ()) // elements created via handler have no classid.
        {
        SetBaseSpline(params.m_splinePrimitive);
        }
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Jonas.Valiunas                  12/2017
+---------------+---------------+---------------+---------------+---------------+------*/
SketchSplineGridSurfacePtr             SketchSplineGridSurface::Create
(
CreateParams const& params
)
    {
    SketchSplineGridSurfacePtr surface = new SketchSplineGridSurface (params);

    if (surface.IsNull() || DgnDbStatus::Success != surface->_Validate())
        return nullptr;
    
    return surface;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Jonas.Valiunas                  12/2017
+---------------+---------------+---------------+---------------+---------------+------*/
Dgn::DgnDbStatus                SketchSplineGridSurface::_OnUpdate
(
Dgn::DgnElementCR original
)
    {
    Dgn::DgnDbStatus status = T_Super::_OnUpdate(original);
    if (Dgn::DgnDbStatus::Success != status)
        return status;

    double height = GetEndElevation() - GetStartElevation();

    if (DoubleOps::AlmostEqualFraction(height, 0.0))    //should have a height
        return Dgn::DgnDbStatus::ValidationFailed;

    ICurvePrimitivePtr baseSpline = GetBaseSpline();

    if (baseSpline.IsNull())
        return Dgn::DgnDbStatus::ValidationFailed;  //if failed to get base spline, fail

    Transform translation = Transform::From(0.0, 0.0, GetStartElevation());

    CurveVectorPtr newBase = CurveVector::Create(baseSpline);
    DVec3d up = DVec3d::From(0, 0, height);
    DgnExtrusionDetail detail = DgnExtrusionDetail(newBase->Clone(translation), up, false);
    ISolidPrimitivePtr geometry = ISolidPrimitive::CreateDgnExtrusion(detail);

    SetGeometry(geometry);
    return status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Jonas.Valiunas                  12/2017
+---------------+---------------+---------------+---------------+---------------+------*/
void                            SketchSplineGridSurface::SetBaseSpline
(
ICurvePrimitivePtr splinePrimitive
)
    {
    if (!splinePrimitive.IsValid()) //if spline is null, return
        return;

    IGeometryPtr geometryPtr = IGeometry::Create (splinePrimitive);

    ECN::ECValue spline2dValue;
    spline2dValue.SetIGeometry (*geometryPtr);

    SetPropertyValue (prop_Spline2d (), spline2dValue);
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Jonas.Valiunas                  12/2017
+---------------+---------------+---------------+---------------+---------------+------*/
ICurvePrimitivePtr              SketchSplineGridSurface::GetBaseSpline
(
) const
    {
    ECN::ECValue spline2dValue;

    GetPropertyValue (spline2dValue, prop_Spline2d());

    IGeometryPtr geometryPtr = spline2dValue.GetIGeometry ();
    if (!geometryPtr.IsValid ())
        return nullptr;

    ICurvePrimitivePtr curve = geometryPtr->GetAsICurvePrimitive ();

    return curve;
    }

END_GRIDS_NAMESPACE