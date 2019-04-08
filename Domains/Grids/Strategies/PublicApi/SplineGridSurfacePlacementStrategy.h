/*--------------------------------------------------------------------------------------+
|
|     $Source: Grids/Strategies/PublicApi/SplineGridSurfacePlacementStrategy.h $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

BEGIN_GRIDS_NAMESPACE
namespace BBS = BENTLEY_BUILDING_SHARED_NAMESPACE_NAME;
//=======================================================================================
// @bsiclass                                     Martynas.Saulius             02/2018
//=======================================================================================
struct SplineGridSurfacePlacementStrategy : public SketchGridSurfacePlacementStrategy
    {
        DEFINE_T_SUPER(SketchGridSurfacePlacementStrategy)
        
        private:
            SplineGridSurfaceManipulationStrategyPtr m_manipulationStrategy;
        
        protected:
            SplineGridSurfacePlacementStrategy(BBS::SplinePlacementStrategyType strategyType, Dgn::DgnDbR db);
            
            virtual SketchGridSurfaceManipulationStrategyCR _GetSketchGridSurfaceManipulationStrategy() const { return *m_manipulationStrategy; }
            virtual SketchGridSurfaceManipulationStrategyR _GetSketchGridSurfaceManipulationStrategyForEdit() { return *m_manipulationStrategy; }

            virtual SplineGridSurfaceManipulationStrategyCR _GetSplineGridSurfaceManipulationStrategy() const { return *m_manipulationStrategy; }
            virtual SplineGridSurfaceManipulationStrategyR _GetSplineGridSurfaceManipulationStrategyForEdit() { return *m_manipulationStrategy; }

        public:
            GRIDSTRATEGIES_EXPORT static SplineGridSurfacePlacementStrategyPtr Create(BBS::SplinePlacementStrategyType strategyType, Dgn::DgnDbR db);
    };
END_GRIDS_NAMESPACE 