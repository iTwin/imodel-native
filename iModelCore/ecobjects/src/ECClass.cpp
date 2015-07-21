/*--------------------------------------------------------------------------------------+
|
|     $Source: src/ECClass.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
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
void ECClass::SetErrorHandling (bool doAssert) 
    { 
    s_noAssert = !doAssert; 
    ECProperty::SetErrorHandling(doAssert);
    }

/*---------------------------------------------------------------------------------**//**
 @bsimethod                                                 
+---------------+---------------+---------------+---------------+---------------+------*/
ECClass::ECClass (ECSchemaCR schema)
    :
    m_schema(schema), m_isStruct(false), m_isCustomAttributeClass(false), m_isDomainClass(true), m_ecClassId(0)
    {
    //
    };

/*---------------------------------------------------------------------------------**//**
 @bsimethod                                                 
+---------------+---------------+---------------+---------------+---------------+------*/
ECClass::~ECClass ()
    {
    m_propertyList.clear();
    
    for (PropertyMap::iterator entry=m_propertyMap.begin(); entry != m_propertyMap.end(); ++entry)
        delete entry->second;
    
    m_propertyMap.clear();

    RemoveBaseClasses ();
    RemoveDerivedClasses ();

    m_defaultStandaloneEnabler = NULL;
    }

/*---------------------------------------------------------------------------------**//**
 @bsimethod                                                     
+---------------+---------------+---------------+---------------+---------------+------*/
WStringCR ECClass::GetName () const
    {        
    return m_validatedName.GetName();
    }

/*---------------------------------------------------------------------------------**//**
 @bsimethod                                                      Affan.Khan        12/12
+---------------+---------------+---------------+---------------+---------------+------*/
ECClassId ECClass::GetId () const
    {
    BeAssert (HasId());
    return m_ecClassId;
    }

/*---------------------------------------------------------------------------------**//**
 @bsimethod                                                     
+---------------+---------------+---------------+---------------+---------------+------*/
WCharCP ECClass::GetFullName () const
    {
    if (m_fullName.size() == 0)
        m_fullName = GetSchema().GetName() + L":" + GetName();
        
    return m_fullName.c_str();
    }
    
/*---------------------------------------------------------------------------------**//**
 @bsimethod                                                     
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus ECClass::SetName (WStringCR name)
    {
    m_validatedName.SetName (name.c_str());
    m_fullName = GetSchema().GetName() + L":" + GetName();
    
    return ECOBJECTS_STATUS_Success;
    }

/*---------------------------------------------------------------------------------**//**
 @bsimethod                                                     
+---------------+---------------+---------------+---------------+---------------+------*/
WStringCR ECClass::GetDescription () const
    {
    return GetSchema().GetLocalizedStrings().GetClassDescription(this, m_description);
    }

/*---------------------------------------------------------------------------------**//**
 @bsimethod                                                     
+---------------+---------------+---------------+---------------+---------------+------*/
WStringCR ECClass::GetInvariantDescription () const
    {
    return m_description;
    }

/*---------------------------------------------------------------------------------**//**
 @bsimethod                                                     
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus ECClass::SetDescription (WStringCR description)
    {        
    m_description = description;
    return ECOBJECTS_STATUS_Success;
    }

/*---------------------------------------------------------------------------------**//**
 @bsimethod                                                     
+---------------+---------------+---------------+---------------+---------------+------*/
WStringCR ECClass::GetDisplayLabel () const
    {
    return GetSchema().GetLocalizedStrings().GetClassDisplayLabel(this, GetInvariantDisplayLabel());
    }

/*---------------------------------------------------------------------------------**//**
 @bsimethod                                                     
+---------------+---------------+---------------+---------------+---------------+------*/
WStringCR ECClass::GetInvariantDisplayLabel() const
    {
    return m_validatedName.GetDisplayLabel();
    }

/*---------------------------------------------------------------------------------**//**
 @bsimethod                                                     
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus ECClass::SetDisplayLabel (WStringCR displayLabel)
    {        
    m_validatedName.SetDisplayLabel (displayLabel.c_str());
    return ECOBJECTS_STATUS_Success;
    }

/*---------------------------------------------------------------------------------**//**
 @bsimethod                                                     
+---------------+---------------+---------------+---------------+---------------+------*/
bool ECClass::GetIsDisplayLabelDefined () const
    {
    return m_validatedName.IsDisplayLabelDefined();
    }

/*---------------------------------------------------------------------------------**//**
 @bsimethod                                                     
+---------------+---------------+---------------+---------------+---------------+------*/
bool ECClass::GetIsStruct () const
    {
    return m_isStruct; 
    }

/*---------------------------------------------------------------------------------**//**
 @bsimethod                                                     
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus ECClass::SetIsStruct (bool isStruct)
    {        
    m_isStruct = isStruct;
    return ECOBJECTS_STATUS_Success;
    }

/*---------------------------------------------------------------------------------**//**
 @bsimethod                                                     
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus ECClass::SetIsStruct (WCharCP isStruct)
    {        
    PRECONDITION (NULL != isStruct, ECOBJECTS_STATUS_PreconditionViolated);

    ECObjectsStatus status = ECXml::ParseBooleanString (m_isStruct, isStruct);
    if (ECOBJECTS_STATUS_Success != status)
        LOG.warningv  (L"Failed to parse the isStruct string '%ls' for ECClass '%ls'.  Expected values are True or False", isStruct, this->GetName().c_str());
        
    return status;
    }

/*---------------------------------------------------------------------------------**//**
 @bsimethod                                                     
+---------------+---------------+---------------+---------------+---------------+------*/
bool ECClass::GetIsCustomAttributeClass () const
    {
    return m_isCustomAttributeClass; 
    }

/*---------------------------------------------------------------------------------**//**
 @bsimethod                                                     
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus ECClass::SetIsCustomAttributeClass (bool isCustomAttributeClass)
    {        
    m_isCustomAttributeClass = isCustomAttributeClass;
    return ECOBJECTS_STATUS_Success;
    }

/*---------------------------------------------------------------------------------**//**
 @bsimethod                                                     
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus ECClass::SetIsCustomAttributeClass (WCharCP isCustomAttributeClass)
    {      
    PRECONDITION (NULL != isCustomAttributeClass, ECOBJECTS_STATUS_PreconditionViolated);

    ECObjectsStatus status = ECXml::ParseBooleanString (m_isCustomAttributeClass, isCustomAttributeClass);
    if (ECOBJECTS_STATUS_Success != status)
        LOG.warningv  (L"Failed to parse the isCustomAttributeClass string '%ls' for ECClass '%ls'.  Expected values are True or False", isCustomAttributeClass, this->GetName().c_str());
        
    return status;
    }

/*---------------------------------------------------------------------------------**//**
 @bsimethod                                                     
+---------------+---------------+---------------+---------------+---------------+------*/
bool ECClass::GetIsDomainClass () const
    {
    return m_isDomainClass; 
    }

/*---------------------------------------------------------------------------------**//**
 @bsimethod                                                     
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus ECClass::SetIsDomainClass (bool isDomainClass)
    {        
    m_isDomainClass = isDomainClass;
    return ECOBJECTS_STATUS_Success;
    }

/*---------------------------------------------------------------------------------**//**
 @bsimethod                                                     
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus ECClass::SetIsDomainClass (WCharCP isDomainClass)
    {
    PRECONDITION (NULL != isDomainClass, ECOBJECTS_STATUS_PreconditionViolated);

    ECObjectsStatus status = ECXml::ParseBooleanString (m_isDomainClass, isDomainClass);
    if (ECOBJECTS_STATUS_Success != status)
        LOG.warningv  (L"Failed to parse the isDomainClass string '%ls' for ECClass '%ls'.  Expected values are True or False", isDomainClass, this->GetName().c_str());
        
    return status;
    }

/*---------------------------------------------------------------------------------**//**
 @bsimethod                                                     
+---------------+---------------+---------------+---------------+---------------+------*/
ECSchemaCR ECClass::GetSchema () const
    {
    return m_schema;
    }

/*---------------------------------------------------------------------------------**//**
 @bsimethod                                                     
+---------------+---------------+---------------+---------------+---------------+------*/
ECRelationshipClassCP ECClass::GetRelationshipClassCP() const
    {
    return _GetRelationshipClassCP();
    }

/*---------------------------------------------------------------------------------**//**
 @bsimethod                                                     
+---------------+---------------+---------------+---------------+---------------+------*/
ECRelationshipClassP ECClass::GetRelationshipClassP()
    {
    return _GetRelationshipClassP();
    }

