/*--------------------------------------------------------------------------------------+
|
|     $Source: GeometryManipulationStrategies/PublicApi/ArcPlacementStrategy.h $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

BEGIN_BUILDING_SHARED_NAMESPACE

enum class ArcPlacementStrategyType
    {
    StartCenter = 0,
    CenterStart = 1,
    StartMidEnd = 2,
    StartEndMid = 3
    };

//=======================================================================================
// @bsiclass                                     Mindaugas.Butkus               12/2017
//=======================================================================================
struct ArcPlacementStrategy : public CurvePrimitivePlacementStrategy
    {
    DEFINE_T_SUPER(CurvePrimitivePlacementStrategy)

    private:
        ArcManipulationStrategyPtr m_manipulationStrategy;

    protected:
        GEOMETRYMANIPULATIONSTRATEGIES_EXPORT ArcPlacementStrategy(ArcManipulationStrategyP manipulationStrategy);
    
        DPoint3d CalculateVec90KeyPoint(DPoint3dCR endPoint) const;

        virtual GeometryManipulationStrategyCR _GetManipulationStrategy() const override { return *m_manipulationStrategy; }
        virtual GeometryManipulationStrategyR _GetManipulationStrategyForEdit() override { return *m_manipulationStrategy; }
        ArcManipulationStrategyCR GetArcManipulationStrategy() const { return *m_manipulationStrategy; }
        ArcManipulationStrategyR GetArcManipulationStrategyForEdit() { return *m_manipulationStrategy; }

    public:
        static constexpr Utf8CP prop_Normal() { return EllipseManipulationStrategy::prop_Normal(); }

        static ArcPlacementStrategyPtr Create(ArcPlacementStrategyType strategyType);
        GEOMETRYMANIPULATIONSTRATEGIES_EXPORT static ArcPlacementStrategyPtr Create(ArcPlacementStrategyType strategyType, ArcManipulationStrategyR manipulationStrategy);
    };

END_BUILDING_SHARED_NAMESPACE