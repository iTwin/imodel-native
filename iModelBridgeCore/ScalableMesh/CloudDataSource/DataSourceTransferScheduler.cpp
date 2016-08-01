#include "stdafx.h"

#include <assert.h>
#include "DataSourceTransferScheduler.h"
#include "DataSource.h"
#include "DataSourceAccount.h"
#include "DataSourceBuffer.h"
#include "include\DataSourceTransferScheduler.h"
#ifndef NDEBUG
#define NOMINMAX
#include <windows.h>
const DWORD MS_VC_EXCEPTION = 0x406D1388;

#pragma pack(push,8)
typedef struct tagTHREADNAME_INFO
    {
    DWORD dwType; // Must be 0x1000.
    LPCSTR szName; // Pointer to name (in user addr space).
    DWORD dwThreadID; // Thread ID (-1=caller thread).
    DWORD dwFlags; // Reserved for future use, must be zero.
    } THREADNAME_INFO;
#pragma pack(pop)

void SetThreadName(DWORD dwThreadID, char* threadName)
    {
    THREADNAME_INFO info;
    info.dwType = 0x1000;
    info.szName = threadName;
    info.dwThreadID = dwThreadID;
    info.dwFlags = 0;

    __try
        {
        RaiseException(MS_VC_EXCEPTION, 0, sizeof(info) / sizeof(ULONG_PTR), (ULONG_PTR*)&info);
        }
    __except (EXCEPTION_EXECUTE_HANDLER)
        {}
    }
#endif
#include <algorithm>


void DataSourceTransferScheduler::setMaxTasks(TaskIndex numTasks)
    {
    maxTasks = numTasks;
    }

DataSourceTransferScheduler::TaskIndex DataSourceTransferScheduler::getMaxTasks(void)
    {
    return maxTasks;
    }

DataSourceTransferScheduler::DataSourceTransferScheduler(void)
    {
    // Initially not shutting down
    setShutDownFlag(false);
    }


DataSourceTransferScheduler::~DataSourceTransferScheduler(void)
    {

    }


void DataSourceTransferScheduler::shutDown(void)
    {
    // Set shutdown flag for transfer worker threads
    setShutDownFlag(true);
    // Notify all blocked transfer worker threads (get them running again to shut down cleanly
    dataSourceBufferReady.notify_all();

    auto transferThreadJoin = [](std::thread &t)
        {
        t.join();
        };
    // Wait for all threads to complete
    std::for_each(transferThreads.begin(), transferThreads.end(), transferThreadJoin);
    }


DataSourceStatus DataSourceTransferScheduler::addBuffer(DataSourceBuffer & buffer)
    {
    // Lock the DataSourceBuffer queue
    std::unique_lock<std::mutex>    dataSourceBuffersLock(dataSourceBuffersMutex);

    // Begin processing segments
    buffer.initializeSegments();
    // Add a DataSourceBuffer to the back of the queue
    dataSourceBuffers.push_back(&buffer);


    DataSourceBuffer::SegmentIndex numSegments;
    // Number of tasks to notify if min of number of tasks availabe and number of segments
    numSegments = std::min(buffer.getNumSegments(), getMaxTasks());
    // Notify one task for each new segment.
    // If tasks are currently busy, one will just pick up the job without notification
    for (unsigned int s = 0; s < numSegments; s++)
        {
        dataSourceBufferReady.notify_one();
        }
    // Return OK
    return DataSourceStatus();
    }

void DataSourceTransferScheduler::setShutDownFlag(bool value)
    {
    shutDownFlag = value;
    }


bool DataSourceTransferScheduler::getShutDownFlag(void)
    {
    return shutDownFlag;
    }

