/*--------------------------------------------------------------------------------------+
|
|     $Source: folly/BeTimekeeper.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <folly/BeTimekeeper.h>
#include <folly/Singleton.h>
#include <folly/executors/InlineExecutor.h>

#include <Bentley/DateTime.h>

#include <thread>

using namespace BeFolly;

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
int64_t GetCurrentTimeAsUnixMillis()
    {
    return BeTimeUtilities::GetCurrentTimeAsUnixMillis();
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BeTimekeeper::BeTimekeeper()
    {
    auto ctx = new Context();
    std::thread thread([ctx]
        {
        ctx->ProcessQueue();
        delete ctx;
        });
    thread.detach();
    m_ctx = ctx;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BeTimekeeper::~BeTimekeeper()
    {
    m_ctx->m_stopping = true;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BeTimekeeper& BeTimekeeper::Get()
    {
    // Non-POD types should not have static instances. However, BeTimekeeper is
    static std::unique_ptr<BeTimekeeper> s_instance(new BeTimekeeper());
    return *s_instance;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void BeTimekeeper::Context::ProcessQueue()
    {
    while (!m_stopping)
        {
        std::unique_lock<std::mutex> lk(m_mutex);

        auto currentTime = GetCurrentTimeAsUnixMillis();

        PopTimedCallbacks(currentTime);

        if (m_queue.empty())
            {
            m_hasWorkCV.wait(lk, [this]
                {
                if (m_stopping)
                    return true;
                return !m_queue.empty();
                });
            continue;
            }

        auto sleepFor = m_queue.top()->executionTime - currentTime;
        m_hasWorkCV.wait_for(lk, std::chrono::milliseconds(sleepFor), [this, &currentTime, &sleepFor]
            {
            if (m_stopping)
                return true;

            auto newCurrentTime = GetCurrentTimeAsUnixMillis();
            auto timeElapsed = newCurrentTime - currentTime;
            currentTime = newCurrentTime;

            auto timeLeftToSleep = sleepFor - timeElapsed;

            auto newSleepFor = m_queue.top()->executionTime - currentTime;

            return newSleepFor < timeLeftToSleep;
            });
        }
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void BeTimekeeper::Context::PopTimedCallbacks(int64_t currentTime)
    {
    while (!m_queue.empty())
        {
        if (m_queue.top()->executionTime > currentTime)
            return;

        m_queue.top()->promise.setValue();
        m_queue.pop();
        }
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
folly::Future<folly::Unit> BeTimekeeper::after(folly::Duration duration)
    {
    auto container = std::make_shared<Callback>();
    container->executionTime = GetCurrentTimeAsUnixMillis() + duration.count();
    auto future = container->promise.getFuture().via(&folly::InlineExecutor::instance());

    std::unique_lock<std::mutex> lk(m_ctx->m_mutex);
    m_ctx->m_queue.push(container);
    lk.unlock();

    m_ctx->m_hasWorkCV.notify_one();

    return future;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
* This is similar implementation to original, just using BeTimekeeper to avoid dependcies
+---------------+---------------+---------------+---------------+---------------+------*/
folly::Future<folly::Unit> folly::futures::sleep(folly::Duration dur, folly::Timekeeper* tk) {
    std::shared_ptr<folly::Timekeeper> tks;
    if (nullptr == tk)
        tk = &BeTimekeeper::Get();
    return tk->after(dur);
    }