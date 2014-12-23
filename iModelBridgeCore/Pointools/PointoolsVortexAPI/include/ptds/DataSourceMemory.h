#pragma once

#include <ptds/DataSource.h>
#include <PTRMI/DataBuffer.h>


namespace ptds
{

class DataSourceMemory : public DataSource
{
protected:

	PTRMI::DataBuffer		dataBuffer;

public:

							DataSourceMemory	(void);
						   ~DataSourceMemory	(void);

	DataSourceForm			getDataSourceForm	(void);

	static DataSourcePtr	createNew			(const FilePath *path, DataSource::Data *sourceBuffer = NULL, DataSource::DataSize sourceBufferSize = 0);

	void					destroy				(void);

	bool					openForRead			(const FilePath *filepath, bool create = false);
	bool					openForWrite		(const FilePath *filepath, bool create = false);
	bool					openForReadWrite	(const FilePath *filepath, bool create = false);

	bool					validHandle			(void);
	static bool				isValidPath			(const FilePath *filepath);

	void					close				(void);
	bool					closeAndDelete		(void);

	Size					readBytes			(Data *buffer, Size number_bytes);
	Size					writeBytes			(const Data *buffer, Size number_bytes);

	Size					readBytesFrom		(Data *buffer, DataPointer position, Size numBytes);

	DataSize				getFileSize			(void);

	bool					movePointerBy		(DataPointer number_bytes);
	bool					movePointerTo		(DataPointer number_bytes);

	Data				*	getBuffer			(DataSize &bufferSize);
};


} // End ptds namespace