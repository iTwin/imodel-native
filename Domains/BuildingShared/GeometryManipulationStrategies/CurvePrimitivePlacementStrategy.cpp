/*--------------------------------------------------------------------------------------+
|
|     $Source: GeometryManipulationStrategies/CurvePrimitivePlacementStrategy.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "PublicApi/GeometryManipulationStrategiesApi.h"

USING_NAMESPACE_BUILDING_SHARED

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                12/2017
//---------------+---------------+---------------+---------------+---------------+------
CurveVectorPtr CurvePrimitivePlacementStrategy::_Finish() const
    {
    return CurveVector::Create(CurveVector::BOUNDARY_TYPE_Open, _FinishPrimitive());
    }

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