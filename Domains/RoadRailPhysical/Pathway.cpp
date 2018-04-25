/*--------------------------------------------------------------------------------------+
|
|     $Source: Pathway.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "RoadRailPhysicalInternal.h"
#include <RoadRailPhysical/RoadRailPhysicalDomain.h>
#include <RoadRailPhysical/Pathway.h>
#include <RoadRailPhysical/TypicalSectionPoint.h>

HANDLER_DEFINE_MEMBERS(PathwayElementHandler)
HANDLER_DEFINE_MEMBERS(PathwayPortionElementHandler)
HANDLER_DEFINE_MEMBERS(RailwayHandler)
HANDLER_DEFINE_MEMBERS(RoadRailPhysicalModelHandler)
HANDLER_DEFINE_MEMBERS(RoadwayHandler)
HANDLER_DEFINE_MEMBERS(TravelPortionElementHandler)
HANDLER_DEFINE_MEMBERS(TravelSeparationPortionElementHandler)
HANDLER_DEFINE_MEMBERS(TravelPortionHandler)
HANDLER_DEFINE_MEMBERS(TravelSeparationPortionHandler)

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
* @bsimethod                                    Diego.Diaz                      01/2018
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus PathwayElement::AddRepresentedBy(PathwayElementCR pathway, GeometrySourceCR representedBy)
    {
    if (!representedBy.ToElement()->GetElementId().IsValid() || !pathway.GetElementId().IsValid())
        return DgnDbStatus::BadElement;

    Utf8String relClassName;
    if (representedBy.Is2d())
        relClassName = BRRP_REL_DrawingGraphicRepresentsPathway;
    else
        relClassName = BRRP_REL_GraphicalElement3dRepresentsPathway;

    ECInstanceKey insKey;
    if (DbResult::BE_SQLITE_OK != pathway.GetDgnDb().InsertLinkTableRelationship(insKey,
        *pathway.GetDgnDb().Schemas().GetClass(BRRP_SCHEMA_NAME, relClassName)->GetRelationshipClassCP(),
        ECInstanceId(representedBy.ToElement()->GetElementId().GetValue()), ECInstanceId(pathway.GetElementId().GetValue())))
        return DgnDbStatus::BadElement;

    return DgnDbStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      04/2018
+---------------+---------------+---------------+---------------+---------------+------*/
bool PathwayElement::QueryIsRepresentedBy(GeometrySourceCR geometrySource) const
    {
    Utf8String relClassName;
    if (geometrySource.Is2d())
        relClassName = BRRP_REL_DrawingGraphicRepresentsPathway;
    else
        relClassName = BRRP_REL_GraphicalElement3dRepresentsPathway;

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
DgnElementIdSet PathwayElement::QueryPortionIds() const
    {
    auto stmtPtr = GetDgnDb().GetPreparedECSqlStatement("SELECT ECInstanceId FROM "
        BRRP_SCHEMA(BRRP_CLASS_PathwayPortionElement) " WHERE Parent.Id = ?;");
    BeAssert(stmtPtr.IsValid());

    stmtPtr->BindId(1, GetElementId());

    DgnElementIdSet retVal;
    while (DbResult::BE_SQLITE_ROW == stmtPtr->Step())
        retVal.insert(stmtPtr->GetValueId<DgnElementId>(0));

    return retVal;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      04/2018
+---------------+---------------+---------------+---------------+---------------+------*/
PathwayElement::TravelSide getSideForRelClassId(DgnDbCR dgnDb, DgnClassId relClassId)
    {
    auto leftSideClassId = dgnDb.Schemas().GetClassId(BRRP_SCHEMA_NAME, BRRP_REL_PathwayRefersToLeftTravelPortion);
    auto rightSideClassId = dgnDb.Schemas().GetClassId(BRRP_SCHEMA_NAME, BRRP_REL_PathwayRefersToRightTravelPortion);
    auto singleSideClassId = dgnDb.Schemas().GetClassId(BRRP_SCHEMA_NAME, BRRP_REL_PathwayRefersToSingleTravelPortion);

    if (relClassId == leftSideClassId)
        return PathwayElement::TravelSide::Left;
    else if (relClassId == rightSideClassId)
        return PathwayElement::TravelSide::Right;
    else if (relClassId == singleSideClassId)
        return PathwayElement::TravelSide::Single;
    else
        {
        BeAssert(false); // Unknown side-type
        return PathwayElement::TravelSide::Single;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      04/2018
+---------------+---------------+---------------+---------------+---------------+------*/
DgnClassId getRelClassIdForSide(DgnDbCR dgnDb, PathwayElement::TravelSide side)
    {
    if (side == PathwayElement::TravelSide::Left)
        return dgnDb.Schemas().GetClassId(BRRP_SCHEMA_NAME, BRRP_REL_PathwayRefersToLeftTravelPortion);
    else if (side == PathwayElement::TravelSide::Right)
        return dgnDb.Schemas().GetClassId(BRRP_SCHEMA_NAME, BRRP_REL_PathwayRefersToRightTravelPortion);
    else if (side == PathwayElement::TravelSide::Single)
        return dgnDb.Schemas().GetClassId(BRRP_SCHEMA_NAME, BRRP_REL_PathwayRefersToSingleTravelPortion);
    else
        {
        BeAssert(false); // Unknown side-type
        return DgnClassId();
        }
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      04/2018
+---------------+---------------+---------------+---------------+---------------+------*/
bset<PathwayElement::TravelPortionInfo> PathwayElement::QueryTravelPortionInfos() const
    {
    auto stmtPtr = GetDgnDb().GetPreparedECSqlStatement("SELECT TargetECInstanceId, ECClassId FROM "
        BRRP_SCHEMA(BRRP_REL_PathwayRefersToTravelPortion) " WHERE SourceECInstanceId = ?;");
    BeAssert(stmtPtr.IsValid());

    stmtPtr->BindId(1, GetElementId());

    bset<TravelPortionInfo> retVal;
    while (DbResult::BE_SQLITE_ROW == stmtPtr->Step())
        retVal.insert(TravelPortionInfo(stmtPtr->GetValueId<DgnElementId>(0), getSideForRelClassId(GetDgnDb(), stmtPtr->GetValueId<DgnClassId>(1))));

    return retVal;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      04/2018
+---------------+---------------+---------------+---------------+---------------+------*/
DgnElementId PathwayElement::QueryTravelPortionId(TravelSide side) const
    {
    auto stmtPtr = GetDgnDb().GetPreparedECSqlStatement("SELECT TargetECInstanceId FROM "
        BRRP_SCHEMA(BRRP_REL_PathwayRefersToTravelPortion) " WHERE SourceECInstanceId = ? AND ECClassId = ?;");

    stmtPtr->BindId(1, GetElementId());
    stmtPtr->BindId(2, getRelClassIdForSide(GetDgnDb(), side));

    if (DbResult::BE_SQLITE_ROW == stmtPtr->Step())
        return stmtPtr->GetValueId<DgnElementId>(0);

    return DgnElementId();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      04/2018
+---------------+---------------+---------------+---------------+---------------+------*/
DgnElementId PathwayElement::QueryTravelSeparationId() const
    {
    auto stmtPtr = GetDgnDb().GetPreparedECSqlStatement("SELECT ECInstanceId FROM "
        BRRP_SCHEMA(BRRP_CLASS_TravelSeparationPortionElement) " WHERE Parent.Id = ?;");

    stmtPtr->BindId(1, GetElementId());

    if (DbResult::BE_SQLITE_ROW == stmtPtr->Step())
        return stmtPtr->GetValueId<DgnElementId>(0);

    return DgnElementId();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      09/2016
+---------------+---------------+---------------+---------------+---------------+------*/
RoadwayPtr Roadway::Create(PhysicalModelR model)
    {
    if (!model.GetModelId().IsValid())
        return nullptr;

    CreateParams createParams(model.GetDgnDb(), model.GetModelId(), QueryClassId(model.GetDgnDb()), 
        RoadRailCategory::GetRoadway(model.GetDgnDb()));

    return new Roadway(createParams);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      09/2016
+---------------+---------------+---------------+---------------+---------------+------*/
RailwayPtr Railway::Create(PhysicalModelR model)
    {
    if (!model.GetModelId().IsValid())
        return nullptr;

    CreateParams createParams(model.GetDgnDb(), model.GetModelId(), QueryClassId(model.GetDgnDb()), 
        RoadRailCategory::GetRailway(model.GetDgnDb()));

    return new Railway(createParams);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      09/2017
+---------------+---------------+---------------+---------------+---------------+------*/
CodeSpecId PathwayPortionElement::QueryCodeSpecId(DgnDbCR dgndb)
    {
    CodeSpecId codeSpecId = dgndb.CodeSpecs().QueryCodeSpecId(BRRP_CODESPEC_PathwayPortion);
    BeAssert(codeSpecId.IsValid());
    return codeSpecId;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      09/2017
+---------------+---------------+---------------+---------------+---------------+------*/
DgnCode PathwayPortionElement::CreateCode(PhysicalModelCR scope, Utf8StringCR value)
    {
    return CodeSpec::CreateCode(BRRP_CODESPEC_PathwayPortion, scope, value);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      09/2017
+---------------+---------------+---------------+---------------+---------------+------*/
PathwayPortionElementCPtr PathwayPortionElement::QueryByCode(PhysicalModelCR model, Utf8StringCR code)
    {
    auto id = model.GetDgnDb().Elements().QueryElementIdByCode(CreateCode(model, code));
    if (!id.IsValid())
        return nullptr;

    return PathwayPortionElement::Get(model.GetDgnDb(), id);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      04/2018
+---------------+---------------+---------------+---------------+---------------+------*/
PathwayElement::TravelSide TravelPortionElement::QueryTravelSide() const
    {
    auto stmtPtr = GetDgnDb().GetPreparedECSqlStatement("SELECT ECClassId FROM "
        BRRP_SCHEMA(BRRP_REL_PathwayRefersToTravelPortion) " WHERE TargetECInstanceId = ?;");
    BeAssert(stmtPtr.IsValid());

    stmtPtr->BindId(1, GetElementId());

    if (DbResult::BE_SQLITE_ROW == stmtPtr->Step())
        return getSideForRelClassId(GetDgnDb(), stmtPtr->GetValueId<DgnClassId>(0));

    BeAssert(false);
    return PathwayElement::TravelSide::Single;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      03/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TravelPortionPtr TravelPortion::Create(PathwayElementCR pathway, ILinearElementCR linearElement)
    {
    if (!pathway.GetElementId().IsValid() || !linearElement.ToElement().GetElementId().IsValid())
        return nullptr;

    CreateParams params(pathway.GetDgnDb(), pathway.GetModelId(), QueryClassId(pathway.GetDgnDb()), pathway.GetCategoryId());
    params.SetParentId(pathway.GetElementId(), 
        DgnClassId(pathway.GetDgnDb().Schemas().GetClassId(BRRP_SCHEMA_NAME, BRRP_REL_PathwayAssemblesElements)));

    TravelPortionPtr ptr(new TravelPortion(params));
    ptr->SetMainLinearElement(&linearElement);
    return ptr;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      04/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TravelPortionCPtr TravelPortion::Insert(PathwayElement::TravelSide side, Dgn::DgnDbStatus* status)
    {
    Utf8String relClassName;
    if (side == PathwayElement::TravelSide::Single)
        relClassName = BRRP_REL_PathwayRefersToSingleTravelPortion;
    else if (side == PathwayElement::TravelSide::Left)
        relClassName = BRRP_REL_PathwayRefersToLeftTravelPortion;
    else if (side == PathwayElement::TravelSide::Right)
        relClassName = BRRP_REL_PathwayRefersToRightTravelPortion;
    else
        {
        if (status) *status = DgnDbStatus::BadArg;
        return nullptr;
        }

    auto retVal = GetDgnDb().Elements().Insert<TravelPortion>(*this, status);

    ECInstanceKey key;
    GetDgnDb().InsertLinkTableRelationship(key, 
        *GetDgnDb().Schemas().GetClass(BRRP_SCHEMA_NAME, relClassName)->GetRelationshipClassCP(),
        retVal->GetParentId(), retVal->GetElementId());

    return retVal;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      03/2018
+---------------+---------------+---------------+---------------+---------------+------*/
TravelSeparationPortionPtr TravelSeparationPortion::Create(PathwayElementCR pathway, ILinearElementCR linearElement)
    {
    if (!pathway.GetElementId().IsValid())
        return nullptr;

    CreateParams params(pathway.GetDgnDb(), pathway.GetModelId(), QueryClassId(pathway.GetDgnDb()), pathway.GetCategoryId());
    params.SetParentId(pathway.GetElementId(), 
        DgnClassId(pathway.GetDgnDb().Schemas().GetClassId(BRRP_SCHEMA_NAME, BRRP_REL_PathwayAssemblesElements)));

    TravelSeparationPortionPtr ptr(new TravelSeparationPortion(params));
    ptr->SetMainLinearElement(&linearElement);
    return ptr;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      04/2018
+---------------+---------------+---------------+---------------+---------------+------*/
PathwayPortionElementCPtr ILinearElementUtilities::QueryRelatedPathwayPortion(ILinearElementCR linearElement,
    DgnElementId& significantPointDefId)
    {
    auto& linearElementCR = linearElement.ToElement();
    auto stmtPtr = linearElementCR.GetDgnDb().GetPreparedECSqlStatement("SELECT TargetECInstanceId, SignificantPointDef.Id FROM "
        BRRP_SCHEMA(BRRP_REL_ILinearElementRelatesToPathwayPortion) " WHERE SourceECInstanceId = ?;");
    BeAssert(stmtPtr.IsValid());

    stmtPtr->BindId(1, linearElementCR.GetElementId());
    if (DbResult::BE_SQLITE_ROW == stmtPtr->Step())
        {
        significantPointDefId = stmtPtr->GetValueId<DgnElementId>(1);
        return PathwayPortionElement::Get(linearElementCR.GetDgnDb(), stmtPtr->GetValueId<DgnElementId>(0));
        }

    return nullptr;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      03/2018
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus ILinearElementUtilities::SetRelatedPathwayPortion(ILinearElementCR linearElement, PathwayPortionElementCR pathwayPortion,
    SignificantPointDefinitionCR significantPointDef)
    {
    if (!linearElement.ToElement().GetElementId().IsValid() || 
        !pathwayPortion.GetElementId().IsValid() ||
        !significantPointDef.GetElementId().IsValid())
        return DgnDbStatus::BadArg;

    auto& linearElementCR = linearElement.ToElement();

    auto stmtPtr = linearElementCR.GetDgnDb().GetPreparedECSqlStatement("SELECT ECClassId, ECInstanceId FROM "
        BRRP_SCHEMA(BRRP_REL_ILinearElementRelatesToPathwayPortion) " WHERE SourceECInstanceId = ?;");
    BeAssert(stmtPtr.IsValid());

    stmtPtr->BindId(1, linearElementCR.GetElementId());
    if (DbResult::BE_SQLITE_ROW == stmtPtr->Step())
        linearElementCR.GetDgnDb().DeleteLinkTableRelationship(ECInstanceKey(stmtPtr->GetValueId<ECClassId>(0), stmtPtr->GetValueId<ECInstanceId>(1)));

    auto relClassCP = linearElementCR.GetDgnDb().Schemas().GetClass(BRRP_SCHEMA_NAME, BRRP_REL_ILinearElementRelatesToPathwayPortion)->GetRelationshipClassCP();
    auto relEnablerPtr = StandaloneECRelationshipEnabler::CreateStandaloneRelationshipEnabler(*relClassCP);
    auto instancePtr = relEnablerPtr->CreateRelationshipInstance();
    instancePtr->SetValue("SignificantPointDef", ECValue(significantPointDef.GetElementId()));

    ECInstanceKey key;
    if (DbResult::BE_SQLITE_OK != linearElementCR.GetDgnDb().InsertLinkTableRelationship(key,
        *relClassCP, linearElementCR.GetElementId(), pathwayPortion.GetElementId(), instancePtr.get()))
            return DgnDbStatus::WriteError;

    return DgnDbStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      05/2017
+---------------+---------------+---------------+---------------+---------------+------*/
RoadRailPhysicalModelPtr RoadRailPhysicalModel::Query(Dgn::SubjectCR parentSubject)
    {
    DgnDbR db = parentSubject.GetDgnDb();
    DgnCode partitionCode = PhysicalPartition::CreateCode(parentSubject, RoadRailPhysicalDomain::GetDefaultPhysicalPartitionName());
    DgnElementId partitionId = db.Elements().QueryElementIdByCode(partitionCode);
    PhysicalPartitionCPtr partition = db.Elements().Get<PhysicalPartition>(partitionId);
    if (!partition.IsValid())
        return nullptr;
    return dynamic_cast<RoadRailPhysicalModelP>(partition->GetSubModel().get());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      04/2018
+---------------+---------------+---------------+---------------+---------------+------*/
SubjectCPtr RoadRailPhysicalModel::GetParentSubject() const
    {
    auto partitionCP = dynamic_cast<PhysicalPartitionCP>(GetModeledElement().get());
    BeAssert(partitionCP != nullptr);

    return GetDgnDb().Elements().Get<Subject>(partitionCP->GetParentId());
    }