/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once

BEGIN_GRIDS_NAMESPACE

//=======================================================================================
//! An abstract grid surface element
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE GridSurface : Dgn::SpatialLocationElement
{
    DEFINE_T_SUPER (Dgn::SpatialLocationElement);

public:
    struct CreateParams : T_Super::CreateParams
        {
        DEFINE_T_SUPER (GridSurface::T_Super::CreateParams);
        Dgn::DgnElementId m_gridAxisId;

        //! Creates create parameters for orthogonal grid
        //! @param[in] model              model for the PlanCartesianGridSurface
        CreateParams (Dgn::SpatialLocationModelCR model, Dgn::DgnClassId classId, Dgn::DgnElementId gridAxisId) :
            T_Super::CreateParams (model.GetDgnDb (), model.GetModelId (), classId, Dgn::SpatialCategory::QueryCategoryId (model.GetDgnDb ().GetDictionaryModel (), GRIDS_CATEGORY_CODE_GridSurface)), m_gridAxisId (gridAxisId)
            {
            }

        //! Constructor from base params. Chiefly for internal use.
        //! @param[in]      params   The base element parameters
        //! @return 
        explicit CreateParams (Dgn::DgnElement::CreateParams const& params)
            : T_Super (params), m_gridAxisId (Dgn::DgnElementId())
            {
            }
        };
protected:
    explicit GRIDELEMENTS_EXPORT GridSurface (CreateParams const& params);
    explicit GRIDELEMENTS_EXPORT GridSurface (CreateParams const& params, CurveVectorPtr surfaceVector);
    explicit GRIDELEMENTS_EXPORT GridSurface (CreateParams const& params, ISolidPrimitivePtr surface);

    BE_PROP_NAME(Axis)
        
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

    GRIDELEMENTS_EXPORT virtual void _OnInserted(Dgn::DgnElementP copiedFrom) const override;

    //! Rotates grid by given angle in radians
    //! @param[in] theta            angle in radians
    GRIDELEMENTS_EXPORT virtual BentleyStatus _RotateXY(double theta);

    //! Rotates grid around point by given angle in radians
    //! @param[in] point            point to rotate around
    //! @param[in] theta            angle in radians
    GRIDELEMENTS_EXPORT virtual BentleyStatus _RotateXY(DPoint3d point, double theta);

    //! Translates gridsurface by given vector
    //! @param[in] translation   vector to translate by
    GRIDELEMENTS_EXPORT virtual BentleyStatus _TranslateXY(DVec2d translation);

    //! Translates gridsurface by given vector
    //! @param[in] translation   vector to translate by
    GRIDELEMENTS_EXPORT virtual BentleyStatus _Translate(DVec3d translation);

    virtual BentleyStatus   _SetGeometry(ISolidPrimitivePtr surface);
    virtual bool            _ValidateGeometry(ISolidPrimitivePtr surface) const { return false; };
    
    static GRIDELEMENTS_EXPORT CreateParams        CreateParamsFromModelAxisClassId (Dgn::SpatialLocationModelCR model, GridAxisCR axis, Dgn::DgnClassId classId);

protected:

    //! Sets geometry for this grid surface
    //! @param[in]  surface     new geometry for the surface
    //! @returns BentleyStatus::SUCCESS if geometry is valid for the surface and there has been no error in setting surface geometry
    //! For specifications of geometry validation see derived grid surface elements' creation documentation
    GRIDELEMENTS_EXPORT BentleyStatus SetGeometry(ISolidPrimitivePtr surface);

    //! Changes grid surface's base curve. For specifications of base curve see derived elements' creation documentation
    //! @param[in] base         new base curve
    //! @return BentleyStatus::SUCCESS if there was no error in changing grid surface's base curve
    GRIDELEMENTS_EXPORT BentleyStatus SetBaseCurve(CurveVectorPtr base);

public:
    DECLARE_GRIDS_ELEMENT_BASE_METHODS (GridSurface, GRIDELEMENTS_EXPORT)

    //---------------------------------------------------------------------------------------
    // Geometry modification
    //---------------------------------------------------------------------------------------
    //! Rotates grid by given angle in radians
    //! @param[in] theta            angle in radians
    GRIDELEMENTS_EXPORT BentleyStatus RotateXY(double theta) { return _RotateXY(theta); };

    //! Rotates grid around point by given angle in radians
    //! @param[in] point            point to rotate around
    //! @param[in] theta            angle in radians
    GRIDELEMENTS_EXPORT BentleyStatus RotateXY(DPoint3d point, double theta) { return _RotateXY(point, theta); };

    //! Translates gridsurface by given vector
    //! @param[in] translation   vector to translate by
    GRIDELEMENTS_EXPORT BentleyStatus TranslateXY(DVec2d translation) { return _TranslateXY(translation); };

    //! Translates gridsurface by given vector
    //! @param[in] translation   vector to translate by
    GRIDELEMENTS_EXPORT BentleyStatus Translate(DVec3d translation) { return _Translate(translation); };

    //---------------------------------------------------------------------------------------
    // Setters and getters
    //---------------------------------------------------------------------------------------
    //! Returns id of grid that has this surface
    //! @return grid id of the surface
    GRIDELEMENTS_EXPORT Dgn::DgnElementId GetGridId () const;

    //!Returns id of axis that has this surface
    //! @return axis id of the surface
    GRIDELEMENTS_EXPORT Dgn::DgnElementId GetAxisId () const { return GetPropertyValueId<Dgn::DgnElementId> (prop_Axis ()); };

    //! Sets gridsurface axis Id
    //! @param[in] axisId to set
    GRIDELEMENTS_EXPORT void SetAxisId(Dgn::DgnElementId axisId) { SetPropertyValue(prop_Axis(), axisId, GetDgnDb().Schemas().GetClassId(GRIDS_SCHEMA_NAME, GRIDS_REL_GridAxisContainsGridSurfaces)); };

    //! Returns base curve of this surface
    //! @return a ptr to a curve vector of this surface
    GRIDELEMENTS_EXPORT CurveVectorPtr    GetSurfaceVector () const;

    //! Tries to return height of this surface which is essencially the magnitude of its extrusion vector
    //! @param[out] height height of this surface
    //! @return     BentleyStatus::ERROR if error occured when trying to get surface height
    GRIDELEMENTS_EXPORT BentleyStatus   TryGetHeight (double& height) const;

    //! Tries to return length of this surface which is essencially the length of its base curve
    //! @param[out] length  length of this surface
    //! @return     BentleyStatus::ERROR if error occured when trying to get surface length
    GRIDELEMENTS_EXPORT BentleyStatus   TryGetLength (double& length) const;

    //---------------------------------------------------------------------------------------
    // Queries
    //---------------------------------------------------------------------------------------
    //! Returns an iterator over GridCurveBundles that this surface drives.
    //! @return     ElementIdIterator over GridCurveBundles driven by this surface
    GRIDELEMENTS_EXPORT BENTLEY_BUILDING_SHARED_NAMESPACE_NAME::ElementIdIterator MakeGridCurveBundleIterator() const;

    //! Returns `GridLabel` owned by this `GridSurface`
    //! @return id of `GridLabel` owned by this `GridSurface`
    GRIDELEMENTS_EXPORT Dgn::DgnElementId GetGridLabelId() const;
    
    //! Returns `GridLabel` owned by this `GridSurface`
    //! @return id of `GridLabel` owned by this `GridSurface`
    GRIDELEMENTS_EXPORT GridLabelCPtr GetGridLabel() const;
};

