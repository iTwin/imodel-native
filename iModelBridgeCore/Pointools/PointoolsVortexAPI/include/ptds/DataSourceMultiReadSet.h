#pragma once


#include <ptds/DataSource.h>
#include <ptds/DataSourceMultiRead.h>
#include <PTRMI/DataBuffer.h>
#include <PTRMI/Status.h>

using namespace PTRMI;

#define	PTDS_DATA_SOURCE_MULTI_READ_SET_MAX_MULTI_READS 1024


namespace ptds
{

class DataSourceMultiRead;

class DataSourceMultiReadSet
{

public:

	typedef unsigned int								MultiReadIndex;
	typedef unsigned int								NumMultiReads;
	typedef ptds::DataSize								DataSize;

protected:

	typedef std::vector<DataSourceMultiRead>			MultiReadSet;

protected:

	MultiReadSet					multiReadSet;

	DataSize						totalReadSize;

	bool							enabled;

protected:

	void							setTotalReadSize			(DataSize initTotalReadSize);

public:
									DataSourceMultiReadSet		(void);

	void							clear						(bool deleteReadSets = false);
	Status							deleteAll					(void);

	void							setEnabled					(bool initEnabled);
	bool							getEnabled					(void);

	DataSourceMultiRead			*	addMultiRead				(DataSourceMultiRead &multiRead);

	NumMultiReads					getNumMultiReads			(void) const;
	ptds::DataSize					getTotalReadSize			(void) const;
	unsigned int					getNumReads					(void) const;

	DataSourceMultiRead			 *	getMultiRead				(MultiReadIndex index);
	const DataSourceMultiRead	 *	getMultiRead				(MultiReadIndex index) const;

	void							read						(PTRMI::DataBuffer &buffer);
	void							write						(PTRMI::DataBuffer &buffer) const;

	DataSource::Data			*	executeMultiReadSet			(DataSource &dataSource, DataSource::Data *buffer);
};

} // End ptds namespace
