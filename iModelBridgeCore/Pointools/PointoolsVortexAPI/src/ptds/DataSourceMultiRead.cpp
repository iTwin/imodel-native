
#include <ptds/DataSourceMultiRead.h>
#include <ptds/DataSourceReadSet.h>


namespace ptds
{



DataSourceMultiRead::DataSourceMultiRead(void)
{
	clear();
}

DataSourceMultiRead::DataSourceMultiRead(DataSourceReadSet &initReadSet, PTRMI::GUID &initGUID)
{
	setDataSourceReadSet(&initReadSet);

	setDataSourceServerInterface(initGUID);
}


void DataSourceMultiRead::clear(void)
{
															// Initially no ReadSet
	setDataSourceReadSet(NULL);
}




PTRMI::Status DataSourceMultiRead::deleteAll(void)
{
															// Delte the Read Set
	if(getDataSourceReadSet())
	{
		delete getDataSourceReadSet();
	}

	clear();

	return Status();
}


void DataSourceMultiRead::setDataSourceReadSet(DataSourceReadSet *readSet)
{
	dataSourceReadSet = readSet;
}


DataSourceReadSet *DataSourceMultiRead::getDataSourceReadSet(void) const
{
	return dataSourceReadSet;
}


void DataSourceMultiRead::setDataSourceServerInterface(PTRMI::GUID &object)
{
	dataSourceServerInterface = object;
}


PTRMI::GUID DataSourceMultiRead::getDataSourceServerInterface(void) const
{
	return dataSourceServerInterface;
}


void DataSourceMultiRead::read(DataBuffer &buffer)
{
	DataSourceReadSet	*	readSet;

	if(readSet = new DataSourceReadSet())
	{
															// Bind the new ReadSet to the 
		setDataSourceReadSet(readSet);

		buffer >> dataSourceServerInterface;

		buffer >> (*readSet);
	}
}


void DataSourceMultiRead::write(DataBuffer &buffer) const
{
															// If ReadSet is defined (should be)
	if(getDataSourceReadSet())
	{
															// Write remote data source object name
		buffer << getDataSourceServerInterface();
															// Write the ReadSet
		buffer << (*getDataSourceReadSet());
	}
}


DataSource::DataSize DataSourceMultiRead::getTotalReadSize(void) const
{
	DataSourceReadSet *	readSet;

	if(readSet = getDataSourceReadSet())
	{
		return readSet->getTotalReadSize();
	}

	return 0;
}


unsigned int DataSourceMultiRead::getNumReads(void) const
{
	DataSourceReadSet *	readSet;

	if(readSet = getDataSourceReadSet())
	{
		return readSet->getNumReads();
	}

	return 0;
}

} // End ptds namespace