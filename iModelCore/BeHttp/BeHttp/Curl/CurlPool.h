/*--------------------------------------------------------------------------------------+
 |
 |     $Source: BeHttp/Curl/CurlPool.h $
 |
 |  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
 |
 +--------------------------------------------------------------------------------------*/

#pragma once

#include <Bentley/bvector.h>
#include <curl/curl.h>
#include <Bentley/BeThread.h>
#include <BeHttp/Http.h>

BEGIN_BENTLEY_HTTP_NAMESPACE

/*--------------------------------------------------------------------------------------+
*  @bsiclass                                                    Vincas.Razma    03/2013
+---------------+---------------+---------------+---------------+---------------+------*/
struct CurlPool
    {
private:
    size_t              m_maximumPoolSize;
    bvector<CURL*>      m_pool;
    BeMutex   m_poolCriticalSection;

public:
    CurlPool (size_t maximumPoolSize = 0);
    ~CurlPool();

    void  Resize (size_t maximumPoolSize);
    CURL* RetrieveHandle();
    void  ReturnHandle(CURL* handle);
    };

END_BENTLEY_HTTP_NAMESPACE
