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
    , m_manipulationStrategy(ArcGridSurfaceManipulationStrategy::Create())
    , m_geometryPlacementStrategy(m_manipulationStrategy->CreateArcPlacementStrategy(arcPlacementStrategyType))
    {
    BeAssert(m_manipulationStrategy.IsValid());
    BeAssert(m_geometryPlacementStrategy.IsValid());
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                02/2018
//---------------+---------------+---------------+---------------+---------------+------
void ArcGridSurfacePlacementStrategy::SetPlacementMethod
(
    BBS::ArcPlacementMethod method
)
    {
    m_geometryPlacementStrategy->SetPlacementMethod(method);
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                02/2018
//---------------+---------------+---------------+---------------+---------------+------
void ArcGridSurfacePlacementStrategy::SetUseSweep
(
    bool useSweep
)
    {
    m_geometryPlacementStrategy->SetUseSweep(useSweep);
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                02/2018
//---------------+---------------+---------------+---------------+---------------+------
void ArcGridSurfacePlacementStrategy::SetSweep
(
    double sweep
)
    {
    m_geometryPlacementStrategy->SetSweep(sweep);
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                02/2018
//---------------+---------------+---------------+---------------+---------------+------
void ArcGridSurfacePlacementStrategy::SetUseRadius
(
    bool useRadius
)
    {
    m_geometryPlacementStrategy->SetUseRadius(useRadius);
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                02/2018
//---------------+---------------+---------------+---------------+---------------+------
void ArcGridSurfacePlacementStrategy::SetRadius
(
    double radius
)
    {
    m_geometryPlacementStrategy->SetRadius(radius);
    }