#pragma once

#include <ptds/DataSource.h>

namespace ptds
{
	class DataSourceBufferedWrite 
	{
	public:
		DataSourceBufferedWrite( DataSourcePtr dataSrc, DataSource::Size bufferSize );
		~DataSourceBufferedWrite();

		DataSource::Size		bufferSize				(void) const;

		DataSource::Size		writeBytes				(const DataSource::Data *buffer, DataSource::Size numBytes);

		DataSource::Size		writeBytes				(const void *buffer, DataSource::Size number_bytes)
		{
			return writeBytes(reinterpret_cast<const DataSource::Data *>(buffer), number_bytes);
		}

		template<typename T> DataSource::Size writeBytes	(const T &buffer)
		{
			return writeBytes(reinterpret_cast<const DataSource::Data *>(&buffer), sizeof(T));
		}
		DataSource::Size		writeString( const pt::String &string );

		bool					flush();

	private:
		bool					checkSize				(DataSource::Size s) const;

		bool					allocateBuffer			(DataSource::Size sz);

		void					releaseBuffer			(void);

		DataSourcePtr			m_dataSrc;
		DataSource::Data		*m_buffer;
		DataSource::Size		m_position;
		DataSource::Size		m_bufferSize;
	};	
}