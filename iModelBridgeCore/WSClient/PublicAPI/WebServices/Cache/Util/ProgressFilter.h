/*--------------------------------------------------------------------------------------+
|
|   Supplied under applicable software license agreement.
|
|   Copyright (c) 2016 Bentley Systems, Incorporated. All rights reserved.
|
+---------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__
#include <WebServices/WebServices.h>
#include <Bentley/BeTimeUtilities.h>

BEGIN_BENTLEY_WEBSERVICES_NAMESPACE
/*--------------------------------------------------------------------------------------+
* @bsiclass                                                     Bentley Systems 02/2016
+---------------+---------------+---------------+---------------+---------------+------*/

struct ProgressFilter
    {
    //! Create a filteredCallback that filters the given onProgress callback.
    //! The filteredCallback allow onProgress callback to be called just once
    //! for a given miliseconds range. On default this range is 250 miliseconds.
    //!
    //! @param[in] onProgress - the given callback to be filtered
    //! @param[in] waitInMs - the duration that should separate calls between two onProgress callbacks
    //! @param[in] shouldSkipFilter - the function that checks if filtering should be skipped
    //! @returns a callback that calls onProgress if duration of waitInMiliSeconds is passed

    template<typename Ret, typename ...Args>
    static const std::function<Ret(Args...)> Create
        (
        const std::function<Ret(Args...)>& onProgress,
        const std::function<bool(Args...)>& shouldSkipFilter,
        uint64_t waitInMs = 250
        )
        {
        uint64_t lastTimeReportedMs = 0;

        return [=] (Args... arg) mutable
            {
            if (onProgress == nullptr)
                return;

            bool shouldSkipFilterBool = shouldSkipFilter != nullptr ? shouldSkipFilter(arg...) : false;

            uint64_t currentTimeMs = BeTimeUtilities::GetCurrentTimeAsUnixMillis();
            if (currentTimeMs - lastTimeReportedMs >= waitInMs || shouldSkipFilterBool)
                {
                lastTimeReportedMs = currentTimeMs;
                onProgress(arg...);
                }
            };
        };

    template<typename Ret, typename ...Args>
    static const std::function<Ret(Args...)> Create
        (
        const std::function<Ret(Args...)>& onProgress,
        uint64_t waitInMs = 250
        )
        {
        std::function<bool(Args...)> shouldSkipFilter = nullptr;
        return Create(onProgress, shouldSkipFilter, waitInMs);
        };
    };

END_BENTLEY_WEBSERVICES_NAMESPACE
