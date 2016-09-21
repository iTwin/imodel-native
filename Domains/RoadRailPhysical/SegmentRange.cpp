/*--------------------------------------------------------------------------------------+
|
|     $Source: SegmentRange.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <RoadRailPhysicalInternal.h>

HANDLER_DEFINE_MEMBERS(SegmentRangeElementHandler)
HANDLER_DEFINE_MEMBERS(RoadRangeHandler)

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      09/2016
+---------------+---------------+---------------+---------------+---------------+------*/
RoadRangePtr RoadRange::Create(SpatialModelR model, AlignmentCR alignment)
    {
    if (!alignment.GetElementId().IsValid() || !model.GetModelId().IsValid())
        return nullptr;

    CreateParams createParams(model.GetDgnDb(), model.GetModelId(), QueryClassId(model.GetDgnDb()),
        RoadRailPhysicalDomain::QueryRoadCategoryId(alignment.GetDgnDb()));

    return new RoadRange(createParams, alignment);
    }