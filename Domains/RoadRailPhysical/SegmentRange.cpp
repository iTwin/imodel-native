/*--------------------------------------------------------------------------------------+
|
|     $Source: SegmentRange.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <RoadRailPhysicalInternal.h>

HANDLER_DEFINE_MEMBERS(SegmentRangeElementHandler)
HANDLER_DEFINE_MEMBERS(RailRangeHandler)
HANDLER_DEFINE_MEMBERS(RoadRangeHandler)

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      09/2016
+---------------+---------------+---------------+---------------+---------------+------*/
RoadRange::RoadRange(CreateParams const& params, AlignmentCR alignment):
    T_Super(params)
    {
    SetAlignmentId(alignment);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      09/2016
+---------------+---------------+---------------+---------------+---------------+------*/
RoadRangePtr RoadRange::Create(PhysicalModelR model, AlignmentCR alignment)
    {
    if (!model.GetModelId().IsValid())
        return nullptr;

    CreateParams createParams(model.GetDgnDb(), model.GetModelId(), QueryClassId(model.GetDgnDb()),
        RoadRailPhysicalDomain::QueryRoadCategoryId(model.GetDgnDb()));

    return new RoadRange(createParams, alignment);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      09/2016
+---------------+---------------+---------------+---------------+---------------+------*/
RailRange::RailRange(CreateParams const& params, AlignmentCR alignment):
    T_Super(params)
    {
    SetAlignmentId(alignment);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      09/2016
+---------------+---------------+---------------+---------------+---------------+------*/
RailRangePtr RailRange::Create(PhysicalModelR model, AlignmentCR alignment)
    {
    if (!model.GetModelId().IsValid())
        return nullptr;

    CreateParams createParams(model.GetDgnDb(), model.GetModelId(), QueryClassId(model.GetDgnDb()),
        RoadRailPhysicalDomain::QueryTrackCategoryId(model.GetDgnDb()));

    return new RailRange(createParams, alignment);
    }