#pragma once

#include <PTRMI/KeyKeyMap.h>
#include <PTRMI/URL.h>
#include <ptengine/StreamDataSource.h>
#include <ptengine/StreamHost.h>


namespace ptds
{
	class DataSource;
}

namespace pointsengine
{
	class StreamManager;
	class StreamManagerParameters;
}

namespace pcloud
{
	class Voxel;
}

namespace pointsengine
{


class StreamHostManager
{

protected:
																											// Host URL -> DataSourcePtr -> StreamDataSource Set of Sets
	typedef PTRMI::KeyKeyMap<PTRMI::URL, ptds::DataSourcePtr, StreamDataSource>	StreamHostStreamDataSourceMap;


public:

	typedef std::map<PTRMI::URL, StreamHost>						URLStreamHostMap;

	typedef StreamHostStreamDataSourceMap::Group					StreamDataSourceGroup;

	typedef std::map<PTRMI::URL, StreamHost *>						URLStreamHostPtrMap;


protected:
																											// Set of connected StreamHosts
	URLStreamHostMap					streamHosts;
																											// Set of actively streaming hosts with each host's set of active data sources
	StreamHostStreamDataSourceMap		streamHostDataSourceGroups;

	URLStreamHostPtrMap					streamHostsActive;

protected:

	bool								addStreamHost					(const PTRMI::URL &streamDataSourceGroupURL);
	bool								deleteStreamHost				(const PTRMI::URL &streamDataSourceGroupURL);

public:

	void								clear							(void);

	void								clearActiveStreamHosts			(void);
	bool								addActiveStreamHost				(const PTRMI::URL &streamDataSourceGroupURL);
	StreamHost					*		getActiveStreamHost				(const PTRMI::URL &streamDataSourceGroupURL);

	void								beginStreaming					(void);
	void								endStreaming					(void);

	StreamDataSource			*		addStreamDataSource				(DataSourcePtr dataSource);
	StreamDataSource			*		getStreamDataSource				(DataSourcePtr dataSource);

	bool								addStreamDataSourceVoxel		(DataSourcePtr dataSource, pcloud::Voxel *voxel);

	bool								applyStreamHosts				(StreamManager &streamManager, PTRMI::Status (StreamManager::*func)(const PTRMI::URL &, StreamManagerParameters &), StreamManagerParameters &parameters);
	bool								applyStreamHostDataSources		(const PTRMI::URL &groupURL, StreamManager &streamManager, PTRMI::Status (StreamManager::*func)(StreamDataSource &, StreamManagerParameters &), StreamManagerParameters &parameters);
	bool								applyStreamHostDataSources		(StreamDataSourceGroup *group, StreamManager &streamManager, PTRMI::Status (StreamManager::*func)(StreamDataSource &, StreamManagerParameters &), StreamManagerParameters &parameters);
																		
	DataSource::DataSourceForm			getStreamHostForm				(const PTRMI::URL &groupURL);

	StreamHost					*		getStreamHost					(const PTRMI::URL &streamDataSourceGroupURL);
	StreamDataSourceGroup		*		getStreamHostDataSourceGroup	(const PTRMI::URL &groupURL);

	unsigned int						getNumStreamDataSourceGroups	(void);
	unsigned int						getNumStreamDataSources			(void);
	unsigned int						getNumGroupDataSources			(PTRMI::URL &groupURL);
	unsigned int						getNumVoxels					(void);
};


} // End pointsengine namespace