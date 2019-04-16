/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#include "PublicApi/GeometryManipulationStrategiesApi.h"

USING_NAMESPACE_BUILDING_SHARED

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                01/2018
//---------------+---------------+---------------+---------------+---------------+------
CurveVectorPlacementStrategy::CurveVectorPlacementStrategy() 
    : T_Super()
    , m_manipulationStrategy(CurveVectorManipulationStrategy::Create())
    {
    BeAssert(m_manipulationStrategy.IsValid());
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Haroldas.Vitunskas              02/2018
//---------------+---------------+---------------+---------------+---------------+------
CurveVectorPlacementStrategy::CurveVectorPlacementStrategy
(
    CurveVectorManipulationStrategyR manipulationStrategy
) 
    : T_Super()
    , m_manipulationStrategy(&manipulationStrategy) 
    {
    BeAssert(m_manipulationStrategy.IsValid() && "Manipulation strategy should be valid");
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                12/2017
//---------------+---------------+---------------+---------------+---------------+------
CurveVectorPtr CurveVectorPlacementStrategy::_Finish
(
    bool connectEndStart
) const
    {
    return m_manipulationStrategy->Finish(connectEndStart);
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                12/2017
//---------------+---------------+---------------+---------------+---------------+------
CurveVectorPtr CurveVectorPlacementStrategy::Finish
(
    bool connectEndStart
) const
    {
    return _Finish(connectEndStart);
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                01/2018
//---------------+---------------+---------------+---------------+---------------+------
void CurveVectorPlacementStrategy::ChangeDefaultNewGeometryType
(
    DefaultNewGeometryType newGeometryType
)
    {
    m_manipulationStrategy->ChangeDefaultNewGeometryType(newGeometryType);
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Haroldas Vitunskas              01/2018
//---------------+---------------+---------------+---------------+---------------+------
bool CurveVectorPlacementStrategy::FinishContiniousPrimitive()
    {
    return m_manipulationStrategy->FinishContiniousPrimitive();
    }
    
//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                01/2018
//---------------+---------------+---------------+---------------+---------------+------
void CurveVectorPlacementStrategy::ChangeDefaultPlacementStrategy
(
    LinePlacementStrategyType newPlacementStrategyType
)
    {
    m_manipulationStrategy->ChangeDefaultPlacementStrategy(newPlacementStrategyType);
    }
    

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                01/2018
//---------------+---------------+---------------+---------------+---------------+------
void CurveVectorPlacementStrategy::ChangeDefaultPlacementStrategy
(
    ArcPlacementMethod method
)
    {
    m_manipulationStrategy->ChangeDefaultPlacementStrategy(method);
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                01/2018
//---------------+---------------+---------------+---------------+---------------+------
void CurveVectorPlacementStrategy::ChangeDefaultPlacementStrategy
(
    LineStringPlacementStrategyType newPlacementStrategyType
)
    {
    m_manipulationStrategy->ChangeDefaultPlacementStrategy(newPlacementStrategyType);
    }
