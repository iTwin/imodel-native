/*--------------------------------------------------------------------------------------+
|
|     $Source: GeometryManipulationStrategies/PublicApi/LineManipulationStrategy.h $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

BUILDING_SHARED_REFCOUNTED_PTR_AND_TYPEDEFS(LineManipulationStrategy)

BEGIN_BUILDING_SHARED_NAMESPACE

//=======================================================================================
// @bsiclass                                     Mindaugas.Butkus               12/2017
//=======================================================================================
struct LineManipulationStrategy : public CurvePrimitiveManipulationStrategy
    {
    DEFINE_T_SUPER(CurvePrimitiveManipulationStrategy)

    private:
        LineManipulationStrategy() : T_Super() {}

    protected:
        GEOMETRYMANIPULATIONSTRATEGIES_EXPORT virtual void _AppendKeyPoint(DPoint3dCR newKeyPoint) override;

        GEOMETRYMANIPULATIONSTRATEGIES_EXPORT virtual ICurvePrimitivePtr _Finish() const override;

    public:
        static LineManipulationStrategyPtr Create() { return new LineManipulationStrategy(); }
    };

END_BUILDING_SHARED_NAMESPACE
