/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/WebServices/Cache/Util/ECInstanceKeyFilter.h $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#pragma once
//__PUBLISH_SECTION_START__

#include <WebServices/Cache/WebServicesCache.h>

#include <WebServices/Cache/Transactions/CacheTransaction.h>

BEGIN_BENTLEY_WEBSERVICES_NAMESPACE

/*--------------------------------------------------------------------------------------+
* @bsiclass                                                    
+---------------+---------------+---------------+---------------+---------------+------*/
struct ECInstanceKeyFilter
    {
private:
    static BentleyStatus FilterByWhereClause
        (
        CacheTransactionCR txn,
        ECInstanceKeyMultiMap::const_iterator from,
        ECInstanceKeyMultiMap::const_iterator to,
        ECInstanceKeyMultiMap& result,
        Utf8StringCR whereClause,
        const std::function<void(ECSqlStatement& statement, int firstArgIndex)>& bindArgs
        );

    static BentleyStatus FilterByLabel
        (
        CacheTransactionCR txn,
        ECInstanceKeyMultiMap::const_iterator from,
        ECInstanceKeyMultiMap::const_iterator to,
        ECInstanceKeyMultiMap& result,
        Utf8StringCR label
        );

public:
    WSCACHE_EXPORT static BentleyStatus FilterByLabel
        (
        CacheTransactionCR txn,
        const ECInstanceKeyMultiMap& instances,
        ECInstanceKeyMultiMap& result,
        Utf8StringCR label
        );
    WSCACHE_EXPORT static BentleyStatus FilterByLabelAndClass
        (
        CacheTransactionCR txn,
        const ECInstanceKeyMultiMap& instances,
        ECInstanceKeyMultiMap& result,
        Utf8StringCR label,
        ECClassCR ecClass
        );
    WSCACHE_EXPORT static BentleyStatus FilterByLabelAndClass
        (
        CacheTransactionCR txn,
        const ECInstanceKeyMultiMap& instances,
        ECInstanceKeyMultiMap& result,
        Utf8StringCR label,
        bset<ECClassCP> ecClasses
        );

    WSCACHE_EXPORT static BentleyStatus FilterWherePropertiesLike
        (
        CacheTransactionCR txn,
        const ECInstanceKeyMultiMap& instances,
        ECInstanceKeyMultiMap& result,
        bset<Utf8String> properties,
        Utf8String searchTerm
        );
    };

END_BENTLEY_WEBSERVICES_NAMESPACE
