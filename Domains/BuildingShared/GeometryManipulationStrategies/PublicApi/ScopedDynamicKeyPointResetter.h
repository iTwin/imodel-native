/*--------------------------------------------------------------------------------------+
|
|     $Source: GeometryManipulationStrategies/PublicApi/ScopedDynamicKeyPointResetter.h $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

BEGIN_BUILDING_SHARED_NAMESPACE

//=======================================================================================
// Create object of this struct to temporarily reset DynamicKeyPoints.
// Dynamic key points are restored when this object is destroyed.
// @bsiclass                                     Mindaugas.Butkus               04/2018
//=======================================================================================
struct ScopedDynamicKeyPointResetter : NonCopyableClass
    {
    private:
        DynamicStateBaseCPtr m_initialDynamicState;
        IResettableDynamicR m_strategy;

    public:
        GEOMETRYMANIPULATIONSTRATEGIES_EXPORT ScopedDynamicKeyPointResetter(IResettableDynamicR strategy);
        GEOMETRYMANIPULATIONSTRATEGIES_EXPORT ~ScopedDynamicKeyPointResetter();
    };

END_BUILDING_SHARED_NAMESPACE