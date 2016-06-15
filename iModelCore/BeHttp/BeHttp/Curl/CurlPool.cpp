/*--------------------------------------------------------------------------------------+
|
|     $Source: BeHttp/Curl/CurlPool.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#include "CurlPool.h"

USING_NAMESPACE_BENTLEY_HTTP

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    03/2013
+---------------+---------------+---------------+---------------+---------------+------*/
CurlPool::CurlPool (size_t maximumPoolSize)
: m_maximumPoolSize (maximumPoolSize)
    {
    BeAssert (m_maximumPoolSize >= 0);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    03/2013
+---------------+---------------+---------------+---------------+---------------+------*/
CurlPool::~CurlPool ()
    {
    m_poolCriticalSection.Enter ();

    for (CURL* handle : m_pool)
        {
        curl_easy_cleanup (handle);
        }

    m_poolCriticalSection.Leave ();
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    03/2013
+---------------+---------------+---------------+---------------+---------------+------*/
void CurlPool::Resize (size_t maximumPoolSize)
    {
    m_poolCriticalSection.Enter ();

    m_maximumPoolSize = maximumPoolSize;
    for (size_t i = m_maximumPoolSize; i < m_pool.size (); i++)
        {
        curl_easy_cleanup (m_pool[i]);
        }
    if (m_pool.size () > m_maximumPoolSize)
        {
        m_pool.resize (m_maximumPoolSize);
        }

    m_poolCriticalSection.Leave ();
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    03/2013
+---------------+---------------+---------------+---------------+---------------+------*/
CURL* CurlPool::RetrieveHandle ()
    {
    CURL* handle;
    m_poolCriticalSection.Enter ();

    if (m_pool.size () > 0)
        {
        handle = m_pool.back ();
        m_pool.pop_back ();
        }
    else
        {
        handle = curl_easy_init ();
        }

    m_poolCriticalSection.Leave ();
    return handle;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    03/2013
+---------------+---------------+---------------+---------------+---------------+------*/
void  CurlPool::ReturnHandle (CURL* handle)
    {
    m_poolCriticalSection.Enter ();

    if (m_pool.size () < m_maximumPoolSize)
        {
        curl_easy_reset (handle);
        m_pool.push_back (handle);
        }
    else
        {
        curl_easy_cleanup (handle);
        }

    m_poolCriticalSection.Leave ();
    }
