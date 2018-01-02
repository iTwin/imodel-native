/*--------------------------------------------------------------------------------------+
|
|     $Source: GeometryManipulationStrategies/PublicApi/CurveVectorPlacementStrategy.h $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

BUILDING_SHARED_REFCOUNTED_PTR_AND_TYPEDEFS(CurveVectorPlacementStrategy)

BEGIN_BUILDING_SHARED_NAMESPACE

//=======================================================================================
// @bsiclass                                     Mindaugas.Butkus               12/2017
//=======================================================================================
struct CurveVectorPlacementStrategy : public GeometryPlacementStrategy
    {
    DEFINE_T_SUPER(GeometryPlacementStrategy)

    protected:
        CurveVectorPlacementStrategy(CurveVectorManipulationStrategyP manipulationStrategy) 
            : T_Super(manipulationStrategy) {}

        GEOMETRYMANIPULATIONSTRATEGIES_EXPORT virtual CurveVectorPtr _Finish() const;

    public:
        GEOMETRYMANIPULATIONSTRATEGIES_EXPORT CurveVectorPtr Finish() const;
    };

END_BUILDING_SHARED_NAMESPACE
