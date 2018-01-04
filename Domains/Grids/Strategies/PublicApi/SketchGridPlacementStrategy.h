/*--------------------------------------------------------------------------------------+
|
|     $Source: Grids/Strategies/PublicApi/SketchGridPlacementStrategy.h $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

GRIDS_REFCOUNTED_PTR_AND_TYPEDEFS(ISketchGridPlacementStrategy)
GRIDS_REFCOUNTED_PTR_AND_TYPEDEFS(SketchGridPlacementStrategy)

BEGIN_GRIDS_NAMESPACE

struct EXPORT_VTABLE_ATTRIBUTE ISketchGridPlacementStrategy : public RefCountedBase
    {
    protected:
        // Called to create the initial grid surface
        virtual GridSurfacePtr  _CreateAndInsertGridSurface() = 0;

        // Called to update current grid surface
        virtual BentleyStatus   _UpdateGridSurface() = 0;

        // Called to update current grid surface's key points
        virtual BentleyStatus   _UpdateByKeyPoints() = 0;

        // Called to retreive current grid surface
        virtual Grids::GridSurfacePtr  _GetGridSurface() = 0;

        // Called to set current grid surface
        virtual void _SetGridSurface(Grids::GridSurfacePtr) = 0;

        // Called when adding new point
        virtual size_t  _GetMaxPointCount() = 0;
    };


struct EXPORT_VTABLE_ATTRIBUTE SketchGridPlacementStrategy : ISketchGridPlacementStrategy
    {
    private:
        Dgn::DgnDbR m_db;
        double m_elevation;
        double m_height;
        
        bool m_inDynamics = false;

        Grids::SketchGridPtr m_grid;
        Grids::GridAxisPtr m_axis;
       
        Grids::GridAxisPtr      _CreateAndInsertNewAxis();

    protected:
        bvector<DPoint3d> m_points;
        DPoint3d m_dynamicPoint;
        bool m_useDynamicPoint = false;

        explicit SketchGridPlacementStrategy(Dgn::DgnDbR db, double elevation, double height, Grids::SketchGridPtr grid) :
            m_db(db),
            m_elevation(elevation),
            m_height(height),
            m_grid(grid)
            { 
            BeAssert(m_grid.IsValid() && "Should not procceed with unitialized grid");
            };

        void            _AcceptDynamicPoint() { m_points.push_back(m_dynamicPoint); };
        virtual void            _SetDynamicPoint(DPoint3d point) { m_dynamicPoint = point; }
        void            _BeginDynamics() { m_inDynamics = true; }
        void            _EndDynamics() { m_inDynamics = false; }

    public:
        //! Returns db this strategy is working in
        //! @return a reference to db that this strategy is working in
        GRIDSTRATEGIES_EXPORT Dgn::DgnDbR         GetDb() const           { return m_db; }

        //! Returns elevation of grid surface that is about to or is being created
        //! @return elevation of current grid surface
        GRIDSTRATEGIES_EXPORT double              GetElevation() const    { return m_elevation; }
        
        //! Returns height of grid surface that is about to or is being created
        //! @return height of current grid surface
        GRIDSTRATEGIES_EXPORT double              GetHeight() const       { return m_height; }
        
        //! Returns name of grid in which grid surfaces are being created
        //! @return grid name
        GRIDSTRATEGIES_EXPORT Utf8String          GetName() const         { return m_grid->GetName(); }
        
        //! Returns grid in which grid surfaces are being created
        //! @return grid
        GRIDSTRATEGIES_EXPORT Grids::SketchGridPtr      GetGrid() const         { return m_grid; }
        
        //! Returns axis in which grid surfaces are being created
        //! @return axis
        GRIDSTRATEGIES_EXPORT Grids::GridAxisPtr  GetAxis() const         { return m_axis; }

        //! If in dynamics mode, returns current grid surface
        //! Else begins dynamics and creates initial grid surface by current parameters
        GRIDSTRATEGIES_EXPORT Grids::GridSurfacePtr GetGridSurface();

        //! Checks if strategy is currently in dynamics mode.
        //! In dynamics the dynamic point will appear on created grid surface 
        //! but could in some cases be discarded after dynamics have ended.
        //! Additionally, during dynamics mode any changes to grid surface 
        //! (like changing axis, height, elevation, etc.) will update existing grid
        //! surface rather than creating a new one.
        //! @return true if strategy is in dynamics mode.
        GRIDSTRATEGIES_EXPORT bool                IsInDynamics() const    { return m_inDynamics; }

        //! Ends dynamic mode for strategy.
        //! After ending dynamics, recreates grid surface with final parameters
        //! @returns    final grid surface. In case of failure doesn't end dynamics and returns the old grid surface.
        GRIDSTRATEGIES_EXPORT Grids::GridSurfacePtr Finish();

        //! Updates current grid surface's elevation and height
        //! Additionally in dynamics mode updates existing grid surface to match given parameters
        //! @param[in]  elevation   new elevation for grid surface
        //! @return                 updated grid surface
        GRIDSTRATEGIES_EXPORT Grids::GridSurfacePtr SetBottomElevation(double elevation);
        
        //! Updates current grid surface's elevation and height
        //! Additionally in dynamics mode updates existing grid surface to match given parameters
        //! @param[in]  elevation   new elevation for grid surface
        //! @param[in]  height      new height for grid surface
        //! @return                 updated grid surface
        GRIDSTRATEGIES_EXPORT Grids::GridSurfacePtr SetTopElevation(double elevation);

        //! Updates current grid's axis
        //! @param[in]  axis    new axis for grid
        //! @return             updated grid surface
        GRIDSTRATEGIES_EXPORT Grids::GridSurfacePtr SetAxis(Grids::GridAxisPtr axis);

        //! Creates and sets new axis for grid
        //! @return created axis
        GRIDSTRATEGIES_EXPORT Grids::GridAxisPtr CreateAndInsertNewAxis() { return _CreateAndInsertNewAxis(); }

        //! Updates current grid and axis
        //! @param[in]  grid    new grid
        //! @param[in]  axis    new axis for grid. If not passed, will create a new axis
        //! @return             updated grid surface
        GRIDSTRATEGIES_EXPORT Grids::GridSurfacePtr SetGridAndAxis(Grids::SketchGridPtr grid, Grids::GridAxisPtr axis = nullptr);

        //! Adds dynamic point to point pool. 
        //! @return             updated grid surface
        GRIDSTRATEGIES_EXPORT Grids::GridSurfacePtr AcceptDynamicPoint();

        //! Updates dynamic point
        //! @param[in] point    new dynamic point
        //! @return             updated grid surface
        GRIDSTRATEGIES_EXPORT Grids::GridSurfacePtr SetDynamicPoint(DPoint3d point);
    };

END_GRIDS_NAMESPACE