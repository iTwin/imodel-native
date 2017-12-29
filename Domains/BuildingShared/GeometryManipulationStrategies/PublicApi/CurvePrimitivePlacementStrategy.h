/*--------------------------------------------------------------------------------------+
|
|     $Source: GeometryManipulationStrategies/PublicApi/CurvePrimitivePlacementStrategy.h $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

BUILDING_SHARED_REFCOUNTED_PTR_AND_TYPEDEFS(CurvePrimitivePlacementStrategy)

BEGIN_BUILDING_SHARED_NAMESPACE

//=======================================================================================
// @bsiclass                                     Mindaugas.Butkus               12/2017
//=======================================================================================
struct CurvePrimitivePlacementStrategy : public CurveVectorPlacementStrategy
    {
    DEFINE_T_SUPER(CurveVectorPlacementStrategy)

    protected:
        CurvePrimitivePlacementStrategy(CurvePrimitiveManipulationStrategyP manipulationStrategy)
            : T_Super(manipulationStrategy) {}

        GEOMETRYMANIPULATIONSTRATEGIES_EXPORT virtual CurveVectorPtr _Finish() const override;
        GEOMETRYMANIPULATIONSTRATEGIES_EXPORT virtual ICurvePrimitivePtr _FinishPrimitive() const;
    public:
        GEOMETRYMANIPULATIONSTRATEGIES_EXPORT ICurvePrimitivePtr FinishPrimitive() const;
    };

END_BUILDING_SHARED_NAMESPACE