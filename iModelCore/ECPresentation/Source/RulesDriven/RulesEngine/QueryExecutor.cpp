/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include <ECPresentationPch.h>
#include <ECPresentation/RulesDriven/PresentationManager.h>
#include <ECPresentation/RulesDriven/Rules/SpecificationVisitor.h>
#include <ECPresentation/LabelDefinition.h>
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
QueryExecutor::QueryExecutor(IConnectionCR connection)
    : m_connection(connection), m_query(nullptr), m_readStarted(false), m_readFinished(false)
    {
    }

/*---------------------------------------------------------------------------------**//**
// @bsimethod                                    Grigas.Petraitis                07/2015
+---------------+---------------+---------------+---------------+---------------+------*/
QueryExecutor::QueryExecutor(IConnectionCR connection, PresentationQueryBase const& query)
    : m_connection(connection), m_query(&query), m_readStarted(false), m_readFinished(false)
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
    return m_connection.GetStatementCache().GetPreparedStatement(GetConnection().GetECDb().Schemas(), GetConnection().GetDb(), query);
    }

/*---------------------------------------------------------------------------------**//**
// @bsimethod                                    Grigas.Petraitis                07/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void QueryExecutor::_Reset()
    {
    if (nullptr != m_query && m_readStarted)
        {
        CachedECSqlStatementPtr statement = GetStatement();
        if (statement.IsValid())
            statement->ClearBindings();
        }

    m_readStarted = false;
    m_readFinished = false;
    }

//#define RULES_ENGINE_MEASURE_QUERY_PERFORMANCE 1
/*---------------------------------------------------------------------------------**//**
// @bsimethod                                    Grigas.Petraitis                01/2016
+---------------+---------------+---------------+---------------+---------------+------*/
void QueryExecutor::ReadRecords(ICancelationTokenCP cancelationToken)
    {
    if (nullptr == m_query)
        return;

    if (m_readFinished)
        return;

    // get the statement
    RefCountedPtr<PerformanceLogger> _l = LoggingHelper::CreatePerformanceLogger(Log::Default, "[QueryExecutor] Getting prepared statement", NativeLogging::LOG_TRACE);

    Savepoint txn(m_connection.GetDb(), "QueryExecutor::ReadRecords");
    BeAssert(txn.IsActive());

    CachedECSqlStatementPtr statement = GetStatement();
    if (statement.IsNull())
        {
        if (nullptr != cancelationToken && cancelationToken->IsCanceled())
            {
            LoggingHelper::LogMessage(Log::Default, "[QueryExecutor] Records read canceled while preparing statement", NativeLogging::LOG_TRACE);
            return;
            }
        LoggingHelper::LogMessage(Log::Default, Utf8PrintfString("[QueryExecutor] Failed to prepare query (%s): '%s'",
            m_connection.GetDb().GetLastError().c_str(), m_queryString.c_str()).c_str(), NativeLogging::LOG_ERROR);
        BeAssert(false);
        return;
        }
    _l = nullptr;

    // bind query variables
    _l = LoggingHelper::CreatePerformanceLogger(Log::Default, "[QueryExecutor] Binding query variable values", NativeLogging::LOG_TRACE);
    m_query->BindValues(*statement);
    _l = nullptr;

    if (nullptr != cancelationToken && cancelationToken->IsCanceled())
        {
        LoggingHelper::LogMessage(Log::Default, "[QueryExecutor] Records read canceled before started reading", NativeLogging::LOG_TRACE);
        return;
        }

    // read the records
    m_readStarted = true;
    _l = LoggingHelper::CreatePerformanceLogger(Log::Default, "[QueryExecutor] Reading and caching new records", NativeLogging::LOG_TRACE);
    DbResult result = DbResult::BE_SQLITE_ERROR;
    uint32_t recordsRead = 0;
#ifdef RULES_ENGINE_MEASURE_QUERY_PERFORMANCE
    uint64_t startTime = BeTimeUtilities::GetCurrentTimeAsUnixMillis();
#endif
    while (DbResult::BE_SQLITE_ROW == (result = statement->Step()))
        {
#ifdef RULES_ENGINE_MEASURE_QUERY_PERFORMANCE
        if (0 == recordsRead)
            {
            uint64_t elapsedTime = BeTimeUtilities::GetCurrentTimeAsUnixMillis() - startTime;
            LoggingHelper::LogMessage(Log::Default, Utf8PrintfString("[QueryExecutor] First step took %" PRIu64 " ms", elapsedTime).c_str(), NativeLogging::LOG_ERROR);
            Utf8String plan = m_connection.GetDb().ExplainQuery(statement->GetNativeSql());
            LoggingHelper::LogMessage(Log::Default, Utf8PrintfString("[QueryExecutor] Query plan: %s", plan.c_str()).c_str(), NativeLogging::LOG_ERROR);
            }
#endif
        _ReadRecord(*statement);
        recordsRead++;

        if (nullptr != cancelationToken && cancelationToken->IsCanceled())
            {
            Reset();
            LoggingHelper::LogMessage(Log::Default, "[QueryExecutor] Records read canceled while reading records", NativeLogging::LOG_TRACE);
            return;
            }
        }
    _l = nullptr;

    switch (result)
        {
        case BE_SQLITE_DONE:
            m_readFinished = true;
            LoggingHelper::LogMessage(Log::Default, Utf8PrintfString("[QueryExecutor] Finished reading %d records", recordsRead).c_str(), NativeLogging::LOG_TRACE);
            break;
        case BE_SQLITE_INTERRUPT:
            Reset();
            LoggingHelper::LogMessage(Log::Default, "[QueryExecutor] Records read interrupted", NativeLogging::LOG_TRACE);
            break;
        default:
            BeAssert(false && "Unexpected result");
        }
    }

