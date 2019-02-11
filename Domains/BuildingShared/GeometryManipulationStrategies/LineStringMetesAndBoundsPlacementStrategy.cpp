/*--------------------------------------------------------------------------------------+
|
|     $Source: GeometryManipulationStrategies/LineStringMetesAndBoundsPlacementStrategy.cpp $
|
|  $Copyright: (c) 2019 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "PublicApi/GeometryManipulationStrategiesApi.h"

USING_NAMESPACE_BUILDING_SHARED

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                01/2018
//---------------+---------------+---------------+---------------+---------------+------
LineStringMetesAndBoundsPlacementStrategy::LineStringMetesAndBoundsPlacementStrategy()
    : T_Super()
    , m_workingPlane(DPlane3d::FromOriginAndNormal({0,0,0},DVec3d::From(0,0,1)))
    {
    RegisterDPlane3dProperty(prop_WorkingPlane());
    RegisterCustomProperty(prop_MetesAndBounds());
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                01/2018
//---------------+---------------+---------------+---------------+---------------+------
void LineStringMetesAndBoundsPlacementStrategy::_SetProperty
(
    Utf8CP key, 
    DPlane3dCR value
)
    {
    if (0 == strcmp(key, prop_WorkingPlane()))
        {
        m_workingPlane = value;
        }

    T_Super::_SetProperty(key, value);
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                01/2018
//---------------+---------------+---------------+---------------+---------------+------
BentleyStatus LineStringMetesAndBoundsPlacementStrategy::_TryGetProperty
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

    return T_Super::_TryGetProperty(key, value);
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                01/2018
//---------------+---------------+---------------+---------------+---------------+------
void LineStringMetesAndBoundsPlacementStrategy::_SetProperty
(
    Utf8CP key, 
    GeometryManipulationStrategyProperty const& value
)
    {
    if (0 == strcmp(key, prop_MetesAndBounds()))
        {
        MetesAndBounds const* mab = dynamic_cast<MetesAndBounds const*>(&value);
        if (nullptr != mab)
            {
            m_metesAndBounds = *mab;
            }
        }

    T_Super::_SetProperty(key, value);
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                01/2018
//---------------+---------------+---------------+---------------+---------------+------
BentleyStatus LineStringMetesAndBoundsPlacementStrategy::_TryGetProperty
(
    Utf8CP key, 
    GeometryManipulationStrategyProperty& value
) const
    {
    if (0 == strcmp(key, prop_MetesAndBounds()))
        {
        value = m_metesAndBounds;
        return BentleyStatus::SUCCESS;
        }

    return T_Super::_TryGetProperty(key, value);
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                01/2018
//---------------+---------------+---------------+---------------+---------------+------
bool LineStringMetesAndBoundsPlacementStrategy::_IsComplete() const
    {
    return GetLineStringManipulationStrategy().GetAcceptedKeyPoints().size() > 1;
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                01/2018
//---------------+---------------+---------------+---------------+---------------+------
bool LineStringMetesAndBoundsPlacementStrategy::_CanAcceptMorePoints() const
    {
    return !_IsComplete();
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                01/2018
//---------------+---------------+---------------+---------------+---------------+------
bvector<DPoint3d> LineStringMetesAndBoundsPlacementStrategy::CalculateKeyPoints() const
    {
    bvector<DPoint3d> const& keyPoints = _GetKeyPoints();
    if (keyPoints.empty())
        return bvector<DPoint3d>();

    bvector<DPoint3d> newKeyPoints;

    for (MetesAndBounds::ValuePair const& directionLengthPair : m_metesAndBounds.GetValue())
        {
        if (DoubleOps::AlmostEqual(directionLengthPair.second, 0))
            continue;

        LineMetesAndBoundsPlacementStrategyPtr lineStrategy = LineMetesAndBoundsPlacementStrategy::Create(m_workingPlane);
        lineStrategy->SetProperty(LineMetesAndBoundsPlacementStrategy::prop_MetesAndBounds(), MetesAndBounds(directionLengthPair));
        lineStrategy->AddKeyPoint(newKeyPoints.empty() ? keyPoints.front() : newKeyPoints.back());

        bvector<DPoint3d> lineKeyPoints = lineStrategy->GetKeyPoints();
        if (lineKeyPoints.size() != 2 || lineKeyPoints.front().AlmostEqual(lineKeyPoints.back()))
            break;

        newKeyPoints.push_back(lineKeyPoints.back());
        }

    return newKeyPoints;
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                01/2018
//---------------+---------------+---------------+---------------+---------------+------
void LineStringMetesAndBoundsPlacementStrategy::_AddKeyPoint
(
    DPoint3dCR newKeyPoint
)
    {
    if (_IsComplete())
        return;

    GetLineStringManipulationStrategyForEdit().AppendKeyPoint(newKeyPoint);
    GetLineStringManipulationStrategyForEdit().AppendKeyPoints(CalculateKeyPoints());
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                01/2018
//---------------+---------------+---------------+---------------+---------------+------
void LineStringMetesAndBoundsPlacementStrategy::_OnPropertySet
(
    Utf8CP key
)
    {
    if (0 == strcmp(key, prop_MetesAndBounds()) && !GetLineStringManipulationStrategy().IsEmpty())
        {
        DPoint3d start = GetLineStringManipulationStrategy().GetFirstKeyPoint();

        if (!_IsDynamicKeyPointSet())
            {
            GetLineStringManipulationStrategyForEdit().Clear();
            _AddKeyPoint(start);
            }
        else
            {
            _AddDynamicKeyPoint(start);
            }
        }

    T_Super::_OnPropertySet(key);
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                01/2018
//---------------+---------------+---------------+---------------+---------------+------
void LineStringMetesAndBoundsPlacementStrategy::_AddDynamicKeyPoint
(
    DPoint3dCR newDynamicKeyPoint
)
    {
    if (_IsComplete())
        return;

    GetLineStringManipulationStrategyForEdit().AppendDynamicKeyPoint(newDynamicKeyPoint);
    bvector<DPoint3d> withCalculatedPoints {newDynamicKeyPoint};
    bvector<DPoint3d> calculatedPoints = CalculateKeyPoints();
    withCalculatedPoints.insert(withCalculatedPoints.end(), calculatedPoints.begin(), calculatedPoints.end());
    GetLineStringManipulationStrategyForEdit().AppendDynamicKeyPoints(withCalculatedPoints);
    }