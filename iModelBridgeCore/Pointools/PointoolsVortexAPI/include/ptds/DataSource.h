#pragma once

#include <ptds/FilePath.h>
#include <pt/ptstring.h>

namespace PTRMI
{
	class URL;

	class Name;
}


#define DATA_SOURCE_DEFAULT_DATA_POINTER	0


namespace ptds
{

class DataSourceReadSet;
class DataSourceMultiReadSet;

class DataSource
{

public:

	typedef int64_t			DataPointer;
	typedef int64_t			DataSize;

	typedef unsigned char	Data;
	typedef DataSize		Size;

	enum ReadMode
	{
		ReadModeNormal,
		ReadModeReadSet,

		ReadModeCount
	};


public:

	enum DataSourceType
	{
		DataSourceTypeNull = 0,

		DataSourceTypeFile,
		DataSourceTypeCache,
		DataSourceTypeStructuredStorage,
		DataSourceTypeStructuredStorageExt,
		DataSourceTypeServer,
		DataSourceTypeMemBlock,

		DataSourceTypeCount
	};

	enum DataSourceForm
	{
		DataSourceFormNULL,

		DataSourceFormLocal,
		DataSourceFormRemote,

		DataSourceFormCount
	};

	enum DataSourceOpenState
	{
		DataSourceStateClosed,
		DataSourceStateOpenForRead,
		DataSourceStateOpenForWrite,
		DataSourceStateOpenForReadWrite,

		DataSourceOpenCount
	};

protected:

	FilePath				filePath;

	ReadMode				readMode;

	DataSourceOpenState		openState;


protected:

	DataPointer				dataPointer;

	DataSourceReadSet	*	readSet;
	void				*	readSetClientID;
	bool					readSetEnabled;
	bool					canWrite;
	bool					canRead;

protected:

	void					setOpenState				(DataSourceOpenState state);
	DataSourceOpenState		getOpenState				(void);

	void					setDataPointer				(DataPointer pointer);
	void					resetDataPointer			(void);
	void					advanceDataPointer			(DataSize numBytes);

	unsigned int			getBudgetParallelReadNormal	(DataSize budget, DataSourceReadSet &readSet, DataSize &budgetUsed);

	void					setReadSet					(DataSourceReadSet *readSet);
	DataSourceReadSet	*	getReadSet					(void);

	void					setReadSetClientID			(void *id);

#ifndef NO_DATA_SOURCE_SERVER
	DataSource::Size		addReadSetItem				(Data *buffer, DataPointer position, Size numBytes);
#endif

public:

							DataSource					(void);
	virtual				   ~DataSource					(void) {}

	virtual void			destroy						(void) = 0;

	virtual DataSourceForm	getDataSourceForm			(void) = 0;

			void			setFilePath					(const FilePath *path);
	const	FilePath	*	getFilePath					(void);
	virtual void			getURL						(PTRMI::URL &url);

	virtual void			getHostName					(PTRMI::Name &hostName);

	virtual	bool			openForRead					(const FilePath *path, bool create = false) = 0;
	virtual	bool			openForWrite				(const FilePath *path, bool create = false) = 0;
	virtual	bool			openForReadWrite			(const FilePath *path, bool create = false) = 0;

	virtual bool			validHandle					(void) = 0;

	virtual void			close						(void) = 0;
	virtual bool			closeAndDelete				(void) = 0;

	virtual void			beginRead					(Size numBytes);
	virtual void			endRead						(Size numBytes);

	virtual bool			beginReadSet				(DataSourceReadSet *readSet);
	virtual bool			endReadSet					(DataSourceReadSet *readSet);
	virtual void			beginReadSetClientID		(void *clientID);
	virtual void			endReadSetClientID			(void);
	virtual void		*	getReadSetClientID			(void);
	virtual void			clearReadSet				(void);

#ifndef NO_DATA_SOURCE_SERVER
	virtual DataSize		getReadSetTotalSize			(void);
#endif

	virtual Size			readBytes					(Data *buffer, Size numBytes) = 0;
	virtual Size			writeBytes					(const Data *buffer, Size numBytes) = 0;

	virtual Size			readBytesFrom				(Data * /*buffer*/, DataPointer /*position*/, Size /*numBytes*/) {return 0;}
	virtual Size			writeBytesFrom				(const Data * /*buffer*/, DataPointer /*position*/, Size /*numBytes*/) {return 0;}

#ifndef NO_DATA_SOURCE_SERVER
	virtual Size			readBytesReadSet			(Data *buffer, DataSourceReadSet *readSet);
	virtual Size			readBytesMultiReadSet		(Data *buffer, DataSourceMultiReadSet *multiReadSet);
#endif
	virtual bool			moveFile					(const FilePath *path, const FilePath *newPath);

	virtual DataSize		getFileSize					(void) = 0;

	virtual bool			movePointerBy				(DataPointer numBytes);
	virtual bool			movePointerTo				(DataPointer position);

	DataSourceType			getPathDataSourceType		(const ptds::FilePath *path);

	virtual DataPointer		getDataPointer				(void);

#ifndef NO_DATA_SOURCE_SERVER
	virtual unsigned int	getBudgetParallelRead		(DataSize budget, DataSourceReadSet &readSet, DataSize &budgetUsed);
#endif

	void					setReadSetEnabled			(bool enabled);
	bool					getReadSetEnabled			(void);

	bool					isReadWrite					(void) const;
	bool					isReadOnly					(void) const;
	bool					isWrite						(void) const;

															// void * buffer versions
	Size readBytes(void *buffer, Size number_bytes)
	{
		return readBytes(reinterpret_cast<Data *>(buffer), number_bytes);
	}

	Size writeBytes(const void *buffer, Size number_bytes)
	{
		return writeBytes(reinterpret_cast<const Data *>(buffer), number_bytes);
	}

	template<typename T> Size readBytes(T &buffer)
	{
		return readBytes(reinterpret_cast<Data *>(&buffer), sizeof(T));
	}

	template<typename T> Size writeBytes(const T &buffer)
	{
		return writeBytes(reinterpret_cast<const Data *>(&buffer), sizeof(T));
	}

	Size readString( pt::String &string );

	Size writeString( const pt::String &string );

	bool isReadSetDefined(void)
	{
		return getReadSet() != NULL;
	}

	virtual bool isCached(void)
	{
		return false;
	}
};

typedef DataSource *	DataSourcePtr;

}
