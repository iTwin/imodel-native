/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#include "PublicApi/GeometryManipulationStrategiesApi.h"

#define DEFAULT_MANIPULATION_STRATEGY LineManipulationStrategy::Create()

BUILDING_SHARED_REFCOUNTED_PTR_AND_TYPEDEFS(CurveVectorDynamicState)
BEGIN_BUILDING_SHARED_NAMESPACE

//=======================================================================================
// @bsiclass                                     Mindaugas.Butkus               04/2018
//=======================================================================================
struct CurveVectorDynamicState : DynamicStateBase
    {
    typedef bpair<CurvePrimitiveManipulationStrategyPtr, DynamicStateBaseCPtr> StatePair;

    private:
        bvector<StatePair> m_state;

        CurveVectorDynamicState(bvector<StatePair> const& state) : m_state(state) {}

    protected:
        DynamicStateBasePtr _Clone() const override
            {
            return Create(m_state);
            }

    public:
        static CurveVectorDynamicStatePtr Create(bvector<StatePair> const& state) { return new CurveVectorDynamicState(state); }
        bvector<StatePair> const& GetState() const { return m_state; }
    };

END_BUILDING_SHARED_NAMESPACE

USING_NAMESPACE_BUILDING_SHARED

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                02/2018
//---------------+---------------+---------------+---------------+---------------+------
CurvePrimitiveStrategyContainer::CurvePrimitiveStrategyContainer()
    : m_defaultNewGeometryType(DefaultNewGeometryType::Line)
    , m_defaultLinePlacementStrategyType(LinePlacementStrategyType::Points)
    , m_defaultArcPlacementMethod(ArcPlacementMethod::StartMidEnd)
    , m_defaultLineStringPlacementStrategyType(LineStringPlacementStrategyType::Points)
    {}

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                02/2018
//---------------+---------------+---------------+---------------+---------------+------
void CurvePrimitiveStrategyContainer::Clear()
    {
    m_primitiveManipulationStrategies.clear();
    m_primitivePlacementStrategies.clear();
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                02/2018
//---------------+---------------+---------------+---------------+---------------+------
void CurvePrimitiveStrategyContainer::Pop()
    {
    m_primitiveManipulationStrategies.pop_back();
    m_primitivePlacementStrategies.pop_back();
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                02/2019
//---------------+---------------+---------------+---------------+---------------+------
void CurvePrimitiveStrategyContainer::ResetDynamicKeyPoint()
    {
    for (CurvePrimitivePlacementStrategyPtr const& strategy : m_primitivePlacementStrategies)
        {
        strategy->ResetDynamicKeyPoint();
        }
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                02/2019
//---------------+---------------+---------------+---------------+---------------+------
void CurvePrimitiveStrategyContainer::PrepareNextStrategyAfterTypeOrPlacementChange()
    {
    if (m_primitivePlacementStrategies.empty())
        return;

    bvector<DPoint3d> keyPoints = m_primitivePlacementStrategies.back()->GetKeyPoints();

    CurvePrimitivePlacementStrategyPtr oldStrategy = m_primitivePlacementStrategies.back();
    if (!oldStrategy->IsComplete())
        {
        Pop();
        AddNext();
        CurvePrimitivePlacementStrategyPtr newStrategy = m_primitivePlacementStrategies.back();

        newStrategy->PopKeyPoint();
        oldStrategy->PrepareReplacement(*newStrategy);
        
        if (newStrategy->IsComplete() && !newStrategy->CanAcceptMorePoints())
            newStrategy->PopKeyPoint();
        }
    else
        {
        AddNext();
        }
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                02/2018
//---------------+---------------+---------------+---------------+---------------+------
void CurvePrimitiveStrategyContainer::SetDefaultNewGeometryType
(
    DefaultNewGeometryType type
)
    {
    if (type == m_defaultNewGeometryType)
        return;

    ResetDynamicKeyPoint();
    m_defaultNewGeometryType = type;
    PrepareNextStrategyAfterTypeOrPlacementChange();
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                02/2018
//---------------+---------------+---------------+---------------+---------------+------
void CurvePrimitiveStrategyContainer::SetDefaultPlacementStrategy
(
    LinePlacementStrategyType type
)
    {
    if (type == m_defaultLinePlacementStrategyType)
        return;

    ResetDynamicKeyPoint();
    m_defaultLinePlacementStrategyType = type;
    PrepareNextStrategyAfterTypeOrPlacementChange();
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                02/2018
//---------------+---------------+---------------+---------------+---------------+------
void CurvePrimitiveStrategyContainer::SetDefaultPlacementStrategy
(
    ArcPlacementMethod method
)
    {
    if (method == m_defaultArcPlacementMethod)
        return;

    ResetDynamicKeyPoint();
    m_defaultArcPlacementMethod = method;
    PrepareNextStrategyAfterTypeOrPlacementChange();
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                02/2018
//---------------+---------------+---------------+---------------+---------------+------
void CurvePrimitiveStrategyContainer::SetDefaultPlacementStrategy
(
    LineStringPlacementStrategyType type
)
    {
    if (type == m_defaultLineStringPlacementStrategyType)
        return;

    ResetDynamicKeyPoint();
    m_defaultLineStringPlacementStrategyType = type;
    PrepareNextStrategyAfterTypeOrPlacementChange();
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                02/2018
//---------------+---------------+---------------+---------------+---------------+------
bool CurvePrimitiveStrategyContainer::IsEmpty() const
    {
    return m_primitiveManipulationStrategies.empty();
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                02/2018
//---------------+---------------+---------------+---------------+---------------+------
bool CurvePrimitiveStrategyContainer::IsComplete() const
    {
    if (IsEmpty())
        return false;

    for (auto iter = m_primitivePlacementStrategies.begin(); m_primitivePlacementStrategies.end() != iter; ++iter)
        {
        if ((m_primitivePlacementStrategies.end() - 1) == iter)
            {
            if ((*iter)->GetKeyPoints().size() == 1 && m_primitivePlacementStrategies.size() > 1)
                {
                DPoint3d firstPoint = (*iter)->GetKeyPoints().front();
                DPoint3d lastPoint = (*(iter - 1))->GetKeyPoints().back();
                if (firstPoint.AlmostEqual(lastPoint))
                    continue;
                }
            }

        if (!(*iter)->IsComplete())
            return false;
        }

    return true;
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                02/2018
//---------------+---------------+---------------+---------------+---------------+------
bool CurvePrimitiveStrategyContainer::CanAcceptMorePoints() const
    {
    if (IsEmpty())
        return false;

    return m_primitivePlacementStrategies.back()->CanAcceptMorePoints();
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                02/2019
//---------------+---------------+---------------+---------------+---------------+------
void CurvePrimitiveStrategyContainer::CreateNext
(
    CurvePrimitiveManipulationStrategyPtr& manipStrategy, 
    CurvePrimitivePlacementStrategyPtr& placementStrategy
) const
    {
    switch (m_defaultNewGeometryType)
        {
        case DefaultNewGeometryType::LineString:
            {
            LineStringManipulationStrategyPtr lsManipStrategy = LineStringManipulationStrategy::Create();
            manipStrategy = lsManipStrategy;
            placementStrategy = LineStringPlacementStrategy::Create(m_defaultLineStringPlacementStrategyType, *lsManipStrategy);
            }
            break;
        case DefaultNewGeometryType::Line:
            {
            LineManipulationStrategyPtr lManipStrategy = LineManipulationStrategy::Create();
            manipStrategy = lManipStrategy;
            placementStrategy = LinePlacementStrategy::Create(m_defaultLinePlacementStrategyType, *lManipStrategy);
            }
            break;
        case DefaultNewGeometryType::Arc:
            {
            ArcManipulationStrategyPtr aManipStrategy = ArcManipulationStrategy::Create();
            manipStrategy = aManipStrategy;
            placementStrategy = ArcPlacementStrategy::Create(m_defaultArcPlacementMethod, *aManipStrategy);
            }
            break;
        case DefaultNewGeometryType::Spline:
            {
            SplineControlPointsManipulationStrategyPtr scpManipStrategy = SplineControlPointsManipulationStrategy::Create(SplineControlPointsManipulationStrategy::default_Order);
            manipStrategy = scpManipStrategy;
            placementStrategy = SplineControlPointsPlacementStrategy::Create(*scpManipStrategy);
            }
            break;
        case DefaultNewGeometryType::InterpolationCurve:
            {
            SplineThroughPointsManipulationStrategyPtr stpManipStrategy = SplineThroughPointsManipulationStrategy::Create();
            manipStrategy = stpManipStrategy;
            placementStrategy = SplineThroughPointsPlacementStrategy::Create(*stpManipStrategy);
            }
            break;
        default:
            BeAssert(false);
        }
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                02/2018
//---------------+---------------+---------------+---------------+---------------+------
void CurvePrimitiveStrategyContainer::AddNext()
    {
    BeAssert(IsEmpty() || IsComplete());

    CurvePrimitiveManipulationStrategyPtr manipStrategy;
    CurvePrimitivePlacementStrategyPtr placementStrategy;
    CreateNext(manipStrategy, placementStrategy);
    if (manipStrategy.IsNull() || placementStrategy.IsNull())
        return;

    if (!m_primitiveManipulationStrategies.empty())
        placementStrategy->AddKeyPoint(m_primitiveManipulationStrategies.back()->GetLastKeyPoint());

    BeAssert(manipStrategy.IsValid());
    m_primitiveManipulationStrategies.push_back(manipStrategy);
    m_primitivePlacementStrategies.push_back(placementStrategy);
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                02/2018
//---------------+---------------+---------------+---------------+---------------+------
void CurvePrimitiveStrategyContainer::Append
(
    CurvePrimitiveManipulationStrategyR manipStrategy
)
    {
    m_primitiveManipulationStrategies.push_back(&manipStrategy);
    m_primitivePlacementStrategies.push_back(manipStrategy.CreateDefaultPlacementStrategy());
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                01/2018
//---------------+---------------+---------------+---------------+---------------+------
CurveVectorManipulationStrategyPtr CurveVectorManipulationStrategy::Create
(
    CurveVectorCR cv
)
    {
    CurveVectorManipulationStrategyPtr strategy = Create();
    strategy->Init(cv);

    if (cv.GetBoundaryType() == CurveVector::BOUNDARY_TYPE_Outer)
        {
        bvector<DPoint3d> const& keyPoints = strategy->_GetKeyPoints();
        if (keyPoints.size() > 1 && keyPoints.front().AlmostEqual(keyPoints.back()))
            strategy->_PopKeyPoint();

        DPoint3d centroid;
        DVec3d normal;
        double area;
        cv.CentroidNormalArea(centroid, normal, area);
        strategy->m_workingPlane = DPlane3d::FromOriginAndNormal(centroid, normal);
        }

    return strategy;
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                01/2018
//---------------+---------------+---------------+---------------+---------------+------
CurveVectorManipulationStrategy::CurveVectorManipulationStrategy()
    : T_Super()
    , m_workingPlane(DPlane3d::FromOriginAndNormal({0,0,0}, DVec3d::From(0, 0, 1)))
    {
    RegisterDPlane3dProperty(prop_WorkingPlane());
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                01/2018
//---------------+---------------+---------------+---------------+---------------+------
void CurveVectorManipulationStrategy::ConnectStartEnd
(
    CurveVectorR cv
) const
    {
    if (cv.empty())
        return;

    DPoint3d start, end;
    if (!cv.GetStartEnd(start, end))
        return;
    if (!start.AlmostEqual(end))
        {
        ICurvePrimitivePtr lastPrimitive = cv.back();
        if (lastPrimitive.IsNull())
            return;

        if (nullptr != lastPrimitive->GetLineStringP())
            lastPrimitive->GetLineStringP()->push_back(start);
        else
            {
            ICurvePrimitivePtr endStart = ICurvePrimitive::CreateLine(DSegment3d::From(end, start));
            cv.Add(endStart);
            }
        }
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                12/2017
//---------------+---------------+---------------+---------------+---------------+------
CurveVectorPtr CurveVectorManipulationStrategy::_Finish
(
    bool connectEndStart
) const
    {
    CurveVectorPtr cv = CurveVector::Create(CurveVector::BOUNDARY_TYPE_Open);
    
    for (CurvePrimitiveManipulationStrategyPtr const& strategy : m_primitiveStrategyContainer.GetManipulationStrategies())
        {
        ICurvePrimitivePtr primitive = strategy->FinishPrimitive();
        if (primitive.IsNull())
            continue;

        if (primitive->GetCurvePrimitiveType() == ICurvePrimitive::CurvePrimitiveType::CURVE_PRIMITIVE_TYPE_CurveVector)
            {
            if (!cv->empty() && cv->GetBoundaryType() != CurveVector::BOUNDARY_TYPE_ParityRegion)
                {
                CurveVectorPtr child = cv->Clone();
                child->SetBoundaryType(CurveVector::BOUNDARY_TYPE_Outer);
                cv = CurveVector::Create(CurveVector::BOUNDARY_TYPE_ParityRegion);
                cv->Add(child);
                primitive->GetChildCurveVectorP()->SetBoundaryType(CurveVector::BOUNDARY_TYPE_Inner);
                }
            else
                {
                cv->SetBoundaryType(CurveVector::BOUNDARY_TYPE_ParityRegion);
                primitive->GetChildCurveVectorP()->SetBoundaryType(CurveVector::BOUNDARY_TYPE_Outer);
                }
            }

        cv->Add(primitive);
        }

    if (cv->empty())
        return nullptr;

    if (cv->GetBoundaryType() != CurveVector::BOUNDARY_TYPE_ParityRegion && connectEndStart)
        ConnectStartEnd(*cv);

    return cv;
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                07/2018
//---------------+---------------+---------------+---------------+---------------+------
bvector<ConstructionGeometry> CurveVectorManipulationStrategy::_FinishConstructionGeometry() const
    {
    bvector<CurvePrimitiveManipulationStrategyPtr> const& strategies = m_primitiveStrategyContainer.GetManipulationStrategies();
    if (strategies.empty())
        return bvector<ConstructionGeometry>();

    if (strategies.back().IsNull())
        {
        BeAssert(strategies.back().IsValid());
        return bvector<ConstructionGeometry>();
        }

    return strategies.back()->FinishConstructionGeometry();
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                12/2017
//---------------+---------------+---------------+---------------+---------------+------
CurveVectorPtr CurveVectorManipulationStrategy::Finish
(
    bool connectEndStart
) const
    {
    return _Finish(connectEndStart);
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                01/2018
//---------------+---------------+---------------+---------------+---------------+------
bool CurveVectorManipulationStrategy::_IsComplete() const
    {
    return m_primitiveStrategyContainer.IsComplete();
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                01/2018
//---------------+---------------+---------------+---------------+---------------+------
bvector<DPoint3d> CurveVectorManipulationStrategy::_GetKeyPoints() const
    {
    bvector<DPoint3d> allKeyPoints;

    for (CurvePrimitivePlacementStrategyPtr const& strategy : m_primitiveStrategyContainer.GetPlacementStrategies())
        {
        bvector<DPoint3d> const& strategyKeyPoints = strategy->GetKeyPoints();
        if (strategyKeyPoints.empty())
            continue;

        if (!allKeyPoints.empty() && allKeyPoints.back().AlmostEqual(strategyKeyPoints.front()))
            allKeyPoints.pop_back();

        allKeyPoints.insert(allKeyPoints.end(), strategyKeyPoints.begin(), strategyKeyPoints.end());
        }

    return allKeyPoints;
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                01/2018
//---------------+---------------+---------------+---------------+---------------+------
bool CurveVectorManipulationStrategy::_IsDynamicKeyPointSet() const
    {
    bvector<CurvePrimitiveManipulationStrategyPtr> const& primitiveStrategies = m_primitiveStrategyContainer.GetManipulationStrategies();
    return std::any_of(primitiveStrategies.rbegin(), primitiveStrategies.rend(),
                       [] (CurvePrimitiveManipulationStrategyPtr const& s) { return s->IsDynamicKeyPointSet(); });
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                01/2018
//---------------+---------------+---------------+---------------+---------------+------
void CurveVectorManipulationStrategy::_ResetDynamicKeyPoint()
    {
    m_primitiveStrategyContainer.ResetDynamicKeyPoint();
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                01/2018
//---------------+---------------+---------------+---------------+---------------+------
bool CurveVectorManipulationStrategy::IsLastStrategyReadyForPop() const
    {
    bvector<CurvePrimitivePlacementStrategyPtr> const& primitiveStrategies = m_primitiveStrategyContainer.GetPlacementStrategies();

    if (primitiveStrategies.empty())
        return false;

    if (primitiveStrategies.size() == 1 && primitiveStrategies.back()->IsEmpty())
        return true;

    if (primitiveStrategies.size() > 1)
        {
        CurvePrimitivePlacementStrategyR lastStrategy = *primitiveStrategies.back();
        if (lastStrategy.IsEmpty())
            return true;

        CurvePrimitivePlacementStrategyR secondToLastStrategy = *primitiveStrategies[primitiveStrategies.size() - 2];
        if (lastStrategy.GetKeyPoints().size() == 1 &&
            lastStrategy.GetKeyPoints().front().AlmostEqual(secondToLastStrategy.GetKeyPoints().back()))
            return true;
        }

    return false;
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                01/2018
//---------------+---------------+---------------+---------------+---------------+------
void CurveVectorManipulationStrategy::_PopKeyPoint()
    {
    if (m_primitiveStrategyContainer.IsEmpty())
        return;

    if (IsLastStrategyReadyForPop())
        {
        DPoint3d firstKeyPoint;
        bool wasEmpty = m_primitiveStrategyContainer.GetPlacementStrategies().back()->GetKeyPoints().empty();
        if (!wasEmpty)
            firstKeyPoint = m_primitiveStrategyContainer.GetPlacementStrategies().back()->GetKeyPoints().front();

        m_primitiveStrategyContainer.Pop();
        if (!wasEmpty && m_primitiveStrategyContainer.GetPlacementStrategies().back()->GetKeyPoints().back().AlmostEqual(firstKeyPoint))
            m_primitiveStrategyContainer.GetPlacementStrategies().back()->PopKeyPoint();
        }
    else
        {
        m_primitiveStrategyContainer.GetPlacementStrategies().back()->PopKeyPoint();
        }
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                02/2018
//---------------+---------------+---------------+---------------+---------------+------
CurvePrimitivePlacementStrategyR CurveVectorManipulationStrategy::GetStrategyForSetProperty()
    {
    if (m_primitiveStrategyContainer.IsEmpty())
        m_primitiveStrategyContainer.AddNext();

    return *m_primitiveStrategyContainer.GetPlacementStrategies().back();
    }
    
//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                01/2018
//---------------+---------------+---------------+---------------+---------------+------
CurvePrimitivePlacementStrategyR CurveVectorManipulationStrategy::GetStrategyForAppend()
    {
    if (m_primitiveStrategyContainer.IsEmpty() || (m_primitiveStrategyContainer.IsComplete() && !m_primitiveStrategyContainer.CanAcceptMorePoints()))
        m_primitiveStrategyContainer.AddNext();

    return *m_primitiveStrategyContainer.GetPlacementStrategies().back();
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                01/2018
//---------------+---------------+---------------+---------------+---------------+------
void CurveVectorManipulationStrategy::_AppendKeyPoint
(
    DPoint3dCR newKeyPoint
)
    {
    GetStrategyForAppend().AddKeyPoint(AdjustPoint(newKeyPoint));
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                01/2018
//---------------+---------------+---------------+---------------+---------------+------
void CurveVectorManipulationStrategy::_AppendDynamicKeyPoint
(
    DPoint3dCR newDynamicKeyPoint
)
    {
    GetStrategyForAppend().AddDynamicKeyPoint(AdjustPoint(newDynamicKeyPoint));
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                03/2018
//---------------+---------------+---------------+---------------+---------------+------
void CurveVectorManipulationStrategy::_AppendDynamicKeyPoints
(
    bvector<DPoint3d> const& newDynamicKeyPoints
)
    {
    bvector<DPoint3d> adjusted(newDynamicKeyPoints.size());
    std::transform(newDynamicKeyPoints.begin(),
                   newDynamicKeyPoints.end(),
                   adjusted.begin(),
                   [&] (DPoint3d point) { return AdjustPoint(point); });
    GetStrategyForAppend().AddDynamicKeyPoints(adjusted);
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                01/2018
//---------------+---------------+---------------+---------------+---------------+------
void CurveVectorManipulationStrategy::_Clear()
    {
    m_primitiveStrategyContainer.Clear();
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                01/2018
//---------------+---------------+---------------+---------------+---------------+------
void CurveVectorManipulationStrategy::ChangeDefaultNewGeometryType
(
    DefaultNewGeometryType newGeometryType
)
    {
    m_primitiveStrategyContainer.SetDefaultNewGeometryType(newGeometryType);
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Haroldas Vitunskas              01/2018
//---------------+---------------+---------------+---------------+---------------+------
bool CurveVectorManipulationStrategy::FinishContiniousPrimitive()
    {
    if (m_primitiveStrategyContainer.IsComplete() && m_primitiveStrategyContainer.GetManipulationStrategies().back()->IsContinious())
        m_primitiveStrategyContainer.AddNext();
    else
        return false;
    
    return true;
     }
     
//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                01/2018
//---------------+---------------+---------------+---------------+---------------+------
void CurveVectorManipulationStrategy::ChangeDefaultPlacementStrategy
(
    LinePlacementStrategyType newPlacementStrategyType
)
    {
    m_primitiveStrategyContainer.SetDefaultPlacementStrategy(newPlacementStrategyType);
    }

    
//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                01/2018
//---------------+---------------+---------------+---------------+---------------+------
void CurveVectorManipulationStrategy::ChangeDefaultPlacementStrategy
(
    ArcPlacementMethod method
)
    {
    m_primitiveStrategyContainer.SetDefaultPlacementStrategy(method);
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                01/2018
//---------------+---------------+---------------+---------------+---------------+------
void CurveVectorManipulationStrategy::ChangeDefaultPlacementStrategy
(
    LineStringPlacementStrategyType newPlacementStrategyType
)
    {
    m_primitiveStrategyContainer.SetDefaultPlacementStrategy(newPlacementStrategyType);
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                01/2018
//---------------+---------------+---------------+---------------+---------------+------
DPoint3d CurveVectorManipulationStrategy::_AdjustPoint
(
    DPoint3d keyPoint
) const
    {
    DPoint3d projection;
    m_workingPlane.ProjectPoint(projection, keyPoint);
    return projection;
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                01/2018
//---------------+---------------+---------------+---------------+---------------+------
void CurveVectorManipulationStrategy::_SetProperty
(
    Utf8CP key,
    DPlane3dCR value
)
    {
    if (0 == strcmp(key, prop_WorkingPlane()))
        {
        m_workingPlane = value;
        }

    GetStrategyForSetProperty().SetProperty(key, value);
    T_Super::_SetProperty(key, value);
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                01/2018
//---------------+---------------+---------------+---------------+---------------+------
BentleyStatus CurveVectorManipulationStrategy::_TryGetProperty
(
    Utf8CP key,
    DPlane3dR value
) const
    {
    if (0 == strcmp(key, prop_WorkingPlane()))
        {
        value = m_workingPlane;
        return BentleyStatus::SUCCESS;
        }

    if (m_primitiveStrategyContainer.IsEmpty())
        return BentleyStatus::ERROR;
    BentleyStatus status = m_primitiveStrategyContainer.GetPlacementStrategies().back()->TryGetProperty(key, value);

    if (BentleyStatus::SUCCESS != status)
        return T_Super::_TryGetProperty(key, value);

    return status;
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                02/2018
//---------------+---------------+---------------+---------------+---------------+------
bvector<PrimitiveStrategyWithKeyPointIndexRange> CurvePrimitiveStrategyContainer::GetStrategies
(
    size_t keyPointIndex
) const
    {
    bvector<PrimitiveStrategyWithKeyPointIndexRange> primitiveStrategies;

    size_t currentPrimitiveBeginIndex = 0;
    DPoint3d lastPrimitiveKeyPoint;
    for (CurvePrimitiveManipulationStrategyPtr const& primitiveStrategy : m_primitiveManipulationStrategies)
        {
        bvector<DPoint3d> const& keyPoints = primitiveStrategy->GetKeyPoints();
        if (keyPoints.empty())
            continue;

        if (currentPrimitiveBeginIndex > 0 && !lastPrimitiveKeyPoint.AlmostEqual(keyPoints.front()))
            ++currentPrimitiveBeginIndex;

        size_t currentPrimitiveEndIndex = currentPrimitiveBeginIndex + keyPoints.size() - 1;
        
        if (keyPointIndex >= currentPrimitiveBeginIndex && keyPointIndex <= currentPrimitiveEndIndex)
            primitiveStrategies.push_back({primitiveStrategy, {currentPrimitiveBeginIndex, currentPrimitiveEndIndex}});

        lastPrimitiveKeyPoint = keyPoints.back();
        currentPrimitiveBeginIndex = currentPrimitiveEndIndex;
        }

    return primitiveStrategies;
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                02/2018
//---------------+---------------+---------------+---------------+---------------+------
template <typename T> void CurveVectorManipulationStrategy::UpdateKeyPoint
(
    size_t index,
    T updateFn
)
    {
    bvector<DPoint3d> keyPoints = _GetKeyPoints();
    
    if ((keyPoints.size() > 1) &&
        (0 == index || keyPoints.size() - 1 == index) &&
        (keyPoints.front().AlmostEqual(keyPoints.back())))
        {
        bvector<PrimitiveStrategyWithKeyPointIndexRange> primitiveStrategiesFront = m_primitiveStrategyContainer.GetStrategies(0);
        bvector<PrimitiveStrategyWithKeyPointIndexRange> primitiveStrategiesBack = m_primitiveStrategyContainer.GetStrategies(keyPoints.size() - 1);
        BeAssert(primitiveStrategiesFront.size() == 1);
        BeAssert(primitiveStrategiesBack.size() == 1);

        updateFn(primitiveStrategiesFront.front(), 0);
        updateFn(primitiveStrategiesBack.front(), keyPoints.size() - 1 - primitiveStrategiesBack.front().second.first);
        return;
        }

    bvector<PrimitiveStrategyWithKeyPointIndexRange> primitiveStrategies = m_primitiveStrategyContainer.GetStrategies(index);
    for (PrimitiveStrategyWithKeyPointIndexRange const& strategyWithRange : primitiveStrategies)
        {
        PrimitiveStrategyKeyPointIndexRange indexRange = strategyWithRange.second;
        if (!(index >= indexRange.first && index <= indexRange.second))
            {
            BeAssert(false && "Bad KeyPoint index range");
            _ResetDynamicKeyPoint();
            return;
            }

        updateFn(strategyWithRange, index - indexRange.first);
        }
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                02/2018
//---------------+---------------+---------------+---------------+---------------+------
void CurveVectorManipulationStrategy::_UpdateDynamicKeyPoint
(
    DPoint3dCR newDynamicKeyPoint, 
    size_t index
)
    {
    UpdateKeyPoint(index, [&] (PrimitiveStrategyWithKeyPointIndexRange const& strategyWithRange, size_t indexInPrimitive)
        {
        strategyWithRange.first->UpdateDynamicKeyPoint(AdjustPoint(newDynamicKeyPoint), indexInPrimitive);
        });
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                02/2018
//---------------+---------------+---------------+---------------+---------------+------
void CurveVectorManipulationStrategy::_ReplaceKeyPoint
(
    DPoint3dCR newKeyPoint, 
    size_t index
)
    {
    UpdateKeyPoint(index, [&] (PrimitiveStrategyWithKeyPointIndexRange const& strategyWithRange, size_t indexInPrimitive)
        {
        strategyWithRange.first->ReplaceKeyPoint(AdjustPoint(newKeyPoint), indexInPrimitive);
        });
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                02/2018
//---------------+---------------+---------------+---------------+---------------+------
IGeometryPtr CurveVectorManipulationStrategy::_FinishGeometry() const
    {
    CurveVectorPtr geometry = Finish();
    if (geometry.IsNull())
        return nullptr;

    return IGeometry::Create(geometry);
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                07/2018
//---------------+---------------+---------------+---------------+---------------+------
LineStringManipulationStrategyPtr CurveVectorManipulationStrategy::_InitLineStringManipulationStrategy
(
    ICurvePrimitiveCR primitive
) const
    {
    BeAssert(ICurvePrimitive::CURVE_PRIMITIVE_TYPE_LineString == primitive.GetCurvePrimitiveType());
    return LineStringManipulationStrategy::Create(primitive);
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                07/2018
//---------------+---------------+---------------+---------------+---------------+------
ChildCurveVectorManipulationStrategyPtr CurveVectorManipulationStrategy::_InitChildCurveVectorManipulationStrategy
(
    ICurvePrimitiveCR primitive
) const
    {
    BeAssert(ICurvePrimitive::CURVE_PRIMITIVE_TYPE_CurveVector == primitive.GetCurvePrimitiveType());
    return ChildCurveVectorManipulationStrategy::Create(*primitive.GetChildCurveVectorCP());
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                07/2018
//---------------+---------------+---------------+---------------+---------------+------
ArcManipulationStrategyPtr CurveVectorManipulationStrategy::_InitArcManipulationStrategy
(
    ICurvePrimitiveCR primitive
) const
    {
    BeAssert(ICurvePrimitive::CURVE_PRIMITIVE_TYPE_Arc == primitive.GetCurvePrimitiveType());
    return ArcManipulationStrategy::Create(primitive);
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                03/2018
//---------------+---------------+---------------+---------------+---------------+------
void CurveVectorManipulationStrategy::Init
(
    CurveVectorCR cv
)
    {
    Clear();

    for (ICurvePrimitivePtr const& primitive : cv)
        {
        if (primitive.IsNull())
            continue;

        switch (primitive->GetCurvePrimitiveType())
            {
            case ICurvePrimitive::CURVE_PRIMITIVE_TYPE_LineString:
                {
                LineStringManipulationStrategyPtr primitiveStrategy = _InitLineStringManipulationStrategy(*primitive);
                m_primitiveStrategyContainer.Append(*primitiveStrategy);
                break;
                }
            case ICurvePrimitive::CURVE_PRIMITIVE_TYPE_CurveVector:
                {
                ChildCurveVectorManipulationStrategyPtr primitiveStrategy = _InitChildCurveVectorManipulationStrategy(*primitive);
                m_primitiveStrategyContainer.Append(*primitiveStrategy);
                break;
                }
            case ICurvePrimitive::CURVE_PRIMITIVE_TYPE_Arc:
                {
                ArcManipulationStrategyPtr primitiveStrategy = _InitArcManipulationStrategy(*primitive);
                m_primitiveStrategyContainer.Append(*primitiveStrategy);
                break;
                }
            default:
                BeAssert(false && "Not implemented");
                break;
            }
        }
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                04/2018
//---------------+---------------+---------------+---------------+---------------+------
void CurveVectorManipulationStrategy::_SetDynamicState
(
    DynamicStateBaseCR state
)
    {
    BooleanDynamicStateCP booleanState = dynamic_cast<BooleanDynamicStateCP>(&state);
    if (nullptr != booleanState)
        {
        for (CurvePrimitiveManipulationStrategyPtr const& strategy : m_primitiveStrategyContainer.GetManipulationStrategies())
            {
            strategy->SetDynamicState(state);
            }

        return;
        }

    CurveVectorDynamicStateCP cvState = dynamic_cast<CurveVectorDynamicStateCP>(&state);
    if (nullptr == cvState)
        return;

    for (CurveVectorDynamicState::StatePair const& innerState : cvState->GetState())
        {
        if (innerState.first.IsNull() || innerState.second.IsNull())
            continue;

        innerState.first->SetDynamicState(*innerState.second);
        }
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                04/2018
//---------------+---------------+---------------+---------------+---------------+------
DynamicStateBaseCPtr CurveVectorManipulationStrategy::_GetDynamicState() const
    {
    bvector<CurveVectorDynamicState::StatePair> state;
    for (CurvePrimitiveManipulationStrategyPtr const& strategy : m_primitiveStrategyContainer.GetManipulationStrategies())
        {
        DynamicStateBaseCPtr innerState = strategy->GetDynamicState();
        if (innerState.IsNull())
            return nullptr;

        state.push_back({strategy, innerState});
        }

    return DynamicStateBaseCPtr(CurveVectorDynamicState::Create(state));
    }

#define GMS_PROPERTY_OVERRIDE_IMPL(value_type) \
    void CurveVectorManipulationStrategy::_SetProperty(Utf8CP key, value_type const& value) \
        { \
        GetStrategyForSetProperty().SetProperty(key, value); \
        } \
    BentleyStatus CurveVectorManipulationStrategy::_TryGetProperty(Utf8CP key, value_type& value) const \
        { \
        if(m_primitiveStrategyContainer.IsEmpty()) \
            return BentleyStatus::ERROR; \
        return m_primitiveStrategyContainer.GetPlacementStrategies().back()->TryGetProperty(key, value); \
        }

GMS_PROPERTY_OVERRIDE_IMPL(bool)
GMS_PROPERTY_OVERRIDE_IMPL(int)
GMS_PROPERTY_OVERRIDE_IMPL(double)
GMS_PROPERTY_OVERRIDE_IMPL(DVec3d)
GMS_PROPERTY_OVERRIDE_IMPL(RotMatrix)
GMS_PROPERTY_OVERRIDE_IMPL(Utf8String)
GMS_PROPERTY_OVERRIDE_IMPL(bvector<double>)
GMS_PROPERTY_OVERRIDE_IMPL(bvector<Utf8String>)
GMS_PROPERTY_OVERRIDE_IMPL(GeometryManipulationStrategyProperty)