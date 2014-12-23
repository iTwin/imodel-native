#pragma once

#include <ptcloud2/Voxel.h>
#include <ptengine/pointsScene.h>
#include <ptengine/pointspager.h>
#include <ptengine/globalpagerdata.h>
#include <ptengine/StreamDataSource.h>
#include <ptengine/StreamManagerDataSourceSet.h>
#include <ptengine/StreamHost.h>

#define	STREAM_MANAGER_RACE_LIMIT_SECONDS	1


namespace pointsengine
{

class StreamManager;


class StreamManagerParameters
{
public:

typedef pointsengine::PointsScene::VOXELSLIST	VoxelList;
typedef pointsengine::PointsPager::MemMode		MemMode;

protected:

	StreamManager				*	streamManager;

	PointsPager::Pager			*	pager;
	VoxelList					*	voxelList;
	MemMode							memoryMode;
	GlobalPagerData				*	globalPagerData;

	StreamHost					*	streamHost;
	DataSource::DataSize			streamBudgetPerVoxel;
	DataSource::DataSize			streamBudgetTotal;
	DataSource::DataSize			streamBudgetIteration;

	DataSource::DataSourceForm		streamHostForm;

	StreamHost::StreamTime			streamPeriodTotal;
	StreamHost::NumVoxels			numVoxelsTotal;

public:

	StreamManagerParameters(StreamManager &initStreamManager, PointsPager::Pager &initPager, VoxelList &initVoxelList, MemMode initMemoryMode, GlobalPagerData &initGlobalPagerData, StreamHost::StreamTime initStreamPeriodTotal = STREAM_MANAGER_RACE_LIMIT_SECONDS)
	{
		setStreamManager(&initStreamManager);

		setPager(initPager);

		setVoxelList(initVoxelList);

		setMemoryMode(initMemoryMode);

		setGlobalPagerData(initGlobalPagerData);

		setStreamDataSourceStreamHost(NULL);

		setStreamBudgetTotal(0);
		setStreamBudgetIteration(0);
		setStreamBudgetPerVoxel(0);

		setStreamPeriodTotal(initStreamPeriodTotal);
	}

	void							setStreamManager					(StreamManager *initStreamManager)		{streamManager = initStreamManager;}
	StreamManager				*	getStreamManager					(void)									{return streamManager;}

	void							setPager							(PointsPager::Pager	&initPager)			{pager = &initPager;}
	PointsPager::Pager			&	getPager							(void)									{return *pager;}

	void							setVoxelList						(VoxelList &initVoxelList)				{voxelList = &initVoxelList;}
	VoxelList					&	getVoxelList						(void)									{return *voxelList;}

	void							setMemoryMode						(MemMode initMemoryMode)				{memoryMode = initMemoryMode;}
	MemMode							getMemoryMode						(void)									{return memoryMode;}

	void							setGlobalPagerData					(GlobalPagerData &initGlobalPagerData)	{globalPagerData = &initGlobalPagerData;}
	GlobalPagerData				&	getGlobalPagerData					(void)									{return *globalPagerData;}

	void							setStreamBudgetPerVoxel				(DataSource::DataSize streamBudgetVoxel);
	DataSource::DataSize			getStreamBudgetPerVoxel				(void);

	void							setStreamDataSourceStreamHost		(StreamHost *streamHost);
	StreamHost					*	getStreamDataSourceStreamHost		(void);

	void							setStreamBudgetTotal				(DataSource::DataSize streamBudgetTotal);
	DataSource::DataSize			getStreamBudgetTotal				(void);

	void							setStreamBudgetIteration			(DataSource::DataSize streamBudgetIteration);
	DataSource::DataSize			getStreamBudgetIteration			(void);

	void							setStreamHostForm					(DataSource::DataSourceForm form);
	DataSource::DataSourceForm		getStreamHostForm					(void);

	void							setStreamPeriodTotal				(StreamHost::StreamTime initStreamTimeTotal);
	StreamHost::StreamTime			getStreamPeriodTotal				(void);
};


} // End pointsengine namespace

