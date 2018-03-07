#pragma once


namespace BBS = BENTLEY_BUILDING_SHARED_NAMESPACE_NAME;

BEGIN_GRIDS_NAMESPACE
//=======================================================================================
// @bsiclass                                     Martynas.Saulius             02/2018
//=======================================================================================
struct SplineGridSurfacePlacementStrategy : public SketchGridSurfacePlacementStrategy
    {
        DEFINE_T_SUPER(SketchGridSurfacePlacementStrategy)
        
        private:
            SplineGridSurfaceManipulationStrategyPtr m_manipulationStrategy;
        
        protected:
            SplineGridSurfacePlacementStrategy(BBS::SplinePlacementStrategyType strategyType);
            
            virtual SketchGridSurfaceManipulationStrategyCR _GetSketchGridSurfaceManipulationStrategy() const { return *m_manipulationStrategy; }
            virtual SketchGridSurfaceManipulationStrategyR _GetSketchGridSurfaceManipulationStrategyForEdit() { return *m_manipulationStrategy; }

            virtual SplineGridSurfaceManipulationStrategyCR _GetSplineGridSurfaceManipulationStrategy() const { return *m_manipulationStrategy; }
            virtual SplineGridSurfaceManipulationStrategyR _GetSplineGridSurfaceManipulationStrategyForEdit() { return *m_manipulationStrategy; }

        public:
            GRIDSTRATEGIES_EXPORT static SplineGridSurfacePlacementStrategyPtr Create(BBS::SplinePlacementStrategyType strategyType) { return new SplineGridSurfacePlacementStrategy(strategyType); }
    };
END_GRIDS_NAMESPACE 