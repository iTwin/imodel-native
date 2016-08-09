/*--------------------------------------------------------------------------------------+
|
|     $Source: src/ECEnumeration.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#include "ECObjectsPch.h"
#include "SchemaXml.h"
#if defined (_WIN32) // WIP_NONPORT - iostreams not support on Android
#include <iomanip>
#endif
#include <Bentley/BeFileName.h>
#include <Bentley/BeFile.h>
#include <Bentley/BeFileListIterator.h>

#include <ECObjects/StronglyConnectedGraph.h>

BEGIN_BENTLEY_ECOBJECT_NAMESPACE
// If you are developing schemas, particularly when editing them by hand, you want to have this variable set to false so you get the asserts to help you figure out what is going wrong.
// Test programs generally want to get error status back and not BeAssert, so they call ECSchema::AssertOnXmlError (false);
static  bool        s_noAssert = false;

/*---------------------------------------------------------------------------------**//**
 @bsimethod                                                     
+---------------+---------------+---------------+---------------+---------------+------*/
ECEnumeration::ECEnumeration(ECSchemaCR schema)
    : m_schema(schema), m_primitiveType(PrimitiveType::PRIMITIVETYPE_Integer), m_isStrict(true), m_enumeratorList(), m_enumeratorIterable(m_enumeratorList)
    {
    }

/*---------------------------------------------------------------------------------**//**
 @bsimethod                                                 
+---------------+---------------+---------------+---------------+---------------+------*/
ECEnumeration::~ECEnumeration()
    {
    for (auto entry : m_enumeratorList)
        {
        delete entry;
        }

    m_enumeratorList.clear();
    }

/*---------------------------------------------------------------------------------**//**
 @bsimethod                                                     
+---------------+---------------+---------------+---------------+---------------+------*/
ECSchemaCR ECEnumeration::GetSchema() const
    {
    return m_schema;
    }

/*---------------------------------------------------------------------------------**//**
 @bsimethod                                                     
+---------------+---------------+---------------+---------------+---------------+------*/
void ECEnumeration::SetName(Utf8CP name)
    {
    m_validatedName.SetName(name);
    m_fullName = GetSchema().GetName() + ":" + GetName();
    }

/*---------------------------------------------------------------------------------**//**
 @bsimethod                                                     
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8StringCR ECEnumeration::GetName () const
    {        
    return m_validatedName.GetName();
    }

/*---------------------------------------------------------------------------------**//**
 @bsimethod                                                     
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8StringCR ECEnumeration::GetFullName () const
    {
    if (m_fullName.size() == 0)
        m_fullName = GetSchema().GetName() + ":" + GetName();
        
    return m_fullName;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String ECEnumeration::GetQualifiedEnumerationName
(
ECSchemaCR primarySchema,
ECEnumerationCR  ecEnumeration
)
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

ECObjectsStatus ECEnumeration::ParseEnumerationName(Utf8StringR alias, Utf8StringR enumName, Utf8StringCR qualifiedEnumName)
    {
    if (0 == qualifiedEnumName.length())
        {
        return ECObjectsStatus::ParseError;
        }

    Utf8String::size_type colonIndex = qualifiedEnumName.find(':');
    if (Utf8String::npos == colonIndex)
        {
        alias.clear();
        enumName = qualifiedEnumName;
        return ECObjectsStatus::Success;
        }

    if (qualifiedEnumName.length() == colonIndex + 1)
        {
        return ECObjectsStatus::ParseError;
        }

    if (0 == colonIndex)
        alias.clear();
    else
        alias = qualifiedEnumName.substr(0, colonIndex);

    enumName = qualifiedEnumName.substr(colonIndex + 1);

    return ECObjectsStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
 @bsimethod                                                     
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus ECEnumeration::SetType(PrimitiveType value)
    {
    if (value != PRIMITIVETYPE_Integer && value != PRIMITIVETYPE_String)
        {
        return ECObjectsStatus::DataTypeNotSupported;
        }

    m_primitiveType = value;
    return ECObjectsStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
 @bsimethod                                                     
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String ECEnumeration::GetTypeName() const
    {
    return ECXml::GetPrimitiveTypeName(m_primitiveType);
    }

/*---------------------------------------------------------------------------------**//**
 @bsimethod                                                     
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus ECEnumeration::SetTypeName(Utf8CP typeName)
    {
    PrimitiveType primitiveType;
    ECObjectsStatus status = ECXml::ParsePrimitiveType(primitiveType, typeName);
    if (ECObjectsStatus::Success != status)
        {
        return status;
        }

    return SetType(primitiveType);
    }

/*---------------------------------------------------------------------------------**//**
 @bsimethod                                                     
+---------------+---------------+---------------+---------------+---------------+------*/
bool ECEnumeration::GetIsDisplayLabelDefined () const
    {
    return m_validatedName.IsDisplayLabelDefined();
    }

