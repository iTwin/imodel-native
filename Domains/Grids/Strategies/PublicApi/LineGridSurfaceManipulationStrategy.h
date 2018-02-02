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

    protected:
        LineGridSurfaceManipulationStrategy(BBS::LinePlacementStrategyType linePlacementStrategyType);

        // GeometryManipulationStrategyBase
        virtual BBS::GeometryManipulationStrategyCR _GetGeometryManipulationStrategy() const override { return *m_geometryManipulationStrategy; }
        virtual BBS::GeometryManipulationStrategyR _GetGeometryManipulationStrategyR() override { return *m_geometryManipulationStrategy; }
        virtual bool _IsComplete() const override;

        virtual bool _CanAcceptMorePoints() const override;
        virtual void _SetProperty(Utf8CP key, double const & value) override;
        virtual BentleyStatus _TryGetProperty(Utf8CP key, double & value) const override;
        
        // ElementManipulationStrategy
        virtual Dgn::DgnElementPtr _FinishElement(Dgn::DgnModelR model) override;

        // SketchGridSurfaceManipulationStrategy
        virtual BentleyStatus _UpdateGridSurface() override;
        virtual Utf8String _GetMessage() const override;

        // LineGridSurfaceManipulationStrategy
        virtual BBS::CurvePrimitivePlacementStrategyPtr _GetStrategyForAppend() override;
        
    public:
        GRIDSTRATEGIES_EXPORT static LineGridSurfaceManipulationStrategyPtr Create(BBS::LinePlacementStrategyType linePlacementStrategyType) { return new LineGridSurfaceManipulationStrategy(linePlacementStrategyType); }
        GRIDSTRATEGIES_EXPORT void ChangeCurrentPlacementType(BBS::LinePlacementStrategyType newLinePlacementStrategyType);
    };

END_GRIDS_NAMESPACE