/*---------------------------------------------------------------------------------**//**
// @bsimethod                                    Grigas.Petraitis                07/2015
+---------------+---------------+---------------+---------------+---------------+------*/
NavigationQueryExecutor::NavigationQueryExecutor(JsonNavNodesFactory const& nodesFactory, IConnectionCR connection, Utf8StringCR locale)
    : QueryExecutor(connection), m_nodesFactory(nodesFactory), m_locale(locale)
    {
    }

/*---------------------------------------------------------------------------------**//**
// @bsimethod                                    Grigas.Petraitis                07/2015
+---------------+---------------+---------------+---------------+---------------+------*/
NavigationQueryExecutor::NavigationQueryExecutor(JsonNavNodesFactory const& nodesFactory, IConnectionCR connection, Utf8StringCR locale, NavigationQuery const& query)
    : QueryExecutor(connection, query), m_nodesFactory(nodesFactory), m_query(&query), m_locale(locale)
    {
    m_reader = NavNodeReader::Create(m_nodesFactory, GetConnection(), m_locale, *query.GetContract(), query.GetResultParameters().GetResultType());
    }

/*---------------------------------------------------------------------------------**//**
// @bsimethod                                    Grigas.Petraitis                07/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void NavigationQueryExecutor::SetQuery(NavigationQuery const& query, bool clearCache)
    {
    if (clearCache)
        _Reset();

    if (m_query != &query)
        {
        m_reader = NavNodeReader::Create(m_nodesFactory, GetConnection(), m_locale, *query.GetContract(), query.GetResultParameters().GetResultType());
        m_query = &query;
        }

    QueryExecutor::SetQuery(query);
    }

/*---------------------------------------------------------------------------------**//**
// @bsimethod                                   Grigas.Petraitis                10/2016
+---------------+---------------+---------------+---------------+---------------+------*/
void NavigationQueryExecutor::_Reset()
    {
    QueryExecutor::_Reset();
    m_nodes.clear();
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
    if (m_nodes.empty() || index >= m_nodes.size())
        return nullptr;
    return m_nodes[index];
    }

