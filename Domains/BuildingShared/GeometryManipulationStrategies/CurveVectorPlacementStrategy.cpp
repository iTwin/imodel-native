/*--------------------------------------------------------------------------------------+
|
|     $Source: GeometryManipulationStrategies/CurveVectorPlacementStrategy.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "PublicApi/GeometryManipulationStrategiesApi.h"

USING_NAMESPACE_BUILDING_SHARED

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                01/2018
//---------------+---------------+---------------+---------------+---------------+------
CurveVectorPlacementStrategy::CurveVectorPlacementStrategy() 
    : T_Super()
    , m_manipulationStrategy(CurveVectorManipulationStrategy::Create())
    {
    BeAssert(m_manipulationStrategy.IsValid());
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                12/2017
//---------------+---------------+---------------+---------------+---------------+------
CurveVectorPtr CurveVectorPlacementStrategy::_Finish() const
    {
    return m_manipulationStrategy->Finish();
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                12/2017
//---------------+---------------+---------------+---------------+---------------+------
CurveVectorPtr CurveVectorPlacementStrategy::Finish() const
    {
    return _Finish();
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Mindaugas.Butkus                01/2018
//---------------+---------------+---------------+---------------+---------------+------
void CurveVectorPlacementStrategy::ChangeDefaultNewGeometryType
(
    DefaultNewGeometryType newGeometryType
)
    {
    m_manipulationStrategy->ChangeDefaultNewGeometryType(newGeometryType);
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Haroldas Vitunskas              01/2018
//---------------+---------------+---------------+---------------+---------------+------
bool CurveVectorPlacementStrategy::FinishContiniousPrimitive()
    {
    return m_manipulationStrategy->FinishContiniousPrimitive();
    }
