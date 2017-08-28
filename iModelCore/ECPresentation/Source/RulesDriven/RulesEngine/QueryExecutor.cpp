/*--------------------------------------------------------------------------------------+
|
|     $Source: Source/RulesDriven/RulesEngine/QueryExecutor.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <ECPresentationPch.h>
#include <ECPresentation/RulesDriven/PresentationManager.h>
#include <ECPresentation/RulesDriven/Rules/SpecificationVisitor.h>
#include "QueryExecutor.h"
#include "QueryContracts.h"
#include "ExtendedData.h"
#include "JsonNavNode.h"
#include "LoggingHelper.h"
#include "ImageHelper.h"
#include <regex>

/*---------------------------------------------------------------------------------**//**
// @bsimethod                                    Grigas.Petraitis                07/2015
+---------------+---------------+---------------+---------------+---------------+------*/
QueryExecutor::QueryExecutor(ECDbR db, ECSqlStatementCache const& statementCache)
    : m_db(db), m_statementCache(statementCache), m_query(nullptr), m_readStarted(false), m_readFinished(false)
    {
    }

/*---------------------------------------------------------------------------------**//**
// @bsimethod                                    Grigas.Petraitis                07/2015
+---------------+---------------+---------------+---------------+---------------+------*/
QueryExecutor::QueryExecutor(ECDbR db, ECSqlStatementCache const& statementCache, PresentationQueryBase const& query)
    : m_db(db), m_statementCache(statementCache), m_query(&query), m_readStarted(false), m_readFinished(false)
    {
    }

/*---------------------------------------------------------------------------------**//**
// @bsimethod                                    Grigas.Petraitis                07/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void QueryExecutor::SetQuery(PresentationQueryBase const& query)
    {
    if (m_query == &query)
        return;
    
    m_query = &query;
    m_queryString.clear();
    }

/*---------------------------------------------------------------------------------**//**
// @bsimethod                                    Grigas.Petraitis                07/2015
+---------------+---------------+---------------+---------------+---------------+------*/
PresentationQueryBase const* QueryExecutor::GetQuery() const {return m_query;}

/*---------------------------------------------------------------------------------**//**
// @bsimethod                                    Grigas.Petraitis                01/2017
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8CP QueryExecutor::GetQueryString() const
    {
    if (m_queryString.empty())
        m_queryString = m_query->ToString();
    return m_queryString.c_str();
    }

/*---------------------------------------------------------------------------------**//**
// @bsimethod                                    Grigas.Petraitis                01/2017
+---------------+---------------+---------------+---------------+---------------+------*/
CachedECSqlStatementPtr QueryExecutor::GetStatement() const
    {
    Utf8CP query = GetQueryString();
    return m_statementCache.GetPreparedStatement(m_db, query);
    }

/*---------------------------------------------------------------------------------**//**
// @bsimethod                                    Grigas.Petraitis                07/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void QueryExecutor::Reset()
    {
    m_readStarted = false;
    m_readFinished = false;

    if (nullptr != m_query)
        {
        CachedECSqlStatementPtr statement = GetStatement();
        if (statement.IsNull())
            BeAssert(false);
        else
            statement->ClearBindings();
        }
    }

/*---------------------------------------------------------------------------------**//**
// @bsimethod                                    Grigas.Petraitis                01/2016
+---------------+---------------+---------------+---------------+---------------+------*/
void QueryExecutor::ReadRecords() const
    {
    if (nullptr == m_query)
        return;
    
    if (m_readFinished)
        return;

    // get the statement
    RefCountedPtr<PerformanceLogger> _l = LoggingHelper::CreatePerformanceLogger(Log::Default, "[QueryExecutor] Getting prepared statement", NativeLogging::LOG_TRACE);
    CachedECSqlStatementPtr statement = GetStatement();
    if (statement.IsNull())
        {
        BeAssert(false);
        return;
        }
    _l = nullptr;
    
    // bind query variables
    _l = LoggingHelper::CreatePerformanceLogger(Log::Default, "[QueryExecutor] Binding query variable values", NativeLogging::LOG_TRACE);
    bvector<BoundQueryValue const*> boundValues = m_query->GetBoundValues();
    for (size_t i = 0; i < boundValues.size(); ++i)
        {
        BoundQueryValue const* value = boundValues[i];
        ECSqlStatus status = value->Bind(*statement, (uint32_t)(i + 1));
        BeAssert(status.IsSuccess());
        }
    _l = nullptr;
            
    // read the records
    m_readStarted = true;
    _l = LoggingHelper::CreatePerformanceLogger(Log::Default, "[QueryExecutor] Reading and caching new records", NativeLogging::LOG_TRACE);
    DbResult result = DbResult::BE_SQLITE_ERROR;
    uint32_t recordsRead = 0;
    while (DbResult::BE_SQLITE_ROW == (result = statement->Step()))
        {
        const_cast<QueryExecutor*>(this)->_ReadRecord(*statement);
        recordsRead++;
        }
    _l = nullptr;

    BeAssert(DbResult::BE_SQLITE_DONE == result);
    m_readFinished = true;
    LoggingHelper::LogMessage(Log::Default, Utf8PrintfString("Finished reading %d records", recordsRead).c_str(), NativeLogging::LOG_TRACE);
    }