/*---------------------------------------------------------------------------------**//**
 @bsimethod                                                     
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8StringCR ECEnumeration::GetDisplayLabel () const
    {
    return GetSchema().GetLocalizedStrings().GetEnumerationDisplayLabel(*this, GetInvariantDisplayLabel());
    }

/*---------------------------------------------------------------------------------**//**
 @bsimethod                                                     
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8StringCR ECEnumeration::GetInvariantDisplayLabel() const
    {
    return m_validatedName.GetDisplayLabel();
    }

/*---------------------------------------------------------------------------------**//**
 @bsimethod                                                     
+---------------+---------------+---------------+---------------+---------------+------*/
void ECEnumeration::SetDisplayLabel (Utf8CP displayLabel)
    {        
    m_validatedName.SetDisplayLabel (displayLabel);
    }

/*---------------------------------------------------------------------------------**//**
 @bsimethod                                                     
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8StringCR ECEnumeration::GetDescription () const
    {
    return GetSchema().GetLocalizedStrings().GetEnumerationDescription(*this, GetInvariantDescription());
    }

/*---------------------------------------------------------------------------------**//**
 @bsimethod                                                     
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8StringCR ECEnumeration::GetInvariantDescription () const
    {
    return m_description;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Robert.Schili                  11/2015
+---------------+---------------+---------------+---------------+---------------+------*/
SchemaWriteStatus ECEnumeration::WriteXml (BeXmlWriterR xmlWriter, int ecXmlVersionMajor, int ecXmlVersionMinor) const
    {
    if (ecXmlVersionMajor < 3)
        { //Enumerations will only be serialized in 3.0 and later
        return SchemaWriteStatus::Success;
        }

    Utf8CP elementName = EC_ENUMERATION_ELEMENT;
    SchemaWriteStatus status = SchemaWriteStatus::Success;
    
    xmlWriter.WriteElementStart(elementName);
    
    xmlWriter.WriteAttribute(TYPE_NAME_ATTRIBUTE, this->GetName().c_str());
    xmlWriter.WriteAttribute(BACKING_TYPE_NAME_ATTRIBUTE, GetTypeName().c_str());
    xmlWriter.WriteAttribute(DESCRIPTION_ATTRIBUTE, this->GetInvariantDescription().c_str());
    if (GetIsDisplayLabelDefined())
        xmlWriter.WriteAttribute(DISPLAY_LABEL_ATTRIBUTE, this->GetInvariantDisplayLabel().c_str());

    xmlWriter.WriteAttribute(IS_STRICT_ATTRIBUTE, this->GetIsStrict());

    bool isIntType = GetType() == PrimitiveType::PRIMITIVETYPE_Integer;
    for (auto enumerator : m_enumeratorList)
        {
        xmlWriter.WriteElementStart(EC_ENUMERATOR_ELEMENT);
        Utf8StringCR displayLabel = enumerator->GetInvariantDisplayLabel();
        if(isIntType)
            xmlWriter.WriteAttribute(ENUMERATOR_VALUE_ATTRIBUTE, enumerator->GetInteger());
        else
            {
            xmlWriter.WriteAttribute(ENUMERATOR_VALUE_ATTRIBUTE, enumerator->GetString().c_str());
            }

        if(enumerator->m_hasExplicitDisplayLabel)
            xmlWriter.WriteAttribute(DISPLAY_LABEL_ATTRIBUTE, displayLabel.c_str());

        xmlWriter.WriteElementEnd();
        }
    
    //WriteCustomAttributes (xmlWriter);
    xmlWriter.WriteElementEnd();
    return status;
    }

