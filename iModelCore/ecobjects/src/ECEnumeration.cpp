/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/

#include "ECObjectsPch.h"

BEGIN_BENTLEY_ECOBJECT_NAMESPACE
// If you are developing schemas, particularly when editing them by hand, you want to have this variable set to false so you get the asserts to help you figure out what is going wrong.
// Test programs generally want to get error status back and not BeAssert, so they call ECSchema::AssertOnXmlError (false);
static  bool        s_noAssert = false;

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------+---------------+---------------+---------------+---------------+-------
void ECEnumeration::SetDisplayLabel(Utf8CP displayLabel) {m_validatedName.SetDisplayLabel(displayLabel);}
Utf8StringCR ECEnumeration::GetDescription() const {return GetSchema().GetLocalizedStrings().GetEnumerationDescription(*this, GetInvariantDescription());}
Utf8String ECEnumeration::GetTypeName() const {return SchemaParseUtils::PrimitiveTypeToString(m_primitiveType);}
Utf8StringCR ECEnumeration::GetDisplayLabel() const {return GetSchema().GetLocalizedStrings().GetEnumerationDisplayLabel(*this, GetInvariantDisplayLabel());}

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------+---------------+---------------+---------------+---------------+-------
ECObjectsStatus ECEnumeration::SetName(Utf8CP name)
    {
    if (!m_validatedName.SetValidName(name, false))
        return ECObjectsStatus::InvalidName;

    m_fullName.RecomputeName(*this);

    return ECObjectsStatus::Success;
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------+---------------+---------------+---------------+---------------+-------
Utf8StringCR ECEnumeration::GetFullName () const
    {
    return m_fullName.GetName(*this);
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                   Caleb.Shafer                    06/2018
//--------------------------------------------------------------------------------------
Utf8String ECEnumeration::GetQualifiedName(ECSchemaCR primarySchema) const
    {
    return SchemaParseUtils::GetQualifiedName<ECEnumeration>(primarySchema, *this);
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------+---------------+---------------+---------------+---------------+-------
ECObjectsStatus ECEnumeration::SetType(PrimitiveType value)
    {
    if (value != PRIMITIVETYPE_Integer && value != PRIMITIVETYPE_String
        && !GetSchema().OriginalECXmlVersionGreaterThan(ECVersion::Latest)) // If is it greater than we know about allow the parsed primitive type to be set.
        return ECObjectsStatus::DataTypeNotSupported;

    m_primitiveType = value;
    return ECObjectsStatus::Success;
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------+---------------+---------------+---------------+---------------+-------
ECObjectsStatus ECEnumeration::SetTypeName(Utf8CP typeName)
    {
    PrimitiveType primitiveType;
    ECObjectsStatus status = SchemaParseUtils::ParsePrimitiveType(primitiveType, typeName);
    if (ECObjectsStatus::Success != status)
        {
        if (!GetSchema().OriginalECXmlVersionGreaterThan(ECVersion::Latest))
            return status;

        LOG.warningv("ECEnumeration '%s' has an unknown backing type '%s'. Setting to string", GetFullName().c_str(), typeName);
        primitiveType = PRIMITIVETYPE_String;
        }

    return SetType(primitiveType);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Robert.Schili                  11/2015
+---------------+---------------+---------------+---------------+---------------+------*/
SchemaWriteStatus ECEnumeration::WriteXml (BeXmlWriterR xmlWriter, ECVersion ecXmlVersion) const
    {
    if (ecXmlVersion < ECVersion::V3_0) //Enumerations will only be serialized in 3.0 and later
        return SchemaWriteStatus::Success;

    xmlWriter.WriteElementStart(ECXML_ENUMERATION_ELEMENT);

    xmlWriter.WriteAttribute(TYPE_NAME_ATTRIBUTE, this->GetName().c_str());
    xmlWriter.WriteAttribute(ECXML_BACKING_TYPE_NAME_ATTRIBUTE, GetTypeName().c_str());
    xmlWriter.WriteAttribute(DESCRIPTION_ATTRIBUTE, this->GetInvariantDescription().c_str());
    if (GetIsDisplayLabelDefined())
        xmlWriter.WriteAttribute(ECXML_DISPLAY_LABEL_ATTRIBUTE, this->GetInvariantDisplayLabel().c_str());

    xmlWriter.WriteAttribute(IS_STRICT_ATTRIBUTE, this->GetIsStrict());

    bool isIntType = GetType() == PrimitiveType::PRIMITIVETYPE_Integer;
    for (auto enumerator : m_enumeratorList)
        {
        xmlWriter.WriteElementStart(ECXML_ENUMERATOR_ELEMENT);

        if (ecXmlVersion >= ECVersion::V3_2)
            {
            xmlWriter.WriteAttribute(NAME_ATTRIBUTE, enumerator->GetName().c_str());
            if (enumerator->GetInvariantDescription().length() > 0)
                xmlWriter.WriteAttribute(DESCRIPTION_ATTRIBUTE, enumerator->GetInvariantDescription().c_str());
            }
        if(isIntType)
            xmlWriter.WriteAttribute(ENUMERATOR_VALUE_ATTRIBUTE, enumerator->GetInteger());
        else
            xmlWriter.WriteAttribute(ENUMERATOR_VALUE_ATTRIBUTE, enumerator->GetString().c_str());

        if(enumerator->GetIsDisplayLabelDefined())
            xmlWriter.WriteAttribute(ECXML_DISPLAY_LABEL_ATTRIBUTE, enumerator->GetInvariantDisplayLabel().c_str());

        xmlWriter.WriteElementEnd();
        }

    xmlWriter.WriteElementEnd();
    return SchemaWriteStatus::Success;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Victor.Cushman              11/2017
//---------------+---------------+---------------+---------------+---------------+-------
bool ECEnumeration::ToJson(Json::Value& outValue, bool standalone, bool includeSchemaVersion) const
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

    outValue[ECJSON_SCHEMA_ITEM_TYPE] = ECJSON_ENUMERATION_ELEMENT;

    if (this->GetIsDisplayLabelDefined())
        outValue[ECJSON_DISPLAY_LABEL_ATTRIBUTE] = GetInvariantDisplayLabel();
    if (0 != this->GetInvariantDescription().length())
        outValue[DESCRIPTION_ATTRIBUTE] = GetInvariantDescription();

    // ECEnumeration Properties
    outValue[TYPE_ATTRIBUTE] = GetTypeName();
    outValue[IS_STRICT_ATTRIBUTE] = GetIsStrict();

    Json::Value enumeratorArr = Json::Value(Json::ValueType::arrayValue);
    for (const auto enumerator : GetEnumerators())
        {
        Json::Value enumeratorObj = Json::Value(Json::ValueType::objectValue);
        if (GetSchema().GetECVersion() >= ECVersion::V3_2)
            {
            enumeratorObj[NAME_ATTRIBUTE] = enumerator->GetName();
            if (enumerator->GetInvariantDescription().length() > 0)
                enumeratorObj[DESCRIPTION_ATTRIBUTE] = enumerator->GetInvariantDescription();
            }
        if (enumerator->GetIsDisplayLabelDefined())
            enumeratorObj[ECJSON_DISPLAY_LABEL_ATTRIBUTE] = enumerator->GetInvariantDisplayLabel();
        if (GetTypeName() == EC_PRIMITIVE_TYPENAME_INTEGER)
            enumeratorObj[ENUMERATOR_VALUE_ATTRIBUTE] = Json::Value(enumerator->GetInteger());
        else if (GetTypeName() == EC_PRIMITIVE_TYPENAME_STRING)
            enumeratorObj[ENUMERATOR_VALUE_ATTRIBUTE] = Json::Value(enumerator->GetString());
        else
            return false;
        enumeratorArr.append(enumeratorObj);
        }
    outValue[ECJSON_ENUMERATOR_ELEMENT] = enumeratorArr;

    return true;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Victor.Cushman                 01/2018
//---------------+---------------+---------------+---------------+---------------+-------
bool ECEnumeration::EnumeratorIsUnique(Utf8CP enumeratorName, int32_t enumeratorValue) const
    {
    for (auto const& entry : m_enumeratorList)
        {
        if (entry->GetName().EqualsI(enumeratorName) || entry->GetInteger() == enumeratorValue)
            return false;
        }

    return true;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Victor.Cushman                 01/2018
//---------------+---------------+---------------+---------------+---------------+-------
bool ECEnumeration::EnumeratorIsUnique(Utf8CP enumeratorName, Utf8CP enumeratorValue) const
    {
    for (auto const& entry : m_enumeratorList)
        {
        if (entry->GetName().EqualsI(enumeratorName) || entry->GetString().EqualsI(enumeratorValue))
            return false;
        }

    return true;
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------+---------------+---------------+---------------+---------------+-------
static BentleyStatus createEnumeratorFromXmlNode(ECEnumerationP enumeration, BeXmlNodeP childNode)
    {
    PrimitiveType const primitiveType = enumeration->GetType();
    Utf8CP childNodeName = childNode->GetName();
    if (0 != strcmp(childNodeName, ECXML_ENUMERATOR_ELEMENT))
        {
        if (enumeration->GetSchema().OriginalECXmlVersionGreaterThan(ECVersion::Latest))
            return SUCCESS;
        return ERROR;
        }

    // Enumerator Value
    int32_t enumeratorValueInteger {};
    Utf8String enumeratorValueString;
    BeXmlStatus xmlStatus {};
    if (PrimitiveType::PRIMITIVETYPE_Integer == primitiveType)
        xmlStatus = childNode->GetAttributeInt32Value(enumeratorValueInteger, ENUMERATOR_VALUE_ATTRIBUTE);
    else if (PrimitiveType::PRIMITIVETYPE_String == primitiveType)
        xmlStatus = childNode->GetAttributeStringValue(enumeratorValueString, ENUMERATOR_VALUE_ATTRIBUTE);
    else if (enumeration->GetSchema().OriginalECXmlVersionGreaterThan(ECVersion::Latest))
        {
        LOG.warningv("Enumeration %s has unknown primitive type possibly because of newer version", enumeration->GetName().c_str());
        return SUCCESS;
        }
    else
        {
        LOG.errorv("Enumeration %s has invalid primitive type", enumeration->GetName().c_str());
        return ERROR;
        }

    if (BeXmlStatus::BEXML_Success != xmlStatus)
        {
        LOG.errorv("Missing xml element '%s' on ECEnumerator for Enumeration '%s'.", ENUMERATOR_VALUE_ATTRIBUTE, enumeration->GetName().c_str());
        return ERROR;
        }

    // Enumerator Display Label
    Utf8String enumeratorDisplayLabel;
    bool enumeratorHasDisplayLabel = BeXmlStatus::BEXML_Success == childNode->GetAttributeStringValue(enumeratorDisplayLabel, ECXML_DISPLAY_LABEL_ATTRIBUTE);

    // Enumerator Name and Description
    // Optional enumerator descriptions were added in ECVersion::V3_2 so unless the original schema version is >= 3.2 and the description is
    // explicitly defined on the xml node, the enumeration will description will be left as the empty string.
    Utf8String enumeratorName;
    Utf8String enumeratorDescription;
    if (enumeration->GetSchema().OriginalECXmlVersionAtLeast(ECVersion::V3_2))
        {
        if (BeXmlStatus::BEXML_Success != childNode->GetAttributeStringValue(enumeratorName, NAME_ATTRIBUTE))
            {
            LOG.errorv("Missing xml element '%s' on ECEnumerator for Enumeration '%s'.", NAME_ATTRIBUTE, enumeration->GetName().c_str());
            return ERROR;
            }
        childNode->GetAttributeStringValue(enumeratorDescription, DESCRIPTION_ATTRIBUTE);
        }
    else
        {
        if (PrimitiveType::PRIMITIVETYPE_Integer == primitiveType)
            enumeratorName = ECEnumerator::DetermineName(enumeration->GetName(), nullptr, &enumeratorValueInteger);
        else if (PrimitiveType::PRIMITIVETYPE_String == primitiveType)
            enumeratorName = ECEnumerator::DetermineName(enumeration->GetName(), enumeratorValueString.c_str(), nullptr);
        }

    const auto ecobjectsStatusToMsg = [](ECObjectsStatus status) -> Utf8CP 
        {
        switch (status)
            {
            case ECObjectsStatus::NamedItemAlreadyExists:
                return "Named item already exists";
            case ECObjectsStatus::DataTypeMismatch:
                return "Datatype mismatch";
            case ECObjectsStatus::InvalidName:
                return "Invalid name";
            default:
                return "Error creating enumerator.";
            }
        };

    ECEnumeratorP enumerator = nullptr;
    if (PrimitiveType::PRIMITIVETYPE_Integer == primitiveType)
        {
        ECObjectsStatus status = enumeration->CreateEnumerator(enumerator, enumeratorName, enumeratorValueInteger);
        if (ECObjectsStatus::Success != status)
            {
            LOG.errorv("Failed to add value '%" PRId32 "' to ECEnumeration '%s'. %s.", enumeratorValueInteger, enumeration->GetName().c_str(), ecobjectsStatusToMsg(status));
            return ERROR;
            }
        }
    else if (PrimitiveType::PRIMITIVETYPE_String == primitiveType)
        {
        ECObjectsStatus status = enumeration->CreateEnumerator(enumerator, enumeratorName, enumeratorValueString.c_str());
        if (ECObjectsStatus::Success != status)
            {
            LOG.errorv("Failed to add value '%s' to ECEnumeration '%s'. %s.", enumeratorValueString.c_str(), enumeration->GetName().c_str(), ecobjectsStatusToMsg(status));
            return ERROR;
            }
        }
    enumerator->SetDescription(enumeratorDescription);
    if (enumeratorHasDisplayLabel)
        enumerator->SetDisplayLabel(enumeratorDisplayLabel);

    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------+---------------+---------------+---------------+---------------+-------
SchemaReadStatus ECEnumeration::ReadXml(BeXmlNodeR enumerationNode, ECSchemaReadContextR context)
    {
    Utf8String value; // used by the macros.
    if (GetName().length() == 0)
        READ_REQUIRED_XML_ATTRIBUTE(enumerationNode, TYPE_NAME_ATTRIBUTE, this, Name, enumerationNode.GetName())

    if (BEXML_Success == enumerationNode.GetAttributeStringValue(value, DESCRIPTION_ATTRIBUTE))
        SetDescription(value.c_str());

    if (BEXML_Success == enumerationNode.GetAttributeStringValue(value, ECXML_DISPLAY_LABEL_ATTRIBUTE))
        SetDisplayLabel(value.c_str());
    
    // BACKING_TYPE_NAME_ATTRIBUTE is a required attribute.  If it is missing, an error will be returned.
    if (BEXML_Success != enumerationNode.GetAttributeStringValue(value, ECXML_BACKING_TYPE_NAME_ATTRIBUTE))
        {
        BeAssert(s_noAssert);
        LOG.errorv("Invalid ECSchemaXML: %s element must contain a %s attribute", enumerationNode.GetName(), ECXML_BACKING_TYPE_NAME_ATTRIBUTE);
        return SchemaReadStatus::InvalidECSchemaXml;
        }

    if (ECObjectsStatus::Success != this->SetTypeName(value.c_str()))
        {
        LOG.errorv("Invalid type name on enumeration '%s': '%s'.", this->GetName().c_str(), value.c_str());
        return SchemaReadStatus::InvalidPrimitiveType;
        }

    bool isStrict = true;
    if (BEXML_Success == enumerationNode.GetAttributeBooleanValue(isStrict, IS_STRICT_ATTRIBUTE))
        SetIsStrict(isStrict);

    for (BeXmlNodeP childNode = enumerationNode.GetFirstChild(); childNode != nullptr; childNode = childNode->GetNextSibling())
        { 
        if (SUCCESS != createEnumeratorFromXmlNode(this, childNode))
            return SchemaReadStatus::InvalidECSchemaXml;
        }

    return SchemaReadStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Robert.Schili                  12/2015
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus ECEnumeration::CreateEnumerator(ECEnumeratorP& enumerator, Utf8StringCR name, Utf8CP value)
    {
    BeAssert(nullptr != value);
    if (GetType() != PrimitiveType::PRIMITIVETYPE_String)
        return ECObjectsStatus::DataTypeMismatch;

    if (!EnumeratorIsUnique(name.c_str(), value))
        {
        enumerator = nullptr;
        return ECObjectsStatus::NamedItemAlreadyExists;
        }

    enumerator = new ECEnumerator(*this, value);
    if (ECObjectsStatus::Success != enumerator->SetName(name))
        {
        delete enumerator;
        return ECObjectsStatus::InvalidName;
        }

    m_enumeratorList.push_back(enumerator);
    return ECObjectsStatus::Success;
    }

/*--------------------------------------------------------------------------------------/
* @bsimethod                                    Robert.Schili                  12/2015
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus ECEnumeration::CreateEnumerator(ECEnumeratorP& enumerator, Utf8StringCR name, int32_t value)
    {
    if (GetType() != PrimitiveType::PRIMITIVETYPE_Integer)
        return ECObjectsStatus::DataTypeMismatch;

    if (!EnumeratorIsUnique(name.c_str(), value))
        {
        enumerator = nullptr;
        return ECObjectsStatus::NamedItemAlreadyExists;
        }

    enumerator = new ECEnumerator(*this, value);
    if (ECObjectsStatus::Success != enumerator->SetName(name))
        {
        delete enumerator;
        return ECObjectsStatus::InvalidName;
        }

    m_enumeratorList.push_back(enumerator);
    return ECObjectsStatus::Success;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Kyle.Abramowitz                 08/2018
//---------------+---------------+---------------+---------------+---------------+-------
ECObjectsStatus ECEnumeration::CreateEnumerator(ECEnumeratorP& enumerator, Utf8CP value)
    {
    auto name = ECEnumerator::DetermineName(GetName(), value, nullptr);
    return CreateEnumerator(enumerator, name, value);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Kyle.Abramowitz                 08/2018
//---------------+---------------+---------------+---------------+---------------+-------
ECObjectsStatus ECEnumeration::CreateEnumerator(ECEnumeratorP& enumerator, int32_t value)
    {
    const auto& name = ECEnumerator::DetermineName(GetName(), nullptr, &value);
    return CreateEnumerator(enumerator, name, value);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Victor.Cushman                 01/2018
//---------------+---------------+---------------+---------------+---------------+-------
ECEnumeratorP ECEnumeration::FindEnumeratorByName(Utf8CP name) const
    {
    for (auto const& entry : m_enumeratorList)
        {
        if (entry->GetName().EqualsI(name))
            return entry;
        }

    return nullptr;
    }

/*--------------------------------------------------------------------------------------/
* @bsimethod                                    Robert.Schili                  12/2015
+---------------+---------------+---------------+---------------+---------------+------*/
ECEnumeratorP ECEnumeration::FindEnumerator(int32_t value) const
    {
    if (GetType() != PrimitiveType::PRIMITIVETYPE_Integer)
        return nullptr;

    for (auto const& entry : m_enumeratorList)
        {
        if (entry->GetInteger() == value)
            return entry;
        }

    return nullptr;
    }

/*--------------------------------------------------------------------------------------/
* @bsimethod                                    Robert.Schili                  12/2015
+---------------+---------------+---------------+---------------+---------------+------*/
ECEnumeratorP ECEnumeration::FindEnumerator(Utf8CP value) const
    {
    if (GetType() != PrimitiveType::PRIMITIVETYPE_String)
        return nullptr;

    for (auto const& entry : m_enumeratorList)
        {
        Utf8StringCR strValue = entry->GetString();
        if(strValue.EqualsI(value))
            return entry;
        }

    return nullptr;
    }

/*--------------------------------------------------------------------------------------/
* @bsimethod                                    Robert.Schili                  12/2015
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus ECEnumeration::DeleteEnumerator(ECEnumeratorCR enumerator)
    {
    auto existing = std::find(m_enumeratorList.begin(), m_enumeratorList.end(), &enumerator);
    if (existing == nullptr)
        return ECObjectsStatus::EnumeratorNotFound;

    m_enumeratorList.erase(existing);
    delete(&enumerator);

    return ECObjectsStatus::Success;
    }

/*--------------------------------------------------------------------------------------/
* @bsimethod                                    Robert.Schili                  12/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void ECEnumeration::Clear()
    {
    for (auto pEnumerator : m_enumeratorList)
        delete(pEnumerator);
    m_enumeratorList.clear();
    }

//===========================================================================//
//--------------------------------ECEnumerator-------------------------------//
//===========================================================================//

//---------------------------------------------------------------------------------------
// @bsimethod                                   Victor.Cushman                 12/2017
//---------------+---------------+---------------+---------------+---------------+-------
void ECEnumerator::SetDisplayLabel(Utf8StringCR value)
    {
    m_validatedName.SetDisplayLabel(value.c_str());
    }

/*--------------------------------------------------------------------------------------/
* @bsimethod                                    Robert.Schili                  12/2015
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8StringCR ECEnumerator::GetDisplayLabel() const
    {
    if (GetIsDisplayLabelDefined())
        return GetEnumeration().GetSchema().GetLocalizedStrings().GetEnumeratorDisplayLabel(*this, GetInvariantDisplayLabel());
    return GetInvariantDisplayLabel();
    }

/*--------------------------------------------------------------------------------------/
* @bsimethod                                    Robert.Schili                  12/2015
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8StringCR ECEnumerator::GetInvariantDisplayLabel() const
    {
    if (GetIsDisplayLabelDefined())
        return m_validatedName.GetDisplayLabel();
    return m_validatedName.GetName();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Victor.Cushman                 12/2017
//---------------+---------------+---------------+---------------+---------------+-------
ECObjectsStatus ECEnumerator::SetName(Utf8StringCR name)
    {
    if (!m_validatedName.SetValidName(name.c_str(), false))
        return ECObjectsStatus::InvalidName;

    return ECObjectsStatus::Success;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Victor.Cushman                 12/2017
//---------------+---------------+---------------+---------------+---------------+-------
Utf8StringCR ECEnumerator::GetDescription() const
    {
    return GetEnumeration().GetSchema().GetLocalizedStrings().GetEnumeratorDescription(*this, GetInvariantDescription());
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                   Caleb.Shafer                    01/2018
//--------------------------------------------------------------------------------------
bool ECEnumerator::IsInteger() const
    {
    return m_enum.GetType() == PrimitiveType::PRIMITIVETYPE_Integer;
    }

/*--------------------------------------------------------------------------------------/
* @bsimethod                                    Robert.Schili                  12/2015
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus ECEnumerator::SetInteger(int32_t integer)
    {
    if (!IsInteger())
        return ECObjectsStatus::DataTypeMismatch;

    if (nullptr != m_enum.FindEnumerator(integer))
        return ECObjectsStatus::NamedItemAlreadyExists;

    m_intValue = integer;
    return ECObjectsStatus::Success;
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                   Caleb.Shafer                    01/2018
//--------------------------------------------------------------------------------------
bool ECEnumerator::IsString() const
    {
    return m_enum.GetType() == PrimitiveType::PRIMITIVETYPE_String;
    }

/*--------------------------------------------------------------------------------------/
* @bsimethod                                    Robert.Schili                  12/2015
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus ECEnumerator::SetString(Utf8CP string)
    {
    if (!IsString())
        return ECObjectsStatus::DataTypeMismatch;

    if (nullptr != m_enum.FindEnumerator(string))
        return ECObjectsStatus::NamedItemAlreadyExists;

    m_stringValue = string;
    return ECObjectsStatus::Success;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Victor.Cushman                 01/2018
//---------------+---------------+---------------+---------------+---------------+-------
Utf8String ECEnumerator::DetermineName(Utf8StringCR enumerationName, Utf8CP enumeratorValueString, int32_t const* enumeratorValueInteger)
    {
    Utf8String name;
    if (nullptr != enumeratorValueString)
        name.assign(enumeratorValueString);
    else if (nullptr != enumeratorValueInteger)
        name.Sprintf("%s%" PRId32, enumerationName.c_str(), *enumeratorValueInteger);
    else
        BeAssert(false && "All enumerator value pointers were set to nullptr.");
    return ECNameValidation::EncodeToValidName(name);
    }

END_BENTLEY_ECOBJECT_NAMESPACE
