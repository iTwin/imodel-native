/*--------------------------------------------------------------------------------------+
|
|     $Source: GeometryManipulationStrategies/ExtrusionManipulationStrategy.cpp $
|
|  $Copyright: (c) 2019 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "PublicApi/GeometryManipulationStrategiesApi.h"

BUILDING_SHARED_REFCOUNTED_PTR_AND_TYPEDEFS(ExtrusionDynamicState)
BEGIN_BUILDING_SHARED_NAMESPACE

//=======================================================================================
// @bsiclass                                     Mindaugas.Butkus               04/2018
//=======================================================================================
struct ExtrusionDynamicState : DynamicStateBase
    {
    private:
        DynamicStateBaseCPtr m_baseState;
        bool m_heightState;
        bool m_sweepState;

        ExtrusionDynamicState(DynamicStateBaseCR baseState, bool heightState, bool sweepState)
            : m_baseState(baseState.Clone())
            , m_heightState(heightState)
            , m_sweepState(sweepState)
            {}

    protected:
        DynamicStateBasePtr _Clone() const override
            {
            return Create(*m_baseState, m_heightState, m_sweepState);
            }

    public:
        static ExtrusionDynamicStatePtr Create(DynamicStateBaseCR baseState, bool heightState, bool sweepState)
            {
            return new ExtrusionDynamicState(baseState, heightState, sweepState);
            }

        DynamicStateBaseCPtr GetBaseState() const { return m_baseState; }
        bool GetHeightState() const { return m_heightState; }
        bool GetSweepState() const { return m_sweepState; }
    };

END_BUILDING_SHARED_NAMESPACE

USING_NAMESPACE_BUILDING_SHARED

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                02/2019
//---------------+---------------+---------------+---------------+---------------+------
ExtrusionManipulationStrategy::ExtrusionManipulationStrategy() 
    : ExtrusionManipulationStrategy(*CurveVectorManipulationStrategy::Create()) 
    {}

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                02/2019
//---------------+---------------+---------------+---------------+---------------+------
ExtrusionManipulationStrategy::ExtrusionManipulationStrategy
(
    CurveVectorManipulationStrategyR baseShapeManipulationStrategy
)
    : T_Super()
    , m_baseComplete(false)
    , m_heightSet(false)
    , m_dynamicHeightSet(false)
    , m_useFixedHeight(false)
    , m_sweepDirectionSet(false)
    , m_dynamicSweepDirectionSet(false)
    , m_useFixedSweepDirection(false)
    , m_height(0)
    , m_dynamicHeight(0)
    , m_fixedHeight(0)
    , m_sweepDirection(DVec3d::From(0, 0, 0))
    , m_dynamicSweepDirection(DVec3d::From(0, 0, 0))
    , m_fixedSweepDirection(DVec3d::From(0, 0, 0))
    , m_baseShapeManipulationStrategy(&baseShapeManipulationStrategy)
    {
    RegisterBoolProperty(prop_BaseComplete());
    RegisterBoolProperty(prop_IsHeightSet());
    RegisterBoolProperty(prop_IsDynamicHeightSet());
    RegisterBoolProperty(prop_UseFixedHeight());
    RegisterBoolProperty(prop_IsSweepDirectionSet());
    RegisterBoolProperty(prop_IsDynamicSweepDirectionSet());
    RegisterBoolProperty(prop_UseFixedSweepDirection());
    RegisterDoubleProperty(prop_Height());
    RegisterDoubleProperty(prop_FixedHeight());
    RegisterDVec3dProperty(prop_SweepDirection());
    RegisterDVec3dProperty(prop_FixedSweepDirection());
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                01/2018
//---------------+---------------+---------------+---------------+---------------+------
bool ExtrusionManipulationStrategy::_IsBaseComplete() const
    {
    return m_baseComplete && T_Super::_IsBaseComplete();
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                01/2018
//---------------+---------------+---------------+---------------+---------------+------
void ExtrusionManipulationStrategy::_SetBaseComplete
(
    bool value
)
    {
    if (m_baseComplete)
        {
        m_baseComplete = value;
        return;
        }

    if (T_Super::_IsBaseComplete())
        {
        m_baseComplete = value;
        }
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                01/2018
//---------------+---------------+---------------+---------------+---------------+------
double ExtrusionManipulationStrategy::CalculateHeight
(
    DPoint3dCR keyPoint
) const
    {
    DPoint3d projection;
    m_baseShapeManipulationStrategy->GetWorkingPlane().ProjectPoint(projection, keyPoint);
    return projection.Distance(keyPoint);
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                05/2018
//---------------+---------------+---------------+---------------+---------------+------
DPlane3d ExtrusionManipulationStrategy::GetBasePlane() const
    {
    return m_baseShapeManipulationStrategy->GetWorkingPlane();
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                05/2018
//---------------+---------------+---------------+---------------+---------------+------
DPlane3d ExtrusionManipulationStrategy::GetTopPlane() const
    {
    DPlane3d basePlane = GetBasePlane();

    DVec3d planeTranslation = basePlane.normal;
    planeTranslation.ScaleToLength(GetHeight());

    DPoint3d topPlaneOrigin = basePlane.origin;
    topPlaneOrigin.Add(planeTranslation);

    DPlane3d topPlane = DPlane3d::FromOriginAndNormal(topPlaneOrigin, basePlane.normal);
    return topPlane;
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                05/2018
//---------------+---------------+---------------+---------------+---------------+------
DVec3d ExtrusionManipulationStrategy::GetScaledSweepDirection
(
    DPlane3d const& base, 
    DPlane3d const& top,
    DVec3d sweepDirection
) const
    {
    DPoint3d baseOriginTopProjection;
    top.ProjectPoint(baseOriginTopProjection, base.origin);

    DVec3d originToOrigin = DVec3d::FromStartEnd(base.origin, baseOriginTopProjection);
    if (originToOrigin.IsZero() || sweepDirection.IsZero())
        return DVec3d::From(0, 0, 0);

    double fraction;
    sweepDirection.ProjectToVector(originToOrigin, fraction);
    sweepDirection.Scale(1 / fraction);
    return sweepDirection;
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                01/2018
//---------------+---------------+---------------+---------------+---------------+------
DVec3d ExtrusionManipulationStrategy::CalculateSweepDirection
(
    DPoint3dCR keyPoint
) const
    {
    DPlane3d basePlane = GetBasePlane();
    DPlane3d topPlane = GetTopPlane();

    DPoint3d sweepDirectionEndPoint;
    topPlane.ProjectPoint(sweepDirectionEndPoint, keyPoint);

    DPoint3d lastBaseKeyPoint = m_baseShapeManipulationStrategy->GetKeyPoints().back();
    return DVec3d::FromStartEnd(lastBaseKeyPoint, sweepDirectionEndPoint);
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                01/2018
//---------------+---------------+---------------+---------------+---------------+------
void ExtrusionManipulationStrategy::_AppendDynamicKeyPoint
(
    DPoint3dCR newDynamicKeyPoint
)
    {
    if (!m_baseComplete)
        {
        T_Super::_AppendDynamicKeyPoint(newDynamicKeyPoint);
        return;
        }

    if (!m_heightSet && !m_dynamicHeightSet)
        {
        m_dynamicHeight = CalculateHeight(newDynamicKeyPoint);
        m_dynamicHeightSet = true;
        return;
        }

    if (!m_sweepDirectionSet && !m_dynamicSweepDirectionSet)
        {
        m_dynamicSweepDirection = CalculateSweepDirection(newDynamicKeyPoint);
        m_dynamicSweepDirectionSet = true;
        return;
        }
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                04/2018
//---------------+---------------+---------------+---------------+---------------+------
void ExtrusionManipulationStrategy::_AppendDynamicKeyPoints
(
    bvector<DPoint3d> const& newDynamicKeyPoints
)
    {
    for (DPoint3d const& newDynamicKeyPoint : newDynamicKeyPoints)
        _AppendDynamicKeyPoint(newDynamicKeyPoint);
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                01/2018
//---------------+---------------+---------------+---------------+---------------+------
void ExtrusionManipulationStrategy::_AppendKeyPoint
(
    DPoint3dCR newKeyPoint
)
    {
    if (!m_baseComplete)
        {
        T_Super::_AppendKeyPoint(newKeyPoint);
        return;
        }

    if (!IsHeightSet())
        {
        m_height = CalculateHeight(newKeyPoint);
        m_heightSet = true;
        return;
        }

    if (!IsSweepDirectionSet())
        {
        m_sweepDirection = CalculateSweepDirection(newKeyPoint);
        m_sweepDirectionSet = true;
        return;
        }
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                01/2018
//---------------+---------------+---------------+---------------+---------------+------
void ExtrusionManipulationStrategy::_PopKeyPoint()
    {
    if (m_sweepDirectionSet)
        {
        m_sweepDirectionSet = false;
        return;
        }

    if (m_heightSet)
        {
        m_heightSet = false;
        return;
        }

    m_baseComplete = false;
    T_Super::_PopKeyPoint();
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                01/2018
//---------------+---------------+---------------+---------------+---------------+------
bvector<DPoint3d> ExtrusionManipulationStrategy::_GetKeyPoints() const
    {
    bvector<DPoint3d> keyPoints = m_baseShapeManipulationStrategy->GetKeyPoints();

    if (!m_heightSet && !m_dynamicHeightSet)
        return keyPoints;

    DVec3d translation = m_baseShapeManipulationStrategy->GetWorkingPlane().normal;
    translation.ScaleToLength(GetHeight());

    DPoint3d heightKeyPoint = keyPoints.back();
    heightKeyPoint.Add(translation);
    keyPoints.push_back(heightKeyPoint);
     
    if (!m_sweepDirectionSet && !m_dynamicSweepDirectionSet)
        return keyPoints;

    CurveVectorPtr base = m_baseShapeManipulationStrategy->Finish();
    base->SetBoundaryType(CurveVector::BOUNDARY_TYPE_Outer);
    double baseArea;
    DPoint3d baseCentroid;
    DVec3d baseNormal;
    base->CentroidNormalArea(baseCentroid, baseNormal, baseArea);

    DPoint3d sweepDirectionKeyPoint = baseCentroid;
    sweepDirectionKeyPoint.Add(GetSweepDirection());
    keyPoints.push_back(sweepDirectionKeyPoint);

    return keyPoints;
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                01/2018
//---------------+---------------+---------------+---------------+---------------+------
bool ExtrusionManipulationStrategy::_IsDynamicKeyPointSet() const
    {
    return m_baseShapeManipulationStrategy->IsDynamicKeyPointSet() || m_dynamicHeightSet || m_dynamicSweepDirectionSet;
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                01/2018
//---------------+---------------+---------------+---------------+---------------+------
void ExtrusionManipulationStrategy::_ResetDynamicKeyPoint()
    {
    m_baseShapeManipulationStrategy->ResetDynamicKeyPoint();
    m_dynamicHeightSet = false;
    m_dynamicSweepDirectionSet = false;
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                01/2018
//---------------+---------------+---------------+---------------+---------------+------
bool ExtrusionManipulationStrategy::_IsComplete() const
    {
    return m_baseShapeManipulationStrategy->IsComplete() && IsHeightSet() && IsSweepDirectionSet();
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                01/2018
//---------------+---------------+---------------+---------------+---------------+------
bool ExtrusionManipulationStrategy::_CanAcceptMorePoints() const
    {
    return m_baseShapeManipulationStrategy->CanAcceptMorePoints() && !m_baseComplete || !IsHeightSet() || !IsSweepDirectionSet();
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                01/2018
//---------------+---------------+---------------+---------------+---------------+------
double ExtrusionManipulationStrategy::GetHeight() const
    {
    if (m_useFixedHeight)
        return m_fixedHeight;

    if (m_dynamicHeightSet)
        return m_dynamicHeight;

    if (m_heightSet)
        return m_height;

    return 0;
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                01/2018
//---------------+---------------+---------------+---------------+---------------+------
DVec3d ExtrusionManipulationStrategy::GetSweepDirection() const
    {
    if (m_useFixedSweepDirection)
        return GetScaledSweepDirection(GetBasePlane(), GetTopPlane(), m_fixedSweepDirection);

    if (m_dynamicSweepDirectionSet)
        return GetScaledSweepDirection(GetBasePlane(), GetTopPlane(), m_dynamicSweepDirection);

    if (m_sweepDirectionSet)
        return GetScaledSweepDirection(GetBasePlane(), GetTopPlane(), m_sweepDirection);

    return DVec3d::From(0, 0, GetHeight());
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                01/2018
//---------------+---------------+---------------+---------------+---------------+------
ISolidPrimitivePtr ExtrusionManipulationStrategy::_FinishSolidPrimitive() const
    {
    return FinishExtrusion();
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                01/2018
//---------------+---------------+---------------+---------------+---------------+------
void ExtrusionManipulationStrategy::_SetProperty
(
    Utf8CP key, 
    bool const& value
)
    {
    if (0 == strcmp(key, prop_ContinuousBaseShapePrimitiveComplete()))
        {
        if (!_IsBaseComplete())
            {
            m_baseShapeManipulationStrategy->FinishContiniousPrimitive();
            }
        }

    if (0 == strcmp(key, prop_UseFixedHeight()))
        {
        m_useFixedHeight = value;
        }

    if (0 == strcmp(key, prop_UseFixedSweepDirection()))
        {
        m_useFixedSweepDirection = value;
        }

    m_baseShapeManipulationStrategy->SetProperty(key, value);
    T_Super::_SetProperty(key, value);
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                01/2018
//---------------+---------------+---------------+---------------+---------------+------
void ExtrusionManipulationStrategy::_SetProperty
(
    Utf8CP key, 
    GeometryManipulationStrategyProperty const& value
)
    {
    if (0 == strcmp(key, prop_BaseShapeStrategy()))
        {
        BaseShapeStrategyChangeProperty const* bsscp = dynamic_cast<BaseShapeStrategyChangeProperty const*>(&value);
        if (nullptr != bsscp)
            {
            if (bsscp->IsDefaultNewGeometryTypeSet())
                m_baseShapeManipulationStrategy->ChangeDefaultNewGeometryType(bsscp->GetDefaultNewGeometryType());
            if (bsscp->IsDefaultLinePlacementStrategyTypeSet())
                m_baseShapeManipulationStrategy->ChangeDefaultPlacementStrategy(bsscp->GetDefaultLinePlacementStrategyType());
            if (bsscp->IsDefaultArcPlacementStrategyTypeSet())
                m_baseShapeManipulationStrategy->ChangeDefaultPlacementStrategy(bsscp->GetDefaultArcPlacementStrategyType());
            if (bsscp->IsDefaultLineStringPlacementStrategyTypeSet())
                m_baseShapeManipulationStrategy->ChangeDefaultPlacementStrategy(bsscp->GetDefaultLineStringPlacementStrategyType());
            }
        }

    m_baseShapeManipulationStrategy->SetProperty(key, value);
    T_Super::_SetProperty(key, value);
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                05/2018
//---------------+---------------+---------------+---------------+---------------+------
bool ExtrusionManipulationStrategy::IsHeightSet() const
    {
    return m_useFixedHeight || m_heightSet;
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                05/2018
//---------------+---------------+---------------+---------------+---------------+------
bool ExtrusionManipulationStrategy::IsSweepDirectionSet() const
    {
    return m_useFixedSweepDirection || m_sweepDirectionSet;
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                05/2018
//---------------+---------------+---------------+---------------+---------------+------
void ExtrusionManipulationStrategy::SetFixedHeight
(
    double const& newHeight
)
    {
    m_fixedHeight = newHeight;
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                05/2018
//---------------+---------------+---------------+---------------+---------------+------
void ExtrusionManipulationStrategy::SetFixedSweepDirection
(
    DVec3d const& newSweepDirection
)
    {
    m_fixedSweepDirection = newSweepDirection;
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                05/2018
//---------------+---------------+---------------+---------------+---------------+------
void ExtrusionManipulationStrategy::_SetProperty
(
    Utf8CP key, 
    int const& value
)
    {
    if (0 == strcmp(key, prop_FixedHeight()))
        {
        SetFixedHeight(value);
        }

    m_baseShapeManipulationStrategy->SetProperty(key, value);
    T_Super::_SetProperty(key, value);
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                05/2018
//---------------+---------------+---------------+---------------+---------------+------
void ExtrusionManipulationStrategy::_SetProperty
(
    Utf8CP key, 
    double const& value
)
    {
    if (0 == strcmp(key, prop_FixedHeight()))
        {
        SetFixedHeight(value);
        }

    m_baseShapeManipulationStrategy->SetProperty(key, value);
    T_Super::_SetProperty(key, value);
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                05/2018
//---------------+---------------+---------------+---------------+---------------+------
void ExtrusionManipulationStrategy::_SetProperty
(
    Utf8CP key, 
    DVec3d const& value
)
    {
    if (0 == strcmp(key, prop_FixedSweepDirection()))
        {
        SetFixedSweepDirection(value);
        }

    m_baseShapeManipulationStrategy->SetProperty(key, value);
    T_Super::_SetProperty(key, value);
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                01/2018
//---------------+---------------+---------------+---------------+---------------+------
BentleyStatus ExtrusionManipulationStrategy::_TryGetProperty
(
    Utf8CP key, 
    double& value
) const
    {
    if (0 == strcmp(key, prop_Height()))
        {
        if (!m_heightSet && !m_dynamicHeightSet && !m_useFixedHeight)
            return BentleyStatus::ERROR;

        value = GetHeight();
        return BentleyStatus::SUCCESS;
        }

    if (0 == strcmp(key, prop_FixedHeight()))
        {
        if (!m_useFixedHeight)
            return BentleyStatus::ERROR;

        value = m_fixedHeight;
        return BentleyStatus::SUCCESS;
        }

    BentleyStatus baseShapeRetVal = m_baseShapeManipulationStrategy->TryGetProperty(key, value);
    if (BentleyStatus::SUCCESS == baseShapeRetVal)
        return baseShapeRetVal;

    return T_Super::_TryGetProperty(key, value);
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                01/2018
//---------------+---------------+---------------+---------------+---------------+------
BentleyStatus ExtrusionManipulationStrategy::_TryGetProperty
(
    Utf8CP key, 
    bool& value
) const
    {
    if (0 == strcmp(key, prop_IsHeightSet()))
        {
        value = m_heightSet;
        return BentleyStatus::SUCCESS;
        }

    if (0 == strcmp(key, prop_IsSweepDirectionSet()))
        {
        value = m_sweepDirectionSet;
        return BentleyStatus::SUCCESS;
        }

    if (0 == strcmp(key, prop_IsDynamicHeightSet()))
        {
        value = m_dynamicHeightSet;
        return BentleyStatus::SUCCESS;
        }

    if (0 == strcmp(key, prop_IsDynamicSweepDirectionSet()))
        {
        value = m_dynamicSweepDirectionSet;
        return BentleyStatus::SUCCESS;
        }

    if (0 == strcmp(key, prop_UseFixedHeight()))
        {
        value = m_useFixedHeight;
        return BentleyStatus::SUCCESS;
        }

    if (0 == strcmp(key, prop_UseFixedSweepDirection()))
        {
        value = m_useFixedSweepDirection;
        return BentleyStatus::SUCCESS;
        }

    BentleyStatus baseShapeRetVal = m_baseShapeManipulationStrategy->TryGetProperty(key, value);
    if (BentleyStatus::SUCCESS == baseShapeRetVal)
        return baseShapeRetVal;

    return T_Super::_TryGetProperty(key, value);
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                01/2018
//---------------+---------------+---------------+---------------+---------------+------
BentleyStatus ExtrusionManipulationStrategy::_TryGetProperty
(
    Utf8CP key, 
    DVec3d& value
) const
    {
    if (0 == strcmp(key, prop_SweepDirection()))
        {
        if (!m_sweepDirectionSet && !m_dynamicSweepDirectionSet && !m_useFixedSweepDirection)
            return BentleyStatus::ERROR;

        value = GetSweepDirection();
        return BentleyStatus::SUCCESS;
        }

    if (0 == strcmp(key, prop_FixedSweepDirection()))
        {
        if (!m_useFixedSweepDirection)
            return BentleyStatus::ERROR;

        value = m_fixedSweepDirection;
        return BentleyStatus::SUCCESS;
        }

    BentleyStatus baseShapeRetVal = m_baseShapeManipulationStrategy->TryGetProperty(key, value);
    if (BentleyStatus::SUCCESS == baseShapeRetVal)
        return baseShapeRetVal;

    return T_Super::_TryGetProperty(key, value);
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                01/2018
//---------------+---------------+---------------+---------------+---------------+------
ISolidPrimitivePtr ExtrusionManipulationStrategy::FinishExtrusion
(
    bool closedBaseShape, 
    bool capped
) const
    {
    CurveVectorPtr baseShape = m_baseShapeManipulationStrategy->Finish(closedBaseShape);
    if (baseShape.IsNull())
        return nullptr;

    return ISolidPrimitive::CreateDgnExtrusion(DgnExtrusionDetail(baseShape, GetSweepDirection(), capped));
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                04/2018
//---------------+---------------+---------------+---------------+---------------+------
void ExtrusionManipulationStrategy::_SetDynamicState
(
    DynamicStateBaseCR state
)
    {
    BooleanDynamicStateCP booleanState = dynamic_cast<BooleanDynamicStateCP>(&state);
    if (nullptr != booleanState)
        {
        m_baseShapeManipulationStrategy->SetDynamicState(state);
        m_dynamicHeightSet = booleanState->GetState();
        m_dynamicSweepDirectionSet = booleanState->GetState();
        return;
        }

    ExtrusionDynamicStateCP extrusionState = dynamic_cast<ExtrusionDynamicStateCP>(&state);
    if (nullptr == extrusionState)
        {
        BeAssert(nullptr != extrusionState);
        return;
        }

    if (extrusionState->GetBaseState().IsNull())
        {
        BeAssert(extrusionState->GetBaseState().IsValid());
        return;
        }

    m_baseShapeManipulationStrategy->SetDynamicState(*extrusionState->GetBaseState());
    m_dynamicHeightSet = extrusionState->GetHeightState();
    m_dynamicSweepDirectionSet = extrusionState->GetSweepState();
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                04/2018
//---------------+---------------+---------------+---------------+---------------+------
DynamicStateBaseCPtr ExtrusionManipulationStrategy::_GetDynamicState() const
    {
    DynamicStateBaseCPtr baseState = m_baseShapeManipulationStrategy->GetDynamicState();
    if (baseState.IsNull())
        return nullptr;

    return DynamicStateBaseCPtr(ExtrusionDynamicState::Create(*baseState, m_dynamicHeightSet, m_dynamicSweepDirectionSet));
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                07/2018
//---------------+---------------+---------------+---------------+---------------+------
bvector<ConstructionGeometry> ExtrusionManipulationStrategy::_FinishConstructionGeometry() const
    {
    return m_baseShapeManipulationStrategy->FinishConstructionGeometry();
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                08/2018
//---------------+---------------+---------------+---------------+---------------+------
void ExtrusionManipulationStrategy::_SetProperty
(
    Utf8CP key, 
    DPlane3d const& value
)
    {
    m_baseShapeManipulationStrategy->SetProperty(key, value);
    T_Super::_SetProperty(key, value);
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                08/2018
//---------------+---------------+---------------+---------------+---------------+------
void ExtrusionManipulationStrategy::_SetProperty
(
    Utf8CP key, 
    RotMatrix const& value
)
    {
    m_baseShapeManipulationStrategy->SetProperty(key, value);
    T_Super::_SetProperty(key, value);
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                08/2018
//---------------+---------------+---------------+---------------+---------------+------
void ExtrusionManipulationStrategy::_SetProperty
(
    Utf8CP key, 
    Utf8String const& value
)
    {
    m_baseShapeManipulationStrategy->SetProperty(key, value);
    T_Super::_SetProperty(key, value);
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                08/2018
//---------------+---------------+---------------+---------------+---------------+------
void ExtrusionManipulationStrategy::_SetProperty
(
    Utf8CP key, 
    bvector<double> const& value
)
    {
    m_baseShapeManipulationStrategy->SetProperty(key, value);
    T_Super::_SetProperty(key, value);
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                08/2018
//---------------+---------------+---------------+---------------+---------------+------
void ExtrusionManipulationStrategy::_SetProperty
(
    Utf8CP key, 
    bvector<Utf8String> const& value
)
    {
    m_baseShapeManipulationStrategy->SetProperty(key, value);
    T_Super::_SetProperty(key, value);
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                08/2018
//---------------+---------------+---------------+---------------+---------------+------
BentleyStatus ExtrusionManipulationStrategy::_TryGetProperty
(
    Utf8CP key, 
    DPlane3d& value
) const
    {
    BentleyStatus baseShapeRetVal = m_baseShapeManipulationStrategy->TryGetProperty(key, value);
    if (BentleyStatus::SUCCESS == baseShapeRetVal)
        return baseShapeRetVal;

    return T_Super::_TryGetProperty(key, value);
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                08/2018
//---------------+---------------+---------------+---------------+---------------+------
BentleyStatus ExtrusionManipulationStrategy::_TryGetProperty
(
    Utf8CP key,
    int& value
) const
    {
    BentleyStatus baseShapeRetVal = m_baseShapeManipulationStrategy->TryGetProperty(key, value);
    if (BentleyStatus::SUCCESS == baseShapeRetVal)
        return baseShapeRetVal;

    return T_Super::_TryGetProperty(key, value);
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                08/2018
//---------------+---------------+---------------+---------------+---------------+------
BentleyStatus ExtrusionManipulationStrategy::_TryGetProperty
(
    Utf8CP key, 
    RotMatrix& value
) const
    {
    BentleyStatus baseShapeRetVal = m_baseShapeManipulationStrategy->TryGetProperty(key, value);
    if (BentleyStatus::SUCCESS == baseShapeRetVal)
        return baseShapeRetVal;

    return T_Super::_TryGetProperty(key, value);
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                08/2018
//---------------+---------------+---------------+---------------+---------------+------
BentleyStatus ExtrusionManipulationStrategy::_TryGetProperty
(
    Utf8CP key,
    Utf8String& value
) const
    {
    BentleyStatus baseShapeRetVal = m_baseShapeManipulationStrategy->TryGetProperty(key, value);
    if (BentleyStatus::SUCCESS == baseShapeRetVal)
        return baseShapeRetVal;

    return T_Super::_TryGetProperty(key, value);
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                08/2018
//---------------+---------------+---------------+---------------+---------------+------
BentleyStatus ExtrusionManipulationStrategy::_TryGetProperty
(
    Utf8CP key,
    bvector<double>& value
) const
    {
    BentleyStatus baseShapeRetVal = m_baseShapeManipulationStrategy->TryGetProperty(key, value);
    if (BentleyStatus::SUCCESS == baseShapeRetVal)
        return baseShapeRetVal;

    return T_Super::_TryGetProperty(key, value);
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                08/2018
//---------------+---------------+---------------+---------------+---------------+------
BentleyStatus ExtrusionManipulationStrategy::_TryGetProperty
(
    Utf8CP key,
    bvector<Utf8String>& value
) const
    {
    BentleyStatus baseShapeRetVal = m_baseShapeManipulationStrategy->TryGetProperty(key, value);
    if (BentleyStatus::SUCCESS == baseShapeRetVal)
        return baseShapeRetVal;

    return T_Super::_TryGetProperty(key, value);
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                08/2018
//---------------+---------------+---------------+---------------+---------------+------
BentleyStatus ExtrusionManipulationStrategy::_TryGetProperty
(
    Utf8CP key,
    GeometryManipulationStrategyProperty& value
) const
    {
    BentleyStatus baseShapeRetVal = m_baseShapeManipulationStrategy->TryGetProperty(key, value);
    if (BentleyStatus::SUCCESS == baseShapeRetVal)
        return baseShapeRetVal;

    return T_Super::_TryGetProperty(key, value);
    }