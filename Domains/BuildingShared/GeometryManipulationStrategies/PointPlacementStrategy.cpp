/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "PublicApi/GeometryManipulationStrategiesApi.h"
#include <BuildingShared/Utils/UtilsApi.h>
#include <BuildingShared/Units/UnitsApi.h>

BEGIN_BUILDING_SHARED_NAMESPACE

//--------------------------------------------------------------------------------------
// @bsimethod                                    Vytautas.Kaniusonis             10/2018
//---------------+---------------+---------------+---------------+---------------+------
PointPlacementStrategy::PointPlacementStrategy
(
    PointManipulationStrategyR manipulationStrategy
)
    : T_Super()
    , m_manipulationStrategy(&manipulationStrategy)
    {
    BeAssert(m_manipulationStrategy.IsValid());
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Vytautas.Kaniusonis             10/2018
//---------------+---------------+---------------+---------------+---------------+------
void PointPlacementStrategy::_AddKeyPoint
(
    DPoint3dCR newKeyPoint
)
    {
    if (_GetKeyPoints().size() < 1)
        T_Super::_AddKeyPoint(newKeyPoint);
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Vytautas.Kaniusonis             10/2018
//---------------+---------------+---------------+---------------+---------------+------
void PointPlacementStrategy::_AddDynamicKeyPoint
(
    DPoint3dCR newDynamicKeyPoint
)
    {
    if ((!IsDynamicKeyPointSet() && _GetKeyPoints().size() < 1) ||
        (IsDynamicKeyPointSet() && _GetKeyPoints().size() <= 1))
        T_Super::_AddDynamicKeyPoint(newDynamicKeyPoint);
    }

END_BUILDING_SHARED_NAMESPACE