/*---------------------------------------------------------------------------------**//**
// @bsimethod                                    Grigas.Petraitis                01/2016
+---------------+---------------+---------------+---------------+---------------+------*/
size_t NavigationQueryExecutor::GetNodesCount() const
    {
    return m_nodes.size();
    }

/*---------------------------------------------------------------------------------**//**
// @bsimethod                                    Grigas.Petraitis                04/2016
+---------------+---------------+---------------+---------------+---------------+------*/
ContentQueryExecutor::ContentQueryExecutor(IConnectionCR connection)
    : QueryExecutor(connection), m_propertyFormatter(nullptr)
    {
    }

/*---------------------------------------------------------------------------------**//**
// @bsimethod                                    Grigas.Petraitis                04/2016
+---------------+---------------+---------------+---------------+---------------+------*/
ContentQueryExecutor::ContentQueryExecutor(IConnectionCR connection, ContentQuery const& query)
    : QueryExecutor(connection, query), m_query(&query), m_propertyFormatter(nullptr)
    {
    }

/*---------------------------------------------------------------------------------**//**
// @bsimethod                                   Grigas.Petraitis                10/2016
+---------------+---------------+---------------+---------------+---------------+------*/
void ContentQueryExecutor::_Reset()
    {
    QueryExecutor::_Reset();
    m_records.clear();
    m_readKeys.clear();
    }

/*---------------------------------------------------------------------------------**//**
// @bsimethod                                   Grigas.Petraitis                04/2016
+---------------+---------------+---------------+---------------+---------------+------*/
void ContentQueryExecutor::SetQuery(ContentQuery const& query, bool clearCache)
    {
    if (clearCache)
        _Reset();

    m_query = &query;
    QueryExecutor::SetQuery(query);
    }

/*---------------------------------------------------------------------------------**//**
// @bsimethod                                    Grigas.Petraitis                04/2016
+---------------+---------------+---------------+---------------+---------------+------*/
ContentSetItemPtr ContentQueryExecutor::GetRecord(size_t index) const
    {
    if (m_records.empty() || index >= m_records.size())
        return nullptr;
    return m_records[index];
    }

/*---------------------------------------------------------------------------------**//**
// @bsimethod                                    Grigas.Petraitis                04/2016
+---------------+---------------+---------------+---------------+---------------+------*/
size_t ContentQueryExecutor::GetRecordsCount() const
    {
    return m_records.size();
    }

/*---------------------------------------------------------------------------------**//**
// @bsimethod                                    Grigas.Petraitis                10/2016
+---------------+---------------+---------------+---------------+---------------+------*/
static bool IsValueMerged(Utf8CP value)
    {
    static std::once_flag s_formatOnceFlag;
    static Utf8String s_format;
    std::call_once(s_formatOnceFlag, []()
        {
        s_format = "^" CONTENTRECORD_MERGED_VALUE_FORMAT "$";
        s_format.ReplaceAll("*", "\\*");
        s_format.ReplaceAll("%s", ".*?");
        });
    static std::regex s_regex(s_format.c_str());
    return std::regex_search(value, s_regex);
    }

/*---------------------------------------------------------------------------------**//**
// @bsimethod                                    Grigas.Petraitis                07/2019
+---------------+---------------+---------------+---------------+---------------+------*/
static bool IsNullQuotedDouble(PrimitiveType primitiveType, IECSqlValue const& value)
    {
    return primitiveType == PRIMITIVETYPE_Double
        && value.GetText() && 0 == strcmp("NULL", value.GetText());
    }

#define NULL_FORMATTED_VALUE_PRECONDITION(sqlValue) \
    if (sqlValue.IsNull()) \
        return rapidjson::Document();

#define NULL_FORMATTED_PRIMITIVE_VALUE_PRECONDITION(sqlValue, primitiveType) \
    NULL_FORMATTED_VALUE_PRECONDITION(sqlValue) \
    if (IsNullQuotedDouble(primitiveType, sqlValue)) \
        return rapidjson::Document();

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

