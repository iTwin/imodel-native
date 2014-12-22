/*--------------------------------------------------------------------------------------+
|
|     $Source: logging/native/log4cxx/cinterface.cpp $
|
|  $Copyright: (c) 2011 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#pragma warning ( disable : 4786 )

#include "log4cxx.h"

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <bentley/bentley.h>


BEGIN_BENTLEY_LOGGING_NAMESPACE;

namespace Provider {

/*---------------------------------------------------------------------------------**//**
 * @bsimethod                                                      VijaiKalyan   02/06
 +---------------+---------------+---------------+---------------+---------------+------*/
static Log4cxxProvider* log4cxx_castToLog4cxxProvider(HANDLE pHandle)
    {
    if(NULL == pHandle)
        return NULL;

    ILogProvider* provider = static_cast<ILogProvider*>(pHandle);

    return dynamic_cast<Log4cxxProvider*>(provider);
    }

/*---------------------------------------------------------------------------------**//**
 * @bsimethod                                                      VijaiKalyan   02/06
 +---------------+---------------+---------------+---------------+---------------+------*/
DWORD log4cxx_createLog4cxxProvider
(
HANDLE* pProvider
)
    {
    if(NULL == pProvider)
        return 0;

    Log4cxxProvider* provider = new Log4cxxProvider;

    if(NULL == provider)
        return 0;

    *pProvider = provider;

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
 * @bsimethod                                                      VijaiKalyan   02/06
 +---------------+---------------+---------------+---------------+---------------+------*/
void log4cxx_destroyLog4cxxProvider
(
HANDLE* pProvider
)
    {
    if(NULL == pProvider || NULL == *pProvider)
        return;

    Log4cxxProvider* provider = log4cxx_castToLog4cxxProvider(*pProvider);
    delete provider;

    *pProvider = NULL;
    }

/*---------------------------------------------------------------------------------**//**
 * @bsimethod                                                      VijaiKalyan   02/06
 +---------------+---------------+---------------+---------------+---------------+------*/
DWORD log4cxx_initializeLog4cxxProvider
(
HANDLE pProvider,
wchar_t const* const configFile,
int watchInterval
)
    {
    if(NULL == pProvider)
        return 0;

    Log4cxxProvider* provider = log4cxx_castToLog4cxxProvider(pProvider);

    if(NULL == provider)
        return 0;

    struct _stat info = {0};

    if ( NULL == configFile || 0 != _wstat(configFile, &info) )
        {
        provider->BasicConfiguration();
        }
    else
        {
        if(watchInterval <= 0)
            provider->LoadConfiguration(configFile);
        else
            provider->LoadAndWatchConfiguration(configFile, watchInterval);
        }

    return SUCCESS;
    }

}

END_BENTLEY_LOGGING_NAMESPACE;