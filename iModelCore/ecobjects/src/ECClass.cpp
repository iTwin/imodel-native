/*--------------------------------------------------------------------------------------+
|
|     $Source: src/ECClass.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#include "ECObjectsPch.h"

BEGIN_BENTLEY_ECOBJECT_NAMESPACE

extern ECObjectsStatus ResolveStructType(ECStructClassCP& structClass, Utf8StringCR typeName, ECClassCR ecClass, bool doLogging);

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
    m_schema(schema), m_modifier(ECClassModifier::None), m_xmlComments(), m_contentXmlComments()
    {
    //
    };

/*---------------------------------------------------------------------------------**//**
 @bsimethod                                                 
+---------------+---------------+---------------+---------------+---------------+------*/
ECClass::~ECClass ()
    {
    RemoveDerivedClasses ();
    RemoveBaseClasses ();

    m_propertyList.clear();
    m_xmlComments.clear();
    m_contentXmlComments.clear();
    
    for (PropertyMap::iterator entry=m_propertyMap.begin(); entry != m_propertyMap.end(); ++entry)
        delete entry->second;
    
    m_propertyMap.clear();

    m_defaultStandaloneEnabler = NULL;
    }

/*---------------------------------------------------------------------------------**//**
 @bsimethod                                                     
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8StringCR ECClass::GetName () const
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
Utf8CP ECClass::GetFullName () const
    {
    if (m_fullName.empty())
        m_fullName = GetSchema().GetName() + ":" + GetName();
        
    return m_fullName.c_str();
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                 11/2015
//+---------------+---------------+---------------+---------------+---------------+------
Utf8StringCR ECClass::GetECSqlName() const
    {
    if (m_ecsqlName.empty())
        m_ecsqlName.append("[").append(GetSchema().GetName()).append("].[").append(GetName()).append("]");

    return m_ecsqlName;
    }

/*---------------------------------------------------------------------------------**//**
 @bsimethod                                                     
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus ECClass::SetName (Utf8StringCR name)
    {
    if (!ECNameValidation::IsValidName (name.c_str()))
        return ECObjectsStatus::InvalidName;

    m_validatedName.SetName (name.c_str());
    m_fullName = GetSchema().GetName() + ":" + GetName();
    
    return ECObjectsStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
 @bsimethod                                                     
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8StringCR ECClass::GetDescription () const
    {
    return GetSchema().GetLocalizedStrings().GetClassDescription(this, m_description);
    }

/*---------------------------------------------------------------------------------**//**
 @bsimethod                                                     
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8StringCR ECClass::GetInvariantDescription () const
    {
    return m_description;
    }

/*---------------------------------------------------------------------------------**//**
 @bsimethod                                                     
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus ECClass::SetDescription (Utf8StringCR description)
    {        
    m_description = description;
    return ECObjectsStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
 @bsimethod                                                     
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8StringCR ECClass::GetDisplayLabel () const
    {
    return GetSchema().GetLocalizedStrings().GetClassDisplayLabel(this, GetInvariantDisplayLabel());
    }

/*---------------------------------------------------------------------------------**//**
 @bsimethod                                                     
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8StringCR ECClass::GetInvariantDisplayLabel() const
    {
    return m_validatedName.GetDisplayLabel();
    }

/*---------------------------------------------------------------------------------**//**
 @bsimethod                                                     
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus ECClass::SetDisplayLabel (Utf8StringCR displayLabel)
    {        
    m_validatedName.SetDisplayLabel (displayLabel.c_str());
    return ECObjectsStatus::Success;
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
ECSchemaCR ECClass::GetSchema () const
    {
    return m_schema;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            10/2015
//---------------+---------------+---------------+---------------+---------------+-------
ECClassType ECClass::GetClassType() const
    {
    return _GetClassType();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            10/2015
//---------------+---------------+---------------+---------------+---------------+-------
ECEntityClassCP ECClass::GetEntityClassCP() const
    {
    return _GetEntityClassCP();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            10/2015
//---------------+---------------+---------------+---------------+---------------+-------
ECEntityClassP ECClass::GetEntityClassP()
    {
    return _GetEntityClassP();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            10/2015
//---------------+---------------+---------------+---------------+---------------+-------
ECCustomAttributeClassCP ECClass::GetCustomAttributeClassCP() const
    {
    return _GetCustomAttributeClassCP();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            10/2015
//---------------+---------------+---------------+---------------+---------------+-------
ECCustomAttributeClassP ECClass::GetCustomAttributeClassP()
    {
    return _GetCustomAttributeClassP();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            10/2015
//---------------+---------------+---------------+---------------+---------------+-------
ECStructClassCP ECClass::GetStructClassCP() const
    {
    return _GetStructClassCP();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            10/2015
//---------------+---------------+---------------+---------------+---------------+-------
ECStructClassP ECClass::GetStructClassP()
    {
    return _GetStructClassP();
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

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            10/2015
//---------------+---------------+---------------+---------------+---------------+-------
ECClassModifier ECClass::GetClassModifier() const
    {
    return m_modifier;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            10/2015
//---------------+---------------+---------------+---------------+---------------+-------
void ECClass::SetClassModifier(ECClassModifier modifier)
    {
    m_modifier = modifier;
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
        {
        if (ECObjectsStatus::Success != (*found)->SetBaseProperty (baseProperty.GetBaseProperty()))
            (*found)->SetBaseProperty(nullptr); // see comments in SetBaseProperty()
        }
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
        return ECObjectsStatus::PropertyNotFound;

    m_propertyMap.erase (iter);
    m_propertyList.erase (std::find (m_propertyList.begin(), m_propertyList.end(), &prop));

    InvalidateDefaultStandaloneEnabler();

    for (ECClassP derivedClass : m_derivedClasses)
        derivedClass->OnBaseClassPropertyRemoved (prop);

    return ECObjectsStatus::Success;
    }
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   03/13
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus ECClass::DeleteProperty (ECPropertyR prop)
    {
    ECObjectsStatus status = RemoveProperty (prop);
    if (ECObjectsStatus::Success == status)
        delete &prop;

    return status;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            02/2016
//---------------+---------------+---------------+---------------+---------------+-------
void ECClass::FindUniquePropertyName(Utf8StringR newName, Utf8CP prefix, Utf8CP originalName)
    {
    Utf8PrintfString testName("%s_%s", prefix, originalName);
    bool conflict = true;
    while (conflict)
        {
        PropertyMap::iterator iter = m_propertyMap.find(testName.c_str());
        if (iter == m_propertyMap.end())
            conflict = false;
        testName.append("_");
        }
    newName = testName;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            02/2016
//---------------+---------------+---------------+---------------+---------------+-------
ECObjectsStatus ECClass::RenameConflictProperty(ECPropertyP prop, bool renameDerivedProperties)
    {
    Utf8String newName;
    FindUniquePropertyName(newName, prop->GetClass().GetSchema().GetAlias().c_str(), prop->GetName().c_str());
    return RenameConflictProperty(prop, renameDerivedProperties, newName);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            02/2016
//---------------+---------------+---------------+---------------+---------------+-------
ECObjectsStatus ECClass::RenameConflictProperty(ECPropertyP prop, bool renameDerivedProperties, Utf8String newName)
    {
    PropertyMap::iterator iter = m_propertyMap.find(prop->GetName().c_str());
    if (iter == m_propertyMap.end())
        return ECObjectsStatus::PropertyNotFound;
    ECPropertyP thisProp = iter->second; // Since the property that is passed in might come from a base class, we need the actual pointer of the property from this class in order to search the propertyList for it

    ECPropertyP newProperty;
    ECObjectsStatus status;
    if (ECObjectsStatus::Success != (status = CopyProperty(newProperty, prop, newName.c_str(), true, false)))
        {
        delete newProperty;
        return status;
        }

    iter = m_propertyMap.find(prop->GetName().c_str());
    m_propertyMap.erase(iter);
    auto iter2 = std::find(m_propertyList.begin(), m_propertyList.end(), thisProp);
    if (iter2 != m_propertyList.end())
        m_propertyList.erase(iter2);
    InvalidateDefaultStandaloneEnabler();

    status = AddProperty(newProperty, newName);
    if (ECObjectsStatus::Success != status)
        {
        delete newProperty;
        return status;
        }

    if (renameDerivedProperties)
        {
        for (ECClassP derivedClass : m_derivedClasses)
            {
            ECPropertyP fromDerived = derivedClass->GetPropertyP(newName.c_str(), false);
            if (nullptr != fromDerived && !fromDerived->GetName().Equals(newName))
                derivedClass->RenameConflictProperty(fromDerived, renameDerivedProperties, newName);
            for (ECClassP derivedClassBaseClass : derivedClass->GetBaseClasses())
                {
                ECPropertyP fromBaseDerived = derivedClassBaseClass->GetPropertyP(newName.c_str(), true);
                if (nullptr == fromBaseDerived || fromBaseDerived->GetName().Equals(newName))
                    continue;
                derivedClassBaseClass->RenameConflictProperty(fromBaseDerived, true, newName);
                }
            }
        }
    return ECObjectsStatus::Success;
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
        return ECObjectsStatus::OperationNotSupported;

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
        return ECObjectsStatus::PropertyNotFound;

    switch (kind)
        {
    case VALUEKIND_Primitive:   newProperty = new PrimitiveECProperty (*this); break;
    case VALUEKIND_Array:       newProperty = new ArrayECProperty (*this); break;
    case VALUEKIND_Struct:      newProperty = new StructECProperty (*this); break;
    default:                    return ECObjectsStatus::Error;
        }

    m_propertyMap.erase (m_propertyMap.find (propertyToRemove.GetName().c_str()));

    newProperty->SetName (propertyToRemove.GetName());
    m_propertyMap[newProperty->GetName().c_str()] = newProperty;
    m_propertyList[propertyIndex] = newProperty;

    delete &propertyToRemove;

    InvalidateDefaultStandaloneEnabler();

    return ECObjectsStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   03/13
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus ECClass::RenameProperty (ECPropertyR prop, Utf8CP newName)
    {
    if (HasBaseClasses() || 0 < m_derivedClasses.size())
        return ECObjectsStatus::OperationNotSupported;

    ECObjectsStatus status = RemoveProperty (prop);
    if (ECObjectsStatus::Success == status)
        {
        ECPropertyP propertyP = &prop;
        Utf8String oldName = prop.GetName();

        status = prop.SetName (newName);
        if (ECObjectsStatus::Success == status)
            {
            status = AddProperty (propertyP);
            if (ECObjectsStatus::Success != status)
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
ECObjectsStatus ECClass::OnBaseClassPropertyAdded (ECPropertyCR baseProperty, bool resolveConflicts)
    {
    InvalidateDefaultStandaloneEnabler();

    // This is a case-insensitive search
    ECPropertyP derivedProperty = GetPropertyP (baseProperty.GetName(), false);
    ECObjectsStatus status = ECObjectsStatus::Success;
    if (nullptr != derivedProperty)
        {
        // If the property names do not have the same case, this is an error
        if (!baseProperty.GetName().Equals(derivedProperty->GetName()))
            {
            if (!resolveConflicts)
                return ECObjectsStatus::CaseCollision;
            LOG.debugv("Case-collision between %s:%s and %s:%s", baseProperty.GetClass().GetFullName(), baseProperty.GetName().c_str(), GetFullName(), derivedProperty->GetName().c_str());
            RenameConflictProperty(derivedProperty, true);
            }

        // TFS#246533: Silly multiple inheritance scenarios...does derived property already have a different base property? Does the new property
        // have priority over that one based on the order of base class declarations?
        else if (nullptr == derivedProperty->GetBaseProperty() || GetBaseClassPropertyP (baseProperty.GetName().c_str()) == &baseProperty)
            {
            if (ECObjectsStatus::Success == (status = CanPropertyBeOverridden (baseProperty, *derivedProperty)))
                derivedProperty->SetBaseProperty (&baseProperty);
            else if (ECObjectsStatus::DataTypeMismatch == status && resolveConflicts)
                {
                LOG.debugv("DataTypeMismatch when adding base property '%s:%s' to '%s:%s;", baseProperty.GetClass().GetFullName(), baseProperty.GetName().c_str(), GetFullName(), GetName().c_str());
                RenameConflictProperty(derivedProperty, true);
                }
            }
        }
    for (ECClassP derivedClass : m_derivedClasses)
        status = derivedClass->OnBaseClassPropertyAdded (baseProperty, resolveConflicts);

    return status;
    }

/*---------------------------------------------------------------------------------**//**
 @bsimethod                                                     
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus ECClass::AddProperty (ECPropertyP& pProperty, bool resolveConflicts)
    {
    PropertyMap::const_iterator propertyIterator = m_propertyMap.find(pProperty->GetName().c_str());
    if (m_propertyMap.end() != propertyIterator)
        {
        if (!resolveConflicts)
            {
            LOG.errorv("Cannot create property '%s' because it already exists in this ECClass (%s:%s)", pProperty->GetName().c_str(), pProperty->GetClass().GetSchema().GetFullSchemaName().c_str(),
                       pProperty->GetClass().GetName().c_str());
            return ECObjectsStatus::NamedItemAlreadyExists;
            }
        Utf8String newName;
        FindUniquePropertyName(newName, pProperty->GetClass().GetSchema().GetAlias().c_str(), pProperty->GetName().c_str());
        pProperty->SetName(newName);
        }

    // It isn't part of this schema, but does it exist as a property on a baseClass?
    ECPropertyP baseProperty = GetPropertyP(pProperty->GetName());
    if (NULL != baseProperty)
        {
        ECObjectsStatus status = CanPropertyBeOverridden (*baseProperty, *pProperty);
        if (ECObjectsStatus::Success != status)
            {
            if (!resolveConflicts)
                return status;
            else
                {
                Utf8String newName;
                FindUniquePropertyName(newName, pProperty->GetClass().GetSchema().GetAlias().c_str(), pProperty->GetName().c_str());
                pProperty->SetName(newName);
                }
            }
        else if (!baseProperty->GetName().Equals(pProperty->GetName()) && resolveConflicts)
            pProperty->SetName(baseProperty->GetName());
        pProperty->SetBaseProperty (baseProperty);
        }

    m_propertyMap.insert (bpair<Utf8CP, ECPropertyP> (pProperty->GetName().c_str(), pProperty));
    m_propertyList.push_back(pProperty);

    InvalidateDefaultStandaloneEnabler();

    for (ECClassP derivedClass : m_derivedClasses)
        derivedClass->OnBaseClassPropertyAdded (*pProperty, false);

    return ECObjectsStatus::Success;
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
    return CopyProperty(destProperty, sourceProperty, sourceProperty->GetName().c_str(), copyCustomAttributes);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            02/2016
//---------------+---------------+---------------+---------------+---------------+-------
ECObjectsStatus ECClass::CopyProperty
(
ECPropertyP& destProperty, 
ECPropertyCP sourceProperty,
Utf8CP destPropertyName,
bool copyCustomAttributes,
bool andAddProperty
)
    {
    if (sourceProperty->GetIsPrimitive())
        {
        PrimitiveECPropertyP destPrimitive;
        PrimitiveECPropertyCP sourcePrimitive = sourceProperty->GetAsPrimitiveProperty();
        destPrimitive = new PrimitiveECProperty(*this);
        ECEnumerationCP enumeration = sourcePrimitive->GetEnumeration();
        if (enumeration != nullptr)
            {
            destPrimitive->SetType(*enumeration);
            }
        else
            {
            destPrimitive->SetType(sourcePrimitive->GetType());
            }

        destProperty = destPrimitive;
        }
    else if (sourceProperty->GetIsStructArray())
        {
        StructArrayECPropertyP destArray;
        StructArrayECPropertyCP sourceArray = sourceProperty->GetAsStructArrayProperty();
        destArray = new StructArrayECProperty(*this);
        ECStructClassCP structElementType = sourceArray->GetStructElementType();
        destArray->SetStructElementType(structElementType);

        destArray->SetMaxOccurs(sourceArray->GetMaxOccurs());
        destArray->SetMinOccurs(sourceArray->GetMinOccurs());

        destProperty = destArray;
        }
    else if (sourceProperty->GetIsArray())
        {
        ArrayECPropertyP destArray;
        ArrayECPropertyCP sourceArray = sourceProperty->GetAsArrayProperty();
        destArray = new ArrayECProperty(*this);
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
        ECStructClassCR sourceType = sourceStruct->GetType();
        destStruct->SetType(sourceType);
        destProperty = destStruct;
        }

    destProperty->SetDescription(sourceProperty->GetInvariantDescription());
    if (sourceProperty->GetIsDisplayLabelDefined())
        destProperty->SetDisplayLabel(sourceProperty->GetInvariantDisplayLabel());
    destProperty->SetName(sourceProperty->GetName());
    destProperty->SetIsReadOnly(sourceProperty->IsReadOnlyFlagSet());
    if (copyCustomAttributes)
        sourceProperty->CopyCustomAttributesTo(*destProperty);

    ECObjectsStatus status = ECObjectsStatus::Success;
    if (andAddProperty)
        {
        status = AddProperty(destProperty, Utf8String(destPropertyName));
        if (ECObjectsStatus::Success != status)
            delete destProperty;
        }
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
    if (ECObjectsStatus::Success == status)
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
    Utf8String propName;
    BeStringUtilities::WCharToUtf8(propName, propertyName);
    PropertyMap::const_iterator  propertyIterator = m_propertyMap.find (propName.c_str());
    
    if ( propertyIterator != m_propertyMap.end() )
        return propertyIterator->second;
    else
        return includeBaseClasses ? GetBaseClassPropertyP (propName.c_str()) : NULL;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   07/15
+---------------+---------------+---------------+---------------+---------------+------*/
ECPropertyP ECClass::GetBaseClassPropertyP (Utf8CP propertyName) const
    {
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
Utf8StringCR propertyName,
bool includeBaseClasses
) const
    {
    return  GetPropertyP (propertyName.c_str(), includeBaseClasses);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   12/12
+---------------+---------------+---------------+---------------+---------------+------*/
ECPropertyP ECClass::GetPropertyP (Utf8CP name, bool includeBaseClasses) const
    {
    PropertyMap::const_iterator found = m_propertyMap.find(name);
    if (m_propertyMap.end() != found)
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

static const Utf8CP s_schemasThatAllowOverridingArrays[] =
    {
    "jclass.01",
    "jclass.02",
    "jclass.03",
    "ECXA_ams.01",
    "ECXA_ams_user.01",
    "ams.01",
    "ams_user.01",
    "Bentley_JSpace_CustomAttributes.02",
    "Bentley_Plant.06"
    };

static const size_t s_numSchemasThatAllowOverridingArrays = 9;

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
    Utf8Char buf[1024];
    BeStringUtilities::Snprintf(buf, "%s.%02d", schema->GetName().c_str(), schema->GetVersionMajor());
    for (size_t i = 0; i < s_numSchemasThatAllowOverridingArrays; i++)
        if (0 == strcmp(s_schemasThatAllowOverridingArrays[i], buf))
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
            return ECObjectsStatus::DataTypeMismatch;
        }
    
    if (!newProperty._CanOverride(baseProperty))
        {
        LOG.errorv("The datatype of ECProperty %s.%s (%s) does not match the datatype of ECProperty %s.%s (%s)... which it overrides.", 
            newProperty.GetClass().GetFullName(), newProperty.GetName().c_str(), newProperty.GetTypeName().c_str(), 
            baseProperty.GetClass().GetFullName(), baseProperty.GetName().c_str(), baseProperty.GetTypeName().c_str());

        return ECObjectsStatus::DataTypeMismatch;
        }
    return ECObjectsStatus::Success; 
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Carole.MacDonald                03/2010
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus ECClass::RemoveProperty (Utf8StringCR name)
    {
    PropertyMap::iterator  propertyIterator = m_propertyMap.find (name.c_str());
    
    if ( propertyIterator == m_propertyMap.end() )
        return ECObjectsStatus::PropertyNotFound;
        
    ECPropertyP ecProperty = propertyIterator->second;
    return DeleteProperty (*ecProperty);
    }
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Carole.MacDonald                01/2010
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus ECClass::AddProperty (ECPropertyP ecProperty, Utf8StringCR name)
    {
    ECObjectsStatus status = ecProperty->SetName (name);
    if (ECObjectsStatus::Success != status)
        return status;

    return AddProperty (ecProperty);
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Carole.MacDonald                01/2010
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus ECClass::CreatePrimitiveProperty (PrimitiveECPropertyP &ecProperty, Utf8StringCR name)
    {
    ecProperty = new PrimitiveECProperty(*this);
    ECObjectsStatus status = AddProperty(ecProperty, name);
    if (status != ECObjectsStatus::Success)
        {
        delete ecProperty;
        ecProperty = NULL;
        return status;
        }
    return ECObjectsStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Carole.MacDonald                01/2010
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus ECClass::CreatePrimitiveProperty (PrimitiveECPropertyP &ecProperty, Utf8StringCR name, PrimitiveType primitiveType)
    {
    ecProperty = new PrimitiveECProperty(*this);
    ecProperty->SetType(primitiveType);
    ECObjectsStatus status = AddProperty(ecProperty, name);
    if (status != ECObjectsStatus::Success)
        {
        delete ecProperty;
        ecProperty = NULL;
        return status;
        }
    return ECObjectsStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Robert.Schili                   11/2015
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus ECClass::CreateEnumerationProperty(PrimitiveECPropertyP & ecProperty, Utf8StringCR name, ECEnumerationCR enumerationType)
    {
    ecProperty = new PrimitiveECProperty(*this);
    ecProperty->SetType(enumerationType);
    ECObjectsStatus status = AddProperty(ecProperty, name);
    if (status != ECObjectsStatus::Success)
        {
        delete ecProperty;
        ecProperty = NULL;
        return status;
        }
    return ECObjectsStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Carole.MacDonald                01/2010
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus ECClass::CreateStructProperty (StructECPropertyP &ecProperty, Utf8StringCR name)
    {
    ecProperty = new StructECProperty(*this);
    ECObjectsStatus status = AddProperty(ecProperty, name);
    if (status != ECObjectsStatus::Success)
        {
        delete ecProperty;
        ecProperty = NULL;
        return status;
        }
    return ECObjectsStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Carole.MacDonald                01/2010
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus ECClass::CreateStructProperty (StructECPropertyP &ecProperty, Utf8StringCR name, ECStructClassCR structType)
    {
    ecProperty = new StructECProperty(*this);
    ECObjectsStatus status = ecProperty->SetType(structType);
    if (ECObjectsStatus::Success == status)
        status = AddProperty(ecProperty, name);
    if (ECObjectsStatus::Success != status)
        {
        delete ecProperty;
        ecProperty = NULL;
        return status;
        }
    return ECObjectsStatus::Success;
    }
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Carole.MacDonald                01/2010
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus ECClass::CreateArrayProperty (ArrayECPropertyP &ecProperty, Utf8StringCR name)
    {
    ecProperty = new ArrayECProperty(*this);
    ECObjectsStatus status = AddProperty(ecProperty, name);
    if (status != ECObjectsStatus::Success)
        {
        delete ecProperty;
        ecProperty = NULL;
        return status;
        }
    return ECObjectsStatus::Success;
    }
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Carole.MacDonald                01/2010
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus ECClass::CreateArrayProperty (ArrayECPropertyP &ecProperty, Utf8StringCR name, PrimitiveType primitiveType)
    {
    ecProperty = new ArrayECProperty(*this);
    ecProperty->SetPrimitiveElementType (primitiveType);
    ECObjectsStatus status = AddProperty(ecProperty, name);
    if (status != ECObjectsStatus::Success)
        {
        delete ecProperty;
        ecProperty = NULL;
        return status;
        }
    return ECObjectsStatus::Success;
    }
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Carole.MacDonald                01/2010
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus ECClass::CreateStructArrayProperty (StructArrayECPropertyP &ecProperty, Utf8StringCR name, ECStructClassCP structType)
    {
    ecProperty = new StructArrayECProperty(*this);
    ECObjectsStatus status = ecProperty->SetStructElementType(structType);
    if (ECObjectsStatus::Success == status)
        status = AddProperty(ecProperty, name);
    if (ECObjectsStatus::Success != status)
        {
        delete ecProperty;
        ecProperty = NULL;
        return status;
        }
    return ECObjectsStatus::Success;
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
ECObjectsStatus ECClass::AddBaseClass(ECClassCR baseClass, bool insertAtBeginning, bool resolveConflicts)
    {
    return _AddBaseClass(baseClass, insertAtBeginning, resolveConflicts, true);
    }

//-------------------------------------------------------------------------------------
//* @bsimethod                                              
//+---------------+---------------+---------------+---------------+---------------+------
ECObjectsStatus ECClass::_AddBaseClass(ECClassCR baseClass, bool insertAtBeginning, bool resolveConflicts, bool validate)
    {
    if (&(baseClass.GetSchema()) != &(this->GetSchema()))
        {
        if (!ECSchema::IsSchemaReferenced(this->GetSchema(), baseClass.GetSchema()))
            return ECObjectsStatus::SchemaNotFound;
        }

    if (this == &baseClass || ClassesAreEqualByName(this, &baseClass) || baseClass.TraverseBaseClasses(&CheckBaseClassCycles, true, this))
        return ECObjectsStatus::BaseClassUnacceptable;

    if (ECClassModifier::Sealed == baseClass.m_modifier)
        {
        LOG.errorv("Cannot add class '%s' as a base class to '%s' because it is declared as Sealed", baseClass.GetFullName(), GetFullName());
        return ECObjectsStatus::BaseClassUnacceptable;
        }

    if (GetClassType() != baseClass.GetClassType())
        {
        if (!resolveConflicts)
            LOG.errorv("Cannot add class '%s' as a base class to '%s' because they are of differing class types", baseClass.GetFullName(), GetFullName());
        return ECObjectsStatus::BaseClassUnacceptable;
        }

    ECBaseClassesList::const_iterator baseClassIterator;
    for (baseClassIterator = m_baseClasses.begin(); baseClassIterator != m_baseClasses.end(); baseClassIterator++)
        {
        if (*baseClassIterator == (ECClassP) &baseClass)
            {
            LOG.errorv("Cannot add class '%s' as a base class to '%s' because it already exists as a base class", baseClass.GetFullName(), GetFullName());
            return ECObjectsStatus::NamedItemAlreadyExists;
            }
        }

    PropertyList baseClassProperties;
    ECObjectsStatus status = baseClass.GetProperties(true, &baseClassProperties);
    if (ECObjectsStatus::Success != status)
        return status;

    for (ECPropertyP prop : baseClassProperties)
        {
        ECPropertyP thisProperty;
        // This is a case-insensitive search
        if (NULL != (thisProperty = this->GetPropertyP(prop->GetName())))
            {
            status = ECClass::CanPropertyBeOverridden(*prop, *thisProperty);

            // If the property names do not have the same case, this is an error
            if (!prop->GetName().Equals(thisProperty->GetName()))
                {
                if (!resolveConflicts)
                    {
                    LOG.errorv("Failed to add base class due to case-collision between %s:%s and %s:%s.", prop->GetClass().GetFullName(), prop->GetName().c_str(), GetFullName(), thisProperty->GetName().c_str());
                    return ECObjectsStatus::CaseCollision;
                    }

                // Same type, different case, simply rename the second property to match the first
                if (ECObjectsStatus::Success == status)
                    {
                    LOG.debugv("Case-collision between %s:%s and %s:%s.  Renaming to %s", prop->GetClass().GetFullName(), prop->GetName().c_str(), GetFullName(), thisProperty->GetName().c_str(), prop->GetName().c_str());
                    ECClassP conflictClass = const_cast<ECClassP> (&thisProperty->GetClass());
                    conflictClass->RenameConflictProperty(thisProperty, true, prop->GetName());
                    }
                }

            if (ECObjectsStatus::Success != status)
                {
                if (ECObjectsStatus::DataTypeMismatch == status && resolveConflicts)
                    {
                    LOG.debugv("Case-collision between %s:%s and %s:%s", prop->GetClass().GetFullName(), prop->GetName().c_str(), GetFullName(), thisProperty->GetName().c_str());
                    ECClassP conflictClass = const_cast<ECClassP> (&thisProperty->GetClass());
                    conflictClass->RenameConflictProperty(thisProperty, true);
                    }
                else
                    {
                    LOG.errorv("Attempt to override a %s property of class %s with a different type property in derived class %s", thisProperty->GetName().c_str(), baseClass.GetName().c_str(), GetName().c_str());
                    return status;
                    }
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
        OnBaseClassPropertyAdded(*baseProperty, resolveConflicts);

    baseClass.AddDerivedClass(*this);

    return ECObjectsStatus::Success;
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    
+---------------+---------------+---------------+---------------+---------------+------*/
bool ECClass::HasBaseClasses () const
    {
    return (m_baseClasses.size() > 0);
    }

/*---------------------------------------------------------------------------------**//**
* @bsistruct                                                    Paul.Connelly   10/15
+---------------+---------------+---------------+---------------+---------------+------*/
struct DuplicateInheritanceDetector
{
    ECClassCR       m_baseClass;
    mutable bool    m_baseClassFound;

    DuplicateInheritanceDetector(ECClassCR baseClass) : m_baseClass(baseClass), m_baseClassFound(false) { }

    static bool HasDuplicateInheritance(ECClassCP thisClass, const void* arg)
        {
        DuplicateInheritanceDetector const& det = *reinterpret_cast<DuplicateInheritanceDetector const*>(arg);
        if (ECClass::ClassesAreEqualByName(thisClass, &det.m_baseClass))
            {
            if (det.m_baseClassFound)
                return true;
            det.m_baseClassFound = true;
            }

        return false;
        }
};

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   10/15
+---------------+---------------+---------------+---------------+---------------+------*/
bool ECClass::IsSingularlyDerivedFrom(ECClassCR baseClass) const
    {
    DuplicateInheritanceDetector det(baseClass);
    if (TraverseBaseClasses(&DuplicateInheritanceDetector::HasDuplicateInheritance, true, &det))
        return false;   // multiply-derived
    else
        return det.m_baseClassFound; // singularly-derived
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
        LOG.errorv("Class '%s' is not a base class of class '%s'", baseClass.GetName().c_str(), GetName().c_str());
        return ECObjectsStatus::ClassNotFound;
        }
        
    baseClass.RemoveDerivedClass(*this);

    InvalidateDefaultStandaloneEnabler();

    for (ECPropertyP baseProperty : baseClass.GetProperties(true))
        OnBaseClassPropertyRemoved (*baseProperty);

    return ECObjectsStatus::Success;
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
* @bsimethod                                                    Paul.Connelly   06/15
+---------------+---------------+---------------+---------------+---------------+------*/
static bool containsProperty (Utf8CP name, PropertyList const& props)
    {
    return props.end() != std::find_if (props.begin(), props.end(), [&name](ECPropertyP const& prop)
        {
        return prop->GetName().Equals (name);
        });
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus ECClass::GetProperties (bool includeBaseProperties, PropertyList* propertyList) const
    {
    for (ECPropertyP prop: m_propertyList)
        propertyList->push_back(prop);
        
    if (!includeBaseProperties || m_baseClasses.empty())
        return ECObjectsStatus::Success;
        
    // replicate managed code behavior - specific ordering expected. Probably slower, but at least correct.
    PropertyList inheritedProperties;
    for (auto const& baseClass : m_baseClasses)
        {
        for (ECPropertyP const& baseProp : baseClass->GetProperties (true))
            {
            if (!containsProperty (baseProp->GetName().c_str(), *propertyList) && !containsProperty (baseProp->GetName().c_str(), inheritedProperties))
                inheritedProperties.push_back (baseProp);
            }
        }

    // inherited properties come before this class's properties
    propertyList->reserve (propertyList->size() + inheritedProperties.size());
    propertyList->insert (propertyList->begin(), inheritedProperties.begin(), inheritedProperties.end());

    return ECObjectsStatus::Success;
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
    Utf8String value;      // used by the macros.
    if (GetName().length() == 0)
        {
        READ_REQUIRED_XML_ATTRIBUTE (classNode, TYPE_NAME_ATTRIBUTE,        this, Name,     classNode.GetName())    
        }
    
    // OPTIONAL attributes - If these attributes exist they MUST be valid    
    READ_OPTIONAL_XML_ATTRIBUTE (classNode, DESCRIPTION_ATTRIBUTE,         this, Description)
    READ_OPTIONAL_XML_ATTRIBUTE (classNode, DISPLAY_LABEL_ATTRIBUTE,       this, DisplayLabel)

    Utf8String     modifierString;
    BeXmlStatus modifierStatus = classNode.GetAttributeStringValue(modifierString, MODIFIER_ATTRIBUTE);
    if (BEXML_Success == modifierStatus)
        if (ECObjectsStatus::Success != ECXml::ParseModifierString(m_modifier, modifierString))
            {
            LOG.errorv("Class %s has an invalid modifier attribute value %s", this->GetName().c_str(), modifierString.c_str());
            return SchemaReadStatus::InvalidECSchemaXml;
            }

    return SchemaReadStatus::Success;
    }

void ECClass::_ReadCommentsInSameLine(BeXmlNodeR childNode, bvector<Utf8String>& comments)
    {
    BeXmlNodeP currentNode = &childNode;
    currentNode = currentNode->GetNextSibling(BEXMLNODE_Any);
    if (nullptr != currentNode && currentNode->type == BEXMLNODE_Comment)
        {
        Utf8String comment;
        currentNode->GetContent(comment);
        comments.push_back(comment);
        childNode = *currentNode->GetNextSibling(BEXMLNODE_Any);
        }

    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   
+---------------+---------------+---------------+---------------+---------------+------*/
SchemaReadStatus ECClass::_ReadXmlContents (BeXmlNodeR classNode, ECSchemaReadContextR context, ECSchemaCP conversionSchema, bvector<NavigationECPropertyP>& navigationProperties)
    {
    bvector<Utf8String> comments;

    bool isSchemaSupplemental = Utf8String::npos != GetSchema().GetName().find("_Supplemental_");
    // Get the BaseClass child nodes.
    for (BeXmlNodeP childNode = classNode.GetFirstChild (BEXMLNODE_Any); NULL != childNode; childNode = childNode->GetNextSibling (BEXMLNODE_Any))
        {
        if (context.GetPreserveXmlComments())
            {
            if (childNode->type == BEXMLNODE_Comment)
                {
                Utf8String comment;
                childNode->GetContent(comment);
                comments.push_back(comment);
                }
             }

        if (childNode->type != BEXMLNODE_Element)
            continue;

        Utf8CP childNodeName = childNode->GetName ();
        if (0 == strcmp (childNodeName, EC_PROPERTY_ELEMENT))
            {
            PrimitiveECPropertyP ecProperty = new PrimitiveECProperty(*this);
            SchemaReadStatus status = _ReadPropertyFromXmlAndAddToClass (ecProperty, childNode, context, conversionSchema, childNodeName);
            if (SchemaReadStatus::Success != status)
                return status;

            if (context.GetPreserveXmlComments())
                {
                _ReadCommentsInSameLine(*childNode, comments);
                Utf8String contentIdentifier = ecProperty->GetName();
                m_contentXmlComments[contentIdentifier] = comments;
                }
            }
        else if (!isSchemaSupplemental && (0 == strcmp (childNodeName, EC_BASE_CLASS_ELEMENT)))
            {
            SchemaReadStatus status = _ReadBaseClassFromXml(childNode, context, conversionSchema);
            if (SchemaReadStatus::Success != status)
                return status;

            if (context.GetPreserveXmlComments())
                {
                _ReadCommentsInSameLine(*childNode, comments);
                Utf8String contentIdentifier = EC_BASE_CLASS_ELEMENT;
                m_contentXmlComments[contentIdentifier] = comments;
                }
            }
        else if (0 == strcmp (childNodeName, EC_ARRAYPROPERTY_ELEMENT))
            {
            ECPropertyP ecProperty;
            if (2 == m_schema.GetOriginalECXmlVersionMajor())
                {
                Utf8String boolStr;
                bool isStruct = false;
                Utf8String typeName;
                // Ignore the isStruct attribute since it is irrelevant.  If there is a typeName and it is resolvable as a struct, then it is a struct array.  Otherwise, it isn't.  There are invalid
                // schemas out there that claim to be a StructArray but either use an unresolvable typeName or actually use a primitive type.
                if (BEXML_Success == childNode->GetAttributeStringValue(typeName, TYPE_NAME_ATTRIBUTE))
                    {
                    ECStructClassCP structClass;
                    ECObjectsStatus status = ResolveStructType(structClass, typeName, *this, false);
                    if (ECObjectsStatus::Success == status && NULL != structClass)
                        isStruct = true;  
                    }
                if (isStruct)
                    ecProperty = new StructArrayECProperty(*this);
                else
                    ecProperty = new ArrayECProperty(*this);
                }
            else 
                ecProperty = new ArrayECProperty(*this);
            SchemaReadStatus status = _ReadPropertyFromXmlAndAddToClass (ecProperty, childNode, context, conversionSchema, childNodeName);
            if (SchemaReadStatus::Success != status)
                return status;

            if (context.GetPreserveXmlComments())
                {
                _ReadCommentsInSameLine(*childNode, comments);

                Utf8String contentIdentifier = ecProperty->GetName();
                m_contentXmlComments[contentIdentifier] = comments;
                }
            }
        else if (0 == strcmp(childNodeName, EC_STRUCTARRAYPROPERTY_ELEMENT)) // technically, this only happens in EC3.0 and higher, but no harm in checking 2.0 schemas
            {
            ECPropertyP ecProperty = new StructArrayECProperty(*this);
            SchemaReadStatus status = _ReadPropertyFromXmlAndAddToClass(ecProperty, childNode, context, conversionSchema, childNodeName);
            if (SchemaReadStatus::Success != status)
                return status;

            if (context.GetPreserveXmlComments())
                {
                _ReadCommentsInSameLine(*childNode, comments);

                Utf8String contentIdentifier = ecProperty->GetName();
                m_contentXmlComments[contentIdentifier] = comments;
                }
            }
        else if (0 == strcmp (childNodeName, EC_STRUCTPROPERTY_ELEMENT))
            {
            ECPropertyP ecProperty = new StructECProperty (*this);
            SchemaReadStatus status = _ReadPropertyFromXmlAndAddToClass (ecProperty, childNode, context, conversionSchema, childNodeName);
            if (SchemaReadStatus::Success != status)
                return status;
            if (context.GetPreserveXmlComments())
                {
                _ReadCommentsInSameLine(*childNode, comments);

                Utf8String contentIdentifier = ecProperty->GetName();
                m_contentXmlComments[contentIdentifier] = comments;
                }
            }
        else if (0 == strcmp(childNodeName, EC_NAVIGATIONPROPERTY_ELEMENT)) // also EC3.0 only
            {
            NavigationECPropertyP ecProperty = new NavigationECProperty(*this);
            SchemaReadStatus status = _ReadPropertyFromXmlAndAddToClass(ecProperty, childNode, context, conversionSchema, childNodeName);
            if (SchemaReadStatus::Success != status)
                return status;
            navigationProperties.push_back(ecProperty);

            if (context.GetPreserveXmlComments())
                {
                _ReadCommentsInSameLine(*childNode, comments);

                Utf8String contentIdentifier = ecProperty->GetName();
                m_contentXmlComments[contentIdentifier] = comments;
                }
            }
        else if (0 == strcmp(childNodeName, EC_CUSTOM_ATTRIBUTES_ELEMENT))
            {
            if (context.GetPreserveXmlComments())
                {
                Utf8String contentIdentifier = EC_CUSTOM_ATTRIBUTES_ELEMENT;
                m_contentXmlComments[contentIdentifier] = comments;
                }
            }

        comments.clear();
        }
    
    // Add Custom Attributes
    if (CustomAttributeReadStatus::InvalidCustomAttributes == ReadCustomAttributes(classNode, context, GetSchema()))
        {
        LOG.errorv("Failed to read class %s because one or more invalid custom attributes were applied to it.", this->GetName().c_str());
        return SchemaReadStatus::InvalidECSchemaXml;
        }

    return SchemaReadStatus::Success;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Colin.Kerr            09/2012
//---------------+---------------+---------------+---------------+---------------+-------
SchemaReadStatus ECClass::_ReadBaseClassFromXml (BeXmlNodeP childNode, ECSchemaReadContextR context, ECSchemaCP conversionSchema)
    {
    Utf8String qualifiedClassName;
    childNode->GetContent (qualifiedClassName);

    // Parse the potentially qualified class name into an alias and short class name
    Utf8String alias;
    Utf8String className;
    if (ECObjectsStatus::Success != ECClass::ParseClassName (alias, className, qualifiedClassName))
        {
        LOG.errorv ("Invalid ECSchemaXML: The ECClass '%s' contains a %s element with the value '%s' that can not be parsed.",  
            GetName().c_str(), EC_BASE_CLASS_ELEMENT, qualifiedClassName.c_str());

        return SchemaReadStatus::InvalidECSchemaXml;
        }

    ECSchemaCP resolvedSchema = GetSchema().GetSchemaByAliasP (alias);
    if (NULL == resolvedSchema)
        {
        LOG.errorv("Invalid ECSchemaXML: The ECClass '%s' contains a %s element with the alias '%s' that can not be resolved to a referenced schema.",
            GetName().c_str(), EC_BASE_CLASS_ELEMENT, alias.c_str());
        return SchemaReadStatus::InvalidECSchemaXml;
        }

    context.ResolveClassName (className, *resolvedSchema);
    ECClassCP baseClass = resolvedSchema->GetClassCP (className.c_str());
    if (NULL == baseClass)
        {
        LOG.errorv("Invalid ECSchemaXML: The ECClass '%s' contains a %s element with the value '%s' that can not be resolved to an ECClass named '%s' in the ECSchema '%s'",
            GetName ().c_str (), EC_BASE_CLASS_ELEMENT, qualifiedClassName.c_str (), className.c_str (), resolvedSchema->GetName ().c_str ());
        return SchemaReadStatus::InvalidECSchemaXml;
        }

    bool resolveConflicts = false;
    if (nullptr != conversionSchema)
        {
        ECClassCP conversionClass = conversionSchema->GetClassCP(GetName().c_str());
        if (nullptr != conversionClass && conversionClass->GetCustomAttribute("IgnoreBaseClass").IsValid())
            return SchemaReadStatus::Success;
        if (conversionSchema->IsDefined("ResolvePropertyNameConflicts"))
            resolveConflicts = true;

        }

    ECObjectsStatus stat;
    if (ECObjectsStatus::Success != (stat = _AddBaseClass(*baseClass, false, resolveConflicts, false)))
        {
        if (stat == ECObjectsStatus::BaseClassUnacceptable)
            {
            LOG.errorv("Invalid ECSchemaXML: The ECClass '%s:%s' (%d) has a base class '%s:%s' (%d) but their types differ.",
                       GetSchema().GetFullSchemaName().c_str(), GetName().c_str(), GetClassType(),
                       baseClass->GetSchema().GetFullSchemaName().c_str(), baseClass->GetName().c_str(), baseClass->GetClassType());
            return SchemaReadStatus::InvalidECSchemaXml;
            }
        LOG.errorv("Invalid ECSchemaXML: Unable to add ECClass '%s:%s' as a base class to ECClass '%s:%s'",
                   baseClass->GetSchema().GetFullSchemaName().c_str(), baseClass->GetName().c_str(),
                   GetSchema().GetFullSchemaName().c_str(), GetName().c_str());
        return SchemaReadStatus::InvalidECSchemaXml;
        }
    return SchemaReadStatus::Success;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Colin.Kerr            09/2012
//---------------+---------------+---------------+---------------+---------------+-------
SchemaReadStatus ECClass::_ReadPropertyFromXmlAndAddToClass( ECPropertyP ecProperty, BeXmlNodeP& childNode, ECSchemaReadContextR context, ECSchemaCP conversionSchema, Utf8CP childNodeName)
    {
    // read the property data.
    SchemaReadStatus status = ecProperty->_ReadXml (*childNode, context);
    if (status != SchemaReadStatus::Success)
        {
        LOG.errorv ("Invalid ECSchemaXML: Failed to read properties of ECClass '%s:%s'", this->GetSchema().GetName().c_str(), this->GetName().c_str());
        delete ecProperty;
        return status;
        }

    bool resolveConflicts = false;
    if (nullptr != conversionSchema)
        {
        resolveConflicts = conversionSchema->IsDefined("ResolvePropertyNameConflicts");
        ECClassCP conversionClass = conversionSchema->GetClassCP(GetName().c_str());
        if (nullptr != conversionClass)
            {
            ECPropertyP conversionProp = conversionClass->GetPropertyP(ecProperty->GetName().c_str());
            if (nullptr != conversionProp)
                {
                IECInstancePtr renamePtr = conversionProp->GetCustomAttribute("OverwriteTypeName");
                if (renamePtr.IsValid())
                    {
                    ECValue ecValue;
                    renamePtr->GetValue(ecValue, "TypeName");
                    if (!ecValue.IsNull())
                        ecProperty->SetName(ecValue.GetUtf8CP());
                    }
                }
            }
        }

    if (ECObjectsStatus::Success != this->AddProperty (ecProperty, resolveConflicts))
        {
        delete ecProperty;
        return SchemaReadStatus::InvalidECSchemaXml;
        }
    
    return SchemaReadStatus::Success;
    }



/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Carole.MacDonald                01/2010
+---------------+---------------+---------------+---------------+---------------+------*/
SchemaWriteStatus ECClass::_WriteXml (BeXmlWriterR xmlWriter, int ecXmlVersionMajor, int ecXmlVersionMinor, Utf8CP elementName, bmap<Utf8CP, Utf8CP>* additionalAttributes, bool doElementEnd) const
    {
    SchemaWriteStatus status = SchemaWriteStatus::Success;
    // No need to check here if comments need to be preserved. If they're not preserved m_xmlComments will be empty
    for (auto comment : m_xmlComments)
        {
        xmlWriter.WriteComment(comment.c_str());
        }

    xmlWriter.WriteElementStart(elementName);
    
    xmlWriter.WriteAttribute(TYPE_NAME_ATTRIBUTE, this->GetName().c_str());
    xmlWriter.WriteAttribute(DESCRIPTION_ATTRIBUTE, this->GetInvariantDescription().c_str());
    if (GetIsDisplayLabelDefined())
        xmlWriter.WriteAttribute(DISPLAY_LABEL_ATTRIBUTE, this->GetInvariantDisplayLabel().c_str());

    if (2 == ecXmlVersionMajor)
        {
        xmlWriter.WriteAttribute(IS_STRUCT_ATTRIBUTE, IsStructClass());
        xmlWriter.WriteAttribute(IS_CUSTOMATTRIBUTE_ATTRIBUTE, IsCustomAttributeClass());
        bool isConcrete = this->GetClassModifier() != ECClassModifier::Abstract;
        xmlWriter.WriteAttribute(IS_DOMAINCLASS_ATTRIBUTE, isConcrete && !(IsStructClass() || IsCustomAttributeClass()));
        }
    else if (m_modifier != ECClassModifier::None)
        {
        xmlWriter.WriteAttribute(MODIFIER_ATTRIBUTE, ECXml::ModifierToString(m_modifier));
        }

    if (nullptr != additionalAttributes)
        {
        for (bmap<Utf8CP, Utf8CP>::iterator iter = additionalAttributes->begin(); iter != additionalAttributes->end(); ++iter)
            xmlWriter.WriteAttribute(iter->first, iter->second);
        }
    
    for (const ECClassP& baseClass: m_baseClasses)
        {
        auto comments = m_contentXmlComments.find(EC_BASE_CLASS_ELEMENT);
        if (comments != m_contentXmlComments.end())
            {
            for (auto comment : comments->second)
                {
                xmlWriter.WriteComment(comment.c_str());
                }
            }


        xmlWriter.WriteElementStart(EC_BASE_CLASS_ELEMENT);
        xmlWriter.WriteText((ECClass::GetQualifiedClassName(GetSchema(), *baseClass)).c_str());
        xmlWriter.WriteElementEnd();
        }

    auto comments = m_contentXmlComments.find(EC_CUSTOM_ATTRIBUTES_ELEMENT);
    if (comments != m_contentXmlComments.end())
        {
        for (auto comment : comments->second)
            {
            xmlWriter.WriteComment(comment.c_str());
            }
        }
    WriteCustomAttributes (xmlWriter);
            
    for (ECPropertyP prop: GetProperties(false))
        { 
        auto comments = m_contentXmlComments.find(prop->GetName());

        if (comments != m_contentXmlComments.end())
            {
            for (auto comment : comments->second)
                {
                xmlWriter.WriteComment(comment.c_str());
                }
            }


        prop->_WriteXml (xmlWriter, ecXmlVersionMajor, ecXmlVersionMinor);
        }
    if (doElementEnd)
        xmlWriter.WriteElementEnd();
    return status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   
+---------------+---------------+---------------+---------------+---------------+------*/
SchemaWriteStatus ECClass::_WriteXml (BeXmlWriterR xmlWriter, int ecXmlVersionMajor, int ecXmlVersionMinor) const
    {
    return _WriteXml (xmlWriter, ecXmlVersionMajor, ecXmlVersionMinor, EC_CLASS_ELEMENT, nullptr, true);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus ECClass::ParseClassName 
(
Utf8StringR  alias, 
Utf8StringR  className, 
Utf8StringCR qualifiedClassName
)
    {
    if (0 == qualifiedClassName.length())
        {
        LOG.error("Failed to parse an alias and class name from a qualified class name because the string is empty.");
        return ECObjectsStatus::ParseError;
        }
        
    Utf8String::size_type colonIndex = qualifiedClassName.find (':');
    if (Utf8String::npos == colonIndex)
        {
        alias.clear();
        className = qualifiedClassName;
        return ECObjectsStatus::Success;
        }

    if (qualifiedClassName.length() == colonIndex + 1)
        {
        LOG.errorv("Failed to parse an alias and class name from the qualified class name '%s' because the string ends with a colon. There must be characters after the colon.",
            qualifiedClassName.c_str());
        return ECObjectsStatus::ParseError;
        }

    if (0 == colonIndex)
        alias.clear();
    else
        alias = qualifiedClassName.substr (0, colonIndex);

    className = qualifiedClassName.substr (colonIndex + 1);

    return ECObjectsStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String ECClass::GetQualifiedClassName
(
ECSchemaCR primarySchema,
ECClassCR  ecClass
)
    {
    Utf8String alias;
    if (!EXPECTED_CONDITION (ECObjectsStatus::Success == primarySchema.ResolveAlias (ecClass.GetSchema(), alias)))
        {
        LOG.warningv ("warning: Cannot qualify an ECClass name with an alias unless the schema containing the ECClass is referenced by the primary schema."
            "The class name will remain unqualified.\n  Primary ECSchema: %s\n  ECClass: %s\n ECSchema containing ECClass: %s", primarySchema.GetName().c_str(), ecClass.GetName().c_str(), ecClass.GetSchema().GetName().c_str());
        return ecClass.GetName();
        }
    if (alias.empty())
        return ecClass.GetName();
    else
        return alias + ":" + ecClass.GetName();
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
    IECInstancePtr caInstance = this->GetCustomAttribute("InstanceLabelSpecification");
    if (caInstance.IsValid())
        {
        ECValue value;
        if (ECObjectsStatus::Success == caInstance->GetValue (value, "PropertyName") && !value.IsNull())
            {
            Utf8CP propertyName = value.GetUtf8CP();
            instanceLabelProperty = this->GetPropertyP (propertyName);
            if (NULL != instanceLabelProperty)
                return instanceLabelProperty;
            }
        }

    Utf8String instanceLabelPropertyNames[6] = 
        {"DisplayLabel", "DISPLAYLABEL", "displaylabel", "Name", "NAME", "name"};
    FOR_EACH (Utf8StringCR propName, instanceLabelPropertyNames)
        {
        instanceLabelProperty = this->GetPropertyP (propName.c_str());
        if (NULL != instanceLabelProperty)
            return instanceLabelProperty;
        }

    return NULL;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            10/2015
//---------------+---------------+---------------+---------------+---------------+-------
ECEntityClass::ECEntityClass(ECSchemaCR schema) : ECClass(schema)
    {
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            11/2015
//---------------+---------------+---------------+---------------+---------------+-------
SchemaWriteStatus ECEntityClass::_WriteXml(BeXmlWriterR xmlWriter, int ecXmlVersionMajor, int ecXmlVersionMinor) const
    {
    if (2 == ecXmlVersionMajor)
        return T_Super::_WriteXml(xmlWriter, ecXmlVersionMajor, ecXmlVersionMinor);

    else
        return T_Super::_WriteXml(xmlWriter, ecXmlVersionMajor, ecXmlVersionMinor, EC_ENTITYCLASS_ELEMENT, nullptr, true);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Colin.Kerr                  12/2015
//---------------+---------------+---------------+---------------+---------------+-------
ECObjectsStatus ECEntityClass::CreateNavigationProperty(NavigationECPropertyP& ecProperty, Utf8StringCR name, ECRelationshipClassCR relationshipClass, ECRelatedInstanceDirection direction, PrimitiveType type, bool verify)
    {
    ecProperty = new NavigationECProperty(*this);
    ecProperty->SetType(type);
    ECObjectsStatus status = ecProperty->SetRelationshipClass(relationshipClass, direction, verify);
    if (ECObjectsStatus::Success == status)
        status = AddProperty(ecProperty, name);

    if (ECObjectsStatus::Success != status)
        {
        delete ecProperty;
        ecProperty = nullptr;
        }
    return status;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            10/2015
//---------------+---------------+---------------+---------------+---------------+-------
ECCustomAttributeClass::ECCustomAttributeClass(ECSchemaCR schema) : ECClass(schema)
    {
    m_containerType = CustomAttributeContainerType::Any;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            11/2015
//---------------+---------------+---------------+---------------+---------------+-------
SchemaWriteStatus ECCustomAttributeClass::_WriteXml(BeXmlWriterR xmlWriter, int ecXmlVersionMajor, int ecXmlVersionMinor) const
    {
    if (2 == ecXmlVersionMajor)
        return T_Super::_WriteXml(xmlWriter, ecXmlVersionMajor, ecXmlVersionMinor);

    else
        {
        Utf8String appliesToAttributeValue = ECXml::ContainerTypeToString(m_containerType);
        bmap<Utf8CP, Utf8CP> additionalAttributes;
        additionalAttributes[CUSTOM_ATTRIBUTE_APPLIES_TO] = appliesToAttributeValue.c_str();
        return T_Super::_WriteXml(xmlWriter, ecXmlVersionMajor, ecXmlVersionMinor, EC_CUSTOMATTRIBUTECLASS_ELEMENT, &additionalAttributes, true);
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            11/2015
//---------------+---------------+---------------+---------------+---------------+-------
SchemaReadStatus ECCustomAttributeClass::_ReadXmlAttributes(BeXmlNodeR classNode)
    {
    SchemaReadStatus status;
    if (SchemaReadStatus::Success != (status = T_Super::_ReadXmlAttributes(classNode)))
        return status;

    Utf8String appliesTo;
    if (BEXML_Success == classNode.GetAttributeStringValue(appliesTo, CUSTOM_ATTRIBUTE_APPLIES_TO))
        {
        if (ECObjectsStatus::Success != ECXml::ParseContainerString(this->m_containerType, appliesTo))
            {
            LOG.errorv("appliesTo attribute value '%s' invalid on ECCustomAttributeClass %s:%s.  ", appliesTo.c_str(), this->GetSchema().GetName().c_str(), this->GetName().c_str());
            return SchemaReadStatus::InvalidECSchemaXml;
            }
        }
    else
        m_containerType = CustomAttributeContainerType::Any;

    return status;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            10/2015
//---------------+---------------+---------------+---------------+---------------+-------
ECStructClass::ECStructClass(ECSchemaCR schema) : ECClass(schema)
    {
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            11/2015
//---------------+---------------+---------------+---------------+---------------+-------
SchemaWriteStatus ECStructClass::_WriteXml(BeXmlWriterR xmlWriter, int ecXmlVersionMajor, int ecXmlVersionMinor) const
    {
    if (2 == ecXmlVersionMajor)
        return T_Super::_WriteXml(xmlWriter, ecXmlVersionMajor, ecXmlVersionMinor);

    else
        return T_Super::_WriteXml(xmlWriter, ecXmlVersionMajor, ecXmlVersionMinor, EC_STRUCTCLASS_ELEMENT, nullptr, true);
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
ECPropertyCP ECPropertyIterable::FindByDisplayLabel (Utf8CP label) const
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
    
static RelationshipMultiplicity s_zeroOneMultiplicity(0, 1);
static RelationshipMultiplicity s_zeroManyMultiplicity(0, UINT_MAX);
static RelationshipMultiplicity s_oneOneMultiplicity(1, 1);
static RelationshipMultiplicity s_oneManyMultiplicity(1, UINT_MAX);

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Carole.MacDonald                02/2010
+---------------+---------------+---------------+---------------+---------------+------*/
RelationshipMultiplicity::RelationshipMultiplicity
(
)
    {
    m_lowerLimit = 0;
    m_upperLimit = 1;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Carole.MacDonald                02/2010
+---------------+---------------+---------------+---------------+---------------+------*/
RelationshipMultiplicity::RelationshipMultiplicity
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
* @bsimethod                                    
* If lhs is equal to rhs,     return 0
* If lhs is greater than rhs, return -1
* If lhs is less than rhs,    return 1
+---------------+---------------+---------------+---------------+---------------+------*/
int RelationshipMultiplicity::Compare
(
RelationshipMultiplicity const& lhs,
RelationshipMultiplicity const& rhs
)
    {
    if (lhs.GetLowerLimit() == rhs.GetLowerLimit() && 
        lhs.GetUpperLimit() == rhs.GetUpperLimit())
        return 0;

    return (lhs.GetLowerLimit() > rhs.GetLowerLimit() || rhs.GetUpperLimit() > lhs.GetUpperLimit()) ? 1 : -1;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Carole.MacDonald                02/2010
+---------------+---------------+---------------+---------------+---------------+------*/
uint32_t RelationshipMultiplicity::GetLowerLimit
(
) const
    {
    return m_lowerLimit;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Carole.MacDonald                02/2010
+---------------+---------------+---------------+---------------+---------------+------*/
uint32_t RelationshipMultiplicity::GetUpperLimit
(
) const
    {
    return m_upperLimit;
    }
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Carole.MacDonald                02/2010
+---------------+---------------+---------------+---------------+---------------+------*/
bool RelationshipMultiplicity::IsUpperLimitUnbounded
(
) const
    {
    return m_upperLimit == UINT_MAX;
    }
  
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Carole.MacDonald                03/2010
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String RelationshipMultiplicity::ToString
(
) const
    {
    Utf8Char multiplicityString[32];
    
    if (UINT_MAX == m_upperLimit)
        BeStringUtilities::Snprintf(multiplicityString, "(%d..*)", m_lowerLimit);
    else
        BeStringUtilities::Snprintf(multiplicityString, "(%d..%d)", m_lowerLimit, m_upperLimit);
        
    return multiplicityString;
    }
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Carole.MacDonald                03/2010
+---------------+---------------+---------------+---------------+---------------+------*/
RelationshipMultiplicityCR RelationshipMultiplicity::ZeroOne
(
)
    {
    return s_zeroOneMultiplicity;
    }
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Carole.MacDonald                03/2010
+---------------+---------------+---------------+---------------+---------------+------*/
RelationshipMultiplicityCR RelationshipMultiplicity::ZeroMany
(
)
    {
    return s_zeroManyMultiplicity;
    }
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Carole.MacDonald                03/2010
+---------------+---------------+---------------+---------------+---------------+------*/
RelationshipMultiplicityCR RelationshipMultiplicity::OneOne
(
)
    {
    return s_oneOneMultiplicity;
    }
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Carole.MacDonald                03/2010
+---------------+---------------+---------------+---------------+---------------+------*/
RelationshipMultiplicityCR RelationshipMultiplicity::OneMany
(
)
    {
    return s_oneManyMultiplicity;
    }
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Carole.MacDonald                02/2010
+---------------+---------------+---------------+---------------+---------------+------*/
ECRelationshipConstraint::ECRelationshipConstraint
(
ECRelationshipClassP relationshipClass, 
bool isSource
) :m_constraintClasses(relationshipClass), m_isSource(isSource)
    {
    m_relClass = relationshipClass;
    m_multiplicity = &s_zeroOneMultiplicity;
    m_isPolymorphic = true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Affan.Khan                       06/2016
+---------------+---------------+---------------+---------------+---------------+------*/
ECRelationshipClassCR  ECRelationshipConstraint::GetRelationshipClass() const
    {
    BeAssert(m_relClass != nullptr);
    return *m_relClass;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Carole.MacDonald                03/2010
+---------------+---------------+---------------+---------------+---------------+------*/
ECRelationshipConstraint::~ECRelationshipConstraint
(
)
    {
     if ((m_multiplicity != &s_zeroOneMultiplicity) && (m_multiplicity != &s_zeroManyMultiplicity) &&
        (m_multiplicity != &s_oneOneMultiplicity) && (m_multiplicity != &s_oneManyMultiplicity))
        delete m_multiplicity;
    } 
   
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Carole.MacDonald                06/2010
+---------------+---------------+---------------+---------------+---------------+------*/
ECSchemaCP ECRelationshipConstraint::_GetContainerSchema() const
    {
    return &(GetRelationshipClass().GetSchema());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Caleb.Shafer                08/2016
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus ECRelationshipConstraint::ValidateClassConstraint() const
    {
    for (const auto &constraint : m_constraintClasses)
        {
        ECObjectsStatus status = _ValidateClassConstraint(constraint->GetClass());
        if (ECObjectsStatus::Success != status)
            return status;
        }

    return ECObjectsStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus ECRelationshipConstraint::_ValidateClassConstraint
(
ECEntityClassCR constraintClass
) const
    {
    ECRelationshipClassCP relationshipClass = m_relClass;
    if (!m_relClass->HasBaseClasses())
        return ECObjectsStatus::Success;

    // Check if this is the source or target constraint. Then Iterate over the base classes and check 
    // if the constraintClass is equal to or larger in scope than the possibly defined scope on the
    // baseclasses. 
    bool isSourceConstraint = &relationshipClass->GetSource() == this;
    for (auto baseClass : relationshipClass->GetBaseClasses())
        {
        // Get the relationship base class
        ECRelationshipClassCP relationshipBaseClass = baseClass->GetRelationshipClassCP();
        ECRelationshipConstraintCP baseClassConstraint = (isSourceConstraint) ? &relationshipBaseClass->GetSource() 
                                                                              : &relationshipBaseClass->GetTarget();

        // Validate against the base class again...
        ECObjectsStatus validationStatus = baseClassConstraint->_ValidateClassConstraint(constraintClass);
        if (validationStatus != ECObjectsStatus::Success)
            {
            return validationStatus;
            }

        // Iterate over the constraint classes and check if they meet the scopeing requirements.
        for (auto ecClassIterator : baseClassConstraint->GetConstraintClasses())
            {
            auto baseConstraintClass = &ecClassIterator->GetClass();
            if (!constraintClass.Is(baseConstraintClass))
                {
                LOG.errorv("Class Constraint Violation: The class '%s' on %s-Constraint of '%s' is not nor derived from Class '%s' as speficied in Class '%s'",
                            constraintClass.GetName().c_str(), (isSourceConstraint) ? "Source" : "Target", m_relClass->GetName().c_str(), baseConstraintClass->GetName().c_str(), relationshipBaseClass->GetName().c_str());
                return ECObjectsStatus::RelationshipConstraintsNotCompatible;
                }
            }
        }

    return ECObjectsStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus ECRelationshipConstraint::ValidateMultiplicityConstraint() const
    {
    uint32_t lowerLimit = GetMultiplicity().GetLowerLimit();
    uint32_t upperLimit = GetMultiplicity().GetUpperLimit();
    return _ValidateMultiplicityConstraint(lowerLimit, upperLimit);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus ECRelationshipConstraint::_ValidateMultiplicityConstraint(uint32_t& lowerLimit, uint32_t& upperLimit) const
    {
    ECRelationshipClassCP relationshipClass = m_relClass;
    if (!m_relClass->HasBaseClasses())
        return ECObjectsStatus::Success;

    bool isSourceConstraint = &relationshipClass->GetSource() == this;
    for (auto baseClass : relationshipClass->GetBaseClasses())
        {
        // Get the relationship base class
        ECRelationshipClassCP relationshipBaseClass = baseClass->GetRelationshipClassCP();
        ECRelationshipConstraintP baseClassConstraint = (isSourceConstraint) ? &relationshipBaseClass->GetSource()
                                                                              : &relationshipBaseClass->GetTarget();

        // Checks the multiplicity: 0 = equal multiplicity, -1 = leftside is bigger, 1 = rightside is bigger
        // since the left side is the current multiplicity and the right side the base class, it is expected the baseclass is bigger.
        if (RelationshipMultiplicity::Compare(RelationshipMultiplicity(lowerLimit, upperLimit), baseClassConstraint->GetMultiplicity()) == -1)
            {
            LOG.errorv("Multiplicity Violation: The Multiplicity (%u..%u) of %s is larger than the Multiplicity of it's base class %s (%u..%u)",
                        lowerLimit, upperLimit, relationshipClass->GetName().c_str(),
                        relationshipBaseClass->GetName().c_str(), baseClassConstraint->GetMultiplicity().GetLowerLimit(), baseClassConstraint->GetMultiplicity().GetUpperLimit());

            if (m_relClass->GetSchema().GetOriginalECXmlVersionMajor() != 2)
                return ECObjectsStatus::RelationshipConstraintsNotCompatible;

            // For legacy 2.0 schemas we change the base class constraint multiplicity to bigger derived class constraint in order for it to pass the validation rules.
            LOG.warningv("The Multiplicity of %s's base class, %s, has been changed from (%u..%u) to (%u..%u) to conform to new relationship constraint rules.",
                        relationshipClass->GetName().c_str(), relationshipBaseClass->GetName().c_str(), 
                        baseClassConstraint->GetMultiplicity().GetLowerLimit(), baseClassConstraint->GetMultiplicity().GetUpperLimit(), lowerLimit, upperLimit);
            baseClassConstraint->SetMultiplicity(lowerLimit, upperLimit);
            }
        }

    return ECObjectsStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Carole.MacDonald                03/2010
+---------------+---------------+---------------+---------------+---------------+------*/
SchemaReadStatus ECRelationshipConstraint::ReadXml (BeXmlNodeR constraintNode, ECSchemaReadContextR schemaContext)
    {
    SchemaReadStatus status = SchemaReadStatus::Success;
    
    Utf8String value;  // needed for macros.
    ECObjectsStatus setterStatus;
    READ_OPTIONAL_XML_ATTRIBUTE_IGNORING_SET_ERRORS (constraintNode, POLYMORPHIC_ATTRIBUTE, this, IsPolymorphic);
    READ_OPTIONAL_XML_ATTRIBUTE (constraintNode, ROLELABEL_ATTRIBUTE, this, RoleLabel);

    if (m_relClass->GetSchema().GetOriginalECXmlVersionMajor() >= 3 && m_relClass->GetSchema().GetOriginalECXmlVersionMinor() >= 1)
        {
        Utf8String multiplicity;
        if (BEXML_Success == constraintNode.GetAttributeStringValue(multiplicity, MULTIPLICITY_ATTRIBUTE))
            _SetMultiplicity(multiplicity.c_str());
        }
    else
        {
        READ_OPTIONAL_XML_ATTRIBUTE (constraintNode, CARDINALITY_ATTRIBUTE, this, Cardinality);
        }
    
    // Add Custom Attributes
    if (CustomAttributeReadStatus::InvalidCustomAttributes == ReadCustomAttributes(constraintNode, schemaContext, m_relClass->GetSchema()))
        {
        LOG.error("Failed to read relationship constraint because one or more invalid custom attributes were applied to it.");
        return SchemaReadStatus::InvalidECSchemaXml;
        }

    // For supplemental schemas, only read in the attributes and custom attributes
    if (Utf8String::npos != _GetContainerSchema()->GetName().find("_Supplemental"))
        return SchemaReadStatus::Success;

    for (BeXmlNodeP constraintClassNode = constraintNode.GetFirstChild(); nullptr != constraintClassNode; constraintClassNode = constraintClassNode->GetNextSibling())
        {
        if (0 != strcmp(constraintClassNode->GetName(), EC_CONSTRAINTCLASS_ELEMENT))
            continue;
        
        Utf8String     constraintClassName;
        if (BEXML_Success != constraintClassNode->GetAttributeStringValue(constraintClassName, CONSTRAINTCLASSNAME_ATTRIBUTE))
            return SchemaReadStatus::InvalidECSchemaXml;
        
        // Parse the potentially qualified class name into an alias and short class name
        Utf8String alias;
        Utf8String className;
        if (ECObjectsStatus::Success != ECClass::ParseClassName (alias, className, constraintClassName))
            {
            LOG.errorv("Invalid ECSchemaXML: The ECRelationshipConstraint contains a %s attribute with the value '%s' that can not be parsed.",
                CONSTRAINTCLASSNAME_ATTRIBUTE, constraintClassName.c_str());
            return SchemaReadStatus::InvalidECSchemaXml;
            }
        
        ECSchemaCP resolvedSchema = m_relClass->GetSchema().GetSchemaByAliasP (alias);
        if (NULL == resolvedSchema)
            {
            LOG.errorv("Invalid ECSchemaXML: ECRelationshipConstraint contains a %s attribute with the alias '%s' that can not be resolved to a referenced schema.",
                CONSTRAINTCLASSNAME_ATTRIBUTE, alias.c_str());
            return SchemaReadStatus::InvalidECSchemaXml;
            }

        ECClassCP constraintClass = resolvedSchema->GetClassCP (className.c_str());
        if (NULL == constraintClass)
            {
            LOG.errorv("Invalid ECSchemaXML: The ECRelationshipConstraint contains a %s attribute with the value '%s' that can not be resolved to an ECClass named '%s' in the ECSchema '%s'",
                CONSTRAINTCLASSNAME_ATTRIBUTE, constraintClassName.c_str(), className.c_str(), resolvedSchema->GetName().c_str());
            return SchemaReadStatus::InvalidECSchemaXml;
            }
        ECEntityClassCP constraintAsEntity = constraintClass->GetEntityClassCP();
        if (nullptr == constraintAsEntity)
            {
            LOG.errorv("Invalid ECSchemaXML: The ECRelationshipConstraint contains a %s attribute with the value '%s' that does not resolve to an ECEntityClass named '%s' in the ECSchema '%s'",
                         CONSTRAINTCLASSNAME_ATTRIBUTE, constraintClassName.c_str(), className.c_str(), resolvedSchema->GetName().c_str());
            return SchemaReadStatus::InvalidECSchemaXml;
            }

        ECRelationshipConstraintClassP ecRelationshipconstaintClass;
        m_constraintClasses.Add(ecRelationshipconstaintClass, *constraintAsEntity);
        if (ecRelationshipconstaintClass != nullptr)
            {
            for (BeXmlNodeP keyNode = constraintClassNode->GetFirstChild(); nullptr != keyNode; keyNode = keyNode->GetNextSibling())
                {
                for (BeXmlNodeP propertyNode = keyNode->GetFirstChild(); nullptr != propertyNode; propertyNode = propertyNode->GetNextSibling())
                    {
                    Utf8String propertyName;
                    if (BEXML_Success != propertyNode->GetAttributeStringValue(propertyName, KEYPROPERTYNAME_ATTRIBUTE))
                        return SchemaReadStatus::InvalidECSchemaXml;
                    ecRelationshipconstaintClass->AddKey(propertyName.c_str());
                    }
                }
            }
        }

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
SchemaWriteStatus ECRelationshipConstraint::WriteXml (BeXmlWriterR xmlWriter, Utf8CP elementName, int ecXmlVersionMajor, int ecXmlVersionMinor) const
    {
    SchemaWriteStatus status = SchemaWriteStatus::Success;
    
    xmlWriter.WriteElementStart(elementName);
    
    if (ecXmlVersionMajor >= 3 && ecXmlVersionMinor >= 1)
        xmlWriter.WriteAttribute(MULTIPLICITY_ATTRIBUTE, m_multiplicity->ToString().c_str());
    else
        xmlWriter.WriteAttribute(CARDINALITY_ATTRIBUTE, ECXml::MultiplicityToLegacyString(*m_multiplicity).c_str());
    
    if (IsRoleLabelDefined())
        xmlWriter.WriteAttribute(ROLELABEL_ATTRIBUTE, m_roleLabel.c_str());

    xmlWriter.WriteAttribute(POLYMORPHIC_ATTRIBUTE, this->GetIsPolymorphic());
        
    WriteCustomAttributes (xmlWriter);

    for (const auto &constraint : m_constraintClasses)
        {
        xmlWriter.WriteElementStart(EC_CONSTRAINTCLASS_ELEMENT);
        xmlWriter.WriteAttribute(CONSTRAINTCLASSNAME_ATTRIBUTE, ECClass::GetQualifiedClassName(m_relClass->GetSchema(), constraint->GetClass()).c_str());
        for (auto key : constraint->GetKeys())
            {
            xmlWriter.WriteElementStart(EC_CONSTRAINTKEY_ELEMENT);
            xmlWriter.WriteElementStart(EC_KEYPROPERTY_ELEMENT);
            xmlWriter.WriteAttribute(KEYPROPERTYNAME_ATTRIBUTE, key.c_str());
            xmlWriter.WriteElementEnd();
            xmlWriter.WriteElementEnd();
            }
        xmlWriter.WriteElementEnd();
        }
    xmlWriter.WriteElementEnd();
    return status;
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Carole.MacDonald                03/2010
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus ECRelationshipConstraint::AddClass(ECEntityClassCR classConstraint)
    {

    ECRelationshipConstraintClassP ecRelationShipconstraintClass;
    return  AddConstraintClass(ecRelationShipconstraintClass, classConstraint);
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                       MUHAMMAD.ZAIGHUM                             01/2015
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus           ECRelationshipConstraint::AddConstraintClass(ECRelationshipConstraintClass*& classConstraint, ECEntityClassCR ecClass)
    {
    ECObjectsStatus validationStatus = _ValidateClassConstraint(ecClass);
    if (validationStatus != ECObjectsStatus::Success)
        {
        return validationStatus;
        }

    return  m_constraintClasses.Add(classConstraint, ecClass);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Carole.MacDonald                03/2010
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus ECRelationshipConstraint::RemoveClass (ECEntityClassCR classConstraint)
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
        listOfClasses.push_back (const_cast<ECEntityClassP>(&constraintClassIterator->GetClass ()));
        }
    return listOfClasses;
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                   Muhammad.Zaighum                 11/14
+---------------+---------------+---------------+---------------+---------------+------*/
ECRelationshipConstraintClassList::ECRelationshipConstraintClassList(ECRelationshipClassP relClass) :m_relClass(relClass)
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

//---------------------------------------------------------------------------------------
// @bsimethod                                   Colin.Kerr                  12/2015
//---------------+---------------+---------------+---------------+---------------+-------
bool ECRelationshipConstraint::SupportsClass(ECClassCR ecClass) const
    {
    for (auto constraint : GetConstraintClasses())
        {
        ECClassCR constraintClass = constraint->GetClass();
        if (constraintClass.GetName().EqualsI("AnyClass"))
            return true;
        
        if (ECClass::ClassesAreEqualByName(&constraintClass, &ecClass) || (m_isPolymorphic && ecClass.Is(&constraintClass)))
            return true;
        }
    return false;
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
    return ECObjectsStatus::Success;
    }
   
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Carole.MacDonald                03/2010
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus ECRelationshipConstraint::SetIsPolymorphic (Utf8CP isPolymorphic)
    {
    PRECONDITION (NULL != isPolymorphic, ECObjectsStatus::PreconditionViolated);

    ECObjectsStatus status = ECXml::ParseBooleanString (m_isPolymorphic, isPolymorphic);
    if (ECObjectsStatus::Success != status)
        LOG.errorv("Failed to parse the isPolymorphic string '%s' for ECRelationshipConstraint. Expected values are True or False", isPolymorphic);
        
    return status;
    }
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Carole.MacDonald                03/2010
+---------------+---------------+---------------+---------------+---------------+------*/
RelationshipMultiplicityCR ECRelationshipConstraint::GetMultiplicity () const
    {
    return *m_multiplicity;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Carole.MacDonald                03/2010
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus ECRelationshipConstraint::SetMultiplicity (uint32_t& lowerLimit, uint32_t& upperLimit)
    {
    if (lowerLimit == 0 && upperLimit == 1)
        m_multiplicity = &s_zeroOneMultiplicity;
    else if (lowerLimit == 0 && upperLimit == UINT_MAX)
        m_multiplicity = &s_zeroManyMultiplicity;
    else if (lowerLimit == 1 && upperLimit == 1)
        m_multiplicity = &s_oneOneMultiplicity;
    else if (lowerLimit == 1 && upperLimit == UINT_MAX)
        m_multiplicity = &s_oneManyMultiplicity;
    else
        m_multiplicity = new RelationshipMultiplicity(lowerLimit, upperLimit);

    return ECObjectsStatus::Success;
    }
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Carole.MacDonald                03/2010
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus ECRelationshipConstraint::SetMultiplicity (RelationshipMultiplicityCR multiplicity)
    {
    uint32_t lowerLimit = multiplicity.GetLowerLimit();
    uint32_t upperLimit = multiplicity.GetUpperLimit();

    ECObjectsStatus validationStatus = _ValidateMultiplicityConstraint(lowerLimit, upperLimit);
    if (validationStatus != ECObjectsStatus::Success)
        return validationStatus;

    return SetMultiplicity(lowerLimit, upperLimit);
    }
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Caleb.Shafer                    08/2016
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus ECRelationshipConstraint::_SetMultiplicity(Utf8CP multiplicity, bool validate)
    {
    PRECONDITION (NULL != multiplicity, ECObjectsStatus::PreconditionViolated);
    uint32_t lowerLimit;
    uint32_t upperLimit;
    ECObjectsStatus status = ECXml::ParseMultiplicityString(lowerLimit, upperLimit, multiplicity);
    if (ECObjectsStatus::Success != status)
        {
        LOG.errorv ("Failed to parse the RelationshipMultiplicity string '%s'.", multiplicity);
        return ECObjectsStatus::ParseError;
        }

    ECObjectsStatus validationStatus = _ValidateMultiplicityConstraint(lowerLimit, upperLimit);
    if (validationStatus != ECObjectsStatus::Success)
        return validationStatus;

    return SetMultiplicity(lowerLimit, upperLimit);
    }
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Caleb.Shafer                    08/2016
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus ECRelationshipConstraint::SetMultiplicity (Utf8CP multiplicity)
    {   
    return _SetMultiplicity(multiplicity, true);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Carole.MacDonald                03/2010
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus ECRelationshipConstraint::SetCardinality (Utf8CP cardinality)
    {
    PRECONDITION (NULL != cardinality, ECObjectsStatus::PreconditionViolated);
    uint32_t lowerLimit;
    uint32_t upperLimit;
    ECObjectsStatus status = ECXml::ParseCardinalityString(lowerLimit, upperLimit, cardinality);
    if (ECObjectsStatus::Success != status)
        {
        LOG.errorv ("Failed to parse the RelationshipCardinality string '%s'.", cardinality);
        return ECObjectsStatus::ParseError;
        }
        
    return SetMultiplicity(lowerLimit, upperLimit);
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
Utf8String const ECRelationshipConstraint::GetRoleLabel () const
    {
    if(&(m_relClass->GetTarget()) == this)
        return m_relClass->GetSchema().GetLocalizedStrings().GetRelationshipTargetRoleLabel(m_relClass, GetInvariantRoleLabel());
    else
        return m_relClass->GetSchema().GetLocalizedStrings().GetRelationshipSourceRoleLabel(m_relClass, GetInvariantRoleLabel());
    }
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Carole.MacDonald                03/2010
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String const ECRelationshipConstraint::GetInvariantRoleLabel () const
    {
    if (m_roleLabel.length() != 0)
        return m_roleLabel;
        
    if (&(m_relClass->GetTarget()) == this)
        return m_relClass->GetInvariantDisplayLabel() + " (Reversed)";
    return m_relClass->GetInvariantDisplayLabel();
    }
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Carole.MacDonald                03/2010
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus ECRelationshipConstraint::SetRoleLabel (Utf8StringCR value)
    {
    m_roleLabel = value;
    return ECObjectsStatus::Success;
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

    toRelationshipConstraint.SetMultiplicity(GetMultiplicity());
    toRelationshipConstraint.SetIsPolymorphic(GetIsPolymorphic());

    ECObjectsStatus status;
    ECSchemaP destSchema = const_cast<ECSchemaP>(toRelationshipConstraint._GetContainerSchema());
    for (auto constraintClass : GetConstraintClasses())
        {
        ECClassP destConstraintClass = destSchema->GetClassP(constraintClass->GetClass().GetName().c_str());
        if (NULL == destConstraintClass)
            {
            status = destSchema->CopyClass(destConstraintClass, constraintClass->GetClass());
            if (ECObjectsStatus::Success != status)
                return status;
            }
        ECEntityClassP destAsEntity = destConstraintClass->GetEntityClassP();
        if (nullptr == destAsEntity)
            return ECObjectsStatus::DataTypeNotSupported;

        status = toRelationshipConstraint.AddClass(*destAsEntity);
        if (ECObjectsStatus::Success != status)
            return status;
        }

    return CopyCustomAttributesTo(toRelationshipConstraint);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sylvain.Pucci                  09/2013
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus ECRelationshipConstraint::GetOrderedRelationshipPropertyName (Utf8String& propertyName)  const
    {
    // see if the custom attribute signifying a Ordered relationship is defined
    IECInstancePtr caInstance = GetCustomAttribute("Bentley_Standard_CustomAttributes", "OrderedRelationshipsConstraint");
    if (caInstance.IsValid())
        {
        ECN::ECValue value;
        Utf8CP propertyName = "OrderIdProperty";
        if (ECObjectsStatus::Success == caInstance->GetValue (value, propertyName))
            {
            propertyName = value.GetUtf8CP();
            return ECObjectsStatus::Success;
            }
        }
    return ECObjectsStatus::Error;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sylvain.Pucci                  09/2013
+---------------+---------------+---------------+---------------+---------------+------*/
bool ECRelationshipConstraint::GetIsOrdered () const
    {
    // see if the custom attribute signifying a Ordered relationship is defined
    IECInstancePtr caInstance = GetCustomAttribute("Bentley_Standard_CustomAttributes", "OrderedRelationshipsConstraint");
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
    IECInstancePtr caInstance = GetCustomAttribute("Bentley_Standard_CustomAttributes", "OrderedRelationshipsConstraint");
    if (caInstance.IsValid())
        {
        ECN::ECValue value;
        Utf8CP propertyName = "OrderIdStorageMode";
        if (ECObjectsStatus::Success == caInstance->GetValue (value, propertyName))
            return (OrderIdStorageMode)value.GetInteger ();
        }
    return ORDERIDSTORAGEMODE_None;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Carole.MacDonald                03/2010
+---------------+---------------+---------------+---------------+---------------+------*/
ECRelationshipClass::ECRelationshipClass (ECN::ECSchemaCR schema) : ECEntityClass (schema), m_strength( StrengthType::Referencing), m_strengthDirection(ECRelatedInstanceDirection::Forward) 
    {
    m_source = new ECRelationshipConstraint(this, true);
    m_target = new ECRelationshipConstraint(this, false);
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
    if (!ValidateStrengthConstraint(strength, false))
        return ECObjectsStatus::RelationshipConstraintsNotCompatible;
    
    m_strength = strength;
    return ECObjectsStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Carole.MacDonald                02/2010
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus ECRelationshipClass::SetStrength (Utf8CP strength)
    {
    PRECONDITION (NULL != strength, ECObjectsStatus::PreconditionViolated);

    StrengthType strengthType;
    ECObjectsStatus status = ECXml::ParseStrengthType(strengthType, strength);
    if (ECObjectsStatus::Success != status)
        LOG.errorv ("Failed to parse the Strength string '%s' for ECRelationshipClass '%s'.", strength, this->GetName().c_str());
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
    if (!ValidateStrengthDirectionConstraint(direction, false))
        return ECObjectsStatus::RelationshipConstraintsNotCompatible;

    m_strengthDirection = direction;
    return ECObjectsStatus::Success;
    }
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Carole.MacDonald                02/2010
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus ECRelationshipClass::SetStrengthDirection (Utf8CP directionString)
    {
    PRECONDITION (NULL != directionString, ECObjectsStatus::PreconditionViolated);

    ECRelatedInstanceDirection direction;
    ECObjectsStatus status = ECXml::ParseDirectionString(direction, directionString);
    if (ECObjectsStatus::Success != status)
        LOG.errorv ("Failed to parse the ECRelatedInstanceDirection string '%s' for ECRelationshipClass '%s'.", directionString, this->GetName().c_str());
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
    IECInstancePtr caInstance = GetCustomAttribute("Bentley_Standard_CustomAttributes", "SupportsOrderedRelationships");
    if (caInstance.IsValid())
        return true;

    return false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Bill.Steinbock                  09/2010
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus ECRelationshipClass::GetOrderedRelationshipPropertyName (Utf8String& propertyName, ECRelationshipEnd end) const
    {
    // see if the struct has a custom attribute to custom persist itself
    IECInstancePtr caInstance = GetCustomAttribute("Bentley_Standard_CustomAttributes", "SupportsOrderedRelationships");
    if (caInstance.IsValid())
        {
        ECN::ECValue value;
        Utf8CP propertyName = "OrderIdTargetProperty";

        if (end == ECRelationshipEnd_Source)
            propertyName = "OrderIdSourceProperty";

        if (ECObjectsStatus::Success == caInstance->GetValue (value, propertyName))
            {
            propertyName = value.GetUtf8CP();
            return ECObjectsStatus::Success;
            }
        }

    return ECObjectsStatus::Error;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   
+---------------+---------------+---------------+---------------+---------------+------*/
SchemaWriteStatus ECRelationshipClass::_WriteXml (BeXmlWriterR xmlWriter, int ecXmlVersionMajor, int ecXmlVersionMinor) const
    {
    SchemaWriteStatus   status;
    bmap<Utf8CP, Utf8CP> additionalAttributes;
    additionalAttributes[STRENGTH_ATTRIBUTE] = ECXml::StrengthToString(m_strength);
    if (m_strengthDirection != ECRelatedInstanceDirection::Forward)
        { //skip the attribute for "forward" as it is the default value.
        additionalAttributes[STRENGTHDIRECTION_ATTRIBUTE] = ECXml::DirectionToString(m_strengthDirection);
        }

    if (SchemaWriteStatus::Success != (status = ECClass::_WriteXml (xmlWriter, ecXmlVersionMajor, ecXmlVersionMinor, EC_RELATIONSHIP_CLASS_ELEMENT, &additionalAttributes, false)))
        return status;
        
    // verify that this really is the current relationship class element // CGM 07/15 - Can't do this with an XmlWriter
    //if (0 != strcmp (classNode->GetName(), EC_RELATIONSHIP_CLASS_ELEMENT))
    //    {
    //    BeAssert (false);
    //    return SchemaWriteStatus::FailedToCreateXml;
    //    }
        
    m_source->WriteXml (xmlWriter, EC_SOURCECONSTRAINT_ELEMENT, ecXmlVersionMajor, ecXmlVersionMinor);
    m_target->WriteXml (xmlWriter, EC_TARGETCONSTRAINT_ELEMENT, ecXmlVersionMajor, ecXmlVersionMinor);
    xmlWriter.WriteElementEnd();

    return status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Carole.MacDonald                02/2010
+---------------+---------------+---------------+---------------+---------------+------*/
SchemaReadStatus ECRelationshipClass::_ReadXmlAttributes (BeXmlNodeR classNode)
    {
    SchemaReadStatus status;
    if (SchemaReadStatus::Success != (status = T_Super::_ReadXmlAttributes (classNode)))
        return status;
        
    Utf8String value;
    READ_OPTIONAL_XML_ATTRIBUTE (classNode, STRENGTH_ATTRIBUTE, this, Strength)
    READ_OPTIONAL_XML_ATTRIBUTE (classNode, STRENGTHDIRECTION_ATTRIBUTE, this, StrengthDirection)
    
    return SchemaReadStatus::Success;
    }
 
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Carole.MacDonald                02/2010
+---------------+---------------+---------------+---------------+---------------+------*/
SchemaReadStatus ECRelationshipClass::_ReadXmlContents (BeXmlNodeR classNode, ECSchemaReadContextR context, ECSchemaCP conversionSchema, bvector<NavigationECPropertyP>& navigationProperties)
    {
    SchemaReadStatus status = T_Super::_ReadXmlContents (classNode, context, conversionSchema, navigationProperties);
    if (status != SchemaReadStatus::Success)
        return status;

    BeXmlNodeP sourceNode = classNode.SelectSingleNode (EC_NAMESPACE_PREFIX ":" EC_SOURCECONSTRAINT_ELEMENT);
    if (NULL != sourceNode)
        status = m_source->ReadXml (*sourceNode, context);
    if (status != SchemaReadStatus::Success)
        return status;
    
    BeXmlNodeP  targetNode = classNode.SelectSingleNode (EC_NAMESPACE_PREFIX ":" EC_TARGETCONSTRAINT_ELEMENT);
    if (NULL != targetNode)
        status = m_target->ReadXml (*targetNode, context);
    if (status != SchemaReadStatus::Success)
        return status;
        
    return status;
    }

 /*---------------------------------------------------------------------------------**//**
* @bsimethod                                    
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus ECRelationshipClass::_AddBaseClass(ECClassCR baseClass, bool insertAtBeginning, bool resolveConflicts, bool validate)
    {
    if (baseClass.IsRelationshipClass())
        {
        // Get the relationship base class and compare it's strength and direction
        ECRelationshipClassCP relationshipBaseClass = baseClass.GetRelationshipClassCP();
        if (!ValidateStrengthConstraint(relationshipBaseClass->GetStrength()) ||
            !ValidateStrengthDirectionConstraint(relationshipBaseClass->GetStrengthDirection()))
            {
            return ECObjectsStatus::RelationshipConstraintsNotCompatible;
            }

        if (validate)
            {
            if (RelationshipMultiplicity::Compare(GetSource().GetMultiplicity(), relationshipBaseClass->GetSource().GetMultiplicity()) == -1)
                {
			    LOG.errorv("Multiplicity Violation: The Source Multiplicity (%u..%u) of %s is larger than the Multiplicity of it's base class %s (%u..%u)",
				    GetSource().GetMultiplicity().GetLowerLimit(), GetSource().GetMultiplicity().GetUpperLimit(), GetName().c_str(), relationshipBaseClass->GetName().c_str(), relationshipBaseClass->GetSource().GetMultiplicity().GetLowerLimit(), relationshipBaseClass->GetSource().GetMultiplicity().GetUpperLimit());
			    return ECObjectsStatus::RelationshipConstraintsNotCompatible;
                }

            if (RelationshipMultiplicity::Compare(GetTarget().GetMultiplicity(), relationshipBaseClass->GetTarget().GetMultiplicity()) == -1)
                {
			    LOG.errorv("Multiplicity Violation: The Target Multiplicity (%u..%u) of %s is larger than the Multiplicity of it's base class %s (%u..%u)",
				    GetTarget().GetMultiplicity().GetLowerLimit(), GetTarget().GetMultiplicity().GetUpperLimit(), GetName().c_str(), relationshipBaseClass->GetName().c_str(), relationshipBaseClass->GetTarget().GetMultiplicity().GetLowerLimit(), relationshipBaseClass->GetTarget().GetMultiplicity().GetUpperLimit());
			    return ECObjectsStatus::RelationshipConstraintsNotCompatible;
                }
            }
        }

    return ECClass::_AddBaseClass(baseClass, insertAtBeginning, resolveConflicts, validate);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    
+---------------+---------------+---------------+---------------+---------------+------*/
bool ECRelationshipClass::ValidateStrengthConstraint(StrengthType value, bool compareValue) const
    {
    if (HasBaseClasses())
        {
        for (auto baseClass : GetBaseClasses())
            {
            ECRelationshipClassCP relationshipBaseClass = baseClass->GetRelationshipClassCP();
            if (relationshipBaseClass != nullptr && !relationshipBaseClass->ValidateStrengthConstraint(value))
                {
                LOG.errorv("Strength Constraint: ECRelationshipClass '%s' has different Strength (%d) than it's baseclass '%s' (%d).",
                            GetName().c_str(), value, relationshipBaseClass->GetName().c_str(), relationshipBaseClass->GetStrength());
                return false;
                }
            }
        }

    return (!compareValue || GetStrength() == value);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    
+---------------+---------------+---------------+---------------+---------------+------*/
bool ECRelationshipClass::ValidateStrengthDirectionConstraint(ECRelatedInstanceDirection value, bool compareValue) const
    {
    if (HasBaseClasses())
        {
        for (auto baseClass : GetBaseClasses())
            {
            ECRelationshipClassCP relationshipBaseClass = baseClass->GetRelationshipClassCP();
            if (relationshipBaseClass != nullptr && !relationshipBaseClass->ValidateStrengthDirectionConstraint(value))
                {
                LOG.errorv("Strength Direction Constraint Violation: ECRelationshipClass '%s' has different StrengthDirection (%d) than it's baseclass '%s' (%d).",
                            GetName().c_str(), value, relationshipBaseClass->GetName().c_str(), relationshipBaseClass->GetStrengthDirection());
                return false;
                }
            }
        }

    return (!compareValue || GetStrengthDirection() == value);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Caleb.Shafer                    08/2016
+---------------+---------------+---------------+---------------+---------------+------*/
bool ECRelationshipClass::ValidateMultiplicityConstraint() const
    {
    if (ECObjectsStatus::Success != GetSource().ValidateMultiplicityConstraint() || 
        ECObjectsStatus::Success != GetTarget().ValidateMultiplicityConstraint())
        return false;

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Caleb.Shafer                    08/2016
+---------------+---------------+---------------+---------------+---------------+------*/
bool ECRelationshipClass::ValidateClassConstraint() const
    {
    if (ECObjectsStatus::Success != GetSource().ValidateClassConstraint() || 
        ECObjectsStatus::Success != GetTarget().ValidateClassConstraint())
        return false;

    return true;
    }
   
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  12/2012
+---------------+---------------+---------------+---------------+---------------+------*/
bool            ECClass::Is(Utf8CP name) const
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
bool ECClass::Is (Utf8CP schemaname, Utf8CP classname) const
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
    return ECObjectsStatus::Success;
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
ECObjectsStatus ECRelationshipConstraintClassList::Remove(ECEntityClassCR constraintClass)
    {
    for (auto itor = m_constraintClasses.begin(); itor != m_constraintClasses.end(); itor++)
        {
        if (&itor->get()->GetClass() == &constraintClass)
            {
            m_constraintClasses.erase(itor);
            return ECObjectsStatus::Success;
            }
        }

    return ECObjectsStatus::ClassNotFound;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                             Muhammad.Zaighum                   11/14
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus ECRelationshipConstraintClassList::Add(ECRelationshipConstraintClass*& classConstraint, ECEntityClassCR ecClass)
    {
    classConstraint = nullptr;
    if (&(ecClass.GetSchema()) != &(m_relClass->GetSchema()))
        {
        ECSchemaReferenceListCR referencedSchemas = m_relClass->GetSchema().GetReferencedSchemas();
        ECSchemaReferenceList::const_iterator schemaIterator = referencedSchemas.find(ecClass.GetSchema().GetSchemaKey());
        if (schemaIterator == referencedSchemas.end())
            return ECObjectsStatus::SchemaNotFound;
        }

    for (auto &constraintClassIterator : m_constraintClasses)
        {
        if (&constraintClassIterator->GetClass() == &ecClass)
            {
            classConstraint = constraintClassIterator.get();
            return ECObjectsStatus::Success;
            }
        }
    auto newConstraintClass =  std::unique_ptr<ECRelationshipConstraintClass>(new ECRelationshipConstraintClass(ecClass));
    classConstraint = newConstraintClass.get();
    m_constraintClasses.push_back(std::move(newConstraintClass));
    return ECObjectsStatus::Success;
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
ECRelationshipConstraintClass::ECRelationshipConstraintClass(ECEntityClassCR ecClass) : m_ecClass(&ecClass)
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
void ECRelationshipConstraintClass::AddKey(Utf8CP keyPropertyName)
    {
    if (Utf8String::IsNullOrEmpty(keyPropertyName))
        {
        BeAssert(false && "keyPropertyName arg must not be nullptr or empty string.");
        return;
        }

    m_keys.push_back(keyPropertyName);
    }

END_BENTLEY_ECOBJECT_NAMESPACE
