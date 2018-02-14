/*--------------------------------------------------------------------------------------+
|
|     $Source: Grids/Strategies/PublicApi/LineGridSurfaceManipulationStrategy.h $
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
struct LineGridSurfaceManipulationStrategy : public SketchGridSurfaceManipulationStrategy
    {
    DEFINE_T_SUPER(SketchGridSurfaceManipulationStrategy)

    private:
        BBS::LineManipulationStrategyPtr m_geometryManipulationStrategy;
        SketchLineGridSurfacePtr m_surface;

    protected:
        LineGridSurfaceManipulationStrategy();

        // ElementManipulationStrategy
        virtual Dgn::DgnElementPtr _FinishElement(Dgn::DgnModelR model) override;
        virtual BBS::GeometryManipulationStrategyCR _GetGeometryManipulationStrategy() const override { return *m_geometryManipulationStrategy; }
        virtual BBS::GeometryManipulationStrategyR _GetGeometryManipulationStrategyForEdit() override { return *m_geometryManipulationStrategy; }

        // SketchGridSurfaceManipulationStrategy
        virtual BentleyStatus _UpdateGridSurface() override;
        virtual Utf8String _GetMessage() const override;
        virtual PlanGridPlanarSurfaceCP _GetGridSurfaceCP() override { return m_surface.get(); }
        virtual PlanGridPlanarSurfaceP _GetGridSurfaceP() override { return m_surface.get(); }
        virtual BBS::CurvePrimitiveManipulationStrategyCR _GetCurvePrimitiveManipulationStrategy() const override { return *m_geometryManipulationStrategy; }
        virtual BBS::CurvePrimitiveManipulationStrategyR _GetCurvePrimitiveManipulationStrategyForEdit() override { return *m_geometryManipulationStrategy; }

    public:
        GRIDSTRATEGIES_EXPORT static LineGridSurfaceManipulationStrategyPtr Create() { return new LineGridSurfaceManipulationStrategy(); }
        BBS::LinePlacementStrategyPtr CreateLinePlacementStrategy(BBS::LinePlacementStrategyType linePlacementStrategyType) {return BBS::LinePlacementStrategy::Create(linePlacementStrategyType, *m_geometryManipulationStrategy); }
    };

END_GRIDS_NAMESPACE