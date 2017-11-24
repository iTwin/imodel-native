/*--------------------------------------------------------------------------------------+
|
|     $Source: Grids/Elements/PublicApi/GridPlaneSurface.h $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

#include <Bentley/Heapzone.h>
#include <DgnPlatform/Render.h>
#include <DgnPlatform/ClipPrimitive.h>
#include <DgnPlatform/DgnElement.h>
#include <ConstraintSystem/ConstraintSystemApi.h>
#include <Grids/gridsApi.h>

BEGIN_GRIDS_NAMESPACE

//=======================================================================================
//! Physical building element
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE GridPlaneSurface : GridSurface, ConstraintModel::IConstrainable
{
    DGNELEMENT_DECLARE_MEMBERS (GRIDS_CLASS_GridPlaneSurface, GridSurface);
    DEFINE_T_SUPER(GridSurface);

private:

protected:
    explicit GRIDELEMENTS_EXPORT GridPlaneSurface (CreateParams const& params);
    explicit GRIDELEMENTS_EXPORT GridPlaneSurface (CreateParams const& params, GridAxisCPtr gridAxis, CurveVectorPtr surfaceVector);
    explicit GRIDELEMENTS_EXPORT GridPlaneSurface (CreateParams const& params, GridAxisCPtr gridAxis, ISolidPrimitivePtr  surface);
    friend struct GridPlaneSurfaceHandler;

    virtual bool            _ValidateGeometry(ISolidPrimitivePtr surface) const override;

    GRIDELEMENTS_EXPORT virtual Dgn::DgnDbStatus _Validate () const override;
public:
    //IConstrainable TODO: remove IConstrainable
    GRIDELEMENTS_EXPORT virtual bool GetGeomIdPlane (int geomId, DPlane3dR planeOut) const override;
    GRIDELEMENTS_EXPORT virtual bool StretchGeomIdToPlane (int geomId, DPlane3dR targetPlane) override;

    DECLARE_GRIDS_ELEMENT_BASE_METHODS (GridPlaneSurface, GRIDELEMENTS_EXPORT)

    //---------------------------------------------------------------------------------------
    // Creation
    //---------------------------------------------------------------------------------------
    //! Creates a gridplane surface
    //! @param[in]  model           model for the grid surface
    //! @param[in]  surfaceVector   surface geometry
    //! @return                     gridplane surface
    //! Note: Only planar curve vectors pass as valid geometry
    GRIDELEMENTS_EXPORT static  GridPlaneSurfacePtr Create (Dgn::SpatialLocationModelCR model, GridAxisCPtr gridAxis, CurveVectorPtr surfaceVector);

    //! Creates a gridplane surface
    //! @param[in]  model           model for the grid surface
    //! @param[in]  surface         surface geometry
    //! @return                     gridplane surface
    //! @Note: Only solid primitives from DgnExtrusionDetails with LineString, PointString or Line as base curve pass as valid geometry
    GRIDELEMENTS_EXPORT static  GridPlaneSurfacePtr Create (Dgn::SpatialLocationModelCR model, GridAxisCPtr gridAxis, ISolidPrimitivePtr  surface);

    //! Creates a gridplane surface
    //! @param[in]  model           model for the grid surface
    //! @param[in]  extDetail       surface geometry
    //! @return                     gridplane surface
    //! Note: Only DgnExtrusionDetails with LineString, PointString or Line as base curve pass as valid geometry
    GRIDELEMENTS_EXPORT static  GridPlaneSurfacePtr Create (Dgn::SpatialLocationModelCR model, GridAxisCPtr gridAxis, DgnExtrusionDetail  extDetail);

    //---------------------------------------------------------------------------------------
    // Getters and setters
    //---------------------------------------------------------------------------------------
    //! gets the plane of this gridplanesurface
    //! @return     plane of this gridplanesurface
    GRIDELEMENTS_EXPORT         DPlane3d            GetPlane() const;

    //---------------------------------------------------------------------------------------
    // Geometry modification
    //---------------------------------------------------------------------------------------
    //! sets curveVector for this gridPlane
    //! @param[in]  newShape        new curvevector shape for the GridPlaneSurface
    GRIDELEMENTS_EXPORT void    SetCurveVector (CurveVectorR newShape);

    
};

END_GRIDS_NAMESPACE