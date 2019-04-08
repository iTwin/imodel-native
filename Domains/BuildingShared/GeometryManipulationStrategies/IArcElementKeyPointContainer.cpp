/*--------------------------------------------------------------------------------------+
|
|     $Source: GeometryManipulationStrategies/IArcElementKeyPointContainer.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "PublicApi/GeometryManipulationStrategiesApi.h"

USING_NAMESPACE_BUILDING_SHARED

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                11/2018
//---------------+---------------+---------------+---------------+---------------+------
template<typename T>
bool IArcElementKeyPointContainer::TryGetKeyPoint
(
    T fn
) const
    {
    IArcKeyPointContainer const& container = _GetIArcKeyPointContainer();
    if (this == &container)
        {
        BeAssert(this != &container);
        return false; // If we don't return here, an infinite loop will start.
        }
    return fn(container);
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                11/2018
//---------------+---------------+---------------+---------------+---------------+------
bool IArcElementKeyPointContainer::_TryGetStartKeyPoint
(
    DPoint3dR startKeyPoint
) const
    {
    return TryGetKeyPoint([&] (IArcKeyPointContainer const& container) { return container.TryGetStartKeyPoint(startKeyPoint); });
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                11/2018
//---------------+---------------+---------------+---------------+---------------+------
bool IArcElementKeyPointContainer::_TryGetCenterKeyPoint
(
    DPoint3dR centerKeyPoint
) const
    {
    return TryGetKeyPoint([&] (IArcKeyPointContainer const& container) { return container.TryGetCenterKeyPoint(centerKeyPoint); });
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                11/2018
//---------------+---------------+---------------+---------------+---------------+------
bool IArcElementKeyPointContainer::_TryGetMidKeyPoint
(
    DPoint3dR midKeyPoint
) const
    {
    return TryGetKeyPoint([&] (IArcKeyPointContainer const& container) { return container.TryGetMidKeyPoint(midKeyPoint); });
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                11/2018
//---------------+---------------+---------------+---------------+---------------+------
bool IArcElementKeyPointContainer::_TryGetEndKeyPoint
(
    DPoint3dR endKeyPoint
) const
    {
    return TryGetKeyPoint([&] (IArcKeyPointContainer const& container) { return container.TryGetEndKeyPoint(endKeyPoint); });
    }