/*--------------------------------------------------------------------------------------+
|
|     $Source: GeometryManipulationStrategies/PublicApi/ArcStartMidEndPlacementStrategy.h $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

BUILDING_SHARED_REFCOUNTED_PTR_AND_TYPEDEFS(ArcStartMidEndPlacementStrategy)

BEGIN_BUILDING_SHARED_NAMESPACE

//=======================================================================================
// @bsiclass                                     Mindaugas.Butkus               12/2017
//=======================================================================================
struct ArcStartMidEndPlacementStrategy : public ArcPlacementStrategy
    {
    DEFINE_T_SUPER(ArcPlacementStrategy)

    private:
        ArcStartMidEndPlacementStrategy()
            : T_Super(ArcManipulationStrategy::Create().get())
            {}

        ArcManipulationStrategyR GetArcManipulationStrategyR();

    protected:
        GEOMETRYMANIPULATIONSTRATEGIES_EXPORT virtual void _AddKeyPoint(DPoint3dCR newKeyPoint) override;
        GEOMETRYMANIPULATIONSTRATEGIES_EXPORT virtual void _PopKeyPoint() override;

        GEOMETRYMANIPULATIONSTRATEGIES_EXPORT virtual void _AddDynamicKeyPoint(DPoint3dCR newDynamicKeyPoint) override;

    public:
        static ArcStartMidEndPlacementStrategyPtr Create() { return new ArcStartMidEndPlacementStrategy(); }
    };

END_BUILDING_SHARED_NAMESPACE