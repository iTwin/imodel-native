/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include <ECPresentationPch.h>
#include "NavigationQueryResultsReader.h"

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                01/2016
+---------------+---------------+---------------+---------------+---------------+------*/
static GroupedInstanceKeysList ParseInstanceKeys(ECSqlStatementCR stmt, NavigationQueryContract const& contract, Utf8CP fieldName)
    {
    Utf8CP str = stmt.GetValueText(contract.GetIndex(fieldName));
    return ValueHelpers::GetECInstanceKeysFromSerializedJson(str);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                01/2016
+---------------+---------------+---------------+---------------+---------------+------*/
template<typename Contract>
static GroupedInstanceKeysList ParseInstanceKeys(ECSqlStatementCR stmt, NavigationQueryContract const& contract)
    {
    return ParseInstanceKeys(stmt, contract, Contract::GroupedInstanceKeysFieldName);
    }

/*=================================================================================**//**
* @bsiclass                                     Grigas.Petraitis                07/2015
+===============+===============+===============+===============+===============+======*/
struct ECInstanceNodeReader : NavNodesReader
{
typedef ECInstanceNodesQueryContract Contract;
protected:
    JsonNavNodePtr _ReadNode(ECSqlStatementCR statement) const override
        {
        ECClassId ecClassId = statement.GetValueId<ECClassId>(GetContract().GetIndex(Contract::ECClassIdFieldName));
        ECInstanceId ecInstanceId = statement.GetValueId<ECInstanceId>(GetContract().GetIndex(Contract::ECInstanceIdFieldName));
        Utf8CP displayLabel = statement.GetValueText(GetContract().GetIndex(Contract::DisplayLabelFieldName));
        Utf8CP relatedInstanceInfo = statement.GetValueText(GetContract().GetIndex(Contract::RelatedInstanceInfoFieldName));
        Utf8CP skippedInstanceKeys = statement.GetValueText(GetContract().GetIndex(Contract::SkippedInstanceKeysFieldName));
        JsonNavNodePtr node = GetFactory().CreateECInstanceNode(GetConnection().GetId(), GetLocale(), ecClassId, ecInstanceId, displayLabel);
        if (node.IsValid())
            {
            NavNodesHelper::AddRelatedInstanceInfo(*node, relatedInstanceInfo);
            NavNodesHelper::SetSkippedInstanceKeys(*node, skippedInstanceKeys);
            }
        return node;
        }
public:
    ECInstanceNodeReader(JsonNavNodesFactory const& factory, NavigationQueryContract const& contract) : NavNodesReader(factory, contract) {}
};

/*=================================================================================**//**
* @bsiclass                                     Grigas.Petraitis                07/2015
+===============+===============+===============+===============+===============+======*/
struct MultiECInstanceNodeReader : NavNodesReader
{
typedef MultiECInstanceNodesQueryContract Contract;
protected:
    JsonNavNodePtr _ReadNode(ECSqlStatementCR statement) const override
        {
        Utf8CP displayLabel = statement.GetValueText(GetContract().GetIndex(Contract::DisplayLabelFieldName));
        Utf8CP relatedInstanceInfo = statement.GetValueText(GetContract().GetIndex(Contract::RelatedInstanceInfoFieldName));
        Utf8CP skippedInstanceKeys = statement.GetValueText(GetContract().GetIndex(Contract::SkippedInstanceKeysFieldName));
        GroupedInstanceKeysList keys = ParseInstanceKeys(statement, GetContract(), Contract::InstanceKeysFieldName);
        JsonNavNodePtr node = GetFactory().CreateECInstanceNode(GetConnection().GetId(), GetLocale(), keys, displayLabel);
        if (node.IsValid())
            {
            NavNodesHelper::AddRelatedInstanceInfo(*node, relatedInstanceInfo);
            NavNodesHelper::SetSkippedInstanceKeys(*node, skippedInstanceKeys);
            }
        return node;
        }
public:
    MultiECInstanceNodeReader(JsonNavNodesFactory const& factory, NavigationQueryContract const& contract) : NavNodesReader(factory, contract) {}
};

/*=================================================================================**//**
* @bsiclass                                     Grigas.Petraitis                07/2015
+===============+===============+===============+===============+===============+======*/
struct DisplayLabelGroupingNodeReader : NavNodesReader
{
typedef DisplayLabelGroupingNodesQueryContract Contract;
protected:
    JsonNavNodePtr _ReadNode(ECSqlStatementCR statement) const override
        {
        Utf8CP displayLabel = statement.GetValueText(GetContract().GetIndex(Contract::DisplayLabelFieldName));
        GroupedInstanceKeysList keys = ParseInstanceKeys<Contract>(statement, GetContract());
        JsonNavNodePtr node = GetFactory().CreateDisplayLabelGroupingNode(GetConnection().GetId(), GetLocale(), displayLabel, keys);
        if (node.IsValid())
            {
            Utf8CP skippedInstanceKeys = statement.GetValueText(GetContract().GetIndex(Contract::SkippedInstanceKeysFieldName));
            NavNodesHelper::SetSkippedInstanceKeys(*node, skippedInstanceKeys);
            }
        return node;
        }
public:
    DisplayLabelGroupingNodeReader(JsonNavNodesFactory const& factory, NavigationQueryContract const& contract) : NavNodesReader(factory, contract) {}
};

/*=================================================================================**//**
* @bsiclass                                     Grigas.Petraitis                07/2015
+===============+===============+===============+===============+===============+======*/
struct ECClassGroupingNodeReader : NavNodesReader
{
typedef ECClassGroupingNodesQueryContract Contract;
protected:
    JsonNavNodePtr _ReadNode(ECSqlStatementCR statement) const override
        {
        ECClassId classId = statement.GetValueId<ECClassId>(GetContract().GetIndex(Contract::ECClassIdFieldName));
        if (!classId.IsValid())
            {
            BeAssert(false);
            return nullptr;
            }

        ECClassCP ecClass = nullptr;
        if (nullptr == (ecClass = statement.GetECDb()->Schemas().GetClass(classId)))
            {
            BeAssert(false);
            return nullptr;
            }

        Utf8CP displayLabel = statement.GetValueText(GetContract().GetIndex(Contract::DisplayLabelFieldName));
        GroupedInstanceKeysList keys = ParseInstanceKeys<Contract>(statement, GetContract());
        JsonNavNodePtr node = GetFactory().CreateECClassGroupingNode(GetConnection().GetId(), GetLocale(), *ecClass, displayLabel, keys);
        if (node.IsValid())
            {
            Utf8CP skippedInstanceKeys = statement.GetValueText(GetContract().GetIndex(Contract::SkippedInstanceKeysFieldName));
            NavNodesHelper::SetSkippedInstanceKeys(*node, skippedInstanceKeys);
            }
        return node;
        }
public:
    ECClassGroupingNodeReader(JsonNavNodesFactory const& factory, NavigationQueryContract const& contract) : NavNodesReader(factory, contract) {}
};

/*=================================================================================**//**
* @bsiclass                                     Grigas.Petraitis                07/2015
+===============+===============+===============+===============+===============+======*/
struct BaseClassGroupingNodeReader : NavNodesReader
{
typedef BaseECClassGroupingNodesQueryContract Contract;
protected:
    JsonNavNodePtr _ReadNode(ECSqlStatementCR statement) const override
        {
        ECClassId classId = statement.GetValueId<ECClassId>(GetContract().GetIndex(Contract::BaseClassIdFieldName));
        if (!classId.IsValid())
            {
            BeAssert(false);
            return nullptr;
            }

        ECClassCP ecClass = nullptr;
        if (nullptr == (ecClass = statement.GetECDb()->Schemas().GetClass(classId)))
            {
            BeAssert(false);
            return nullptr;
            }

        Utf8CP displayLabel = statement.GetValueText(GetContract().GetIndex(Contract::DisplayLabelFieldName));
        GroupedInstanceKeysList keys = ParseInstanceKeys<Contract>(statement, GetContract());
        JsonNavNodePtr node = GetFactory().CreateECClassGroupingNode(GetConnection().GetId(), GetLocale(), *ecClass, displayLabel, keys);
        if (node.IsValid())
            {
            Utf8CP skippedInstanceKeys = statement.GetValueText(GetContract().GetIndex(Contract::SkippedInstanceKeysFieldName));
            NavNodesHelper::SetSkippedInstanceKeys(*node, skippedInstanceKeys);
            }
        return node;
        }
public:
    BaseClassGroupingNodeReader(JsonNavNodesFactory const& factory, NavigationQueryContract const& contract) : NavNodesReader(factory, contract) {}
};

/*=================================================================================**//**
* @bsiclass                                     Grigas.Petraitis                07/2015
+===============+===============+===============+===============+===============+======*/
struct ECRelationshipClassGroupingNodeReader : NavNodesReader
{
typedef ECRelationshipGroupingNodesQueryContract Contract;
protected:
    JsonNavNodePtr _ReadNode(ECSqlStatementCR statement) const override
        {
        return nullptr;
        }
public:
    ECRelationshipClassGroupingNodeReader(JsonNavNodesFactory const& factory, NavigationQueryContract const& contract) : NavNodesReader(factory, contract) {}
};

/*=================================================================================**//**
* @bsiclass                                     Saulius.Skliutas                06/2017
+===============+===============+===============+===============+===============+======*/
template <typename ValueType>
struct ValueParser
{
protected:
    virtual ValueType _ParseValueFromString(Utf8StringCR valueStr) const = 0;
    virtual void _SetJsonValue(RapidJsonDocumentR jsonDoc, ValueType const& value) const = 0;
public:
    rapidjson::Document GetValueAsJson(Utf8CP valueStr, bool alwaysSetAsArray = false) const
        {
        bvector<Utf8String> valuesStr;
        BeStringUtilities::Split(valueStr, ",", valuesStr);
        bset<ValueType> values;
        for (Utf8StringCR valueStr : valuesStr)
            values.insert(_ParseValueFromString(valueStr));

        if (values.empty())
            {
            BeAssert(false);
            return rapidjson::Document();
            }

        if (!alwaysSetAsArray && 1 == values.size())
            {
            rapidjson::Document json;
            _SetJsonValue(json, *values.begin());
            return json;
            }

        rapidjson::Document valuesJson;
        valuesJson.SetArray();
        for (ValueType value : values)
            valuesJson.PushBack(value, valuesJson.GetAllocator());
        return valuesJson;
        }
};

/*=================================================================================**//**
* @bsiclass                                     Saulius.Skliutas                06/2017
+===============+===============+===============+===============+===============+======*/
struct IntValueParser : ValueParser<int>
{
protected:
    int _ParseValueFromString(Utf8StringCR valueStr) const override
        {
        return atoi(valueStr.c_str());
        }
    void _SetJsonValue(RapidJsonDocumentR jsonDoc, int const& value) const override
        {
        jsonDoc.SetInt(value);
        }
};

/*=================================================================================**//**
* @bsiclass                                     Saulius.Skliutas                06/2017
+===============+===============+===============+===============+===============+======*/
struct LongValueParser : ValueParser<uint64_t>
{
protected:
    uint64_t _ParseValueFromString(Utf8StringCR valueStr) const override
        {
        uint64_t value;
        if (SUCCESS == BeStringUtilities::ParseUInt64(value, valueStr.c_str()))
            return value;
        BeAssert(false);
        return 0;
        }
    void _SetJsonValue(RapidJsonDocumentR jsonDoc, uint64_t const& value) const override
        {
        jsonDoc.SetUint64(value);
        }
};

/*=================================================================================**//**
* @bsiclass                                     Saulius.Skliutas                06/2017
+===============+===============+===============+===============+===============+======*/
struct DoubleValueParser : ValueParser<double>
{
protected:
    double _ParseValueFromString(Utf8StringCR valueStr) const override
        {
        return atof(valueStr.c_str());
        }
    void _SetJsonValue(RapidJsonDocumentR jsonDoc, double const& value) const override
        {
        jsonDoc.SetDouble(value);
        }
};

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Saulius.Skliutas                06/2017
+---------------+---------------+---------------+---------------+---------------+------*/
static rapidjson::Document GetPointAsJsonFromString(Utf8StringCR valueStr)
    {
    Utf8String jsonStr;
    //Wrapping valueStr with '[' ']' so json document will parse it as array of json objects
    jsonStr.append("[").append(valueStr).append("]");
    rapidjson::Document jsonDoc;
    jsonDoc.Parse(jsonStr.c_str());
    return jsonDoc;
    }

/*=================================================================================**//**
* @bsiclass                                     Grigas.Petraitis                07/2015
+===============+===============+===============+===============+===============+======*/
struct ECPropertyGroupingNodeReader : NavNodesReader
{
typedef ECPropertyGroupingNodesQueryContract Contract;

private:
    static rapidjson::Document GetGroupingValueAsJson(ECPropertyCR prop, Utf8CP valueStr, bool isRangeIndex)
        {
        rapidjson::Document doc;
        if (isRangeIndex)
            {
            doc.SetInt(atoi(valueStr));
            return doc;
            }

        if (nullptr == valueStr)
            return rapidjson::Document();

        if (prop.GetIsNavigation())
            return LongValueParser().GetValueAsJson(valueStr);

        if (!prop.GetIsPrimitive())
            {
            BeAssert(false);
            return rapidjson::Document();
            }
        PrimitiveECPropertyCR primitive = *prop.GetAsPrimitiveProperty();
        switch (primitive.GetType())
            {
            case PRIMITIVETYPE_Boolean:
                doc.SetBool(0 == BeStringUtilities::Stricmp("true", valueStr) || 0 == BeStringUtilities::Stricmp("1", valueStr));
                return doc;
            case PRIMITIVETYPE_String:
                doc.CopyFrom(rapidjson::Value(valueStr, doc.GetAllocator()).Move(), doc.GetAllocator());
                return doc;
            case PRIMITIVETYPE_Point2d:
            case PRIMITIVETYPE_Point3d:
                return GetPointAsJsonFromString(valueStr);
            case PRIMITIVETYPE_Double:
            case PRIMITIVETYPE_DateTime:
                return DoubleValueParser().GetValueAsJson(valueStr, true);
            case PRIMITIVETYPE_Integer:
                return IntValueParser().GetValueAsJson(valueStr);
            case PRIMITIVETYPE_Long:
                return LongValueParser().GetValueAsJson(valueStr);
            }
        BeAssert(false);
        return rapidjson::Document();
        }

protected:
    JsonNavNodePtr _ReadNode(ECSqlStatementCR statement) const override
        {
        ECClassId classId = statement.GetValueId<ECClassId>(GetContract().GetIndex(Contract::ECPropertyClassIdFieldName));
        if (!classId.IsValid())
            {
            BeAssert(false);
            return nullptr;
            }

        ECClassCP ecClass = nullptr;
        if (nullptr == (ecClass = statement.GetECDb()->Schemas().GetClass(classId)))
            {
            BeAssert(false);
            return nullptr;
            }

        Utf8CP propertyName = statement.GetValueText(GetContract().GetIndex(Contract::ECPropertyNameFieldName));
        if (nullptr == propertyName)
            {
            BeAssert(false);
            return nullptr;
            }

        ECPropertyCP ecProperty = ecClass->GetPropertyP(propertyName);
        if (nullptr == ecProperty)
            {
            BeAssert(false);
            return nullptr;
            }

        Utf8CP displayLabel = statement.GetValueText(GetContract().GetIndex(Contract::DisplayLabelFieldName));
        bool isRangeGroupingNode = statement.GetValueBoolean(GetContract().GetIndex(Contract::IsRangeFieldName));
        rapidjson::Document groupingValue = GetGroupingValueAsJson(*ecProperty, statement.GetValueText(GetContract().GetIndex(Contract::GroupingValuesFieldName)), isRangeGroupingNode);
        Utf8CP imageId = statement.GetValueText(GetContract().GetIndex(Contract::ImageIdFieldName));
        GroupedInstanceKeysList keys = ParseInstanceKeys<Contract>(statement, GetContract());
        JsonNavNodePtr node = GetFactory().CreateECPropertyGroupingNode(GetConnection().GetId(), GetLocale(), *ecClass, *ecProperty, displayLabel, imageId, groupingValue, isRangeGroupingNode, keys);
        if (node.IsValid())
            {
            Utf8CP skippedInstanceKeys = statement.GetValueText(GetContract().GetIndex(Contract::SkippedInstanceKeysFieldName));
            NavNodesHelper::SetSkippedInstanceKeys(*node, skippedInstanceKeys);
            }
        return node;
        }

public:
    ECPropertyGroupingNodeReader(JsonNavNodesFactory const& factory, NavigationQueryContract const& contract) : NavNodesReader(factory, contract) {}
};

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                07/2015
+---------------+---------------+---------------+---------------+---------------+------*/
std::unique_ptr<NavNodesReader> NavNodesReader::Create(JsonNavNodesFactory const& factory, IConnectionCR connection, Utf8String locale, NavigationQueryContract const& contract,
    NavigationQueryResultType resultType, NavNodeExtendedData const* extendedData)
    {
    std::unique_ptr<NavNodesReader> reader;
    switch (resultType)
        {
        case NavigationQueryResultType::ECRelationshipClassNodes:
            reader = std::make_unique<ECRelationshipClassGroupingNodeReader>(factory, contract);
            break;
        case NavigationQueryResultType::ClassGroupingNodes:
            reader = std::make_unique<ECClassGroupingNodeReader>(factory, contract);
            break;
        case NavigationQueryResultType::BaseClassGroupingNodes:
            reader = std::make_unique<BaseClassGroupingNodeReader>(factory, contract);
            break;
        case NavigationQueryResultType::PropertyGroupingNodes:
            reader = std::make_unique<ECPropertyGroupingNodeReader>(factory, contract);
            break;
        case NavigationQueryResultType::DisplayLabelGroupingNodes:
            reader = std::make_unique<DisplayLabelGroupingNodeReader>(factory, contract);
            break;
        case NavigationQueryResultType::ECInstanceNodes:
            reader = std::make_unique<ECInstanceNodeReader>(factory, contract);
            break;
        case NavigationQueryResultType::MultiECInstanceNodes:
            reader = std::make_unique<MultiECInstanceNodeReader>(factory, contract);
            break;
        default:
            BeAssert(false && "Not implemented.");
            return nullptr;
        }
    reader->m_connection = &connection;
    reader->m_locale = locale;
    reader->m_navNodeExtendedData = extendedData;
    return reader;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Grigas.Petraitis                07/2015
+---------------+---------------+---------------+---------------+---------------+------*/
QueryResultReaderStatus NavNodesReader::_ReadRecord(JsonNavNodePtr& node, ECSqlStatementCR stmt)
    {
    node = _ReadNode(stmt);
    if (node.IsNull())
        {
        BeAssert(false);
        return QueryResultReaderStatus::Error;
        }

    if (nullptr != m_navNodeExtendedData)
        {
        NavNodeExtendedData extendedData(*node);
        extendedData.MergeWith(*m_navNodeExtendedData);
        }

    return QueryResultReaderStatus::Row;
    }