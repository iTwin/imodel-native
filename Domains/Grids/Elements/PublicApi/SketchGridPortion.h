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
#include <Grids/gridsApi.h>

BEGIN_GRIDS_NAMESPACE

//=======================================================================================
//! Physical building element
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE SketchGrid : PlanGrid
{
    DGNELEMENT_DECLARE_MEMBERS (GRIDS_CLASS_SketchGrid, PlanGrid);

protected:
    explicit GRIDELEMENTS_EXPORT SketchGrid (T_Super::CreateParams const& params);
    friend struct SketchGridHandler;

public:
    DECLARE_GRIDS_ELEMENT_BASE_METHODS (SketchGrid, GRIDELEMENTS_EXPORT)

    //---------------------------------------------------------------------------------------
    // Creation
    //---------------------------------------------------------------------------------------
    //! Creates an empty sketch grid
    //! @param[in]  model   model for the radialgridportion
    //! @return             sketch grid
    GRIDELEMENTS_EXPORT static SketchGridPtr Create (Dgn::DgnModelCR model, Utf8CP name);
    };

END_GRIDS_NAMESPACE