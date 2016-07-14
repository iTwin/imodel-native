#pragma once
#include "DataSourceDefs.h"
#include <vector>
#include "DataSourceStatus.h"
#include "ActivitySemaphore.h"
#include "DataSourceLocator.h"


class DataSourceBuffer

{
public:

    typedef unsigned char                BufferData;
    typedef unsigned long long            BufferSize;
    typedef unsigned int                SegmentIndex;
    typedef ActivitySemaphore::Timeout    Timeout;
    typedef ActivitySemaphore::Status    TimeoutStatus;

protected:
    typedef std::vector<BufferData>        Buffer;

protected:

    DataSourceLocator                    locator;

    std::mutex                            segmentMutex;
    ActivitySemaphore                    activitySemaphore;

    Buffer                                buffer;

    BufferData                        *    externalBuffer;
    BufferSize                            externalBufferSize;

    BufferSize                            segmentSize;
    SegmentIndex                        currentSegmentIndex;

    void                                setSegmentSize                        (BufferSize size);
    BufferSize                            getSegmentSize                        (void);
    BufferSize                            getLastSegmentSize                    (void);

    void                                setCurrentSegmentIndex                (SegmentIndex index);
    SegmentIndex                        getCurrentSegmentIndex                (void);

    BufferData                        *    getSegment                            (SegmentIndex index);

    void                                setExternalBuffer                    (BufferData *extBuffer);

    void                                setExternalBufferSize                (BufferSize size);

    ActivitySemaphore                &    getActivitySemaphore                (void);

public:

                                        DataSourceBuffer                    (void);
                                        DataSourceBuffer                    (BufferSize size, BufferData *extBuffer = nullptr);

    void                                initializeSegments                    (void);
    void                                initializeSegments                    (BufferSize segmentSize);

    void                                setLocator                            (const DataSourceLocator &newLocator);
    DataSourceLocator                &    getLocator                            (void);

    BufferSize                            getSize                                (void);
    SegmentIndex                        getNumSegments                        (void);

    DataSourceStatus                    clear                                (void);
    DataSourceStatus                    append                                (BufferData *source, BufferSize size);
    DataSourceStatus                    expand                                (BufferSize size);

    SegmentIndex                        getAndAdvanceCurrentSegment            (BufferData ** dest, BufferSize * size);
    bool                                signalSegmentProcessed                (void);
    TimeoutStatus                        waitForSegments                        (Timeout timeoutMilliseconds);

    BufferData                        *    getExternalBuffer                    (void);
    BufferSize                            getExternalBufferSize                (void);

};

