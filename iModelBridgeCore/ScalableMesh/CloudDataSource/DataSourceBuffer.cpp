#include "stdafx.h"
#include <mutex>

#include "DataSourceBuffer.h"
#include "include\DataSourceBuffer.h"


bool DataSourceBuffer::isSegmented(void)
    {
    return m_isSegmented;
    }

void DataSourceBuffer::setSegmentSize(BufferSize size)
{
    m_segmentSize = size;
}

DataSourceBuffer::BufferSize DataSourceBuffer::getSegmentSize(void)
{
    return m_segmentSize;
}


DataSourceBuffer::BufferSize DataSourceBuffer::getLastSegmentSize(void)
{
    BufferSize lastSegmentSize;

    if (getSegmentSize() > 0)
    {
                                                            // Get the full buffer size remainder when divided by the number of segments. This is the size of the final segment.
        if ((lastSegmentSize = getSize() % getSegmentSize()) > 0)
        {
                                                            // Remainder is non zero, so this is the size of the final segment
            return lastSegmentSize;
        }
    }
                                                            // There is no remainder in the divison, so the last segment is full size
    return getSegmentSize();
}

void DataSourceBuffer::setCurrentSegmentIndex(SegmentIndex index)
{
    m_currentSegmentIndex = index;
}


DataSourceBuffer::SegmentIndex DataSourceBuffer::getCurrentSegmentIndex(void)
{
    return m_currentSegmentIndex;
}

void DataSourceBuffer::setSegmented(const bool & value)
    {
    m_isSegmented = value;
    }

DataSourceBuffer::BufferData * DataSourceBuffer::getSegment(SegmentIndex index)
{

    BufferSize offset = (index * getSegmentSize());

    if (offset < getSize())
    {
                                                            // If external buffer is defined (e.g. for reads) then use it
        if (getExternalBuffer())
        {
            return (getExternalBuffer() + offset);
        }
        else
        {
                                                            // Otherwise use internally managed buffer
            return (&(buffer[0]) + offset);
        }
    }

    return nullptr;
}

DataSourceBuffer::BufferVectorData * DataSourceBuffer::getBuffer()
    {

    assert(!isSegmented()); // Should use segemented buffers instead

    return &buffer;
    }

void DataSourceBuffer::setExternalBuffer(BufferData *extBuffer)
{
    externalBuffer = extBuffer;
}

void DataSourceBuffer::setExternalVector(std::vector<BufferData>* extVector)
    {
    externalVector = extVector;
    }

DataSourceBuffer::BufferData *DataSourceBuffer::getExternalBuffer(void)
{
    return externalBuffer;
}

std::vector<DataSourceBuffer::BufferData>* DataSourceBuffer::getExternalVector(void)
    {
    return externalVector;
    }

void DataSourceBuffer::setExternalBufferSize(BufferSize size)
{
    externalBufferSize = size;
}

DataSourceBuffer::BufferSize DataSourceBuffer::getExternalBufferSize(void)
{
    return externalBufferSize;
}

void DataSourceBuffer::updateReadSize(DataSourceBuffer::BufferSize readSize)
    {
    m_totalReadSize += readSize;
    }

DataSourceBuffer::BufferSize DataSourceBuffer::getReadSize(void)
    {
    return m_totalReadSize;
    }

DataSourceBuffer::DataSourceBuffer(void)
{
    clear();

    initializeSegments(0);
}

DataSourceBuffer::DataSourceBuffer(BufferSize size, BufferData * extBuffer)
{
    initializeSegments(0);

    if (extBuffer)
    {
        setExternalBuffer(extBuffer);
        setExternalBufferSize(size);
    }
    else
        {
        setExternalBuffer(nullptr);
        setExternalBufferSize(0);
        }
}

DataSourceBuffer::DataSourceBuffer(std::vector<BufferData>& extBuffer)
    {
    clear();
    initializeSegments(0);
    setExternalVector(&extBuffer);
    }

DataSourceBuffer::~DataSourceBuffer()
    {
    std::unique_lock<std::mutex> lock(mutex);
    clear();
    }

ActivitySemaphore &DataSourceBuffer::getActivitySemaphore(void)
{
    return activitySemaphore;
}


void DataSourceBuffer::setLocator(const DataSourceLocator & newLocator)
{
    locator = newLocator;
}

DataSourceLocator & DataSourceBuffer::getLocator(void)
{
    return locator;
}

DataSourceBuffer::SegmentIndex DataSourceBuffer::getNumSegments(void)
{
    if (getSegmentSize() == 0)
        return 1;

    SegmentIndex numSegments = static_cast<SegmentIndex>(getSize() / getSegmentSize());

    if(getSize() % getSegmentSize() > 0)
        numSegments++;

    return numSegments;
}

DataSourceBuffer::BufferSize DataSourceBuffer::getSize(void)
{
    if (getExternalBuffer())
    {
        return getExternalBufferSize();
    }

    return buffer.size();
}

