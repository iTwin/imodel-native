#pragma once

#include <PTRMI/Name.h>
#include <ptengine/StreamHost.h>
#include <ptengine/StreamManagerParameters.h>


namespace pointsengine
{

class StreamScheduler
{

protected:
	typedef std::map<PTRMI::Name, StreamHost>						NameStreamHostMap;
	typedef std::set<StreamHost *>									StreamHostPtrSet;
																											// Host Name -> DataSourcePtr -> StreamDataSource Set of Sets
                                                                                                            // Set of connected StreamHosts
	NameStreamHostMap					streamHosts;
	StreamHostPtrSet					streamHostsActive;

	unsigned int						numVoxelsActive;

protected:

	StreamHost						*	addStreamHost					(const PTRMI::Name &hostName);
	bool								deleteStreamHost				(const PTRMI::Name &hostName);

	void								setNumVoxelsActive				(unsigned int numVoxels);
	unsigned int						getNumVoxelsActive				(void);
	void								incrementNumVoxelsActive		(void);
	void								clearNumVoxelsActive			(void);

	bool								processStreamHostsNonStreamed	(StreamManagerParameters &params);

	void								generateStreamHostReadSets		(StreamHost &streamHost, StreamManagerParameters &params);
	bool								generateStreamHostMultiReadSet	(StreamHost &streamHost);

	bool								streamStreamHostReadSets		(StreamHost &streamHost);
	bool								streamStreamHostMultiReadSet	(StreamHost &streamHost);

public:

	virtual void						clear							(void);

	virtual void						clearActive						(void);

	StreamHost					*		addStreamHostActive				(const PTRMI::Name &hostName);
	bool								isStreamHostActive				(StreamHost *streamHost);
	unsigned int						getNumStreamHostsActive			(void);

	StreamHost					*		getStreamHost					(const PTRMI::Name &hostName);

	StreamDataSource			*		addActiveDataSourceVoxel		(DataSourcePtr dataSource, Voxel *voxel, bool *streamDataSourceCreated = NULL);

	virtual void						beginStreaming					(void);
	virtual void						endStreaming					(void);

	virtual bool						processStreamHosts				(StreamManagerParameters &params) = 0;
	virtual bool						processStreamHostsRead			(void) = 0;

	void								endStreamHostStreaming			(void);
};

} // End pointsengine namespace
