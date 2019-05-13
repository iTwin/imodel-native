/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#include "RoadRailPhysicalInternal.h"
#include <RoadRailPhysical/ElementAspects.h>

HANDLER_DEFINE_MEMBERS(AssociatedFacetAspectHandler)

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      09/2016
+---------------+---------------+---------------+---------------+---------------+------*/
AssociatedFacetAspectPtr AssociatedFacetAspect::Create(AssociatedFacetEnum associatedFacet)
    {
    return new AssociatedFacetAspect(associatedFacet);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      09/2016
+---------------+---------------+---------------+---------------+---------------+------*/
AssociatedFacetAspectCP AssociatedFacetAspect::Get(DgnElementCR el)
    {
    return DgnElement::UniqueAspect::Get<AssociatedFacetAspect>(el, *QueryClass(el.GetDgnDb()));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      09/2016
+---------------+---------------+---------------+---------------+---------------+------*/
AssociatedFacetAspectP AssociatedFacetAspect::GetP(DgnElementR el)
    {
    return dynamic_cast<AssociatedFacetAspectP>(GetAspectP(el, *QueryClass(el.GetDgnDb())));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      09/2016
+---------------+---------------+---------------+---------------+---------------+------*/
void AssociatedFacetAspect::Set(DgnElementR el, AssociatedFacetAspectR aspect)
    {
    SetAspect(el, aspect);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      09/2016
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus AssociatedFacetAspect::_LoadProperties(DgnElementCR el)
    {
    auto stmtPtr = el.GetDgnDb().GetPreparedECSqlStatement(
        "SELECT " BRRP_PROP_AssociatedFacetAspect_AssociatedFacet " FROM " BRRP_SCHEMA(BRRP_CLASS_AssociatedFacetAspect) " WHERE Element.Id = ?;");
    BeAssert(stmtPtr.IsValid());

    stmtPtr->BindId(1, el.GetElementId());

    if (DbResult::BE_SQLITE_ROW != stmtPtr->Step())
        return DgnDbStatus::BadElement;

    m_associatedFacet = static_cast<AssociatedFacetEnum>(stmtPtr->GetValueInt(0));

    return DgnDbStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      09/2016
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus AssociatedFacetAspect::_UpdateProperties(DgnElementCR el, ECCrudWriteToken const* writeToken)
    {
    auto stmtPtr = el.GetDgnDb().GetNonSelectPreparedECSqlStatement(
        "UPDATE " BRRP_SCHEMA(BRRP_CLASS_AssociatedFacetAspect) " SET " BRRP_PROP_AssociatedFacetAspect_AssociatedFacet " = ? WHERE Element.Id = ?;", writeToken);
    BeAssert(stmtPtr.IsValid());

    stmtPtr->BindInt(1, static_cast<int>(m_associatedFacet));
    stmtPtr->BindId(2, el.GetElementId());

    if (DbResult::BE_SQLITE_DONE != stmtPtr->Step())
        return DgnDbStatus::WriteError;

    return DgnDbStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      07/2017
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus AssociatedFacetAspect::_GetPropertyValue(ECValueR value, Utf8CP propertyName, PropertyArrayIndex const& arrayIndex) const
    {
    if (0 != strcmp(BRRP_PROP_AssociatedFacetAspect_AssociatedFacet, propertyName))
        return DgnDbStatus::BadRequest;

    value.SetInteger(static_cast<int32_t>(GetAssociatedFacet()));

    return DgnDbStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      07/2017
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus AssociatedFacetAspect::_SetPropertyValue(Utf8CP propertyName, ECValueCR value, PropertyArrayIndex const& arrayIndex)
    {
    if (0 != strcmp(BRRP_PROP_AssociatedFacetAspect_AssociatedFacet, propertyName) || value.IsNull())
        return DgnDbStatus::BadRequest;

    SetAssociatedFacet(static_cast<AssociatedFacetEnum>(value.GetInteger()));

    return DgnDbStatus::Success;
    }
