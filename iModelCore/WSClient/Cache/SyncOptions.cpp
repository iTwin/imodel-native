/*--------------------------------------------------------------------------------------+
 |
 |     $Source: Cache/SyncOptions.cpp $
 |
 |  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
 |
 +--------------------------------------------------------------------------------------*/

#include <WebServices/Cache/SyncOptions.h>
#include "ICachingDataSource.xliff.h"

USING_NAMESPACE_BENTLEY_WEBSERVICES

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
SyncOptions::SyncOptions() :
m_useChangesets (false),
m_maxChangesetSizeBytes(4*1024*1024),
m_maxChangesetInstances(0)
    {
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void SyncOptions::SetUseChangesets(bool useChangesets)
    {
    m_useChangesets = useChangesets;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool SyncOptions::GetUseChangesets() const
    {
    return m_useChangesets;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void SyncOptions::SetMaxChangesetSize(size_t bytes)
    {
    m_maxChangesetSizeBytes = bytes;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
size_t SyncOptions::GetMaxChangesetSize() const
    {
    return m_maxChangesetSizeBytes;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void SyncOptions::SetMaxChangesetInstanceCount(size_t count)
    {
    m_maxChangesetInstances = count;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
size_t SyncOptions::GetMaxChangesetInstanceCount() const
    {
    return m_maxChangesetInstances;
    }
