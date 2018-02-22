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
struct ArcGridSurfacePlacementStrategy : public SketchGridSurfacePlacementStrategy
    {
    DEFINE_T_SUPER(SketchGridSurfacePlacementStrategy)

    private:
        ArcGridSurfaceManipulationStrategyPtr m_manipulationStrategy;
        BBS::ArcPlacementStrategyPtr m_geometryPlacementStrategy;

    protected:
        ArcGridSurfacePlacementStrategy(BBS::ArcPlacementStrategyType arcPlacementStrategyType);

        virtual SketchGridSurfaceManipulationStrategyCR _GetSketchGridSurfaceManipulationStrategy() const { return *m_manipulationStrategy; }
        virtual SketchGridSurfaceManipulationStrategyR _GetSketchGridSurfaceManipulationStrategyForEdit() { return *m_manipulationStrategy; }

        virtual ArcGridSurfaceManipulationStrategyCR _GetArcGridSurfaceManipulationStrategy() const { return *m_manipulationStrategy; }
        virtual ArcGridSurfaceManipulationStrategyR _GetArcGridSurfaceManipulationStrategyForEdit() { return *m_manipulationStrategy; }

        virtual BBS::GeometryPlacementStrategyCPtr _TryGetGeometryPlacementStrategy() const  const override { return m_geometryPlacementStrategy.get(); }
        virtual BBS::GeometryPlacementStrategyPtr _TryGetGeometryPlacementStrategyForEdit() override { return m_geometryPlacementStrategy.get(); }

    public:
        GRIDSTRATEGIES_EXPORT static ArcGridSurfacePlacementStrategyPtr Create(BBS::ArcPlacementStrategyType arcPlacementStrategyType) { return new ArcGridSurfacePlacementStrategy(arcPlacementStrategyType); }
    };

END_GRIDS_NAMESPACE