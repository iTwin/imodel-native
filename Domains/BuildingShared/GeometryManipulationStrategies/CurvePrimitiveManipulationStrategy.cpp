/*--------------------------------------------------------------------------------------+
|
|     $Source: GeometryManipulationStrategies/CurvePrimitiveManipulationStrategy.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "PublicApi/GeometryManipulationStrategiesApi.h"

USING_NAMESPACE_BUILDING_SHARED

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                12/2017
//---------------+---------------+---------------+---------------+---------------+------
ICurvePrimitivePtr CurvePrimitiveManipulationStrategy::Finish() const
    {
    return _Finish();
    }