/*---------------------------------------------------------------------------------**//**
// @bsimethod                                    Grigas.Petraitis                07/2015
+---------------+---------------+---------------+---------------+---------------+------*/
NavigationQueryExecutor::NavigationQueryExecutor(JsonNavNodesFactory const& nodesFactory, ECDbR db, ECSqlStatementCache const& statementCache)
    : QueryExecutor(db, statementCache), m_nodesFactory(nodesFactory)
    {
    }

/*---------------------------------------------------------------------------------**//**
// @bsimethod                                    Grigas.Petraitis                07/2015
+---------------+---------------+---------------+---------------+---------------+------*/
NavigationQueryExecutor::NavigationQueryExecutor(JsonNavNodesFactory const& nodesFactory, ECDbR db, ECSqlStatementCache const& statementCache, NavigationQuery const& query)
    : QueryExecutor(db, statementCache, query), m_nodesFactory(nodesFactory), m_query(&query)
    {
    m_reader = NavNodeReader::Create(m_nodesFactory, GetDb(), *query.GetContract(), query.GetResultParameters().GetResultType());
    }

/*---------------------------------------------------------------------------------**//**
// @bsimethod                                    Grigas.Petraitis                07/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void NavigationQueryExecutor::SetQuery(NavigationQuery const& query, bool clearCache)
    {
    if (clearCache)
        {
        Reset();
        m_nodes.clear();
        }

    if (m_query != &query)
        {
        m_reader = NavNodeReader::Create(m_nodesFactory, GetDb(), *query.GetContract(), query.GetResultParameters().GetResultType());
        m_query = &query;
        }

    QueryExecutor::SetQuery(query);
    }

/*---------------------------------------------------------------------------------**//**
// @bsimethod                                    Grigas.Petraitis                01/2016
+---------------+---------------+---------------+---------------+---------------+------*/
void NavigationQueryExecutor::_ReadRecord(BeSQLite::EC::ECSqlStatement& statement)
    {
    JsonNavNodePtr tmpNode = m_reader->ReadNode(statement);
    if (!tmpNode.IsValid())
        {
        BeAssert(false);
        return;
        }

    NavNodeExtendedData extendedData(*tmpNode);
    extendedData.MergeWith(m_query->GetResultParameters().GetNavNodeExtendedData());

    m_nodes.push_back(tmpNode);
    }

/*---------------------------------------------------------------------------------**//**
// @bsimethod                                    Grigas.Petraitis                01/2016
+---------------+---------------+---------------+---------------+---------------+------*/
JsonNavNodePtr NavigationQueryExecutor::GetNode(size_t index) const
    {
    ReadRecords();
    if (m_nodes.empty() || index >= m_nodes.size())
        return nullptr;
    return m_nodes[index];
    }

/*---------------------------------------------------------------------------------**//**
// @bsimethod                                    Grigas.Petraitis                01/2016
+---------------+---------------+---------------+---------------+---------------+------*/
size_t NavigationQueryExecutor::GetNodesCount() const
    {
    ReadRecords();
    return m_nodes.size();
    }

