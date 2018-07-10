/*--------------------------------------------------------------------------------------+
|
|     $Source: Tests/iModelHubClient/Integration/RequestBehaviorOptions.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "RequestBehaviorOptions.h"

BEGIN_BENTLEY_IMODELHUB_UNITTESTS_NAMESPACE

//---------------------------------------------------------------------------------------
//@bsimethod                                    Karolis.Uzkuraitis            07/2018
//---------------------------------------------------------------------------------------
RequestBehaviorOptionsEnum operator| (RequestBehaviorOptionsEnum a, RequestBehaviorOptionsEnum b)
    {
    return static_cast<RequestBehaviorOptionsEnum>(static_cast<int>(a) | static_cast<int>(b));
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                    Karolis.Uzkuraitis            07/2018
//---------------------------------------------------------------------------------------
RequestBehaviorOptionsEnum operator& (RequestBehaviorOptionsEnum a, RequestBehaviorOptionsEnum b)
    {
    return static_cast<RequestBehaviorOptionsEnum>(static_cast<int>(a) & static_cast<int>(b));
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                    Karolis.Uzkuraitis            07/2018
//---------------------------------------------------------------------------------------
RequestBehaviorOptionsEnum operator~ (RequestBehaviorOptionsEnum a)
    {
    return static_cast<RequestBehaviorOptionsEnum>(~static_cast<int>(a));
    }

//---------------------------------------------------------------------------------------
//@bsimethod                                    Karolis.Uzkuraitis            07/2018
//---------------------------------------------------------------------------------------
bpair<Utf8String, Utf8String> RequestBehaviorOptions::GetBehaviorOptionsResultPair()
        {
        std::vector<Utf8String> nameList;
        if (RequestBehaviorOptionsEnum::DoNotScheduleRenderThumbnailJob == (m_behaviorOptions & RequestBehaviorOptionsEnum::DoNotScheduleRenderThumbnailJob))
            nameList.push_back("DoNotScheduleRenderThumbnailJob");
        if (RequestBehaviorOptionsEnum::DisableGlobalEvents == (m_behaviorOptions & RequestBehaviorOptionsEnum::DisableGlobalEvents))
            nameList.push_back("DisableGlobalEvents");

        Utf8String outString = "";
        for (unsigned int i = 0; i < nameList.size(); i++)
            {
            outString += nameList[i];
            if (i < nameList.size() - 1)
                {
                outString += ",";
                }
            }
        return bpair<Utf8String, Utf8String>("BehaviourOptions", outString);
        }

END_BENTLEY_IMODELHUB_UNITTESTS_NAMESPACE