/*--------------------------------------------------------------------------------------+
|
|     $Source: src/UnitSystem.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#include "ECObjectsPch.h"

BEGIN_BENTLEY_ECOBJECT_NAMESPACE

// If you are developing schemas, particularly when editing them by hand, you want to have this variable set to false so you get the asserts to help you figure out what is going wrong.
// Test programs generally want to get error status back and not BeAssert, so they call ECSchema::AssertOnXmlError (false);
static bool s_noAssert = false;

//--------------------------------------------------------------------------------------
// @bsimethod                                   Caleb.Shafer                    02/2018
//--------------------------------------------------------------------------------------
// static
UnitSystemP UnitSystem::_Create(Utf8CP name)
    {
    // Unfortunately we need to do the name encoding here instead of using the ECValidatedName struct in order to use the name required in Units::UnitSystem.
    if (!ECNameValidation::IsValidName(name))
        {
        LOG.errorv("A UnitSystem cannot be created with the name '%s' because it is not a valid ECName", name);
        return nullptr;
        }

    Utf8String encodedName;
    ECNameValidation::EncodeToValidName(encodedName, name);

    auto ptrSystem = new UnitSystem(encodedName.c_str());
    if (nullptr == ptrSystem)
        return nullptr;

    ECNameValidation::DecodeFromValidName(ptrSystem->m_displayLabel, ptrSystem->GetName().c_str());

    return ptrSystem;
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                   Caleb.Shafer                    01/2018
//--------------------------------------------------------------------------------------
Utf8StringCR UnitSystem::GetFullName() const
    {
    if (m_fullName.size() == 0) 
        m_fullName = m_schema->GetName() + ":" + GetName(); 
    return m_fullName; 
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
ECObjectsStatus UnitSystem::SetDisplayLabel(Utf8CP value) 
    {
    if (Utf8String::IsNullOrEmpty(value))
        {
        m_explicitDisplayLabel = false;
        m_displayLabel.clear();
        ECNameValidation::DecodeFromValidName(m_displayLabel, GetName());
        }
    else
        {
        m_explicitDisplayLabel = true;
        m_displayLabel = value;
        }
    return ECObjectsStatus::Success;
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
    if (BEXML_Success == unitSystemNode.GetAttributeStringValue(value, TYPE_NAME_ATTRIBUTE) && Utf8String::IsNullOrEmpty(value.c_str()))
        {
        LOG.errorv("Invalid ECSchemaXML: The %s element must contain a Name attribute", unitSystemNode.GetName());
        return SchemaReadStatus::InvalidECSchemaXml;
        }

    system = Units::UnitRegistry::Instance().AddSystem<UnitSystem>(value.c_str());
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
    if (GetInvariantDescription().length())
        outValue[DESCRIPTION_ATTRIBUTE] = GetInvariantDescription();

    return SchemaWriteStatus::Success;
    }

END_BENTLEY_ECOBJECT_NAMESPACE
