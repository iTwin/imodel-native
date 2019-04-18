/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
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
        template <typename T> void UpdateKeyPoint(size_t index, T updateFn);

    protected:
        LineStringManipulationStrategy() : T_Super() {}
        GEOMETRYMANIPULATIONSTRATEGIES_EXPORT virtual ICurvePrimitivePtr _FinishPrimitive() const override;
        GEOMETRYMANIPULATIONSTRATEGIES_EXPORT virtual bvector<ConstructionGeometry> _FinishConstructionGeometry() const override;

        GEOMETRYMANIPULATIONSTRATEGIES_EXPORT CurvePrimitiveManipulationStrategyPtr _Clone() const override;

        GEOMETRYMANIPULATIONSTRATEGIES_EXPORT virtual void _UpdateDynamicKeyPoint(DPoint3dCR newDynamicKeyPoint, size_t index) override;
        GEOMETRYMANIPULATIONSTRATEGIES_EXPORT virtual void _ReplaceKeyPoint(DPoint3dCR newKeyPoint, size_t index) override;

        GEOMETRYMANIPULATIONSTRATEGIES_EXPORT virtual bool _IsComplete() const override;
        virtual bool _CanAcceptMorePoints() const override { return true; }

        GEOMETRYMANIPULATIONSTRATEGIES_EXPORT virtual bool _IsContinious() const override { return true; }

        GEOMETRYMANIPULATIONSTRATEGIES_EXPORT virtual CurvePrimitivePlacementStrategyPtr _CreateDefaultPlacementStrategy() override;
        GEOMETRYMANIPULATIONSTRATEGIES_EXPORT virtual LineStringPlacementStrategyPtr _CreateLineStringPlacementStrategy(LineStringPlacementStrategyType strategyType) override;

        virtual ICurvePrimitive::CurvePrimitiveType _GetResultCurvePrimitiveType() const override { return ICurvePrimitive::CURVE_PRIMITIVE_TYPE_LineString; }

    public:
        static LineStringManipulationStrategyPtr Create() { return new LineStringManipulationStrategy(); }
        static LineStringManipulationStrategyPtr Create(ICurvePrimitiveCR primitive);
    };

END_BUILDING_SHARED_NAMESPACE