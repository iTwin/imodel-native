#pragma once

#include <PTRMI/GUID.h>
#include <PTRMI/DataBuffer.h>
#include <ptds/DataSource.h>


using namespace PTRMI;

namespace ptds
{

class DataSourceReadSet;

class DataSourceMultiRead
{

	typedef DataSource::DataSize	DataSize;


protected:

	PTRMI::GUID				dataSourceServerInterface;

	DataSourceReadSet	*	dataSourceReadSet;

public:

							DataSourceMultiRead				(void);
							DataSourceMultiRead				(DataSourceReadSet &readSet, PTRMI::GUID &guid);

	void					clear							(void);

	Status					deleteAll						(void);

	void					setDataSourceReadSet			(DataSourceReadSet *readSet);
	DataSourceReadSet	*	getDataSourceReadSet			(void) const;

	void					setDataSourceServerInterface	(PTRMI::GUID &object);
	PTRMI::GUID				getDataSourceServerInterface	(void) const;

	DataSize				getTotalReadSize				(void) const;
	unsigned int			getNumReads						(void) const;
	void					read							(DataBuffer &buffer);
	void					write							(DataBuffer &buffer) const;

};


} // End ptds Namespace