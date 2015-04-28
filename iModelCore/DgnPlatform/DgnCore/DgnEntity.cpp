/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnCore/DgnEntity.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <DgnPlatformInternal.h>

//---------------------------------------------------------------------------------------
// @bsimethod                                   Shaun.Sewall                    10/2014
//---------------------------------------------------------------------------------------
BentleyStatus DgnElements::DeleteElement(DgnElementId elementId)
    {
    if (!elementId.IsValid())
        return BentleyStatus::ERROR;

    CachedStatementPtr statementPtr;
    m_dgndb.GetCachedStatement(statementPtr, "DELETE FROM " DGN_TABLE(DGN_CLASSNAME_Element) " WHERE Id=?");

    statementPtr->BindId(1, elementId);
    return (BE_SQLITE_DONE == statementPtr->Step()) ? BentleyStatus::SUCCESS : BentleyStatus::ERROR;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Shaun.Sewall                    01/2015
//---------------------------------------------------------------------------------------
BentleyStatus DgnElements::UpdateLastModifiedTime(DgnElementId elementId)
    {
    if (!elementId.IsValid())
        return BentleyStatus::ERROR;

    CachedStatementPtr statementPtr;
    m_dgndb.GetCachedStatement(statementPtr, "UPDATE " DGN_TABLE(DGN_CLASSNAME_Element) " SET Id=Id WHERE Id=?"); // Minimal SQL to cause trigger to run

    statementPtr->BindId(1, elementId);
    return (BE_SQLITE_DONE == statementPtr->Step()) ? BentleyStatus::SUCCESS : BentleyStatus::ERROR;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Shaun.Sewall                    01/2015
//---------------------------------------------------------------------------------------
DateTime DgnElements::QueryLastModifiedTime(DgnElementId elementId) const
    {
    if (!elementId.IsValid())
        return DateTime();

    CachedECSqlStatementPtr statementPtr = GetDgnDb().GetPreparedECSqlStatement("SELECT LastMod FROM " DGN_SCHEMA(DGN_CLASSNAME_Element) " WHERE ECInstanceId=?");
    if (!statementPtr.IsValid())
        return DateTime();

    statementPtr->BindId(1, elementId);

    if (ECSqlStepStatus::HasRow != statementPtr->Step())
        return DateTime();

    return statementPtr->GetValueDateTime(0);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Shaun.Sewall                    10/2014
//---------------------------------------------------------------------------------------
DgnElementKey DgnElements::QueryElementKey(DgnElementId elementId) const
    {
    if (!elementId.IsValid())
        return DgnElementKey();

    CachedStatementPtr statementPtr;
    m_dgndb.GetCachedStatement(statementPtr, "SELECT ECClassId FROM " DGN_TABLE(DGN_CLASSNAME_Element) " WHERE Id=?");

    statementPtr->BindId(1, elementId);
    
    return (BE_SQLITE_ROW != statementPtr->Step()) ? DgnElementKey() : DgnElementKey(statementPtr->GetValueInt64(0), elementId);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Shaun.Sewall                    03/2015
//---------------------------------------------------------------------------------------
BentleyStatus DgnElements::UpdateParentElementId(DgnElementId parentElementId, DgnElementId childElementId)
    {
    if (!parentElementId.IsValid() || !childElementId.IsValid())
        return BentleyStatus::ERROR;

    CachedStatementPtr statementPtr;
    m_dgndb.GetCachedStatement(statementPtr, "UPDATE " DGN_TABLE(DGN_CLASSNAME_Element) " SET ParentId=? WHERE Id=?");

    statementPtr->BindId(1, parentElementId);
    statementPtr->BindId(2, childElementId);

    return (BE_SQLITE_DONE != statementPtr->Step()) ? BentleyStatus::ERROR : BentleyStatus::SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                  Krischan.Eberle                    03/2015
//---------------------------------------------------------------------------------------
BentleyStatus DgnElements::InsertElementGroupsElements(DgnElementKeyCR groupElementKey, DgnElementKeyCR memberElementKey)
    {
    if (!groupElementKey.IsValid() || !memberElementKey.IsValid())
        return BentleyStatus::ERROR;

    CachedECSqlStatementPtr statementPtr = GetDgnDb().GetPreparedECSqlStatement
        ("INSERT INTO " DGN_SCHEMA(DGN_RELNAME_ElementGroupsElements) 
        " (SourceECClassId,SourceECInstanceId,TargetECClassId,TargetECInstanceId) VALUES (?,?,?,?)");

    if (!statementPtr.IsValid())
        return BentleyStatus::ERROR;

    statementPtr->BindInt64(1, groupElementKey.GetECClassId());
    statementPtr->BindId   (2, groupElementKey.GetECInstanceId());
    statementPtr->BindInt64(3, memberElementKey.GetECClassId());
    statementPtr->BindId   (4, memberElementKey.GetECInstanceId());

    return (ECSqlStepStatus::Done != statementPtr->Step()) ? BentleyStatus::ERROR : BentleyStatus::SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Shaun.Sewall                    03/2015
//---------------------------------------------------------------------------------------
DgnElementId DgnElements::QueryParentElementId(DgnElementId childElementId) const
    {
    if (!childElementId.IsValid())
        return DgnElementId();

    CachedStatementPtr statementPtr;
    m_dgndb.GetCachedStatement(statementPtr, "SELECT ParentId FROM " DGN_TABLE(DGN_CLASSNAME_Element) " WHERE Id=?");

    statementPtr->BindId(1, childElementId);

    return (BE_SQLITE_ROW != statementPtr->Step()) ? DgnElementId() : statementPtr->GetValueId<DgnElementId>(0);
    }
