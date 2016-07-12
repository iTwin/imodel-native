#pragma once
#include "DataSourceDefs.h"
#include "DataSource.h"
#include <fstream>

class DataSourceFile : public DataSource
{
public:

        typedef DataSource            Super;

protected:

        std::fstream                stream;

protected:

        std::fstream    &            getStream                (void);

public:

                                    DataSourceFile            (DataSourceAccount *sourceAccount);
                                      ~DataSourceFile            (void);

        DataSourceStatus            open                    (const DataSourceURL &sourceURL, DataSourceMode mode);
        DataSourceStatus            close                    (void);

        DataSource::DataSize        getSize                    (void);

        DataSourceStatus            read                    (Buffer *dest, DataSize destSize, DataSize &readSize, DataSize size = 0);
        DataSourceStatus            write                    (Buffer *source, DataSize size);

        DataSourceStatus            move                    (DataPtr position);

};



inline std::fstream &DataSourceFile::getStream(void)
{
    return stream;
}