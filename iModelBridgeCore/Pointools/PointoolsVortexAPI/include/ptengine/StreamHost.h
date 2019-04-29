#pragma once

#include <PTRMI/URL.h>
#include <ptds/DataSourceAnalyzer.h>
#include <ptds/DataSource.h>

#include <ptengine/VoxelPriorityQueue.h>

#include <ptengine/StreamDataSource.h>

#include <ptds/DataSourceMultiReadSet.h>

#include <PTRMI/DataBuffer.h>


#define STREAM_HOST_DEFAULT_MULTI_READ_BUFFER_SIZE	1024 * 1024 * 5


namespace pointsengine
{

class StreamManagerParameters;


class StreamHost
{

public:

	typedef double										StreamTime;
	typedef unsigned int								NumVoxels;

	typedef std::map<DataSourcePtr, StreamDataSource>	StreamDataSourceMap;

	typedef StreamVoxelPriorityQueue::iterator			StreamVoxelIterator;

	typedef StreamDataSourceMap::iterator				StreamDataSourceIterator;

protected:

	bool										hostInitialized;
	PTRMI::Name									hostName;
	bool										hostFeatureMultiReadSet;

	ptds::DataSourceAnalyzer					analyzer;

	DataSourceMultiReadSet						multiReadSet;
	DataBuffer									multiReadSetBuffer;

	StreamDataSourceMap							streamDataSourcesActive;
	StreamVoxelPriorityQueue					voxelsActive;

	static ptds::DataSource::DataSize			streamMinDefault;
	static ptds::DataSource::DataSize			streamMaxDefault;
	static ptds::DataSource::DataSize			streamRefreshDefault;
	static ptds::DataSource::DataSize			streamMinVoxelBudget;
	static unsigned int							streamMaxReadsDefault;
	static double								streamScalarDefault;

	static bool									streamMinDefaultEnforce;

	StreamTime									streamTime;

protected:

	void										setHostInitialized					(bool value);
	bool										getHostInitialized					(void);

	void										setHostFeatureMultiReadSet			(bool supported);
	bool										GetHostFeatureMultiReadSet			(void);

	void										setHostName							(const PTRMI::Name &name);
	const PTRMI::Name &							getHostName							(void);

	StreamDataSource						*	getOrCreateActiveStreamDataSource	(ptds::DataSourcePtr dataSource, bool *streamDataSourceCreated = NULL);
	unsigned int								getNumActiveStreamDataSources		(void);
	StreamDataSource						*	getFirstActiveStreamDataSource		(void);

	bool										isDataSourceValid					(ptds::DataSourcePtr dataSource);

	void										setMultiReadSetEnabled				(bool enabled);

	NumVoxels									getNumVoxelsActiveInReadBudget		(unsigned int maxReads);

public:

												StreamHost							(void);

	bool										initialize							(ptds::DataSize multiReadSetBufferSize);
	bool										initializeHost						(DataSourcePtr dataSource);

	bool										getHostFeatureMultiReadSet			(void);
	bool										getMultiReadSetEnabled				(void);

	void										clear								(void);

	void										clearActive							(void);

	StreamDataSource *							addActiveDataSourceVoxel			(ptds::DataSourcePtr dataSource, pcloud::Voxel *voxel, bool *streamDataSourceCreated = NULL);

	NumVoxels									getNumVoxelsActive					(void);

	void										setStreamPeriod						(StreamTime time);
	StreamTime									getStreamPeriod						(void);

	bool										beginRead							(unsigned int numMultiReads, unsigned int numReads, ptds::DataSource::DataSize numBytes);
	bool										endRead								(unsigned int numMultiReads, unsigned int numReads, ptds::DataSource::DataSize numBytes);

	bool										beginStreaming						(void);
	bool										endStreaming						(void);

	ptds::DataSize								executeMultiReadSet					(void);
	bool										loadMultiReadSetVoxelData			(void);

	bool										streamRoundTrip						(void);

	StreamVoxelIterator							getActiveVoxelBegin					(void);
	StreamVoxelIterator							getActiveVoxelEnd					(void);

	StreamDataSourceIterator					getActiveStreamDataSourceBegin		(void);
	StreamDataSourceIterator					getActiveStreamDataSourceEnd		(void);

	StreamDataSource						*	getActiveStreamDataSource			(ptds::DataSourcePtr dataSource);

	bool										generateMultiReadSet				(void);

	bool										operator<							(const StreamHost &other) const;

	static void									setStreamMinDefault					(ptds::DataSource::DataSize min);
	static ptds::DataSource::DataSize			getStreamMinDefault					(void);
	static void									setStreamMinDefaultEnforce			(bool enable);
	static bool									getStreamMinDefaultEnforce			(void);

	static void									setStreamMaxDefault					(ptds::DataSource::DataSize max);
	static ptds::DataSource::DataSize			getStreamMaxDefault					(void);

	static void									setStreamRefreshDefault				(ptds::DataSource::DataSize refresh);
	static ptds::DataSource::DataSize			getStreamRefreshDefault				(void);

	static void									setStreamScalarDefault				(float scalar);
	static float								getStreamScalarDefault				(void);

	static void									setStreamMaxReadsDefault			(unsigned int maxReads);
	static unsigned int							getStreamMaxReadsDefault			(void);

	static void									setStreamMinVoxelBudget				(ptds::DataSize budget);
	static ptds::DataSource::DataSize			getStreamMinVoxelBudget				(void);

	ptds::DataSize								getStreamBudget						(StreamManagerParameters &params, ptds::DataSource::DataSize &voxelBudget);

	ptds::DataSize								getStreamBudgetPerPeriod			(StreamTime targetStreamPeriod, StreamTime &useStreamPeriod);

	unsigned int								getNumMultiReads					(void);
	unsigned int								getNumReads							(void);
	ptds::DataSize								getTotalReadSize					(void);
};


} // End pointsengine namespace
