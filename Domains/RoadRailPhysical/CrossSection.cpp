/*--------------------------------------------------------------------------------------+
|
|     $Source: CrossSection.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <RoadRailPhysicalInternal.h>

HANDLER_DEFINE_MEMBERS(CrossSectionDefinitionModelHandler)
HANDLER_DEFINE_MEMBERS(CrossSectionElementHandler)
HANDLER_DEFINE_MEMBERS(CrossSectionPortionBreakDownModelHandler)
HANDLER_DEFINE_MEMBERS(CrossSectionPortionDefinitionHandler)
HANDLER_DEFINE_MEMBERS(CrossSectionPortionDefinitionElementHandler)
HANDLER_DEFINE_MEMBERS(EndConditionDefinitionHandler)
HANDLER_DEFINE_MEMBERS(EndConditionDefinitionModelHandler)
HANDLER_DEFINE_MEMBERS(RoadCrossSectionHandler)
HANDLER_DEFINE_MEMBERS(RoadTravelwayDefinitionHandler)
HANDLER_DEFINE_MEMBERS(TravelwayDefinitionElementHandler)
HANDLER_DEFINE_MEMBERS(TravelwayDefinitionModelHandler)

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      10/2016
+---------------+---------------+---------------+---------------+---------------+------*/
CrossSectionPortionDefinitionElement::CrossSectionPortionDefinitionElement(CreateParams const& params):
    T_Super(params)
    {
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      10/2016
+---------------+---------------+---------------+---------------+---------------+------*/
CrossSectionPortionDefinition::CrossSectionPortionDefinition(CreateParams const& params):
    T_Super(params)
    {
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      04/2017
+---------------+---------------+---------------+---------------+---------------+------*/
CrossSectionPortionDefinitionPtr CrossSectionPortionDefinition::Create(CrossSectionDefinitionModelCR model, Utf8CP label)
    {
    if (!model.GetModelId().IsValid())
        return nullptr;

    CreateParams createParams(model.GetDgnDb(), model.GetModelId(), QueryClassId(model.GetDgnDb()), DgnCode(), label);
    return new CrossSectionPortionDefinition(createParams);
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
RoadTravelwayDefinitionCPtr RoadTravelwayDefinition::Insert(CrossSectionPortionBreakDownModelPtr& breakDownModelPtr, DgnDbStatus* stat)
    {
    auto retVal = GetDgnDb().Elements().Insert<RoadTravelwayDefinition>(*this, stat);
    if (retVal.IsNull())
        return nullptr;

    breakDownModelPtr = CrossSectionPortionBreakDownModel::Create(CrossSectionPortionBreakDownModel::CreateParams(GetDgnDb(), retVal->GetElementId()));

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
EndConditionDefinitionCPtr EndConditionDefinition::Insert(CrossSectionPortionBreakDownModelPtr& breakDownModelPtr, Dgn::DgnDbStatus* stat)
    {
    auto retVal = GetDgnDb().Elements().Insert<EndConditionDefinition>(*this, stat);
    if (retVal.IsNull())
        return nullptr;

    breakDownModelPtr = CrossSectionPortionBreakDownModel::Create(CrossSectionPortionBreakDownModel::CreateParams(GetDgnDb(), retVal->GetElementId()));

    DgnDbStatus status;
    if (DgnDbStatus::Success != (status = breakDownModelPtr->Insert()))
        {
        if (stat) *stat = status;
        return nullptr;
        }

    return retVal;
    }

/********************************************************************************************/

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      10/2016
+---------------+---------------+---------------+---------------+---------------+------*/
CrossSectionElement::CrossSectionElement(CreateParams const& params):
    T_Super(params)
    {
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      10/2016
+---------------+---------------+---------------+---------------+---------------+------*/
RoadCrossSection::RoadCrossSection(CreateParams const& params):
    T_Super(params)
    {
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      10/2016
+---------------+---------------+---------------+---------------+---------------+------*/
CodeSpecId RoadCrossSection::QueryCodeSpecId(Dgn::DgnDbCR dgndb)
    {
    CodeSpecId codeSpecId = dgndb.CodeSpecs().QueryCodeSpecId(BRRP_CODESPEC_RoadCrossSection);
    BeAssert(codeSpecId.IsValid());
    return codeSpecId;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      10/2016
+---------------+---------------+---------------+---------------+---------------+------*/
DgnCode RoadCrossSection::CreateCode(Dgn::DgnDbR dgndb, Utf8StringCR value)
    {
    return CodeSpec::CreateCode(dgndb, BRRP_CODESPEC_RoadCrossSection, value);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      10/2016
+---------------+---------------+---------------+---------------+---------------+------*/
RoadCrossSectionPtr RoadCrossSection::Create(CrossSectionDefinitionModelCR model, Utf8StringCR code)
    {
    if (!model.GetModelId().IsValid())
        return nullptr;

    CreateParams createParams(model.GetDgnDb(), model.GetModelId(), QueryClassId(model.GetDgnDb()), CreateCode(model.GetDgnDb(), code));

    return new RoadCrossSection(createParams);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      10/2016
+---------------+---------------+---------------+---------------+---------------+------*/
/*RoadCrossSectionCPtr RoadCrossSection::Insert(CrossSectionBreakDownModelPtr& breakDownModelPtr, DgnDbStatus* stat)
    {
    auto retVal = GetDgnDb().Elements().Insert<RoadCrossSection>(*this, stat);
    if (retVal.IsNull())
        return nullptr;

    breakDownModelPtr = CrossSectionBreakDownModel::Create(DgnModel::CreateParams(GetDgnDb(), CrossSectionBreakDownModel::QueryClassId(GetDgnDb()),
        retVal->GetElementId()));

    DgnDbStatus status;
    if (DgnDbStatus::Success != (status = breakDownModelPtr->Insert()))
        {
        if (stat) *stat = status;
        return nullptr;
        }

    return retVal;
    }*/