private:
    /*---------------------------------------------------------------------------------**//**
    // @bsimethod                                    Grigas.Petraitis                09/2017
    +---------------+---------------+---------------+---------------+---------------+------*/
    static rapidjson::Document GetFormattedValue(IECPropertyFormatter const& formatter, ECPropertyCR prop,
        IECSqlValue const& value, rapidjson::MemoryPoolAllocator<>* allocator);
    /*---------------------------------------------------------------------------------**//**
    // @bsimethod                                    Grigas.Petraitis                09/2018
    +---------------+---------------+---------------+---------------+---------------+------*/
    static rapidjson::Document GetFallbackValue(ECPropertyCR prop, IECSqlValue const& value,
        rapidjson::MemoryPoolAllocator<>* allocator);
    /*---------------------------------------------------------------------------------**//**
    // @bsimethod                                    Grigas.Petraitis                09/2018
    +---------------+---------------+---------------+---------------+---------------+------*/
    static rapidjson::Document GetFallbackPrimitiveValue(ECPropertyCR prop, PrimitiveType type, IECSqlValue const& value, rapidjson::MemoryPoolAllocator<>* allocator)
        {
        NULL_FORMATTED_PRIMITIVE_VALUE_PRECONDITION(value, type);
        rapidjson::Document json(allocator);
        ECValue v = ValueHelpers::GetECValueFromSqlValue(type, value);
        Utf8String stringValue;
        if (v.ConvertPrimitiveToString(stringValue))
            json.SetString(stringValue.c_str(), json.GetAllocator());
        else
            {
            BeAssert(false);
            json.SetString("");
            }
        return json;
        }
    /*---------------------------------------------------------------------------------**//**
    // @bsimethod                                    Grigas.Petraitis                10/2016
    +---------------+---------------+---------------+---------------+---------------+------*/
    static rapidjson::Document GetFormattedPrimitiveValue(IECPropertyFormatter const& formatter, ECPropertyCR prop, PrimitiveType type,
        IECSqlValue const& value, rapidjson::MemoryPoolAllocator<>* allocator)
        {
        NULL_FORMATTED_PRIMITIVE_VALUE_PRECONDITION(value, type);
        rapidjson::Document json(allocator);
        Utf8String formattedValue;
        if (SUCCESS != formatter.GetFormattedPropertyValue(formattedValue, prop, ValueHelpers::GetECValueFromSqlValue(type, value)))
            return GetFallbackPrimitiveValue(prop, type, value, allocator);

        json.SetString(formattedValue.c_str(), json.GetAllocator());
        return json;
        }
    /*---------------------------------------------------------------------------------**//**
    // @bsimethod                                    Grigas.Petraitis                09/2018
    +---------------+---------------+---------------+---------------+---------------+------*/
    static rapidjson::Document GetFallbackStructValue(IECSqlValue const& structValue, rapidjson::MemoryPoolAllocator<>* allocator)
        {
        NULL_FORMATTED_VALUE_PRECONDITION(structValue);
        rapidjson::Document json(allocator);
        json.SetObject();
        for (IECSqlValue const& value : structValue.GetStructIterable())
            {
            ECPropertyCP memberProperty = value.GetColumnInfo().GetProperty();
            json.AddMember(rapidjson::Value(memberProperty->GetName().c_str(), json.GetAllocator()),
                GetFallbackValue(*memberProperty, value, &json.GetAllocator()), json.GetAllocator());
            }
        return json;
        }
    /*---------------------------------------------------------------------------------**//**
    // @bsimethod                                    Grigas.Petraitis                09/2017
    +---------------+---------------+---------------+---------------+---------------+------*/
    static rapidjson::Document GetFormattedStructValue(IECPropertyFormatter const& formatter,
        IECSqlValue const& structValue, rapidjson::MemoryPoolAllocator<>* allocator)
        {
        NULL_FORMATTED_VALUE_PRECONDITION(structValue);
        rapidjson::Document json(allocator);
        json.SetObject();
        for (IECSqlValue const& value : structValue.GetStructIterable())
            {
            ECPropertyCP memberProperty = value.GetColumnInfo().GetProperty();
            json.AddMember(rapidjson::Value(memberProperty->GetName().c_str(), json.GetAllocator()),
                GetFormattedValue(formatter, *memberProperty, value, &json.GetAllocator()), json.GetAllocator());
            }
        return json;
        }
    /*---------------------------------------------------------------------------------**//**
    // @bsimethod                                    Grigas.Petraitis                09/2018
    +---------------+---------------+---------------+---------------+---------------+------*/
    static rapidjson::Document GetFallbackArrayValue(ArrayECPropertyCR prop, IECSqlValue const& arrayValue, rapidjson::MemoryPoolAllocator<>* allocator)
        {
        NULL_FORMATTED_VALUE_PRECONDITION(arrayValue);
        rapidjson::Document json(allocator);
        json.SetArray();
        for (IECSqlValue const& value : arrayValue.GetArrayIterable())
            {
            if (prop.GetIsStructArray())
                {
                json.PushBack(GetFallbackStructValue(value, &json.GetAllocator()), json.GetAllocator());
                }
            else if (prop.GetIsPrimitiveArray())
                {
                PrimitiveType primitiveType = prop.GetAsPrimitiveArrayProperty()->GetPrimitiveElementType();
                json.PushBack(GetFallbackPrimitiveValue(prop, primitiveType, value, &json.GetAllocator()), json.GetAllocator());
                }
            else
                {
                BeAssert(false);
                break;
                }
            }
        return json;
        }
    /*---------------------------------------------------------------------------------**//**
    // @bsimethod                                    Grigas.Petraitis                09/2017
    +---------------+---------------+---------------+---------------+---------------+------*/
    static rapidjson::Document GetFormattedArrayValue(IECPropertyFormatter const& formatter, ArrayECPropertyCR prop,
        IECSqlValue const& arrayValue, rapidjson::MemoryPoolAllocator<>* allocator)
        {
        NULL_FORMATTED_VALUE_PRECONDITION(arrayValue);
        rapidjson::Document json(allocator);
        json.SetArray();
        for (IECSqlValue const& value : arrayValue.GetArrayIterable())
            {
            if (prop.GetIsStructArray())
                {
                json.PushBack(GetFormattedStructValue(formatter, value, &json.GetAllocator()), json.GetAllocator());
                }
            else if (prop.GetIsPrimitiveArray())
                {
                PrimitiveType primitiveType = prop.GetAsPrimitiveArrayProperty()->GetPrimitiveElementType();
                json.PushBack(GetFormattedPrimitiveValue(formatter, prop, primitiveType, value, &json.GetAllocator()), json.GetAllocator());
                }
            else
                {
                BeAssert(false);
                break;
                }
            }
        return json;
        }

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
            if (ecProperty.GetIsNavigation())
                AddValue(name, rapidjson::Value(), rapidjson::Value(value.GetText(), m_displayValues.GetAllocator()), ADD_DisplayValue);
            else
                AddValue(name, rapidjson::Value(value.GetText(), m_values.GetAllocator()), rapidjson::Value(value.GetText(), m_displayValues.GetAllocator()));
            m_mergedFieldNames.push_back(name);
            }
        else if (ecProperty.GetIsPrimitive())
            {
            PrimitiveECPropertyCR primitiveProperty = *ecProperty.GetAsPrimitiveProperty();
            if (IsNullQuotedDouble(primitiveProperty.GetType(), value))
                {
                AddNull(name, ADD_Both);
                }
            else
                {
                rapidjson::Document valueJson = ValueHelpers::GetJsonFromPrimitiveValue(primitiveProperty.GetType(), value, &m_values.GetAllocator());
                rapidjson::Document formattedValueJson(&m_displayValues.GetAllocator());
                if (nullptr != propertyFormatter)
                    formattedValueJson = GetFormattedPrimitiveValue(*propertyFormatter, primitiveProperty, primitiveProperty.GetType(), value, &m_displayValues.GetAllocator());
                else
                    formattedValueJson = GetFallbackPrimitiveValue(primitiveProperty, primitiveProperty.GetType(), value, &m_displayValues.GetAllocator());
                AddValue(name, std::move(valueJson), std::move(formattedValueJson));
                }
            }
        else if (ecProperty.GetIsStruct())
            {
            StructECPropertyCR structProperty = *ecProperty.GetAsStructProperty();
            rapidjson::Document valueJson = ValueHelpers::GetJsonFromStructValue(structProperty.GetType(), value, &m_values.GetAllocator());
            rapidjson::Document formattedValueJson(&m_displayValues.GetAllocator());
            if (nullptr != propertyFormatter)
                formattedValueJson = GetFormattedStructValue(*propertyFormatter, value, &m_displayValues.GetAllocator());
            else
                formattedValueJson = GetFallbackStructValue(value, &m_displayValues.GetAllocator());
            AddValue(name, std::move(valueJson), std::move(formattedValueJson));
            }
        else if (ecProperty.GetIsArray())
            {
            ArrayECPropertyCR arrayProperty = *ecProperty.GetAsArrayProperty();
            rapidjson::Document valueJson = ValueHelpers::GetJsonFromArrayValue(value, &m_values.GetAllocator());
            rapidjson::Document formattedValueJson(&m_displayValues.GetAllocator());
            if (nullptr != propertyFormatter)
                formattedValueJson = GetFormattedArrayValue(*propertyFormatter, arrayProperty, value, &m_displayValues.GetAllocator());
            else
                formattedValueJson = GetFallbackArrayValue(arrayProperty, value, &m_displayValues.GetAllocator());
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
/*---------------------------------------------------------------------------------**//**
// @bsimethod                                    Grigas.Petraitis                09/2017
+---------------+---------------+---------------+---------------+---------------+------*/
rapidjson::Document ContentValueAppender::GetFormattedValue(IECPropertyFormatter const& formatter, ECPropertyCR prop,
    IECSqlValue const& value, rapidjson::MemoryPoolAllocator<>* allocator)
    {
    if (prop.GetIsPrimitive())
        return GetFormattedPrimitiveValue(formatter, prop, prop.GetAsPrimitiveProperty()->GetType(), value, allocator);
    if (prop.GetIsArray())
        return GetFormattedArrayValue(formatter, *prop.GetAsArrayProperty(), value, allocator);
    if (prop.GetIsStruct())
        return GetFormattedStructValue(formatter, value, allocator);
    BeAssert(false && "Unexpected property type");
    return rapidjson::Document(allocator);
    }
/*---------------------------------------------------------------------------------**//**
// @bsimethod                                    Grigas.Petraitis                09/2018
+---------------+---------------+---------------+---------------+---------------+------*/
rapidjson::Document ContentValueAppender::GetFallbackValue(ECPropertyCR prop,
    IECSqlValue const& value, rapidjson::MemoryPoolAllocator<>* allocator)
    {
    if (prop.GetIsPrimitive())
        return GetFallbackPrimitiveValue(prop, prop.GetAsPrimitiveProperty()->GetType(), value, allocator);
    if (prop.GetIsArray())
        return GetFallbackArrayValue(*prop.GetAsArrayProperty(), value, allocator);
    if (prop.GetIsStruct())
        return GetFallbackStructValue(value, allocator);
    BeAssert(false && "Unexpected property type");
    return rapidjson::Document(allocator);
    }

//=======================================================================================
// @bsiclass                                     Grigas.Petraitis               06/2017
//=======================================================================================
struct FieldValueInstanceKeyReader
{
private:
    ECDbCR m_db;
    bool m_trackKeys;
    bmap<ContentDescriptor::ECPropertiesField const*, bvector<ECClassInstanceKey>> m_relatedFieldKeys;
    bmap<ContentDescriptor::ECPropertiesField const*, int> m_fieldProperties;
    bvector<ECClassInstanceKey> const& m_primaryKeys;

private:
    /*---------------------------------------------------------------------------------**//**
    // @bsimethod                                    Grigas.Petraitis               06/2017
    +---------------+---------------+---------------+---------------+---------------+------*/
    void ReadECInstanceKey(RapidJsonValueCR json, bvector<ContentDescriptor::ECPropertiesField const*> const& keyFields)
        {
        ECClassId classId(json["c"].GetUint64());
        ECInstanceId instanceId(json["i"].GetUint64());
        ECClassCP ecClass = m_db.Schemas().GetClass(classId);
        ECClassInstanceKey key(ecClass, instanceId);
        for (ContentDescriptor::ECPropertiesField const* field : keyFields)
            m_relatedFieldKeys[field].push_back(key);
        }

public:
    /*---------------------------------------------------------------------------------**//**
    // @bsimethod                                    Grigas.Petraitis               06/2017
    +---------------+---------------+---------------+---------------+---------------+------*/
    FieldValueInstanceKeyReader(ECDbCR db, bvector<ECClassInstanceKey> const& primaryKeys, bool trackKeys)
        : m_db(db), m_primaryKeys(primaryKeys), m_trackKeys(trackKeys)
        {}

    /*---------------------------------------------------------------------------------**//**
    // @bsimethod                                    Grigas.Petraitis               06/2017
    +---------------+---------------+---------------+---------------+---------------+------*/
    void ReadFieldKeys(ContentDescriptor::ECInstanceKeyField const& field, ECSqlStatement& statement, int columnIndex)
        {
        if (!m_trackKeys)
            return;

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
        if (!m_trackKeys)
            return;

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
                bvector<ECClassInstanceKey> const* sourceVector = &m_primaryKeys;
                auto relatedFieldKeyIter = m_relatedFieldKeys.find(pair.first);
                if (m_relatedFieldKeys.end() != relatedFieldKeyIter)
                    sourceVector = &relatedFieldKeyIter->second;

                size_t propertyIndex = 0;
                for (ContentDescriptor::Property const& p : pair.first->GetProperties())
                    {
                    ContentSetItem::FieldProperty fieldProperty(*pair.first, propertyIndex++);
                    for (ECClassInstanceKeyCR key : *sourceVector)
                        {
                        if (key.GetClass()->Is(&p.GetPropertyClass()))
                            keys[fieldProperty].push_back(key);
                        }
                    }
                }
            else
                {
                // here we know exactly which property was changed, so just copy all ECInstanceKeys in that field
                // to the output map
                ContentSetItem::FieldProperty fieldProperty(*pair.first, pair.second);
                bvector<ECClassInstanceKey> const* sourceVector = &m_primaryKeys;
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
    ContentQueryContractCPtr contract = m_query->GetContract();
    ContentDescriptorCR descriptor = contract->GetDescriptor();
    bool resultsMerged = descriptor.HasContentFlag(ContentFlags::MergeResults);
    int columnIndex = 0;
    uint64_t contractId = 0;
    Utf8CP relatedInstanceInfo = nullptr;

    bvector<ECClassInstanceKey> primaryRecordKeys;
    if (!descriptor.OnlyDistinctValues())
        {
        contractId = statement.GetValueUInt64(columnIndex++);
        bvector<ECInstanceKey> ecInstanceKeys = ValueHelpers::GetECInstanceKeysFromSerializedJson(statement.GetValueText(columnIndex++));
        BeAssert(1 == ecInstanceKeys.size() || resultsMerged);
        if (!resultsMerged)
            {
            auto readKeysIter = m_readKeys.find(ecInstanceKeys.front());
            if (m_readKeys.end() != readKeysIter)
                return;
            m_readKeys.insert(readKeysIter, ecInstanceKeys.front());
            }
        for (ECInstanceKeyCR key : ecInstanceKeys)
            {
            ECClassCP keyClass = GetConnection().GetECDb().Schemas().GetClass(key.GetClassId());
            primaryRecordKeys.push_back(ECClassInstanceKey(keyClass, key.GetInstanceId()));
            }
        }

    bool needFieldValueKeys = (0 == (descriptor.GetContentFlags() & (int)ContentFlags::ExcludeEditingData));
    FieldValueInstanceKeyReader fieldValueInstanceKeyReader(GetConnection().GetECDb(), primaryRecordKeys, needFieldValueKeys);

    ContentValueAppender values;
    Utf8String imageId;
    LabelDefinitionPtr displayLabelDefinition = LabelDefinition::Create();
    ECClassCP recordClass = nullptr;
    if (0 == ((int)ContentFlags::KeysOnly & descriptor.GetContentFlags()))
        {
        for (ECClassInstanceKeyCR primaryKey : primaryRecordKeys)
            {
            if (nullptr == recordClass)
                recordClass = primaryKey.GetClass();
            else if (recordClass != primaryKey.GetClass())
                {
                recordClass = nullptr;
                break;
                }
            }

        bool possiblyMerged = (resultsMerged && primaryRecordKeys.size() > 1);
        // note: the record consists of multiple merged ECInstance values - not clear
        // which one's related instance info should be used for customizing
        if (!possiblyMerged)
            relatedInstanceInfo = statement.GetValueText(columnIndex);
        columnIndex++;

        ContentQueryContractCPtr fieldsContract = m_query->GetContract(contractId);
        for (ContentDescriptor::Field const* field : descriptor.GetAllFields())
            {
            Utf8StringCR fieldName = field->GetUniqueName();
            if (field->IsDisplayLabelField() && descriptor.ShowLabels())
                {
                // if this is a display label field, set the display label definition
                displayLabelDefinition = LabelDefinition::FromString(statement.GetValueText(columnIndex));
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
                if (contract->ShouldSkipCompositePropertyFields() && propertiesField.IsCompositePropertiesField())
                    continue; // do not increase columnIndex if we didn't use the field

                ContentDescriptor::Property const* matchingProperty = nullptr;
                if (resultsMerged)
                    {
                    if (!propertiesField.GetProperties().empty())
                        matchingProperty = &propertiesField.GetProperties().front();
                    }
                else
                    {
                    // find the field property that's appropriate for this instance
                    matchingProperty = fieldsContract->FindMatchingProperty(propertiesField, recordClass);
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
                Utf8String propertiesFieldName = field->AsSystemField()->AsECNavigationInstanceIdField()->GetPropertiesField().GetUniqueName();
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

    if (descriptor.GetDistinctField() != nullptr)
        {
        for (size_t i = 0; i < m_records.size(); i++)
            {
            if (values.GetDisplayValues() == m_records[i]->GetDisplayValues() && *displayLabelDefinition == m_records[i]->GetDisplayLabelDefinition())
                return;
            }
        }

    ContentSetItemPtr record = ContentSetItem::Create(primaryRecordKeys, *displayLabelDefinition, imageId,
        values.GetValues(), values.GetDisplayValues(), values.GetMergedFieldNames(), fieldValueInstanceKeyReader.GetKeys());
    record->SetClass(recordClass);
    ContentSetItemExtendedData(*record).SetContractId(contractId);
    ContentSetItemExtendedData(*record).SetRelatedInstanceKeys(relatedInstanceInfo);

    m_records.push_back(record);
    }

/*---------------------------------------------------------------------------------**//**
// @bsimethod                                    Grigas.Petraitis                05/2016
+---------------+---------------+---------------+---------------+---------------+------*/
size_t CountQueryExecutor::GetResult() const
    {
    return m_result;
    }

/*---------------------------------------------------------------------------------**//**
// @bsimethod                                    Grigas.Petraitis                05/2016
+---------------+---------------+---------------+---------------+---------------+------*/
void CountQueryExecutor::_ReadRecord(BeSQLite::EC::ECSqlStatement& statement)
    {
    m_result = (size_t)statement.GetValueInt64(0);
    }
