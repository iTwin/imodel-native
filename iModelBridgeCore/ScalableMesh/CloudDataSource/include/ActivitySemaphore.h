#pragma once

#include <atomic>
#include <mutex>
#include <condition_variable>


class ActivitySemaphore
{
public:

    typedef unsigned int                Counter;
    typedef std::chrono::milliseconds   Timeout;

    enum Status
    {
        Status_Error,

        Status_NoTimeout,
        Status_Timeout
    };

protected:

    std::mutex                          mutex;
    std::condition_variable             condition;
    std::atomic<Counter>                counter;

public:
                                        ActivitySemaphore       (void);
                                        ActivitySemaphore       (Counter value);
                                       ~ActivitySemaphore       (void);

    void                                setCounter              (Counter value);
    Counter                             getCounter              (void);

    void                                increment               (void);
    bool                                decrement               (void);

    void                                wait                    (void);
    Status                              waitFor                 (Timeout timeout);

    void                                releaseAll              (void);
};

