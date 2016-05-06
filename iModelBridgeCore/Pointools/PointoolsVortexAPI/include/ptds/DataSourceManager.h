#pragma once

#include <ptds/FilePath.h>
#include <ptds/DataSource.h>
#include <ptds/DataSourceNull.h>
#include <ptds/DataSourceFile.h>
#include <ptds/DataSourceStructuredStorage.h>

namespace PTRMI
{
	class ClientInterfaceExtData;
	class GUID;
}
namespace pcloud
{
	class Voxel;
}

namespace ptds
{


class DataSourceManager
{
protected:

public:
									DataSourceManager					(void);

	DataSourcePtr					openForRead							(const FilePath *filepath, DataSource::Data *sourceBuffer = NULL, DataSource::DataSize sourceBufferSize = 0);
	DataSourcePtr					openForWrite						(const FilePath *filepath);
	DataSourcePtr					openForReadWrite					(const FilePath *filepath);

	bool							close								(DataSource *dataSource);

	DataSourcePtr					createDataSource					(const FilePath *filepath, DataSource::Data *sourceBuffer = NULL, DataSource::DataSize sourceBufferSize = 0);
	bool							deleteDataSource					(DataSourcePtr fileHandler);

#ifndef NO_DATA_SOURCE_SERVER
#ifdef NEEDS_WORK_VORTEX_DGNDB
	DataSource					*	createRemoteDataSourceServer		(const wchar_t *clientFilePath, const wchar_t *serverFilePath);
	DataSource					*	createDataSourceCache				(const wchar_t *clientFilePath, const wchar_t *serverFilePath, PTRMI::GUID *fileGUID);
#endif
	DataSource					*	getVoxelDataSource					(pcloud::Voxel *voxel, int thread);
	DataSource					*	getOrCreateOpenVoxelDataSource	(pcloud::Voxel *voxel, int thread);

#endif

};


extern DataSourceManager	dataSourceManager;

DataSourceManager &getDataSourceManager(void);

} // End ptds namespace
