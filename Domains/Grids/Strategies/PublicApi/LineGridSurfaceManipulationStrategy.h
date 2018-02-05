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
        BBS::LinePlacementStrategyType m_currentPlacementType;
        BBS::LineManipulationStrategyPtr m_geometryManipulationStrategy;
        SketchLineGridSurfacePtr m_surface;

    protected:
        LineGridSurfaceManipulationStrategy(BBS::LinePlacementStrategyType linePlacementStrategyType);

        // ElementManipulationStrategy
        virtual Dgn::DgnElementPtr _FinishElement(Dgn::DgnModelR model) override;
        virtual BBS::GeometryManipulationStrategyCR _GetGeometryManipulationStrategy() const override { return *m_geometryManipulationStrategy; }
        virtual BBS::GeometryManipulationStrategyR _GetGeometryManipulationStrategyR() override { return *m_geometryManipulationStrategy; }

        // SketchGridSurfaceManipulationStrategy
        virtual BentleyStatus _UpdateGridSurface() override;
        virtual Utf8String _GetMessage() const override;
        virtual PlanGridPlanarSurfaceCP _GetGridSurfaceCP() override { return m_surface.get(); }
        virtual PlanGridPlanarSurfaceP _GetGridSurfaceP() override { return m_surface.get(); }
        virtual BBS::CurvePrimitivePlacementStrategyPtr _GetGeometryPlacementStrategyP() override;
        virtual BBS::CurvePrimitivePlacementStrategyCPtr _GetGeometryPlacementStrategy() const override;

    public:
        GRIDSTRATEGIES_EXPORT static LineGridSurfaceManipulationStrategyPtr Create(BBS::LinePlacementStrategyType linePlacementStrategyType) { return new LineGridSurfaceManipulationStrategy(linePlacementStrategyType); }
        GRIDSTRATEGIES_EXPORT void ChangeCurrentPlacementType(BBS::LinePlacementStrategyType newLinePlacementStrategyType);
    };

END_GRIDS_NAMESPACE