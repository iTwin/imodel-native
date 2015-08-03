/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/WebServices/Cache/Util/ExtendedDataAdapter.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__

#include <WebServices/Cache/WebServicesCache.h>
#include <WebServices/Cache/Util/ECDbAdapter.h>
#include <WebServices/Cache/Util/IExtendedDataAdapter.h>
#include <WebServices/Cache/Util/ECSqlStatementCache.h>

BEGIN_BENTLEY_WEBSERVICES_NAMESPACE

USING_NAMESPACE_BENTLEY_SQLITE_EC

/*--------------------------------------------------------------------------------------+
* @bsiclass
+---------------+---------------+---------------+---------------+---------------+------*/
struct EXPORT_VTABLE_ATTRIBUTE ExtendedDataAdapter : public IExtendedDataAdapter
    {
    private:
        ECDbAdapter m_dbAdapter;
        ECSqlStatementCache m_statementCache;

    public:
        WSCACHE_EXPORT ExtendedDataAdapter (ObservableECDb& db);

        //! Call import schema once to initialize ECDb for extended data storage
        WSCACHE_EXPORT BentleyStatus ImportSchema ();

        WSCACHE_EXPORT ExtendedData GetData (ECInstanceKeyCR instanceKey) override;
        WSCACHE_EXPORT BentleyStatus UpdateData (ExtendedData& data) override;
    };

END_BENTLEY_WEBSERVICES_NAMESPACE
