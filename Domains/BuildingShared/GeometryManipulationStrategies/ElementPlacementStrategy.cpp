/*--------------------------------------------------------------------------------------+
|
|     $Source: GeometryManipulationStrategies/ElementPlacementStrategy.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "PublicApi/GeometryManipulationStrategiesApi.h"

USING_NAMESPACE_BUILDING_SHARED
USING_NAMESPACE_BENTLEY_DGN

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                01/2018
//---------------+---------------+---------------+---------------+---------------+------
DgnElementPtr ElementPlacementStrategy::_FinishElement
(
    DgnModelR model
) const
    {
    return _GetElementManipulationStrategy().FinishElement(model);
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                01/2018
//---------------+---------------+---------------+---------------+---------------+------
DgnElementPtr ElementPlacementStrategy::FinishElement
(
    DgnModelR model
) const
    {
    return _FinishElement(model);
    }