/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once

#include <mutex>
#include <condition_variable>
#include <atomic>


class ActivitySemaphore
{
public:

    typedef int                         CounterValue;
    typedef std::atomic<CounterValue>   Counter;
    typedef std::chrono::milliseconds   Timeout;

    enum Status
    {
        Status_Error,

        Status_NoTimeout,
        Status_Timeout
    };

protected:

    std::condition_variable             condition;
    Counter                             counter;
    std::mutex                          mutex;

public:
                                        ActivitySemaphore       (void);
                               explicit ActivitySemaphore       (CounterValue value);
                                       ~ActivitySemaphore       (void);

    void                                setCounter              (CounterValue value);
    CounterValue                        getCounter              (void);

    void                                increment               (void);
    bool                                decrement               (void);

    void                                wait                    (void);
    Status                              waitFor                 (Timeout timeout);

    void                                releaseAll              (void);
};