/*---------------------------------------------------------------------------------**//**
 @bsimethod                                                     
+---------------+---------------+---------------+---------------+---------------+------*/
StandaloneECEnablerP ECClass::GetDefaultStandaloneEnabler() const
    {
    if (!m_defaultStandaloneEnabler.IsValid())
        {
        ClassLayoutPtr classLayout   = ClassLayout::BuildFromClass (*this);
        m_defaultStandaloneEnabler = StandaloneECEnabler::CreateEnabler (*this, *classLayout, NULL);
        }

    BeAssert(m_defaultStandaloneEnabler.IsValid());
    return m_defaultStandaloneEnabler.get();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   03/15
+---------------+---------------+---------------+---------------+---------------+------*/
void ECClass::OnBaseClassPropertyRemoved (ECPropertyCR baseProperty)
    {
    InvalidateDefaultStandaloneEnabler();
    auto found = std::find_if (m_propertyList.begin(), m_propertyList.end(), [&baseProperty](ECPropertyCP arg) { return arg->GetBaseProperty() == &baseProperty; });
    if (m_propertyList.end() != found)
        (*found)->SetBaseProperty (baseProperty.GetBaseProperty());
    else
        {
        for (ECClassP derivedClass : m_derivedClasses)
            derivedClass->OnBaseClassPropertyRemoved (baseProperty);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   03/13
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus ECClass::RemoveProperty (ECPropertyR prop)
    {
    PropertyMap::iterator iter = m_propertyMap.find (prop.GetName().c_str());
    if (iter == m_propertyMap.end() || iter->second != &prop)
        return ECOBJECTS_STATUS_PropertyNotFound;

    m_propertyMap.erase (iter);
    m_propertyList.erase (std::find (m_propertyList.begin(), m_propertyList.end(), &prop));

    InvalidateDefaultStandaloneEnabler();

    for (ECClassP derivedClass : m_derivedClasses)
        derivedClass->OnBaseClassPropertyRemoved (prop);

    return ECOBJECTS_STATUS_Success;
    }
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   03/13
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus ECClass::DeleteProperty (ECPropertyR prop)
    {
    ECObjectsStatus status = RemoveProperty (prop);
    if (ECOBJECTS_STATUS_Success == status)
        delete &prop;

    return status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   03/13
+---------------+---------------+---------------+---------------+---------------+------*/
ECPropertyP ECClass::GetPropertyByIndex (uint32_t index) const
    {
    if (index >= (uint32_t)m_propertyList.size())
        return NULL;

    return m_propertyList[index];
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   03/13
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus ECClass::ReplaceProperty (ECPropertyP& newProperty, ValueKind kind, ECPropertyR propertyToRemove)
    {
    if (HasBaseClasses() || 0 < m_derivedClasses.size())
        return ECOBJECTS_STATUS_OperationNotSupported;

    newProperty = NULL;

    uint32_t propertyIndex = -1;
    for (size_t i = 0; i < m_propertyList.size(); i++)
        {
        if (m_propertyList[i] == &propertyToRemove)
            {
            propertyIndex = (uint32_t)i;
            break;
            }
        }

    if (-1 == propertyIndex)
        return ECOBJECTS_STATUS_PropertyNotFound;

    switch (kind)
        {
    case VALUEKIND_Primitive:   newProperty = new PrimitiveECProperty (*this); break;
    case VALUEKIND_Array:       newProperty = new ArrayECProperty (*this); break;
    case VALUEKIND_Struct:      newProperty = new StructECProperty (*this); break;
    default:                    return ECOBJECTS_STATUS_Error;
        }

    m_propertyMap.erase (m_propertyMap.find (propertyToRemove.GetName().c_str()));

    newProperty->SetName (propertyToRemove.GetName());
    m_propertyMap[newProperty->GetName().c_str()] = newProperty;
    m_propertyList[propertyIndex] = newProperty;

    delete &propertyToRemove;

    InvalidateDefaultStandaloneEnabler();

    return ECOBJECTS_STATUS_Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   03/13
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus ECClass::RenameProperty (ECPropertyR prop, WCharCP newName)
    {
    if (HasBaseClasses() || 0 < m_derivedClasses.size())
        return ECOBJECTS_STATUS_OperationNotSupported;

    ECObjectsStatus status = RemoveProperty (prop);
    if (ECOBJECTS_STATUS_Success == status)
        {
        ECPropertyP propertyP = &prop;
        WString oldName = prop.GetName();

        status = prop.SetName (newName);
        if (ECOBJECTS_STATUS_Success == status)
            {
            status = AddProperty (propertyP);
            if (ECOBJECTS_STATUS_Success != status)
                {
                // Failed to add (duplicate name?) Add back with the old name
                prop.SetName (oldName);
                AddProperty (propertyP);
                }
            }
        else
            {
            // Failed to rename, add it back with the existing name
            AddProperty (propertyP);
            }
        }

    return status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   03/13
+---------------+---------------+---------------+---------------+---------------+------*/
void ECClass::InvalidateDefaultStandaloneEnabler() const
    {
    // When class structure changes, the ClassLayout stored in this enabler becomes out-of-date
    // nullify it so it will be reconstructed on next call to GetDefaultStandaloneEnabler()
    m_defaultStandaloneEnabler = NULL;
    for (ECClassP derivedClass : m_derivedClasses)
        derivedClass->InvalidateDefaultStandaloneEnabler();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   03/15
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus ECClass::OnBaseClassPropertyAdded (ECPropertyCR baseProperty)
    {
    InvalidateDefaultStandaloneEnabler();
    ECPropertyP derivedProperty = GetPropertyP (baseProperty.GetName(), false);
    ECObjectsStatus status = ECOBJECTS_STATUS_Success;
    if (nullptr != derivedProperty)
        {
        if (ECOBJECTS_STATUS_Success == (status = CanPropertyBeOverridden (baseProperty, *derivedProperty)))
            derivedProperty->SetBaseProperty (&baseProperty);
        }
    else
        {
        for (ECClassP derivedClass : m_derivedClasses)
            status = derivedClass->OnBaseClassPropertyAdded (baseProperty);
        }

    return status;
    }

/*---------------------------------------------------------------------------------**//**
 @bsimethod                                                     
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus ECClass::AddProperty (ECPropertyP& pProperty)
    {
    PropertyMap::const_iterator propertyIterator = m_propertyMap.find(pProperty->GetName().c_str());
    if (m_propertyMap.end() != propertyIterator)
        {
        LOG.warningv  (L"Can not create property '%ls' because it already exists in this ECClass", pProperty->GetName().c_str());
        return ECOBJECTS_STATUS_NamedItemAlreadyExists;
        }

    // It isn't part of this schema, but does it exist as a property on a baseClass?
    ECPropertyP baseProperty = GetPropertyP(pProperty->GetName());
    if (NULL != baseProperty)
        {
        ECObjectsStatus status = CanPropertyBeOverridden (*baseProperty, *pProperty);
        if (ECOBJECTS_STATUS_Success != status)
            return status;

        pProperty->SetBaseProperty (baseProperty);
        }

    m_propertyMap.insert (bpair<WCharCP, ECPropertyP> (pProperty->GetName().c_str(), pProperty));
    m_propertyList.push_back(pProperty);

    InvalidateDefaultStandaloneEnabler();

    for (ECClassP derivedClass : m_derivedClasses)
        derivedClass->OnBaseClassPropertyAdded (*pProperty);

    return ECOBJECTS_STATUS_Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Carole.MacDonald                05/2012
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus ECClass::CopyProperty
(
ECPropertyP& destProperty, 
ECPropertyCP sourceProperty,
bool copyCustomAttributes
)
    {
    if (sourceProperty->GetIsPrimitive())
        {
        PrimitiveECPropertyP destPrimitive;
        PrimitiveECPropertyCP sourcePrimitive = sourceProperty->GetAsPrimitiveProperty();
        destPrimitive = new PrimitiveECProperty(*this);
        destPrimitive->SetType(sourcePrimitive->GetType());

        destProperty = destPrimitive;
        }
    else if (sourceProperty->GetIsArray())
        {
        ArrayECPropertyP destArray;
        ArrayECPropertyCP sourceArray = sourceProperty->GetAsArrayProperty();
        destArray = new ArrayECProperty (*this);
        ECClassCP structElementType = sourceArray->GetStructElementType();
        if (structElementType != nullptr)
            destArray->SetStructElementType(structElementType);
        else
            destArray->SetPrimitiveElementType(sourceArray->GetPrimitiveElementType());

        destArray->SetMaxOccurs(sourceArray->GetMaxOccurs());
        destArray->SetMinOccurs(sourceArray->GetMinOccurs());

        destProperty = destArray;
        }
    else if (sourceProperty->GetIsStruct())
        {
        StructECPropertyP destStruct;
        StructECPropertyCP sourceStruct = sourceProperty->GetAsStructProperty();
        destStruct = new StructECProperty (*this);
        ECClassCR sourceType = sourceStruct->GetType();
        destStruct->SetType(sourceType);
        destProperty = destStruct;
        }

    destProperty->SetDescription(sourceProperty->GetInvariantDescription());
    if (sourceProperty->GetIsDisplayLabelDefined())
        destProperty->SetDisplayLabel(sourceProperty->GetInvariantDisplayLabel());
    destProperty->SetName(sourceProperty->GetName());
    destProperty->SetIsReadOnly(sourceProperty->GetIsReadOnly());
    if (copyCustomAttributes)
        sourceProperty->CopyCustomAttributesTo(*destProperty);

    ECObjectsStatus status = AddProperty(destProperty, sourceProperty->GetName());
    if (ECOBJECTS_STATUS_Success != status)
        delete destProperty;

    return status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Carole.MacDonald                01/2013
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus ECClass::CopyPropertyForSupplementation
(
ECPropertyP& destProperty, 
ECPropertyCP sourceProperty, 
bool copyCustomAttributes
)
    {
    ECObjectsStatus status = CopyProperty(destProperty, sourceProperty, copyCustomAttributes);
    if (ECOBJECTS_STATUS_Success == status)
        destProperty->m_forSupplementation = true;

    return status;
    }
/*---------------------------------------------------------------------------------**//**
 @bsimethod                                                     
+---------------+---------------+---------------+---------------+---------------+------*/
ECPropertyP ECClass::GetPropertyP
(
WCharCP propertyName,
bool includeBaseClasses
) const
    {
    PropertyMap::const_iterator  propertyIterator = m_propertyMap.find (propertyName);
    
    if ( propertyIterator != m_propertyMap.end() )
        return propertyIterator->second;

    if (!includeBaseClasses)
        return NULL;

    // not found yet, search the inheritence hierarchy
    for (const ECClassP& baseClass: m_baseClasses)
        {
        ECPropertyP baseProperty = baseClass->GetPropertyP (propertyName);
        if (NULL != baseProperty)
            return baseProperty;
        }
    return NULL;
    }

/*---------------------------------------------------------------------------------**//**
 @bsimethod                                                     
+---------------+---------------+---------------+---------------+---------------+------*/
ECPropertyP ECClass::GetPropertyP
(
WStringCR propertyName,
bool includeBaseClasses
) const
    {
    return  GetPropertyP (propertyName.c_str(), includeBaseClasses);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   12/12
+---------------+---------------+---------------+---------------+---------------+------*/
static int asciiCmp (WCharCP w, Utf8CP u)
    {
    while (*w)
        {
        // Do we care about endian-ness here?
        int cmp = (Utf8Char)(*w) - (*u);
        if (0 != cmp)
            return cmp;

        ++w;
        ++u;
        }

    // all characters in w match; check lengths match
    return 0 == *u ? 0 : -1;
    }

/*---------------------------------------------------------------------------------**//**
* On Graphite, Casey wants to be able to get a property by its name in Utf8, taking
* advantage of the fact that EC names are constrained to ASCII range.
* @bsimethod                                                    Paul.Connelly   12/12
+---------------+---------------+---------------+---------------+---------------+------*/
ECPropertyP ECClass::GetPropertyP (Utf8CP name, bool includeBaseClasses) const
    {
    struct Comparator
        {
        bool operator()(PropertyMap::value_type const& entry, Utf8CP u) const
            {
            return asciiCmp (entry.first, u) < 0;
            }
        };

    Comparator comp;
    PropertyMap::const_iterator found = std::lower_bound (m_propertyMap.begin(), m_propertyMap.end(), name, comp);
    if (m_propertyMap.end() != found && 0 == asciiCmp (found->first, name))
        {
        return found->second;
        }
    else if (includeBaseClasses)
        {
        for (ECClassCP ecClass: m_baseClasses)
            {
            ECPropertyP prop = ecClass->GetPropertyP (name, true);
            if (NULL != prop)
                return prop;
            }
        }

    return NULL;
    }

static const WCharCP s_schemasThatAllowOverridingArrays[] =
    {
    L"jclass.01",
    L"jclass.02",
    L"jclass.03",
    L"ECXA_ams.01",
    L"ECXA_ams_user.01",
    L"ams.01",
    L"ams_user.01",
    L"Bentley_JSpace_CustomAttributes.02",
    L"Bentley_Plant.06"
    };

static const size_t s_numSchemasThatAllowOverridingArrays = sizeof(s_schemasThatAllowOverridingArrays)/sizeof(WCharCP);

/*---------------------------------------------------------------------------------**//**
From .NET implementation:
///<summary>
///     Fixing defect D-33626 (Overriding a array property with a non-array property of the same type is allowed)
///     caused application breaks because some delivered schemas contained errors (non-array types were overriding
///     array types or vice-versa). So these schemas are included in an exception list and the
///     exception for overriding non-array type with array types is not thrown for these schemas.
///     Schemas that are part of this list are hard-coded
///
* @bsimethod                                    Carole.MacDonald                11/2011
+---------------+---------------+---------------+---------------+---------------+------*/
bool ECClass::SchemaAllowsOverridingArrays
(
ECSchemaCP schema
)
    {
    wchar_t buf[1024]; 

    BeStringUtilities::Snwprintf (buf, L"%ls.%02d", schema->GetName().c_str(), schema->GetVersionMajor());
    for (size_t i = 0; i < s_numSchemasThatAllowOverridingArrays; i++)
        if (0 == wcscmp (s_schemasThatAllowOverridingArrays[i], buf))
            return true;

    return false;
    }


/*---------------------------------------------------------------------------------**//**
 @bsimethod                                                     
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus ECClass::CanPropertyBeOverridden (ECPropertyCR baseProperty, ECPropertyCR newProperty) const
    {
    // If the type of base property is an array and the type of the current property is not an array (or vice-versa),
    // return an error immediately.  Unfortunately, there are a class of schemas that have been delivered with this type
    // of override.  So need to check if this is one of those schemas before returning an error
    if ((baseProperty.GetIsArray() && !newProperty.GetIsArray()) || (!baseProperty.GetIsArray() && newProperty.GetIsArray()))
        {
        if (!SchemaAllowsOverridingArrays(&this->GetSchema()))
            return ECOBJECTS_STATUS_DataTypeMismatch;
        }
    
    if (!newProperty._CanOverride(baseProperty))
        {
        LOG.errorv(L"The datatype of ECProperty %ls.%ls (%ls) does not match the datatype of ECProperty %ls.%ls (%ls)... which it overrides.", 
            newProperty.GetClass().GetFullName(), newProperty.GetName().c_str(), newProperty.GetTypeName().c_str(), 
            baseProperty.GetClass().GetFullName(), baseProperty.GetName().c_str(), baseProperty.GetTypeName().c_str());

        return ECOBJECTS_STATUS_DataTypeMismatch;
        }
    return ECOBJECTS_STATUS_Success; 
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Carole.MacDonald                03/2010
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus ECClass::RemoveProperty (WStringCR name)
    {
    PropertyMap::iterator  propertyIterator = m_propertyMap.find (name.c_str());
    
    if ( propertyIterator == m_propertyMap.end() )
        return ECOBJECTS_STATUS_ClassNotFound;
        
    ECPropertyP ecProperty = propertyIterator->second;
    return DeleteProperty (*ecProperty);
    }
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Carole.MacDonald                01/2010
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus ECClass::AddProperty (ECPropertyP ecProperty, WStringCR name)
    {
    ECObjectsStatus status = ecProperty->SetName (name);
    if (ECOBJECTS_STATUS_Success != status)
        return status;

    return AddProperty (ecProperty);
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Carole.MacDonald                01/2010
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus ECClass::CreatePrimitiveProperty (PrimitiveECPropertyP &ecProperty, WStringCR name)
    {
    ecProperty = new PrimitiveECProperty(*this);
    ECObjectsStatus status = AddProperty(ecProperty, name);
    if (status != ECOBJECTS_STATUS_Success)
        {
        delete ecProperty;
        ecProperty = NULL;
        return status;
        }
    return ECOBJECTS_STATUS_Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Carole.MacDonald                01/2010
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus ECClass::CreatePrimitiveProperty (PrimitiveECPropertyP &ecProperty, WStringCR name, PrimitiveType primitiveType)
    {
    ecProperty = new PrimitiveECProperty(*this);
    ecProperty->SetType(primitiveType);
    ECObjectsStatus status = AddProperty(ecProperty, name);
    if (status != ECOBJECTS_STATUS_Success)
        {
        delete ecProperty;
        ecProperty = NULL;
        return status;
        }
    return ECOBJECTS_STATUS_Success;
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Carole.MacDonald                01/2010
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus ECClass::CreateStructProperty (StructECPropertyP &ecProperty, WStringCR name)
    {
    ecProperty = new StructECProperty(*this);
    ECObjectsStatus status = AddProperty(ecProperty, name);
    if (status != ECOBJECTS_STATUS_Success)
        {
        delete ecProperty;
        ecProperty = NULL;
        return status;
        }
    return ECOBJECTS_STATUS_Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Carole.MacDonald                01/2010
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus ECClass::CreateStructProperty (StructECPropertyP &ecProperty, WStringCR name, ECClassCR structType)
    {
    ecProperty = new StructECProperty(*this);
    ECObjectsStatus status = ecProperty->SetType(structType);
    if (ECOBJECTS_STATUS_Success == status)
        status = AddProperty(ecProperty, name);
    if (ECOBJECTS_STATUS_Success != status)
        {
        delete ecProperty;
        ecProperty = NULL;
        return status;
        }
    return ECOBJECTS_STATUS_Success;
    }
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Carole.MacDonald                01/2010
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus ECClass::CreateArrayProperty (ArrayECPropertyP &ecProperty, WStringCR name)
    {
    ecProperty = new ArrayECProperty(*this);
    ECObjectsStatus status = AddProperty(ecProperty, name);
    if (status != ECOBJECTS_STATUS_Success)
        {
        delete ecProperty;
        ecProperty = NULL;
        return status;
        }
    return ECOBJECTS_STATUS_Success;
    }
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Carole.MacDonald                01/2010
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus ECClass::CreateArrayProperty (ArrayECPropertyP &ecProperty, WStringCR name, PrimitiveType primitiveType)
    {
    ecProperty = new ArrayECProperty(*this);
    ecProperty->SetPrimitiveElementType (primitiveType);
    ECObjectsStatus status = AddProperty(ecProperty, name);
    if (status != ECOBJECTS_STATUS_Success)
        {
        delete ecProperty;
        ecProperty = NULL;
        return status;
        }
    return ECOBJECTS_STATUS_Success;
    }
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Carole.MacDonald                01/2010
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus ECClass::CreateArrayProperty (ArrayECPropertyP &ecProperty, WStringCR name, ECClassCP structType)
    {
    ecProperty = new ArrayECProperty(*this);
    ECObjectsStatus status = ecProperty->SetStructElementType(structType);
    if (ECOBJECTS_STATUS_Success == status)
        status = AddProperty(ecProperty, name);
    if (ECOBJECTS_STATUS_Success != status)
        {
        delete ecProperty;
        ecProperty = NULL;
        return status;
        }
    return ECOBJECTS_STATUS_Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    09/10
+---------------+---------------+---------------+---------------+---------------+------*/
void    ECClass::AddDerivedClass (ECClassCR derivedClass) const
    {
    m_derivedClasses.push_back((ECClassP) &derivedClass);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    09/10
+---------------+---------------+---------------+---------------+---------------+------*/
void    ECClass::RemoveDerivedClass (ECClassCR derivedClass) const
    {
    ECDerivedClassesList::iterator derivedClassIterator;

    for (derivedClassIterator = m_derivedClasses.begin(); derivedClassIterator != m_derivedClasses.end(); derivedClassIterator++)
        {
        if (*derivedClassIterator == (ECClassP)&derivedClass)
            {
            m_derivedClasses.erase(derivedClassIterator);
            return;
            }
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Andrius.Zonys                   07/2012
+---------------+---------------+---------------+---------------+---------------+------*/
void    ECClass::RemoveDerivedClasses ()
    {
    for (ECDerivedClassesList::iterator iter = m_derivedClasses.end(); iter != m_derivedClasses.begin(); )
        (*--iter)->RemoveBaseClass (*this);

    m_derivedClasses.clear ();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    09/10
+---------------+---------------+---------------+---------------+---------------+------*/
const ECDerivedClassesList& ECClass::GetDerivedClasses () const
    {
    return m_derivedClasses;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Carole.MacDonald                02/2010
+---------------+---------------+---------------+---------------+---------------+------*/
bool ECClass::CheckBaseClassCycles (ECClassCP thisClass, const void * arg)
    {
    ECClassCP proposedParent = static_cast<ECClassCP>(arg);
    if (NULL == proposedParent)
        return true;
        
    if (thisClass == proposedParent || ClassesAreEqualByName(thisClass, arg))
        return true;
    return false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus ECClass::AddBaseClass (ECClassCR baseClass)
    {
    return AddBaseClass(baseClass, false);
    }


//-------------------------------------------------------------------------------------
//* @bsimethod                                              
//+---------------+---------------+---------------+---------------+---------------+------
ECObjectsStatus ECClass::AddBaseClass(ECClassCR baseClass, bool insertAtBeginning)
    {
    if (&(baseClass.GetSchema()) != &(this->GetSchema()))
        {
        if (!ECSchema::IsSchemaReferenced(this->GetSchema(), baseClass.GetSchema()))
            return ECOBJECTS_STATUS_SchemaNotFound;
        }

    if (this == &baseClass || ClassesAreEqualByName(this, &baseClass) || baseClass.TraverseBaseClasses(&CheckBaseClassCycles, true, this))
        return ECOBJECTS_STATUS_BaseClassUnacceptable;

    ECBaseClassesList::const_iterator baseClassIterator;
    for (baseClassIterator = m_baseClasses.begin(); baseClassIterator != m_baseClasses.end(); baseClassIterator++)
        {
        if (*baseClassIterator == (ECClassP) &baseClass)
            {
            LOG.warningv(L"Cannot add class '%ls' as a base class to '%ls' because it already exists as a base class", baseClass.GetName().c_str(), GetName().c_str());
            return ECOBJECTS_STATUS_NamedItemAlreadyExists;
            }
        }

    PropertyList baseClassProperties;
    ECObjectsStatus status = baseClass.GetProperties(true, &baseClassProperties);
    if (ECOBJECTS_STATUS_Success != status)
        return status;

    for (ECPropertyP prop : baseClassProperties)
        {
        ECPropertyP thisProperty;
        if (NULL != (thisProperty = this->GetPropertyP(prop->GetName())))
            {
            if (ECOBJECTS_STATUS_Success != (status = ECClass::CanPropertyBeOverridden(*prop, *thisProperty)))
                {
                LOG.errorv(L"Attempt to override a %ls property of class %ls with a different type property in derived class %ls", thisProperty->GetName().c_str(), baseClass.GetName().c_str(), GetName().c_str());
                return status;
                }
            }
        }

    // NEEDSWORK - what if the base class being set is just a stub and does not contain 
    // any properties.  How do we handle property overrides?
    if (!insertAtBeginning)
        m_baseClasses.push_back((ECClassP) &baseClass);
    else
        m_baseClasses.insert(m_baseClasses.begin(), (ECClassP) &baseClass);

    InvalidateDefaultStandaloneEnabler();

    for (ECPropertyP baseProperty : baseClass.GetProperties())
        OnBaseClassPropertyAdded(*baseProperty);

    baseClass.AddDerivedClass(*this);

    return ECOBJECTS_STATUS_Success;
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    
+---------------+---------------+---------------+---------------+---------------+------*/
bool ECClass::HasBaseClasses () const
    {
    return (m_baseClasses.size() > 0);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Carole.MacDonald                03/2010
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus ECClass::RemoveBaseClass (ECClassCR baseClass)
    {
    bool baseClassRemoved = false;

    ECBaseClassesList::iterator baseClassIterator;
    for (baseClassIterator = m_baseClasses.begin(); baseClassIterator != m_baseClasses.end(); baseClassIterator++)
        {
        if (*baseClassIterator == (ECClassP)&baseClass)
            {
            m_baseClasses.erase(baseClassIterator);
            baseClassRemoved = true;
            break;
            }
        }
        
    if (!baseClassRemoved)
        {
        LOG.warningv(L"Class '%ls' is not a base class of class '%ls'", baseClass.GetName().c_str(), GetName().c_str());
        return ECOBJECTS_STATUS_ClassNotFound;
        }
        
    baseClass.RemoveDerivedClass(*this);

    InvalidateDefaultStandaloneEnabler();

    for (ECPropertyP baseProperty : baseClass.GetProperties())
        OnBaseClassPropertyRemoved (*baseProperty);

    return ECOBJECTS_STATUS_Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Andrius.Zonys                   07/2012
+---------------+---------------+---------------+---------------+---------------+------*/
void    ECClass::RemoveBaseClasses ()
    {
    for (ECBaseClassesList::iterator iter = m_baseClasses.begin(); iter != m_baseClasses.end(); iter++)
        (*iter)->RemoveDerivedClass (*this);

    m_baseClasses.clear ();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Carole.MacDonald                02/2010
+---------------+---------------+---------------+---------------+---------------+------*/
bool ECClass::Is (ECClassCP targetClass) const
    {
    if (NULL == targetClass)
        return false;
    
    if (ClassesAreEqualByName(this, targetClass))
        return true;
            
    return TraverseBaseClasses(&ClassesAreEqualByName, true, targetClass);
    }
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Carole.MacDonald                02/2010
+---------------+---------------+---------------+---------------+---------------+------*/
bool ECClass::ClassesAreEqualByName (ECClassCP thisClass, const void * arg)
    {
    ECClassCP thatClass = static_cast<ECClassCP> (arg);
    if (NULL == arg)
        return true;
        
    return ((thisClass == thatClass) ||
            ( (0 == thisClass->GetName().compare(thatClass->GetName())) &&
              (0 == thisClass->GetSchema().GetName().compare(thatClass->GetSchema().GetName())) &&
              (thisClass->GetSchema().GetVersionMajor() == thatClass->GetSchema().GetVersionMajor()) &&
              (thisClass->GetSchema().GetVersionMinor() == thatClass->GetSchema().GetVersionMinor())));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   
+---------------+---------------+---------------+---------------+---------------+------*/
ECPropertyIterable ECClass::GetProperties () const
    {
    return ECPropertyIterable(*this, true);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Carole.MacDonald                04/2010
+---------------+---------------+---------------+---------------+---------------+------*/
ECPropertyIterable ECClass::GetProperties (bool includeBaseProperties) const
    {
    return ECPropertyIterable(*this, includeBaseProperties);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus ECClass::GetProperties (bool includeBaseProperties, PropertyList* propertyList) const
    {
    for (ECPropertyP prop: m_propertyList)
        propertyList->push_back(prop);
        
    if (!includeBaseProperties)
        return ECOBJECTS_STATUS_Success;
        
    if (m_baseClasses.size() == 0)
        return ECOBJECTS_STATUS_Success;
        
    TraverseBaseClasses(&AddUniquePropertiesToList, true, propertyList);

    return ECOBJECTS_STATUS_Success;
    }
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Carole.MacDonald                04/2010
+---------------+---------------+---------------+---------------+---------------+------*/
bool ECClass::AddUniquePropertiesToList (ECClassCP currentBaseClass, const void *arg)
    {
    const PropertyList* props = static_cast<const PropertyList*>(arg);
    PropertyList* propertyList = const_cast<PropertyList*>(props);
    
    PropertyList newProperties;
    PropertyList::iterator currentEnd = propertyList->end();
    for (ECPropertyP prop: currentBaseClass->GetProperties(false))
        {
        PropertyList::iterator testIter;
        for (testIter = propertyList->begin(); testIter != currentEnd; testIter++)
            {
            ECPropertyP testProperty = *testIter;
            if (testProperty->GetName().Equals(prop->GetName()))
                break;
            }
        // we didn't find it
        if (testIter == currentEnd)
            newProperties.push_back(prop);
        }
        
    // add properties in reverse order to front of list, so base class properties come first
    for (size_t i = newProperties.size(); i>0; i--)
        propertyList->insert(propertyList->begin(), newProperties[i-1]);

    return false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Carole.MacDonald                02/2010
+---------------+---------------+---------------+---------------+---------------+------*/
bool ECClass::TraverseBaseClasses (TraversalDelegate traverseMethod, bool recursive, const void* arg) const
    {
    if (m_baseClasses.size() == 0)
        return false;
        
    for (const ECClassP& baseClass: m_baseClasses)
        {
        if (traverseMethod(baseClass, arg))
            return true;
            
        if (recursive)
            {
            if (baseClass->TraverseBaseClasses(traverseMethod, recursive, arg))
                return true;
            }
        }
        
    return false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    
+---------------+---------------+---------------+---------------+---------------+------*/
SchemaReadStatus ECClass::_ReadXmlAttributes (BeXmlNodeR classNode)
    {                
    WString value;      // used by the macros.
    if (GetName().length() == 0)
        {
        READ_REQUIRED_XML_ATTRIBUTE (classNode, TYPE_NAME_ATTRIBUTE,        this, Name,     classNode.GetName())    
        }
    
    // OPTIONAL attributes - If these attributes exist they MUST be valid    
    READ_OPTIONAL_XML_ATTRIBUTE (classNode, DESCRIPTION_ATTRIBUTE,         this, Description)
    READ_OPTIONAL_XML_ATTRIBUTE (classNode, DISPLAY_LABEL_ATTRIBUTE,       this, DisplayLabel)

    // OPTIONAL attributes - If these attributes exist they do not need to be valid.  We will ignore any errors setting them and use default values.
    // NEEDSWORK This is due to the current implementation in managed ECObjects.  We should reconsider whether it is the correct behavior.
    ECObjectsStatus setterStatus;
    READ_OPTIONAL_XML_ATTRIBUTE_IGNORING_SET_ERRORS (classNode, IS_STRUCT_ATTRIBUTE,           this, IsStruct)
    READ_OPTIONAL_XML_ATTRIBUTE_IGNORING_SET_ERRORS (classNode, IS_CUSTOMATTRIBUTE_ATTRIBUTE,  this, IsCustomAttributeClass)

    // when isDomainClass is not specified in the ECSchemaXML and isCustomAttributeClass is specified and set to true, we will default to a non-domain class
    if (BEXML_Success == classNode.GetAttributeStringValue (value, IS_DOMAINCLASS_ATTRIBUTE))
        setterStatus = this->SetIsDomainClass(value.c_str());
    else if (this->GetIsCustomAttributeClass())
        this->SetIsDomainClass (false);

    return SCHEMA_READ_STATUS_Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   
+---------------+---------------+---------------+---------------+---------------+------*/
SchemaReadStatus ECClass::_ReadXmlContents (BeXmlNodeR classNode, ECSchemaReadContextR context)
    {            
	bool isSchemaSupplemental = WString::npos != GetSchema().GetName().find(L"_Supplemental_");
    // Get the BaseClass child nodes.
    for (BeXmlNodeP childNode = classNode.GetFirstChild (); NULL != childNode; childNode = childNode->GetNextSibling ())
        {
        Utf8CP childNodeName = childNode->GetName ();
        if (0 == strcmp (childNodeName, EC_PROPERTY_ELEMENT))
            {
            ECPropertyP ecProperty = new PrimitiveECProperty (*this);
            SchemaReadStatus status = _ReadPropertyFromXmlAndAddToClass (ecProperty, childNode, context, childNodeName);
            if (SCHEMA_READ_STATUS_Success != status)
                return status;
            }
        else if (!isSchemaSupplemental && (0 == strcmp (childNodeName, EC_BASE_CLASS_ELEMENT)))
            {
            SchemaReadStatus status = _ReadBaseClassFromXml(childNode, context);
            if (SCHEMA_READ_STATUS_Success != status)
                return status;
            }
        else if (0 == strcmp (childNodeName, EC_ARRAYPROPERTY_ELEMENT))
            {
            ECPropertyP ecProperty = new ArrayECProperty (*this);
            SchemaReadStatus status = _ReadPropertyFromXmlAndAddToClass (ecProperty, childNode, context, childNodeName);
            if (SCHEMA_READ_STATUS_Success != status)
                return status;
            }
        else if (0 == strcmp (childNodeName, EC_STRUCTPROPERTY_ELEMENT))
            {
            ECPropertyP ecProperty = new StructECProperty (*this);
            SchemaReadStatus status = _ReadPropertyFromXmlAndAddToClass (ecProperty, childNode, context, childNodeName);
            if (SCHEMA_READ_STATUS_Success != status)
                return status;
            }
        }
    
    // Add Custom Attributes
    ReadCustomAttributes (classNode, context, GetSchema());

    return SCHEMA_READ_STATUS_Success;
    }

SchemaReadStatus ECClass::_ReadBaseClassFromXml (BeXmlNodeP childNode, ECSchemaReadContextR context)
    {
    WString qualifiedClassName;
    childNode->GetContent (qualifiedClassName);

    // Parse the potentially qualified class name into a namespace prefix and short class name
    WString namespacePrefix;
    WString className;
    if (ECOBJECTS_STATUS_Success != ECClass::ParseClassName (namespacePrefix, className, qualifiedClassName))
        {
        LOG.warningv ("Invalid ECSchemaXML: The ECClass '%s' contains a %s element with the value '%s' that can not be parsed.",  
            Utf8String (GetName()).c_str(), EC_BASE_CLASS_ELEMENT, Utf8String (qualifiedClassName).c_str());

        return SCHEMA_READ_STATUS_InvalidECSchemaXml;
        }

    ECSchemaCP resolvedSchema = GetSchema().GetSchemaByNamespacePrefixP (namespacePrefix);
    if (NULL == resolvedSchema)
        {
        LOG.warningv  ("Invalid ECSchemaXML: The ECClass '%s' contains a %s element with the namespace prefix '%s' that can not be resolved to a referenced schema.", 
            Utf8String (GetName()).c_str(), EC_BASE_CLASS_ELEMENT, Utf8String (namespacePrefix).c_str());
        return SCHEMA_READ_STATUS_InvalidECSchemaXml;
        }

    context.ResolveClassName (className, *resolvedSchema);
    ECClassCP baseClass = resolvedSchema->GetClassCP (className.c_str());
    if (NULL == baseClass)
        {
        LOG.warningv  ("Invalid ECSchemaXML: The ECClass '%s' contains a %s element with the value '%s' that can not be resolved to an ECClass named '%s' in the ECSchema '%s'", 
            Utf8String (GetName ()).c_str (), EC_BASE_CLASS_ELEMENT, Utf8String (qualifiedClassName).c_str (), Utf8String (className).c_str (), Utf8String (resolvedSchema->GetName ()).c_str ());
        return SCHEMA_READ_STATUS_InvalidECSchemaXml;
        }

    if (ECOBJECTS_STATUS_Success != AddBaseClass (*baseClass))
        return SCHEMA_READ_STATUS_InvalidECSchemaXml;
    
    return SCHEMA_READ_STATUS_Success;
    }


SchemaReadStatus ECClass::_ReadPropertyFromXmlAndAddToClass( ECPropertyP ecProperty, BeXmlNodeP& childNode, ECSchemaReadContextR context, Utf8CP childNodeName )
    {
    // read the property data.
    SchemaReadStatus status = ecProperty->_ReadXml (*childNode, context);
    if (status != SCHEMA_READ_STATUS_Success)
        {
        LOG.warningv  (L"Invalid ECSchemaXML: Failed to read properties of ECClass '%ls:%ls'", this->GetSchema().GetName().c_str(), this->GetName().c_str());                
        delete ecProperty;
        return status;
        }

    if (ECOBJECTS_STATUS_Success != this->AddProperty (ecProperty))
        {
        LOG.warningv  (L"Invalid ECSchemaXML: Failed to read ECClass '%ls:%ls' because a problem occurred while adding ECProperty '%ls'", 
            this->GetName().c_str(), this->GetSchema().GetName().c_str(), WString (childNodeName, BentleyCharEncoding::Utf8).c_str ());
        delete ecProperty;
        return SCHEMA_READ_STATUS_InvalidECSchemaXml;
        }
    
    return SCHEMA_READ_STATUS_Success;
    }



/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Carole.MacDonald                01/2010
+---------------+---------------+---------------+---------------+---------------+------*/
SchemaWriteStatus ECClass::_WriteXml (BeXmlNodeP& classNode, BeXmlNodeR parentNode, Utf8CP elementName) const
    {
    SchemaWriteStatus status = SCHEMA_WRITE_STATUS_Success;

    classNode = parentNode.AddEmptyElement (elementName);
    
    classNode->AddAttributeStringValue (TYPE_NAME_ATTRIBUTE, this->GetName().c_str());
    classNode->AddAttributeStringValue (DESCRIPTION_ATTRIBUTE, this->GetInvariantDescription().c_str());
    if (GetIsDisplayLabelDefined())
        classNode->AddAttributeStringValue (DISPLAY_LABEL_ATTRIBUTE, this->GetInvariantDisplayLabel().c_str());

    classNode->AddAttributeBooleanValue (IS_STRUCT_ATTRIBUTE, this->GetIsStruct());
    classNode->AddAttributeBooleanValue (IS_DOMAINCLASS_ATTRIBUTE, this->GetIsDomainClass());
    classNode->AddAttributeBooleanValue (IS_CUSTOMATTRIBUTE_ATTRIBUTE, this->GetIsCustomAttributeClass());
    
    for (const ECClassP& baseClass: m_baseClasses)
        classNode->AddElementStringValue (EC_BASE_CLASS_ELEMENT, (ECClass::GetQualifiedClassName(GetSchema(), *baseClass)).c_str() );

    WriteCustomAttributes (*classNode);
            
    for (ECPropertyP prop: GetProperties(false))
        {
        BeXmlNodeP  propertyNode;
        prop->_WriteXml (propertyNode, *classNode);
        }

    return status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   
+---------------+---------------+---------------+---------------+---------------+------*/
SchemaWriteStatus ECClass::_WriteXml (BeXmlNodeP& childNode, BeXmlNodeR parentNode) const
    {
    return _WriteXml (childNode, parentNode, EC_CLASS_ELEMENT);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus ECClass::ParseClassName 
(
WStringR  prefix, 
WStringR  className, 
WStringCR qualifiedClassName
)
    {
    if (0 == qualifiedClassName.length())
        {
        LOG.warningv  (L"Failed to parse a prefix and class name from a qualified class name because the string is empty.");
        return ECOBJECTS_STATUS_ParseError;
        }
        
    WString::size_type colonIndex = qualifiedClassName.find (':');
    if (WString::npos == colonIndex)
        {
        prefix.clear();
        className = qualifiedClassName;
        return ECOBJECTS_STATUS_Success;
        }

    if (qualifiedClassName.length() == colonIndex + 1)
        {
        LOG.warningv  (L"Failed to parse a prefix and class name from the qualified class name '%ls' because the string ends with a colon.  There must be characters after the colon.", 
            qualifiedClassName.c_str());
        return ECOBJECTS_STATUS_ParseError;
        }

    if (0 == colonIndex)
        prefix.clear();
    else
        prefix = qualifiedClassName.substr (0, colonIndex);

    className = qualifiedClassName.substr (colonIndex + 1);

    return ECOBJECTS_STATUS_Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   
+---------------+---------------+---------------+---------------+---------------+------*/
WString ECClass::GetQualifiedClassName
(
ECSchemaCR primarySchema,
ECClassCR  ecClass
)
    {
    WString namespacePrefix;
    if (!EXPECTED_CONDITION (ECOBJECTS_STATUS_Success == primarySchema.ResolveNamespacePrefix (ecClass.GetSchema(), namespacePrefix)))
        {
        LOG.warningv (L"warning: Can not qualify an ECClass name with a namespace prefix unless the schema containing the ECClass is referenced by the primary schema."
            L"The class name will remain unqualified.\n  Primary ECSchema: %ls\n  ECClass: %ls\n ECSchema containing ECClass: %ls", primarySchema.GetName().c_str(), ecClass.GetName().c_str(), ecClass.GetSchema().GetName().c_str());
        return ecClass.GetName();
        }
    if (namespacePrefix.empty())
        return ecClass.GetName();
    else
        return namespacePrefix + L":" + ecClass.GetName();
    }
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   
+---------------+---------------+---------------+---------------+---------------+------*/
const ECBaseClassesList& ECClass::GetBaseClasses
(
) const
    {
    return m_baseClasses;
    }
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Carole.MacDonald                06/2010
+---------------+---------------+---------------+---------------+---------------+------*/
void ECClass::_GetBaseContainers
(
bvector<IECCustomAttributeContainerP>& returnList
) const
    {
    for (ECClassP baseClass: m_baseClasses)
        returnList.push_back(baseClass);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Carole.MacDonald                06/2010
+---------------+---------------+---------------+---------------+---------------+------*/
ECSchemaCP ECClass::_GetContainerSchema
(
) const
    {
    return &m_schema;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   03/13
+---------------+---------------+---------------+---------------+---------------+------*/
size_t ECClass::GetPropertyCount (bool includeBaseClasses) const
    {
    size_t nProperties = m_propertyList.size();
    if (includeBaseClasses)
        {
        for (const ECClassP& baseClass: m_baseClasses)
            nProperties += baseClass->GetPropertyCount (true);
        }

    return nProperties;
    }
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                 Ramanujam.Raman                10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
ECPropertyP ECClass::GetInstanceLabelProperty() const
    {
    /*
     * Note: The ugly case by case comparisions is just a way to make the instance 
     * labels from legacy ECschemas (that didn't follow consistent property naming)
     * acceptable.
     */

    ECPropertyP instanceLabelProperty = NULL;
    IECInstancePtr caInstance = this->GetCustomAttribute(L"InstanceLabelSpecification");
    if (caInstance.IsValid())
        {
        ECValue value;
        if (ECOBJECTS_STATUS_Success == caInstance->GetValue (value, L"PropertyName") && !value.IsNull())
            {
            WCharCP propertyName = value.GetString();
            instanceLabelProperty = this->GetPropertyP (propertyName);
            if (NULL != instanceLabelProperty)
                return instanceLabelProperty;
            }
        }

    WString instanceLabelPropertyNames[6] = 
        {L"DisplayLabel", L"DISPLAYLABEL", L"displaylabel", L"Name", L"NAME", L"name"};
    FOR_EACH (WStringCR propName, instanceLabelPropertyNames)
        {
        instanceLabelProperty = this->GetPropertyP (propName.c_str());
        if (NULL != instanceLabelProperty)
            return instanceLabelProperty;
        }

    return NULL;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Carole.MacDonald                04/2010
+---------------+---------------+---------------+---------------+---------------+------*/
ECPropertyIterable::const_iterator::const_iterator
(
ECClassCR ecClass, 
bool includeBaseProperties
)
    {
    m_state = IteratorState::Create (ecClass, includeBaseProperties); 
    if (m_state->m_listIterator == m_state->m_properties->end())
        m_isEnd = true;
    else
        m_isEnd = false; 
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   
+---------------+---------------+---------------+---------------+---------------+------*/
ECPropertyIterable::const_iterator  ECPropertyIterable::begin () const
    {
    return ECPropertyIterable::const_iterator(m_ecClass, m_includeBaseProperties);        
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   
+---------------+---------------+---------------+---------------+---------------+------*/
ECPropertyIterable::const_iterator  ECPropertyIterable::end () const
    {
    return ECPropertyIterable::const_iterator();        
    }   

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   03/14
+---------------+---------------+---------------+---------------+---------------+------*/
ECPropertyCP ECPropertyIterable::FindByDisplayLabel (WCharCP label) const
    {
    for (auto const& prop : *this)
        if (prop->GetDisplayLabel().Equals (label))
            return prop;

    return nullptr;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   
+---------------+---------------+---------------+---------------+---------------+------*/
ECPropertyIterable::const_iterator& ECPropertyIterable::const_iterator::operator++()
    {
    m_state->m_listIterator++;
    if (m_state->m_listIterator == m_state->m_properties->end())
        m_isEnd = true;
    return *this;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   
+---------------+---------------+---------------+---------------+---------------+------*/
bool            ECPropertyIterable::const_iterator::operator!= (const_iterator const& rhs) const
    {
    if (m_isEnd && rhs.m_isEnd)
        return false;
    if (m_state.IsNull() && !(rhs.m_state.IsNull()))
        return true;
    if (!(m_state.IsNull()) && rhs.m_state.IsNull())
        return true;
    return (m_state->m_listIterator != rhs.m_state->m_listIterator);
    }

static const ECPropertyP s_nullPropertyPtr = NULL;

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   
+---------------+---------------+---------------+---------------+---------------+------*/
ECPropertyP const& ECPropertyIterable::const_iterator::operator*() const
    {
    if (m_isEnd)
        return s_nullPropertyPtr;

    ECPropertyP const& ecProperty = *(m_state->m_listIterator);
    return ecProperty;
    }

ECPropertyIterable::IteratorState::IteratorState
(
ECClassCR ecClass,
bool includeBaseProperties
)
    {
    m_properties = new PropertyList();
    ecClass.GetProperties(includeBaseProperties, m_properties);
    m_listIterator = m_properties->begin();
    }
    
ECPropertyIterable::IteratorState::~IteratorState()
    {
    delete m_properties;
    }    
    
static RelationshipCardinality s_zeroOneCardinality(0, 1);
static RelationshipCardinality s_zeroManyCardinality(0, UINT_MAX);
static RelationshipCardinality s_oneOneCardinality(1, 1);
static RelationshipCardinality s_oneManyCardinality(1, UINT_MAX);

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Carole.MacDonald                02/2010
+---------------+---------------+---------------+---------------+---------------+------*/
RelationshipCardinality::RelationshipCardinality
(
)
    {
    m_lowerLimit = 0;
    m_upperLimit = 1;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Carole.MacDonald                02/2010
+---------------+---------------+---------------+---------------+---------------+------*/
RelationshipCardinality::RelationshipCardinality
(
uint32_t lowerLimit,
uint32_t upperLimit
)
    {
    EXPECTED_CONDITION (lowerLimit <= upperLimit);
    //EXPECTED_CONDITION (lowerLimit >= 0); -- always true of a UInt32
    EXPECTED_CONDITION (upperLimit > 0);
    m_lowerLimit = lowerLimit;
    m_upperLimit = upperLimit;
    }
  
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Carole.MacDonald                02/2010
+---------------+---------------+---------------+---------------+---------------+------*/
uint32_t RelationshipCardinality::GetLowerLimit
(
) const
    {
    return m_lowerLimit;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Carole.MacDonald                02/2010
+---------------+---------------+---------------+---------------+---------------+------*/
uint32_t RelationshipCardinality::GetUpperLimit
(
) const
    {
    return m_upperLimit;
    }
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Carole.MacDonald                02/2010
+---------------+---------------+---------------+---------------+---------------+------*/
bool RelationshipCardinality::IsUpperLimitUnbounded 
(
) const
    {
    return m_upperLimit == UINT_MAX;
    }
  
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Carole.MacDonald                03/2010
+---------------+---------------+---------------+---------------+---------------+------*/
WString RelationshipCardinality::ToString
(
) const
    {
    wchar_t cardinalityString[32];
    
    if (UINT_MAX == m_upperLimit)
        {
        BeStringUtilities::Snwprintf(cardinalityString, 32, L"(%d,N)", m_lowerLimit);
        }
    else
        BeStringUtilities::Snwprintf(cardinalityString, 32, L"(%d,%d)", m_lowerLimit, m_upperLimit);
        
    return cardinalityString;
        
    }
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Carole.MacDonald                03/2010
+---------------+---------------+---------------+---------------+---------------+------*/
RelationshipCardinalityCR RelationshipCardinality::ZeroOne
(
)
    {
    return s_zeroOneCardinality;
    }
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Carole.MacDonald                03/2010
+---------------+---------------+---------------+---------------+---------------+------*/
RelationshipCardinalityCR RelationshipCardinality::ZeroMany
(
)
    {
    return s_zeroManyCardinality;
    }
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Carole.MacDonald                03/2010
+---------------+---------------+---------------+---------------+---------------+------*/
RelationshipCardinalityCR RelationshipCardinality::OneOne
(
)
    {
    return s_oneOneCardinality;
    }
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Carole.MacDonald                03/2010
+---------------+---------------+---------------+---------------+---------------+------*/
RelationshipCardinalityCR RelationshipCardinality::OneMany
(
)
    {
    return s_oneManyCardinality;
    }
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Carole.MacDonald                02/2010
+---------------+---------------+---------------+---------------+---------------+------*/
ECRelationshipConstraint::ECRelationshipConstraint
(
ECRelationshipClassP relationshipClass
) :m_constraintClasses(relationshipClass)
    {
    m_relClass = relationshipClass;
    m_cardinality = &s_zeroOneCardinality;
    m_isPolymorphic = true;
    }
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Carole.MacDonald                02/2010
+---------------+---------------+---------------+---------------+---------------+------*/
ECRelationshipConstraint::ECRelationshipConstraint
(
ECRelationshipClassP relationshipClass, 
bool isMultiple
) :m_constraintClasses(relationshipClass,isMultiple)
    {
    m_relClass = relationshipClass;
    m_cardinality = &s_zeroOneCardinality;
    m_isPolymorphic = true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Carole.MacDonald                03/2010
+---------------+---------------+---------------+---------------+---------------+------*/
ECRelationshipConstraint::~ECRelationshipConstraint
(
)
    {
     if ((m_cardinality != &s_zeroOneCardinality) && (m_cardinality != &s_zeroManyCardinality) &&
        (m_cardinality != &s_oneOneCardinality) && (m_cardinality != &s_oneManyCardinality))
        delete m_cardinality;
    } 
   
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Carole.MacDonald                06/2010
+---------------+---------------+---------------+---------------+---------------+------*/
ECSchemaCP ECRelationshipConstraint::_GetContainerSchema() const
    {
    return &(m_relClass->GetSchema());
    }
 
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Carole.MacDonald                03/2010
+---------------+---------------+---------------+---------------+---------------+------*/
SchemaReadStatus ECRelationshipConstraint::ReadXml (BeXmlNodeR constraintNode, ECSchemaReadContextR schemaContext)
    {
    SchemaReadStatus status = SCHEMA_READ_STATUS_Success;
    
    WString value;  // needed for macros.
    ECObjectsStatus setterStatus;
    READ_OPTIONAL_XML_ATTRIBUTE_IGNORING_SET_ERRORS (constraintNode, POLYMORPHIC_ATTRIBUTE, this, IsPolymorphic);
    READ_OPTIONAL_XML_ATTRIBUTE (constraintNode, ROLELABEL_ATTRIBUTE, this, RoleLabel);
    READ_OPTIONAL_XML_ATTRIBUTE (constraintNode, CARDINALITY_ATTRIBUTE, this, Cardinality);
    
    for (BeXmlNodeP constraintClassNode = constraintNode.GetFirstChild(); nullptr != constraintClassNode; constraintClassNode = constraintClassNode->GetNextSibling())
        {
        if (0 != strcmp(constraintClassNode->GetName(), EC_CONSTRAINTCLASS_ELEMENT))
            continue;
        
        WString     constraintClassName;
        if (BEXML_Success != constraintClassNode->GetAttributeStringValue(constraintClassName, CONSTRAINTCLASSNAME_ATTRIBUTE))
            return SCHEMA_READ_STATUS_InvalidECSchemaXml;
        
        // Parse the potentially qualified class name into a namespace prefix and short class name
        WString namespacePrefix;
        WString className;
        if (ECOBJECTS_STATUS_Success != ECClass::ParseClassName (namespacePrefix, className, constraintClassName))
            {
            LOG.warningv ("Invalid ECSchemaXML: The ECRelationshipConstraint contains a %s attribute with the value '%s' that can not be parsed.", 
                CONSTRAINTCLASSNAME_ATTRIBUTE, Utf8String (constraintClassName).c_str());
            return SCHEMA_READ_STATUS_InvalidECSchemaXml;
            }
        
        ECSchemaCP resolvedSchema = m_relClass->GetSchema().GetSchemaByNamespacePrefixP (namespacePrefix);
        if (NULL == resolvedSchema)
            {
            LOG.warningv  ("Invalid ECSchemaXML: ECRelationshipConstraint contains a %s attribute with the namespace prefix '%s' that can not be resolved to a referenced schema.", 
                CONSTRAINTCLASSNAME_ATTRIBUTE, Utf8String (namespacePrefix).c_str());
            return SCHEMA_READ_STATUS_InvalidECSchemaXml;
            }

        ECClassCP constraintClass = resolvedSchema->GetClassCP (className.c_str());
        if (NULL == constraintClass)
            {
            LOG.warningv  ("Invalid ECSchemaXML: The ECRelationshipConstraint contains a %s attribute with the value '%s' that can not be resolved to an ECClass named '%s' in the ECSchema '%s'", 
                CONSTRAINTCLASSNAME_ATTRIBUTE, Utf8String (constraintClassName).c_str(), Utf8String (className).c_str(), Utf8String (resolvedSchema->GetName()).c_str());
            return SCHEMA_READ_STATUS_InvalidECSchemaXml;
            }
        ECRelationshipConstraintClassP ecRelationshipconstaintClass;
        m_constraintClasses.Add(ecRelationshipconstaintClass, *constraintClass);
        if (ecRelationshipconstaintClass != nullptr)
            {
            for (BeXmlNodeP keyNode = constraintClassNode->GetFirstChild(); nullptr != keyNode; keyNode = keyNode->GetNextSibling())
                {
                for (BeXmlNodeP propertyNode = keyNode->GetFirstChild(); nullptr != propertyNode; propertyNode = propertyNode->GetNextSibling())
                    {
                    WString propertyName;
                    if (BEXML_Success != propertyNode->GetAttributeStringValue(propertyName, KEYPROPERTYNAME_ATTRIBUTE))
                        return SCHEMA_READ_STATUS_InvalidECSchemaXml;
                    ecRelationshipconstaintClass->AddKey(propertyName.c_str());
                    }
                }
            }
        }

    // Add Custom Attributes
    ReadCustomAttributes (constraintNode, schemaContext, m_relClass->GetSchema());
    return status;
    }
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                  const ECRelationshipConstraintClass & operator = (ECRelationshipConstraintClass const && rhs)
            {
            m_ecClass = rhs.m_ecClass;
            m_keys = std::move(rhs.m_keys);
            return *this;
            }  Carole.MacDonald                03/2010
+---------------+---------------+---------------+---------------+---------------+------*/
SchemaWriteStatus ECRelationshipConstraint::WriteXml (BeXmlNodeR parentNode, Utf8CP elementName) const
    {
    SchemaWriteStatus status = SCHEMA_WRITE_STATUS_Success;
    
    BeXmlNodeP constraintNode = parentNode.AddEmptyElement (elementName);
    
    constraintNode->AddAttributeStringValue (CARDINALITY_ATTRIBUTE, m_cardinality->ToString().c_str());
    if (IsRoleLabelDefined())
        constraintNode->AddAttributeStringValue (ROLELABEL_ATTRIBUTE, m_roleLabel.c_str());

    constraintNode->AddAttributeBooleanValue (POLYMORPHIC_ATTRIBUTE, this->GetIsPolymorphic());
        
    WriteCustomAttributes (*constraintNode);

    for (const auto &constraint : m_constraintClasses)
        {
        BeXmlNodeP  constraintClassNode = constraintNode->AddEmptyElement(EC_CONSTRAINTCLASS_ELEMENT);
        constraintClassNode->AddAttributeStringValue(CONSTRAINTCLASSNAME_ATTRIBUTE, ECClass::GetQualifiedClassName(m_relClass->GetSchema(), constraint->GetClass()).c_str());
        for (auto key : constraint->GetKeys())
            {
            auto constraintKey = constraintClassNode->AddEmptyElement(EC_CONSTRAINTKEY_ELEMENT);
            constraintKey->AddEmptyElement(EC_KEYPROPERTY_ELEMENT)->AddAttributeStringValue(KEYPROPERTYNAME_ATTRIBUTE, key.c_str());
            }
        }
    
    return status;
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Carole.MacDonald                03/2010
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus ECRelationshipConstraint::AddClass(ECClassCR classConstraint)
    {

    ECRelationshipConstraintClassP ecRelationShipconstraintClass;
    return  m_constraintClasses.Add(ecRelationShipconstraintClass, classConstraint);
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                       MUHAMMAD.ZAIGHUM                             01/2015
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus           ECRelationshipConstraint::AddConstraintClass(ECRelationshipConstraintClass*& classConstraint, ECClassCR ecClass)
    {
    return  m_constraintClasses.Add(classConstraint, ecClass);

    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Carole.MacDonald                03/2010
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus ECRelationshipConstraint::RemoveClass (ECClassCR classConstraint)
    {
    return m_constraintClasses.Remove(classConstraint);
    }
   
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                   Muhammad.Zaighum                 11/14
+---------------+---------------+---------------+---------------+---------------+------*/
const ECConstraintClassesList ECRelationshipConstraint::GetClasses() const
    {
    ECConstraintClassesList listOfClasses;
    for (auto const &constraintClassIterator : m_constraintClasses)
        {
        listOfClasses.push_back (const_cast<ECClassP>(&constraintClassIterator->GetClass ()));
        }
    return listOfClasses;
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                   Muhammad.Zaighum                 11/14
+---------------+---------------+---------------+---------------+---------------+------*/
ECRelationshipConstraintClassList::ECRelationshipConstraintClassList(ECRelationshipClassP relClass, bool isMultiple) :m_relClass(relClass)
    {}
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Carole.MacDonald                03/2010
+---------------+---------------+---------------+---------------+---------------+------*/
ECRelationshipConstraintClassList const& ECRelationshipConstraint::GetConstraintClasses() const
    {
    return m_constraintClasses;
    }
ECRelationshipConstraintClassList& ECRelationshipConstraint::GetConstraintClassesR() 
    {
    return m_constraintClasses;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Carole.MacDonald                02/2010
+---------------+---------------+---------------+---------------+---------------+------*/
bool ECRelationshipConstraint::GetIsPolymorphic () const
    {
    return m_isPolymorphic;
    }
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Carole.MacDonald                02/2010
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus ECRelationshipConstraint::SetIsPolymorphic (bool value)
    {
    m_isPolymorphic = value;
    return ECOBJECTS_STATUS_Success;
    }
   
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Carole.MacDonald                03/2010
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus ECRelationshipConstraint::SetIsPolymorphic (WCharCP isPolymorphic)
    {
    PRECONDITION (NULL != isPolymorphic, ECOBJECTS_STATUS_PreconditionViolated);

    ECObjectsStatus status = ECXml::ParseBooleanString (m_isPolymorphic, isPolymorphic);
    if (ECOBJECTS_STATUS_Success != status)
        LOG.warningv  (L"Failed to parse the isPolymorphic string '%ls' for ECRelationshipConstraint.  Expected values are True or False", isPolymorphic);
        
    return status;
    }
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Carole.MacDonald                03/2010
+---------------+---------------+---------------+---------------+---------------+------*/
RelationshipCardinalityCR ECRelationshipConstraint::GetCardinality () const
    {
    return *m_cardinality;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Carole.MacDonald                03/2010
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus ECRelationshipConstraint::SetCardinality (uint32_t& lowerLimit, uint32_t& upperLimit)
    {
    if (lowerLimit == 0 && upperLimit == 1)
        m_cardinality = &s_zeroOneCardinality;
    else if (lowerLimit == 0 && upperLimit == UINT_MAX)
        m_cardinality = &s_zeroManyCardinality;
    else if (lowerLimit == 1 && upperLimit == 1)
        m_cardinality = &s_oneOneCardinality;
    else if (lowerLimit == 1 && upperLimit == UINT_MAX)
        m_cardinality = &s_oneManyCardinality;
    else
        m_cardinality = new RelationshipCardinality(lowerLimit, upperLimit);
    return ECOBJECTS_STATUS_Success;
    }
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Carole.MacDonald                03/2010
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus ECRelationshipConstraint::SetCardinality (RelationshipCardinalityCR cardinality)
    {
    m_cardinality = new RelationshipCardinality(cardinality.GetLowerLimit(), cardinality.GetUpperLimit());
    return ECOBJECTS_STATUS_Success;
    }
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Carole.MacDonald                03/2010
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus ECRelationshipConstraint::SetCardinality (WCharCP cardinality)
    {
    PRECONDITION (NULL != cardinality, ECOBJECTS_STATUS_PreconditionViolated);
    uint32_t lowerLimit;
    uint32_t upperLimit;
    ECObjectsStatus status = ECXml::ParseCardinalityString(lowerLimit, upperLimit, cardinality);
    if (ECOBJECTS_STATUS_Success != status)
        {
        LOG.errorv (L"Failed to parse the RelationshipCardinality string '%ls'.", cardinality);
        return ECOBJECTS_STATUS_ParseError;
        }
    else
        m_cardinality = new RelationshipCardinality(lowerLimit, upperLimit);
        
    return ECOBJECTS_STATUS_Success;
    }
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Carole.MacDonald                03/2010
+---------------+---------------+---------------+---------------+---------------+------*/
bool ECRelationshipConstraint::IsRoleLabelDefined () const
    {
    return m_roleLabel.length() != 0;
    }
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Carole.MacDonald                03/2010
+---------------+---------------+---------------+---------------+---------------+------*/
WString const ECRelationshipConstraint::GetRoleLabel () const
    {
    if(&(m_relClass->GetTarget()) == this)
        return m_relClass->GetSchema().GetLocalizedStrings().GetRelationshipTargetRoleLabel(m_relClass, GetInvariantRoleLabel());
    else
        return m_relClass->GetSchema().GetLocalizedStrings().GetRelationshipSourceRoleLabel(m_relClass, GetInvariantRoleLabel());
    }
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Carole.MacDonald                03/2010
+---------------+---------------+---------------+---------------+---------------+------*/
WString const ECRelationshipConstraint::GetInvariantRoleLabel () const
    {
    if (m_roleLabel.length() != 0)
        return m_roleLabel;
        
    if (&(m_relClass->GetTarget()) == this)
        return m_relClass->GetInvariantDisplayLabel() + L" (Reversed)";
    return m_relClass->GetInvariantDisplayLabel();
    }
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Carole.MacDonald                03/2010
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus ECRelationshipConstraint::SetRoleLabel (WStringCR value)
    {
    m_roleLabel = value;
    return ECOBJECTS_STATUS_Success;
    }
  
  /*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Carole.MacDonald                05/2012
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus ECRelationshipConstraint::CopyTo
(
ECRelationshipConstraintR toRelationshipConstraint
)
    {
    if (IsRoleLabelDefined())
        toRelationshipConstraint.SetRoleLabel(GetInvariantRoleLabel());

    toRelationshipConstraint.SetCardinality(GetCardinality());
    toRelationshipConstraint.SetIsPolymorphic(GetIsPolymorphic());

    ECObjectsStatus status;
    ECSchemaP destSchema = const_cast<ECSchemaP>(toRelationshipConstraint._GetContainerSchema());
    for (auto constraintClass : GetConstraintClasses())
        {
        ECClassP destConstraintClass = destSchema->GetClassP(constraintClass->GetClass().GetName().c_str());
        if (NULL == destConstraintClass)
            {
            status = destSchema->CopyClass(destConstraintClass, constraintClass->GetClass());
            if (ECOBJECTS_STATUS_Success != status)
                return status;
            }

        status = toRelationshipConstraint.AddClass(*destConstraintClass);
        if (ECOBJECTS_STATUS_Success != status)
            return status;
        }

    return CopyCustomAttributesTo(toRelationshipConstraint);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sylvain.Pucci                  09/2013
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus ECRelationshipConstraint::GetOrderedRelationshipPropertyName (WString& propertyName)  const
    {
    // see if the custom attribute signifying a Ordered relationship is defined
    IECInstancePtr caInstance = GetCustomAttribute(L"OrderedRelationshipsConstraint");
    if (caInstance.IsValid())
        {
        ECN::ECValue value;
        WCharCP propertyName=L"OrderIdProperty";
        if (ECOBJECTS_STATUS_Success == caInstance->GetValue (value, propertyName))
            {
            propertyName = value.GetString ();
            return ECOBJECTS_STATUS_Success;
            }
        }
    return ECOBJECTS_STATUS_Error;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sylvain.Pucci                  09/2013
+---------------+---------------+---------------+---------------+---------------+------*/
bool ECRelationshipConstraint::GetIsOrdered () const
    {
    // see if the custom attribute signifying a Ordered relationship is defined
    IECInstancePtr caInstance = GetCustomAttribute(L"OrderedRelationshipsConstraint");
    if (caInstance.IsValid())
        return true;
    return false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sylvain.Pucci                  09/2013
+---------------+---------------+---------------+---------------+---------------+------*/
OrderIdStorageMode ECRelationshipConstraint::GetOrderIdStorageMode () const
    {
    // see if the custom attribute signifying a Ordered relationship is defined
    IECInstancePtr caInstance = GetCustomAttribute(L"OrderedRelationshipsConstraint");
    if (caInstance.IsValid())
        {
        ECN::ECValue value;
        WCharCP propertyName=L"OrderIdStorageMode";
        if (ECOBJECTS_STATUS_Success == caInstance->GetValue (value, propertyName))
            return (OrderIdStorageMode)value.GetInteger ();
        }
    return ORDERIDSTORAGEMODE_None;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Carole.MacDonald                03/2010
+---------------+---------------+---------------+---------------+---------------+------*/
ECRelationshipClass::ECRelationshipClass (ECN::ECSchemaCR schema) : ECClass (schema), m_strength( STRENGTHTYPE_Referencing), m_strengthDirection(ECRelatedInstanceDirection::Forward) 
    {
    m_source = new ECRelationshipConstraint(this, false);
    m_target = new ECRelationshipConstraint(this, true);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Carole.MacDonald                03/2010
+---------------+---------------+---------------+---------------+---------------+------*/
ECRelationshipClass::~ECRelationshipClass()
    {
    delete m_source;
    delete m_target;
    }
        
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Carole.MacDonald                02/2010
+---------------+---------------+---------------+---------------+---------------+------*/
StrengthType ECRelationshipClass::GetStrength () const
    {
    return m_strength;
    }
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Carole.MacDonald                02/2010
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus ECRelationshipClass::SetStrength (StrengthType strength)
    {
    m_strength = strength;
    return ECOBJECTS_STATUS_Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Carole.MacDonald                02/2010
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus ECRelationshipClass::SetStrength (WCharCP strength)
    {
    PRECONDITION (NULL != strength, ECOBJECTS_STATUS_PreconditionViolated);

    StrengthType strengthType;
    ECObjectsStatus status = ECXml::ParseStrengthType(strengthType, strength);
    if (ECOBJECTS_STATUS_Success != status)
        LOG.errorv (L"Failed to parse the Strength string '%ls' for ECRelationshipClass '%ls'.", strength, this->GetName().c_str());
    else
        SetStrength (strengthType);
        
    return status;
    }
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Carole.MacDonald                02/2010
+---------------+---------------+---------------+---------------+---------------+------*/
ECRelatedInstanceDirection ECRelationshipClass::GetStrengthDirection () const
    {
    return m_strengthDirection;
    }
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Carole.MacDonald                02/2010
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus ECRelationshipClass::SetStrengthDirection (ECRelatedInstanceDirection direction)
    {
    m_strengthDirection = direction;
    return ECOBJECTS_STATUS_Success;
    }
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Carole.MacDonald                02/2010
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus ECRelationshipClass::SetStrengthDirection (WCharCP directionString)
    {
    PRECONDITION (NULL != directionString, ECOBJECTS_STATUS_PreconditionViolated);

    ECRelatedInstanceDirection direction;
    ECObjectsStatus status = ECXml::ParseDirectionString(direction, directionString);
    if (ECOBJECTS_STATUS_Success != status)
        LOG.errorv (L"Failed to parse the ECRelatedInstanceDirection string '%ls' for ECRelationshipClass '%ls'.", directionString, this->GetName().c_str());
    else
        SetStrengthDirection (direction);
        
    return status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Carole.MacDonald                03/2010
+---------------+---------------+---------------+---------------+---------------+------*/
ECRelationshipConstraintR ECRelationshipClass::GetSource () const
    {
    return *m_source;
    }
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Carole.MacDonald                03/2010
+---------------+---------------+---------------+---------------+---------------+------*/
ECRelationshipConstraintR ECRelationshipClass::GetTarget () const
    {
    return *m_target;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Bill.Steinbock                  05/2011
+---------------+---------------+---------------+---------------+---------------+------*/
bool ECRelationshipClass::GetIsOrdered () const
    {
    // see if the custom attribute signifying a Ordered relationship is defined
    IECInstancePtr caInstance = GetCustomAttribute(L"SupportsOrderedRelationships");
    if (caInstance.IsValid())
        return true;

    return false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Bill.Steinbock                  09/2010
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus ECRelationshipClass::GetOrderedRelationshipPropertyName (WString& propertyName, ECRelationshipEnd end) const
    {
    // see if the struct has a custom attribute to custom persist itself
    IECInstancePtr caInstance = GetCustomAttribute(L"SupportsOrderedRelationships");
    if (caInstance.IsValid())
        {
        ECN::ECValue value;
        WCharCP propertyName=L"OrderIdTargetProperty";

        if (end == ECRelationshipEnd_Source)
            propertyName = L"OrderIdSourceProperty";

        if (ECOBJECTS_STATUS_Success == caInstance->GetValue (value, propertyName))
            {
            propertyName = value.GetString ();
            return ECOBJECTS_STATUS_Success;
            }
        }

    return ECOBJECTS_STATUS_Error;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   
+---------------+---------------+---------------+---------------+---------------+------*/
SchemaWriteStatus ECRelationshipClass::_WriteXml (BeXmlNodeP& classNode, BeXmlNodeR parentNode) const
    {
    SchemaWriteStatus   status;
    if (SCHEMA_WRITE_STATUS_Success != (status = T_Super::_WriteXml (classNode, parentNode, EC_RELATIONSHIP_CLASS_ELEMENT)))
        return status;
        
    // verify that this really is the current relationship class element
    if (0 != strcmp (classNode->GetName(), EC_RELATIONSHIP_CLASS_ELEMENT))
        {
        BeAssert (false);
        return SCHEMA_WRITE_STATUS_FailedToCreateXml;
        }
        
    classNode->AddAttributeStringValue (STRENGTH_ATTRIBUTE, ECXml::StrengthToString(m_strength));
    classNode->AddAttributeStringValue (STRENGTHDIRECTION_ATTRIBUTE, ECXml::DirectionToString(m_strengthDirection));
    
    m_source->WriteXml (*classNode, EC_SOURCECONSTRAINT_ELEMENT);
    m_target->WriteXml (*classNode, EC_TARGETCONSTRAINT_ELEMENT);
    
    return status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Carole.MacDonald                02/2010
+---------------+---------------+---------------+---------------+---------------+------*/
SchemaReadStatus ECRelationshipClass::_ReadXmlAttributes (BeXmlNodeR classNode)
    {
    SchemaReadStatus status;
    if (SCHEMA_READ_STATUS_Success != (status = T_Super::_ReadXmlAttributes (classNode)))
        return status;
        
    
    WString value;
    READ_OPTIONAL_XML_ATTRIBUTE (classNode, STRENGTH_ATTRIBUTE, this, Strength)
    READ_OPTIONAL_XML_ATTRIBUTE (classNode, STRENGTHDIRECTION_ATTRIBUTE, this, StrengthDirection)
    
    return SCHEMA_READ_STATUS_Success;
    }
 
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Carole.MacDonald                02/2010
+---------------+---------------+---------------+---------------+---------------+------*/
SchemaReadStatus ECRelationshipClass::_ReadXmlContents (BeXmlNodeR classNode, ECSchemaReadContextR context)
    {
    SchemaReadStatus status = T_Super::_ReadXmlContents (classNode, context);
    if (status != SCHEMA_READ_STATUS_Success)
        return status;

    // skip relationship constraint classes for all supplemental schemas because they should never exist
    if (WString::npos != GetSchema().GetName().find(L"_Supplemental"))  
        return SCHEMA_READ_STATUS_Success;
        
    BeXmlNodeP sourceNode = classNode.SelectSingleNode (EC_NAMESPACE_PREFIX ":" EC_SOURCECONSTRAINT_ELEMENT);
    if (NULL != sourceNode)
        status = m_source->ReadXml (*sourceNode, context);
    if (status != SCHEMA_READ_STATUS_Success)
        return status;
    
    BeXmlNodeP  targetNode = classNode.SelectSingleNode (EC_NAMESPACE_PREFIX ":" EC_TARGETCONSTRAINT_ELEMENT);
    if (NULL != targetNode)
        status = m_target->ReadXml (*targetNode, context);
    if (status != SCHEMA_READ_STATUS_Success)
        return status;
        
    return SCHEMA_READ_STATUS_Success;
    }
    

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  12/2012
+---------------+---------------+---------------+---------------+---------------+------*/
bool            ECClass::Is(WCharCP name) const
    {
    // NEEDSWORK: this is ambiguous without schema name...collisions between unrelated class names are not wholly unexpected.
    if (0 == GetName().CompareTo(name))
        return true;

    const ECBaseClassesList& baseClass = GetBaseClasses();
    for (ECBaseClassesList::const_iterator iter = baseClass.begin(); iter != baseClass.end(); ++iter)
        {
        if ((*iter)->Is(name))
            return true;
        }

    return false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   10/13
+---------------+---------------+---------------+---------------+---------------+------*/
bool ECClass::Is (WCharCP schemaname, WCharCP classname) const
    {
    if (0 == GetName().CompareTo (classname) && 0 == GetSchema().GetName().CompareTo (schemaname))
        return true;

    const ECBaseClassesList& baseClass = GetBaseClasses();
    for (ECBaseClassesList::const_iterator iter = baseClass.begin(); iter != baseClass.end(); ++iter)
        {
        if ((*iter)->Is(schemaname, classname))
            return true;
        }

    return false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsiclass                            Muhammad.Zaighum                   11/14
+---------------+---------------+---------------+---------------+---------------+------*/
struct ECRelationshipConstraintClassList::iterator::Impl
    {
    typedef std::vector<std::unique_ptr<ECRelationshipConstraintClass>>::const_iterator const_iterator;
    private:
        const_iterator m_iterator;

    public:
    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                             Muhammad.Zaighum                   11/14
    +---------------+---------------+---------------+---------------+---------------+------*/
        Impl (const_iterator& iterator)
            :m_iterator (iterator)
            {
            }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                             Muhammad.Zaighum                   11/14
    +---------------+---------------+---------------+---------------+---------------+------*/
    const_iterator const& GetIterator () const { return m_iterator; }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                             Muhammad.Zaighum                   11/14
    +---------------+---------------+---------------+---------------+---------------+------*/
    const_iterator& GetIteratorR ()  { return m_iterator; }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                             Muhammad.Zaighum                   11/14
    +---------------+---------------+---------------+---------------+---------------+------*/
    Impl& operator = (Impl& rhs)
        {
        m_iterator = rhs.GetIterator();
        return *this;
        }
    };
/*---------------------------------------------------------------------------------**//**
* @bsimethod                             Muhammad.Zaighum                   11/14
+---------------+---------------+---------------+---------------+---------------+------*/
ECRelationshipConstraintClassList::iterator::iterator (std::vector<std::unique_ptr<ECRelationshipConstraintClass>>::const_iterator x)
:m_pimpl (new Impl (x))
    {
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                             Muhammad.Zaighum                   11/14
+---------------+---------------+---------------+---------------+---------------+------*/
ECRelationshipConstraintClassList::iterator::iterator(const ECRelationshipConstraintClassList::iterator & it)
:m_pimpl (new Impl (it.m_pimpl->GetIteratorR()))
    {
    } 

/*---------------------------------------------------------------------------------**//**
* @bsimethod                             Muhammad.Zaighum                   11/14
+---------------+---------------+---------------+---------------+---------------+------*/
ECRelationshipConstraintClassList::iterator& ECRelationshipConstraintClassList::iterator::operator = (ECRelationshipConstraintClassList::iterator const& rhs)
    {
    *(this->m_pimpl) =  *(rhs.m_pimpl);
    return *this;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                             Muhammad.Zaighum                   11/14
+---------------+---------------+---------------+---------------+---------------+------*/
ECRelationshipConstraintClassList::iterator::iterator()
    {}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                             Muhammad.Zaighum                   11/14
+---------------+---------------+---------------+---------------+---------------+------*/
ECRelationshipConstraintClassList::iterator& ECRelationshipConstraintClassList::iterator:: operator++()
    {
    ++(m_pimpl->GetIteratorR());
    return *this;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                             Muhammad.Zaighum                   11/14
+---------------+---------------+---------------+---------------+---------------+------*/
bool ECRelationshipConstraintClassList::iterator::operator==(const iterator& rhs)const
    {
    return m_pimpl->GetIteratorR () == rhs.m_pimpl->GetIteratorR ();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                             Muhammad.Zaighum                   11/14
+---------------+---------------+---------------+---------------+---------------+------*/
bool ECRelationshipConstraintClassList::iterator::operator!=(const iterator& rhs)const
    {
    return m_pimpl->GetIteratorR () != rhs.m_pimpl->GetIteratorR ();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                             Muhammad.Zaighum                   11/14
+---------------+---------------+---------------+---------------+---------------+------*/
ECRelationshipConstraintClassCP ECRelationshipConstraintClassList::iterator::operator*()const
    {
    return m_pimpl->GetIteratorR ()->get ();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                             Muhammad.Zaighum                   11/14
+---------------+---------------+---------------+---------------+---------------+------*/
ECRelationshipConstraintClassList::iterator::~iterator()
    {
    delete m_pimpl;
    m_pimpl = nullptr;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                             Muhammad.Zaighum                   11/14
+---------------+---------------+---------------+---------------+---------------+------*/
ECRelationshipConstraintClassCP ECRelationshipConstraintClassList::iterator::operator->()const
    {
    return m_pimpl->GetIteratorR ()->get ();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                             Muhammad.Zaighum                   11/14
+---------------+---------------+---------------+---------------+---------------+------*/
ECRelationshipConstraintClassList::iterator ECRelationshipConstraintClassList::begin()const
    {
    return iterator(m_constraintClasses.begin());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                             Muhammad.Zaighum                   11/14
+---------------+---------------+---------------+---------------+---------------+------*/
ECRelationshipConstraintClassList::iterator ECRelationshipConstraintClassList::end()const
    {
    return iterator(m_constraintClasses.end());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                             Muhammad.Zaighum                   11/14
+---------------+---------------+---------------+---------------+---------------+------*/
ECRelationshipConstraintClassCP ECRelationshipConstraintClassList::operator[](size_t x)const
    {
    return m_constraintClasses.at(x).get();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                             Muhammad.Zaighum                   11/14
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus ECRelationshipConstraintClassList::clear()
    {
    m_constraintClasses.clear();
    return ECObjectsStatus::ECOBJECTS_STATUS_Success;
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                             Muhammad.Zaighum                   11/14
+---------------+---------------+---------------+---------------+---------------+------*/
uint32_t ECRelationshipConstraintClassList::size()const
    {
    return (uint32_t)m_constraintClasses.size();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                             Muhammad.Zaighum                   11/14
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus ECRelationshipConstraintClassList::Remove(ECClassCR constainClass)
    {
    for (auto itor = m_constraintClasses.begin(); itor != m_constraintClasses.end(); itor++)
        {
        if (&itor->get()->GetClass() == &constainClass)
            {
            m_constraintClasses.erase(itor);
            return ECOBJECTS_STATUS_Success;
            }
        }

    return ECOBJECTS_STATUS_ClassNotFound;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                             Muhammad.Zaighum                   11/14
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus ECRelationshipConstraintClassList::Add(ECRelationshipConstraintClass*& classConstraint, ECClassCR ecClass)
    {
    classConstraint = nullptr;
    if (&(ecClass.GetSchema()) != &(m_relClass->GetSchema()))
        {
        ECSchemaReferenceListCR referencedSchemas = m_relClass->GetSchema().GetReferencedSchemas();
        ECSchemaReferenceList::const_iterator schemaIterator = referencedSchemas.find(ecClass.GetSchema().GetSchemaKey());
        if (schemaIterator == referencedSchemas.end())
            return ECOBJECTS_STATUS_SchemaNotFound;
        }

    for (auto &constraintClassIterator : m_constraintClasses)
        {
        if (&constraintClassIterator->GetClass() == &ecClass)
            {
            classConstraint = constraintClassIterator.get();
            return ECOBJECTS_STATUS_Success;
            }
        }
    auto newConstraintClass =  std::unique_ptr<ECRelationshipConstraintClass>(new ECRelationshipConstraintClass(ecClass));
    classConstraint = newConstraintClass.get();
    m_constraintClasses.push_back(std::move(newConstraintClass));
    return ECOBJECTS_STATUS_Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                   Muhammad.Zaighum                 11/14
+---------------+---------------+---------------+---------------+---------------+------*/
ECRelationshipConstraintClassList::~ECRelationshipConstraintClassList()
    {
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                   Muhammad.Zaighum                 11/14
+---------------+---------------+---------------+---------------+---------------+------*/
ECRelationshipConstraintClass::ECRelationshipConstraintClass(ECClassCR ecClass) : m_ecClass(&ecClass)
    {}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                   Muhammad.Zaighum                 11/14
+---------------+---------------+---------------+---------------+---------------+------*/
ECRelationshipConstraintClass::ECRelationshipConstraintClass(ECRelationshipConstraintClass&& rhs) 
    : m_ecClass(std::move(rhs.m_ecClass)), m_keys(std::move(rhs.m_keys))
    { }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                             Muhammad.Zaighum                   11/14
+---------------+---------------+---------------+---------------+---------------+------*/
ECRelationshipConstraintClass& ECRelationshipConstraintClass::operator=(ECRelationshipConstraintClass&& rhs)
    {
    if (this != &rhs)
        {
        m_ecClass = std::move(rhs.m_ecClass);
        m_keys = std::move(rhs.m_keys);
        }

    return *this;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                             Muhammad.Zaighum                   11/14
+---------------+---------------+---------------+---------------+---------------+------*/
void ECRelationshipConstraintClass::AddKey(WCharCP keyPropertyName)
    {
    if (WString::IsNullOrEmpty(keyPropertyName))
        {
        BeAssert(false && "keyPropertyName arg must not be nullptr or empty string.");
        return;
        }

    m_keys.push_back(keyPropertyName);
    }

END_BENTLEY_ECOBJECT_NAMESPACE
