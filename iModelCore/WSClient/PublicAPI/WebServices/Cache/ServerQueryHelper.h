/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/WebServices/Cache/ServerQueryHelper.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__

#include <WebServices/Cache/WebServicesCache.h>
#include <WebServices/Cache/Util/ISelectProvider.h>
#include <ECDb/ECDbApi.h>

BEGIN_BENTLEY_WEBSERVICES_NAMESPACE

USING_NAMESPACE_EC
USING_NAMESPACE_BENTLEY_SQLITE_EC

/*--------------------------------------------------------------------------------------+
* @bsiclass                                                     Vincas.Razma    06/2014
+---------------+---------------+---------------+---------------+---------------+------*/
struct ServerQueryHelper
    {
    private:
        const ISelectProvider& m_selectProvider;

    private:
        static bset<ECPropertyCP> GetRequiredProperties(ECPropertyCP ecProperty);

    public:
        WSCACHE_EXPORT ServerQueryHelper(const ISelectProvider& selectProvider);

        //! Returns empty when selecting all classes
        WSCACHE_EXPORT bset<Utf8String> GetAllSelectedProperties(const ECSchemaList& ecSchemas) const;
        WSCACHE_EXPORT Utf8String GetSelect(ECClassCR ecClass) const;
    };

END_BENTLEY_WEBSERVICES_NAMESPACE
