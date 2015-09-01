//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: Tests/IppGraLibs/ImageAllocatorTester.cpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
#include "../imagepptestpch.h"
#include <ImagePP/all/h/HFCThread.h>
#include <Logging\bentleylogging.h>





struct AllocationJob : public HFCEvent, public RefCountedBase
{
private:
    ImageAllocatorPool& m_pool;

public:
    static RefCountedPtr<AllocationJob> Create(ImageAllocatorPool& pool) { return new AllocationJob(pool); }

    AllocationJob(ImageAllocatorPool& pool) : HFCEvent(true/*pi_ManualReset*/, false/*signaled*/), m_pool(pool) {}

    bool Execute()
        {
        ImageAllocatorRefPtr allocatorRef = m_pool.GetAllocatorRef();
        IImageAllocatorR allocator = allocatorRef->GetAllocator();

        std::vector<Byte*> myVec;

        for (size_t i = 0; i < 50; ++i)
            myVec.push_back(allocator._AllocMemory(64 * 64 + i));

        for (uint64_t i = 0; i < 25; ++i)
            {
            allocator._FreeMemory(*myVec.begin());
            myVec.erase(myVec.begin());
            }

        for (size_t i = 0; i < 50; ++i)
            myVec.push_back(allocator._AllocMemory(64 * 64 + i));

        for (uint64_t i = 0; i < 75; ++i)
            {
            allocator._FreeMemory(*myVec.begin());
            myVec.erase(myVec.begin());
            }
        return true;
        }

};

typedef std::list<RefCountedPtr<AllocationJob>> Jobs;
class ImageAllocatorTester : public testing::Test
{

};


struct AllocatorThread : public HFCThread
{
private:
    HFCSemaphore& m_processEvent;
    Jobs&    m_jobs;
    HFCExclusiveKey& m_jobsKey;

public:
    AllocatorThread(Jobs& jobs, HFCExclusiveKey& jobsKey, HFCSemaphore& processEvent) : m_jobs(jobs), m_jobsKey(jobsKey), m_processEvent(processEvent) {}
    virtual AllocatorThread::~AllocatorThread()
        {
        StopThread();
        WaitUntilSignaled();
        }

    void Go()
        {
        HFCSynchroContainer Synchros;
        Synchros.AddSynchro(&m_StopEvent);
        Synchros.AddSynchro(&m_processEvent);
        while (0 != WaitForMultipleObjects(Synchros, false))
            {
            HFCMonitor monitor(m_jobsKey);
            RefCountedPtr<AllocationJob> job = *m_jobs.begin();
            m_jobs.pop_front();
            monitor.ReleaseKey();

            job->Execute();
            job->Signal();
            }
        }
};

/*---------------------------------------------------------------------------------**//**
* Coverage , no validation performed
* @bsimethod                                    Stephane.Poulin                 10/2014
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(ImageAllocatorTester, ImageAllocatorTest)
    {
    std::vector<uint32_t> blocksPerAllocator = { 0, 1, 2, 3, 4, 255 };
    std::vector<uint32_t> numberOfAllocators = { 0, 1, 2, 3, 4, 255 };
    std::vector<uint32_t> aligments = { 1, sizeof(Byte), sizeof(int32_t), sizeof(int64_t), 64, 256 };

    Jobs jobs;
    std::vector<AllocatorThread*> threads;
    HFCSemaphore                  processEvent;
    HFCExclusiveKey               jobsKey;

    const uint64_t numberOfThreads = 4;
    const uint64_t numberOfJobs = 8;

    // Create threads, then start them
    for (uint64_t i = 0; i < numberOfThreads; ++i)
        threads.push_back(new AllocatorThread(jobs, jobsKey, processEvent));

    for (auto thread : threads)
        {
        if (!thread->StartThread())
            thread = 0;
        }


    for (auto numBlocks : blocksPerAllocator)
        {
        for (auto numAlloc : numberOfAllocators)
            {
            for (auto aligment : aligments)
                {
                ImageAllocatorPool pool(numAlloc, aligment, numBlocks);

                // Create allocation jobs.
                for (uint64_t i = 0; i < numberOfJobs; ++i)
                    jobs.push_back(AllocationJob::Create(pool));

                // Make a local copy. Threads remove items in jobs.
                Jobs jobCopy(jobs);

                // Once signaled, threads will try to remove items from jobs
                HFCMonitor monitor(jobsKey);

                // Signal jobs. 
                for (auto job : jobs)
                    processEvent.Signal();

                // Release jobs to allow threads to get them from the list run them.
                monitor.ReleaseKey();

                // Wait for all job get completed.
                for (auto job : jobCopy)
                    job->WaitUntilSignaled();

                jobs.clear();
                }
            }
        }

    for (auto thread : threads)
        delete thread;
    }

