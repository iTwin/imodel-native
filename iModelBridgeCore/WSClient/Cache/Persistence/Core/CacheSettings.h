/*--------------------------------------------------------------------------------------+
|
|     $Source: Cache/Persistence/Core/CacheSettings.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#pragma once

#include <WebServices/Cache/Util/ECDbAdapter.h>

BEGIN_BENTLEY_WEBSERVICES_NAMESPACE

/*--------------------------------------------------------------------------------------+
* @bsiclass                                                     Vincas.Razma    02/2014
+---------------+---------------+---------------+---------------+---------------+------*/
struct CacheSettings
    {
    private:
        ECDbAdapter m_dbAdapter;
        uint32_t m_version;

    private:
        BentleyStatus ReadLegacyVersion();

    public:
        CacheSettings(ObservableECDb& db);

        BentleyStatus Read();
        BentleyStatus Save();

        void SetVersion(uint32_t version);
        uint32_t GetVersion() const;
    };

END_BENTLEY_WEBSERVICES_NAMESPACE
