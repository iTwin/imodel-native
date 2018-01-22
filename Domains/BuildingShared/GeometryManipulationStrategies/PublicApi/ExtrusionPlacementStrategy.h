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
        
    public:
        GEOMETRYMANIPULATIONSTRATEGIES_EXPORT static const Utf8CP prop_BaseShapeStrategy;
        GEOMETRYMANIPULATIONSTRATEGIES_EXPORT static const Utf8CP prop_Height;
        GEOMETRYMANIPULATIONSTRATEGIES_EXPORT static const Utf8CP prop_ContinuousBaseShapePrimitiveComplete;

        static ExtrusionPlacementStrategyPtr Create() { return new ExtrusionPlacementStrategy(); }
    };

END_BUILDING_SHARED_NAMESPACE