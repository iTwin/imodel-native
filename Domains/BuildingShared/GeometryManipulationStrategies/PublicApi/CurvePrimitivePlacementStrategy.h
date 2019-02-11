/*--------------------------------------------------------------------------------------+
|
|     $Source: GeometryManipulationStrategies/PublicApi/CurvePrimitivePlacementStrategy.h $
|
|  $Copyright: (c) 2019 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

BEGIN_BUILDING_SHARED_NAMESPACE

//=======================================================================================
// @bsiclass                                     Mindaugas.Butkus               12/2017
//=======================================================================================
struct CurvePrimitivePlacementStrategy : public GeometryPlacementStrategy
    {
    DEFINE_T_SUPER(GeometryPlacementStrategy)

    protected:
        CurvePrimitivePlacementStrategy()
            : T_Super() {}

        virtual CurvePrimitiveManipulationStrategyCR _GetCurvePrimitiveManipulationStrategy() const = 0;
        virtual CurvePrimitiveManipulationStrategyR _GetCurvePrimitiveManipulationStrategyForEdit() = 0;

        GEOMETRYMANIPULATIONSTRATEGIES_EXPORT virtual ICurvePrimitivePtr _FinishPrimitive() const;

        GEOMETRYMANIPULATIONSTRATEGIES_EXPORT virtual bool _IsContinious() const;
        GEOMETRYMANIPULATIONSTRATEGIES_EXPORT virtual bool _IsEmpty() const;

        GEOMETRYMANIPULATIONSTRATEGIES_EXPORT virtual void _CopyKeyPointsTo(ArcPlacementStrategyR) const;
        GEOMETRYMANIPULATIONSTRATEGIES_EXPORT virtual void _CopyKeyPointsTo(LinePlacementStrategyR) const;
        GEOMETRYMANIPULATIONSTRATEGIES_EXPORT virtual void _CopyKeyPointsTo(LineStringPlacementStrategyR) const;
        GEOMETRYMANIPULATIONSTRATEGIES_EXPORT virtual void _CopyKeyPointsTo(SplineControlPointsPlacementStrategyR) const;
        GEOMETRYMANIPULATIONSTRATEGIES_EXPORT virtual void _CopyKeyPointsTo(SplineThroughPointsPlacementStrategyR) const;

    public:
        GEOMETRYMANIPULATIONSTRATEGIES_EXPORT ICurvePrimitivePtr FinishPrimitive() const;
        bool IsEmpty() const;
    };

END_BUILDING_SHARED_NAMESPACE