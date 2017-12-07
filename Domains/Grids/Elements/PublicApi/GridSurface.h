/*--------------------------------------------------------------------------------------+
|
|     $Source: Grids/Elements/PublicApi/GridSurface.h $
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

//=======================================================================================
//! Physical building element
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

    virtual BentleyStatus   _SetGeometry(ISolidPrimitivePtr surface);
    virtual bool            _ValidateGeometry(ISolidPrimitivePtr surface) const { return false; };
    //! Sets gridsurface axis Id
    //! @param[in] axisId to set
    void SetAxisId (Dgn::DgnElementId axisId) { SetPropertyValue (prop_Axis (), axisId, GetDgnDb().Schemas().GetClassId(GRIDS_SCHEMA_NAME, GRIDS_REL_GridAxisContainsGridSurfaces)); };

    static GRIDELEMENTS_EXPORT CreateParams        CreateParamsFromModelAxisClassId (Dgn::SpatialLocationModelCR model, GridAxisCR axis, Dgn::DgnClassId classId);

protected:

public:
    DECLARE_GRIDS_ELEMENT_BASE_METHODS (GridSurface, GRIDELEMENTS_EXPORT)

    //---------------------------------------------------------------------------------------
    // Geometry modification
    //---------------------------------------------------------------------------------------
    //! Rotates grid by given angle in radians
    //! @param[in] theta            angle in radians
    GRIDELEMENTS_EXPORT void RotateXY(double theta);

    //! Rotates grid around point by given angle in radians
    //! @param[in] point            point to rotate around
    //! @param[in] theta            angle in radians
    GRIDELEMENTS_EXPORT void RotateXY(DPoint3d point, double theta);

    //! Translates grid to given point
    //! @param[in] target   point to move
    GRIDELEMENTS_EXPORT void MoveToPoint(DPoint3d target);

    //! Translates grid to given point
    //! @param[in] translation   vector to translate by
    GRIDELEMENTS_EXPORT void TranslateXY(DVec3d translation);

    //! Translates grid to given point
    //! @param[in] translation   vector to translate by
    GRIDELEMENTS_EXPORT void Translate(DVec3d translation);

    //! Sets geometry for this grid surface
    //! @param[in]  surface     new geometry for the surface
    //! @returns BentleyStatus::SUCCESS if geometry is valid for the surface and there has been no error in setting surface geometry
    //! For specifications of geometry validation see derived grid surface elements' creation documentation
    GRIDELEMENTS_EXPORT BentleyStatus SetGeometry(ISolidPrimitivePtr surface);

    //---------------------------------------------------------------------------------------
    // Setters and getters
    //---------------------------------------------------------------------------------------
    //! Returns id of grid that has this surface
    //! @return grid id of the surface
    GRIDELEMENTS_EXPORT Dgn::DgnElementId GetGridId () const;

    //!Returns id of axis that has this surface
    //! @return axis id of the surface
    GRIDELEMENTS_EXPORT Dgn::DgnElementId GetAxisId () const { return GetPropertyValueId<Dgn::DgnElementId> (prop_Axis ()); };

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
    //! Returns an iterator over created intersection grid curves
    //! @return     ElementIterator over this grid surface's grid curves
    GRIDELEMENTS_EXPORT Dgn::ElementIterator MakeCreatedCurvesIterator() const;
};

//=======================================================================================
//! an IPlanGridSurface mixin - this is limited to GridSurface subclasses
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
        //! @param[in] model              model for the PlanCartesianGridSurface
        CreateParams (double staElevation, double endElevation) :
            m_startElevation(staElevation), m_endElevation(endElevation)
            {}
        };
private:
    Dgn::DgnElementR m_thisElem;
protected:

    BE_PROP_NAME (StartElevation)
    BE_PROP_NAME (EndElevation)

    //! initialized this mixin with element reference (pass *this)
    IPlanGridSurface (Dgn::DgnElementR thisElem, CreateParams const& params);
public:

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