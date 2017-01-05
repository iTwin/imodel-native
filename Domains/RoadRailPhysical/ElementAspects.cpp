/*--------------------------------------------------------------------------------------+
|
|     $Source: ElementAspects.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <RoadRailPhysicalInternal.h>

HANDLER_DEFINE_MEMBERS(StatusAspectHandler)

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      09/2016
+---------------+---------------+---------------+---------------+---------------+------*/
StatusAspectPtr StatusAspect::Create(Status status)
    {
    return new StatusAspect(status);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      09/2016
+---------------+---------------+---------------+---------------+---------------+------*/
StatusAspectCP StatusAspect::Get(Dgn::PhysicalElementCR el)
    {
    return DgnElement::UniqueAspect::Get<StatusAspect>(el, *QueryClass(el.GetDgnDb()));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      09/2016
+---------------+---------------+---------------+---------------+---------------+------*/
StatusAspectP StatusAspect::GetP(Dgn::PhysicalElementR el, StatusAspectR aspect)
    {
    return dynamic_cast<StatusAspectP>(GetAspectP(el, *QueryClass(el.GetDgnDb())));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      09/2016
+---------------+---------------+---------------+---------------+---------------+------*/
void StatusAspect::Set(Dgn::PhysicalElementR el, StatusAspectR aspect)
    {
    SetAspect(el, aspect);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      09/2016
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus StatusAspect::_LoadProperties(DgnElementCR el)
    {
    auto stmtPtr = el.GetDgnDb().GetPreparedECSqlStatement(
        "SELECT " BRRP_PROP_StatusAspect_Status " FROM " BRRP_SCHEMA(BRRP_CLASS_StatusAspect) " WHERE Element.Id = ?;");
    BeAssert(stmtPtr.IsValid());

    stmtPtr->BindId(1, el.GetElementId());

    if (DbResult::BE_SQLITE_ROW != stmtPtr->Step())
        return DgnDbStatus::BadElement;

    m_status = static_cast<Status>(stmtPtr->GetValueInt(0));

    return DgnDbStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      09/2016
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus StatusAspect::_UpdateProperties(DgnElementCR el, ECCrudWriteToken const* writeToken)
    {
    auto stmtPtr = el.GetDgnDb().GetPreparedECSqlStatement(
        "UPDATE " BRRP_SCHEMA(BRRP_CLASS_StatusAspect) " SET " BRRP_PROP_StatusAspect_Status " = ? WHERE Element.Id = ?;");
    BeAssert(stmtPtr.IsValid());

    stmtPtr->BindInt(1, static_cast<int>(m_status));
    stmtPtr->BindId(2, el.GetElementId());

    if (DbResult::BE_SQLITE_DONE != stmtPtr->Step())
        return DgnDbStatus::WriteError;

    return DgnDbStatus::Success;
    }