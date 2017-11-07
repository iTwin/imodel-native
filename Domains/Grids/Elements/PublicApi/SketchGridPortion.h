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
struct EXPORT_VTABLE_ATTRIBUTE SketchGridPortion : GridPortion
{
    DGNELEMENT_DECLARE_MEMBERS (GRIDS_CLASS_SketchGridPortion, GridPortion);

protected:
    explicit GRIDELEMENTS_EXPORT SketchGridPortion (T_Super::CreateParams const& params);
    explicit GRIDELEMENTS_EXPORT SketchGridPortion (T_Super::CreateParams const& params, DVec3d normal);
    friend struct SketchGridPortionHandler;

public:
    DECLARE_GRIDS_ELEMENT_BASE_METHODS (SketchGridPortion, GRIDELEMENTS_EXPORT)

    //! Creates an empty sketch grid
    //! @param[in]  model   model for the radialgridportion
    //! @param[in]  normal  perpendicularity plane of this Grid
    //! @return             sketch grid
    GRIDELEMENTS_EXPORT static SketchGridPortionPtr Create (Dgn::DgnModelCR model, DVec3d normal, Utf8CP name);

    //! Returns a sketch grid portion with given parent element and name
    GRIDELEMENTS_EXPORT static SketchGridPortionPtr TryGet(Dgn::DgnDbR db, Dgn::DgnElementId parentId, Utf8CP gridName);
};

END_GRIDS_NAMESPACE