/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
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
#if ANDROID
    typedef uint64_t               BufferSize;
#else
    typedef unsigned long long          BufferSize;
#endif
    typedef std::vector<BufferData>     BufferVectorData;
    typedef unsigned int                SegmentIndex;
    typedef ActivitySemaphore::Timeout  Timeout;
    typedef ActivitySemaphore::Status   TimeoutStatus;

private:
    friend class DataSourceTransferScheduler;
    bool m_isSegmented = false;
    BufferSize m_totalReadSize = 0;
    bool m_isProcessing = false;

protected:

    DataSourceLocator                   locator;

    std::mutex                          segmentMutex;
    ActivitySemaphore                   activitySemaphore;

    BufferVectorData                    buffer;
    std::mutex                          mutex;

    BufferData                        * externalBuffer;
    BufferSize                          externalBufferSize;

    BufferVectorData                  * externalVector = nullptr;

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

    void                                setExternalVector                   (BufferVectorData *extVector);

    ActivitySemaphore                &  getActivitySemaphore                (void);

    DataSourceStatus                    getDataSourceStatus                 (TimeoutStatus status);

public:

                                        DataSourceBuffer                    (void);
                               explicit DataSourceBuffer                    (BufferSize size, BufferData *extBuffer = nullptr);
                               explicit DataSourceBuffer                    (BufferVectorData& extBuffer);

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

    BufferVectorData                  * getBuffer                           ();

    SegmentIndex                        getAndAdvanceCurrentSegment         (BufferData ** dest, BufferSize * size);
    bool                                signalSegmentProcessed              (void);
    void                                signalCancelled                     (void);
    DataSourceStatus                    waitForSegments                     (Timeout timeoutMilliseconds, int numRetries = 1);

    void                                setIsProcessing                     (const bool& isProcessing) { m_isProcessing = isProcessing;}
    bool                                getIsProcessing                     () { return m_isProcessing;}

    BufferData                        * getExternalBuffer                   (void);
    BufferSize                          getExternalBufferSize               (void);
    BufferVectorData                  * getExternalVector                   (void);

    void                                updateReadSize                      (DataSourceBuffer::BufferSize readSize);
    DataSourceBuffer::BufferSize        getReadSize                         (void);

    void                                setTransferStatus(DataSourceStatus status);
    DataSourceStatus                    getTransferStatus(void);


};

