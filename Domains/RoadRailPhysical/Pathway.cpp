/*--------------------------------------------------------------------------------------+
|
|     $Source: Pathway.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "RoadRailPhysicalInternal.h"
#include <RoadRailPhysical/Pathway.h>

HANDLER_DEFINE_MEMBERS(PathwayElementHandler)
HANDLER_DEFINE_MEMBERS(RailwayHandler)
HANDLER_DEFINE_MEMBERS(RoadwayHandler)

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      09/2016
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus PathwayElement::SetMainAlignment(AlignmentCP alignment)
    {
    return SetPropertyValue("MainAlignment", (alignment) ? alignment->GetElementId() : DgnElementId(), Alignment::QueryClassId(GetDgnDb()));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      01/2018
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus PathwayElement::AddRepresentedBy(PathwayElementCR pathway, DgnElementCR representedBy)
    {
    if (!representedBy.GetElementId().IsValid() || !pathway.GetElementId().IsValid())
        return DgnDbStatus::BadElement;

    Utf8String schemaName, relClassName;
    if (representedBy.ToGeometrySource2d())
        {
        schemaName = BIS_ECSCHEMA_NAME;
        relClassName = BIS_REL_DrawingGraphicRepresentsElement;
        }
    else
        {
        schemaName = BRRA_SCHEMA_NAME;
        relClassName = BRRA_REL_ElementRepresentsAlignment;
        }

    ECInstanceKey insKey;
    if (DbResult::BE_SQLITE_OK != pathway.GetDgnDb().InsertLinkTableRelationship(insKey,
        *pathway.GetDgnDb().Schemas().GetClass(schemaName, relClassName)->GetRelationshipClassCP(),
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