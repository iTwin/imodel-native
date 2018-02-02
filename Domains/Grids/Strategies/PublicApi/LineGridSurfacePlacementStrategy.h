/*--------------------------------------------------------------------------------------+
|
|     $Source: Grids/Strategies/PublicApi/LineGridSurfacePlacementStrategy.h $
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
struct LineGridSurfacePlacementStrategy : public SketchGridSurfacePlacementStrategy
    {
    DEFINE_T_SUPER(SketchGridSurfacePlacementStrategy)

    private:
        LineGridSurfaceManipulationStrategyPtr m_manipulationStrategy;

    protected:
        LineGridSurfacePlacementStrategy(BBS::LinePlacementStrategyType linePlacementStrategyType);

        virtual SketchGridSurfaceManipulationStrategyCR _GetSketchGridSurfaceManipulationStrategy() const { return *m_manipulationStrategy; }
        virtual SketchGridSurfaceManipulationStrategyR _GetSketchGridSurfaceManipulationStrategyR() { return *m_manipulationStrategy; }
    
        virtual LineGridSurfaceManipulationStrategyCR _GetLineGridSurfaceManipulationStrategy() const { return *m_manipulationStrategy; }
        virtual LineGridSurfaceManipulationStrategyR _GetLineGridSurfaceManipulationStrategyR() { return *m_manipulationStrategy; }

        virtual BentleyStatus _TryGetProperty(Utf8CP key, double & value) const override;
        virtual void _SetProperty(Utf8CP key, double const & value) override;

    public:
        GRIDSTRATEGIES_EXPORT static LineGridSurfacePlacementStrategyPtr Create(BBS::LinePlacementStrategyType linePlacementStrategyType) { return new LineGridSurfacePlacementStrategy(linePlacementStrategyType); }
        GRIDSTRATEGIES_EXPORT void ChangeCurrentPlacementType(BBS::LinePlacementStrategyType newLinePlacementStrategyType);
    };

END_GRIDS_NAMESPACE