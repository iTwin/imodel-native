/*--------------------------------------------------------------------------------------+
|
|     $Source: GeometryManipulationStrategies/LineStringMetesAndBoundsPlacementStrategy.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <BuildingShared/BuildingSharedApi.h>

USING_NAMESPACE_BUILDING_SHARED

const Utf8CP LineStringMetesAndBoundsPlacementStrategy::prop_WorkingPlane = "WorkingPlane";
const Utf8CP LineStringMetesAndBoundsPlacementStrategy::prop_DirectionStrings = "DirectionStrings";
const Utf8CP LineStringMetesAndBoundsPlacementStrategy::prop_Lengths = "Lengths";

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                01/2018
//---------------+---------------+---------------+---------------+---------------+------
LineStringMetesAndBoundsPlacementStrategy::LineStringMetesAndBoundsPlacementStrategy()
    : T_Super()
    , m_workingPlane(DPlane3d::FromOriginAndNormal({0,0,0},DVec3d::From(0,0,1)))
    {
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
    if (0 == strcmp(key, prop_WorkingPlane))
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
    if (0 == strcmp(key, prop_WorkingPlane))
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
    bvector<Utf8String> const& value
)
    {
    if (0 == strcmp(key, prop_DirectionStrings))
        {
        m_directionStrings = value;
        }

    T_Super::_SetProperty(key, value);
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                01/2018
//---------------+---------------+---------------+---------------+---------------+------
BentleyStatus LineStringMetesAndBoundsPlacementStrategy::_TryGetProperty
(
    Utf8CP key, 
    bvector<Utf8String>& value
) const
    {
    if (0 == strcmp(key, prop_DirectionStrings))
        {
        value = m_directionStrings;
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
    bvector<double> const& value
)
    {
    if (0 == strcmp(key, prop_Lengths))
        {
        m_lengths = value;
        }

    T_Super::_SetProperty(key, value);
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                01/2018
//---------------+---------------+---------------+---------------+---------------+------
BentleyStatus LineStringMetesAndBoundsPlacementStrategy::_TryGetProperty
(
    Utf8CP key, 
    bvector<double>& value
) const
    {
    if (0 == strcmp(key, prop_Lengths))
        {
        value = m_lengths;
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
void LineStringMetesAndBoundsPlacementStrategy::_AddKeyPoint
(
    DPoint3dCR newKeyPoint
)
    {
    if (_IsComplete())
        return;

    if (m_directionStrings.empty() || m_lengths.empty() || m_directionStrings.size() != m_lengths.size())
        return;

    bvector<DPoint3d> keyPoints = {newKeyPoint};

    auto directionsIter = m_directionStrings.begin();
    auto lengthsIter = m_lengths.begin();
    for (; m_directionStrings.end() != directionsIter && m_lengths.end() != lengthsIter;
         ++directionsIter, ++lengthsIter)
        {
        if (DoubleOps::AlmostEqual(*lengthsIter, 0))
            continue;

        LineMetesAndBoundsPlacementStrategyPtr lineStrategy = LineMetesAndBoundsPlacementStrategy::Create(m_workingPlane);
        bpair<Utf8String, double> metesAndBounds {*directionsIter, *lengthsIter};
        lineStrategy->SetProperty(LineMetesAndBoundsPlacementStrategy::prop_MetesAndBounds, MetesAndBounds({metesAndBounds}));
        lineStrategy->AddKeyPoint(keyPoints.back());

        bvector<DPoint3d> lineKeyPoints = lineStrategy->GetKeyPoints();
        if (lineKeyPoints.size() != 2 || lineKeyPoints.front().AlmostEqual(lineKeyPoints.back()))
            return;

        keyPoints.push_back(lineKeyPoints.back());
        }

    GetLineStringManipulationStrategyR().AppendKeyPoints(keyPoints);
    }