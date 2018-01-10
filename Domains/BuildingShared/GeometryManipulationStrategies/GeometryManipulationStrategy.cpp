/*--------------------------------------------------------------------------------------+
|
|     $Source: GeometryManipulationStrategies/GeometryManipulationStrategy.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
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
    ManipulateKeyPoint([&] { _AppendKeyPoint(newKeyPoint); });
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                01/2018
//---------------+---------------+---------------+---------------+---------------+------
void GeometryManipulationStrategy::AppendKeyPoints
(
    bvector<DPoint3d> const& newKeyPoints
)
    {
    ManipulateKeyPoint([&] { _AppendKeyPoints(newKeyPoints); });
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
    ManipulateKeyPoint([&] { _InsertKeyPoint(newKeyPoint, index); });
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
    ManipulateKeyPoint([&] { _ReplaceKeyPoint(newKeyPoint, index); });
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
    ManipulateKeyPoint([&] { _AppendDynamicKeyPoint(newDynamicKeyPoint); });
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                12/2017
//---------------+---------------+---------------+---------------+---------------+------
void GeometryManipulationStrategy::AppendDynamicKeyPoints
(
    bvector<DPoint3d> const& newDynamicKeyPoints
)
    {
    ManipulateKeyPoint([&] { _AppendDynamicKeyPoints(newDynamicKeyPoints); });
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
    ManipulateKeyPoint([&] { _InsertDynamicKeyPoint(newDynamicKeyPoint, index); });
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
    ManipulateKeyPoint([&] { _InsertDynamicKeyPoints(newDynamicKeyPoints, index); });
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
    ManipulateKeyPoint([&] { _UpdateDynamicKeyPoint(newDynamicKeyPoint, index); });
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
    ManipulateKeyPoint([&] { _UpdateDynamicKeyPoints(newDynamicKeyPoints, index); });
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
    ManipulateKeyPoint([&] { _UpsertDynamicKeyPoint(newDynamicKeyPoint, index); });
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
    ManipulateKeyPoint([&] { _UpsertDynamicKeyPoints(newDynamicKeyPoints, index); });
    }