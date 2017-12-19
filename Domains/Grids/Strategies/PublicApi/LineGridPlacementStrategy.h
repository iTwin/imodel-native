/*--------------------------------------------------------------------------------------+
|
|     $Source: Grids/Strategies/PublicApi/LineGridPlacementStrategy.h $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

GRIDS_REFCOUNTED_PTR_AND_TYPEDEFS(LineGridPlacementStrategy)

BEGIN_GRIDS_NAMESPACE

struct LineGridPlacementStrategy : SketchGridPlacementStrategy
    {
    DEFINE_T_SUPER(SketchGridPlacementStrategy);

    private:
        Grids::GridPlanarSurfacePtr m_surface;

        BentleyStatus GetEndPoints(DPoint3dR start, DPoint3dR end);

    protected:
        LineGridPlacementStrategy(Dgn::DgnDbR db, double elevation, double height, Grids::SketchGridPtr grid) :
            T_Super(db, elevation, height, grid)
            {
            };

        // Called to create the initial grid surface
        virtual Grids::GridSurfacePtr  _CreateAndInsertGridSurface() override;

        // Called to update current grid surface
        virtual BentleyStatus   _UpdateGridSurface() override;

        // Called to update current grid surface's key points
        virtual BentleyStatus   _UpdateByKeyPoints() override;

        // Called to retreive current grid surface
        virtual Grids::GridSurfacePtr  _GetGridSurface() override;

        // Called to set current grid surface
        virtual void _SetGridSurface(Grids::GridSurfacePtr) override;

        // Called when adding new point
        virtual size_t  _GetMaxPointCount() override;

    public:
        static GRIDSTRATEGIES_EXPORT LineGridPlacementStrategyPtr Create(Dgn::DgnDbR db, double elevation, double height, Grids::SketchGridPtr grid);
    };

END_GRIDS_NAMESPACE