/*---------------------------------------------------------------------------------**//**
// @bsimethod                                    Grigas.Petraitis                04/2016
+---------------+---------------+---------------+---------------+---------------+------*/
ContentQueryExecutor::ContentQueryExecutor(ECDbR db, ECSqlStatementCache const& statementCache)
    : QueryExecutor(db, statementCache), m_propertyFormatter(nullptr)
    {
    }

/*---------------------------------------------------------------------------------**//**
// @bsimethod                                    Grigas.Petraitis                04/2016
+---------------+---------------+---------------+---------------+---------------+------*/
ContentQueryExecutor::ContentQueryExecutor(ECDbR db, ECSqlStatementCache const& statementCache, ContentQuery const& query)
    : QueryExecutor(db, statementCache, query), m_query(&query), m_propertyFormatter(nullptr)
    {
    }

/*---------------------------------------------------------------------------------**//**
// @bsimethod                                   Grigas.Petraitis                10/2016
+---------------+---------------+---------------+---------------+---------------+------*/
void ContentQueryExecutor::ClearCache()
    {
    Reset();
    m_records.clear();
    }

/*---------------------------------------------------------------------------------**//**
// @bsimethod                                   Grigas.Petraitis                04/2016
+---------------+---------------+---------------+---------------+---------------+------*/
void ContentQueryExecutor::SetQuery(ContentQuery const& query, bool clearCache)
    {
    if (clearCache)
        ClearCache();

    m_query = &query;
    QueryExecutor::SetQuery(query);
    }

/*---------------------------------------------------------------------------------**//**
// @bsimethod                                    Grigas.Petraitis                04/2016
+---------------+---------------+---------------+---------------+---------------+------*/
ContentSetItemPtr ContentQueryExecutor::GetRecord(size_t index) const
    {
    ReadRecords();
    if (m_records.empty() || index >= m_records.size())
        return nullptr;
    return m_records[index];
    }

/*---------------------------------------------------------------------------------**//**
// @bsimethod                                    Grigas.Petraitis                04/2016
+---------------+---------------+---------------+---------------+---------------+------*/
size_t ContentQueryExecutor::GetRecordsCount() const
    {
    ReadRecords();
    return m_records.size();
    }

/*---------------------------------------------------------------------------------**//**
// @bsimethod                                    Grigas.Petraitis                10/2016
+---------------+---------------+---------------+---------------+---------------+------*/
static rapidjson::Document GetFormattedValue(IECPropertyFormatter const& formatter, PrimitiveECPropertyCR prop, 
    IECSqlValue const& value, rapidjson::MemoryPoolAllocator<>* allocator)
    {
    Utf8String formattedValue;
    if (SUCCESS != formatter.GetFormattedPropertyValue(formattedValue, prop, ValueHelpers::GetECValueFromSqlValue(prop.GetType(), value)))
        return ValueHelpers::GetJsonFromPrimitiveValue(prop.GetType(), value, allocator);
        
    rapidjson::Document json(allocator);
    json.SetString(formattedValue.c_str(), json.GetAllocator());
    return json;
    }

/*---------------------------------------------------------------------------------**//**
// @bsimethod                                    Grigas.Petraitis                10/2016
+---------------+---------------+---------------+---------------+---------------+------*/
static bool IsValueMerged(Utf8CP value)
    {
    static Utf8String s_format;
    if (s_format.empty())
        {
        s_format = "^" CONTENTRECORD_MERGED_VALUE_FORMAT "$";
        s_format.ReplaceAll("*", "\\*");
        s_format.ReplaceAll("%s", ".*?");
        }
    static std::regex s_regex(s_format.c_str());
    return std::regex_search(value, s_regex);
    }

