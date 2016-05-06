#include "PointoolsVortexAPIInternal.h"
#include <iostream>

#include <ptds/DataSourceMemBlock.h>
#include <PTRMI/URL.h>

#ifndef NO_DATA_SOURCE_SERVER
#include <PTRMI/Status.h>
#endif

#define START_MEM_BLOCK_SIZE (1024*1024)

namespace ptds
{

	DataSourceMemBlock::DataSourceMemBlock(void)
	{
		handle = NULL;
		m_readPos = 0;
	}

	bool DataSourceMemBlock::openForRead(const FilePath *filepath, bool create)
	{
		return false;	// not supported
	}

	bool DataSourceMemBlock::openForWrite(const FilePath *filepath, bool create)
	{
		return false;
	}

	bool DataSourceMemBlock::openForWrite( void )
	{
		bool success = m_mem.allocate( START_MEM_BLOCK_SIZE );
		if (success)
		{
			canWrite = true;
		}
		return success;
	}

	bool DataSourceMemBlock::openForReadWrite(const FilePath *filepath, bool create)
	{
		return false;
	}

	bool DataSourceMemBlock::movePointerBy(DataPointer numBytes)
	{
        m_mem.move_wptr_by(static_cast<int>(numBytes));
		return true;
	}

	bool DataSourceMemBlock::movePointerTo(DataPointer numBytes)
	{
		m_mem.move_wptr_by(static_cast<int>(numBytes ));
		return true;
	}

	bool DataSourceMemBlock::validHandle(void)
	{
		return true;
	}

	void DataSourceMemBlock::close(void)
	{
		m_readPos =0;
		m_mem.set_rdata(0);
	}

	bool DataSourceMemBlock::closeAndDelete(void)
	{
		m_readPos =0;
		m_mem.set_rdata(0);
		m_mem.release();
		return true;
	}

	DataSource::Size DataSourceMemBlock::readBytes(Data *buffer, Size numBytes)
	{
		m_mem.read_bytes(buffer, static_cast<int>(numBytes), static_cast<int>(m_readPos));
		return numBytes;
	}


	DataSource::Size DataSourceMemBlock::readBytesFrom(Data *buffer, DataPointer position, Size numBytes)
	{
		int pos = static_cast<int>(position);
        m_mem.read_bytes(buffer, static_cast<int>(numBytes), pos);
		return numBytes;
	}


	DataSource::Size DataSourceMemBlock::writeBytes(const Data *buffer, Size number_bytes)
	{
		if (m_mem.write_bytes( buffer, static_cast<int>(number_bytes) ))
			return number_bytes;
		return 0;
	}

	int64_t DataSourceMemBlock::getFileSize(void) 
	{ 
		return m_mem.byte_size;

	};


	bool DataSourceMemBlock::isValidPath(const FilePath *filepath)
	{
		return false;
	}

	ptds::DataSourcePtr DataSourceMemBlock::createNew( const Data *buffer )
	{
		DataSourceMemBlock *dataSource;

		if((dataSource = new DataSourceMemBlock()) == NULL)
			return NULL;

		dataSource->openForRead( buffer );
		return dataSource;
	}


	ptds::DataSourcePtr DataSourceMemBlock::createNew( void )
	{
		DataSourceMemBlock *dataSource;

		if((dataSource = new DataSourceMemBlock()) == NULL)
			return NULL;

		dataSource->openForWrite();
		return dataSource;
	}

	DataSource::DataSourceType DataSourceMemBlock::getDataSourceType( void )
	{
		return DataSourceTypeMemBlock;
	}


	void DataSourceMemBlock::destroy(void)
	{
		delete this;
	}


	DataSource::DataSourceForm DataSourceMemBlock::getDataSourceForm(void)
	{
		return DataSourceFormLocal;
	}

	void * DataSourceMemBlock::getWriteBuffer( void )
	{
		return m_mem.wdata;
	}

	bool DataSourceMemBlock::openForRead( const Data *buffer )
	{
		m_mem.set_rdata( buffer );
		return true;
	}


} // End ptds namespace
