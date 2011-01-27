/*--------------------------------------------------------------------------------------+
|
|     $Source: ecobjects/native/ECClass.cpp $
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

//#define DEBUG_CLASS_LEAKS
#ifdef DEBUG_CLASS_LEAKS
LeakDetector<ECClass> g_leakDetector (L"ECClass", L"ECClasss", true);
#else
LeakDetector<ECClass> g_leakDetector (L"ECClass", L"ECClasss", false);
#endif

/*---------------------------------------------------------------------------------**//**
 @bsimethod                                                 
+---------------+---------------+---------------+---------------+---------------+------*/
ECClass::ECClass (ECSchemaCR schema, bool hideFromLeakDetection)
    :
    m_schema(schema), m_isStruct(false), m_isCustomAttributeClass(false), m_isDomainClass(true), m_hideFromLeakDetection (hideFromLeakDetection)
    {
    if ( ! m_hideFromLeakDetection)
        g_leakDetector.ObjectCreated(*this);
    };

/*---------------------------------------------------------------------------------**//**
 @bsimethod                                                 
+---------------+---------------+---------------+---------------+---------------+------*/
ECClass::~ECClass ()
    {
    // NEEDSWORK make sure everything is destroyed
    ECObjectsLogger::Log()->tracev (L"~~~~ Destroying ECClass %s\n", this->Name.c_str());
    ECObjectsLogger::Log()->tracev  (L"     Freeing memory for %d properties\n", m_propertyMap.size());
    
    m_propertyList.clear();
    
    for each (bpair<wchar_t const*, ECPropertyP> entry in m_propertyMap)
        delete entry.second;
    
    m_propertyMap.clear();

    if ( ! m_hideFromLeakDetection)
        g_leakDetector.ObjectDestroyed(*this);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    09/10
+---------------+---------------+---------------+---------------+---------------+------*/
ILeakDetector&  ECClass::Debug_GetLeakDetector() { return g_leakDetector; }

/*---------------------------------------------------------------------------------**//**
 @bsimethod                                                     
+---------------+---------------+---------------+---------------+---------------+------*/
bwstring const& ECClass::GetName
(
) const
    {        
    return m_name;
    }

/*---------------------------------------------------------------------------------**//**
 @bsimethod                                                     
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus ECClass::SetName
(
bwstring const& name
)
    {
    
    if (!NameValidator::Validate(name))
        return ECOBJECTS_STATUS_InvalidName;
        
    m_name = name;        
    return ECOBJECTS_STATUS_Success;
    }

/*---------------------------------------------------------------------------------**//**
 @bsimethod                                                     
+---------------+---------------+---------------+---------------+---------------+------*/
bwstring const& ECClass::GetDescription
(
) const
    {
    return m_description;        
    }

/*---------------------------------------------------------------------------------**//**
 @bsimethod                                                     
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus ECClass::SetDescription
(
bwstring const& description
)
    {        
    m_description = description;
    return ECOBJECTS_STATUS_Success;
    }

/*---------------------------------------------------------------------------------**//**
 @bsimethod                                                     
+---------------+---------------+---------------+---------------+---------------+------*/
bwstring const& ECClass::GetDisplayLabel
(
) const
    {
    return (m_displayLabel.empty()) ? Name : m_displayLabel;
    }

/*---------------------------------------------------------------------------------**//**
 @bsimethod                                                     
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus ECClass::SetDisplayLabel
(
bwstring const& displayLabel
)
    {        
    m_displayLabel = displayLabel;
    return ECOBJECTS_STATUS_Success;
    }

/*---------------------------------------------------------------------------------**//**
 @bsimethod                                                     
+---------------+---------------+---------------+---------------+---------------+------*/
bool ECClass::GetIsDisplayLabelDefined
(
) const
    {
    return (!m_displayLabel.empty());        
    }

/*---------------------------------------------------------------------------------**//**
 @bsimethod                                                     
+---------------+---------------+---------------+---------------+---------------+------*/
bool ECClass::GetIsStruct
(
) const
    {
    return m_isStruct; 
    }

/*---------------------------------------------------------------------------------**//**
 @bsimethod                                                     
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus ECClass::SetIsStruct
(
bool isStruct
)
    {        
    m_isStruct = isStruct;
    return ECOBJECTS_STATUS_Success;
    }

/*---------------------------------------------------------------------------------**//**
 @bsimethod                                                     
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus ECClass::SetIsStruct
(
const wchar_t * isStruct
)
    {        
    PRECONDITION (NULL != isStruct, ECOBJECTS_STATUS_PreconditionViolated);

    ECObjectsStatus status = ECXml::ParseBooleanString (m_isStruct, isStruct);
    if (ECOBJECTS_STATUS_Success != status)
        ECObjectsLogger::Log()->warningv  (L"Failed to parse the isStruct string '%s' for ECClass '%s'.  Expected values are " ECXML_TRUE L" or " ECXML_FALSE L"\n", isStruct, this->Name.c_str());
        
    return status;
    }

/*---------------------------------------------------------------------------------**//**
 @bsimethod                                                     
+---------------+---------------+---------------+---------------+---------------+------*/
bool ECClass::GetIsCustomAttributeClass
(
) const
    {
    return m_isCustomAttributeClass; 
    }

/*---------------------------------------------------------------------------------**//**
 @bsimethod                                                     
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus ECClass::SetIsCustomAttributeClass
(
bool isCustomAttributeClass
)
    {        
    m_isCustomAttributeClass = isCustomAttributeClass;
    return ECOBJECTS_STATUS_Success;
    }

/*---------------------------------------------------------------------------------**//**
 @bsimethod                                                     
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus ECClass::SetIsCustomAttributeClass
(
const wchar_t * isCustomAttributeClass
)
    {      
    PRECONDITION (NULL != isCustomAttributeClass, ECOBJECTS_STATUS_PreconditionViolated);

    ECObjectsStatus status = ECXml::ParseBooleanString (m_isCustomAttributeClass, isCustomAttributeClass);
    if (ECOBJECTS_STATUS_Success != status)
        ECObjectsLogger::Log()->warningv  (L"Failed to parse the isCustomAttributeClass string '%s' for ECClass '%s'.  Expected values are " ECXML_TRUE L" or " ECXML_FALSE L"\n", isCustomAttributeClass, this->Name.c_str());
        
    return status;
    }

/*---------------------------------------------------------------------------------**//**
 @bsimethod                                                     
+---------------+---------------+---------------+---------------+---------------+------*/
bool ECClass::GetIsDomainClass
(
) const
    {
    return m_isDomainClass; 
    }

/*---------------------------------------------------------------------------------**//**
 @bsimethod                                                     
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus ECClass::SetIsDomainClass
(
bool isDomainClass
)
    {        
    m_isDomainClass = isDomainClass;
    return ECOBJECTS_STATUS_Success;
    }

/*---------------------------------------------------------------------------------**//**
 @bsimethod                                                     
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus ECClass::SetIsDomainClass
(
const wchar_t * isDomainClass
)
    {
    PRECONDITION (NULL != isDomainClass, ECOBJECTS_STATUS_PreconditionViolated);

    ECObjectsStatus status = ECXml::ParseBooleanString (m_isDomainClass, isDomainClass);
    if (ECOBJECTS_STATUS_Success != status)
        ECObjectsLogger::Log()->warningv  (L"Failed to parse the isDomainClass string '%s' for ECClass '%s'.  Expected values are " ECXML_TRUE L" or " ECXML_FALSE L"\n", isDomainClass, this->Name.c_str());
        
    return status;
    }

/*---------------------------------------------------------------------------------**//**
 @bsimethod                                                     
+---------------+---------------+---------------+---------------+---------------+------*/
ECSchemaCR ECClass::GetSchema
(
) const
    {
    return m_schema;
    }

/*---------------------------------------------------------------------------------**//**
 @bsimethod                                                     
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus ECClass::AddProperty
(
ECPropertyP&                 pProperty
)
    {
    PropertyMap::const_iterator propertyIterator;
    
    propertyIterator = m_propertyMap.find(pProperty->Name.c_str());
    if (m_propertyMap.end() != propertyIterator)
        {
        ECObjectsLogger::Log()->warningv  (L"Can not create property '%s' because it already exists in this ECClass", pProperty->Name.c_str());
        return ECOBJECTS_STATUS_NamedItemAlreadyExists;
        }

    // It isn't part of this schema, but does it exist as a property on a baseClass?
    ECPropertyP baseProperty = GetPropertyP(pProperty->Name);
    if (NULL == baseProperty)
        {
        m_propertyMap.insert (bpair<const wchar_t *, ECPropertyP> (pProperty->Name.c_str(), pProperty));
        m_propertyList.push_back(pProperty);
        return ECOBJECTS_STATUS_Success;
        }

    ECObjectsStatus status = CanPropertyBeOverridden(*baseProperty, *pProperty);
    if (ECOBJECTS_STATUS_Success != status)
        return status;

    pProperty->BaseProperty = baseProperty;
    m_propertyMap.insert (bpair<const wchar_t *, ECPropertyP> (pProperty->Name.c_str(), pProperty));
    m_propertyList.push_back(pProperty);
    return ECOBJECTS_STATUS_Success;
    }

/*---------------------------------------------------------------------------------**//**
 @bsimethod                                                     
+---------------+---------------+---------------+---------------+---------------+------*/
ECPropertyP ECClass::GetPropertyP
(
wchar_t const* propertyName
) const
    {
    PropertyMap::const_iterator  propertyIterator;
    propertyIterator = m_propertyMap.find (propertyName);
    
    if ( propertyIterator != m_propertyMap.end() )
        return propertyIterator->second;

    // not found yet, search the inheritence hierarchy
    for each (const ECClassP& baseClass in m_baseClasses)
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
bwstring const& propertyName
) const
    {
    return  GetPropertyP (propertyName.c_str());
    }

ECObjectsStatus ECClass::CanPropertyBeOverridden
(
ECPropertyCR baseProperty,
ECPropertyCR newProperty
) const
    {
    
    // If the type of base property is an array and the type of the current property is not an array (or vice-versa),
    // return an error immediately.  Unfortunately, there are a class of schemas that have been delivered with this type
    // of override.  So need to check if this is one of those schemas before returning an error
    if ((baseProperty.IsArray && !newProperty.IsArray) || (!baseProperty.IsArray && newProperty.IsArray))
        {
        //if (!EC::ECSchema::SchemaAllowsOverridingArrays(this->Schema))
        //    return ECOBJECTS_STATUS_DataTypeMismatch;
        }
    
    if (!newProperty._CanOverride(baseProperty))
        return ECOBJECTS_STATUS_DataTypeMismatch;
    return ECOBJECTS_STATUS_Success; 
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Carole.MacDonald                03/2010
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus ECClass::RemoveProperty
(
const bwstring &name
)
    {
    PropertyMap::iterator  propertyIterator;
    propertyIterator = m_propertyMap.find (name.c_str());
    
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
ECObjectsStatus ECClass::AddProperty
(
ECPropertyP ecProperty,
const bwstring &name
)
    {
    ECObjectsStatus status = ecProperty->SetName (name);
    if (ECOBJECTS_STATUS_Success != status)
        return status;

    return AddProperty (ecProperty);
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Carole.MacDonald                01/2010
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus ECClass::CreatePrimitiveProperty
(
PrimitiveECPropertyP &ecProperty, 
const bwstring &name
)
    {
    ecProperty = new PrimitiveECProperty(*this, m_hideFromLeakDetection);
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
ECObjectsStatus ECClass::CreatePrimitiveProperty
(
PrimitiveECPropertyP &ecProperty, 
const bwstring &name,
PrimitiveType primitiveType
)
    {
    ecProperty = new PrimitiveECProperty(*this, m_hideFromLeakDetection);
    ecProperty->Type = primitiveType;
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
ECObjectsStatus ECClass::CreateStructProperty
(
StructECPropertyP &ecProperty, 
const bwstring &name
)
    {
    ecProperty = new StructECProperty(*this, m_hideFromLeakDetection);
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
ECObjectsStatus ECClass::CreateStructProperty
(
StructECPropertyP &ecProperty, 
const bwstring &name,
ECClassCR structType
)
    {
    ecProperty = new StructECProperty(*this, m_hideFromLeakDetection);
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
ECObjectsStatus ECClass::CreateArrayProperty
(
ArrayECPropertyP &ecProperty, 
const bwstring &name
)
    {
    ecProperty = new ArrayECProperty(*this, m_hideFromLeakDetection);
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
ECObjectsStatus ECClass::CreateArrayProperty
(
ArrayECPropertyP &ecProperty, 
const bwstring &name,
PrimitiveType primitiveType
)
    {
    ecProperty = new ArrayECProperty(*this, m_hideFromLeakDetection);
    ecProperty->PrimitiveElementType = primitiveType;
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
ECObjectsStatus ECClass::CreateArrayProperty
(
ArrayECPropertyP &ecProperty, 
const bwstring &name,
ECClassCP structType
)
    {
    ecProperty = new ArrayECProperty(*this, m_hideFromLeakDetection);
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
bool ECClass::CheckBaseClassCycles
(
ECClassCP thisClass, 
const void * arg
)
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
ECObjectsStatus ECClass::AddBaseClass
(
ECClassCR baseClass
)
    {
    if (&(baseClass.Schema) != &(this->Schema))
        {
        if (!ECSchema::IsSchemaReferenced(this->Schema, baseClass.Schema))
            return ECOBJECTS_STATUS_SchemaNotFound;
        }

    if (this == &baseClass || ClassesAreEqualByName(this, &baseClass) || baseClass.TraverseBaseClasses(&CheckBaseClassCycles, true, this))
        return ECOBJECTS_STATUS_BaseClassUnacceptable;
        
    ECBaseClassesList::const_iterator baseClassIterator;
    for (baseClassIterator = m_baseClasses.begin(); baseClassIterator != m_baseClasses.end(); baseClassIterator++)
        {
        if (*baseClassIterator == (ECClassP)&baseClass)
            {
            ECObjectsLogger::Log()->warningv (L"Can not add class '%s' as a base class to '%s' because it already exists as a base class", baseClass.Name.c_str(), m_name.c_str());
            return ECOBJECTS_STATUS_NamedItemAlreadyExists;
            }
        }

    PropertyList baseClassProperties;
    ECObjectsStatus status = baseClass.GetProperties(true, &baseClassProperties);
    if (ECOBJECTS_STATUS_Success != status)
        return status;

    for each (ECPropertyP prop in baseClassProperties)
        {
        ECPropertyP thisProperty;
        if (NULL != (thisProperty = this->GetPropertyP(prop->Name)))
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
bool ECClass::HasBaseClasses
(
) const
    {
    return (m_baseClasses.size() > 0);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Carole.MacDonald                03/2010
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus ECClass::RemoveBaseClass
(
ECClassCR baseClass
)
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
        ECObjectsLogger::Log()->warningv(L"Class '%s' is not a base class of class '%s'", baseClass.Name, m_name);
        return ECOBJECTS_STATUS_ClassNotFound;
        }
        
    baseClass.RemoveDerivedClass(*this);

    return ECOBJECTS_STATUS_Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Carole.MacDonald                02/2010
+---------------+---------------+---------------+---------------+---------------+------*/
bool ECClass::Is
(
ECClassCP targetClass
) const
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
bool ECClass::ClassesAreEqualByName
(
ECClassCP thisClass, 
const void * arg
)
    {
    ECClassCP thatClass = static_cast<ECClassCP> (arg);
    if (NULL == arg)
        return true;
        
    return ((thisClass == thatClass) ||
            ( (0 == thisClass->Name.compare(thatClass->Name)) &&
              (0 == thisClass->Schema.Name.compare(thatClass->Schema.Name)) &&
              (thisClass->Schema.VersionMajor == thatClass->Schema.VersionMajor) &&
              (thisClass->Schema.VersionMinor == thatClass->Schema.VersionMinor)));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   
+---------------+---------------+---------------+---------------+---------------+------*/
ECPropertyIterable ECClass::GetProperties
(
) const
    {
    return ECPropertyIterable(*this, true);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Carole.MacDonald                04/2010
+---------------+---------------+---------------+---------------+---------------+------*/
ECPropertyIterable ECClass::GetProperties
(
bool includeBaseProperties
) const
    {
    return ECPropertyIterable(*this, includeBaseProperties);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus ECClass::GetProperties
(
bool includeBaseProperties,
PropertyList* propertyList
) const
    {
    for each (ECPropertyP prop in m_propertyList)
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
bool ECClass::AddUniquePropertiesToList
(
ECClassCP currentBaseClass, 
const void *arg
)
    {
    const PropertyList* props = static_cast<const PropertyList*>(arg);
    PropertyList* propertyList = const_cast<PropertyList*>(props);
    
    PropertyList newProperties;
    PropertyList::iterator currentEnd = propertyList->end();
    for each (ECPropertyP prop in currentBaseClass->GetProperties(false))
        {
        PropertyList::iterator testIter;
        for (testIter = propertyList->begin(); testIter != currentEnd; testIter++)
            {
            ECPropertyP testProperty = *testIter;
            if (testProperty->Name.compare(prop->Name) == 0)
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
bool ECClass::TraverseBaseClasses
(
TraversalDelegate traverseMethod, 
bool              recursive,
const void*       arg
) const
    {
    if (m_baseClasses.size() == 0)
        return false;
        
    for each (const ECClassP& baseClass in m_baseClasses)
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
SchemaDeserializationStatus ECClass::ReadXmlAttributes
(
MSXML2::IXMLDOMNode& classNode,
IStandaloneEnablerLocatorR  standaloneEnablerLocator
)
    {                
    MSXML2::IXMLDOMNamedNodeMapPtr nodeAttributesPtr = classNode.attributes;
    MSXML2::IXMLDOMNodePtr attributePtr;        

    if (m_name.length() == 0)
        {
        READ_REQUIRED_XML_ATTRIBUTE (TYPE_NAME_ATTRIBUTE,           this, Name,     classNode.baseName)    
        }
    
    // OPTIONAL attributes - If these attributes exist they MUST be valid    
    READ_OPTIONAL_XML_ATTRIBUTE (DESCRIPTION_ATTRIBUTE,         this, Description)
    READ_OPTIONAL_XML_ATTRIBUTE (DISPLAY_LABEL_ATTRIBUTE,       this, DisplayLabel)

    // OPTIONAL attributes - If these attributes exist they do not need to be valid.  We will ignore any errors setting them and use default values.
    // NEEDSWORK This is due to the current implementation in managed ECObjects.  We should reconsider whether it is the correct behavior.
    ECObjectsStatus setterStatus;
    READ_OPTIONAL_XML_ATTRIBUTE_IGNORING_SET_ERRORS (IS_STRUCT_ATTRIBUTE,           this, IsStruct)
    READ_OPTIONAL_XML_ATTRIBUTE_IGNORING_SET_ERRORS (IS_CUSTOMATTRIBUTE_ATTRIBUTE,  this, IsCustomAttributeClass)
    READ_OPTIONAL_XML_ATTRIBUTE_IGNORING_SET_ERRORS (IS_DOMAINCLASS_ATTRIBUTE,      this, IsDomainClass)

    // when isDomainClass is not specified in the ECSchemaXML and isCustomAttributeClass is specified and set to true, we will default to a non-domain class
    if ((NULL == attributePtr) && (this->IsCustomAttributeClass))
        this->SetIsDomainClass (false);

    return SCHEMA_DESERIALIZATION_STATUS_Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   
+---------------+---------------+---------------+---------------+---------------+------*/
SchemaDeserializationStatus ECClass::ReadXmlContents
(
MSXML2::IXMLDOMNode&        classNode,
IStandaloneEnablerLocatorR  standaloneEnablerLocator
)
    {            
    // Build inheritance hierarchy 
    MSXML2::IXMLDOMNodeListPtr xmlNodeListPtr = classNode.selectNodes (EC_NAMESPACE_PREFIX L":" EC_BASE_CLASS_ELEMENT);
    MSXML2::IXMLDOMNodePtr xmlNodePtr;
    while (NULL != (xmlNodePtr = xmlNodeListPtr->nextNode()))
        {        
        bwstring qualifiedClassName = xmlNodePtr->text;
        
        // Parse the potentially qualified class name into a namespace prefix and short class name
        bwstring namespacePrefix;
        bwstring className;
        if (ECOBJECTS_STATUS_Success != ECClass::ParseClassName (namespacePrefix, className, qualifiedClassName))
            {
            ECObjectsLogger::Log()->warningv (L"Invalid ECSchemaXML: The ECClass '%s' contains a " EC_BASE_CLASS_ELEMENT L" element with the value '%s' that can not be parsed.", 
                this->Name.c_str(), qualifiedClassName.c_str());
            return SCHEMA_DESERIALIZATION_STATUS_InvalidECSchemaXml;
            }
        
        ECSchemaP resolvedSchema = Schema.GetSchemaByNamespacePrefixP (namespacePrefix);
        if (NULL == resolvedSchema)
            {
            ECObjectsLogger::Log()->warningv  (L"Invalid ECSchemaXML: The ECClass '%s' contains a " EC_BASE_CLASS_ELEMENT L" element with the namespace prefix '%s' that can not be resolved to a referenced schema.", 
                this->Name.c_str(), namespacePrefix.c_str());
            return SCHEMA_DESERIALIZATION_STATUS_InvalidECSchemaXml;
            }

        ECClassP baseClass = resolvedSchema->GetClassP (className.c_str());
        if (NULL == baseClass)
            {
            ECObjectsLogger::Log()->warningv  (L"Invalid ECSchemaXML: The ECClass '%s' contains a " EC_BASE_CLASS_ELEMENT L" element with the value '%s' that can not be resolved to an ECClass named '%s' in the ECSchema '%s'", 
                this->Name.c_str(), qualifiedClassName.c_str(), className.c_str(), resolvedSchema->Name.c_str());
            return SCHEMA_DESERIALIZATION_STATUS_InvalidECSchemaXml;
            }

        if (ECOBJECTS_STATUS_Success != AddBaseClass(*baseClass))
            return SCHEMA_DESERIALIZATION_STATUS_InvalidECSchemaXml;
        }

    // Build properties
    xmlNodeListPtr = classNode.selectNodes (EC_NAMESPACE_PREFIX L":" EC_PROPERTY_ELEMENT L" | " EC_NAMESPACE_PREFIX L":" EC_ARRAYPROPERTY_ELEMENT L" | " EC_NAMESPACE_PREFIX L":" EC_STRUCTPROPERTY_ELEMENT);
    while (NULL != (xmlNodePtr = xmlNodeListPtr->nextNode()))
        {   
        ECPropertyP pProperty = NULL;
        if (0 == wcscmp (xmlNodePtr->baseName, EC_PROPERTY_ELEMENT))
            pProperty = new PrimitiveECProperty (*this, m_hideFromLeakDetection);
        else if (0 == wcscmp (xmlNodePtr->baseName, EC_ARRAYPROPERTY_ELEMENT))
            pProperty = new ArrayECProperty (*this, m_hideFromLeakDetection);
        else if (0 == wcscmp (xmlNodePtr->baseName, EC_STRUCTPROPERTY_ELEMENT))
            pProperty = new StructECProperty (*this, m_hideFromLeakDetection);
        else
            {
            ECObjectsLogger::Log()->warningv (L"Invalid ECSchemaXML: Unknown property type '%s' of ECClass '%s' in the ECSchema '%s'\n", xmlNodePtr->baseName, this->Name.c_str(), this->Schema.Name.c_str());
            return SCHEMA_DESERIALIZATION_STATUS_InvalidECSchemaXml;
            }

        SchemaDeserializationStatus status = pProperty->_ReadXml(xmlNodePtr, standaloneEnablerLocator);
        if (status != SCHEMA_DESERIALIZATION_STATUS_Success)
            {
            ECObjectsLogger::Log()->warningv  (L"Invalid ECSchemaXML: Failed to deserialize properties of ECClass '%s' in the ECSchema '%s'\n", this->Name.c_str(), this->Schema.Name.c_str());                
            delete pProperty;
            return status;
            }
        
        if (ECOBJECTS_STATUS_Success != this->AddProperty (pProperty))
            {
            ECObjectsLogger::Log()->warningv  (L"Invalid ECSchemaXML: Failed to deserialize ECClass '%s' in the ECSchema '%s' because a problem occurred while adding ECProperty '%s'\n", 
                this->Name.c_str(), this->Schema.Name.c_str(), pProperty->Name.c_str());
            delete pProperty;
            return SCHEMA_DESERIALIZATION_STATUS_InvalidECSchemaXml;
            }
        }

    // Add Custom Attributes
    ReadCustomAttributes(classNode, m_schema, standaloneEnablerLocator);

    return SCHEMA_DESERIALIZATION_STATUS_Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Carole.MacDonald                01/2010
+---------------+---------------+---------------+---------------+---------------+------*/
SchemaSerializationStatus ECClass::WriteXml
(
MSXML2::IXMLDOMElement &parentNode, 
const wchar_t *elementName
) const
    {
    SchemaSerializationStatus status = SCHEMA_SERIALIZATION_STATUS_Success;
    MSXML2::IXMLDOMTextPtr textPtr = NULL;
    MSXML2::IXMLDOMAttributePtr attributePtr;

    MSXML2::IXMLDOMElementPtr classPtr = NULL;
    
    classPtr = parentNode.ownerDocument->createNode(NODE_ELEMENT, elementName, ECXML_URI_2_0);
    
    APPEND_CHILD_TO_PARENT(classPtr, (&parentNode));
    
    WRITE_XML_ATTRIBUTE(TYPE_NAME_ATTRIBUTE, this->Name.c_str(), classPtr);
    WRITE_OPTIONAL_XML_ATTRIBUTE(DESCRIPTION_ATTRIBUTE, Description, classPtr);
    if (IsDisplayLabelDefined)
        WRITE_OPTIONAL_XML_ATTRIBUTE(DISPLAY_LABEL_ATTRIBUTE, DisplayLabel, classPtr);
    WRITE_OPTIONAL_BOOL_XML_ATTRIBUTE(IS_STRUCT_ATTRIBUTE, IsStruct, classPtr);
    WRITE_BOOL_XML_ATTRIBUTE(IS_DOMAINCLASS_ATTRIBUTE, IsDomainClass, classPtr);
    WRITE_OPTIONAL_BOOL_XML_ATTRIBUTE(IS_CUSTOMATTRIBUTE_ATTRIBUTE, IsCustomAttributeClass, classPtr);
    
    for each (const ECClassP& baseClass in m_baseClasses)
        {
        MSXML2::IXMLDOMElementPtr basePtr = parentNode.ownerDocument->createNode(NODE_ELEMENT, EC_BASE_CLASS_ELEMENT, ECXML_URI_2_0);
        basePtr->text = (ECClass::GetQualifiedClassName(Schema, *baseClass)).c_str();
        
        APPEND_CHILD_TO_PARENT(basePtr, classPtr);
        }

    WriteCustomAttributes(classPtr);
            
    for each (ECPropertyP prop in GetProperties(false))
        {
        prop->_WriteXml(classPtr);
        }
    return status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   
+---------------+---------------+---------------+---------------+---------------+------*/
SchemaSerializationStatus ECClass::WriteXml
(
MSXML2::IXMLDOMElement& parentNode
) const
    {
    return WriteXml(parentNode, EC_CLASS_ELEMENT);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus ECClass::ParseClassName 
(
bwstring & prefix, 
bwstring & className, 
bwstring const& qualifiedClassName
)
    {
    if (0 == qualifiedClassName.length())
        {
        ECObjectsLogger::Log()->warningv  (L"Failed to parse a prefix and class name from a qualified class name because the string is empty.");
        return ECOBJECTS_STATUS_ParseError;
        }
        
    bwstring::size_type colonIndex = qualifiedClassName.find (':');
    if (bwstring::npos == colonIndex)
        {
        prefix.clear();
        className = qualifiedClassName;
        return ECOBJECTS_STATUS_Success;
        }

    if (qualifiedClassName.length() == colonIndex + 1)
        {
        ECObjectsLogger::Log()->warningv  (L"Failed to parse a prefix and class name from the qualified class name '%s' because the string ends with a colon.  There must be characters after the colon.", 
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
bwstring ECClass::GetQualifiedClassName
(
ECSchemaCR primarySchema,
ECClassCR  ecClass
)
    {
    bwstring namespacePrefix;
    if (!EXPECTED_CONDITION (ECOBJECTS_STATUS_Success == primarySchema.ResolveNamespacePrefix (ecClass.Schema, namespacePrefix)))
        {
        ECObjectsLogger::Log()->warningv (L"warning: Can not qualify an ECClass name with a namespace prefix unless the schema containing the ECClass is referenced by the primary schema.\n"
            L"The class name will remain unqualified.\n  Primary ECSchema: %s\n  ECClass: %s\n ECSchema containing ECClass: %s\n", primarySchema.Name.c_str(), ecClass.Name.c_str(), ecClass.Schema.Name.c_str());
        return ecClass.Name;
        }
    if (namespacePrefix.empty())
        return ecClass.Name;
    else
        return namespacePrefix + L":" + ecClass.Name;
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
    for each (ECClassP baseClass in m_baseClasses)
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
ECPropertyP       ECPropertyIterable::const_iterator::operator*() const
    {
    if (m_isEnd)
        return NULL;
    ECPropertyP pProperty = *(m_state->m_listIterator);
    return pProperty;
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
bwstring RelationshipCardinality::ToString
(
) const
    {
    wchar_t cardinalityString[32];
    
    if (UINT_MAX == m_upperLimit)
        {
        swprintf(cardinalityString, 32, L"(%d,N)", m_lowerLimit);
        }
    else
        swprintf(cardinalityString, 32, L"(%d,%d)", m_lowerLimit, m_upperLimit);
        
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
    return &(m_relClass->Schema);
    }
 
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Carole.MacDonald                03/2010
+---------------+---------------+---------------+---------------+---------------+------*/
SchemaDeserializationStatus ECRelationshipConstraint::ReadXml
(
MSXML2::IXMLDOMNode         &constraintNode,
IStandaloneEnablerLocatorR  standaloneEnablerLocator
)
    {
    SchemaDeserializationStatus status = SCHEMA_DESERIALIZATION_STATUS_Success;
    
    MSXML2::IXMLDOMNamedNodeMapPtr nodeAttributesPtr = constraintNode.attributes;
    MSXML2::IXMLDOMNodePtr attributePtr;
 
    ECObjectsStatus setterStatus;
    READ_OPTIONAL_XML_ATTRIBUTE_IGNORING_SET_ERRORS(POLYMORPHIC_ATTRIBUTE, this, IsPolymorphic);
    READ_OPTIONAL_XML_ATTRIBUTE(ROLELABEL_ATTRIBUTE, this, RoleLabel);
    READ_OPTIONAL_XML_ATTRIBUTE(CARDINALITY_ATTRIBUTE, this, Cardinality);
    
    MSXML2::IXMLDOMNodeListPtr xmlNodeListPtr = constraintNode.selectNodes(EC_NAMESPACE_PREFIX L":" EC_CONSTRAINTCLASS_ELEMENT);
    MSXML2::IXMLDOMNodePtr xmlNodePtr;
    
    while (NULL != (xmlNodePtr = xmlNodeListPtr->nextNode()))
        {
        MSXML2::IXMLDOMNamedNodeMapPtr constraintClassAttributesPtr = xmlNodePtr->attributes;
        if (NULL == (attributePtr = constraintClassAttributesPtr->getNamedItem(CONSTRAINTCLASSNAME_ATTRIBUTE)))
            return SCHEMA_DESERIALIZATION_STATUS_InvalidECSchemaXml;
        bwstring constraintClassName = attributePtr->text;  
        
        // Parse the potentially qualified class name into a namespace prefix and short class name
        bwstring namespacePrefix;
        bwstring className;
        if (ECOBJECTS_STATUS_Success != ECClass::ParseClassName (namespacePrefix, className, constraintClassName))
            {
            ECObjectsLogger::Log()->warningv (L"Invalid ECSchemaXML: The ECRelationshipConstraint contains a " CONSTRAINTCLASSNAME_ATTRIBUTE L" attribute with the value '%s' that can not be parsed.", 
                constraintClassName.c_str());
            return SCHEMA_DESERIALIZATION_STATUS_InvalidECSchemaXml;
            }
        
        ECSchemaP resolvedSchema = m_relClass->Schema.GetSchemaByNamespacePrefixP (namespacePrefix);
        if (NULL == resolvedSchema)
            {
            ECObjectsLogger::Log()->warningv  (L"Invalid ECSchemaXML: ECRelationshipConstraint contains a " CONSTRAINTCLASSNAME_ATTRIBUTE L" attribute with the namespace prefix '%s' that can not be resolved to a referenced schema.", 
                namespacePrefix.c_str());
            return SCHEMA_DESERIALIZATION_STATUS_InvalidECSchemaXml;
            }

        ECClassP constraintClass = resolvedSchema->GetClassP (className.c_str());
        if (NULL == constraintClass)
            {
            ECObjectsLogger::Log()->warningv  (L"Invalid ECSchemaXML: The ECRelationshipConstraint contains a " CONSTRAINTCLASSNAME_ATTRIBUTE L" attribute with the value '%s' that can not be resolved to an ECClass named '%s' in the ECSchema '%s'", 
                constraintClassName.c_str(), className.c_str(), resolvedSchema->Name.c_str());
            return SCHEMA_DESERIALIZATION_STATUS_InvalidECSchemaXml;
            }
        AddClass(*constraintClass);
        }

    // Add Custom Attributes
    ReadCustomAttributes(constraintNode, m_relClass->Schema, standaloneEnablerLocator);

    return status;
    }
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Carole.MacDonald                03/2010
+---------------+---------------+---------------+---------------+---------------+------*/
SchemaSerializationStatus ECRelationshipConstraint::WriteXml
(
MSXML2::IXMLDOMElement &parentNode, 
const bwstring &elementName
) const
    {
    SchemaSerializationStatus status = SCHEMA_SERIALIZATION_STATUS_Success;
    MSXML2::IXMLDOMTextPtr textPtr = NULL;
    MSXML2::IXMLDOMAttributePtr attributePtr;

    MSXML2::IXMLDOMElementPtr constraintPtr = NULL;
    
    constraintPtr = parentNode.ownerDocument->createNode(NODE_ELEMENT, elementName.c_str(), ECXML_URI_2_0);
    
    APPEND_CHILD_TO_PARENT(constraintPtr, (&parentNode));
    
    WRITE_XML_ATTRIBUTE(CARDINALITY_ATTRIBUTE, m_cardinality->ToString().c_str(), constraintPtr);
    if (IsRoleLabelDefined())
        {
        WRITE_XML_ATTRIBUTE(ROLELABEL_ATTRIBUTE, m_roleLabel.c_str(), constraintPtr);
        }
    WRITE_BOOL_XML_ATTRIBUTE(POLYMORPHIC_ATTRIBUTE, IsPolymorphic, constraintPtr);
        
    WriteCustomAttributes(constraintPtr);

    for each (ECClassP constraint in m_constraintClasses)
        {
        MSXML2::IXMLDOMElementPtr constraintClassPtr = NULL;
        constraintClassPtr = parentNode.ownerDocument->createNode(NODE_ELEMENT, EC_CONSTRAINTCLASS_ELEMENT, ECXML_URI_2_0);
        APPEND_CHILD_TO_PARENT(constraintClassPtr, constraintPtr);
        WRITE_XML_ATTRIBUTE(CONSTRAINTCLASSNAME_ATTRIBUTE, ECClass::GetQualifiedClassName(m_relClass->Schema, *constraint).c_str(), constraintClassPtr);
        }
    
    return status;
    }
   
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Carole.MacDonald                03/2010
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus ECRelationshipConstraint::AddClass
(
ECClassCR classConstraint
)
    {
    if (&(classConstraint.Schema) != &(m_relClass->Schema))
        {
        bool foundRefSchema = false;
        ECSchemaReferenceList referencedSchemas = m_relClass->Schema.GetReferencedSchemas();
        ECSchemaReferenceList::const_iterator schemaIterator;
        for (schemaIterator = referencedSchemas.begin(); schemaIterator != referencedSchemas.end(); schemaIterator++)
            {
            ECSchemaP refSchema = *schemaIterator;
            if (refSchema == &(classConstraint.Schema))
                {
                foundRefSchema = true;
                break;
                }
            }
        if (foundRefSchema == false)
            {
            return ECOBJECTS_STATUS_SchemaNotFound;
            }
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
ECObjectsStatus ECRelationshipConstraint::RemoveClass
(
ECClassCR classConstraint
)
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
const ECConstraintClassesList& ECRelationshipConstraint::GetClasses
(
) const
    {
    return m_constraintClasses;
    }
     
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Carole.MacDonald                02/2010
+---------------+---------------+---------------+---------------+---------------+------*/
bool ECRelationshipConstraint::GetIsMultiple
(
) const
    {
    return m_isMultiple;
    }
  
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Carole.MacDonald                02/2010
+---------------+---------------+---------------+---------------+---------------+------*/
bool ECRelationshipConstraint::GetIsPolymorphic
(
) const
    {
    return m_isPolymorphic;
    }
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Carole.MacDonald                02/2010
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus ECRelationshipConstraint::SetIsPolymorphic
(
bool value
)
    {
    m_isPolymorphic = value;
    return ECOBJECTS_STATUS_Success;
    }
   
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Carole.MacDonald                03/2010
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus ECRelationshipConstraint::SetIsPolymorphic
(
const wchar_t *isPolymorphic
)
    {
    PRECONDITION (NULL != isPolymorphic, ECOBJECTS_STATUS_PreconditionViolated);

    ECObjectsStatus status = ECXml::ParseBooleanString (m_isPolymorphic, isPolymorphic);
    if (ECOBJECTS_STATUS_Success != status)
        ECObjectsLogger::Log()->warningv  (L"Failed to parse the isPolymorphic string '%s' for ECRelationshipConstraint.  Expected values are " ECXML_TRUE L" or " ECXML_FALSE L"\n", isPolymorphic);
        
    return status;
    }
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Carole.MacDonald                03/2010
+---------------+---------------+---------------+---------------+---------------+------*/
RelationshipCardinalityCR ECRelationshipConstraint::GetCardinality
(
) const
    {
    return *m_cardinality;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Carole.MacDonald                03/2010
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus ECRelationshipConstraint::SetCardinality
(
UInt32& lowerLimit,
UInt32& upperLimit
)
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
ECObjectsStatus ECRelationshipConstraint::SetCardinality
(
RelationshipCardinalityCR cardinality
)
    {
    m_cardinality = new RelationshipCardinality(cardinality.LowerLimit, cardinality.UpperLimit);
    return ECOBJECTS_STATUS_Success;
    }
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Carole.MacDonald                03/2010
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus ECRelationshipConstraint::SetCardinality
(
const wchar_t *cardinality
)
    {
    PRECONDITION (NULL != cardinality, ECOBJECTS_STATUS_PreconditionViolated);
    UInt32 lowerLimit;
    UInt32 upperLimit;
    ECObjectsStatus status = ECXml::ParseCardinalityString(lowerLimit, upperLimit, cardinality);
    if (ECOBJECTS_STATUS_Success != status)
        {
        ECObjectsLogger::Log()->errorv (L"Failed to parse the RelationshipCardinality string '%s'.\n", cardinality);
        return ECOBJECTS_STATUS_ParseError;
        }
    else
        m_cardinality = new RelationshipCardinality(lowerLimit, upperLimit);
        
    return ECOBJECTS_STATUS_Success;
    }
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Carole.MacDonald                03/2010
+---------------+---------------+---------------+---------------+---------------+------*/
bool ECRelationshipConstraint::IsRoleLabelDefined
(
) const
    {
    return m_roleLabel.length() != 0;
    }
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Carole.MacDonald                03/2010
+---------------+---------------+---------------+---------------+---------------+------*/
bwstring const ECRelationshipConstraint::GetRoleLabel
(
) const
    {
    if (m_roleLabel.length() != 0)
        return m_roleLabel;
        
    if (&(m_relClass->Target) == this)
        return m_relClass->DisplayLabel + L" (Reversed)";
    return m_relClass->DisplayLabel;
    }
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Carole.MacDonald                03/2010
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus ECRelationshipConstraint::SetRoleLabel
(
const bwstring value
)
    {
    m_roleLabel = value;
    return ECOBJECTS_STATUS_Success;
    }
  
     
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Carole.MacDonald                03/2010
+---------------+---------------+---------------+---------------+---------------+------*/
ECRelationshipClass::ECRelationshipClass
(
EC::ECSchemaCR schema
): ECClass (schema, false), m_strength( STRENGTHTYPE_Referencing), m_strengthDirection(STRENGTHDIRECTION_Forward) 
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
StrengthType ECRelationshipClass::GetStrength
(
) const
    {
    return m_strength;
    }
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Carole.MacDonald                02/2010
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus ECRelationshipClass::SetStrength
(
StrengthType strength
)
    {
    m_strength = strength;
    return ECOBJECTS_STATUS_Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Carole.MacDonald                02/2010
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus ECRelationshipClass::SetStrength
(
const wchar_t *strength
)
    {
    PRECONDITION (NULL != strength, ECOBJECTS_STATUS_PreconditionViolated);

    StrengthType strengthType;
    ECObjectsStatus status = ECXml::ParseStrengthType(strengthType, strength);
    if (ECOBJECTS_STATUS_Success != status)
        ECObjectsLogger::Log()->errorv (L"Failed to parse the Strength string '%s' for ECRelationshipClass '%s'.\n", strength, this->Name.c_str());
    else
        SetStrength (strengthType);
        
    return status;
    }
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Carole.MacDonald                02/2010
+---------------+---------------+---------------+---------------+---------------+------*/
ECRelatedInstanceDirection ECRelationshipClass::GetStrengthDirection
(
) const
    {
    return m_strengthDirection;
    }
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Carole.MacDonald                02/2010
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus ECRelationshipClass::SetStrengthDirection
(
ECRelatedInstanceDirection direction
)
    {
    m_strengthDirection = direction;
    return ECOBJECTS_STATUS_Success;
    }
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Carole.MacDonald                02/2010
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus ECRelationshipClass::SetStrengthDirection
(
const wchar_t *directionString
)
    {
    PRECONDITION (NULL != directionString, ECOBJECTS_STATUS_PreconditionViolated);

    ECRelatedInstanceDirection direction;
    ECObjectsStatus status = ECXml::ParseDirectionString(direction, directionString);
    if (ECOBJECTS_STATUS_Success != status)
        ECObjectsLogger::Log()->errorv (L"Failed to parse the ECRelatedInstanceDirection string '%s' for ECRelationshipClass '%s'.\n", directionString, this->Name.c_str());
    else
        SetStrengthDirection (direction);
        
    return status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Carole.MacDonald                03/2010
+---------------+---------------+---------------+---------------+---------------+------*/
ECRelationshipConstraintR ECRelationshipClass::GetSource
(
) const
    {
    return *m_source;
    }
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Carole.MacDonald                03/2010
+---------------+---------------+---------------+---------------+---------------+------*/
ECRelationshipConstraintR ECRelationshipClass::GetTarget
(
) const
    {
    return *m_target;
    }
        
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   
+---------------+---------------+---------------+---------------+---------------+------*/
SchemaSerializationStatus ECRelationshipClass::WriteXml
(
MSXML2::IXMLDOMElement& parentNode
) const
    {
        
    SchemaSerializationStatus status = __super::WriteXml(parentNode, EC_RELATIONSHIP_CLASS_ELEMENT);
    
    if (status != SCHEMA_SERIALIZATION_STATUS_Success)
        return status;
        
    MSXML2::IXMLDOMAttributePtr attributePtr;

    MSXML2::IXMLDOMElementPtr propertyPtr = parentNode.lastChild;
    if (NULL == propertyPtr)
        return SCHEMA_SERIALIZATION_STATUS_FailedToCreateXml;
        
    // verify that this really is the current relationship class element
    if (wcscmp(propertyPtr->nodeName, EC_RELATIONSHIP_CLASS_ELEMENT) != 0)
        return SCHEMA_SERIALIZATION_STATUS_FailedToCreateXml;
        
    WRITE_XML_ATTRIBUTE(STRENGTH_ATTRIBUTE, ECXml::StrengthToString(m_strength).c_str(), propertyPtr);
    WRITE_XML_ATTRIBUTE(STRENGTHDIRECTION_ATTRIBUTE, ECXml::DirectionToString(m_strengthDirection).c_str(), propertyPtr);
    
    m_source->WriteXml(propertyPtr, EC_SOURCECONSTRAINT_ELEMENT);
    m_target->WriteXml(propertyPtr, EC_TARGETCONSTRAINT_ELEMENT);
    
    return status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Carole.MacDonald                02/2010
+---------------+---------------+---------------+---------------+---------------+------*/
SchemaDeserializationStatus ECRelationshipClass::ReadXmlAttributes
(
MSXML2::IXMLDOMNode &classNode, 
IStandaloneEnablerLocatorR  standaloneEnablerLocator
)
    {
    SchemaDeserializationStatus status = __super::ReadXmlAttributes(classNode, standaloneEnablerLocator);
    if (status != SCHEMA_DESERIALIZATION_STATUS_Success)
        return status;
        
    MSXML2::IXMLDOMNamedNodeMapPtr nodeAttributesPtr = classNode.attributes;
    MSXML2::IXMLDOMNodePtr attributePtr;
    
    READ_OPTIONAL_XML_ATTRIBUTE (STRENGTH_ATTRIBUTE, this, Strength)
    READ_OPTIONAL_XML_ATTRIBUTE (STRENGTHDIRECTION_ATTRIBUTE, this, StrengthDirection)
    
    return SCHEMA_DESERIALIZATION_STATUS_Success;
    }
 
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Carole.MacDonald                02/2010
+---------------+---------------+---------------+---------------+---------------+------*/
SchemaDeserializationStatus ECRelationshipClass::ReadXmlContents
(
MSXML2::IXMLDOMNode &classNode, 
IStandaloneEnablerLocatorR  standaloneEnablerLocator
)
    {
    SchemaDeserializationStatus status = __super::ReadXmlContents(classNode, standaloneEnablerLocator);
    if (status != SCHEMA_DESERIALIZATION_STATUS_Success)
        return status;
        
    MSXML2::IXMLDOMNodePtr xmlNodePtr = classNode.selectSingleNode (EC_NAMESPACE_PREFIX L":" EC_SOURCECONSTRAINT_ELEMENT);
    if (NULL != xmlNodePtr)
        m_source->ReadXml(xmlNodePtr, standaloneEnablerLocator);
    
    xmlNodePtr = classNode.selectSingleNode (EC_NAMESPACE_PREFIX L":" EC_TARGETCONSTRAINT_ELEMENT);
    if (NULL != xmlNodePtr)
        m_target->ReadXml(xmlNodePtr, standaloneEnablerLocator);
        
    return SCHEMA_DESERIALIZATION_STATUS_Success;
    }
    
END_BENTLEY_EC_NAMESPACE
