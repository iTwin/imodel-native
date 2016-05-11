#pragma once

#include <pt/memrw.h>
#include <ptds/DataSource.h>

namespace ptds
{

	class DataSourceMemBlock : public DataSource
	{

	public:

		typedef DataSource		Super;

	protected:

		bool					openForRead			(const Data*buffer);
		bool					openForWrite		(void);

	public:

		DataSourceMemBlock		(void);

		DataSourceForm			getDataSourceForm	(void);

		static DataSourcePtr	createNew			(const Data *buffer);		// read mode
		static DataSourcePtr	createNew			(void);					// write mode

		void					destroy				(void);

		static DataSourceType	getDataSourceType	(void);

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

		void *					getWriteBuffer		(void);

	private:

		MemRW					m_mem;
		int						m_readPos;
	};

} // End ptds namespace


