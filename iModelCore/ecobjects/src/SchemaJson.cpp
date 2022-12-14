/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/

#include "ECObjectsPch.h"
#include "SchemaJson.h"

BEGIN_BENTLEY_ECOBJECT_NAMESPACE

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------+---------------+---------------+---------------+---------------+-------
SchemaJsonWriter::SchemaJsonWriter(BeJsValue jsonRoot, ECSchemaCR ecSchema) : m_jsonRoot(jsonRoot), m_ecSchema(ecSchema)
    {
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------+---------------+---------------+---------------+---------------+-------
bool SchemaJsonWriter::WriteSchemaReferences()
    {
    auto jsonRefArr = m_jsonRoot[ECJSON_REFERENCES_ATTRIBUTE];
    for (auto const& pair : m_ecSchema.GetReferencedSchemas())
        {
        auto v = jsonRefArr.appendValue();
        v[NAME_ATTRIBUTE] = pair.first.GetName();
        v[SCHEMAREF_VERSION_ATTRIBUTE] = pair.first.GetVersionString();
        }

    if (0  == jsonRefArr.size())
        m_jsonRoot.removeMember(ECJSON_REFERENCES_ATTRIBUTE);

    return true;
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------+---------------+---------------+---------------+---------------+-------
bool SchemaJsonWriter::WriteClass(ECClassCR ecClass)
    {
    // Don't write any classes that aren't in the schema we're writing.
    if (&(ecClass.GetSchema()) != &m_ecSchema)
        return true;

    auto itemObj = m_jsonRoot[ECJSON_SCHEMA_ITEMS_ATTRIBUTE][ecClass.GetName()];
    return ecClass._ToJson(BeJsValue(itemObj), false, false, false);
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------+---------------+---------------+---------------+---------------+-------
bool SchemaJsonWriter::WriteFormat(ECFormatCR format)
    {
    // Don't write any classes that aren't in the schema we're writing.
    if (&(format.GetSchema()) != &m_ecSchema)
        return true;

    auto itemObj = m_jsonRoot[ECJSON_SCHEMA_ITEMS_ATTRIBUTE][format.GetName()];
    return format.ToJsonInternal(BeJsValue(itemObj), false, false);
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------+---------------+---------------+---------------+---------------+-------
bool SchemaJsonWriter::WriteSchemaItems()
    {
    m_jsonRoot[ECJSON_SCHEMA_ITEMS_ATTRIBUTE].SetEmptyObject();

    for (auto const ecClass : m_ecSchema.GetClasses())
        if (!WriteClass(*ecClass))
            return false;

    for (auto const enumeration : m_ecSchema.GetEnumerations())
        if (!WriteSchemaItem<ECEnumeration>(*enumeration))
            return false;

    for (auto const koq : m_ecSchema.GetKindOfQuantities())
        if (!WriteSchemaItem<KindOfQuantity>(*koq))
            return false;

    for (auto const pc : m_ecSchema.GetPropertyCategories())
        if (!WriteSchemaItem<PropertyCategory>(*pc))
            return false;

    for (auto const us : m_ecSchema.GetUnitSystems())
        if (!WriteSchemaItem<UnitSystem>(*us))
            return false;

    for (auto const ph : m_ecSchema.GetPhenomena())
        if (!WriteSchemaItem<Phenomenon>(*ph))
            return false;

    for (auto const ecu : m_ecSchema.GetUnits())
        if (!WriteSchemaItem<ECUnit>(*ecu))
            return false;

    for (auto const format : m_ecSchema.GetFormats())
        if (!WriteFormat(*format))
            return false;

    if (0 == m_jsonRoot[ECJSON_SCHEMA_ITEMS_ATTRIBUTE].size())
        m_jsonRoot.removeMember(ECJSON_SCHEMA_ITEMS_ATTRIBUTE);

    return true;
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------+---------------+---------------+---------------+---------------+-------
bool SchemaJsonWriter::Serialize()
    {
    if (m_ecSchema.GetECVersion() < ECVersion::V3_1)
        {
        LOG.errorv("Schema Serialization Violation: JSON serialization only supported for schemas with ECVersion 3.1 or later.");
        return false;
        }

    m_jsonRoot.SetEmptyObject();
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

    m_ecSchema.WriteCustomAttributes(m_jsonRoot);

    if (!WriteSchemaItems())
        return false;

    return true;
    }

END_BENTLEY_ECOBJECT_NAMESPACE
