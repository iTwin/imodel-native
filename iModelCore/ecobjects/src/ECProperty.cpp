/*--------------------------------------------------------------------------------------+
|
|     $Source: src/ECProperty.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#include "ECObjectsPch.h"

BEGIN_BENTLEY_ECOBJECT_NAMESPACE

// If you are developing schemas, particularly when editing them by hand, you want to have this variable set to false so you get the asserts to help you figure out what is going wrong.
// Test programs generally want to get error status back and not BeAssert, so they call ECSchema::AssertOnXmlError (false);
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
ECProperty::ECProperty (ECClassCR ecClass) : m_class(ecClass), m_readOnly(false), m_baseProperty(nullptr), m_forSupplementation(false),
                                                m_cachedTypeAdapter(nullptr), m_maximumLength(0)
    {}

/*---------------------------------------------------------------------------------**//**
 @bsimethod                                                 
+---------------+---------------+---------------+---------------+---------------+------*/
ECProperty::~ECProperty () {}

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
    if (nullptr != baseProperty)
        {
        // Silly property overriding is silly but we support it.
        // We can end up here if Child derives from Parent which derives from Grandparent and all 3 have a property called "X"
        // And then we remove Parent as a base class of Child, which invokes child.OnBasePropertyRemoved, which wants to set the base property
        // to the base property of Parent.X (=Grandparent.X), which is invalid since there is no longer an inheritance relationship between
        // Child and Grandparent.
        // (Presumably this is useful to someone and not totally confusing?)
        if (!GetClass().Is(&baseProperty->GetClass()))
            return ECObjectsStatus::BaseClassUnacceptable;
        }

    m_baseProperty = baseProperty;
    SetCachedTypeAdapter (NULL);
    return ECObjectsStatus::Success;
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
Utf8StringCR ECProperty::GetName () const
    {
    return m_validatedName.GetName();
    }

/*---------------------------------------------------------------------------------**//**
 @bsimethod                                                      Affan.Khan        12/12
+---------------+---------------+---------------+---------------+---------------+------*/
ECPropertyId ECProperty::GetId () const
    {
    BeAssert (m_ecPropertyId.IsValid());
    return m_ecPropertyId;
    }

