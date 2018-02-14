/*--------------------------------------------------------------------------------------+
|
|     $Source: GeometryManipulationStrategies/CurveVectorManipulationStrategy.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "PublicApi/GeometryManipulationStrategiesApi.h"

#define DEFAULT_MANIPULATION_STRATEGY LineManipulationStrategy::Create()

USING_NAMESPACE_BUILDING_SHARED

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                01/2018
//---------------+---------------+---------------+---------------+---------------+------
CurveVectorManipulationStrategyPtr CurveVectorManipulationStrategy::Create
(
    CurveVectorCR cv
)
    {
    CurveVectorManipulationStrategyPtr strategy = Create();

    for (ICurvePrimitivePtr const& primitive : cv)
        {
        if (primitive.IsNull())
            continue;

        switch (primitive->GetCurvePrimitiveType())
            {
            case ICurvePrimitive::CURVE_PRIMITIVE_TYPE_LineString:
                {
                LineStringManipulationStrategyPtr primitiveStrategy = LineStringManipulationStrategy::Create(*primitive);
                strategy->m_primitiveStrategies.push_back(primitiveStrategy);
                break;
                }
            default:
                BeAssert(false && "Not implemented");
                break;
            }
        }

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
    , m_defaultNewGeometryType(DefaultNewGeometryType::Line)
    , m_defaultLinePlacementStrategyType(LinePlacementStrategyType::Points)
    , m_defaultArcPlacementStrategyType(ArcPlacementStrategyType::StartMidEnd)
    , m_defaultLineStringPlacementStrategyType(LineStringPlacementStrategyType::Points)
    , m_workingPlane(DPlane3d::FromOriginAndNormal({0,0,0}, DVec3d::From(0, 0, 1)))
    {}

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

    CurveLocationDetail start, end;
    if (!cv.GetStartEnd(start, end))
        return;
    if (!start.point.AlmostEqual(end.point))
        {
        ICurvePrimitivePtr lastPrimitive = cv.back();
        if (lastPrimitive.IsNull())
            return;

        if (nullptr != lastPrimitive->GetLineStringP())
            lastPrimitive->GetLineStringP()->push_back(start.point);
        else
            {
            ICurvePrimitivePtr endStart = ICurvePrimitive::CreateLine(end.point, start.point);
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
    
    for (CurvePrimitiveManipulationStrategyPtr const& strategy : m_primitiveStrategies)
        {
        ICurvePrimitivePtr primitive = strategy->FinishPrimitive();
        if (primitive.IsNull())
            continue;

        cv->Add(primitive);
        }

    if (cv->empty())
        return nullptr;

    if (connectEndStart)
        ConnectStartEnd(*cv);

    return cv;
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
    return !m_primitiveStrategies.empty() &&
        std::none_of(m_primitiveStrategies.begin(), m_primitiveStrategies.end(),
                    [] (CurvePrimitiveManipulationStrategyPtr const& s) { return !s->IsComplete(); });
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                01/2018
//---------------+---------------+---------------+---------------+---------------+------
bvector<DPoint3d> CurveVectorManipulationStrategy::_GetKeyPoints() const
    {
    bvector<DPoint3d> allKeyPoints;

    for (CurvePrimitiveManipulationStrategyPtr const& strategy : m_primitiveStrategies)
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
    return std::any_of(m_primitiveStrategies.rbegin(), m_primitiveStrategies.rend(),
                       [] (CurvePrimitiveManipulationStrategyPtr const& s) { return s->IsDynamicKeyPointSet(); });
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                01/2018
//---------------+---------------+---------------+---------------+---------------+------
void CurveVectorManipulationStrategy::_ResetDynamicKeyPoint()
    {
    for (CurvePrimitiveManipulationStrategyPtr const& strategy : m_primitiveStrategies)
        {
        strategy->ResetDynamicKeyPoint();
        }

    std::remove_if(m_primitiveStrategies.begin(), m_primitiveStrategies.end(),
                   [] (CurvePrimitiveManipulationStrategyPtr const& strategy) { return strategy->GetKeyPoints().empty(); });
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                01/2018
//---------------+---------------+---------------+---------------+---------------+------
bool CurveVectorManipulationStrategy::IsLastStrategyReadyForPop() const
    {
    if (m_primitiveStrategies.empty())
        return false;

    if (m_primitiveStrategies.size() == 1 && m_primitiveStrategies.back()->IsEmpty())
        return true;

    if (m_primitiveStrategies.size() > 1)
        {
        CurvePrimitiveManipulationStrategyR lastStrategy = *m_primitiveStrategies.back();
        if (lastStrategy.IsEmpty())
            return true;

        CurvePrimitiveManipulationStrategyR secondToLastStrategy = *m_primitiveStrategies[m_primitiveStrategies.size() - 2];
        if (lastStrategy.IsSingleKeyPointLeft() &&
            lastStrategy.GetFirstKeyPoint().AlmostEqual(secondToLastStrategy.GetLastKeyPoint()))
            return true;
        }

    return false;
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                01/2018
//---------------+---------------+---------------+---------------+---------------+------
void CurveVectorManipulationStrategy::_PopKeyPoint()
    {
    if (m_primitiveStrategies.empty())
        return;

    m_primitiveStrategies.back()->CreateDefaultPlacementStrategy()->PopKeyPoint();

    if (IsLastStrategyReadyForPop())
        {
        m_primitiveStrategies.pop_back();
        }
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Haroldas Vitunskas              01/2018
//---------------+---------------+---------------+---------------+---------------+------
CurvePrimitivePlacementStrategyPtr CurveVectorManipulationStrategy::ResetCurrentManipulationStrategy()
    {
    CurvePrimitiveManipulationStrategyPtr manipulationStrategy;

    switch (m_defaultNewGeometryType)
        {
        case DefaultNewGeometryType::Arc:
            manipulationStrategy = ArcManipulationStrategy::Create();
            break;
        case DefaultNewGeometryType::Line:
            manipulationStrategy = LineManipulationStrategy::Create();
            break;
        case DefaultNewGeometryType::Spline:
            manipulationStrategy = SplineControlPointsManipulationStrategy::Create(SplineControlPointsManipulationStrategy::default_Order);
            break;
        case DefaultNewGeometryType::InterpolationCurve:
            manipulationStrategy = SplineThroughPointsManipulationStrategy::Create();
            break;
        case DefaultNewGeometryType::LineString:
            manipulationStrategy = LineStringManipulationStrategy::Create();   
            break; 
        default:
            if (m_primitiveStrategies.empty())
                manipulationStrategy = DEFAULT_MANIPULATION_STRATEGY;
            else
                manipulationStrategy = m_primitiveStrategies.back()->Clone();
            break;
        }

    BeAssert(manipulationStrategy.IsValid());

    CurvePrimitivePlacementStrategyPtr placementStrategy = GetPlacementStrategy(*manipulationStrategy);
    if (!m_primitiveStrategies.empty())
        {
        bvector<DPoint3d> lastStrategyAcceptedKeyPoints = m_primitiveStrategies.back()->GetAcceptedKeyPoints();
        if (!lastStrategyAcceptedKeyPoints.empty())
            placementStrategy->AddKeyPoint(lastStrategyAcceptedKeyPoints.back());
        }
    m_primitiveStrategies.push_back(manipulationStrategy);
    
    return placementStrategy;
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                01/2018
//---------------+---------------+---------------+---------------+---------------+------
CurvePrimitivePlacementStrategyPtr CurveVectorManipulationStrategy::GetPlacementStrategy
(
    CurvePrimitiveManipulationStrategyR manipulationStrategy
) const
    {
    switch (m_defaultNewGeometryType)
        {
        case DefaultNewGeometryType::Arc:
            return manipulationStrategy.CreateArcPlacementStrategy(m_defaultArcPlacementStrategyType);
        case DefaultNewGeometryType::Line:
            return manipulationStrategy.CreateLinePlacementStrategy(m_defaultLinePlacementStrategyType);
        case DefaultNewGeometryType::LineString:
            return manipulationStrategy.CreateLineStringPlacementStrategy(m_defaultLineStringPlacementStrategyType);
        default:
            return manipulationStrategy.CreateDefaultPlacementStrategy();
        }
    }

    
//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                01/2018
//---------------+---------------+---------------+---------------+---------------+------
CurvePrimitivePlacementStrategyPtr CurveVectorManipulationStrategy::GetStrategyForAppend()
    {
    if (!m_primitiveStrategies.empty() && m_primitiveStrategies.back()->CanAcceptMorePoints())
        return GetPlacementStrategy(*m_primitiveStrategies.back());
    else
        return ResetCurrentManipulationStrategy();
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                01/2018
//---------------+---------------+---------------+---------------+---------------+------
void CurveVectorManipulationStrategy::_AppendKeyPoint
(
    DPoint3dCR newKeyPoint
)
    {
    GetStrategyForAppend()->AddKeyPoint(AdjustKeyPoint(newKeyPoint));
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                01/2018
//---------------+---------------+---------------+---------------+---------------+------
void CurveVectorManipulationStrategy::_AppendDynamicKeyPoint
(
    DPoint3dCR newDynamicKeyPoint
)
    {
    GetStrategyForAppend()->AddDynamicKeyPoint(AdjustKeyPoint(newDynamicKeyPoint));
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                01/2018
//---------------+---------------+---------------+---------------+---------------+------
void CurveVectorManipulationStrategy::_Clear()
    {
    m_primitiveStrategies.clear();
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                01/2018
//---------------+---------------+---------------+---------------+---------------+------
void CurveVectorManipulationStrategy::ChangeDefaultNewGeometryType
(
    DefaultNewGeometryType newGeometryType
)
    {
    if (m_defaultNewGeometryType == newGeometryType)
        return;

    if (!m_primitiveStrategies.empty() && !m_primitiveStrategies.back()->IsComplete())
        {
        bvector<DPoint3d> const& lastPrimitiveKeyPoints = m_primitiveStrategies.back()->GetKeyPoints();
        CurvePrimitiveManipulationStrategyPtr newPrimitiveStrategy;
        switch (newGeometryType)
            {
            case DefaultNewGeometryType::Line:
            {
                newPrimitiveStrategy = LineManipulationStrategy::Create();
                CurvePrimitivePlacementStrategyPtr tmpPlacementStrategy = newPrimitiveStrategy->CreateDefaultPlacementStrategy();
                if (!lastPrimitiveKeyPoints.empty())
                    tmpPlacementStrategy->AddKeyPoint(lastPrimitiveKeyPoints.front());
                break;
            }
            case DefaultNewGeometryType::Arc:
            {
                newPrimitiveStrategy = ArcManipulationStrategy::Create();
                CurvePrimitivePlacementStrategyPtr tmpPlacementStrategy = newPrimitiveStrategy->CreateDefaultPlacementStrategy();
                for (int i = 0; i < 2; ++i)
                    {
                    if (lastPrimitiveKeyPoints.size() - 1 < i)
                        break;
                    tmpPlacementStrategy->AddKeyPoint(lastPrimitiveKeyPoints[i]);
                    }
                break;
            }
            case DefaultNewGeometryType::Spline:
                {
                newPrimitiveStrategy = SplineControlPointsManipulationStrategy::Create(SplineControlPointsManipulationStrategy::default_Order);
                CurvePrimitivePlacementStrategyPtr tmpPlacementStrategy = newPrimitiveStrategy->CreateDefaultPlacementStrategy();
                for (DPoint3d point : lastPrimitiveKeyPoints)
                    if (!point.AlmostEqual(DPoint3d::From(std::numeric_limits<double>::max(), std::numeric_limits<double>::max(), std::numeric_limits<double>::max())))
                        tmpPlacementStrategy->AddKeyPoint(point);
                }
                break;
            case DefaultNewGeometryType::InterpolationCurve:
                {
                newPrimitiveStrategy = SplineThroughPointsManipulationStrategy::Create();
                CurvePrimitivePlacementStrategyPtr tmpPlacementStrategy = newPrimitiveStrategy->CreateDefaultPlacementStrategy();
                for (DPoint3d point : lastPrimitiveKeyPoints)
                    if (!point.AlmostEqual(DPoint3d::From(std::numeric_limits<double>::max(), std::numeric_limits<double>::max(), std::numeric_limits<double>::max())))
                        tmpPlacementStrategy->AddKeyPoint(point);
                }
                break;
            case DefaultNewGeometryType::LineString:
            {
                newPrimitiveStrategy = LineStringManipulationStrategy::Create();
	    }
	    break;
            default:
                return;
            }

        m_primitiveStrategies[m_primitiveStrategies.size() - 1] = newPrimitiveStrategy;
        }

    m_defaultNewGeometryType = newGeometryType;
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Haroldas Vitunskas              01/2018
//---------------+---------------+---------------+---------------+---------------+------
bool CurveVectorManipulationStrategy::FinishContiniousPrimitive()
    {
    if (!m_primitiveStrategies.empty() && m_primitiveStrategies.back()->IsContinious())
        ResetCurrentManipulationStrategy();
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
    if (m_defaultLinePlacementStrategyType == newPlacementStrategyType)
        return;

    m_defaultLinePlacementStrategyType = newPlacementStrategyType;
    }

    
//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                01/2018
//---------------+---------------+---------------+---------------+---------------+------
void CurveVectorManipulationStrategy::ChangeDefaultPlacementStrategy
(
    ArcPlacementStrategyType newPlacementStrategyType
)
    {
    if (m_defaultArcPlacementStrategyType == newPlacementStrategyType)
        return;

    m_defaultArcPlacementStrategyType = newPlacementStrategyType;
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                01/2018
//---------------+---------------+---------------+---------------+---------------+------
void CurveVectorManipulationStrategy::ChangeDefaultPlacementStrategy
(
    LineStringPlacementStrategyType newPlacementStrategyType
)
    {
    if (m_defaultLineStringPlacementStrategyType == newPlacementStrategyType)
        return;

    m_defaultLineStringPlacementStrategyType = newPlacementStrategyType;
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                01/2018
//---------------+---------------+---------------+---------------+---------------+------
DPoint3d CurveVectorManipulationStrategy::AdjustKeyPoint
(
    DPoint3dCR keyPoint
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

    GetStrategyForAppend()->SetProperty(key, value);
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

    if (m_primitiveStrategies.empty())
        return BentleyStatus::ERROR;
    BentleyStatus status = m_primitiveStrategies.back()->TryGetProperty(key, value);

    if (BentleyStatus::SUCCESS != status)
        return T_Super::_TryGetProperty(key, value);

    return status;
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                02/2018
//---------------+---------------+---------------+---------------+---------------+------
bvector<CurveVectorManipulationStrategy::PrimitiveStrategyWithKeyPointIndexRange> CurveVectorManipulationStrategy::GetPrimitiveStrategies
(
    size_t index
) const
    {
    bvector<PrimitiveStrategyWithKeyPointIndexRange> primitiveStrategies;

    size_t currentPrimitiveBeginIndex = 0;
    DPoint3d lastPrimitiveKeyPoint;
    for (CurvePrimitiveManipulationStrategyPtr const& primitiveStrategy : m_primitiveStrategies)
        {
        bvector<DPoint3d> const& keyPoints = primitiveStrategy->GetKeyPoints();
        if (keyPoints.empty())
            continue;

        if (currentPrimitiveBeginIndex > 0 && !lastPrimitiveKeyPoint.AlmostEqual(keyPoints.front()))
            ++currentPrimitiveBeginIndex;

        size_t currentPrimitiveEndIndex = currentPrimitiveBeginIndex + keyPoints.size() - 1;
        
        if (index >= currentPrimitiveBeginIndex && index <= currentPrimitiveEndIndex)
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
    bvector<PrimitiveStrategyWithKeyPointIndexRange> primitiveStrategies = GetPrimitiveStrategies(index);
    for (PrimitiveStrategyWithKeyPointIndexRange const& strategyWithRange : primitiveStrategies)
        {
        PrimitiveStrategyKeyPointIndexRange indexRange = strategyWithRange.second;
        if (!(index >= indexRange.first && index <= indexRange.second))
            {
            BeAssert(false && "Bad KeyPoint index range");
            _ResetDynamicKeyPoint();
            return;
            }

        updateFn(strategyWithRange, index - strategyWithRange.second.first);
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
        strategyWithRange.first->UpdateDynamicKeyPoint(newDynamicKeyPoint, indexInPrimitive);
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
        strategyWithRange.first->ReplaceKeyPoint(newKeyPoint, indexInPrimitive);
        });
    }

#define GMS_PROPERTY_OVERRIDE_IMPL(value_type) \
    void CurveVectorManipulationStrategy::_SetProperty(Utf8CP key, value_type const& value) \
        { \
        GetStrategyForAppend()->SetProperty(key, value); \
        } \
    BentleyStatus CurveVectorManipulationStrategy::_TryGetProperty(Utf8CP key, value_type& value) const \
        { \
        if(m_primitiveStrategies.empty()) \
            return BentleyStatus::ERROR; \
        return m_primitiveStrategies.back()->TryGetProperty(key, value); \
        }

GMS_PROPERTY_OVERRIDE_IMPL(bool)
GMS_PROPERTY_OVERRIDE_IMPL(int)
GMS_PROPERTY_OVERRIDE_IMPL(double)
GMS_PROPERTY_OVERRIDE_IMPL(DVec3d)
GMS_PROPERTY_OVERRIDE_IMPL(Dgn::DgnElementId)
GMS_PROPERTY_OVERRIDE_IMPL(Dgn::DgnElement)
GMS_PROPERTY_OVERRIDE_IMPL(Utf8String)
GMS_PROPERTY_OVERRIDE_IMPL(bvector<double>)
GMS_PROPERTY_OVERRIDE_IMPL(bvector<Utf8String>)
GMS_PROPERTY_OVERRIDE_IMPL(GeometryManipulationStrategyProperty)