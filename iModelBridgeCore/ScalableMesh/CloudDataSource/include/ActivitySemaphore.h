#pragma once

#include <atomic>
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
                                        ActivitySemaphore       (CounterValue value);
                                       ~ActivitySemaphore       (void);

    void                                setCounter              (CounterValue value);
    CounterValue                        getCounter              (void);

    void                                increment               (void);
    bool                                decrement               (void);

    void                                wait                    (void);
    Status                              waitFor                 (Timeout timeout);

    void                                releaseAll              (void);
};

