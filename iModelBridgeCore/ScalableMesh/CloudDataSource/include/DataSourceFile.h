#pragma once
#include "DataSourceDefs.h"
#include "DataSource.h"
#include <fstream>
#include <Bentley/BeFile.h>

class DataSourceFile : public DataSource
{
public:

        typedef DataSource            Super;

protected:

        //std::fstream                stream;
        BeFile                        stream;

protected:

        //std::fstream    &            getStream              (void);
    BeFile            &           getStream(void);

public:

                                    DataSourceFile          (DataSourceAccount *sourceAccount, const SessionName &session);
                                   ~DataSourceFile          (void);

        DataSourceStatus            open                    (const DataSourceURL &sourceURL, DataSourceMode mode);
        DataSourceStatus            close                   (void);

        DataSource::DataSize        getSize                 (void);

        DataSourceStatus            read                    (Buffer *dest, DataSize destSize, DataSize &readSize, DataSize size = 0);
        DataSourceStatus            read                    (std::vector<Buffer>& dest);
        DataSourceStatus            write                   (const Buffer *source, DataSize size);

        DataSourceStatus            move                    (DataPtr position);
};

inline BeFile &DataSourceFile::getStream(void)
    {
    return stream;
    }

//inline std::fstream &DataSourceFile::getStream(void)
//{
//    return stream;
//}