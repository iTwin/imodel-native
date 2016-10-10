/*--------------------------------------------------------------------------------------+
|
|     $Source: RoadDesignSpeed.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <RoadRailPhysicalInternal.h>

HANDLER_DEFINE_MEMBERS(RoadDesignSpeedHandler)

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
RoadDesignSpeed::RoadDesignSpeed(CreateParams const& params, double fromDistanceAlong, double toDistanceAlong):
    T_Super(params), ILinearlyLocatedSingleFromTo(fromDistanceAlong, toDistanceAlong)
    {
    _AddLinearlyReferencedLocation(*_GetUnpersistedFromToLocation());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Diego.Diaz                      10/2016
+---------------+---------------+---------------+---------------+---------------+------*/
RoadDesignSpeedPtr RoadDesignSpeed::Create(RoadRangeCR roadRange, double fromDistanceAlong, double toDistanceAlong)
    {
    if (!roadRange.GetElementId().IsValid())
        return nullptr;

    auto alignmentId = roadRange.QueryAlignmentId();
    if (!alignmentId.IsValid())
        return nullptr;

    CreateParams params(roadRange.GetDgnDb(), roadRange.GetModelId(), QueryClassId(roadRange.GetDgnDb()));
    params.SetParentId(roadRange.GetElementId());

    auto retVal = new RoadDesignSpeed(params, fromDistanceAlong, toDistanceAlong);
    retVal->_SetLinearElementId(alignmentId);
    return retVal;
    }