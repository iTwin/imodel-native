/*--------------------------------------------------------------------------------------+
|
|     $Source: src/ECProperty.cpp $
|
|  $Copyright: (c) 2013 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#include "ECObjectsPch.h"

BEGIN_BENTLEY_ECOBJECT_NAMESPACE

// If you are developing schemas, particularly when editing them by hand, you want to have this variable set to false so you get the asserts to help you figure out what is going wrong.
// Test programs generally want to get error status back and not assert, so they call ECSchema::AssertOnXmlError (false);
static  bool        s_noAssert = false;
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Carole.MacDonald                10/2011
+---------------+---------------+---------------+---------------+---------------+------*/
void ECProperty::SetErrorHandling (bool doAssert) 
    { 
    s_noAssert = !doAssert; 
    }


/*---------------------------------------------------------------------------------**//**
 @bsimethod                                                 
+---------------+---------------+---------------+---------------+---------------+------*/
ECProperty::ECProperty (ECClassCR ecClass)
    :
    m_class(ecClass), m_readOnly(false), m_baseProperty(NULL), m_forSupplementation(false), m_cachedTypeAdapter(NULL)
    {
    //
    }

/*---------------------------------------------------------------------------------**//**
 @bsimethod                                                 
+---------------+---------------+---------------+---------------+---------------+------*/
ECProperty::~ECProperty ()
    {
    //
    }

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
    return m_validatedName.GetName();
    }

/*---------------------------------------------------------------------------------**//**
 @bsimethod                                                     
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus ECProperty::SetName (WStringCR name)
    {        
    m_validatedName.SetName (name.c_str());
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
    return m_validatedName.GetDisplayLabel();
    }

/*---------------------------------------------------------------------------------**//**
 @bsimethod                                                     
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus ECProperty::SetDisplayLabel (WStringCR displayLabel)
    {        
    m_validatedName.SetDisplayLabel (displayLabel.c_str());
    return ECOBJECTS_STATUS_Success;
    }

/*---------------------------------------------------------------------------------**//**
 @bsimethod                                                     
+---------------+---------------+---------------+---------------+---------------+------*/
bool ECProperty::GetIsReadOnly () const
    {
    CalculatedPropertySpecificationCP calcSpec;
    if (m_readOnly)
        return true;
    else if (GetIsPrimitive() && NULL != (calcSpec = GetAsPrimitiveProperty()->GetCalculatedPropertySpecification()))
        return calcSpec->IsReadOnly();
    else
        return false;
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
        LOG.errorv (L"Failed to parse the isReadOnly string '%ls' for ECProperty '%ls'.", isReadOnly, this->GetName().c_str());
    else
        SetIsReadOnly (bReadOnly);
        
    return status;
    }

