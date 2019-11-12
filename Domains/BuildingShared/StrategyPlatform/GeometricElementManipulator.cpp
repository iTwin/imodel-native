/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "PublicAPI/StrategyPlatformAPI.h"

USING_NAMESPACE_BUILDING_SHARED

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                01/2018
//---------------+---------------+---------------+---------------+---------------+------
void GeometricElementManipulator::ControlPoint::UpdatePoint
(
    DPoint3dCR newPoint,
    bool isDynamics
)
    {
    if (isDynamics)
        m_strategy.UpdateDynamicKeyPoint(newPoint, m_keyPointIndex);
    else
        m_strategy.ReplaceKeyPoint(newPoint, m_keyPointIndex);

    m_strategy.FinishElement();
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                04/2018
//---------------+---------------+---------------+---------------+---------------+------
bool GeometricElementManipulator::_IsDisplayedInView
(
    Dgn::DgnViewportR vp
)
    {
    if (!vp.GetViewController().IsModelViewed(_GetElement().GetModelId()))
        return false;

    Dgn::GeometrySourceCP geomSource = _GetElement().ToGeometrySource();
    if (nullptr == geomSource)
        return false;

    if (!vp.GetViewController().GetViewedCategories().Contains(geomSource->GetCategoryId()))
        return false;

    return true;
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                04/2018
//---------------+---------------+---------------+---------------+---------------+------
StatusInt GeometricElementManipulator::_OnModifyAccept
(
    Dgn::DgnButtonEventCR ev
)
    {
    StatusInt status = T_Super::_OnModifyAccept(ev);
    if (SUCCESS != status)
        return status;

    return _GetElement().GetDgnDb().SaveChanges();
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                04/2018
//---------------+---------------+---------------+---------------+---------------+------
void GeometricElementManipulator::_OnModifyCancel
(
    Dgn::DgnButtonEventCR ev
)
    {
    _GetElement().GetDgnDb().AbandonChanges();
    _GetStrategy().ResetDynamicKeyPoint();
    T_Super::_OnModifyCancel(ev);
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                04/2018
//---------------+---------------+---------------+---------------+---------------+------
StatusInt GeometricElementManipulator::_DoModify
(
    Dgn::DgnButtonEventCR ev,
    bool isDynamics
)
    {
    ControlPoint* controlPoint = dynamic_cast<ControlPoint*>(m_controls.m_locations[m_controls.FindFirstSelected()]);
    if (nullptr == controlPoint)
        return ERROR;

    DPoint3dCP point = ev.GetPoint();
    if (nullptr == point)
        return ERROR;

    controlPoint->UpdatePoint(*point, isDynamics);

    return SUCCESS;
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                04/2018
//---------------+---------------+---------------+---------------+---------------+------
bool GeometricElementManipulator::_DoCreateControls()
    {
    m_controls.ClearControls();
    T_Super::_DoCreateControls();

    bvector<DPoint3d> keyPoints = _GetStrategy().GetKeyPoints();
    if (keyPoints.size() > 1 && keyPoints.front().AlmostEqual(keyPoints.back()))
        keyPoints.pop_back();
        
    for (size_t index = 0; index < keyPoints.size(); ++index)
        {
        m_controls.m_locations.push_back(new ControlPoint(_GetStrategy(), index, keyPoints[index]));
        }

    return true;
    }