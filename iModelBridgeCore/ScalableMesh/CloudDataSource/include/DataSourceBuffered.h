#pragma once
#include "DataSourceDefs.h"
#include "DataSourceBuffer.h"
#include "DataSource.h"


class DataSourceBuffered : public DataSource
{

protected:

	typedef	DataSource			Super;

	DataSourceBuffer		*	buffer;
	DataSize					segmentSize;

protected:

	DataSourceStatus			initializeBuffer		(DataSourceBuffer::BufferSize size = 0, DataSource::Buffer * existingBuffer = nullptr);

public:
CLOUD_EXPORT								DataSourceBuffered		(DataSourceAccount *sourceAccount);

CLOUD_EXPORT	bool						isValid					(void);

CLOUD_EXPORT	DataSourceStatus			open					(const DataSourceURL & sourceURL, DataSourceMode sourceMode);
CLOUD_EXPORT	DataSourceStatus			close					(void);

CLOUD_EXPORT	DataSourceStatus			read					(Buffer *dest, DataSize destSize, DataSize &readSize, DataSize size = 0);
CLOUD_EXPORT	DataSourceStatus			write					(Buffer * source, DataSize size);

CLOUD_EXPORT	DataSourceStatus			flush					(void);

CLOUD_EXPORT	void						setBuffer				(DataSourceBuffer *newBuffer);
CLOUD_EXPORT	DataSourceBuffer		*	getBuffer				(void);

CLOUD_EXPORT	DataSourceBuffer		*	transferBuffer			(void);

CLOUD_EXPORT	DataSourceStatus			setSegmentSize			(DataSource::DataSize size);
CLOUD_EXPORT	DataSource::DataSize		getSegmentSize			(void);

CLOUD_EXPORT	TimeoutStatus				waitForSegments			(DataSourceBuffer::Timeout timeoutMilliseconds);
};


