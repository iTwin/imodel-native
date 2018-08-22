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

    Utf8CP elementName = ECXML_ENUMERATION_ELEMENT;
    SchemaWriteStatus status = SchemaWriteStatus::Success;
    
    xmlWriter.WriteElementStart(elementName);
    
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
        Utf8StringCR displayLabel = enumerator->GetInvariantDisplayLabel();
        if(isIntType)
            xmlWriter.WriteAttribute(ENUMERATOR_VALUE_ATTRIBUTE, enumerator->GetInteger());
        else
            xmlWriter.WriteAttribute(ENUMERATOR_VALUE_ATTRIBUTE, enumerator->GetString().c_str());

        if(enumerator->m_hasExplicitDisplayLabel)
            xmlWriter.WriteAttribute(ECXML_DISPLAY_LABEL_ATTRIBUTE, displayLabel.c_str());

        xmlWriter.WriteElementEnd();
        }

    xmlWriter.WriteElementEnd();
    return status;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Victor.Cushman              11/2017
//---------------+---------------+---------------+---------------+---------------+-------
SchemaWriteStatus ECEnumeration::WriteJson(Json::Value& outValue, bool standalone, bool includeSchemaVersion) const
    {
    // Common properties to all Schema items
    if (standalone)
        {
        outValue[ECJSON_URI_SPEC_ATTRIBUTE] = ECJSON_SCHEMA_ITEM_URI;
        outValue[ECJSON_PARENT_SCHEMA_ATTRIBUTE] = GetSchema().GetName();
        if (includeSchemaVersion)
            outValue[ECJSON_PARENT_VERSION_ATTRIBUTE] = GetSchema().GetSchemaKey().GetVersionString();
        outValue[ECJSON_SCHEMA_ITEM_NAME_ATTRIBUTE] = GetName();
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
SchemaReadStatus ECEnumeration::ReadXml(BeXmlNodeR enumerationNode, ECSchemaReadContextR context)
    {
    Utf8String value;      // used by the macros.
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

    PrimitiveType primitiveType = GetType();

    for (BeXmlNodeP childNode = enumerationNode.GetFirstChild(); childNode != nullptr; childNode = childNode->GetNextSibling())
        {
        Utf8CP childNodeName = childNode->GetName();
        if (0 != strcmp(childNodeName, ECXML_ENUMERATOR_ELEMENT))
            continue;

        ECEnumeratorP enumerator;
        if (primitiveType == PrimitiveType::PRIMITIVETYPE_Integer)
            {
            int32_t intValue;
            BeXmlStatus status = childNode->GetAttributeInt32Value(intValue, ENUMERATOR_VALUE_ATTRIBUTE);
            if (status != BeXmlStatus::BEXML_Success)
                {
                LOG.warningv("Failed to read int attribute '%s' on ECEnumerator for Enumeration '%s'.", ENUMERATOR_VALUE_ATTRIBUTE, this->GetName().c_str());
                continue;
                }
            
            if (this->CreateEnumerator(enumerator, intValue) != ECObjectsStatus::Success)
                {
                LOG.warningv("Failed to add value '%" PRId32 "' to ECEnumeration '%s'. Duplicate or invalid entry?", intValue, this->GetName().c_str());
                continue;
                }
            }
        else
            {
            Utf8String stringValue;
            BeXmlStatus status = childNode->GetAttributeStringValue(stringValue, ENUMERATOR_VALUE_ATTRIBUTE);
            if (status != BeXmlStatus::BEXML_Success)
                {
                LOG.warningv("Missing xml element '%s' on ECEnumerator for Enumeration '%s'.", ENUMERATOR_VALUE_ATTRIBUTE, this->GetName().c_str());
                continue;
                }

            if (this->CreateEnumerator(enumerator, stringValue.c_str()) != ECObjectsStatus::Success)
                {
                LOG.warningv("Failed to add value '%s' to ECEnumeration '%s'. Duplicate or invalid entry?", stringValue.c_str(), this->GetName().c_str());
                continue;
                }
            }

        Utf8String displayLabel;
        if (childNode->GetAttributeStringValue(displayLabel, ECXML_DISPLAY_LABEL_ATTRIBUTE) == BeXmlStatus::BEXML_Success)
            enumerator->SetDisplayLabel(displayLabel.c_str());
        }

    return SchemaReadStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Robert.Schili                  12/2015
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus ECEnumeration::CreateEnumerator(ECEnumeratorP& enumerator, Utf8CP value)
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
    m_enumeratorList.push_back(enumerator);
    return ECObjectsStatus::Success;
    }

/*--------------------------------------------------------------------------------------/
* @bsimethod                                    Robert.Schili                  12/2015
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus ECEnumeration::CreateEnumerator(ECEnumeratorP& enumerator, int32_t value)
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

/*--------------------------------------------------------------------------------------/
* @bsimethod                                    Robert.Schili                  12/2015
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8StringCR ECEnumerator::GetDisplayLabel() const
    {
    return GetEnumeration().GetSchema().GetLocalizedStrings().GetEnumeratorDisplayLabel(*this, GetInvariantDisplayLabel());
    }

/*--------------------------------------------------------------------------------------/
* @bsimethod                                    Robert.Schili                  12/2015
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8StringCR ECEnumerator::GetInvariantDisplayLabel() const
    {
    if (m_hasExplicitDisplayLabel)
        return m_displayLabel;

    if (IsInteger())
        m_displayLabel.Sprintf("%" PRId32, m_intValue);
    else
        m_displayLabel = m_stringValue;

    return m_displayLabel;
    }

/*--------------------------------------------------------------------------------------/
* @bsimethod                                    Robert.Schili                  12/2015
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus ECEnumerator::SetInteger(int32_t integer)
    {
    if (!IsInteger())
        return ECObjectsStatus::DataTypeMismatch;

    if (m_enum.FindEnumerator(integer) != nullptr)
        return ECObjectsStatus::NamedItemAlreadyExists;

    m_intValue = integer;
    return ECObjectsStatus::Success;
    }

/*--------------------------------------------------------------------------------------/
* @bsimethod                                    Robert.Schili                  12/2015
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus ECEnumerator::SetString(Utf8CP string)
    {
    if (!IsInteger())
        return ECObjectsStatus::DataTypeMismatch;

    m_stringValue = string;
    return ECObjectsStatus::Success;
    }

END_BENTLEY_ECOBJECT_NAMESPACE
