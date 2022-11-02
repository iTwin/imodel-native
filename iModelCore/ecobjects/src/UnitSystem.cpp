/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/

#include "ECObjectsPch.h"

BEGIN_BENTLEY_ECOBJECT_NAMESPACE

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------+---------------+---------------+---------------+---------------+-------
UnitSystem::UnitSystem(ECSchemaCR schema, Utf8CP name) : Units::UnitSystem(name), m_schema(schema), m_hasExplicitDisplayLabel(false)
    {
    m_unitsContext = &schema.GetUnitsContext();
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------+---------------+---------------+---------------+---------------+-------
Utf8StringCR UnitSystem::GetFullName() const
    {
    return m_fullName.GetName(*this);
    }

//--------------------------------------------------------------------------------------
// @bsimethod
//--------------------------------------------------------------------------------------
Utf8String UnitSystem::GetQualifiedName(ECSchemaCR primarySchema) const
    {
    return SchemaParseUtils::GetQualifiedName<UnitSystem>(primarySchema, *this);
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
ECObjectsStatus UnitSystem::SetDisplayLabel(Utf8StringCR label)
{
	m_hasExplicitDisplayLabel = label != "";
	m_displayLabel = label.c_str();

	return ECObjectsStatus::Success;
}

//--------------------------------------------------------------------------------------
// @bsimethod
//--------------------------------------------------------------------------------------
Utf8StringCR UnitSystem::GetDisplayLabel() const
    {
    return GetInvariantDisplayLabel();
    }

//--------------------------------------------------------------------------------------
// @bsimethod
//--------------------------------------------------------------------------------------
Utf8StringCR UnitSystem::GetDescription() const
    {
    return m_description;
    }

//--------------------------------------------------------------------------------------
// @bsimethod
//--------------------------------------------------------------------------------------
SchemaReadStatus UnitSystem::ReadXml(pugi::xml_node unitSystemNode, ECSchemaReadContextR context)
    {
    Utf8String value;
    READ_OPTIONAL_XML_ATTRIBUTE(unitSystemNode, DESCRIPTION_ATTRIBUTE, this, Description)
    READ_OPTIONAL_XML_ATTRIBUTE(unitSystemNode, ECXML_DISPLAY_LABEL_ATTRIBUTE, this, DisplayLabel)

    return SchemaReadStatus::Success;
    }

//--------------------------------------------------------------------------------------
// @bsimethod
//--------------------------------------------------------------------------------------
SchemaWriteStatus UnitSystem::WriteXml(BeXmlWriterR xmlWriter, ECVersion ecXmlVersion) const
    {
    if (ecXmlVersion < ECVersion::V3_2)
        return SchemaWriteStatus::Success;

    Utf8CP elementName = UNIT_SYSTEM_ELEMENT;
    SchemaWriteStatus status = SchemaWriteStatus::Success;

    xmlWriter.WriteElementStart(elementName);

    xmlWriter.WriteAttribute(TYPE_NAME_ATTRIBUTE, GetName().c_str());
    xmlWriter.WriteAttribute(DESCRIPTION_ATTRIBUTE, GetInvariantDescription().c_str());

    if (GetIsDisplayLabelDefined())
        xmlWriter.WriteAttribute(ECXML_DISPLAY_LABEL_ATTRIBUTE, GetInvariantDisplayLabel().c_str());

    xmlWriter.WriteElementEnd();
    return status;
    }

//--------------------------------------------------------------------------------------
// @bsimethod
//--------------------------------------------------------------------------------------
bool UnitSystem::ToJson(BeJsValue outValue, bool includeSchemaVersion) const
    {
    return ToJson(outValue, true, includeSchemaVersion);
    }

//--------------------------------------------------------------------------------------
// @bsimethod
//--------------------------------------------------------------------------------------
bool UnitSystem::ToJson(BeJsValue outValue, bool standalone, bool includeSchemaVersion) const
    {
    // Common properties to all Schema children
    if (standalone)
        {
        outValue[ECJSON_URI_SPEC_ATTRIBUTE] = ECJSON_SCHEMA_ITEM_URI;
        outValue[ECJSON_PARENT_SCHEMA_ATTRIBUTE] = GetSchema().GetName();
        if (includeSchemaVersion)
            outValue[ECJSON_PARENT_VERSION_ATTRIBUTE] = GetSchema().GetSchemaKey().GetVersionString();
        outValue[NAME_ATTRIBUTE] = GetName();
        }

    outValue[ECJSON_SCHEMA_ITEM_TYPE] = UNIT_SYSTEM_ELEMENT;

    if (GetIsDisplayLabelDefined())
        outValue[ECJSON_DISPLAY_LABEL_ATTRIBUTE] = GetInvariantDisplayLabel();
    if (GetIsDescriptionDefined())
        outValue[DESCRIPTION_ATTRIBUTE] = GetInvariantDescription();

    return true;
    }

END_BENTLEY_ECOBJECT_NAMESPACE
