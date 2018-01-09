/*--------------------------------------------------------------------------------------+
|
|     $Source: GeometryManipulationStrategies/PublicApi/LineStringManipulationStrategy.h $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

BEGIN_BUILDING_SHARED_NAMESPACE

//=======================================================================================
// @bsiclass                                     Mindaugas.Butkus               01/2018
//=======================================================================================
struct LineStringManipulationStrategy : public CurvePrimitiveManipulationStrategy
    {
    DEFINE_T_SUPER(CurvePrimitiveManipulationStrategy)

    private:
        LineStringManipulationStrategy() : T_Super() {}

    protected:
        virtual ICurvePrimitivePtr _FinishPrimitive() const override;

        CurvePrimitiveManipulationStrategyPtr _Clone() const override;

        virtual bool _IsComplete() const override;
        virtual bool _CanAcceptMorePoints() const override { return true; }

        virtual CurvePrimitivePlacementStrategyPtr _CreateDefaultPlacementStrategy() override;

    public:
        static LineStringManipulationStrategyPtr Create() { return new LineStringManipulationStrategy(); }
    };

END_BUILDING_SHARED_NAMESPACE