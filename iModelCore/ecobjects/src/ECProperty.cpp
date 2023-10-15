/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/

#include "ECObjectsPch.h"

BEGIN_BENTLEY_ECOBJECT_NAMESPACE

// If you are developing schemas, particularly when editing them by hand, you want to have this variable set to false so you get the asserts to help you figure out what is going wrong.
// Test programs generally want to get error status back and not BeAssert, so they call ECSchema::AssertOnXmlError (false);
static  bool        s_noAssert = false;

SchemaReadStatus ReadTypeNameWithPruning(pugi::xml_node node, ECPropertyP prop, ECSchemaReadContextR readContext, bool ignoreParseErrors = false)
    {
    auto typeAttr = node.attribute(TYPE_NAME_ATTRIBUTE);
    if (!typeAttr)
        {
        LOG.errorv ("Invalid ECSchemaXML: %s element must contain a %s attribute", node.name(), TYPE_NAME_ATTRIBUTE);
        return SchemaReadStatus::InvalidECSchemaXml;
        }
    Utf8String typeName = typeAttr.as_string();

    auto setTypeStatus = prop->SetTypeName(typeName);
    if (setTypeStatus == ECObjectsStatus::Success)
        return SchemaReadStatus::Success;

    Utf8String alias;
    Utf8String className;
    if (ECObjectsStatus::Success != ECClass::ParseClassName(alias, className, typeName))
        {
        LOG.errorv ("Invalid ECSchemaXML: The Property '%s.%s' has a type name '%s' that can not be parsed.",
            prop->GetClass().GetFullName(), prop->GetName().c_str(), typeName.c_str());
        return SchemaReadStatus::InvalidECSchemaXml;
        }

    if (readContext.IsAliasToPrune(prop->GetClass().GetSchema().GetName(), alias))
        {
        LOG.infov("Unresolved typeName '%s' on property '%s.%s' came from a schema that was pruned, '%s'. pruning instead of error.",
                    typeName.c_str(), prop->GetClass().GetFullName(), prop->GetName().c_str(), prop->GetClass().GetSchema().GetName().c_str());
        return SchemaReadStatus::PruneItem;
        }

    // must come after prune check to make sure we don't default to string properties that should be pruned
    if (setTypeStatus == ECObjectsStatus::ParseError && ignoreParseErrors)
        {
        LOG.warningv ("Defaulting the type of ECProperty '%s' to '%s' in reaction to non-fatal parse error.", prop->GetName().c_str(), prop->GetTypeName().c_str());
        return SchemaReadStatus::Success;
        }

    if (setTypeStatus == ECObjectsStatus::SchemaNotFound)
        {
        LOG.errorv("Cannot resolve the type name '%s.%s' as a struct type because the alias '%s' can not be resolved to the primary or a referenced schema.",
            alias.c_str(), className.c_str(), alias.c_str());
        }

    return SchemaReadStatus::InvalidECSchemaXml;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void ECProperty::SetErrorHandling (bool doAssert)
    {
    s_noAssert = !doAssert;
    }

/*---------------------------------------------------------------------------------**//**
 @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ECProperty::ECProperty (ECClassCR ecClass) : m_class(ecClass), m_baseProperty(nullptr),
                                                m_kindOfQuantity(nullptr),
                                                m_propertyCategory(nullptr),
                                                m_priority(0), m_flags()
    {}

/*---------------------------------------------------------------------------------**//**
 @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ECProperty::~ECProperty () {}

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ECPropertyCP ECProperty::GetBaseProperty () const
    {
    return m_baseProperty;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
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
 @bsimethod
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
    if (!m_validatedName.SetValidName(name.c_str(), m_class.GetSchema().OriginalECXmlVersionLessThan(ECVersion::V3_1)))
        return ECObjectsStatus::InvalidName;

    return ECObjectsStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
 @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8StringCR ECProperty::GetDescription () const
    {
    return m_description;
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

// Helper method to resolve primitive type of any ECProperty, if available
bool resolvePrimitiveType(ECPropertyCP prop, PrimitiveType& type)
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
        PrimitiveArrayECPropertyCP arr = prop->GetAsPrimitiveArrayProperty();
        type = arr->GetPrimitiveElementType();
        return true;
        }

    return false;
    }

// Min/Max Value and Min/Max Length support is mutually exclusive, primitive types support at most one
bool propertySupportsMinMaxValue(ECPropertyCP prop, PrimitiveType& pt)
    {
    if (resolvePrimitiveType(prop, pt) && (
        pt == PrimitiveType::PRIMITIVETYPE_Double ||
        pt == PrimitiveType::PRIMITIVETYPE_Integer ||
        pt == PrimitiveType::PRIMITIVETYPE_Long))
        {
        return true;
        }
    
    return false;
    }

bool propertySupportsMinMaxLength(ECPropertyCP prop, PrimitiveType& pt)
    {
    if (resolvePrimitiveType(prop, pt) && (
        pt == PrimitiveType::PRIMITIVETYPE_String ||
        pt == PrimitiveType::PRIMITIVETYPE_Binary))
        {
        return true;
        }
    
    return false;
    }

/*---------------------------------------------------------------------------------**//**
@bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus ECProperty::SetMinimumValue(ECValueCR min)
    {
    if (min.IsNull() || !min.IsPrimitive())
        return ECObjectsStatus::DataTypeNotSupported;

    PrimitiveType pt = PrimitiveType::PRIMITIVETYPE_Integer;
    if (!propertySupportsMinMaxValue(this, pt))
        return ECObjectsStatus::DataTypeNotSupported;

    if (min.GetPrimitiveType() != pt)
        return ECObjectsStatus::DataTypeNotSupported;

    m_minimumValueOrLength = min;
    return ECObjectsStatus::Success;
    }


/*---------------------------------------------------------------------------------**//**
@bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool ECProperty::IsMinimumValueDefined() const
    {
    if (m_minimumValueOrLength.IsNull())
        return false;
    
    PrimitiveType pt = PrimitiveType::PRIMITIVETYPE_Integer;
    return propertySupportsMinMaxValue(this, pt);
    }

/*---------------------------------------------------------------------------------**//**
@bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void ECProperty::ResetMinimumValue()
    {
    if (IsMinimumValueDefined())
        m_minimumValueOrLength.SetToNull();
    }

/*---------------------------------------------------------------------------------**//**
@bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus ECProperty::GetMinimumValue(ECValueR value) const
    {
    if(!IsMinimumValueDefined())
        return ECObjectsStatus::Error;

    value = m_minimumValueOrLength;
    return ECObjectsStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
@bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus ECProperty::SetMaximumValue(ECValueCR max)
    {
    if (max.IsNull() || !max.IsPrimitive())
        return ECObjectsStatus::DataTypeNotSupported;

    PrimitiveType pt = PrimitiveType::PRIMITIVETYPE_Integer;
    if (!propertySupportsMinMaxValue(this, pt))
        return ECObjectsStatus::DataTypeNotSupported;

    if (max.GetPrimitiveType() != pt)
        return ECObjectsStatus::DataTypeNotSupported;

    m_maximumValueOrLength = max;
    return ECObjectsStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
@bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool ECProperty::IsMaximumValueDefined() const
    {
    if (m_maximumValueOrLength.IsNull())
        return false;

    PrimitiveType pt = PrimitiveType::PRIMITIVETYPE_Integer;
    return propertySupportsMinMaxValue(this, pt);
    }

/*---------------------------------------------------------------------------------**//**
@bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void ECProperty::ResetMaximumValue()
    {
    if (IsMaximumValueDefined())
        m_maximumValueOrLength.SetToNull();
    }

/*---------------------------------------------------------------------------------**//**
@bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus ECProperty::GetMaximumValue(ECValueR value) const
    {
    if (!IsMaximumValueDefined())
        return ECObjectsStatus::Error;

    value = m_maximumValueOrLength;
    return ECObjectsStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
@bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
uint32_t ECProperty::GetMaximumLength() const
{   
    if (!IsMaximumLengthDefined())
        return 0;
    
    return (uint32_t)m_maximumValueOrLength.GetLong();
}

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------+---------------+---------------+---------------+---------------+-------
ECObjectsStatus ECProperty::SetMinimumLength(uint32_t min)
    {
    PrimitiveType pt = PrimitiveType::PRIMITIVETYPE_Integer;
    if (!propertySupportsMinMaxLength(this, pt))
        return ECObjectsStatus::DataTypeNotSupported;

    m_minimumValueOrLength = ECValue((int64_t)min); 
    
    return ECObjectsStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
@bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool ECProperty::IsMinimumLengthDefined() const
    {
    if (m_minimumValueOrLength.IsNull())
        return false;

    PrimitiveType pt = PrimitiveType::PRIMITIVETYPE_Integer;
    if (!propertySupportsMinMaxLength(this, pt))
        return false;
    
    if (m_minimumValueOrLength.GetLong() <= 0)
        return false;
    
    return true;
    }

/*---------------------------------------------------------------------------------**//**
@bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void ECProperty::ResetMinimumLength()
    {
    if (IsMinimumLengthDefined())
        m_minimumValueOrLength.SetToNull();
    }

/*---------------------------------------------------------------------------------**//**
@bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
uint32_t ECProperty::GetMinimumLength() const
    {
    if (!IsMinimumLengthDefined())
        return 0;
    
    return (uint32_t)m_minimumValueOrLength.GetLong();
    }

/*---------------------------------------------------------------------------------**//**
@bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus ECProperty::SetMaximumLength(uint32_t max)
    {
    PrimitiveType pt = PrimitiveType::PRIMITIVETYPE_Integer;
    if (!propertySupportsMinMaxLength(this, pt))
        return ECObjectsStatus::DataTypeNotSupported;

    m_maximumValueOrLength = ECValue((int64_t)max);

    return ECObjectsStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
@bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool ECProperty::IsMaximumLengthDefined() const
    {
    if (m_maximumValueOrLength.IsNull())
        return false;

    PrimitiveType pt = PrimitiveType::PRIMITIVETYPE_Integer;
    if (!propertySupportsMinMaxLength(this, pt))
        return false;
    
    if (m_maximumValueOrLength.GetLong() <= 0)
        return false;
    
    return true;
    }

/*---------------------------------------------------------------------------------**//**
@bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void ECProperty::ResetMaximumLength()
    {
     if (IsMaximumLengthDefined())
        m_maximumValueOrLength.SetToNull();
    }

/*---------------------------------------------------------------------------------**//**
@bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void ECProperty::_AdjustMinMaxAfterTypeChange()
    {
    PrimitiveType pt = PrimitiveType::PRIMITIVETYPE_Integer;
    if (!resolvePrimitiveType(this, pt))
        {
        m_maximumValueOrLength.SetToNull();
        m_minimumValueOrLength.SetToNull();
        return;
        }

    if (pt == PrimitiveType::PRIMITIVETYPE_Integer || pt == PrimitiveType::PRIMITIVETYPE_Long || pt == PrimitiveType::PRIMITIVETYPE_Double)
        {
        if (!m_maximumValueOrLength.ConvertToPrimitiveType(pt))
            m_maximumValueOrLength.SetToNull();

        if (!m_minimumValueOrLength.ConvertToPrimitiveType(pt))
            m_minimumValueOrLength.SetToNull();

        return;
        }

    if (pt == PrimitiveType::PRIMITIVETYPE_String || pt == PrimitiveType::PRIMITIVETYPE_Binary)
        {
        m_maximumValueOrLength.SetToNull();
        m_minimumValueOrLength.SetToNull();
        return;
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
int32_t ECProperty::GetPriority() const
    {
    if (m_flags.prioritySet)
        return m_priority;

    if (HasBaseProperty())
        return GetBaseProperty()->GetPriority();

    return m_priority;
    }

/*---------------------------------------------------------------------------------**//**
 @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8StringCR ECProperty::GetDisplayLabel () const
    {
    return GetInvariantDisplayLabel();
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
ECObjectsStatus ECProperty::SetIsReadOnly (bool readOnly)
    {
    if (m_flags.readOnly != readOnly)
        {
        m_flags.readOnly = readOnly;
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
    ECObjectsStatus status = SchemaParseUtils::ParseBooleanXmlString (bReadOnly, isReadOnly);
    if (ECObjectsStatus::Success != status)
        LOG.errorv (L"Failed to parse the isReadOnly string '%s' for ECProperty '%s'.", isReadOnly, this->GetName().c_str());
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

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
Utf8String ECProperty::GetTypeFullName() const
    {
    return this->_GetTypeName(true);
    }

/*---------------------------------------------------------------------------------**//**
 @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus ECProperty::SetTypeName(Utf8StringCR typeName)
    {
    return this->_SetTypeName(typeName);
    }

/*---------------------------------------------------------------------------------**//**
@bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool ECProperty::HasExtendedType () const
    {
    return this->_HasExtendedType();
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
PropertyCategoryCP ECProperty::GetCategory() const
    {
    if (m_propertyCategory == nullptr)
        {
        if (HasBaseProperty())
            return GetBaseProperty()->GetCategory();
        }

    return m_propertyCategory;
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
ECObjectsStatus ECProperty::SetCategory(PropertyCategoryCP propertyCategory)
    {
    if (nullptr == propertyCategory)
        {
        m_propertyCategory = propertyCategory;
        return ECObjectsStatus::Success;
        }

    if (&(propertyCategory->GetSchema()) != &(this->GetClass().GetSchema()))
        {
        if (!ECSchema::IsSchemaReferenced(this->GetClass().GetSchema(), propertyCategory->GetSchema()))
            return ECObjectsStatus::SchemaNotFound;
        }

    m_propertyCategory = propertyCategory;
    return ECObjectsStatus::Success;
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------+---------------+---------------+---------------+---------------+-------
bool comparePropertyValues(ECValuesCollectionCR lhs, IECInstancePtr rhs)
    {
    for (ECPropertyValueCR lhsProp : lhs)
        {
        ECValueCR lhsVal = lhsProp.GetValue();
        ECValue rhsVal;
        if (ECObjectsStatus::Success != rhs->GetValueUsingAccessor(rhsVal, lhsProp.GetValueAccessor()))
            return false;
        if (lhsVal != rhsVal)
            return false;
        if (lhsProp.HasChildValues())
            {
            if (!comparePropertyValues(*lhsProp.GetChildValues(), rhs))
                return false;
            }
        }
    return true;
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------+---------------+---------------+---------------+---------------+-------
bool ECProperty::_IsSame(ECPropertyCR target) const
    {
    if (!GetName().EqualsIAscii(target.GetName().c_str()))
        return false;

    if (!GetTypeName().Equals(target.GetTypeName()))
        return false;

    if (GetIsDisplayLabelDefined() && !GetDisplayLabel().Equals(target.GetDisplayLabel()))
        return false;

    if (!GetDescription().Equals(target.GetDescription()))
        return false;

    if (IsMinimumValueDefined() != target.IsMinimumValueDefined())
        return false;

    if (IsMinimumValueDefined())
        {
        ECValue left;
        ECValue right;
        ECObjectsStatus stat1 = GetMinimumValue(left);
        ECObjectsStatus stat2 = target.GetMinimumValue(right);
        if (stat1 != stat2)
            return false;
        if (left != right)
            return false;
        }

    if (IsMaximumValueDefined() != target.IsMaximumValueDefined())
        return false;

    if (IsMaximumValueDefined())
        {
        ECValue left;
        ECValue right;
        GetMaximumValue(left);
        target.GetMaximumValue(right);
        if (left != right)
            return false;
        }

    if (IsMinimumLengthDefined() != target.IsMinimumLengthDefined())
        return false;

    if (IsMinimumLengthDefined() && GetMinimumLength() != target.GetMinimumLength())
        return false;

    if (IsMaximumLengthDefined() != target.IsMaximumLengthDefined())
        return false;

    if (IsMaximumLengthDefined() && GetMaximumLength() != target.GetMaximumLength())
        return false;

    if (IsPriorityLocallyDefined() != target.IsPriorityLocallyDefined())
        return false;

    if (IsPriorityLocallyDefined() && GetPriority() != target.GetPriority())
        return false;

    if (GetIsReadOnly() != target.GetIsReadOnly())
        return false;

    // These are EC 3.1 concepts and currently we are only concerned about comparing 2.0 properties before they are converted
    //if (IsKindOfQuantityDefinedLocally() != target.IsKindOfQuantityDefinedLocally())
    //    return false;

    //if (IsKindOfQuantityDefinedLocally())
    //    {
    //    KindOfQuantityCP left = GetKindOfQuantity();
    //    KindOfQuantityCP right = target.GetKindOfQuantity();
    //    if (*left != *right)
    //        return false;
    //    }

    //if (IsCategoryDefinedLocally() != target.IsCategoryDefinedLocally())
    //    return false;

    //if (IsCategoryDefinedLocally())
    //    {
    //    if (nullptr == target.GetCategory())
    //        return false;
    //    if (*GetCategory() != *target.GetCategory())
    //    return false;
    //    }

    int lhsCount = 0;
    for (auto ca : GetCustomAttributes(false))
        {
        // It is too complicated to try to figure out whether the units are the same before they have been converted to the new system.  Therefore, we just
        // automatically declare that if the property has a UnitSpecification custom attribute on it, it is different from the other property
        if (0 == strcmp("Unit_Attributes:UnitSpecificationAttr", ca->GetClass().GetFullName()))
            return false;

        IECInstancePtr rhs = target.GetCustomAttribute(ca->GetClass());
        if (!rhs.IsValid())
            return false;

        ECValuesCollectionPtr lhsValues = ECValuesCollection::Create(*ca);
        if (!comparePropertyValues(*lhsValues, rhs))
            return false;
        lhsCount++;
        }

    int rhsCount = 0;
    for (auto ca : target.GetCustomAttributes(false))
        rhsCount++;

    if (lhsCount != rhsCount)
        return false;

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void ECProperty::_GetBaseContainers (bvector<IECCustomAttributeContainerP>& returnList) const
    {
    if (NULL != m_baseProperty)
        returnList.push_back((const_cast<ECPropertyP>(m_baseProperty)));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ECSchemaCP ECProperty::_GetContainerSchema () const
    {
    return &(m_class.GetSchema());
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------+---------------+---------------+---------------+---------------+-------
Utf8String ECProperty::_GetContainerName() const
    {
    return Utf8String(GetClass().GetFullName()) + ":" + GetName();
    }

ECObjectsStatus resolveKindOfQuantityType(KindOfQuantityCP& kindOfQuantity, Utf8StringCR typeName, ECSchemaCR parentSchema)
    {
    // typeName may potentially be qualified so we must parse into an alias and short class name
    Utf8String alias;
    Utf8String kindOfQuantityName;
    if (ECObjectsStatus::Success != SchemaParseUtils::ParseName(alias, kindOfQuantityName, typeName))
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

ECObjectsStatus resolvePropertyCategory(PropertyCategoryCP& propertyCategory, Utf8StringCR typeName, ECSchemaCR parentSchema)
    {
    // typeName may potentially be qualified so we must parse into an alias and short class name
    Utf8String alias;
    Utf8String propertyCategoryName;
    if (ECObjectsStatus::Success != SchemaParseUtils::ParseXmlFullyQualifiedName(alias, propertyCategoryName, typeName))
        {
        LOG.warningv("Cannot resolve the type name '%s'.", typeName.c_str());
        return ECObjectsStatus::ParseError;
        }

    ECSchemaCP resolvedSchema = parentSchema.GetSchemaByAliasP(alias);
    if (nullptr == resolvedSchema)
        return ECObjectsStatus::SchemaNotFound;

    auto result = resolvedSchema->GetPropertyCategoryCP(propertyCategoryName.c_str());
    if (nullptr == result)
        return ECObjectsStatus::DataTypeNotSupported;

    propertyCategory = result;
    return ECObjectsStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
SchemaReadStatus ECProperty::_ReadXml (pugi::xml_node propertyNode, ECSchemaReadContextR context)
    {
    Utf8String value;
    READ_REQUIRED_XML_ATTRIBUTE(propertyNode, PROPERTY_NAME_ATTRIBUTE, this, Name, propertyNode.name())

    // OPTIONAL attributes - If these attributes exist they MUST be valid
    READ_OPTIONAL_XML_ATTRIBUTE(propertyNode, DESCRIPTION_ATTRIBUTE, this, Description)

    uint32_t priority;
    if (BePugiXmlHelper::GetAttributeUInt32Value(propertyNode, priority,PRIORITY_ATTRIBUTE) == BePugiXmlValueResult::Success)
        {
        SetPriority(priority);
        }

    READ_OPTIONAL_XML_ATTRIBUTE (propertyNode, ECXML_DISPLAY_LABEL_ATTRIBUTE, this, DisplayLabel)

    // OPTIONAL attributes - If these attributes exist they do not need to be valid.  We will ignore any errors setting them and use default values.
    // NEEDSWORK This is due to the current implementation in managed ECObjects.  We should reconsider whether it is the correct behavior.
    ECObjectsStatus setterStatus;
    UNUSED_VARIABLE(setterStatus);
    READ_OPTIONAL_XML_ATTRIBUTE_IGNORING_SET_ERRORS (propertyNode, ECXML_READONLY_ATTRIBUTE, this, IsReadOnly)

    auto koqAttr = propertyNode.attribute(KIND_OF_QUANTITY_ATTRIBUTE);
    if (koqAttr)
        {
        value = koqAttr.as_string();
        //split
        KindOfQuantityCP kindOfQuantity;
        if (resolveKindOfQuantityType(kindOfQuantity, value, GetClass().GetSchema()) != ECObjectsStatus::Success)
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

    auto categoryAttr = propertyNode.attribute(CATEGORY_ATTRIBUTE);
    if (categoryAttr)
        {
        value = categoryAttr.as_string();
        PropertyCategoryCP propertyCategory;
        if (resolvePropertyCategory(propertyCategory, value, GetClass().GetSchema()) != ECObjectsStatus::Success)
            {
            LOG.errorv("Could not resolve PropertyCategory '%s' found on property '%s.%s'.",
                value.c_str(),
                GetClass().GetFullName(),
                GetName().c_str());

            return SchemaReadStatus::InvalidECSchemaXml;
            }

        SetCategory(propertyCategory);
        }

    return SchemaReadStatus::Success;
    }

bool isKindOfQuantityCompatible(ECPropertyCR ecProp, ECPropertyCP baseProp, KindOfQuantityCP compareKOQ)
    {
    if (nullptr == compareKOQ || nullptr == baseProp)
        return true;

    KindOfQuantityCP baseKOQ = baseProp->GetKindOfQuantity();
    if (nullptr == baseKOQ)
        return true;

    Units::UnitCP baseUnit = baseKOQ->GetPersistenceUnit();
    Units::UnitCP compareUnit = compareKOQ->GetPersistenceUnit();

    if (nullptr == baseUnit || nullptr == compareUnit)
        return true;

    if (!Units::Unit::AreEqual(baseUnit, compareUnit))
        {
        LOG.errorv("The ECProperty '%s:%s' has a base property '%s:%s' with KindOfQuantity '%s' with persistence unit '%s' which is not the same as the persistence unit '%s' of the provided KindOfQuantity '%s'.",
                   ecProp.GetClass().GetFullName(), ecProp.GetName().c_str(), baseProp->GetClass().GetFullName(), baseProp->GetName().c_str(), baseKOQ->GetFullName().c_str(), baseUnit->GetName().c_str(),
                   compareUnit->GetName().c_str(), compareKOQ->GetFullName().c_str());
        return false;
        }

    return true;
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
KindOfQuantityCP ECProperty::GetKindOfQuantity() const
    {
    if (m_kindOfQuantity == nullptr)
        {
        if (HasBaseProperty())
            {
            PrimitiveECPropertyCP basePrim = GetBaseProperty()->GetAsPrimitiveProperty();
            if (nullptr != basePrim)
                return basePrim->GetKindOfQuantity();
            }
        }

    return m_kindOfQuantity;
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
ECObjectsStatus ECProperty::SetKindOfQuantity(KindOfQuantityCP kindOfQuantity)
    {
    if (nullptr == kindOfQuantity)
        {
        m_kindOfQuantity = kindOfQuantity;
        return ECObjectsStatus::Success;
        }

    if (&(kindOfQuantity->GetSchema()) != &(this->GetClass().GetSchema()))
        {
        if (!ECSchema::IsSchemaReferenced(this->GetClass().GetSchema(), kindOfQuantity->GetSchema()))
            return ECObjectsStatus::SchemaNotFound;
        }

    if (!isKindOfQuantityCompatible(*this, this->GetBaseProperty(), kindOfQuantity))
        return ECObjectsStatus::KindOfQuantityNotCompatible;

    for (ECClassCP derived : this->GetClass().GetDerivedClasses())
        {
        ECPropertyP derivedProp = derived->GetPropertyP(this->GetName().c_str());
        if (nullptr == derivedProp)
            continue;
        if (!isKindOfQuantityCompatible(*this, derivedProp, kindOfQuantity))
            return ECObjectsStatus::KindOfQuantityNotCompatible;
        }
    m_kindOfQuantity = kindOfQuantity;
    return ECObjectsStatus::Success;
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------+---------------+---------------+---------------+---------------+-------
static bool IsCategorySerializable(const ECProperty& prop)
    {
    return prop.IsCategoryDefinedLocally() && prop.GetCategory()->GetName() != "";
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
SchemaWriteStatus ECProperty::_WriteXml (BeXmlWriterR xmlWriter, ECVersion ecXmlVersion)
    {
    return _WriteXml (xmlWriter, EC_PROPERTY_ELEMENT, ecXmlVersion);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
SchemaWriteStatus ECProperty::_WriteXml (BeXmlWriterR xmlWriter, Utf8CP elementName, ECVersion ecXmlVersion, bvector<bpair<Utf8CP, Utf8CP>>* additionalAttributes, bool writeType)
    {
    SchemaWriteStatus status = SchemaWriteStatus::Success;

    xmlWriter.WriteElementStart(elementName);

    xmlWriter.WriteAttribute(PROPERTY_NAME_ATTRIBUTE, this->GetName().c_str());

    if (writeType)
        {
        xmlWriter.WriteAttribute(TYPE_NAME_ATTRIBUTE, this->_GetTypeNameForXml(ecXmlVersion).c_str());
        }

    xmlWriter.WriteAttribute(DESCRIPTION_ATTRIBUTE, this->GetInvariantDescription().c_str());
    if (GetIsDisplayLabelDefined())
        xmlWriter.WriteAttribute(ECXML_DISPLAY_LABEL_ATTRIBUTE, this->GetInvariantDisplayLabel().c_str());
    if(IsReadOnlyFlagSet())
        xmlWriter.WriteAttribute(ECXML_READONLY_ATTRIBUTE, true);
    if (IsMinimumValueDefined())
        {
        Utf8String minValue;
        if (m_minimumValueOrLength.ConvertPrimitiveToString(minValue))
            {
            if (GetClass().GetSchema().OriginalECXmlVersionLessThan(ECVersion::V3_0))
                xmlWriter.WriteAttribute("MinimumValue", minValue.c_str());
            else
                xmlWriter.WriteAttribute(ECXML_MINIMUM_VALUE_ATTRIBUTE, minValue.c_str());
            }
        }

    if (IsMaximumValueDefined())
        {
        Utf8String maxValue;
        if (m_maximumValueOrLength.ConvertPrimitiveToString(maxValue))
            {
            if (GetClass().GetSchema().OriginalECXmlVersionLessThan(ECVersion::V3_0))
                xmlWriter.WriteAttribute("MaximumValue", maxValue.c_str());
            else
                xmlWriter.WriteAttribute(ECXML_MAXIMUM_VALUE_ATTRIBUTE, maxValue.c_str());
            }
        }
        
    if (IsMaximumLengthDefined())
        {
        xmlWriter.WriteAttribute(ECXML_MAXIMUM_LENGTH_ATTRIBUTE, GetMaximumLength());
        }

    if (IsMinimumLengthDefined())
        {
        xmlWriter.WriteAttribute(ECXML_MINIMUM_LENGTH_ATTRIBUTE, GetMinimumLength());
        }

    // Only serialize for 3.1 or newer
    if (ECVersion::V3_1 <= ecXmlVersion)
        {
        if (IsCategorySerializable(*this))
            xmlWriter.WriteAttribute(CATEGORY_ATTRIBUTE, GetCategory()->GetQualifiedName(GetClass().GetSchema()).c_str());

        if (IsPriorityLocallyDefined())
            xmlWriter.WriteAttribute(PRIORITY_ATTRIBUTE, GetPriority());
        }

    if (nullptr != additionalAttributes && !additionalAttributes->empty())
        {
        for (auto& attribute : *additionalAttributes)
            xmlWriter.WriteAttribute(attribute.first, attribute.second);
        }

    if (ecXmlVersion >= ECVersion::V3_0 && IsKindOfQuantityDefinedLocally())
        {
        Utf8String koqQualifiedName;
        koqQualifiedName = GetKindOfQuantity()->GetQualifiedName(GetClass().GetSchema());
        xmlWriter.WriteAttribute(KIND_OF_QUANTITY_ATTRIBUTE, koqQualifiedName.c_str());
        }

    WriteCustomAttributes(xmlWriter, ecXmlVersion);
    xmlWriter.WriteElementEnd();

    return status;
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------+---------------+---------------+---------------+---------------+-------
bool ECProperty::_ToJson(BeJsValue outValue, bool isInherited) const
    {
    return _ToJson(outValue, isInherited, bvector<bpair<Utf8String, Json::Value>>());
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------+---------------+---------------+---------------+---------------+-------
bool ECProperty::_ToJson(BeJsValue outValue, bool isInherited, bvector<bpair<Utf8String, Json::Value>> additionalAttributes) const
    {
    outValue[NAME_ATTRIBUTE] = GetName();

    Utf8String propertyType;
    if (GetIsPrimitive())
        propertyType = ECJSON_ECPROPERTY_PRIMITIVE;
    else if (GetIsStruct())
        propertyType = ECJSON_ECPROPERTY_STRUCT;
    else if (GetIsPrimitiveArray())
        propertyType = ECJSON_ECPROPERTY_PRIMITIVEARRAY;
    else if (GetIsStructArray())
        propertyType = ECJSON_ECPROPERTY_STRUCTARRAY;
    else if (GetIsNavigation())
        propertyType = ECJSON_ECPROPERTY_NAVIGATION;
    else
        return false;

    outValue[TYPE_ATTRIBUTE] = propertyType;

    if (GetInvariantDescription().length() != 0)
        outValue[DESCRIPTION_ATTRIBUTE] = GetInvariantDescription();

    if (GetIsDisplayLabelDefined())
        outValue[ECJSON_DISPLAY_LABEL_ATTRIBUTE] = GetInvariantDisplayLabel();

    if (IsReadOnlyFlagSet()) // Only need to specify this property if it's true.
        outValue[ECJSON_READONLY_ATTRIBUTE] = true;

    if (IsCategorySerializable(*this))
        outValue[CATEGORY_ATTRIBUTE] = ECJsonUtilities::FormatPropertyCategoryName(*GetCategory());

    if (IsPriorityLocallyDefined())
        outValue[PRIORITY_ATTRIBUTE] = GetPriority();

    if (GetIsArray())
        {
        ArrayECPropertyCP propArr = GetAsArrayProperty();
        outValue[MIN_OCCURS_ATTRIBUTE] = propArr->GetMinOccurs();
        if (propArr->GetStoredMaxOccurs() != INT_MAX)
            outValue[MAX_OCCURS_ATTRIBUTE] = propArr->GetStoredMaxOccurs();
        }

    if (IsKindOfQuantityDefinedLocally())
        outValue[KIND_OF_QUANTITY_ATTRIBUTE] = ECJsonUtilities::FormatKindOfQuantityName(*GetKindOfQuantity());

    WriteCustomAttributes(outValue);

    if (isInherited)
        outValue[ECJSON_INHERITED_ATTRIBUTE] = true;

    for (auto const& attribute : additionalAttributes)
        outValue[attribute.first].From(attribute.second);

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
SchemaReadStatus PrimitiveECProperty::_ReadXml (pugi::xml_node propertyNode, ECSchemaReadContextR context)
    {
    SchemaReadStatus status = T_Super::_ReadXml (propertyNode, context);
    if (status != SchemaReadStatus::Success)
        return status;

    // For Primitive & Array properties we ignore parse errors and default to string. Struct properties will require a resolvable typename.
    status = ReadTypeNameWithPruning(propertyNode, this, context, true);
    if (status != SchemaReadStatus::Success)
        return status;

    auto extendedTypeAttr = propertyNode.attribute(EXTENDED_TYPE_NAME_ATTRIBUTE);
    if (extendedTypeAttr)
        {
        this->SetExtendedTypeName(extendedTypeAttr.as_string());
        }

    status = ReadMinMaxXml(propertyNode);
    if (SchemaReadStatus::Success != status)
        return status;

    return status;
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------+---------------+---------------+---------------+---------------+-------
static SchemaWriteStatus WriteCommonPrimitivePropertyJsonAttributes(bvector<bpair<Utf8String, Json::Value>>& attributes, ECPropertyCP ecProperty)
    {
    PrimitiveArrayECPropertyCP primArrProp = ecProperty->GetAsPrimitiveArrayProperty();
    PrimitiveECPropertyCP primProp = ecProperty->GetAsPrimitiveProperty();

    if (nullptr == primArrProp && nullptr == primProp)
        return SchemaWriteStatus::FailedToCreateJson;

    attributes.push_back(bpair<Utf8String, Json::Value>(TYPE_NAME_ATTRIBUTE, primProp ? primProp->GetTypeFullName() : primArrProp->GetTypeFullName()));
    if (primProp ? primProp->IsExtendedTypeDefinedLocally() : primArrProp->IsExtendedTypeDefinedLocally())
        attributes.push_back(bpair<Utf8String, Json::Value>(
            EXTENDED_TYPE_NAME_ATTRIBUTE,
            primProp ? primProp->GetExtendedTypeName() : primArrProp->GetExtendedTypeName()
            ));

    ECObjectsStatus status;

    if (primProp ? primProp->IsMinimumValueDefined() : primArrProp->IsMinimumValueDefined())
        {
        ECValue minVal;
        Json::Value tmpJson;
        if (ECObjectsStatus::Success != (status = primProp ? primProp->GetMinimumValue(minVal) : primArrProp->GetMinimumValue(minVal)))
            return SchemaWriteStatus::FailedToCreateJson;
        if (minVal.IsInteger())
            tmpJson = minVal.GetInteger();
        else if (minVal.IsLong())
            tmpJson = Json::Value(minVal.GetLong());
        else if (minVal.IsDouble())
            tmpJson = minVal.GetDouble();
        else
            return SchemaWriteStatus::FailedToCreateJson;
        attributes.push_back(bpair<Utf8String, Json::Value>(ECJSON_MINIMUM_VALUE_ATTRIBUTE, tmpJson));
        }

    if (primProp ? primProp->IsMaximumValueDefined() : primArrProp->IsMaximumValueDefined())
        {
        ECValue maxVal;
        Json::Value tmpJson;
        if (ECObjectsStatus::Success != (status = primProp ? primProp->GetMaximumValue(maxVal) : primArrProp->GetMaximumValue(maxVal)))
            return SchemaWriteStatus::FailedToCreateJson;
        if (maxVal.IsInteger())
            tmpJson = maxVal.GetInteger();
        else if (maxVal.IsLong())
            tmpJson = Json::Value(maxVal.GetLong());
        else if (maxVal.IsDouble())
            tmpJson = maxVal.GetDouble();
        else
            return SchemaWriteStatus::FailedToCreateJson;
        attributes.push_back(bpair<Utf8String, Json::Value>(ECJSON_MAXIMUM_VALUE_ATTRIBUTE, tmpJson));
        }
    if (primProp ? primProp->IsMinimumLengthDefined() : primArrProp->IsMinimumLengthDefined())
        attributes.push_back(bpair<Utf8String, Json::Value>(
            ECJSON_MINIMUM_LENGTH_ATTRIBUTE,
            primProp ? primProp->GetMinimumLength() : primArrProp->GetMinimumLength()
            ));
    if (primProp ? primProp->IsMaximumLengthDefined() : primArrProp->IsMaximumLengthDefined())
        attributes.push_back(bpair<Utf8String, Json::Value>(
            ECJSON_MAXIMUM_LENGTH_ATTRIBUTE,
            primProp ? primProp->GetMaximumLength() : primArrProp->GetMaximumLength()
            ));

    return SchemaWriteStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
 * @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
SchemaWriteStatus PrimitiveECProperty::_WriteXml(BeXmlWriterR xmlWriter, ECVersion ecXmlVersion)
    {
    bvector<bpair<Utf8CP, Utf8CP>> attributes;

    if (IsExtendedTypeDefinedLocally())
        attributes.push_back(make_bpair(EXTENDED_TYPE_NAME_ATTRIBUTE, GetExtendedTypeName().c_str()));

    return T_Super::_WriteXml(xmlWriter, EC_PROPERTY_ELEMENT, ecXmlVersion, &attributes);
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------+---------------+---------------+---------------+---------------+-------
bool PrimitiveECProperty::_ToJson(BeJsValue outValue, bool isInherited) const
    {
    bvector<bpair<Utf8String, Json::Value>> attributes;
    WriteCommonPrimitivePropertyJsonAttributes(attributes, this);
    return T_Super::_ToJson(outValue, isInherited, attributes);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool PrimitiveECProperty::_CanOverride (ECPropertyCR baseProperty, Utf8StringR errMsg) const
    {
    if (!baseProperty.GetIsPrimitive() && !baseProperty.GetIsPrimitiveArray())
        {
        errMsg.Sprintf("The property %s:%s cannot be overriden by PrimitiveECProperty %s:%s because it is not a PrimitiveECProperty.",
                   baseProperty.GetClass().GetFullName(), baseProperty.GetName().c_str(), GetClass().GetFullName(), GetName().c_str());
        return false;
        }

    // normally, we do not allow a primitive property to override an array property.  However, there is a set of schemas that
    // have been delivered that allow this behavior.  If the primitive property type is the same as the type used in the array, then
    // we allow it to be overridden.
    PrimitiveType basePrimitiveType;
    if (baseProperty.GetIsArray())
        basePrimitiveType = baseProperty.GetAsPrimitiveArrayProperty()->GetPrimitiveElementType();

    else if (baseProperty.GetIsNavigation())
        return false;
    else
        basePrimitiveType = baseProperty.GetAsPrimitiveProperty()->GetType();

    if (basePrimitiveType != m_primitiveType)
        {
        errMsg.Sprintf("The ECProperty %s:%s has a primitive type '%s' that does not match the primitive type '%s' of ECProperty %s:%s.",
                   baseProperty.GetClass().GetFullName(), baseProperty.GetName().c_str(), SchemaParseUtils::PrimitiveTypeToString(basePrimitiveType),
                       SchemaParseUtils::PrimitiveTypeToString(m_primitiveType), GetClass().GetFullName(), GetName().c_str());
        return false;
        }

    return isKindOfQuantityCompatible(*this, &baseProperty, this->GetKindOfQuantity());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String PrimitiveECProperty::_GetTypeName (bool useFullName) const
    {
    if(m_enumeration == nullptr)
        return SchemaParseUtils::PrimitiveTypeToString (m_primitiveType);
    if (useFullName)
        return ECJsonUtilities::FormatEnumerationName(*m_enumeration);
    return SchemaParseUtils::GetQualifiedName<ECEnumeration>(this->GetClass().GetSchema(), *m_enumeration);
    }

Utf8String PrimitiveECProperty::_GetTypeNameForXml(ECVersion ecXmlVersion) const
    {
    if (ecXmlVersion <= ECVersion::V2_0 && m_enumeration != nullptr)
        return m_enumeration->GetTypeName();

    return GetTypeName();
    }

ECObjectsStatus ResolveEnumerationType(ECEnumerationCP& enumeration, Utf8StringCR typeName, ECSchemaCR parentSchema)
    {
    // typeName may potentially be qualified so we must parse into an alias and short class name
    Utf8String alias;
    Utf8String enumName;
    ECObjectsStatus status = SchemaParseUtils::ParseName(alias, enumName, typeName);
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
    ECObjectsStatus status = SchemaParseUtils::ParsePrimitiveType (primitiveType, typeName);
    if (ECObjectsStatus::Success == status)
        {
        return SetType(primitiveType);
        }

    ECEnumerationCP enumeration;
    if (ECObjectsStatus::Success == ResolveEnumerationType(enumeration, typeName, this->GetClass().GetSchema()))
        {
        return SetType(*enumeration);
        }

    LOG.warningv("Unrecognized typeName '%s' found in '%s:%s.%s'. A type of 'string' will be used.",
                 typeName.c_str(),
                 GetClass().GetSchema().GetName().c_str(),
                 GetClass().GetName().c_str(),
                 GetName().c_str());
    return status;
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------+---------------+---------------+---------------+---------------+-------
bool PrimitiveECProperty::_IsSame(ECPropertyCR target) const
    {
    PrimitiveECPropertyCP rhs = target.GetAsPrimitiveProperty();
    if (nullptr == rhs)
        return false;

    if (!T_Super::_IsSame(target))
        return false;

    if (GetType() != rhs->GetType())
        return false;

    if (IsExtendedTypeDefinedLocally())
        return false;

    return true;
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
    if (&(enumerationType.GetSchema()) != &(this->GetClass().GetSchema()))
        {
        if (!ECSchema::IsSchemaReferenced(this->GetClass().GetSchema(), enumerationType.GetSchema()))
            return ECObjectsStatus::SchemaNotFound;
        }

    auto primitiveType = enumerationType.GetType();
    if (m_primitiveType != primitiveType)
        {
        m_primitiveType = primitiveType;
        InvalidateClassLayout();
        }

    m_enumeration = &enumerationType;

    return ECObjectsStatus::Success;
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
Utf8StringCR PrimitiveECProperty::GetExtendedTypeName() const
    {
    if (!IsExtendedTypeDefinedLocally())
        {
        if (HasBaseProperty())
            {
            PrimitiveECPropertyCP baseExtended = GetBaseProperty()->GetAsPrimitiveProperty();
            if (nullptr != baseExtended)
                return baseExtended->GetExtendedTypeName();
            }
        }

    return m_extendedTypeName;
    }

/*---------------------------------------------------------------------------------**//**
 @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus PrimitiveECProperty::SetExtendedTypeName(Utf8CP extendedTypeName)
    {
    m_extendedTypeName.clear();
    if (!Utf8String::IsNullOrEmpty(extendedTypeName))
        m_extendedTypeName.assign(extendedTypeName);

    return ECObjectsStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
SchemaReadStatus StructECProperty::_ReadXml (pugi::xml_node propertyNode, ECSchemaReadContextR context)
    {
    SchemaReadStatus status = T_Super::_ReadXml (propertyNode, context);
    if (status != SchemaReadStatus::Success)
        return status;

    Utf8String typeName;
    // For Primitive & Array properties we ignore parse errors and default to string. Struct properties will require a resolvable typename.
    return ReadTypeNameWithPruning(propertyNode, this, context);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
SchemaWriteStatus StructECProperty::_WriteXml (BeXmlWriterR xmlWriter, ECVersion ecXmlVersion)
    {
    return T_Super::_WriteXml (xmlWriter, EC_STRUCTPROPERTY_ELEMENT, ecXmlVersion);
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------+---------------+---------------+---------------+---------------+-------
bool StructECProperty::_ToJson(BeJsValue outValue, bool isInherited) const
    {
    bvector<bpair<Utf8String, Json::Value>> attributes;
    attributes.push_back(bpair<Utf8String, Json::Value>(TYPE_NAME_ATTRIBUTE, GetTypeFullName()));
    return T_Super::_ToJson(outValue, isInherited, attributes);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool StructECProperty::_CanOverride (ECPropertyCR baseProperty, Utf8StringR errMsg) const
    {
    if (!baseProperty.GetIsStruct())
        {
        errMsg.Sprintf("The property %s:%s cannot be overriden by StructECProperty %s:%s because it is not a StructECProperty.",
                   baseProperty.GetClass().GetFullName(), baseProperty.GetName().c_str(), GetClass().GetFullName(), GetName().c_str());
        return false;
        }

    // if the struct type hasn't been set yet, we will say it can override
    if (NULL == m_structType)
        return true;

    // This used to always compare GetTypeName(). Type names for struct arrays include the alias as defined in the referencing schema. That is weird and easily breaks if:
    //  -Base property is defined in same schema as the struct class (cannot be worked around), or
    //  -Base property's schema declares different alias for struct class's schema than the overriding property's schema (dumb workaround: make them use the same alias).
    // Instead, compare the full-qualified class name.
    Utf8String baseStructName = baseProperty.GetAsStructProperty()->GetType().GetFullName();
    if (0 != strcmp(m_structType->GetFullName(), baseStructName.c_str()))
        {
        errMsg.Sprintf("The StructECProperty %s:%s with type %s cannot be overriden by %s:%s with type %s because they have different types.",
                   baseProperty.GetClass().GetFullName(), baseProperty.GetName().c_str(), baseStructName.c_str(),
                   GetClass().GetFullName(), GetName().c_str(), m_structType->GetFullName());
        return false;
        }

    return isKindOfQuantityCompatible(*this, &baseProperty, this->GetKindOfQuantity());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String StructECProperty::_GetTypeName (bool useFullName) const
    {
    if (!EXPECTED_CONDITION (NULL != m_structType))
        return EMPTY_STRING;
    if (useFullName)
        return ECJsonUtilities::FormatClassName(*m_structType);
    return ECClass::GetQualifiedClassName (this->GetClass().GetSchema(), *m_structType);
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------+---------------+---------------+---------------+---------------+-------
bool StructECProperty::_IsSame(ECPropertyCR target) const
    {
    StructECPropertyCP rhs = target.GetAsStructProperty();
    if (nullptr == rhs)
        return false;

    if (!T_Super::_IsSame(target))
        return false;

    if (!GetType().Is(&rhs->GetType()))
        return false;

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus ResolveStructType(ECStructClassCP& structClass, Utf8StringCR typeName, ECClassCR containingClass, bool doLogging = true)
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

    // schema resolution errors must be logged by caller
    ECSchemaCP resolvedSchema = containingClass.GetSchema().GetSchemaByAliasP (alias);
    if (NULL == resolvedSchema)
        return ECObjectsStatus::SchemaNotFound;

    ECClassCP ecClass = resolvedSchema->GetClassCP (className.c_str());
    if (NULL == ecClass)
        {
        if (doLogging)
            LOG.errorv("Cannot resolve the type name '%s.%s' as a struct type because ECClass '%s' does not exist in the schema '%s'.",
                alias.c_str(), className.c_str(), className.c_str(), resolvedSchema->GetName().c_str());
        return ECObjectsStatus::ClassNotFound;
        }

    structClass = ecClass->GetStructClassCP();
    if (NULL == structClass)
        {
        if (doLogging)
            LOG.errorv("ECClass '%s' does exists in the schema '%s' but is not of type ECStructClass.",
                className.c_str(), resolvedSchema->GetName().c_str());
        return ECObjectsStatus::ClassNotFound;
        }

    return ECObjectsStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
 @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus StructECProperty::_SetTypeName(Utf8StringCR typeName)
    {
    ECStructClassCP structClass;
    ECObjectsStatus resolveStatus = ResolveStructType(structClass, typeName, this->GetClass());
    // error must be logged by caller
    if (ECObjectsStatus::Success != resolveStatus)
        return resolveStatus;

    if (structClass->Is(&this->GetClass()))
        {
        LOG.errorv("Failed to set typename of ECStructProperty '%s:%s' to '%s' because the struct type is the same class as the containing class.", this->GetClass().GetName().c_str(), this->GetName().c_str(), typeName.c_str());
        return ECObjectsStatus::ECClassNotSupported;
        }

    // SetType fails if the resolved type's schema is not referenced
    return SetType(*structClass);
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
        InvalidateClassLayout();
        }

    return ECObjectsStatus::Success;
    }

SchemaReadStatus ECProperty::ReadMinMaxXml(pugi::xml_node propertyNode)
    {
    uint32_t minLength;
    auto minLengthAttr = propertyNode.attribute(ECXML_MINIMUM_LENGTH_ATTRIBUTE);
    if (minLengthAttr)
        {
        minLength = minLengthAttr.as_uint();
        if (ECObjectsStatus::Success != SetMinimumLength(minLength) && GetContainerSchema()->OriginalECXmlVersionAtLeast(ECVersion::V3_0))
            {
            LOG.errorv("Cannot set the minimum length attribute on ECProperty, %s.%s, because the datatype is not supported. Minimum length can only be set on a property with a primitive type of string and binary.",
                    GetClass().GetFullName(), GetName().c_str());
            return SchemaReadStatus::InvalidECSchemaXml;
            }
        }

    uint32_t maxLength;
    auto maxLengthAttr = propertyNode.attribute(ECXML_MAXIMUM_LENGTH_ATTRIBUTE);
    if (maxLengthAttr)
        {
        maxLength = maxLengthAttr.as_uint();
        if (ECObjectsStatus::Success != SetMaximumLength(maxLength) && GetContainerSchema()->OriginalECXmlVersionAtLeast(ECVersion::V3_0))
            {
            LOG.errorv("Cannot set the maximum length attribute on ECProperty, %s.%s, because the datatype is not supported. Maximum length can only be set on a property with a primitive type of string and binary.",
                GetClass().GetFullName(), GetName().c_str());
            return SchemaReadStatus::InvalidECSchemaXml;
            }
        }

    
    pugi::xml_attribute minValueAttr;
    
    if (GetClass().GetSchema().OriginalECXmlVersionLessThan(ECVersion::V3_0))
        minValueAttr = propertyNode.attribute("MinimumValue");
    else
        minValueAttr = propertyNode.attribute(ECXML_MINIMUM_VALUE_ATTRIBUTE);

    Utf8String minValue;
    if (minValueAttr)
        {
        minValue = minValueAttr.as_string();
        ECValue minECValue(minValue.c_str());
        PrimitiveType pt;
        resolvePrimitiveType(this, pt);

        if ((!minECValue.ConvertToPrimitiveType(pt) || ECObjectsStatus::Success != SetMinimumValue(minECValue)) &&
            GetContainerSchema()->OriginalECXmlVersionAtLeast(ECVersion::V3_0))
            {
            LOG.errorv("Cannot set the minimum value attribute on ECProperty, %s.%s, because the datatype is not supported. Minimum value can only be set on a property with a primitive type of double, int, or long.",
                GetClass().GetFullName(), GetName().c_str());
            return SchemaReadStatus::InvalidECSchemaXml;
            }
        }

    pugi::xml_attribute maxValueAttr;
    
    if (GetClass().GetSchema().OriginalECXmlVersionLessThan(ECVersion::V3_0))
        maxValueAttr = propertyNode.attribute("MaximumValue");
    else
        maxValueAttr = propertyNode.attribute(ECXML_MAXIMUM_VALUE_ATTRIBUTE);

    Utf8String maxValue;
    if (maxValueAttr)
        {
        maxValue = maxValueAttr.as_string();
        PrimitiveType pt;
        resolvePrimitiveType(this, pt);

        ECValue maxECValue(maxValue.c_str());

        if ((!maxECValue.ConvertToPrimitiveType(pt) || ECObjectsStatus::Success != SetMaximumValue(maxECValue)) &&
            GetContainerSchema()->OriginalECXmlVersionAtLeast(ECVersion::V3_0))
            {
            LOG.errorv("Cannot set the maximum value attribute on ECProperty, %s.%s, because the datatype is not supported. Maximum value can only be set on a property with a primitive type of double, int, or long.",
                GetClass().GetFullName(), GetName().c_str());
            return SchemaReadStatus::InvalidECSchemaXml;
            }
        }

    return SchemaReadStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
SchemaReadStatus ArrayECProperty::_ReadXml (pugi::xml_node propertyNode, ECSchemaReadContextR context)
    {
    SchemaReadStatus status = T_Super::_ReadXml (propertyNode, context);
    if (status != SchemaReadStatus::Success)
        return status;

    // OPTIONAL attributes - If these attributes exist they do not need to be valid.  We will ignore any errors setting them and use default values.
    // NEEDSWORK This is due to the current implementation in managed ECObjects.  We should reconsider whether it is the correct behavior.
    ECObjectsStatus setterStatus;   // needed for macro.
    Utf8String         value;          // needed for macro.
    READ_OPTIONAL_XML_ATTRIBUTE_IGNORING_SET_ERRORS (propertyNode, MIN_OCCURS_ATTRIBUTE, this, MinOccurs)
    READ_OPTIONAL_XML_ATTRIBUTE_IGNORING_SET_ERRORS (propertyNode, MAX_OCCURS_ATTRIBUTE, this, MaxOccurs)

    // For Primitive & Array properties we ignore parse errors and default to string. Struct properties will require a resolvable typename.
    status = ReadTypeNameWithPruning(propertyNode, this, context, true);
    if (status != SchemaReadStatus::Success)
        return status;

    status = ReadMinMaxXml(propertyNode);
    if (SchemaReadStatus::Success != status)
        return status;

    if (ECObjectsStatus::Success != setterStatus)
        {
        LOG.warningv ("Defaulting the type of ECProperty '%s' to '%s' in reaction to non-fatal parse error.", GetName().c_str(), GetTypeName().c_str());
        return SchemaReadStatus::Success;
        }

    return SchemaReadStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
SchemaWriteStatus ArrayECProperty::_WriteXml (BeXmlWriterR xmlWriter, ECVersion ecXmlVersion)
    {
    bvector<bpair<Utf8CP, Utf8CP>> additionalAttributes;

    Utf8PrintfString minOccursString("%u", m_minOccurs);
    additionalAttributes.push_back({ MIN_OCCURS_ATTRIBUTE, minOccursString.c_str() });

    Utf8String maxOccursString = m_maxOccurs != INT_MAX ? Utf8PrintfString("%u", m_maxOccurs) : Utf8String("unbounded");
    additionalAttributes.push_back({ MAX_OCCURS_ATTRIBUTE, maxOccursString.c_str() });

    // This is needed to keep the qualified name in scope for calling the super class below....
    Utf8String koqQualifiedName;

    Utf8CP elementName = EC_ARRAYPROPERTY_ELEMENT;
    if (m_arrayKind == ARRAYKIND_Struct)
        {
        if (ECVersion::V2_0 == ecXmlVersion)
            additionalAttributes.push_back(make_bpair(IS_STRUCT_ATTRIBUTE, "true"));
        else
            elementName = EC_STRUCTARRAYPROPERTY_ELEMENT;
        }
    else // ARRAYKIND_Primitive
        {
        if (this->GetAsPrimitiveArrayProperty()->IsExtendedTypeDefinedLocally())
            additionalAttributes.push_back(make_bpair(EXTENDED_TYPE_NAME_ATTRIBUTE, GetAsPrimitiveArrayProperty()->GetExtendedTypeName().c_str()));
        }

    return T_Super::_WriteXml (xmlWriter, elementName, ecXmlVersion, &additionalAttributes);
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------+---------------+---------------+---------------+---------------+-------
bool ArrayECProperty::_IsSame(ECPropertyCR target) const
    {
    ArrayECPropertyCP rhs = target.GetAsArrayProperty();
    if (nullptr == rhs)
        return false;

    if (!T_Super::_IsSame(target))
        return false;

    if (GetKind() == rhs->GetKind())
        return false;

    if (GetMaxOccurs() != rhs->GetMaxOccurs())
        return false;

    if (GetMinOccurs() != rhs->GetMinOccurs())
        return false;

    return true;
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
    int count = Utf8String::Sscanf_safe (minOccurs.c_str(), "%u", &iMinOccurs);
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
    int count = Utf8String::Sscanf_safe (maxOccurs.c_str(), "%u", &iMaxOccurs);
    if (count != 1)
        {
        if (0 == strcmp (maxOccurs.c_str(), ECXML_UNBOUNDED))
            iMaxOccurs = INT_MAX;
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
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool PrimitiveArrayECProperty::_CanOverride (ECPropertyCR baseProperty, Utf8StringR errMsg) const
    {
    if (!baseProperty.GetIsPrimitive() && !baseProperty.GetIsPrimitiveArray())
        {
        errMsg.Sprintf("The property %s:%s cannot be overriden by PrimitiveArrayECProperty %s:%s because it is not a PrimitiveArrayECProperty.",
                   baseProperty.GetClass().GetFullName(), baseProperty.GetName().c_str(), GetClass().GetFullName(), GetName().c_str());
        return false;
        }

    // Apparently this is a thing...overriding a primitive property with a primitive array of same type.
    PrimitiveType basePrimitiveType;
    if (baseProperty.GetIsPrimitive())
        basePrimitiveType = baseProperty.GetAsPrimitiveProperty()->GetType();
    else
        basePrimitiveType = baseProperty.GetAsPrimitiveArrayProperty()->GetPrimitiveElementType();

    if (basePrimitiveType != m_primitiveType)
        {
        errMsg.Sprintf("The ECProperty %s:%s cannot override the base property %s:%s as they have differing types (%s vs. %s).",
                   GetClass().GetFullName(), GetName().c_str(), baseProperty.GetClass().GetFullName(), baseProperty.GetName().c_str(),
                       SchemaParseUtils::PrimitiveTypeToString(m_primitiveType), SchemaParseUtils::PrimitiveTypeToString(basePrimitiveType));
        return false;
        }

    return isKindOfQuantityCompatible(*this, &baseProperty, this->GetKindOfQuantity());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String PrimitiveArrayECProperty::_GetTypeName (bool useFullName) const
    {
    if (nullptr == m_enumeration)
        return SchemaParseUtils::PrimitiveTypeToString (m_primitiveType);
    if (useFullName)
        return ECJsonUtilities::FormatEnumerationName(*m_enumeration);
    return SchemaParseUtils::GetQualifiedName<ECEnumeration>(this->GetClass().GetSchema(), *m_enumeration);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String PrimitiveArrayECProperty::_GetTypeNameForXml(ECVersion ecXmlVersion) const
    {
    if (ecXmlVersion <= ECVersion::V2_0 && m_enumeration != nullptr)
        return m_enumeration->GetTypeName();

    return GetTypeName();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus PrimitiveArrayECProperty::_SetTypeName(Utf8StringCR typeName)
    {
    PrimitiveType primitiveType;
    ECObjectsStatus status = SchemaParseUtils::ParsePrimitiveType (primitiveType, typeName);
    if (ECObjectsStatus::Success == status)
        return SetPrimitiveElementType (primitiveType);

    ECEnumerationCP enumeration;
    if (ECObjectsStatus::Success == ResolveEnumerationType(enumeration, typeName, this->GetClass().GetSchema()))
        {
        return SetType(*enumeration);
        }

    LOG.warningv ("TypeName '%s' of '%s.%s.%s' was not recognized. We will use 'string' instead.",
                                    typeName.c_str(),
                                    GetClass().GetSchema().GetName().c_str(),
                                    GetClass().GetName().c_str(),
                                    GetName().c_str() );
    return ECObjectsStatus::ParseError;
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------+---------------+---------------+---------------+---------------+-------
ECObjectsStatus PrimitiveArrayECProperty::SetType(PrimitiveType value)
    {
    return SetPrimitiveElementType(value);
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------+---------------+---------------+---------------+---------------+-------
ECObjectsStatus PrimitiveArrayECProperty::SetPrimitiveElementType(PrimitiveType value)
    {
    m_arrayKind = ARRAYKIND_Primitive;
    m_primitiveType = value;

    _AdjustMinMaxAfterTypeChange();
    InvalidateClassLayout();

    return ECObjectsStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
 @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus PrimitiveArrayECProperty::SetType (ECEnumerationCR enumerationType)
    {
    if (&(enumerationType.GetSchema()) != &(this->GetClass().GetSchema()))
        {
        if (!ECSchema::IsSchemaReferenced(this->GetClass().GetSchema(), enumerationType.GetSchema()))
            return ECObjectsStatus::SchemaNotFound;
        }

    auto primitiveType = enumerationType.GetType();
    if (m_primitiveType != primitiveType)
        {
        m_primitiveType = primitiveType;
        InvalidateClassLayout();
        }

    m_enumeration = &enumerationType;

    return ECObjectsStatus::Success;
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------+---------------+---------------+---------------+---------------+-------
Utf8StringCR PrimitiveArrayECProperty::GetExtendedTypeName() const
    {
    if (!IsExtendedTypeDefinedLocally())
        {
        if (HasBaseProperty())
            {
            PrimitiveArrayECPropertyCP baseExtended = GetBaseProperty()->GetAsPrimitiveArrayProperty();
            if (nullptr != baseExtended)
                return baseExtended->GetExtendedTypeName();
            }
        }

    return m_extendedTypeName;
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------+---------------+---------------+---------------+---------------+-------
ECObjectsStatus PrimitiveArrayECProperty::SetExtendedTypeName(Utf8CP extendedTypeName)
    {
    m_extendedTypeName.clear();
    if (!Utf8String::IsNullOrEmpty(extendedTypeName))
        m_extendedTypeName.assign(extendedTypeName);

    return ECObjectsStatus::Success;
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
SchemaReadStatus PrimitiveArrayECProperty::_ReadXml(pugi::xml_node propertyNode, ECSchemaReadContextR context)
    {
    SchemaReadStatus status = T_Super::_ReadXml(propertyNode, context);
    if (status != SchemaReadStatus::Success)
        return status;

    auto extendedTypeAttr = propertyNode.attribute(EXTENDED_TYPE_NAME_ATTRIBUTE);
    if (extendedTypeAttr)
        {
        this->SetExtendedTypeName(extendedTypeAttr.as_string());
        }

    _AdjustMinMaxAfterTypeChange();
    return SchemaReadStatus::Success;
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------+---------------+---------------+---------------+---------------+-------
bool PrimitiveArrayECProperty::_ToJson(BeJsValue outValue, bool isInherited) const
    {
    bvector<bpair<Utf8String, Json::Value>> attributes;
    WriteCommonPrimitivePropertyJsonAttributes(attributes, this);
    return T_Super::_ToJson(outValue, isInherited, attributes);
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------+---------------+---------------+---------------+---------------+-------
bool StructArrayECProperty::_CanOverride (ECPropertyCR baseProperty, Utf8StringR errMsg) const
    {
    if (!baseProperty.GetIsStructArray())
        {
        errMsg.Sprintf("The property %s:%s cannot be overriden by StructArrayECProperty %s:%s because it is not a StructArrayECProperty.",
                   baseProperty.GetClass().GetFullName(), baseProperty.GetName().c_str(), GetClass().GetFullName(), GetName().c_str());
        return false;
        }

    // if the struct type hasn't been set yet, we will say it cannot override
    if (nullptr == m_structType)
        return false;

    // This used to always compare GetTypeName(). Type names for struct arrays include the alias as defined in the referencing schema. That is weird and easily breaks if:
    //  -Base property is defined in same schema as the struct class (cannot be worked around), or
    //  -Base property's schema declares different alias for struct class's schema than the overriding property's schema (dumb workaround: make them use the same alias).
    // Instead, compare the full-qualified class name.
    Utf8String baseStructName = baseProperty.GetAsStructArrayProperty()->GetStructElementType().GetFullName();
    if (0 != strcmp(m_structType->GetFullName(), baseStructName.c_str()))
        {
        errMsg.Sprintf("The StructArrayECProperty %s:%s with type %s cannot be overriden by %s:%s with type %s because they have different types.",
                   baseProperty.GetClass().GetFullName(), baseProperty.GetName().c_str(), baseStructName.c_str(),
                   GetClass().GetFullName(), GetName().c_str(), m_structType->GetFullName());
        return false;
        }

    return isKindOfQuantityCompatible(*this, &baseProperty, this->GetKindOfQuantity());
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------+---------------+---------------+---------------+---------------+-------
ECObjectsStatus StructArrayECProperty::_SetTypeName(Utf8StringCR typeName)
    {
    ECStructClassCP structClass;
    ECObjectsStatus status = ResolveStructType(structClass, typeName, this->GetClass());
    if (ECObjectsStatus::Success == status)
        return SetStructElementType(*structClass);

    LOG.warningv("TypeName '%s' of '%s.%s.%s' was not recognized. We will use 'string' instead.",
                 typeName.c_str(),
                 GetClass().GetSchema().GetName().c_str(),
                 GetClass().GetName().c_str(),
                 GetName().c_str());
    return ECObjectsStatus::ParseError;
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------+---------------+---------------+---------------+---------------+-------
bool StructArrayECProperty::_ToJson(BeJsValue outValue, bool isInherited) const
    {
    bvector<bpair<Utf8String, Json::Value>> attributes;
    attributes.push_back(bpair<Utf8String, Json::Value>(TYPE_NAME_ATTRIBUTE, GetTypeFullName()));
    return T_Super::_ToJson(outValue, isInherited, attributes);
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------+---------------+---------------+---------------+---------------+-------
Utf8String StructArrayECProperty::_GetTypeName(bool useFullName) const
    {
    if (useFullName)
        return ECJsonUtilities::FormatClassName(*m_structType);
    return ECClass::GetQualifiedClassName(this->GetClass().GetSchema(), *m_structType);
    }

/*---------------------------------------------------------------------------------**//**
 @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ECStructClassCR StructArrayECProperty::GetStructElementType () const
    {
    DEBUG_EXPECT(NULL != m_structType);
    return *m_structType;
    }

/*---------------------------------------------------------------------------------**//**
 @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus StructArrayECProperty::SetStructElementType (ECStructClassCR structType)
    {
    if (&(structType.GetSchema()) != &(this->GetClass().GetSchema()))
        {
        if (!ECSchema::IsSchemaReferenced(this->GetClass().GetSchema(), structType.GetSchema()))
            return ECObjectsStatus::SchemaNotFound;
        }

    m_arrayKind = ARRAYKIND_Struct;
    if (m_structType != &structType)
        {
        m_structType = &structType;
        InvalidateClassLayout();
        }

    return ECObjectsStatus::Success;
    }

//---------------------------------------------------------------------------------------
// @bsimethod
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
// @bsimethod
//---------------+---------------+---------------+---------------+---------------+-------
ECObjectsStatus NavigationECProperty::SetDirection(Utf8CP direction)
    {
    PRECONDITION(nullptr != direction, ECObjectsStatus::PreconditionViolated);

    ECObjectsStatus status = SchemaParseUtils::ParseDirectionString(m_direction, direction);
    if (ECObjectsStatus::Success != status)
        LOG.errorv("Failed to parse the ECRelatedInstanceDirection string '%s' for NavigationECProperty '%s'.", direction, GetName().c_str());

    m_valueKind = ValueKind::VALUEKIND_Uninitialized;

    return status;
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------+---------------+---------------+---------------+---------------+-------
Utf8String NavigationECProperty::GetRelationshipClassName() const
    {
    if (!EXPECTED_CONDITION(nullptr != m_relationshipClass))
        return EMPTY_STRING;
    return ECClass::GetQualifiedClassName(this->GetClass().GetSchema(), *m_relationshipClass);
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------+---------------+---------------+---------------+---------------+-------
bool NavigationECProperty::Verify()
    {
    if (IsVerified())
        return true;

    if (nullptr == m_relationshipClass)
        return false;

    for (const auto& prop : GetClass().GetProperties(false))
        if (prop != this && prop->GetIsNavigation() && prop->GetAsNavigationProperty()->GetRelationshipClass() == m_relationshipClass)
            {
            LOG.errorv("'%s.%s' is invalid because '%s.%s' already references relationship '%s'.",
                GetClass().GetFullName(), GetName().c_str(), GetClass().GetFullName(), prop->GetName().c_str(), m_relationshipClass->GetFullName());
            return false;
            }

    if (m_relationshipClass->HasBaseClasses())
        {
        LOG.errorv("The referenced relationship '%s' used in NavigationECProperty %s.%s is not the root relationship.",
            m_relationshipClass->GetFullName(), GetClass().GetFullName(), GetName().c_str());
        return false;
        }

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

    if (nullptr != thatConstraint->GetAbstractConstraint() && thatConstraint->GetAbstractConstraint()->IsRelationshipClass())
        return false;

    // If this property is an override the only thing that matters is if the override is valid.
    // If it is not an override the class holding this navigation property must be explicitly listed as a constraint class
    if (HasBaseProperty())
        {
        Utf8String errorMessage;
        if (!_CanOverride(*GetBaseProperty(), errorMessage))
            {
            // NOTE: _CanOverride was called when the property was added to the class but due to deserialization order the relationship class may not have been fully loaded then 
            // so the check would have returned before checking all details and another call is required here to ensure it is a valid override.
            LOG.error(errorMessage.c_str());
            return false;
            }
        }
    else
        {
        auto const& constraintClasses = thisConstraint->GetConstraintClasses();
        auto propClass = &GetClass();
        bool isConstraint = std::any_of(constraintClasses.begin(), constraintClasses.end(), [&propClass](ECClassCP constraint){ return ECClass::ClassesAreEqualByName(propClass, constraint); } );
        if (!isConstraint)
            {
            LOG.errorv("The navigation property '%s.%s' cannot be added to '%s' because the class is not a constraint class in the referenced relationship '%s'",
                GetClass().GetFullName(), GetName().c_str(), GetClass().GetFullName(), m_relationshipClass->GetFullName());
            return false;
            }
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
        return false;

    return true;
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------+---------------+---------------+---------------+---------------+-------
SchemaReadStatus NavigationECProperty::_ReadXml(pugi::xml_node propertyNode, ECSchemaReadContextR schemaContext)
    {
    SchemaReadStatus status = T_Super::_ReadXml(propertyNode, schemaContext);
    if (status != SchemaReadStatus::Success)
        return status;

    // relationshipName and direction are required properties
    Utf8String value; // neede for macro.
    READ_REQUIRED_XML_ATTRIBUTE (propertyNode, RELATIONSHIP_NAME_ATTRIBUTE, this, RelationshipClassName, propertyNode.name())

    READ_REQUIRED_XML_ATTRIBUTE (propertyNode, DIRECTION_ATTRIBUTE, this, Direction, propertyNode.name())

    return SchemaReadStatus::Success;
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------+---------------+---------------+---------------+---------------+-------
SchemaWriteStatus NavigationECProperty::_WriteXml(BeXmlWriterR xmlWriter, ECVersion ecXmlVersion)
    {
    if (ECVersion::V2_0 == ecXmlVersion)
        return T_Super::_WriteXml(xmlWriter, EC_PROPERTY_ELEMENT, ecXmlVersion);

    bvector<bpair<Utf8CP, Utf8CP>> additionalAttributes;
    auto relClassName = GetRelationshipClassName(); // must be automatic variable to hold value in scope until this routine returns
    additionalAttributes.push_back(make_bpair(RELATIONSHIP_NAME_ATTRIBUTE, relClassName.c_str()));
    additionalAttributes.push_back(make_bpair(DIRECTION_ATTRIBUTE, SchemaParseUtils::DirectionToXmlString(m_direction)));

    return T_Super::_WriteXml(xmlWriter, EC_NAVIGATIONPROPERTY_ELEMENT, ecXmlVersion, &additionalAttributes, false);
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------+---------------+---------------+---------------+---------------+-------
bool NavigationECProperty::_ToJson(BeJsValue outValue, bool isInherited) const
    {
    bvector<bpair<Utf8String, Json::Value>> attributes;

    attributes.push_back(bpair<Utf8String, Json::Value>(RELATIONSHIP_NAME_ATTRIBUTE, ECJsonUtilities::FormatClassName(*GetRelationshipClass())));

    ECRelatedInstanceDirection direction = GetDirection();
    Utf8String directionString;
    attributes.push_back(bpair<Utf8String, Json::Value>(DIRECTION_ATTRIBUTE, SchemaParseUtils::DirectionToJsonString(direction)));

    return T_Super::_ToJson(outValue, isInherited, attributes);
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------+---------------+---------------+---------------+---------------+-------
Utf8String NavigationECProperty::_GetTypeName(bool useFullName) const { return SchemaParseUtils::PrimitiveTypeToString(m_type); }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------+---------------+---------------+---------------+---------------+-------
bool NavigationECProperty::_CanOverride(ECPropertyCR baseProperty, Utf8StringR errMsg) const
    {
    NavigationECPropertyCP baseNavProperty = baseProperty.GetAsNavigationProperty();
    if (nullptr == baseNavProperty)
        {
        errMsg.Sprintf("The property %s:%s cannot be overridden by a NavigationECProperty %s:%s because it is not a NavigationECProperty.",
                   baseProperty.GetClass().GetFullName(), baseProperty.GetName().c_str(), GetClass().GetFullName(), GetName().c_str());
        return false;
        }

    ECRelatedInstanceDirection baseDirection = baseNavProperty->GetDirection();
    if (GetDirection() != baseDirection)
        {
        errMsg.Sprintf("The NavigationECProperty %s:%s cannot be overridden by %s:%s because they have different directions.",
                   baseNavProperty->GetClass().GetFullName(), baseNavProperty->GetName().c_str(), GetClass().GetFullName(), GetName().c_str());
        return false;
        }

    // Following the example of StructECProperty we will allow override if the current relationship has not yet been set.
    if (nullptr == m_relationshipClass)
        return true;

    ECRelationshipClassCP baseRelClass = baseNavProperty->GetRelationshipClass();
    if (!ECClass::ClassesAreEqualByName(m_relationshipClass, baseRelClass))
        {
        errMsg.Sprintf("The NavigationECProperty %s:%s cannot be overridden by %s:%s because the relationship was changed. A derived property cannot change the referenced relationship.",
            baseNavProperty->GetClass().GetFullName(), baseNavProperty->GetName().c_str(), GetClass().GetFullName(), GetName().c_str());
        return false;
        }

    return true;
    }

//---------------------------------------------------------------------------------------
// @bsimethod
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

    InvalidateClassLayout();

    return ECObjectsStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void ECProperty::InvalidateClassLayout()
    {
    // TFS#290587: When a property is modified in a way that affects the ClassLayout, must
    // invalidate the containing class's default standalone enabler
    m_class.InvalidateDefaultStandaloneEnabler();
    }

END_BENTLEY_ECOBJECT_NAMESPACE
