/*--------------------------------------------------------------------------------------+
|
|     $Source: RoadClass.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <RoadRailPhysicalInternal.h>

HANDLER_DEFINE_MEMBERS(RoadClassDefinitionModelHandler)
HANDLER_DEFINE_MEMBERS(RoadClassDefinitionTableHandler)
HANDLER_DEFINE_MEMBERS(RoadClassDefinitionTableModelHandler)
HANDLER_DEFINE_MEMBERS(RoadClassDefinitionHandler)
HANDLER_DEFINE_MEMBERS(RoadClassHandler)
HANDLER_DEFINE_MEMBERS(RoadClassStandardsHandler)

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      11/2016
+---------------+---------------+---------------+---------------+---------------+------*/
RoadClassStandards::RoadClassStandards(CreateParams const& params):
    T_Super(params)
    {
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      11/2016
+---------------+---------------+---------------+---------------+---------------+------*/
RoadClassStandardsPtr RoadClassStandards::Create(RoadwayStandardsModelCR model)
    {
    CreateParams params(model.GetDgnDb(), model.GetModelId(), QueryClassId(model.GetDgnDb()));
    return new RoadClassStandards(params);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      11/2016
+---------------+---------------+---------------+---------------+---------------+------*/
RoadClassStandardsCPtr RoadClassStandards::Insert(RoadClassDefinitionTableModelPtr& breakDownModelPtr, Dgn::DgnDbStatus* stat)
    {
    auto retVal = GetDgnDb().Elements().Insert<RoadClassStandards>(*this, stat);
    if (retVal.IsNull())
        return nullptr;

    breakDownModelPtr = RoadClassDefinitionTableModel::Create(DgnModel::CreateParams(GetDgnDb(), RoadClassDefinitionTableModel::QueryClassId(GetDgnDb()),
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
RoadClassDefinitionTable::RoadClassDefinitionTable(CreateParams const& params):
    T_Super(params)
    {
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      11/2016
+---------------+---------------+---------------+---------------+---------------+------*/
DgnCode RoadClassDefinitionTable::CreateCode(DgnDbR dgndb, Utf8StringCR value)
    {
    return CodeSpec::CreateCode(dgndb, BRRP_CODESPEC_RoadClassDefinitionTable, value);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      11/2016
+---------------+---------------+---------------+---------------+---------------+------*/
RoadClassDefinitionTablePtr RoadClassDefinitionTable::Create(RoadClassDefinitionTableModelCR model, Utf8StringCR code)
    {
    CreateParams params(model.GetDgnDb(), model.GetModelId(), QueryClassId(model.GetDgnDb()), CreateCode(model.GetDgnDb(), code));
    return new RoadClassDefinitionTable(params);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      11/2016
+---------------+---------------+---------------+---------------+---------------+------*/
RoadClassDefinitionTableCPtr RoadClassDefinitionTable::Insert(RoadClassDefinitionModelPtr& breakDownModelPtr, Dgn::DgnDbStatus* stat)
    {
    auto retVal = GetDgnDb().Elements().Insert<RoadClassDefinitionTable>(*this, stat);
    if (retVal.IsNull())
        return nullptr;

    breakDownModelPtr = RoadClassDefinitionModel::Create(DgnModel::CreateParams(GetDgnDb(), RoadClassDefinitionModel::QueryClassId(GetDgnDb()),
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
RoadClassDefinition::RoadClassDefinition(CreateParams const& params):
    T_Super(params)
    {
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      11/2016
+---------------+---------------+---------------+---------------+---------------+------*/
DgnCode RoadClassDefinition::CreateCode(Dgn::DgnDbR dgndb, Utf8StringCR roadClass)
    {
    return CodeSpec::CreateCode(dgndb, BRRP_CODESPEC_RoadClassDefinition, roadClass);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      11/2016
+---------------+---------------+---------------+---------------+---------------+------*/
RoadClassDefinitionPtr RoadClassDefinition::Create(RoadClassDefinitionModelCR model, Utf8StringCR roadClassCode, Utf8CP roadClassLabel)
    {
    auto tableCPtr = model.GetModeledClassDefinitionTable();

    CreateParams params(model.GetDgnDb(), model.GetModelId(), QueryClassId(model.GetDgnDb()), CreateCode(model.GetDgnDb(), roadClassCode));

    auto retVal = new RoadClassDefinition(params);

    retVal->SetUserLabel(roadClassLabel);

    return retVal;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      10/2016
+---------------+---------------+---------------+---------------+---------------+------*/
RoadClass::RoadClass(CreateParams const& params):
    T_Super(params)
    {
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      10/2016
+---------------+---------------+---------------+---------------+---------------+------*/
RoadClass::RoadClass(CreateParams const& params, RoadClassDefinitionCR ClassDef, double fromDistanceAlong, double toDistanceAlong):
    T_Super(params), ILinearlyLocatedSingleFromTo(fromDistanceAlong, toDistanceAlong)
    {
    SetClassDefinition(ClassDef);
    _AddLinearlyReferencedLocation(*_GetUnpersistedFromToLocation());    
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      10/2016
+---------------+---------------+---------------+---------------+---------------+------*/
RoadClassPtr RoadClass::Create(RoadwayCR roadway, RoadClassDefinitionCR ClassDef, double fromDistanceAlong, double toDistanceAlong)
    {
    if (!roadway.GetElementId().IsValid())
        return nullptr;

    auto alignmentId = roadway.GetAlignmentId();
    if (!alignmentId.IsValid())
        return nullptr;

    CreateParams params(roadway.GetDgnDb(), roadway.GetModelId(), QueryClassId(roadway.GetDgnDb()));
    params.SetParentId(roadway.GetElementId(),
        DgnClassId(roadway.GetDgnDb().Schemas().GetClassId(BRRP_SCHEMA_NAME, BRRP_REL_RoadwayHasRoadClasses)));

    auto retVal = new RoadClass(params, ClassDef, fromDistanceAlong, toDistanceAlong);
    retVal->_SetLinearElement(alignmentId);
    return retVal;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      12/2016
+---------------+---------------+---------------+---------------+---------------+------*/
void RoadClass::SetClassDefinition(RoadClassDefinitionCR ClassDef) 
    { 
    SetPropertyValue("RoadClassDefinition", ClassDef.GetElementId(),
        GetDgnDb().Schemas().GetClassId(BRRP_SCHEMA_NAME, BRRP_REL_RoadClassRefersToDefinition));
    }