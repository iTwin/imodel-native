#pragma once
#include "DataSourceDefs.h"
#include "DataSourceBuffer.h"
#include "DataSource.h"


class DataSourceBuffered : public DataSource
{

protected:

    typedef DataSource          Super;

    DataSourceBuffer        *   buffer;
    DataSize                    segmentSize;

protected:

    DataSourceStatus            initializeBuffer        (DataSourceBuffer::BufferSize size = 0, DataSource::Buffer * existingBuffer = nullptr);

public:
                                DataSourceBuffered      (DataSourceAccount *sourceAccount);

    bool                        isValid                 (void);
    bool                        isEmpty                 (void);

    DataSourceStatus            open                    (const DataSourceURL & sourceURL, DataSourceMode sourceMode);
    DataSourceStatus            close                   (void);

    DataSourceStatus            read                    (Buffer *dest, DataSize destSize, DataSize &readSize, DataSize size = 0);
    DataSourceStatus            write                   (const Buffer * source, DataSize size);

    DataSourceStatus            flush                   (void);

                    void                        setBuffer               (DataSourceBuffer *newBuffer);
    CLOUD_EXPORT    DataSourceBuffer        *   getBuffer               (void);

    DataSourceBuffer        *   transferBuffer          (void);

    CLOUD_EXPORT    DataSourceStatus            setSegmentSize          (DataSource::DataSize size);
    DataSource::DataSize        getSegmentSize          (void);

    DataSourceStatus            waitForSegments         (DataSourceBuffer::Timeout timeoutMilliseconds);
};


