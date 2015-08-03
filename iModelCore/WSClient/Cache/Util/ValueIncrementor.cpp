/*--------------------------------------------------------------------------------------+
 |
 |     $Source: Cache/Util/ValueIncrementor.cpp $
 |
 |  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
 |
 +--------------------------------------------------------------------------------------*/

#include <WebServices/Cache/Util/ValueIncrementor.h>

USING_NAMESPACE_EC
USING_NAMESPACE_BENTLEY_SQLITE

USING_NAMESPACE_BENTLEY_WEBSERVICES

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    01/2014
+---------------+---------------+---------------+---------------+---------------+------*/
ValueIncrementor::ValueIncrementor(ECDb& db, ECSqlStatementCache& m_statementCache, ECPropertyCR ecProperty) :
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
    ECClassP propertyClass = nullptr;
    m_db->GetEC().GetSchemaManager().GetECClass(propertyClass, m_propertyClassId);

    Utf8PrintfString key("ValueIncrementor::IncrementWithoutSaving:%lld:%s", m_propertyClassId, m_propertyName.c_str());
    auto statement = m_statementCache->GetPreparedStatement(key, [&]
        {
        ECSqlSelectBuilder builder;
        builder.SelectAll().From(*propertyClass, false).Limit("1");
        return builder.ToString();
        });

    Json::Value instance;
    if (ECSqlStepStatus::HasRow == statement->Step())
        {
        JsonECSqlSelectAdapter adapter(*statement, JsonECSqlSelectAdapter::FormatOptions(ECValueFormat::RawNativeValues));
        if (!adapter.GetRowInstance(instance, m_propertyClassId))
            {
            return ERROR;
            }
        }

    if (instance.isNull())
        {
        instance[m_propertyName] = "0";
        JsonInserter inserter(*m_db, *propertyClass);
        if (SUCCESS != inserter.Insert(instance))
            {
            return ERROR;
            }
        }

    int64_t value = BeJsonUtilities::Int64FromValue(instance[m_propertyName]);
    if (LLONG_MAX == value)
        {
        BeAssert(false);
        return ERROR;
        }
    value++;
    instance[m_propertyName] = BeJsonUtilities::StringValueFromInt64(value);

    JsonUpdater updater(*m_db, *propertyClass);
    if (SUCCESS != updater.Update(instance))
        {
        return ERROR;
        }

    valueOut = instance[m_propertyName].asString();
    return SUCCESS;
    }
