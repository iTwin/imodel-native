/*--------------------------------------------------------------------------------------+
|
|     $Source: GeometryManipulationStrategies/PublicApi/ChildCurveVectorManipulationStrategy.h $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

BUILDING_SHARED_REFCOUNTED_PTR_AND_TYPEDEFS(ChildCurveVectorManipulationStrategy)

BEGIN_BUILDING_SHARED_NAMESPACE

//=======================================================================================
// @bsiclass                                     Mindaugas.Butkus               01/2018
//=======================================================================================
struct ChildCurveVectorManipulationStrategy : public CurvePrimitiveManipulationStrategy
    {
    DEFINE_T_SUPER(CurvePrimitiveManipulationStrategy)

    private:
        CurveVectorManipulationStrategyPtr m_cvManipulationStrategy;

        ChildCurveVectorManipulationStrategy();

    protected:
        virtual ICurvePrimitivePtr _FinishPrimitive() const override;

    public:
        ChildCurveVectorManipulationStrategyPtr Create() { return new ChildCurveVectorManipulationStrategy(); }
    };

END_BUILDING_SHARED_NAMESPACE