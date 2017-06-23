/*--------------------------------------------------------------------------------------+
|
|     $Source: Grids/Elements/PublicApi/RadialGridPortion.h $
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

GRIDS_REFCOUNTED_PTR_AND_TYPEDEFS (RadialGridPortion)

BEGIN_GRIDS_NAMESPACE

//=======================================================================================
//! Physical building element
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE RadialGridPortion : GridPortion
{
    DGNELEMENT_DECLARE_MEMBERS (GRIDS_CLASS_RadialGridPortion, GridPortion);

protected:
    explicit GRIDELEMENTS_EXPORT RadialGridPortion (CreateParams const& params);
    friend struct RadialGridPortionHandler;

public:
    DECLARE_GRIDS_ELEMENT_BASE_METHODS (RadialGridPortion, GRIDELEMENTS_EXPORT)

    GRIDELEMENTS_EXPORT static RadialGridPortionPtr Create (Dgn::DgnModelCR model);
};

END_GRIDS_NAMESPACE