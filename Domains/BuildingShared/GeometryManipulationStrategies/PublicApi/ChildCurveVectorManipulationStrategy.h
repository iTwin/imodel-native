/*--------------------------------------------------------------------------------------+
|
|     $Source: GeometryManipulationStrategies/PublicApi/ChildCurveVectorManipulationStrategy.h $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

BEGIN_BUILDING_SHARED_NAMESPACE

//=======================================================================================
// @bsiclass                                     Mindaugas.Butkus               01/2018
//=======================================================================================
struct ChildCurveVectorManipulationStrategy : public CurvePrimitiveManipulationStrategy
    {
    DEFINE_T_SUPER(CurvePrimitiveManipulationStrategy)

    private:
        CurveVector::BoundaryType m_boundaryType;
        CurveVectorManipulationStrategyPtr m_cvManipulationStrategy;

        ChildCurveVectorManipulationStrategy();

    protected:
        virtual ICurvePrimitivePtr _FinishPrimitive() const override;
        virtual CurvePrimitivePlacementStrategyPtr _CreateDefaultPlacementStrategy() override;

        virtual bool _IsComplete() const override;
        virtual bool _CanAcceptMorePoints() const override;

        virtual bool _IsContinious() const override { return false; }

        CurvePrimitiveManipulationStrategyPtr _Clone() const override;

    public:
        static ChildCurveVectorManipulationStrategyPtr Create() { return new ChildCurveVectorManipulationStrategy(); }
        static ChildCurveVectorManipulationStrategyPtr Create(CurveVectorCR cv);
    };

END_BUILDING_SHARED_NAMESPACE