//=======================================================================================
// @bsiclass                                     Grigas.Petraitis                10/2016
//=======================================================================================
struct ContentValueAppender
{
    enum ValueToAdd
        {
        ADD_Value = 1,
        ADD_DisplayValue = 2,
        ADD_Both = ADD_Value | ADD_DisplayValue,
        };

private:
    rapidjson::Document m_values;
    rapidjson::Document m_displayValues;
    bvector<Utf8String> m_mergedFieldNames;

public:
    ContentValueAppender()
        {
        m_values.SetObject();
        m_displayValues.SetObject();
        }
    rapidjson::Document&& GetValues() {return std::move(m_values);}
    rapidjson::Document& GetValuesR() {return m_values;}
    rapidjson::Document&& GetDisplayValues() {return std::move(m_displayValues);}
    rapidjson::Document& GetDisplayValuesR() {return m_displayValues;}
    bvector<Utf8String> GetMergedFieldNames() const {return m_mergedFieldNames;}
    /*---------------------------------------------------------------------------------**//**
    // @bsimethod                                    Grigas.Petraitis                06/2017
    +---------------+---------------+---------------+---------------+---------------+------*/
    void AddValue(Utf8CP name, rapidjson::Value&& value, rapidjson::Value&& displayValue, int add = ADD_Both)
        {
        if (0 != (ADD_Value & add))
            m_values.AddMember(rapidjson::Value(name, m_values.GetAllocator()), value, m_values.GetAllocator());
        if (0 != (ADD_DisplayValue & add))
            m_displayValues.AddMember(rapidjson::Value(name, m_displayValues.GetAllocator()), displayValue, m_displayValues.GetAllocator());
        }
    /*---------------------------------------------------------------------------------**//**
    // @bsimethod                                    Grigas.Petraitis                06/2017
    +---------------+---------------+---------------+---------------+---------------+------*/
    void AddNull(Utf8CP name, int add = ADD_Both)
        {
        AddValue(name, rapidjson::Value(), rapidjson::Value(), add);
        }
    /*---------------------------------------------------------------------------------**//**
    // @bsimethod                                    Grigas.Petraitis                06/2017
    +---------------+---------------+---------------+---------------+---------------+------*/
    void AddValue(Utf8CP name, Utf8CP value, Utf8CP displayValue, bool isMerged, int add = ADD_Both)
        {
        if (nullptr == value)
            AddNull(name, add);
        else
            AddValue(name, rapidjson::Value(value, m_values.GetAllocator()), rapidjson::Value(displayValue, m_displayValues.GetAllocator()), add);

        if (isMerged)
            m_mergedFieldNames.push_back(name);
        }
    /*---------------------------------------------------------------------------------**//**
    // @bsimethod                                    Grigas.Petraitis                06/2017
    +---------------+---------------+---------------+---------------+---------------+------*/
    void AddValue(Utf8CP name, ECPropertyCR ecProperty, IECSqlValue const& value, IECPropertyFormatter const* propertyFormatter, bool possiblyMerged)
        {
        if (value.IsNull())
            {
            AddNull(name, ecProperty.GetIsNavigation() ? ADD_DisplayValue : ADD_Both);
            return;
            }

        if (possiblyMerged && IsValueMerged(value.GetText()))
            {
            AddValue(name, rapidjson::Value(value.GetText(), m_values.GetAllocator()), rapidjson::Value(value.GetText(), m_displayValues.GetAllocator()));
            m_mergedFieldNames.push_back(name);
            }
        else if (ecProperty.GetIsPrimitive())
            {
            PrimitiveECPropertyCR primitiveProperty = *ecProperty.GetAsPrimitiveProperty();
            rapidjson::Document valueJson = ValueHelpers::GetJsonFromPrimitiveValue(primitiveProperty.GetType(), value, &m_values.GetAllocator());
            rapidjson::Document formattedValueJson(&m_displayValues.GetAllocator());
            if (nullptr != propertyFormatter)
                formattedValueJson = GetFormattedValue(*propertyFormatter, primitiveProperty, value, &m_displayValues.GetAllocator());
            else
                formattedValueJson.CopyFrom(valueJson, formattedValueJson.GetAllocator());
            AddValue(name, std::move(valueJson), std::move(formattedValueJson));
            }
        else if (ecProperty.GetIsStruct())
            {
            rapidjson::Document valueJson = ValueHelpers::GetJsonFromStructValue(ecProperty.GetAsStructProperty()->GetType(), value, &m_values.GetAllocator());
            rapidjson::Document formattedValueJson(&m_displayValues.GetAllocator());
            formattedValueJson.CopyFrom(valueJson, formattedValueJson.GetAllocator()); // no formatting for struct values
            AddValue(name, std::move(valueJson), std::move(formattedValueJson));
            }
        else if (ecProperty.GetIsArray())
            {
            rapidjson::Document valueJson = ValueHelpers::GetJsonFromArrayValue(value, &m_values.GetAllocator());
            rapidjson::Document formattedValueJson(&m_displayValues.GetAllocator());
            formattedValueJson.CopyFrom(valueJson, formattedValueJson.GetAllocator()); // no formatting for array values
            AddValue(name, std::move(valueJson), std::move(formattedValueJson));
            }
        else if (ecProperty.GetIsNavigation())
            {
            AddValue(name, rapidjson::Value(), rapidjson::Value(value.GetText(), m_displayValues.GetAllocator()), ADD_DisplayValue);
            }
        else
            {
            // if the property is not primitive, append NULL
            AddNull(name);
            }
        }

