/*--------------------------------------------------------------------------------------+
|
|     $Source: Tests/iModelHubClient/Integration/RequestBehaviorOptions.h $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
#include "Helpers.h"

BEGIN_BENTLEY_IMODELHUB_UNITTESTS_NAMESPACE

//=======================================================================================
//@bsiclass                                      Karolis.Uzkuraitis             07/2018
//=======================================================================================
enum class RequestBehaviorOptionsEnum
    {
    DoNotScheduleRenderThumbnailJob = 1 << 0,
    DisableGlobalEvents = 1 << 1,
    DisableNotifications = 1 << 2
    };

RequestBehaviorOptionsEnum operator| (RequestBehaviorOptionsEnum a, RequestBehaviorOptionsEnum b);

RequestBehaviorOptionsEnum operator& (RequestBehaviorOptionsEnum a, RequestBehaviorOptionsEnum b);

RequestBehaviorOptionsEnum operator~ (RequestBehaviorOptionsEnum a);

//=======================================================================================
//@bsiclass                                      Karolis.Uzkuraitis             07/2018
//=======================================================================================
struct RequestBehaviorOptions
    {
private:
    RequestBehaviorOptionsEnum m_behaviorOptions = 
        RequestBehaviorOptionsEnum::DoNotScheduleRenderThumbnailJob | RequestBehaviorOptionsEnum::DisableGlobalEvents | RequestBehaviorOptionsEnum::DisableNotifications;

public:
    void EnableOption(RequestBehaviorOptionsEnum option)
        {
        m_behaviorOptions = m_behaviorOptions | option;
        }

    void DisableOption(RequestBehaviorOptionsEnum option)
        {
        m_behaviorOptions = m_behaviorOptions & ~option;
        }

    bpair<Utf8String, Utf8String> GetBehaviorOptionsResultPair();
    };

END_BENTLEY_IMODELHUB_UNITTESTS_NAMESPACE