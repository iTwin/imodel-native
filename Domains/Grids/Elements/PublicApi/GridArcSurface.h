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
#include <GridsMacros.h>
#include "GridSurface.h"

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
    explicit GRIDELEMENTS_EXPORT GridArcSurface (CreateParams const& params, CurveVectorPtr surfaceVector);
    explicit GRIDELEMENTS_EXPORT GridArcSurface (CreateParams const& params, ISolidPrimitivePtr surface);
    friend struct GridArcSurfaceHandler;

public:
    DECLARE_GRIDS_ELEMENT_BASE_METHODS (GridArcSurface, GRIDELEMENTS_EXPORT)

    GRIDELEMENTS_EXPORT static GridArcSurfacePtr Create (Dgn::DgnModelCR model, CurveVectorPtr surfaceVector);
    GRIDELEMENTS_EXPORT static GridArcSurfacePtr Create (Dgn::DgnModelCR model, ISolidPrimitivePtr surface);
};

END_GRIDS_NAMESPACE