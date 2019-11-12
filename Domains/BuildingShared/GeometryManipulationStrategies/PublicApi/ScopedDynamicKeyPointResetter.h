/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
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