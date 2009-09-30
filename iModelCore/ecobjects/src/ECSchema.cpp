/*--------------------------------------------------------------------------------------+
|
|     $Source: ecobjects/native/ECSchema.cpp $
|    $RCSfile: file.tpl,v $
|   $Revision: 1.10 $
|       $Date: 2005/11/07 15:38:45 $
|     $Author: EarlinLutz $
|
|  $Copyright: (c) 2009 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#include "ECObjectsPch.h"

BEGIN_BENTLEY_EC_NAMESPACE

/*---------------------------------------------------------------------------------**//**
 @bsimethod                                                     
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt Class::GetECProperty 
(
PropertyP & ecProperty, 
const wchar_t * propertyName
) const
    {
    return SUCCESS;
    }


/*---------------------------------------------------------------------------------**//**
 @bsimethod                                                     
+---------------+---------------+---------------+---------------+---------------+------*/
void Schema::init 
(
std::wstring & name, 
std::wstring & prefix
)
    {
    Schema::SchemaNameIsValid (name.c_str());    
    //CECClass::ECNameIsValid (prefix.c_str(), TRUE);        
    m_schemaName = name;
    m_schemaPrefix = prefix;
    m_majorVersion = 0;
    m_minorVersion = 0;
    /*m_referencedSchemas.clear();
    m_ecRelationshipUpgradeHandler = NULL;
    m_ecClassResolutionHelper = NULL;*/    
    }

/*---------------------------------------------------------------------------------**//**
 @bsimethod                                                     
+---------------+---------------+---------------+---------------+---------------+------*/
Schema::Schema 
(
const wchar_t *   prefix, 
const wchar_t *   name, 
UInt32      majorVersion,
UInt32      minorVersion
) 
    {
    init (std::wstring(name), std::wstring(prefix));
    m_majorVersion = majorVersion;
    m_minorVersion = minorVersion;
    }

/*---------------------------------------------------------------------------------**//**
 @bsimethod                                                 
+---------------+---------------+---------------+---------------+---------------+------*/
Schema::~Schema
(
)
    {
    /*for (CECClassVector::iterator it = m_CECClassVector.begin(); it != m_CECClassVector.end(); it++)
        {
        CECClass * pClass = *it;
        if (NULL != pClass)
            delete pClass;
        }

    for (CECRelationshipClassVector::iterator rcit = m_CECRelationshipClassVector.begin(); rcit != m_CECRelationshipClassVector.end(); rcit++)
        {
        CECRelationshipClass * pRelationshipClass = *rcit;
        *rcit = NULL;
        if (NULL != pRelationshipClass)
            delete pRelationshipClass;
        }
    m_CECRelationshipClassVector.clear();

    for (ECSchemaReferenceVector::iterator sit = m_referencedSchemas.begin(); sit != m_referencedSchemas.end(); sit++)
        {
        CECSchemaReference & schemaRef = *sit;
        if (NULL != schemaRef.m_pECSchema)
            delete schemaRef.m_pECSchema; //needswork: are we sure that something else isn't holding it... we need a SchemaManager
        }
    m_referencedSchemas.clear();*/
    }

/*---------------------------------------------------------------------------------**//**
 @bsimethod                                                     
+---------------+---------------+---------------+---------------+---------------+------*/
const wchar_t * Schema::GetSchemaName 
(
) const
    {
    return m_schemaName.c_str();
    }

/*---------------------------------------------------------------------------------**//**
 @bsimethod                                                     
+---------------+---------------+---------------+---------------+---------------+------*/
void Schema::SetSchemaName 
(
const wchar_t * schemaName
)
    {
    Schema::SchemaNameIsValid (schemaName);
    m_schemaName = schemaName;
    }

/*---------------------------------------------------------------------------------**//**
 @bsimethod                                                     
+---------------+---------------+---------------+---------------+---------------+------*/

const wchar_t * Schema::GetSchemaPrefix
(
) const
    {        
    return m_schemaPrefix.c_str();    
    }

/*---------------------------------------------------------------------------------**//**
 @bsimethod                                                     
+---------------+---------------+---------------+---------------+---------------+------*/

void Schema::SetSchemaPrefix 
(
const wchar_t * schemaPrefix
)
    {
    Schema::SchemaNameIsValid (schemaPrefix);
    m_schemaPrefix = schemaPrefix;
    }

/*---------------------------------------------------------------------------------**//**
 @bsimethod                                                     
+---------------+---------------+---------------+---------------+---------------+------*/
const wchar_t * Schema::GetDisplayLabel
(
) const
    {
    return m_displayLabel.c_str();
    }

/*---------------------------------------------------------------------------------**//**
 @bsimethod                                                     
+---------------+---------------+---------------+---------------+---------------+------*/
void Schema::SetDisplayLabel 
(
const wchar_t * displayLabel
)
    {    
    m_displayLabel = displayLabel;
    }

/*---------------------------------------------------------------------------------**//**
 @bsimethod                                                     
+---------------+---------------+---------------+---------------+---------------+------*/
bool Schema::SchemaNameIsValid 
(
const wchar_t * name
)
    {
    // AZK TODO - implement this method.  How should errors be handled.  Return code or exception?
    return true;    
    }

END_BENTLEY_EC_NAMESPACE
