/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/

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
// Description  :    Base class for abstracted data sources for file or cloud access.
// Author       :    Lee Bull
// Date         :    18 Feb 2016
// Notes        :    
//
// ***********************************************************************************************************************************************


class DataSource : public DataSourceLocator
{

public:

    typedef unsigned long long int                  DataPtr;
#if ANDROID
    typedef uint64_t                                DataSize;
#else
    typedef unsigned long long int                  DataSize;
#endif
    typedef unsigned char                           Buffer;

    typedef DataSourceBuffer::TimeoutStatus         TimeoutStatus;
    typedef DataSourceBuffer::Timeout               Timeout;

protected:


    Timeout                                         timeout;
    bool                                            m_isFromCache = false;

protected:

    void                                            setStoreConfig      (DataSourceStoreConfig *newConfig);
    DataSourceStoreConfig                      *    getStoreConfig      (void);



public:

                                                    DataSource          (DataSourceAccount *sourceAccount, const SessionName &session);
    virtual                                        ~DataSource          (void);

    DataSourceService                           *   getService          (void);

    virtual            DataSourceStatus             open                (const DataSourceURL & sourceURL, DataSourceMode sourceMode);
    virtual            DataSourceStatus             close               (void) = 0;

    virtual            bool                         destroyAll          (void);

    virtual            DataSourceStatus             read                (Buffer *dest,   DataSize destSize, DataSize &readSize, DataSize size = 0) = 0;
    virtual            DataSourceStatus             read                (std::vector<Buffer>& dest) = 0;
    virtual            DataSourceStatus             write               (const Buffer *source, DataSize size) = 0;

    virtual            bool                         isValid             (void);
    virtual            bool                         isEmpty             (void);
    virtual            bool                         isFromCache         (void);

    virtual            void                         setTimeout          (Timeout timeMilliseconds);
    virtual            Timeout                      getTimeout          (void);

    virtual            void                         setCachingEnabled   (bool enabled);
    virtual            bool                         getCachingEnabled   (void);

    virtual            void                         setForceWriteToCache(void) {}

    virtual            DataSource *                 getCacheDataSource  (void);
};