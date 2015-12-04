/*--------------------------------------------------------------------------------------+
 |
 |     $Source: Cache/Util/ValueIncrementor.cpp $
 |
 |  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
 |
 +--------------------------------------------------------------------------------------*/

#include <WebServices/Cache/Util/ValueIncrementor.h>

USING_NAMESPACE_BENTLEY_EC
USING_NAMESPACE_BENTLEY_SQLITE

USING_NAMESPACE_BENTLEY_WEBSERVICES

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    01/2014
+---------------+---------------+---------------+---------------+---------------+------*/
ValueIncrementor::ValueIncrementor
(
ECDb& db, 
WebServices::ECSqlStatementCache& m_statementCache,
ECPropertyCR ecProperty
) :
m_db(&db),
m_statementCache(&m_statementCache),
m_propertyClassId(ecProperty.GetClass().GetId()),
m_propertyName(ecProperty.GetName())
    {}

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    01/2014
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus ValueIncrementor::IncrementWithoutSaving(Utf8StringR valueOut)
    {
    ECClassCP propertyClass = m_db->Schemas().GetECClass(m_propertyClassId);

    Utf8PrintfString key("ValueIncrementor::IncrementWithoutSaving:%lld:%s", m_propertyClassId, m_propertyName.c_str());
    auto statement = m_statementCache->GetPreparedStatement(key, [&]
        {
        return 
            "SELECT ECInstanceId, [" + m_propertyName + "] "
            "FROM ONLY " + ECSqlBuilder::ToECSqlSnippet(*propertyClass) + " LIMIT 1 ";
        });

    ECInstanceId instanceId;
    int64_t value = 0;
    if (BE_SQLITE_ROW == statement->Step())
        {
        instanceId = statement->GetValueId<ECInstanceId>(0);
        value = statement->GetValueInt64(1);
        }

    if (!instanceId.IsValid())
        {
        Utf8String ecsql = 
            "INSERT INTO " + ECSqlBuilder::ToECSqlSnippet(*propertyClass) + " "
            "([" + m_propertyName + "]) VALUES (1) ";

        ECSqlStatement statement;
        statement.Prepare(*m_db, ecsql.c_str());
        if (BE_SQLITE_OK != statement.Step())
            {
            return ERROR;
            }

        valueOut = "1";
        return SUCCESS;
        }

    if (LLONG_MAX == value)
        {
        BeAssert(false);
        return ERROR;
        }

    value++;

    statement = m_statementCache->GetPreparedStatement(key + "Update", [&]
        {
        return
            "UPDATE ONLY " + ECSqlBuilder::ToECSqlSnippet(*propertyClass) + " "
            "SET [" + m_propertyName + "] = ? ";
        });

    statement->BindInt64(1, value);
    if (BE_SQLITE_OK != statement->Step())
        {
        return ERROR;
        }

    valueOut.Sprintf("%lld", value);

    return SUCCESS;
    }
