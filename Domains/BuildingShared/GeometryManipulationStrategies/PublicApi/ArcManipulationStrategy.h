/*--------------------------------------------------------------------------------------+
|
|     $Source: GeometryManipulationStrategies/PublicApi/ArcManipulationStrategy.h $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

BUILDING_SHARED_REFCOUNTED_PTR_AND_TYPEDEFS(ArcManipulationStrategy)

BEGIN_BUILDING_SHARED_NAMESPACE

//=======================================================================================
// @bsiclass                                     Mindaugas.Butkus               12/2017
//=======================================================================================
struct ArcManipulationStrategy : public CurvePrimitiveManipulationStrategy
    {
    DEFINE_T_SUPER(CurvePrimitiveManipulationStrategy)

    private:
        static const size_t s_startIndex = 0;
        static const size_t s_centerIndex = 1;
        static const size_t s_middleIndex = 2;
        static const size_t s_endIndex = 3;

        double m_sweep;

        ArcManipulationStrategy() : T_Super(), m_sweep(0) {}

        bool DidSweepDirectionChange(double newSweep) const;
        void UpdateSweep(DPoint3dCR endPoint);
        double CalculateSweep(DPoint3dCR endPoint) const;

        DPoint3d GetStart() const;
        DPoint3d GetCenter() const;
        DVec3d GetVec0() const;
        DVec3d GetVec90() const;
        DVec3d GetEndVec() const;

    protected:
        GEOMETRYMANIPULATIONSTRATEGIES_EXPORT virtual void _AppendKeyPoint(DPoint3dCR newKeyPoint) override;
        GEOMETRYMANIPULATIONSTRATEGIES_EXPORT virtual void _AppendDynamicKeyPoint(DPoint3dCR newDynamicKeyPoint) override;

        GEOMETRYMANIPULATIONSTRATEGIES_EXPORT virtual ICurvePrimitivePtr _FinishPrimitive() const override;

    public:
        static ArcManipulationStrategyPtr Create() { return new ArcManipulationStrategy(); }
    };

END_BUILDING_SHARED_NAMESPACE