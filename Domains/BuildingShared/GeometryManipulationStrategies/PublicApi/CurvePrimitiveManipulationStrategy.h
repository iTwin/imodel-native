/*--------------------------------------------------------------------------------------+
|
|     $Source: GeometryManipulationStrategies/PublicApi/CurvePrimitiveManipulationStrategy.h $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

BUILDING_SHARED_REFCOUNTED_PTR_AND_TYPEDEFS(CurvePrimitiveManipulationStrategy)

BEGIN_BUILDING_SHARED_NAMESPACE

//=======================================================================================
// @bsiclass                                     Mindaugas.Butkus               12/2017
//=======================================================================================
struct CurvePrimitiveManipulationStrategy : public CurveVectorManipulationStrategy
    {
    DEFINE_T_SUPER(CurveVectorManipulationStrategy)

    protected:
        CurvePrimitiveManipulationStrategy() : T_Super() {}

        GEOMETRYMANIPULATIONSTRATEGIES_EXPORT virtual CurveVectorPtr _Finish() const override;
        virtual ICurvePrimitivePtr _FinishPrimitive() const = 0;
    public:
        GEOMETRYMANIPULATIONSTRATEGIES_EXPORT ICurvePrimitivePtr FinishPrimitive() const;
    };

END_BUILDING_SHARED_NAMESPACE