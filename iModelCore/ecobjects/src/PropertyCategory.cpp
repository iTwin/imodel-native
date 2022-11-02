/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/

#include "ECObjectsPch.h"

BEGIN_BENTLEY_ECOBJECT_NAMESPACE

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------+---------------+---------------+---------------+---------------+-------
ECObjectsStatus PropertyCategory::SetName(Utf8CP name)
    {
    if (!m_validatedName.SetValidName(name, false))
        return ECObjectsStatus::InvalidName;

    m_fullName.RecomputeName(*this);
    return ECObjectsStatus::Success;
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------+---------------+---------------+---------------+---------------+-------
Utf8StringCR PropertyCategory::GetFullName() const
    {
    return m_fullName.GetName(*this);
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------+---------------+---------------+---------------+---------------+-------
Utf8String PropertyCategory::GetQualifiedName(ECSchemaCR primarySchema) const
    {
    Utf8String alias;
    Utf8StringCR name = GetName();
    if (!EXPECTED_CONDITION (ECObjectsStatus::Success == primarySchema.ResolveAlias (GetSchema(), alias)))
        {
        LOG.warningv ("warning: Cannot qualify a PropertyCategory name with an alias unless the schema containing the PropertyCategry is referenced by the primary schema."
            "The name will remain unqualified.\n  Primary ECSchema: %s\n  PropertyCategory: %s\n ECSchema containing PropertyCategory: %s", primarySchema.GetName().c_str(), name.c_str(), GetSchema().GetName().c_str());
        return name;
        }

    if (alias.empty())
        return name;
    else
        return alias + ":" + name;
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------+---------------+---------------+---------------+---------------+-------
ECObjectsStatus PropertyCategory::SetDisplayLabel(Utf8StringCR displayLabel)
    {
    m_validatedName.SetDisplayLabel(displayLabel.c_str());
    return ECObjectsStatus::Success;
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------+---------------+---------------+---------------+---------------+-------
Utf8StringCR PropertyCategory::GetDisplayLabel() const
    {
    return GetInvariantDisplayLabel();
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------+---------------+---------------+---------------+---------------+-------
Utf8StringCR PropertyCategory::GetDescription() const
    {
    return m_description;
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------+---------------+---------------+---------------+---------------+-------
SchemaReadStatus PropertyCategory::ReadXml(pugi::xml_node propertyCategoryNode, ECSchemaReadContextR context)
    {
    Utf8String value;
    READ_REQUIRED_XML_ATTRIBUTE(propertyCategoryNode, TYPE_NAME_ATTRIBUTE, this, Name, propertyCategoryNode.name())
    READ_OPTIONAL_XML_ATTRIBUTE(propertyCategoryNode, ECXML_DISPLAY_LABEL_ATTRIBUTE, this, DisplayLabel)
    READ_OPTIONAL_XML_ATTRIBUTE(propertyCategoryNode, DESCRIPTION_ATTRIBUTE, this, Description)

    uint32_t priority = 0;
    if (BePugiXmlHelper::GetAttributeUInt32Value(propertyCategoryNode, priority, PRIORITY_ATTRIBUTE) == BePugiXmlValueResult::Success)
        SetPriority(priority);

    return SchemaReadStatus::Success;
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------+---------------+---------------+---------------+---------------+-------
SchemaWriteStatus PropertyCategory::WriteXml(BeXmlWriterR xmlWriter, ECVersion ecXmlVersion) const
    {
    if (ecXmlVersion < ECVersion::V3_1)
        return SchemaWriteStatus::Success;

    Utf8CP elementName = PROPERTY_CATEGORY_ELEMENT;
    SchemaWriteStatus status = SchemaWriteStatus::Success;

    xmlWriter.WriteElementStart(elementName);

    xmlWriter.WriteAttribute(TYPE_NAME_ATTRIBUTE, GetName().c_str());
    xmlWriter.WriteAttribute(DESCRIPTION_ATTRIBUTE, GetInvariantDescription().c_str());
    if (GetIsDisplayLabelDefined())
        xmlWriter.WriteAttribute(ECXML_DISPLAY_LABEL_ATTRIBUTE, GetInvariantDisplayLabel().c_str());
    xmlWriter.WriteAttribute(PRIORITY_ATTRIBUTE, GetPriority());

    xmlWriter.WriteElementEnd();
    return status;
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------+---------------+---------------+---------------+---------------+-------
bool PropertyCategory::ToJson(BeJsValue outValue, bool standalone, bool includeSchemaVersion) const
    {
    // Common properties to all Schema items
    if (standalone)
        {
        outValue[ECJSON_URI_SPEC_ATTRIBUTE] = ECJSON_SCHEMA_ITEM_URI;
        outValue[ECJSON_PARENT_SCHEMA_ATTRIBUTE] = GetSchema().GetName();
        if (includeSchemaVersion)
            outValue[ECJSON_PARENT_VERSION_ATTRIBUTE] = GetSchema().GetSchemaKey().GetVersionString();
        outValue[NAME_ATTRIBUTE] = GetName();
        }

    outValue[ECJSON_SCHEMA_ITEM_TYPE] = PROPERTY_CATEGORY_ELEMENT;

    if (GetIsDisplayLabelDefined())
        outValue[ECJSON_DISPLAY_LABEL_ATTRIBUTE] = GetInvariantDisplayLabel();
    if (GetInvariantDescription().length())
        outValue[DESCRIPTION_ATTRIBUTE] = GetInvariantDescription();

    // Property Category properties
    outValue[PRIORITY_ATTRIBUTE] = GetPriority();
    return true;
    }

END_BENTLEY_ECOBJECT_NAMESPACE
