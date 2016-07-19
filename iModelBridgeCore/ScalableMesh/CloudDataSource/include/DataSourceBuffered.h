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

    DataSourceStatus            open                    (const DataSourceURL & sourceURL, DataSourceMode sourceMode);
    DataSourceStatus            close                   (void);

    DataSourceStatus            read                    (Buffer *dest, DataSize destSize, DataSize &readSize, DataSize size = 0);
    DataSourceStatus            write                   (Buffer * source, DataSize size);

    DataSourceStatus            flush                   (void);

    void                        setBuffer               (DataSourceBuffer *newBuffer);
    DataSourceBuffer        *   getBuffer               (void);

    DataSourceBuffer        *   transferBuffer          (void);

    DataSourceStatus            setSegmentSize          (DataSource::DataSize size);
    DataSource::DataSize        getSegmentSize          (void);

    TimeoutStatus               waitForSegments         (DataSourceBuffer::Timeout timeoutMilliseconds);
};


