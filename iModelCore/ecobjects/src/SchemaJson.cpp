/*--------------------------------------------------------------------------------------+
|
|     $Source: src/SchemaJson.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#include "ECObjectsPch.h"
#include "SchemaJson.h"

BEGIN_BENTLEY_ECOBJECT_NAMESPACE

//---------------------------------------------------------------------------------------
// @bsimethod                                   Victor.Cushman              11/2017
//---------------+---------------+---------------+---------------+---------------+-------
SchemaJsonWriter::SchemaJsonWriter(Json::Value& jsonRoot, ECSchemaCR ecSchema) : m_jsonRoot(jsonRoot), m_ecSchema(ecSchema)
    {
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Victor.Cushman              11/2017
//---------------+---------------+---------------+---------------+---------------+-------
SchemaWriteStatus SchemaJsonWriter::WriteSchemaReferences()
    {
    auto jsonRefArr = Json::Value(Json::ValueType::arrayValue);
    for (auto const& pair : m_ecSchema.GetReferencedSchemas())
        {
        Json::Value v = Json::Value(Json::ValueType::objectValue);
        v[SCHEMAREF_NAME_ATTRIBUTE] = pair.first.GetName();
        v[SCHEMAREF_VERSION_ATTRIBUTE] = pair.first.GetVersionString();
        jsonRefArr.append(v);
        }

    if (0 != jsonRefArr.size())
        m_jsonRoot[ECJSON_REFERENCES_ATTRIBUTE] = jsonRefArr;

    return SchemaWriteStatus::Success;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Victor.Cushman              11/2017
//---------------+---------------+---------------+---------------+---------------+-------
SchemaWriteStatus SchemaJsonWriter::WriteClass(ECClassCR ecClass)
    {
    // Don't write any classes that aren't in the schema we're writing.
    if (&(ecClass.GetSchema()) != &m_ecSchema)
        return SchemaWriteStatus::Success;

    Json::Value& childObj = m_jsonRoot[ECJSON_SCHEMA_CHILDREN_ATTRIBUTE][ecClass.GetName()];
    return ecClass._WriteJson(childObj, false, false);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Victor.Cushman              11/2017
//---------------+---------------+---------------+---------------+---------------+-------
SchemaWriteStatus SchemaJsonWriter::WriteEnumeration(ECEnumerationCR ecEnumeration)
    {
    // Don't write any enumerations that aren't in the schema we're writing.
    if (&(ecEnumeration.GetSchema()) != &m_ecSchema)
        return SchemaWriteStatus::Success;

    Json::Value& childObj = m_jsonRoot[ECJSON_SCHEMA_CHILDREN_ATTRIBUTE][ecEnumeration.GetName()];
    return ecEnumeration.WriteJson(childObj, false, false);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Victor.Cushman              11/2017
//---------------+---------------+---------------+---------------+---------------+-------
SchemaWriteStatus SchemaJsonWriter::WriteKindOfQuantity(KindOfQuantityCR kindOfQuantity)
    {
    // Don't write any KOQs that aren't in the schema we're writing.
    if (&(kindOfQuantity.GetSchema()) != &m_ecSchema)
        return SchemaWriteStatus::Success;

    Json::Value& childObj = m_jsonRoot[ECJSON_SCHEMA_CHILDREN_ATTRIBUTE][kindOfQuantity.GetName()];
    return kindOfQuantity.WriteJson(childObj, false, false);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Victor.Cushman              11/2017
//---------------+---------------+---------------+---------------+---------------+-------
SchemaWriteStatus SchemaJsonWriter::WritePropertyCategory(PropertyCategoryCR propertyCategory)
    {
    // Don't write any elements that aren't in the schema we're writing.
    if (&(propertyCategory.GetSchema()) != &m_ecSchema)
        return SchemaWriteStatus::Success;

    Json::Value& childObj = m_jsonRoot[ECJSON_SCHEMA_CHILDREN_ATTRIBUTE][propertyCategory.GetName()];
    return propertyCategory.WriteJson(childObj, false, false);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Victor.Cushman              11/2017
//---------------+---------------+---------------+---------------+---------------+-------
SchemaWriteStatus SchemaJsonWriter::WriteSchemaChildren()
    {
    m_jsonRoot[ECJSON_SCHEMA_CHILDREN_ATTRIBUTE] = Json::Value(Json::ValueType::objectValue);
    SchemaWriteStatus status;

    for (auto const ecClass : m_ecSchema.GetClasses())
        {
        if (SchemaWriteStatus::Success != (status = WriteClass(*ecClass)))
            return status;
        }

    for (auto const enumeration : m_ecSchema.GetEnumerations())
        {
        if (SchemaWriteStatus::Success != (status = WriteEnumeration(*enumeration)))
            return status;
        }

    for (auto const koq : m_ecSchema.GetKindOfQuantities())
        {
        if (SchemaWriteStatus::Success != (status = WriteKindOfQuantity(*koq)))
            return status;
        }

    for (auto const pc : m_ecSchema.GetPropertyCategories())
        {
        if (SchemaWriteStatus::Success != (status = WritePropertyCategory(*pc)))
            return status;
        }

    if (0 == m_jsonRoot[ECJSON_SCHEMA_CHILDREN_ATTRIBUTE].size())
        m_jsonRoot.removeMember(ECJSON_SCHEMA_CHILDREN_ATTRIBUTE);

    return SchemaWriteStatus::Success;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Victor.Cushman              11/2017
//---------------+---------------+---------------+---------------+---------------+-------
SchemaWriteStatus SchemaJsonWriter::Serialize()
    {
    if (m_ecSchema.GetECVersion() < ECVersion::V3_1)
        {
        LOG.errorv("Schema Serialization Violation: JSON serialization only supported for schemas with ECVersion 3.1 or later.");
        return SchemaWriteStatus::FailedToCreateJson;
        }

    m_jsonRoot = Json::Value(Json::ValueType::objectValue);
    m_jsonRoot[ECJSON_URI_SPEC_ATTRIBUTE] = ECJSON_URI;
    m_jsonRoot[ECJSON_NAME_ATTRIBUTE] = m_ecSchema.GetName();
    m_jsonRoot[SCHEMA_VERSION_ATTRIBUTE] = m_ecSchema.GetSchemaKey().GetVersionString();
    m_jsonRoot[ALIAS_ATTRIBUTE] = m_ecSchema.GetAlias();

    if (m_ecSchema.GetIsDisplayLabelDefined())
        m_jsonRoot[ECJSON_DISPLAY_LABEL_ATTRIBUTE] = m_ecSchema.GetInvariantDisplayLabel();

    if (m_ecSchema.GetInvariantDescription().length())
        m_jsonRoot[DESCRIPTION_ATTRIBUTE] = m_ecSchema.GetInvariantDescription();

    SchemaWriteStatus status;
    if (SchemaWriteStatus::Success != (status = WriteSchemaReferences()))
        return status;

    Json::Value customAttributesArr;
    if (SchemaWriteStatus::Success != (status = m_ecSchema.WriteCustomAttributes(customAttributesArr)))
        return status;
    if (!customAttributesArr.empty())
        m_jsonRoot[ECJSON_CUSTOM_ATTRIBUTES_ELEMENT] = customAttributesArr;

    if (SchemaWriteStatus::Success != (status = WriteSchemaChildren()))
        return status;

    return SchemaWriteStatus::Success;
    }

END_BENTLEY_ECOBJECT_NAMESPACE
