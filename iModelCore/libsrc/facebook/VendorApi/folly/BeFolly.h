/*--------------------------------------------------------------------------------------+
|
|     $Source: VendorApi/folly/BeFolly.h $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

#if !defined (BENTLEY_CONFIG_NO_THREAD_SUPPORT)

#include <vector>
#include <queue>

#include <folly/portability/Config.h>
#include <folly/Executor.h>

namespace BeFolly {

//=======================================================================================
// @bsiclass                                                    Keith.Bentley   06/16
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE ThreadPool : folly::Executor
{
private:
    struct Worker : BentleyApi::RefCountedBase
    {
        int m_id;
        ThreadPool& m_pool;

        Worker(ThreadPool& pool, int id) noexcept : m_pool(pool), m_id(id) {}
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
    BentleyApi::BeConditionVariable m_cv;
    bool m_stop = false;

    Utf8CP GetName() const {return m_name.c_str();}
    bool HasWork() const {return !m_tasks.empty();}
    bool IsStopped() const {return m_stop;}
    virtual void addWithPriority(folly::Func func, int8_t) override {add(std::move(func));}
    BE_FOLLY_EXPORT virtual void add(folly::Func func) override;

protected:
    BE_FOLLY_EXPORT ThreadPool(int nThreads, Utf8CP name);
    BE_FOLLY_EXPORT ~ThreadPool();

public:
    BE_FOLLY_EXPORT static ThreadPool& GetIoPool();
    BE_FOLLY_EXPORT static ThreadPool& GetCpuPool();
    BE_FOLLY_EXPORT void WaitForIdle();
};

} // namespace BeFolly

#endif //BENTLEY_CONFIG_NO_THREAD_SUPPORT
