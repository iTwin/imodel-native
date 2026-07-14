/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "ECObjectsPch.h"
#include <rapidjson/schema.h>

BEGIN_BENTLEY_ECOBJECT_NAMESPACE

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
JsonDescription::~JsonDescription()
    {
    delete static_cast<rapidjson::SchemaDocument*>(m_compiledJsonSchema);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8StringCR JsonDescription::GetFullName() const
    {
    return m_fullName.GetName(*this);
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------+---------------+---------------+---------------+---------------+-------
void JsonDescription::SetName(Utf8CP name)
    {
    if (!m_name.SetValidName(name, false))
        return;

    m_fullName.RecomputeName(*this);
    }

/*---------------------------------------------------------------------------------**//**
 @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus JsonDescription::SetJsonSchema(Utf8CP jsonSchema)
    {
    // Discard any previously compiled json schema before replacing the body.
    delete static_cast<rapidjson::SchemaDocument*>(m_compiledJsonSchema);
    m_compiledJsonSchema = nullptr;

    if (!Utf8String::IsNullOrEmpty(jsonSchema))
        {
        rapidjson::Document schemaDoc;
        schemaDoc.Parse(jsonSchema);
        if (schemaDoc.HasParseError())
            return ECObjectsStatus::ParseError;

        m_compiledJsonSchema = new rapidjson::SchemaDocument(schemaDoc);
        }

    m_jsonSchema = jsonSchema ? jsonSchema : "";
    return ECObjectsStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
 @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool JsonDescription::ValidateJson(rapidjson::Document const& doc) const
    {
    if (m_compiledJsonSchema == nullptr)
        return true;

    rapidjson::SchemaValidator validator(*static_cast<const rapidjson::SchemaDocument*>(m_compiledJsonSchema));
    return doc.Accept(validator);
    }

/*---------------------------------------------------------------------------------**//**
 @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
SchemaReadStatus JsonDescription::ReadXml(pugi::xml_node jsonDescriptionNode, ECSchemaReadContextR context)
    {
    auto nameAttr = jsonDescriptionNode.attribute(TYPE_NAME_ATTRIBUTE);
    if (!nameAttr)
        {
        LOG.errorv("Invalid ECSchemaXML: %s element must contain a %s attribute", JSON_DESCRIPTION_ELEMENT, TYPE_NAME_ATTRIBUTE);
        return SchemaReadStatus::InvalidECSchemaXml;
        }

    SetName(nameAttr.as_string());

    // optional metadata
    READ_OPTIONAL_XML_ATTRIBUTE(jsonDescriptionNode, ECXML_DISPLAY_LABEL_ATTRIBUTE, this, DisplayLabel)
    READ_OPTIONAL_XML_ATTRIBUTE(jsonDescriptionNode, DESCRIPTION_ATTRIBUTE, this, Description)

    Utf8CP text = jsonDescriptionNode.text().as_string(nullptr);
    if (Utf8String::IsNullOrEmpty(text))
        {
        LOG.errorv("Invalid ECSchemaXML: %s '%s' must have a JSON Schema body", JSON_DESCRIPTION_ELEMENT, m_name.GetName().c_str());
        return SchemaReadStatus::InvalidECSchemaXml;
        }

    if (ECObjectsStatus::Success != SetJsonSchema(text))
        {
        LOG.errorv("Invalid ECSchemaXML: %s '%s' has malformed JSON Schema content", JSON_DESCRIPTION_ELEMENT, m_name.GetName().c_str());
        return SchemaReadStatus::InvalidECSchemaXml;
        }

    return SchemaReadStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
 @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String JsonDescription::GetQualifiedName(ECSchemaCR primarySchema) const
    {
    return SchemaParseUtils::GetQualifiedName<JsonDescription>(primarySchema, *this);
    }

/*---------------------------------------------------------------------------------**//**
 @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
SchemaWriteStatus JsonDescription::WriteXml(BePugiXmlWriterR writer, ECVersion ecXmlVersion) const
    {
    if (ecXmlVersion < ECVersion::V3_3)
        return SchemaWriteStatus::Success;

    writer.WriteElementStart(JSON_DESCRIPTION_ELEMENT);
    writer.WriteAttribute(TYPE_NAME_ATTRIBUTE, m_name.GetName().c_str());

    if (!m_displayLabel.empty())
        writer.WriteAttribute(ECXML_DISPLAY_LABEL_ATTRIBUTE, m_displayLabel.c_str());
    
    if (!m_description.empty())
        writer.WriteAttribute(DESCRIPTION_ATTRIBUTE, m_description.c_str());
    
    if (!m_jsonSchema.empty())
        writer.WriteText(m_jsonSchema.c_str());

    writer.WriteElementEnd();
    return SchemaWriteStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
 @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool JsonDescription::ToJson(BeJsValue outValue, bool standalone, bool includeSchemaVersion) const
    {
    if (standalone)
        {
        if (includeSchemaVersion)
            outValue[ECJSON_PARENT_VERSION_ATTRIBUTE] = GetSchema().GetSchemaKey().GetVersionString();
        outValue[NAME_ATTRIBUTE] = GetName();
        }

    outValue[ECJSON_SCHEMA_ITEM_TYPE] = JSON_DESCRIPTION_ELEMENT;

    if (!m_displayLabel.empty())
        outValue[ECJSON_DISPLAY_LABEL_ATTRIBUTE] = m_displayLabel;

    if (!m_description.empty())
        outValue[DESCRIPTION_ATTRIBUTE] = m_description;

    if (!m_jsonSchema.empty())
        outValue["jsonSchema"] = m_jsonSchema;

    return true;
    }

END_BENTLEY_ECOBJECT_NAMESPACE