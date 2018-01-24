/*--------------------------------------------------------------------------------------+
|
|     $Source: Grids/Strategies/SketchGridSurfacePlacementStrategy.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <Grids\GridsApi.h>

USING_NAMESPACE_BUILDING_SHARED
USING_NAMESPACE_GRIDS

const Utf8CP SketchGridSurfacePlacementStrategy::prop_BottomElevation = SketchGridSurfaceManipulationStrategy::prop_BottomElevation;
const Utf8CP SketchGridSurfacePlacementStrategy::prop_TopElevation = SketchGridSurfaceManipulationStrategy::prop_TopElevation;
const Utf8CP SketchGridSurfacePlacementStrategy::prop_Axis = SketchGridSurfaceManipulationStrategy::prop_Axis;
const Utf8CP SketchGridSurfacePlacementStrategy::prop_Name = SketchGridSurfaceManipulationStrategy::prop_Name;
const Utf8CP SketchGridSurfacePlacementStrategy::prop_Grid = SketchGridSurfaceManipulationStrategy::prop_Grid;

//--------------------------------------------------------------------------------------
// @bsimethod                                    Haroldas.Vitunskas              01/2018
//---------------+---------------+---------------+---------------+---------------+------
SketchGridSurfacePlacementStrategy::SketchGridSurfacePlacementStrategy
(
    BBS::ExtrusionManipulationStrategyR geometryManipulationStrategy
)
    : T_Super()
    , m_manipulationStrategy(_CreateSketchGridSurfaceManipulationStrategy(geometryManipulationStrategy))
    {
    BeAssert(m_manipulationStrategy.IsValid() && "Manipulation strategy should be valid");
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Haroldas.Vitunskas              01/2018
//---------------+---------------+---------------+---------------+---------------+------
void SketchGridSurfacePlacementStrategy::OnDynamicOperationEnd()
    {
    m_manipulationStrategy->OnDynamicOperationEnd();
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Haroldas.Vitunskas              01/2018
//---------------+---------------+---------------+---------------+---------------+------
Utf8String SketchGridSurfacePlacementStrategy::GetMessage() const
    {
    return m_manipulationStrategy->GetMessage();
    }