DataSourceBuffer *DataSourceTransferScheduler::getNextSegmentJob(DataSourceBuffer::BufferData **buffer, DataSourceBuffer::BufferSize *bufferSize, DataSourceBuffer::SegmentIndex *index)
    {
    if (buffer == nullptr || bufferSize == nullptr || index == nullptr)
        return nullptr;

    DataSourceBuffer::BufferData    *   segmentBuffer = nullptr;
    DataSourceBuffer::BufferSize        segmentSize = 0;
    DataSourceBuffer::SegmentIndex      segmentIndex;
    // For each scheduled DataBuffer to transfer
    for (auto& dataBuffer : dataSourceBuffers)
        {
        if (dataBuffer == nullptr) continue;
        // Try each buffer until a segment is found that needs processing
        // Usually this will be in one of the first, but in some cases the first may be pending completion
        segmentIndex = dataBuffer->getAndAdvanceCurrentSegment(&segmentBuffer, &segmentSize);
        // If a job was specified, return it
        if (segmentSize > 0)
            {
            *buffer = segmentBuffer;
            *bufferSize = segmentSize;
            *index = segmentIndex;

            return dataBuffer;
            }
        }
    // Return no jobs available
    return nullptr;
    }


DataSourceStatus DataSourceTransferScheduler::initializeTransferTasks(unsigned int numTasks)
    {
    // If not configured to use multi-threaded transfers
    if (numTasks == 0)
        {
        // Just return OK
        return DataSourceStatus();
        }
    // Multi-threaded segment transfer function
    auto transferDataSourceBufferSegments = [this](void) -> DataSourceStatus
        {
        while (getShutDownFlag() == false)
            {
            DataSourceBuffer                *   buffer;
            DataSourceBuffer::BufferData    *   segmentBuffer;
            DataSourceBuffer::BufferSize        segmentSize;
            DataSourceBuffer::BufferSize        readSize;
            DataSourceBuffer::SegmentIndex      segmentIndex;
            DataSourceAccount                *  account;
            DataSourceStatus                    status;

            // Scope the mutex while the next job is obtained
            {
            std::unique_lock<std::mutex>    dataSourceBuffersLock(dataSourceBuffersMutex);
            // Wait while queue of DataSourceBuffers is empty or no longer need processing
            while (((buffer = getNextSegmentJob(&segmentBuffer, &segmentSize, &segmentIndex)) == nullptr) && getShutDownFlag() == false)
                {
                dataSourceBufferReady.wait(dataSourceBuffersLock);
                }
            }
            // If shutting down
            if (getShutDownFlag())
                {
                if (buffer)
                    {
                    buffer->signalCancelled();
                    }

                return DataSourceStatus();
                }

            DataSourceLocator &locator = buffer->getLocator();

            DataSourceURL    url;
            locator.getURL(url);
            // Segment name is blob name with index
            DataSourceURL segmentName = url + DataSourceURL(L"-" + std::to_wstring(segmentIndex));

            // Get the Account
            if ((account = locator.getAccount()) == nullptr)
                {
                assert(false);
                return DataSourceStatus(DataSourceStatus::Status_Error);
                }

            // Download or Upload blob based on mode
            if (locator.getMode() == DataSourceMode_Read)
                {
                // Attempt to download a single segment
                if ((status = account->downloadBlobSync(segmentName, segmentBuffer, readSize, segmentSize)).isFailed())
                    {
                    assert(false);
                    return DataSourceStatus(DataSourceStatus::Status_Error_Failed_To_Download);
                    }
                }
            else
                if (locator.getMode() == DataSourceMode_Write || locator.getMode() == DataSourceMode_Write_Segmented)
                    {
                    // Attempt to upload a single segment
                    if ((status = account->uploadBlobSync(segmentName, segmentBuffer, segmentSize)).isFailed())
                        {
                        assert(false);
                        return DataSourceStatus(DataSourceStatus::Status_Error_Failed_To_Upload);
                        }
                    }

            // Segment has been processed, so signal this. Waiting threads are signalled once all are completed.
            if (buffer->signalSegmentProcessed())
                {
                // All buffer's segments have been processed and waiting threads have been notified
                std::unique_lock<std::mutex>    dataSourceBuffersLock(dataSourceBuffersMutex);

                // Remove buffer from buffer queue
                for (auto& dataBuffer : dataSourceBuffers)
                    {
                    if (dataBuffer == buffer) dataBuffer = nullptr;
                    }
                while (!dataSourceBuffers.empty() && dataSourceBuffers.front() == nullptr) dataSourceBuffers.pop_front();
                //if (locator.getMode() == DataSourceMode_Write || locator.getMode() == DataSourceMode_Write_Segmented)
                //    {
                //    // Upload of this buffer is complete, delete it
                //    delete buffer;
                //    }
                }
            }

        return DataSourceStatus();
        };

    // Set up uploader worker tasks
    setMaxTasks(numTasks);

    // Run max number of transfer threads
    for (unsigned int t = 0; t < numTasks; t++)
        {
        transferThreads.push_back(std::thread(transferDataSourceBufferSegments));
#ifndef NDEBUG
        DWORD ThreadId = ::GetThreadId(static_cast<HANDLE>(transferThreads.back().native_handle()));
        SetThreadName(ThreadId, "DataSourceTransferThread");
#endif
        }

    // Return OK
    return DataSourceStatus();
    }

