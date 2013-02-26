/*--------------------------------------------------------------------------------------+
|
|     $Source: src/ECClass.cpp $
|    $RCSfile: file.tpl,v $
|   $Revision: 1.10 $
|       $Date: 2005/11/07 15:38:45 $
|     $Author: EarlinLutz $
|
|  $Copyright: (c) 2013 Bentley Systems, Incorporated. All rights reserved. $
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
    // NEEDSWORK make sure everything is destroyed
    m_propertyList.clear();
    
    for (PropertyMap::iterator entry=m_propertyMap.begin(); entry != m_propertyMap.end(); ++entry)
        delete entry->second;
    
    m_propertyMap.clear();
    
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

    BeAssert (0 != m_ecClassId);
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
        ECObjectsLogger::Log()->warningv  (L"Failed to parse the isStruct string '%ls' for ECClass '%ls'.  Expected values are True or False", isStruct, this->GetName().c_str());
        
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
        ECObjectsLogger::Log()->warningv  (L"Failed to parse the isCustomAttributeClass string '%ls' for ECClass '%ls'.  Expected values are True or False", isCustomAttributeClass, this->GetName().c_str());
        
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
        ECObjectsLogger::Log()->warningv  (L"Failed to parse the isDomainClass string '%ls' for ECClass '%ls'.  Expected values are True or False", isDomainClass, this->GetName().c_str());
        
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
        ClassLayoutP classLayout   = ClassLayout::BuildFromClass (*this);
        m_defaultStandaloneEnabler = StandaloneECEnabler::CreateEnabler (*this, *classLayout, NULL, true);
        }

    BeAssert(m_defaultStandaloneEnabler.IsValid());
    return m_defaultStandaloneEnabler.get();
    }

/*---------------------------------------------------------------------------------**//**
 @bsimethod                                                     
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus ECClass::AddProperty (ECPropertyP& pProperty)
    {
    PropertyMap::const_iterator propertyIterator = m_propertyMap.find(pProperty->GetName().c_str());
    if (m_propertyMap.end() != propertyIterator)
        {
        ECObjectsLogger::Log()->warningv  (L"Can not create property '%ls' because it already exists in this ECClass", pProperty->GetName().c_str());
        return ECOBJECTS_STATUS_NamedItemAlreadyExists;
        }

    // It isn't part of this schema, but does it exist as a property on a baseClass?
    ECPropertyP baseProperty = GetPropertyP(pProperty->GetName());
    if (NULL == baseProperty)
        {
        m_propertyMap.insert (bpair<WCharCP, ECPropertyP> (pProperty->GetName().c_str(), pProperty));
        m_propertyList.push_back(pProperty);
        return ECOBJECTS_STATUS_Success;
        }

    ECObjectsStatus status = CanPropertyBeOverridden(*baseProperty, *pProperty);
    if (ECOBJECTS_STATUS_Success != status)
        return status;

    pProperty->SetBaseProperty (baseProperty);
    m_propertyMap.insert (bpair<WCharCP, ECPropertyP> (pProperty->GetName().c_str(), pProperty));
    m_propertyList.push_back(pProperty);
    return ECOBJECTS_STATUS_Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Carole.MacDonald                05/2012
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus ECClass::CopyProperty
(
ECPropertyP& destProperty, 
ECPropertyP sourceProperty,
bool copyCustomAttributes
)
    {
    if (sourceProperty->GetIsPrimitive())
        {
        PrimitiveECPropertyP destPrimitive;
        PrimitiveECPropertyP sourcePrimitive = sourceProperty->GetAsPrimitiveProperty();
        destPrimitive = new PrimitiveECProperty(*this);
        destPrimitive->SetType(sourcePrimitive->GetType());

        destProperty = destPrimitive;
        }
    else if (sourceProperty->GetIsArray())
        {
        ArrayECPropertyP destArray;
        ArrayECPropertyP sourceArray = sourceProperty->GetAsArrayProperty();
        destArray = new ArrayECProperty (*this);
        if (NULL != sourceArray->GetStructElementType())
            destArray->SetStructElementType(sourceArray->GetStructElementType());
        else
            destArray->SetPrimitiveElementType(sourceArray->GetPrimitiveElementType());

        destArray->SetMaxOccurs(sourceArray->GetMaxOccurs());
        destArray->SetMinOccurs(sourceArray->GetMinOccurs());

        destProperty = destArray;
        }
    else if (sourceProperty->GetIsStruct())
        {
        StructECPropertyP destStruct;
        StructECPropertyP sourceStruct = sourceProperty->GetAsStructProperty();
        destStruct = new StructECProperty (*this);
        destStruct->SetType(sourceStruct->GetType());

        destProperty = destStruct;
        }

    destProperty->SetDescription(sourceProperty->GetDescription());
    if (sourceProperty->GetIsDisplayLabelDefined())
        destProperty->SetDisplayLabel(sourceProperty->GetDisplayLabel());
    destProperty->SetName(sourceProperty->GetName());
    destProperty->SetIsReadOnly(sourceProperty->GetIsReadOnly());
    destProperty->m_forSupplementation = true;
    if (copyCustomAttributes)
        sourceProperty->CopyCustomAttributesTo(*destProperty);

    ECObjectsStatus status = AddProperty(destProperty, sourceProperty->GetName());
    if (ECOBJECTS_STATUS_Success != status)
        delete destProperty;

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
    FOR_EACH (const ECClassP& baseClass, m_baseClasses)
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

static bvector<WString> s_schemasThatAllowOverridingArrays;
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Carole.MacDonald                11/2011
+---------------+---------------+---------------+---------------+---------------+------*/
void initSchemasThatAllowOverridingArrays ()
    {
    if (!s_schemasThatAllowOverridingArrays.empty())
        return;
    s_schemasThatAllowOverridingArrays.push_back (L"jclass.01");
    s_schemasThatAllowOverridingArrays.push_back (L"jclass.02");
    s_schemasThatAllowOverridingArrays.push_back (L"ECXA_ams.01");
    s_schemasThatAllowOverridingArrays.push_back (L"ECXA_ams_user.01");
    s_schemasThatAllowOverridingArrays.push_back (L"ams.01");
    s_schemasThatAllowOverridingArrays.push_back (L"ams_user.01");
    s_schemasThatAllowOverridingArrays.push_back (L"Bentley_JSpace_CustomAttributes.02");
    s_schemasThatAllowOverridingArrays.push_back (L"Bentley_Plant.06");
    }

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
    initSchemasThatAllowOverridingArrays();
    wchar_t buf[1024]; 

    BeStringUtilities::Snwprintf (buf, L"%ls.%02d", schema->GetName().c_str(), schema->GetVersionMajor());
    WString nameAndVersion(buf);
    bvector<WString>::iterator iter = std::find(s_schemasThatAllowOverridingArrays.begin(), s_schemasThatAllowOverridingArrays.end(), nameAndVersion);
    return (iter != s_schemasThatAllowOverridingArrays.end());
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
        ECObjectsLogger::Log()->errorv(L"The datatype of ECProperty %ls.%ls (%ls) does not match the datatype of ECProperty %ls.%ls (%ls)... which it overrides.", 
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
 
    m_propertyMap.erase(propertyIterator);

    // remove property from vector m_propertyList
    PropertyList::iterator propertyListIterator;
    for (propertyListIterator = m_propertyList.begin(); propertyListIterator != m_propertyList.end(); propertyListIterator++)
        {
        if (*propertyListIterator == ecProperty)
            {
            m_propertyList.erase(propertyListIterator);
            break;
            }
        }

    delete ecProperty;

    return ECOBJECTS_STATUS_Success;
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
        if (*baseClassIterator == (ECClassP)&baseClass)
            {
            ECObjectsLogger::Log()->warningv (L"Can not add class '%ls' as a base class to '%ls' because it already exists as a base class", baseClass.GetName().c_str(), GetName().c_str());
            return ECOBJECTS_STATUS_NamedItemAlreadyExists;
            }
        }

    PropertyList baseClassProperties;
    ECObjectsStatus status = baseClass.GetProperties(true, &baseClassProperties);
    if (ECOBJECTS_STATUS_Success != status)
        return status;

    FOR_EACH (ECPropertyP prop, baseClassProperties)
        {
        ECPropertyP thisProperty;
        if (NULL != (thisProperty = this->GetPropertyP(prop->GetName())))
            {
            if (ECOBJECTS_STATUS_Success != (status = ECClass::CanPropertyBeOverridden(*prop, *thisProperty)))
                return status;
            }
        }

    // NEEDSWORK - what if the base class being set is just a stub and does not contain 
    // any properties.  How do we handle property overrides?
    m_baseClasses.push_back((ECClassP)&baseClass);

    baseClass.AddDerivedClass (*this);

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
        ECObjectsLogger::Log()->warningv(L"Class '%ls' is not a base class of class '%ls'", baseClass.GetName().c_str(), GetName().c_str());
        return ECOBJECTS_STATUS_ClassNotFound;
        }
        
    baseClass.RemoveDerivedClass(*this);

    return ECOBJECTS_STATUS_Success;
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
    FOR_EACH (ECPropertyP prop, m_propertyList)
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
    FOR_EACH (ECPropertyP prop, currentBaseClass->GetProperties(false))
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
        
    FOR_EACH (const ECClassP& baseClass, m_baseClasses)
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
    READ_OPTIONAL_XML_ATTRIBUTE_IGNORING_SET_ERRORS (classNode, IS_DOMAINCLASS_ATTRIBUTE,      this, IsDomainClass)

    // when isDomainClass is not specified in the ECSchemaXML and isCustomAttributeClass is specified and set to true, we will default to a non-domain class
    if (this->GetIsCustomAttributeClass())
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
            SchemaReadStatus status = _ReadBaseClassFromXml(childNode);
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

