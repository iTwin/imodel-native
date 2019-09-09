/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnV8OpenRoadsDesignerDomain.cpp $
|
|  $Copyright: (c) 2019 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "DgnV8OpenRoadsDesignerInternal.h"
#include <DgnV8OpenRoadsDesigner/DgnV8OpenRoadsDesignerDomain.h>

#include <DgnV8OpenRoadsDesigner/Handlers.h>

DOMAIN_DEFINE_MEMBERS(DgnV8OpenRoadsDesignerDomain)

/*---------------------------------------------------------------------------------**//**
* @bsimethod                              Diego.Diaz                              10/2018
+---------------+---------------+---------------+---------------+---------------+------*/
DgnV8OpenRoadsDesignerDomain::DgnV8OpenRoadsDesignerDomain() : DgnDomain(V8ORD_SCHEMA_NAME, "Bentley DgnV8OpenRoadsDesigner Domain", 2)
    {
    RegisterHandler(AlignmentAspectHandler::GetHandler());
    RegisterHandler(CorridorAspectHandler::GetHandler());
    RegisterHandler(CorridorSurfaceAspectHandler::GetHandler());
    RegisterHandler(DiscreteQuantityAspectHandler::GetHandler());
    RegisterHandler(FeatureAspectHandler::GetHandler());
    RegisterHandler(LinearQuantityAspectHandler::GetHandler());
    RegisterHandler(StationRangeAspectHandler::GetHandler());
    RegisterHandler(SuperelevationAspectHandler::GetHandler());
    RegisterHandler(TemplateDropAspectHandler::GetHandler());
    RegisterHandler(VolumetricQuantityAspectHandler::GetHandler());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      02/2018
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus DgnV8OpenRoadsDesignerDomain::SetGeometricElementAsBoundingContentForSheet(GeometricElementCR boundingElm, Sheet::ElementCR sheet)
    {
    if (!boundingElm.GetElementId().IsValid() || !sheet.GetElementId().IsValid())
        return DgnDbStatus::BadArg;

    ECInstanceKey insKey;
    if (DbResult::BE_SQLITE_OK != boundingElm.GetDgnDb().InsertLinkTableRelationship(insKey,
        *boundingElm.GetDgnDb().Schemas().GetClass(V8ORD_SCHEMA_NAME, V8ORD_REL_GeometricElementBoundsContentForSheet)->GetRelationshipClassCP(),
        ECInstanceId(boundingElm.GetElementId().GetValue()), ECInstanceId(sheet.GetElementId().GetValue())))
        return DgnDbStatus::WriteError;

    return DgnDbStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      02/2018
+---------------+---------------+---------------+---------------+---------------+------*/
DgnElementIdSet DgnV8OpenRoadsDesignerDomain::QueryElementIdsBoundingContentForSheets(DgnDbCR dgnDb)
    {
    auto stmtPtr = dgnDb.GetPreparedECSqlStatement("SELECT DISTINCT SourceECInstanceId FROM "
        V8ORD_SCHEMA(V8ORD_REL_GeometricElementBoundsContentForSheet) " WHERE TargetECInstanceId NOT IS NULL;");
    BeAssert(stmtPtr.IsValid());

    DgnElementIdSet retVal;
    while (DbResult::BE_SQLITE_ROW == stmtPtr->Step())
        retVal.insert(stmtPtr->GetValueId<DgnElementId>(0));

    return retVal;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      02/2018
+---------------+---------------+---------------+---------------+---------------+------*/
DgnElementIdSet DgnV8OpenRoadsDesignerDomain::QuerySheetIdsBoundedBy(GeometricElementCR boundingElm)
    {
    auto stmtPtr = boundingElm.GetDgnDb().GetPreparedECSqlStatement("SELECT TargetECInstanceId FROM "
        V8ORD_SCHEMA(V8ORD_REL_GeometricElementBoundsContentForSheet) " WHERE SourceECInstanceId = ?;");
    BeAssert(stmtPtr.IsValid());

    stmtPtr->BindId(1, boundingElm.GetElementId());

    DgnElementIdSet retVal;
    while (DbResult::BE_SQLITE_ROW == stmtPtr->Step())
        retVal.insert(stmtPtr->GetValueId<DgnElementId>(0));

    return retVal;
    }