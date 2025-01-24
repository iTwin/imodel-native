/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once

#if !defined (BENTLEY_CONFIG_NO_THREAD_SUPPORT)

#include <vector>
#include <queue>

#include <Bentley/Bentley.h>

PUSH_CLANG_IGNORE(unknown-warning-option)
PUSH_CLANG_IGNORE(implicit-const-int-float-conversion)
#include <folly/portability/Config.h>
#include <folly/Executor.h>
#include <folly/futures/Future.h>
POP_CLANG_IGNORE
POP_CLANG_IGNORE

namespace BeFolly
{

//=======================================================================================
// @bsiclass
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE ThreadPool : folly::Executor
{
  private:
    struct Worker : BentleyApi::RefCountedBase
    {
        int m_id;
        bool m_stopped = false;
        ThreadPool& m_pool;
        std::shared_ptr<BentleyApi::BeConditionVariable> m_cv;

        Worker(ThreadPool& pool, int id) noexcept : m_pool(pool), m_cv(m_pool.m_cv), m_id(id) {}
        Worker(Worker const&) = delete;
        void Work();
        void Start();
        void SetName();
        THREAD_MAIN_DECL Main(void* arg);
    };
    typedef RefCountedPtr<Worker> WorkerPtr;

    friend struct Worker;
    Utf8String m_name;
    std::vector<WorkerPtr> m_workers;
    std::queue<folly::Func> m_tasks;
    std::shared_ptr<BentleyApi::BeConditionVariable> m_cv;

    Utf8CP GetName() const { return m_name.c_str(); }
    bool HasWork() const { return !m_tasks.empty(); }
    virtual void addWithPriority(folly::Func func, int8_t) override { add(std::move(func)); }
    BE_FOLLY_EXPORT virtual void add(folly::Func func) override;

  protected:
    BE_FOLLY_EXPORT ThreadPool(int nThreads, Utf8CP name);
    BE_FOLLY_EXPORT ~ThreadPool();

  public:
    BE_FOLLY_EXPORT static ThreadPool& GetIoPool();
    BE_FOLLY_EXPORT static ThreadPool& GetCpuPool();
    BE_FOLLY_EXPORT void WaitForIdle();
    size_t size() const { return m_workers.size(); }
};

//=======================================================================================
// A queue that limits the number of parallel execution of tasks.
// @bsiclass
//=======================================================================================
template <typename Result_T>
struct LimitingTaskQueue
{
  private:
    typedef std::function<folly::Future<Result_T>()> TaskType;
    typedef std::deque<std::pair<TaskType, std::shared_ptr<folly::Promise<Result_T>>>> DequeType;
    BeMutex m_mutex;
    DequeType m_tasks;
    BeAtomic<uint32_t> m_limit;
    BeAtomic<uint32_t> m_runningTasks;
    folly::Executor& m_executor;

  private:
    void CheckQueue()
    {
        if (m_runningTasks >= m_limit)
            return;

        BeMutexHolder _lock(m_mutex);

        auto tasksToRun = m_limit - m_runningTasks;
        while (tasksToRun-- && !m_tasks.empty())
        {
            auto task = m_tasks.front();
            m_tasks.pop_front();
            RunTaskAsync(task);
        }
    }

    void RunTaskAsync(typename DequeType::value_type task)
    {
        ++m_runningTasks;

        folly::via(&m_executor, [=] {
            return task.first(); // execute task
        })
            .then([=, this](Result_T result) {
                --m_runningTasks;
                task.second->setValue(result); // fulfill the promise
                CheckQueue();
            });
    }

  public:
    LimitingTaskQueue(folly::Executor& executor, uint32_t limit) : m_executor(executor), m_limit(limit), m_runningTasks(0) { BeAssert(limit > 0); }

    //! Schedule a task for execution.
    template <class Func>
    folly::Future<Result_T> Push(Func&& func)
    {
        auto follyPromise = std::make_shared<folly::Promise<Result_T>>();

        BeMutexHolder _lock(m_mutex);
        m_tasks.push_back(std::make_pair(func, follyPromise));

        CheckQueue();
        return follyPromise->getFuture();
    }
};

} // namespace BeFolly

#endif //BENTLEY_CONFIG_NO_THREAD_SUPPORT
