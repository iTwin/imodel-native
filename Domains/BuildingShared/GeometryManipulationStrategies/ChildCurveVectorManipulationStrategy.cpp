/*--------------------------------------------------------------------------------------+
|
|     $Source: GeometryManipulationStrategies/ChildCurveVectorManipulationStrategy.cpp $
|
|  $Copyright: (c) 2019 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "PublicApi/GeometryManipulationStrategiesApi.h"

USING_NAMESPACE_BUILDING_SHARED

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                03/2018
//---------------+---------------+---------------+---------------+---------------+------
ChildCurveVectorManipulationStrategyPtr ChildCurveVectorManipulationStrategy::Create
(
    CurveVectorCR cv
)
    {
    ChildCurveVectorManipulationStrategyPtr strategy = Create();
    strategy->Init(cv);
    return strategy;
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                07/2018
//---------------+---------------+---------------+---------------+---------------+------
ChildCurveVectorManipulationStrategyPtr ChildCurveVectorManipulationStrategy::Create() 
    { 
    return new ChildCurveVectorManipulationStrategy(); 
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                07/2018
//---------------+---------------+---------------+---------------+---------------+------
void ChildCurveVectorManipulationStrategy::Init
(
    CurveVectorCR cv
)
    {
    m_cvManipulationStrategy = _InitCurveVectorManipulationStrategy(cv);
    m_boundaryType = cv.GetBoundaryType();
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                07/2018
//---------------+---------------+---------------+---------------+---------------+------
CurveVectorManipulationStrategyPtr ChildCurveVectorManipulationStrategy::_InitCurveVectorManipulationStrategy
(
    CurveVectorCR cv
) const
    {
    CurveVectorManipulationStrategyPtr strategy = CurveVectorManipulationStrategy::Create();
    strategy->Init(cv);
    return strategy;
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                01/2018
//---------------+---------------+---------------+---------------+---------------+------
ChildCurveVectorManipulationStrategy::ChildCurveVectorManipulationStrategy()
    : T_Super()
    , m_cvManipulationStrategy(CurveVectorManipulationStrategy::Create())
    , m_boundaryType(CurveVector::BOUNDARY_TYPE_Outer)
    {
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                01/2018
//---------------+---------------+---------------+---------------+---------------+------
ICurvePrimitivePtr ChildCurveVectorManipulationStrategy::_FinishPrimitive() const
    {
    CurveVectorPtr child = m_cvManipulationStrategy->Finish();
    if (child.IsNull())
        return nullptr;

    return ICurvePrimitive::CreateChildCurveVector(child);
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                01/2018
//---------------+---------------+---------------+---------------+---------------+------
bool ChildCurveVectorManipulationStrategy::_IsComplete() const 
    {
    return m_cvManipulationStrategy->IsComplete();
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                01/2018
//---------------+---------------+---------------+---------------+---------------+------
bool ChildCurveVectorManipulationStrategy::_CanAcceptMorePoints() const 
    {
    return m_cvManipulationStrategy->CanAcceptMorePoints();
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                01/2018
//---------------+---------------+---------------+---------------+---------------+------
CurvePrimitiveManipulationStrategyPtr ChildCurveVectorManipulationStrategy::_Clone() const
    {
    return Create();
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                03/2018
//---------------+---------------+---------------+---------------+---------------+------
CurvePrimitivePlacementStrategyPtr ChildCurveVectorManipulationStrategy::_CreateDefaultPlacementStrategy()
    {
    return ChildCurveVectorPlacementStrategy::Create(*this);
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                04/2018
//---------------+---------------+---------------+---------------+---------------+------
bvector<DPoint3d> ChildCurveVectorManipulationStrategy::_GetKeyPoints() const
    {
    return m_cvManipulationStrategy->GetKeyPoints();
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                04/2018
//---------------+---------------+---------------+---------------+---------------+------
bool ChildCurveVectorManipulationStrategy::_IsDynamicKeyPointSet() const
    {
    return m_cvManipulationStrategy->IsDynamicKeyPointSet();
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                04/2018
//---------------+---------------+---------------+---------------+---------------+------
void ChildCurveVectorManipulationStrategy::_AppendDynamicKeyPoint
(
    DPoint3dCR newDynamicKeyPoint
)
    {
    m_cvManipulationStrategy->AppendDynamicKeyPoint(newDynamicKeyPoint);
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                04/2018
//---------------+---------------+---------------+---------------+---------------+------
void ChildCurveVectorManipulationStrategy::_AppendDynamicKeyPoints
(
    bvector<DPoint3d> const& newDynamicKeyPoints
)
    {
    m_cvManipulationStrategy->AppendDynamicKeyPoints(newDynamicKeyPoints);
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                04/2018
//---------------+---------------+---------------+---------------+---------------+------
void ChildCurveVectorManipulationStrategy::_InsertDynamicKeyPoint
(
    DPoint3dCR newDynamicKeyPoint, 
    size_t index
)
    {
    m_cvManipulationStrategy->InsertDynamicKeyPoint(newDynamicKeyPoint, index);
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                04/2018
//---------------+---------------+---------------+---------------+---------------+------
void ChildCurveVectorManipulationStrategy::_InsertDynamicKeyPoints
(
    bvector<DPoint3d> const& newDynamicKeyPoints, 
    size_t index
)
    {
    m_cvManipulationStrategy->InsertDynamicKeyPoints(newDynamicKeyPoints, index);
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                04/2018
//---------------+---------------+---------------+---------------+---------------+------
void ChildCurveVectorManipulationStrategy::_UpdateDynamicKeyPoint
(
    DPoint3dCR newDynamicKeyPoint, 
    size_t index
)
    {
    m_cvManipulationStrategy->UpdateDynamicKeyPoint(newDynamicKeyPoint, index);
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                04/2018
//---------------+---------------+---------------+---------------+---------------+------
void ChildCurveVectorManipulationStrategy::_UpdateDynamicKeyPoints
(
    bvector<DPoint3d> const& newDynamicKeyPoints, 
    size_t index
)
    {
    m_cvManipulationStrategy->UpdateDynamicKeyPoints(newDynamicKeyPoints, index);
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                04/2018
//---------------+---------------+---------------+---------------+---------------+------
void ChildCurveVectorManipulationStrategy::_UpsertDynamicKeyPoint
(
    DPoint3d newDynamicKeyPoint, 
    size_t index
)
    {
    m_cvManipulationStrategy->UpsertDynamicKeyPoint(newDynamicKeyPoint, index);
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                04/2018
//---------------+---------------+---------------+---------------+---------------+------
void ChildCurveVectorManipulationStrategy::_UpsertDynamicKeyPoints
(
    bvector<DPoint3d> const& newDynamicKeyPoints, 
    size_t index
)
    {
    m_cvManipulationStrategy->UpsertDynamicKeyPoints(newDynamicKeyPoints, index);
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                04/2018
//---------------+---------------+---------------+---------------+---------------+------
void ChildCurveVectorManipulationStrategy::_ResetDynamicKeyPoint()
    {
    m_cvManipulationStrategy->ResetDynamicKeyPoint();
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                04/2018
//---------------+---------------+---------------+---------------+---------------+------
void ChildCurveVectorManipulationStrategy::_AppendKeyPoint
(
    DPoint3dCR newKeyPoint
)
    {
    m_cvManipulationStrategy->AppendKeyPoint(newKeyPoint);
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                04/2018
//---------------+---------------+---------------+---------------+---------------+------
void ChildCurveVectorManipulationStrategy::_AppendKeyPoints
(
    bvector<DPoint3d> const& newKeyPoints
)
    {
    m_cvManipulationStrategy->AppendKeyPoints(newKeyPoints);
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                04/2018
//---------------+---------------+---------------+---------------+---------------+------
void ChildCurveVectorManipulationStrategy::_InsertKeyPoint
(
    DPoint3dCR newKeyPoint, 
    size_t index
)
    {
    m_cvManipulationStrategy->InsertKeyPoint(newKeyPoint, index);
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                04/2018
//---------------+---------------+---------------+---------------+---------------+------
void ChildCurveVectorManipulationStrategy::_ReplaceKeyPoint
(
    DPoint3dCR newKeyPoint, 
    size_t index
)
    {
    m_cvManipulationStrategy->ReplaceKeyPoint(newKeyPoint, index);
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                04/2018
//---------------+---------------+---------------+---------------+---------------+------
void ChildCurveVectorManipulationStrategy::_PopKeyPoint()
    {
    m_cvManipulationStrategy->PopKeyPoint();
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                04/2018
//---------------+---------------+---------------+---------------+---------------+------
void ChildCurveVectorManipulationStrategy::_RemoveKeyPoint
(
    size_t index
)
    {
    m_cvManipulationStrategy->RemoveKeyPoint(index);
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                04/2018
//---------------+---------------+---------------+---------------+---------------+------
void ChildCurveVectorManipulationStrategy::_Clear()
    {
    m_cvManipulationStrategy->Clear();
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                04/2018
//---------------+---------------+---------------+---------------+---------------+------
void ChildCurveVectorManipulationStrategy::_SetDynamicState(DynamicStateBaseCR state)
    {
    m_cvManipulationStrategy->SetDynamicState(state);
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                04/2018
//---------------+---------------+---------------+---------------+---------------+------
DynamicStateBaseCPtr ChildCurveVectorManipulationStrategy::_GetDynamicState() const
    {
    return m_cvManipulationStrategy->GetDynamicState();
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                07/2018
//---------------+---------------+---------------+---------------+---------------+------
bvector<ConstructionGeometry> ChildCurveVectorManipulationStrategy::_FinishConstructionGeometry() const
    {
    return m_cvManipulationStrategy->FinishConstructionGeometry();
    }