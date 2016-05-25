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
        "WHERE ECInstanceId IN (%s) AND %s",
        ECSqlBuilder::ToECSqlSnippet(*ecClassP).c_str(),
        ECDbHelper::ToECInstanceIdList(from, to).c_str(),
        whereClause.c_str());

    ECSqlStatement statement;
    if (SUCCESS != txn.GetCache().GetAdapter().PrepareStatement(statement, sql))
        {
        return ERROR;
        }

    bindArgs(statement, 1);

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