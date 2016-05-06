#include "PointoolsVortexAPIInternal.h"

#include <ptds/DataSourceMultiReadSet.h>
#include <PTRMI/Manager.h>
#include <ptds/DataSourceServer.h>

namespace ptds
{


DataSourceMultiReadSet::DataSourceMultiReadSet(void)
{
	clear();
															// Enabled by default
	setEnabled(true);
}


void DataSourceMultiReadSet::clear(bool deleteReadSets)
{
															// If deleting ReadSets
	if(deleteReadSets)
	{
		deleteAll();
	}
															// Clear all MultiRead entries
	multiReadSet.clear();
															// Initially no data
	setTotalReadSize(0);
}


Status DataSourceMultiReadSet::deleteAll(void)
{
	MultiReadIndex			t;
	NumMultiReads			numMultiReads;
	DataSourceMultiRead	*	multiRead;

	numMultiReads = getNumMultiReads();

	for(t = 0; t < numMultiReads; t++)
	{
		if(multiRead = getMultiRead(t))
		{
			multiRead->deleteAll();
		}
	}

	return Status();
}


DataSourceMultiRead *DataSourceMultiReadSet::addMultiRead(DataSourceMultiRead &multiRead)
{
	Status				status;
	DataSize			readSize = 0;

															// Add MultiRead
	multiReadSet.push_back(multiRead);
															// Add new MultiRead's read size to MultiRead total
	setTotalReadSize(getTotalReadSize() + multiRead.getTotalReadSize());

	return &(multiReadSet.back());
}


unsigned int DataSourceMultiReadSet::getNumMultiReads(void) const
{
	return multiReadSet.size();
}

DataSourceMultiRead *DataSourceMultiReadSet::getMultiRead(MultiReadIndex index)
{
	if(index < getNumMultiReads())
	{
		return &(multiReadSet[index]);
	}

	return NULL;
}


const DataSourceMultiRead *DataSourceMultiReadSet::getMultiRead(MultiReadIndex index) const
{
	if(index < getNumMultiReads())
	{
		return &(multiReadSet[index]);
	}

	return NULL;
}


void DataSourceMultiReadSet::read(PTRMI::DataBuffer &buffer)
{
	NumMultiReads		numMultiReads;
	MultiReadIndex		t;
															// Note: This is a server side read (receive) method

															// Make sure buffer is OK
	if(buffer.isValid() == false)
	{
		return;
	}
															// Get number of MultiReads
	buffer >> numMultiReads;
															// Validate number of read sets in case data buffer has bad data
	if(numMultiReads > PTDS_DATA_SOURCE_MULTI_READ_SET_MAX_MULTI_READS)
	{
		Status status(Status::Status_Error_Reading_Multi_Read_Set);

		return;
	}
															// For each MultiRead
	for(t = 0; t < numMultiReads; t++)
	{
		DataSourceMultiRead	multiRead;
															// Read the MultiRead from buffer
		buffer >> multiRead;
															// Add it to the MultiReadSet
		addMultiRead(multiRead);
	}
}


void DataSourceMultiReadSet::write(PTRMI::DataBuffer &buffer) const
{
	MultiReadIndex					t;
	NumMultiReads					numMultiReads = getNumMultiReads();
	const DataSourceMultiRead	*	multiRead;
															// Note: This is a Client side write (Send) method

															// Make sure buffer is OK
	if(buffer.isValid() == false)
	{
		return;
	}
															// Write number of MutliReads (ReadSets)
	buffer << numMultiReads;
															// For each multiRead
	for(t = 0; t < numMultiReads; t++)
	{
															// Get multiRead
		if(multiRead = getMultiRead(t))
		{
															// Write MultiRead to buffer
			buffer << (*multiRead);
		}
		else
		{
			Status status(Status::Status_Error_Writing_Multi_Read_Set);
		}
	}
}


void DataSourceMultiReadSet::setTotalReadSize(DataSize initTotalReadSize)
{
	totalReadSize = initTotalReadSize;
}


DataSourceMultiReadSet::DataSize DataSourceMultiReadSet::getTotalReadSize(void) const
{
	return totalReadSize;
}


unsigned int DataSourceMultiReadSet::getNumReads(void) const
{
	MultiReadIndex					t;
	unsigned int					numMultiReads = getNumMultiReads();
	const DataSourceMultiRead	*	multiRead;
	unsigned int					numReadsTotal = 0;

	for(t = 0; t < numMultiReads; t++)
	{
		if(multiRead = getMultiRead(t))
		{
			numReadsTotal += multiRead->getNumReads();
		}
	}

	return numReadsTotal;
}


DataSource::Data *DataSourceMultiReadSet::executeMultiReadSet(DataSource &dataSource, DataSource::Data *buffer)
{
	NumMultiReads						numMultiReads;
	MultiReadIndex						t;
	DataSourceMultiRead				*	multiRead; 
	DataSourceReadSet				*	readSet;
	Status								status;
	bool								bufferAllocated = false;
	DataSource::Data				*	dest;
															// Executes a local multi batch of read sets
	if((numMultiReads = getNumMultiReads()) == 0)
	{
		return NULL;
	}
															// If no buffer specified
	if(buffer == NULL)
	{
															// Create a new buffer
		if((buffer = new DataSource::Data[getTotalReadSize()]) == NULL)
			return NULL;

		bufferAllocated = true;
	}

	dest = buffer;


#ifdef PTRMI_LOGGING
PTRMI::Status::log(L"executeMultiReadSet Multi Reads: ", numMultiReads);
PTRMI::Status::log(L"executeMultiReadSet Total Reads: ", getNumReads());
PTRMI::Status::log(L"executeMultiReadSet Total Size : ", getTotalReadSize());
pt::SimpleTimer timer;
timer.start();
#endif


	try
	{
															// For each MultiRead (ReadSet)
		for(t = 0; t < numMultiReads; t++)
		{
															// Get the MultiRead
			if(multiRead = getMultiRead(t))
			{
															// Get MultiRead's ReadSet
				if(readSet = multiRead->getDataSourceReadSet())
				{			
#ifdef NEEDS_WORK_VORTEX_DGNDB 
												// Get the DataSourceServerInterface GUID from the MultiRead to know which DataSource to read ReadSet from
                    PTRMI::GUID	dataSourceServerInterfaceGUID = multiRead->getDataSourceServerInterface();

					if(dataSourceServerInterfaceGUID.isValidGUID())
					{
                        ServerInterfaceBase* dataSourceServerInterface;
															// Lock the Server Interface for use
						if((dataSourceServerInterface = getManager().getObjectManager().lockServerInterface(dataSourceServerInterfaceGUID)))
						{
                            DataSourceServer* dataSourceServer;

                            // Get the DataSource object represented by the ServerInterface
							if(dataSourceServer = reinterpret_cast<DataSourceServer *>(dataSourceServerInterface->getObject()))
							{
															// Execute the ReadSet on the data source
                                ptds::DataSize readSize = dataSourceServer->readBytesReadSet(dest, readSet);
															// Advance the destination buffer
								dest += readSize;
							}
							else
							{
								throw Status(Status::Status_Error_Failed_To_Find_Server_Interface);
							}
															// Release the ServerInterface
							getManager().getObjectManager().releaseServerInterface(dataSourceServerInterface);
						}
						else
						{
							throw Status(Status::Status_Error_Failed_To_Lock_Server_Interface);
						}
					}
					else
					{
						throw Status(Status::Status_Error_Server_Interface_Invalid);
					}
#endif
				}
				else
				{
					throw Status(Status::Status_Error_Failed_To_Find_ReadSet);
				}
			}
			else
			{
				throw Status(Status::Status_Error_Failed_To_Find_MultiRead);
			}
		}
	}
	catch(Status)
	{
															// If buffer was allocated locally, delete it
		if(bufferAllocated && buffer)
		{
			delete buffer;
		}
															// Return Read failed
		return NULL;
	}

#ifdef PTRMI_LOGGING
timer.stop();
pt::SimpleTimer::Time time = timer.getEllapsedTimeSeconds();
wchar_t message[256];
swprintf(message, L"%f", timer.getTimeSeconds());
PTRMI::Status::log(L"executeMultiReadSet Time (s)   : ", message);
#endif

	return buffer;
}


void DataSourceMultiReadSet::setEnabled(bool initEnabled)
{
	enabled = initEnabled;
}


bool DataSourceMultiReadSet::getEnabled(void)
{
	return enabled;
}




} // End ptds namespace
