/*--------------------------------------------------------------------------------------+
|
|     $Source: GeometryManipulationStrategies/SolidPrimitiveManipulationStrategy.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "PublicApi/GeometryManipulationStrategiesApi.h"

USING_NAMESPACE_BUILDING_SHARED

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                01/2018
//---------------+---------------+---------------+---------------+---------------+------
bvector<DPoint3d> SolidPrimitiveManipulationStrategy::_GetKeyPoints() const
    {
    return _GetBaseShapeManipulationStrategy().GetKeyPoints();
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                01/2018
//---------------+---------------+---------------+---------------+---------------+------
bool SolidPrimitiveManipulationStrategy::_IsDynamicKeyPointSet() const
    {
    return _GetBaseShapeManipulationStrategy().IsDynamicKeyPointSet();
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                01/2018
//---------------+---------------+---------------+---------------+---------------+------
void SolidPrimitiveManipulationStrategy::_ResetDynamicKeyPoint()
    {
    _GetBaseShapeManipulationStrategyR().ResetDynamicKeyPoint();
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                01/2018
//---------------+---------------+---------------+---------------+---------------+------
bool SolidPrimitiveManipulationStrategy::_IsComplete() const
    {
    return _GetBaseShapeManipulationStrategy().IsComplete();
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                01/2018
//---------------+---------------+---------------+---------------+---------------+------
bool SolidPrimitiveManipulationStrategy::_CanAcceptMorePoints() const
    {
    return _GetBaseShapeManipulationStrategy().CanAcceptMorePoints();
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                01/2018
//---------------+---------------+---------------+---------------+---------------+------
void SolidPrimitiveManipulationStrategy::_AppendDynamicKeyPoint
(
    DPoint3dCR newDynamicKeyPoint
)
    {
    _GetBaseShapeManipulationStrategyR().AppendDynamicKeyPoint(newDynamicKeyPoint);
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                01/2018
//---------------+---------------+---------------+---------------+---------------+------
void SolidPrimitiveManipulationStrategy::_AppendDynamicKeyPoints
(
    bvector<DPoint3d> const& newDynamicKeyPoints
)
    {
    _GetBaseShapeManipulationStrategyR().AppendDynamicKeyPoints(newDynamicKeyPoints);
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                01/2018
//---------------+---------------+---------------+---------------+---------------+------
void SolidPrimitiveManipulationStrategy::_InsertDynamicKeyPoint
(
    DPoint3dCR newDynamicKeyPoint, 
    size_t index
)
    {
    _GetBaseShapeManipulationStrategyR().InsertDynamicKeyPoint(newDynamicKeyPoint, index);
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                01/2018
//---------------+---------------+---------------+---------------+---------------+------
void SolidPrimitiveManipulationStrategy::_InsertDynamicKeyPoints
(
    bvector<DPoint3d> const& newDynamicKeyPoints, 
    size_t index
)
    {
    _GetBaseShapeManipulationStrategyR().InsertDynamicKeyPoints(newDynamicKeyPoints, index);
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                01/2018
//---------------+---------------+---------------+---------------+---------------+------
void SolidPrimitiveManipulationStrategy::_UpdateDynamicKeyPoint
(
    DPoint3dCR newDynamicKeyPoint, 
    size_t index
)
    {
    _GetBaseShapeManipulationStrategyR().UpdateDynamicKeyPoint(newDynamicKeyPoint, index);
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                01/2018
//---------------+---------------+---------------+---------------+---------------+------
void SolidPrimitiveManipulationStrategy::_UpdateDynamicKeyPoints
(
    bvector<DPoint3d> const& newDynamicKeyPoints, 
    size_t index
)
    {
    _GetBaseShapeManipulationStrategyR().UpdateDynamicKeyPoints(newDynamicKeyPoints, index);
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                01/2018
//---------------+---------------+---------------+---------------+---------------+------
void SolidPrimitiveManipulationStrategy::_UpsertDynamicKeyPoint
(
    DPoint3d newDynamicKeyPoint, 
    size_t index
)
    {
    _GetBaseShapeManipulationStrategyR().UpsertDynamicKeyPoint(newDynamicKeyPoint, index);
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                01/2018
//---------------+---------------+---------------+---------------+---------------+------
void SolidPrimitiveManipulationStrategy::_UpsertDynamicKeyPoints
(
    bvector<DPoint3d> const& newDynamicKeyPoints, 
    size_t index
)
    {
    _GetBaseShapeManipulationStrategyR().UpsertDynamicKeyPoints(newDynamicKeyPoints, index);
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                01/2018
//---------------+---------------+---------------+---------------+---------------+------
void SolidPrimitiveManipulationStrategy::_AppendKeyPoint
(
    DPoint3dCR newKeyPoint
)
    {
    _GetBaseShapeManipulationStrategyR().AppendKeyPoint(newKeyPoint);
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                01/2018
//---------------+---------------+---------------+---------------+---------------+------
void SolidPrimitiveManipulationStrategy::_AppendKeyPoints
(
    bvector<DPoint3d> const& newKeyPoints
)
    {
    _GetBaseShapeManipulationStrategyR().AppendKeyPoints(newKeyPoints);
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                01/2018
//---------------+---------------+---------------+---------------+---------------+------
void SolidPrimitiveManipulationStrategy::_InsertKeyPoint
(
    DPoint3dCR newKeyPoint, 
    size_t index
)
    {
    _GetBaseShapeManipulationStrategyR().InsertKeyPoint(newKeyPoint, index);
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                01/2018
//---------------+---------------+---------------+---------------+---------------+------
void SolidPrimitiveManipulationStrategy::_ReplaceKeyPoint
(
    DPoint3dCR newKeyPoint, 
    size_t index
)
    {
    _GetBaseShapeManipulationStrategyR().ReplaceKeyPoint(newKeyPoint, index);
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                01/2018
//---------------+---------------+---------------+---------------+---------------+------
void SolidPrimitiveManipulationStrategy::_PopKeyPoint()
    {
    _GetBaseShapeManipulationStrategyR().PopKeyPoint();
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                01/2018
//---------------+---------------+---------------+---------------+---------------+------
void SolidPrimitiveManipulationStrategy::_RemoveKeyPoint
(
    size_t index
)
    {
    _GetBaseShapeManipulationStrategyR().RemoveKeyPoint(index);
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                01/2018
//---------------+---------------+---------------+---------------+---------------+------
void SolidPrimitiveManipulationStrategy::_Clear()
    {
    _GetBaseShapeManipulationStrategyR().Clear();
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                01/2018
//---------------+---------------+---------------+---------------+---------------+------
bool SolidPrimitiveManipulationStrategy::_IsBaseComplete() const
    {
    return _GetBaseShapeManipulationStrategy().IsComplete();
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                01/2018
//---------------+---------------+---------------+---------------+---------------+------
bool SolidPrimitiveManipulationStrategy::IsBaseComplete() const
    {
    return _IsBaseComplete();
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                01/2018
//---------------+---------------+---------------+---------------+---------------+------
void SolidPrimitiveManipulationStrategy::SetBaseComplete
(
    bool value
)
    {
    _SetBaseComplete(value);
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                01/2018
//---------------+---------------+---------------+---------------+---------------+------
void SolidPrimitiveManipulationStrategy::_SetProperty
(
    Utf8CP key, 
    bool const& value
)
    {
    if (0 == strcmp(key, prop_BaseComplete()))
        {
        _SetBaseComplete(value);
        }

    T_Super::_SetProperty(key, value);
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                01/2018
//---------------+---------------+---------------+---------------+---------------+------
BentleyStatus SolidPrimitiveManipulationStrategy::_TryGetProperty
(
    Utf8CP key, 
    bool& value
) const
    {
    if (0 == strcmp(key, prop_BaseComplete()))
        {
        value = _IsBaseComplete();
        return BentleyStatus::SUCCESS;
        }

    return T_Super::_TryGetProperty(key, value);
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                01/2018
//---------------+---------------+---------------+---------------+---------------+------
ISolidPrimitivePtr SolidPrimitiveManipulationStrategy::FinishSolidPrimitive() const
    {
    return _FinishSolidPrimitive();
    }