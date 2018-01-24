/*--------------------------------------------------------------------------------------+
|
|     $Source: GeometryManipulationStrategies/PublicApi/SolidPrimitivePlacementStrategy.h $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

BEGIN_BUILDING_SHARED_NAMESPACE

//=======================================================================================
// @bsiclass                                     Mindaugas.Butkus               01/2018
//=======================================================================================
struct SolidPrimitivePlacementStrategy : public GeometryPlacementStrategy
    {
    DEFINE_T_SUPER(GeometryPlacementStrategy)

    private:
        SolidPrimitiveManipulationStrategyPtr m_manipulationStrategy;

    protected:
        SolidPrimitivePlacementStrategy(SolidPrimitiveManipulationStrategyR manipulationStrategy) 
            : T_Super()
            , m_manipulationStrategy(&manipulationStrategy)
            {}

        virtual GeometryManipulationStrategyCR _GetManipulationStrategy() const override { return *m_manipulationStrategy; }
        virtual GeometryManipulationStrategyR _GetManipulationStrategyR() override { return *m_manipulationStrategy; }

    public:
        static constexpr Utf8CP prop_BaseComplete() { return SolidPrimitiveManipulationStrategy::prop_BaseComplete(); }

        GEOMETRYMANIPULATIONSTRATEGIES_EXPORT ISolidPrimitivePtr FinishSolidPrimitive() const;
    };

END_BUILDING_SHARED_NAMESPACE