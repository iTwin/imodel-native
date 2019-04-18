/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#include <Grids/Strategies/GridStrategies.h>

USING_NAMESPACE_BUILDING_SHARED
USING_NAMESPACE_GRIDS

const Utf8CP SketchGridSurfacePlacementStrategy::prop_BottomElevation = SketchGridSurfaceManipulationStrategy::prop_BottomElevation;
const Utf8CP SketchGridSurfacePlacementStrategy::prop_TopElevation = SketchGridSurfaceManipulationStrategy::prop_TopElevation;
const Utf8CP SketchGridSurfacePlacementStrategy::prop_Axis = SketchGridSurfaceManipulationStrategy::prop_Axis;
const Utf8CP SketchGridSurfacePlacementStrategy::prop_Name = SketchGridSurfaceManipulationStrategy::prop_Name;
const Utf8CP SketchGridSurfacePlacementStrategy::prop_WorkingPlane = SketchGridSurfaceManipulationStrategy::prop_WorkingPlane;
const Utf8CP SketchGridSurfacePlacementStrategy::prop_Length = SketchGridSurfaceManipulationStrategy::prop_Length;
const Utf8CP SketchGridSurfacePlacementStrategy::prop_Angle = SketchGridSurfaceManipulationStrategy::prop_Angle;

//--------------------------------------------------------------------------------------
// @bsimethod                                    Vytautas.Kaniusonis             03/2018
//---------------+---------------+---------------+---------------+---------------+------
Utf8String SketchGridSurfacePlacementStrategy::_GetMessage() const
    {
    return _GetSketchGridSurfaceManipulationStrategy().GetMessage();
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Vytautas.Kaniusonis             03/2018
//---------------+---------------+---------------+---------------+---------------+------
void SketchGridSurfacePlacementStrategy::_AddViewOverlay
(
    Dgn::Render::GraphicBuilderR builder,
    DRange3dCR viewRange,
    TransformCR worldToView,
    Dgn::ColorDefCR contrastingToBackgroundColor
) const {}

//--------------------------------------------------------------------------------------
// @bsimethod                                    Haroldas.Vitunskas              01/2018
//---------------+---------------+---------------+---------------+---------------+------
BentleyStatus SketchGridSurfacePlacementStrategy::_TryGetProperty(Utf8CP key, double & value) const
    {
    if (BentleyStatus::SUCCESS == _GetSketchGridSurfaceManipulationStrategy().TryGetProperty(key, value) ||
        BentleyStatus::SUCCESS == T_Super::_TryGetProperty(key, value))
        return BentleyStatus::SUCCESS;

    return BentleyStatus::ERROR;
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Haroldas.Vitunskas              01/2018
//---------------+---------------+---------------+---------------+---------------+------
void SketchGridSurfacePlacementStrategy::_SetProperty(Utf8CP key, double const & value)
    {
    _GetSketchGridSurfaceManipulationStrategyForEdit().SetProperty(key, value);
    T_Super::_SetProperty(key, value);
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Haroldas.Vitunskas              01/2018
//---------------+---------------+---------------+---------------+---------------+------
BentleyStatus SketchGridSurfacePlacementStrategy::_TryGetProperty(Utf8CP key, Utf8String & value) const
    {
    if (BentleyStatus::SUCCESS == _GetSketchGridSurfaceManipulationStrategy().TryGetProperty(key, value) ||
        BentleyStatus::SUCCESS == T_Super::_TryGetProperty(key, value))
        return BentleyStatus::SUCCESS;

    return BentleyStatus::ERROR;
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Haroldas.Vitunskas              01/2018
//---------------+---------------+---------------+---------------+---------------+------
BentleyStatus SketchGridSurfacePlacementStrategy::_TryGetProperty(Utf8CP key, Dgn::DgnElementCP & value) const
    {
    if (BentleyStatus::SUCCESS == _GetSketchGridSurfaceManipulationStrategy().TryGetProperty(key, value) ||
        BentleyStatus::SUCCESS == T_Super::_TryGetProperty(key, value))
        return BentleyStatus::SUCCESS;

    return BentleyStatus::ERROR;
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Haroldas.Vitunskas              01/2018
//---------------+---------------+---------------+---------------+---------------+------
void SketchGridSurfacePlacementStrategy::_SetProperty(Utf8CP key, Dgn::DgnElementCP const & value)
    {
    _GetSketchGridSurfaceManipulationStrategyForEdit().SetProperty(key, value);
    T_Super::_SetProperty(key, value);
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Haroldas.Vitunskas              02/2018
//---------------+---------------+---------------+---------------+---------------+------
void SketchGridSurfacePlacementStrategy::_AddDynamicKeyPoint(DPoint3dCR newDynamicKeyPoint)
    {
    T_Super::_AddDynamicKeyPoint(newDynamicKeyPoint);
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Haroldas.Vitunskas              02/2018
//---------------+---------------+---------------+---------------+---------------+------
void SketchGridSurfacePlacementStrategy::_AddDynamicKeyPoints(bvector<DPoint3d> const & newDynamicKeyPoints)
    {
    T_Super::_AddDynamicKeyPoints(newDynamicKeyPoints);
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Haroldas.Vitunskas              02/2018
//---------------+---------------+---------------+---------------+---------------+------
void SketchGridSurfacePlacementStrategy::_AddKeyPoint(DPoint3dCR newKeyPoint)
    {
    T_Super::_AddKeyPoint(newKeyPoint);
    }
