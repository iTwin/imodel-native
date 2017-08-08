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
#include <Grids/Domain/GridsMacros.h>
#include "GridSurface.h"

GRIDS_REFCOUNTED_PTR_AND_TYPEDEFS (GridPlaneSurface)

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
    explicit GRIDELEMENTS_EXPORT GridPlaneSurface (CreateParams const& params, CurveVectorPtr surfaceVector);
    explicit GRIDELEMENTS_EXPORT GridPlaneSurface (CreateParams const& params, ISolidPrimitivePtr  surface);
    friend struct GridPlaneSurfaceHandler;

public:
    DECLARE_GRIDS_ELEMENT_BASE_METHODS (GridPlaneSurface, GRIDELEMENTS_EXPORT)

    //IConstrainable
    GRIDELEMENTS_EXPORT virtual bool GetGeomIdPlane(int geomId, DPlane3dR planeOut) const override;
    GRIDELEMENTS_EXPORT virtual bool StretchGeomIdToPlane(int geomId, DPlane3dR targetPlane) override;

    GRIDELEMENTS_EXPORT static  GridPlaneSurfacePtr Create (Dgn::SpatialLocationModelCR model, CurveVectorPtr surfaceVector);
    GRIDELEMENTS_EXPORT static  GridPlaneSurfacePtr Create (Dgn::SpatialLocationModelCR model, ISolidPrimitivePtr  surface);
    GRIDELEMENTS_EXPORT static  GridPlaneSurfacePtr Create (Dgn::SpatialLocationModelCR model, DgnExtrusionDetail  extDetail);

    GRIDELEMENTS_EXPORT         DPlane3d            GetPlane () const;
};

END_GRIDS_NAMESPACE