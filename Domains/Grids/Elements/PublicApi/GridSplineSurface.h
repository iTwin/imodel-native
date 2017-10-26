/*--------------------------------------------------------------------------------------+
|
|     $Source: Grids/Elements/PublicApi/GridSplineSurface.h $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

#include <Bentley/Heapzone.h>
#include <DgnPlatform/Render.h>
#include <DgnPlatform/ClipPrimitive.h>
#include <DgnPlatform/DgnElement.h>
#include <Grids/Domain/GridsMacros.h>
#include "GridSurface.h"

GRIDS_REFCOUNTED_PTR_AND_TYPEDEFS (GridSplineSurface)

BEGIN_GRIDS_NAMESPACE

//=======================================================================================
//! Physical building element
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE GridSplineSurface : GridSurface
{
    DGNELEMENT_DECLARE_MEMBERS (GRIDS_CLASS_GridSplineSurface, GridSurface);

protected:
    explicit GRIDELEMENTS_EXPORT GridSplineSurface (CreateParams const& params);
    explicit GRIDELEMENTS_EXPORT GridSplineSurface (CreateParams const& params, ISolidPrimitivePtr surface);
    friend struct GridSplineSurfaceHandler;

public:
    DECLARE_GRIDS_ELEMENT_BASE_METHODS (GridSplineSurface, GRIDELEMENTS_EXPORT)

    //! Creates a gridarc surface
    //! @param[in]  model           model for the grid surface
    //! @param[in]  extDetail       surface geometry
    //! @return                     gridarc surface
    GRIDELEMENTS_EXPORT static GridSplineSurfacePtr Create (Dgn::SpatialLocationModelCR model, ISolidPrimitivePtr surface);

    //! Creates a gridarc surface
    //! @param[in]  model           model for the grid surface
    //! @param[in]  extDetail       surface geometry
    //! @return                     gridarc surface
    GRIDELEMENTS_EXPORT static GridSplineSurfacePtr Create (Dgn::SpatialLocationModelCR model, DgnExtrusionDetail extDetail);
};

END_GRIDS_NAMESPACE