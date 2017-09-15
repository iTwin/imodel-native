/*--------------------------------------------------------------------------------------+
|
|     $Source: TypicalSection.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "RoadRailPhysicalInternal.h"
#include <RoadRailPhysical/TypicalSection.h>
#include <RoadRailPhysical/TypicalSectionPoint.h>
#include <RoadRailPhysical/RoadRailCategory.h>


HANDLER_DEFINE_MEMBERS(OverallTypicalSectionHandler)
HANDLER_DEFINE_MEMBERS(OverallTypicalSectionBreakDownModelHandler)
HANDLER_DEFINE_MEMBERS(RoadLaneComponentHandler)
HANDLER_DEFINE_MEMBERS(RoadTravelwayDefinitionHandler)
HANDLER_DEFINE_MEMBERS(TravelwayComponentElementHandler)
HANDLER_DEFINE_MEMBERS(TravelwayDefinitionElementHandler)
HANDLER_DEFINE_MEMBERS(TravelwaySideComponentHandler)
HANDLER_DEFINE_MEMBERS(TravelwaySideDefinitionHandler)
HANDLER_DEFINE_MEMBERS(TravelwayStructureComponentHandler)
HANDLER_DEFINE_MEMBERS(TravelwayStructureDefinitionHandler)
HANDLER_DEFINE_MEMBERS(TypicalSectionComponentElementHandler)
HANDLER_DEFINE_MEMBERS(TypicalSectionPortionBreakDownModelHandler)
HANDLER_DEFINE_MEMBERS(TypicalSectionPortionElementHandler)

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      10/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TypicalSectionPortionElement::TypicalSectionPortionElement(CreateParams const& params):
    T_Super(params)
    {
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      10/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TravelwayDefinitionElement::TravelwayDefinitionElement(CreateParams const& params):
    T_Super(params)
    {
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      10/2016
+---------------+---------------+---------------+---------------+---------------+------*/
RoadTravelwayDefinition::RoadTravelwayDefinition(CreateParams const& params):
    T_Super(params)
    {
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      10/2016
+---------------+---------------+---------------+---------------+---------------+------*/
CodeSpecId RoadTravelwayDefinition::QueryCodeSpecId(DgnDbCR dgndb)
    {
    CodeSpecId codeSpecId = dgndb.CodeSpecs().QueryCodeSpecId(BRRP_CODESPEC_RoadTravelway);
    BeAssert(codeSpecId.IsValid());
    return codeSpecId;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      10/2016
+---------------+---------------+---------------+---------------+---------------+------*/
DgnCode RoadTravelwayDefinition::CreateCode(DefinitionModelCR scope, Utf8StringCR value)
    {
    return CodeSpec::CreateCode(BRRP_CODESPEC_RoadTravelway, scope, value);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      10/2016
+---------------+---------------+---------------+---------------+---------------+------*/
RoadTravelwayDefinitionPtr RoadTravelwayDefinition::Create(DefinitionModelCR model, Utf8StringCR code)
    {
    if (!model.GetModelId().IsValid())
        return nullptr;

    CreateParams createParams(model.GetDgnDb(), model.GetModelId(), QueryClassId(model.GetDgnDb()), CreateCode(model, code));

    return new RoadTravelwayDefinition(createParams);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      10/2016
+---------------+---------------+---------------+---------------+---------------+------*/
RoadTravelwayDefinitionCPtr RoadTravelwayDefinition::Insert(TypicalSectionPortionBreakDownModelPtr& breakDownModelPtr, DgnDbStatus* stat)
    {
    auto retVal = GetDgnDb().Elements().Insert<RoadTravelwayDefinition>(*this, stat);
    if (retVal.IsNull())
        return nullptr;

    breakDownModelPtr = TypicalSectionPortionBreakDownModel::Create(TypicalSectionPortionBreakDownModel::CreateParams(GetDgnDb(), retVal->GetElementId()));
    breakDownModelPtr->SetIsTemplate(true);

    DgnDbStatus status;
    if (DgnDbStatus::Success != (status = breakDownModelPtr->Insert()))
        {
        if (stat) *stat = status;
        return nullptr;
        }

    return retVal;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      04/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TravelwaySideDefinition::TravelwaySideDefinition(CreateParams const& params):
    T_Super(params)
    {
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      04/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TravelwaySideDefinitionCPtr TravelwaySideDefinition::Insert(TypicalSectionPortionBreakDownModelPtr& breakDownModelPtr, DgnDbStatus* stat)
    {
    auto retVal = GetDgnDb().Elements().Insert<TravelwaySideDefinition>(*this, stat);
    if (retVal.IsNull())
        return nullptr;

    breakDownModelPtr = TypicalSectionPortionBreakDownModel::Create(TypicalSectionPortionBreakDownModel::CreateParams(GetDgnDb(), retVal->GetElementId()));
    breakDownModelPtr->SetIsTemplate(true);

    DgnDbStatus status;
    if (DgnDbStatus::Success != (status = breakDownModelPtr->Insert()))
        {
        if (stat) *stat = status;
        return nullptr;
        }

    return retVal;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      04/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TravelwayStructureDefinition::TravelwayStructureDefinition(CreateParams const& params):
    T_Super(params)
    {
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      04/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TravelwayStructureDefinitionCPtr TravelwayStructureDefinition::Insert(TypicalSectionPortionBreakDownModelPtr& breakDownModelPtr, DgnDbStatus* stat)
    {
    auto retVal = GetDgnDb().Elements().Insert<TravelwayStructureDefinition>(*this, stat);
    if (retVal.IsNull())
        return nullptr;

    breakDownModelPtr = TypicalSectionPortionBreakDownModel::Create(TypicalSectionPortionBreakDownModel::CreateParams(GetDgnDb(), retVal->GetElementId()));
    breakDownModelPtr->SetIsTemplate(true);

    DgnDbStatus status;
    if (DgnDbStatus::Success != (status = breakDownModelPtr->Insert()))
        {
        if (stat) *stat = status;
        return nullptr;
        }

    return retVal;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      08/2017
+---------------+---------------+---------------+---------------+---------------+------*/
OverallTypicalSection::OverallTypicalSection(CreateParams const& params):
    T_Super(params)
    {
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      08/2017
+---------------+---------------+---------------+---------------+---------------+------*/
OverallTypicalSectionPtr OverallTypicalSection::Create(DefinitionModelCR model, Utf8StringCR code)
    {
    if (!model.GetModelId().IsValid())
        return nullptr;

    CreateParams createParams(model.GetDgnDb(), model.GetModelId(), QueryClassId(model.GetDgnDb()), CreateCode(model, code));

    return new OverallTypicalSection(createParams);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      08/2017
+---------------+---------------+---------------+---------------+---------------+------*/
DgnCode OverallTypicalSection::CreateCode(DefinitionModelCR scope, Utf8StringCR value)
    {
    return CodeSpec::CreateCode(BRRP_CODESPEC_OverallTypicalSection, scope, value);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      04/2017
+---------------+---------------+---------------+---------------+---------------+------*/
OverallTypicalSectionCPtr OverallTypicalSection::Insert(OverallTypicalSectionBreakDownModelPtr& breakDownModelPtr, DgnDbStatus* stat)
    {
    auto retVal = GetDgnDb().Elements().Insert<OverallTypicalSection>(*this, stat);
    if (retVal.IsNull())
        return nullptr;

    breakDownModelPtr = OverallTypicalSectionBreakDownModel::Create(OverallTypicalSectionBreakDownModel::CreateParams(GetDgnDb(), retVal->GetElementId()));
    breakDownModelPtr->SetIsTemplate(true);

    DgnDbStatus status;
    if (DgnDbStatus::Success != (status = breakDownModelPtr->Insert()))
        {
        if (stat) *stat = status;
        return nullptr;
        }

    return retVal;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      08/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TypicalSectionComponentElement::TypicalSectionComponentElement(CreateParams const& params):
    T_Super(params)
    {
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      09/2017
+---------------+---------------+---------------+---------------+---------------+------*/
bvector<DgnElementId> TypicalSectionComponentElement::QueryPointIds() const
    {
    auto stmtPtr = GetDgnDb().GetPreparedECSqlStatement("SELECT TargetECInstanceId FROM "
        BRRP_SCHEMA(BRRP_REL_TypicalSectionComponentGroupsPoints) " WHERE SourceECInstanceId = ? ORDER BY MemberPriority;");
    BeAssert(stmtPtr.IsValid());

    stmtPtr->BindId(1, GetElementId());

    bvector<DgnElementId> retVal;
    while (DbResult::BE_SQLITE_ROW == stmtPtr->Step())
        retVal.push_back(stmtPtr->GetValueId<DgnElementId>(0));

    return retVal;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      09/2017
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus TypicalSectionComponentElement::SetPoints(TypicalSectionComponentElementCR component, bvector<TypicalSectionPointCP> const& points)
    {
    auto pointIds = component.QueryPointIds();
    bvector<size_t> pointIndices;
    pointIndices.resize(pointIds.size(), 0); // 1-based indices. Zero means unknown.

    auto originalPointIdSize = pointIds.size();
    for (size_t index = 0; index < points.size(); index++)
        {
        auto pointCP = points[index];
        if (!pointCP)
            return DgnDbStatus::BadArg;

        auto iter = std::find(pointIds.begin(), pointIds.end(), pointCP->GetElementId());
        if (iter == pointIds.end()) // New point
            {
            pointIds.push_back(pointCP->GetElementId());
            pointIndices.push_back(index + 1);
            }
        else
            pointIndices[index] = std::distance(pointIds.begin(), iter);
        }

    auto rowIdStmtPtr = component.GetDgnDb().GetPreparedECSqlStatement("SELECT ECClassId, ECInstanceId FROM "
        BRRP_SCHEMA(BRRP_REL_TypicalSectionComponentGroupsPoints) " WHERE SourceECInstanceId = ? AND TargetECInstanceId = ?;");
    BeAssert(rowIdStmtPtr.IsValid());

    auto relClassCP = component.GetDgnDb().Schemas().GetClass(BRRP_SCHEMA_NAME, BRRP_REL_TypicalSectionComponentGroupsPoints)->GetRelationshipClassCP();;
    auto relClassEnablerP = relClassCP->GetDefaultStandaloneEnabler();
    auto relInstancePtr = relClassEnablerP->CreateInstance();

    for (size_t index = 0; index < pointIndices.size(); index++)
        {
        if (pointIndices[index] - 1 == index) // Existing row, position didn't change
            continue;

        if (index >= originalPointIdSize) // New row
            {
            ECInstanceKey newInstanceKey;
            relInstancePtr->SetValue("MemberPriority", ECValue(static_cast<int>(pointIndices[index] - 1)));
            if (DbResult::BE_SQLITE_OK != component.GetDgnDb().InsertLinkTableRelationship(newInstanceKey, *relClassCP,
                component.GetElementId(), pointIds[index], 
                dynamic_cast<IECRelationshipInstanceP>(relInstancePtr.get())))
                    return DgnDbStatus::WriteError;

            continue;
            }

        rowIdStmtPtr->BindId(1, component.GetElementId());
        rowIdStmtPtr->BindId(2, pointIds[index]);
        if (DbResult::BE_SQLITE_ROW != rowIdStmtPtr->Step())
            return DgnDbStatus::InvalidId;

        ECInstanceKey instanceKey(rowIdStmtPtr->GetValueId<ECClassId>(0), rowIdStmtPtr->GetValueId<ECInstanceId>(1));
        if (pointIndices[index] == 0) // Delete row
            {
            if (DbResult::BE_SQLITE_OK != component.GetDgnDb().DeleteLinkTableRelationship(instanceKey))
                return DgnDbStatus::WriteError;
            }
        else // Existing row, different position
            {
            relInstancePtr->SetValue("MemberPriority", ECValue(static_cast<int>(pointIndices[index] - 1)));
            if (DbResult::BE_SQLITE_OK != component.GetDgnDb().UpdateLinkTableRelationshipProperties(instanceKey, *relInstancePtr))
                return DgnDbStatus::WriteError;
            }
        }

    return DgnDbStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      08/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TravelwayStructureComponent::TravelwayStructureComponent(CreateParams const& params):
    T_Super(params)
    {
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      08/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TravelwayStructureComponentPtr TravelwayStructureComponent::Create(TypicalSectionPortionBreakDownModelCR model)
    {
    if (!model.GetModelId().IsValid())
        return nullptr;

    CreateParams createParams(model.GetDgnDb(), model.GetModelId(), QueryClassId(model.GetDgnDb()), 
        RoadRailCategory::GetTravelwayStructureDefComponent(model.GetDgnDb()));

    return new TravelwayStructureComponent(createParams);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      08/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TravelwayComponentElement::TravelwayComponentElement(CreateParams const& params):
    T_Super(params)
    {
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      08/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TravelwaySideComponent::TravelwaySideComponent(CreateParams const& params):
    T_Super(params)
    {
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      08/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TravelwaySideComponentPtr TravelwaySideComponent::Create(TypicalSectionPortionBreakDownModelCR model)
    {
    if (!model.GetModelId().IsValid())
        return nullptr;

    CreateParams createParams(model.GetDgnDb(), model.GetModelId(), QueryClassId(model.GetDgnDb()),
        RoadRailCategory::GetTravelwaySideDefComponent(model.GetDgnDb()));

    return new TravelwaySideComponent(createParams);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      08/2017
+---------------+---------------+---------------+---------------+---------------+------*/
RoadLaneComponentPtr RoadLaneComponent::Create(TypicalSectionPortionBreakDownModelCR model)
    {
    if (!model.GetModelId().IsValid())
        return nullptr;

    // ModeledElement must be a RoadTravelwayDefinition
    if (!dynamic_cast<RoadTravelwayDefinitionCP>(model.GetModeledElement().get()))
        return nullptr;

    CreateParams createParams(model.GetDgnDb(), model.GetModelId(), QueryClassId(model.GetDgnDb()),
        RoadRailCategory::GetTravelwayDefComponent(model.GetDgnDb()));

    return new RoadLaneComponent(createParams);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      09/2017
+---------------+---------------+---------------+---------------+---------------+------*/
RoadLaneComponentCPtr RoadLaneComponent::CreateAndInsert(TypicalSectionPortionBreakDownModelCR model, bvector<TypicalSectionPointCP> const& points)
    {
    auto ptr = Create(model);
    if (ptr.IsNull())
        return nullptr;

    auto cPtr = ptr->Insert();
    if (cPtr.IsValid())
        SetPoints(*cPtr, points);

    return cPtr;
    }