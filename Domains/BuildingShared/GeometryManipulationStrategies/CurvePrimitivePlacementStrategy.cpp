/*--------------------------------------------------------------------------------------+
|
|     $Source: GeometryManipulationStrategies/CurvePrimitivePlacementStrategy.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "PublicApi/GeometryManipulationStrategiesApi.h"

USING_NAMESPACE_BUILDING_SHARED

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                12/2017
//---------------+---------------+---------------+---------------+---------------+------
ICurvePrimitivePtr CurvePrimitivePlacementStrategy::_FinishPrimitive() const
    {
    CurvePrimitiveManipulationStrategyCR strategy = dynamic_cast<CurvePrimitiveManipulationStrategyCR>(GetManipulationStrategy());
    return strategy.FinishPrimitive();
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                12/2017
//---------------+---------------+---------------+---------------+---------------+------
ICurvePrimitivePtr CurvePrimitivePlacementStrategy::FinishPrimitive() const
    {
    return _FinishPrimitive();
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Haroldas.Vitunskas              01/2018
//---------------+---------------+---------------+---------------+---------------+------
bool CurvePrimitivePlacementStrategy::_IsContinious() const
    {
    return _GetCurvePrimitiveManipulationStrategy().IsContinious();
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                03/2018
//---------------+---------------+---------------+---------------+---------------+------
bool CurvePrimitivePlacementStrategy::IsEmpty() const
    {
    return _IsEmpty();
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                03/2018
//---------------+---------------+---------------+---------------+---------------+------
bool CurvePrimitivePlacementStrategy::_IsEmpty() const
    {
    return _GetCurvePrimitiveManipulationStrategy().IsEmpty();
    }
