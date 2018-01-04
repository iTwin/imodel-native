/*--------------------------------------------------------------------------------------+
|
|     $Source: GeometryManipulationStrategies/LineManipulationStrategy.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "PublicApi/GeometryManipulationStrategiesApi.h"

USING_NAMESPACE_BUILDING_SHARED

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                12/2017
//---------------+---------------+---------------+---------------+---------------+------
ICurvePrimitivePtr LineManipulationStrategy::_FinishPrimitive() const
    {
    bvector<DPoint3d> const& keyPoints = GetKeyPoints();
    if (keyPoints.size() < 2)
        return nullptr;

    return ICurvePrimitive::CreateLine(keyPoints[0], keyPoints[1]);
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                12/2017
//---------------+---------------+---------------+---------------+---------------+------
void LineManipulationStrategy::_AppendKeyPoint
(
    DPoint3dCR newKeyPoint
)
    {
    bvector<DPoint3d> const& keyPoints = GetKeyPoints();
    if ((IsDynamicKeyPointSet() && keyPoints.size() <= 2) ||
        (!IsDynamicKeyPointSet() && keyPoints.size() < 2))
        T_Super::_AppendKeyPoint(newKeyPoint);
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                01/2018
//---------------+---------------+---------------+---------------+---------------+------
bool LineManipulationStrategy::_IsComplete() const 
    {
    return GetAcceptedKeyPoints().size() == 2;
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                01/2018
//---------------+---------------+---------------+---------------+---------------+------
bool LineManipulationStrategy::_CanAcceptMorePoints() const 
    {
    return !_IsComplete();
    }