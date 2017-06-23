/*--------------------------------------------------------------------------------------+
|
|     $Source: Grids/Elements/PublicApi/GridArc.h $
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
#include "GridCurve.h"

GRIDS_REFCOUNTED_PTR_AND_TYPEDEFS (GridArc)

BEGIN_GRIDS_NAMESPACE

//=======================================================================================
//! Physical building element
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE GridArc : GridCurve
{
    DGNELEMENT_DECLARE_MEMBERS (GRIDS_CLASS_GridArc, GridCurve);
    DEFINE_T_SUPER(GridCurve);

protected:
    explicit GRIDELEMENTS_EXPORT GridArc (CreateParams const& params);
    explicit GRIDELEMENTS_EXPORT GridArc (CreateParams const& params, ICurvePrimitivePtr curve);
    friend struct GridArcHandler;

    virtual GRIDELEMENTS_EXPORT void                _CopyFrom (Dgn::DgnElementCR source) override;

public:
    DECLARE_GRIDS_ELEMENT_BASE_METHODS (GridArc, GRIDELEMENTS_EXPORT)

    GRIDELEMENTS_EXPORT static GridArcPtr Create (Dgn::DgnModelCR model, ICurvePrimitivePtr curve);
};

END_GRIDS_NAMESPACE