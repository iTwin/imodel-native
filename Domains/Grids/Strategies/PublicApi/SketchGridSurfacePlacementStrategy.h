/*--------------------------------------------------------------------------------------+
|
|     $Source: Grids/Strategies/PublicApi/SketchGridSurfacePlacementStrategy.h $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

BEGIN_GRIDS_NAMESPACE

namespace BBS = BENTLEY_BUILDING_SHARED_NAMESPACE_NAME;

//=======================================================================================
// @bsiclass                                     Haroldas.Vitunskas             01/2018
//=======================================================================================
struct SketchGridSurfacePlacementStrategy : public BBS::ElementPlacementStrategy
    {
    DEFINE_T_SUPER(BBS::ElementPlacementStrategy)
        
    protected:
        virtual BBS::ElementManipulationStrategyCR _GetElementManipulationStrategy() const override { return _GetSketchGridSurfaceManipulationStrategy(); }
        virtual BBS::ElementManipulationStrategyR _GetElementManipulationStrategyR() override { return _GetSketchGridSurfaceManipulationStrategyR(); }
        virtual BBS::GeometryManipulationStrategyCR _GetManipulationStrategy() const override { return _GetSketchGridSurfaceManipulationStrategy(); }
        virtual BBS::GeometryManipulationStrategyR _GetManipulationStrategyR() override { return _GetSketchGridSurfaceManipulationStrategyR(); }

        virtual SketchGridSurfaceManipulationStrategyCR _GetSketchGridSurfaceManipulationStrategy() const = 0;
        virtual SketchGridSurfaceManipulationStrategyR _GetSketchGridSurfaceManipulationStrategyR() = 0;

    public:
        GRIDSTRATEGIES_EXPORT void OnDynamicOperationEnd();
        
        GRIDSTRATEGIES_EXPORT static const Utf8CP prop_BottomElevation;
        GRIDSTRATEGIES_EXPORT static const Utf8CP prop_TopElevation;
        GRIDSTRATEGIES_EXPORT static const Utf8CP prop_Axis;
        GRIDSTRATEGIES_EXPORT static const Utf8CP prop_Name;
        GRIDSTRATEGIES_EXPORT static const Utf8CP prop_Grid;

        GRIDSTRATEGIES_EXPORT Utf8String GetMessage() const;
    };

END_GRIDS_NAMESPACE