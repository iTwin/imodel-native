#include "stdafx.h"
#include "DataSourceBuffered.h"
#include "DataSourceAccount.h"
#include <assert.h>


DataSourceBuffered::DataSourceBuffered(DataSourceAccount *sourceAccount) : Super(sourceAccount)
{
                                                            // Default segment size is zero until initialized
    setSegmentSize(0);
}

void DataSourceBuffered::setBuffer(DataSourceBuffer * newBuffer)
{
    buffer = newBuffer;
}

DataSourceBuffer *DataSourceBuffered::getBuffer(void)
{
    return buffer;
}

DataSourceBuffer * DataSourceBuffered::transferBuffer(void)
{
    DataSourceBuffer *    sourceBuffer;
                                                            // Get the buffer
    sourceBuffer = getBuffer();
                                                            // Set internal pointer to buffer to NULL because it will be transferred to the caller
    setBuffer(nullptr);
                                                            // Copy data source information
    sourceBuffer->setLocator(*this);
                                                            // Return buffer to the caller
    return sourceBuffer;
}

DataSourceStatus DataSourceBuffered::setSegmentSize(DataSource::DataSize size)
{
    segmentSize = size;

    return DataSourceStatus();
}

DataSource::DataSize DataSourceBuffered::getSegmentSize(void)
{
    return segmentSize;
}

DataSourceStatus DataSourceBuffered::waitForSegments(DataSourceBuffer::Timeout timeoutMilliseconds)
{
    if (getBuffer())
    {
        return getBuffer()->waitForSegments(timeoutMilliseconds);
    }

    return DataSourceStatus(DataSourceStatus::Status_Error);
}


bool DataSourceBuffered::isValid(void)
{
    return (getBuffer() != nullptr);
}

bool DataSourceBuffered::isEmpty(void)
    {
    return (0 == getBuffer()->getSize());
    }

DataSourceStatus DataSourceBuffered::read(Buffer *dest, DataSize destSize, DataSize &readSize, DataSize size)
{
    DataSourceStatus            status;
    DataSourceAccount *         account;

                                                                // Make buffer is at least the right size
    if (destSize < size)
        return DataSourceStatus(DataSourceStatus::Status_Error_Dest_Buffer_Too_Small);

                                                                // Get the associated account
    if ((account = getAccount()) == nullptr)
        return DataSourceStatus(DataSourceStatus::Status_Error);

                                                                // If size to read is specified
    if(size > 0)
    {
                                                                // Size the buffer ready to read segmented data into it
        if ((status = initializeBuffer(size, dest)).isFailed())
            return status;

                                                                // Download segments to the buffer
        status = account->downloadSegments(*this, dest, size);
                                                                // If all segments read OK, total size read is given size
        if (status.isOK())
        {
            readSize = size;
        }
    }
    else
    {
                                                                // Download unknown size
        status = account->downloadBlobSync(*this, dest, destSize, readSize);
        assert(destSize >= readSize); // Not enough memory was allocated to the buffer!
     }

    assert(status.isOK());
                                                                // Return status
    return status;
}

DataSourceStatus DataSourceBuffered::write(const Buffer * source, DataSize size)
{
    DataSourceStatus    status;
                                                                // If buffer is not defined, initialize one
    if (getBuffer() == nullptr)
    {
        if ((status = initializeBuffer()).isFailed())
            return status;
    }
                                                                // Append give data to internal buffer
    status = getBuffer()->append(source, size);

    return status;
}



DataSourceStatus DataSourceBuffered::initializeBuffer(DataSourceBuffer::BufferSize size, DataSource::Buffer *existingBuffer)
{
    if (getBuffer())
    {
        delete getBuffer();
    }

    setBuffer(new DataSourceBuffer(size, existingBuffer));

    if (getBuffer())
    {
        getBuffer()->initializeSegments(getSegmentSize());

        return DataSourceStatus();
    }

    return DataSourceStatus(DataSourceStatus::Status_Error);
}


DataSourceStatus DataSourceBuffered::open(const DataSourceURL & sourceURL, DataSourceMode sourceMode)
{
    return Super::open(sourceURL, sourceMode);
}

DataSourceStatus DataSourceBuffered::flush(void)
{
    DataSourceAccount    *    account;

    if ((account = getAccount()) == nullptr)
        return DataSourceStatus(DataSourceStatus::Status_Error);

    if (getMode() == DataSourceMode_Write_Segmented)
    {
        account->uploadSegments(*this);
    }
    else if (getMode() == DataSourceMode_Write)
        {
        auto buffer = this->getBuffer();

        account->uploadBlobSync(*this, buffer->getSegment(0), buffer->getSize());
                          // Upload of this buffer is complete, delete it
        delete buffer;
        setBuffer(nullptr);
        }

    return DataSourceStatus();
}

DataSourceStatus DataSourceBuffered::close(void)
{
    return flush();
}

