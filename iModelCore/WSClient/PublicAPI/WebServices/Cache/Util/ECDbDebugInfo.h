/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/WebServices/Cache/Util/ECDbDebugInfo.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__

#include <WebServices/Cache/WebServicesCache.h>

#include "ECDbAdapter.h"

BEGIN_BENTLEY_WEBSERVICES_NAMESPACE

/*--------------------------------------------------------------------------------------+
*  @bsiclass                                                    Vincas.Razma    01/2014
+---------------+---------------+---------------+---------------+---------------+------*/
class ECDbDebugInfo
    {
    private:
        ECDbAdapter m_dbAdapter;

    public:
        WSCACHE_EXPORT ECDbDebugInfo(ObservableECDb& ecDb);
        WSCACHE_EXPORT Utf8String GetDataDebugInfo(ECSchemaCP schema);
    };

/*--------------------------------------------------------------------------------------+
*  @bsiclass                                                    Vincas.Razma    01/2014
+---------------+---------------+---------------+---------------+---------------+------*/
//! Class that simplifies cache state logging. Constructor and destructor will write ECDbDebugInfo::GetDataDebugInfo debug log for specified schemas.
class ECDbDebugInfoHolder
    {
    private:
        ECDbDebugInfo m_info;
        bvector<ECSchemaCP> m_schemas;
        Utf8String m_message;
        Utf8String m_context;

    private:
        void Log(Utf8StringCR context);

    public:
        WSCACHE_EXPORT ECDbDebugInfoHolder(ObservableECDb& ecDb, const bvector<ECSchemaCP>& schemas, Utf8StringCR message, Utf8StringCR context);
        WSCACHE_EXPORT ~ECDbDebugInfoHolder();
    };

#ifdef DEBUG
#define LogECDbContext(ecDb,message,ecschemas) ECDbDebugInfoHolder _ecdb_debug_info_holder_ (ecDb,ecschemas,message,Utf8String(__FUNCTION__));
#else
#define LogECDbContext(message,ecschemas)
#endif

END_BENTLEY_WEBSERVICES_NAMESPACE
