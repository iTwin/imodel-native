#pragma once
#include "DataSourceDefs.h"
#include <thread>
#include <mutex>

#include <condition_variable>
#include <deque>
#include "DataSourceBuffer.h"
#include "DataSourceStatus.h"

const unsigned int DEFAULT_NUM_TRANSFERS = 16;


class DataSourceTransferScheduler
{

protected:

    typedef std::deque<DataSourceBuffer *>          DataSourceBufferSet;
    typedef unsigned int                            TaskIndex;

    typedef std::vector<std::thread>                TransferThreads;

    typedef std::mutex                              DataSourceBuffersMutex;

protected:

    TaskIndex                       maxTasks;

    DataSourceBuffersMutex          dataSourceBuffersMutex;
    std::condition_variable         dataSourceBufferReady;

    DataSourceBufferSet             dataSourceBuffers;
    
    TransferThreads                 transferThreads;

protected:

    DataSourceBuffer *              getBuffer;
    volatile bool                   shutDownFlag;

protected:

    DataSourceBuffer *              getNextSegmentJob               (DataSourceBuffer::BufferData ** buffer, DataSourceBuffer::BufferSize * bufferSize, DataSourceBuffer::SegmentIndex * index);

    DataSourceStatus                scheduleUploadJob               (void);

    DataSourceStatus                getAndProcessUploadJob          (void);

    void                            setMaxTasks                     (TaskIndex numTasks);
    TaskIndex                       getMaxTasks                     (void);

    void                            setShutDownFlag                 (bool shutDown);
    bool                            getShutDownFlag                 (void);

public:

                                    DataSourceTransferScheduler     (void);
                                   ~DataSourceTransferScheduler     (void);

    DataSourceStatus                initializeTransferTasks         (unsigned int maxTasks);

    DataSourceStatus                addBuffer                       (DataSourceBuffer &buffer);
    DataSourceStatus                removeBuffer                    (DataSourceBuffer &buffer);

    void                            shutDown                        (void);

};

