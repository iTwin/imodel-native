/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/

#include "CurlPool.h"

USING_NAMESPACE_BENTLEY_HTTP

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    03/2013
+---------------+---------------+---------------+---------------+---------------+------*/
CurlPool::CurlPool(size_t maximumPoolSize) : m_maximumPoolSize(maximumPoolSize) {}

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    03/2013
+---------------+---------------+---------------+---------------+---------------+------*/
CurlPool::~CurlPool ()
    {
    BeMutexHolder lock(m_poolCriticalSection);

    for (CURL* handle : m_pool)
        curl_easy_cleanup(handle);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    03/2013
+---------------+---------------+---------------+---------------+---------------+------*/
void CurlPool::Resize (size_t maximumPoolSize)
    {
    BeMutexHolder lock(m_poolCriticalSection);

    m_maximumPoolSize = maximumPoolSize;

    for (size_t i = m_maximumPoolSize; i < m_pool.size(); i++)
        curl_easy_cleanup(m_pool[i]);

    if (m_pool.size() > m_maximumPoolSize)
        m_pool.resize(m_maximumPoolSize);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    03/2013
+---------------+---------------+---------------+---------------+---------------+------*/
CURL* CurlPool::RetrieveHandle ()
    {
    BeMutexHolder lock(m_poolCriticalSection);
    CURL* handle;

    if (m_pool.size () > 0)
        {
        handle = m_pool.back ();
        m_pool.pop_back ();
        }
    else
        {
        handle = curl_easy_init ();
        }

    return handle;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    03/2013
+---------------+---------------+---------------+---------------+---------------+------*/
void  CurlPool::ReturnHandle (CURL* handle)
    {
    BeMutexHolder lock(m_poolCriticalSection);

    if (nullptr == handle)
        return;

    if (m_pool.size () < m_maximumPoolSize)
        {
        curl_easy_reset (handle);
        m_pool.push_back (handle);
        }
    else
        {
        curl_easy_cleanup (handle);
        }
    }
