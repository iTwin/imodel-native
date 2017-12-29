/*--------------------------------------------------------------------------------------+
|
|     $Source: GeometryManipulationStrategies/CurveVectorPlacementStrategy.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "PublicApi/GeometryManipulationStrategiesApi.h"

USING_NAMESPACE_BUILDING_SHARED

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                12/2017
//---------------+---------------+---------------+---------------+---------------+------
CurveVectorPtr CurveVectorPlacementStrategy::_Finish() const
    {
    BeAssert(false && "Not implemented.");
    return nullptr;
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                12/2017
//---------------+---------------+---------------+---------------+---------------+------
CurveVectorPtr CurveVectorPlacementStrategy::Finish() const
    {
    return _Finish();
    }