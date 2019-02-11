/*--------------------------------------------------------------------------------------+
|
|     $Source: GeometryManipulationStrategies/PublicApi/LineManipulationStrategy.h $
|
|  $Copyright: (c) 2019 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

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

        GEOMETRYMANIPULATIONSTRATEGIES_EXPORT virtual ICurvePrimitivePtr _FinishPrimitive() const override;
        GEOMETRYMANIPULATIONSTRATEGIES_EXPORT virtual bvector<ConstructionGeometry> _FinishConstructionGeometry() const override;

        GEOMETRYMANIPULATIONSTRATEGIES_EXPORT virtual CurvePrimitivePlacementStrategyPtr _CreateDefaultPlacementStrategy() override;
        GEOMETRYMANIPULATIONSTRATEGIES_EXPORT virtual LinePlacementStrategyPtr _CreateLinePlacementStrategy(LinePlacementStrategyType strategyType) override;

        GEOMETRYMANIPULATIONSTRATEGIES_EXPORT virtual bool _IsComplete() const override;
        GEOMETRYMANIPULATIONSTRATEGIES_EXPORT virtual bool _CanAcceptMorePoints() const override;

        GEOMETRYMANIPULATIONSTRATEGIES_EXPORT virtual bool _IsContinious() const override;

        GEOMETRYMANIPULATIONSTRATEGIES_EXPORT CurvePrimitiveManipulationStrategyPtr _Clone() const override;

        virtual ICurvePrimitive::CurvePrimitiveType _GetResultCurvePrimitiveType() const override { return ICurvePrimitive::CURVE_PRIMITIVE_TYPE_Line; }

    public:
        static LineManipulationStrategyPtr Create() { return new LineManipulationStrategy(); }
        GEOMETRYMANIPULATIONSTRATEGIES_EXPORT static LineManipulationStrategyPtr Create(DSegment3dCR line);
        GEOMETRYMANIPULATIONSTRATEGIES_EXPORT static LineManipulationStrategyPtr Create(DPoint3dCR start, DPoint3dCR end);
    };

END_BUILDING_SHARED_NAMESPACE
