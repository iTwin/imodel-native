/*--------------------------------------------------------------------------------------+
|
|     $Source: Grids/Strategies/PublicApi/ArcGridPlacementStrategy.h $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

GRIDS_REFCOUNTED_PTR_AND_TYPEDEFS(CSEArcGridPlacementStrategy)

BEGIN_GRIDS_NAMESPACE

// For placing arc with Center-Start-End method
struct CSEArcGridPlacementStrategy : SketchGridPlacementStrategy
    {
    DEFINE_T_SUPER(SketchGridPlacementStrategy);

    private:
        Grids::GridArcSurfacePtr m_surface;
        bool m_ccw = true;

        BentleyStatus GetArcPoints(DPoint3dR center, DPoint3dR start, DPoint3dR end);

    protected:
        CSEArcGridPlacementStrategy(Dgn::DgnDbR db, double elevation, double height, Grids::SketchGridPtr grid) :
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

        // Called when dynamic point is being set
        virtual void _SetDynamicPoint(DPoint3d point) override;

    public:
        static GRIDSTRATEGIES_EXPORT CSEArcGridPlacementStrategyPtr Create(Dgn::DgnDbR db, double elevation, double height, Grids::SketchGridPtr grid);
    };

END_GRIDS_NAMESPACE