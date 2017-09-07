/*--------------------------------------------------------------------------------------+
|
|     $Source: RoadDesignSpeed.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "RoadRailPhysicalInternal.h"
#include <RoadRailPhysical/RoadDesignSpeed.h>

HANDLER_DEFINE_MEMBERS(DesignSpeedDefinitionElementHandler)
HANDLER_DEFINE_MEMBERS(DesignSpeedDefinitionModelHandler)
HANDLER_DEFINE_MEMBERS(DesignSpeedDefinitionTableHandler)
HANDLER_DEFINE_MEMBERS(DesignSpeedHandler)
HANDLER_DEFINE_MEMBERS(RoadDesignSpeedDefinitionHandler)
HANDLER_DEFINE_MEMBERS(RoadwayStandardsModelHandler)

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      11/2016
+---------------+---------------+---------------+---------------+---------------+------*/
DesignSpeedDefinitionTable::DesignSpeedDefinitionTable(CreateParams const& params):
    T_Super(params)
    {
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      11/2016
+---------------+---------------+---------------+---------------+---------------+------*/
DesignSpeedDefinitionTable::DesignSpeedDefinitionTable(CreateParams const& params, UnitSystem unitSystem) :
    T_Super(params)
    {
    SetPropertyValue("UnitSystem", static_cast<int32_t>(unitSystem));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      11/2016
+---------------+---------------+---------------+---------------+---------------+------*/
DgnCode DesignSpeedDefinitionTable::CreateCode(DefinitionModelCR scope, Utf8StringCR value)
    {
    return CodeSpec::CreateCode(BRRP_CODESPEC_DesignSpeedDefinitionTable, scope, value);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      11/2016
+---------------+---------------+---------------+---------------+---------------+------*/
DesignSpeedDefinitionTablePtr DesignSpeedDefinitionTable::Create(DefinitionModelCR model, Utf8StringCR code, UnitSystem unitSystem)
    {
    CreateParams params(model.GetDgnDb(), model.GetModelId(), QueryClassId(model.GetDgnDb()), CreateCode(model, code));
    return new DesignSpeedDefinitionTable(params, unitSystem);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      11/2016
+---------------+---------------+---------------+---------------+---------------+------*/
DesignSpeedDefinitionTableCPtr DesignSpeedDefinitionTable::Insert(DesignSpeedDefinitionModelPtr& breakDownModelPtr, Dgn::DgnDbStatus* stat)
    {
    auto retVal = GetDgnDb().Elements().Insert<DesignSpeedDefinitionTable>(*this, stat);
    if (retVal.IsNull())
        return nullptr;

    breakDownModelPtr = DesignSpeedDefinitionModel::Create(DgnModel::CreateParams(GetDgnDb(), DesignSpeedDefinitionModel::QueryClassId(GetDgnDb()),
        retVal->GetElementId()));

    DgnDbStatus status;
    if (DgnDbStatus::Success != (status = breakDownModelPtr->Insert()))
        {
        if (stat) *stat = status;
        return nullptr;
        }

    return retVal;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      11/2016
+---------------+---------------+---------------+---------------+---------------+------*/
DesignSpeedDefinitionElement::DesignSpeedDefinitionElement(CreateParams const& params):
    T_Super(params)
    {
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      11/2016
+---------------+---------------+---------------+---------------+---------------+------*/
RoadDesignSpeedDefinition::RoadDesignSpeedDefinition(CreateParams const& params):
    T_Super(params)
    {
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      11/2016
+---------------+---------------+---------------+---------------+---------------+------*/
RoadDesignSpeedDefinition::RoadDesignSpeedDefinition(CreateParams const& params, double designSpeed, double sideSlopeFrictionFactor, double maxTransitionRadius, double desirableSpiralLength,
    double type1CrestVertical, double type3SagVertical, double crestVerticalPsd, double averageRunningSpeed, double maximumRelativeGradient,
    double crestK_SSD, double sagK_SSD, double ignoreLength):
    T_Super(params)
    {
    SetPropertyValue("DesignSpeed", designSpeed);
    SetPropertyValue("SideSlopeFrictionFactor", sideSlopeFrictionFactor);
    SetPropertyValue("MaxTransitionRadius", maxTransitionRadius);
    SetPropertyValue("DesirableSpiralLength", desirableSpiralLength);
    SetPropertyValue("Type1CrestVertical", type1CrestVertical);
    SetPropertyValue("Type3SagVertical", type3SagVertical);
    SetPropertyValue("CrestVerticalPsd", crestVerticalPsd);
    SetPropertyValue("AverageRunningSpeed", averageRunningSpeed);
    SetPropertyValue("MaximumRelativeGradient", maximumRelativeGradient);
    SetPropertyValue("CrestK_SSD", crestK_SSD);
    SetPropertyValue("SagK_SSD", sagK_SSD);
    SetPropertyValue("IgnoreLength", ignoreLength);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      11/2016
+---------------+---------------+---------------+---------------+---------------+------*/
DgnCode DesignSpeedDefinitionElement::CreateCode(DefinitionModelCR scope, double speed, UnitSystem unitSystem)
    {
    return CodeSpec::CreateCode(BRRP_CODESPEC_DesignSpeedDefinition, scope,
        Utf8PrintfString("%.0f %s", speed, (unitSystem == UnitSystem::Metric) ? "Km/h" : "mph"));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      11/2016
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String DesignSpeedDefinitionElement::GetDefaultUserLabel(double speed, Dgn::UnitSystem unitSystem)
    {
    if (unitSystem == UnitSystem::Metric)
        return Utf8PrintfString("%3.0f Km/h", speed);
    else
        return Utf8PrintfString("%2.0f mph", speed);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      11/2016
+---------------+---------------+---------------+---------------+---------------+------*/
RoadDesignSpeedDefinitionPtr RoadDesignSpeedDefinition::Create(DesignSpeedDefinitionModelCR model, double designSpeed, double sideSlopeFrictionFactor,
    double maxTransitionRadius, double desirableSpiralLength, double type1CrestVertical, double type3SagVertical, double crestVerticalPsd, double averageRunningSpeed,
    double maximumRelativeGradient, double crestK_SSD, double sagK_SSD, double ignoreLength)
    {
    auto tableCPtr = model.GetModeledDesignSpeedDefinitionTable();

    CreateParams params(model.GetDgnDb(), model.GetModelId(), QueryClassId(model.GetDgnDb()), CreateCode(model, designSpeed, tableCPtr->GetUnitSystem()));

    auto retVal = new RoadDesignSpeedDefinition(params, designSpeed, sideSlopeFrictionFactor, maxTransitionRadius, desirableSpiralLength, type1CrestVertical,
        type3SagVertical, crestVerticalPsd, averageRunningSpeed, maximumRelativeGradient, crestK_SSD, sagK_SSD, ignoreLength);

    retVal->SetUserLabel(GetDefaultUserLabel(designSpeed, tableCPtr->GetUnitSystem()).c_str());

    return retVal;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      10/2016
+---------------+---------------+---------------+---------------+---------------+------*/
DesignSpeed::DesignSpeed(CreateParams const& params):
    T_Super(params)
    {
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      10/2016
+---------------+---------------+---------------+---------------+---------------+------*/
DesignSpeed::DesignSpeed(CreateParams const& params, DesignSpeedDefinitionElementCR designSpeedDef, double fromDistanceAlong, double toDistanceAlong):
    T_Super(params), ILinearlyLocatedSingleFromTo(fromDistanceAlong, toDistanceAlong)
    {
    SetDesignSpeedDefinition(designSpeedDef);
    _AddLinearlyReferencedLocation(*_GetUnpersistedFromToLocation());    
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      10/2016
+---------------+---------------+---------------+---------------+---------------+------*/
DesignSpeedPtr DesignSpeed::Create(PathwayElementCR pathway, DesignSpeedDefinitionElementCR designSpeedDef, double fromDistanceAlong, double toDistanceAlong)
    {
    if (!pathway.GetElementId().IsValid())
        return nullptr;

    auto alignmentId = pathway.GetAlignmentId();
    if (!alignmentId.IsValid())
        return nullptr;

    CreateParams params(pathway.GetDgnDb(), pathway.GetModelId(), QueryClassId(pathway.GetDgnDb()));
    params.SetParentId(pathway.GetElementId(),
        DgnClassId(pathway.GetDgnDb().Schemas().GetClassId(BRRP_SCHEMA_NAME, BRRP_REL_PathwayOwnsDesignSpeeds)));

    auto retVal = new DesignSpeed(params, designSpeedDef, fromDistanceAlong, toDistanceAlong);
    retVal->_SetLinearElement(alignmentId);
    return retVal;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      12/2016
+---------------+---------------+---------------+---------------+---------------+------*/
void DesignSpeed::SetDesignSpeedDefinition(DesignSpeedDefinitionElementCR designSpeedDef)
    { 
    SetPropertyValue("DesignSpeedDefinition", designSpeedDef.GetElementId(),
        GetDgnDb().Schemas().GetClassId(BRRP_SCHEMA_NAME, BRRP_REL_DesignSpeedRefersToDefinition));
    }