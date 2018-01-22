/*--------------------------------------------------------------------------------------+
|
|     $Source: GeometryManipulationStrategies/ExtrusionManipulationStrategy.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "PublicApi/GeometryManipulationStrategiesApi.h"

USING_NAMESPACE_BUILDING_SHARED

const Utf8CP ExtrusionManipulationStrategy::prop_BaseShapeStrategy = "BaseShapeStrategy";
const Utf8CP ExtrusionManipulationStrategy::prop_Height = "Height";
const Utf8CP ExtrusionManipulationStrategy::prop_ContinuousBaseShapePrimitiveComplete = "ContinuousBaseShapePrimitiveComplete";

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
// @bsimethod                                    Mindaugas.Butkus                01/2018
//---------------+---------------+---------------+---------------+---------------+------
DVec3d ExtrusionManipulationStrategy::CalculateSweepDirection
(
    DPoint3dCR keyPoint
) const
    {
    DPlane3d basePlane = m_baseShapeManipulationStrategy->GetWorkingPlane();

    DVec3d planeTranslation = basePlane.normal;
    planeTranslation.ScaleToLength(m_height);

    DPoint3d topPlaneOrigin = basePlane.origin;
    topPlaneOrigin.Add(planeTranslation);

    DPlane3d topPlane = DPlane3d::FromOriginAndNormal(topPlaneOrigin, basePlane.normal);

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

    if (!m_heightSet)
        {
        m_dynamicHeight = CalculateHeight(newDynamicKeyPoint);
        m_dynamicHeightSet = true;
        return;
        }

    if (!m_sweepDirectionSet)
        {
        m_dynamicSweepDirection = CalculateSweepDirection(newDynamicKeyPoint);
        m_dynamicSweepDirectionSet = true;
        return;
        }
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

    if (!m_heightSet)
        {
        m_height = CalculateHeight(newKeyPoint);
        m_heightSet = true;
        return;
        }

    if (!m_sweepDirectionSet)
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
    return m_baseShapeManipulationStrategy->IsComplete() && m_heightSet && m_sweepDirectionSet;
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                01/2018
//---------------+---------------+---------------+---------------+---------------+------
bool ExtrusionManipulationStrategy::_CanAcceptMorePoints() const
    {
    return m_baseShapeManipulationStrategy->CanAcceptMorePoints() && !m_baseComplete && !m_heightSet && !m_sweepDirectionSet;
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                01/2018
//---------------+---------------+---------------+---------------+---------------+------
double ExtrusionManipulationStrategy::GetHeight() const
    {
    if (!m_heightSet && !m_dynamicHeightSet)
        return 0;

    return m_dynamicHeightSet ? m_dynamicHeight : m_height;
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                01/2018
//---------------+---------------+---------------+---------------+---------------+------
DVec3d ExtrusionManipulationStrategy::GetSweepDirection() const
    {
    if (!m_sweepDirectionSet && !m_dynamicSweepDirectionSet)
        return DVec3d::From(0, 0, 0);

    return m_dynamicSweepDirectionSet ? m_dynamicSweepDirection : m_sweepDirection;
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                01/2018
//---------------+---------------+---------------+---------------+---------------+------
ISolidPrimitivePtr ExtrusionManipulationStrategy::_FinishSolidPrimitive() const
    {
    CurveVectorPtr baseShape = m_baseShapeManipulationStrategy->Finish();
    if (baseShape.IsNull())
        return nullptr;
    
    baseShape->SetBoundaryType(CurveVector::BOUNDARY_TYPE_Outer);
    if (!baseShape->AreStartEndAlmostEqual())
        {
        CurveLocationDetail start, end;
        baseShape->GetStartEnd(start, end);
        ICurvePrimitivePtr closingLine = ICurvePrimitive::CreateLine(end.point, start.point);
        baseShape->Add(closingLine);
        baseShape->ConsolidateAdjacentPrimitives(true);
        }

    return ISolidPrimitive::CreateDgnExtrusion(DgnExtrusionDetail(baseShape, GetSweepDirection(), true));
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
    if (0 == strcmp(key, prop_ContinuousBaseShapePrimitiveComplete))
        {
        if (!_IsBaseComplete())
            {
            m_baseShapeManipulationStrategy->FinishContiniousPrimitive();
            }
        }

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
    if (0 == strcmp(key, prop_BaseShapeStrategy))
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
    if (0 == strcmp(key, prop_Height))
        {
        if (!m_heightSet && !m_dynamicHeightSet)
            return BentleyStatus::ERROR;

        value = GetHeight();
        return BentleyStatus::SUCCESS;
        }

    return T_Super::_TryGetProperty(key, value);
    }