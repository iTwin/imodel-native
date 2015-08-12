/*--------------------------------------------------------------------------------------+
 |
 |     $Source: PublicAPI/WebServices/Cache/Util/ValueIncrementor.h $
 |
 |  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
 |
 +--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__

#include <ECDb/ECDbApi.h>
#include <ECObjects/ECObjects.h>
#include <WebServices/Cache/Util/ECSqlStatementCache.h>

#include <WebServices/Cache/WebServicesCache.h>

BEGIN_BENTLEY_WEBSERVICES_NAMESPACE

USING_NAMESPACE_EC
USING_NAMESPACE_BENTLEY_SQLITE_EC

/*--------------------------------------------------------------------------------------+
*  @bsiclass                                                    Vincas.Razma    01/2014
+---------------+---------------+---------------+---------------+---------------+------*/
struct ValueIncrementor
    {
    private:
        ECDb* m_db;
        ECClassId m_propertyClassId;
        Utf8String m_propertyName;
        ECSqlStatementCache* m_statementCache;

    public:
        //! Construct new unique name manager
        //! @param db
        //! @param statementCache
        //! @param ecProperty - property of type "long". First found instance of this property class is used to save last increment
        WSCACHE_EXPORT ValueIncrementor(ECDb& db, ECSqlStatementCache& statementCache, ECPropertyCR ecProperty);

        //! @param[out] valueOut - new numeric value
        WSCACHE_EXPORT BentleyStatus IncrementWithoutSaving(Utf8StringR valueOut);
    };

END_BENTLEY_WEBSERVICES_NAMESPACE
