#include "PointoolsVortexAPIInternal.h"

#include <ptds/DataSourceStructuredStorage.h>

#ifdef NEEDS_WORK_VORTEX_DGNDB
#include <PTRMI/Name.h>

namespace ptds
{

DataSourceStructuredStorage::PathStreamMap DataSourceStructuredStorage::pathStreamMap;


DataSourceStructuredStorage::DataSourceStructuredStorage(void)
{
	clear();
}


DataSourceStructuredStorage::~DataSourceStructuredStorage(void)
{
	close();
}


DataSourceStructuredStorage::DataSourceStructuredStorage(const FilePath *path, IStream *initStream, bool isClone)
{
	clear();

	setStream(initStream);

	setClone(isClone);

	if(isClone == false)
	{
		addExistingPathStream(*path, initStream);
	}
}


void DataSourceStructuredStorage::clear(void)
{
	setStream(NULL);
}


bool DataSourceStructuredStorage::getStreamFilename(const FilePath &filepath, std::wstring &filename)
{
	filename = filepath.filename();

	return true;
}


bool DataSourceStructuredStorage::openForRead(const FilePath *filepath, bool create)
{
	if(validHandle() == false)
		return false;

	unsigned int mode;

	if(getStreamMode(mode) == false)
		return false;

	if(mode != STGM_READ)
		return false;

	movePointerTo(0);

	filePath = (*filepath);
	
	canRead = true;

	setOpenState(DataSourceStateOpenForRead);


	return true;
}


bool DataSourceStructuredStorage::openForWrite(const FilePath *filepath, bool create)
{
	if(validHandle() == false)
		return false;

	unsigned int mode;

	if(getStreamMode(mode) == false)
		return false;

	if(mode != STGM_WRITE)
		return false;

	movePointerTo(0);

	filePath = (*filepath);

	canWrite = true;

	setOpenState(DataSourceStateOpenForWrite);


	return true;
}


bool DataSourceStructuredStorage::openForReadWrite(const FilePath *filepath, bool create)
{
	if(validHandle() == false)
		return false;

	unsigned int mode;

	if(getStreamMode(mode) == false)
		return false;

	if(mode != STGM_READWRITE)
		return false;

	movePointerTo(0);

	filePath = (*filepath);

	canRead = true;
	canWrite = true;

	setOpenState(DataSourceStateOpenForReadWrite);

	return true;
}


bool DataSourceStructuredStorage::validHandle(void)
{
	return (getStream() != NULL);
}


void DataSourceStructuredStorage::close(void)
{
															// If stream is defined
	if(validHandle())
	{
															// Commit any outstanding transactions
		getStream()->Commit(0);
															// If this is a cloned stream, it was created internally, so release it
		if(getClone())
		{
			getStream()->Release();
		}
		else
		{
															// Not a clone, so remove Path/Stream pair
//			if(getFilePath())
//				removeExistingPathStream(*getFilePath());
		}
															// No longer a valid handle
		clear();

		setOpenState(DataSourceStateClosed);
	}
}

bool DataSourceStructuredStorage::closeAndDelete(void)
{
	close();

	// Note: Don't delete as this stream is externally managed

	return true;
}


DataSource::Size DataSourceStructuredStorage::readBytes(Data *buffer, Size numberByesToRead)
{
	unsigned long	numberBytesRead = 0;

	if(validHandle())
	{
		HRESULT hres;

		beginRead(numberByesToRead);

		if(FAILED(hres = getStream()->Read(buffer, static_cast<ULONG>(numberByesToRead), &numberBytesRead)))
			return 0;

		endRead(numberByesToRead);
	}

	return static_cast<Size>(numberBytesRead);
}


DataSource::Size DataSourceStructuredStorage::writeBytes(const Data *buffer, Size numberBytesToWrite)
{
	unsigned long	numberBytesWritten = 0;

	if(validHandle())
	{
		if(FAILED(getStream()->Write(buffer, static_cast<ULONG>(numberBytesToWrite), &numberBytesWritten)))
			return 0;
	}

	return static_cast<Size>(numberBytesWritten);
}


DataSource::DataSize DataSourceStructuredStorage::getFileSize(void)
{
	IMalloc	*	pMalloc;
	STATSTG		statstg;

	//DataSize	size;

	if(FAILED(::CoGetMalloc(MEMCTX_TASK, &pMalloc)))
		return 0;

	if(validHandle())
	{
		if(FAILED(stream->Stat(&statstg, STATFLAG_DEFAULT)))
			return 0;
	}

	pMalloc->Free(statstg.pwcsName);

	return static_cast<DataSize>(statstg.cbSize.QuadPart);
};


bool DataSourceStructuredStorage::movePointerBy(DataPointer numberBytes)
{
//	Super::movePointerBy(numberBytes);
															// NOTE: Appears to be the same function
	return movePointerTo(numberBytes);
}


bool DataSourceStructuredStorage::movePointerTo(DataPointer numberBytes)
{
	LARGE_INTEGER	newPosition;
	ULARGE_INTEGER	newPositionRet;

	Super::movePointerTo(numberBytes);

	newPosition.QuadPart = numberBytes;

	if(validHandle())
	{
		if(SUCCEEDED(stream->Seek(newPosition, STREAM_SEEK_SET, &newPositionRet)))
			return true;
	}

	return false;
}


bool DataSourceStructuredStorage::getStreamMode(unsigned int &result)
{
	if(validHandle() == false)
	{
		result = 0;
		return false;
	}

	IMalloc	*	pMalloc;
	STATSTG		statstg;

	if(FAILED(::CoGetMalloc(MEMCTX_TASK, &pMalloc)))
		return false;

	if(FAILED(stream->Stat(&statstg, STATFLAG_DEFAULT)))
		return false;

	result = statstg.grfMode & 0x3;

	pMalloc->Free(statstg.pwcsName);

	return true;
}


DataSourcePtr DataSourceStructuredStorage::createNew(const FilePath *path)
{

	DataSource	*	dataSource = NULL;
	IStream		*	existingStream;
	IStream		*	stream;

	if(isValidPath(path) == false)
		return NULL;
															// If stream to given path exists already
	if((existingStream = getExistingPathStream(*path)) != NULL)
	{
															// Clone existing stream for thread safety
		if(FAILED(existingStream->Clone(&stream)))
			return NULL;
															// Create new data source based on new stream
		if((dataSource = new ptds::DataSourceStructuredStorage(path, stream, true)) == NULL)
			return NULL;
	}

	return dataSource;
}


bool DataSourceStructuredStorage::isValidPath(const FilePath *filepath)
{
	if(filepath == NULL)
		return false;

	PTRMI::URL	url = filepath->path();

	return url.isProtocol(PTRMI::URL::PT_PTSS);
}


bool DataSourceStructuredStorage::addExistingPathStream(const FilePath &path, IStream *initStream)
{
															// Check args
	if((path.path() == NULL) || (initStream == NULL))
		return false;
															// If we already have a different existing stream for this path, remove it so the new one can be added
	IStream* stream = getExistingPathStream(path);
	if(stream && (stream != initStream))
	{
		if (!removeExistingPathStream(path))
			return false;
	}
															// Add path/stream pair
	pathStreamMap[path.path()] = initStream;
															// Return OK
	return true;
}


bool DataSourceStructuredStorage::removeExistingPathStream(const FilePath &path)
{
	if(path.path() == NULL)
		return false;

	PathStreamMap::iterator i;
															// Find path entry
	i = pathStreamMap.find(path.path());
															// If found
	if(i != pathStreamMap.end())
	{
															// Delete entry
		pathStreamMap.erase(i);
															// Return OK
		return true;
	}
															// Return path not found
	return false;

}


IStream *DataSourceStructuredStorage::getExistingPathStream(const FilePath &path)
{
	PathStreamMap::iterator i;

	i = pathStreamMap.find(path.path());

	if(i != pathStreamMap.end())
		return i->second;

	return NULL;
}


void DataSourceStructuredStorage::setClone(bool isClone)
{
	clone = isClone;
}


bool DataSourceStructuredStorage::getClone(void)
{
	return clone;
}

DataSource::DataSourceType DataSourceStructuredStorage::getDataSourceType(void)
{
	return DataSourceTypeStructuredStorageExt;
}


HRESULT DataSourceStructuredStorage::openStructuredStorage(const FilePath &filepath, IStorage *parent, unsigned int mode, IStorage **result, bool create)
{
	HRESULT hres;

	unsigned int modeFull;

	switch(mode)
	{
	case STGM_READ:
		modeFull = STGM_READ | STGM_SHARE_DENY_WRITE;
		break;

	case STGM_WRITE:
		modeFull = STGM_WRITE | STGM_SHARE_EXCLUSIVE;

	case STGM_READWRITE:
		modeFull = STGM_READWRITE | STGM_SHARE_EXCLUSIVE;
	}

	if(parent == NULL)
	{
		hres = StgOpenStorageEx(filepath.path(), modeFull, STGFMT_STORAGE, 0, 0, NULL, IID_IStorage, (void **) result);

		if(hres == 0x80030002)
		{
			if(create && (mode == STGM_WRITE || mode == STGM_READWRITE))
			{
				hres = StgCreateStorageEx(filepath.path(), modeFull | STGM_CREATE, STGFMT_STORAGE, 0, 0, NULL, IID_IStorage, reinterpret_cast<void **>(result));
			}
		}
	}
	else
	{
		hres = parent->OpenStorage(filepath.path(), NULL, modeFull, NULL, 0, result);

		if(hres == 0x80030002)
		{
			if(create && (mode == STGM_WRITE || mode == STGM_READWRITE))
			{
				hres = parent->CreateStorage(filepath.path(), modeFull | STGM_CREATE, 0, 0, result);
			}
		}
	}

	return hres;
}


HRESULT DataSourceStructuredStorage::openStructuredStorageStream(const FilePath &filepath, IStorage *parent, unsigned int mode, IStream **result, bool create)
{
	if(parent == NULL)
		return S_FALSE;

	HRESULT			error;
	unsigned int	modeFull;

	switch(mode)
	{
	case STGM_READ:
		modeFull = STGM_READ | STGM_SHARE_DENY_WRITE;
		break;

	case STGM_WRITE:
		modeFull = STGM_WRITE | STGM_SHARE_EXCLUSIVE;
		break;

	case STGM_READWRITE:
		modeFull = STGM_READWRITE | STGM_SHARE_EXCLUSIVE;
		break;
	}


	if(FAILED(error = parent->OpenStream(filepath.path(), 0, modeFull, 0, result)))
	{
		if(create && (mode == STGM_WRITE || mode == STGM_READWRITE))
		{
			if(FAILED(error = parent->CreateStream(filepath.path(), modeFull | STGM_CREATE, 0, 0, result)))
			{
				return error;
			}
		}
		else
		{
			return error;
		}
	}

	return error;
}


void DataSourceStructuredStorage::destroy(void)
{
	delete this;
}


DataSource::DataSourceForm DataSourceStructuredStorage::getDataSourceForm(void)
{
	return DataSource::DataSourceFormLocal;
}



} // End ptds namespace

#endif