/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__

#include <Licensing/Licensing.h>

#include <Bentley/BeTimeUtilities.h>

#include <memory>

BEGIN_BENTLEY_LICENSING_NAMESPACE

/*--------------------------------------------------------------------------------------+
* @bsiclass
+---------------+---------------+---------------+---------------+---------------+------*/
typedef std::shared_ptr<struct ITimeRetriever> ITimeRetrieverPtr;
struct ITimeRetriever
    {
public:
    virtual int64_t GetCurrentTimeAsUnixMillis() = 0;
    };

/*--------------------------------------------------------------------------------------+
* @bsiclass
+---------------+---------------+---------------+---------------+---------------+------*/
typedef std::shared_ptr<struct TimeRetriever> TimeRetrieverPtr;
struct TimeRetriever : ITimeRetriever
    {
public:
    static TimeRetrieverPtr Get()
        {
        static TimeRetrieverPtr s_instance = std::shared_ptr<TimeRetriever>(new TimeRetriever());
        return s_instance;
        }

    virtual int64_t GetCurrentTimeAsUnixMillis() override
        {
        return BeTimeUtilities::GetCurrentTimeAsUnixMillis();
        }

    virtual ~TimeRetriever() {}
    };

END_BENTLEY_LICENSING_NAMESPACE