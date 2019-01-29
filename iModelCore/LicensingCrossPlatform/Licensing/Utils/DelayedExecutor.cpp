/*--------------------------------------------------------------------------------------+
|
|     $Source: Licensing/Utils/DelayedExecutor.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <Licensing/Utils/DelayedExecutor.h>
#include <folly/Singleton.h>

#include <Bentley/DateTime.h>

#include <thread>

USING_NAMESPACE_BENTLEY_LICENSING

int64_t GetCurrentUnixMilliseconds()
{
    return BeTimeUtilities::GetCurrentTimeAsUnixMillis();
}

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DelayedExecutor::DelayedExecutor()
    {
    std::thread t([this]
        {
        ProcessQueue();
        });
    t.detach();
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DelayedExecutorPtr DelayedExecutor::Get()
    {
    static DelayedExecutorPtr s_instance = std::shared_ptr<DelayedExecutor>(new DelayedExecutor());
    return s_instance;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void DelayedExecutor::RunThread()
    {
    ProcessQueue();
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void DelayedExecutor::ProcessQueue()
    {
    auto currentTime = GetCurrentUnixMilliseconds();

    while (true)
        {
        std::unique_lock<std::mutex> lk(m_mutex);

        PopTimedCallbacks(currentTime);

        if (m_queue.empty())
            m_hasWorkCV.wait(lk, [this]
                {
                return !m_queue.empty();
                });
        else
            {
            auto sleepFor = m_queue.top()->executionTime - currentTime;
            m_hasWorkCV.wait_for(lk, std::chrono::milliseconds(sleepFor), [this, &currentTime, &sleepFor]
                {
                auto newCurrentTime = GetCurrentUnixMilliseconds();
                auto timeElapsed = newCurrentTime - currentTime;
                currentTime = newCurrentTime;

                auto timeLeftToSleep = sleepFor - timeElapsed;

                auto newSleepFor = m_queue.top()->executionTime - currentTime;

                return newSleepFor < timeLeftToSleep;
                });
            }
        }
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void DelayedExecutor::PopTimedCallbacks(int64_t currentTime)
    {
    if (m_queue.empty())
        return;

    if (m_queue.top()->executionTime > currentTime)
        return;

    m_queue.top()->promise.setValue();
    m_queue.pop();

    PopTimedCallbacks(currentTime);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
folly::Future<folly::Unit> DelayedExecutor::AddCallback(uint64_t delay)
    {
    auto container = std::make_shared<CallbackContainer>();
    container->executionTime = GetCurrentUnixMilliseconds() + delay;
    auto future = container->promise.getFuture().via(&BeFolly::ThreadPool::GetCpuPool());

    std::unique_lock<std::mutex> lk(m_mutex);
    m_queue.push(container);
    lk.unlock();

    m_hasWorkCV.notify_one();

    return future;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
folly::Future<folly::Unit> DelayedExecutor::Delayed(uint64_t ms)
    {
    return AddCallback(ms);
    }
