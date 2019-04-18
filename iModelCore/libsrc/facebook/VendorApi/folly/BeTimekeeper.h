/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#pragma once

#if !defined (BENTLEY_CONFIG_NO_THREAD_SUPPORT)

#include <folly/BeFolly.h>
#include <folly/futures/Future.h>

namespace BeFolly {

/*--------------------------------------------------------------------------------------+
* @bsiclass
* Implements folly::Timekeeper for folly::futures::sleep() to work - that is not available
* in build due to large amount of dependencies.
* Implementation uses one thread for waiting and executing after() requests.
+---------------+---------------+---------------+---------------+---------------+------*/
struct BeTimekeeper : folly::Timekeeper
    {
    private:
        struct Callback
            {
            folly::Promise<folly::Unit> promise;
            int64_t executionTime;
            };
        typedef std::shared_ptr<Callback> CallbackPtr;

        struct CallbackCompare
            {
            bool operator()(const CallbackPtr& a, const CallbackPtr& b) const
                {
                return a->executionTime > b->executionTime;
                }
            };

        typedef std::priority_queue<CallbackPtr, std::vector<CallbackPtr>, CallbackCompare> CallbackQueue;

        struct Context
            {
            std::atomic_bool m_stopping;
            mutable std::mutex m_mutex;
            std::condition_variable m_hasWorkCV;
            CallbackQueue m_queue;

            void ProcessQueue();
            void PopTimedCallbacks(int64_t currentTime);
            };

    private:
        Context* m_ctx;

    protected:
        //! Multiple instances is to be avoided as each uses its own thread for delays.
        BE_FOLLY_EXPORT BeTimekeeper();

    public:
        BE_FOLLY_EXPORT virtual ~BeTimekeeper();

        //! Use signeton to ensure that only one thread is wasted for delays.
        BE_FOLLY_EXPORT static BeTimekeeper& Get();

        //! Implements Timekeeper.
        //! Consider using seperate via executor to avoid delaying timer thread and making it inacurate.
        //! This is mostly important if delaying code that takes time to complete, so using seperate executor like BeFolly::ThreadPool::GetIoPool() is recommended.
        BE_FOLLY_EXPORT folly::Future<folly::Unit> after(folly::Duration duration) override;
    };

} // namespace BeFolly

#endif //BENTLEY_CONFIG_NO_THREAD_SUPPORT