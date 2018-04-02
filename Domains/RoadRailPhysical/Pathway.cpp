/*--------------------------------------------------------------------------------------+
|
|     $Source: Pathway.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "RoadRailPhysicalInternal.h"
#include <RoadRailPhysical/Pathway.h>
#include <RoadRailPhysical/TypicalSectionPoint.h>

HANDLER_DEFINE_MEMBERS(PathwayElementHandler)
HANDLER_DEFINE_MEMBERS(PathwayPortionElementHandler)
HANDLER_DEFINE_MEMBERS(RailwayHandler)
HANDLER_DEFINE_MEMBERS(RoadwayHandler)
HANDLER_DEFINE_MEMBERS(ThruTravelCompositeHandler)
HANDLER_DEFINE_MEMBERS(ThruTravelSeparationCompositeHandler)
HANDLER_DEFINE_MEMBERS(ThruwayCompositeHandler)
HANDLER_DEFINE_MEMBERS(ThruwaySeparationCompositeHandler)

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      09/2016
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus IMainLinearElementSource::SetMainLinearElement(ILinearElementCP linearElement)
    {
    auto& dgnElement = *const_cast<DgnElementP>(&_ILinearElementSourceToDgnElement());
    if (linearElement)
        return dgnElement.SetPropertyValue("MainLinearElement", linearElement->ToElement().GetElementId(), linearElement->ToElement().GetElementClassId());
    else
        return dgnElement.SetPropertyValue("MainLinearElement", DgnElementId(), DgnClassId());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      01/2018
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus PathwayElement::AddRepresentedBy(PathwayElementCR pathway, DgnElementCR representedBy)
    {
    if (!representedBy.GetElementId().IsValid() || !pathway.GetElementId().IsValid())
        return DgnDbStatus::BadElement;

    Utf8String relClassName;
    if (representedBy.ToGeometrySource2d())
        relClassName = BRRP_REL_DrawingGraphicRepresentsPathway;
    else
        relClassName = BRRP_REL_GraphicalElement3dRepresentsPathway;

    ECInstanceKey insKey;
    if (DbResult::BE_SQLITE_OK != pathway.GetDgnDb().InsertLinkTableRelationship(insKey,
        *pathway.GetDgnDb().Schemas().GetClass(BRRP_SCHEMA_NAME, relClassName)->GetRelationshipClassCP(),
        ECInstanceId(representedBy.GetElementId().GetValue()), ECInstanceId(pathway.GetElementId().GetValue())))
        return DgnDbStatus::BadElement;

    return DgnDbStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      09/2016
+---------------+---------------+---------------+---------------+---------------+------*/
RoadwayPtr Roadway::Create(PhysicalModelR model)
    {
    if (!model.GetModelId().IsValid())
        return nullptr;

    CreateParams createParams(model.GetDgnDb(), model.GetModelId(), QueryClassId(model.GetDgnDb()), RoadRailCategory::GetRoad(model.GetDgnDb()));

    return new Roadway(createParams);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      09/2016
+---------------+---------------+---------------+---------------+---------------+------*/
RailwayPtr Railway::Create(PhysicalModelR model)
    {
    if (!model.GetModelId().IsValid())
        return nullptr;

    CreateParams createParams(model.GetDgnDb(), model.GetModelId(), QueryClassId(model.GetDgnDb()), RoadRailCategory::GetTrack(model.GetDgnDb()));

    return new Railway(createParams);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      03/2018
+---------------+---------------+---------------+---------------+---------------+------*/
ThruwayCompositePtr ThruwayComposite::Create(PathwayElementCR pathway)
    {
    if (!pathway.GetElementId().IsValid())
        return nullptr;

    CreateParams params(pathway.GetDgnDb(), pathway.GetModelId(), QueryClassId(pathway.GetDgnDb()), pathway.GetCategoryId());
    params.SetParentId(pathway.GetElementId(), 
        DgnClassId(pathway.GetDgnDb().Schemas().GetClassId(BRRP_SCHEMA_NAME, BRRP_REL_PathwayAssemblesElements)));

    return new ThruwayComposite(params);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      03/2018
+---------------+---------------+---------------+---------------+---------------+------*/
ThruwaySeparationCompositePtr ThruwaySeparationComposite::Create(PathwayElementCR pathway)
    {
    if (!pathway.GetElementId().IsValid())
        return nullptr;

    CreateParams params(pathway.GetDgnDb(), pathway.GetModelId(), QueryClassId(pathway.GetDgnDb()), pathway.GetCategoryId());
    params.SetParentId(pathway.GetElementId(), 
        DgnClassId(pathway.GetDgnDb().Schemas().GetClassId(BRRP_SCHEMA_NAME, BRRP_REL_PathwayAssemblesElements)));

    return new ThruwaySeparationComposite(params);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      03/2018
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus ILinearElementUtilities::SetAssociatedSignificantPointDef(ILinearElementCR linearElement, SignificantPointDefinitionCP significantPointDef)
    {
    if (!linearElement.ToElement().GetElementId().IsValid() || (significantPointDef && !significantPointDef->GetElementId().IsValid()))
        return DgnDbStatus::BadArg;

    auto& linearElementCR = linearElement.ToElement();

    auto stmtPtr = linearElementCR.GetDgnDb().GetPreparedECSqlStatement("SELECT ECClassId, ECInstanceId FROM "
        BRRP_SCHEMA(BRRP_REL_ILinearElementAssociatedWithSignificantPointDef) " WHERE SourceECInstanceId = ?;");
    BeAssert(stmtPtr.IsValid());

    stmtPtr->BindId(1, linearElementCR.GetElementId());
    if (DbResult::BE_SQLITE_ROW == stmtPtr->Step())
        linearElementCR.GetDgnDb().DeleteLinkTableRelationship(ECInstanceKey(stmtPtr->GetValueId<ECClassId>(0), stmtPtr->GetValueId<ECInstanceId>(1)));

    auto relClassCP = linearElementCR.GetDgnDb().Schemas().GetClass(BRRP_SCHEMA_NAME, BRRP_REL_ILinearElementAssociatedWithSignificantPointDef)->GetRelationshipClassCP();

    ECInstanceKey key;
    if (DbResult::BE_SQLITE_OK != linearElementCR.GetDgnDb().InsertLinkTableRelationship(key,
        *relClassCP, linearElementCR.GetElementId(), significantPointDef->GetElementId()))
            return DgnDbStatus::WriteError;

    return DgnDbStatus::Success;
    }