/*--------------------------------------------------------------------------------------+
|
|     $Source: Grids/Elements/PublicApi/SketchGridPortion.h $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

#include <Bentley/Heapzone.h>
#include <DgnPlatform/Render.h>
#include <DgnPlatform/ClipPrimitive.h>
#include <DgnPlatform/DgnElement.h>
#include "GridPortion.h"

GRIDS_REFCOUNTED_PTR_AND_TYPEDEFS (SketchGridPortion)

BEGIN_GRIDS_NAMESPACE

//=======================================================================================
//! Physical building element
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE SketchGridPortion : GridPortion
{
    DGNELEMENT_DECLARE_MEMBERS (GRIDS_CLASS_SketchGridPortion, GridPortion);

protected:
    explicit GRIDELEMENTS_EXPORT SketchGridPortion (CreateParams const& params);
    friend struct SketchGridPortionHandler;

public:
    DECLARE_GRIDS_ELEMENT_BASE_METHODS (SketchGridPortion, GRIDELEMENTS_EXPORT)

    GRIDELEMENTS_EXPORT static SketchGridPortionPtr Create (Dgn::DgnModelCR model);
};

END_GRIDS_NAMESPACE