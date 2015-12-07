/*--------------------------------------------------------------------------------------+
|
|     $Source: src/ECEnumeration.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
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
    : m_schema(schema), m_primitiveType(PrimitiveType::PRIMITIVETYPE_Integer), m_enumeratorList(), m_enumeratorIterable(m_enumeratorList)
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
ECObjectsStatus ECEnumeration::SetName(Utf8StringCR name)
    {
    m_validatedName.SetName(name.c_str());
    m_fullName = GetSchema().GetName() + ":" + GetName();

    return ECObjectsStatus::Success;
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
Utf8CP ECEnumeration::GetFullName () const
    {
    if (m_fullName.size() == 0)
        m_fullName = GetSchema().GetName() + ":" + GetName();
        
    return m_fullName.c_str();
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
    Utf8String namespacePrefix;
    Utf8StringCR enumName = ecEnumeration.GetName();
    if (!EXPECTED_CONDITION (ECObjectsStatus::Success == primarySchema.ResolveNamespacePrefix (ecEnumeration.GetSchema(), namespacePrefix)))
        {
        LOG.warningv ("warning: Can not qualify an ECEnumeration name with a namespace prefix unless the schema containing the ECEnumeration is referenced by the primary schema."
            "The name will remain unqualified.\n  Primary ECSchema: %s\n  ECEnumeration: %s\n ECSchema containing ECEnumeration: %s", primarySchema.GetName().c_str(), enumName.c_str(), ecEnumeration.GetSchema().GetName().c_str());
        return enumName;
        }
    if (namespacePrefix.empty())
        return enumName;
    else
        return namespacePrefix + ":" + enumName;
    }

ECObjectsStatus ECEnumeration::ParseEnumerationName(Utf8StringR prefix, Utf8StringR enumName, Utf8StringCR qualifiedEnumName)
    {
    if (0 == qualifiedEnumName.length())
        {
        LOG.warningv("Failed to parse a prefix and name from a qualified name because the string is empty.");
        return ECObjectsStatus::ParseError;
        }

    Utf8String::size_type colonIndex = qualifiedEnumName.find(':');
    if (Utf8String::npos == colonIndex)
        {
        prefix.clear();
        enumName = qualifiedEnumName;
        return ECObjectsStatus::Success;
        }

    if (qualifiedEnumName.length() == colonIndex + 1)
        {
        LOG.warningv("Failed to parse a prefix and name from the qualified name '%s' because the string ends with a colon.  There must be characters after the colon.",
                     qualifiedEnumName.c_str());
        return ECObjectsStatus::ParseError;
        }

    if (0 == colonIndex)
        prefix.clear();
    else
        prefix = qualifiedEnumName.substr(0, colonIndex);

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
PrimitiveType ECEnumeration::GetType() const
    {
    return m_primitiveType;
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
ECObjectsStatus ECEnumeration::SetTypeName(Utf8StringCR typeName)
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
    return GetInvariantDisplayLabel(); //TODO: Support Localization
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
ECObjectsStatus ECEnumeration::SetDisplayLabel (Utf8StringCR displayLabel)
    {        
    m_validatedName.SetDisplayLabel (displayLabel.c_str());
    return ECObjectsStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
 @bsimethod                                                     
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8StringCR ECEnumeration::GetDescription () const
    {
    return GetInvariantDescription(); //TODO: Support Localization
    }

/*---------------------------------------------------------------------------------**//**
 @bsimethod                                                     
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8StringCR ECEnumeration::GetInvariantDescription () const
    {
    return m_description;
    }

/*---------------------------------------------------------------------------------**//**
 @bsimethod                                                     
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus ECEnumeration::SetDescription (Utf8StringCR description)
    {        
    m_description = description;
    return ECObjectsStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Robert.Schili                  11/2015
+---------------+---------------+---------------+---------------+---------------+------*/
SchemaWriteStatus ECEnumeration::_WriteXml (BeXmlWriterR xmlWriter, int ecXmlVersionMajor, int ecXmlVersionMinor) const
    {
    Utf8CP elementName = EC_ENUMERATION_ELEMENT;
    SchemaWriteStatus status = SchemaWriteStatus::Success;
    
    xmlWriter.WriteElementStart(elementName);
    
    xmlWriter.WriteAttribute(TYPE_NAME_ATTRIBUTE, this->GetName().c_str());
    xmlWriter.WriteAttribute(BACKING_TYPE_NAME_ATTRIBUTE, GetTypeName().c_str());
    xmlWriter.WriteAttribute(DESCRIPTION_ATTRIBUTE, this->GetInvariantDescription().c_str());
    if (GetIsDisplayLabelDefined())
        xmlWriter.WriteAttribute(DISPLAY_LABEL_ATTRIBUTE, this->GetInvariantDisplayLabel().c_str());
    
    //WriteCustomAttributes (xmlWriter);
    xmlWriter.WriteElementEnd();
    return status;
    }

