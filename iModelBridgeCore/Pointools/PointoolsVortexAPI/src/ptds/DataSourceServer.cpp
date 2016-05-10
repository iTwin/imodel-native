#include "PointoolsVortexAPIInternal.h"

#ifdef NEEDS_WORK_VORTEX_DGNDB 
#include <ptds/DataSourceServer.h>
#include <ptds/DataSourceManager.h>

#include <PTRMI/Manager.h>

#include <PTRMI/ClientInterfaceExtDataBentley.h>

#include <ptds/DataSourceMultiReadSet.h>

namespace ptds
{

DataSourceServerClientInterface::PathPathMap	DataSourceServerClientInterface::pathPathMap;
PTRMI::Mutex									DataSourceServerClientInterface::pathPathMapMutex;




DataSourceServerClientInterface::DataSourceServerClientInterface()
{
															// Enable read sets by default for this DataSource type
	setReadSetEnabled(true);
}


DataSourcePtr DataSourceServer::createNew(const FilePath *path, const ptds::FilePath *clientFile)
{
	PTRMI::Name							className(DATA_SOURCE_CLASS_SERVER);
	PTRMI::Name							dataSourceServerName;
	Status								status;

	if(isValidPath(path) == false)
		return NULL;
															// Create remote DataSourceServer object
	return ptds::dataSourceManager.createRemoteDataSourceServer(clientFile->path(), path->path());
}


bool DataSourceServer::isValidPath(const FilePath *filepath)
{
	if(filepath == NULL)
		return false;

	URL url = filepath->path();

	if(url.isValidURL() == false)
		return false;

	PTRMI::URL	protocol;

	if(url.getProtocol(protocol) == false)
		return false;

	return (protocol == PTRMI::URL::PT_PTRE || protocol == PTRMI::URL::PT_PTRI);
}


bool DataSourceServer::getURLFilePath(const ptds::FilePath &url, ptds::FilePath &result)
{
	PTRMI::URL	URL;
	PTRMI::URL	filepath;
															// Get object part of URL, which should be the file path
	URL = url.path();
	URL.getObject(filepath);

	result = filepath.getString().c_str();

	return true;
}

bool DataSourceServer::openForRead(const FilePath *path, bool create)
{
	FilePath	filepath;

	if(path == NULL)
		return false;

	getURLFilePath(*path, filepath);

	return DataSourceFile::openForRead(&filepath, create);
}


bool DataSourceServer::openForWrite(const FilePath *path, bool create)
{
	FilePath	filepath;

	if(path == NULL)
		return false;

	getURLFilePath(*path, filepath);

	return DataSourceFile::openForWrite(&filepath, create);
}


bool DataSourceServer::openForReadWrite(const FilePath *path, bool create)
{
	FilePath	filepath;

	if(path == NULL)
		return false;

	getURLFilePath(*path, filepath);

	return DataSourceFile::openForReadWrite(&filepath, create);
}


void DataSourceServer::getHostName(PTRMI::Name &hostName)
{
															// Handled by Interface, so return invalid GUID
	hostName.clear();
}


void DataSourceServerClientInterface::destroy(void)
{
	RemotePtr<ptds::DataSourceServer> thisObject(this);
															// Discard remote object and delete the local client object (which is this DataSource)
	PTRMI::getManager().discardRemoteObject(thisObject, this->getExtData());
}


Status DataSourceServerClientInterface::recoverRemoteObject(void)
{
	PTRMI::RemotePtr<ptds::DataSourceServer>	thisPtr(this);
	Status										status;
	bool										opened = false;
	DataSourceOpenState							openState;
	Status::Error								statusError;

															// Request that Manager attempts to recover the remote object
	if((status = PTRMI::getManager().recoverRemoteObject<ptds::DataSourceServer>(thisPtr)).isFailed())
	{
															// If not recovered, return status
		return status;
	}
															// Reproduce the current file open state
	switch(openState = getOpenState())
	{
	case DataSourceStateOpenForRead:
		opened		= openForRead(getFilePath(), false);
		statusError = Status::Status_Error_File_Open_For_Read;
		break;

	case DataSourceStateOpenForWrite:
		opened = openForWrite(getFilePath(), false);
		statusError = Status::Status_Error_File_Open_For_Write;
		break;

	case DataSourceStateOpenForReadWrite:
		opened = openForReadWrite(getFilePath(), false);
		statusError = Status::Status_Error_File_Open_For_Read_Write;
		break;

	case DataSourceStateClosed:
		default: ;
	}
															// If being opened
	if(openState != DataSourceStateClosed)
	{
															// If open was successful
		if(opened)
		{
															// Move pointer to current position (Note: Due to DataSourceServerClientInterface optimizations, this is not likely needed
			movePointerTo(getDataPointer());
		}
		else
		{
															// Failed, so set the relevant error status
			status.set(statusError);
		}
	}

															// Return status
	return status;		
}


ptds::DataSource::DataSourceForm DataSourceServerClientInterface::getDataSourceForm(void)
{
	return DataSource::DataSourceFormRemote;
}


void DataSourceServerClientInterface::getHostName(PTRMI::Name &hostName)
{
	hostName.clear();
															// Get the remote Host's Manager GUID from the ClientInterface
	hostName = PTRMI::InterfaceBase::getHostGUID();
}


const bool DataSourceServerClientInterface::addClientSideFilePath(const std::wstring &serverFilePath, const std::wstring &clientFilePath)
{
	removeClientSideFilePath(serverFilePath);

    std::lock_guard<std::recursive_mutex> mutexScope(pathPathMapMutex);

	pathPathMap[serverFilePath] = clientFilePath;

	return true;
}


const std::wstring *DataSourceServerClientInterface::getClientSideFilePath(const std::wstring &serverFilePath)
{
	PathPathMap::iterator it;

    std::lock_guard<std::recursive_mutex> mutexScope(pathPathMapMutex);

	if((it = pathPathMap.find(serverFilePath)) != pathPathMap.end())
	{
		return &(it->second);
	}

	return NULL;
}


bool DataSourceServerClientInterface::removeClientSideFilePath(const std::wstring &serverFilePath)
{
	PathPathMap::iterator it;

    std::lock_guard<std::recursive_mutex> mutexScope(pathPathMapMutex);

	if((it = pathPathMap.find(serverFilePath)) != pathPathMap.end())
	{
		pathPathMap.erase(it);
		return true;
	}

	return false;
}


ptds::DataSourcePtr DataSourceServerClientInterface::createNew(const ptds::FilePath *serverPath)
{
	if(serverPath == NULL)
	{
		return NULL;
	}
															// Look up client side file path
	const std::wstring *clientSideFilepath = getClientSideFilePath(serverPath->path());

															// If a client side path is found
	if(clientSideFilepath)
	{

		const FilePath fp(clientSideFilepath->c_str());
															// Call with client side file usage
		return DataSourceServer::createNew(serverPath, &fp);
	}
	else
	{
		Status status(Status::Status_Warning_Failed_To_Get_Client_Side_File_Path);
	}

															// Call without client side file usage
	return DataSourceServer::createNew(serverPath, NULL);
}


bool DataSourceServerClientInterface::isValidPath(const FilePath *filepath)
{
	return DataSourceServer::isValidPath(filepath);
}


bool DataSourceServerClientInterface::validHandle(void)
{
															// NOTE: This does not map over the network for performance reasons

															// If this data source has already failed, fail immediately
	if(getStatus().isFailed())
		return false;
															// Return OK
	return true;
															// If OK, do RMI call
															//		return sendAuto<bool>(false, L"validHandle");
}


void DataSourceServerClientInterface::setClientFilePath(const URL &filePath)
{
	clientFilePath = filePath;
}


const URL & DataSourceServerClientInterface::getClientFilePath(void)
{
	return clientFilePath;
}


bool DataSourceServerClientInterface::movePointerBy(DataPointer numBytes)
{
															// Move locally but don't send move message
	return Super::movePointerBy(numBytes);
}


bool DataSourceServerClientInterface::movePointerTo(DataPointer position)
{
															// Move locally but don't send move message
	return Super::movePointerTo(position);
}


DataSource::Size DataSourceServerClientInterface::readBytes(Data *buffer, Size numBytes)
{
															// Map readBytes to readBytesFrom to reduce bandwidth
	return readBytesFrom(buffer, getDataPointer(), numBytes);

//	return sendAuto<Size, PC<Out<ArrayDirect<>>>, PC<In<Size>> >(0, DATA_SOURCE_METHOD_READ_BYTES, ArrayDirect<>(numBytes, reinterpret_cast<Data *>(buffer)), numBytes);
}


DataSource::Size DataSourceServerClientInterface::writeBytes(const Data *buffer, Size numBytes)
{
	return sendAuto<Size, PC<In<Array<const Data>>>, PC<In<Size>> >(0, DATA_SOURCE_METHOD_WRITE_BYTES, 
                                                                    Array<const Data>(numBytes, reinterpret_cast<const Data *>(buffer)), 
                                                                    numBytes);
}


DataSource::Size DataSourceServerClientInterface::readBytesFrom(Data *buffer, DataPointer position, Size numBytes)
{
	Size numRead;
															// If a read set is defined
	if(getReadSetEnabled() && isReadSetDefined())
	{
															// Add item to the read set
		numRead = addReadSetItem(buffer, getDataPointer(), numBytes);
	}
	else
	{
															// Otherwise, do a Standard RMI read
		beginRead(numBytes);
															// Read data from givne position
		numRead = sendAuto<Size, PC<Out<ArrayDirect<>>>, PC<In<DataPointer>>, PC<In<Size>> >(0, 
                                        DATA_SOURCE_METHOD_READ_BYTES_FROM, ArrayDirect<>(numBytes, reinterpret_cast<Data *>(buffer)), 
                                        position, numBytes);

		endRead(numBytes);
	}
															// Advance local read pointer
	setDataPointer(position + numRead);

															// Return number of bytes read
	return numRead;
}


DataSource::Size DataSourceServerClientInterface::readBytesReadSet(Data *buffer, DataSourceReadSet *readSet)
{
	Size numRead;
	Size totalReadSize;
															// Make sure buffer and read set are defined
	if(buffer == NULL || readSet == NULL)
		return 0;
															// Make sure there's data to be read		
	if((totalReadSize = readSet->getTotalReadSize()) == 0)
		return 0;
															// Signal start of reading
	beginRead(totalReadSize);
															// Read all ReadSet reads into the result buffer
	numRead = sendAuto<Size, PC<Out<ArrayDirect<>>>, PC<In<DataSourceReadSet *>> >(0, DATA_SOURCE_METHOD_READ_BYTES_READ_SET, ArrayDirect<>(totalReadSize, reinterpret_cast<Data *>(buffer)), readSet);
															// Signal end of reading
	endRead(totalReadSize);

															// If data was read
	if(numRead > 0)
	{
															// Set data pointer to end of the final read
		setDataPointer(readSet->getFinalDataPointer());
	}
															// Return total number of bytes read by all reads
	return numRead;
}


DataSource::Size DataSourceServerClientInterface::readBytesMultiReadSet(Data *buffer, DataSourceMultiReadSet *multiReadSet)
{
	Size numRead;
	Size totalReadSize;
															// Make sure buffer and read set are defined
	if(buffer == NULL || multiReadSet == NULL)
		return 0;
															// Make sure there's data to be read		
	if((totalReadSize = multiReadSet->getTotalReadSize()) == 0)
		return 0;

															// Read all ReadSet reads into the result buffer
	numRead = sendAuto<Size, PC<Out<ArrayDirect<>>>, PC<In<DataSourceMultiReadSet *>> >(0, DATA_SOURCE_METHOD_READ_BYTES_MULTI_READ_SET, ArrayDirect<>(totalReadSize, reinterpret_cast<Data *>(buffer)), multiReadSet);
															// Signal end of reading
															// If this call failed
	if(getStatus().isFailed())
	{
															// If failure was due to the MultiReadSet method not being found
		if(getStatus().is(Status::Status_Server_Class_Method_Not_Found))
		{
															// Disable the MultiReadSet for this StreamHost
			multiReadSet->setEnabled(false);
															// Reset status so that this DataSource can continue to be used
			resetStatus();
		}
	}
															// Return total number of bytes read by all reads
	return numRead;
}


bool DataSourceServerClientInterface::openForRead(const FilePath *path, bool create)
{
	if(sendAuto<bool, PC<In<const FilePath *>>, PC<In<bool>>>(false, DATA_SOURCE_METHOD_OPEN_FOR_READ, path, create))
	{
		setOpenState(DataSourceStateOpenForRead);

		resetDataPointer();

		DataSource::setFilePath(path);

		return true;
	}

	return false;
}


bool DataSourceServerClientInterface::openForWrite(const FilePath *path, bool create)
{
	if(sendAuto<bool, PC<In<const FilePath *>>, PC<In<bool>>>(false, DATA_SOURCE_METHOD_OPEN_FOR_WRITE, path, create))
	{
		setOpenState(DataSourceStateOpenForWrite);
		resetDataPointer();
		return true;
	}

	return false;
}


bool DataSourceServerClientInterface::openForReadWrite(const FilePath *path, bool create)
{
	if(sendAuto<bool, PC<In<const FilePath *>>, PC<In<bool>>>(false, DATA_SOURCE_METHOD_OPEN_FOR_READ_WRITE, path, create))
	{
		setOpenState(DataSourceStateOpenForReadWrite);
		resetDataPointer();
		return true;
	}

	return false;
}

void DataSourceServerClientInterface::close(void)
{
	sendAutoVoid(DATA_SOURCE_METHOD_CLOSE);

	setOpenState(DataSourceStateClosed);

	resetDataPointer();
}


bool DataSourceServerClientInterface::closeAndDelete(void)
{
	if(sendAuto<bool>(false, DATA_SOURCE_METHOD_CLOSE_AND_DELETE))
	{
		setOpenState(DataSourceStateClosed);

		setOpenState(DataSourceStateClosed);

		resetDataPointer();

		return true;
	}

	return false;
}



} // End ptds namespace

#endif