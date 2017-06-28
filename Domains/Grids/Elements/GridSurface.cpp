/*--------------------------------------------------------------------------------------+
|
|     $Source: Grids/Elements/GridSurface.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#include "PublicApi/GridSurface.h"
#include <DgnPlatform/DgnDb.h>
#include <DgnPlatform/DgnCategory.h>
#include <DgnPlatform/ElementGeometry.h>
#include <DgnPlatform/ViewController.h>
#include <GeometryUtils.h>
#include "PublicApi\GridArcSurface.h"
#include "PublicApi\GridPlaneSurface.h"

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
ISolidPrimitivePtr  surface
) : T_Super(params, surface) 
    {
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Jonas.Valiunas                  03/2017
+---------------+---------------+---------------+---------------+---------------+------*/
GridSurface::GridSurface
(
CreateParams const& params,
CurveVectorPtr  surfaceVector
) : T_Super(params, surfaceVector) 
    {
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

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Jonas.Valiunas                  03/2017
+---------------+---------------+---------------+---------------+---------------+------*/
GridSurfacePtr                 GridSurface::Create 
(
Dgn::SpatialLocationModelCR model,
CurveVectorPtr  surfaceVector
)
    {
    return new GridSurface (CreateParamsFromModel(model, QueryClassId(model.GetDgnDb())), surfaceVector);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Haroldas.Vitunskas                  03/17
//---------------------------------------------------------------------------------------
GridSurfacePtr                 GridSurface::Create 
(
Dgn::SpatialLocationModelCR model,
ISolidPrimitivePtr surface
)
    {
    return new GridSurface (CreateParamsFromModel(model, QueryClassId(model.GetDgnDb())), surface);
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
// @bsimethod                                    Haroldas.Vitunskas                  06/17
//---------------------------------------------------------------------------------------
GridSurfacePtr GridSurface::Create(Dgn::SpatialLocationModelCR model, DgnExtrusionDetail extDetail)
    {
    return GridSurface::Create(model, ISolidPrimitive::CreateDgnExtrusion(extDetail));
    }

END_GRIDS_NAMESPACE