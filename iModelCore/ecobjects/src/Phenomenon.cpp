/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/

#include "ECObjectsPch.h"

BEGIN_BENTLEY_ECOBJECT_NAMESPACE

//--------------------------------------------------------------------------------------
// @bsimethod                                   Caleb.Shafer                    03/2018
//--------------------------------------------------------------------------------------
Phenomenon::Phenomenon(ECSchemaCR schema, Utf8CP name) : Phenomenon(schema, name, nullptr)
    {
    m_unitsContext = &schema.GetUnitsContext();
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                   Caleb.Shafer                    03/2018
//--------------------------------------------------------------------------------------
Phenomenon::Phenomenon(ECSchemaCR schema, Utf8CP name, Utf8CP definition) : Units::Phenomenon(name, definition), m_schema(schema), m_isDisplayLabelExplicitlyDefined(false)
    {
    m_unitsContext = &schema.GetUnitsContext();
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                   Caleb.Shafer                    03/2018
//--------------------------------------------------------------------------------------
Utf8StringCR Phenomenon::GetFullName() const
    {
    return m_fullName.GetName(*this);
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                   Kyle.Abramowitz                 02/2018
//--------------------------------------------------------------------------------------
Utf8String Phenomenon::GetQualifiedName(ECSchemaCR primarySchema) const
    {
    return SchemaParseUtils::GetQualifiedName<Phenomenon>(primarySchema, *this);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Chris.Lawson                    03/2020
//---------------------------------------------------------------------------------------
ECObjectsStatus Phenomenon ::SetDisplayLabel(Utf8StringCR label)
{
	m_isDisplayLabelExplicitlyDefined = label != "";
	Units::Phenomenon::SetLabel(label.c_str());

	return ECObjectsStatus::Success;
}

//--------------------------------------------------------------------------------------
// @bsimethod                                   Kyle.Abramowitz                 02/2018
//--------------------------------------------------------------------------------------
Utf8StringCR Phenomenon::GetDisplayLabel() const 
    {
    return GetSchema().GetLocalizedStrings().GetPhenomenonDisplayLabel(*this, GetInvariantDisplayLabel());
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                   Kyle.Abramowitz                 02/2018
//--------------------------------------------------------------------------------------
Utf8StringCR Phenomenon::GetInvariantDisplayLabel() const 
    {
    if(GetIsDisplayLabelDefined())
        return Units::Phenomenon::GetInvariantDisplayLabel();
    else
        return GetName();
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                   Kyle.Abramowitz                 02/2018
//--------------------------------------------------------------------------------------
Utf8StringCR Phenomenon::GetDescription() const 
    {
    return GetSchema().GetLocalizedStrings().GetPhenomenonDescription(*this, m_description);
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                   Kyle.Abramowitz                 02/2018
//--------------------------------------------------------------------------------------
SchemaReadStatus Phenomenon::ReadXml(BeXmlNodeR PhenomenonNode, ECSchemaReadContextR context)
    {
    Utf8String value;
    if (BEXML_Success != PhenomenonNode.GetAttributeStringValue(value, DEFINITION_ATTRIBUTE))
        {
        LOG.errorv("Invalid ECSchemaXML: The %s element must contain a %s attribute", GetName().c_str(), DEFINITION_ATTRIBUTE);
        return SchemaReadStatus::InvalidECSchemaXml;
        }

    if (SUCCESS != SetDefinition(value.c_str()))
        {
        LOG.errorv("Invalid ECSchemaXML: The Phenomenon %s contains an invalid %s attribute", GetName().c_str(), DEFINITION_ATTRIBUTE);
        return SchemaReadStatus::InvalidECSchemaXml;
        }

    READ_OPTIONAL_XML_ATTRIBUTE(PhenomenonNode, DESCRIPTION_ATTRIBUTE, this, Description)
    READ_OPTIONAL_XML_ATTRIBUTE(PhenomenonNode, ECXML_DISPLAY_LABEL_ATTRIBUTE, this, DisplayLabel)

    return SchemaReadStatus::Success;
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                   Kyle.Abramowitz                 02/2018
//--------------------------------------------------------------------------------------
SchemaWriteStatus Phenomenon::WriteXml(BeXmlWriterR xmlWriter, ECVersion ecXmlVersion) const
    {
    if (ecXmlVersion < ECVersion::V3_2)
        return SchemaWriteStatus::Success;

    Utf8CP elementName = PHENOMENON_ELEMENT;
    SchemaWriteStatus status = SchemaWriteStatus::Success;

    xmlWriter.WriteElementStart(elementName);

    xmlWriter.WriteAttribute(TYPE_NAME_ATTRIBUTE, this->GetName().c_str());
    xmlWriter.WriteAttribute(DEFINITION_ATTRIBUTE, this->GetDefinition().c_str());
    xmlWriter.WriteAttribute(DESCRIPTION_ATTRIBUTE, GetInvariantDescription().c_str());

    if (GetIsDisplayLabelDefined())
        xmlWriter.WriteAttribute(ECXML_DISPLAY_LABEL_ATTRIBUTE, GetInvariantDisplayLabel().c_str());

    xmlWriter.WriteElementEnd();
    return status;
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                   Kyle.Abramowitz                 02/2018
//--------------------------------------------------------------------------------------
bool Phenomenon::ToJson(Json::Value& outValue, bool includeSchemaVersion) const
    {
    return ToJson(outValue, true, includeSchemaVersion);
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                   Kyle.Abramowitz                 02/2018
//--------------------------------------------------------------------------------------
bool Phenomenon::ToJson(Json::Value& outValue, bool standalone, bool includeSchemaVersion) const
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

    outValue[ECJSON_SCHEMA_ITEM_TYPE] = PHENOMENON_ELEMENT;
    outValue[DEFINITION_ATTRIBUTE] = GetDefinition();
    
    if (GetIsDisplayLabelDefined())
        outValue[ECJSON_DISPLAY_LABEL_ATTRIBUTE] = GetInvariantDisplayLabel();
    if (GetIsDescriptionDefined())
        outValue[DESCRIPTION_ATTRIBUTE] = GetInvariantDescription();

    return true;
    }

END_BENTLEY_ECOBJECT_NAMESPACE
