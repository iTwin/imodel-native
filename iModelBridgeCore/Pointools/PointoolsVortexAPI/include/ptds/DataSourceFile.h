#pragma once

#include <ptds/DataSource.h>

namespace ptds
{

class DataSourceFile : public DataSource
{

public:

	typedef DataSource		Super;

protected:

	HANDLE					handle;

protected:

	void					invalidateHandle	(void);


public:

							DataSourceFile		(void);
						   ~DataSourceFile		(void);

	DataSourceForm			getDataSourceForm	(void);

	static DataSourcePtr	createNew			(const FilePath *path);

	void					destroy				(void);

	void					getHostName			(PTRMI::Name &hostName);

	static DataSourceType	getDataSourceType	(void);

	bool					openForRead			(const FilePath *filepath, bool create = false);
	bool					openForWrite		(const FilePath *filepath, bool create = false);
	bool					openForReadWrite	(const FilePath *filepath, bool create = false);

	void					getURL				(PTRMI::URL &url);

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
};

} // End ptds namespace


