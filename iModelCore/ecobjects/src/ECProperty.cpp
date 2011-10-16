/*--------------------------------------------------------------------------------------+
|
|     $Source: src/ECProperty.cpp $
|    $RCSfile: file.tpl,v $
|   $Revision: 1.10 $
|       $Date: 2005/11/07 15:38:45 $
|     $Author: EarlinLutz $
|
|  $Copyright: (c) 2011 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#include "ECObjectsPch.h"

BEGIN_BENTLEY_EC_NAMESPACE

//#define DEBUG_PROPERTY_LEAKS
#ifdef DEBUG_PROPERTY_LEAKS
static LeakDetector<ECProperty> g_leakDetector (L"ECProperty", L"ECProperties", true);
#else
static LeakDetector<ECProperty> g_leakDetector (L"ECProperty", L"ECProperties", false);
#endif
/*---------------------------------------------------------------------------------**//**
 @bsimethod                                                 
+---------------+---------------+---------------+---------------+---------------+------*/
ECProperty::ECProperty (ECClassCR ecClass, bool hideFromLeakDetection)
    :
    m_class(ecClass), m_readOnly(false), m_baseProperty(NULL), m_hideFromLeakDetection (hideFromLeakDetection)
    {
    if ( ! m_hideFromLeakDetection)
        g_leakDetector.ObjectCreated(*this);
    };

