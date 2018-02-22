/*--------------------------------------------------------------------------------------+
|
|     $Source: Grids/Strategies/GridPlanarSurfaceManipulationStrategy.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <Grids\GridsApi.h>

USING_NAMESPACE_BUILDING_SHARED
USING_NAMESPACE_GRIDS

//--------------------------------------------------------------------------------------
// @bsimethod                                    Haroldas.Vitunskas              01/2018
//---------------+---------------+---------------+---------------+---------------+------
BentleyStatus GridPlanarSurfaceManipulationStrategy::_UpdateGridSurface()
    {
    BentleyStatus superStatus = T_Super::_UpdateGridSurface();
    if (BentleyStatus::SUCCESS != superStatus)
        return superStatus;

    PlanGridPlanarSurfacePtr surface = _GetPlanGridPlanarSurfaceP();

    if (m_axis.IsNull() || surface.IsNull())
        return BentleyStatus::ERROR;

    if (m_axis->GetElementId() != surface->GetAxisId())
        surface->SetAxisId(m_axis->GetElementId());

    return BentleyStatus::SUCCESS;
    }