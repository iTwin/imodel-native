//:>--------------------------------------------------------------------------------------+
//:>
//:>
//:>  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
//:>
//:>+--------------------------------------------------------------------------------------
#include "../imagepptestpch.h"
#include <Logging/bentleylogging.h>

struct AllocationJob : public WorkerPool::Task
{
private:
    ImageAllocatorPool& m_pool;

public:
    static RefCountedPtr<AllocationJob> Create(ImageAllocatorPool& pool) { return new AllocationJob(pool); }

    AllocationJob(ImageAllocatorPool& pool) :m_pool(pool) {}

    virtual void _Run() override
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
        }

};

typedef std::list<RefCountedPtr<AllocationJob>> Jobs;
class ImageAllocatorTester : public testing::Test
{

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

    const uint64_t numberOfThreads = 4;
    const uint64_t numberOfJobs = 8;

    WorkerPool workerPool(numberOfThreads);

    for (auto numBlocks : blocksPerAllocator)
        {
        for (auto numAlloc : numberOfAllocators)
            {
            for (auto aligment : aligments)
                {
                ImageAllocatorPool pool(numAlloc, aligment, numBlocks);

                Jobs jobs;

                // Create allocation jobs.
                for (uint64_t i = 0; i < numberOfJobs; ++i)
                    {
                    jobs.push_back(AllocationJob::Create(pool));
                    workerPool.Enqueue(*jobs.back());
                    }
                
                // Wait for all job get completed.
                for (auto job : jobs)
                    job->Wait();
                }
            }
        }
    }

