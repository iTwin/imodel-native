/*--------------------------------------------------------------------------------------+
|
|     $Source: Cache/Persistence/Responses/CachedResponseInfo.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#pragma once

#include <BeJsonCpp/BeJsonUtilities.h>
#include <ECDb/ECDb.h>

#include <WebServices/Cache/WebServicesCache.h>

USING_NAMESPACE_BENTLEY_SQLITE_EC
USING_NAMESPACE_EC

BEGIN_BENTLEY_WEBSERVICES_NAMESPACE

/*--------------------------------------------------------------------------------------+
* @bsiclass                                                     Vincas.Razma    06/2014
+---------------+---------------+---------------+---------------+---------------+------*/
struct CachedResponseInfo
    {
    private:
        ECInstanceKey m_parent;
        ECInstanceKey m_holder;

        Json::Value m_infoJson;
        ECClassId m_infoClassId;

    public:
        //! Create invalid query info
        CachedResponseInfo();

        //! Create existing query info instance
        CachedResponseInfo(ECInstanceKeyCR parent, ECInstanceKeyCR holder, JsonValueCR infoJson, ECClassId infoClassId);

        //! Create new query info
        CachedResponseInfo(ECInstanceKeyCR parent, ECInstanceKeyCR holder, Utf8StringCR name, ECClassId infoClassId);

        JsonValueCR GetJsonData() const;
        JsonValueR GetJsonData();

        ECInstanceKeyCR GetParent() const;
        ECInstanceKeyCR GetHolder() const;
        ECInstanceKey GetKey() const;

        bool IsCached() const;
        Utf8CP GetName() const;

        DateTime GetCacheDate() const;
        void SetCacheDate(DateTimeCR utcDate);

        DateTime GetAccessDate() const;
        void SetAccessDate(DateTimeCR utcDate);

        Utf8CP GetCacheTag() const;
        void SetCacheTag(Utf8StringCR tag);
    };

typedef const CachedResponseInfo& CachedResponseInfoCR;
typedef CachedResponseInfo& CachedResponseInfoR;

END_BENTLEY_WEBSERVICES_NAMESPACE
