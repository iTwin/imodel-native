#pragma once
#include "DataSourceDefs.h"
#include <vector>
#include "DataSourceStatus.h"
#include "ActivitySemaphore.h"
#include "DataSourceLocator.h"


class DataSourceBuffer

{
public:

	typedef unsigned char				BufferData;
	typedef unsigned long long			BufferSize;
	typedef unsigned int				SegmentIndex;
	typedef ActivitySemaphore::Timeout	Timeout;
	typedef ActivitySemaphore::Status	TimeoutStatus;

protected:
	typedef std::vector<BufferData>		Buffer;

protected:

	DataSourceLocator					locator;

	std::mutex							segmentMutex;
	ActivitySemaphore					activitySemaphore;

	Buffer								buffer;

	BufferData						*	externalBuffer;
	BufferSize							externalBufferSize;

	BufferSize							segmentSize;
	SegmentIndex						currentSegmentIndex;

	void								setSegmentSize						(BufferSize size);
	BufferSize							getSegmentSize						(void);
	BufferSize							getLastSegmentSize					(void);

	void								setCurrentSegmentIndex				(SegmentIndex index);
	SegmentIndex						getCurrentSegmentIndex				(void);

	BufferData						*	getSegment							(SegmentIndex index);

	void								setExternalBuffer					(BufferData *extBuffer);

	void								setExternalBufferSize				(BufferSize size);

	ActivitySemaphore				&	getActivitySemaphore				(void);

public:

CLOUD_EXPORT										DataSourceBuffer					(void);
CLOUD_EXPORT										DataSourceBuffer					(BufferSize size, BufferData *extBuffer = nullptr);

CLOUD_EXPORT	void								initializeSegments					(void);
CLOUD_EXPORT	void								initializeSegments					(BufferSize segmentSize);

CLOUD_EXPORT	void								setLocator							(const DataSourceLocator &newLocator);
CLOUD_EXPORT	DataSourceLocator				&	getLocator							(void);

CLOUD_EXPORT	BufferSize							getSize								(void);
CLOUD_EXPORT	SegmentIndex						getNumSegments						(void);

CLOUD_EXPORT	DataSourceStatus					clear								(void);
CLOUD_EXPORT	DataSourceStatus					append								(BufferData *source, BufferSize size);
CLOUD_EXPORT	DataSourceStatus					expand								(BufferSize size);

CLOUD_EXPORT	SegmentIndex						getAndAdvanceCurrentSegment			(BufferData ** dest, BufferSize * size);
CLOUD_EXPORT	bool								signalSegmentProcessed				(void);
CLOUD_EXPORT	TimeoutStatus						waitForSegments						(Timeout timeoutMilliseconds);

CLOUD_EXPORT	BufferData						*	getExternalBuffer					(void);
CLOUD_EXPORT	BufferSize							getExternalBufferSize				(void);

};

