#pragma once

#include <ptds/DataSourceFile.h>

#include <PTRMI/ClientInterface.h>
#include <PTRMI/ServerInterface.h>
#include <PTRMI/Mutex.h>
#include <ptds/DataSourceReadSet.h>
#include <PTRMI/Manager.h>
#include <map>

#define DATA_SOURCE_CLASS_SERVER						L"DataSourceServer"

#define DATA_SOURCE_METHOD_OPEN_FOR_READ				L"openForRead"
#define DATA_SOURCE_METHOD_OPEN_FOR_WRITE				L"openForWrite"
#define DATA_SOURCE_METHOD_OPEN_FOR_READ_WRITE			L"openForReadWrite"
#define	DATA_SOURCE_METHOD_CLOSE						L"close"
#define DATA_SOURCE_METHOD_CLOSE_AND_DELETE				L"closeAndDelete"
#define DATA_SOURCE_METHOD_READ_BYTES					L"readBytes"
#define DATA_SOURCE_METHOD_READ_BYTES_FROM				L"readBytesFrom"
#define DATA_SOURCE_METHOD_READ_BYTES_READ_SET			L"readBytesReadSet"
#define	DATA_SOURCE_METHOD_READ_BYTES_MULTI_READ_SET	L"readBytesMultiReadSet"
#define DATA_SOURCE_METHOD_WRITE_BYTES					L"writeBytes"
#define DATA_SOURCE_METHOD_SET_FILE_PATH				L"setFilePath"
#define DATA_SOURCE_METHOD_VALID_HANDLE					L"validHandle"
#define DATA_SOURCE_METHOD_GET_FILE_SIZE				L"getFileSize"
#define DATA_SOURCE_METHOD_MOVE_POINTER_BY				L"movePointerBy"
#define DATA_SOURCE_METHOD_MOVE_POINTER_TO				L"movePointerTo"


namespace ptds
{

class DataSourceServer : public DataSourceFile
{

public:

	typedef DataSourceFile	Super;

protected:

	HANDLE					handle;

protected:

	bool					getURLFilePath		(const ptds::FilePath &url, ptds::FilePath &result);

public:

							DataSourceServer	(void) {}

//	static DataSourcePtr	createNew			(const FilePath *path);
	static DataSourcePtr	createNew			(const FilePath *path, const ptds::FilePath *clientFile);
	static bool				isValidPath			(const FilePath *filepath);

	bool					openForRead			(const FilePath *filepath, bool create = false);
	bool					openForWrite		(const FilePath *filepath, bool create = false);
	bool					openForReadWrite	(const FilePath *filepath, bool create = false);