/*---------------------------------------------------------------------------------**//**
 @bsimethod                                                 
+---------------+---------------+---------------+---------------+---------------+------*/
ECProperty::~ECProperty ()
    {
    if ( ! m_hideFromLeakDetection)
        g_leakDetector.ObjectDestroyed(*this);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    09/10
+---------------+---------------+---------------+---------------+---------------+------*/
ILeakDetector&  ECProperty::Debug_GetLeakDetector() { return g_leakDetector; }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Carole.MacDonald                06/2010
+---------------+---------------+---------------+---------------+---------------+------*/
ECPropertyCP ECProperty::GetBaseProperty () const
    {
    return m_baseProperty;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Carole.MacDonald                06/2010
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus ECProperty::SetBaseProperty (ECPropertyCP baseProperty)
    {
    m_baseProperty = baseProperty;
    return ECOBJECTS_STATUS_Success;
    }

/*---------------------------------------------------------------------------------**//**
 @bsimethod                                                     
+---------------+---------------+---------------+---------------+---------------+------*/
ECClassCR ECProperty::GetClass () const
    {
    return m_class;
    }

/*---------------------------------------------------------------------------------**//**
 @bsimethod                                                     
+---------------+---------------+---------------+---------------+---------------+------*/
WStringCR ECProperty::GetName () const
    {
    return m_name;
    }

/*---------------------------------------------------------------------------------**//**
 @bsimethod                                                     
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus ECProperty::SetName (WStringCR name)
    {        
    if (!NameValidator::Validate(name))
        return ECOBJECTS_STATUS_InvalidName;
 
    m_name = name;        
    return ECOBJECTS_STATUS_Success;
    }

/*---------------------------------------------------------------------------------**//**
 @bsimethod                                                     
+---------------+---------------+---------------+---------------+---------------+------*/
WStringCR ECProperty::GetDescription () const
    {
    return m_description;        
    }

/*---------------------------------------------------------------------------------**//**
 @bsimethod                                                     
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus ECProperty::SetDescription (WStringCR description)
    {        
    m_description = description;
    return ECOBJECTS_STATUS_Success;
    }

/*---------------------------------------------------------------------------------**//**
 @bsimethod                                                     
+---------------+---------------+---------------+---------------+---------------+------*/
WStringCR ECProperty::GetDisplayLabel () const
    {
    return (m_displayLabel.empty()) ? GetName() : m_displayLabel;
    }

/*---------------------------------------------------------------------------------**//**
 @bsimethod                                                     
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus ECProperty::SetDisplayLabel (WStringCR displayLabel)
    {        
    m_displayLabel = displayLabel;
    return ECOBJECTS_STATUS_Success;
    }

/*---------------------------------------------------------------------------------**//**
 @bsimethod                                                     
+---------------+---------------+---------------+---------------+---------------+------*/
bool ECProperty::GetIsReadOnly () const
    {
    return m_readOnly;
    }

/*---------------------------------------------------------------------------------**//**
 @bsimethod                                                     
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus ECProperty::SetIsReadOnly (bool readOnly)
    {        
    m_readOnly = readOnly;
    return ECOBJECTS_STATUS_Success;
    }

/*---------------------------------------------------------------------------------**//**
 @bsimethod                                                     
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus ECProperty::SetIsReadOnly (WCharCP isReadOnly)
    {        
    PRECONDITION (NULL != isReadOnly, ECOBJECTS_STATUS_PreconditionViolated);

    bool bReadOnly;
    ECObjectsStatus status = ECXml::ParseBooleanString (bReadOnly, isReadOnly);
    if (ECOBJECTS_STATUS_Success != status)
        ECObjectsLogger::Log()->errorv (L"Failed to parse the isReadOnly string '%s' for ECProperty '%s'.", isReadOnly, this->GetName().c_str());
    else
        SetIsReadOnly (bReadOnly);
        
    return status;
    }

/*---------------------------------------------------------------------------------**//**
 @bsimethod                                                     
+---------------+---------------+---------------+---------------+---------------+------*/
bool ECProperty::GetIsDisplayLabelDefined () const
    {
    return (!m_displayLabel.empty());        
    }

/*---------------------------------------------------------------------------------**//**
 @bsimethod                                                     
+---------------+---------------+---------------+---------------+---------------+------*/
WString ECProperty::GetTypeName () const
    {
    return this->_GetTypeName();
    }

/*---------------------------------------------------------------------------------**//**
 @bsimethod                                                     
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus ECProperty::SetTypeName (WString typeName)
    {
    return this->_SetTypeName (typeName);
    }

/*---------------------------------------------------------------------------------**//**
 @bsimethod                                                     
+---------------+---------------+---------------+---------------+---------------+------*/
bool ECProperty::GetIsPrimitive () const
    {
    return this->_IsPrimitive();
    }

/*---------------------------------------------------------------------------------**//**
 @bsimethod                                                     
+---------------+---------------+---------------+---------------+---------------+------*/
PrimitiveECPropertyP ECProperty::GetAsPrimitiveProperty () const
    {
    // virtual get method is significantly faster than dynamic_cast
    assert (dynamic_cast<PrimitiveECPropertyP>(const_cast<ECPropertyP>(this)) == const_cast<ECPropertyP>(this)->_GetAsPrimitiveECProperty());
    return const_cast<ECPropertyP>(this)->_GetAsPrimitiveECProperty();
    }

/*---------------------------------------------------------------------------------**//**
 @bsimethod                                                     
+---------------+---------------+---------------+---------------+---------------+------*/
bool ECProperty::GetIsStruct () const
    {
    return this->_IsStruct();
    }

/*---------------------------------------------------------------------------------**//**
 @bsimethod                                                     
+---------------+---------------+---------------+---------------+---------------+------*/
StructECPropertyP ECProperty::GetAsStructProperty () const
    {
    return dynamic_cast<StructECPropertyP>((ECPropertyP)this);
    }

/*---------------------------------------------------------------------------------**//**
 @bsimethod                                                     
+---------------+---------------+---------------+---------------+---------------+------*/
bool ECProperty::GetIsArray () const
    {
    return this->_IsArray();
    }

/*---------------------------------------------------------------------------------**//**
 @bsimethod                                                     
+---------------+---------------+---------------+---------------+---------------+------*/
ArrayECPropertyP ECProperty::GetAsArrayProperty () const
    {
    return dynamic_cast<ArrayECPropertyP>((ECPropertyP)this);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Carole.MacDonald                06/2010
+---------------+---------------+---------------+---------------+---------------+------*/
void ECProperty::_GetBaseContainers (bvector<IECCustomAttributeContainerP>& returnList) const
    {
    if (NULL != m_baseProperty)
        returnList.push_back((const_cast<ECPropertyP>(m_baseProperty)));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Carole.MacDonald                06/2010
+---------------+---------------+---------------+---------------+---------------+------*/
ECSchemaCP ECProperty::_GetContainerSchema () const
    {
    return &(m_class.GetSchema());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   
+---------------+---------------+---------------+---------------+---------------+------*/
SchemaReadStatus ECProperty::_ReadXml (BeXmlNodeR propertyNode, IStandaloneEnablerLocaterP standaloneEnablerLocater)
    {  
    WString value;
    READ_REQUIRED_XML_ATTRIBUTE (propertyNode, PROPERTY_NAME_ATTRIBUTE, this, Name, propertyNode.GetName())        
    
    // OPTIONAL attributes - If these attributes exist they MUST be valid    
    READ_OPTIONAL_XML_ATTRIBUTE (propertyNode, DESCRIPTION_ATTRIBUTE,         this, Description)
    READ_OPTIONAL_XML_ATTRIBUTE (propertyNode, DISPLAY_LABEL_ATTRIBUTE,       this, DisplayLabel)    

    // OPTIONAL attributes - If these attributes exist they do not need to be valid.  We will ignore any errors setting them and use default values.
    // NEEDSWORK This is due to the current implementation in managed ECObjects.  We should reconsider whether it is the correct behavior.
    ECObjectsStatus setterStatus;
    READ_OPTIONAL_XML_ATTRIBUTE_IGNORING_SET_ERRORS (propertyNode, READONLY_ATTRIBUTE,            this, IsReadOnly)

    ReadCustomAttributes (propertyNode, m_class.GetSchema(), standaloneEnablerLocater);
    return SCHEMA_READ_STATUS_Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   
+---------------+---------------+---------------+---------------+---------------+------*/
SchemaWriteStatus ECProperty::_WriteXml (BeXmlNodeP& propertyNode, BeXmlNodeR parentNode)
    {
    return _WriteXml (propertyNode, parentNode, EC_PROPERTY_ELEMENT);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   
+---------------+---------------+---------------+---------------+---------------+------*/
SchemaWriteStatus ECProperty::_WriteXml (BeXmlNodeP& propertyNode, BeXmlNodeR parentNode, Utf8CP elementName)
    {
    SchemaWriteStatus status = SCHEMA_WRITE_STATUS_Success;

    propertyNode = parentNode.AddEmptyElement (elementName);

    propertyNode->AddAttributeStringValue (PROPERTY_NAME_ATTRIBUTE, this->GetName().c_str());

    if (m_originalTypeName.size() > 0)
        propertyNode->AddAttributeStringValue (TYPE_NAME_ATTRIBUTE, m_originalTypeName.c_str());
    else
    	propertyNode->AddAttributeStringValue (TYPE_NAME_ATTRIBUTE, this->GetTypeName().c_str());
        
    propertyNode->AddAttributeStringValue (DESCRIPTION_ATTRIBUTE, this->GetDescription().c_str());
    if (GetIsDisplayLabelDefined())
        propertyNode->AddAttributeStringValue (DISPLAY_LABEL_ATTRIBUTE, this->GetDisplayLabel().c_str());
    propertyNode->AddAttributeBooleanValue (READONLY_ATTRIBUTE, this->GetIsReadOnly());
    
    WriteCustomAttributes (*propertyNode);

    return status;    
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   
+---------------+---------------+---------------+---------------+---------------+------*/
SchemaReadStatus PrimitiveECProperty::_ReadXml (BeXmlNodeR propertyNode, IStandaloneEnablerLocaterP standaloneEnablerLocater)
    {  
    SchemaReadStatus status = __super::_ReadXml (propertyNode, standaloneEnablerLocater);
    if (status != SCHEMA_READ_STATUS_Success)
        return status;

    // typeName is a required attribute.  If it is missing, an error will be returned.
    // For Primitive & Array properties we ignore parse errors and default to string.  Struct properties will require a resolvable typename.
    WString value;  // needed for macro.
    if (BEXML_Success != propertyNode.GetAttributeStringValue (value, TYPE_NAME_ATTRIBUTE))
        {
        assert (false);
        ECObjectsLogger::Log()->errorv (L"Invalid ECSchemaXML: %hs element must contain a %hs attribute",  propertyNode.GetName(), TYPE_NAME_ATTRIBUTE);
        return SCHEMA_READ_STATUS_InvalidECSchemaXml;
        }
    else if (ECOBJECTS_STATUS_ParseError == this->SetTypeName (value.c_str()))
        ECObjectsLogger::Log()->warningv (L"Defaulting the type of ECProperty '%s' to '%s' in reaction to non-fatal parse error.", this->GetName().c_str(), this->GetTypeName().c_str());
    return SCHEMA_READ_STATUS_Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   
+---------------+---------------+---------------+---------------+---------------+------*/
SchemaWriteStatus PrimitiveECProperty::_WriteXml (BeXmlNodeP& propertyNode, BeXmlNodeR parentNode)
    {
    return __super::_WriteXml (propertyNode, parentNode, EC_PROPERTY_ELEMENT);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Carole.MacDonald                05/2010
+---------------+---------------+---------------+---------------+---------------+------*/
bool PrimitiveECProperty::_CanOverride (ECPropertyCR baseProperty) const
    {
    PrimitiveType basePrimitiveType;
    
    // normally, we do not allow a primitive property to override an array property.  However, there is a set of schemas that
    // have been delivered that allow this behavior.  If the primitive property type is the same as the type used in the array, then
    // we allow it to be overridden.
    if (baseProperty.GetIsArray())
        {
        ArrayECPropertyP arrayProperty = baseProperty.GetAsArrayProperty();
        if (ARRAYKIND_Struct == arrayProperty->GetKind())
            return false;
        basePrimitiveType = arrayProperty->GetPrimitiveElementType();
        }
    else if (baseProperty.GetIsStruct())
        return false;
    else
        {
        basePrimitiveType = baseProperty.GetAsPrimitiveProperty()->GetType();
        }
        
    return (basePrimitiveType == m_primitiveType);
    }
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   
+---------------+---------------+---------------+---------------+---------------+------*/
WString PrimitiveECProperty::_GetTypeName () const
    {
    return ECXml::GetPrimitiveTypeName (m_primitiveType);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus PrimitiveECProperty::_SetTypeName (WStringCR typeName)
    {
    PrimitiveType primitiveType;
    ECObjectsStatus status = ECXml::ParsePrimitiveType (primitiveType, typeName);
    if (ECOBJECTS_STATUS_Success != status)
        {            
        m_originalTypeName = typeName; // Remember this for when we serialize the ECSchema again, later.
        ECObjectsLogger::Log()->warningv (L"Unrecognized primitive typeName '%s' found in '%s:%s.%s'. A type of 'string' will be used.",
                                typeName.c_str(),
                                this->GetClass().GetSchema().GetName().c_str(),
                                this->GetClass().GetName().c_str(),
                                this->GetName().c_str() );
        return status;
        }

    return SetType (primitiveType);
    }

/*---------------------------------------------------------------------------------**//**
 @bsimethod                                                     
+---------------+---------------+---------------+---------------+---------------+------*/
PrimitiveType PrimitiveECProperty::GetType () const
    {
    return m_primitiveType;
    }

/*---------------------------------------------------------------------------------**//**
 @bsimethod                                                     
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus PrimitiveECProperty::SetType (PrimitiveType primitiveType)
    {        
    m_primitiveType = primitiveType;        
    return ECOBJECTS_STATUS_Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   
+---------------+---------------+---------------+---------------+---------------+------*/
SchemaReadStatus StructECProperty::_ReadXml (BeXmlNodeR propertyNode, IStandaloneEnablerLocaterP standaloneEnablerLocater)
    {  
    SchemaReadStatus status = __super::_ReadXml (propertyNode, standaloneEnablerLocater);
    if (status != SCHEMA_READ_STATUS_Success)
        return status;

    // For Primitive & Array properties we ignore parse errors and default to string.  Struct properties will require a resolvable typename.
    WString value;  // needed for macro.
    READ_REQUIRED_XML_ATTRIBUTE (propertyNode, TYPE_NAME_ATTRIBUTE, this, TypeName, propertyNode.GetName())        

    return SCHEMA_READ_STATUS_Success;
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   
+---------------+---------------+---------------+---------------+---------------+------*/
SchemaWriteStatus StructECProperty::_WriteXml (BeXmlNodeP& propertyNode, BeXmlNodeR parentNode)
    {
    return __super::_WriteXml (propertyNode, parentNode, EC_STRUCTPROPERTY_ELEMENT);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Carole.MacDonald                05/2010
+---------------+---------------+---------------+---------------+---------------+------*/
bool StructECProperty::_CanOverride (ECPropertyCR baseProperty) const
    {
    if (baseProperty.GetIsPrimitive())
        return false;
        
    if (baseProperty.GetIsArray())
        {
        ArrayECPropertyP arrayProp = baseProperty.GetAsArrayProperty();
        if (ARRAYKIND_Struct != arrayProp->GetKind())
            return false;
        }

    // if the struct type hasn't been set yet, we will say it can override
    if (NULL == m_structType)
        return true;

    return (GetTypeName() == baseProperty.GetTypeName());
    }
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   
+---------------+---------------+---------------+---------------+---------------+------*/
WString StructECProperty::_GetTypeName () const
    {
    if (!EXPECTED_CONDITION (NULL != m_structType))
        return EMPTY_STRING;
    return ECClass::GetQualifiedClassName (this->GetClass().GetSchema(), *m_structType);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus ResolveStructType (ECClassP& structClass, WStringCR typeName, ECPropertyCR ecProperty)
    {
    // typeName may potentially be qualified so we must parse into a namespace prefix and short class name
    WString namespacePrefix;
    WString className;
    ECObjectsStatus status = ECClass::ParseClassName (namespacePrefix, className, typeName);
    if (ECOBJECTS_STATUS_Success != status)
        {
        ECObjectsLogger::Log()->warningv (L"Cannot resolve the type name '%s' as a struct type because the typeName could not be parsed.", typeName.c_str());
        return status;
        }
    
    ECSchemaP resolvedSchema = ecProperty.GetClass().GetSchema().GetSchemaByNamespacePrefixP (namespacePrefix);
    if (NULL == resolvedSchema)
        {
        ECObjectsLogger::Log()->warningv (L"Cannot resolve the type name '%s' as a struct type because the namespacePrefix '%s' can not be resolved to the primary or a referenced schema.", 
            typeName.c_str(), namespacePrefix.c_str());
        return ECOBJECTS_STATUS_SchemaNotFound;
        }

    structClass = resolvedSchema->GetClassP (className.c_str());
    if (NULL == structClass)
        {
        ECObjectsLogger::Log()->warningv (L"Cannot resolve the type name '%s' as a struct type because ECClass '%s' does not exist in the schema '%s'.", 
            typeName.c_str(), className.c_str(), resolvedSchema->GetName().c_str());
        return ECOBJECTS_STATUS_ClassNotFound;
        }

    return ECOBJECTS_STATUS_Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus StructECProperty::_SetTypeName (WStringCR typeName)
    {
    ECClassP structClass;
    ECObjectsStatus status = ResolveStructType (structClass, typeName, *this);
    if (ECOBJECTS_STATUS_Success != status)
        {
        ECObjectsLogger::Log()->errorv (L"Failed to set the type name of ECStructProperty '%s' to '%s' because the typeName could not be parsed into a resolvable ECClass.", this->GetName().c_str(), typeName.c_str());        
        return status;
        }
    else
        return SetType (*structClass);
    }

/*---------------------------------------------------------------------------------**//**
 @bsimethod                                                     
+---------------+---------------+---------------+---------------+---------------+------*/
ECClassCR StructECProperty::GetType () const
    {        
    DEBUG_EXPECT (NULL != m_structType);
    return *m_structType;
    }

/*---------------------------------------------------------------------------------**//**
 @bsimethod                                                     
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus StructECProperty::SetType (ECClassCR structType)
    {            
    PRECONDITION (structType.GetIsStruct(), ECOBJECTS_STATUS_PreconditionViolated);

    if (&(structType.GetSchema()) != &(this->GetClass().GetSchema()))
        {
        if (!ECSchema::IsSchemaReferenced(this->GetClass().GetSchema(), structType.GetSchema()))
            return ECOBJECTS_STATUS_SchemaNotFound;
        }
    
    m_structType = &structType;
    return ECOBJECTS_STATUS_Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   
+---------------+---------------+---------------+---------------+---------------+------*/
SchemaReadStatus ArrayECProperty::_ReadXml (BeXmlNodeR propertyNode, IStandaloneEnablerLocaterP standaloneEnablerLocater)
    {  
    SchemaReadStatus status = __super::_ReadXml (propertyNode, standaloneEnablerLocater);
    if (status != SCHEMA_READ_STATUS_Success)
        return status;

    // OPTIONAL attributes - If these attributes exist they do not need to be valid.  We will ignore any errors setting them and use default values.
    // NEEDSWORK This is due to the current implementation in managed ECObjects.  We should reconsider whether it is the correct behavior.
    ECObjectsStatus setterStatus;   // needed for macro.
    WString         value;          // needed for macro.
    READ_OPTIONAL_XML_ATTRIBUTE_IGNORING_SET_ERRORS (propertyNode, MIN_OCCURS_ATTRIBUTE, this, MinOccurs)    
    READ_OPTIONAL_XML_ATTRIBUTE_IGNORING_SET_ERRORS (propertyNode, MAX_OCCURS_ATTRIBUTE, this, MaxOccurs)

    // For Primitive & Array properties we ignore parse errors and default to string.  Struct properties will require a resolvable typename.
    READ_REQUIRED_XML_ATTRIBUTE_IGNORING_SET_ERRORS (propertyNode, TYPE_NAME_ATTRIBUTE, this, TypeName, propertyNode.GetName())  

    if (SCHEMA_READ_STATUS_FailedToParseXml == setterStatus)
        {
        ECObjectsLogger::Log()->warningv (L"Defaulting the type of ECProperty '%ls' to '%ls' in reaction to non-fatal parse error.", this->GetName().c_str(), this->GetTypeName().c_str());
        return SCHEMA_READ_STATUS_Success;
        }

    return SCHEMA_READ_STATUS_Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   
+---------------+---------------+---------------+---------------+---------------+------*/
SchemaWriteStatus ArrayECProperty::_WriteXml (BeXmlNodeP& propertyNode, BeXmlNodeR parentNode)
    {
    SchemaWriteStatus status = __super::_WriteXml (propertyNode, parentNode, EC_ARRAYPROPERTY_ELEMENT);
    if (status != SCHEMA_WRITE_STATUS_Success)
        return status;
        
    if (NULL == propertyNode)
        {
        assert (false);
        return SCHEMA_WRITE_STATUS_FailedToCreateXml;
        }
        
    // verify that this really is the current array property element
    if (0 != strcmp (propertyNode->GetName(), EC_ARRAYPROPERTY_ELEMENT))
        {
        assert (false);
        return SCHEMA_WRITE_STATUS_FailedToCreateXml;
        }

    propertyNode->AddAttributeUInt32Value (MIN_OCCURS_ATTRIBUTE, m_minOccurs);

    if (m_maxOccurs != UINT_MAX)
        {
        propertyNode->AddAttributeUInt32Value (MAX_OCCURS_ATTRIBUTE, m_maxOccurs);
        }
    else
        {
        propertyNode->AddAttributeStringValue (MAX_OCCURS_ATTRIBUTE, ECXML_UNBOUNDED);
        }

    if (m_arrayKind == ARRAYKIND_Struct)
        {
        propertyNode->AddAttributeBooleanValue (IS_STRUCT_ATTRIBUTE, true);
        }
        
    return status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Carole.MacDonald                05/2010
+---------------+---------------+---------------+---------------+---------------+------*/
bool ArrayECProperty::_CanOverride (ECPropertyCR baseProperty) const
    {
    return (GetTypeName() == EMPTY_STRING) || (GetTypeName() == baseProperty.GetTypeName());
    }
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   
+---------------+---------------+---------------+---------------+---------------+------*/
WString ArrayECProperty::_GetTypeName () const
    {    
    switch (GetKind())
        {
        case ARRAYKIND_Primitive:
            return ECXml::GetPrimitiveTypeName (m_primitiveType);
        case ARRAYKIND_Struct:
            return ECClass::GetQualifiedClassName (this->GetClass().GetSchema(), *m_structType);
        default:
            return EMPTY_STRING;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus ArrayECProperty::_SetTypeName (WStringCR typeName)
    {
    PrimitiveType primitiveType;
    ECObjectsStatus status = ECXml::ParsePrimitiveType (primitiveType, typeName);
    if (ECOBJECTS_STATUS_Success == status)
        return SetPrimitiveElementType (primitiveType);
    
    ECClassP structClass;
    status = ResolveStructType (structClass, typeName, *this);
    if (ECOBJECTS_STATUS_Success == status)
        return SetStructElementType (structClass);

    m_originalTypeName = typeName;
    ECObjectsLogger::Log()->warningv (L"TypeName '%s' of '%s.%s.%s' was not recognized. We will use 'string' intead.",
                                    typeName.c_str(),
                                    this->GetClass().GetSchema().GetName().c_str(),
                                    this->GetClass().GetName().c_str(),
                                    this->GetName().c_str() );
    return ECOBJECTS_STATUS_ParseError;
    }

/*---------------------------------------------------------------------------------**//**
 @bsimethod                                                     
+---------------+---------------+---------------+---------------+---------------+------*/
ArrayKind ArrayECProperty::GetKind () const
    {
    return m_arrayKind;
    }

/*---------------------------------------------------------------------------------**//**
 @bsimethod                                                     
+---------------+---------------+---------------+---------------+---------------+------*/
PrimitiveType ArrayECProperty::GetPrimitiveElementType () const
    {
    return m_primitiveType;
    }

/*---------------------------------------------------------------------------------**//**
 @bsimethod                                                     
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus ArrayECProperty::SetPrimitiveElementType (PrimitiveType primitiveType)
    {        
    m_arrayKind = ARRAYKIND_Primitive;
    m_primitiveType = primitiveType;
    return ECOBJECTS_STATUS_Success;
    }

/*---------------------------------------------------------------------------------**//**
 @bsimethod                                                     
+---------------+---------------+---------------+---------------+---------------+------*/
ECClassCP ArrayECProperty::GetStructElementType () const
    {
    if (ARRAYKIND_Struct == m_arrayKind)
        return m_structType;
    else
        return NULL;
    }

/*---------------------------------------------------------------------------------**//**
 @bsimethod                                                     
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus ArrayECProperty::SetStructElementType (ECClassCP structType)
    {        
    PRECONDITION (NULL != structType, ECOBJECTS_STATUS_PreconditionViolated);
    if (!structType->GetIsStruct())
        {
        ECObjectsLogger::Log()->errorv (L"ECArrayProperty '%s' uses ECClass '%s', but isStructClass='false' on '%s'", GetName().c_str(), structType->GetName().c_str(), structType->GetName().c_str());
        return ECOBJECTS_STATUS_ParseError;
        }

    if (&(structType->GetSchema()) != &(this->GetClass().GetSchema()))
        {
        if (!ECSchema::IsSchemaReferenced(this->GetClass().GetSchema(), structType->GetSchema()))
            return ECOBJECTS_STATUS_SchemaNotFound;
        }

    m_arrayKind = ARRAYKIND_Struct;
    m_structType = structType;
    return ECOBJECTS_STATUS_Success;
    }

/*---------------------------------------------------------------------------------**//**
 @bsimethod                                                     
+---------------+---------------+---------------+---------------+---------------+------*/
UInt32 ArrayECProperty::GetMinOccurs () const
    {
    return m_minOccurs;
    }

/*---------------------------------------------------------------------------------**//**
 @bsimethod                                                     
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus ArrayECProperty::SetMinOccurs (UInt32 minOccurs)
    {
    PRECONDITION (minOccurs <= m_maxOccurs, ECOBJECTS_STATUS_PreconditionViolated);
    m_minOccurs = minOccurs;
    return ECOBJECTS_STATUS_Success;
    }

/*---------------------------------------------------------------------------------**//**
 @bsimethod                                                     
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus ArrayECProperty::SetMinOccurs (WStringCR minOccurs)
    {    
    UInt32 iMinOccurs;
    int count = swscanf (minOccurs.c_str(), L"%u", &iMinOccurs);
    if (count != 1)
        {
        ECObjectsLogger::Log()->errorv (L"Failed to set MinOccurs of ECProperty '%s' to '%s' because the value could not be parsed.  It must be a valid unsigned integer.",
                 this->GetName().c_str(), minOccurs.c_str());        
        return ECOBJECTS_STATUS_ParseError;
        }    
    SetMinOccurs (iMinOccurs);
    return ECOBJECTS_STATUS_Success;
    }


/*---------------------------------------------------------------------------------**//**
 @bsimethod                                                     
+---------------+---------------+---------------+---------------+---------------+------*/
UInt32 ArrayECProperty::GetMaxOccurs () const
    {
    return m_maxOccurs;
    }

/*---------------------------------------------------------------------------------**//**
 @bsimethod                                                     
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus ArrayECProperty::SetMaxOccurs (UInt32 maxOccurs)
    {
    PRECONDITION (maxOccurs >= m_minOccurs, ECOBJECTS_STATUS_PreconditionViolated);
    m_maxOccurs = maxOccurs;
    return ECOBJECTS_STATUS_Success;
    }

/*---------------------------------------------------------------------------------**//**
 @bsimethod                                                     
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus ArrayECProperty::SetMaxOccurs (WStringCR maxOccurs)
    {    
    UInt32 iMaxOccurs;
    int count = swscanf (maxOccurs.c_str(), L"%u", &iMaxOccurs);
    if (count != 1)
        {
        if (0 == wcscmp (maxOccurs.c_str(), ECXML_UNBOUNDED))
            iMaxOccurs = UINT_MAX;
        else
            {
            ECObjectsLogger::Log()->errorv (L"Failed to set MaxOccurs of ECProperty '%s' to '%s' because the value could not be parsed.  It must be a valid unsigned integer or the string 'unbounded'.",
                     this->GetName().c_str(), maxOccurs.c_str());        
            return ECOBJECTS_STATUS_ParseError;
            }
        }
    SetMaxOccurs (iMaxOccurs);
    return ECOBJECTS_STATUS_Success;
    }

END_BENTLEY_EC_NAMESPACE