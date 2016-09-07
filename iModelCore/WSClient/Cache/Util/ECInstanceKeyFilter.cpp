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
ECInstanceKeyFilter::ECInstanceKeyFilter()
    {
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ECInstanceKeyFilter::ECInstanceKeyFilter(ECClassCR ecClass, bool polymorphically)
    {
    m_classesToFilter.insert(&ecClass);
    m_classFilterAdded = true;
    m_filterClasesPolymorphically = polymorphically;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ECInstanceKeyFilter::ECInstanceKeyFilter(bset<ECClassCP> ecClasses, bool polymorphically)
    {
    m_classesToFilter = ecClasses;
    m_classFilterAdded = true;
    m_filterClasesPolymorphically = polymorphically;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void ECInstanceKeyFilter::SetLimit(int limit)
    {
    m_limit = limit;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool ECInstanceKeyFilter::IsClassInFilter(ECClassCR ecClass)
    {
    if (!m_classFilterAdded)
        return true;

    for (auto searchClass : m_classesToFilter)
        {
        if (searchClass == nullptr)
            continue;

        if (m_filterClasesPolymorphically)
            {
            if (ecClass.Is(searchClass))
                return true;
            }
        else
            {
            if (ecClass.GetId() == searchClass->GetId())
                return true;
            }
        }

    return false;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus ECInstanceKeyFilter::Filter
(
CacheTransactionCR txn, 
const ECInstanceKeyMultiMap& instances, 
ECInstanceKeyMultiMap& result,
ICancellationTokenPtr ct
)
    {
    for (auto start = instances.begin(), end = start; start != instances.end(); start = end)
        {
        if (ct && ct->IsCanceled())
            return SUCCESS;

        end = instances.upper_bound(start->first);

        auto ecClass = txn.GetCache().GetAdapter().GetECClass(start->first);
        if (ecClass == nullptr)
            return ERROR;

        if (!IsClassInFilter(*ecClass))
            continue;

        ECInstanceIdSet idSet;
        for (auto it = start; it != end; it++)
            {
            idSet.insert(it->second);
            }

        if (SUCCESS != DoFilter(txn, *ecClass, idSet, result, ct))
            return ERROR;
        }

    return SUCCESS;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus ECInstanceKeyFilter::Filter
(
CacheTransactionCR txn,
ECClassCR ecClass,
const bset<ECInstanceId>& instances,
ECInstanceKeyMultiMap& result,
ICancellationTokenPtr ct
)
    {
    if (!IsClassInFilter(ecClass))
        return SUCCESS;

    ECInstanceIdSet idSet;
    for (auto instanceId : instances)
        idSet.insert(instanceId);

    return DoFilter(txn, ecClass, idSet, result, ct);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus ECInstanceKeyFilter::DoFilter
(
CacheTransactionCR txn,
ECClassCR ecClass,
const ECInstanceIdSet& instances,
ECInstanceKeyMultiMap& result,
ICancellationTokenPtr ct
)
    {
    // No Filters added
    if (m_whereClauseCallbacks.empty())
        {
        for (auto instanceId : instances)
            {
            if (m_limit <= 0 || result.size() < m_limit)
                result.insert({ecClass.GetId(), instanceId});
            }

        return SUCCESS;
        }

    AddTmpWhereClauseCallbacks(instances);

    if (SUCCESS != BuildWhereClause(ecClass))
        return ERROR;

    if (SUCCESS != ExecuteDbQuery(txn, ecClass, result, ct))
        return ERROR;

    m_tmpWhereClauseCallbacks.clear();

    return SUCCESS;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void ECInstanceKeyFilter::AddTmpWhereClauseCallbacks(const ECInstanceIdSet& instances)
    {
    m_tmpWhereClauseCallbacks.push_back([&] (ECClassCR ecClass, Utf8StringR outWhereClause, BindArgsCallback& outBindArgs)
        {
        outWhereClause = "InVirtualSet(? , ECInstanceId)";

        outBindArgs = [&] (ECSqlStatement& statement, int firstArgIndex)
            {
            if (ECSqlStatus::Success == statement.BindInt64(firstArgIndex, (int64_t)&instances))
                return SUCCESS;

            return ERROR;
            };

        return SUCCESS;
        });
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus ECInstanceKeyFilter::BuildWhereClause(ECClassCR ecClass)
    {
    m_nextBindingIndex = 1;
    m_whereClause = "";
    m_bindArgsCallbacks.clear();

    for (auto& callback : m_whereClauseCallbacks)
        {
        if (SUCCESS != AddWhereClauseSentence(ecClass, callback))
            return ERROR;
        }

    for (auto& callback : m_tmpWhereClauseCallbacks)
        {
        if (SUCCESS != AddWhereClauseSentence(ecClass, callback))
            return ERROR;
        }

    return SUCCESS;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus ECInstanceKeyFilter::AddWhereClauseSentence(ECClassCR ecClass, const WhereClauseCallback& whereClauseCallback)
    {
    Utf8String whereSentence;
    BindArgsCallback bindArgsCallback;
    if (SUCCESS != whereClauseCallback(ecClass, whereSentence, bindArgsCallback))
        return ERROR;

    if (whereSentence.empty())
        return SUCCESS;

    if (!m_whereClause.empty())
        m_whereClause += " AND ";

    m_whereClause += "(" + whereSentence + ")";

    int bindingCount = (int)std::count(whereSentence.begin(), whereSentence.end(), '?');
    if (bindingCount <= 0)
        return SUCCESS;

    m_bindArgsCallbacks.push_back(std::bind(bindArgsCallback, std::placeholders::_1, m_nextBindingIndex));

    m_nextBindingIndex += bindingCount;

    return SUCCESS;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus ECInstanceKeyFilter::ExecuteDbQuery
(
CacheTransactionCR txn,
ECClassCR ecClass,
ECInstanceKeyMultiMap& result,
ICancellationTokenPtr ct
)
    {
    auto sql = Utf8PrintfString(
        "SELECT GetECClassId(), ECInstanceId FROM ONLY %s "
        "WHERE %s",
        ecClass.GetECSqlName().c_str(),
        m_whereClause.c_str());

    if (m_limit > 0)
        sql += Utf8PrintfString(" LIMIT %d", m_limit);

    ECSqlStatement statement;
    if (SUCCESS != txn.GetCache().GetAdapter().PrepareStatement(statement, sql))
        return ERROR;

    for (auto callback : m_bindArgsCallbacks)
        {
        if (SUCCESS != callback(statement))
            return ERROR;
        }

    DbResult status;
    while (DbResult::BE_SQLITE_ROW == (status = statement.Step()))
        {
        result.Insert(statement.GetValueId<ECClassId>(0), statement.GetValueId<ECInstanceId>(1));

        if (ct && ct->IsCanceled())
            return SUCCESS;
        }

    return DbResult::BE_SQLITE_DONE == status ? SUCCESS : ERROR;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void ECInstanceKeyFilter::AddLabelFilter(Utf8StringCR label)
    {
    m_whereClauseCallbacks.push_back([=] (ECClassCR ecClass, Utf8StringR outWhereClause, BindArgsCallback& outBindArgs)
        {
        ECPropertyCP property = ecClass.GetInstanceLabelProperty();
        if (nullptr == property)
            return ERROR;

        Utf8String labelProperty(property->GetName());
        if (labelProperty.empty())
            return ERROR;

        outWhereClause = labelProperty + " = ?";

        outBindArgs = [=] (ECSqlStatement& statement, int firstArgIndex)
            {
            if (ECSqlStatus::Success == statement.BindText(firstArgIndex, label.c_str(), IECSqlBinder::MakeCopy::No))
                return SUCCESS;

            return ERROR;
            };

        return SUCCESS;
        });
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void ECInstanceKeyFilter::AddAnyPropertiesLikeFilter(bset<Utf8String> propertiesToSearch, Utf8StringCR searchTerm)
    {
    AddAnyPropertiesLikeFilter([=] (ECClassCR)
        {
        return propertiesToSearch;
        }, searchTerm);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void ECInstanceKeyFilter::AddAnyPropertiesLikeFilter
(
const std::function<bset<Utf8String>(ECClassCR ecClass)>& propertiesToSearch,
Utf8StringCR searchTerm
)
    {
    m_whereClauseCallbacks.push_back([=] (ECClassCR ecClass, Utf8StringR outWhereClause, BindArgsCallback& outBindArgs)
        {
        auto properties = propertiesToSearch(ecClass);

        for (Utf8StringCR property : properties)
            {
            if (!outWhereClause.empty())
                {
                outWhereClause += " OR ";
                }

            outWhereClause += "[" + property + "] LIKE ?";
            }

        outBindArgs = [=] (ECSqlStatement& statement, int firstArgIndex)
            {
            Utf8String searchValue = "%" + searchTerm + "%";

            for (int i = 0; i < properties.size(); i++)
                {
                if (ECSqlStatus::Success != statement.BindText(firstArgIndex + i, searchValue.c_str(), IECSqlBinder::MakeCopy::Yes))
                    return ERROR;
                }
            return SUCCESS;
            };

        return SUCCESS;
        });
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void ECInstanceKeyFilter::AddWhereClauseFilter(Utf8StringCR whereClause, const BindArgsCallback& bindArgsCallback)
    {
    m_whereClauseCallbacks.push_back([=] (ECClassCR ecClass, Utf8StringR outWhereClause, BindArgsCallback& outBindArgs)
        {
        outWhereClause = whereClause;

        outBindArgs = bindArgsCallback;

        return SUCCESS;
        });
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void ECInstanceKeyFilter::AddWhereClauseFilter(const WhereClauseCallback& whereClauseCallback)
    {
    m_whereClauseCallbacks.push_back(whereClauseCallback);
    }
