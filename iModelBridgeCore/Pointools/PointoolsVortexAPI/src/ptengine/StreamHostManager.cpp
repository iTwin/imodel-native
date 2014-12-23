#include <ptengine/StreamHostManager.h>
#include <ptengine/StreamManager.h>
#include <PTRMI/URL.h>

namespace pointsengine
{

StreamDataSource *StreamHostManager::addStreamDataSource(DataSourcePtr dataSource)
{
	if(dataSource == NULL)
	{
		return NULL;
	}

	PTRMI::URL					groupURL;
	StreamDataSource			streamDataSource;
	bool						groupCreated;
															// Create example StreamDataSource
	streamDataSource.setDataSource(dataSource);
															// Get DataSource's URL for Group Key
	dataSource->getURL(groupURL);
															// Create a StreamDataSource in Group's Map 
	streamHostDataSourceGroups.addItem(groupURL, dataSource, streamDataSource, &groupCreated);

															// If a new StreamDataSource group was created
	if(groupCreated)
	{
															// Create a new StreamHost entry
		addStreamHost(groupURL);
	}
															// Retrieve the actual StreamDataSource that was added to the Group
	return streamHostDataSourceGroups.getItemPtr(dataSource);
}


StreamDataSource *StreamHostManager::getStreamDataSource(DataSourcePtr dataSource)
{
	if(dataSource == NULL)
	{
		return NULL;
	}

	return streamHostDataSourceGroups.getItemPtr(dataSource);
}


void StreamHostManager::clear(void)
{
	streamHostDataSourceGroups.clear();

	streamHosts.clear();
}


void StreamHostManager::beginStreaming(void)
{

}


void StreamHostManager::endStreaming(void)
{
	streamHostDataSourceGroups.clear();
}



bool StreamHostManager::applyStreamHosts(StreamManager &streamManager, PTRMI::Status (StreamManager::*func)(const PTRMI::URL &, StreamManagerParameters &), StreamManagerParameters &parameters)
{
															// Apply all StreamDataSource group keys (URLs) to the given function
	if(streamHostDataSourceGroups.applyGroupKeys<StreamManager, StreamManagerParameters &>(&streamManager, func, parameters, true).isOK())
	{
		return true;
	}

	return false;
}


bool StreamHostManager::applyStreamHostDataSources(const PTRMI::URL &groupURL, StreamManager &streamManager, PTRMI::Status (StreamManager::*func)(StreamDataSource &, StreamManagerParameters &), StreamManagerParameters &parameters)
{
	if(streamHostDataSourceGroups.applyGroupItems<StreamManager, StreamManagerParameters &>(groupURL, &streamManager, func, parameters, true).isOK())
	{
		return true;
	}

	return false;
}


bool StreamHostManager::applyStreamHostDataSources(StreamDataSourceGroup *group, StreamManager &streamManager, PTRMI::Status (StreamManager::*func)(StreamDataSource &, StreamManagerParameters &), StreamManagerParameters &parameters)
{
	if(streamHostDataSourceGroups.applyGroupItems<StreamManager, StreamManagerParameters &>(group, &streamManager, func, parameters, true).isOK())
	{
		return true;
	}

	return false;
}


DataSource::DataSourceForm StreamHostManager::getStreamHostForm(const PTRMI::URL &groupURL)
{
	StreamHostStreamDataSourceMap::Group			*	group;
	DataSource										*	dataSource;
	StreamHostStreamDataSourceMap::Group::iterator		it;

	if((group = streamHostDataSourceGroups.getGroup(groupURL)) != NULL)
	{
		if((it = group->begin()) != group->end())
		{
			StreamDataSource *streamDataSource = &(it->second);

			if((dataSource = streamDataSource->getDataSource()) != NULL)
			{
				return dataSource->getDataSourceForm();
			}
		}
	}

	return DataSource::DataSourceFormNULL;
}


unsigned int StreamHostManager::getNumStreamDataSourceGroups(void)
{
	return streamHostDataSourceGroups.getNumGroups();
}


unsigned int StreamHostManager::getNumStreamDataSources(void)
{
	return streamHostDataSourceGroups.getNumItems();
}


unsigned int StreamHostManager::getNumGroupDataSources(PTRMI::URL &groupURL)
{
	return streamHostDataSourceGroups.getNumGroupItems(groupURL);
}


StreamHostManager::StreamDataSourceGroup *StreamHostManager::getStreamHostDataSourceGroup(const PTRMI::URL &groupURL)
{
	return streamHostDataSourceGroups.getGroup(groupURL);
}


bool StreamHostManager::addStreamHost(const PTRMI::URL &streamDataSourceGroupURL)
{
	StreamHost *streamHost;
															// If StreamHost does not already exist
	if(getStreamHost(streamDataSourceGroupURL) == NULL)
	{
															// Add stream host
		streamHosts[streamDataSourceGroupURL] = StreamHost();
															// Get the StreamHost just added
		if((streamHost = getStreamHost(streamDataSourceGroupURL)) == NULL)
		{
			return false;
		}
															// Initialize buffer
		if(streamHost->initialize(STREAM_HOST_DEFAULT_MULTI_READ_BUFFER_SIZE) == false)
		{
															// Delete the StreamHost
			deleteStreamHost(streamDataSourceGroupURL);

			return false;
		}
	}
															// Return not added
	return false;
}


bool StreamHostManager::deleteStreamHost(const PTRMI::URL &streamDataSourceGroupURL)
{
	URLStreamHostMap::iterator	it;

	if((it = streamHosts.find(streamDataSourceGroupURL)) != streamHosts.end())
	{
		streamHosts.erase(it);

		return true;
	}

	return false;
}


StreamHost *StreamHostManager::getStreamHost(const PTRMI::URL &streamDataSourceGroupURL)
{
	URLStreamHostMap::iterator	it;
															// Look for item with group URL as Key
	if((it = streamHosts.find(streamDataSourceGroupURL)) != streamHosts.end())
	{
															// If found, return StreamDataSourceGroupInfo
		return &(it->second);
	}
															// Not found, so return NULL
	return NULL;
}


unsigned int StreamHostManager::getNumVoxels(void)
{
	return 0;
}


bool StreamHostManager::addStreamDataSourceVoxel(DataSourcePtr dataSource, Voxel *voxel)
{
	PTRMI::URL			dataSourceURL;
	PTRMI::URL			streamHostURL;
	StreamHost		*	streamHost;
/*
															// Get DataSource URL
	dataSource->getURL(dataSourceURL);
															// Extract the host's URL
	dataSourceURL.getProtocolHostAddress(streamHostURL);
															// Get or create an active StreamHost
	if((streamHost = getActiveStreamHost(streamHostURL)) == NULL)
	{
		return false;
	}

	streamHost->addVoxel(voxel);
*/
	return true;


/*
	StreamDataSource			*	streamDataSource;
	StreamHost					*	streamHost;

	if(dataSource == NULL || voxel == NULL)
	{
		return false;
	}
															// Get the StreamDataSource associated with the given DataSource
	if((streamDataSource = getStreamDataSource(dataSource)) == NULL)
	{
		if((streamDataSource = addStreamDataSource(dataSource)) == NULL)
		{
			return false;
		}
	}

	PTRMI::URL	dataSourceURL;

	dataSource->getURL(dataSourceURL);
															// Get StreamDataSource's group info
	if((streamHost = getStreamHost(dataSourceURL)) == NULL)
	{
		return false;
	}
															// Add the given voxel to the StreamDataSource
	streamDataSource->addVoxel(voxel);
															// Increment count of total number of voxels in this group
	streamHost->incrementNumGroupVoxels();

															// Return OK
	return true;
*/
}

void StreamHostManager::clearActiveStreamHosts(void)
{
	streamHostsActive.clear();
}




} // End pointsengine namespace
