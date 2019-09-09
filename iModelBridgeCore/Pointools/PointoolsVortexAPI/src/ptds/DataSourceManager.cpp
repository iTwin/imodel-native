#include "PointoolsVortexAPIInternal.h"

#include <ptds/DataSourceManager.h>
#include <ptds/DataSourceCache.h>
#include <ptds/DataSourceMemory.h>

#ifdef NEEDS_WORK_VORTEX_DGNDB 
#include <PTRMI/ClientInterfaceExtDataBentley.h>
#ifndef NO_DATA_SOURCE_SERVER
#include <PTRMI/Manager.h>
#include <ptds/DataSourceServer.h>
#endif
#endif



#include <ptengine/globalpagerdata.h>

#define	DATA_SOURCE_MANAGER_DEFAULT_CLIENT_SERVER_CACHING_ENABLED	false


namespace ptds
{

DataSourceManager	dataSourceManager;

DataSourceManager &getDataSourceManager(void)
{
	return dataSourceManager;
}


DataSourceManager::DataSourceManager(void)
{

}


DataSourcePtr DataSourceManager::createDataSource(const FilePath *path, DataSource::Data *sourceBuffer, DataSource::DataSize sourceBufferSize)
{
	//DataSourcePtr result;
															// NOTE: Register and poll DataSource modules in future

#ifdef NEEDS_WORK_VORTEX_DGNDB
															// If path refers to a structured storage (Externally managed) create new data source
	if(DataSourceStructuredStorage::isValidPath(path))
		return DataSourceStructuredStorage::createNew(path);
															// If path is a URL to a server, create new server data source
#ifndef NO_DATA_SOURCE_SERVER
	if(DataSourceServer::isValidPath(path))
		return DataSourceServerClientInterface::createNew(path);
															// If path is URL to a data source cache, create a new data source cache
	if(DataSourceCache::isValidPath(path))
		return DataSourceCache::createNew(path);

#endif

#endif
															// If path is URL to data in buffer, create a new memory based data source
	if(DataSourceMemory::isValidPath(path))
		return DataSourceMemory::createNew(path, sourceBuffer, sourceBufferSize);
															// If path refers to a file, create new data source
	if(DataSourceFile::isValidPath(path))
		return DataSourceFile::createNew(path);

															// Not recognized path format, so return NULL data source
	return DataSourceNull::createNew(path);
}


bool DataSourceManager::deleteDataSource(DataSourcePtr dataSource)
{
	if(dataSource)
	{
		delete dataSource;

		return true;
	}

	return false;
}

DataSourcePtr DataSourceManager::openForRead(const FilePath *filepath, DataSource::Data *sourceBuffer, DataSource::DataSize sourceBufferSize)
{
	DataSourcePtr	dataSource;

	if((dataSource = dataSourceManager.createDataSource(filepath, sourceBuffer, sourceBufferSize)) == NULL)
		return NULL;

	if(dataSource->openForRead(filepath) == false)
	{
		dataSourceManager.deleteDataSource(dataSource);
		return new DataSourceNull;
	}

	return dataSource;
}


DataSourcePtr DataSourceManager::openForWrite(const FilePath *filepath)
{
	DataSourcePtr dataSource;

	if((dataSource = dataSourceManager.createDataSource(filepath)) == NULL)
		return NULL;

	if(dataSource->openForWrite(filepath, true) == false)
	{
		dataSourceManager.deleteDataSource(dataSource);
		return NULL;
	}

	return dataSource;
}


DataSourcePtr DataSourceManager::openForReadWrite(const FilePath *filepath)
{

	DataSourcePtr dataSource;

	if((dataSource = dataSourceManager.createDataSource(filepath)) == NULL)
		return NULL;

	if(dataSource->openForReadWrite(filepath, true) == false)
	{
		dataSourceManager.deleteDataSource(dataSource);
		return NULL;
	}

	return dataSource;

}


bool DataSourceManager::close(DataSource *dataSource)
{
															// If data source given
	if(dataSource)
	{
															// Close data source
		dataSource->close();
															// Ask DataSource to destroy itself
		dataSource->destroy();

		return true;
	}
															// Return data source not closed
	return false;
}


#ifndef NO_DATA_SOURCE_SERVER

#ifdef NEEDS_WORK_VORTEX_DGNDB 
DataSource *DataSourceManager::createRemoteDataSourceServer(const wchar_t *clientFilePath, const wchar_t *serverFilePath)
{
	PTRMI::Name								className(L"DataSourceServer");
	ptds::DataSourceServerClientInterface *	clientInterface;
	PTRMI::Name								dataSourceServerName;
	PTRMI::ClientInterfaceExtDataBentley  *	clientExtData = NULL;
	PTRMI::Status							status;

															// If a client side stub file is specified
	if(clientFilePath)
	{
															// If client side file specified
															// try to read Ext data from file
															// Create new Bentley Ext Data
		if((clientExtData = new ClientInterfaceExtDataBentley) == NULL)
			return NULL;
															// Read client side ext data
		if((status = clientExtData->readFile(clientFilePath)).isFailed())
		{
			return NULL;
		}
	}
	
															// Get GUID based name
	dataSourceServerName.generateGUIDName(serverFilePath);
															// Create a new data source on the remote server
	PTRMI::RemotePtr<ptds::DataSourceServer> remoteDataSource = PTRMI::getManager().newRemoteObject<ptds::DataSourceServer>(className, dataSourceServerName, clientExtData);
															// If not created, return NULL
	if((clientInterface = dynamic_cast<ptds::DataSourceServerClientInterface *>(remoteDataSource.getObjectClientInterface())) == NULL)
		return NULL;
															// Set Ext payload data in Client interface
	clientInterface->setExtData(clientExtData);
															// Store mapping from remote filepath to local fake file
	if(clientFilePath)
	{
		clientInterface->addClientSideFilePath(serverFilePath, clientFilePath);
	}

	return clientInterface;
}


DataSource *DataSourceManager::createDataSourceCache(const wchar_t *clientFilePath, const wchar_t *serverFilePath, PTRMI::GUID *fileGUID)
{
	ptds::DataSourceCache *dataSourceCache;
															// Pre-register server file path to local client file path (to stub file)
															// for data sources needing access to it (currently DataSourceServer)
	if(clientFilePath)
	{
		PTRMI::URL	fullFileSubProtocolFilePath;
															// Map DataSourceCache's full file path protocol to that used inside the data source cache
		DataSourceCache::getFullFileSubFilePath(PTRMI::URL(serverFilePath), fullFileSubProtocolFilePath);

		DataSourceServerClientInterface::addClientSideFilePath(fullFileSubProtocolFilePath.getString(), std::wstring(clientFilePath));
	}
															// Create new DataSourceCache
	if((dataSourceCache = new ptds::DataSourceCache()) == NULL)
		return NULL;
															// Set the cache file name automatically
	if(dataSourceCache->setAutoCacheFilePath(serverFilePath, *fileGUID) == false)
		return NULL;
															// Return data source
	return dataSourceCache;
}
#endif

DataSource *DataSourceManager::getVoxelDataSource(pcloud::Voxel *voxel, int userThread)
{
	if(voxel)
	{
		return getGlobalPagerData().files[voxel->fileIndex()].handle[userThread];
	}

	return NULL;
}


DataSource *DataSourceManager::getOrCreateOpenVoxelDataSource(pcloud::Voxel *voxel, int thread)
{
	DataSource *dataSource = NULL;

	if(voxel)
	{
		if((dataSource = getVoxelDataSource(voxel, thread)) == NULL)
		{
			dataSource = pointsengine::VoxelLoader::openFile(voxel, thread);
		}
	}

	return dataSource;
}	


#endif


} // End ptds Namespace
