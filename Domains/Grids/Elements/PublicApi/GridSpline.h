/*--------------------------------------------------------------------------------------+
|
|     $Source: Grids/Elements/PublicApi/GridSpline.h $
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
#include "GridCurve.h"

GRIDS_REFCOUNTED_PTR_AND_TYPEDEFS (GridSpline)

BEGIN_GRIDS_NAMESPACE

//=======================================================================================
//! Physical building element
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE GridSpline : GridCurve
{
    DGNELEMENT_DECLARE_MEMBERS (GRIDS_CLASS_GridSpline, GridCurve);
    DEFINE_T_SUPER(GridCurve);

protected:
    explicit GRIDELEMENTS_EXPORT GridSpline (CreateParams const& params);
    explicit GRIDELEMENTS_EXPORT GridSpline (CreateParams const& params, ICurvePrimitivePtr curve);
    friend struct GridSplineHandler;

    virtual GRIDELEMENTS_EXPORT void                _CopyFrom (Dgn::DgnElementCR source) override;

public:
    DECLARE_GRIDS_ELEMENT_BASE_METHODS (GridSpline, GRIDELEMENTS_EXPORT)

    GRIDELEMENTS_EXPORT static GridSplinePtr Create (Dgn::DgnModelCR model, ICurvePrimitivePtr curve);
};

END_GRIDS_NAMESPACE