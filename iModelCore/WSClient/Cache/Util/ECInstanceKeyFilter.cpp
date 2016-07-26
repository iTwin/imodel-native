/*--------------------------------------------------------------------------------------+
|
|     $Source: Cache/Util/ECInstanceKeyFilter.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#include <WebServices/Cache/Util/ECInstanceKeyFilter.h>

#include <WebServices/Cache/Util/ECDbHelper.h>

USING_NAMESPACE_BENTLEY_WEBSERVICES

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus ECInstanceKeyFilter::FilterByWhereClause
(
CacheTransactionCR txn,
ECInstanceKeyMultiMap::const_iterator from,
ECInstanceKeyMultiMap::const_iterator to,
ECInstanceKeyMultiMap& result,
Utf8StringCR whereClause,
const std::function<void(ECSqlStatement& statement, int firstArgIndex)>& bindArgs
)
    {
    auto ecClassP = txn.GetCache().GetAdapter().GetECClass(from->first);

    auto sql = Utf8PrintfString(
        "SELECT ECInstanceId FROM %s "
        "WHERE InVirtualSet(?, ECInstanceId) AND %s",
        ecClassP->GetECSqlName().c_str(),
        whereClause.c_str());

    ECSqlStatement statement;
    if (SUCCESS != txn.GetCache().GetAdapter().PrepareStatement(statement, sql))
        {
        return ERROR;
        }

    ECInstanceIdSet idSet;
    for (auto it = from; it != to; it++)
        {
        idSet.insert(it->second);
        }

    statement.BindInt64(1, (int64_t)&idSet);

    bindArgs(statement, 2);

    return txn.GetCache().GetAdapter().ExtractECInstanceKeyMultiMapFromStatement(statement, 0, ecClassP->GetId(), result);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                  
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus ECInstanceKeyFilter::FilterByLabel
(
CacheTransactionCR txn,
const ECInstanceKeyMultiMap& instances,
ECInstanceKeyMultiMap& result,
Utf8StringCR label
)
    {
    for (auto start = instances.begin(); start != instances.end(); )
        {
        auto rangeEnd = instances.upper_bound(start->first);

        if (SUCCESS != FilterByLabel(txn, start, rangeEnd, result, label))
            return ERROR;

        start = rangeEnd;
        }

    return SUCCESS;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus ECInstanceKeyFilter::FilterByLabelAndClass
(
CacheTransactionCR txn,
const ECInstanceKeyMultiMap& instances,
ECInstanceKeyMultiMap& result,
Utf8StringCR label,
ECClassCR ecClass
)
    {
    bset<ECClassCP> ecClasses;
    ecClasses.insert(&ecClass);

    return FilterByLabelAndClass(txn, instances, result, label, ecClasses);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus ECInstanceKeyFilter::FilterByLabelAndClass
(
CacheTransactionCR txn,
const ECInstanceKeyMultiMap& instances,
ECInstanceKeyMultiMap& result,
Utf8StringCR label,
bset<ECClassCP> ecClasses
)
    {
    for (auto ecClass : ecClasses)
        {
        if (ecClass == nullptr)
            {
            BeAssert(false);
            continue;
            }

        auto range = instances.equal_range(ecClass->GetId());

        if (SUCCESS != FilterByLabel(txn, range.first, range.second, result, label))
            return ERROR;
        }
       
    return SUCCESS;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus ECInstanceKeyFilter::FilterByLabel
(
CacheTransactionCR txn,
ECInstanceKeyMultiMap::const_iterator from,
ECInstanceKeyMultiMap::const_iterator to,
ECInstanceKeyMultiMap& result,
Utf8StringCR label
)
    {
    auto ecClass = txn.GetCache().GetAdapter().GetECClass(from->first);
    if (ecClass == nullptr)
        {
        BeAssert(false);
        return ERROR;
        }

    ECPropertyCP property = ecClass->GetInstanceLabelProperty();
    if (nullptr == property)
        {
        BeAssert(false);
        return ERROR;
        }

    Utf8String labelProperty(property->GetName());

    if (labelProperty.empty())
        return SUCCESS;

    Utf8String whereClause = labelProperty + " = ?";
    return FilterByWhereClause (
        txn,
        from,
        to,
        result,
        whereClause,
        [&] (ECSqlStatement& statement, int firstArgIndex)
        {
        statement.BindText(firstArgIndex, label.c_str(), IECSqlBinder::MakeCopy::No);
        });
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus ECInstanceKeyFilter::FilterWherePropertiesLike
(
CacheTransactionCR txn,
const ECInstanceKeyMultiMap& instances,
ECInstanceKeyMultiMap& result,
bset<Utf8String> properties,
Utf8String searchTerm
)
    {
    Utf8String whereClause = "";
    for (Utf8StringCR property : properties)
        {
        if (!whereClause.empty())
            {
            whereClause += " OR ";
            }

        whereClause += "[" + property + "] LIKE ?";
        }
    whereClause = "(" + whereClause + ")";

    searchTerm = "%" + searchTerm + "%";

    return FilterByWhereClause(
        txn,
        instances.begin(),
        instances.end(),
        result,
        whereClause,
        [&] (ECSqlStatement& statement, int firstArgIndex)
        {
        for (int i = 0; i < properties.size(); i++)
            {
            statement.BindText(firstArgIndex + i, searchTerm.c_str(), IECSqlBinder::MakeCopy::No);
            }
        });
    }