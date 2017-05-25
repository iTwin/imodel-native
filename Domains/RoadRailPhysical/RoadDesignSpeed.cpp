/*--------------------------------------------------------------------------------------+
|
|     $Source: RoadDesignSpeed.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <RoadRailPhysicalInternal.h>

HANDLER_DEFINE_MEMBERS(RoadDesignSpeedDefinitionModelHandler)
HANDLER_DEFINE_MEMBERS(RoadDesignSpeedDefinitionTableHandler)
HANDLER_DEFINE_MEMBERS(RoadDesignSpeedDefinitionTableModelHandler)
HANDLER_DEFINE_MEMBERS(RoadDesignSpeedDefinitionHandler)
HANDLER_DEFINE_MEMBERS(RoadDesignSpeedHandler)
HANDLER_DEFINE_MEMBERS(RoadDesignSpeedStandardsHandler)
HANDLER_DEFINE_MEMBERS(RoadwayStandardsModelHandler)

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      11/2016
+---------------+---------------+---------------+---------------+---------------+------*/
RoadDesignSpeedStandards::RoadDesignSpeedStandards(CreateParams const& params):
    T_Super(params)
    {
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      11/2016
+---------------+---------------+---------------+---------------+---------------+------*/
RoadDesignSpeedStandardsPtr RoadDesignSpeedStandards::Create(RoadwayStandardsModelCR model)
    {
    CreateParams params(model.GetDgnDb(), model.GetModelId(), QueryClassId(model.GetDgnDb()));
    return new RoadDesignSpeedStandards(params);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      11/2016
+---------------+---------------+---------------+---------------+---------------+------*/
RoadDesignSpeedStandardsCPtr RoadDesignSpeedStandards::Insert(RoadDesignSpeedDefinitionTableModelPtr& breakDownModelPtr, Dgn::DgnDbStatus* stat)
    {
    auto retVal = GetDgnDb().Elements().Insert<RoadDesignSpeedStandards>(*this, stat);
    if (retVal.IsNull())
        return nullptr;

    breakDownModelPtr = RoadDesignSpeedDefinitionTableModel::Create(DgnModel::CreateParams(GetDgnDb(), RoadDesignSpeedDefinitionTableModel::QueryClassId(GetDgnDb()),
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
RoadDesignSpeedDefinitionTable::RoadDesignSpeedDefinitionTable(CreateParams const& params):
    T_Super(params)
    {
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      11/2016
+---------------+---------------+---------------+---------------+---------------+------*/
RoadDesignSpeedDefinitionTable::RoadDesignSpeedDefinitionTable(CreateParams const& params, UnitSystem unitSystem) :
    T_Super(params)
    {
    SetPropertyValue("UnitSystem", static_cast<int32_t>(unitSystem));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      11/2016
+---------------+---------------+---------------+---------------+---------------+------*/
DgnCode RoadDesignSpeedDefinitionTable::CreateCode(DgnDbR dgndb, Utf8StringCR value)
    {
    return CodeSpec::CreateCode(dgndb, BRRP_CODESPEC_RoadDesignSpeedDefinitionTable, value);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      11/2016
+---------------+---------------+---------------+---------------+---------------+------*/
RoadDesignSpeedDefinitionTablePtr RoadDesignSpeedDefinitionTable::Create(RoadDesignSpeedDefinitionTableModelCR model, Utf8StringCR code, UnitSystem unitSystem)
    {
    CreateParams params(model.GetDgnDb(), model.GetModelId(), QueryClassId(model.GetDgnDb()), CreateCode(model.GetDgnDb(), code));
    return new RoadDesignSpeedDefinitionTable(params, unitSystem);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      11/2016
+---------------+---------------+---------------+---------------+---------------+------*/
RoadDesignSpeedDefinitionTableCPtr RoadDesignSpeedDefinitionTable::Insert(RoadDesignSpeedDefinitionModelPtr& breakDownModelPtr, Dgn::DgnDbStatus* stat)
    {
    auto retVal = GetDgnDb().Elements().Insert<RoadDesignSpeedDefinitionTable>(*this, stat);
    if (retVal.IsNull())
        return nullptr;

    breakDownModelPtr = RoadDesignSpeedDefinitionModel::Create(DgnModel::CreateParams(GetDgnDb(), RoadDesignSpeedDefinitionModel::QueryClassId(GetDgnDb()),
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
DgnCode RoadDesignSpeedDefinition::CreateCode(Dgn::DgnDbR dgndb, double speed, UnitSystem unitSystem)
    {
    return CodeSpec::CreateCode(dgndb, BRRP_CODESPEC_RoadDesignSpeedDefinition,
        Utf8PrintfString("%.0f %s", speed, (unitSystem == UnitSystem::Metric) ? "Km/h" : "mph"));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      11/2016
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String RoadDesignSpeedDefinition::GetDefaultUserLabel(double speed, Dgn::UnitSystem unitSystem)
    {
    if (unitSystem == UnitSystem::Metric)
        return Utf8PrintfString("%3.0f Km/h", speed);
    else
        return Utf8PrintfString("%2.0f mph", speed);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      11/2016
+---------------+---------------+---------------+---------------+---------------+------*/
RoadDesignSpeedDefinitionPtr RoadDesignSpeedDefinition::Create(RoadDesignSpeedDefinitionModelCR model, double designSpeed, double sideSlopeFrictionFactor,
    double maxTransitionRadius, double desirableSpiralLength, double type1CrestVertical, double type3SagVertical, double crestVerticalPsd, double averageRunningSpeed,
    double maximumRelativeGradient, double crestK_SSD, double sagK_SSD, double ignoreLength)
    {
    auto tableCPtr = model.GetModeledDesignSpeedDefinitionTable();

    CreateParams params(model.GetDgnDb(), model.GetModelId(), QueryClassId(model.GetDgnDb()), CreateCode(model.GetDgnDb(), designSpeed, tableCPtr->GetUnitSystem()));

    auto retVal = new RoadDesignSpeedDefinition(params, designSpeed, sideSlopeFrictionFactor, maxTransitionRadius, desirableSpiralLength, type1CrestVertical,
        type3SagVertical, crestVerticalPsd, averageRunningSpeed, maximumRelativeGradient, crestK_SSD, sagK_SSD, ignoreLength);

    retVal->SetUserLabel(GetDefaultUserLabel(designSpeed, tableCPtr->GetUnitSystem()).c_str());

    return retVal;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      10/2016
+---------------+---------------+---------------+---------------+---------------+------*/
RoadDesignSpeed::RoadDesignSpeed(CreateParams const& params):
    T_Super(params)
    {
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      10/2016
+---------------+---------------+---------------+---------------+---------------+------*/
RoadDesignSpeed::RoadDesignSpeed(CreateParams const& params, RoadDesignSpeedDefinitionCR designSpeedDef, double fromDistanceAlong, double toDistanceAlong):
    T_Super(params), ILinearlyLocatedSingleFromTo(fromDistanceAlong, toDistanceAlong)
    {
    SetDesignSpeedDefinition(designSpeedDef);
    _AddLinearlyReferencedLocation(*_GetUnpersistedFromToLocation());    
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      10/2016
+---------------+---------------+---------------+---------------+---------------+------*/
RoadDesignSpeedPtr RoadDesignSpeed::Create(RoadwayCR roadway, RoadDesignSpeedDefinitionCR designSpeedDef, double fromDistanceAlong, double toDistanceAlong)
    {
    if (!roadway.GetElementId().IsValid())
        return nullptr;

    auto alignmentId = roadway.GetAlignmentId();
    if (!alignmentId.IsValid())
        return nullptr;

    CreateParams params(roadway.GetDgnDb(), roadway.GetModelId(), QueryClassId(roadway.GetDgnDb()));
    params.SetParentId(roadway.GetElementId(),
        DgnClassId(roadway.GetDgnDb().Schemas().GetClassId(BRRP_SCHEMA_NAME, BRRP_REL_RoadwayHasDesignSpeeds)));

    auto retVal = new RoadDesignSpeed(params, designSpeedDef, fromDistanceAlong, toDistanceAlong);
    retVal->_SetLinearElement(alignmentId);
    return retVal;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      12/2016
+---------------+---------------+---------------+---------------+---------------+------*/
void RoadDesignSpeed::SetDesignSpeedDefinition(RoadDesignSpeedDefinitionCR designSpeedDef)
    { 
    SetPropertyValue("RoadDesignSpeedDefinition", designSpeedDef.GetElementId(),
        GetDgnDb().Schemas().GetClassId(BRRP_SCHEMA_NAME, BRRP_REL_RoadDesignSpeedRefersToDefinition));
    }