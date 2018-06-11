/*--------------------------------------------------------------------------------------+
|
|     $Source: src/SchemaJson.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
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
bool SchemaJsonWriter::WriteSchemaReferences()
    {
    auto jsonRefArr = Json::Value(Json::ValueType::arrayValue);
    for (auto const& pair : m_ecSchema.GetReferencedSchemas())
        {
        Json::Value v = Json::Value(Json::ValueType::objectValue);
        v[NAME_ATTRIBUTE] = pair.first.GetName();
        v[SCHEMAREF_VERSION_ATTRIBUTE] = pair.first.GetVersionString();
        jsonRefArr.append(v);
        }

    if (0 != jsonRefArr.size())
        m_jsonRoot[ECJSON_REFERENCES_ATTRIBUTE] = jsonRefArr;

    return true;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Victor.Cushman              11/2017
//---------------+---------------+---------------+---------------+---------------+-------
bool SchemaJsonWriter::WriteClass(ECClassCR ecClass)
    {
    // Don't write any classes that aren't in the schema we're writing.
    if (&(ecClass.GetSchema()) != &m_ecSchema)
        return true;

    Json::Value& itemObj = m_jsonRoot[ECJSON_SCHEMA_ITEMS_ATTRIBUTE][ecClass.GetName()];
    return ecClass._ToJson(itemObj, false, false, false);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Victor.Cushman              11/2017
//---------------+---------------+---------------+---------------+---------------+-------
bool SchemaJsonWriter::WriteEnumeration(ECEnumerationCR ecEnumeration)
    {
    // Don't write any enumerations that aren't in the schema we're writing.
    if (&(ecEnumeration.GetSchema()) != &m_ecSchema)
        return true;

    Json::Value& itemObj = m_jsonRoot[ECJSON_SCHEMA_ITEMS_ATTRIBUTE][ecEnumeration.GetName()];
    return ecEnumeration.ToJson(itemObj, false, false);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Victor.Cushman              11/2017
//---------------+---------------+---------------+---------------+---------------+-------
bool SchemaJsonWriter::WriteKindOfQuantity(KindOfQuantityCR kindOfQuantity)
    {
    // Don't write any KOQs that aren't in the schema we're writing.
    if (&(kindOfQuantity.GetSchema()) != &m_ecSchema)
        return true;

    Json::Value& itemObj = m_jsonRoot[ECJSON_SCHEMA_ITEMS_ATTRIBUTE][kindOfQuantity.GetName()];
    return kindOfQuantity.ToJson(itemObj, false, false);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Victor.Cushman              11/2017
//---------------+---------------+---------------+---------------+---------------+-------
bool SchemaJsonWriter::WritePropertyCategory(PropertyCategoryCR propertyCategory)
    {
    // Don't write any elements that aren't in the schema we're writing.
    if (&(propertyCategory.GetSchema()) != &m_ecSchema)
        return true;

    Json::Value& itemObj = m_jsonRoot[ECJSON_SCHEMA_ITEMS_ATTRIBUTE][propertyCategory.GetName()];
    return propertyCategory.ToJson(itemObj, false, false);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Kyle.Abramowitz             02/2018
//---------------+---------------+---------------+---------------+---------------+-------
bool SchemaJsonWriter::WriteUnitSystem(UnitSystemCR unitSystem)
    {
    // Don't write any elements that aren't in the schema we're writing.
    if (&(unitSystem.GetSchema()) != &m_ecSchema)
        return true;

    Json::Value& childObj = m_jsonRoot[ECJSON_SCHEMA_ITEMS_ATTRIBUTE][unitSystem.GetName()];
    return unitSystem.ToJson(childObj, false, false);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Kyle.Abramowitz             02/2018
//---------------+---------------+---------------+---------------+---------------+-------
bool SchemaJsonWriter::WritePhenomenon(PhenomenonCR phenomenon)
    {
    // Don't write any elements that aren't in the schema we're writing.
    if (&(phenomenon.GetSchema()) != &m_ecSchema)
        return true;

    Json::Value& childObj = m_jsonRoot[ECJSON_SCHEMA_ITEMS_ATTRIBUTE][phenomenon.GetName()];
    return phenomenon.ToJson(childObj, false, false);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Kyle.Abramowitz             02/2018
//---------------+---------------+---------------+---------------+---------------+-------
bool SchemaJsonWriter::WriteUnit(ECUnitCR unit)
    {
    // Don't write any elements that aren't in the schema we're writing.
    if (&(unit.GetSchema()) != &m_ecSchema)
        return true;

    Json::Value& childObj = m_jsonRoot[ECJSON_SCHEMA_ITEMS_ATTRIBUTE][unit.GetName()];
    return unit.ToJson(childObj, false, false);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Victor.Cushman              11/2017
//---------------+---------------+---------------+---------------+---------------+-------
bool SchemaJsonWriter::WriteSchemaItems()
    {
    m_jsonRoot[ECJSON_SCHEMA_ITEMS_ATTRIBUTE] = Json::Value(Json::ValueType::objectValue);

    for (auto const ecClass : m_ecSchema.GetClasses())
        {
        if (!WriteClass(*ecClass))
            return false;
        }

    for (auto const enumeration : m_ecSchema.GetEnumerations())
        {
        if (!WriteEnumeration(*enumeration))
            return false;
        }

    for (auto const koq : m_ecSchema.GetKindOfQuantities())
        {
        if (!WriteKindOfQuantity(*koq))
            return false;
        }

    for (auto const pc : m_ecSchema.GetPropertyCategories())
        {
        if (!WritePropertyCategory(*pc))
            return false;
        }

    for (auto const us : m_ecSchema.GetUnitSystems())
        {
        if (!WriteUnitSystem(*us))
            return false;
        }

    for (auto const ph : m_ecSchema.GetPhenomena())
        {
        if (!WritePhenomenon(*ph))
            return false;
        }

    for (auto const ecu : m_ecSchema.GetUnits())
        {
        if (!WriteUnit(*ecu))
            return false;
        }

    if (0 == m_jsonRoot[ECJSON_SCHEMA_ITEMS_ATTRIBUTE].size())
        m_jsonRoot.removeMember(ECJSON_SCHEMA_ITEMS_ATTRIBUTE);

    return true;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Victor.Cushman              11/2017
//---------------+---------------+---------------+---------------+---------------+-------
bool SchemaJsonWriter::Serialize()
    {
    if (m_ecSchema.GetECVersion() < ECVersion::V3_1)
        {
        LOG.errorv("Schema Serialization Violation: JSON serialization only supported for schemas with ECVersion 3.1 or later.");
        return false;
        }

    m_jsonRoot = Json::Value(Json::ValueType::objectValue);
    m_jsonRoot[ECJSON_URI_SPEC_ATTRIBUTE] = ECJSON_URI;
    m_jsonRoot[NAME_ATTRIBUTE] = m_ecSchema.GetName();
    m_jsonRoot[SCHEMA_VERSION_ATTRIBUTE] = m_ecSchema.GetSchemaKey().GetVersionString();
    m_jsonRoot[ALIAS_ATTRIBUTE] = m_ecSchema.GetAlias();

    if (m_ecSchema.GetIsDisplayLabelDefined())
        m_jsonRoot[ECJSON_DISPLAY_LABEL_ATTRIBUTE] = m_ecSchema.GetInvariantDisplayLabel();

    if (m_ecSchema.GetInvariantDescription().length())
        m_jsonRoot[DESCRIPTION_ATTRIBUTE] = m_ecSchema.GetInvariantDescription();

    if (!WriteSchemaReferences())
        return false;

    Json::Value customAttributesArr;
    if (!m_ecSchema.WriteCustomAttributes(customAttributesArr))
        return false;
    if (!customAttributesArr.empty())
        m_jsonRoot[ECJSON_CUSTOM_ATTRIBUTES_ELEMENT] = customAttributesArr;

    if (!WriteSchemaItems())
        return false;

    return true;
    }

END_BENTLEY_ECOBJECT_NAMESPACE
