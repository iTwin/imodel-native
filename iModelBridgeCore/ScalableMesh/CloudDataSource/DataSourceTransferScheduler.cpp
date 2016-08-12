#include "stdafx.h"

#include <algorithm>

#include "DataSourceTransferScheduler.h"
#include "DataSource.h"
#include "DataSourceAccount.h"
#include "DataSourceBuffer.h"
#include "include\DataSourceTransferScheduler.h"


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

    DataSourceBuffer::BufferData    *   segmentBuffer    = nullptr;
    DataSourceBuffer::BufferSize        segmentSize      = 0;
    DataSourceBuffer::SegmentIndex      segmentIndex;
                                                            // For each scheduled DataBuffer to transfer
    for (auto dataBuffer : dataSourceBuffers)
    {
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
                std::unique_lock<DataSourceBuffersMutex>    dataSourceBuffersLock(dataSourceBuffersMutex);
                                                            // Wait while queue of DataSourceBuffers is empty or no longer need processing
                while (((buffer = getNextSegmentJob(&segmentBuffer, &segmentSize, &segmentIndex)) == nullptr) && getShutDownFlag() == false)
                {
                                                            // Wait for buffer data. Note: wait() releases the mutex and blocks
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
                                                            // Remove bufer from buffer queue
                removeBuffer(*buffer);

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

                                                            // Run max number of transfer threads
    for (unsigned int t = 0; t < numTasks; t++)
    {
        transferThreads.push_back(std::thread(transferDataSourceBufferSegments));
    }

                                                            // Return OK
    return DataSourceStatus();
}


