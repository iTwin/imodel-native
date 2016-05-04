
#pragma once

#include <ptcloud2/Voxel.h>
#include <ptengine/pointsScene.h>
#include <ptengine/pointspager.h>
#include <ptengine/globalpagerdata.h>
#include <ptengine/StreamDataSource.h>
#include <ptengine/StreamManagerDataSourceSet.h>

#include <ptengine/StreamManagerParameters.h>

#include <PTRMI/Mutex.h>

//#include <set>

//using namespace pointsengine;
//using namespace pcloud;
using namespace ptds;

namespace pointsengine
{

class StreamScheduler;

class StreamManager
{

public:

	typedef pointsengine::PointsScene::VOXELSLIST							VoxelList;
	typedef pointsengine::PointsPager::MemMode								MemMode;

	typedef	uint64_t												Iteration;

protected:

	PTRMI::Mutex					streamManagerMutex;

	Iteration						iteration;
	bool							beginEnd;

	StreamScheduler				*	streamScheduler;


protected:

	bool							applyMemoryMode							(MemMode memoryMode, bool ooc, bool &new_ooc, Voxel::LOD req, Voxel::LOD lod, float &am, bool vis, Voxel::LOD &ami);
	void							calculateLODChanges						(float &am, Voxel *voxel, Voxel::LOD &req, Voxel::LOD &lod, bool &ooc, bool &new_ooc, bool &vis, MemMode memoryMode, Voxel::LOD &ami);

	bool							loadLOD									(Voxel *voxel, float ami, Voxel::LOD lod, bool ooc, bool new_ooc);
	bool							unloadLOD								(Voxel *voxel, float am, Voxel::LOD ami, Voxel::LOD lod, bool ooc, bool new_ooc);
	void							load									(Voxel *voxel, bool new_ooc, float am, GlobalPagerData &globalPagerData, PointsPager::Pager &pager, float lodRead);
	void							unload									(Voxel *voxel, bool new_ooc, float am, GlobalPagerData &globalPagerData, PointsPager::Pager &pager);

	void							loadStreamBudget						(Voxel *voxel, DataSourcePtr dataSource, bool new_ooc, float am, GlobalPagerData &globalPagerData, PointsPager::Pager &pager, ptds::DataSize streamBudgetVoxel, ptds::DataSize &streamBudgetUsed);

	void							setBeginEnd								(bool inBeginEnd);
	bool							getBeginEnd								(void);

protected:

	void							setStreamScheduler						(StreamScheduler *scheduler);
	StreamScheduler				*	getStreamScheduler						(void);
	StreamScheduler				*	getOrCreateStreamScheduler				(void);

	int								getVoxelReadInfo						(Voxel *vox, bool full, bool lock);

	ptds::DataSize					getVoxelStreamRequirement				(Voxel *voxel, DataSourcePtr dataSource);
	ptds::DataSize					getTotalStreamRequirement				(Voxel *voxel, DataSourcePtr dataSource);
	ptds::DataSize					getVoxelStreamBudgetUniform				(StreamDataSource &dataSourceStream, ptds::DataSize totalStreamBudget);
	ptds::DataSize					getVoxelStreamBudgetUniform				(StreamHost &streamHost, ptds::DataSize streamBudgetTotal);
	void							incrementCurrentIteration				(void);

	bool							initializeStreamHostDataSourceVoxels	(StreamManagerParameters &streamManagerParameters, Iteration iteration);

	bool							addStreamedVoxel						(DataSourcePtr dataSource, Voxel *voxel);

//	bool							processRequestsDataSourceStream			(PointsPager::Pager &pager, StreamDataSource &dataSourceStream, MemMode memoryMode, GlobalPagerData &globalPagerData);

	bool							raceLimitStreaming						(unsigned int minTimeMilliseconds);

	bool							lockStreamManager						(void);
	bool							spinLockStreamManager					(void);
	bool							releaseStreamManager					(void);

public:
									StreamManager							(void);
								   ~StreamManager							(void);

	void							clear									(void);
	void							clearActive								(void);

	bool							begin									(void);
	bool							end										(void);

	bool							processRequestsStreamed					(StreamManagerParameters &streamManagerParameters);

	void							processRequestsVoxelStream				(Voxel *voxel, DataSourcePtr dataSource, GlobalPagerData &globalPagerData, PointsPager::Pager &pager, ptds::DataSize streamBudgetVoxel, ptds::DataSize &streamBudgetVoxelUsed, ptds::DataSize &streamBudgetTotalUsed, ptds::DataSize &streamBudgetIterationUsed, ptds::DataSize &streamBudgetTotalNotUsed, unsigned int &numVoxelsReceivedBudget);

	PTRMI::Status					processStreamDataSourceNonStreamed		(StreamDataSource &dataSourceStream, StreamManagerParameters &parameters);

	void							setCurrentIteration						(Iteration iteration);
	Iteration						getCurrentIteration						(void);

	StreamDataSource			*	addReadVoxel							(Voxel *voxel, DataSource *dataSource, bool *streamDataSourceCreated = NULL);
	void							endReadSets								(void);

	bool							processStreamHostsRead					(void);

	DataSourcePtr					getVoxelDataSource						(Voxel *voxel, GlobalPagerData &globalPagerData, int userThread);
};


pointsengine::StreamManager &getStreamManager(void);


} // End pointsengine namespace
