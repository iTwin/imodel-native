/*--------------------------------------------------------------------------------------+
|
|     $Source: GeometryManipulationStrategies/PublicApi/ExtrusionPlacementStrategy.h $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

BEGIN_BUILDING_SHARED_NAMESPACE

//=======================================================================================
// @bsiclass                                     Mindaugas.Butkus               01/2018
//=======================================================================================
struct ExtrusionPlacementStrategy : public SolidPrimitivePlacementStrategy
    {
    DEFINE_T_SUPER(SolidPrimitivePlacementStrategy)

    private:
        static ExtrusionManipulationStrategyPtr CreateDefaultExtrusionManipulationStrategy();

    protected:
        GEOMETRYMANIPULATIONSTRATEGIES_EXPORT ExtrusionPlacementStrategy();
        ExtrusionPlacementStrategy(ExtrusionManipulationStrategyR manipulationStrategy) : T_Super(manipulationStrategy) {}

    public:
        static constexpr Utf8CP prop_BaseShapeStrategy() { return ExtrusionManipulationStrategy::prop_BaseShapeStrategy(); }
        static constexpr Utf8CP prop_Height() { return ExtrusionManipulationStrategy::prop_Height(); }
        static constexpr Utf8CP prop_ContinuousBaseShapePrimitiveComplete() { return ExtrusionManipulationStrategy::prop_ContinuousBaseShapePrimitiveComplete(); }

        static ExtrusionPlacementStrategyPtr Create() { return new ExtrusionPlacementStrategy(); }
        GEOMETRYMANIPULATIONSTRATEGIES_EXPORT static ExtrusionPlacementStrategyPtr Create(ExtrusionManipulationStrategyR manipulationStrategy) { return new ExtrusionPlacementStrategy(manipulationStrategy); }

        GEOMETRYMANIPULATIONSTRATEGIES_EXPORT ISolidPrimitivePtr FinishExtrusion(bool closedBaseShape = false, bool capped = false) const;
    };

END_BUILDING_SHARED_NAMESPACE