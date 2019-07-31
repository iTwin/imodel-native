/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#include "PublicApi/GeometryManipulationStrategiesApi.h"

USING_NAMESPACE_BUILDING_SHARED

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                01/2018
//---------------+---------------+---------------+---------------+---------------+------
template <typename Func> 
void GeometryManipulationStrategy::ManipulateKeyPoint
(
    Func manipulationFn
)
    {
    _ResetDynamicKeyPoint();
    manipulationFn();
    _OnKeyPointsChanged();
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                12/2017
//---------------+---------------+---------------+---------------+---------------+------
void GeometryManipulationStrategy::AppendKeyPoint
(
    DPoint3dCR newKeyPoint
)
    {
    ManipulateKeyPoint([&] { _AppendKeyPoint(AdjustPoint(newKeyPoint)); });
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                01/2018
//---------------+---------------+---------------+---------------+---------------+------
void GeometryManipulationStrategy::AppendKeyPoints
(
    bvector<DPoint3d> const& newKeyPoints
)
    {
    bvector<DPoint3d> adjusted(newKeyPoints.size());
    std::transform(newKeyPoints.begin(),
                   newKeyPoints.end(),
                   adjusted.begin(),
                   [&](DPoint3d point) {return AdjustPoint(point); });
    ManipulateKeyPoint([&] { _AppendKeyPoints(adjusted); });
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                12/2017
//---------------+---------------+---------------+---------------+---------------+------
void GeometryManipulationStrategy::InsertKeyPoint
(
    DPoint3dCR newKeyPoint,
    size_t index
)
    {
    ManipulateKeyPoint([&] { _InsertKeyPoint(AdjustPoint(newKeyPoint), index); });
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                12/2017
//---------------+---------------+---------------+---------------+---------------+------
void GeometryManipulationStrategy::ReplaceKeyPoint
(
    DPoint3dCR newKeyPoint,
    size_t index
)
    {
    ManipulateKeyPoint([&] { _ReplaceKeyPoint(AdjustPoint(newKeyPoint), index); });
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                12/2017
//---------------+---------------+---------------+---------------+---------------+------
void GeometryManipulationStrategy::PopKeyPoint()
    {
    ManipulateKeyPoint([&] { _PopKeyPoint(); });
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                12/2017
//---------------+---------------+---------------+---------------+---------------+------
void GeometryManipulationStrategy::RemoveKeyPoint
(
    size_t index
)
    {
    ManipulateKeyPoint([&] { _RemoveKeyPoint(index); });
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                12/2017
//---------------+---------------+---------------+---------------+---------------+------
void GeometryManipulationStrategy::AppendDynamicKeyPoint
(
    DPoint3dCR newDynamicKeyPoint
)
    {
    ManipulateKeyPoint([&] { _AppendDynamicKeyPoint(AdjustPoint(newDynamicKeyPoint)); });
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                12/2017
//---------------+---------------+---------------+---------------+---------------+------
void GeometryManipulationStrategy::AppendDynamicKeyPoints
(
    bvector<DPoint3d> const& newDynamicKeyPoints
)
    {
    bvector<DPoint3d> adjusted(newDynamicKeyPoints.size());
    std::transform(newDynamicKeyPoints.begin(),
                   newDynamicKeyPoints.end(),
                   adjusted.begin(),
                   [&](DPoint3d point) {return AdjustPoint(point); });
    ManipulateKeyPoint([&] { _AppendDynamicKeyPoints(adjusted); });
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                12/2017
//---------------+---------------+---------------+---------------+---------------+------
void GeometryManipulationStrategy::InsertDynamicKeyPoint
(
    DPoint3dCR newDynamicKeyPoint,
    size_t index
)
    {
    ManipulateKeyPoint([&] { _InsertDynamicKeyPoint(AdjustPoint(newDynamicKeyPoint), index); });
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                12/2017
//---------------+---------------+---------------+---------------+---------------+------
void GeometryManipulationStrategy::InsertDynamicKeyPoints
(
    bvector<DPoint3d> const& newDynamicKeyPoints,
    size_t index
)
    {
    bvector<DPoint3d> adjusted(newDynamicKeyPoints.size());
    std::transform(newDynamicKeyPoints.begin(),
                   newDynamicKeyPoints.end(),
                   adjusted.begin(),
                   [&](DPoint3d point) {return AdjustPoint(point); });
    ManipulateKeyPoint([&] { _InsertDynamicKeyPoints(adjusted, index); });
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                12/2017
//---------------+---------------+---------------+---------------+---------------+------
void GeometryManipulationStrategy::UpdateDynamicKeyPoint
(
    DPoint3dCR newDynamicKeyPoint,
    size_t index
)
    {
    ManipulateKeyPoint([&] { _UpdateDynamicKeyPoint(AdjustPoint(newDynamicKeyPoint), index); });
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                12/2017
//---------------+---------------+---------------+---------------+---------------+------
void GeometryManipulationStrategy::UpdateDynamicKeyPoints
(
    bvector<DPoint3d> const& newDynamicKeyPoints,
    size_t index
)
    {
    bvector<DPoint3d> adjusted(newDynamicKeyPoints.size());
    std::transform(newDynamicKeyPoints.begin(),
                   newDynamicKeyPoints.end(),
                   adjusted.begin(),
                   [&](DPoint3d point) {return AdjustPoint(point); });
    ManipulateKeyPoint([&] { _UpdateDynamicKeyPoints(adjusted, index); });
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                12/2017
//---------------+---------------+---------------+---------------+---------------+------
void GeometryManipulationStrategy::UpsertDynamicKeyPoint
(
    DPoint3d newDynamicKeyPoint,
    size_t index
)
    {
    ManipulateKeyPoint([&] { _UpsertDynamicKeyPoint(AdjustPoint(newDynamicKeyPoint), index); });
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                12/2017
//---------------+---------------+---------------+---------------+---------------+------
void GeometryManipulationStrategy::UpsertDynamicKeyPoints
(
    bvector<DPoint3d> const& newDynamicKeyPoints,
    size_t index
)
    {
    bvector<DPoint3d> adjusted(newDynamicKeyPoints.size());
    std::transform(newDynamicKeyPoints.begin(),
                   newDynamicKeyPoints.end(),
                   adjusted.begin(),
                   [&](DPoint3d point) {return AdjustPoint(point); });
    ManipulateKeyPoint([&] { _UpsertDynamicKeyPoints(adjusted, index); });
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                01/2018
//---------------+---------------+---------------+---------------+---------------+------
void GeometryManipulationStrategy::Clear()
    {
    ManipulateKeyPoint([&] { _Clear(); });
    }