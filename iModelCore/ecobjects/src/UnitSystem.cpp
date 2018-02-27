/*--------------------------------------------------------------------------------------+
|
|     $Source: src/UnitSystem.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#include "ECObjectsPch.h"

BEGIN_BENTLEY_ECOBJECT_NAMESPACE

//--------------------------------------------------------------------------------------
// @bsimethod                                   Caleb.Shafer                    02/2018
//--------------------------------------------------------------------------------------
// static
UnitSystemP UnitSystem::_Create(Utf8CP name)
    {
    // Deconstruct the name, the format should be {SchemaName}.{UnitSystemName}
    Utf8String schemaName;
    Utf8String systemName;
    ECClass::ParseClassName(schemaName, systemName, name);
    BeAssert(!schemaName.empty());

    // Check if it's a valid name here to avoid having to construct the UnitSystem unnecessarily.
    if (!ECNameValidation::IsValidName(systemName.c_str()))
        {
        LOG.errorv("A UnitSystem cannot be created with the name '%s' because it is not a valid ECName", systemName.c_str());
        return nullptr;
        }

    auto systemP = new UnitSystem(name);
    if (nullptr == systemP)
        return nullptr;

    systemP->SetName(systemName);
    return systemP;
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                   Caleb.Shafer                    01/2018
//--------------------------------------------------------------------------------------
Utf8String UnitSystem::GetQualifiedName(ECSchemaCR primarySchema) const
    {
    Utf8String alias;
    Utf8StringCR name = GetName();
    if (!EXPECTED_CONDITION (ECObjectsStatus::Success == primarySchema.ResolveAlias(GetSchema(), alias)))
        {
        LOG.warningv ("warning: Cannot qualify an UnitSystem name with an alias unless the schema containing the UnitSystem is referenced by the primary schema."
            "The name will remain unqualified.\n  Primary ECSchema: %s\n  UnitSystem: %s\n ECSchema containing UnitSystem: %s", primarySchema.GetName().c_str(), name.c_str(), GetSchema().GetName().c_str());
        return name;
        }

    if (alias.empty())
        return name;
    else
        return alias + ":" + name;
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                   Caleb.Shafer                    01/2018
//--------------------------------------------------------------------------------------
Utf8StringCR UnitSystem::GetDisplayLabel() const 
    {
    return GetSchema().GetLocalizedStrings().GetUnitSystemDisplayLabel(*this, GetInvariantDisplayLabel());
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                   Caleb.Shafer                    01/2018
//--------------------------------------------------------------------------------------
Utf8StringCR UnitSystem::GetDescription() const 
    {
    return GetSchema().GetLocalizedStrings().GetUnitSystemDescription(*this, m_description);
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                   Caleb.Shafer                    01/2018
//--------------------------------------------------------------------------------------
SchemaReadStatus UnitSystem::ReadXml(UnitSystemP& system, BeXmlNodeR unitSystemNode, ECSchemaCR schema, ECSchemaReadContextR context)
    {
    Utf8String value;
    if (BEXML_Success != unitSystemNode.GetAttributeStringValue(value, TYPE_NAME_ATTRIBUTE) || Utf8String::IsNullOrEmpty(value.c_str()))
        {
        LOG.errorv("Invalid ECSchemaXML: The %s element must contain a Name attribute", unitSystemNode.GetName());
        return SchemaReadStatus::InvalidECSchemaXml;
        }

    Utf8String fullName = schema.GetName() + ":" + value;
    system = Units::UnitRegistry::Instance().AddSystem<UnitSystem>(fullName.c_str());
    if (nullptr == system)
        return SchemaReadStatus::InvalidECSchemaXml;

    system->SetSchema(schema);

    READ_OPTIONAL_XML_ATTRIBUTE(unitSystemNode, DESCRIPTION_ATTRIBUTE, system, Description)
    READ_OPTIONAL_XML_ATTRIBUTE(unitSystemNode, ECXML_DISPLAY_LABEL_ATTRIBUTE, system, DisplayLabel)

    return SchemaReadStatus::Success;
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                   Caleb.Shafer                    01/2018
//--------------------------------------------------------------------------------------
SchemaWriteStatus UnitSystem::WriteXml(BeXmlWriterR xmlWriter, ECVersion ecXmlVersion) const
    {
    if (ecXmlVersion < ECVersion::V3_2)
        return SchemaWriteStatus::Success;

    Utf8CP elementName = UNIT_SYSTEM_ELEMENT;
    SchemaWriteStatus status = SchemaWriteStatus::Success;

    xmlWriter.WriteElementStart(elementName);

    xmlWriter.WriteAttribute(TYPE_NAME_ATTRIBUTE, GetName().c_str());
    if (GetIsDescriptionDefined())
        xmlWriter.WriteAttribute(DESCRIPTION_ATTRIBUTE, GetInvariantDescription().c_str());
    if (GetIsDisplayLabelDefined())
        xmlWriter.WriteAttribute(ECXML_DISPLAY_LABEL_ATTRIBUTE, GetInvariantDisplayLabel().c_str());

    xmlWriter.WriteElementEnd();
    return status;
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                   Caleb.Shafer                    01/2018
//--------------------------------------------------------------------------------------
SchemaWriteStatus UnitSystem::WriteJson(Json::Value& outValue, bool includeSchemaVersion) const
    {
    return WriteJson(outValue, true, includeSchemaVersion);
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                   Caleb.Shafer                    01/2018
//--------------------------------------------------------------------------------------
SchemaWriteStatus UnitSystem::WriteJson(Json::Value& outValue, bool standalone, bool includeSchemaVersion) const
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

    outValue[ECJSON_SCHEMA_CHILD_TYPE] = UNIT_SYSTEM_ELEMENT;

    if (GetIsDisplayLabelDefined())
        outValue[ECJSON_DISPLAY_LABEL_ATTRIBUTE] = GetInvariantDisplayLabel();
    if (GetIsDescriptionDefined())
        outValue[DESCRIPTION_ATTRIBUTE] = GetInvariantDescription();

    return SchemaWriteStatus::Success;
    }

END_BENTLEY_ECOBJECT_NAMESPACE
