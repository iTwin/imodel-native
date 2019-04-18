/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__

#include <Licensing/Licensing.h>
#include <folly/BeFolly.h>
#include <folly/futures/Future.h>

BEGIN_BENTLEY_LICENSING_NAMESPACE

/*--------------------------------------------------------------------------------------+
* @bsiclass
+---------------+---------------+---------------+---------------+---------------+------*/
typedef std::shared_ptr<struct IDelayedExecutor> IDelayedExecutorPtr;
struct IDelayedExecutor
{
public:
    virtual folly::Future<folly::Unit> Delayed(uint64_t ms) = 0;
};

/*--------------------------------------------------------------------------------------+
* @bsiclass
+---------------+---------------+---------------+---------------+---------------+------*/
typedef std::shared_ptr<struct DelayedExecutor> DelayedExecutorPtr;
struct DelayedExecutor : IDelayedExecutor
{
private:
    struct CallbackContainer
        {
        folly::Promise<folly::Unit> promise;
        int64_t executionTime;
        };
    typedef std::shared_ptr<CallbackContainer> CallbackContainerPtr;

    struct CallbackContainerCompare
        {
        bool operator()(const CallbackContainerPtr &a, const CallbackContainerPtr &b) const
            {
            return a->executionTime > b->executionTime;
            }
        };

    typedef std::priority_queue<CallbackContainerPtr, std::vector<CallbackContainerPtr>, CallbackContainerCompare> CallbackContainerQueue;

private:
    mutable std::mutex m_mutex;
    std::condition_variable m_hasWorkCV;

    CallbackContainerQueue m_queue;

    bool m_stop = false;

private:
    void RunThread();

    folly::Future<folly::Unit> AddCallback(uint64_t delay);
    void ProcessQueue();
    void PopTimedCallbacks(int64_t currentTime);

    DelayedExecutor();

public:
    LICENSING_EXPORT static DelayedExecutorPtr Get();


    //! Future will be completed after specified time. Time can be slightly bigger due to system lags.
    LICENSING_EXPORT virtual folly::Future<folly::Unit> Delayed(uint64_t ms) override;

    virtual ~DelayedExecutor();
};

END_BENTLEY_LICENSING_NAMESPACE
