/*--------------------------------------------------------------------------------------+
|
|     $Source: src/ECEnumeration.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#include "ECObjectsPch.h"

BEGIN_BENTLEY_ECOBJECT_NAMESPACE
// If you are developing schemas, particularly when editing them by hand, you want to have this variable set to false so you get the asserts to help you figure out what is going wrong.
// Test programs generally want to get error status back and not BeAssert, so they call ECSchema::AssertOnXmlError (false);
static  bool        s_noAssert = false;

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------+---------------+---------------+---------------+---------------+-------
ECEnumeration::ECEnumeration(ECSchemaCR schema)
    : m_schema(schema), m_primitiveType(PrimitiveType::PRIMITIVETYPE_Integer), m_isStrict(true), m_enumeratorList(), m_enumeratorIterable(m_enumeratorList)
    {
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------+---------------+---------------+---------------+---------------+-------
ECEnumeration::~ECEnumeration()
    {
    for (ECEnumerator* entry : m_enumeratorList)
        delete entry;

    m_enumeratorList.clear();
    }

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
    if (!ECNameValidation::IsValidName(name))
        return ECObjectsStatus::InvalidName;

    m_validatedName.SetName(name);
    m_fullName = GetSchema().GetName() + ":" + GetName();
    return ECObjectsStatus::Success;
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------+---------------+---------------+---------------+---------------+-------
Utf8StringCR ECEnumeration::GetFullName () const
    {
    if (m_fullName.size() == 0)
        m_fullName = GetSchema().GetName() + ":" + GetName();
        
    return m_fullName;
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------+---------------+---------------+---------------+---------------+-------
Utf8String ECEnumeration::GetQualifiedEnumerationName(ECSchemaCR primarySchema, ECEnumerationCR ecEnumeration)
    {
    Utf8String alias;
    Utf8StringCR enumName = ecEnumeration.GetName();
    if (!EXPECTED_CONDITION (ECObjectsStatus::Success == primarySchema.ResolveAlias (ecEnumeration.GetSchema(), alias)))
        {
        LOG.warningv ("warning: Cannot qualify an ECEnumeration name with an alias unless the schema containing the ECEnumeration is referenced by the primary schema."
            "The name will remain unqualified.\n  Primary ECSchema: %s\n  ECEnumeration: %s\n ECSchema containing ECEnumeration: %s", primarySchema.GetName().c_str(), enumName.c_str(), ecEnumeration.GetSchema().GetName().c_str());
        return enumName;
        }

    if (alias.empty())
        return enumName;
    else
        return alias + ":" + enumName;
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------+---------------+---------------+---------------+---------------+-------
ECObjectsStatus ECEnumeration::ParseEnumerationName(Utf8StringR alias, Utf8StringR enumName, Utf8StringCR qualifiedEnumName)
    {
    if (0 == qualifiedEnumName.length())
        return ECObjectsStatus::ParseError;

    Utf8String::size_type colonIndex = qualifiedEnumName.find(':');
    if (Utf8String::npos == colonIndex)
        {
        alias.clear();
        enumName = qualifiedEnumName;
        return ECObjectsStatus::Success;
        }

    if (qualifiedEnumName.length() == colonIndex + 1)
        return ECObjectsStatus::ParseError;

    if (0 == colonIndex)
        alias.clear();
    else
        alias = qualifiedEnumName.substr(0, colonIndex);

    enumName = qualifiedEnumName.substr(colonIndex + 1);

    return ECObjectsStatus::Success;
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------+---------------+---------------+---------------+---------------+-------
ECObjectsStatus ECEnumeration::SetType(PrimitiveType value)
    {
    if (value != PRIMITIVETYPE_Integer && value != PRIMITIVETYPE_String)
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
        return status;

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
    xmlWriter.WriteAttribute(BACKING_TYPE_NAME_ATTRIBUTE, GetTypeName().c_str());
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
SchemaWriteStatus ECEnumeration::WriteJson(Json::Value& outValue, bool standalone, bool includeSchemaVersion) const
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

    outValue[ECJSON_SCHEMA_CHILD_TYPE] = ECJSON_ENUMERATION_ELEMENT;

    if (this->GetIsDisplayLabelDefined())
        outValue[ECJSON_DISPLAY_LABEL_ATTRIBUTE] = GetInvariantDisplayLabel();
    if (0 != this->GetInvariantDescription().length())
        outValue[DESCRIPTION_ATTRIBUTE] = GetInvariantDescription();

    // ECEnumeration Properties
    outValue[BACKING_TYPE_NAME_ATTRIBUTE] = GetTypeName();
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
            return SchemaWriteStatus::FailedToCreateJson;
        enumeratorArr.append(enumeratorObj);
        }
    outValue[ECJSON_ENUMERATOR_ELEMENT] = enumeratorArr;

    return SchemaWriteStatus::Success;
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------+---------------+---------------+---------------+---------------+-------
static BentleyStatus createEnumeratorFromXmlNode(ECEnumerationP enumeration, BeXmlNodeP childNode)
    {
    PrimitiveType const primitiveType = enumeration->GetType();
    Utf8CP childNodeName = childNode->GetName();
    if (0 != strcmp(childNodeName, ECXML_ENUMERATOR_ELEMENT))
        return ERROR;

    // Enumerator Value
    int32_t enumeratorValueInteger {};
    Utf8String enumeratorValueString;
    BeXmlStatus xmlStatus {};
    if (PrimitiveType::PRIMITIVETYPE_Integer == primitiveType)
        xmlStatus = childNode->GetAttributeInt32Value(enumeratorValueInteger, ENUMERATOR_VALUE_ATTRIBUTE);
    else if (PrimitiveType::PRIMITIVETYPE_String == primitiveType)
        xmlStatus = childNode->GetAttributeStringValue(enumeratorValueString, ENUMERATOR_VALUE_ATTRIBUTE);

    if (BeXmlStatus::BEXML_Success != xmlStatus)
        {
        LOG.warningv("Missing xml element '%s' on ECEnumerator for Enumeration '%s'.", ENUMERATOR_VALUE_ATTRIBUTE, enumeration->GetName().c_str());
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
            LOG.warningv("Missing xml element '%s' on ECEnumerator for Enumeration '%s'.", NAME_ATTRIBUTE, enumeration->GetName().c_str());
            return ERROR;
            }
        childNode->GetAttributeStringValue(enumeratorDescription, DESCRIPTION_ATTRIBUTE);
        }
    else
        {
        // For schema versions less than ECVersion::V3_2 the enumerator name attribute required as of ECVersion::V3_2 is constructed from:
        // (1) The enumerator display label if the enumerator display label exists for the enumeration.
        // (2) <enumeration name> + <enumerator value> if the enumerator display label does not exist for the enumeration.
        if (enumeratorHasDisplayLabel)
            enumeratorName = ECNameValidation::EncodeToValidName(enumeratorDisplayLabel);
        else
            {
            if (PrimitiveType::PRIMITIVETYPE_Integer == primitiveType)
                enumeratorName.Sprintf("%s%" PRId32, enumeration->GetName().c_str(), enumeratorValueInteger);
            else if (PrimitiveType::PRIMITIVETYPE_String == primitiveType)
                enumeratorName = enumeration->GetName() + enumeratorValueString;
            enumeratorName = ECNameValidation::EncodeToValidName(enumeratorName);
            }
        }

    ECEnumeratorP enumerator = nullptr;
    if (PrimitiveType::PRIMITIVETYPE_Integer == primitiveType)
        {
        if (ECObjectsStatus::Success != enumeration->CreateEnumerator(enumerator, enumeratorName, enumeratorValueInteger))
            {
            LOG.warningv("Failed to add value '%" PRId32 "' to ECEnumeration '%s'. Duplicate or invalid entry?", enumeratorValueInteger, enumeration->GetName().c_str());
            return ERROR;
            }
        }
    else if (PrimitiveType::PRIMITIVETYPE_String == primitiveType)
        {
        if (ECObjectsStatus::Success != enumeration->CreateEnumerator(enumerator, enumeratorName, enumeratorValueString.c_str()))
            {
            LOG.warningv("Failed to add value '%s' to ECEnumeration '%s'. Duplicate or invalid entry?", enumeratorValueString.c_str(), enumeration->GetName().c_str());
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
    if (BEXML_Success != enumerationNode.GetAttributeStringValue(value, BACKING_TYPE_NAME_ATTRIBUTE))
        {
        BeAssert(s_noAssert);
        LOG.errorv("Invalid ECSchemaXML: %s element must contain a %s attribute", enumerationNode.GetName(), BACKING_TYPE_NAME_ATTRIBUTE);
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
        createEnumeratorFromXmlNode(this, childNode);

    return SchemaReadStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Robert.Schili                  12/2015
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus ECEnumeration::CreateEnumerator(ECEnumeratorP& enumerator, Utf8StringCR name, Utf8CP value)
    {
    if (GetType() != PrimitiveType::PRIMITIVETYPE_String)
        return ECObjectsStatus::DataTypeMismatch;

    enumerator = FindEnumerator(value);
    if (enumerator != nullptr)
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

    enumerator = FindEnumerator(value);
    if (enumerator != nullptr)
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
        if(strcmp(strValue.c_str(), value) == 0)
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
    if (!ECNameValidation::IsValidName(name.c_str()))
        return ECObjectsStatus::InvalidName;

    m_validatedName.SetName(name.c_str());
    return ECObjectsStatus::Success;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Victor.Cushman                 12/2017
//---------------+---------------+---------------+---------------+---------------+-------
Utf8StringCR ECEnumerator::GetDescription() const
    {
    return GetEnumeration().GetSchema().GetLocalizedStrings().GetEnumeratorDescription(*this, GetInvariantDescription());
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

END_BENTLEY_ECOBJECT_NAMESPACE
