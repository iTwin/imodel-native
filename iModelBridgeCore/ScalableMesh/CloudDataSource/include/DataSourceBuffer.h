#pragma once
#include "DataSourceDefs.h"
#include <vector>
#include "DataSourceStatus.h"
#include "ActivitySemaphore.h"
#include "DataSourceLocator.h"


class DataSourceBuffer

{
public:

    typedef unsigned char               BufferData;
    typedef unsigned long long          BufferSize;
    typedef unsigned int                SegmentIndex;
    typedef ActivitySemaphore::Timeout  Timeout;
    typedef ActivitySemaphore::Status   TimeoutStatus;

private:
    friend class DataSourceTransferScheduler;
    bool m_isSegmented = false;
    BufferSize m_totalReadSize = 0;

protected:
    typedef std::vector<BufferData>     Buffer;

protected:

    DataSourceLocator                   locator;

    std::mutex                          segmentMutex;
    ActivitySemaphore                   activitySemaphore;

    Buffer                              buffer;
    std::mutex                          mutex;

    BufferData                        * externalBuffer;
    BufferSize                          externalBufferSize;

    BufferSize                          m_segmentSize;
    SegmentIndex                        m_currentSegmentIndex;

    DataSourceStatus                    transferStatus;

protected:

    void                                setSegmentSize                      (BufferSize size);
    BufferSize                          getSegmentSize                      (void);
    BufferSize                          getLastSegmentSize                  (void);

    void                                setCurrentSegmentIndex              (SegmentIndex index);
    SegmentIndex                        getCurrentSegmentIndex              (void);

    void                                setExternalBuffer                   (BufferData *extBuffer);

    void                                setExternalBufferSize               (BufferSize size);

    ActivitySemaphore                &  getActivitySemaphore                (void);

    DataSourceStatus                    getDataSourceStatus                 (TimeoutStatus status);

public:

                                        DataSourceBuffer                    (void);
                                        DataSourceBuffer                    (BufferSize size, BufferData *extBuffer = nullptr);

                                        ~DataSourceBuffer                   ();

    bool                                isSegmented                         (void);

    void                                initializeSegments                  (void);
    void                                initializeSegments                  (BufferSize segmentSize);

    void                                setLocator                          (const DataSourceLocator &newLocator);
    DataSourceLocator                &  getLocator                          (void);

    CLOUD_EXPORT BufferSize             getSize                             (void);
    SegmentIndex                        getNumSegments                      (void);

    DataSourceStatus                    clear                               (void);
    DataSourceStatus                    append                              (const BufferData *source, BufferSize size);
    DataSourceStatus                    expand                              (BufferSize size);

    void                                setSegmented                        (const bool& value);
    BufferData                        * getSegment                          (SegmentIndex index);

    SegmentIndex                        getAndAdvanceCurrentSegment         (BufferData ** dest, BufferSize * size);
    bool                                signalSegmentProcessed              (void);
    void                                signalCancelled                     (void);
    DataSourceStatus                    waitForSegments                     (Timeout timeoutMilliseconds, int numRetries = 1);

    BufferData                        * getExternalBuffer                   (void);
    BufferSize                          getExternalBufferSize               (void);

    void                                updateReadSize                      (DataSourceBuffer::BufferSize readSize);
    DataSourceBuffer::BufferSize        getReadSize                         (void);

    void                                setTransferStatus(DataSourceStatus status);
    DataSourceStatus                    getTransferStatus(void);


};

