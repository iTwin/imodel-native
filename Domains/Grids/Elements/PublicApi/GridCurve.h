/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#pragma once

BEGIN_GRIDS_NAMESPACE

//=======================================================================================
//! Physical building element
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE GridCurve : Dgn::SpatialLocationElement
{
    DEFINE_T_SUPER(Dgn::SpatialLocationElement);

protected:
    explicit GRIDELEMENTS_EXPORT GridCurve (CreateParams const& params);
    explicit GRIDELEMENTS_EXPORT GridCurve (CreateParams const& params, ICurvePrimitivePtr curve);
    explicit GRIDELEMENTS_EXPORT GridCurve (CreateParams const& params, CurveVectorPtr curve);

    GRIDELEMENTS_EXPORT void                InitGeometry (ICurvePrimitivePtr curve);
    GRIDELEMENTS_EXPORT void                InitGeometry (CurveVectorPtr curve);

    virtual GRIDELEMENTS_EXPORT void                _CopyFrom (Dgn::DgnElementCR source) override;
    static  GRIDELEMENTS_EXPORT Dgn::GeometricElement3d::CreateParams        CreateParamsFromModel (Dgn::DgnModelCR model, Dgn::DgnClassId classId);

    virtual bool            _ValidateGeometry(ICurvePrimitivePtr curve) const { return true; };

    GRIDELEMENTS_EXPORT Dgn::DgnDbStatus CheckDependancyToModel() const;

    //! Called when an element is about to be inserted into the DgnDb.
    //! @return DgnDbStatus::Success to allow the insert, otherwise it will fail with the returned status.
    //! @note If you override this method, you @em must call T_Super::_OnInsert, forwarding its status.
    GRIDELEMENTS_EXPORT virtual Dgn::DgnDbStatus _OnInsert() override;

    //! Called when this element is about to be replace its original element in the DgnDb.
    //! @param [in] original the original state of this element.
    //! Subclasses may override this method to control whether their instances are updated.
    //! @return DgnDbStatus::Success to allow the update, otherwise the update will fail with the returned status.
    //! @note If you override this method, you @em must call T_Super::_OnUpdate, forwarding its status.
    GRIDELEMENTS_EXPORT virtual Dgn::DgnDbStatus _OnUpdate(Dgn::DgnElementCR original) override;

    //! Returns first intersecting `GridSurface` that is not an `ElevationGridSurface`
    //! @return first non-`ElevationGridSurface` used to create this `GridCurve`
    GRIDELEMENTS_EXPORT GridSurfaceCPtr GetFirstNonElevationIntersectingGridSurface() const;

public:
    DECLARE_GRIDS_ELEMENT_BASE_METHODS (GridCurve, GRIDELEMENTS_EXPORT)

    //---------------------------------------------------------------------------------------
    // Getters and setters
    //---------------------------------------------------------------------------------------
    //! gets curve geometry of the gridcurve
    //! @return             GridCurve geometry
    GRIDELEMENTS_EXPORT ICurvePrimitivePtr      GetCurve () const;

    //! gets the ElementIds of intersecting GridSurfaces which creates this GridCurve
    //! @return a list of ElementIds of intersecting GridSurfaces
    GRIDELEMENTS_EXPORT bvector<Dgn::DgnElementId> GetIntersectingSurfaceIds() const;

    //! Returns `GridLabel` of first non elevation `GridSurface` used to create this `GridCurve`
    //! @return `GridLabel` for this `GridCurve`. Nullptr if `GridCurve` was created from intersecting two `ElevationGridSurface`s.
    //!         If `GridCurve` has been created from two non-`ElevationGridSurface`s, returns `GridLabel` of first surface in IntersectingSurfaces list.
    GRIDELEMENTS_EXPORT GridLabelCPtr GetNonElevationSurfaceGridLabel() const;

    //---------------------------------------------------------------------------------------
    // Geometry modification
    //---------------------------------------------------------------------------------------
    //! sets curve geometry of the gridcurve
    //! @param[in]  curve   model for the gridcurve
    GRIDELEMENTS_EXPORT void    SetCurve (CurveVectorPtr curve);

    //! sets curve geometry of the gridcurve
    //! @param[in]  curve   model for the gridcurve
    GRIDELEMENTS_EXPORT void    SetCurve (ICurvePrimitivePtr curve);    
};


//=======================================================================================
//! Physical building element
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE GeneralGridCurve : GridCurve
    {
    DGNELEMENT_DECLARE_MEMBERS(GRIDS_CLASS_GeneralGridCurve, GridCurve);

    protected:
        explicit GRIDELEMENTS_EXPORT GeneralGridCurve(CreateParams const& params);
        explicit GRIDELEMENTS_EXPORT GeneralGridCurve(CreateParams const& params, ICurvePrimitivePtr curve);
        friend struct GeneralGridCurveHandler;

    public:
        DECLARE_GRIDS_ELEMENT_BASE_METHODS(GeneralGridCurve, GRIDELEMENTS_EXPORT)

        //---------------------------------------------------------------------------------------
        // Creation
        //---------------------------------------------------------------------------------------
        //! Creates a grid curve
        //! @param[in]  model   model for the GeneralGridCurve
        //! @param[in]  curve   curve geometry
        //! @return             Grid curve
        GRIDELEMENTS_EXPORT static GeneralGridCurvePtr Create(Dgn::DgnModelCR model, ICurvePrimitivePtr curve);
    };

END_GRIDS_NAMESPACE