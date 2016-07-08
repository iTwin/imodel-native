#pragma once
#include "DataSourceDefs.h"
#include "DataSource.h"
#include <fstream>

class DataSourceFile : public DataSource
{
public:

		typedef DataSource			Super;

protected:

		std::fstream				stream;

protected:

		std::fstream	&			getStream				(void);

public:

CLOUD_EXPORT									DataSourceFile			(DataSourceAccount *sourceAccount);
CLOUD_EXPORT   								   ~DataSourceFile			(void);

CLOUD_EXPORT		DataSourceStatus			open					(const DataSourceURL &sourceURL, DataSourceMode mode);
CLOUD_EXPORT		DataSourceStatus			close					(void);

CLOUD_EXPORT		DataSource::DataSize		getSize					(void);

CLOUD_EXPORT		DataSourceStatus			read					(Buffer *dest, DataSize destSize, DataSize &readSize, DataSize size = 0);
CLOUD_EXPORT		DataSourceStatus			write					(Buffer *source, DataSize size);

CLOUD_EXPORT		DataSourceStatus			move					(DataPtr position);

};



inline std::fstream &DataSourceFile::getStream(void)
{
	return stream;
}