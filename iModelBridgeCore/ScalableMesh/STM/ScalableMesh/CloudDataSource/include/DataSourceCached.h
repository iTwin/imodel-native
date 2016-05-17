#pragma once

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

	DataSourceStatus			readFromCache			(DataSourceBuffer::BufferData *dest, DataSourceBuffer::BufferSize size);
	DataSourceStatus			writeToCache			(DataSourceBuffer::BufferData *source, DataSourceBuffer::BufferSize size);

	void						setWriteToCache			(bool write);
	bool						getWriteToCache			(void);

	void						setCacheURL				(const DataSourceURL & url);
	const DataSourceURL		&	getCacheURL				(void);

	void						setCacheDataSource		(DataSource *source);
	DataSource				*	getCacheDataSource		(void);

public:

								DataSourceCached		(DataSourceAccount *account);

	void						setCachingEnabled		(bool enabled);
	bool						getCachingEnabled		(void);

	DataSourceStatus			open					(const DataSourceURL & sourceURL, DataSourceMode sourceMode);
	DataSourceStatus			close					(void);

	DataSourceStatus			read					(Buffer * dest, DataSize size);
	DataSourceStatus			write					(Buffer * source, DataSize size);

};