/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#pragma once

BEGIN_BUILDING_SHARED_NAMESPACE

//=======================================================================================
// @bsiclass                                     Haroldas.Vitunskas             01/2018
//=======================================================================================
enum class SplinePlacementStrategyType
{
    ControlPoints = 0,
    ThroughPoints
};

struct SplineManipulationStrategy : public CurvePrimitiveManipulationStrategy
    {
    DEFINE_T_SUPER(CurvePrimitiveManipulationStrategy)

    protected:
        SplineManipulationStrategy() : T_Super() {}

        GEOMETRYMANIPULATIONSTRATEGIES_EXPORT virtual bool _CanAcceptMorePoints() const override { return true; }

        GEOMETRYMANIPULATIONSTRATEGIES_EXPORT virtual bool _IsContinious() const override { return true; }

        virtual SplinePlacementStrategyType _GetType() const = 0;
        virtual SplinePlacementStrategyPtr _CreatePlacement() = 0;

    public:
        GEOMETRYMANIPULATIONSTRATEGIES_EXPORT static SplineManipulationStrategyPtr Create(SplinePlacementStrategyType placementStrategy);
        GEOMETRYMANIPULATIONSTRATEGIES_EXPORT SplinePlacementStrategyPtr CreatePlacement();
        GEOMETRYMANIPULATIONSTRATEGIES_EXPORT SplinePlacementStrategyType GetType() const;
    };

//=======================================================================================
// @bsiclass                                     Haroldas.Vitunskas             01/2018
//=======================================================================================
struct SplineControlPointsManipulationStrategy : public SplineManipulationStrategy
    {
    DEFINE_T_SUPER(SplineManipulationStrategy)

    private:
        int m_order;

        SplineControlPointsManipulationStrategy(int order);

    protected:
        GEOMETRYMANIPULATIONSTRATEGIES_EXPORT virtual ICurvePrimitivePtr _FinishPrimitive() const override;
        virtual bvector<ConstructionGeometry> _FinishConstructionGeometry() const override;
        virtual CurvePrimitivePlacementStrategyPtr _CreateDefaultPlacementStrategy() override;
        virtual bool _IsComplete() const override;

        void _SetOrder(int order) { m_order = order; }
        int _GetOrder() const { return m_order; }

        GEOMETRYMANIPULATIONSTRATEGIES_EXPORT CurvePrimitiveManipulationStrategyPtr _Clone() const override { return new SplineControlPointsManipulationStrategy(*this); };

        virtual SplinePlacementStrategyType _GetType() const override {return SplinePlacementStrategyType::ControlPoints;}
        virtual ICurvePrimitive::CurvePrimitiveType _GetResultCurvePrimitiveType() const override { return ICurvePrimitive::CURVE_PRIMITIVE_TYPE_BsplineCurve; }

        virtual SplinePlacementStrategyPtr _CreatePlacement() override;

        virtual void _SetProperty(Utf8CP key, int const& value) override;
        virtual BentleyStatus _TryGetProperty(Utf8CP key, int& value) const override;
        using T_Super::_SetProperty;
        using T_Super::_TryGetProperty;

    public:
        static constexpr Utf8CP prop_Order() { return "Order"; }

        static SplineControlPointsManipulationStrategyPtr Create(int order) { return new SplineControlPointsManipulationStrategy(order); }

        void SetOrder(int order) { _SetOrder(order); }
        int GetOrder() const { return _GetOrder(); }
        
        GEOMETRYMANIPULATIONSTRATEGIES_EXPORT static const int default_Order;
    };

//=======================================================================================
// @bsiclass                                     Haroldas.Vitunskas             01/2018
//=======================================================================================
struct SplineThroughPointsManipulationStrategy : public SplineManipulationStrategy
    {
    DEFINE_T_SUPER(SplineManipulationStrategy)

    private:
        DVec3d m_startTangent = DVec3d::From(0, 0, 0);
        DVec3d m_endTangent = DVec3d::From(0, 0, 0);

        SplineThroughPointsManipulationStrategy();

    protected:
        GEOMETRYMANIPULATIONSTRATEGIES_EXPORT virtual ICurvePrimitivePtr _FinishPrimitive() const override;
        virtual bvector<ConstructionGeometry> _FinishConstructionGeometry() const override;
        virtual CurvePrimitivePlacementStrategyPtr _CreateDefaultPlacementStrategy() override;
        virtual bool _IsComplete() const override;

        void _SetStartTangent(DVec3d startTangent) { m_startTangent = startTangent; }
        void _RemoveStartTangent() { m_startTangent.Zero(); }
        DVec3d _GetStartTangent() const { return m_startTangent; }

        void _SetEndTangent(DVec3d endTangent) { m_endTangent = endTangent; }
        void _RemoveEndTangent() { m_endTangent.Zero(); }
        DVec3d _GetEndTangent() const { return m_endTangent; }

        GEOMETRYMANIPULATIONSTRATEGIES_EXPORT CurvePrimitiveManipulationStrategyPtr _Clone() const override { return new SplineThroughPointsManipulationStrategy(*this); };

        virtual SplinePlacementStrategyType _GetType() const override { return SplinePlacementStrategyType::ThroughPoints; }
        virtual ICurvePrimitive::CurvePrimitiveType _GetResultCurvePrimitiveType() const override { return ICurvePrimitive::CURVE_PRIMITIVE_TYPE_InterpolationCurve; }

        virtual SplinePlacementStrategyPtr _CreatePlacement() override;


        virtual void _SetProperty(Utf8CP key, DVec3d const& value) override;
        virtual BentleyStatus _TryGetProperty(Utf8CP key, DVec3d& value) const override;
        using T_Super::_SetProperty;
        using T_Super::_TryGetProperty;

    public:
        static constexpr Utf8CP prop_StartTangent() { return "StartTangend"; }
        static constexpr Utf8CP prop_EndTangent() { return "EndTangent"; }

        static SplineThroughPointsManipulationStrategyPtr Create() { return new SplineThroughPointsManipulationStrategy(); }

        void SetStartTangent(DVec3d startTangent);
        void RemoveStartTangent() { _RemoveStartTangent(); }
        DVec3d GetStartTangent() const { return _GetStartTangent(); }

        void SetEndTangent(DVec3d endTangent);
        void RemoveEndTangent() { _RemoveEndTangent(); }
        DVec3d GetEndTangent() const { return _GetEndTangent(); }
    };

END_BUILDING_SHARED_NAMESPACE
