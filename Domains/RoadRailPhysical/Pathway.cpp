/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#include "RoadRailPhysicalInternal.h"
#include <RoadRailPhysical/RoadRailPhysicalDomain.h>
#include <RoadRailPhysical/Pathway.h>
#include <RoadRailPhysical/TypicalSectionPoint.h>

HANDLER_DEFINE_MEMBERS(CorridorHandler)
HANDLER_DEFINE_MEMBERS(CorridorPortionElementHandler)
HANDLER_DEFINE_MEMBERS(PathwayElementHandler)
HANDLER_DEFINE_MEMBERS(RailwayHandler)
HANDLER_DEFINE_MEMBERS(RoadwayHandler)


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      09/2016
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus IMainLinearElementSource::SetMainLinearElement(ILinearElementCP linearElement)
    {
    auto& dgnElement = *const_cast<DgnElementP>(&_ILinearElementSourceToDgnElement());
    if (linearElement)
        return dgnElement.SetPropertyValue("MainLinearElement", linearElement->ToElement().GetElementId(), linearElement->ToElement().GetElementClassId());
    else
        return dgnElement.SetPropertyValue("MainLinearElement", DgnElementId(), DgnClassId());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      05/2018
+---------------+---------------+---------------+---------------+---------------+------*/
DgnElementIdSet IMainLinearElementSource::QueryLinearElementSourceIds(ILinearElementCR mainLinearElement, DgnClassId filterBaseClassId)
    {
    auto& elmCR = mainLinearElement.ToElement();

    Utf8String ecSql = "SELECT iMainLE.SourceECInstanceId FROM "
        BRRP_SCHEMA(BRRP_REL_ILinearElementSourceRefersToMainLinearElement) " iMainLE";

    if (filterBaseClassId.IsValid())
        {
        ecSql.append(", meta.ClassHasAllBaseClasses WHERE ");
        ecSql.append("iMainLE.TargetECInstanceId = ? AND ");
        ecSql.append("meta.ClassHasAllBaseClasses.SourceECInstanceId = iMainLE.SourceECClassId AND ");
        ecSql.append("meta.ClassHasAllBaseClasses.TargetECInstanceId = ?;");
        }
    else
        ecSql.append(" WHERE iMainLE.TargetECInstanceId = ?;");

    ECSqlStatement stmt;
    stmt.Prepare(elmCR.GetDgnDb(), ecSql.c_str());
    BeAssert(stmt.IsPrepared());

    stmt.BindId(1, elmCR.GetElementId());

    if (filterBaseClassId.IsValid())
        stmt.BindId(2, filterBaseClassId);

    DgnElementIdSet retVal;
    while (DbResult::BE_SQLITE_ROW == stmt.Step())
        retVal.insert(stmt.GetValueId<DgnElementId>(0));

    return retVal;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      09/2017
+---------------+---------------+---------------+---------------+---------------+------*/
CodeSpecId Corridor::QueryCodeSpecId(DgnDbCR dgndb)
    {
    CodeSpecId codeSpecId = dgndb.CodeSpecs().QueryCodeSpecId(BRRP_CODESPEC_Corridor);
    BeAssert(codeSpecId.IsValid());
    return codeSpecId;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      09/2017
+---------------+---------------+---------------+---------------+---------------+------*/
DgnCode Corridor::CreateCode(PhysicalModelCR scope, Utf8StringCR value)
    {
    return CodeSpec::CreateCode(BRRP_CODESPEC_Corridor, scope, value);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      09/2017
+---------------+---------------+---------------+---------------+---------------+------*/
CorridorCPtr Corridor::QueryByCode(PhysicalModelCR model, Utf8StringCR code)
    {
    auto id = model.GetDgnDb().Elements().QueryElementIdByCode(CreateCode(model, code));
    if (!id.IsValid())
        return nullptr;

    return Corridor::Get(model.GetDgnDb(), id);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      01/2018
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus Corridor::AddRepresentedBy(CorridorCR corridor, GeometrySourceCR representedBy)
    {
    if (!representedBy.ToElement()->GetElementId().IsValid() || !corridor.GetElementId().IsValid())
        return DgnDbStatus::BadElement;

    Utf8String relClassName;
    if (representedBy.Is2d())
        relClassName = BRRP_REL_DrawingGraphicRepresentsCorridor;
    else
        relClassName = BRRP_REL_GraphicalElement3dRepresentsCorridor;

    ECInstanceKey insKey;
    if (DbResult::BE_SQLITE_OK != corridor.GetDgnDb().InsertLinkTableRelationship(insKey,
        *corridor.GetDgnDb().Schemas().GetClass(BRRP_SCHEMA_NAME, relClassName)->GetRelationshipClassCP(),
        ECInstanceId(representedBy.ToElement()->GetElementId().GetValue()), ECInstanceId(corridor.GetElementId().GetValue())))
        return DgnDbStatus::BadElement;

    return DgnDbStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      04/2018
+---------------+---------------+---------------+---------------+---------------+------*/
bool Corridor::QueryIsRepresentedBy(GeometrySourceCR geometrySource) const
    {
    Utf8String relClassName;
    if (geometrySource.Is2d())
        relClassName = BRRP_REL_DrawingGraphicRepresentsCorridor;
    else
        relClassName = BRRP_REL_GraphicalElement3dRepresentsCorridor;

    auto ecsql = Utf8PrintfString("SELECT ECInstanceId FROM %s.%s WHERE SourceECInstanceId = ? AND TargetECInstanceId = ?;", 
        BRRP_SCHEMA_NAME, relClassName.c_str());
    auto stmtPtr = GetDgnDb().GetPreparedECSqlStatement(ecsql.c_str());
    BeAssert(stmtPtr.IsValid());

    stmtPtr->BindId(1, geometrySource.ToElement()->GetElementId());
    stmtPtr->BindId(2, GetElementId());

    if (DbResult::BE_SQLITE_ROW == stmtPtr->Step())
        return true;

    return false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      04/2018
+---------------+---------------+---------------+---------------+---------------+------*/
bvector<DgnElementId> Corridor::QueryOrderedPathwayIds() const
    {
    auto stmtPtr = GetDgnDb().GetPreparedECSqlStatement("SELECT TargetECInstanceId, ECClassId FROM "
        BRRP_SCHEMA(BRRP_REL_CorridorRefersToOrderedPathways) " WHERE SourceECInstanceId = ? ORDER BY MemberPriority;");
    BeAssert(stmtPtr.IsValid());

    stmtPtr->BindId(1, GetElementId());

    bvector<DgnElementId> retVal;
    while (DbResult::BE_SQLITE_ROW == stmtPtr->Step())
        retVal.push_back(stmtPtr->GetValueId<DgnElementId>(0));

    return retVal;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      09/2016
+---------------+---------------+---------------+---------------+---------------+------*/
CorridorPtr Corridor::Create(PhysicalModelR model)
    {
    if (!model.GetModelId().IsValid())
        return nullptr;

    CreateParams createParams(model.GetDgnDb(), model.GetModelId(), QueryClassId(model.GetDgnDb()),
        RoadRailCategory::GetCorridor(model.GetDgnDb()));

    return new Corridor(createParams);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      09/2017
+---------------+---------------+---------------+---------------+---------------+------*/
CodeSpecId PathwayElement::QueryCodeSpecId(DgnDbCR dgndb)
    {
    CodeSpecId codeSpecId = dgndb.CodeSpecs().QueryCodeSpecId(BRRP_CODESPEC_Pathway);
    BeAssert(codeSpecId.IsValid());
    return codeSpecId;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      09/2017
+---------------+---------------+---------------+---------------+---------------+------*/
DgnCode PathwayElement::CreateCode(PhysicalModelCR scope, Utf8StringCR value)
    {
    return CodeSpec::CreateCode(BRRP_CODESPEC_Pathway, scope, value);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      09/2017
+---------------+---------------+---------------+---------------+---------------+------*/
PathwayElementCPtr PathwayElement::QueryByCode(PhysicalModelCR model, Utf8StringCR code)
    {
    auto id = model.GetDgnDb().Elements().QueryElementIdByCode(CreateCode(model, code));
    if (!id.IsValid())
        return nullptr;

    return PathwayElement::Get(model.GetDgnDb(), id);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      04/2018
+---------------+---------------+---------------+---------------+---------------+------*/
PathwayElementCPtr PathwayElement::Insert(Order order, DgnDbStatus* status)
    {
    auto retVal = GetDgnDb().Elements().Insert<PathwayElement>(*this, status);

    auto relClassCP = GetDgnDb().Schemas().GetClass(BRRP_SCHEMA_NAME, BRRP_REL_CorridorRefersToOrderedPathways)->GetRelationshipClassCP();
    auto relEnablerPtr = StandaloneECRelationshipEnabler::CreateStandaloneRelationshipEnabler(*relClassCP);
    auto relInstPtr = relEnablerPtr->CreateRelationshipInstance();
    relInstPtr->SetValue("MemberPriority", ECValue(order));

    ECInstanceKey key;
    GetDgnDb().InsertLinkTableRelationship(key, *relClassCP, retVal->GetParentId(), retVal->GetElementId(), relInstPtr.get());

    return retVal;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      09/2016
+---------------+---------------+---------------+---------------+---------------+------*/
RoadwayPtr Roadway::Create(CorridorCR corridor)
    {
    if (!corridor.GetElementId().IsValid())
        return nullptr;

    CreateParams createParams(corridor.GetDgnDb(), corridor.GetModelId(), QueryClassId(corridor.GetDgnDb()),
        RoadRailCategory::GetRoadway(corridor.GetDgnDb()));
    createParams.m_parentId = corridor.GetElementId();
    createParams.m_parentRelClassId = corridor.GetDgnDb().Schemas().GetClassId(BRRP_SCHEMA_NAME, BRRP_REL_CorridorAssemblesPortions);

    return new Roadway(createParams);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      09/2016
+---------------+---------------+---------------+---------------+---------------+------*/
RailwayPtr Railway::Create(CorridorCR corridor)
    {
    if (!corridor.GetElementId().IsValid())
        return nullptr;

    CreateParams createParams(corridor.GetDgnDb(), corridor.GetModelId(), QueryClassId(corridor.GetDgnDb()),
        RoadRailCategory::GetRailway(corridor.GetDgnDb()));
    createParams.m_parentId = corridor.GetElementId();
    createParams.m_parentRelClassId = corridor.GetDgnDb().Schemas().GetClassId(BRRP_SCHEMA_NAME, BRRP_REL_CorridorAssemblesPortions);

    return new Railway(createParams);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      04/2018
+---------------+---------------+---------------+---------------+---------------+------*/
CorridorPortionElementCPtr ILinearElementUtilities::QueryRelatedCorridorPortion(ILinearElementCR linearElement,
    DgnElementId& typicalSectionPointDefId)
    {
    auto& linearElementCR = linearElement.ToElement();
    auto stmtPtr = linearElementCR.GetDgnDb().GetPreparedECSqlStatement("SELECT TargetECInstanceId, TypicalSectionPointDef.Id FROM "
        BRRP_SCHEMA(BRRP_REL_ILinearElementRelatesToCorridorPortion) " WHERE SourceECInstanceId = ?;");
    BeAssert(stmtPtr.IsValid());

    stmtPtr->BindId(1, linearElementCR.GetElementId());
    if (DbResult::BE_SQLITE_ROW == stmtPtr->Step())
        {
        typicalSectionPointDefId = stmtPtr->GetValueId<DgnElementId>(1);
        return CorridorPortionElement::Get(linearElementCR.GetDgnDb(), stmtPtr->GetValueId<DgnElementId>(0));
        }

    return nullptr;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      03/2018
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus ILinearElementUtilities::SetRelatedCorridorPortion(ILinearElementCR linearElement, CorridorPortionElementCR corridorPortion,
    TypicalSectionPointDefinitionCR significantPointDef)
    {
    if (!linearElement.ToElement().GetElementId().IsValid() || 
        !corridorPortion.GetElementId().IsValid() ||
        !significantPointDef.GetElementId().IsValid())
        return DgnDbStatus::BadArg;

    auto& linearElementCR = linearElement.ToElement();

    auto stmtPtr = linearElementCR.GetDgnDb().GetPreparedECSqlStatement("SELECT ECClassId, ECInstanceId FROM "
        BRRP_SCHEMA(BRRP_REL_ILinearElementRelatesToCorridorPortion) " WHERE SourceECInstanceId = ?;");
    BeAssert(stmtPtr.IsValid());

    stmtPtr->BindId(1, linearElementCR.GetElementId());
    if (DbResult::BE_SQLITE_ROW == stmtPtr->Step())
        linearElementCR.GetDgnDb().DeleteLinkTableRelationship(ECInstanceKey(stmtPtr->GetValueId<ECClassId>(0), stmtPtr->GetValueId<ECInstanceId>(1)));

    auto relClassCP = linearElementCR.GetDgnDb().Schemas().GetClass(BRRP_SCHEMA_NAME, BRRP_REL_ILinearElementRelatesToCorridorPortion)->GetRelationshipClassCP();
    auto relEnablerPtr = StandaloneECRelationshipEnabler::CreateStandaloneRelationshipEnabler(*relClassCP);
    auto instancePtr = relEnablerPtr->CreateRelationshipInstance();
    instancePtr->SetValue("TypicalSectionPointDef", ECValue(significantPointDef.GetElementId()));

    ECInstanceKey key;
    if (DbResult::BE_SQLITE_OK != linearElementCR.GetDgnDb().InsertLinkTableRelationship(key,
        *relClassCP, linearElementCR.GetElementId(), corridorPortion.GetElementId(), instancePtr.get()))
            return DgnDbStatus::WriteError;

    return DgnDbStatus::Success;
    }