	void					getHostName			(PTRMI::Name &hostName);


/*
	static DataSourceType	getDataSourceType	(void);

	bool					openForRead			(const FilePath &filepath, bool create = false);
	bool					openForWrite		(const FilePath &filepath, bool create = false);
	bool					openForReadWrite	(const FilePath &filepath, bool create = false);

	bool					validHandle			(void);
	static bool				isValidPath			(const FilePath &filepath);

	void					close				(void);
	bool					closeAndDelete		(void);

	unsigned int			readBytes			(void* lpBuffer, unsigned int number_bytes);
	unsigned int			writeBytes			(const void* lpBuffer, unsigned int number_bytes);

	DataSize				getFileSize			(void);

	bool					movePointerBy		(DataPointer number_bytes);
	bool					movePointerTo		(DataPointer number_bytes);
*/
};


// **************************************************************************************************
//
// **************************************************************************************************

using namespace PTRMI;

PTRMI_DECLARE_CLIENT_INTERFACE(DataSourceServerClientInterface), public DataSource
{

public:

	typedef DataSource								Super;

	typedef std::map<std::wstring, std::wstring>	PathPathMap;

protected:

	static PathPathMap			pathPathMap;
	static PTRMI::Mutex			pathPathMapMutex;

	URL							clientFilePath;

public:

								DataSourceServerClientInterface			(void);

	DataSourceForm				getDataSourceForm						(void);

	void						getHostName								(PTRMI::Name &hostName);

	static const bool			addClientSideFilePath					(const std::wstring &serverFilePath, const std::wstring &clientFilePath);
	static const std::wstring *	getClientSideFilePath					(const std::wstring &serverFilePath);
	static bool					removeClientSideFilePath				(const std::wstring &serverFilePath);

	static DataSourcePtr		createNew								(const ptds::FilePath *serverPath);

	static bool					isValidPath								(const FilePath *filepath);
	bool						validHandle								(void);

	void						setClientFilePath						(const URL &filePath);
	const URL &					getClientFilePath						(void);

	bool 						movePointerBy							(DataPointer numBytes);

	bool 						movePointerTo							(DataPointer position);

	Size 						readBytes								(Data *buffer, Size numBytes);
	Size 						writeBytes								(const Data *buffer, Size numBytes);

	Size 						readBytesFrom							(Data *buffer, DataPointer position, Size numBytes);
	Size 						readBytesReadSet						(Data *buffer, DataSourceReadSet *readSet);
	Size						readBytesMultiReadSet					(Data *buffer, DataSourceMultiReadSet *multiReadSet);

	Status						recoverRemoteObject						(void);

	// -------------------------

	void						setFilePath								(const FilePath *path)										{sendAutoVoid<PC<In<const FilePath *>>>(DATA_SOURCE_METHOD_SET_FILE_PATH, path);}

	bool						openForRead								(const FilePath *path, bool create = false);
	bool						openForWrite							(const FilePath *path, bool create = false);
	bool						openForReadWrite						(const FilePath *path, bool create = false);
	void						close									(void);
	bool						closeAndDelete							(void);

//	Size						writeBytesFrom							(const Data *buffer, DataPointer position, Size numBytes)	{return sendAuto<Size, PC<In<Array<const Data>>>, PC<In<DataPointer>>, PC<In<Size>> >(0, L"writeBytesFrom", Array<const Data>(numBytes, reinterpret_cast<const Data *>(buffer)), position, numBytes);}

	int64_t						getFileSize								(void)														{return sendAuto<int64_t>(0, DATA_SOURCE_METHOD_GET_FILE_SIZE);}

	void						destroy									(void);
//	DataSourceType				getPathDataSourceType					(const ptds::FilePath *path)								{return DataSourceTypeServer;}


};


// **************************************************************************************************
//
// **************************************************************************************************


PTRMI_DECLARE_SERVER_INTERFACE(DataSourceServerServerInterface, Obj)
{

protected:

	typedef DataSource::Size	Size;

public:

	DataSourceServerServerInterface(void)
	{
		addServerMethod(PTRMI::Name(DATA_SOURCE_METHOD_SET_FILE_PATH), &Super::receiveAutoVoid<PS<In<const ptds::FilePath *>>, &DataSource::setFilePath>);

		addServerMethod(PTRMI::Name(DATA_SOURCE_METHOD_OPEN_FOR_READ), &Super::receiveAuto<bool, PS<In<const ptds::FilePath *>>, PS<In<bool>>, &DataSource::openForRead>);
		addServerMethod(PTRMI::Name(DATA_SOURCE_METHOD_OPEN_FOR_WRITE), &Super::receiveAuto<bool, PS<In<const ptds::FilePath *>>, PS<In<bool>>, &DataSource::openForWrite>);
		addServerMethod(PTRMI::Name(DATA_SOURCE_METHOD_OPEN_FOR_READ_WRITE), &Super::receiveAuto<bool, PS<In<const ptds::FilePath *>>, PS<In<bool>>, &DataSource::openForReadWrite>);

		addServerMethod(PTRMI::Name(DATA_SOURCE_METHOD_VALID_HANDLE), &Super::receiveAuto<bool, &DataSource::validHandle>);

		addServerMethod(PTRMI::Name(DATA_SOURCE_METHOD_CLOSE), &Super::receiveAutoVoid<&DataSource::close>);
		addServerMethod(PTRMI::Name(DATA_SOURCE_METHOD_CLOSE_AND_DELETE), &Super::receiveAuto<bool, &DataSource::closeAndDelete>);

		addServerMethod(PTRMI::Name(DATA_SOURCE_METHOD_READ_BYTES), &Super::receiveAuto<Size, PS<Out<ArrayDirect<>>>, PS<In<Size>>, &DataSource::readBytes>);
//		addServerMethod(PTRMI::Name(DATA_SOURCE_METHOD_WRITE_BYTES), &Super::receiveAuto<Size, PS<In<Array<>>>, PS<In<Size>>, &DataSource::writeBytes>);

		addServerMethod(PTRMI::Name(DATA_SOURCE_METHOD_READ_BYTES_FROM), &Super::receiveAuto<Size, PS<Out<ArrayDirect<>>>, PS<In<DataPointer>>, PS<In<Size>>, &DataSource::readBytesFrom>);
//		addServerMethod(PTRMI::Name(L"writeBytesT"), &Super::receiveAuto<Size, PS<In<Array<>>>, PS<In<Size>>, &DataSource::writeBytesT>);

		addServerMethod(PTRMI::Name(DATA_SOURCE_METHOD_READ_BYTES_READ_SET), &Super::receiveAuto<Size, PS<Out<ArrayDirect<>>>, PS<In<DataSourceReadSet *>>, &DataSource::readBytesReadSet>);
		addServerMethod(PTRMI::Name(DATA_SOURCE_METHOD_READ_BYTES_MULTI_READ_SET), &Super::receiveAuto<Size, PS<Out<ArrayDirect<>>>, PS<In<DataSourceMultiReadSet *>>, &DataSource::readBytesMultiReadSet>);

		addServerMethod(PTRMI::Name(DATA_SOURCE_METHOD_GET_FILE_SIZE), &Super::receiveAuto<DataSize, &DataSource::getFileSize>);

		addServerMethod(PTRMI::Name(DATA_SOURCE_METHOD_MOVE_POINTER_BY), &Super::receiveAuto<bool, PS<In<DataPointer>>, &DataSource::movePointerBy>);

		addServerMethod(PTRMI::Name(DATA_SOURCE_METHOD_MOVE_POINTER_TO), &Super::receiveAuto<bool, PS<In<DataPointer>>, &DataSource::movePointerTo>);

	}

	DataSourceServer *getDataSource(void)
	{
		return reinterpret_cast<DataSourceServer *>(getObject());
	}

};


} // End ptds namespace

PTRMI_USE_CLIENT_INTERFACE(ptds::DataSourceServerClientInterface, ptds::DataSourceServer)

PTRMI_USE_SERVER_INTERFACE(ptds::DataSourceServerServerInterface<ptds::DataSource>, ptds::DataSource)
PTRMI_INHERIT_SERVER_INTERFACE(ptds::DataSourceServerServerInterface<ptds::DataSource>, ptds::DataSourceServer)


