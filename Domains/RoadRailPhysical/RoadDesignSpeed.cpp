/*--------------------------------------------------------------------------------------+
|
|     $Source: RoadDesignSpeed.cpp $
|
|  $Copyright: (c) 2019 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "RoadRailPhysicalInternal.h"
#include <RoadRailPhysical/RoadDesignSpeed.h>
#include <RoadRailPhysical/RoadRailPhysicalDomain.h>

HANDLER_DEFINE_MEMBERS(DesignSpeedHandler)
HANDLER_DEFINE_MEMBERS(DesignSpeedDefinitionHandler)
HANDLER_DEFINE_MEMBERS(DesignSpeedElementHandler)
HANDLER_DEFINE_MEMBERS(DesignSpeedTransitionHandler)
HANDLER_DEFINE_MEMBERS(RailwayStandardsModelHandler)
HANDLER_DEFINE_MEMBERS(RoadwayStandardsModelHandler)

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      12/2017
+---------------+---------------+---------------+---------------+---------------+------*/
RoadwayStandardsModelPtr RoadwayStandardsModel::Query(SubjectCR parentSubject)
    {
    DgnDbR db = parentSubject.GetDgnDb();
    DgnCode partitionCode = DefinitionPartition::CreateCode(parentSubject, RoadRailPhysicalDomain::GetRoadwayStandardsPartitionName());
    DgnElementId partitionId = db.Elements().QueryElementIdByCode(partitionCode);
    DefinitionPartitionCPtr partition = db.Elements().Get<DefinitionPartition>(partitionId);
    if (!partition.IsValid())
        return nullptr;
    return dynamic_cast<RoadwayStandardsModelP>(partition->GetSubModel().get());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      12/2017
+---------------+---------------+---------------+---------------+---------------+------*/
RailwayStandardsModelPtr RailwayStandardsModel::Query(SubjectCR parentSubject)
    {
    DgnDbR db = parentSubject.GetDgnDb();
    DgnCode partitionCode = DefinitionPartition::CreateCode(parentSubject, RoadRailPhysicalDomain::GetRailwayStandardsPartitionName());
    DgnElementId partitionId = db.Elements().QueryElementIdByCode(partitionCode);
    DefinitionPartitionCPtr partition = db.Elements().Get<DefinitionPartition>(partitionId);
    if (!partition.IsValid())
        return nullptr;
    return dynamic_cast<RailwayStandardsModelP>(partition->GetSubModel().get());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      11/2016
+---------------+---------------+---------------+---------------+---------------+------*/
DesignSpeedDefinition::DesignSpeedDefinition(CreateParams const& params):
    T_Super(params)
    {
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      05/2018
+---------------+---------------+---------------+---------------+---------------+------*/
DesignSpeedDefinition::DesignSpeedDefinition(CreateParams const& params, double designSpeed, UnitSystem unitSystem):
    T_Super(params)
    {
    SetPropertyValue("DesignSpeed", designSpeed);
    SetPropertyValue("UnitSystem", static_cast<int32_t>(unitSystem));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      11/2016
+---------------+---------------+---------------+---------------+---------------+------*/
DgnCode DesignSpeedDefinition::CreateCode(DefinitionModelCR scope, double speed, UnitSystem unitSystem)
    {
    Utf8String suffix = (unitSystem == UnitSystem::SI) ? "Km/h" : "mph";
    return CodeSpec::CreateCode(BRRP_CODESPEC_DesignSpeedDefinition, scope,
        Utf8PrintfString("%.0f %s", speed, suffix.c_str()));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      11/2018
+---------------+---------------+---------------+---------------+---------------+------*/
DesignSpeedDefinitionCPtr DesignSpeedDefinition::QueryByCode(DefinitionModelCR model, double speed, UnitSystem unitSystem)
    {
    auto defId = model.GetDgnDb().Elements().QueryElementIdByCode(CreateCode(model, speed, unitSystem));
    if (!defId.IsValid())
        return nullptr;

    return Get(model.GetDgnDb(), defId);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      05/2018
+---------------+---------------+---------------+---------------+---------------+------*/
DesignSpeedDefinitionPtr DesignSpeedDefinition::Create(DefinitionModelCR model, double designSpeed, UnitSystem unitSystem)
    {
    CreateParams params(model.GetDgnDb(), model.GetModelId(), QueryClassId(model.GetDgnDb()), CreateCode(model, designSpeed, unitSystem));

    return new DesignSpeedDefinition(params, designSpeed, unitSystem);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      11/2018
+---------------+---------------+---------------+---------------+---------------+------*/
DesignSpeedDefinition::UnitSystem DesignSpeedDefinition::GetUnitSystem() const
    {
    return static_cast<UnitSystem>(GetPropertyValueInt32("UnitSystem"));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      10/2016
+---------------+---------------+---------------+---------------+---------------+------*/
DesignSpeedElement::DesignSpeedElement(CreateParams const& params):
    T_Super(params)
    {
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      10/2016
+---------------+---------------+---------------+---------------+---------------+------*/
DesignSpeedElement::DesignSpeedElement(CreateParams const& params, CreateFromToParams const& fromToParams):
    T_Super(params), ICorridorPortionSingleFromTo(fromToParams)
    {
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
DesignSpeed::DesignSpeed(CreateParams const& params, CreateFromToParams const& fromToParams):
    T_Super(params, fromToParams)
    {
    SetDesignSpeedDefinition(*fromToParams.m_designSpeedDefCPtr);
    _AddLinearlyReferencedLocation(*_GetUnpersistedFromToLocation());    
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      12/2016
+---------------+---------------+---------------+---------------+---------------+------*/
void DesignSpeed::SetDesignSpeedDefinition(DesignSpeedDefinitionCR designSpeedDef)
    { 
    SetPropertyValue("Definition", designSpeedDef.GetElementId(),
        GetDgnDb().Schemas().GetClassId(BRRP_SCHEMA_NAME, BRRP_REL_DesignSpeedRefersToDefinition));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      02/2018
+---------------+---------------+---------------+---------------+---------------+------*/
DesignSpeedPtr DesignSpeed::Create(CreateFromToParams const& linearParams) 
    {
    if (!ValidateParams(linearParams)) 
        return nullptr; 

    auto& dgnDbR = linearParams.m_corridorPortionCPtr->GetDgnDb();
    CreateParams params(dgnDbR, linearParams.m_corridorPortionCPtr->GetModelId(), QueryClassId(dgnDbR));
    params.SetParentId(linearParams.m_corridorPortionCPtr->GetElementId(),
        DgnClassId(dgnDbR.Schemas().GetClassId(BRRP_SCHEMA_NAME, BRRP_REL_PathwayOwnsLinearlyLocatedAttribution)));
    
    auto retVal = new DesignSpeed(params, linearParams);
    retVal->_OnCreate(linearParams); 
    return retVal; 
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      10/2016
+---------------+---------------+---------------+---------------+---------------+------*/
DesignSpeedTransition::DesignSpeedTransition(CreateParams const& params):
    T_Super(params)
    {
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      10/2016
+---------------+---------------+---------------+---------------+---------------+------*/
DesignSpeedTransition::DesignSpeedTransition(CreateParams const& params, CreateFromToParams const& fromToParams):
    T_Super(params, fromToParams)
    {
    _AddLinearlyReferencedLocation(*_GetUnpersistedFromToLocation());    
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      02/2018
+---------------+---------------+---------------+---------------+---------------+------*/
DesignSpeedTransitionPtr DesignSpeedTransition::Create(CreateFromToParams const& linearParams) 
    {
    if (!ValidateParams(linearParams)) 
        return nullptr; 

    auto& dgnDbR = linearParams.m_corridorPortionCPtr->GetDgnDb();
    CreateParams params(dgnDbR, linearParams.m_corridorPortionCPtr->GetModelId(), QueryClassId(dgnDbR));
    params.SetParentId(linearParams.m_corridorPortionCPtr->GetElementId(),
        DgnClassId(dgnDbR.Schemas().GetClassId(BRRP_SCHEMA_NAME, BRRP_REL_PathwayOwnsLinearlyLocatedAttribution)));
    
    auto retVal = new DesignSpeedTransition(params, linearParams);
    retVal->_OnCreate(linearParams); 
    return retVal; 
    }