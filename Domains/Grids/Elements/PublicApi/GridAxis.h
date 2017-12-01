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
#include <Grids/gridsApi.h>

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

    Dgn::DgnDbStatus Validate () const;

    //! Called when an element is about to be inserted into the DgnDb.
    //! @return DgnDbStatus::Success to allow the insert, otherwise it will fail with the returned status.
    //! @note If you override this method, you @em must call T_Super::_OnInsert, forwarding its status.
    GRIDELEMENTS_EXPORT virtual Dgn::DgnDbStatus _OnInsert () override;

    //! Called when this element is about to be replace its original element in the DgnDb.
    //! @param [in] original the original state of this element.
    //! Subclasses may override this method to control whether their instances are updated.
    //! @return DgnDbStatus::Success to allow the update, otherwise the update will fail with the returned status.
    //! @note If you override this method, you @em must call T_Super::_OnUpdate, forwarding its status.
    GRIDELEMENTS_EXPORT virtual Dgn::DgnDbStatus _OnUpdate (Dgn::DgnElementCR original) override;
public:
    DECLARE_GRIDS_ELEMENT_BASE_METHODS (GridAxis, GRIDELEMENTS_EXPORT)

    //---------------------------------------------------------------------------------------
    // Creation
    //---------------------------------------------------------------------------------------
    //! Creates an empty grid axis
    //! @param[in]  model   model for the axis
    //! @param[in]  grid    grid this axis belongs to
    //! @return             sketch grid
    GRIDELEMENTS_EXPORT static GridAxisPtr Create (Dgn::DgnModelCR model, GridPortionCR grid);

    //! Creates and inserts an empty grid axis
    //! @param[in]  model   model for the axis
    //! @param[in]  grid    grid this axis belongs to
    //! @return             sketch grid
    GRIDELEMENTS_EXPORT static GridAxisPtr CreateAndInsert (Dgn::DgnModelCR model, GridPortionCR grid);

    //---------------------------------------------------------------------------------------
    // Getters and setters
    //---------------------------------------------------------------------------------------
    //! Sets gridAxis grid Id value
    //! @param gridId a value to set
    void SetGridId (Dgn::DgnElementId gridId) { SetPropertyValue (prop_Grid (), gridId, GetDgnDb().Schemas().GetClassId(GRIDS_SCHEMA_NAME, GRIDS_REL_GridPortionHasAxes)); };

    //! Gets grid's that contains this axis id
    //! @return element id of the grid
    Dgn::DgnElementId GetGridId () const { return GetPropertyValueId<Dgn::DgnElementId> (prop_Grid ()); };

    //---------------------------------------------------------------------------------------
    // Queries
    //---------------------------------------------------------------------------------------
    //! Make an iterator over gridSurfaces that compose this GridAxis
    //! @returns an iterator over grid surfaces that this grid axis contains
    GRIDELEMENTS_EXPORT Dgn::ElementIterator MakeIterator () const;
};

END_GRIDS_NAMESPACE