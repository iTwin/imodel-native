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
struct CurvePrimitiveManipulationStrategy : public GeometryManipulationStrategy
    {
    DEFINE_T_SUPER(GeometryManipulationStrategy)

    protected:
        CurvePrimitiveManipulationStrategy() : T_Super() {}

        virtual ICurvePrimitivePtr _Finish() const = 0;

    public:
        GEOMETRYMANIPULATIONSTRATEGIES_EXPORT ICurvePrimitivePtr Finish() const;
    };

END_BUILDING_SHARED_NAMESPACE