/*---------------------------------------------------------------------------------**//**
 @bsimethod                                                     
+---------------+---------------+---------------+---------------+---------------+------*/
bool ECProperty::GetIsDisplayLabelDefined () const
    {
    return m_validatedName.IsDisplayLabelDefined();
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
bool ECProperty::GetIsStruct () const
    {
    return this->_IsStruct();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   01/13
+---------------+---------------+---------------+---------------+---------------+------*/
PrimitiveECPropertyCP   ECProperty::GetAsPrimitiveProperty() const  { return GetIsPrimitive() ? static_cast<PrimitiveECPropertyCP>(this) : NULL; }
PrimitiveECPropertyP    ECProperty::GetAsPrimitivePropertyP()       { return const_cast<PrimitiveECPropertyP>(GetAsPrimitiveProperty()); }
ArrayECPropertyCP       ECProperty::GetAsArrayProperty() const      { return GetIsArray() ? static_cast<ArrayECPropertyCP>(this) : NULL; }
ArrayECPropertyP        ECProperty::GetAsArrayPropertyP()           { return const_cast<ArrayECPropertyP>(GetAsArrayProperty()); }
StructECPropertyCP      ECProperty::GetAsStructProperty() const     { return GetIsStruct() ? static_cast<StructECPropertyCP>(this) : NULL; }
StructECPropertyP       ECProperty::GetAsStructPropertyP()          { return const_cast<StructECPropertyP>(GetAsStructProperty()); }

/*---------------------------------------------------------------------------------**//**
 @bsimethod                                                     
+---------------+---------------+---------------+---------------+---------------+------*/
bool ECProperty::GetIsArray () const
    {
    return this->_IsArray();
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
SchemaReadStatus ECProperty::_ReadXml (BeXmlNodeR propertyNode, ECSchemaReadContextR context)
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

    ReadCustomAttributes (propertyNode, context, GetClass().GetSchema());
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

    // If this property was created during supplementation as a local property on the class, then don't serialize it
    if (m_forSupplementation)
        return status;

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
SchemaReadStatus PrimitiveECProperty::_ReadXml (BeXmlNodeR propertyNode, ECSchemaReadContextR context)
    {  
    SchemaReadStatus status = T_Super::_ReadXml (propertyNode, context);
    if (status != SCHEMA_READ_STATUS_Success)
        return status;

    // typeName is a required attribute.  If it is missing, an error will be returned.
    // For Primitive & Array properties we ignore parse errors and default to string.  Struct properties will require a resolvable typename.
    WString value;  // needed for macro.
    if (BEXML_Success != propertyNode.GetAttributeStringValue (value, TYPE_NAME_ATTRIBUTE))
        {
        BeAssert (s_noAssert);
        LOG.errorv (L"Invalid ECSchemaXML: %hs element must contain a %hs attribute",  propertyNode.GetName(), TYPE_NAME_ATTRIBUTE);
        return SCHEMA_READ_STATUS_InvalidECSchemaXml;
        }
    else if (ECOBJECTS_STATUS_ParseError == this->SetTypeName (value.c_str()))
        LOG.warningv (L"Defaulting the type of ECProperty '%ls' to '%ls' in reaction to non-fatal parse error.", this->GetName().c_str(), this->GetTypeName().c_str());
    return SCHEMA_READ_STATUS_Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   
+---------------+---------------+---------------+---------------+---------------+------*/
SchemaWriteStatus PrimitiveECProperty::_WriteXml (BeXmlNodeP& propertyNode, BeXmlNodeR parentNode)
    {
    return T_Super::_WriteXml (propertyNode, parentNode, EC_PROPERTY_ELEMENT);
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
        ArrayECPropertyCP arrayProperty = baseProperty.GetAsArrayProperty();
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
        LOG.warningv (L"Unrecognized primitive typeName '%ls' found in '%ls:%ls.%ls'. A type of 'string' will be used.",
                                typeName.c_str(),
                                this->GetClass().GetSchema().GetName().c_str(),
                                this->GetClass().GetName().c_str(),
                                this->GetName().c_str() );
        return status;
        }
    else if (PRIMITIVETYPE_IGeometry == primitiveType)
        m_originalTypeName = typeName; // Internally we treat everything as the common Bentley.Geometry.Common.IGeometry, but we need to preserve the actual type

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
* @bsimethod                                                    Paul.Connelly   08/12
+---------------+---------------+---------------+---------------+---------------+------*/
bool PrimitiveECProperty::IsCalculated() const
    {
    return GetCustomAttribute (L"CalculatedECPropertySpecification").IsValid();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   08/12
+---------------+---------------+---------------+---------------+---------------+------*/
CalculatedPropertySpecificationCP PrimitiveECProperty::GetCalculatedPropertySpecification() const
    {
    if (m_calculatedSpec.IsNull())
        m_calculatedSpec = CalculatedPropertySpecification::Create (*this);

    return m_calculatedSpec.get();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   
+---------------+---------------+---------------+---------------+---------------+------*/
SchemaReadStatus StructECProperty::_ReadXml (BeXmlNodeR propertyNode, ECSchemaReadContextR context)
    {  
    SchemaReadStatus status = T_Super::_ReadXml (propertyNode, context);
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
    return T_Super::_WriteXml (propertyNode, parentNode, EC_STRUCTPROPERTY_ELEMENT);
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
        ArrayECPropertyCP arrayProp = baseProperty.GetAsArrayProperty();
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
ECObjectsStatus ResolveStructType (ECClassCP& structClass, WStringCR typeName, ECPropertyCR ecProperty)
    {
    // typeName may potentially be qualified so we must parse into a namespace prefix and short class name
    WString namespacePrefix;
    WString className;
    ECObjectsStatus status = ECClass::ParseClassName (namespacePrefix, className, typeName);
    if (ECOBJECTS_STATUS_Success != status)
        {
        LOG.warningv (L"Cannot resolve the type name '%ls' as a struct type because the typeName could not be parsed.", typeName.c_str());
        return status;
        }
    
    ECSchemaCP resolvedSchema = ecProperty.GetClass().GetSchema().GetSchemaByNamespacePrefixP (namespacePrefix);
    if (NULL == resolvedSchema)
        {
        LOG.warningv (L"Cannot resolve the type name '%ls' as a struct type because the namespacePrefix '%ls' can not be resolved to the primary or a referenced schema.", 
            typeName.c_str(), namespacePrefix.c_str());
        return ECOBJECTS_STATUS_SchemaNotFound;
        }

    structClass = resolvedSchema->GetClassCP (className.c_str());
    if (NULL == structClass)
        {
        LOG.warningv (L"Cannot resolve the type name '%ls' as a struct type because ECClass '%ls' does not exist in the schema '%ls'.", 
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
    ECClassCP structClass;
    ECObjectsStatus status = ResolveStructType (structClass, typeName, *this);
    if (ECOBJECTS_STATUS_Success != status)
        {
        LOG.errorv (L"Failed to set the type name of ECStructProperty '%ls' to '%ls' because the typeName could not be parsed into a resolvable ECClass.", this->GetName().c_str(), typeName.c_str());        
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
SchemaReadStatus ArrayECProperty::_ReadXml (BeXmlNodeR propertyNode, ECSchemaReadContextR context)
    {  
    SchemaReadStatus status = T_Super::_ReadXml (propertyNode, context);
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
        LOG.warningv (L"Defaulting the type of ECProperty '%ls' to '%ls' in reaction to non-fatal parse error.", this->GetName().c_str(), this->GetTypeName().c_str());
        return SCHEMA_READ_STATUS_Success;
        }

    return SCHEMA_READ_STATUS_Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   
+---------------+---------------+---------------+---------------+---------------+------*/
SchemaWriteStatus ArrayECProperty::_WriteXml (BeXmlNodeP& propertyNode, BeXmlNodeR parentNode)
    {
    SchemaWriteStatus status = T_Super::_WriteXml (propertyNode, parentNode, EC_ARRAYPROPERTY_ELEMENT);
    if (status != SCHEMA_WRITE_STATUS_Success)
        return status;
        
    if (NULL == propertyNode)
        {
        BeAssert (false);
        return SCHEMA_WRITE_STATUS_FailedToCreateXml;
        }
        
    // verify that this really is the current array property element
    if (0 != strcmp (propertyNode->GetName(), EC_ARRAYPROPERTY_ELEMENT))
        {
        BeAssert (false);
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
    
    ECClassCP structClass;
    status = ResolveStructType (structClass, typeName, *this);
    if (ECOBJECTS_STATUS_Success == status)
        return SetStructElementType (structClass);

    m_originalTypeName = typeName;
    LOG.warningv (L"TypeName '%ls' of '%ls.%ls.%ls' was not recognized. We will use 'string' intead.",
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
        LOG.errorv (L"ECArrayProperty '%ls' uses ECClass '%ls', but isStructClass='false' on '%ls'", GetName().c_str(), structType->GetName().c_str(), structType->GetName().c_str());
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
    int count = BeStringUtilities::Swscanf (minOccurs.c_str(), L"%u", &iMinOccurs);
    if (count != 1)
        {
        LOG.errorv (L"Failed to set MinOccurs of ECProperty '%ls' to '%ls' because the value could not be parsed.  It must be a valid unsigned integer.",
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
    return /* m_maxOccurs; */ UINT_MAX; // D-106653
    }

/*---------------------------------------------------------------------------------**//**
 @bsimethod                                                     
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus ArrayECProperty::SetMaxOccurs (UInt32 maxOccurs)
    {
    // D-106653: Note: We store maxOccurs as read from schema, but we do not enforce it - callers can increase the size of the array beyond maxOccurs and array elements are always stored in variable-size section
    // of ECD buffer.
    // ###TODO: should we allocate space for maxOccurs elements when initializing ECD buffer? Test code expects this, but does real code?
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
    int count = BeStringUtilities::Swscanf (maxOccurs.c_str(), L"%u", &iMaxOccurs);
    if (count != 1)
        {
        if (0 == wcscmp (maxOccurs.c_str(), ECXML_UNBOUNDED))
            iMaxOccurs = UINT_MAX;
        else
            {
            LOG.errorv (L"Failed to set MaxOccurs of ECProperty '%ls' to '%ls' because the value could not be parsed.  It must be a valid unsigned integer or the string 'unbounded'.",
                     this->GetName().c_str(), maxOccurs.c_str());        
            return ECOBJECTS_STATUS_ParseError;
            }
        }
    SetMaxOccurs (iMaxOccurs);
    return ECOBJECTS_STATUS_Success;
    }

/*---------------------------------------------------------------------------------**//**
 @bsimethod                                                     
+---------------+---------------+---------------+---------------+---------------+------*/
bool IECTypeAdapter::HasStandardValues() const                                                                       { return _HasStandardValues(); }
bool IECTypeAdapter::CanConvertFromString () const                                                                   { return _CanConvertFromString (); }
bool IECTypeAdapter::CanConvertToString () const                                                                     { return _CanConvertToString (); }
bool IECTypeAdapter::IsStruct() const                                                                                { return _IsStruct(); }
bool IECTypeAdapter::IsTreatedAsString() const                                                                       { return _IsTreatedAsString(); }
IECInstancePtr IECTypeAdapter::CreateDefaultFormatter (bool includeAllValues, bool forDwg) const                     { return _CreateDefaultFormatter (includeAllValues, forDwg); }
IECInstancePtr IECTypeAdapter::CondenseFormatterForSerialization (IECInstanceCR formatter) const                     { return _CondenseFormatterForSerialization (formatter); }
IECInstancePtr IECTypeAdapter::PopulateDefaultFormatterProperties (IECInstanceCR formatter) const                    { return _PopulateDefaultFormatterProperties (formatter); }
bool IECTypeAdapter::ConvertToString (WStringR str, ECValueCR v, IECTypeAdapterContextCR context, IECInstanceCP opts) { return _ConvertToString (str, v, context, opts); }
bool IECTypeAdapter::ConvertFromString (ECValueR v, WCharCP str, IECTypeAdapterContextCR context)                    { return _ConvertFromString (v, str, context); }
bool IECTypeAdapter::ConvertToExpressionType (ECValueR v, IECTypeAdapterContextCR context)                           { return _ConvertToExpressionType (v, context); }
bool IECTypeAdapter::ConvertFromExpressionType (ECValueR v, IECTypeAdapterContextCR context)                         { return _ConvertFromExpressionType (v, context); }
bool IECTypeAdapter::RequiresExpressionTypeConversion() const                                                        { return _RequiresExpressionTypeConversion(); }

ECPropertyCP        IECTypeAdapterContext::GetProperty() const           { return _GetProperty(); }
UInt32              IECTypeAdapterContext::GetComponentIndex() const     { return _GetComponentIndex(); }
bool                IECTypeAdapterContext::Is3d() const                  { return _Is3d(); }
IECInstanceCP       IECTypeAdapterContext::GetECInstance() const         { return _GetECInstance(); }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   01/13
+---------------+---------------+---------------+---------------+---------------+------*/
static IECTypeAdapter::Factory const* s_typeAdapterFactory;
void IECTypeAdapter::SetFactory (Factory const& factory)                { s_typeAdapterFactory = &factory; }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   01/13
+---------------+---------------+---------------+---------------+---------------+------*/
IECTypeAdapter* ECProperty::GetTypeAdapter() const
    {
    if (NULL != GetCachedTypeAdapter())
        return GetCachedTypeAdapter();
    
    return NULL != s_typeAdapterFactory ? &s_typeAdapterFactory->GetForProperty (*this) : NULL;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   01/13
+---------------+---------------+---------------+---------------+---------------+------*/
IECTypeAdapter* ArrayECProperty::GetMemberTypeAdapter() const
    {
    if (NULL != GetCachedMemberTypeAdapter())
        return GetCachedMemberTypeAdapter();

    return NULL != s_typeAdapterFactory ? &s_typeAdapterFactory->GetForArrayMember (*this) : NULL;
    }

END_BENTLEY_ECOBJECT_NAMESPACE
