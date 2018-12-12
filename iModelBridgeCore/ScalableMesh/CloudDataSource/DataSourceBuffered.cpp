#include "stdafx.h"
#include "DataSourceBuffered.h"
#include "DataSourceAccount.h"
#include <assert.h>
#include "include/DataSourceBuffered.h"


DataSourceBuffered::DataSourceBuffered(DataSourceAccount *sourceAccount, const SessionName &session) : Super(sourceAccount, session)
{
                                                            // Default segment size is zero until initialized
    setSegmentSize(0);
}

DataSourceBuffered::~DataSourceBuffered(void)
    {
    if (this->buffer)
        {
        delete this->buffer;
        }
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
        if ((status = initializeBuffer(size, dest, true)).isFailed())
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
        if ((status = initializeBuffer(destSize, dest, false)).isFailed())
            return status;

        status = account->download(*this, dest, destSize, readSize);
        assert(destSize >= readSize); // Not enough memory was allocated to the buffer!
     }

    //assert(status.isOK());
                                                                // Return status
    return status;
}

DataSourceStatus DataSourceBuffered::read(std::vector<Buffer>& dest)
    {
    DataSourceStatus            status;
    DataSourceAccount *         account;

    // Get the associated account
    if ((account = getAccount()) == nullptr)
        return DataSourceStatus(DataSourceStatus::Status_Error);

    if ((status = initializeBuffer(dest)).isFailed())
        return status;

    status = account->download(*this, dest);
    assert(!dest.empty()); // Nothing was downloaded to the buffer!

    return status;
    }

DataSourceStatus DataSourceBuffered::write(const Buffer * source, DataSize size)
{
    DataSourceStatus    status;
                                                                // If buffer is not defined, initialize one
    if (getBuffer() == nullptr)
    {
        if ((status = initializeBuffer(size)).isFailed())
            return status;
    }
                                                                // Append give data to internal buffer
    status = getBuffer()->append(source, size);

    return status;
}



DataSourceStatus DataSourceBuffered::initializeBuffer(DataSourceBuffer::BufferSize size, DataSource::Buffer *existingBuffer, bool sizeKnown)
{
    if (getBuffer())
    {
        delete getBuffer();
    }

    setBuffer(new DataSourceBuffer(size, nullptr));

    if (getBuffer())
    {
        if (sizeKnown)
            {
            getBuffer()->setSegmented(true);
            getBuffer()->initializeSegments(getSegmentSize());
            }
        else
            {
            getBuffer()->setSegmented(false);
                                                    // this will likely create only one segment (if size if large enough)
            getBuffer()->initializeSegments(size);
            }
        return DataSourceStatus();
    }

    return DataSourceStatus(DataSourceStatus::Status_Error);
}

DataSourceStatus DataSourceBuffered::initializeBuffer(std::vector<DataSourceBuffer::BufferData>& existingBuffer)
    {
    if (getBuffer())
        {
        delete getBuffer();
        }

    setBuffer(new DataSourceBuffer(existingBuffer));

    getBuffer()->setSegmented(false);
    return DataSourceStatus();
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
        account->upload(*this);
        }

    return DataSourceStatus();
}

DataSourceStatus DataSourceBuffered::close(void)
{
    return flush();
}

