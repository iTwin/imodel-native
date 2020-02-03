/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "stdafx.h"

#include "DataSourceTransferScheduler.h"
#include "DataSource.h"
#include "DataSourceAccount.h"
#include "DataSourceBuffer.h"
#include "include/DataSourceTransferScheduler.h"
#include <mutex>
extern std::mutex s_consoleMutex;

#ifndef SM_STREAMING_PERF
#include <iostream>
#include <chrono>
#endif

#include <algorithm>
#include <assert.h>
//#include <iostream>
//#include <mutex>

//std::mutex s_consoleMutex;


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
    shutDown();
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
    transferThreads.clear();
    }


//static std::atomic<int> s_nIdealQueueSize = 0;

DataSourceStatus DataSourceTransferScheduler::addBuffer(DataSourceBuffer & buffer)
    {
    //++s_nIdealQueueSize;
                                                            // Lock the DataSourceBuffer queue
    std::unique_lock<DataSourceBuffersMutex>    dataSourceBuffersLock(dataSourceBuffersMutex);

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


DataSourceStatus DataSourceTransferScheduler::removeBuffer(DataSourceBuffer &buffer)
{
                                                            // Lock the DataSourceBuffer queue
    std::unique_lock<DataSourceBuffersMutex>    dataSourceBuffersLock(dataSourceBuffersMutex);

                                                            // Find buffer
    for (auto it = dataSourceBuffers.begin(); it != dataSourceBuffers.end(); it++)
    {
                                                            // If found, delete the entry
        if (*it == &buffer)
        {
            dataSourceBuffers.erase(it);
            //--s_nIdealQueueSize;
                                                            // Return OK
            return DataSourceStatus();
        }
    }
                                                            // Return not found
    return DataSourceStatus(DataSourceStatus::Status_Error_Not_Found);
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
    for (auto dataBuffer : dataSourceBuffers)
    {
        if (!dataBuffer->isSegmented())
            {
            if (!dataBuffer->getIsProcessing())
                {
                dataBuffer->setIsProcessing(true);
                return dataBuffer;
                }
            else
                {
                return nullptr;
                }
            }
                                                            // Try each buffer until a segment is found that needs processing
                                                            // Usually this will be in one of the first, but in some cases the first may be pending completion
        segmentIndex = dataBuffer->getAndAdvanceCurrentSegment(&segmentBuffer, &segmentSize);
                                                            // If a job was specified, return it
        if(segmentSize > 0)
        {
            *buffer         = segmentBuffer;
            *bufferSize     = segmentSize;
            *index          = segmentIndex;

            return dataBuffer;
        }
    }
                                                            // Return no jobs available
    return nullptr;
    }


DataSourceStatus DataSourceTransferScheduler::initializeTransferTasks(unsigned int numTasks)
    {
                                                            // If not configured to use multi-threaded transfers
    if (numTasks == 0 || !transferThreads.empty())
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

#ifdef SM_STREAMING_PERF
            static std::atomic<int> s_nWorkThreads = 0;
            static std::atomic<int> s_nTotalTransfers = 0;
            static std::atomic<uint64_t> s_nTotalSize = 0;
            static std::atomic<long long> s_nDownloadTime = 0;
#endif

                                                            // Scope the mutex while the next job is obtained
            {
                std::unique_lock<DataSourceBuffersMutex>    dataSourceBuffersLock(dataSourceBuffersMutex);
                                                            // Wait while queue of DataSourceBuffers is empty or no longer need processing
            while (((buffer = getNextSegmentJob(&segmentBuffer, &segmentSize, &segmentIndex)) == nullptr) && getShutDownFlag() == false)
                {
#ifdef SM_STREAMING_PERF
                if (s_nWorkThreads > 0) s_nWorkThreads -= 1;
#endif
                //    {
                //    std::lock_guard<std::mutex> clk(s_consoleMutex);
                //    //std::cout << "[" << std::this_thread::get_id() << "] Waiting for work" << std::endl;
                //    std::cout << "DataSource number of working threads: " << s_nWorkThreads << std::endl;
                //    }
                                                            // Wait for buffer data. Note: wait() releases the mutex and blocks
                dataSourceBufferReady.wait(dataSourceBuffersLock);
#ifdef SM_STREAMING_PERF
                s_nWorkThreads += 1;
#endif
                //    //{
                //    //std::lock_guard<std::mutex> clk(s_consoleMutex);
                //    //std::cout << "[" << std::this_thread::get_id() << "] Going to perform work" << std::endl;
                //    //}
                }
            }
#ifdef SM_STREAMING_PERF
            static std::chrono::steady_clock::time_point start_time = std::chrono::steady_clock::now();
            std::chrono::steady_clock::time_point end_time = std::chrono::steady_clock::now();

#endif

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
            DataSourceURL segmentName(url);
            
            if (buffer->isSegmented())
                {
                segmentName += DataSourceURL(L"-" + std::to_wstring(segmentIndex));
                }
                                                            // Get the Account
            if ((account = locator.getAccount()) == nullptr)
                {
                //assert(false);
                buffer->setTransferStatus(DataSourceStatus::Status_Error);

                //return DataSourceStatus(DataSourceStatus::Status_Error);
                }
            else
                {

                // Download or Upload blob based on mode
                if (locator.getMode() == DataSourceMode_Read)
                    {
#ifdef SM_STREAMING_PERF
                    std::chrono::steady_clock::time_point start_time = std::chrono::steady_clock::now();
#endif
                    if (buffer->isSegmented())
                        {
                        // Attempt to download a single segment
                        if ((status = account->downloadBlobSync(segmentName, segmentBuffer, readSize, segmentSize, locator.getSessionName())).isFailed())
                            {
                            buffer->setTransferStatus(DataSourceStatus::Status_Error_Failed_To_Download);
                            }
                        else
                            {

#ifdef SM_STREAMING_PERF
                            std::chrono::steady_clock::time_point end_time = std::chrono::steady_clock::now();
                            s_nDownloadTime += std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time).count();
                            s_nTotalSize += readSize;
                            if (true/*std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time).count() > 500*/)
                                {
                                std::lock_guard<std::mutex> clk(s_consoleMutex);
                                //std::cout << s_nWorkThreads << "    " << dataSourceBuffers.size() << "    " << s_nTotalTransfers << std::endl;
#ifndef NDEBUG
                                FILE* pOutputFileStream = fopen("c:\\tmp\\scalablemesh\\transferscheduler_performance_debug.txt", "a+");
#else
                                FILE* pOutputFileStream = fopen("c:\\tmp\\scalablemesh\\transferscheduler_performance_release.txt", "a+");
#endif
                                char TempBuffer[500];
                                int  NbChars;

                                NbChars = sprintf(TempBuffer, "Segment name: %ls   Number of threads doing work: %i   Queue size: %lli   Ideal queue size: %i   Total work done: %i   size: %lli   total size: %i   download speed: %fMB/s\n", segmentName.c_str(), (int)s_nWorkThreads, dataSourceBuffers.size(), (int)s_nIdealQueueSize, (int)s_nTotalTransfers, readSize, (int)s_nTotalSize, (double)((double)(readSize) / (double)s_nDownloadTime) / 8000);

                                size_t NbWrittenChars = fwrite(TempBuffer, 1, NbChars, pOutputFileStream);
                                assert(NbWrittenChars == NbChars);
                                fclose(pOutputFileStream);

                                // restart timer
                                start_time = std::chrono::steady_clock::now();
                                }
#endif

                            buffer->updateReadSize(readSize);
                            }
                        }
                    else if ((status = account->downloadBlobSync(segmentName, buffer, locator.getSessionName())).isFailed())
                        {
                        buffer->setTransferStatus(DataSourceStatus::Status_Error_Failed_To_Download);
                        }
                    }
                else
                    if (locator.getMode() == DataSourceMode_Write_Segmented || locator.getMode() == DataSourceMode_Write)
                        {
                        // Attempt to upload a single segment
                        std::wstring filename = locator.getSubPath();
                        std::size_t found = filename.find_last_of(L"/\\");
                        if (found != std::wstring::npos)
                            {
                            filename = filename.substr(found);
                            }
                        found = filename.find(L"~2F");
                        if (found != std::wstring::npos)
                            {
                            filename = filename.substr(found + 3);
                            }

                        if (buffer->isSegmented())
                            {
                            filename += L"-" + std::to_wstring(segmentIndex);
                            if ((status = account->uploadBlobSync(segmentName, filename, segmentBuffer, segmentSize)).isFailed())
                                {
                                if ((status = account->uploadBlobSync(segmentName, segmentBuffer, segmentSize)).isFailed())
                                    {
                                    buffer->setTransferStatus(DataSourceStatus::Status_Error_Failed_To_Upload);
                                    }
                                }
                            }
                        else
                            {
                            if ((status = account->uploadBlobSync(segmentName, buffer)).isFailed())
                                {
                                buffer->setTransferStatus(DataSourceStatus::Status_Error_Failed_To_Upload);
                                }

                            }
                        }
                }

                                                            // Segment has been processed, so signal this. Waiting threads are signalled once all are completed.
            if (buffer->signalSegmentProcessed())
            {
                buffer->setExternalBufferSize(buffer->getReadSize());
                                                            // Remove bufer from buffer queue
                removeBuffer(*buffer);
#ifdef SM_STREAMING_PERF
                s_nTotalTransfers += 1;
#endif

                // TODO: if the transfer scheduler has full ownership of the buffer, then it should be deleted. 
                //       For now, this is not the case and the buffer is deleted by the thread that asked the transfer.
                if(locator.getMode() == DataSourceMode_Write_Segmented || locator.getMode() == DataSourceMode_Write)
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

                                                            // Run max number of transfer threads
    for (unsigned int t = 0; t < numTasks; t++)
        {
        transferThreads.push_back(std::thread(transferDataSourceBufferSegments));
#ifndef NDEBUG
    #ifdef _WIN32
            DWORD ThreadId = ::GetThreadId(static_cast<HANDLE>(transferThreads.back().native_handle()));
            SetThreadName(ThreadId, "DataSourceTransferThread");
    #endif
#endif
        }

                                                            // Return OK
    return DataSourceStatus();
    }