/*---------------------------------------------------------------------------------**//**
 @bsimethod                                                     
+---------------+---------------+---------------+---------------+---------------+------*/
SchemaReadStatus ECEnumeration::_ReadXml(BeXmlNodeR enumerationNode, ECSchemaReadContextR context)
    {
    Utf8String value;      // used by the macros.
    if (GetName().length() == 0)
        {
        READ_REQUIRED_XML_ATTRIBUTE(enumerationNode, TYPE_NAME_ATTRIBUTE, this, Name, enumerationNode.GetName())
        }

    // OPTIONAL attributes - If these attributes exist they MUST be valid    
    READ_OPTIONAL_XML_ATTRIBUTE(enumerationNode, DESCRIPTION_ATTRIBUTE, this, Description)
    READ_OPTIONAL_XML_ATTRIBUTE(enumerationNode, DISPLAY_LABEL_ATTRIBUTE, this, DisplayLabel)

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

    return SchemaReadStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Robert.Schili                  12/2015
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus ECEnumeration::CreateEnumerator(ECEnumeratorP& enumerator, Utf8StringCR value)
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
ECEnumeratorP ECEnumeration::FindEnumerator(Utf8StringCR value) const
    {
    if (GetType() != PrimitiveType::PRIMITIVETYPE_String)
        {
        return nullptr;
        }

    for (auto const& entry : m_enumeratorList)
        {
        Utf8StringCP strValue = entry->GetString();
        if(strValue != nullptr && strcmp(strValue->c_str(), value.c_str()) == 0)
            return entry;
        }

    return nullptr;
    }

/*--------------------------------------------------------------------------------------/
* @bsimethod                                    Robert.Schili                  12/2015
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus ECEnumeration::DeleteEnumerator(ECEnumeratorR enumerator)
    {
    ECEnumeratorP pEnumerator = nullptr;
    if (GetType() == PrimitiveType::PRIMITIVETYPE_Integer)
        {
        pEnumerator = FindEnumerator(enumerator.GetInteger());
        }
    else //assume it's a string
        {
        Utf8StringCP stringValue = enumerator.GetString();
        if (stringValue != nullptr)
            {
            pEnumerator = FindEnumerator(*stringValue);
            }
        }

    if (pEnumerator == nullptr)
        return ECObjectsStatus::EnumerationNotFound;

    m_enumeratorList.erase(&pEnumerator);
    return ECObjectsStatus::Success;
    }

/*--------------------------------------------------------------------------------------/
* @bsimethod                                    Robert.Schili                  12/2015
+---------------+---------------+---------------+---------------+---------------+------*/
EnumeratorIterable ECEnumeration::GetEnumerators() const
    {
    return m_enumeratorIterable;
    }

/*--------------------------------------------------------------------------------------/
* @bsimethod                                    Robert.Schili                  12/2015
+---------------+---------------+---------------+---------------+---------------+------*/
size_t ECEnumeration::GetEnumeratorCount() const
    {
    return m_enumeratorList.size();
    }

//===========================================================================//
//----------------------------Enumerator------------------------------------//
//===========================================================================//

/*--------------------------------------------------------------------------------------/
* @bsimethod                                    Robert.Schili                  12/2015
+---------------+---------------+---------------+---------------+---------------+------*/
bool ECEnumerator::GetIsDisplayLabelDefined() const
    {
    return m_hasExplicitDisplayLabel;
    }

/*--------------------------------------------------------------------------------------/
* @bsimethod                                    Robert.Schili                  12/2015
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus ECEnumerator::SetDisplayLabel(Utf8StringCR value)
    {
    m_hasExplicitDisplayLabel = true;
    m_displayLabel = value;
    return ECObjectsStatus::Success;
    }

/*--------------------------------------------------------------------------------------/
* @bsimethod                                    Robert.Schili                  12/2015
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8StringCP ECEnumerator::GetDisplayLabel() const
    {
    return GetInvariantDisplayLabel(); //TODO: localization support.
    }

/*--------------------------------------------------------------------------------------/
* @bsimethod                                    Robert.Schili                  12/2015
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8StringCP ECEnumerator::GetInvariantDisplayLabel() const
    {
    if (m_hasExplicitDisplayLabel)
        return &m_displayLabel;

    return GetString();
    }

/*--------------------------------------------------------------------------------------/
* @bsimethod                                    Robert.Schili                  12/2015
+---------------+---------------+---------------+---------------+---------------+------*/
bool ECEnumerator::IsInteger() const
    {
    return m_enum.GetType() == PrimitiveType::PRIMITIVETYPE_Integer;
    }

/*--------------------------------------------------------------------------------------/
* @bsimethod                                    Robert.Schili                  12/2015
+---------------+---------------+---------------+---------------+---------------+------*/
bool ECEnumerator::IsString() const
    {
    return m_enum.GetType() == PrimitiveType::PRIMITIVETYPE_String;
    }

/*--------------------------------------------------------------------------------------/
* @bsimethod                                    Robert.Schili                  12/2015
+---------------+---------------+---------------+---------------+---------------+------*/
int32_t ECEnumerator::GetInteger() const
    {
    PRECONDITION(IsInteger() && "Tried to get int32_t value from an ECN::ECEnumerator that is not an int32_t.", 0);
    return m_intValue;
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
Utf8StringCP ECEnumerator::GetString() const
    {
    if (IsString())
        {
        return &m_stringValue;
        }

    return nullptr;
    }

/*--------------------------------------------------------------------------------------/
* @bsimethod                                    Robert.Schili                  12/2015
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus ECEnumerator::SetString(Utf8StringCR string)
    {
    if (!IsInteger())
        {
        return ECObjectsStatus::DataTypeMismatch;
        }

    m_stringValue = string;
    return ECObjectsStatus::Success;
    }

END_BENTLEY_ECOBJECT_NAMESPACE



