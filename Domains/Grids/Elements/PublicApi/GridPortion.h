/*--------------------------------------------------------------------------------------+
|
|     $Source: Grids/Elements/PublicApi/GridPortion.h $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

#include <Bentley/Heapzone.h>
#include <DgnPlatform/Render.h>
#include <DgnPlatform/ClipPrimitive.h>
#include <DgnPlatform/DgnElement.h>
#include <Grids/gridsApi.h>

BEGIN_GRIDS_NAMESPACE

#define GRIDLINE_LENGTH_COEFF 0.1

//=======================================================================================
//! Physical building element
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE Grid : Dgn::SpatialLocationPortion
{
    DEFINE_T_SUPER (Dgn::SpatialLocationPortion);
private:

    Dgn::SpatialLocationModelPtr    CreateSubModel () const;

protected:

    //! creates the Grid.. !!!DEFAULT parameters makes the gridportion INVALID!!! elements should not be constructed via handler
    //! @param[in]          params  params for creation
    explicit GRIDELEMENTS_EXPORT Grid (T_Super::CreateParams const& params);

    static  GRIDELEMENTS_EXPORT CreateParams        CreateParamsFromModel (Dgn::DgnModelCR model, Dgn::DgnClassId classId);

    //! Called when an element is about to be inserted or updated in the DgnDb.
    //! @return DgnDbStatus::Success to allow the operation, otherwise it will fail with the returned status.
    //! @note If you override this method, you @em must call T_Super::_Validate, forwarding its status.
    GRIDELEMENTS_EXPORT virtual Dgn::DgnDbStatus _Validate () const;

    //! Called when an element is about to be inserted into the DgnDb.
    //! @return DgnDbStatus::Success to allow the insert, otherwise it will fail with the returned status.
    //! @note If you override this method, you @em must call T_Super::_OnInsert, forwarding its status.
    GRIDELEMENTS_EXPORT virtual Dgn::DgnDbStatus _OnInsert () override;

    //! Called when this element is about to be replace its original element in the DgnDb.
    //! @param [in] original the original state of this element.
    //! Subclasses may override this method to control whether their instances are updated.
    //! @return DgnDbStatus::Success to allow the update, otherwise the update will fail with the returned status.
    //! @note If you override this method, you @em must call T_Super::_OnUpdate, forwarding its status.
    GRIDELEMENTS_EXPORT virtual Dgn::DgnDbStatus _OnUpdate (Dgn::DgnElementCR original) override;

    GRIDELEMENTS_EXPORT virtual Dgn::DgnDbStatus _OnDelete() const override;
public:
    DECLARE_GRIDS_ELEMENT_BASE_METHODS (Grid, GRIDELEMENTS_EXPORT)

    //---------------------------------------------------------------------------------------
    // Getters and setters
    //---------------------------------------------------------------------------------------
    //! gets the surfaces model
    //! @return                 the surfaces model
    GRIDELEMENTS_EXPORT Dgn::SpatialLocationModelPtr    GetSurfacesModel() const;

    //! Returns an angle the grid elements have rotated to
    //! @param[out] angle   angle that the grid elements have rotated to 
    //! @return BentleyStatus::ERROR if grid has no elements
    GRIDELEMENTS_EXPORT BentleyStatus GetGridRotationAngleXY(double& angle) const;

    //! Returns name of this grid
    //! @return name of model this grid is in
    GRIDELEMENTS_EXPORT Utf8CP  GetName() const;

    //---------------------------------------------------------------------------------------
    // Geometry modification
    //---------------------------------------------------------------------------------------
    //! Translates grid to given point, 
    //! @param[in] point    point to translate to
    //! @returuns           Dgn::RepositoryStatus::Success if no error has occured when translating grid
    GRIDELEMENTS_EXPORT Dgn::RepositoryStatus TranslateToPoint(DPoint3d point);

    //! Rotates grid to given angle
    //! @param[in] theta    angle to rotate to
    //! @returns            Dgn::RepositoryStatus::Success if no error has occured when rotating grid
    GRIDELEMENTS_EXPORT Dgn::RepositoryStatus RotateToAngleXY(double theta);

    //---------------------------------------------------------------------------------------
    // Queries
    //---------------------------------------------------------------------------------------
    //! Make an iterator over gridSurfaces that compose this Grid
    //! @returns            an iterator over grid surfaces this grid portion owns
    GRIDELEMENTS_EXPORT Dgn::ElementIterator MakeIterator() const;

    //! Make an iterator over gridAxis that compose this Grid
    //! @returns            an iterator over axes this grid portion owns
    GRIDELEMENTS_EXPORT Dgn::ElementIterator MakeAxesIterator() const;

    //! Returns a grid portion with given parent element and name
    //! @param[in]  db          db to try get grid portion from
    //! @param[in]  parentId    id of parent of grid portion
    //! @param[in]  gridName    name of grid portion
    //! @return                 if grid portion has been found, returns a ptr to it
    //!                         otherwise a nullptr is returned
    GRIDELEMENTS_EXPORT static GridPtr TryGet(Dgn::DgnDbR db, Dgn::DgnElementId parentId, Utf8CP gridName);

    //---------------------------------------------------------------------------------------
    // Other
    //---------------------------------------------------------------------------------------
    //! intersects this grid with the given surface, creates gridcurves
    //! @param[in] surface      surface to intersect
    //! @param[in] targetModel  model to create gridcurves in
    //! @return BentleyStatus for the operation result
    GRIDELEMENTS_EXPORT BentleyStatus   IntersectGridSurface(GridSurfaceCPtr surface, Dgn::DgnModelCR targetModel) const;
    };
    
//=======================================================================================
//! Physical building element
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE PlanGrid : Grid
{
    DEFINE_T_SUPER (Grid);
private:

protected:


    //! gets the perpendicularity plane of this Grid
    //! @return             perpendicularity plane of this Grid
    GRIDELEMENTS_EXPORT DPlane3d    GetPlane () const;

    //! creates the Grid.. !!!DEFAULT parameters makes the gridportion INVALID!!! elements should not be constructed via handler
    //! @param[in]          params  params for creation
    explicit GRIDELEMENTS_EXPORT PlanGrid (T_Super::CreateParams const& params) : T_Super (params) {};
public:
    DECLARE_GRIDS_ELEMENT_BASE_METHODS (PlanGrid, GRIDELEMENTS_EXPORT)

    };

//=======================================================================================
//! Physical building element
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE ElevationGrid : Grid
    {
    DEFINE_T_SUPER (Grid);
    private:

    protected:

        static  BentleyStatus           ValidateSurfaces (bvector<CurveVectorPtr> const& surfaces);

                BentleyStatus           CreateElevationGridPlanes (bvector<CurveVectorPtr> const& surfaces, GridAxisCR gridAxis, bool createDimensions);
        //! creates the Grid.. !!!DEFAULT parameters makes the gridportion INVALID!!! elements should not be constructed via handler
        //! @param[in]          params  params for creation
        explicit GRIDELEMENTS_EXPORT ElevationGrid (T_Super::CreateParams const& params) : T_Super (params) {};
    public:
        DECLARE_GRIDS_ELEMENT_BASE_METHODS (ElevationGrid, GRIDELEMENTS_EXPORT)
        friend struct ElevationGridHandler;

        //---------------------------------------------------------------------------------------
        // Creation
        //---------------------------------------------------------------------------------------
        //! Creates orthogonal grid
        //! @param[in]  params          grid parameters containing information about the grid. For more info look up CreateParams
        //! @param[in]  surfaces        gridsurface geometry
        //! @return                     grid with gridsurfaces in submodel
        //! @note all surfaces will be tested to be planar and normal needs to be in Z direction
        GRIDELEMENTS_EXPORT static ElevationGridPtr CreateAndInsertWithSurfaces (CreateParams const& params, bvector<CurveVectorPtr> const& surfaces, bool createDimensions);

        GRIDELEMENTS_EXPORT static ElevationGridPtr Create (CreateParams const& params);

    };
END_GRIDS_NAMESPACE
