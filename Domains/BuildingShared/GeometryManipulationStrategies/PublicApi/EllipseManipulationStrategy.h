/*--------------------------------------------------------------------------------------+
|
|     $Source: GeometryManipulationStrategies/PublicApi/EllipseManipulationStrategy.h $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

BEGIN_BUILDING_SHARED_NAMESPACE

//=======================================================================================
// @bsiclass                                     Mindaugas.Butkus               12/2017
//=======================================================================================
struct EllipseManipulationStrategy : public CurvePrimitiveManipulationStrategy
    {
    DEFINE_T_SUPER(CurvePrimitiveManipulationStrategy)

    private:
        static const size_t s_startIndex = 0;
        static const size_t s_centerIndex = 1;
        static const size_t s_vec90EndIndex = 2;
        static const size_t s_endIndex = 3;

        DVec3d m_normal;
        double m_sweep;

        bool DidSweepDirectionChange(double newSweep) const;
        double CalculateSweep(DPoint3dCR start, DPoint3dCR center, DPoint3dCR end) const;
        DVec3d CalculateVec90(DPoint3dCR start, DPoint3dCR center, DPoint3dCR vec90Point) const;

        DPoint3d GetStart() const;
        DPoint3d GetCenter() const;
        DVec3d GetVec0() const;
        DVec3d GetVec90() const;
        DVec3d GetEndVec() const;

    protected:
        void UpdateSweep(DPoint3dCR start, DPoint3dCR center, DPoint3dCR end);
        double GetSweep() const { return m_sweep; }

        EllipseManipulationStrategy() : T_Super(), m_sweep(0), m_normal(DVec3d::From(0, 0, 0)) {}

        GEOMETRYMANIPULATIONSTRATEGIES_EXPORT virtual void _AppendKeyPoint(DPoint3dCR newKeyPoint) override;
        GEOMETRYMANIPULATIONSTRATEGIES_EXPORT virtual void _AppendDynamicKeyPoint(DPoint3dCR newDynamicKeyPoint) override;
        GEOMETRYMANIPULATIONSTRATEGIES_EXPORT virtual void _AppendDynamicKeyPoints(bvector<DPoint3d> const& newDynamicKeyPoints) override;

        GEOMETRYMANIPULATIONSTRATEGIES_EXPORT virtual ICurvePrimitivePtr _FinishPrimitive() const override;
        virtual CurvePrimitivePlacementStrategyPtr _CreateDefaultPlacementStrategy() override { BeAssert(false && "Not implemented"); return nullptr; }

        GEOMETRYMANIPULATIONSTRATEGIES_EXPORT virtual void _OnKeyPointsChanged() override;

        GEOMETRYMANIPULATIONSTRATEGIES_EXPORT virtual void _SetProperty(Utf8CP key, DVec3d const& value) override;
        GEOMETRYMANIPULATIONSTRATEGIES_EXPORT virtual BentleyStatus _TryGetProperty(Utf8CP key, DVec3d& value) const override;

        GEOMETRYMANIPULATIONSTRATEGIES_EXPORT virtual bool _IsComplete() const override;
        GEOMETRYMANIPULATIONSTRATEGIES_EXPORT virtual bool _CanAcceptMorePoints() const override;

    public:
        static EllipseManipulationStrategyPtr Create() { return new EllipseManipulationStrategy(); }

        BE_PROP_NAME(Normal)
    };

END_BUILDING_SHARED_NAMESPACE