    /*---------------------------------------------------------------------------------**//**
    // @bsimethod                                   Saulius.Skliutas                08/2017
    +---------------+---------------+---------------+---------------+---------------+------*/
    void SetNavigationPropertyValue(Utf8CP name, IECSqlValue const& value)
        {
        rapidjson::Value jsonValue;
        if (!value.IsNull())
            jsonValue.SetInt64(value.GetInt64());
        AddValue(name, std::move(jsonValue), rapidjson::Value(), ADD_Value);
        }
};

//=======================================================================================
// @bsiclass                                     Grigas.Petraitis               06/2017
//=======================================================================================
struct FieldValueInstanceKeyReader
{
private:
    bmap<ContentDescriptor::ECPropertiesField const*, bvector<ECInstanceKey>> m_relatedFieldKeys;
    bmap<ContentDescriptor::ECPropertiesField const*, int> m_fieldProperties;
    bvector<ECInstanceKey> const& m_primaryKeys;

private:
    /*---------------------------------------------------------------------------------**//**
    // @bsimethod                                    Grigas.Petraitis               06/2017
    +---------------+---------------+---------------+---------------+---------------+------*/
    void ReadECInstanceKey(RapidJsonValueCR json, bvector<ContentDescriptor::ECPropertiesField const*> const& keyFields)
        {
        ECClassId classId(json["ECClassId"].GetUint64());
        ECInstanceId instanceId(json["ECInstanceId"].GetUint64());
        ECInstanceKey key(classId, instanceId);
        for (ContentDescriptor::ECPropertiesField const* field : keyFields)
            m_relatedFieldKeys[field].push_back(key);
        }

public:    
    /*---------------------------------------------------------------------------------**//**
    // @bsimethod                                    Grigas.Petraitis               06/2017
    +---------------+---------------+---------------+---------------+---------------+------*/
    FieldValueInstanceKeyReader(bvector<ECInstanceKey> const& primaryKeys) : m_primaryKeys(primaryKeys) {}

    /*---------------------------------------------------------------------------------**//**
    // @bsimethod                                    Grigas.Petraitis               06/2017
    +---------------+---------------+---------------+---------------+---------------+------*/
    void ReadFieldKeys(ContentDescriptor::ECInstanceKeyField const& field, ECSqlStatement& statement, int columnIndex)
        {        
        if (statement.IsValueNull(columnIndex))
            return;

        Utf8CP serializedKeys = statement.GetValueText(columnIndex);
        rapidjson::Document jsonKeys;
        jsonKeys.Parse(serializedKeys);
        if (jsonKeys.IsNull())
            {
            BeAssert(false);
            return;
            }

        if (jsonKeys.IsObject())
            {
            ReadECInstanceKey(jsonKeys, field.GetKeyFields());
            }
        else if (jsonKeys.IsArray())
            {
            for (rapidjson::SizeType i = 0; i < jsonKeys.Size(); ++i)
                ReadECInstanceKey(jsonKeys[i], field.GetKeyFields());
            }
        else
            {
            BeAssert(false);
            }
        }
    
