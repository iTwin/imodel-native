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
    explicit GRIDELEMENTS_EXPORT SketchGrid (T_Super::CreateParams const& params, DVec3d normal);
    friend struct SketchGridHandler;

public:
    DECLARE_GRIDS_ELEMENT_BASE_METHODS (SketchGrid, GRIDELEMENTS_EXPORT)

    //---------------------------------------------------------------------------------------
    // Creation
    //---------------------------------------------------------------------------------------
    //! Creates an empty sketch grid
    //! @param[in]  model   model for the radialgridportion
    //! @param[in]  normal  perpendicularity plane of this Grid
    //! @return             sketch grid
    GRIDELEMENTS_EXPORT static SketchGridPtr Create (Dgn::DgnModelCR model, DVec3d normal, Utf8CP name);

    //---------------------------------------------------------------------------------------
    // Queries
    //---------------------------------------------------------------------------------------
    //! Returns a sketch grid portion with given parent element and name
    //! @param[in]  db          db to try get sketch grid portion from
    //! @param[in]  parentId    id of parent element for grid portion
    //! @param[in]  gridName    name for grid portion
    //! @return                 if grid portion is found, returns a ptr to it
    //!                         else nullptr is returned
    GRIDELEMENTS_EXPORT static SketchGridPtr TryGet(Dgn::DgnDbR db, Dgn::DgnElementId parentId, Utf8CP gridName);
};

END_GRIDS_NAMESPACE