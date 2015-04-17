/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnCore/DgnItem.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <DgnPlatformInternal.h>

//---------------------------------------------------------------------------------------
// @bsimethod                                   Sam.Wilson                      05/2015
//---------------------------------------------------------------------------------------
DgnClassId ElementItemHandler::GetItemClassId(DgnDbR db)
    {
    return db.Domains().GetClassId(*this);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Sam.Wilson                      05/2015
//---------------------------------------------------------------------------------------
ElementItemHandler* ElementItemHandler::GetItemHandler(DgnDbR db, DgnClassId itemClassId)
    {
    DgnDomain::Handler* handler = db.Domains().FindHandler(itemClassId, GetItemClassId(db));
    return handler ? handler->_ToElementItemHandler() : nullptr;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Brien.Bastings                  01/2015
//---------------------------------------------------------------------------------------
BentleyStatus ElementItemHandler::InsertElementGeomUsesParts(DgnDbR db, DgnElementId elementId, PhysicalGeometryCR physicalGeometry)
    {
    for (PlacedGeomPart part : physicalGeometry)
        {
        if (BentleyStatus::SUCCESS != db.GeomParts().InsertElementGeomUsesParts(elementId, part.GetPartPtr()->GetId()))
            return BentleyStatus::ERROR;
        }

    return BentleyStatus::SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Shaun.Sewall                    10/2014
//---------------------------------------------------------------------------------------
BentleyStatus DgnItems::DeleteItem(DgnElementId elementId)
    {
    if (!elementId.IsValid())
        return BentleyStatus::ERROR;
                                                // *** WIP_ECSQL_BUG: Polymorphism is not yet supported in ECSQL DELETE statements
    CachedECSqlStatementPtr statementPtr = GetDgnDb().GetPreparedECSqlStatement("DELETE FROM ONLY " DGN_SCHEMA(DGN_CLASSNAME_ElementItem) " WHERE ECInstanceId=?");
    if (!statementPtr.IsValid())
        return BentleyStatus::ERROR;

    if (ECSqlStatus::Success != statementPtr->BindId(1, elementId))
        return BentleyStatus::ERROR;

    return (ECSqlStepStatus::Done != statementPtr->Step()) ? BentleyStatus::ERROR : BentleyStatus::SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Shaun.Sewall                    02/2015
//---------------------------------------------------------------------------------------
ElementItemKey DgnItems::QueryItemKey(DgnElementId elementId)
    {
    if (!elementId.IsValid())
        return ElementItemKey();

    CachedECSqlStatementPtr statementPtr = GetDgnDb().GetPreparedECSqlStatement("SELECT GetECClassId() FROM " DGN_SCHEMA(DGN_CLASSNAME_ElementItem) " WHERE ECInstanceId=?");
    if (!statementPtr.IsValid())
        return ElementItemKey();

    statementPtr->BindId(1, elementId);

    if (ECSqlStepStatus::HasRow != statementPtr->Step())
        return ElementItemKey();

    return ElementItemKey(statementPtr->GetValueInt64(0), elementId);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Shaun.Sewall                    01/2015
//---------------------------------------------------------------------------------------
PhysicalGeometryPtr DgnItems::QueryPhysicalGeometry(ElementItemKeyCR itemKey)
    {
    // WIP_ELEMENTGEOM_REFACTOR : Need to read from ElementGeom.Geom stream to load PhysicalGeometry
    BeAssert(false);
    return nullptr;
    }

#if defined (NEEDS_WORK_ELEMDSCR_REWORK)
//---------------------------------------------------------------------------------------
// @bsimethod                                   Shaun.Sewall                    01/2015
//---------------------------------------------------------------------------------------
BentleyStatus DgnItems::QueryItemPlacement(DPoint3dR origin, YawPitchRollAnglesR angles, ElementItemKeyCR itemKey)
    {
    CachedStatementPtr statementPtr;
    GetDgnDb().GetCachedStatement(statementPtr, "SELECT Placement FROM " DGN_TABLE(DGN_CLASSNAME_ElementGeom) " WHERE ElementId=?");
    statementPtr->BindId(1, itemKey.GetElementId());
        
    if (BE_SQLITE_ROW != statementPtr->Step())
        return BentleyStatus::ERROR;

    return DbElementAndGeomReader::GetPlacement(origin, angles, *statementPtr, 0);
    }
#endif
