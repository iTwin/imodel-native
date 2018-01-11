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
CurveVectorManipulationStrategy::CurveVectorManipulationStrategy()
    : T_Super()
    , m_defaultNewGeometryType(DefaultNewGeometryType::Line)
    , m_defaultLinePlacementStrategyType(LinePlacementStrategyType::Points)
    , m_defaultArcPlacementStrategyType(ArcPlacementStrategyType::StartMidEnd)
    , m_defaultLineStringPlacementStrategyType(LineStringPlacementStrategyType::Points)
    {}

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                12/2017
//---------------+---------------+---------------+---------------+---------------+------
CurveVectorPtr CurveVectorManipulationStrategy::_Finish() const
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

    return cv;
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                12/2017
//---------------+---------------+---------------+---------------+---------------+------
CurveVectorPtr CurveVectorManipulationStrategy::Finish() const
    {
    return _Finish();
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
        return m_primitiveStrategies.back()->CreateDefaultPlacementStrategy();
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
    GetStrategyForAppend()->AddKeyPoint(newKeyPoint);
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                01/2018
//---------------+---------------+---------------+---------------+---------------+------
void CurveVectorManipulationStrategy::_AppendDynamicKeyPoint
(
    DPoint3dCR newDynamicKeyPoint
)
    {
    GetStrategyForAppend()->AddDynamicKeyPoint(newDynamicKeyPoint);
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
(
    LinePlacementStrategyType newPlacementStrategyType
)
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

GMS_PROPERTY_OVERRIDE_IMPL(int)
GMS_PROPERTY_OVERRIDE_IMPL(double)
GMS_PROPERTY_OVERRIDE_IMPL(DVec3d)
GMS_PROPERTY_OVERRIDE_IMPL(DPlane3d)
GMS_PROPERTY_OVERRIDE_IMPL(Dgn::DgnElementId)
GMS_PROPERTY_OVERRIDE_IMPL(Dgn::DgnElement)
GMS_PROPERTY_OVERRIDE_IMPL(Utf8String)
GMS_PROPERTY_OVERRIDE_IMPL(bvector<double>)
GMS_PROPERTY_OVERRIDE_IMPL(bvector<Utf8String>)
GMS_PROPERTY_OVERRIDE_IMPL(GeometryManipulationStrategyProperty)