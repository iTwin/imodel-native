/*--------------------------------------------------------------------------------------+
|
|     $Source: GeometryManipulationStrategies/PublicApi/ArcCenterStartPlacementStrategy.h $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

BEGIN_BUILDING_SHARED_NAMESPACE

//=======================================================================================
// @bsiclass                                     Mindaugas.Butkus               12/2017
//=======================================================================================
struct ArcCenterStartPlacementStrategy : public ArcPlacementStrategy
    {
    DEFINE_T_SUPER(ArcPlacementStrategy)

    private:
        ArcCenterStartPlacementStrategy() : T_Super(ArcManipulationStrategy::Create().get()) {}

    protected:
        GEOMETRYMANIPULATIONSTRATEGIES_EXPORT virtual void _AddKeyPoint(DPoint3dCR newKeyPoint) override;
        GEOMETRYMANIPULATIONSTRATEGIES_EXPORT virtual void _PopKeyPoint() override;

        GEOMETRYMANIPULATIONSTRATEGIES_EXPORT virtual void _AddDynamicKeyPoint(DPoint3dCR newDynamicKeyPoint) override;

    public:
        static ArcCenterStartPlacementStrategyPtr Create() { return new ArcCenterStartPlacementStrategy(); }
    };

END_BUILDING_SHARED_NAMESPACE