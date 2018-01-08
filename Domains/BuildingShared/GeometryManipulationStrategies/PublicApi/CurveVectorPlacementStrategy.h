/*--------------------------------------------------------------------------------------+
|
|     $Source: GeometryManipulationStrategies/PublicApi/CurveVectorPlacementStrategy.h $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
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

        GEOMETRYMANIPULATIONSTRATEGIES_EXPORT virtual CurveVectorPtr _Finish() const;

        virtual GeometryManipulationStrategyCR _GetManipulationStrategy() const override { return *m_manipulationStrategy; }
        virtual GeometryManipulationStrategyR _GetManipulationStrategyR() override { return *m_manipulationStrategy; }
        CurveVectorManipulationStrategyCR GetCurveVectorManipulationStrategy() const { return *m_manipulationStrategy; }
        CurveVectorManipulationStrategyR GetCurveVectorManipulationStrategyR() { return *m_manipulationStrategy; }

    public:
        GEOMETRYMANIPULATIONSTRATEGIES_EXPORT CurveVectorPtr Finish() const;

        static CurveVectorPlacementStrategyPtr Create() { return new CurveVectorPlacementStrategy(); }

        GEOMETRYMANIPULATIONSTRATEGIES_EXPORT void ChangeDefaultNewGeometryType(DefaultNewGeometryType newGeometryType);
    };

END_BUILDING_SHARED_NAMESPACE