SchemaReadStatus ECClass::_ReadBaseClassFromXml ( BeXmlNodeP childNode )
    {
    WString qualifiedClassName;
    childNode->GetContent (qualifiedClassName);

    // Parse the potentially qualified class name into a namespace prefix and short class name
    WString namespacePrefix;
    WString className;
    if (ECOBJECTS_STATUS_Success != ECClass::ParseClassName (namespacePrefix, className, qualifiedClassName))
        {
        ECObjectsLogger::Log()->warningv (L"Invalid ECSchemaXML: The ECClass '%ls' contains a %hs element with the value '%ls' that can not be parsed.",  
            this->GetName().c_str(), EC_BASE_CLASS_ELEMENT, qualifiedClassName.c_str());

        return SCHEMA_READ_STATUS_InvalidECSchemaXml;
        }

    ECSchemaCP resolvedSchema = GetSchema().GetSchemaByNamespacePrefixP (namespacePrefix);
    if (NULL == resolvedSchema)
        {
        ECObjectsLogger::Log()->warningv  (L"Invalid ECSchemaXML: The ECClass '%ls' contains a %hs element with the namespace prefix '%ls' that can not be resolved to a referenced schema.", 
            this->GetName().c_str(), EC_BASE_CLASS_ELEMENT, namespacePrefix.c_str());
        return SCHEMA_READ_STATUS_InvalidECSchemaXml;
        }

    ECClassCP baseClass = resolvedSchema->GetClassCP (className.c_str());
    if (NULL == baseClass)
        {
        ECObjectsLogger::Log()->warningv  (L"Invalid ECSchemaXML: The ECClass '%ls' contains a %hs element with the value '%ls' that can not be resolved to an ECClass named '%ls' in the ECSchema '%ls'", 
            this->GetName().c_str(), EC_BASE_CLASS_ELEMENT, qualifiedClassName.c_str(), className.c_str(), resolvedSchema->GetName().c_str());
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
        ECObjectsLogger::Log()->warningv  (L"Invalid ECSchemaXML: Failed to read properties of ECClass '%ls:%ls'", this->GetSchema().GetName().c_str(), this->GetName().c_str());                
        delete ecProperty;
        return status;
        }

    if (ECOBJECTS_STATUS_Success != this->AddProperty (ecProperty))
        {
        ECObjectsLogger::Log()->warningv  (L"Invalid ECSchemaXML: Failed to read ECClass '%ls:%ls' because a problem occurred while adding ECProperty '%hs'", 
            this->GetName().c_str(), this->GetSchema().GetName().c_str(), childNodeName);
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
    classNode->AddAttributeStringValue (DESCRIPTION_ATTRIBUTE, this->GetDescription().c_str());
    if (GetIsDisplayLabelDefined())
        classNode->AddAttributeStringValue (DISPLAY_LABEL_ATTRIBUTE, this->GetDisplayLabel().c_str());

    classNode->AddAttributeBooleanValue (IS_STRUCT_ATTRIBUTE, this->GetIsStruct());
    classNode->AddAttributeBooleanValue (IS_DOMAINCLASS_ATTRIBUTE, this->GetIsDomainClass());
    classNode->AddAttributeBooleanValue (IS_CUSTOMATTRIBUTE_ATTRIBUTE, this->GetIsCustomAttributeClass());
    
    FOR_EACH (const ECClassP& baseClass, m_baseClasses)
        classNode->AddElementStringValue (EC_BASE_CLASS_ELEMENT, (ECClass::GetQualifiedClassName(GetSchema(), *baseClass)).c_str() );

    WriteCustomAttributes (*classNode);
            
    FOR_EACH (ECPropertyP prop, GetProperties(false))
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
        ECObjectsLogger::Log()->warningv  (L"Failed to parse a prefix and class name from a qualified class name because the string is empty.");
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
        ECObjectsLogger::Log()->warningv  (L"Failed to parse a prefix and class name from the qualified class name '%ls' because the string ends with a colon.  There must be characters after the colon.", 
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
        ECObjectsLogger::Log()->warningv (L"warning: Can not qualify an ECClass name with a namespace prefix unless the schema containing the ECClass is referenced by the primary schema."
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
    FOR_EACH (ECClassP baseClass, m_baseClasses)
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
        if (SUCCESS == caInstance->GetValue (value, L"PropertyName") && !value.IsNull())
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

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   
+---------------+---------------+---------------+---------------+---------------+------*/
ECPropertyP const& ECPropertyIterable::const_iterator::operator*() const
    {
    static ECPropertyP s_nullPtr;
    if (m_isEnd)
        return s_nullPtr;
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
UInt32 lowerLimit,
UInt32 upperLimit
)
    {
    EXPECTED_CONDITION (lowerLimit <= upperLimit);
    EXPECTED_CONDITION (lowerLimit >= 0);
    EXPECTED_CONDITION (upperLimit > 0);
    m_lowerLimit = lowerLimit;
    m_upperLimit = upperLimit;
    }
  
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Carole.MacDonald                02/2010
+---------------+---------------+---------------+---------------+---------------+------*/
UInt32 RelationshipCardinality::GetLowerLimit
(
) const
    {
    return m_lowerLimit;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Carole.MacDonald                02/2010
+---------------+---------------+---------------+---------------+---------------+------*/
UInt32 RelationshipCardinality::GetUpperLimit
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
)
    {
    m_relClass = relationshipClass;
    m_cardinality = &s_zeroOneCardinality;
    m_isMultiple = false;
    m_isPolymorphic = true;
    }
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Carole.MacDonald                02/2010
+---------------+---------------+---------------+---------------+---------------+------*/
ECRelationshipConstraint::ECRelationshipConstraint
(
ECRelationshipClassP relationshipClass, 
bool isMultiple
)
    {
    m_relClass = relationshipClass;
    m_isMultiple = isMultiple;
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
    
    for (BeXmlNodeP childNode = constraintNode.GetFirstChild (); NULL != childNode; childNode = childNode->GetNextSibling ())
        {
        if (0 != strcmp (childNode->GetName (), EC_CONSTRAINTCLASS_ELEMENT))
            continue;
        
        WString     constraintClassName;
        if (BEXML_Success != childNode->GetAttributeStringValue (constraintClassName, CONSTRAINTCLASSNAME_ATTRIBUTE))
            return SCHEMA_READ_STATUS_InvalidECSchemaXml;
        
        // Parse the potentially qualified class name into a namespace prefix and short class name
        WString namespacePrefix;
        WString className;
        if (ECOBJECTS_STATUS_Success != ECClass::ParseClassName (namespacePrefix, className, constraintClassName))
            {
            ECObjectsLogger::Log()->warningv (L"Invalid ECSchemaXML: The ECRelationshipConstraint contains a %hs attribute with the value '%ls' that can not be parsed.", 
                CONSTRAINTCLASSNAME_ATTRIBUTE, constraintClassName.c_str());
            return SCHEMA_READ_STATUS_InvalidECSchemaXml;
            }
        
        ECSchemaCP resolvedSchema = m_relClass->GetSchema().GetSchemaByNamespacePrefixP (namespacePrefix);
        if (NULL == resolvedSchema)
            {
            ECObjectsLogger::Log()->warningv  (L"Invalid ECSchemaXML: ECRelationshipConstraint contains a %hs attribute with the namespace prefix '%ls' that can not be resolved to a referenced schema.", 
                CONSTRAINTCLASSNAME_ATTRIBUTE, namespacePrefix.c_str());
            return SCHEMA_READ_STATUS_InvalidECSchemaXml;
            }

        ECClassCP constraintClass = resolvedSchema->GetClassCP (className.c_str());
        if (NULL == constraintClass)
            {
            ECObjectsLogger::Log()->warningv  (L"Invalid ECSchemaXML: The ECRelationshipConstraint contains a %hs attribute with the value '%ls' that can not be resolved to an ECClass named '%ls' in the ECSchema '%ls'", 
                CONSTRAINTCLASSNAME_ATTRIBUTE, constraintClassName.c_str(), className.c_str(), resolvedSchema->GetName().c_str());
            return SCHEMA_READ_STATUS_InvalidECSchemaXml;
            }
        AddClass(*constraintClass);
        }

    // Add Custom Attributes
    ReadCustomAttributes (constraintNode, schemaContext, m_relClass->GetSchema());

    return status;
    }
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Carole.MacDonald                03/2010
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

    FOR_EACH (ECClassP constraint, m_constraintClasses)
        {
        BeXmlNodeP  constraintClassNode = constraintNode->AddEmptyElement (EC_CONSTRAINTCLASS_ELEMENT);
        constraintClassNode->AddAttributeStringValue (CONSTRAINTCLASSNAME_ATTRIBUTE, ECClass::GetQualifiedClassName(m_relClass->GetSchema(), *constraint).c_str());
        }
    
    return status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Carole.MacDonald                03/2010
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus ECRelationshipConstraint::AddClass (ECClassCR classConstraint)
    {
    if (&(classConstraint.GetSchema()) != &(m_relClass->GetSchema()))
        {
        ECSchemaReferenceListCR referencedSchemas = m_relClass->GetSchema().GetReferencedSchemas();
        ECSchemaReferenceList::const_iterator schemaIterator = referencedSchemas.find (classConstraint.GetSchema().GetSchemaKey());
        if (schemaIterator == referencedSchemas.end())
            return ECOBJECTS_STATUS_SchemaNotFound;
        }

    if (!m_isMultiple)
        m_constraintClasses.clear();
    else
        {
        ECConstraintClassesList::const_iterator constraintClassIterator;
        for (constraintClassIterator = m_constraintClasses.begin(); constraintClassIterator != m_constraintClasses.end(); constraintClassIterator++)
            {
            if (*constraintClassIterator == (ECClassP)&classConstraint)
                return ECOBJECTS_STATUS_Success;
            }
        }
    m_constraintClasses.push_back((ECClassP)&classConstraint);
    
    return ECOBJECTS_STATUS_Success;       
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Carole.MacDonald                03/2010
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus ECRelationshipConstraint::RemoveClass (ECClassCR classConstraint)
    {
    ECConstraintClassesList::iterator constraintClassIterator;

    for (constraintClassIterator = m_constraintClasses.begin(); constraintClassIterator != m_constraintClasses.end(); constraintClassIterator++)
        {
        if (*constraintClassIterator == (ECClassP)&classConstraint)
            {
            m_constraintClasses.erase(constraintClassIterator);
            return ECOBJECTS_STATUS_Success;
            }
        }
        
    return ECOBJECTS_STATUS_ClassNotFound;
    }
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Carole.MacDonald                03/2010
+---------------+---------------+---------------+---------------+---------------+------*/
const ECConstraintClassesList& ECRelationshipConstraint::GetClasses () const
    {
    return m_constraintClasses;
    }
     
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Carole.MacDonald                02/2010
+---------------+---------------+---------------+---------------+---------------+------*/
bool ECRelationshipConstraint::GetIsMultiple () const
    {
    return m_isMultiple;
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
        ECObjectsLogger::Log()->warningv  (L"Failed to parse the isPolymorphic string '%ls' for ECRelationshipConstraint.  Expected values are True or False", isPolymorphic);
        
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
ECObjectsStatus ECRelationshipConstraint::SetCardinality (UInt32& lowerLimit, UInt32& upperLimit)
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
    UInt32 lowerLimit;
    UInt32 upperLimit;
    ECObjectsStatus status = ECXml::ParseCardinalityString(lowerLimit, upperLimit, cardinality);
    if (ECOBJECTS_STATUS_Success != status)
        {
        ECObjectsLogger::Log()->errorv (L"Failed to parse the RelationshipCardinality string '%ls'.", cardinality);
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
    if (m_roleLabel.length() != 0)
        return m_roleLabel;
        
    if (&(m_relClass->GetTarget()) == this)
        return m_relClass->GetDisplayLabel() + L" (Reversed)";
    return m_relClass->GetDisplayLabel();
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
        toRelationshipConstraint.SetRoleLabel(GetRoleLabel());

    toRelationshipConstraint.SetCardinality(GetCardinality());
    toRelationshipConstraint.SetIsPolymorphic(GetIsPolymorphic());

    ECObjectsStatus status;
    ECSchemaP destSchema = const_cast<ECSchemaP>(toRelationshipConstraint._GetContainerSchema());
    FOR_EACH(ECClassP constraintClass, GetClasses())
        {
        ECClassP destConstraintClass = destSchema->GetClassP(constraintClass->GetName().c_str());
        if (NULL == destConstraintClass)
            {
            status = destSchema->CopyClass(destConstraintClass, *constraintClass);
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
* @bsimethod                                    Carole.MacDonald                03/2010
+---------------+---------------+---------------+---------------+---------------+------*/
ECRelationshipClass::ECRelationshipClass (ECN::ECSchemaCR schema) : ECClass (schema), m_strength( STRENGTHTYPE_Referencing), m_strengthDirection(STRENGTHDIRECTION_Forward) 
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
        ECObjectsLogger::Log()->errorv (L"Failed to parse the Strength string '%ls' for ECRelationshipClass '%ls'.", strength, this->GetName().c_str());
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
        ECObjectsLogger::Log()->errorv (L"Failed to parse the ECRelatedInstanceDirection string '%ls' for ECRelationshipClass '%ls'.", directionString, this->GetName().c_str());
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

        if (SUCCESS == caInstance->GetValue (value, propertyName))
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
        
    classNode->AddAttributeStringValue (STRENGTH_ATTRIBUTE, ECXml::StrengthToString(m_strength).c_str());
    classNode->AddAttributeStringValue (STRENGTHDIRECTION_ATTRIBUTE, ECXml::DirectionToString(m_strengthDirection).c_str());
    
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
        m_source->ReadXml (*sourceNode, context);
    
    BeXmlNodeP  targetNode = classNode.SelectSingleNode (EC_NAMESPACE_PREFIX ":" EC_TARGETCONSTRAINT_ELEMENT);
    if (NULL != targetNode)
        m_target->ReadXml (*targetNode, context);
        
    return SCHEMA_READ_STATUS_Success;
    }
    
END_BENTLEY_ECOBJECT_NAMESPACE