/*
DataSourceStatus DataSourceTransferScheduler::initializeTransferTasks(unsigned int numTasks)
{
    auto transferDataSourceBufferSegments = [this](void) -> DataSourceStatus
    {
        while (getShutDownFlag() == false)
        {
            DataSourceBuffer                *   buffer;
            DataSourceBuffer::BufferData    *   segmentBuffer;
            DataSourceBuffer::BufferSize        segmentSize;
            DataSourceBuffer::BufferSize        readSize;
            DataSourceBuffer::SegmentIndex      segmentIndex;
            DataSourceAccount                *  account;
            DataSourceStatus                    status;

                                                            // Scope the mutex while the next job is obtained
            {
                std::unique_lock<std::mutex>    dataSourceBuffersLock(dataSourceBuffersMutex);
                                                            // Wait while queue of DataSourceBuffers is empty or no longer need processing
                while (((buffer = getNextSegmentJob(&segmentBuffer, &segmentSize, &segmentIndex)) == nullptr) && getShutDownFlag() == false)
                {
                    dataSourceBufferReady.wait(dataSourceBuffersLock);
                }
            }

            if (getShutDownFlag())
            {
                buffer->signalCancelled();
                DataSourceStatus();
            }

            DataSourceLocator &locator = buffer->getLocator();

            DataSourceURL    url;
            locator.getURL(url);
                                                            // Segment name is blob name with index
            DataSourceURL segmentName = url + DataSourceURL(L"-" + std::to_wstring(segmentIndex));

                                                            // Get the Account
            if((account = locator.getAccount()) == nullptr)
                return DataSourceStatus(DataSourceStatus::Status_Error);

                                                            // Download or Upload blob based on mode
            if (locator.getMode() == DataSourceMode_Read)
            {
                                                            // Attempt to download a single segment
                if ((status = account->downloadBlobSync(segmentName, segmentBuffer, readSize, segmentSize)).isFailed())
                    return DataSourceStatus(DataSourceStatus::Status_Error_Failed_To_Upload);
            }
            else
            if(locator.getMode() == DataSourceMode_Write)
            {
                                                            // Attempt to upload a single segment
                if ((status = account->uploadBlobSync(segmentName, segmentBuffer, segmentSize)).isFailed())
                    return DataSourceStatus(DataSourceStatus::Status_Error_Failed_To_Upload);
            }

                                                            // Segment has been processed, so signal this. Waiting threads are signalled once all are completed.
            if (buffer->signalSegmentProcessed())
            {
                                                            // All buffer's segments have been processed and waiting threads have been notified
                std::unique_lock<std::mutex>    dataSourceBuffersLock(dataSourceBuffersMutex);
                                                            // Remove bufer from buffer queue
                dataSourceBuffers.pop_front();

                if(locator.getMode() == DataSourceMode_Write)
                {
                                                            // Upload of this buffer is complete, delete it
                    delete buffer;
                }
            }
        }

        return DataSourceStatus();
    };

                                                            // Set up uploader worker tasks
    setMaxTasks(numTasks);

    for (unsigned int t = 0; t < numTasks; t++)
    {
        transferTasks.run(transferDataSourceBufferSegments);
    }

                                                            // Return OK
    return DataSourceStatus();
}
*/

