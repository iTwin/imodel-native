/*--------------------------------------------------------------------------------------+
|
|     $Source: src/Phenomenon.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#include "ECObjectsPch.h"

BEGIN_BENTLEY_ECOBJECT_NAMESPACE

//--------------------------------------------------------------------------------------
// @bsimethod                                   Kyle.Abramowitz                 02/2018
//--------------------------------------------------------------------------------------
// static
PhenomenonP Phenomenon::_Create(Utf8CP name, Utf8CP definition, uint32_t id)
    {
    // Deconstruct the name. The format should be {SchemaName}.{PhenomenonName}
    Utf8String schemaName;
    Utf8String phenomName;
    ECClass::ParseClassName(schemaName, phenomName, name);
    BeAssert(!schemaName.empty());

    // Check if it's a valid name here to avoid having to construct the Phenomenon Unnecessarily
    if (!ECNameValidation::IsValidName(phenomName.c_str()))
        {
        LOG.errorv("A Phenomenon cannot be created with the name '%s' because it is not a valid ECName", phenomName.c_str());
        return nullptr;
        }

    auto ptrPhenomenon = new Phenomenon(name, definition, id);
    if (nullptr == ptrPhenomenon)
        return nullptr;

    ptrPhenomenon->SetName(phenomName.c_str());
    return ptrPhenomenon;
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                   Kyle.Abramowitz                 02/2018
//--------------------------------------------------------------------------------------
Utf8String Phenomenon::GetQualifiedName(ECSchemaCR primarySchema) const
    {
    Utf8String alias;
    Utf8StringCR name = GetName();
    if (!EXPECTED_CONDITION (ECObjectsStatus::Success == primarySchema.ResolveAlias(GetSchema(), alias)))
        {
        LOG.warningv ("warning: Cannot qualify an Phenomenon name with an alias unless the schema containing the Phenomenon is referenced by the primary schema."
            "The name will remain unqualified.\n  Primary ECSchema: %s\n  Phenomenon: %s\n ECSchema containing Phenomenon: %s", primarySchema.GetName().c_str(), name.c_str(), GetSchema().GetName().c_str());
        return name;
        }

    if (alias.empty())
        return name;
    else
        return alias + ":" + name;
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
        return Units::Phenomenon::GetInvariantLabel();
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
ECObjectsStatus Phenomenon::SetName(Utf8StringCR name) 
    {
    if(!ECNameValidation::IsValidName(name.c_str()))
        return ECObjectsStatus::InvalidName;

    m_name = name.c_str();
    return ECObjectsStatus::Success;
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                   Kyle.Abramowitz                 02/2018
//--------------------------------------------------------------------------------------
SchemaReadStatus Phenomenon::ReadXml(PhenomenonP& phenomenon, BeXmlNodeR PhenomenonNode, ECSchemaCR schema, ECSchemaReadContextR context)
    {
    Utf8String value;
    if (BEXML_Success != PhenomenonNode.GetAttributeStringValue(value, TYPE_NAME_ATTRIBUTE) || Utf8String::IsNullOrEmpty(value.c_str()))
        {
        LOG.errorv("Invalid ECSchemaXML: The %s element must contain a Name attribute", PhenomenonNode.GetName());
        return SchemaReadStatus::InvalidECSchemaXml;
        }
    Utf8String definition;
    if (BEXML_Success != PhenomenonNode.GetAttributeStringValue(definition, DEFINITION_ATTRIBUTE) || Utf8String::IsNullOrEmpty(definition.c_str()))
        {
        LOG.errorv("Invalid ECSchemaXML: The %s element must contain a Definition attribute", PhenomenonNode.GetName());
        return SchemaReadStatus::InvalidECSchemaXml;
        }
    Utf8String fullName = schema.GetName()+ ":" + value;
    phenomenon = Units::UnitRegistry::Instance().AddPhenomenon<Phenomenon>(fullName.c_str(), definition.c_str());
    if (nullptr == phenomenon)
        return SchemaReadStatus::InvalidECSchemaXml;

    phenomenon->SetSchema(schema);

    READ_OPTIONAL_XML_ATTRIBUTE(PhenomenonNode, DESCRIPTION_ATTRIBUTE, phenomenon, Description)
    READ_OPTIONAL_XML_ATTRIBUTE(PhenomenonNode, ECXML_DISPLAY_LABEL_ATTRIBUTE, phenomenon, DisplayLabel)

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
SchemaWriteStatus Phenomenon::WriteJson(Json::Value& outValue, bool includeSchemaVersion) const
    {
    return WriteJson(outValue, true, includeSchemaVersion);
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                   Kyle.Abramowitz                 02/2018
//--------------------------------------------------------------------------------------
SchemaWriteStatus Phenomenon::WriteJson(Json::Value& outValue, bool standalone, bool includeSchemaVersion) const
    {
    // Common properties to all Schema children
    if (standalone)
        {
        outValue[ECJSON_URI_SPEC_ATTRIBUTE] = ECJSON_SCHEMA_CHILD_URI;
        outValue[ECJSON_PARENT_SCHEMA_ATTRIBUTE] = GetSchema().GetName();
        if (includeSchemaVersion)
            outValue[ECJSON_PARENT_VERSION_ATTRIBUTE] = GetSchema().GetSchemaKey().GetVersionString();
        outValue[NAME_ATTRIBUTE] = GetName();
        }

    outValue[ECJSON_SCHEMA_CHILD_TYPE] = PHENOMENON_ELEMENT;
    outValue[DEFINITION_ATTRIBUTE] = GetDefinition();
    if (GetIsDisplayLabelDefined())
        outValue[ECJSON_DISPLAY_LABEL_ATTRIBUTE] = GetInvariantDisplayLabel();
    if (GetInvariantDescription().length())
        outValue[DESCRIPTION_ATTRIBUTE] = GetInvariantDescription();

    return SchemaWriteStatus::Success;
    }

END_BENTLEY_ECOBJECT_NAMESPACE
