#include "stdafx.h"
#include <mutex>

#include "DataSourceBuffer.h"


void DataSourceBuffer::setSegmentSize(BufferSize size)
{
    segmentSize = size;
}

DataSourceBuffer::BufferSize DataSourceBuffer::getSegmentSize(void)
{
    return segmentSize;
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
    currentSegmentIndex = index;
}


DataSourceBuffer::SegmentIndex DataSourceBuffer::getCurrentSegmentIndex(void)
{
    return currentSegmentIndex;
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

void DataSourceBuffer::setExternalBuffer(BufferData *extBuffer)
{
    externalBuffer = extBuffer;
}

DataSourceBuffer::BufferData *DataSourceBuffer::getExternalBuffer(void)
{
    return externalBuffer;
}

void DataSourceBuffer::setExternalBufferSize(BufferSize size)
{
    externalBufferSize = size;
}

DataSourceBuffer::BufferSize DataSourceBuffer::getExternalBufferSize(void)
{
    return externalBufferSize;
}

DataSourceBuffer::DataSourceBuffer(void)
{
    clear();

    initializeSegments(0);
}

DataSourceBuffer::DataSourceBuffer(BufferSize size, BufferData * extBuffer)
{
    initializeSegments(0);

    if (externalBuffer)
    {
        setExternalBuffer(extBuffer);
        setExternalBufferSize(size);
    }
}

ActivitySemaphore &DataSourceBuffer::getActivitySemaphore(void)
{
    return activitySemaphore;
}

void DataSourceBuffer::initializeSegments(BufferSize segmentSize)
{
    setSegmentSize(segmentSize);

    setCurrentSegmentIndex(0);
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
        return 0;

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

    setSegmentSize(0);

    return DataSourceStatus();
}

DataSourceStatus DataSourceBuffer::append(BufferData *source, BufferSize size)
{
    if (getExternalBuffer())
    {
        return DataSourceStatus(DataSourceStatus::Status_Error_Not_Supported);
    }

    std::vector<BufferData>    sourceBuffer(source, &source[size]);

    buffer.insert(buffer.end(), sourceBuffer.begin(), sourceBuffer.end());

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
    getActivitySemaphore().setCounter(getNumSegments());
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

DataSourceBuffer::TimeoutStatus DataSourceBuffer::waitForSegments(Timeout timeoutMilliseconds)
{
                                                            // Wait for all segments to be processed
    return getActivitySemaphore().waitFor(timeoutMilliseconds);
}
