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

/*---------------------------------------------------------------------------------**//**
 @bsimethod                                                     
+---------------+---------------+---------------+---------------+---------------+------*/
ECEnumeration::ECEnumeration(ECSchemaCR schema)
    : m_schema(schema), m_primitiveType(PrimitiveType::PRIMITIVETYPE_Integer)
    {
    }

/*---------------------------------------------------------------------------------**//**
 @bsimethod                                                 
+---------------+---------------+---------------+---------------+---------------+------*/
ECEnumeration::~ECEnumeration()
    {
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

PrimitiveType ECEnumeration::GetType() const
    {
    return m_primitiveType;
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
    Utf8String backingTypeName = ECXml::GetPrimitiveTypeName(this->GetType());
    xmlWriter.WriteAttribute(BACKING_TYPE_NAME_ATTRIBUTE, backingTypeName.c_str());
    xmlWriter.WriteAttribute(DESCRIPTION_ATTRIBUTE, this->GetInvariantDescription().c_str());
    if (GetIsDisplayLabelDefined())
        xmlWriter.WriteAttribute(DISPLAY_LABEL_ATTRIBUTE, this->GetInvariantDisplayLabel().c_str());
    
    //WriteCustomAttributes (xmlWriter);
    xmlWriter.WriteElementEnd();
    return status;
    }
END_BENTLEY_ECOBJECT_NAMESPACE