/*---------------------------------------------------------------------------------**//**
 @bsimethod                                                     
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus ECProperty::SetName (Utf8StringCR name)
    {        
    if (!ECNameValidation::IsValidName (name.c_str()))
        return ECObjectsStatus::InvalidName;

    m_validatedName.SetName (name.c_str());
    return ECObjectsStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
 @bsimethod                                                     
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8StringCR ECProperty::GetDescription () const
    {
    return GetClass().GetSchema().GetLocalizedStrings().GetPropertyDescription(this, m_description);
    }

/*---------------------------------------------------------------------------------**//**
 @bsimethod                                                     
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8StringCR ECProperty::GetInvariantDescription () const
    {
    return m_description;
    }

/*---------------------------------------------------------------------------------**//**
 @bsimethod                                                     
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus ECProperty::SetDescription (Utf8StringCR description)
    {        
    m_description = description;
    return ECObjectsStatus::Success;
    }

//Helper method to resolve primitive type of any ECProperty, if available
bool ResolvePrimitiveType(ECPropertyCP prop, PrimitiveType& type)
    {
    if (prop == nullptr)
        return false;

    if (prop->GetIsPrimitive())
        {
        PrimitiveECPropertyCP prim = prop->GetAsPrimitiveProperty();
        type = prim->GetType();
        return true;
        }

    if (prop->GetIsPrimitiveArray())
        {
        ArrayECPropertyCP arr = prop->GetAsArrayProperty();
        type = arr->GetPrimitiveElementType();
        return true;
        }

    return false;
    }

/*---------------------------------------------------------------------------------**//**
@bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus ECProperty::SetMinimumValue(ECValueCR min)
    {
    if (min.IsNull() || min.IsStruct())
        return ECObjectsStatus::DataTypeNotSupported;

    PrimitiveType pt = PrimitiveType::PRIMITIVETYPE_Integer;
    if (!ResolvePrimitiveType(this, pt))
        {
        return ECObjectsStatus::DataTypeNotSupported;
        }

    if (pt != PrimitiveType::PRIMITIVETYPE_Double &&
        pt != PrimitiveType::PRIMITIVETYPE_Integer &&
        pt != PrimitiveType::PRIMITIVETYPE_Long)
        {
        return ECObjectsStatus::DataTypeNotSupported;
        }

    if (min.GetPrimitiveType() != pt)
        {
        return ECObjectsStatus::DataTypeNotSupported;
        }

    m_minimumValue = min;
    return ECObjectsStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
@bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool ECProperty::IsMinimumValueDefined() const
    {
    return !m_minimumValue.IsNull();
    }

/*---------------------------------------------------------------------------------**//**
@bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void ECProperty::ResetMinimumValue()
    {
    m_minimumValue.SetToNull();
    }

/*---------------------------------------------------------------------------------**//**
@bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus ECProperty::GetMinimumValue(ECValueR value) const
    {
    if(m_minimumValue.IsNull())
        return ECObjectsStatus::Error;

    value = m_minimumValue;
    return ECObjectsStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
@bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus ECProperty::SetMaximumValue(ECValueCR max)
    {
    if (max.IsNull() || max.IsStruct())
        return ECObjectsStatus::DataTypeNotSupported;

    PrimitiveType pt = PrimitiveType::PRIMITIVETYPE_Integer;
    if (!ResolvePrimitiveType(this, pt))
        {
        return ECObjectsStatus::DataTypeNotSupported;
        }

    if (pt != PrimitiveType::PRIMITIVETYPE_Double &&
        pt != PrimitiveType::PRIMITIVETYPE_Integer &&
        pt != PrimitiveType::PRIMITIVETYPE_Long)
        {
        return ECObjectsStatus::DataTypeNotSupported;
        }

    if (max.GetPrimitiveType() != pt)
        {
        return ECObjectsStatus::DataTypeNotSupported;
        }

    m_maximumValue = max;
    return ECObjectsStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
@bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool ECProperty::IsMaximumValueDefined() const
    {
    return !m_maximumValue.IsNull();
    }

/*---------------------------------------------------------------------------------**//**
@bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void ECProperty::ResetMaximumValue()
    {
    m_maximumValue.SetToNull();
    }

/*---------------------------------------------------------------------------------**//**
@bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus ECProperty::GetMaximumValue(ECValueR value) const
    {
    if (m_maximumValue.IsNull())
        return ECObjectsStatus::Error;

    value = m_maximumValue;
    return ECObjectsStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
@bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus ECProperty::SetMaximumLength(uint32_t max)
    {
    PrimitiveType pt = PrimitiveType::PRIMITIVETYPE_Integer;
    if (!ResolvePrimitiveType(this, pt))
        {
        return ECObjectsStatus::DataTypeNotSupported;
        }

    if (pt != PrimitiveType::PRIMITIVETYPE_String &&
        pt != PrimitiveType::PRIMITIVETYPE_Binary)
        {
        return ECObjectsStatus::DataTypeNotSupported;
        }

    m_maximumLength = max;
    return ECObjectsStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
@bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void ECProperty::_AdjustMinMaxAfterTypeChange()
    {
    PrimitiveType pt = PrimitiveType::PRIMITIVETYPE_Integer;
    if (!ResolvePrimitiveType(this, pt))
        {
        m_maximumValue.SetToNull();
        m_minimumValue.SetToNull();
        m_maximumLength = 0;
        return;
        }

    if (pt == PrimitiveType::PRIMITIVETYPE_Integer || pt == PrimitiveType::PRIMITIVETYPE_Long || pt == PrimitiveType::PRIMITIVETYPE_Double)
        {
        if (!m_maximumValue.ConvertToPrimitiveType(pt))
            m_maximumValue.SetToNull();

        if (!m_minimumValue.ConvertToPrimitiveType(pt))
            m_minimumValue.SetToNull();

        m_maximumLength = 0;
        return;
        }

    if (pt == PrimitiveType::PRIMITIVETYPE_String || pt == PrimitiveType::PRIMITIVETYPE_Binary)
        {
        m_maximumValue.SetToNull();
        m_minimumValue.SetToNull();
        return;
        }
    }

/*---------------------------------------------------------------------------------**//**
 @bsimethod                                                     
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8StringCR ECProperty::GetDisplayLabel () const
    {
    return GetClass().GetSchema().GetLocalizedStrings().GetPropertyDisplayLabel(this, GetInvariantDisplayLabel());
    }

/*---------------------------------------------------------------------------------**//**
 @bsimethod                                                     
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8StringCR ECProperty::GetInvariantDisplayLabel () const
    {
    return m_validatedName.GetDisplayLabel();
    }

/*---------------------------------------------------------------------------------**//**
 @bsimethod                                                     
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus ECProperty::SetDisplayLabel (Utf8StringCR displayLabel)
    {        
    m_validatedName.SetDisplayLabel (displayLabel.c_str());
    return ECObjectsStatus::Success;
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
    if (m_readOnly != readOnly)
        {
        m_readOnly = readOnly;
        InvalidateClassLayout();
        }

    return ECObjectsStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
 @bsimethod                                                     
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus ECProperty::SetIsReadOnly (Utf8CP isReadOnly)
    {        
    PRECONDITION (NULL != isReadOnly, ECObjectsStatus::PreconditionViolated);

    bool bReadOnly;
    ECObjectsStatus status = ECXml::ParseBooleanString (bReadOnly, isReadOnly);
    if (ECObjectsStatus::Success != status)
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
Utf8String ECProperty::GetTypeName () const
    {
    return this->_GetTypeName();
    }

/*---------------------------------------------------------------------------------**//**
 @bsimethod                                                     
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus ECProperty::SetTypeName (Utf8String typeName)
    {
    return this->_SetTypeName (typeName);
    }

/*---------------------------------------------------------------------------------**//**
 @bsimethod                                                     
+---------------+---------------+---------------+---------------+---------------+------*/
bool                    ECProperty::SetCalculatedPropertySpecification (IECInstanceP spec)
    {
    bool wasCalculated = IsCalculated();
    bool set = _SetCalculatedPropertySpecification (spec);
    if (set && wasCalculated != IsCalculated())
        InvalidateClassLayout();
    
    return set;
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
SchemaReadStatus ECProperty::_ReadXml (BeXmlNodeR propertyNode, ECSchemaReadContextR context, int ecXmlVersionMajor)
    {  
    Utf8String value;
    READ_REQUIRED_XML_ATTRIBUTE(propertyNode, PROPERTY_NAME_ATTRIBUTE, this, Name, propertyNode.GetName())

    // OPTIONAL attributes - If these attributes exist they MUST be valid    
    READ_OPTIONAL_XML_ATTRIBUTE(propertyNode, DESCRIPTION_ATTRIBUTE, this, Description)

    Utf8String minValue;
    if (propertyNode.GetAttributeStringValue(minValue, MINIMUM_VALUE_ATTRIBUTE) == BEXML_Success)
        {
        m_minimumValue.SetUtf8CP(minValue.c_str(), true); //TODO: cast type
        }

    Utf8String maxValue;
    if (propertyNode.GetAttributeStringValue(maxValue, MAXIMUM_VALUE_ATTRIBUTE) == BEXML_Success)
        {
        m_maximumValue.SetUtf8CP(maxValue.c_str(), true); //TODO: cast type
        }

    uint32_t maxLength;
    if (propertyNode.GetAttributeUInt32Value(maxLength, MAXIMUM_LENGTH_ATTRIBUTE) == BEXML_Success)
        {
        SetMaximumLength(maxLength);
        }

    READ_OPTIONAL_XML_ATTRIBUTE (propertyNode, DISPLAY_LABEL_ATTRIBUTE,       this, DisplayLabel)    

    // OPTIONAL attributes - If these attributes exist they do not need to be valid.  We will ignore any errors setting them and use default values.
    // NEEDSWORK This is due to the current implementation in managed ECObjects.  We should reconsider whether it is the correct behavior.
    ECObjectsStatus setterStatus;
    READ_OPTIONAL_XML_ATTRIBUTE_IGNORING_SET_ERRORS (propertyNode, READONLY_ATTRIBUTE,            this, IsReadOnly)

    if(CustomAttributeReadStatus::InvalidCustomAttributes == ReadCustomAttributes (propertyNode, context, GetClass().GetSchema(), ecXmlVersionMajor))
        {
        LOG.errorv("Failed to read property %s because one or more invalid custom attributes were applied to it.", this->GetName().c_str());
        return SchemaReadStatus::InvalidECSchemaXml;
        }
    return SchemaReadStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   
+---------------+---------------+---------------+---------------+---------------+------*/
SchemaWriteStatus ECProperty::_WriteXml (BeXmlWriterR xmlWriter, int ecXmlVersionMajor, int ecXmlVersionMinor)
    {
    return _WriteXml (xmlWriter, EC_PROPERTY_ELEMENT, ecXmlVersionMajor);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   
+---------------+---------------+---------------+---------------+---------------+------*/
SchemaWriteStatus ECProperty::_WriteXml (BeXmlWriterR xmlWriter, Utf8CP elementName, int ecXmlVersionMajor, bvector<bpair<Utf8CP, Utf8CP>>* additionalAttributes, bool writeType)
    {
    SchemaWriteStatus status = SchemaWriteStatus::Success;

    // If this property was created during supplementation as a local property on the class, then don't serialize it
    if (m_forSupplementation)
        return status;

    xmlWriter.WriteElementStart(elementName);

    xmlWriter.WriteAttribute(PROPERTY_NAME_ATTRIBUTE, this->GetName().c_str());

    if (writeType)
        {
        if (m_originalTypeName.size() > 0 && !m_originalTypeName.Contains("GeometryNET"))
            xmlWriter.WriteAttribute(TYPE_NAME_ATTRIBUTE, m_originalTypeName.c_str());
        else
            xmlWriter.WriteAttribute(TYPE_NAME_ATTRIBUTE, this->_GetTypeNameForXml(ecXmlVersionMajor).c_str());
        }

    xmlWriter.WriteAttribute(DESCRIPTION_ATTRIBUTE, this->GetInvariantDescription().c_str());
    if (GetIsDisplayLabelDefined())
        xmlWriter.WriteAttribute(DISPLAY_LABEL_ATTRIBUTE, this->GetInvariantDisplayLabel().c_str());
    if(IsReadOnlyFlagSet())
        xmlWriter.WriteAttribute(READONLY_ATTRIBUTE, true);
    if (IsMinimumValueDefined())
        {
        Utf8String minValue;
        if (m_minimumValue.ConvertPrimitiveToString(minValue))
            {
            xmlWriter.WriteAttribute(MINIMUM_VALUE_ATTRIBUTE, minValue.c_str());
            }
        }
        
    if (IsMaximumValueDefined())
        {
        Utf8String maxValue;
        if (m_maximumValue.ConvertPrimitiveToString(maxValue))
            {
            xmlWriter.WriteAttribute(MAXIMUM_VALUE_ATTRIBUTE, maxValue.c_str());
            }
        }

    if (IsMaximumLengthDefined())
        {
        xmlWriter.WriteAttribute(MAXIMUM_LENGTH_ATTRIBUTE, m_maximumLength);
        }
    
    if (nullptr != additionalAttributes && !additionalAttributes->empty())
        {
        for (auto& attribute : *additionalAttributes)
            xmlWriter.WriteAttribute(attribute.first, attribute.second);
        }

    WriteCustomAttributes (xmlWriter);
    xmlWriter.WriteElementEnd();

    return status;    
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   
+---------------+---------------+---------------+---------------+---------------+------*/
SchemaReadStatus PrimitiveECProperty::_ReadXml (BeXmlNodeR propertyNode, ECSchemaReadContextR context, int ecXmlVersionMajor)
    {  
    SchemaReadStatus status = T_Super::_ReadXml (propertyNode, context, ecXmlVersionMajor);
    if (status != SchemaReadStatus::Success)
        return status;
    
    // typeName is a required attribute.  If it is missing, an error will be returned.
    // For Primitive & Array properties we ignore parse errors and default to string.  Struct properties will require a resolvable typename.
    Utf8String value;
    if (BEXML_Success != propertyNode.GetAttributeStringValue (value, TYPE_NAME_ATTRIBUTE))
        {
        BeAssert (s_noAssert);
        LOG.errorv("Invalid ECSchemaXML: %s element %s must contain a %s attribute", propertyNode.GetName(), this->GetName().c_str(), TYPE_NAME_ATTRIBUTE);
        return SchemaReadStatus::InvalidECSchemaXml;
        }

    else if (ECObjectsStatus::ParseError == this->SetTypeName (value.c_str()))
        LOG.warningv ("Defaulting the type of ECProperty '%s' to '%s' in reaction to non-fatal parse error.", GetName().c_str(), GetTypeName().c_str());

    return ReadExtendedTypeAndKindOfQuantityXml(propertyNode, context);
    }

/*---------------------------------------------------------------------------------**//**
 * @bsimethod                                                    Stefan.Apfel   11/15
+---------------+---------------+---------------+---------------+---------------+------*/
SchemaWriteStatus PrimitiveECProperty::_WriteXml(BeXmlWriterR xmlWriter, int ecXmlVersionMajor, int ecXmlVersionMinor)
    {
    bvector<bpair<Utf8CP, Utf8CP>> attributes;
    auto status = WriteExtendedTypeAndKindOfQuantityXml(attributes, ecXmlVersionMajor);
    if (status != SchemaWriteStatus::Success)
        return status;

    return T_Super::_WriteXml(xmlWriter, EC_PROPERTY_ELEMENT, ecXmlVersionMajor, &attributes);
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
Utf8String PrimitiveECProperty::_GetTypeName () const
    {
    if(m_enumeration == nullptr)
        return ECXml::GetPrimitiveTypeName (m_primitiveType);

    return ECEnumeration::GetQualifiedEnumerationName(this->GetClass().GetSchema(), *m_enumeration);
    }

Utf8String PrimitiveECProperty::_GetTypeNameForXml(int ecXmlVersionMajor) const
    {
    if (ecXmlVersionMajor <= 2 && m_enumeration != nullptr)
        return m_enumeration->GetTypeName();

    return GetTypeName();
    }

ECObjectsStatus ResolveEnumerationType(ECEnumerationCP& enumeration, Utf8StringCR typeName, ECSchemaCR parentSchema)
    {
    // typeName may potentially be qualified so we must parse into an alias and short class name
    Utf8String alias;
    Utf8String enumName;
    ECObjectsStatus status = ECEnumeration::ParseEnumerationName(alias, enumName, typeName);
    if (ECObjectsStatus::Success != status)
        {
        LOG.warningv("Cannot resolve the type name '%s'.", typeName.c_str());
        return status;
        }

    ECSchemaCP resolvedSchema = parentSchema.GetSchemaByAliasP(alias);
    if (nullptr == resolvedSchema)
        {
        return ECObjectsStatus::SchemaNotFound;
        }

    ECEnumerationCP result = resolvedSchema->GetEnumerationCP(enumName.c_str());
    if (nullptr == result)
        {
        return ECObjectsStatus::DataTypeNotSupported;
        }

    enumeration = result;
    return ECObjectsStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus PrimitiveECProperty::_SetTypeName (Utf8StringCR typeName)
    {
    PrimitiveType primitiveType;
    ECObjectsStatus status = ECXml::ParsePrimitiveType (primitiveType, typeName);
    if (ECObjectsStatus::Success == status)
        {            
        if (PRIMITIVETYPE_IGeometry == primitiveType)
            m_originalTypeName = typeName; // Internally we treat everything as the common Bentley.Geometry.Common.IGeometry, but we need to preserve the actual type

        return SetType(primitiveType);
        }
    
    ECEnumerationCP enumeration;
    if (ECObjectsStatus::Success == ResolveEnumerationType(enumeration, typeName, this->GetClass().GetSchema()))
        {
        return SetType(*enumeration);
        }
        
    m_originalTypeName = typeName; // Remember this for when we serialize the ECSchema again, later.
    LOG.warningv("Unrecognized typeName '%s' found in '%s:%s.%s'. A type of 'string' will be used.",
                 typeName.c_str(),
                 GetClass().GetSchema().GetName().c_str(),
                 GetClass().GetName().c_str(),
                 GetName().c_str());
    return status;
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
ECEnumerationCP PrimitiveECProperty::GetEnumeration() const
    {
    return m_enumeration;
    }

/*---------------------------------------------------------------------------------**//**
 @bsimethod                                                     
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus PrimitiveECProperty::SetType (PrimitiveType primitiveType)
    {
    m_enumeration = nullptr;

    if (m_primitiveType != primitiveType)
        {
        m_primitiveType = primitiveType;        
        SetCachedTypeAdapter (NULL);
        _AdjustMinMaxAfterTypeChange();
        InvalidateClassLayout();
        }

    return ECObjectsStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
 @bsimethod                                                     
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus PrimitiveECProperty::SetType (ECEnumerationCR enumerationType)
    {        
    auto primitiveType = enumerationType.GetType();
    if (m_primitiveType != primitiveType)
        {
        m_primitiveType = primitiveType;        
        SetCachedTypeAdapter (NULL);
        InvalidateClassLayout();
        }

    m_enumeration = &enumerationType;

    return ECObjectsStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   08/12
+---------------+---------------+---------------+---------------+---------------+------*/
bool PrimitiveECProperty::_IsCalculated() const
    {
    return m_calculatedSpec.IsValid() || GetCustomAttribute ("Bentley_Standard_CustomAttributes", "CalculatedECPropertySpecification").IsValid();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Colin.Kerr                02/2016
//+---------------+---------------+---------------+---------------+---------------+------
Utf8StringCR ExtendedTypeECProperty::GetExtendedTypeName() const
    {
    if (!ExtendedTypeLocallyDefined())
        {
        ECPropertyCP baseProperty = GetBaseProperty();
        if (nullptr != baseProperty)
            {
            ExtendedTypeECPropertyCP baseExtended = baseProperty->GetAsExtendedTypeProperty();
            if (nullptr != baseExtended)
                return baseExtended->GetExtendedTypeName();
            }
        }

    return m_extendedTypeName;
    }

/*---------------------------------------------------------------------------------**//**
 @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus ExtendedTypeECProperty::SetExtendedTypeName(Utf8CP extendedTypeName)
    {
    m_extendedTypeName.clear();
    if (!Utf8String::IsNullOrEmpty(extendedTypeName))
        m_extendedTypeName.assign(extendedTypeName);

    return ECObjectsStatus::Success;
    }

ECObjectsStatus ResolveKindOfQuantityType(KindOfQuantityCP& kindOfQuantity, Utf8StringCR typeName, ECSchemaCR parentSchema)
    {
    // typeName may potentially be qualified so we must parse into an alias and short class name
    Utf8String alias;
    Utf8String kindOfQuantityName;
    if (ECObjectsStatus::Success != KindOfQuantity::ParseName(alias, kindOfQuantityName, typeName))
        {
        LOG.warningv("Cannot resolve the type name '%s'.", typeName.c_str());
        return ECObjectsStatus::ParseError;
        }

    ECSchemaCP resolvedSchema = parentSchema.GetSchemaByAliasP(alias);
    if (nullptr == resolvedSchema)
        {
        return ECObjectsStatus::SchemaNotFound;
        }

    auto result = resolvedSchema->GetKindOfQuantityCP(kindOfQuantityName.c_str());
    if (nullptr == result)
        {
        return ECObjectsStatus::DataTypeNotSupported;
        }

    kindOfQuantity = result;
    return ECObjectsStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
 @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
SchemaReadStatus ExtendedTypeECProperty::ReadExtendedTypeAndKindOfQuantityXml(BeXmlNodeR propertyNode, ECSchemaReadContextR schemaContext)
    {
    Utf8String value;
    if (BEXML_Success == propertyNode.GetAttributeStringValue(value, EXTENDED_TYPE_NAME_ATTRIBUTE))
        {
        this->SetExtendedTypeName(value.c_str());
        }

    if (BEXML_Success == propertyNode.GetAttributeStringValue(value, KIND_OF_QUANTITY_ATTRIBUTE))
        {
        //split
        KindOfQuantityCP kindOfQuantity;
        if(ResolveKindOfQuantityType(kindOfQuantity, value, GetClass().GetSchema()) != ECObjectsStatus::Success)
            {
            LOG.warningv("Could not resolve KindOfQuantity '%s' found on property '%s:%s.%s'.",
                         value.c_str(),
                         GetClass().GetSchema().GetName().c_str(),
                         GetClass().GetName().c_str(),
                         GetName().c_str());

            return SchemaReadStatus::InvalidECSchemaXml;
            }

        SetKindOfQuantity(kindOfQuantity);
        }

    _AdjustMinMaxAfterTypeChange();
    return SchemaReadStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
 @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
SchemaWriteStatus ExtendedTypeECProperty::WriteExtendedTypeAndKindOfQuantityXml(bvector<bpair<Utf8CP, Utf8CP>>& attributes, int ecXmlVersionMajor) const
    {
    if (this->ExtendedTypeLocallyDefined())
        {
        attributes.push_back(make_bpair(EXTENDED_TYPE_NAME_ATTRIBUTE, GetExtendedTypeName().c_str()));
        }

    if (ecXmlVersionMajor >= 3)
        {
        auto kindOfQuantity = GetKindOfQuantity();
        if (kindOfQuantity != nullptr)
            {
            attributes.push_back(make_bpair(KIND_OF_QUANTITY_ATTRIBUTE, kindOfQuantity->GetQualifiedName(this->GetClass().GetSchema()).c_str()));
            }
        }

    return SchemaWriteStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
 @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool ExtendedTypeECProperty::RemoveExtendedTypeName()
    {
    return ECObjectsStatus::Success == this->SetExtendedTypeName(nullptr);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   08/12
+---------------+---------------+---------------+---------------+---------------+------*/
CalculatedPropertySpecificationCP PrimitiveECProperty::_GetCalculatedPropertySpecification() const
    {
    // It's possible in pathological cases where schema specifies an invalid CalculatedECPropertySpecification we will attempt to evaluate
    // it every time this method is called, but do we need to cater to such cases by avoiding doing so?
    if (m_calculatedSpec.IsNull())
        m_calculatedSpec = CalculatedPropertySpecification::Create (*this, GetType());

    return m_calculatedSpec.get();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   08/12
+---------------+---------------+---------------+---------------+---------------+------*/
bool ArrayECProperty::_IsCalculated() const
    {
    if (ARRAYKIND_Primitive == GetKind())
        return m_calculatedSpec.IsValid() || GetCustomAttribute ("Bentley_Standard_CustomAttributes", "CalculatedECPropertySpecification").IsValid();
    else
        return false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   02/13
+---------------+---------------+---------------+---------------+---------------+------*/
static bool setCalculatedPropertySpecification (CalculatedPropertySpecificationPtr& spec, IECInstanceP attr, ECPropertyR ecprop, PrimitiveType primitiveType)
    {
    if (NULL == attr)
        {
        spec = NULL;
        return true;
        }
    else
        {
        IECInstancePtr oldAttr = ecprop.GetCustomAttribute ("Bentley_Standard_CustomAttributes", "CalculatedECPropertySpecification");
        ecprop.SetCustomAttribute (*attr);
        CalculatedPropertySpecificationPtr newSpec = CalculatedPropertySpecification::Create (ecprop, primitiveType);
        if (newSpec.IsValid())
            {
            spec = newSpec;
            return true;
            }
        else
            {
            // retain old specification
            if (oldAttr.IsValid())
                ecprop.SetCustomAttribute (*oldAttr);
            else
                ecprop.RemoveCustomAttribute ("Bentley_Standard_CustomAttributes", "CalculatedECPropertySpecification");

            return false;
            }
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   02/13
+---------------+---------------+---------------+---------------+---------------+------*/
bool PrimitiveECProperty::_SetCalculatedPropertySpecification (IECInstanceP attr)
    {
    return setCalculatedPropertySpecification (m_calculatedSpec, attr, *this, GetType());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   08/12
+---------------+---------------+---------------+---------------+---------------+------*/
CalculatedPropertySpecificationCP ArrayECProperty::_GetCalculatedPropertySpecification() const
    {
    if (ARRAYKIND_Primitive == GetKind() && m_calculatedSpec.IsNull())
        m_calculatedSpec = CalculatedPropertySpecification::Create (*this, GetPrimitiveElementType());

    return m_calculatedSpec.get();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   02/13
+---------------+---------------+---------------+---------------+---------------+------*/
bool ArrayECProperty::_SetCalculatedPropertySpecification (IECInstanceP attr)
    {
    if (ARRAYKIND_Primitive == GetKind())
        return setCalculatedPropertySpecification (m_calculatedSpec, attr, *this, GetPrimitiveElementType());
    else
        return false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   
+---------------+---------------+---------------+---------------+---------------+------*/
SchemaReadStatus StructECProperty::_ReadXml (BeXmlNodeR propertyNode, ECSchemaReadContextR context, int ecXmlVersionMajor)
    {  
    SchemaReadStatus status = T_Super::_ReadXml (propertyNode, context, ecXmlVersionMajor);
    if (status != SchemaReadStatus::Success)
        return status;

    // For Primitive & Array properties we ignore parse errors and default to string.  Struct properties will require a resolvable typename.
    Utf8String value;  // needed for macro.
    READ_REQUIRED_XML_ATTRIBUTE (propertyNode, TYPE_NAME_ATTRIBUTE, this, TypeName, propertyNode.GetName())        

    return SchemaReadStatus::Success;
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   
+---------------+---------------+---------------+---------------+---------------+------*/
SchemaWriteStatus StructECProperty::_WriteXml (BeXmlWriterR xmlWriter, int ecXmlVersionMajor, int ecXmlVersionMinor)
    {
    return T_Super::_WriteXml (xmlWriter, EC_STRUCTPROPERTY_ELEMENT, ecXmlVersionMajor);
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
Utf8String StructECProperty::_GetTypeName () const
    {
    if (!EXPECTED_CONDITION (NULL != m_structType))
        return EMPTY_STRING;
    return ECClass::GetQualifiedClassName (this->GetClass().GetSchema(), *m_structType);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus ResolveStructType (ECStructClassCP& structClass, Utf8StringCR typeName, ECClassCR containingClass, bool doLogging = true)
    {
    // typeName may potentially be qualified so we must parse into an alias and short class name
    Utf8String alias;
    Utf8String className;
    ECObjectsStatus status = ECClass::ParseClassName (alias, className, typeName);
    if (ECObjectsStatus::Success != status)
        {
        if (doLogging)
            LOG.errorv("Cannot resolve the type name '%s' as a struct type because the typeName could not be parsed.", typeName.c_str());
        return status;
        }
    
    ECSchemaCP resolvedSchema = containingClass.GetSchema().GetSchemaByAliasP (alias);
    if (NULL == resolvedSchema)
        {
        if (doLogging)
            LOG.errorv("Cannot resolve the type name '%s' as a struct type because the alias '%s' can not be resolved to the primary or a referenced schema.",
                typeName.c_str(), alias.c_str());
        return ECObjectsStatus::SchemaNotFound;
        }

    ECClassCP ecClass = resolvedSchema->GetClassCP (className.c_str());
    if (NULL == ecClass)
        {
        if (doLogging)
            LOG.errorv("Cannot resolve the type name '%s' as a struct type because ECClass '%s' does not exist in the schema '%s'.",
                typeName.c_str(), className.c_str(), resolvedSchema->GetName().c_str());
        return ECObjectsStatus::ClassNotFound;
        }

    structClass = ecClass->GetStructClassCP();
    if (NULL == ecClass)
        {
        if (doLogging)
            LOG.errorv("ECClass '%s' does exists in the schema '%s' but is not of type ECStructClass.",
                className.c_str(), resolvedSchema->GetName().c_str());
        return ECObjectsStatus::ClassNotFound;
        }

    return ECObjectsStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus StructECProperty::_SetTypeName (Utf8StringCR typeName)
    {
    ECStructClassCP structClass;
    ECObjectsStatus status = ResolveStructType (structClass, typeName, this->GetClass());
    if (ECObjectsStatus::Success != status)
        {
        LOG.errorv ("Failed to set the type name of ECStructProperty '%s' to '%s' because the typeName could not be parsed into a resolvable ECClass.", this->GetName().c_str(), typeName.c_str());        
        return status;
        }
    else
        return SetType (*structClass);
    }

/*---------------------------------------------------------------------------------**//**
 @bsimethod                                                     
+---------------+---------------+---------------+---------------+---------------+------*/
ECStructClassCR StructECProperty::GetType () const
    {        
    DEBUG_EXPECT (NULL != m_structType);
    return *m_structType;
    }

/*---------------------------------------------------------------------------------**//**
 @bsimethod                                                     
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus StructECProperty::SetType (ECStructClassCR structType)
    {            
    if (&(structType.GetSchema()) != &(this->GetClass().GetSchema()))
        {
        if (!ECSchema::IsSchemaReferenced(this->GetClass().GetSchema(), structType.GetSchema()))
            return ECObjectsStatus::SchemaNotFound;
        }
    
    if (m_structType != &structType)
        {
        m_structType = &structType;
        SetCachedTypeAdapter (NULL);
        InvalidateClassLayout();
        }

    return ECObjectsStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   
+---------------+---------------+---------------+---------------+---------------+------*/
SchemaReadStatus ArrayECProperty::_ReadXml (BeXmlNodeR propertyNode, ECSchemaReadContextR context, int ecXmlVersionMajor)
    {  
    SchemaReadStatus status = T_Super::_ReadXml (propertyNode, context, ecXmlVersionMajor);
    if (status != SchemaReadStatus::Success)
        return status;

    // OPTIONAL attributes - If these attributes exist they do not need to be valid.  We will ignore any errors setting them and use default values.
    // NEEDSWORK This is due to the current implementation in managed ECObjects.  We should reconsider whether it is the correct behavior.
    ECObjectsStatus setterStatus;   // needed for macro.
    Utf8String         value;          // needed for macro.
    READ_OPTIONAL_XML_ATTRIBUTE_IGNORING_SET_ERRORS (propertyNode, MIN_OCCURS_ATTRIBUTE, this, MinOccurs)    
    READ_OPTIONAL_XML_ATTRIBUTE_IGNORING_SET_ERRORS (propertyNode, MAX_OCCURS_ATTRIBUTE, this, MaxOccurs)

    // For Primitive & Array properties we ignore parse errors and default to string.  Struct properties will require a resolvable typename.
    READ_REQUIRED_XML_ATTRIBUTE_IGNORING_SET_ERRORS (propertyNode, TYPE_NAME_ATTRIBUTE, this, TypeName, propertyNode.GetName())  

    if (ECObjectsStatus::Success != setterStatus)
        {
        LOG.warningv ("Defaulting the type of ECProperty '%s' to '%s' in reaction to non-fatal parse error.", GetName().c_str(), GetTypeName().c_str());
        return SchemaReadStatus::Success;
        }

    return ReadExtendedTypeAndKindOfQuantityXml(propertyNode, context);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   
+---------------+---------------+---------------+---------------+---------------+------*/
SchemaWriteStatus ArrayECProperty::_WriteXml (BeXmlWriterR xmlWriter, int ecXmlVersionMajor, int ecXmlVersionMinor)
    {
    bvector<bpair<Utf8CP, Utf8CP>> additionalAttributes;
    char    valueString[128];
    BeStringUtilities::Snprintf (valueString, _countof (valueString), "%u", m_minOccurs);

    additionalAttributes.push_back(make_bpair(MIN_OCCURS_ATTRIBUTE, valueString));
    if (m_maxOccurs != UINT_MAX)
        {
        BeStringUtilities::Snprintf (valueString, _countof (valueString), "%u", m_maxOccurs);
        additionalAttributes.push_back(make_bpair(MAX_OCCURS_ATTRIBUTE, valueString));
        }
    else
        {
        additionalAttributes.push_back(make_bpair(MAX_OCCURS_ATTRIBUTE, "unbounded"));
        }

    Utf8CP elementName = EC_ARRAYPROPERTY_ELEMENT;
    if (m_arrayKind == ARRAYKIND_Struct)
        {
        if (2 == ecXmlVersionMajor)
            additionalAttributes.push_back(make_bpair(IS_STRUCT_ATTRIBUTE, "true"));
        else
            elementName = EC_STRUCTARRAYPROPERTY_ELEMENT;
        }

    auto status = WriteExtendedTypeAndKindOfQuantityXml(additionalAttributes, ecXmlVersionMajor);
    if (status != SchemaWriteStatus::Success)
        return status;

    status = T_Super::_WriteXml (xmlWriter, elementName, ecXmlVersionMajor, &additionalAttributes);
    if (status != SchemaWriteStatus::Success || m_forSupplementation) // If this property was created during supplementation, don't serialize it
        return status;

    return status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Carole.MacDonald                05/2010
+---------------+---------------+---------------+---------------+---------------+------*/
bool ArrayECProperty::_CanOverride (ECPropertyCR baseProperty) const
    {
    // This used to always compare GetTypeName(). Type names for struct arrays include the alias as defined in the referencing schema. That is weird and easily breaks if:
    //  -Base property is defined in same schema as the struct class (cannot be worked around), or
    //  -Base property's schema declares different alias for struct class's schema than the overriding property's schema (dumb workaround: make them use the same alias).
    // Instead, compare the full-qualified class name.
    auto baseArray = baseProperty.GetAsArrayProperty();
    if (nullptr == baseArray || baseArray->GetKind() != GetKind())
        {
        // Apparently this is a thing...overriding a primitive property with a primitive array of same type.
        if (nullptr == baseArray && GetKind() == ARRAYKIND_Primitive)
            {
            auto basePrim = baseProperty.GetAsPrimitiveProperty();
            return nullptr != basePrim && basePrim->GetType() == GetPrimitiveElementType();
            }
        else
            return false;
        }
    else
        {
        Utf8String typeName = GetTypeName();
        return typeName == EMPTY_STRING || typeName == baseProperty.GetTypeName();
        }
    }
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String ArrayECProperty::_GetTypeName () const
    {    
    return ECXml::GetPrimitiveTypeName (m_primitiveType);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus ArrayECProperty::_SetTypeName (Utf8StringCR typeName)
    {
    PrimitiveType primitiveType;
    ECObjectsStatus status = ECXml::ParsePrimitiveType (primitiveType, typeName);
    if (ECObjectsStatus::Success == status)
        return SetPrimitiveElementType (primitiveType);
    
    m_originalTypeName = typeName;
    LOG.warningv ("TypeName '%s' of '%s.%s.%s' was not recognized. We will use 'string' instead.",
                                    typeName.c_str(),
                                    GetClass().GetSchema().GetName().c_str(),
                                    GetClass().GetName().c_str(),
                                    GetName().c_str() );
    return ECObjectsStatus::ParseError;
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

    SetCachedTypeAdapter (NULL);
    SetCachedMemberTypeAdapter (NULL);
    _AdjustMinMaxAfterTypeChange();
    InvalidateClassLayout();
 
    return ECObjectsStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
 @bsimethod                                                     
+---------------+---------------+---------------+---------------+---------------+------*/
uint32_t ArrayECProperty::GetMinOccurs () const
    {
    return m_minOccurs;
    }

/*---------------------------------------------------------------------------------**//**
 @bsimethod                                                     
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus ArrayECProperty::SetMinOccurs (uint32_t minOccurs)
    {
    PRECONDITION (minOccurs <= m_maxOccurs, ECObjectsStatus::PreconditionViolated);
    if (m_minOccurs != minOccurs)
        {
        m_minOccurs = minOccurs;
        InvalidateClassLayout();
        }

    return ECObjectsStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
 @bsimethod                                                     
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus ArrayECProperty::SetMinOccurs (Utf8StringCR minOccurs)
    {    
    uint32_t iMinOccurs;
    int count = BE_STRING_UTILITIES_UTF8_SSCANF (minOccurs.c_str(), "%u", &iMinOccurs);
    if (count != 1)
        {
        LOG.errorv ("Failed to set MinOccurs of ECProperty '%s' to '%s' because the value could not be parsed.  It must be a valid unsigned integer.",
                 this->GetName().c_str(), minOccurs.c_str());        
        return ECObjectsStatus::ParseError;
        }    
    SetMinOccurs (iMinOccurs);
    return ECObjectsStatus::Success;
    }


/*---------------------------------------------------------------------------------**//**
 @bsimethod                                                     
+---------------+---------------+---------------+---------------+---------------+------*/
uint32_t ArrayECProperty::GetMaxOccurs () const
    {
    return /* m_maxOccurs; */ UINT_MAX; // D-106653
    }

/*---------------------------------------------------------------------------------**//**
 @bsimethod                                                     
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus ArrayECProperty::SetMaxOccurs (uint32_t maxOccurs)
    {
    // D-106653: Note: We store maxOccurs as read from schema, but we do not enforce it - callers can increase the size of the array beyond maxOccurs and array elements are always stored in variable-size section
    // of ECD buffer.
    // ###TODO: should we allocate space for maxOccurs elements when initializing ECD buffer? Test code expects this, but does real code?
    PRECONDITION (maxOccurs >= m_minOccurs, ECObjectsStatus::PreconditionViolated);
    if (m_maxOccurs != maxOccurs)
        {
        m_maxOccurs = maxOccurs;
        InvalidateClassLayout();
        }

    return ECObjectsStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
 @bsimethod                                                     
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus ArrayECProperty::SetMaxOccurs (Utf8StringCR maxOccurs)
    {    
    uint32_t iMaxOccurs;
    int count = BE_STRING_UTILITIES_UTF8_SSCANF (maxOccurs.c_str(), "%u", &iMaxOccurs);
    if (count != 1)
        {
        if (0 == strcmp (maxOccurs.c_str(), ECXML_UNBOUNDED))
            iMaxOccurs = UINT_MAX;
        else
            {
            LOG.errorv ("Failed to set MaxOccurs of ECProperty '%s' to '%s' because the value could not be parsed.  It must be a valid unsigned integer or the string 'unbounded'.",
                     this->GetName().c_str(), maxOccurs.c_str());        
            return ECObjectsStatus::ParseError;
            }
        }
    SetMaxOccurs (iMaxOccurs);
    return ECObjectsStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
@bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool ECProperty::HasExtendedType () const
    {
    return this->_HasExtendedType();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            10/2015
//---------------+---------------+---------------+---------------+---------------+-------
bool StructArrayECProperty::_CanOverride (ECPropertyCR baseProperty) const
    {
    // This used to always compare GetTypeName(). Type names for struct arrays include the alias as defined in the referencing schema. That is weird and easily breaks if:
    //  -Base property is defined in same schema as the struct class (cannot be worked around), or
    //  -Base property's schema declares different alias for struct class's schema than the overriding property's schema (dumb workaround: make them use the same alias).
    // Instead, compare the full-qualified class name.
    auto baseArray = baseProperty.GetAsStructArrayProperty();
    if (nullptr == baseArray || baseArray->GetKind() != GetKind())
        {
        return false;
        }
    else
        {
        auto myType = GetStructElementType(), baseType = baseArray->GetStructElementType();
        return nullptr != myType && nullptr != baseType && 0 == strcmp (myType->GetFullName(), baseType->GetFullName());
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            10/2015
//---------------+---------------+---------------+---------------+---------------+-------
ECObjectsStatus StructArrayECProperty::_SetTypeName(Utf8StringCR typeName)
    {
    ECStructClassCP structClass;
    ECObjectsStatus status = ResolveStructType(structClass, typeName, this->GetClass());
    if (ECObjectsStatus::Success == status)
        return SetStructElementType(structClass);

    m_originalTypeName = typeName;
    LOG.warningv("TypeName '%s' of '%s.%s.%s' was not recognized. We will use 'string' instead.",
                 typeName.c_str(),
                 GetClass().GetSchema().GetName().c_str(),
                 GetClass().GetName().c_str(),
                 GetName().c_str());
    return ECObjectsStatus::ParseError;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            10/2015
//---------------+---------------+---------------+---------------+---------------+-------
Utf8String StructArrayECProperty::_GetTypeName() const
    {
    return ECClass::GetQualifiedClassName(this->GetClass().GetSchema(), *m_structType);
    }

/*---------------------------------------------------------------------------------**//**
 @bsimethod                                                     
+---------------+---------------+---------------+---------------+---------------+------*/
ECStructClassCP StructArrayECProperty::GetStructElementType () const
    {
    return m_structType;
    }

/*---------------------------------------------------------------------------------**//**
 @bsimethod                                                     
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus StructArrayECProperty::SetStructElementType (ECStructClassCP structType)
    {        
    PRECONDITION (NULL != structType, ECObjectsStatus::PreconditionViolated);

    if (&(structType->GetSchema()) != &(this->GetClass().GetSchema()))
        {
        if (!ECSchema::IsSchemaReferenced(this->GetClass().GetSchema(), structType->GetSchema()))
            return ECObjectsStatus::SchemaNotFound;
        }

    m_arrayKind = ARRAYKIND_Struct;
    m_structType = structType;
 
    SetCachedTypeAdapter (NULL);
    SetCachedMemberTypeAdapter (NULL);
    InvalidateClassLayout();

    return ECObjectsStatus::Success;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Colin.Kerr                  12/2015
//---------------+---------------+---------------+---------------+---------------+-------
ECObjectsStatus NavigationECProperty::SetRelationshipClassName(Utf8CP relationshipName)
    {
    PRECONDITION(nullptr != relationshipName, ECObjectsStatus::PreconditionViolated);

    Utf8String alias;
    Utf8String relClassName;
    ECObjectsStatus status = ECClass::ParseClassName(alias, relClassName, relationshipName);
    if (ECObjectsStatus::Success != status)
        {
        LOG.errorv("Cannot resolve the relationship class name '%s' as a relationship class because the name could not be parsed.", relationshipName);
        return status;
        }

    ECSchemaCP resolvedSchema = GetClass().GetSchema().GetSchemaByAliasP(alias);
    if (nullptr == resolvedSchema)
        {
        LOG.errorv("Cannot resolve the relationship class name '%s' as a relationship class because the alias '%s' cannot be resolved to the primary or a referenced schema.",
                     relationshipName, alias.c_str());
        return ECObjectsStatus::SchemaNotFound;
        }
    
    ECClassCP ecClass = resolvedSchema->GetClassCP(relClassName.c_str());
    if (nullptr == ecClass)
        {
        LOG.errorv("Cannot resolve the relationship class name '%s' as a relationship class because the ECClass '%s' does not exist in the schema '%s'.",
                     relationshipName, relClassName.c_str(), resolvedSchema->GetName().c_str());
        return ECObjectsStatus::ClassNotFound;
        }
    
    m_relationshipClass = ecClass->GetRelationshipClassCP();
    if (nullptr == m_relationshipClass)
        {
        LOG.errorv("ECClass '%s' exists in the schema '%s' but is not an ECRelationshipClass.",
                     relClassName.c_str(), resolvedSchema->GetName().c_str());
        return ECObjectsStatus::ECClassNotSupported;
        }
    
    m_valueKind = ValueKind::VALUEKIND_Uninitialized;

    return ECObjectsStatus::Success;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Colin.Kerr                  12/2015
//---------------+---------------+---------------+---------------+---------------+-------
ECObjectsStatus NavigationECProperty::SetDirection(Utf8CP direction)
    {
    PRECONDITION(nullptr != direction, ECObjectsStatus::PreconditionViolated);

    ECObjectsStatus status = ECXml::ParseDirectionString(m_direction, direction);
    if (ECObjectsStatus::Success != status)
        LOG.errorv("Failed to parse the ECRelatedInstanceDirection string '%s' for NavigationECProperty '%s'.", direction, GetName().c_str());

    m_valueKind = ValueKind::VALUEKIND_Uninitialized;

    return status;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Colin.Kerr                  12/2015
//---------------+---------------+---------------+---------------+---------------+-------
Utf8String NavigationECProperty::GetRelationshipClassName() const
    {
    if (!EXPECTED_CONDITION(nullptr != m_relationshipClass))
        return EMPTY_STRING;
    return ECClass::GetQualifiedClassName(this->GetClass().GetSchema(), *m_relationshipClass);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Colin.Kerr                  12/2015
//---------------+---------------+---------------+---------------+---------------+-------
bool NavigationECProperty::Verify()
    {
    if (IsVerified())
        return true;

    if (nullptr == m_relationshipClass)
        return false;

    ECRelationshipConstraintCP thisConstraint;
    ECRelationshipConstraintCP thatConstraint;
    if (ECRelatedInstanceDirection::Forward == m_direction)
        {
        thisConstraint = &m_relationshipClass->GetSource();
        thatConstraint = &m_relationshipClass->GetTarget();
        }
    else
        {
        thisConstraint = &m_relationshipClass->GetTarget();
        thatConstraint = &m_relationshipClass->GetSource();
        }

    bool supportsClass = thisConstraint->SupportsClass(GetClass());
    if (!supportsClass)
        {
        m_valueKind = ValueKind::VALUEKIND_Uninitialized;
        return false;
        }

    if (1 == thatConstraint->GetMultiplicity().GetUpperLimit())
        m_valueKind = ValueKind::VALUEKIND_Primitive;
    else
        m_valueKind = ValueKind::VALUEKIND_Array;

    return true;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Colin.Kerr                  12/2015
//---------------+---------------+---------------+---------------+---------------+-------
SchemaReadStatus NavigationECProperty::_ReadXml(BeXmlNodeR propertyNode, ECSchemaReadContextR schemaContext, int ecXmlVersionMajor)
    {
    SchemaReadStatus status = T_Super::_ReadXml(propertyNode, schemaContext, ecXmlVersionMajor);
    if (status != SchemaReadStatus::Success)
        return status;

    // relationshipName and direction are required properties
    Utf8String value; // neede for macro.
    READ_REQUIRED_XML_ATTRIBUTE (propertyNode, RELATIONSHIP_NAME_ATTRIBUTE, this, RelationshipClassName, propertyNode.GetName())

    READ_REQUIRED_XML_ATTRIBUTE (propertyNode, DIRECTION_ATTRIBUTE, this, Direction, propertyNode.GetName())

    return SchemaReadStatus::Success;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Colin.Kerr                  12/2015
//---------------+---------------+---------------+---------------+---------------+-------
SchemaWriteStatus NavigationECProperty::_WriteXml(BeXmlWriterR xmlWriter, int ecXmlVersionMajor, int ecXmlVersionMinor)
    {
    if (2 == ecXmlVersionMajor)
        return T_Super::_WriteXml(xmlWriter, EC_PROPERTY_ELEMENT, ecXmlVersionMajor);


    bvector<bpair<Utf8CP, Utf8CP>> additionalAttributes;
    additionalAttributes.push_back(make_bpair(RELATIONSHIP_NAME_ATTRIBUTE, GetRelationshipClassName().c_str()));
    additionalAttributes.push_back(make_bpair(DIRECTION_ATTRIBUTE, ECXml::DirectionToString(m_direction)));

    return T_Super::_WriteXml(xmlWriter, EC_NAVIGATIONPROPERTY_ELEMENT, ecXmlVersionMajor, &additionalAttributes, false);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Colin.Kerr                  12/2015
//---------------+---------------+---------------+---------------+---------------+-------
Utf8String NavigationECProperty::_GetTypeName() const { return ECXml::GetPrimitiveTypeName(m_type); }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Colin.Kerr                  12/2015
//---------------+---------------+---------------+---------------+---------------+-------
bool NavigationECProperty::_CanOverride(ECPropertyCR baseProperty) const
    {
    NavigationECPropertyCP baseNavProperty = baseProperty.GetAsNavigationProperty();
    if (nullptr == baseNavProperty)
        return false;

    ECRelatedInstanceDirection baseDirection = baseNavProperty->GetDirection();
    if (GetDirection() != baseDirection)
        return false;

    // Following the example of StructECProperty we will allow override if the current relationship has not het been set.
    if (nullptr == m_relationshipClass)
        return true;

    ECRelationshipClassCP baseRelClass = baseNavProperty->GetRelationshipClass();

    return m_relationshipClass->Is(baseRelClass);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Colin.Kerr                  12/2015
//---------------+---------------+---------------+---------------+---------------+-------
ECObjectsStatus NavigationECProperty::SetRelationshipClass(ECRelationshipClassCR relClass, ECRelatedInstanceDirection direction, bool verify)
    {
    if (&(relClass.GetSchema()) != &(this->GetClass().GetSchema()))
        {
        if (!ECSchema::IsSchemaReferenced(this->GetClass().GetSchema(), relClass.GetSchema()))
            return ECObjectsStatus::SchemaNotFound;
        }

    m_relationshipClass = &relClass;
    m_direction = direction;
    if (verify && !Verify())
        return ECObjectsStatus::RelationshipConstraintsNotCompatible;

    SetCachedTypeAdapter(nullptr);
    InvalidateClassLayout();

    return ECObjectsStatus::Success;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Colin.Kerr                  12/2015
//---------------+---------------+---------------+---------------+---------------+-------
ECObjectsStatus NavigationECProperty::SetType(PrimitiveType type)
    {
    if (m_type != type)
        {
        m_type = type;
        SetCachedTypeAdapter(nullptr);
        InvalidateClassLayout();
        }

    return ECObjectsStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
 @bsimethod                                                     
+---------------+---------------+---------------+---------------+---------------+------*/
bool IECTypeAdapter::HasStandardValues() const                                                      { return _HasStandardValues(); }
bool IECTypeAdapter::CanConvertFromString (IECTypeAdapterContextCR context) const                   { return _CanConvertFromString (context); }
bool IECTypeAdapter::CanConvertToString (IECTypeAdapterContextCR context) const                     { return _CanConvertToString (context); }
bool IECTypeAdapter::IsStruct() const                                                               { return _IsStruct(); }
bool IECTypeAdapter::AllowExpandMembers() const
    {
    // TFS#38705
    return _AllowExpandMembers();
    }
bool IECTypeAdapter::IsTreatedAsString() const                                                          { return _IsTreatedAsString(); }
IECInstancePtr IECTypeAdapter::CreateDefaultFormatter (bool includeAllValues, bool forDwg) const        { return _CreateDefaultFormatter (includeAllValues, forDwg); }
IECInstancePtr IECTypeAdapter::CondenseFormatterForSerialization (IECInstanceCR formatter) const        { return _CondenseFormatterForSerialization (formatter); }
IECInstancePtr IECTypeAdapter::PopulateDefaultFormatterProperties (IECInstanceCR formatter) const       { return _PopulateDefaultFormatterProperties (formatter); }
bool IECTypeAdapter::ConvertFromString (ECValueR v, Utf8CP str, IECTypeAdapterContextCR context) const  { return _ConvertFromString (v, str, context); }
bool IECTypeAdapter::ConvertToExpressionType (ECValueR v, IECTypeAdapterContextCR context) const        { return _ConvertToExpressionType (v, context); }
bool IECTypeAdapter::ConvertFromExpressionType (ECValueR v, IECTypeAdapterContextCR context) const      { return _ConvertFromExpressionType (v, context); }
bool IECTypeAdapter::RequiresExpressionTypeConversion (EvaluationOptions evalOptions) const             { return _RequiresExpressionTypeConversion (evalOptions); }
bool IECTypeAdapter::GetDisplayType (PrimitiveType& type) const                                         { return _GetDisplayType (type); }
bool IECTypeAdapter::ConvertToString (Utf8StringR str, ECValueCR v, IECTypeAdapterContextCR context, IECInstanceCP opts) const { return _ConvertToString (str, v, context, opts); }
bool IECTypeAdapter::ConvertToDisplayType (ECValueR v, IECTypeAdapterContextCR context, IECInstanceCP opts) const           { return _ConvertToDisplayType (v, context, opts); }
bool IECTypeAdapter::GetPropertyNotSetValue (ECValueR v) const                                          { return _GetPropertyNotSetValue (v); }
bool IECTypeAdapter::SupportsUnits() const                                                              { return _SupportsUnits(); }
bool IECTypeAdapter::GetUnits (UnitSpecR unit, IECTypeAdapterContextCR context) const                   { return _GetUnits (unit, context); }
bool IECTypeAdapter::IsOrdinalType () const                                                             { return _IsOrdinalType (); }
bool IECTypeAdapter::GetPlaceholderValue (ECValueR v, IECTypeAdapterContextCR context) const            { return _GetPlaceholderValue (v, context); }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   06/14
+---------------+---------------+---------------+---------------+---------------+------*/
ECPropertyCP            IECTypeAdapterContext::GetProperty() const                                      { return _GetProperty(); }
uint32_t                IECTypeAdapterContext::GetComponentIndex() const                                { return _GetComponentIndex(); }
bool                    IECTypeAdapterContext::Is3d() const                                             { return _Is3d(); }
IECInstanceCP           IECTypeAdapterContext::GetECInstance() const                                    { return _GetECInstance(); }
ECObjectsStatus         IECTypeAdapterContext::GetInstanceValue (ECValueR v, Utf8CP accessor, uint32_t arrayIndex) const { return _GetInstanceValue (v, accessor, arrayIndex); }
EvaluationOptions       IECTypeAdapterContext::GetEvaluationOptions () const                            { return _GetEvaluationOptions (); };
EvaluationOptions       IECTypeAdapterContext::_GetEvaluationOptions () const                           { return m_evalOptions; };
void                    IECTypeAdapterContext::SetEvaluationOptions (EvaluationOptions evalOptions)     { return _SetEvaluationOptions (evalOptions); };
void                    IECTypeAdapterContext::_SetEvaluationOptions (EvaluationOptions evalOptions)    { m_evalOptions = evalOptions; };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   03/13
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus IECTypeAdapterContext::_GetInstanceValue (ECValueR v, Utf8CP accessor, uint32_t arrayIndex) const
    {
    IECInstanceCP instance = GetECInstance();
    if (NULL != instance)
        return -1 != arrayIndex ? instance->GetValue (v, accessor, arrayIndex) : instance->GetValue (v, accessor);
    else
        return ECObjectsStatus::OperationNotSupported;
    }


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

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   09/15
+---------------+---------------+---------------+---------------+---------------+------*/
void ECProperty::InvalidateClassLayout()
    {
    // TFS#290587: When a property is modified in a way that affects the ClassLayout, must
    // invalidate the containing class's default standalone enabler
    m_class.InvalidateDefaultStandaloneEnabler();
    }

END_BENTLEY_ECOBJECT_NAMESPACE
