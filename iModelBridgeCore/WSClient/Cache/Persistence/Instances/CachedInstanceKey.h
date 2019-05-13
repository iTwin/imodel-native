/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/

#pragma once

#include <WebServices/Cache/Util/IECDbAdapter.h>
#include "../Hierarchy/CacheNodeKey.h"

BEGIN_BENTLEY_WEBSERVICES_NAMESPACE

/*--------------------------------------------------------------------------------------+
* @bsiclass                                                     Vincas.Razma    06/2014
+---------------+---------------+---------------+---------------+---------------+------*/
struct CachedInstanceKey
    {
    private:
        CacheNodeKey m_infoKey;
        ECInstanceKey m_instanceKey;

    public:
        CachedInstanceKey() {};
        CachedInstanceKey(CacheNodeKey infoKey, ECInstanceKey instanceKey) :
            m_infoKey(infoKey),
            m_instanceKey(instanceKey)
            {}

        bool IsValid() const
            {
            return m_infoKey.IsValid() && m_instanceKey.IsValid();
            }

        CacheNodeKeyCR GetInfoKey() const
            {
            return m_infoKey;
            }

        ECInstanceKeyCR GetInstanceKey() const
            {
            return m_instanceKey;
            }

        bool operator < (const CachedInstanceKey& other) const
            {
            if (m_infoKey < other.m_infoKey)
                {
                return true;
                }
            if (other.m_infoKey < m_infoKey)
                {
                return false;
                }
            return m_instanceKey < other.m_instanceKey;
            }

        bool operator == (const CachedInstanceKey& other) const
            {
            return m_infoKey == other.m_infoKey && m_instanceKey == other.m_instanceKey;
            }

        bool operator != (const CachedInstanceKey& other) const
            {
            return !(*this == other);
            }
    };

typedef const CachedInstanceKey& CachedInstanceKeyCR;
typedef CachedInstanceKey& CachedInstanceKeyR;

END_BENTLEY_WEBSERVICES_NAMESPACE
