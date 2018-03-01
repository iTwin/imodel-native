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
    BBS::ArcPlacementMethod arcPlacementStrategyType
)
    : T_Super()
    , m_manipulationStrategy(ArcGridSurfaceManipulationStrategy::Create(arcPlacementStrategyType))
    {
    BeAssert(m_manipulationStrategy.IsValid());
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                02/2018
//---------------+---------------+---------------+---------------+---------------+------
ArcPlacementStrategyCPtr ArcGridSurfacePlacementStrategy::GetArcPlacementStrategy() const
    {
    return dynamic_cast<ArcPlacementStrategyCP>(TryGetGeometryPlacementStrategy().get());
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                02/2018
//---------------+---------------+---------------+---------------+---------------+------
ArcPlacementStrategyPtr ArcGridSurfacePlacementStrategy::GetArcPlacementStrategyForEdit()
    {
    return dynamic_cast<ArcPlacementStrategyP>(TryGetGeometryPlacementStrategyForEdit().get());
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                02/2018
//---------------+---------------+---------------+---------------+---------------+------
void ArcGridSurfacePlacementStrategy::SetPlacementMethod
(
    BBS::ArcPlacementMethod method
)
    {
    GetArcPlacementStrategyForEdit()->SetPlacementMethod(method);
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                02/2018
//---------------+---------------+---------------+---------------+---------------+------
void ArcGridSurfacePlacementStrategy::SetUseSweep
(
    bool useSweep
)
    {
    GetArcPlacementStrategyForEdit()->SetUseSweep(useSweep);
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                02/2018
//---------------+---------------+---------------+---------------+---------------+------
void ArcGridSurfacePlacementStrategy::SetSweep
(
    double sweep
)
    {
    GetArcPlacementStrategyForEdit()->SetSweep(sweep);
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                02/2018
//---------------+---------------+---------------+---------------+---------------+------
void ArcGridSurfacePlacementStrategy::SetUseRadius
(
    bool useRadius
)
    {
    GetArcPlacementStrategyForEdit()->SetUseRadius(useRadius);
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                02/2018
//---------------+---------------+---------------+---------------+---------------+------
void ArcGridSurfacePlacementStrategy::SetRadius
(
    double radius
)
    {
    GetArcPlacementStrategyForEdit()->SetRadius(radius);
    }