DataSourceStatus DataSourceBuffer::clear(void)
{
    buffer.clear();

    setExternalBuffer(nullptr);
    setExternalBufferSize(0);
    setExternalVector(nullptr);
    setSegmentSize(0);

    return DataSourceStatus();
}

DataSourceStatus DataSourceBuffer::append(const BufferData *source, BufferSize size)
{
    if (getExternalBuffer())
    {
        return DataSourceStatus(DataSourceStatus::Status_Error_Not_Supported);
    }
    try
        {
        if (getExternalVector())
            {
            getExternalVector()->insert(getExternalVector()->end(), source, source + size);
            return DataSourceStatus();
            }

        buffer.insert(buffer.end(), source, source + size);
        }
    catch (...)
        {
        return DataSourceStatus(DataSourceStatus::Status_Error_Memory_Allocation);
        }

    return DataSourceStatus();
}

DataSourceStatus DataSourceBuffer::expand(BufferSize size)
{
    if (getExternalBuffer())
    {
        return DataSourceStatus(DataSourceStatus::Status_Error_Not_Supported);
    }

    try
    {
        buffer.resize(size);
    }
    catch (...)
    {
        return DataSourceStatus(DataSourceStatus::Status_Error_Memory_Allocation);
    }

    return DataSourceStatus();
}

void DataSourceBuffer::initializeSegments(void)
{
                                                            // Initialize total number of segments
    getActivitySemaphore().setCounter(getNumSegments());
                                                            // Initialize transfer status to OK
    setTransferStatus(DataSourceStatus());
}


void DataSourceBuffer::initializeSegments(BufferSize segmentSize)
{
    setSegmentSize(segmentSize);

    setCurrentSegmentIndex(0);
}


DataSourceBuffer::SegmentIndex DataSourceBuffer::getAndAdvanceCurrentSegment(BufferData ** dest, BufferSize *size)
{
    if (size == nullptr || dest == nullptr)
        return 0;

    BufferData *    segment;
    SegmentIndex    segmentIndex;
                                                            // Mutex obtaining segments
    std::unique_lock<std::mutex>    bufferLock(segmentMutex);

                                                            // Get current segment's address
    segment = getSegment(getCurrentSegmentIndex());

    if(segment)
    {
                                                            // If it exists, return the address in dest
        *dest = segment;
                                                            // If not the last segment
        if (getCurrentSegmentIndex() < getNumSegments() - 1)
        {
                                                            // Return the full segment size
            *size = getSegmentSize();
        }
        else
        {
                                                            // If last segment, return the last segment's size
            *size = getLastSegmentSize();
        }

        segmentIndex = getCurrentSegmentIndex();
                                                            // Advance to next segment
        setCurrentSegmentIndex(segmentIndex + 1);

        return segmentIndex;
    }

                                                            // Segment does not exist, so return null and zero
    *dest = nullptr;
    *size = 0;

    return 0;
}

bool DataSourceBuffer::signalSegmentProcessed(void)
{
                                                            // Task has completed processing segment, so let the activity semaphore know
    return getActivitySemaphore().decrement();
}


void DataSourceBuffer::signalCancelled(void)
{
                                                            // Release all blocked threads waiting for data
    getActivitySemaphore().releaseAll();
}


DataSourceStatus DataSourceBuffer::getDataSourceStatus(TimeoutStatus status)
{
    switch (status)
    {
    case ActivitySemaphore::Status_NoTimeout:
        return DataSourceStatus(DataSourceStatus::Status_OK);


    case ActivitySemaphore::Status_Timeout:
        return DataSourceStatus(DataSourceStatus::Status_Error_Timeout);

    default:;
    }

    return DataSourceStatus(DataSourceStatus::Status_Error);
}

void DataSourceBuffer::setTransferStatus(DataSourceStatus status)
{
    transferStatus = status;
}

DataSourceStatus DataSourceBuffer::getTransferStatus(void)
{
    return transferStatus;
}

DataSourceStatus DataSourceBuffer::waitForSegments(Timeout timeoutMilliseconds, int numRetries)
{
    std::unique_lock<std::mutex> lock(mutex);
    TimeoutStatus   timeoutStatus = TimeoutStatus::Status_Error;
                                                            // Retry after timeout exceeded until activity ceased or numRetries reached
    int retry = 0;
    do
        {
        // Wait for all segments to be processed
        timeoutStatus = getActivitySemaphore().waitFor(timeoutMilliseconds);
        } 
    while (getDataSourceStatus(timeoutStatus).isFailed() && ++retry < numRetries);

    // TODO: Is transfer status being set in the transfer scheduler?      // If the transfer failed, return an error
    if (getTransferStatus().isFailed())
        return getTransferStatus();
                                                            // If the wait timed out, return a timeout
    if (getDataSourceStatus(timeoutStatus).isFailed())
        return getDataSourceStatus(timeoutStatus);
                                                            // Return OK
    return DataSourceStatus();
}
