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


HANDLER_DEFINE_MEMBERS(BarrierComponentHandler)
HANDLER_DEFINE_MEMBERS(BufferComponentHandler)
HANDLER_DEFINE_MEMBERS(CurbComponentHandler)
HANDLER_DEFINE_MEMBERS(OverallTypicalSectionAlignmentHandler)
HANDLER_DEFINE_MEMBERS(OverallTypicalSectionHandler)
HANDLER_DEFINE_MEMBERS(OverallTypicalSectionBreakDownModelHandler)
HANDLER_DEFINE_MEMBERS(OverallTypicalSectionPortionHandler)
HANDLER_DEFINE_MEMBERS(PavementComponentHandler)
HANDLER_DEFINE_MEMBERS(RoadLaneComponentHandler)
HANDLER_DEFINE_MEMBERS(RoadShoulderComponentHandler)
HANDLER_DEFINE_MEMBERS(RoadTravelwayDefinitionHandler)
HANDLER_DEFINE_MEMBERS(SideSlopeConditionComponentHandler)
HANDLER_DEFINE_MEMBERS(TravelwayComponentElementHandler)
HANDLER_DEFINE_MEMBERS(TravelwayDefinitionElementHandler)
HANDLER_DEFINE_MEMBERS(TravelwaySideComponentElementHandler)
HANDLER_DEFINE_MEMBERS(TravelwaySideDefinitionHandler)
HANDLER_DEFINE_MEMBERS(TravelwayStructureComponentElementHandler)
HANDLER_DEFINE_MEMBERS(TravelwayStructureDefinitionHandler)
HANDLER_DEFINE_MEMBERS(TypicalSectionComponentElementHandler)
HANDLER_DEFINE_MEMBERS(TypicalSectionPortionBreakDownModelHandler)
HANDLER_DEFINE_MEMBERS(TypicalSectionPortionDefinitionElementHandler)

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      10/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TypicalSectionPortionDefinitionElement::TypicalSectionPortionDefinitionElement(CreateParams const& params):
    T_Super(params)
    {
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      09/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TypicalSectionPointCPtr TypicalSectionPortionDefinitionElement::QueryOriginPoint() const
    { 
    auto stmtPtr = GetDgnDb().GetPreparedECSqlStatement("SELECT TargetECInstanceId FROM " BRRP_SCHEMA(BRRP_REL_TypicalSectionPortionDefinitionRefersToOriginPoint) " WHERE SourceECInstanceId = ?");
    BeAssert(stmtPtr.IsValid());

    stmtPtr->BindId(1, GetElementId());

    if (DbResult::BE_SQLITE_ROW != stmtPtr->Step())
        return nullptr;

    return TypicalSectionPoint::Get(GetDgnDb(), stmtPtr->GetValueId<DgnElementId>(0));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      09/2017
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus TypicalSectionPortionDefinitionElement::SetOriginPoint(TypicalSectionPortionDefinitionElementCR definition, TypicalSectionPointCR point)
    { 
    if (point.GetModel()->GetModeledElementId() != definition.GetElementId())
        return DgnDbStatus::BadArg;

    auto existingOriginCPtr = definition.QueryOriginPoint();
    if (existingOriginCPtr.IsValid())
        {
        if (DbResult::BE_SQLITE_OK != definition.GetDgnDb().DeleteLinkTableRelationship(
            ECInstanceKey(ECClassId(existingOriginCPtr->GetElementClassId().GetValue()), ECInstanceId(existingOriginCPtr->GetElementId().GetValue()))))
                return DgnDbStatus::BadElement;
        }

    ECInstanceKey insKey;
    if (DbResult::BE_SQLITE_OK != definition.GetDgnDb().InsertLinkTableRelationship(insKey,
        *definition.GetDgnDb().Schemas().GetClass(BRRP_SCHEMA_NAME, BRRP_REL_TypicalSectionPortionDefinitionRefersToOriginPoint)->GetRelationshipClassCP(),
        definition.GetElementId(), point.GetElementId()))
        return DgnDbStatus::BadElement;

    return DgnDbStatus::Success;
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
* @bsimethod                                    Diego.Diaz                      10/2016
+---------------+---------------+---------------+---------------+---------------+------*/
CodeSpecId TravelwaySideDefinition::QueryCodeSpecId(DgnDbCR dgndb)
    {
    CodeSpecId codeSpecId = dgndb.CodeSpecs().QueryCodeSpecId(BRRP_CODESPEC_TravelwaySide);
    BeAssert(codeSpecId.IsValid());
    return codeSpecId;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      10/2016
+---------------+---------------+---------------+---------------+---------------+------*/
DgnCode TravelwaySideDefinition::CreateCode(DefinitionModelCR scope, Utf8StringCR value)
    {
    return CodeSpec::CreateCode(BRRP_CODESPEC_TravelwaySide, scope, value);
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      10/2016
+---------------+---------------+---------------+---------------+---------------+------*/
TravelwaySideDefinitionPtr TravelwaySideDefinition::Create(DefinitionModelCR model, Utf8StringCR code)
    {
    if (!model.GetModelId().IsValid())
        return nullptr;

    CreateParams createParams(model.GetDgnDb(), model.GetModelId(), QueryClassId(model.GetDgnDb()), CreateCode(model, code));

    return new TravelwaySideDefinition(createParams);
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
* @bsimethod                                    Diego.Diaz                      09/2017
+---------------+---------------+---------------+---------------+---------------+------*/
OverallTypicalSectionAlignmentCPtr OverallTypicalSection::QueryMainAlignment() const
    {
    auto stmtPtr = GetDgnDb().GetPreparedECSqlStatement("SELECT TargetECInstanceId FROM " BRRP_SCHEMA(BRRP_REL_OverallTypicalSectionRefersToMainAlignment) " WHERE SourceECInstanceId = ?");
    BeAssert(stmtPtr.IsValid());

    stmtPtr->BindId(1, GetElementId());

    if (DbResult::BE_SQLITE_ROW != stmtPtr->Step())
        return nullptr;

    return OverallTypicalSectionAlignment::Get(GetDgnDb(), stmtPtr->GetValueId<DgnElementId>(0));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      09/2017
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus OverallTypicalSection::SetMainAlignment(OverallTypicalSectionCR typicalSection, OverallTypicalSectionAlignmentCR alignment)
    {
    auto existingMainAlignmentCPtr = typicalSection.QueryMainAlignment();
    if (existingMainAlignmentCPtr.IsValid())
        {
        if (DbResult::BE_SQLITE_OK != alignment.GetDgnDb().DeleteLinkTableRelationship(
            ECInstanceKey(ECClassId(existingMainAlignmentCPtr->GetElementClassId().GetValue()), ECInstanceId(existingMainAlignmentCPtr->GetElementId().GetValue()))))
            return DgnDbStatus::BadElement;
        }

    ECInstanceKey insKey;
    if (DbResult::BE_SQLITE_OK != alignment.GetDgnDb().InsertLinkTableRelationship(insKey,
        *alignment.GetDgnDb().Schemas().GetClass(BRRP_SCHEMA_NAME, BRRP_REL_OverallTypicalSectionRefersToMainAlignment)->GetRelationshipClassCP(),
        typicalSection.GetElementId(), alignment.GetElementId()))
        return DgnDbStatus::BadElement;

    return DgnDbStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      09/2017
+---------------+---------------+---------------+---------------+---------------+------*/
OverallTypicalSectionAlignmentPtr OverallTypicalSectionAlignment::Create(OverallTypicalSectionBreakDownModelCR model, DPoint2dCR origin)
    {
    if (!model.GetModelId().IsValid())
        return nullptr;

    CreateParams createParams(model.GetDgnDb(), model.GetModelId(), QueryClassId(model.GetDgnDb()), 
        RoadRailCategory::GetTypicalSectionPoint(model.GetDgnDb()), Placement2d(origin, AngleInDegrees()));

    return new OverallTypicalSectionAlignment(createParams);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      09/2017
+---------------+---------------+---------------+---------------+---------------+------*/
OverallTypicalSectionAlignmentCPtr OverallTypicalSectionAlignment::CreateAndInsert(OverallTypicalSectionBreakDownModelCR model, DPoint2dCR origin)
    {
    auto ptr = Create(model, origin);
    if (ptr.IsNull())
        return nullptr;

    return ptr->Insert();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      09/2017
+---------------+---------------+---------------+---------------+---------------+------*/
OverallTypicalSectionPortionPtr OverallTypicalSectionPortion::Create(OverallTypicalSectionBreakDownModelCR model, 
    TypicalSectionPortionDefinitionElementCR refDefinition, OverallTypicalSectionAlignmentCR alignment)
    {
    if (!model.GetModelId().IsValid() || !alignment.GetElementId().IsValid() || !refDefinition.GetElementId().IsValid())
        return nullptr;

    CreateParams createParams(model.GetDgnDb(), model.GetModelId(), QueryClassId(model.GetDgnDb()), 
        RoadRailCategory::GetTypicalSectionPoint(model.GetDgnDb()), Placement2d());

    OverallTypicalSectionPortionPtr retVal(new OverallTypicalSectionPortion(createParams));
    retVal->SetAlignment(alignment);
    retVal->SetDefinition(refDefinition);
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
DgnDbStatus TypicalSectionComponentElement::SetPoints(TypicalSectionComponentElementCR component, bvector<ITypicalSectionConstraintPointCP> const& points)
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

        auto iter = std::find(pointIds.begin(), pointIds.end(), pointCP->GetConstraintPointId());
        if (iter == pointIds.end()) // New point
            {
            pointIds.push_back(pointCP->GetConstraintPointId());
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
TravelwayStructureComponentElement::TravelwayStructureComponentElement(CreateParams const& params):
    T_Super(params)
    {
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
TravelwaySideComponentElement::TravelwaySideComponentElement(CreateParams const& params):
    T_Super(params)
    {
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
RoadLaneComponentCPtr RoadLaneComponent::CreateAndInsert(TypicalSectionPortionBreakDownModelCR model, bvector<ITypicalSectionConstraintPointCP> const& points)
    {
    auto ptr = Create(model);
    if (ptr.IsNull())
        return nullptr;

    auto cPtr = ptr->Insert();
    if (cPtr.IsValid())
        SetPoints(*cPtr, points);

    return cPtr;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      08/2017
+---------------+---------------+---------------+---------------+---------------+------*/
RoadShoulderComponentPtr RoadShoulderComponent::Create(TypicalSectionPortionBreakDownModelCR model)
    {
    if (!model.GetModelId().IsValid())
        return nullptr;

    // ModeledElement must be a RoadTravelwayDefinition
    if (!dynamic_cast<RoadTravelwayDefinitionCP>(model.GetModeledElement().get()))
        return nullptr;

    CreateParams createParams(model.GetDgnDb(), model.GetModelId(), QueryClassId(model.GetDgnDb()),
        RoadRailCategory::GetTravelwaySideDefComponent(model.GetDgnDb()));

    return new RoadShoulderComponent(createParams);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      09/2017
+---------------+---------------+---------------+---------------+---------------+------*/
RoadShoulderComponentCPtr RoadShoulderComponent::CreateAndInsert(TypicalSectionPortionBreakDownModelCR model, bvector<ITypicalSectionConstraintPointCP> const& points)
    {
    auto ptr = Create(model);
    if (ptr.IsNull())
        return nullptr;

    auto cPtr = ptr->Insert();
    if (cPtr.IsValid())
        SetPoints(*cPtr, points);

    return cPtr;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      08/2017
+---------------+---------------+---------------+---------------+---------------+------*/
BufferComponentPtr BufferComponent::Create(TypicalSectionPortionBreakDownModelCR model)
    {
    if (!model.GetModelId().IsValid())
        return nullptr;

    CreateParams createParams(model.GetDgnDb(), model.GetModelId(), QueryClassId(model.GetDgnDb()),
        RoadRailCategory::GetTravelwaySideDefComponent(model.GetDgnDb()));

    return new BufferComponent(createParams);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      09/2017
+---------------+---------------+---------------+---------------+---------------+------*/
BufferComponentCPtr BufferComponent::CreateAndInsert(TypicalSectionPortionBreakDownModelCR model, bvector<ITypicalSectionConstraintPointCP> const& points)
    {
    auto ptr = Create(model);
    if (ptr.IsNull())
        return nullptr;

    auto cPtr = ptr->Insert();
    if (cPtr.IsValid())
        SetPoints(*cPtr, points);

    return cPtr;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      09/2017
+---------------+---------------+---------------+---------------+---------------+------*/
SideSlopeConditionComponent::SideSlopeConditionComponent(CreateParams const& params) : T_Super(params)
    {
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      08/2017
+---------------+---------------+---------------+---------------+---------------+------*/
SideSlopeConditionComponentPtr SideSlopeConditionComponent::Create(TypicalSectionPortionBreakDownModelCR model, int32_t priority)
    {
    if (!model.GetModelId().IsValid())
        return nullptr;

    CreateParams createParams(model.GetDgnDb(), model.GetModelId(), QueryClassId(model.GetDgnDb()),
        RoadRailCategory::GetTravelwaySideDefComponent(model.GetDgnDb()));

    SideSlopeConditionComponentPtr retVal(new SideSlopeConditionComponent(createParams));
    retVal->SetPriority(priority);

    return retVal;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      09/2017
+---------------+---------------+---------------+---------------+---------------+------*/
SideSlopeConditionComponentCPtr SideSlopeConditionComponent::CreateAndInsert(TypicalSectionPortionBreakDownModelCR model, int32_t priority, bvector<ITypicalSectionConstraintPointCP> const& points)
    {
    auto ptr = Create(model, priority);
    if (ptr.IsNull())
        return nullptr;

    auto cPtr = ptr->Insert();
    if (cPtr.IsValid())
        SetPoints(*cPtr, points);

    return cPtr;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      09/2017
+---------------+---------------+---------------+---------------+---------------+------*/
Nullable<int32_t> SideSlopeConditionComponent::GetBenchingCount() const
    {
    Nullable<int32_t> retVal;

    ECValue v;
    if (DgnDbStatus::Success == GetPropertyValue(v, "BenchingCount"))
        {
        if (!v.IsNull())
            retVal = v.GetInteger();
        }

    return retVal;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      09/2017
+---------------+---------------+---------------+---------------+---------------+------*/
void SideSlopeConditionComponent::SetBenchingCount(Nullable<int32_t> newVal)
    {
    ECValue v;
    if (newVal.IsNull())
        v.SetIsNull(true);
    else
        v.SetInteger(newVal.Value());

    SetPropertyValue("BenchingCount", v);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      08/2017
+---------------+---------------+---------------+---------------+---------------+------*/
CurbComponentPtr CurbComponent::Create(TypicalSectionPortionBreakDownModelCR model)
    {
    if (!model.GetModelId().IsValid())
        return nullptr;

    CreateParams createParams(model.GetDgnDb(), model.GetModelId(), QueryClassId(model.GetDgnDb()),
        RoadRailCategory::GetTravelwaySideDefComponent(model.GetDgnDb()));

    return new CurbComponent(createParams);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      09/2017
+---------------+---------------+---------------+---------------+---------------+------*/
CurbComponentCPtr CurbComponent::CreateAndInsert(TypicalSectionPortionBreakDownModelCR model, bvector<ITypicalSectionConstraintPointCP> const& points)
    {
    auto ptr = Create(model);
    if (ptr.IsNull())
        return nullptr;

    auto cPtr = ptr->Insert();
    if (cPtr.IsValid())
        SetPoints(*cPtr, points);

    return cPtr;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      08/2017
+---------------+---------------+---------------+---------------+---------------+------*/
BarrierComponentPtr BarrierComponent::Create(TypicalSectionPortionBreakDownModelCR model)
    {
    if (!model.GetModelId().IsValid())
        return nullptr;

    CreateParams createParams(model.GetDgnDb(), model.GetModelId(), QueryClassId(model.GetDgnDb()),
        RoadRailCategory::GetTravelwaySideDefComponent(model.GetDgnDb()));

    return new BarrierComponent(createParams);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      09/2017
+---------------+---------------+---------------+---------------+---------------+------*/
BarrierComponentCPtr BarrierComponent::CreateAndInsert(TypicalSectionPortionBreakDownModelCR model, bvector<ITypicalSectionConstraintPointCP> const& points)
    {
    auto ptr = Create(model);
    if (ptr.IsNull())
        return nullptr;

    auto cPtr = ptr->Insert();
    if (cPtr.IsValid())
        SetPoints(*cPtr, points);

    return cPtr;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      08/2017
+---------------+---------------+---------------+---------------+---------------+------*/
PavementComponentPtr PavementComponent::Create(TypicalSectionPortionBreakDownModelCR model)
    {
    if (!model.GetModelId().IsValid())
        return nullptr;

    CreateParams createParams(model.GetDgnDb(), model.GetModelId(), QueryClassId(model.GetDgnDb()), 
        RoadRailCategory::GetTravelwayStructureDefComponent(model.GetDgnDb()));

    return new PavementComponent(createParams);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      09/2017
+---------------+---------------+---------------+---------------+---------------+------*/
PavementComponentCPtr PavementComponent::CreateAndInsert(TypicalSectionPortionBreakDownModelCR model, bvector<ITypicalSectionConstraintPointCP> const& points)
    {
    auto ptr = Create(model);
    if (ptr.IsNull())
        return nullptr;

    auto cPtr = ptr->Insert();
    if (cPtr.IsValid())
        SetPoints(*cPtr, points);

    return cPtr;
    }
