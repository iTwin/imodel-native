/*--------------------------------------------------------------------------------------+
|
|     $Source: GeometryManipulationStrategies/PublicApi/CurveVectorPlacementStrategy.h $
|
|  $Copyright: (c) 2019 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

BEGIN_BUILDING_SHARED_NAMESPACE

//=======================================================================================
// @bsiclass                                     Mindaugas.Butkus               12/2017
//=======================================================================================
struct CurveVectorPlacementStrategy : public GeometryPlacementStrategy
    {
    DEFINE_T_SUPER(GeometryPlacementStrategy)

    private:
        CurveVectorManipulationStrategyPtr m_manipulationStrategy;

    protected:
        GEOMETRYMANIPULATIONSTRATEGIES_EXPORT CurveVectorPlacementStrategy();
        GEOMETRYMANIPULATIONSTRATEGIES_EXPORT CurveVectorPlacementStrategy(CurveVectorManipulationStrategyR manipulationStrategy);

        GEOMETRYMANIPULATIONSTRATEGIES_EXPORT virtual CurveVectorPtr _Finish(bool connectEndStart) const;

        virtual GeometryManipulationStrategyCR _GetManipulationStrategy() const override { return *m_manipulationStrategy; }
        virtual GeometryManipulationStrategyR _GetManipulationStrategyForEdit() override { return *m_manipulationStrategy; }
        CurveVectorManipulationStrategyCR GetCurveVectorManipulationStrategy() const { return *m_manipulationStrategy; }
        CurveVectorManipulationStrategyR GetCurveVectorManipulationStrategyForEdit() { return *m_manipulationStrategy; }

    public:
        GEOMETRYMANIPULATIONSTRATEGIES_EXPORT CurveVectorPtr Finish(bool connectEndStart = false) const;

        static CurveVectorPlacementStrategyPtr Create() { return new CurveVectorPlacementStrategy(); }
        GEOMETRYMANIPULATIONSTRATEGIES_EXPORT static CurveVectorPlacementStrategyPtr Create(CurveVectorManipulationStrategyR manipulationStrategy) { return new CurveVectorPlacementStrategy(manipulationStrategy); }

        GEOMETRYMANIPULATIONSTRATEGIES_EXPORT void ChangeDefaultNewGeometryType(DefaultNewGeometryType newGeometryType);
        GEOMETRYMANIPULATIONSTRATEGIES_EXPORT CurvePrimitivePlacementStrategyPtr GetCurrentCurvePrimitivePlacementStrategy() { return &m_manipulationStrategy->GetStrategyForAppend(); }

        GEOMETRYMANIPULATIONSTRATEGIES_EXPORT bool FinishContiniousPrimitive();
        
        GEOMETRYMANIPULATIONSTRATEGIES_EXPORT void ChangeDefaultPlacementStrategy(LinePlacementStrategyType newPlacementStrategyType);
        GEOMETRYMANIPULATIONSTRATEGIES_EXPORT void ChangeDefaultPlacementStrategy(ArcPlacementMethod method);
        GEOMETRYMANIPULATIONSTRATEGIES_EXPORT void ChangeDefaultPlacementStrategy(LineStringPlacementStrategyType newPlacementStrategyType);
    };

END_BUILDING_SHARED_NAMESPACE
