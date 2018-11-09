/*--------------------------------------------------------------------------------------+
|
|     $Source: Cache/Persistence/Changes/ChangeInfo.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#pragma once

#include <ECDb/ECDb.h>
#include <WebServices/Cache/Persistence/IChangeManager.h>

USING_NAMESPACE_BENTLEY_SQLITE_EC

BEGIN_BENTLEY_WEBSERVICES_NAMESPACE

/*--------------------------------------------------------------------------------------+
* Base class for accessing change info
*  @bsiclass                                                    Vincas.Razma    08/2014
+---------------+---------------+---------------+---------------+---------------+------*/
struct ChangeInfo
    {
    protected:
        Json::Value m_infoJson;

    public:
        ChangeInfo();
        ChangeInfo(JsonValueCR infoJson);

        bool IsInCache() const;

        IChangeManager::ChangeStatus GetChangeStatus() const;
        void SetChangeStatus(IChangeManager::ChangeStatus status);

        IChangeManager::SyncStatus GetSyncStatus() const;
        void SetSyncStatus(IChangeManager::SyncStatus status);

        uint64_t GetChangeNumber() const;
        void SetChangeNumber(uint64_t number);

        uint64_t GetRevision() const;
        void IncrementRevision();

        bool IsLocal() const;
        void SetIsLocal(bool value);

        void ClearChangeInfo();

        JsonValueCR GetJsonInfo() const;
        JsonValueR GetJsonInfo();
    };

typedef ChangeInfo& ChangeInfoR;
typedef const ChangeInfo& ChangeInfoCR;

END_BENTLEY_WEBSERVICES_NAMESPACE
