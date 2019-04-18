/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/

#pragma once

#include <BeJsonCpp/BeJsonUtilities.h>
#include <ECDb/ECDb.h>

#include <WebServices/Cache/WebServicesCache.h>
#include "../Hierarchy/CacheNodeKey.h"

USING_NAMESPACE_BENTLEY_SQLITE_EC
USING_NAMESPACE_BENTLEY_EC

BEGIN_BENTLEY_WEBSERVICES_NAMESPACE

/*--------------------------------------------------------------------------------------+
* @bsiclass                                                     Vincas.Razma    12/2015
* Represents internal key for cached response
+---------------+---------------+---------------+---------------+---------------+------*/
struct ResponseKey
    {
    private:
        CacheNodeKey m_parent;
        CacheNodeKey m_holder;
        Utf8String m_name;

    public:
        ResponseKey() {}
        ResponseKey(CacheNodeKey parent, CacheNodeKey holder, Utf8StringCR name) :
            m_parent(parent), m_holder(holder), m_name(name)
            {}

        CacheNodeKeyCR GetParent() const
            {
            return m_parent;
            }
        CacheNodeKeyCR GetHolder() const
            {
            return m_holder;
            }
        Utf8StringCR GetName() const
            {
            return m_name;
            }
        bool IsValid() const
            {
            return m_parent.IsValid() && m_holder.IsValid();
            }
    };

typedef ResponseKey& ResponseKeyR;
typedef const ResponseKey& ResponseKeyCR;

/*--------------------------------------------------------------------------------------+
* @bsiclass                                                     Vincas.Razma    06/2014
+---------------+---------------+---------------+---------------+---------------+------*/
struct CachedResponseInfo
    {
    private:
        ResponseKey m_key;
        Json::Value m_infoJson;
        ECClassId m_infoClassId;

    public:
        //! Create invalid query info
        CachedResponseInfo();

        //! Create existing query info instance
        CachedResponseInfo(ResponseKey key, JsonValueCR infoJson, ECClassId infoClassId);

        //! Create new query info
        CachedResponseInfo(ResponseKey key, ECClassId infoClassId);

        JsonValueCR GetJsonData() const;
        JsonValueR GetJsonData();

        ResponseKeyCR GetKey() const;
        CacheNodeKey GetInfoKey() const;

        bool IsCached() const;
        Utf8CP GetName() const;

        DateTime GetAccessDate() const;
        void SetAccessDate(DateTimeCR utcDate);
    };

typedef const CachedResponseInfo& CachedResponseInfoCR;
typedef CachedResponseInfo& CachedResponseInfoR;

END_BENTLEY_WEBSERVICES_NAMESPACE
