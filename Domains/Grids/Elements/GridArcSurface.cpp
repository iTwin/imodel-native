
#include <Grids/Elements/GridElementsAPI.h>
#include <DgnPlatform/DgnDb.h>
#include <DgnPlatform/DgnCategory.h>
#include <DgnPlatform/ElementGeometry.h>
#include <DgnPlatform/ViewController.h>
#include <BuildingShared/DgnUtils/BuildingDgnUtilsApi.h>

BEGIN_GRIDS_NAMESPACE
USING_NAMESPACE_BENTLEY_DGN
USING_NAMESPACE_BUILDING_SHARED

DEFINE_GRIDS_ELEMENT_BASE_METHODS(GridArcSurface)
DEFINE_GRIDS_ELEMENT_BASE_METHODS(PlanGridArcSurface)
DEFINE_GRIDS_ELEMENT_BASE_METHODS(PlanCircumferentialGridSurface)
DEFINE_GRIDS_ELEMENT_BASE_METHODS(SketchArcGridSurface)

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
    
    return surface;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Jonas.Valiunas                  12/2017
+---------------+---------------+---------------+---------------+---------------+------*/
Dgn::DgnDbStatus                SketchArcGridSurface::RecomputeGeometryStream
(
)
    {
    double height = GetEndElevation() - GetStartElevation();

    if (DoubleOps::AlmostEqualFraction(height, 0.0))    //should have a height
        return Dgn::DgnDbStatus::ValidationFailed;

    DEllipse3d arc;
    if (BentleyStatus::SUCCESS != GetBaseArc(arc))
        return Dgn::DgnDbStatus::ValidationFailed;

    if (arc.IsNearZeroRadius()) //if the radius is zero - arc not valid
        return Dgn::DgnDbStatus::ValidationFailed;

    ICurvePrimitivePtr arcPrimitive = ICurvePrimitive::CreateArc(arc);
    CurveVectorPtr newBase = CurveVector::Create(arcPrimitive);

    Transform translation = Transform::From(0.0, 0.0, GetStartElevation());

    DVec3d up = DVec3d::From(0, 0, height);
    DgnExtrusionDetail detail = DgnExtrusionDetail(newBase->Clone(translation), up, false);
    ISolidPrimitivePtr geometry = ISolidPrimitive::CreateDgnExtrusion(detail);


    GridCPtr grid = GetDgnDb().Elements().Get<Grid>(GetGridId());
    SetPlacement(Placement3d()); //set the start local coordinates
    SetGeometry(geometry);
    Transform gridTrans = grid->GetPlacementTransform();
    Placement3d thisPlacement(GetPlacement());
    thisPlacement.TryApplyTransform(gridTrans);

    SetPlacement(thisPlacement);
    return Dgn::DgnDbStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Jonas.Valiunas                  12/2017
+---------------+---------------+---------------+---------------+---------------+------*/
Dgn::DgnDbStatus                SketchArcGridSurface::_OnUpdate
(
Dgn::DgnElementCR original
)
    {
    Dgn::DgnDbStatus status = RecomputeGeometryStream();
    if (Dgn::DgnDbStatus::Success != status)
        return status;

    return T_Super::_OnUpdate(original);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Jonas.Valiunas                  12/2017
+---------------+---------------+---------------+---------------+---------------+------*/
Dgn::DgnDbStatus                SketchArcGridSurface::_OnInsert
(
)
    {
    Dgn::DgnDbStatus status = RecomputeGeometryStream();
    if (Dgn::DgnDbStatus::Success != status)
        return status;

    return T_Super::_OnInsert();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Jonas.Valiunas                  12/2017
+---------------+---------------+---------------+---------------+---------------+------*/
void                            SketchArcGridSurface::SetBaseArc
(
DEllipse3d arc
)
    {
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

    return gridSurface;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Jonas.Valiunas                  01/2018
+---------------+---------------+---------------+---------------+---------------+------*/
PlanCircumferentialGridSurfacePtr             PlanCircumferentialGridSurface::CreateAndInsert
(
CreateParams const& params
)
    {
    PlanCircumferentialGridSurfacePtr gridSurface = PlanCircumferentialGridSurface::Create(params);

    if (!gridSurface->Insert().IsValid())
        return nullptr;

    return gridSurface;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Jonas.Valiunas                  01/2018
+---------------+---------------+---------------+---------------+---------------+------*/
Dgn::DgnDbStatus                PlanCircumferentialGridSurface::RecomputeGeometryStream
(
)
    {
    GridAxisCPtr gridAxis = GetDgnDb().Elements().Get<GridAxis>(GetAxisId());
    GridCPtr grid = GetDgnDb().Elements().Get<Grid>(GetGridId());

    double staElev = GetStartElevation();
    double endElev = GetEndElevation();
    double height = endElev - staElev;

    double radius = GetRadius();
    double startAngle = GetStartAngle();
    double endAngle = GetEndAngle();
    double length = endAngle - startAngle;

    bool isCircularAxis = gridAxis->IsCircularAxis();

    if (!isCircularAxis) //must be on CircularAxis
        return Dgn::DgnDbStatus::ValidationFailed;

    if (DoubleOps::AlmostEqualFraction(height, 0.0))    //should have a height
        return Dgn::DgnDbStatus::ValidationFailed;

    if (DoubleOps::AlmostEqualFraction(length, 0.0))    //should have a length
        return Dgn::DgnDbStatus::ValidationFailed;

    DVec3d xVec = DVec3d::From(radius, 0.0, 0.0);
    DVec3d yVec = DVec3d::From(0.0, radius, 0.0);
    DPoint3d center = DPoint3d::From(0.0, 0.0, staElev);

    DEllipse3d ellipse = DEllipse3d::FromVectors(center, xVec, yVec, startAngle, length);


    //start from the grid transform
    Placement3dCR currGridPlacement = grid->GetPlacement();
    SetPlacement(Placement3d()); //set the start local coordinates

    //now set the geometry
    ICurvePrimitivePtr arcPrimitive = ICurvePrimitive::CreateArc(ellipse);
    DgnExtrusionDetail detail = DgnExtrusionDetail(CurveVector::Create(arcPrimitive), DVec3d::From(0, 0, height), false);
    ISolidPrimitivePtr geometryInLocalCoords = ISolidPrimitive::CreateDgnExtrusion(detail);

    if (BentleyStatus::SUCCESS != SetGeometry(geometryInLocalCoords))
        Dgn::DgnDbStatus::WriteError;

    //translate to member position
    Placement3d thisPlacement(GetPlacement());
    Transform gridTrans = grid->GetPlacementTransform();
    thisPlacement.TryApplyTransform(gridTrans);

    SetPlacement(thisPlacement);

    return Dgn::DgnDbStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Jonas.Valiunas                  01/2018
+---------------+---------------+---------------+---------------+---------------+------*/
Dgn::DgnDbStatus                PlanCircumferentialGridSurface::_OnUpdate
(
Dgn::DgnElementCR original
)
    {
    Dgn::DgnDbStatus status = RecomputeGeometryStream();
    if (Dgn::DgnDbStatus::Success != status)
        return status;

    return T_Super::_OnUpdate(original);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Jonas.Valiunas                  01/2018
+---------------+---------------+---------------+---------------+---------------+------*/
Dgn::DgnDbStatus                PlanCircumferentialGridSurface::_OnInsert
(
)
    {
    Dgn::DgnDbStatus status = RecomputeGeometryStream();
    if (Dgn::DgnDbStatus::Success != status)
        return status;

    return T_Super::_OnInsert();
    }

END_GRIDS_NAMESPACE