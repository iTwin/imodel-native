/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/

#include "RoadRailPhysicalInternal.h"
#include <RoadRailPhysical/TypicalSectionPoint.h>
#include <RoadRailPhysical/TypicalSection.h>
#include <RoadRailPhysical/RoadRailCategory.h>

HANDLER_DEFINE_MEMBERS(TypicalSectionPointDefinitionHandler)
HANDLER_DEFINE_MEMBERS(GenericTypicalSectionPointDefinitionHandler)

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      09/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TypicalSectionPointDefinition::TypicalSectionPointDefinition(CreateParams const& params): T_Super(params)
    {
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      09/2017
+---------------+---------------+---------------+---------------+---------------+------*/
CodeSpecId TypicalSectionPointDefinition::QueryCodeSpecId(DgnDbCR dgndb)
    {
    CodeSpecId codeSpecId = dgndb.CodeSpecs().QueryCodeSpecId(BRRP_CODESPEC_TypicalSectionPointDefinition);
    BeAssert(codeSpecId.IsValid());
    return codeSpecId;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      09/2017
+---------------+---------------+---------------+---------------+---------------+------*/
DgnCode TypicalSectionPointDefinition::CreateCode(DefinitionModelCR scope, Utf8StringCR value)
    {
    return CodeSpec::CreateCode(BRRP_CODESPEC_TypicalSectionPointDefinition, scope, value);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      09/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TypicalSectionPointDefinitionCPtr TypicalSectionPointDefinition::QueryByCode(DefinitionModelCR model, Utf8StringCR pointCode)
    {
    auto pointId = model.GetDgnDb().Elements().QueryElementIdByCode(CreateCode(model, pointCode));
    if (!pointId.IsValid())
        return nullptr;

    return TypicalSectionPointDefinition::Get(model.GetDgnDb(), pointId);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      09/2017
+---------------+---------------+---------------+---------------+---------------+------*/
GenericTypicalSectionPointDefinitionPtr GenericTypicalSectionPointDefinition::Create(DefinitionModelCR model, Utf8StringCR pointCode, Utf8CP userLabel)
    {
    if (!model.GetModelId().IsValid())
        return nullptr;

    CreateParams createParams(model.GetDgnDb(), model.GetModelId(), QueryClassId(model.GetDgnDb()), CreateCode(model, pointCode));

    GenericTypicalSectionPointDefinitionPtr retVal(new GenericTypicalSectionPointDefinition(createParams));

    if (userLabel)
        retVal->SetUserLabel(userLabel);

    return retVal;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      09/2017
+---------------+---------------+---------------+---------------+---------------+------*/
GenericTypicalSectionPointDefinitionCPtr GenericTypicalSectionPointDefinition::CreateAndInsert(DefinitionModelCR model, Utf8StringCR pointCode, Utf8CP userLabel)
    {
    auto ptr = Create(model, pointCode, userLabel);
    if (ptr.IsNull())
        return nullptr;

    return ptr->Insert();
    }