/*---------------------------------------------------------------------------------**//**
 @bsimethod                                                     
+---------------+---------------+---------------+---------------+---------------+------*/
SchemaReadStatus ECEnumeration::ReadXml(BeXmlNodeR enumerationNode, ECSchemaReadContextR context)
    {
    Utf8String value;      // used by the macros.
    if (GetName().length() == 0)
        {
        if (BEXML_Success != enumerationNode.GetAttributeStringValue(value, TYPE_NAME_ATTRIBUTE))
            {
            LOG.errorv("Invalid ECSchemaXML: %s element must contain a %s attribute", TYPE_NAME_ATTRIBUTE, enumerationNode.GetName());
            return SchemaReadStatus::InvalidECSchemaXml;
            }

        SetName(value.c_str());
        }

    if (BEXML_Success == enumerationNode.GetAttributeStringValue(value, DESCRIPTION_ATTRIBUTE))
        {
        SetDescription(value.c_str());
        }

    if (BEXML_Success == enumerationNode.GetAttributeStringValue(value, DISPLAY_LABEL_ATTRIBUTE))
        {
        SetDisplayLabel(value.c_str());
        }
    
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
        {
        SetIsStrict(isStrict);
        }

    PrimitiveType primitiveType = GetType();

    for (BeXmlNodeP childNode = enumerationNode.GetFirstChild(); childNode != nullptr; childNode = childNode->GetNextSibling())
        {
        Utf8CP childNodeName = childNode->GetName();
        if (0 != strcmp(childNodeName, EC_ENUMERATOR_ELEMENT))
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
                LOG.warningv("Failed to add value '%d' to ECEnumeration '%s'. Duplicate or invalid entry?", intValue, this->GetName().c_str());
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
        if (childNode->GetAttributeStringValue(displayLabel, DISPLAY_LABEL_ATTRIBUTE) == BeXmlStatus::BEXML_Success)
            {
            enumerator->SetDisplayLabel(displayLabel.c_str());
            }
        }

    return SchemaReadStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Robert.Schili                  12/2015
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus ECEnumeration::CreateEnumerator(ECEnumeratorP& enumerator, Utf8CP value)
    {
    if (GetType() != PrimitiveType::PRIMITIVETYPE_String)
        {
        return ECObjectsStatus::DataTypeMismatch;
        }

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
        {
        return ECObjectsStatus::DataTypeMismatch;
        }

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
        {
        return nullptr;
        }

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
        {
        return nullptr;
        }

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
        {
        delete(pEnumerator);
        }

    m_enumeratorList.clear();
    }

//===========================================================================//
//----------------------------Enumerator------------------------------------//
//===========================================================================//


/*--------------------------------------------------------------------------------------/
* @bsimethod                                    Robert.Schili                  12/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void ECEnumerator::SetDisplayLabel(Utf8CP value)
    {
    m_hasExplicitDisplayLabel = true;
    m_displayLabel = value;
    }

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
        m_displayLabel.Sprintf("%d", m_intValue);
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
        {
        return ECObjectsStatus::DataTypeMismatch;
        }

    if (m_enum.FindEnumerator(integer) != nullptr)
        {
        return ECObjectsStatus::NamedItemAlreadyExists;
        }

    m_intValue = integer;
    return ECObjectsStatus::Success;
    }

/*--------------------------------------------------------------------------------------/
* @bsimethod                                    Robert.Schili                  12/2015
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus ECEnumerator::SetString(Utf8CP string)
    {
    if (!IsInteger())
        {
        return ECObjectsStatus::DataTypeMismatch;
        }

    m_stringValue = string;
    return ECObjectsStatus::Success;
    }

END_BENTLEY_ECOBJECT_NAMESPACE



