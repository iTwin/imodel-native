/*--------------------------------------------------------------------------------------+
|
|     $Source: Grids/Strategies/PublicApi/GridPlanarSurfaceManipulationStrategy.h $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

BEGIN_GRIDS_NAMESPACE

//=======================================================================================
// @bsiclass                                     Mindaugas.Butkus               02/2018
//=======================================================================================
struct GridPlanarSurfaceManipulationStrategy : public SketchGridSurfaceManipulationStrategy
    {
    DEFINE_T_SUPER(SketchGridSurfaceManipulationStrategy)

    private:

    protected:
        GridPlanarSurfaceManipulationStrategy(Dgn::DgnDbR db) 
            : T_Super(db) 
            {}

        virtual PlanGridPlanarSurfaceCP _GetPlanGridPlanarSurfaceCP() const = 0;
        virtual PlanGridPlanarSurfaceP _GetPlanGridPlanarSurfaceP() const = 0;
        virtual IPlanGridSurface const* _GetPlanGridSurfaceCP() const override { return _GetPlanGridPlanarSurfaceCP(); }
        virtual IPlanGridSurface* _GetPlanGridSurfaceP() const override { return _GetPlanGridPlanarSurfaceP(); }

    public:

    };

END_GRIDS_NAMESPACE