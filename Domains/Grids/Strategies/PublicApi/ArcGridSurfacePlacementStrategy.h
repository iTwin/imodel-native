/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#pragma once

BEGIN_GRIDS_NAMESPACE

namespace BBS = BENTLEY_BUILDING_SHARED_NAMESPACE_NAME;

//=======================================================================================
// @bsiclass                                     Mindaugas.Butkus               02/2018
//=======================================================================================
struct ArcGridSurfacePlacementStrategy : public SketchGridSurfacePlacementStrategy, BBS::IArcPlacementStrategy
    {
    DEFINE_T_SUPER(SketchGridSurfacePlacementStrategy)

    private:
        ArcGridSurfaceManipulationStrategyPtr m_manipulationStrategy;

    protected:
        ArcGridSurfacePlacementStrategy(BBS::ArcPlacementMethod arcPlacementStrategyType, Dgn::DgnDbR db);

        virtual SketchGridSurfaceManipulationStrategyCR _GetSketchGridSurfaceManipulationStrategy() const { return *m_manipulationStrategy; }
        virtual SketchGridSurfaceManipulationStrategyR _GetSketchGridSurfaceManipulationStrategyForEdit() { return *m_manipulationStrategy; }

        virtual ArcGridSurfaceManipulationStrategyCR _GetArcGridSurfaceManipulationStrategy() const { return *m_manipulationStrategy; }
        virtual ArcGridSurfaceManipulationStrategyR _GetArcGridSurfaceManipulationStrategyForEdit() { return *m_manipulationStrategy; }

        BBS::ArcPlacementStrategyCPtr GetArcPlacementStrategy() const;
        BBS::ArcPlacementStrategyPtr GetArcPlacementStrategyForEdit();

        // IArcPlacementStrategy
        virtual void _SetPlacementMethod(BBS::ArcPlacementMethod method) override;
        virtual void _SetUseSweep(bool useSweep) override;
        virtual void _SetSweep(double sweep) override;
        virtual void _SetUseRadius(bool useRadius) override;
        virtual void _SetRadius(double radius) override;

    public:
        GRIDSTRATEGIES_EXPORT static ArcGridSurfacePlacementStrategyPtr Create(BBS::ArcPlacementMethod arcPlacementStrategyType, Dgn::DgnDbR db);
    };

END_GRIDS_NAMESPACE