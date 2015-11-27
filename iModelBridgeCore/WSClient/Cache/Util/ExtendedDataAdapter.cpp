/*--------------------------------------------------------------------------------------+
|
|     $Source: Cache/Util/ExtendedDataAdapter.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#include <WebServices/Cache/Util/ExtendedDataAdapter.h>

#include <WebServices/Cache/Util/ECDbHelper.h>

USING_NAMESPACE_BENTLEY_SQLITE_EC
USING_NAMESPACE_BENTLEY_WEBSERVICES

Utf8CP SCHEMA_ExtendedData_XML = R"xml(<?xml version="1.0" encoding="utf-8"?>
<ECSchema schemaName="ExtendedData" nameSpacePrefix="ExtendedData" version="01.00" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.2.0">
    <ECSchemaReference name="Bentley_Standard_Classes" version="01.00" prefix="bsm" />
    <ECClass typeName="ExtendedData" isDomainClass="True">
        <ECProperty propertyName="Content" typeName="string" />
    </ECClass>
<!-- WIP_ANYCLASS_REFACTORING   <ECRelationshipClass typeName="ExtendedDataRelationship" isDomainClass="True" strength="embedding" strengthDirection="forward">
        <Source cardinality="(1,1)" polymorphic="True">
            <Class class="bsm:AnyClass" />
        </Source>
        <Target cardinality="(0,1)" polymorphic="True">
            <Class class="ExtendedData" />
        </Target>
    </ECRelationshipClass> -->
</ECSchema>)xml";

#define SCHEMA_ExtendedData                         "ExtendedData"
#define CLASS_ExtendedData                          "ExtendedData"
#define CLASS_ExtendedData_PROPERTY_Content         "Content"
#define CLASS_ExtendedDataRelationship              "ExtendedDataRelationship"

#define ECSql_CLASS_ExtendedData                    "[ExtendedData].[ExtendedData]"
#define ECSql_CLASS_ExtendedDataRelationship        "[ExtendedData].[ExtendedDataRelationship]"
#define ECSql_ExtendedData_PROPERTY_Content         "[Content]"

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    03/2015
+---------------+---------------+---------------+---------------+---------------+------*/
ExtendedDataAdapter::ExtendedDataAdapter(ObservableECDb& db) :
m_dbAdapter(db),
m_statementCache(db)
    {}

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    03/2015
+---------------+---------------+---------------+---------------+---------------+------*/
ExtendedData ExtendedDataAdapter::GetData(ECInstanceKeyCR instanceKey)
    {
    ECClassCP extendedDataClass = m_dbAdapter.GetECClass(SCHEMA_ExtendedData, CLASS_ExtendedData);
    if (nullptr == extendedDataClass)
        {
        // Shema not imported, return data ready for modification and update
        return ExtendedData(instanceKey, ECInstanceKey(), nullptr);
        }

    auto statement = m_statementCache.GetPreparedStatement("GetData", [&]
        {
        return
            "SELECT data.ECInstanceId, data.[" CLASS_ExtendedData_PROPERTY_Content "] "
            "FROM " ECSql_CLASS_ExtendedData " data "
            "JOIN " ECSql_CLASS_ExtendedDataRelationship " relationship ON relationship.TargetECInstanceId = data.ECInstanceId "
            "WHERE relationship.SourceECClassId = ? AND relationship.SourceECInstanceId = ? "
            "LIMIT 1 ";
        });

    statement->BindInt64(1, instanceKey.GetECClassId());
    statement->BindId(2, instanceKey.GetECInstanceId());

    ECInstanceId extendedDataId;
    Utf8String content;

    DbResult status;
    if (BE_SQLITE_ROW == (status = statement->Step()))
        {
        extendedDataId = statement->GetValueId<ECInstanceId>(0);
        content = statement->GetValueText(1);
        }

    auto extendedDataJson = std::make_shared<Json::Value>(Json::objectValue);
    Json::Reader::Parse(content, *extendedDataJson);

    return ExtendedData(instanceKey, {extendedDataClass->GetId(), extendedDataId}, extendedDataJson);
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    03/2015
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus ExtendedDataAdapter::UpdateData(ExtendedData& data)
    {
    ECClassCP extendedDataClass = m_dbAdapter.GetECClass(SCHEMA_ExtendedData, CLASS_ExtendedData);
    if (nullptr == extendedDataClass)
        {
        BeAssert(false && "Call ExtendedDataAdapter::ImportSchema() first!");
        return ERROR;
        }

    Utf8String content = data.m_extendedData->toStyledString();

    if (data.m_extendedDataKey.IsValid())
        {
        auto statement = m_statementCache.GetPreparedStatement("UpdateData", [&]
            {
            return
                "UPDATE ONLY " ECSql_CLASS_ExtendedData " "
                "SET " ECSql_ExtendedData_PROPERTY_Content " = ? "
                "WHERE ECInstanceId = ? ";
            });

        statement->BindText(1, content.c_str(), IECSqlBinder::MakeCopy::No);
        statement->BindId(2, data.m_extendedDataKey.GetECInstanceId());

        DbResult status;
        if (BE_SQLITE_DONE != (status = statement->Step()))
            {
            return ERROR;
            }
        return SUCCESS;
        }

    auto statement = m_statementCache.GetPreparedStatement("InsertData", [&]
        {
        return "INSERT INTO " ECSql_CLASS_ExtendedData " (" ECSql_ExtendedData_PROPERTY_Content ") VALUES (?) ";
        });

    statement->BindText(1, content.c_str(), IECSqlBinder::MakeCopy::No);

    DbResult status;
    if (BE_SQLITE_DONE != (status = statement->Step(data.m_extendedDataKey)))
        {
        return ERROR;
        }

    ECRelationshipClassCP relClass = m_dbAdapter.GetECRelationshipClass(SCHEMA_ExtendedData, CLASS_ExtendedDataRelationship);
    if (!m_dbAdapter.RelateInstances(relClass, data.m_instanceKey, data.m_extendedDataKey).IsValid())
        {
        return ERROR;
        }

    return SUCCESS;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    03/2015
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus ExtendedDataAdapter::ImportSchema()
    {
    auto context = ECSchemaReadContext::CreateContext();

    ECSchemaPtr schema;
    if (SchemaReadStatus::SCHEMA_READ_STATUS_Success != ECSchema::ReadFromXmlString(schema, SCHEMA_ExtendedData_XML, *context))
        {
        return ERROR;
        }

    auto cache = ECSchemaCache::Create();
    cache->AddSchema(*schema);

    if (SUCCESS != m_dbAdapter.GetECDb().Schemas().ImportECSchemas(*cache))
        {
        return ERROR;
        }

    m_dbAdapter.GetECDb().NotifyOnSchemaChangedListeners();
    return SUCCESS;
    }
