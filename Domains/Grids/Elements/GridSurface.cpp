/*--------------------------------------------------------------------------------------+
|
|     $Source: Grids/Elements/GridSurface.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#include <Grids/gridsApi.h>
#include <DgnPlatform/DgnDb.h>
#include <DgnPlatform/DgnCategory.h>
#include <DgnPlatform/ElementGeometry.h>
#include <DgnPlatform/ViewController.h>
#include <ConstraintSystem/ConstraintSystemApi.h>

BEGIN_GRIDS_NAMESPACE
USING_NAMESPACE_BENTLEY_DGN
USING_NAMESPACE_BUILDING

DEFINE_GRIDS_ELEMENT_BASE_METHODS (GridSurface)

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Jonas.Valiunas                  03/2017
+---------------+---------------+---------------+---------------+---------------+------*/
GridSurface::GridSurface
(
CreateParams const& params
) : T_Super(params) 
    {

    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Jonas.Valiunas                  03/2017
+---------------+---------------+---------------+---------------+---------------+------*/
GridSurface::GridSurface
(
CreateParams const& params,
GridAxisCPtr gridAxis,
ISolidPrimitivePtr  surface
) : T_Super(params, surface) 
    {
    BeAssert (gridAxis->GetElementId ().IsValid ());
    SetAxisId (gridAxis->GetElementId ());
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Jonas.Valiunas                  03/2017
+---------------+---------------+---------------+---------------+---------------+------*/
GridSurface::GridSurface
(
CreateParams const& params,
GridAxisCPtr gridAxis,
CurveVectorPtr  surfaceVector
) : T_Super(params, surfaceVector) 
    {
    BeAssert (gridAxis->GetElementId ().IsValid ());
    SetAxisId (gridAxis->GetElementId ());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Jonas.Valiunas                  03/2017
+---------------+---------------+---------------+---------------+---------------+------*/
Dgn::GeometricElement3d::CreateParams           GridSurface::CreateParamsFromModel
(
Dgn::SpatialLocationModelCR model,
DgnClassId classId
)
    {
    DgnCategoryId categoryId = SpatialCategory::QueryCategoryId (model.GetDgnDb().GetDictionaryModel (), GRIDS_CATEGORY_CODE_GridSurface);

    CreateParams createParams (model.GetDgnDb (), model.GetModelId (), classId, categoryId);

    return createParams;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Haroldas.Vitunskas                  06/17
//---------------------------------------------------------------------------------------
void GridSurface::RotateXY(double theta)
    {
    Placement3d placement = GetPlacement();
    GeometryUtils::RotatePlacementXY(placement, theta);
    SetPlacement(placement);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Haroldas.Vitunskas                  06/17
//---------------------------------------------------------------------------------------
void GridSurface::RotateXY(DPoint3d point, double theta)
    {
    Placement3d placement = GetPlacement();
    GeometryUtils::RotatePlacementAroundPointXY(placement, point, theta);
    SetPlacement(placement);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Haroldas.Vitunskas                  06/17
//---------------------------------------------------------------------------------------
void GridSurface::MoveToPoint(DPoint3d target)
    {
    Placement3d placement = GetPlacement();
    placement.SetOrigin(target);
    SetPlacement(placement);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Haroldas.Vitunskas                  06/17
//---------------------------------------------------------------------------------------
void GridSurface::TranslateXY(DVec3d translation)
    {
    Placement3d placement = GetPlacement();
    GeometryUtils::TranslatePlacementXY(placement, translation);
    SetPlacement(placement);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Haroldas.Vitunskas                  06/17
//---------------------------------------------------------------------------------------
void GridSurface::Translate(DVec3d translation)
    {
    Placement3d placement = GetPlacement();
    GeometryUtils::TranslatePlacement(placement, translation);
    SetPlacement(placement);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Haroldas.Vitunskas                  10/17
//---------------------------------------------------------------------------------------
BentleyStatus GridSurface::SetGeometry(ISolidPrimitivePtr surface)
    {
    if (_ValidateGeometry(surface))
        return _SetGeometry(surface);

    return BentleyStatus::ERROR;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Haroldas.Vitunskas                  10/17
//---------------------------------------------------------------------------------------
BentleyStatus GridSurface::_SetGeometry(ISolidPrimitivePtr surface)
    {
    Dgn::GeometrySourceP geomElem = ToGeometrySourceP();
    Dgn::GeometryBuilderPtr builder = Dgn::GeometryBuilder::Create(*geomElem);

    if (builder->Append(*surface, Dgn::GeometryBuilder::CoordSystem::World))
        {
        if (SUCCESS != builder->Finish(*geomElem))
            return BentleyStatus::ERROR;
        }

    return BentleyStatus::SUCCESS;
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Jonas.Valiunas                  10/2017
//---------------+---------------+---------------+---------------+---------------+------
DgnDbStatus      GridSurface::_Validate
(
) const
    {
    if (!GetAxisId ().IsValid ()) //grid axis must be set
        return DgnDbStatus::ValidationFailed;

    return DgnDbStatus::Success;
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Jonas.Valiunas                  10/2017
//---------------+---------------+---------------+---------------+---------------+------
DgnDbStatus      GridSurface::_OnInsert
(
)
    {
    DgnDbStatus status = T_Super::_OnInsert ();
    if (status == DgnDbStatus::Success)
        {
        return _Validate ();
        }
    return status;
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Jonas.Valiunas                  10/2017
//---------------+---------------+---------------+---------------+---------------+------
DgnDbStatus      GridSurface::_OnUpdate
(
    DgnElementCR original
)
    {
    DgnDbStatus status = T_Super::_OnUpdate (original);
    if (status == DgnDbStatus::Success)
        {
        return _Validate ();
        }
    return status;
    }

END_GRIDS_NAMESPACE