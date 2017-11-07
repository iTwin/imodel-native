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
protected:
    explicit GRIDELEMENTS_EXPORT GridSurface (CreateParams const& params);
    explicit GRIDELEMENTS_EXPORT GridSurface (CreateParams const& params, GridAxisCPtr gridAxis, CurveVectorPtr surfaceVector);
    explicit GRIDELEMENTS_EXPORT GridSurface (CreateParams const& params, GridAxisCPtr gridAxis, ISolidPrimitivePtr surface);

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

    virtual BentleyStatus   _SetGeometry(ISolidPrimitivePtr surface);
    virtual bool            _ValidateGeometry(ISolidPrimitivePtr surface) const { return false; };
    //! Sets gridsurface axis Id
    //! @param[in] axisId to set
    void SetAxisId (Dgn::DgnElementId axisId) { SetPropertyValue (prop_Axis (), axisId, GetDgnDb().Schemas().GetClassId(GRIDS_SCHEMA_NAME, GRIDS_REL_GridAxisContainsGridSurfaces)); };

    static GRIDELEMENTS_EXPORT Dgn::GeometricElement3d::CreateParams        CreateParamsFromModel (Dgn::SpatialLocationModelCR model, Dgn::DgnClassId classId);

protected:

public:
    DECLARE_GRIDS_ELEMENT_BASE_METHODS (GridSurface, GRIDELEMENTS_EXPORT)

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

    //! @return axis id of the surface
    GRIDELEMENTS_EXPORT Dgn::DgnElementId GetGridId () const;

    //! @return axis id of the surface
    GRIDELEMENTS_EXPORT Dgn::DgnElementId GetAxisId () const { return GetPropertyValueId<Dgn::DgnElementId> (prop_Axis ()); };

    //! Sets geometry for this grid surface
    GRIDELEMENTS_EXPORT BentleyStatus SetGeometry(ISolidPrimitivePtr surface);

    //! Returns base curve of this surface
    //! @return a ptr to a curve vector of this surface
    GRIDELEMENTS_EXPORT CurveVectorPtr    GetSurfaceVector () const;

    //! Tries to return height of this surface which is essencially the magnitude of its extrusion vector
    //! @param[out] height height of this surface
    GRIDELEMENTS_EXPORT BentleyStatus   TryGetHeight (double& height) const;

    //! Tries to return length of this surface which is essencially the length of its base curve
    //! @param[out] length  length of this surface
    //! @return     BentleyStatus::ERROR if error occured when trying to get surface length
    GRIDELEMENTS_EXPORT BentleyStatus   TryGetLength (double& length) const;
};

END_GRIDS_NAMESPACE