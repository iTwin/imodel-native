/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include <Grids/Elements/GridElementsAPI.h>

BEGIN_GRIDS_NAMESPACE
USING_NAMESPACE_BENTLEY_DGN

DEFINE_GRIDS_ELEMENT_BASE_METHODS (GridLabel)

//---------------------------------------------------------------------------------------
// @bsimethod                                    Haroldas.Vitunskas                  04/19
//---------------------------------------------------------------------------------------
GridLabel::CreateParams::CreateParams(DgnElement::CreateParams const& createParams,
                                      Dgn::DgnElementId ownerSurface,
                                      bool labelAtStart,
                                      bool labelAtEnd)
    : T_Super(createParams)
    , m_ownerSurface(ownerSurface)
    , m_labelAtStart(labelAtStart)
    , m_labelAtEnd(labelAtEnd)
    {
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Haroldas.Vitunskas                  04/19
//---------------------------------------------------------------------------------------
GridLabel::GridLabel (CreateParams const& params)
    : T_Super(params)
    {
    if (params.m_isLoadingElement)
        return;

    if (DgnDbStatus::Success != SetOwnerId (params.m_ownerSurface))
        BeAssert (false);

    SetHasLabelAtStart (params.m_labelAtStart);
    SetHasLabelAtEnd (params.m_labelAtEnd);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Haroldas.Vitunskas                  04/19
//---------------------------------------------------------------------------------------
Dgn::DgnDbStatus GridLabel::_OnUpdate (Dgn::DgnElementCR original)
    {
    if (GetOwner().IsNull())
        return DgnDbStatus::ValidationFailed; // Should not exist without a grid surface
    
    return T_Super::_OnUpdate (original);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Haroldas.Vitunskas                  04/19
//---------------------------------------------------------------------------------------
Dgn::DgnDbStatus GridLabel::_OnInsert()
    {
    if (GetOwner().IsNull())
        return DgnDbStatus::ValidationFailed; // Should not exist without a grid surface

    return T_Super::_OnInsert();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Haroldas.Vitunskas                  04/19
//---------------------------------------------------------------------------------------
Dgn::DgnDbStatus GridLabel::SetOwnerId (Dgn::DgnElementId owner)
    {
    GridSurfaceCPtr surface = GetDgnDb().Elements().Get<GridSurface> (owner);
    if (surface.IsNull())
        return DgnDbStatus::ValidationFailed; // Owner should be a grid surface

    return SetOwner (*surface);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Haroldas.Vitunskas                  04/19
//---------------------------------------------------------------------------------------
Dgn::DgnDbStatus GridLabel::SetOwner (GridSurfaceCR owner)
    {
    if (owner.GetGridLabel().IsValid())
        return DgnDbStatus::ValidationFailed; // Owner should not have a valid grid label already

    DgnClassId relationshipId = GetDgnDb().Schemas().GetClassId (GRIDS_SCHEMA_NAME, GRIDS_REL_GridSurfaceOwnsGridLabel);
    return SetParentId (owner.GetElementId(), relationshipId);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Haroldas.Vitunskas                  04/19
//---------------------------------------------------------------------------------------
GridLabelPtr GridLabel::Create (Dgn::DgnDbCR db, Dgn::DgnElementId ownerSurface, Utf8CP label, bool labelAtStart, bool labelAtEnd)
    {
    GridSurfaceCPtr surface = db.Elements().Get<GridSurface> (ownerSurface);
    if (surface.IsNull())
        return nullptr;

    return Create (*surface, label, labelAtStart, labelAtEnd);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Haroldas.Vitunskas                  04/19
//---------------------------------------------------------------------------------------
GridLabelPtr GridLabel::Create (GridSurfaceCR ownerSurface, Utf8CP label, bool labelAtStart, bool labelAtEnd)
    {
    DgnClassId classId = QueryClassId (ownerSurface.GetDgnDb());
    DgnElement::CreateParams elementParams (ownerSurface.GetDgnDb(), ownerSurface.GetModelId(), classId);
    elementParams.m_userLabel = label;
    CreateParams params (elementParams, ownerSurface.GetElementId(), labelAtStart, labelAtEnd);

    return new GridLabel (params);
    }

END_GRIDS_NAMESPACE