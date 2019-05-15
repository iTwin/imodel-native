/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#include "RoadRailPhysicalInternal.h"
#include <RoadRailPhysical/DesignSpeed.h>
#include <RoadRailPhysical/RoadRailPhysicalDomain.h>

HANDLER_DEFINE_MEMBERS(DesignSpeedHandler)
HANDLER_DEFINE_MEMBERS(DesignSpeedDefinitionHandler)

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
DesignSpeedDefinition::DesignSpeedDefinition(CreateParams const& params, double designSpeedInMPerSec, UnitSystem unitSystem):
    T_Super(params)
    {
    SetPropertyValue(BRRP_PROP_DesignSpeedDefinition_DesignSpeed, designSpeedInMPerSec);
    SetPropertyValue(BRRP_PROP_DesignSpeedDefinition_UnitSystem, static_cast<int32_t>(unitSystem));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      11/2016
+---------------+---------------+---------------+---------------+---------------+------*/
DgnCode DesignSpeedDefinition::CreateCode(DefinitionModelCR scope, double designSpeedInCodeUnits, UnitSystem unitSystem)
    {
    Utf8String suffix = (unitSystem == UnitSystem::SI) ? "Km/h" : "mph";
    return CodeSpec::CreateCode(BRRP_CODESPEC_DesignSpeedDefinition, scope,
        Utf8PrintfString("%.0f %s", designSpeedInCodeUnits, suffix.c_str()));
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
DesignSpeedDefinitionPtr DesignSpeedDefinition::Create(DefinitionModelCR model, double designSpeedInCodeUnits, UnitSystem unitSystem)
    {
    CreateParams params(model.GetDgnDb(), model.GetModelId(), QueryClassId(model.GetDgnDb()), CreateCode(model, designSpeedInCodeUnits, unitSystem));
    
    auto kphUnitCP = model.GetDgnDb().Schemas().GetUnit("Units", "KM_PER_HR");
    auto mphUnitCP = model.GetDgnDb().Schemas().GetUnit("Units", "MPH");
    Units::Quantity speedQty(designSpeedInCodeUnits, (unitSystem == UnitSystem::SI) ? *kphUnitCP : *mphUnitCP);

    auto mPerSecUnitCP = model.GetDgnDb().Schemas().GetUnit("Units", "M_PER_SEC");
    auto mPerSecSpeedQty = speedQty.ConvertTo(mPerSecUnitCP);
    return new DesignSpeedDefinition(params, mPerSecSpeedQty.GetMagnitude(), unitSystem);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      11/2018
+---------------+---------------+---------------+---------------+---------------+------*/
DesignSpeedDefinition::UnitSystem DesignSpeedDefinition::GetUnitSystem() const
    {
    return static_cast<UnitSystem>(GetPropertyValueInt32(BRRP_PROP_DesignSpeedDefinition_UnitSystem));
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
DesignSpeed::DesignSpeed(CreateParams const& params, CreateFromToParams const& fromToParams) :
    T_Super(params), ICorridorPortionSingleFromTo(fromToParams)
    {
    SetStartDefinition(*fromToParams.m_startDesignSpeedDefCPtr);
    SetEndDefinition(*fromToParams.m_endDesignSpeedDefCPtr);
    _AddLinearlyReferencedLocation(*_GetUnpersistedFromToLocation());    
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      12/2016
+---------------+---------------+---------------+---------------+---------------+------*/
void DesignSpeed::SetStartDefinition(DesignSpeedDefinitionCR designSpeedDef)
    { 
    SetPropertyValue(BRRP_PROP_DesignSpeed_StartDefinition, designSpeedDef.GetElementId(),
        GetDgnDb().Schemas().GetClassId(BRRP_SCHEMA_NAME, BRRP_REL_DesignSpeedRefersToStartDefinition));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      12/2016
+---------------+---------------+---------------+---------------+---------------+------*/
void DesignSpeed::SetEndDefinition(DesignSpeedDefinitionCR designSpeedDef)
    {
    SetPropertyValue(BRRP_PROP_DesignSpeed_EndDefinition, designSpeedDef.GetElementId(),
        GetDgnDb().Schemas().GetClassId(BRRP_SCHEMA_NAME, BRRP_REL_DesignSpeedRefersToEndDefinition));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      02/2018
+---------------+---------------+---------------+---------------+---------------+------*/
DesignSpeedPtr DesignSpeed::Create(CreateFromToParams const& linearParams) 
    {
    if (!ValidateParams(linearParams)) 
        return nullptr; 

    auto& dgnDbR = linearParams.m_corridorPortionCPtr->GetDgnDb();
    CreateParams params(dgnDbR, linearParams.m_corridorPortionCPtr->GetModelId(), QueryClassId(dgnDbR),
        RoadRailCategory::GetDesignSpeed(dgnDbR));
    params.SetParentId(linearParams.m_corridorPortionCPtr->GetElementId(),
        DgnClassId(dgnDbR.Schemas().GetClassId(BLR_SCHEMA_NAME, BLR_REL_ILinearElementSourceOwnsAttributions)));
    
    auto retVal = new DesignSpeed(params, linearParams);
    retVal->_OnCreate(linearParams); 
    return retVal; 
    }