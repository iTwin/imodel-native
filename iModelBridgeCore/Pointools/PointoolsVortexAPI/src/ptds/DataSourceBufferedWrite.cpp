#include "PointoolsVortexAPIInternal.h"
#include <ptds/DataSourceBufferedWrite.h>
#include <ptds/DataSourceStringIO.h>

using namespace ptds;


DataSourceBufferedWrite::DataSourceBufferedWrite( DataSourcePtr dataSrc, DataSource::Size bufferSize )
{
	m_dataSrc = dataSrc;
	m_position = 0;
	m_bufferSize = 0;
	m_buffer = 0;

	if (!allocateBuffer(bufferSize))
	{
        bufferSize = static_cast<DataSource::Size>(bufferSize * 0.5);
		if (!allocateBuffer(bufferSize))
            bufferSize = static_cast<DataSource::Size>(bufferSize * 0.5);

		allocateBuffer(bufferSize);
	}
}

DataSourceBufferedWrite::~DataSourceBufferedWrite()
{
	flush();
	releaseBuffer();
}

bool DataSourceBufferedWrite::flush()
{
	if (!m_position) return true;

	m_dataSrc->writeBytes( m_buffer, m_position);

	bool suc = m_position ? true : false;
	m_position = 0;
	return suc;
}

bool DataSourceBufferedWrite::checkSize( DataSource::Size s ) const
{
	return (m_position+s>=m_bufferSize) ? false : true;
}

bool DataSourceBufferedWrite::allocateBuffer( DataSource::Size sz )
{
	try
	{
		m_buffer = new DataSource::Data[(size_t)sz];
		m_bufferSize = sz;
		return true;
	}
	catch (std::bad_alloc)
	{
		m_bufferSize = 0;
		return false;
	}
}


void DataSourceBufferedWrite::releaseBuffer()
{
	if (m_bufferSize && m_buffer)
		delete [] m_buffer;
	m_buffer = 0;
	m_bufferSize = 0;
}

DataSource::Size DataSourceBufferedWrite::bufferSize() const
{
	return m_bufferSize;
}

DataSource::Size DataSourceBufferedWrite::writeBytes( const DataSource::Data *buffer, DataSource::Size numBytes )
{
	// check for large write
	if (numBytes >= m_bufferSize)
	{
		flush();
		return m_dataSrc->writeBytes(buffer,numBytes);	//write through
	}
	if (!checkSize(numBytes)) 
		flush();

	memcpy(&m_buffer[m_position], buffer, (size_t) numBytes);
	m_position += numBytes;

	return numBytes;
}

DataSource::Size DataSourceBufferedWrite::writeString( const pt::String &string )
{
	return DataSourceStringIO::write( this, string );
}