    /*---------------------------------------------------------------------------------**//**
    // @bsimethod                                    Grigas.Petraitis               06/2017
    +---------------+---------------+---------------+---------------+---------------+------*/
    void SetFieldProperty(ContentDescriptor::ECPropertiesField const& field, ContentDescriptor::Property const& prop, bool isMerged)
        {
        if (isMerged)
            {
            m_fieldProperties[&field] = -1;
            return;
            }

        size_t propertyIndex;
        for (propertyIndex = 0; propertyIndex < field.GetProperties().size(); ++propertyIndex)
            {
            if (&field.GetProperties()[propertyIndex] == &prop)
                break;
            }
        BeAssert(propertyIndex < field.GetProperties().size());
        m_fieldProperties[&field] = (int)propertyIndex;
        }
    
    /*---------------------------------------------------------------------------------**//**
    // @bsimethod                                    Grigas.Petraitis               06/2017
    +---------------+---------------+---------------+---------------+---------------+------*/
    ContentSetItem::FieldPropertyInstanceKeyMap GetKeys() const
        {
        ContentSetItem::FieldPropertyInstanceKeyMap keys;
        for (auto pair : m_fieldProperties)
            {
            if (-1 == pair.second)
                {
                // note: -1 means any property - in this case we iterate through all field properties and 
                // see whether any ECInstaceKey matches that property
                bvector<ECInstanceKey> const* sourceVector = &m_primaryKeys;
                auto relatedFieldKeyIter = m_relatedFieldKeys.find(pair.first);
                if (m_relatedFieldKeys.end() != relatedFieldKeyIter)
                    sourceVector = &relatedFieldKeyIter->second;

                size_t propertyIndex = 0;
                for (ContentDescriptor::Property const& p : pair.first->GetProperties())
                    {
                    ContentSetItem::FieldProperty fieldProperty(*pair.first, propertyIndex++);
                    for (ECInstanceKeyCR key : *sourceVector)
                        {
                        if (p.GetPropertyClass().GetId() == key.GetClassId())
                            keys[fieldProperty].push_back(key);
                        }
                    }
                }
            else
                {
                // here we know exactly which property was changed, so just copy all ECInstanceKeys in that field
                // to the output map
                ContentSetItem::FieldProperty fieldProperty(*pair.first, pair.second);
                bvector<ECInstanceKey> const* sourceVector = &m_primaryKeys;
                auto relatedFieldKeyIter = m_relatedFieldKeys.find(&fieldProperty.GetField());
                if (m_relatedFieldKeys.end() != relatedFieldKeyIter)
                    sourceVector = &relatedFieldKeyIter->second;
                std::copy(sourceVector->begin(), sourceVector->end(), std::back_inserter(keys[fieldProperty]));
                }
            }
        return keys;
        }
};

