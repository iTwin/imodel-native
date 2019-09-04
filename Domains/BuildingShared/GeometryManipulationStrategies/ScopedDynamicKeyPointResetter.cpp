/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#include "PublicApi/GeometryManipulationStrategiesApi.h"

USING_NAMESPACE_BUILDING_SHARED

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                03/2018
//---------------+---------------+---------------+---------------+---------------+------
ScopedDynamicKeyPointResetter::ScopedDynamicKeyPointResetter
(
    IResettableDynamicR strategy
)
    : m_strategy(strategy)
    , m_initialDynamicState(strategy.GetDynamicState())
    {
    BeAssert(m_initialDynamicState.IsValid());
    m_strategy.SetDynamicState(*BooleanDynamicState::Create(false));
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                03/2018
//---------------+---------------+---------------+---------------+---------------+------
ScopedDynamicKeyPointResetter::~ScopedDynamicKeyPointResetter()
    {
    m_strategy.SetDynamicState(*m_initialDynamicState);
    }