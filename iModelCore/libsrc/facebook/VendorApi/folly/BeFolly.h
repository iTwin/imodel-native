/*--------------------------------------------------------------------------------------+
|
|     $Source: VendorApi/folly/BeFolly.h $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

#include <vector>
#include <queue>

#include <folly/portability/config.h>
#include <folly/Executor.h>

namespace BeFolly {

//=======================================================================================
// @bsiclass                                                    Keith.Bentley   06/16
//=======================================================================================
struct IOThreadPool : folly::Executor
{
private:
    struct Worker : BentleyApi::RefCountedBase
    {
        IOThreadPool& m_pool;

        Worker(IOThreadPool& pool) noexcept : m_pool(pool) {}
        Worker(Worker const&) = delete;
        void Work();
        void Start();
        THREAD_MAIN_DECL Main(void* arg);
    };
    typedef RefCountedPtr<Worker> WorkerPtr;

    friend struct Worker;
    std::vector<WorkerPtr> m_workers;
    std::queue<folly::Func> m_tasks;
    BentleyApi::BeConditionVariable m_cv;
    bool m_stop = false;

    bool HasWork() const {return !m_tasks.empty();}
    bool IsStopped() const {return m_stop;}
    virtual void add(folly::Func func) override;

protected:
    IOThreadPool(int);
    ~IOThreadPool();

public:
    BE_FOLLY_EXPORT static IOThreadPool& GetPool();
    BE_FOLLY_EXPORT void WaitForIdle();
};

} // namespace BeFolly

