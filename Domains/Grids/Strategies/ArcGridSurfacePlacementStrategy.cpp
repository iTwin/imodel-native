/*--------------------------------------------------------------------------------------+
|
|     $Source: Grids/Strategies/ArcGridSurfacePlacementStrategy.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <Grids\GridsApi.h>

USING_NAMESPACE_BUILDING_SHARED
USING_NAMESPACE_GRIDS

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                02/2018
//---------------+---------------+---------------+---------------+---------------+------
ArcGridSurfacePlacementStrategy::ArcGridSurfacePlacementStrategy
(
    BBS::ArcPlacementStrategyType arcPlacementStrategyType
)
    : T_Super()
    , m_manipulationStrategy(ArcGridSurfaceManipulationStrategy::Create())
    , m_geometryPlacementStrategy(m_manipulationStrategy->CreateArcPlacementStrategy(arcPlacementStrategyType))
    {
    BeAssert(m_manipulationStrategy.IsValid());
    BeAssert(m_geometryPlacementStrategy.IsValid());
    }