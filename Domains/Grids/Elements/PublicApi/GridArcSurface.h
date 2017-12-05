/*--------------------------------------------------------------------------------------+
|
|     $Source: Grids/Elements/PublicApi/GridArcSurface.h $
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

GRIDS_REFCOUNTED_PTR_AND_TYPEDEFS (GridArcSurface)

BEGIN_GRIDS_NAMESPACE

//=======================================================================================
//! Physical building element
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE GridArcSurface : GridSurface
{
    DGNELEMENT_DECLARE_MEMBERS (GRIDS_CLASS_GridArcSurface, GridSurface);

protected:
    explicit GRIDELEMENTS_EXPORT GridArcSurface (CreateParams const& params);
    explicit GRIDELEMENTS_EXPORT GridArcSurface (CreateParams const& params, ISolidPrimitivePtr surface);
    friend struct GridArcSurfaceHandler;

    virtual bool            _ValidateGeometry(ISolidPrimitivePtr surface) const override;
    GRIDELEMENTS_EXPORT virtual Dgn::DgnDbStatus _Validate () const override;

public:
    DECLARE_GRIDS_ELEMENT_BASE_METHODS (GridArcSurface, GRIDELEMENTS_EXPORT)

    //---------------------------------------------------------------------------------------
    // Creation
    //---------------------------------------------------------------------------------------
    //! Creates a gridarc surface
    //! @param[in]  model           model for the grid surface
    //! @param[in]  extDetail       surface geometry
    //! @return                     gridarc surface
    //! Note: Only solid primitives with DgnExtrusionDetails with arcs as base curves pass as valid geometry
    GRIDELEMENTS_EXPORT static GridArcSurfacePtr Create (Dgn::SpatialLocationModelCR model, GridAxisCR gridAxis, ISolidPrimitivePtr surface);

    //! Creates a gridarc surface
    //! @param[in]  model           model for the grid surface
    //! @param[in]  extDetail       surface geometry
    //! @return                     gridarc surface
    //! Note: Only DgnExtrusionDetails with arcs as base curves pass as valid geometry
    GRIDELEMENTS_EXPORT static GridArcSurfacePtr Create (Dgn::SpatialLocationModelCR model, GridAxisCR gridAxis, DgnExtrusionDetail extDetail);
};

END_GRIDS_NAMESPACE