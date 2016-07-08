#pragma once
#include "DataSourceDefs.h"
#include "DataSource.h"
#include "DataSourceBuffered.h"



class DataSourceCached : public DataSourceBuffered
{

protected:

	typedef	DataSourceBuffered	Super;

protected:

	bool						cachingEnabled;
	DataSourceURL				cacheURL;
	bool						writeCache;

	DataSource				*	cacheDataSource;

protected:

	bool						isCached				(void);

	DataSourceStatus			readFromCache			(DataSourceBuffer::BufferData *dest, DataSourceBuffer::BufferSize destSize, DataSourceBuffer::BufferSize &readSize, DataSourceBuffer::BufferSize size);
	DataSourceStatus			writeToCache			(DataSourceBuffer::BufferData *source, DataSourceBuffer::BufferSize size);

	void						setWriteToCache			(bool write);
	bool						getWriteToCache			(void);

	void						setCacheURL				(const DataSourceURL & url);
	const DataSourceURL		&	getCacheURL				(void);

	void						setCacheDataSource		(DataSource *source);
	DataSource				*	getCacheDataSource		(void);

public:

CLOUD_EXPORT								DataSourceCached		(DataSourceAccount *account);

CLOUD_EXPORT	void						setCachingEnabled		(bool enabled);
CLOUD_EXPORT	bool						getCachingEnabled		(void);

CLOUD_EXPORT	DataSourceStatus			open					(const DataSourceURL & sourceURL, DataSourceMode sourceMode);
CLOUD_EXPORT	DataSourceStatus			close					(void);

CLOUD_EXPORT	DataSourceStatus			read					(Buffer *dest, DataSize destSize, DataSize &readSize, DataSize size);
CLOUD_EXPORT	DataSourceStatus			write					(Buffer * source, DataSize size);

};