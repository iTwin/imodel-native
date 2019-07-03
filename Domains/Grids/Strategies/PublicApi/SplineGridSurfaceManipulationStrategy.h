/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#pragma once

BEGIN_GRIDS_NAMESPACE
namespace BBS = BENTLEY_BUILDING_SHARED_NAMESPACE_NAME;
//=======================================================================================
// @bsiclass                                     Martynas.Saulius             02/2018
//=======================================================================================
struct SplineGridSurfaceManipulationStrategy : public SketchGridSurfaceManipulationStrategy
    {
    DEFINE_T_SUPER(SketchGridSurfaceManipulationStrategy)

    private:
        BBS::SplineManipulationStrategyPtr m_geometryManipulationStrategy;
        BBS::SplinePlacementStrategyPtr m_geometryPlacementStrategy;
        SketchSplineGridSurfacePtr m_surface;

    protected:
        SplineGridSurfaceManipulationStrategy(BBS::SplinePlacementStrategyType strategyType, Dgn::DgnDbR db);

        virtual Dgn::DgnElementPtr _FinishElement(Dgn::DgnModelR model) override;
        virtual BBS::GeometryManipulationStrategyCR _GetGeometryManipulationStrategy() const override { return *m_geometryManipulationStrategy; }
        virtual BBS::GeometryManipulationStrategyR _GetGeometryManipulationStrategyForEdit() override { return *m_geometryManipulationStrategy; }
        virtual BBS::GeometryPlacementStrategyCPtr _TryGetGeometryPlacementStrategy() const override { return BBS::GeometryPlacementStrategyCPtr(m_geometryPlacementStrategy); }
        virtual BBS::GeometryPlacementStrategyPtr _TryGetGeometryPlacementStrategyForEdit() override { return m_geometryPlacementStrategy; }

        virtual BentleyStatus _UpdateGridSurface() override;
        virtual Utf8String _GetMessage() const override;
        virtual IPlanGridSurface const* _GetPlanGridSurfaceCP() const override { return m_surface.get(); }
        virtual IPlanGridSurface* _GetPlanGridSurfaceP() const override { return m_surface.get(); }
        virtual BBS::CurvePrimitiveManipulationStrategyCR _GetCurvePrimitiveManipulationStrategy() const override { return *m_geometryManipulationStrategy; }
        virtual BBS::CurvePrimitiveManipulationStrategyR _GetCurvePrimitiveManipulationStrategyForEdit() override { return *m_geometryManipulationStrategy; }

        
    public:
        GRIDSTRATEGIES_EXPORT static SplineGridSurfaceManipulationStrategyPtr Create(BBS::SplinePlacementStrategyType strategyType, Dgn::DgnDbR db);
    };
END_GRIDS_NAMESPACE