/*--------------------------------------------------------------------------------------+
|
|     $Source: TypicalSection.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <RoadRailPhysicalInternal.h>

HANDLER_DEFINE_MEMBERS(TypicalSectionModelHandler)
HANDLER_DEFINE_MEMBERS(TypicalSectionPortionBreakDownModelHandler)
HANDLER_DEFINE_MEMBERS(TypicalSectionPortionHandler)
HANDLER_DEFINE_MEMBERS(TypicalSectionPortionElementHandler)
HANDLER_DEFINE_MEMBERS(EndConditionDefinitionHandler)
HANDLER_DEFINE_MEMBERS(EndConditionDefinitionModelHandler)
HANDLER_DEFINE_MEMBERS(RoadTravelwayDefinitionHandler)
HANDLER_DEFINE_MEMBERS(TravelwayDefinitionElementHandler)
HANDLER_DEFINE_MEMBERS(TravelwayDefinitionModelHandler)

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
TypicalSectionPortion::TypicalSectionPortion(CreateParams const& params):
    T_Super(params)
    {
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      04/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TypicalSectionPortionPtr TypicalSectionPortion::Create(TypicalSectionModelCR model, Utf8CP label)
    {
    if (!model.GetModelId().IsValid())
        return nullptr;

    CreateParams createParams(model.GetDgnDb(), model.GetModelId(), QueryClassId(model.GetDgnDb()), DgnCode(), label);
    return new TypicalSectionPortion(createParams);
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
CodeSpecId RoadTravelwayDefinition::QueryCodeSpecId(Dgn::DgnDbCR dgndb)
    {
    CodeSpecId codeSpecId = dgndb.CodeSpecs().QueryCodeSpecId(BRRP_CODESPEC_RoadTravelway);
    BeAssert(codeSpecId.IsValid());
    return codeSpecId;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      10/2016
+---------------+---------------+---------------+---------------+---------------+------*/
DgnCode RoadTravelwayDefinition::CreateCode(Dgn::DgnDbR dgndb, Utf8StringCR value)
    {
    return CodeSpec::CreateCode(dgndb, BRRP_CODESPEC_RoadTravelway, value);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      10/2016
+---------------+---------------+---------------+---------------+---------------+------*/
RoadTravelwayDefinitionPtr RoadTravelwayDefinition::Create(TravelwayDefinitionModelCR model, Utf8StringCR code)
    {
    if (!model.GetModelId().IsValid())
        return nullptr;

    CreateParams createParams(model.GetDgnDb(), model.GetModelId(), QueryClassId(model.GetDgnDb()), CreateCode(model.GetDgnDb(), code));

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
EndConditionDefinition::EndConditionDefinition(CreateParams const& params):
    T_Super(params)
    {
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      04/2017
+---------------+---------------+---------------+---------------+---------------+------*/
EndConditionDefinitionCPtr EndConditionDefinition::Insert(TypicalSectionPortionBreakDownModelPtr& breakDownModelPtr, Dgn::DgnDbStatus* stat)
    {
    auto retVal = GetDgnDb().Elements().Insert<EndConditionDefinition>(*this, stat);
    if (retVal.IsNull())
        return nullptr;

    breakDownModelPtr = TypicalSectionPortionBreakDownModel::Create(TypicalSectionPortionBreakDownModel::CreateParams(GetDgnDb(), retVal->GetElementId()));

    DgnDbStatus status;
    if (DgnDbStatus::Success != (status = breakDownModelPtr->Insert()))
        {
        if (stat) *stat = status;
        return nullptr;
        }

    return retVal;
    }