/*---------------------------------------------------------------------------------**//**
// @bsimethod                                    Grigas.Petraitis                04/2016
+---------------+---------------+---------------+---------------+---------------+------*/
void ContentQueryExecutor::_ReadRecord(ECSqlStatement& statement)
    {
    ContentDescriptorCR descriptor = m_query->GetContract()->GetDescriptor();
    bool resultsMerged = (0 != ((int)ContentFlags::MergeResults & descriptor.GetContentFlags()));
    int columnIndex = 0;

    uint64_t contractId = statement.GetValueUInt64(columnIndex++);

    bvector<ECInstanceKey> primaryRecordKeys = ValueHelpers::GetECInstanceKeysFromSerializedJson(statement.GetValueText(columnIndex++));
    BeAssert(1 == primaryRecordKeys.size() || resultsMerged);
    
    ContentValueAppender values;
    FieldValueInstanceKeyReader fieldValueInstanceKeyReader(primaryRecordKeys);
    Utf8String displayLabel, imageId;
    ECClassCP recordClass = nullptr;
    if (0 == ((int)ContentFlags::KeysOnly & descriptor.GetContentFlags()))
        {
        for (ECInstanceKeyCR primaryKey : primaryRecordKeys)
            {
            if (nullptr == recordClass)
                recordClass = statement.GetECDb()->Schemas().GetClass(primaryKey.GetClassId());
            else if (recordClass->GetId() != primaryKey.GetClassId())
                {
                recordClass = nullptr;
                break;
                }
            }

        bool possiblyMerged = (resultsMerged && primaryRecordKeys.size() > 1);
        for (ContentDescriptor::Field const* field : descriptor.GetAllFields())
            {
            Utf8StringCR fieldName = field->GetName();
            if (field->IsDisplayLabelField())
                {
                // if this is a display label field, set the display label and also append the value
                displayLabel = statement.GetValueText(columnIndex);
                Utf8CP value = statement.GetValueText(columnIndex);
                values.AddValue(fieldName.c_str(), value, value, (possiblyMerged && IsValueMerged(value)));
                }
            else if (field->IsCalculatedPropertyField())
                {
                // if this is a calculated field, just append the value
                Utf8CP value = statement.GetValueText(columnIndex);
                values.AddValue(fieldName.c_str(), value, value, (possiblyMerged && IsValueMerged(value)));
                }
            else if (field->IsPropertiesField())
                {
                ContentDescriptor::ECPropertiesField const& propertiesField = *field->AsPropertiesField();
                ContentDescriptor::Property const* matchingProperty = nullptr;
                if (resultsMerged)
                    {
                    if (!propertiesField.GetProperties().empty())
                        matchingProperty = &propertiesField.GetProperties().front();
                    }
                else
                    {
                    // find the field property that's appropriate for this instance
                    matchingProperty = m_query->GetContract(contractId)->FindMatchingProperty(propertiesField, recordClass);
                    }

                if (nullptr != matchingProperty)
                    {
                    values.AddValue(fieldName.c_str(), matchingProperty->GetProperty(), statement.GetValue(columnIndex), m_propertyFormatter, possiblyMerged);
                    fieldValueInstanceKeyReader.SetFieldProperty(propertiesField, *matchingProperty, possiblyMerged);
                    }
                else
                    {
                    // if the property was not found, append NULL
                    int addFlags = propertiesField.GetProperties().front().GetProperty().GetIsNavigation() ? ContentValueAppender::ADD_DisplayValue : ContentValueAppender::ADD_Both;
                    values.AddNull(fieldName.c_str(), addFlags);
                    }
                }
            else if (field->IsSystemField() && field->AsSystemField()->IsECInstanceKeyField())
                {
                fieldValueInstanceKeyReader.ReadFieldKeys(*field->AsSystemField()->AsECInstanceKeyField(), statement, columnIndex);
                }
            else if (field->IsSystemField() && field->AsSystemField()->IsECNavigationInstanceIdField())
                {
                // if this is ECNavigationInstanceIdField, update value for ECPropertiesField
                Utf8String propertiesFieldName = field->AsSystemField()->AsECNavigationInstanceIdField()->GetPropertiesField().GetName();
                values.SetNavigationPropertyValue(propertiesFieldName.c_str(), statement.GetValue(columnIndex));
                }
            else
                {
                // do not increase columnIndex if we didn't use the field
                continue;
                }
            columnIndex++;
            }

        if (descriptor.ShowImages() && nullptr != recordClass)
            imageId = ImageHelper::GetImageId(*recordClass, true, false);
        }

    ContentSetItemPtr record = ContentSetItem::Create(primaryRecordKeys, displayLabel, imageId, 
        values.GetValues(), values.GetDisplayValues(), values.GetMergedFieldNames(), fieldValueInstanceKeyReader.GetKeys());
    record->SetClass(recordClass);

    m_records.push_back(record);
    }

/*---------------------------------------------------------------------------------**//**
// @bsimethod                                    Grigas.Petraitis                05/2016
+---------------+---------------+---------------+---------------+---------------+------*/
size_t CountQueryExecutor::GetResult() const
    {
    ReadRecords();
    return m_result;
    }

/*---------------------------------------------------------------------------------**//**
// @bsimethod                                    Grigas.Petraitis                05/2016
+---------------+---------------+---------------+---------------+---------------+------*/
void CountQueryExecutor::_ReadRecord(BeSQLite::EC::ECSqlStatement& statement)
    {
    m_result = (size_t)statement.GetValueInt64(0);
    }
