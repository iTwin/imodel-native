/*--------------------------------------------------------------------------------------+
|
|     $Source: TypicalSectionPoint.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#include "RoadRailPhysicalInternal.h"
#include <RoadRailPhysical/TypicalSectionPoint.h>
#include <RoadRailPhysical/TypicalSection.h>
#include <RoadRailPhysical/RoadRailCategory.h>

HANDLER_DEFINE_MEMBERS(SignificantPointDefinitionHandler)
HANDLER_DEFINE_MEMBERS(TravelwaySignificantPointDefHandler)
HANDLER_DEFINE_MEMBERS(TravelwaySideSignificantPointDefHandler)
HANDLER_DEFINE_MEMBERS(TravelwayStructureSignificantPointDefHandler)

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      09/2017
+---------------+---------------+---------------+---------------+---------------+------*/
SignificantPointDefinition::SignificantPointDefinition(CreateParams const& params): T_Super(params)
    {
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      09/2017
+---------------+---------------+---------------+---------------+---------------+------*/
CodeSpecId SignificantPointDefinition::QueryCodeSpecId(DgnDbCR dgndb)
    {
    CodeSpecId codeSpecId = dgndb.CodeSpecs().QueryCodeSpecId(BRRP_CODESPEC_SignificantPointDefinition);
    BeAssert(codeSpecId.IsValid());
    return codeSpecId;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      09/2017
+---------------+---------------+---------------+---------------+---------------+------*/
DgnCode SignificantPointDefinition::CreateCode(DgnModelCR scope, Utf8StringCR value)
    {
    return CodeSpec::CreateCode(BRRP_CODESPEC_SignificantPointDefinition, scope, value);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      09/2017
+---------------+---------------+---------------+---------------+---------------+------*/
SignificantPointDefinitionCPtr SignificantPointDefinition::QueryByCode(DefinitionModelCR model, Utf8StringCR pointCode)
    {
    auto pointId = model.GetDgnDb().Elements().QueryElementIdByCode(CreateCode(model, pointCode));
    if (!pointId.IsValid())
        return nullptr;

    return SignificantPointDefinition::Get(model.GetDgnDb(), pointId);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      09/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TravelwaySignificantPointDefPtr TravelwaySignificantPointDef::Create(DefinitionModelCR model, Utf8StringCR pointCode, Utf8CP userLabel)
    {
    if (!model.GetModelId().IsValid())
        return nullptr;

    CreateParams createParams(model.GetDgnDb(), model.GetModelId(), QueryClassId(model.GetDgnDb()), CreateCode(model, pointCode));

    TravelwaySignificantPointDefPtr retVal(new TravelwaySignificantPointDef(createParams));
    retVal->SetExpectedAtSurface(ExpectedAtSurface::Top);
    retVal->SetGenerateLinearElement(false);

    if (userLabel)
        retVal->SetUserLabel(userLabel);

    return retVal;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      09/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TravelwaySignificantPointDefCPtr TravelwaySignificantPointDef::CreateAndInsert(DefinitionModelCR model, Utf8StringCR pointCode, Utf8CP userLabel)
    {
    auto ptr = Create(model, pointCode, userLabel);
    if (ptr.IsNull())
        return nullptr;

    return ptr->Insert();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      09/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TravelwaySideSignificantPointDefPtr TravelwaySideSignificantPointDef::Create(DefinitionModelCR model, Utf8StringCR pointCode, Utf8CP userLabel)
    {
    if (!model.GetModelId().IsValid())
        return nullptr;

    CreateParams createParams(model.GetDgnDb(), model.GetModelId(), QueryClassId(model.GetDgnDb()), CreateCode(model, pointCode));

    TravelwaySideSignificantPointDefPtr retVal(new TravelwaySideSignificantPointDef(createParams));
    retVal->SetExpectedAtSurface(ExpectedAtSurface::TopAndBottom);
    retVal->SetGenerateLinearElement(false);

    if (userLabel)
        retVal->SetUserLabel(userLabel);

    return retVal;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      09/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TravelwaySideSignificantPointDefCPtr TravelwaySideSignificantPointDef::CreateAndInsert(DefinitionModelCR model, Utf8StringCR pointCode, Utf8CP userLabel)
    {
    auto ptr = Create(model, pointCode, userLabel);
    if (ptr.IsNull())
        return nullptr;

    return ptr->Insert();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      09/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TravelwayStructureSignificantPointDefPtr TravelwayStructureSignificantPointDef::Create(DefinitionModelCR model, Utf8StringCR pointCode, Utf8CP userLabel)
    {
    if (!model.GetModelId().IsValid())
        return nullptr;

    CreateParams createParams(model.GetDgnDb(), model.GetModelId(), QueryClassId(model.GetDgnDb()), CreateCode(model, pointCode));

    TravelwayStructureSignificantPointDefPtr retVal(new TravelwayStructureSignificantPointDef(createParams));
    retVal->SetExpectedAtSurface(ExpectedAtSurface::Internal);
    retVal->SetGenerateLinearElement(false);

    if (userLabel)
        retVal->SetUserLabel(userLabel);

    return retVal;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      09/2017
+---------------+---------------+---------------+---------------+---------------+------*/
TravelwayStructureSignificantPointDefCPtr TravelwayStructureSignificantPointDef::CreateAndInsert(DefinitionModelCR model, Utf8StringCR pointCode, Utf8CP userLabel)
    {
    auto ptr = Create(model, pointCode, userLabel);
    if (ptr.IsNull())
        return nullptr;

    return ptr->Insert();
    }