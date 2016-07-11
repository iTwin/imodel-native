#pragma once
#include "DataSourceDefs.h"
#include <mutex>

#pragma warning(push)
#pragma warning(disable: 4002)
#include <ppl.h>
#pragma warning(pop)

#include <condition_variable>
#include <deque>
#include "DataSourceBuffer.h"
#include "DataSourceStatus.h"


class DataSourceTransferScheduler
{

protected:

	typedef std::deque<DataSourceBuffer *>			DataSourceBufferSet;
	typedef unsigned int							TaskIndex;

protected:

	TaskIndex					maxTasks;

	std::mutex					dataSourceBuffersMutex;
	std::condition_variable		dataSourceBufferReady;

	DataSourceBufferSet			dataSourceBuffers;
	
	concurrency::task_group		transferTasks;

protected:

	DataSourceBuffer *			getBuffer;

protected:

	DataSourceBuffer *			getNextSegmentJob				(DataSourceBuffer::BufferData ** buffer, DataSourceBuffer::BufferSize * bufferSize, DataSourceBuffer::SegmentIndex * index);

	DataSourceStatus			initializeTransferTasks			(unsigned int maxTasks);

	DataSourceStatus			scheduleUploadJob				(void);

	DataSourceStatus			getAndProcessUploadJob			(void);

	void						setMaxTasks						(TaskIndex numTasks);
	TaskIndex					getMaxTasks						(void);

public:

								DataSourceTransferScheduler		(void);

	DataSourceStatus			addBuffer					(DataSourceBuffer &buffer);
};

