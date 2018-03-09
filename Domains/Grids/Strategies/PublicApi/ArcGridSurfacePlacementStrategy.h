/*--------------------------------------------------------------------------------------+
|
|     $Source: Grids/Strategies/PublicApi/ArcGridSurfacePlacementStrategy.h $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
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
        ArcGridSurfacePlacementStrategy(BBS::ArcPlacementMethod arcPlacementStrategyType);

        virtual SketchGridSurfaceManipulationStrategyCR _GetSketchGridSurfaceManipulationStrategy() const { return *m_manipulationStrategy; }
        virtual SketchGridSurfaceManipulationStrategyR _GetSketchGridSurfaceManipulationStrategyForEdit() { return *m_manipulationStrategy; }

        virtual ArcGridSurfaceManipulationStrategyCR _GetArcGridSurfaceManipulationStrategy() const { return *m_manipulationStrategy; }
        virtual ArcGridSurfaceManipulationStrategyR _GetArcGridSurfaceManipulationStrategyForEdit() { return *m_manipulationStrategy; }

        BBS::ArcPlacementStrategyCPtr GetArcPlacementStrategy() const;
        BBS::ArcPlacementStrategyPtr GetArcPlacementStrategyForEdit();

    public:
        GRIDSTRATEGIES_EXPORT static ArcGridSurfacePlacementStrategyPtr Create(BBS::ArcPlacementMethod arcPlacementStrategyType) { return new ArcGridSurfacePlacementStrategy(arcPlacementStrategyType); }
    
        // IArcPlacementStrategy
        void SetPlacementMethod(BBS::ArcPlacementMethod method) override;
        void SetUseSweep(bool useSweep) override;
        void SetSweep(double sweep) override;
        void SetUseRadius(bool useRadius) override;
        void SetRadius(double radius) override;
    };

END_GRIDS_NAMESPACE