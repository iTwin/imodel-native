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

public:
    struct CreateParams : T_Super::CreateParams
        {
        DEFINE_T_SUPER (Grid::T_Super::CreateParams);

        //! Creates create parameters for orthogonal grid
        //! @param[in] model                        model to create the grid in
        //! @param[in] horizontalCount              horizontal lines count
        //! @param[in] verticalCount                vertical lines count
        //! @param[in] horizontalInterval           distance between horizontal lines
        //! @param[in] verticalInterval             distance between vertical lines
        //! @param[in] length                       length of grid lines
        //! @param[in] height                       height of grid lines
        //! @param[in] horizontalExtendTranslation  translation of horizontal lines to be extended
        //! @param[in] verticalExtendTranslation    translation of vertical lines to be extended
        //! @param[in] createDimensions             true to create dimensions between planes
        //! @param[in] extendHeight                 true if grid should be extended to both ends in Z axis
        CreateParams (Dgn::SpatialLocationModelCP model, Dgn::DgnElementId modeledElementId, Utf8CP name, Dgn::DgnClassId classId) :
            T_Super (DgnElement::CreateParams (model->GetDgnDb (), model->GetModelId (), classId, Dgn::DgnCode (model->GetDgnDb ().CodeSpecs ().QueryCodeSpecId (GRIDS_AUTHORITY_Grid), modeledElementId, name)))
            {}

        //! Constructor from base params. Chiefly for internal use.
        //! @param[in]      params   The base element parameters
        //! @return 
        explicit GRIDELEMENTS_EXPORT CreateParams (Dgn::DgnElement::CreateParams const& params)
            : T_Super (params)
            {}
        };

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
    
    BE_PROP_NAME (DefaultStartElevation)
    BE_PROP_NAME (DefaultEndElevation)
protected:
    //! creates the Grid.. !!!DEFAULT parameters makes the gridportion INVALID!!! elements should not be constructed via handler
    //! @param[in]          params  params for creation
    explicit GRIDELEMENTS_EXPORT PlanGrid (T_Super::CreateParams const& params) : T_Super (params) {};
public:
    DECLARE_GRIDS_ELEMENT_BASE_METHODS (PlanGrid, GRIDELEMENTS_EXPORT)

    //! gets the perpendicularity plane of this Grid
    //! @return             perpendicularity plane of this Grid
    GRIDELEMENTS_EXPORT DPlane3d    GetPlane() const;

    //! Gets Default start elevation of this PlanGrid
    //! @return DefaultStartElevation of this PlanGrid
    GRIDELEMENTS_EXPORT double      GetDefaultStartElevation() const { return GetPropertyValueDouble(prop_DefaultStartElevation()); }

    //! Sets Default start elevation of this PlanGrid
    //! @param[in]  staElevation   new DefaultStartElevation for this PlanGrid
    GRIDELEMENTS_EXPORT void        SetDefaultStartElevation(double staElevation) { SetPropertyValue(prop_DefaultStartElevation(), staElevation); };
    
    //! Gets Default end elevation of this PlanGrid
    //! @return DefaultEndElevation of this PlanGrid
    GRIDELEMENTS_EXPORT double      GetDefaultEndElevation() const { return GetPropertyValueDouble(prop_DefaultEndElevation()); }

    //! Sets Default end elevation of this PlanGrid
    //! @param[in]  endElevation   new DefaultEndElevation for this PlanGrid
    GRIDELEMENTS_EXPORT void        SetDefaultEndElevation(double endElevation) { SetPropertyValue(prop_DefaultEndElevation(), endElevation); };
    };

//=======================================================================================
//! Physical building element
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE ElevationGrid : Grid
    {
    DGNELEMENT_DECLARE_MEMBERS (GRIDS_CLASS_ElevationGrid, Grid);
    DEFINE_T_SUPER (Grid);
    public:
        struct CreateParams : T_Super::CreateParams
            {
            DEFINE_T_SUPER (ElevationGrid::T_Super::CreateParams);

            //! Creates create parameters for orthogonal grid
            //! @param[in] model                        model to create the grid in
            //! @param[in] horizontalCount              horizontal lines count
            //! @param[in] verticalCount                vertical lines count
            //! @param[in] horizontalInterval           distance between horizontal lines
            //! @param[in] verticalInterval             distance between vertical lines
            //! @param[in] length                       length of grid lines
            //! @param[in] height                       height of grid lines
            //! @param[in] horizontalExtendTranslation  translation of horizontal lines to be extended
            //! @param[in] verticalExtendTranslation    translation of vertical lines to be extended
            //! @param[in] createDimensions             true to create dimensions between planes
            //! @param[in] extendHeight                 true if grid should be extended to both ends in Z axis
            CreateParams (Dgn::SpatialLocationModelCP model, Dgn::DgnElementId modeledElementId, Utf8CP name) :
                T_Super (model, modeledElementId, name, QueryClassId (model->GetDgnDb ()))
                {}

            //! Constructor from base params. Chiefly for internal use.
            //! @param[in]      params   The base element parameters
            //! @return 
            explicit GRIDELEMENTS_EXPORT CreateParams (Dgn::DgnElement::CreateParams const& params)
                : T_Super (params)
                {}
            };

    private:

        BE_PROP_NAME(DefaultElevationIncrement)
        BE_PROP_NAME(DefaultSurface2d)
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

        //! Gets Default elevation increment of this ElevationGrid
        //! @return DefaultElevationIncrement of this ElevationGrid
        GRIDELEMENTS_EXPORT double      GetDefaultElevationIncrement() const { return GetPropertyValueDouble(prop_DefaultElevationIncrement()); }

        //! Sets Default elevation increment of this ElevationGrid
        //! @param[in]  elevationInc   new DefaultElevationIncrement for this ElevationGrid
        GRIDELEMENTS_EXPORT void        SetDefaultElevationIncrement(double elevationInc) { SetPropertyValue(prop_DefaultElevationIncrement(), elevationInc); };

        //! Gets default Surface2d of this ElevationGrid
        //! @return surface of this ElevationGrid, as curvevector on zero Z plane
        GRIDELEMENTS_EXPORT CurveVectorPtr      GetDefaultSurface2d() const;

        //! Sets default Surface2d of this ElevationGrid
        //! @param[in]   surface        curvevector in local coordinates, on zero Z plane
        GRIDELEMENTS_EXPORT void                SetDefaultSurface2d(CurveVectorPtr surface);
    };
END_GRIDS_NAMESPACE
