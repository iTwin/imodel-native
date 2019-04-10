/*--------------------------------------------------------------------------------------+
|
|     $Source: GeometryManipulationStrategies/SolidPrimitivePlacementStrategy.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "PublicApi/GeometryManipulationStrategiesApi.h"

USING_NAMESPACE_BUILDING_SHARED

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                01/2018
//---------------+---------------+---------------+---------------+---------------+------
ISolidPrimitivePtr SolidPrimitivePlacementStrategy::FinishSolidPrimitive() const
    {
    return m_manipulationStrategy->FinishSolidPrimitive();
    }