/*--------------------------------------------------------------------------------------+
|
|     $Source: Grids/Strategies/ArcGridSurfacePlacementStrategy.cpp $
|
|  $Copyright: (c) 2019 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <Grids/Strategies/GridStrategies.h>

USING_NAMESPACE_BUILDING_SHARED
USING_NAMESPACE_GRIDS

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                02/2018
//---------------+---------------+---------------+---------------+---------------+------
ArcGridSurfacePlacementStrategy::ArcGridSurfacePlacementStrategy
(
    BBS::ArcPlacementMethod arcPlacementStrategyType,
    Dgn::DgnDbR db
)
    : T_Super(db)
    , m_manipulationStrategy(ArcGridSurfaceManipulationStrategy::Create(arcPlacementStrategyType, db))
    {
    BeAssert(m_manipulationStrategy.IsValid());
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                05/2018
//---------------+---------------+---------------+---------------+---------------+------
ArcGridSurfacePlacementStrategyPtr ArcGridSurfacePlacementStrategy::Create
(
    BBS::ArcPlacementMethod arcPlacementStrategyType, 
    Dgn::DgnDbR db
)
    {
    return new ArcGridSurfacePlacementStrategy(arcPlacementStrategyType, db);
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
void ArcGridSurfacePlacementStrategy::_SetPlacementMethod
(
    BBS::ArcPlacementMethod method
)
    {
    GetArcPlacementStrategyForEdit()->SetPlacementMethod(method);
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                02/2018
//---------------+---------------+---------------+---------------+---------------+------
void ArcGridSurfacePlacementStrategy::_SetUseSweep
(
    bool useSweep
)
    {
    GetArcPlacementStrategyForEdit()->SetUseSweep(useSweep);
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                02/2018
//---------------+---------------+---------------+---------------+---------------+------
void ArcGridSurfacePlacementStrategy::_SetSweep
(
    double sweep
)
    {
    GetArcPlacementStrategyForEdit()->SetSweep(sweep);
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                02/2018
//---------------+---------------+---------------+---------------+---------------+------
void ArcGridSurfacePlacementStrategy::_SetUseRadius
(
    bool useRadius
)
    {
    GetArcPlacementStrategyForEdit()->SetUseRadius(useRadius);
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                02/2018
//---------------+---------------+---------------+---------------+---------------+------
void ArcGridSurfacePlacementStrategy::_SetRadius
(
    double radius
)
    {
    GetArcPlacementStrategyForEdit()->SetRadius(radius);
    }