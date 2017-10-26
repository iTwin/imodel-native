/*--------------------------------------------------------------------------------------+
|
|     $Source: Grids/Elements/PublicApi/GridAxis.h $
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
#include <Grids/Domain/GridsMacros.h>

GRIDS_REFCOUNTED_PTR_AND_TYPEDEFS (GridAxis)

BEGIN_GRIDS_NAMESPACE

//=======================================================================================
//! Physical building element
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE GridAxis : Dgn::DefinitionElement
{
    DGNELEMENT_DECLARE_MEMBERS (GRIDS_CLASS_GridAxis, Dgn::DefinitionElement);
    DEFINE_T_SUPER(Dgn::DefinitionElement);

protected:
    explicit GRIDELEMENTS_EXPORT GridAxis (CreateParams const& params);
    friend struct GridAxisHandler;

    static CreateParams CreateParamsFromModel (Dgn::DgnModelCR model, Dgn::DgnClassId classId);

    BE_PROP_NAME (Grid)

    //! Sets opening's isEgress value
    //! @param isEgress a value to set
    void SetGridId (Dgn::DgnElementId gridId) { SetPropertyValue (prop_Grid (), gridId); };

public:
    DECLARE_GRIDS_ELEMENT_BASE_METHODS (GridAxis, GRIDELEMENTS_EXPORT)

    //! Creates an empty grid axis
    //! @param[in]  model   model for the axis
    //! @param[in]  grid    grid this axis belongs to
    //! @return             sketch grid
    GRIDELEMENTS_EXPORT static GridAxisPtr CreateAndInsert (Dgn::DgnModelCR model, GridPortionCR grid);

    //! @return element id of the grid
    GRIDELEMENTS_EXPORT Dgn::DgnElementId GetGridId () const { return GetPropertyValueId<Dgn::DgnElementId> (prop_Grid ()); };

};

END_GRIDS_NAMESPACE