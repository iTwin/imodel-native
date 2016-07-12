/*--------------------------------------------------------------------------------------+
|
|     $Source: CloudDataSource/include/DataSource.h $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#pragma once


// ***********************************************************************************************************************************************
// Includes
// ***********************************************************************************************************************************************

#include <string>
#include "DataSourceDefs.h"
#include "DataSourceURL.h"
#include "DataSourceStatus.h"
#include "DataSourceLocator.h"
#include "DataSourceMode.h"
#include "DataSourceBuffer.h"


// ***********************************************************************************************************************************************
// Opaque types
// ***********************************************************************************************************************************************

class DataSourceService;
class DataSourceAccount;
class DataSourceStoreConfig;


// ***********************************************************************************************************************************************
// @bsiclass    :    DataSource
// Description    :    Base class for abstracted data sources for file or cloud access.
// Author        :    Lee Bull
// Date            :    18 Feb 2016
// Notes        :    
//
// ***********************************************************************************************************************************************


class DataSource : public DataSourceLocator
{

public:

    typedef unsigned long long int                DataPtr;
    typedef unsigned long long int                DataSize;
    typedef unsigned char                         Buffer;

    typedef    DataSourceBuffer::TimeoutStatus        TimeoutStatus;
    typedef DataSourceBuffer::Timeout            Timeout;

    typedef std::wstring                        Name;

protected:

    Timeout                                        timeout;

protected:

    void                                        setStoreConfig        (DataSourceStoreConfig *newConfig);
    DataSourceStoreConfig                    *    getStoreConfig        (void);

public:

                                                DataSource            (DataSourceAccount *sourceAccount);
    virtual                                          ~DataSource            (void);

    DataSourceService                        *    getService            (void);

    virtual            DataSourceStatus            open                (const DataSourceURL & sourceURL, DataSourceMode sourceMode);
    virtual            DataSourceStatus            close                (void) = 0;

    virtual            DataSourceStatus            read                (Buffer *dest,   DataSize destSize, DataSize &readSize, DataSize size = 0) = 0;
    virtual            DataSourceStatus            write                (Buffer *source, DataSize size) = 0;

    virtual            bool                        isValid                (void);

    virtual            void                        setTimeout            (Timeout timeMilliseconds);
    virtual            Timeout                        getTimeout            (void);

    virtual            void                        setCachingEnabled    (bool enabled);
    virtual            bool                        getCachingEnabled    (void);
};