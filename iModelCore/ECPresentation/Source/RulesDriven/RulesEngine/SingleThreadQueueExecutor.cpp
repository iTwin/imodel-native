/*--------------------------------------------------------------------------------------+
|
|     $Source: Source/RulesDriven/RulesEngine/SingleThreadQueueExecutor.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ECPresentationPch.h"
#include "SingleThreadQueueExecutor.h"
#include "LoggingHelper.h"
#include <queue>
#include <folly/futures/SharedPromise.h>

USING_NAMESPACE_BENTLEY_ECPRESENTATION

//=======================================================================================
// @bsiclass                                    Grigas.Petraitis                11/2017
//=======================================================================================
template<typename TPtr, typename TQueue>
struct QueuePredicate : IConditionVariablePredicate
    {
    TPtr const& m_terminatePtr;
    TQueue const& m_queue;
    QueuePredicate(TPtr const& terminatePtr, TQueue const& queue)
        : m_terminatePtr(terminatePtr), m_queue(queue)
        {}
    bool _TestCondition(BeConditionVariable &cv) override
        {
        return (nullptr != m_terminatePtr) || !m_queue.empty();
        }
    };

//=======================================================================================
// @bsiclass                                    Grigas.Petraitis                11/2017
//=======================================================================================
struct SingleThreadedQueueExecutor::ThreadRunner
{
private:
    std::shared_ptr<folly::SharedPromise<folly::Unit>> m_terminatePromise;
    intptr_t m_threadId;
    BeConditionVariable m_cv;
    std::queue<folly::Func> m_queue;

private:
    /*-----------------------------------------------------------------------------**//**
    * @bsimethod                                    Grigas.Petraitis            11/2017
    +---------------+---------------+---------------+---------------+-----------+------*/
    folly::Func PopFunc()
        {
        BeMutexHolder lock(m_cv.GetMutex());
        folly::Func func = std::move(m_queue.front());
        m_queue.pop();
        return func;
        }

    /*-----------------------------------------------------------------------------**//**
    * @bsimethod                                    Grigas.Petraitis            04/2019
    +---------------+---------------+---------------+---------------+-----------+------*/
    void CallFunc(folly::Func&& func)
        {
        func();
        }

public:
    /*-----------------------------------------------------------------------------**//**
    * @bsimethod                                    Grigas.Petraitis            11/2017
    +---------------+---------------+---------------+---------------+-----------+------*/
    ThreadRunner() : m_terminatePromise(nullptr) {}
    
    /*-----------------------------------------------------------------------------**//**
    * @bsimethod                                    Grigas.Petraitis            11/2017
    +---------------+---------------+---------------+---------------+-----------+------*/
    folly::Future<folly::Unit> Terminate()
        {
        BeMutexHolder lock(m_cv.GetMutex());
        if (nullptr == m_terminatePromise)
            {
            m_terminatePromise = std::make_shared<folly::SharedPromise<folly::Unit>>();
            m_cv.notify_all();
            LoggingHelper::LogMessage(Log::Threads, Utf8PrintfString("Thread %d requested terminate for SingleThreadedQueueExecutor worker thread %d. Queue size: %" PRIu64, 
                (int)BeThreadUtilities::GetCurrentThreadId(), (int)m_threadId, (uint64_t)m_queue.size()).c_str());
            }
        return m_terminatePromise->getFuture();
        }
    
    /*-----------------------------------------------------------------------------**//**
    * @bsimethod                                    Grigas.Petraitis            11/2017
    +---------------+---------------+---------------+---------------+-----------+------*/
    void QueueOrExecute(folly::Func func)
        {
        BeMutexHolder lock(m_cv.GetMutex());
        BeAssert(nullptr == m_terminatePromise);

        if (BeThreadUtilities::GetCurrentThreadId() == m_threadId)
            {
            lock.unlock();
            CallFunc(std::move(func));
            }
        else
            {
            m_queue.push(std::move(func));
            m_cv.notify_all();
            }
        }
    
    /*-----------------------------------------------------------------------------**//**
    * @bsimethod                                    Grigas.Petraitis            11/2017
    +---------------+---------------+---------------+---------------+-----------+------*/
    void Run()
        {
        m_threadId = BeThreadUtilities::GetCurrentThreadId();
        LoggingHelper::LogMessage(Log::Threads, Utf8PrintfString("Thread %d started", (int)m_threadId).c_str());

        QueuePredicate<std::shared_ptr<folly::SharedPromise<folly::Unit>>, std::queue<folly::Func>> predicate(m_terminatePromise, m_queue);
        while (true)
            {
            m_cv.WaitOnCondition(&predicate, BeConditionVariable::Infinite);
            if (nullptr != m_terminatePromise)
                break;
            
            CallFunc(PopFunc());
            }
        
        BeMutexHolder lock(m_cv.GetMutex());
        if (!m_queue.empty())
            {
            LoggingHelper::LogMessage(Log::Threads, Utf8PrintfString("Thread %d is being terminated with %" PRIu64 " items in queue. Finishing...",
                (int)m_threadId, (uint64_t)m_queue.size()).c_str(), LOG_INFO);
            StopWatch timer(true);
            while (!m_queue.empty())
                {
                lock.unlock();
                CallFunc(PopFunc());
                lock.lock();
                }
            LoggingHelper::LogMessage(Log::Threads, Utf8PrintfString("Took %.2f seconds to finish.", timer.GetCurrentSeconds()).c_str(), LOG_DEBUG);
            }
        LoggingHelper::LogMessage(Log::Threads, Utf8PrintfString("Thread %d finished.", (int)m_threadId).c_str());
        m_terminatePromise->setValue(folly::unit);
        }
};

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                11/2017
+---------------+---------------+---------------+---------------+---------------+------*/
SingleThreadedQueueExecutor::SingleThreadedQueueExecutor(Utf8String name)
    : m_runner(nullptr), m_name(name)
    {}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                11/2017
+---------------+---------------+---------------+---------------+---------------+------*/
SingleThreadedQueueExecutor::~SingleThreadedQueueExecutor() {Terminate().wait();}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                03/2019
+---------------+---------------+---------------+---------------+---------------+------*/
void SingleThreadedQueueExecutor::Init()
    {
    if (nullptr != m_runner)
        return;

    m_runner = new ThreadRunner();
    std::thread([&, name = m_name]()
        {
        if (!name.empty())
            BeThreadUtilities::SetCurrentThreadName(name.c_str());
        m_runner->Run();
        }).detach();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                11/2017
+---------------+---------------+---------------+---------------+---------------+------*/
folly::Future<folly::Unit> SingleThreadedQueueExecutor::Terminate()
    {
    if (nullptr == m_runner)
        return folly::makeFuture();
    return m_runner->Terminate();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                11/2017
+---------------+---------------+---------------+---------------+---------------+------*/
void SingleThreadedQueueExecutor::add(folly::Func func)
    {
    Init();
    m_runner->QueueOrExecute(std::move(func));
    }