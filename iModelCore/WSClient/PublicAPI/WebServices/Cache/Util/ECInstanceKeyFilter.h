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
public:
    typedef std::function<BentleyStatus(ECSqlStatement& statement, int firstArgIndex)> BindArgsCallback;
    typedef std::function<BentleyStatus(ECClassCR ecClass, Utf8StringR outWhereClause, BindArgsCallback& outBindArgs)> WhereClauseCallback;

private:
    typedef std::function<BentleyStatus(ECSqlStatement& statement)> BindArgsWOIndexCallback;

    bool m_classFilterAdded = false;
    bool m_filterClasesPolymorphically = false;
    bset<ECClassCP> m_classesToFilter;

    int m_limit = 0;

    bvector<WhereClauseCallback> m_whereClauseCallbacks;
    bvector<WhereClauseCallback> m_tmpWhereClauseCallbacks;

    int m_nextBindingIndex = 0;
    Utf8String m_whereClause;
    bvector<BindArgsWOIndexCallback> m_bindArgsCallbacks;

private:
    bool IsClassInFilter(ECClassCR ecClass);

    BentleyStatus DoFilter
        (
        CacheTransactionCR txn,
        ECClassCR ecClass,
        const ECInstanceIdSet& instances,
        ECInstanceKeyMultiMap& result,
        ICancellationTokenPtr ct = nullptr
        );

    void AddTmpWhereClauseCallbacks(const ECInstanceIdSet& instances);

    BentleyStatus BuildWhereClause (ECClassCR ecClass);
    BentleyStatus AddWhereClauseSentence(ECClassCR ecClass, const WhereClauseCallback& whereClauseCallback);

    BentleyStatus ExecuteDbQuery
        (
        CacheTransactionCR txn,
        ECClassCR ecClass,
        ECInstanceKeyMultiMap& result,
        ICancellationTokenPtr ct
        );

public:
    WSCACHE_EXPORT ECInstanceKeyFilter();
    WSCACHE_EXPORT ECInstanceKeyFilter(ECClassCR ecClass, bool polymorphically = false);
    WSCACHE_EXPORT ECInstanceKeyFilter(bset<ECClassCP> ecClasses, bool polymorphically = false);

    WSCACHE_EXPORT void AddLabelFilter(Utf8StringCR label);

    WSCACHE_EXPORT void AddAnyPropertiesLikeFilter(bset<Utf8String> propertiesToSearch, Utf8StringCR searchTerm);
    WSCACHE_EXPORT void AddAnyPropertiesLikeFilter(const std::function<bset<Utf8String>(ECClassCR ecClass)>& propertiesToSearch, Utf8StringCR searchTerm);

    WSCACHE_EXPORT void AddWhereClauseFilter(Utf8StringCR whereClause, const BindArgsCallback& bindArgsCallback);
    WSCACHE_EXPORT void AddWhereClauseFilter(const WhereClauseCallback& whereClauseCallback);

    WSCACHE_EXPORT void SetLimit(int limit);

    WSCACHE_EXPORT BentleyStatus Filter
        (
        CacheTransactionCR txn,
        const ECInstanceKeyMultiMap& instances, 
        ECInstanceKeyMultiMap& result,
        ICancellationTokenPtr ct = nullptr
        );

    WSCACHE_EXPORT BentleyStatus Filter
        (
        CacheTransactionCR txn,
        ECClassCR ecClass,
        const bset<ECInstanceId>& instances,
        ECInstanceKeyMultiMap& result,
        ICancellationTokenPtr ct = nullptr
        );
    };

END_BENTLEY_WEBSERVICES_NAMESPACE