//=======================================================================================
//! An IPlanGridSurface mixin - this is limited to GridSurface subclasses
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE IPlanGridSurface //inherit nothing here (multi-inheritance, base class handled by actual class)
    {
public:
    struct CreateParams //inherit nothing here (multi-inheritance, base class handled by actual class)
        {
        DEFINE_T_SUPER (GridSurface::CreateParams);
        double m_startElevation;
        double m_endElevation;

        //! Creates create parameters for orthogonal grid
        //! @param[in] startElevation       Start elevation of the surface
        //! @param[in] endElevation         End elevation of the surface
        CreateParams (double startElevation, double endElevation) :
            m_startElevation(startElevation), m_endElevation(endElevation)
            {}
        };
private:
    GridSurfaceR m_thisElem;
protected:

    BE_PROP_NAME (StartElevation)
    BE_PROP_NAME (EndElevation)

    //! initialized this mixin with element reference (pass *this)
    IPlanGridSurface (GridSurfaceR thisElem, CreateParams const& params, Dgn::DgnClassId classId);
public:

    GridSurfaceCR GetThisElem() const { return m_thisElem; }
    GridSurfaceR GetThisElemR() const { return m_thisElem; }

    //! Gets start elevation of this IPlanGridSurface
    //! @return StartElevation of this RadialGrid
    GRIDELEMENTS_EXPORT double      GetStartElevation () const { return m_thisElem.GetPropertyValueDouble (prop_StartElevation ()); }

    //! Sets start elevation of this IPlanGridSurface
    //! @param[in]  staElevation   new StartElevation for this RadialGrid
    GRIDELEMENTS_EXPORT void        SetStartElevation (double staElevation) { m_thisElem.SetPropertyValue (prop_StartElevation (), staElevation); };

    //! Gets end elevation of this IPlanGridSurface
    //! @return EndElevation of this RadialGrid
    GRIDELEMENTS_EXPORT double      GetEndElevation () const { return m_thisElem.GetPropertyValueDouble (prop_EndElevation ()); }

    //! Sets end elevation of this IPlanGridSurface
    //! @param[in]  endElevation   new EndElevation for this RadialGrid
    GRIDELEMENTS_EXPORT void        SetEndElevation (double endElevation) { m_thisElem.SetPropertyValue (prop_EndElevation (), endElevation); };
    };

END_GRIDS_NAMESPACE