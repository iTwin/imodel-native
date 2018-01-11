/*--------------------------------------------------------------------------------------+
|
|     $Source: GeometryManipulationStrategies/PublicApi/ElementPlacementStrategy.h $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

BEGIN_BUILDING_SHARED_NAMESPACE

//=======================================================================================
// @bsiclass                                     Mindaugas.Butkus               01/2018
//=======================================================================================
struct ElementPlacementStrategy : public GeometryPlacementStrategy
    {
    DEFINE_T_SUPER(GeometryPlacementStrategy)

    protected:
        ElementPlacementStrategy() : T_Super() {}

        virtual ElementManipulationStrategyCR _GetElementManipulationStrategy() const = 0;
        virtual ElementManipulationStrategyR _GetElementManipulationStrategyR() = 0;

        GEOMETRYMANIPULATIONSTRATEGIES_EXPORT virtual Dgn::DgnElementPtr _FinishElement(Dgn::DgnModelR model) const;

    public:
        GEOMETRYMANIPULATIONSTRATEGIES_EXPORT Dgn::DgnElementPtr FinishElement(Dgn::DgnModelR model) const;
    };

END_BUILDING_SHARED_NAMESPACE