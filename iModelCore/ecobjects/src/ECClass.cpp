/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/

#include "ECObjectsPch.h"

BEGIN_BENTLEY_ECOBJECT_NAMESPACE

static RelationshipMultiplicity s_zeroOneMultiplicity(0, 1);
static RelationshipMultiplicity s_zeroManyMultiplicity(0, INT_MAX);
static RelationshipMultiplicity s_oneOneMultiplicity(1, 1);
static RelationshipMultiplicity s_oneManyMultiplicity(1, INT_MAX);

extern ECObjectsStatus ResolveStructType(ECStructClassCP& structClass, Utf8StringCR typeName, ECClassCR ecClass, bool doLogging = true);

// If you are developing schemas, particularly when editing them by hand, you want to have this variable set to false so you get the asserts to help you figure out what is going wrong.
// Test programs generally want to get error status back and not BeAssert, so they call ECSchema::AssertOnXmlError (false);
static  bool        s_noAssert = false;
/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void ECClass::SetErrorHandling (bool doAssert)
    {
    s_noAssert = !doAssert;
    ECProperty::SetErrorHandling(doAssert);
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------+---------------+---------------+---------------+---------------+-------
ECClass::ECClass (ECClassType classType, ECSchemaCR schema) : m_classType(classType), m_schema(schema), m_modifier(ECClassModifier::None) { }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------+---------------+---------------+---------------+---------------+-------
ECClass::~ECClass ()
    {
    RemoveDerivedClasses ();
    RemoveBaseClasses ();

    m_propertyList.clear();

    for (PropertyMap::iterator entry=m_propertyMap.begin(); entry != m_propertyMap.end(); ++entry)
        delete entry->second;

    m_propertyMap.clear();

    m_defaultStandaloneEnabler.Invalidate();
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------+---------------+---------------+---------------+---------------+-------
Utf8CP ECClass::GetFullName () const
    {
    auto const& fullName = m_fullName.GetName(*this);
    return fullName.c_str();
    }

//--------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
Utf8StringCR ECClass::GetECSqlName() const
    {
    return m_ecsqlName.Get([&]()
        {
        Utf8String name;
        name.append(1, '[').append(GetSchema().GetName()).append("].[").append(GetName()).append(1, ']');
        return name;
        });
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------+---------------+---------------+---------------+---------------+-------
ECObjectsStatus ECClass::SetName (Utf8StringCR name)
    {
    if (!m_validatedName.SetValidName(name.c_str(), m_schema.OriginalECXmlVersionLessThan(ECVersion::V3_1)))
        return ECObjectsStatus::InvalidName;

    m_fullName.RecomputeName(*this);

    return ECObjectsStatus::Success;
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------+---------------+---------------+---------------+---------------+-------
Utf8StringCR ECClass::GetDescription () const
    {
    return m_description;
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------+---------------+---------------+---------------+---------------+-------
Utf8StringCR ECClass::GetDisplayLabel () const
    {
    return GetInvariantDisplayLabel();
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------+---------------+---------------+---------------+---------------+-------
ECObjectsStatus ECClass::SetDisplayLabel (Utf8StringCR displayLabel)
    {
    m_validatedName.SetDisplayLabel (displayLabel.c_str());
    return ECObjectsStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
 @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
StandaloneECEnablerPtr ECClass::GetDefaultStandaloneEnabler() const
    {
    auto enabler = m_defaultStandaloneEnabler.Get([&]()
        {
        ClassLayoutPtr classLayout = ClassLayout::BuildFromClass(*this);
        return StandaloneECEnabler::CreateEnabler(*this, *classLayout, nullptr);
        });

    BeAssert(enabler.IsValid());
    return enabler;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
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
* @bsimethod
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
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus ECClass::DeleteProperty (ECPropertyR prop)
    {
    ECObjectsStatus status = RemoveProperty (prop);
    if (ECObjectsStatus::Success == status)
        delete &prop;

    return status;
    }

//---------------------------------------------------------------------------------------
// @bsimethod
// NOTE: Should always return Success if resolveConflicts is true
//---------------+---------------+---------------+---------------+---------------+-------
ECObjectsStatus ECClass::FindPropertyConflicts(ECPropertyCP prop, ECPropertyP &baseProp, Utf8StringR newName, Utf8StringR errorMessage, bool resolveConflicts)
    {
    bool aliasPrepended = false;
    newName = prop->GetName().c_str();
    ECPropertyP existingProperty = GetPropertyP(newName.c_str(), false);
    if (nullptr != existingProperty)
        {
        if (!resolveConflicts)
            return ECObjectsStatus::NamedItemAlreadyExists;
        
        // Find locally unique property name
        Utf8PrintfString testName("%s_%s_", GetSchema().GetAlias().c_str(), prop->GetName().c_str());
        aliasPrepended = true;
        while (nullptr != GetPropertyP(testName.c_str(), false))
            testName.append("_");

        newName = testName;
        }

    // Find compatible base property
    ECObjectsStatus status = ECObjectsStatus::Success;
    baseProp = nullptr;
    while (nullptr != (baseProp = GetBaseClassPropertyP(newName.c_str())) && ECObjectsStatus::Success != (status = CanPropertyBeOverridden(*baseProp, *prop, errorMessage)))
        {
        if (!resolveConflicts)
            return status;
        if (!aliasPrepended)
            {
            Utf8PrintfString testName("%s_%s_", GetSchema().GetAlias().c_str(), prop->GetName().c_str());
            newName = testName;
            aliasPrepended = true;
            }
        else
            newName.append("_");
        }
    
    if (nullptr != baseProp && !baseProp->GetName().Equals(prop->GetName()))
        {
        if (!resolveConflicts)
            return ECObjectsStatus::CaseCollision;
        newName = baseProp->GetName().c_str();
        }
    
    return ECObjectsStatus::Success;
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------+---------------+---------------+---------------+---------------+-------
ECObjectsStatus ECClass::RenameConflictProperty(ECPropertyP prop, bool renameDerivedProperties, ECPropertyP& renamedProperty)
    {
    Utf8String originalName = prop->GetName();

    Utf8String newName, errorMessage;
    ECPropertyP baseProp = nullptr;
    FindPropertyConflicts(prop, baseProp, newName, errorMessage, true);
    ECObjectsStatus status = RenameConflictProperty(prop, renameDerivedProperties, renamedProperty, newName);
    if (ECObjectsStatus::Success != status)
        LOG.errorv("Failed to rename property %s:%s to %s", GetFullName(), originalName.c_str(), newName.c_str());
    else
        LOG.debugv("The property %s:%s was renamed to %s", GetFullName(), originalName.c_str(), newName.c_str());

    return status;
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------+---------------+---------------+---------------+---------------+-------
ECObjectsStatus ECClass::RenameConflictProperty(ECPropertyP prop, bool renameDerivedProperties, ECPropertyP& renamedProperty, Utf8String newName)
    {
    Utf8String originalName = prop->GetName();

    PropertyMap::iterator iter = m_propertyMap.find(prop->GetName().c_str());
    if (iter == m_propertyMap.end())
        return ECObjectsStatus::PropertyNotFound;
    ECPropertyP thisProp = iter->second; // Since the property that is passed in might come from a base class, we need the actual pointer of the property from this class in order to search the propertyList for it

    ECObjectsStatus status;
    if (ECObjectsStatus::Success != (status = CopyProperty(renamedProperty, thisProp, newName.c_str(), true, false)))
        {
        delete renamedProperty;
        return status;
        }

    iter = m_propertyMap.find(thisProp->GetName().c_str());
    m_propertyMap.erase(iter);
    auto iter2 = std::find(m_propertyList.begin(), m_propertyList.end(), thisProp);
    if (iter2 != m_propertyList.end())
        m_propertyList.erase(iter2);
    InvalidateDefaultStandaloneEnabler();

    status = AddProperty(renamedProperty, newName, true); // Pass true for resolveConflicts because we only rename conflict properties when that is true
    delete thisProp;
    if (ECObjectsStatus::Success != status)
        {
        delete renamedProperty;
        return status;
        }

    if (!renamedProperty->GetIsDisplayLabelDefined())
        renamedProperty->SetDisplayLabel(originalName);

    LOG.infov("Renamed conflict property %s:%s to %s\n", GetFullName(), originalName.c_str(), newName.c_str());

    // If newProperty was successfully added we need to add a CustomAttribute. To help identify the property when doing instance data conversion.
    AddPropertyMapping(originalName.c_str(), newName.c_str());

    if (renameDerivedProperties)
        RenameDerivedProperties(originalName, newName);

    return ECObjectsStatus::Success;
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------+---------------+---------------+---------------+---------------+-------
void ECClass::RenameDerivedProperties(Utf8StringCR originalName, Utf8StringCR newName)
    {
    ECPropertyP newBaseProp = GetPropertyP(newName.c_str());
    for (ECClassP derivedClass : m_derivedClasses)
        {
        // this can happen with multi-inheritance where the property has already been renamed in one path
        if (derivedClass->HasPropertyMapping(originalName.c_str()))
            continue;

        ECPropertyP fromDerived = derivedClass->GetPropertyP(newName.c_str(), false);
        if (nullptr != fromDerived)
            {
            ECPropertyP renamedProperty = nullptr;
            derivedClass->RenameConflictProperty(fromDerived, true, renamedProperty);
            }
        ECPropertyP fromDerivedOld = derivedClass->GetPropertyP(originalName.c_str(), false);
        if (nullptr != fromDerivedOld)
            {
            ECPropertyP renamedProperty = nullptr;
            derivedClass->RenameConflictProperty(fromDerivedOld, true, renamedProperty, newName);
            renamedProperty->SetBaseProperty(newBaseProp);
            }

        derivedClass->RenameDerivedProperties(originalName, newName);
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------+---------------+---------------+---------------+---------------+-------
bool ECClass::HasPropertyMapping(Utf8CP oldPropertyName)
    {
    IECInstancePtr renameInstance = GetCustomAttributeLocal("RenamedPropertiesMapping");
    if (!renameInstance.IsValid())
        return false;

    ECValue v;
    renameInstance->GetValue(v, "PropertyMapping");
    if (v.IsNull())
        return false;

    bvector<Utf8String> components;
    BeStringUtilities::Split(v.GetUtf8CP(), ";", components);
    for (Utf8String mapping : components)
        {
        bvector<Utf8String> components2;
        BeStringUtilities::Split(mapping.c_str(), "|", components2);
        if (components2[0].Equals(oldPropertyName))
            return true;
        }
    return false;
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------+---------------+---------------+---------------+---------------+-------
void ECClass::AddPropertyMapping(Utf8CP originalName, Utf8CP newName)
    {
    IECInstancePtr renameInstance = GetCustomAttributeLocal("RenamedPropertiesMapping");
    if (!renameInstance.IsValid())
        renameInstance = ConversionCustomAttributeHelper::CreateCustomAttributeInstance("RenamedPropertiesMapping");
    if (!renameInstance.IsValid())
        {
        LOG.warningv("Failed to create 'RenamedPropertiesMapping' custom attribute on ECClass '%s' for Property '%s'", GetFullName(), newName);
        return;
        }

    ECValue v;
    renameInstance->GetValue(v, "PropertyMapping");
    Utf8String remapping("");
    if (!v.IsNull())
        {
        Utf8String existing(v.GetUtf8CP());
        Utf8String search("|");
        search.append(originalName);
        size_t index = existing.find(search);
        if (index != std::string::npos)
            {
            index++;
            size_t index2 = existing.find(";", index);
            if (index2 == std::string::npos)
                index2 = existing.length();
            size_t length = index2 - index;
            remapping = existing.replace(index, length, newName);
            }
        else
            {
            remapping = Utf8String(v.GetUtf8CP()).append(";");
            remapping.append(originalName).append("|").append(newName);
            }
        }
    else
        remapping.append(originalName).append("|").append(newName);

    v.SetUtf8CP(remapping.c_str());
    if (ECObjectsStatus::Success != renameInstance->SetValue("PropertyMapping", v))
        {
        LOG.warningv("Failed to create 'RenamedPropertiesMapping' custom attribute for the ECClass '%s' with 'PropertyMapping' set to '%s'.", GetFullName(), remapping.c_str());
        return;
        }

    if (!ECSchema::IsSchemaReferenced(GetSchema(), renameInstance->GetClass().GetSchema()))
        {
        ECClassP nonConstClass = const_cast<ECClassP>(&renameInstance->GetClass());
        if (ECObjectsStatus::Success != GetContainerSchema()->AddReferencedSchema(nonConstClass->GetSchemaR()))
            {
            LOG.warningv("Failed to add %s as a referenced schema to %s.", renameInstance->GetClass().GetSchema().GetName().c_str(), GetSchema().GetName().c_str());
            LOG.warningv("Failed to add 'RenamedPropertiesMapping' custom attribute to ECClass '%s'.", GetFullName(), GetName().c_str());
            return;
            }
        }

    if (ECObjectsStatus::Success != SetCustomAttribute(*renameInstance))
        {
        LOG.warningv("Failed to add 'RenamedPropertiesMapping' custom attribute, with 'PropertyMapping' set to '%s', to ECClass '%s'.", remapping.c_str(), GetFullName());
        return;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ECPropertyP ECClass::GetPropertyByIndex (uint32_t index) const
    {
    if (index >= (uint32_t)m_propertyList.size())
        return nullptr;

    return m_propertyList[index];
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
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
        case VALUEKIND_Array:       newProperty = new PrimitiveArrayECProperty (*this); break;
        case VALUEKIND_Struct:      newProperty = new StructECProperty (*this); break;
        case VALUEKIND_Navigation:  newProperty = new NavigationECProperty(*this); break;
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
* @bsimethod
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
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void ECClass::InvalidateDefaultStandaloneEnabler() const
    {
    // When class structure changes, the ClassLayout stored in this enabler becomes out-of-date
    // nullify it so it will be reconstructed on next call to GetDefaultStandaloneEnabler()
    m_defaultStandaloneEnabler.Invalidate();
    for (ECClassP derivedClass : m_derivedClasses)
        derivedClass->InvalidateDefaultStandaloneEnabler();

    BeMutexHolder holder(GetSchema().GetMutex());
    m_cachedProperties = nullptr;
    }

ECObjectsStatus ECClass::ResolvePropertyOverrideIssue(ECPropertyCR baseProperty, ECPropertyP &derivedProperty, ECObjectsStatus overrideFailure, Utf8StringR failureMessage, bool resolveConflicts)
    {
    if (!Utf8String::IsNullOrEmpty(failureMessage.c_str()))
        LOG.message (resolveConflicts? NativeLogging::LOG_DEBUG : NativeLogging::LOG_ERROR, failureMessage.c_str());
    
    if (!resolveConflicts)
        return overrideFailure;
    
    ECObjectsStatus status = ECObjectsStatus::Success;
    if (ECObjectsStatus::DataTypeMismatch == overrideFailure || ECObjectsStatus::InvalidPrimitiveOverrride == overrideFailure)
        {
        LOG.debugv("Conflict between %s:%s and %s:%s.  Renaming...", derivedProperty->GetClass().GetFullName(), derivedProperty->GetName().c_str(), baseProperty.GetClass().GetFullName(), baseProperty.GetName().c_str());
        ECPropertyP newProperty = nullptr;
        if (ECObjectsStatus::Success != (status = RenameConflictProperty(derivedProperty, true, newProperty)))
            return status;
        derivedProperty = newProperty;
        }
    return ECObjectsStatus::Success;
    }

ECObjectsStatus ECClass::ValidateBaseProperty(ECPropertyCR baseProperty, ECPropertyP &derivedProperty, bool resolveConflicts)
    {
    ECObjectsStatus status = ECObjectsStatus::Success;
    Utf8String errMsg;
    if (ECObjectsStatus::Success != (status = CanPropertyBeOverridden(baseProperty, *derivedProperty, errMsg)))
        {
        if (ECObjectsStatus::Success != (status = ResolvePropertyOverrideIssue(baseProperty, derivedProperty, status, errMsg, resolveConflicts)))
            return status;
        }

    // If the property names do not have the same case, this is an error
    if (baseProperty.GetName().EqualsIAscii(derivedProperty->GetName()) && !baseProperty.GetName().Equals(derivedProperty->GetName()))
        {
        LOG.messagev (resolveConflicts? NativeLogging::LOG_DEBUG : NativeLogging::LOG_ERROR, 
            "Case-collision between %s:%s and %s:%s", baseProperty.GetClass().GetFullName(), baseProperty.GetName().c_str(), GetFullName(), derivedProperty->GetName().c_str());
        
        if (!resolveConflicts)
            return ECObjectsStatus::CaseCollision;

        ECPropertyP newProperty = nullptr;
        if (ECObjectsStatus::Success != (status = RenameConflictProperty(derivedProperty, false, newProperty, baseProperty.GetName())))
            return status;
        
        derivedProperty = newProperty;
        }
    return ECObjectsStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus ECClass::OnBaseClassPropertyAdded (ECPropertyCR baseProperty, bool resolveConflicts)
    {
    InvalidateDefaultStandaloneEnabler();

    // This is a case-insensitive search
    ECPropertyP derivedProperty = GetPropertyP (baseProperty.GetName(), false);
    ECObjectsStatus status = ECObjectsStatus::Success;
    if (nullptr != derivedProperty)
        {
        if (ECObjectsStatus::Success != (status = ValidateBaseProperty(baseProperty, derivedProperty, resolveConflicts)))
            {
            LOG.errorv ("Base Property %s:%s not compatible with %s:%s", 
                baseProperty.GetClass().GetFullName(), baseProperty.GetName().c_str(), GetFullName(), derivedProperty->GetName().c_str());
            return status;
            }

        if (baseProperty.GetName().Equals(derivedProperty->GetName()) && // Only set if names match
            (nullptr == derivedProperty->GetBaseProperty() || GetBaseClassPropertyP (baseProperty.GetName().c_str()) == &baseProperty))
            derivedProperty->SetBaseProperty(&baseProperty);
        }
    for (ECClassP derivedClass : m_derivedClasses)
        status = derivedClass->OnBaseClassPropertyAdded (baseProperty, resolveConflicts);

    return status;
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------+---------------+---------------+---------------+---------------+-------
ECObjectsStatus ECClass::AddProperty (ECPropertyP& pProperty, bool resolveConflicts)
    {
    ECPropertyP baseProperty = nullptr;
    Utf8String newName, errorMessage;
    ECObjectsStatus status = FindPropertyConflicts(pProperty, baseProperty, newName, errorMessage, resolveConflicts);
    switch (status)
        {
        case ECObjectsStatus::Success :
            break;
        case ECObjectsStatus::NamedItemAlreadyExists :
            LOG.errorv("Cannot add property '%s' because it already exists in this ECClass (%s:%s)", 
                pProperty->GetName().c_str(), GetSchema().GetFullSchemaName().c_str(), GetName().c_str());
            return ECObjectsStatus::NamedItemAlreadyExists;
        case ECObjectsStatus::DataTypeMismatch :
        case ECObjectsStatus::InvalidPrimitiveOverrride :
            if (!Utf8String::IsNullOrEmpty(errorMessage.c_str()))
                LOG.error(errorMessage.c_str());
            else
                {
                Utf8String propTypeName = pProperty->GetTypeName();
                LOG.errorv("Could not add property '%s' of type '%s' to '%s:%s' due to a data type mismatch",
                    pProperty->GetName().c_str(), propTypeName.c_str(), GetSchema().GetFullSchemaName().c_str(), GetName().c_str());
                }
            return status;
        case ECObjectsStatus::CaseCollision :
            LOG.errorv("Could not add property '%s' to '%s' due to case-collision with %s:%s", 
                pProperty->GetName().c_str(), GetFullName(), baseProperty->GetClass().GetFullName(), baseProperty->GetName().c_str());
            return ECObjectsStatus::CaseCollision;
        default :
            LOG.errorv("Could not add property '%s' to '%s' due to unknown error", pProperty->GetName().c_str(), GetFullName());
            return ECObjectsStatus::Error;
        }

    if (!newName.Equals(pProperty->GetName()))
        {
        if (!newName.EqualsIAscii(pProperty->GetName()))
            AddPropertyMapping(pProperty->GetName().c_str(), newName.c_str());
        pProperty->SetDisplayLabel(pProperty->GetName());
        pProperty->SetName(newName);
        }
    if (nullptr != baseProperty)
        pProperty->SetBaseProperty(baseProperty);


    m_propertyMap.insert (bpair<Utf8CP, ECPropertyP> (pProperty->GetName().c_str(), pProperty));
    m_propertyList.push_back(pProperty);

    InvalidateDefaultStandaloneEnabler();

    for (ECClassP derivedClass : m_derivedClasses)
        status = derivedClass->OnBaseClassPropertyAdded (*pProperty, resolveConflicts);
    
    if (ECObjectsStatus::Success != status)
        {
        RemoveProperty(*pProperty);
        return status;
        }

    return ECObjectsStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus ECClass::CopyProperty(ECPropertyP& destProperty, ECPropertyCP sourceProperty, bool copyCustomAttributes)
    {
    if (nullptr == sourceProperty)
        return ECObjectsStatus::NullPointerValue;

    return CopyProperty(destProperty, sourceProperty, sourceProperty->GetName().c_str(), copyCustomAttributes);
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------+---------------+---------------+---------------+---------------+-------
ECObjectsStatus ECClass::CopyProperty(ECPropertyP& destProperty, Utf8CP destPropertyName, ECPropertyCP sourceProperty, bool copyCustomAttributes)
    {
    return CopyProperty(destProperty, sourceProperty, destPropertyName, copyCustomAttributes);
    }

template<typename PrimitiveProperty>
ECObjectsStatus setPrimitivePropertyAttributes(ECClassP destClass, PrimitiveProperty * destPrimitive, PrimitiveProperty const* sourcePrimitive, bool copyReferences)
    {
    ECObjectsStatus status = ECObjectsStatus::Success;
    ECEnumerationCP enumeration = sourcePrimitive->GetEnumeration();

    if (nullptr != enumeration)
        {
        ECEnumerationP targetEnum;
        status = destClass->GetSchemaR().GetOrCopyReferencedEnumerationForCopy(sourcePrimitive->GetClass(), targetEnum, enumeration, copyReferences);
        if (ECObjectsStatus::Success != status)
            return status;

        if (ECObjectsStatus::Success != (status = destPrimitive->SetType(*targetEnum)))
            return status;
        }
    else
        destPrimitive->SetType(sourcePrimitive->GetType());


    if (sourcePrimitive->IsMinimumValueDefined())
        {
        ECValue valueToCopy;
        sourcePrimitive->GetMinimumValue(valueToCopy);
        destPrimitive->SetMinimumValue(valueToCopy);
        }

    if (sourcePrimitive->IsMaximumValueDefined())
        {
        ECValue valueToCopy;
        sourcePrimitive->GetMaximumValue(valueToCopy);
        destPrimitive->SetMaximumValue(valueToCopy);
        }

    if (sourcePrimitive->IsMinimumLengthDefined())
        destPrimitive->SetMinimumLength(sourcePrimitive->GetMinimumLength());
    if (sourcePrimitive->IsMaximumLengthDefined())
        destPrimitive->SetMaximumLength(sourcePrimitive->GetMaximumLength());

    if (sourcePrimitive->IsExtendedTypeDefinedLocally())
        destPrimitive->SetExtendedTypeName(sourcePrimitive->GetExtendedTypeName().c_str());

    return status;
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------+---------------+---------------+---------------+---------------+-------
ECObjectsStatus ECClass::CopyProperty(ECPropertyP& destProperty, ECPropertyCP sourceProperty, Utf8CP destPropertyName, bool copyCustomAttributes, bool andAddProperty, bool copyReferences)
    {
    if (nullptr == sourceProperty)
        return ECObjectsStatus::NullPointerValue;

    ECObjectsStatus status;

    if (sourceProperty->GetIsPrimitive())
        {
        auto* destPrimitive = new PrimitiveECProperty(*this);
        if (ECObjectsStatus::Success != (status = setPrimitivePropertyAttributes(this, destPrimitive, sourceProperty->GetAsPrimitiveProperty(), copyReferences)))
            {
            delete(destPrimitive);
            return status;
            }

        destProperty = destPrimitive;
        }
    else if (sourceProperty->GetIsStructArray())
        {
        auto* destStructArray = new StructArrayECProperty(*this);
        StructArrayECPropertyCP sourceStructArray = sourceProperty->GetAsStructArrayProperty();
        ECStructClassCP structClass = &sourceStructArray->GetStructElementType();
        ECClassP target;
        if (ECObjectsStatus::Success != (status = GetSchemaR().GetOrCopyReferencedClassForCopy(sourceProperty->GetClass(), target, structClass, copyReferences)))
            {
            delete(destStructArray);
            return status;
            }

        ECStructClassP targetStruct = target->GetStructClassP();
        if (nullptr == targetStruct)
            {
            LOG.errorv("Could not copy struct property '%s.%s' to '%s' because the class '%s' found in the target schema '%s' is not a struct class",
                sourceProperty->GetClass().GetFullName(), sourceProperty->GetName().c_str(), GetFullName(), structClass->GetFullName(), target->GetSchema().GetFullSchemaName().c_str());
            delete(destStructArray);
            return ECObjectsStatus::ClassTypeNotCorrect;
            }

        if (ECObjectsStatus::Success != (status = destStructArray->SetStructElementType(*targetStruct)))
            {
            delete(destStructArray);
            return status;
            }

        destStructArray->SetMaxOccurs(sourceStructArray->GetStoredMaxOccurs());
        destStructArray->SetMinOccurs(sourceStructArray->GetMinOccurs());
        destProperty = destStructArray;
        }
    else if (sourceProperty->GetIsPrimitiveArray())
        {
        auto* destPrimitiveArray = new PrimitiveArrayECProperty(*this);
        PrimitiveArrayECPropertyCP sourcePrimitiveArray = sourceProperty->GetAsPrimitiveArrayProperty();

        if (ECObjectsStatus::Success != (status = setPrimitivePropertyAttributes(this, destPrimitiveArray, sourcePrimitiveArray, copyReferences)))
            {
            delete(destPrimitiveArray);
            return status;
            }

        destPrimitiveArray->SetMaxOccurs(sourcePrimitiveArray->GetStoredMaxOccurs());
        destPrimitiveArray->SetMinOccurs(sourcePrimitiveArray->GetMinOccurs());

        destProperty = destPrimitiveArray;
        }
    else if (sourceProperty->GetIsStruct())
        {
        auto* destStruct = new StructECProperty(*this);
        StructECPropertyCP sourceStruct = sourceProperty->GetAsStructProperty();
        ECStructClassCP structClass = &sourceStruct->GetType();

        ECClassP target;
        if (ECObjectsStatus::Success != (status = GetSchemaR().GetOrCopyReferencedClassForCopy(sourceProperty->GetClass(), target, structClass, copyReferences)))
            {
            delete(destStruct);
            return status;
            }

        ECStructClassP targetStruct = target->GetStructClassP();
        if (nullptr == targetStruct)
            {
            LOG.errorv("Could not copy struct property '%s.%s' to '%s' because the class '%s' found in the target schema '%s' is not a struct class",
                sourceProperty->GetClass().GetFullName(), sourceProperty->GetName().c_str(), GetFullName(), structClass->GetFullName(), target->GetSchema().GetFullSchemaName().c_str());
            delete(destStruct);
            return ECObjectsStatus::ClassTypeNotCorrect;
            }

        if (ECObjectsStatus::Success != (status = destStruct->SetType(*targetStruct)))
            {
            delete(destStruct);
            return status;
            }

        destProperty = destStruct;
        }
    else if (sourceProperty->GetIsNavigation())
        {
        auto* destNav = new NavigationECProperty(*this);
        NavigationECPropertyCP sourceNav = sourceProperty->GetAsNavigationProperty();

        ECRelationshipClassCP relationshipClass = sourceNav->GetRelationshipClass();

        ECClassP target;
        if (ECObjectsStatus::Success != (status = GetSchemaR().GetOrCopyReferencedClassForCopy(sourceProperty->GetClass(), target, relationshipClass, copyReferences)))
            {
            delete(destNav);
            return status;
            }

        ECRelationshipClassP targetRelationship = target->GetRelationshipClassP();
        if (nullptr == targetRelationship)
            {
            LOG.errorv("Could not copy navigation property '%s.%s' to '%s' because the class '%s' found in the target schema '%s' is not a relationship class",
                sourceProperty->GetClass().GetFullName(), sourceProperty->GetName().c_str(), GetFullName(), relationshipClass->GetFullName(), target->GetSchema().GetFullSchemaName().c_str());
            delete(destNav);
            return ECObjectsStatus::ClassTypeNotCorrect;
            }

        // TODO: for now do not verify because there is an issue with copying navigation property
        if (ECObjectsStatus::Success != (status = destNav->SetRelationshipClass(*targetRelationship, sourceNav->GetDirection(), false)))
            {
            delete(destNav);
            return status;
            }

        destProperty = destNav;
        }

    if (ECObjectsStatus::Success != (status = destProperty->SetName(sourceProperty->GetName())))
        return status;
    if (ECObjectsStatus::Success != (status = destProperty->SetDescription(sourceProperty->GetInvariantDescription())))
        return status;
    if (sourceProperty->GetIsDisplayLabelDefined() && ECObjectsStatus::Success != (status = destProperty->SetDisplayLabel(sourceProperty->GetInvariantDisplayLabel())))
        return status;
    if (ECObjectsStatus::Success != (status = destProperty->SetIsReadOnly(sourceProperty->IsReadOnlyFlagSet())))
        return status;
    if (sourceProperty->IsPriorityLocallyDefined() && ECObjectsStatus::Success != (status = destProperty->SetPriority(sourceProperty->GetPriority())))
        return status;

    if (sourceProperty->IsCategoryDefinedLocally())
        {
        PropertyCategoryCP sourcePropCategory = sourceProperty->GetCategory();
        PropertyCategoryP targetCategory;
        status = GetSchemaR().GetOrCopyReferencedPropertyCategoryForCopy(sourceProperty->GetClass(), targetCategory, sourcePropCategory, copyReferences);
        if (ECObjectsStatus::Success != status)
            return status;

        if (ECObjectsStatus::Success != (status = destProperty->SetCategory(targetCategory)))
            return status;
        }

    if (sourceProperty->IsKindOfQuantityDefinedLocally())
        {
        KindOfQuantityCP sourceKoq = sourceProperty->GetKindOfQuantity();
        KindOfQuantityP targetKoq;
        status = GetSchemaR().GetOrCopyReferencedKindOfQuantityForCopy(sourceProperty->GetClass(), targetKoq, sourceKoq, copyReferences);
        if (ECObjectsStatus::Success != status)
            return status;

        if (ECObjectsStatus::Success != (status = destProperty->SetKindOfQuantity(targetKoq)))
            return status;
        }

    if (copyCustomAttributes && ECObjectsStatus::Success != (status = sourceProperty->CopyCustomAttributesTo(*destProperty, copyReferences)))
        return status;

    if (andAddProperty && ECObjectsStatus::Success != (status = AddProperty(destProperty, Utf8String(destPropertyName))))
        delete destProperty;

    return status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus ECClass::CopyPropertyForSupplementation(ECPropertyP& destProperty, ECPropertyCP sourceProperty, bool copyCustomAttributes)
    {
    ECObjectsStatus status = CopyProperty(destProperty, sourceProperty, copyCustomAttributes);
    if (ECObjectsStatus::Success == status)
        destProperty->m_flags.forSupplementation = true;

    return status;
    }
//---------------------------------------------------------------------------------------
// @bsimethod
//---------------+---------------+---------------+---------------+---------------+-------
ECPropertyP ECClass::GetPropertyP(WCharCP propertyName, bool includeBaseClasses) const
    {
    Utf8String propName;
    BeStringUtilities::WCharToUtf8(propName, propertyName);
    return GetPropertyP(propName.c_str(), includeBaseClasses);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ECPropertyP ECClass::GetBaseClassPropertyP (Utf8CP propertyName) const
    {
    for (const ECClassP& baseClass : m_baseClasses)
        {
        ECPropertyP baseProperty = baseClass->GetPropertyP (propertyName);
        if (nullptr != baseProperty)
            return baseProperty;
        }

    return nullptr;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ECPropertyP ECClass::GetPropertyP(Utf8CP name, bool includeBaseClasses) const
    {
    PropertyMap::const_iterator found = m_propertyMap.find(name);
    if (m_propertyMap.end() != found)
        return found->second;
    else
        return includeBaseClasses ? GetBaseClassPropertyP(name) : nullptr;
    }

static const Utf8CP s_schemasThatAllowOverridingArrays[] =
    {
    "jclass.01",
    "jclass.02",
    "jclass.03",
    "ECXA_ams.01",
    "ECXA_ams_user.01",
    "ams.01",
    "ams_lcs.01",
    "ams_proj.01",
    "ams_user.01",
    "Bentley_JSpace_CustomAttributes.02",
    "Bentley_Plant.06",
    "speedikon.01"
    };

static const size_t s_numSchemasThatAllowOverridingArrays = 12;

/*---------------------------------------------------------------------------------**//**
From .NET implementation:
///<summary>
///     Fixing defect D-33626 (Overriding a array property with a non-array property of the same type is allowed)
///     caused application breaks because some delivered schemas contained errors (non-array types were overriding
///     array types or vice-versa). So these schemas are included in an exception list and the
///     exception for overriding non-array type with array types is not thrown for these schemas.
///     Schemas that are part of this list are hard-coded
///
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool ECClass::SchemaAllowsOverridingArrays(ECSchemaCP schema)
    {
    Utf8Char buf[1024];
    BeStringUtilities::Snprintf(buf, "%s.%02d", schema->GetName().c_str(), schema->GetVersionRead());
    for (size_t i = 0; i < s_numSchemasThatAllowOverridingArrays; i++)
        if (0 == strcmp(s_schemasThatAllowOverridingArrays[i], buf))
            return true;

    return false;
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------+---------------+---------------+---------------+---------------+-------
void ECClass::OnBaseClassPropertyChanged(ECPropertyCR baseProperty, ECPropertyCP newBaseProperty)
    {
    InvalidateDefaultStandaloneEnabler();

    ECPropertyP derivedProperty = GetPropertyP(baseProperty.GetName(), false);
    if (nullptr != derivedProperty)
        if (ECObjectsStatus::Success != derivedProperty->SetBaseProperty(newBaseProperty))
            derivedProperty->SetBaseProperty(nullptr); // see comments in SetBaseProperty()

    for (ECClassP derivedClass : m_derivedClasses)
        derivedClass->OnBaseClassPropertyRemoved(baseProperty);
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------+---------------+---------------+---------------+---------------+-------
// static
bool ECClass::ConvertPropertyToPrimitiveArray(ECClassP ecClass, ECClassCP startingClass, Utf8String propName, bool includeDerivedClasses)
    {
    if (includeDerivedClasses && ecClass->HasDerivedClasses())
        {
        for (ECClassP derivedClass : ecClass->GetDerivedClasses())
            {
            if (ECClass::ClassesAreEqualByName(derivedClass, startingClass))
                continue;

            if (!ConvertPropertyToPrimitiveArray(derivedClass, ecClass, propName, includeDerivedClasses))
                return false;
            }
        }

    if (ecClass->HasBaseClasses())
        {
        for (ECClassP baseClass : ecClass->GetBaseClasses())
            {
            if (ECClass::ClassesAreEqualByName(baseClass, startingClass))
                continue;

            if (!ConvertPropertyToPrimitiveArray(baseClass, ecClass, propName))
                return false;
            }
        }

    ECPropertyP ecProp = ecClass->GetPropertyP(propName, false);
    if (nullptr == ecProp)
        return true;

    // Check if the property is already a primitiveArrayProperty
    PrimitiveArrayECPropertyCP arrProp = ecProp->GetAsPrimitiveArrayProperty();
    if (nullptr != arrProp)
        return true;

    PrimitiveECPropertyCP primProp = ecProp->GetAsPrimitiveProperty();
    if (nullptr == primProp)
        return false;

    uint32_t propertyIndex = -1;
    for (size_t i = 0; i < ecClass->m_propertyList.size(); i++)
        {
        if (ecClass->m_propertyList[i] == ecProp)
            {
            propertyIndex = (uint32_t) i;
            break;
            }
        }

    if (-1 == propertyIndex)
        return true;

    PrimitiveArrayECPropertyP newProperty = new PrimitiveArrayECProperty(*ecClass);
    newProperty->SetName(primProp->GetName());
    newProperty->SetPrimitiveElementType(primProp->GetType());
    newProperty->SetDescription(primProp->GetInvariantDescription());
    if (primProp->GetIsDisplayLabelDefined())
        newProperty->SetDisplayLabel(primProp->GetInvariantDisplayLabel());
    newProperty->SetIsReadOnly(primProp->IsReadOnlyFlagSet());

    // doesn't matter if we pass true or false for copyReferences param, because we are copying to the same class
    ECObjectsStatus status = primProp->CopyCustomAttributesTo(*newProperty, false);
    if (ECObjectsStatus::Success != status)
        {
        LOG.errorv("Failed to convert the property, %s.%s, to a primitive array property because could not copy all original custom attributes.",
                   primProp->GetClass().GetFullName(), primProp->GetName().c_str());
        return false;
        }

    if (primProp->IsKindOfQuantityDefinedLocally())
        newProperty->SetKindOfQuantity(primProp->GetKindOfQuantity());
    if (primProp->IsExtendedTypeDefinedLocally())
        {
        Utf8String extendTypeName = primProp->GetExtendedTypeName();
        status = newProperty->SetExtendedTypeName(extendTypeName.c_str());
        if (ECObjectsStatus::Success != status)
            {
            LOG.errorv("Failed to convert the property, %s.%s, to a primitive array property because could not set the extended type name, %s, on the primitive array property.",
                       primProp->GetClass().GetFullName(), primProp->GetName().c_str(), extendTypeName.c_str());
            delete newProperty;
            return false;
            }
        }

    ecClass->m_propertyMap.erase(ecClass->m_propertyMap.find(ecProp->GetName().c_str()));
    ecClass->InvalidateDefaultStandaloneEnabler();

    ecClass->m_propertyMap[newProperty->GetName().c_str()] = newProperty;
    ecClass->m_propertyList[propertyIndex] = newProperty;


    for (ECClassP derivedClass : ecClass->GetDerivedClasses())
        derivedClass->OnBaseClassPropertyChanged(*ecProp, newProperty);

    if (ECObjectsStatus::Success != status)
        {
        delete newProperty;
        return false;
        }

    delete primProp;

    for (ECClassP derivedClass : ecClass->GetDerivedClasses())
        ConvertPropertyToPrimitiveArray(derivedClass, ecClass, propName, true);

    return true;
    }

/*---------------------------------------------------------------------------------**//**
///<summary>
///     Only to be used to fix a handful of schemas that have primitive array
///     properties override primitive properties. Details about this can be found in
///     the summary of ECClass::SchemaAllowsOverridingArrays.
///
///     All base primitive properties that are overriden by a primitive array property,
///     or vice versa, will be converted to primitive array properties. If the conversion
///     fails the schema will fail to deserialize.
 @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus ECClass::FixArrayPropertyOverrides()
    {
    if (!SchemaAllowsOverridingArrays(&this->GetSchema()))
        return ECObjectsStatus::Error;

    for (ECPropertyP ecProp : GetProperties(false))
        {
        if (nullptr == ecProp->GetBaseProperty() || !(ecProp->GetIsPrimitiveArray() || ecProp->GetIsPrimitive()))
            continue;

        Utf8String propName = ecProp->GetName();
        for (ECClassP baseClass : GetBaseClasses())
            {
            ECPropertyP baseProperty = baseClass->GetPropertyP(propName, true);
            if (nullptr == baseProperty)
                continue;

            if (ecProp->GetIsPrimitive() != baseProperty->GetIsPrimitive() || ecProp->GetIsPrimitiveArray() != baseProperty->GetIsPrimitiveArray())
                {
                if (!ConvertPropertyToPrimitiveArray(this, this, propName))
                    return ECObjectsStatus::Error;
                break;
                }
            }
        }

    return ECObjectsStatus::Success;
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------+---------------+---------------+---------------+---------------+-------
ECObjectsStatus ECClass::CanPropertyBeOverridden (ECPropertyCR baseProperty, ECPropertyCR newProperty, Utf8StringR errMsg) const
    {
    // If the type of base property is an array and the type of the current property is not an array (or vice-versa),
    // return an error immediately.  Unfortunately, there are a class of schemas that have been delivered with this type
    // of override.  So need to check if this is one of those schemas before returning an error
    if ((baseProperty.GetIsArray() && !newProperty.GetIsArray()) || (!baseProperty.GetIsArray() && newProperty.GetIsArray()))
        {
        if (m_schema.OriginalECXmlVersionAtLeast(ECVersion::V3_1) || !SchemaAllowsOverridingArrays(&this->GetSchema()))
            {
            LOG.errorv("The property %s:%s cannot override %s:%s because an array property cannot override a non-array property or vice-versa.",
                       newProperty.GetClass().GetFullName(), newProperty.GetName().c_str(), baseProperty.GetClass().GetFullName(), baseProperty.GetName().c_str());
            return ECObjectsStatus::InvalidPrimitiveOverrride;
            }
        }

    if (!newProperty._CanOverride(baseProperty, errMsg))
        return ECObjectsStatus::DataTypeMismatch;

    return ECObjectsStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus ECClass::RemoveProperty (Utf8StringCR name)
    {
    PropertyMap::iterator  propertyIterator = m_propertyMap.find (name.c_str());

    if (propertyIterator == m_propertyMap.end())
        return ECObjectsStatus::PropertyNotFound;

    ECPropertyP ecProperty = propertyIterator->second;
    return DeleteProperty (*ecProperty);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus ECClass::AddProperty (ECPropertyP ecProperty, Utf8StringCR name, bool resolveConflicts)
    {
    ECObjectsStatus status = ecProperty->SetName (name);
    if (ECObjectsStatus::Success != status)
        return status;

    return AddProperty (ecProperty, resolveConflicts);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus ECClass::CreatePrimitiveProperty (PrimitiveECPropertyP &ecProperty, Utf8StringCR name, PrimitiveType primitiveType, bool resolveConflicts)
    {
    ecProperty = new PrimitiveECProperty(*this);
    ecProperty->SetType(primitiveType);
    ECObjectsStatus status = AddProperty(ecProperty, name, resolveConflicts);
    if (status != ECObjectsStatus::Success)
        {
        delete ecProperty;
        ecProperty = NULL;
        return status;
        }
    return ECObjectsStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
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
* @bsimethod
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
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus ECClass::CreatePrimitiveArrayProperty (PrimitiveArrayECPropertyP &ecProperty, Utf8StringCR name)
    {
    ecProperty = new PrimitiveArrayECProperty(*this);
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
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus ECClass::CreatePrimitiveArrayProperty (PrimitiveArrayECPropertyP &ecProperty, Utf8StringCR name, PrimitiveType primitiveType)
    {
    ecProperty = new PrimitiveArrayECProperty(*this);
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

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------+---------------+---------------+---------------+---------------+-------
ECObjectsStatus ECClass::CreatePrimitiveArrayProperty(PrimitiveArrayECPropertyP& ecProperty, Utf8StringCR name, ECEnumerationCR enumerationType)
    {
    ecProperty = new PrimitiveArrayECProperty(*this);
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
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus ECClass::CreateStructArrayProperty (StructArrayECPropertyP &ecProperty, Utf8StringCR name, ECStructClassCR structType)
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
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void ECClass::RemoveDerivedClass (ECClassCR derivedClass) const
    {
    ECDerivedClassesList::iterator derivedClassIterator;

    for (derivedClassIterator = m_derivedClasses.begin(); derivedClassIterator != m_derivedClasses.end(); derivedClassIterator++)
        {
        if (*derivedClassIterator == (ECClassCP)&derivedClass)
            {
            m_derivedClasses.erase(derivedClassIterator);
            return;
            }
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void ECClass::RemoveDerivedClasses ()
    {
    for (ECDerivedClassesList::iterator iter = m_derivedClasses.end(); iter != m_derivedClasses.begin(); )
        (*--iter)->RemoveBaseClass (*this);

    m_derivedClasses.clear ();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool ECClass::CheckBaseClassCycles (ECClassCP thisClass, const void * arg)
    {
    ECClassCP proposedParent = static_cast<ECClassCP>(arg);
    if (nullptr == proposedParent)
        return true;

    if (thisClass == proposedParent || ClassesAreEqualByName(thisClass, arg))
        return true;
    return false;
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------+---------------+---------------+---------------+---------------+-------
void ECClass::CollectAllBaseClasses(bmap<Utf8String, ECClassP>& baseClasses, ECClassP ecClass)
    {
    baseClasses.insert(bpair<Utf8String, ECClassP>(Utf8String(ecClass->GetFullName()), ecClass));
    for (ECClassP baseClass : ecClass->GetBaseClasses())
        {
        if (baseClasses.find(baseClass->GetFullName()) != baseClasses.end())
            continue;
        CollectAllBaseClasses(baseClasses, baseClass);
        }
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

        IECInstancePtr notSubClassableCA = baseClass.GetCustomAttributeLocal("CoreCustomAttributes", "NotSubclassableInReferencingSchemas");
        if (notSubClassableCA.IsValid())
            {
            bool exceptionFound = false;
            uint32_t exceptionsIndex;
            notSubClassableCA->GetEnabler().GetPropertyIndex(exceptionsIndex, "Exceptions");
            ECValue exceptionsArray;
            if (ECObjectsStatus::Success == notSubClassableCA->GetValue(exceptionsArray, exceptionsIndex))
                {
                Utf8StringCR fullName = m_fullName.GetName(*this);
                ArrayInfo arrayInfo = exceptionsArray.GetArrayInfo();
                for (uint32_t i = 0; i < arrayInfo.GetCount(); ++i)
                    {
                    ECValue exception;
                    if (ECObjectsStatus::Success != notSubClassableCA->GetValue(exception, exceptionsIndex, i) || exception.IsNull())
                        continue;

                    if (fullName.EqualsIAscii(exception.GetUtf8CP()))
                        {
                        exceptionFound = true;
                        break;
                        }
                    }
                }

            if (!exceptionFound)
                {
                LOG.errorv("Cannot add class '%s' as a base class to '%s' because the base class has the 'NotSubclassableInReferencingSchema' CA, this class is in a referencing schema and is not listed as an exception",
                    baseClass.GetFullName(), GetFullName());
                return ECObjectsStatus::BaseClassUnacceptable;
                }
            }
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
        if (*baseClassIterator == (ECClassCP) &baseClass)
            {
            LOG.infov("Cannot add class '%s' as a base class to '%s' because it already exists as a base class", baseClass.GetFullName(), GetFullName());
            return ECObjectsStatus::NamedItemAlreadyExists;
            }
        }

    PropertyList baseClassProperties;
    ECObjectsStatus status = baseClass.GetProperties(true, &baseClassProperties);
    if (ECObjectsStatus::Success != status)
        return status;

    bmap<Utf8String, ECClassP> baseClasses;
    CollectAllBaseClasses(baseClasses, this);
    for (auto it : baseClasses)
        {
        ECClassP testClass = it.second;
        if (testClass->GetPropertyCount(false) == 0)
            continue;
        for (ECPropertyP prop : baseClassProperties)
            {
            // This is a case-insensitive search
            ECPropertyP testProp = testClass->GetPropertyP(prop->GetName(), false);
            if (nullptr != testProp)
                {
                if (ECObjectsStatus::Success != (status = testClass->ValidateBaseProperty(*prop, testProp, resolveConflicts)))
                    {
                    LOG.errorv("Failed to add base class because base property %s:%s not compatible with %s:%s",
                        prop->GetClass().GetFullName(), prop->GetName().c_str(), testClass->GetFullName(), testProp->GetName().c_str());
                    return status;
                    }
                }
            }
        }

    // If the base class is a stub the checks will be done when filling out the stub using the derived class pointers
    if (!insertAtBeginning)
        m_baseClasses.push_back(const_cast<ECClassP>(&baseClass));
    else
        m_baseClasses.insert(m_baseClasses.begin(), const_cast<ECClassP>(&baseClass));

    InvalidateDefaultStandaloneEnabler();

    for (ECPropertyP baseProperty : baseClass.GetProperties())
        status = OnBaseClassPropertyAdded(*baseProperty, resolveConflicts);
    
    if (ECObjectsStatus::Success != status)
        {
        RemoveBaseClass(baseClass);
        return status;
        }

    baseClass.AddDerivedClass(*this);

    return ECObjectsStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsistruct
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
* @bsimethod
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
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus ECClass::_RemoveBaseClass(ECClassCR baseClass)
    {
    bool baseClassRemoved = false;

    ECBaseClassesList::iterator baseClassIterator;
    for (baseClassIterator = m_baseClasses.begin(); baseClassIterator != m_baseClasses.end(); baseClassIterator++)
        {
        if (*baseClassIterator == (ECClassCP)&baseClass)
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
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void ECClass::RemoveBaseClasses()
    {
    bvector<ECClassP> classes;
    for (ECBaseClassesList::iterator iter = m_baseClasses.begin(); iter != m_baseClasses.end(); iter++)
        classes.push_back(*iter);

    for (auto& baseClass : classes)
        RemoveBaseClass(*baseClass);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool ECClass::Is(Utf8CP schemaname, Utf8CP classname) const
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
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool ECClass::Is(ECClassCP targetClass) const
    {
    if (nullptr == targetClass)
        return false;

    if (ClassesAreEqualByName(this, targetClass))
        return true;

    return TraverseBaseClasses(&ClassesAreEqualByName, true, targetClass);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
// static
bool ECClass::ClassesAreEqualByName(ECClassCP thisClass, const void * arg)
    {
    ECClassCP thatClass = static_cast<ECClassCP> (arg);
    if (nullptr == arg)
        return true;

    return ((thisClass == thatClass) ||
            ( (0 == thisClass->GetName().compare(thatClass->GetName())) &&
              (0 == thisClass->GetSchema().GetName().compare(thatClass->GetSchema().GetName())) &&
              (thisClass->GetSchema().GetVersionRead() == thatClass->GetSchema().GetVersionRead()) &&
              (thisClass->GetSchema().GetVersionWrite() == thatClass->GetSchema().GetVersionWrite()) &&
              (thisClass->GetSchema().GetVersionMinor() == thatClass->GetSchema().GetVersionMinor())));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ECPropertyIterable ECClass::GetProperties() const
    {
    return ECPropertyIterable(*this, true);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ECPropertyIterable ECClass::GetProperties(bool includeBaseProperties) const
    {
    return ECPropertyIterable(*this, includeBaseProperties);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static bool containsProperty(Utf8CP name, PropertyList const& props)
    {
    return props.end() != std::find_if (props.begin(), props.end(), [&name](ECPropertyP const& prop)
        {
        return prop->GetName().EqualsIAscii (name);
        });
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus ECClass::GetProperties(bool includeBaseProperties, PropertyList* propertyList) const {
    BeMutexHolder holder(GetSchema().GetMutex());
    if (includeBaseProperties && m_cachedProperties != nullptr) {
        if (propertyList) {
            propertyList->insert(propertyList->begin(), m_cachedProperties->begin(), m_cachedProperties->end());
        }
        return ECObjectsStatus::Success;
    }

    if (!includeBaseProperties || m_baseClasses.empty()) {
        if (propertyList) {
            propertyList->insert(propertyList->begin(), m_propertyList.begin(), m_propertyList.end());
        }
        return ECObjectsStatus::Success;
    }

    m_cachedProperties = std::make_unique<PropertyList>();
    m_cachedProperties->insert(m_cachedProperties->begin(), m_propertyList.begin(), m_propertyList.end());

    // replicate managed code behavior - specific ordering expected. Probably slower, but at least correct.
    PropertyList inheritedProperties;
    for (auto const& baseClass : m_baseClasses) {
        for (ECPropertyP const& baseProp : baseClass->GetProperties (true)) {
            if (!containsProperty(baseProp->GetName().c_str(), *m_cachedProperties) && !containsProperty(baseProp->GetName().c_str(), inheritedProperties))
                inheritedProperties.push_back (baseProp);
        }
    }

    // inherited properties come before this class's properties
    m_cachedProperties->reserve(m_cachedProperties->size() + inheritedProperties.size());
    m_cachedProperties->insert(m_cachedProperties->begin(), inheritedProperties.begin(), inheritedProperties.end());
    
    if (propertyList) {
        propertyList->insert(propertyList->begin(), m_cachedProperties->begin(), m_cachedProperties->end());
    }
    return ECObjectsStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
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

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
bool ECClass::Validate() const { return _Validate(); }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
SchemaReadStatus ECClass::_ReadXmlAttributes (pugi::xml_node classNode)
    {
    Utf8String value;      // used by the macros.
    if (GetName().length() == 0)
        READ_REQUIRED_XML_ATTRIBUTE (classNode, TYPE_NAME_ATTRIBUTE, this, Name, classNode.name())

    // OPTIONAL attributes - If these attributes exist they MUST be valid
    READ_OPTIONAL_XML_ATTRIBUTE (classNode, DESCRIPTION_ATTRIBUTE, this, Description)
    READ_OPTIONAL_XML_ATTRIBUTE (classNode, ECXML_DISPLAY_LABEL_ATTRIBUTE, this, DisplayLabel)

    Utf8String     modifierString;
    auto modifierAttr = classNode.attribute(MODIFIER_ATTRIBUTE);
    if (modifierAttr)
        {
        modifierString = modifierAttr.as_string();
        if (ECObjectsStatus::Success != SchemaParseUtils::ParseModifierXmlString(m_modifier, modifierString))
            {
            // Don't fail if the modifier string is unknown with >EC3.x versions. Default is None.
            if (GetSchema().OriginalECXmlVersionGreaterThan(ECVersion::Latest))
                {
                LOG.debugv("Class '%s' has an unknown modifier '%s'. Setting to None.", this->GetFullName(), modifierString.c_str());
                return SchemaReadStatus::Success;
                }

            LOG.errorv("Class '%s' has an invalid modifier attribute value '%s'", this->GetFullName(), modifierString.c_str());
            return SchemaReadStatus::InvalidECSchemaXml;
            }
        }
    // Modifier is required on ECRelationshipClasses in ecxml versions 3.1 and greater
    else if (IsRelationshipClass() && m_schema.OriginalECXmlVersionAtLeast (ECVersion::V3_1))
        {
        LOG.errorv("Invalid ECSchemaXML: The ECRelationshipClass '%s' must contain a '%s' attribute", this->GetFullName(), MODIFIER_ATTRIBUTE);
        return SchemaReadStatus::InvalidECSchemaXml;
        }

    return SchemaReadStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
SchemaReadStatus ECClass::_ReadXmlContents (pugi::xml_node classNode, ECSchemaReadContextR context, ECSchemaCP conversionSchema, bvector<NavigationECPropertyP>& navigationProperties)
    {
    bool isSchemaSupplemental = Utf8String::npos != GetSchema().GetName().find("_Supplemental_");
    // Get the BaseClass child nodes.
    for (auto childNode : classNode.children())
        {
        if (childNode.type() != pugi::xml_node_type::node_element)
            continue;

        Utf8CP childNodeName = childNode.name();
        if (0 == strcmp (childNodeName, EC_PROPERTY_ELEMENT))
            {
            PrimitiveECPropertyP ecProperty = new PrimitiveECProperty(*this);
            SchemaReadStatus status = _ReadPropertyFromXmlAndAddToClass (ecProperty, childNode, context, conversionSchema, childNodeName);
            if (SchemaReadStatus::Success != status)
                return status;
            }
        else if (!isSchemaSupplemental && (0 == strcmp (childNodeName, ECXML_BASE_CLASS_ELEMENT)))
            {
            SchemaReadStatus status = _ReadBaseClassFromXml(childNode, context, conversionSchema);
            if (SchemaReadStatus::Success != status)
                return status;
            }
        else if (0 == strcmp (childNodeName, EC_ARRAYPROPERTY_ELEMENT))
            {
            ECPropertyP ecProperty;
            if (m_schema.OriginalECXmlVersionLessThan(ECVersion::V3_0))
                {
                Utf8String boolStr;
                bool isStruct = false;
                Utf8String typeName;
                // Ignore the isStruct attribute since it is irrelevant.  If there is a typeName and it is resolvable as a struct, then it is a struct array.  Otherwise, it isn't.  There are invalid
                // schemas out there that claim to be a StructArray but either use an unresolvable typeName or actually use a primitive type.
                auto typeNameAttr = childNode.attribute(TYPE_NAME_ATTRIBUTE);
                if (typeNameAttr)
                    {
                    typeName = typeNameAttr.as_string();
                    ECStructClassCP structClass;
                    ECObjectsStatus status = ResolveStructType(structClass, typeName, *this, false);
                    if (ECObjectsStatus::Success == status && NULL != structClass)
                        {
                        if (structClass == this)
                            {
                            LOG.warningv("Invalid ECSchemaXML: The StructECClass '%s' contains an array element whose type is the struct itself. This is no longer permitted and the property is being dropped.",
                                       GetName().c_str());
                            continue;
                            }
                        isStruct = true;
                        }
                    }
                if (isStruct)
                    ecProperty = new StructArrayECProperty(*this);
                else
                    ecProperty = new PrimitiveArrayECProperty(*this);
                }
            else
                ecProperty = new PrimitiveArrayECProperty(*this);

            SchemaReadStatus status = _ReadPropertyFromXmlAndAddToClass (ecProperty, childNode, context, conversionSchema, childNodeName);
            if (SchemaReadStatus::Success != status)
                return status;
            }
        else if (0 == strcmp(childNodeName, EC_STRUCTARRAYPROPERTY_ELEMENT)) // technically, this only happens in EC3.0 and higher, but no harm in checking 2.0 schemas
            {
            ECPropertyP ecProperty = new StructArrayECProperty(*this);
            SchemaReadStatus status = _ReadPropertyFromXmlAndAddToClass(ecProperty, childNode, context, conversionSchema, childNodeName);
            if (SchemaReadStatus::Success != status)
                return status;
            }
        else if (0 == strcmp (childNodeName, EC_STRUCTPROPERTY_ELEMENT))
            {
            ECPropertyP ecProperty = new StructECProperty (*this);
            SchemaReadStatus status = _ReadPropertyFromXmlAndAddToClass (ecProperty, childNode, context, conversionSchema, childNodeName);
            if (SchemaReadStatus::Success != status)
                return status;
            }
        else if (0 == strcmp(childNodeName, EC_NAVIGATIONPROPERTY_ELEMENT)) // also EC3.0 only
            {
            NavigationECPropertyP ecProperty = new NavigationECProperty(*this);
            SchemaReadStatus status = _ReadPropertyFromXmlAndAddToClass(ecProperty, childNode, context, conversionSchema, childNodeName);
            if (SchemaReadStatus::Success != status)
                return status;
            navigationProperties.push_back(ecProperty);
            }
        }

    return SchemaReadStatus::Success;
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------+---------------+---------------+---------------+---------------+-------
SchemaReadStatus ECClass::_ReadBaseClassFromXml (pugi::xml_node childNode, ECSchemaReadContextR context, ECSchemaCP conversionSchema)
    {
    Utf8String qualifiedClassName = childNode.text().as_string();
    
    // Parse the potentially qualified class name into an alias and short class name
    Utf8String alias;
    Utf8String className;
    if (ECObjectsStatus::Success != ECClass::ParseClassName (alias, className, qualifiedClassName))
        {
        LOG.errorv ("Invalid ECSchemaXML: The ECClass '%s' contains a %s element with the value '%s' that can not be parsed.",
            GetName().c_str(), ECXML_BASE_CLASS_ELEMENT, qualifiedClassName.c_str());

        return SchemaReadStatus::InvalidECSchemaXml;
        }

    if (context.IsAliasToPrune(GetSchema().GetName(), alias))
        {
        context.Issues().ReportV(IssueSeverity::Info, IssueCategory::BusinessProperties, IssueType::ECClass,
            "Dropping base class '%s' from '%s' because the base class is defined in a schema set to be pruned in the schema read context.", qualifiedClassName.c_str(), GetFullName());
        return SchemaReadStatus::Success;
        }

    ECSchemaCP resolvedSchema = GetSchema().GetSchemaByAliasP (alias);
    if (nullptr == resolvedSchema)
        {
        LOG.errorv("Invalid ECSchemaXML: The ECClass '%s' contains a %s element with the alias '%s' that can not be resolved to a referenced schema.",
            GetName().c_str(), ECXML_BASE_CLASS_ELEMENT, alias.c_str());
        return SchemaReadStatus::InvalidECSchemaXml;
        }

    context.ResolveClassName (className, *resolvedSchema);
    ECClassCP baseClass = resolvedSchema->GetClassCP (className.c_str());
    if (NULL == baseClass)
        {
        LOG.errorv("Invalid ECSchemaXML: The ECClass '%s' contains a %s element with the value '%s' that can not be resolved to an ECClass named '%s' in the ECSchema '%s'",
            GetName ().c_str (), ECXML_BASE_CLASS_ELEMENT, qualifiedClassName.c_str (), className.c_str (), resolvedSchema->GetName ().c_str ());
        return SchemaReadStatus::InvalidECSchemaXml;
        }

    bool resolveConflicts = m_schema.OriginalECXmlVersionLessThan(ECVersion::V3_0) || context.ResolveConflicts();
    if (nullptr != conversionSchema)
        {
        ECClassCP conversionClass = conversionSchema->GetClassCP(GetName().c_str());
        if (nullptr != conversionClass)
            {
            IECInstancePtr ca = conversionClass->GetCustomAttribute("IgnoreBaseClass");
            if (ca.IsValid())
                {
                ECValue except;
                ca->GetValue(except, "Except");
                if (except.IsNull() || Utf8String::IsNullOrEmpty(except.GetUtf8CP()))
                    return SchemaReadStatus::Success;
                bvector<Utf8String> baseNames;
                BeStringUtilities::Split(except.GetUtf8CP(), ";", baseNames);
                bool addThisBaseClass = false;
                for (Utf8String baseName : baseNames)
                    if (baseName.Equals(baseClass->GetFullName()))
                        {
                        addThisBaseClass = true;
                        break;
                        }
                if (!addThisBaseClass)
                    return SchemaReadStatus::Success;
                }
            }
        if (conversionSchema->IsDefined("ResolvePropertyNameConflicts"))
            resolveConflicts = true;
        }

    ECObjectsStatus stat;
    if (ECObjectsStatus::Success != (stat = _AddBaseClass(*baseClass, false, resolveConflicts, false)))
        {
        if (stat == ECObjectsStatus::BaseClassUnacceptable)
            {
            if (resolveConflicts)
                {
                LOG.debugv("Invalid ECSchemaXML: The ECClass '%s:%s' (%d) has an invalid base class '%s:%s' (%d) but their types differ.  The base class will not be added.",
                           GetSchema().GetFullSchemaName().c_str(), GetName().c_str(), GetClassType(),
                           baseClass->GetSchema().GetFullSchemaName().c_str(), baseClass->GetName().c_str(), baseClass->GetClassType());
                }
            else
                {
                LOG.errorv("Invalid ECSchemaXML: The ECClass '%s:%s' (%d) has an invalid base class '%s:%s' (%d) because their types differ or the base class is sealed.",
                           GetSchema().GetFullSchemaName().c_str(), GetName().c_str(), GetClassType(),
                           baseClass->GetSchema().GetFullSchemaName().c_str(), baseClass->GetName().c_str(), baseClass->GetClassType());
                return SchemaReadStatus::InvalidECSchemaXml;
                }
            }
        else
            {
            LOG.errorv("Invalid ECSchemaXML: Unable to add ECClass '%s:%s' as a base class to ECClass '%s:%s'",
                       baseClass->GetSchema().GetFullSchemaName().c_str(), baseClass->GetName().c_str(),
                       GetSchema().GetFullSchemaName().c_str(), GetName().c_str());
            return SchemaReadStatus::InvalidECSchemaXml;
            }
        }
    return SchemaReadStatus::Success;
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------+---------------+---------------+---------------+---------------+-------
SchemaReadStatus ECClass::_ReadPropertyFromXmlAndAddToClass( ECPropertyP ecProperty, pugi::xml_node childNode, ECSchemaReadContextR context, ECSchemaCP conversionSchema, Utf8CP childNodeName)
    {
    // read the property data.
    SchemaReadStatus status = ecProperty->_ReadXml(childNode, context);
    if (status == SchemaReadStatus::PruneItem)
        {
        LOG.infov("Dropping ECProperty '%s' from class '%s' because its type comes from a referenced schema that is being pruned.", ecProperty->GetName().c_str(), ecProperty->GetClass().GetFullName());
        delete ecProperty;
        return SchemaReadStatus::Success;
        }
    else if (status != SchemaReadStatus::Success)
        {
        LOG.errorv("Invalid ECSchemaXML: Failed to read properties of ECClass '%s:%s'", this->GetSchema().GetName().c_str(), this->GetName().c_str());
        delete ecProperty;
        return status;
        }

    bool resolveConflicts = m_schema.OriginalECXmlVersionLessThan(ECVersion::V3_0) || context.ResolveConflicts();
    if (nullptr != conversionSchema)
        {
        if (conversionSchema->IsDefined("ResolvePropertyNameConflicts"))
            resolveConflicts = true;
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

    ECObjectsStatus ecStatus;
    if (ECObjectsStatus::Success != (ecStatus = this->AddProperty (ecProperty, resolveConflicts)))
        {
        delete ecProperty;
        return SchemaReadStatus::InvalidECSchemaXml;
        }

    return SchemaReadStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
SchemaWriteStatus ECClass::_WriteXml (BeXmlWriterR xmlWriter, ECVersion ecXmlVersion, Utf8CP elementName, bmap<Utf8CP, Utf8CP>* additionalAttributes, bool doElementEnd) const
    {
    SchemaWriteStatus status = SchemaWriteStatus::Success;

    xmlWriter.WriteElementStart(elementName);

    xmlWriter.WriteAttribute(TYPE_NAME_ATTRIBUTE, this->GetName().c_str());
    xmlWriter.WriteAttribute(DESCRIPTION_ATTRIBUTE, this->GetInvariantDescription().c_str());
    if (GetIsDisplayLabelDefined())
        xmlWriter.WriteAttribute(ECXML_DISPLAY_LABEL_ATTRIBUTE, this->GetInvariantDisplayLabel().c_str());

    if (ECVersion::V2_0 == ecXmlVersion)
        {
        xmlWriter.WriteAttribute(IS_STRUCT_ATTRIBUTE, IsStructClass());
        xmlWriter.WriteAttribute(IS_CUSTOMATTRIBUTE_ATTRIBUTE, IsCustomAttributeClass());
        bool isConcrete = this->GetClassModifier() != ECClassModifier::Abstract;
        xmlWriter.WriteAttribute(IS_DOMAINCLASS_ATTRIBUTE, isConcrete && !(IsStructClass() || IsCustomAttributeClass()));
        }
    else if (m_modifier != ECClassModifier::None || IsRelationshipClass())
        xmlWriter.WriteAttribute(MODIFIER_ATTRIBUTE, SchemaParseUtils::ModifierToString(m_modifier));

    if (nullptr != additionalAttributes)
        {
        for (bmap<Utf8CP, Utf8CP>::iterator iter = additionalAttributes->begin(); iter != additionalAttributes->end(); ++iter)
            xmlWriter.WriteAttribute(iter->first, iter->second);
        }

    for (const ECClassP& baseClass: m_baseClasses)
        {
        xmlWriter.WriteElementStart(ECXML_BASE_CLASS_ELEMENT);
        xmlWriter.WriteText((ECClass::GetQualifiedClassName(GetSchema(), *baseClass)).c_str());
        xmlWriter.WriteElementEnd();
        }

    WriteCustomAttributes(xmlWriter, ecXmlVersion);

    for (ECPropertyP prop: GetProperties(false))
        {
        prop->_WriteXml (xmlWriter, ecXmlVersion);
        }

    if (doElementEnd)
        xmlWriter.WriteElementEnd();
    return status;
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------+---------------+---------------+---------------+---------------+-------
bool ECClass::_ToJson(BeJsValue outValue, bool standalone, bool includeSchemaVersion, bool includeInheritedProperties) const
    {
    return _ToJson(outValue, standalone, includeSchemaVersion, includeInheritedProperties, bvector<bpair<Utf8String, Json::Value>>());
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------+---------------+---------------+---------------+---------------+-------
bool ECClass::_ToJson(BeJsValue outValue, bool standalone, bool includeSchemaVersion, bool includeInheritedProperties, bvector<bpair<Utf8String, Json::Value>> additionalAttributes) const
    {
    // Common properties to all Schema items
    if (standalone)
        {
        outValue[ECJSON_URI_SPEC_ATTRIBUTE] = ECJSON_SCHEMA_ITEM_URI;
        outValue[ECJSON_PARENT_SCHEMA_ATTRIBUTE] = GetSchema().GetName();
        if (includeSchemaVersion)
            outValue[ECJSON_PARENT_VERSION_ATTRIBUTE] = GetSchema().GetSchemaKey().GetVersionString();
        outValue[NAME_ATTRIBUTE] = GetName();
        }

    bool isMixin = false;
    Utf8String itemType;
    if (IsEntityClass())
        {
        if (GetEntityClassCP()->IsMixin())
            {
            isMixin = true;
            itemType = ECJSON_MIXIN_ELEMENT;
            }
        else
            itemType = ECJSON_ENTITYCLASS_ELEMENT;
        }
    else if (IsRelationshipClass())
        itemType = ECJSON_RELATIONSHIP_CLASS_ELEMENT;
    else if (IsStructClass())
        itemType = ECJSON_STRUCTCLASS_ELEMENT;
    else if (IsCustomAttributeClass())
        itemType = ECJSON_CUSTOMATTRIBUTECLASS_ELEMENT;
    else
        return false;
    outValue[ECJSON_SCHEMA_ITEM_TYPE] = itemType;

    if (GetIsDisplayLabelDefined())
        outValue[ECJSON_DISPLAY_LABEL_ATTRIBUTE] = GetInvariantDisplayLabel();
    if (0 != GetInvariantDescription().length())
        outValue[DESCRIPTION_ATTRIBUTE] = GetInvariantDescription();

    // Common ECClass properties
    ECClassModifier const modifier = GetClassModifier();
    if (!isMixin && (ECClassModifier::None != modifier || IsRelationshipClass()))
        outValue[MODIFIER_ATTRIBUTE] = SchemaParseUtils::ModifierToString(modifier);

    if (HasBaseClasses() && !IsEntityClass()) // Entity class assigns base class in its _ToJson method
        {
        auto& baseClasses = GetBaseClasses();
        if (0 != baseClasses.size())
            outValue[ECJSON_BASE_CLASS_ELEMENT] = ECJsonUtilities::FormatClassName(*(baseClasses.at(0)));
        }

    if (GetPropertyCount(includeInheritedProperties))
        {
        auto propertiesArr = outValue[ECJSON_SCHEMA_ITEM_PROPERTIES_ATTRIBUTE];
        for (const auto& prop : GetProperties(includeInheritedProperties))
            {
            auto propJson = propertiesArr.appendValue();
            propJson.toObject();
            if (!prop->_ToJson(propJson, includeInheritedProperties && !ECClass::ClassesAreEqualByName(this, &prop->GetClass())))
                return false;
            }
        }

    if (IsEntityClass() && GetEntityClassCP()->IsMixin())
        {
        auto predicate = [] (Utf8CP x) -> bool { return BeStringUtilities::StricmpAscii(x, "CoreCustomAttributes:IsMixin") == 0; };
        WriteFilteredCustomAttributes(outValue, predicate);
        }
    else
        WriteCustomAttributes(outValue);

    for (auto const& attribute : additionalAttributes)
        outValue[attribute.first].From(attribute.second);

    return true;
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------+---------------+---------------+---------------+---------------+-------
SchemaWriteStatus ECClass::_WriteXml (BeXmlWriterR xmlWriter, ECVersion ecXmlVersion) const
    {
    return _WriteXml (xmlWriter, ecXmlVersion, EC_CLASS_ELEMENT, nullptr, true);
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------+---------------+---------------+---------------+---------------+-------
// static
ECObjectsStatus ECClass::ParseClassName(Utf8StringR alias, Utf8StringR className, Utf8StringCR qualifiedClassName)
    {
    return SchemaParseUtils::ParseName(alias, className, qualifiedClassName);
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------+---------------+---------------+---------------+---------------+-------
// static
Utf8String ECClass::GetQualifiedClassName(ECSchemaCR primarySchema, ECClassCR ecClass)
    {
    return SchemaParseUtils::GetQualifiedName<ECClass>(primarySchema, ecClass);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void ECClass::_GetBaseContainers(bvector<IECCustomAttributeContainerP>& returnList) const
    {
    for (ECClassP baseClass: m_baseClasses)
        returnList.push_back(baseClass);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
size_t ECClass::GetPropertyCount (bool includeBaseClasses) const
    {
    if (!includeBaseClasses || !HasBaseClasses())
        return m_propertyList.size();

    BeMutexHolder holder(GetSchema().GetMutex());
    if (m_cachedProperties == nullptr)
        GetProperties(true, nullptr);

    return m_cachedProperties->size();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void ECClass::ResetId() {
    BeMutexHolder holder(GetSchema().GetMutex());
    for (auto ecLocalProperty : GetProperties(false)) {
        ecLocalProperty->ResetId();
    }
    m_ecClassId.Invalidate();
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ECPropertyP ECClass::GetInstanceLabelProperty() const
    {
    /*
     * Note: The ugly case by case comparisions is just a way to make the instance
     * labels from legacy ECschemas (that didn't follow consistent property naming)
     * acceptable.
     */

    ECPropertyP instanceLabelProperty = nullptr;
    IECInstancePtr caInstance = this->GetCustomAttribute("InstanceLabelSpecification");
    if (caInstance.IsValid())
        {
        ECValue value;
        if (ECObjectsStatus::Success == caInstance->GetValue (value, "PropertyName") && !value.IsNull())
            {
            Utf8CP propertyName = value.GetUtf8CP();
            instanceLabelProperty = this->GetPropertyP (propertyName);
            if (nullptr != instanceLabelProperty)
                return instanceLabelProperty;
            }
        }

    Utf8String instanceLabelPropertyNames[6] =
        {"DisplayLabel", "DISPLAYLABEL", "displaylabel", "Name", "NAME", "name"};
    FOR_EACH (Utf8StringCR propName, instanceLabelPropertyNames)
        {
        instanceLabelProperty = this->GetPropertyP (propName.c_str());
        if (nullptr != instanceLabelProperty)
            return instanceLabelProperty;
        }

    return nullptr;
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------+---------------+---------------+---------------+---------------+-------
SchemaWriteStatus ECEntityClass::_WriteXml(BeXmlWriterR xmlWriter, ECVersion ecXmlVersion) const
    {
    if (ECVersion::V2_0 == ecXmlVersion)
        return T_Super::_WriteXml(xmlWriter, ecXmlVersion);
    else
        return T_Super::_WriteXml(xmlWriter, ecXmlVersion, ECXML_ENTITYCLASS_ELEMENT, nullptr, true);
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------+---------------+---------------+---------------+---------------+-------
bool ECEntityClass::_ToJson(BeJsValue outValue, bool standalone, bool includeSchemaVersion, bool includeInheritedProperties) const
    {
    bvector<bpair<Utf8String, Json::Value>> attributes;

    if (IsMixin())
        {
        ECEntityClassCP appliesTo = GetAppliesToClass();
        BeAssert(nullptr != appliesTo);
        attributes.push_back(bpair<Utf8String, Json::Value>(MIXIN_APPLIES_TO_ATTRIBUTE, ECJsonUtilities::FormatClassName(*appliesTo)));
        if (HasBaseClasses())
            attributes.push_back(bpair<Utf8String, Json::Value>(ECJSON_BASE_CLASS_ELEMENT, ECJsonUtilities::FormatClassName(*(GetBaseClasses()[0]))));
        }
    else
        {
        if (HasBaseClasses())
            {
            Json::Value mixinArr(Json::ValueType::arrayValue);
            for (auto const& baseClass : GetBaseClasses())
                {
                if (baseClass->GetEntityClassCP()->IsMixin())
                    mixinArr.append(ECJsonUtilities::FormatClassName(*baseClass));
                else
                    {
                    BeAssert([](auto const& attr) // Assert base element hasn't already been added.
                        {
                        for (auto const& elem : attr)
                            if (elem.first == ECJSON_BASE_CLASS_ELEMENT)
                                return false;
                        return true;
                        }(attributes));
                    attributes.push_back(bpair<Utf8String, Json::Value>(ECJSON_BASE_CLASS_ELEMENT, ECJsonUtilities::FormatClassName(*baseClass)));
                    }
                }
            if (0 != mixinArr.size())
                attributes.push_back(bpair<Utf8String, Json::Value>(ECJSON_MIXIN_REFERENCES_ATTRIBUTE, mixinArr));
            }
        }

    return T_Super::_ToJson(outValue, standalone, includeSchemaVersion, includeInheritedProperties, attributes);
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------+---------------+---------------+---------------+---------------+-------
bool ECEntityClass::VerifyMixinHierarchy(bool thisIsMixin, ECEntityClassCP baseAsEntity) const
    {
    bool baseIsMixin = baseAsEntity->IsMixin();
    if (thisIsMixin && !baseIsMixin)
        {
        LOG.errorv("Cannot add '%s' as a base class to '%s' because the base class is not a mixin but %s is.",
                   baseAsEntity->GetFullName(), GetFullName(), GetFullName());
        return false;
        }

    if (thisIsMixin && baseIsMixin)
        {
        ECEntityClassCP thisAppliesToClass = GetAppliesToClass();
        ECEntityClassCP baseAppliesToClass = baseAsEntity->GetAppliesToClass();
        if (nullptr == thisAppliesToClass || nullptr == baseAppliesToClass)
            {
            LOG.errorv("Cannot add '%s' as a base class to '%s' because they are mixin classes but at least one does not define the entity class to which it can be applied",
                       baseAsEntity->GetFullName(), GetFullName());
            return false;
            }
        if (!thisAppliesToClass->Is(baseAppliesToClass))
            {
            LOG.errorv("Cannot add '%s' as a base class to '%s' because they are mixins and the base applies to constraint '%s' is not a base class of the derived applies to constraint '%s'",
                       baseAsEntity->GetFullName(), GetFullName(), baseAppliesToClass->GetFullName(), thisAppliesToClass->GetFullName());
            return false;
            }
        return true;
        }

    if(baseIsMixin)
        {
        ECEntityClassCP baseAppliesToClass = baseAsEntity->GetAppliesToClass();
        if (nullptr == baseAppliesToClass)
            {
            LOG.errorv("Cannot add '%s' as a base class to '%s' because they are mixin classes but '%s' one does not define the entity class to which it can be applied",
                       baseAsEntity->GetFullName(), GetFullName(), baseAsEntity->GetFullName());
            return false;
            }

        if(!this->Is(baseAppliesToClass))
            {
            LOG.errorv("Cannot add '%s' as a base class to '%s' because the base class is a mixin and the derived class does not derive from '%s' which is the applies to constraint",
                       baseAsEntity->GetFullName(), GetFullName(), baseAppliesToClass->GetFullName());
            return false;
            }
        }

    return true;
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------+---------------+---------------+---------------+---------------+-------
bool ECEntityClass::_Validate() const
    {
    if(!HasBaseClasses())
        return true;

    bool thisIsMixin = IsMixin();

    if (thisIsMixin && GetBaseClasses().size() > 1)
        {
        LOG.errorv("Base Class Validation: The mixin '%s' has more than one base mixin, a mixin may only have one base mixin.", GetFullName());
        return false;
        }

    for(ECClassCP baseClass : GetBaseClasses())
        {
        ECEntityClassCP baseAsEntity = baseClass->GetEntityClassCP();
        if (nullptr == baseAsEntity)
            {
            Utf8String classTypeName = thisIsMixin ? "mixin" : "entity";
            LOG.errorv("Base Class Validation: The '%s' '%s' has a base '%s' which is not a '%s'.",
                       classTypeName.c_str(), GetFullName(), classTypeName.c_str(), classTypeName.c_str());
            return false;
            }
        if (!VerifyMixinHierarchy(thisIsMixin, baseAsEntity))
            return false;
        }
    return true;
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------+---------------+---------------+---------------+---------------+-------
ECObjectsStatus ECEntityClass::_AddBaseClass(ECClassCR baseClass, bool insertAtBeginning, bool resolveConflicts, bool validate)
    {
    if (validate)
        {
        if (IsMixin() && GetBaseClasses().size() == 1)
            return ECObjectsStatus::BaseClassUnacceptable;

        ECEntityClassCP baseAsEntity = baseClass.GetEntityClassCP();
        if (nullptr != baseAsEntity)
            {
            bool thisIsMixin = IsMixin();
            if (!VerifyMixinHierarchy(thisIsMixin, baseAsEntity))
                return ECObjectsStatus::BaseClassUnacceptable;
            }
        }

    return T_Super::_AddBaseClass(baseClass, insertAtBeginning, resolveConflicts, validate);
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------+---------------+---------------+---------------+---------------+-------
ECObjectsStatus ECEntityClass::CreateNavigationProperty(NavigationECPropertyP& ecProperty, Utf8StringCR name, ECRelationshipClassCR relationshipClass, ECRelatedInstanceDirection direction, bool verify)
    {
    ecProperty = new NavigationECProperty(*this);
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
// @bsimethod
//---------------+---------------+---------------+---------------+---------------+-------
ECObjectsStatus ECEntityClass::SetAppliesToClass(ECEntityClassCP entityClass) const
    {
    if (!IsMixin())
        return ECObjectsStatus::UnableToSetMixinCustomAttribute;

    IECInstancePtr caInstance = GetCustomAttributeLocal("CoreCustomAttributes", "IsMixin");
    if (!caInstance.IsValid())
        return ECObjectsStatus::UnableToSetMixinCustomAttribute;

    if (nullptr == entityClass)
        {
        ECValue nullValue;
        nullValue.SetToNull();
        caInstance->SetValue("AppliesToEntityClass", nullValue);
        return ECObjectsStatus::Success;
        }

    ECValue appliesToValue;
    caInstance->GetValue(appliesToValue, "AppliesToEntityClass");
    if (appliesToValue.IsNull() || !appliesToValue.IsString())
        return ECObjectsStatus::UnableToSetMixinCustomAttribute;
    if (BentleyStatus::SUCCESS != appliesToValue.SetUtf8CP(entityClass->GetFullName()))
        return ECObjectsStatus::UnableToSetMixinCustomAttribute;
    auto caSetStatus = caInstance->SetValue("AppliesToEntityClass", appliesToValue);
    if (ECObjectsStatus::Success != caSetStatus)
        return caSetStatus;

    return ECObjectsStatus::Success;
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------+---------------+---------------+---------------+---------------+-------
ECEntityClassCP ECEntityClass::GetAppliesToClass() const
    {
    if (!IsMixin())
        return nullptr;

    IECInstancePtr caInstance = GetCustomAttributeLocal("CoreCustomAttributes", "IsMixin");
    if (!caInstance.IsValid())
        return nullptr;

    ECValue appliesToValue;
    caInstance->GetValue(appliesToValue, "AppliesToEntityClass");
    if (appliesToValue.IsNull() || !appliesToValue.IsString())
        return nullptr;

    Utf8String alias;
    Utf8String className;
    if (ECObjectsStatus::Success != ECClass::ParseClassName(alias, className, appliesToValue.GetUtf8CP()))
        return nullptr;

    ECSchemaCP resolvedSchema = nullptr;
    if (Utf8String::IsNullOrEmpty(alias.c_str()) || GetSchema().GetAlias().EqualsI(alias.c_str()))
        resolvedSchema = &GetSchema();
    else
        resolvedSchema = GetSchema().GetSchemaByAliasP(alias);

    if (nullptr == resolvedSchema)
        {
        LOG.errorv("Cannot resolve the 'applies to' class '%s' of mixin '%s' because the schema which contains it cannot be found.", appliesToValue.GetUtf8CP(), GetFullName());
        return nullptr;
        }

    ECClassCP appliesToClass = resolvedSchema->GetClassCP(className.c_str());
    if (nullptr == appliesToClass)
        {
        Utf8String fullSchemaName = resolvedSchema->GetFullSchemaName();
        LOG.errorv("Cannot resolve the 'applies to' class '%s' of mixin '%s' because the schema '%s' does not contain the class.", appliesToValue.GetUtf8CP(), GetFullName(), fullSchemaName.c_str());
        return nullptr;
        }

    return appliesToClass->GetEntityClassCP();
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------+---------------+---------------+---------------+---------------+-------
bool ECEntityClass::CanApply(ECEntityClassCR mixinClass) const
    {
    ECEntityClassCP appliesToEntityClass = mixinClass.GetAppliesToClass();
    if (nullptr == appliesToEntityClass)
        return false;

    if ((&GetSchema() != &mixinClass.GetSchema()) && !ECSchema::IsSchemaReferenced(GetSchema(), mixinClass.GetSchema()))
        return false;

    return Is(appliesToEntityClass);
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------+---------------+---------------+---------------+---------------+-------
bool ECEntityClass::IsOrAppliesTo(ECEntityClassCP entityClass) const
    {
    if (nullptr == entityClass)
        return false;

    if (Is(entityClass))
        return true;

    ECEntityClassCP appliesToConstraintClass = GetAppliesToClass();
    if (nullptr == appliesToConstraintClass)
        return false;

    return appliesToConstraintClass->Is(entityClass);
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------+---------------+---------------+---------------+---------------+-------
SchemaWriteStatus ECCustomAttributeClass::_WriteXml(BeXmlWriterR xmlWriter, ECVersion ecXmlVersion) const
    {
    if (ECVersion::V2_0 == ecXmlVersion)
        return T_Super::_WriteXml(xmlWriter, ecXmlVersion);
    else
        {
        Utf8String appliesToAttributeValue = SchemaParseUtils::ContainerTypeToString(m_containerType);
        bmap<Utf8CP, Utf8CP> additionalAttributes;
        additionalAttributes[CUSTOM_ATTRIBUTE_APPLIES_TO_ATTRIBUTE] = appliesToAttributeValue.c_str();
        return T_Super::_WriteXml(xmlWriter, ecXmlVersion, ECXML_CUSTOMATTRIBUTECLASS_ELEMENT, &additionalAttributes, true);
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------+---------------+---------------+---------------+---------------+-------
bool ECCustomAttributeClass::_ToJson(BeJsValue outValue, bool standalone, bool includeSchemaVersion, bool includeInheritedProperties) const
    {
    bvector<bpair<Utf8String, Json::Value>> attributes;
    attributes.push_back(bpair<Utf8String, Json::Value>(CUSTOM_ATTRIBUTE_APPLIES_TO_ATTRIBUTE, SchemaParseUtils::ContainerTypeToString(m_containerType)));
    return T_Super::_ToJson(outValue, standalone, includeSchemaVersion, includeInheritedProperties, attributes);
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------+---------------+---------------+---------------+---------------+-------
SchemaReadStatus ECCustomAttributeClass::_ReadXmlAttributes(pugi::xml_node classNode)
    {
    SchemaReadStatus status;
    if (SchemaReadStatus::Success != (status = T_Super::_ReadXmlAttributes(classNode)))
        return status;

    auto appliesToAttr = classNode.attribute(CUSTOM_ATTRIBUTE_APPLIES_TO_ATTRIBUTE);
    Utf8String appliesTo;
    if (appliesToAttr)
        {
        appliesTo = appliesToAttr.as_string();
        if (ECObjectsStatus::Success != SchemaParseUtils::ParseContainerString(this->m_containerType, appliesTo))
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
// @bsimethod
//---------------+---------------+---------------+---------------+---------------+-------
SchemaWriteStatus ECStructClass::_WriteXml(BeXmlWriterR xmlWriter, ECVersion ecXmlVersion) const
    {
    if (ECVersion::V2_0 == ecXmlVersion)
        return T_Super::_WriteXml(xmlWriter, ecXmlVersion);
    else
        return T_Super::_WriteXml(xmlWriter, ecXmlVersion, ECXML_STRUCTCLASS_ELEMENT, nullptr, true);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ECPropertyIterable::const_iterator::const_iterator(ECClassCR ecClass, bool includeBaseProperties)
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
ECPropertyIterable::const_iterator ECPropertyIterable::begin() const
    {
    return ECPropertyIterable::const_iterator(m_ecClass, m_includeBaseProperties);
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------+---------------+---------------+---------------+---------------+-------
ECPropertyIterable::const_iterator ECPropertyIterable::end() const
    {
    return ECPropertyIterable::const_iterator();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ECPropertyCP ECPropertyIterable::FindByDisplayLabel(Utf8CP label) const
    {
    for (auto const& prop : *this)
        if (prop->GetDisplayLabel().Equals (label))
            return prop;

    return nullptr;
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------+---------------+---------------+---------------+---------------+-------
ECPropertyIterable::const_iterator& ECPropertyIterable::const_iterator::operator++()
    {
    m_state->m_listIterator++;
    if (m_state->m_listIterator == m_state->m_properties->end())
        m_isEnd = true;
    return *this;
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------+---------------+---------------+---------------+---------------+-------
bool ECPropertyIterable::const_iterator::operator!= (const_iterator const& rhs) const
    {
    if (m_isEnd && rhs.m_isEnd)
        return false;
    if (m_state.IsNull() && !(rhs.m_state.IsNull()))
        return true;
    if (!(m_state.IsNull()) && rhs.m_state.IsNull())
        return true;
    return (m_state->m_listIterator != rhs.m_state->m_listIterator);
    }

static const ECPropertyP s_nullPropertyPtr = nullptr;

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------+---------------+---------------+---------------+---------------+-------
ECPropertyP const& ECPropertyIterable::const_iterator::operator*() const
    {
    if (m_isEnd)
        return s_nullPropertyPtr;

    ECPropertyP const& ecProperty = *(m_state->m_listIterator);
    return ecProperty;
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------+---------------+---------------+---------------+---------------+-------
ECPropertyIterable::IteratorState::IteratorState(ECClassCR ecClass, bool includeBaseProperties)
    {
    m_properties = new PropertyList();
    ecClass.GetProperties(includeBaseProperties, m_properties);
    m_listIterator = m_properties->begin();
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------+---------------+---------------+---------------+---------------+-------
ECPropertyIterable::IteratorState::~IteratorState()
    {
    delete m_properties;
    }

//! Checks of the current multiplicity matches or is more restrictive than other.
//! This returns false, if one of the bounds is less restrictive than its counterpart on the parameter.
bool RelationshipMultiplicity::IsEqualToOrMoreRestrictiveThan(RelationshipMultiplicityCR other) const
    {
    return m_lowerLimit >= other.GetLowerLimit() && m_upperLimit <= other.GetUpperLimit();
    }

//! Checks of the current multiplicity matches the other.
bool RelationshipMultiplicity::Equals(RelationshipMultiplicityCR other) const
    {
    return m_lowerLimit == other.GetLowerLimit() && m_upperLimit == other.GetUpperLimit();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String RelationshipMultiplicity::ToString() const
    {
    Utf8Char multiplicityString[32];

    if (INT_MAX == m_upperLimit)
        BeStringUtilities::Snprintf(multiplicityString, "(%d..*)", m_lowerLimit);
    else
        BeStringUtilities::Snprintf(multiplicityString, "(%d..%d)", m_lowerLimit, m_upperLimit);

    return multiplicityString;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
RelationshipMultiplicityCR RelationshipMultiplicity::ZeroOne()
    {
    return s_zeroOneMultiplicity;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
RelationshipMultiplicityCR RelationshipMultiplicity::ZeroMany()
    {
    return s_zeroManyMultiplicity;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
RelationshipMultiplicityCR RelationshipMultiplicity::OneOne()
    {
    return s_oneOneMultiplicity;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
RelationshipMultiplicityCR RelationshipMultiplicity::OneMany()
    {
    return s_oneManyMultiplicity;
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------+---------------+---------------+---------------+---------------+-------
ECRelationshipConstraint::ECRelationshipConstraint(ECRelationshipClassP relationshipClass, bool isSource, bool verify)
    : m_isSource(isSource), m_verify(verify), m_relClass(relationshipClass), m_multiplicity(&s_zeroOneMultiplicity), m_isPolymorphic(true),
    m_abstractConstraint(nullptr), m_verified(false) {}

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ECRelationshipConstraint::~ECRelationshipConstraint()
    {
     if ((m_multiplicity != &s_zeroOneMultiplicity) && (m_multiplicity != &s_zeroManyMultiplicity) &&
        (m_multiplicity != &s_oneOneMultiplicity) && (m_multiplicity != &s_oneManyMultiplicity))
        delete m_multiplicity;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ECSchemaCP ECRelationshipConstraint::_GetContainerSchema() const
    {
    return &(GetRelationshipClass().GetSchema());
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------+---------------+---------------+---------------+---------------+-------
Utf8String ECRelationshipConstraint::_GetContainerName() const
    {
    return Utf8String(m_relClass->GetFullName()) + ":" + GetRoleLabel();
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------+---------------+---------------+---------------+---------------+-------
bool ECRelationshipConstraint::IsValid(bool resolveIssues)
    {
    bool valid = true;

    if (GetConstraintClasses().size() == 0)
        {
        LOG.errorv("Relationship Class Constraint Violation: The %s-Constraint of '%s' does not contain any constraint classes.",
                (m_isSource) ? ECXML_SOURCECONSTRAINT_ELEMENT : ECXML_TARGETCONSTRAINT_ELEMENT, m_relClass->GetFullName());

        valid = false;
        }

    if (ECObjectsStatus::Success != ValidateRoleLabel(resolveIssues))
        {
        LOG.errorv("Relationship Class Constraint Violation: Role Label validation failed for the '%s' constraint of relationship '%s'",
            m_isSource ? ECXML_SOURCECONSTRAINT_ELEMENT : ECXML_TARGETCONSTRAINT_ELEMENT, m_relClass->GetFullName());
        valid = false;
        }
    if (ECObjectsStatus::Success != ValidateMultiplicityConstraint(resolveIssues))
        {
        LOG.errorv("Relationship Class Constraint Violation: Multiplicity validation failed for the '%s' constraint of relationship '%s'",
                   m_isSource ? ECXML_SOURCECONSTRAINT_ELEMENT : ECXML_TARGETCONSTRAINT_ELEMENT, m_relClass->GetFullName());
        valid = false;
        }
    if (ECObjectsStatus::Success != ValidateAbstractConstraint(resolveIssues))
        {
        LOG.errorv("Relationship Class Constraint Violation: Abstract Class Constraint validation failed for the '%s' constraint of relationship '%s'",
                   m_isSource ? ECXML_SOURCECONSTRAINT_ELEMENT : ECXML_TARGETCONSTRAINT_ELEMENT, m_relClass->GetFullName());
        // Need to stop validation if abstract constraint fails, since it will change the error messages from the class constraint validation.
        m_verified = false;
        return m_verified;
        }
    if (ECObjectsStatus::Success != ValidateClassConstraint())
        {
        LOG.errorv("Relationship Class Constraint Violation: Class Constraint validation failed for the '%s' constraint of relationship '%s'",
                   m_isSource ? ECXML_SOURCECONSTRAINT_ELEMENT : ECXML_TARGETCONSTRAINT_ELEMENT, m_relClass->GetFullName());
        valid = false;
        }

    m_verified = valid;

    return m_verified;
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------+---------------+---------------+---------------+---------------+-------
ECObjectsStatus ECRelationshipConstraint::ValidateBaseConstraint(ECRelationshipConstraintCR baseConstraint) const
    {
    if (nullptr != m_abstractConstraint && !baseConstraint.SupportsClass(*m_abstractConstraint))
        {
        LOG.errorv("Abstract Constraint Violation: The abstract constraint class '%s' on %s-Constraint of '%s' is not derived from the abstract constraint class '%s' as specified in Class '%s'",
                    GetAbstractConstraint()->GetFullName(), (m_isSource) ? ECXML_SOURCECONSTRAINT_ELEMENT : ECXML_TARGETCONSTRAINT_ELEMENT, m_relClass->GetFullName(),
                    nullptr == baseConstraint.GetAbstractConstraint() ? "(unknown)" : baseConstraint.GetAbstractConstraint()->GetFullName(), baseConstraint.GetRelationshipClass().GetFullName());
        return ECObjectsStatus::BaseClassUnacceptable;
        }

    if (m_constraintClasses.size() != 0 && baseConstraint.GetConstraintClasses().size() != 0)
        {
        for(ECClassCP constraintClass : m_constraintClasses)
            {
            if (!baseConstraint.SupportsClass(*constraintClass))
                {
                LOG.errorv("Class Constraint Violation: The class '%s' on %s-Constraint of '%s' is not compatible with the constraint specified in Class '%s'",
                           constraintClass->GetFullName(), (m_isSource) ? ECXML_SOURCECONSTRAINT_ELEMENT : ECXML_TARGETCONSTRAINT_ELEMENT, m_relClass->GetFullName(),
                           baseConstraint.GetRelationshipClass().GetFullName());
                return ECObjectsStatus::BaseClassUnacceptable;
                }
            }
        }

    if (!GetMultiplicity().IsEqualToOrMoreRestrictiveThan(baseConstraint.GetMultiplicity()))
        {
        LOG.errorv("Multiplicity Violation: The Multiplicity (%" PRIu32 "..%" PRIu32 ") of the %s-constraint on %s is less restrictive than the Multiplicity of it's base class %s (%" PRIu32 "..%" PRIu32 ")",
                   GetMultiplicity().GetLowerLimit(), GetMultiplicity().GetUpperLimit(), (m_isSource) ? ECXML_SOURCECONSTRAINT_ELEMENT : ECXML_TARGETCONSTRAINT_ELEMENT, m_relClass->GetFullName(),
                   baseConstraint.GetRelationshipClass().GetFullName(), baseConstraint.GetMultiplicity().GetLowerLimit(), baseConstraint.GetMultiplicity().GetUpperLimit());
        return ECObjectsStatus::BaseClassUnacceptable;
        }

    return ECObjectsStatus::Success;
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------+---------------+---------------+---------------+---------------+-------
void ECClass::FindCommonBaseClass(ECEntityClassCP &commonClass, ECEntityClassCP startingClass, bvector<ECClassCP> const& constraintClasses)
    {
    ECEntityClassCP tempCommonClass = startingClass;
    for (const auto &secondConstraint : constraintClasses)
        {
        ECClassCP secondClass = secondConstraint;
        ECEntityClassCP asEntity = secondClass->GetEntityClassCP();
        if (nullptr != asEntity && asEntity->IsMixin() && asEntity->GetAppliesToClass()->Is(tempCommonClass))
            continue;
        if (secondClass->Is(tempCommonClass))
            continue;

        for (const auto baseClass : tempCommonClass->GetBaseClasses())
            {
            if (!baseClass->IsEntityClass())
                continue;

            FindCommonBaseClass(commonClass, baseClass->GetEntityClassCP(), constraintClasses);
            if (commonClass != nullptr)
                return;
            }

        tempCommonClass = nullptr;
        break;
        }

    if (nullptr != tempCommonClass)
        commonClass = tempCommonClass;
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------+---------------+---------------+---------------+---------------+-------
ECObjectsStatus ECRelationshipConstraint::ValidateAbstractConstraint(ECClassCP abstractConstraint, bool resolveIssues)
    {
    resolveIssues &= m_relClass->GetSchema().OriginalECXmlVersionLessThan(ECVersion::V3_1);
    if (abstractConstraint == nullptr)
        {
        if (m_constraintClasses.size() == 0)
            return ECObjectsStatus::Success;

        LOG.messagev(resolveIssues? NativeLogging::SEVERITY::LOG_INFO : NativeLogging::SEVERITY::LOG_ERROR,
            "Abstract Constraint Violation: The %s-Constraint of '%s' does not contain or inherit an %s attribute. It is a required attribute if there is more than one constraint class for EC3.1 or higher.",
                (m_isSource) ? ECXML_SOURCECONSTRAINT_ELEMENT : ECXML_TARGETCONSTRAINT_ELEMENT, m_relClass->GetFullName(), ABSTRACTCONSTRAINT_ATTRIBUTE);

        if (resolveIssues)
            {
            // Attempt to resolve the issue by finding a common base class between all constraint classes
            if (m_constraintClasses.size() > 1)
                {
                ECEntityClassCP commonClass = nullptr;
                ECClass::FindCommonBaseClass(commonClass, m_constraintClasses[0]->GetEntityClassCP(), GetConstraintClasses());

                if (nullptr != commonClass && ECObjectsStatus::Success == ValidateAbstractConstraint(commonClass))
                    {
                    if (ECObjectsStatus::Success == SetAbstractConstraint(*commonClass))
                        {
                        LOG.infov("The %s attribute of %s-Constraint on class '%s' has been set to the class '%s' since it is a common base class of all shared constraint classes.",
                                     ABSTRACTCONSTRAINT_ATTRIBUTE, (m_isSource) ? ECXML_SOURCECONSTRAINT_ELEMENT : ECXML_TARGETCONSTRAINT_ELEMENT,
                                     m_relClass->GetFullName(), m_abstractConstraint->GetFullName());
                        return ECObjectsStatus::Success;
                        }
                    }
                else
                    LOG.errorv("Failed to find a common base class between the constraint classes of %s-Constraint on class '%s'",
                                (m_isSource) ? ECXML_SOURCECONSTRAINT_ELEMENT : ECXML_TARGETCONSTRAINT_ELEMENT, m_relClass->GetFullName());
                }
            }

        return ECObjectsStatus::RelationshipConstraintsNotCompatible;
        }

    bool valid = true;
    for (const auto &baseClass : m_relClass->GetBaseClasses())
        {
        ECRelationshipClassCP baseRelClass = baseClass->GetRelationshipClassCP();
        ECRelationshipConstraintR baseConstraint = (m_isSource) ? baseRelClass->GetSource() : baseRelClass->GetTarget();
        if (!baseConstraint.SupportsClass(*abstractConstraint))
            {
            if (resolveIssues)
                {
                ECEntityClassCP commonClass = nullptr;
                ECClass::FindCommonBaseClass(commonClass, m_constraintClasses[0]->GetEntityClassCP(), baseConstraint.GetConstraintClasses());
                if (nullptr != commonClass)
                    {
                    LOG.infov("Abstract Constraint Violation: The abstract constraint class '%s' on %s-Constraint of '%s' is not supported by the base class constraint '%s'.  Replacing the constraint class with %s instead.",
                               abstractConstraint->GetFullName(), (m_isSource) ? ECXML_SOURCECONSTRAINT_ELEMENT : ECXML_TARGETCONSTRAINT_ELEMENT, m_relClass->GetFullName(),
                               baseConstraint.GetRelationshipClass().GetFullName(), commonClass->GetFullName());
                    baseConstraint.RemoveConstraintClasses();
                    baseConstraint.AddClass(*commonClass);
                    }

                if (!baseConstraint.SupportsClass(*abstractConstraint))
                    {
                    if (!baseConstraint.GetIsPolymorphic() && baseConstraint.GetRelationshipClass().GetSchema().OriginalECXmlVersionLessThan(ECVersion::V3_1))
                        {
                        baseConstraint.SetIsPolymorphic(true);
                        if (!baseConstraint.SupportsClass(*abstractConstraint))
                            valid = false;
                        }
                    else
                        valid = false;

                    if (!valid)
                        {
                        LOG.errorv("Abstract Constraint Violation: The abstract constraint class '%s' on %s-Constraint of '%s' is not supported by the base class constraint '%s'.  An attempt to resolve this issue failed.",
                            abstractConstraint->GetFullName(), (m_isSource) ? ECXML_SOURCECONSTRAINT_ELEMENT : ECXML_TARGETCONSTRAINT_ELEMENT, m_relClass->GetFullName(),
                            baseConstraint.GetRelationshipClass().GetFullName());

                        }
                    }
                }
            else
                {
                LOG.errorv("Abstract Constraint Violation: The abstract constraint class '%s' on %s-Constraint of '%s' is not supported by the base class constraint '%s'",
                           abstractConstraint->GetFullName(), (m_isSource) ? ECXML_SOURCECONSTRAINT_ELEMENT : ECXML_TARGETCONSTRAINT_ELEMENT, m_relClass->GetFullName(),
                           baseConstraint.GetRelationshipClass().GetFullName());

                valid = false;
                }
            }
        for (const auto &constraint : m_constraintClasses)
            {
            if (!baseConstraint.SupportsClass(*constraint) && baseConstraint.GetAbstractConstraint() == abstractConstraint && resolveIssues)
                {
                ECEntityClassCP commonClass = nullptr;
                ECClass::FindCommonBaseClass(commonClass, abstractConstraint->GetEntityClassCP(), GetConstraintClasses());

                if (nullptr != commonClass && ECObjectsStatus::Success == baseConstraint.ValidateAbstractConstraint(commonClass))
                    {
                    if (ECObjectsStatus::Success == baseConstraint.SetAbstractConstraint(*commonClass))
                        {
                        LOG.infov("The %s attribute of %s-Constraint on class '%s' has been set to the class '%s' since it is a common base class of all shared constraint classes.",
                                  ABSTRACTCONSTRAINT_ATTRIBUTE, (m_isSource) ? ECXML_SOURCECONSTRAINT_ELEMENT : ECXML_TARGETCONSTRAINT_ELEMENT,
                                  baseRelClass->GetFullName(), commonClass->GetFullName());
                        return ECObjectsStatus::Success;
                        }
                    }
                else
                    LOG.errorv("Failed to find a common base class between the constraint classes of %s-Constraint on class '%s'",
                               (m_isSource) ? ECXML_SOURCECONSTRAINT_ELEMENT : ECXML_TARGETCONSTRAINT_ELEMENT, m_relClass->GetFullName());
                }
            }
        }

    for (const auto &constraint : m_constraintClasses)
        {
        if ((constraint->IsRelationshipClass() && !constraint->Is(abstractConstraint)) ||
            (constraint->IsEntityClass() && !constraint->GetEntityClassCP()->IsOrAppliesTo(abstractConstraint->GetEntityClassCP())))
            {
            LOG.messagev(resolveIssues ? NativeLogging::SEVERITY::LOG_DEBUG : NativeLogging::SEVERITY::LOG_ERROR,
                "Abstract Constraint Violation: The constraint class '%s' on %s-Constraint of '%s' is not derived from the abstract constraint class '%s'. "
                "All constraint classes must be derived from the abstract constraint in EC3.1 or higher.",
                constraint->GetFullName(), (m_isSource) ? ECXML_SOURCECONSTRAINT_ELEMENT : ECXML_TARGETCONSTRAINT_ELEMENT, m_relClass->GetFullName(),
                abstractConstraint->GetFullName());

            valid = false;
            }
        }

    return (valid) ? ECObjectsStatus::Success : ECObjectsStatus::RelationshipConstraintsNotCompatible;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus ECRelationshipConstraint::ValidateClassConstraint() const
    {
    for (const auto &constraint : m_constraintClasses)
        {
        ECObjectsStatus status = ValidateClassConstraint(*constraint);
        if (ECObjectsStatus::Success != status)
            return status;
        }

    return ECObjectsStatus::Success;
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------+---------------+---------------+---------------+---------------+-------
ECObjectsStatus ECRelationshipConstraint::ValidateClassConstraint(ECClassCR constraintClass) const
    {
    if (ECClass::ClassesAreEqualByName(m_relClass, &constraintClass))
        return ECObjectsStatus::RelationshipConstraintsNotCompatible;

    ECClassCP abstractConstraint = GetAbstractConstraint();
    if (abstractConstraint != nullptr &&
            ((constraintClass.IsEntityClass() && !constraintClass.GetEntityClassCP()->IsOrAppliesTo(abstractConstraint->GetEntityClassCP())) ||
            (constraintClass.IsRelationshipClass() && !constraintClass.Is(abstractConstraint))))
        {
        LOG.messagev(m_relClass->GetSchema().OriginalECXmlVersionAtLeast(ECVersion::V3_1) ? NativeLogging::SEVERITY::LOG_ERROR : NativeLogging::SEVERITY::LOG_WARNING,
            "Class Constraint Violation: The Class '%s' on %s-Constraint of '%s' is not derived from the Abstract Constraint Class '%s'.",
            constraintClass.GetFullName(), (m_isSource) ? ECXML_SOURCECONSTRAINT_ELEMENT : ECXML_TARGETCONSTRAINT_ELEMENT, m_relClass->GetFullName(), abstractConstraint->GetFullName());

        return ECObjectsStatus::RelationshipConstraintsNotCompatible;
        }

    if (!m_relClass->HasBaseClasses())
        return ECObjectsStatus::Success;

    // Check if this is the source or target constraint. Then iterate over the base classes and check
    // if the constraintClass is equal to or larger in scope than the possibly defined scope on the
    // baseclasses.
    for (auto baseClass : m_relClass->GetBaseClasses())
        {
        // Get the relationship base class
        ECRelationshipClassCP relationshipBaseClass = baseClass->GetRelationshipClassCP();
        ECRelationshipConstraintCR baseConstraint = (m_isSource) ? relationshipBaseClass->GetSource() : relationshipBaseClass->GetTarget();

        if (ECObjectsStatus::Success != ValidateBaseConstraint(baseConstraint))
            return ECObjectsStatus::RelationshipConstraintsNotCompatible;
        }

    return ECObjectsStatus::Success;
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------+---------------+---------------+---------------+---------------+-------
ECObjectsStatus ECRelationshipConstraint::ValidateMultiplicityConstraint(bool resolveIssues) const
    {
    uint32_t lowerLimit = GetMultiplicity().GetLowerLimit();
    uint32_t upperLimit = GetMultiplicity().GetUpperLimit();
    return ValidateMultiplicityConstraint(lowerLimit, upperLimit, resolveIssues);
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------+---------------+---------------+---------------+---------------+-------
ECObjectsStatus ECRelationshipConstraint::ValidateMultiplicityConstraint(uint32_t& lowerLimit, uint32_t& upperLimit, bool resolveIssues) const
    {
    if (!m_relClass->HasBaseClasses())
        return ECObjectsStatus::Success;

    for (auto baseClass : m_relClass->GetBaseClasses())
        {
        // Get the relationship base class
        ECRelationshipClassCP relationshipBaseClass = baseClass->GetRelationshipClassCP();
        ECRelationshipConstraintP baseClassConstraint = (m_isSource) ? &relationshipBaseClass->GetSource()
                                                                              : &relationshipBaseClass->GetTarget();
        RelationshipMultiplicity multiplicity(lowerLimit, upperLimit);
        if (!multiplicity.IsEqualToOrMoreRestrictiveThan(baseClassConstraint->GetMultiplicity()))
            {
            resolveIssues &= m_relClass->GetSchema().OriginalECXmlVersionLessThan(ECVersion::V3_0);
            LOG.messagev(resolveIssues? NativeLogging::SEVERITY::LOG_DEBUG : NativeLogging::SEVERITY::LOG_ERROR,
                "Multiplicity Violation: The multiplicity (%" PRIu32 "..%" PRIu32 ") of the %s-constraint on %s is less restrictive than the Multiplicity of its base class %s (%" PRIu32 "..%" PRIu32 ")",
                    lowerLimit, upperLimit, (m_isSource) ? ECXML_SOURCECONSTRAINT_ELEMENT : ECXML_TARGETCONSTRAINT_ELEMENT, m_relClass->GetFullName(),
                    relationshipBaseClass->GetFullName(), baseClassConstraint->GetMultiplicity().GetLowerLimit(), baseClassConstraint->GetMultiplicity().GetUpperLimit());

            if (!resolveIssues)
                return ECObjectsStatus::RelationshipConstraintsNotCompatible;

            bool hasNav = false;
            for (ECN::ECClassCP ecClass2 : relationshipBaseClass->GetSchema().GetClasses())
                {
                if (!ecClass2->IsEntityClass())
                    continue;
                for (ECN::ECPropertyCP prop : ecClass2->GetProperties(false))
                    {
                    ECN::NavigationECPropertyCP navProp = prop->GetAsNavigationProperty();
                    if (nullptr == navProp)
                        continue;
                    if (navProp->GetRelationshipClass() == relationshipBaseClass)
                        {
                        hasNav = true;
                        break;
                        }
                    }
                if (hasNav)
                    break;
                }
            if (hasNav)
                {
                ECRelationshipConstraintP nonConst = const_cast<ECRelationshipConstraintP>(this);
                nonConst->SetMultiplicity(baseClassConstraint->GetMultiplicity());
                }
            else
                {
                // For legacy 2.0 schemas we change the base class constraint multiplicity to bigger derived class constraint in order for it to pass the validation rules.
                LOG.debugv("The Multiplicity of %s's base class, %s, has been changed from (%" PRIu32 "..%" PRIu32 ") to (%" PRIu32 "..%" PRIu32 ") to conform to new relationship constraint rules.",
                             m_relClass->GetFullName(), relationshipBaseClass->GetFullName(),
                             baseClassConstraint->GetMultiplicity().GetLowerLimit(), baseClassConstraint->GetMultiplicity().GetUpperLimit(), lowerLimit, upperLimit);
                baseClassConstraint->SetMultiplicity(lowerLimit, upperLimit);
                }
            }
        }

    return ECObjectsStatus::Success;
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------+---------------+---------------+---------------+---------------+-------
ECObjectsStatus ECRelationshipConstraint::ValidateRoleLabel(bool resolveIssues)
    {
    if (Utf8String::IsNullOrEmpty(GetInvariantRoleLabel().c_str()))
        {
        if (resolveIssues && m_relClass->GetSchema().OriginalECXmlVersionLessThan(ECVersion::V3_1))
            {
            if (m_relClass->HasBaseClasses())
                {
                ECClassCP baseClass = m_relClass->GetBaseClasses()[0];
                if (nullptr == baseClass)
                    return ECObjectsStatus::Error;
                ECRelationshipClassCP baseRel = baseClass->GetRelationshipClassCP();
                m_roleLabel = (m_isSource) ? baseRel->GetSource().GetRoleLabel() : baseRel->GetTarget().GetRoleLabel();
                }

            // Need to check if the roleLabel has been set or not. In the case where there is a base class but it does not have a role
            // label defined, fall back to the default.
            if (!IsRoleLabelDefined())
                {
                m_roleLabel = m_relClass->GetInvariantDisplayLabel();
                if (!m_isSource)
                    m_roleLabel += " (Reversed)";
                }

            return ECObjectsStatus::Success;
            }

        LOG.errorv("Invalid ECSchemaXML: The %s-Constraint of ECRelationshipClass %s must contain or inherit a %s attribute", (m_isSource) ? ECXML_SOURCECONSTRAINT_ELEMENT : ECXML_TARGETCONSTRAINT_ELEMENT,
                    m_relClass->GetFullName(), ROLELABEL_ATTRIBUTE);
        return ECObjectsStatus::Error;
        }

    return ECObjectsStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
SchemaReadStatus ECRelationshipConstraint::ReadXml (pugi::xml_node constraintNode, ECSchemaReadContextR schemaContext)
    {
    SchemaReadStatus status = SchemaReadStatus::Success;

    Utf8String value;  // needed for macros.
    if (m_relClass->GetSchema().OriginalECXmlVersionAtLeast(ECVersion::V3_1))
        {
        READ_REQUIRED_XML_ATTRIBUTE(constraintNode, POLYMORPHIC_ATTRIBUTE, this, IsPolymorphic, constraintNode.name());
        READ_REQUIRED_XML_ATTRIBUTE(constraintNode, ROLELABEL_ATTRIBUTE, this, RoleLabel, constraintNode.name());

        auto abstractConstraintAttr = constraintNode.attribute(ABSTRACTCONSTRAINT_ATTRIBUTE);
        Utf8String abstractConstraint = abstractConstraintAttr.as_string();
        if (abstractConstraintAttr && Utf8String::IsNullOrEmpty(abstractConstraint.c_str()))
            {
            LOG.errorv("Invalid ECSchemaXML: The ECRelationshipClass, %s, cannot have an empty %s attribute.", m_relClass->GetFullName(), ABSTRACTCONSTRAINT_ATTRIBUTE);
            return SchemaReadStatus::InvalidECSchemaXml;
            }

        if (!Utf8String::IsNullOrEmpty(abstractConstraint.c_str()))
            SetAbstractConstraint(abstractConstraint.c_str(), false);

        Utf8String multiplicity = constraintNode.attribute(MULTIPLICITY_ATTRIBUTE).as_string();
        if (Utf8String::IsNullOrEmpty(multiplicity.c_str()))
            {
            LOG.errorv("Invalid ECSchemaXML: The ECRelationshipClass, %s, must have a %s attribute.", m_relClass->GetFullName(), MULTIPLICITY_ATTRIBUTE);
            return SchemaReadStatus::InvalidECSchemaXml;
            }

        if (m_relClass->GetSchema().OriginalECXmlVersionAtLeast(ECVersion::V3_2))
            SetMultiplicity(multiplicity.c_str(), false);
        else
            SetMultiplicityFromLegacyString(multiplicity.c_str(), false);
        }
    else
        {
        ECObjectsStatus setterStatus; // need for macros
        UNUSED_VARIABLE(setterStatus);
        READ_OPTIONAL_XML_ATTRIBUTE_IGNORING_SET_ERRORS(constraintNode, POLYMORPHIC_ATTRIBUTE, this, IsPolymorphic);
        READ_OPTIONAL_XML_ATTRIBUTE_IGNORING_SET_ERRORS(constraintNode, ROLELABEL_ATTRIBUTE, this, RoleLabel);
        READ_OPTIONAL_XML_ATTRIBUTE(constraintNode, CARDINALITY_ATTRIBUTE, this, Cardinality);
        }

    // For supplemental schemas, only read in the attributes and custom attributes
    if (Utf8String::npos != _GetContainerSchema()->GetName().find("_Supplemental"))
        return SchemaReadStatus::Success;

    for (pugi::xml_node constraintClassNode : constraintNode.children(EC_CONSTRAINTCLASS_ELEMENT))
        {
        auto constraintClassNameAttr = constraintClassNode.attribute(CONSTRAINTCLASSNAME_ATTRIBUTE);
        
        if (!constraintClassNameAttr)
            return SchemaReadStatus::InvalidECSchemaXml;

        Utf8String constraintClassName = constraintClassNameAttr.as_string();
        // Parse the potentially qualified class name into an alias and short class name
        Utf8String alias;
        Utf8String className;
        if (ECObjectsStatus::Success != ECClass::ParseClassName (alias, className, constraintClassName))
            {
            LOG.errorv("Invalid ECSchemaXML: The %s ECRelationshipConstraint on ECRelationship %s contains a %s attribute with the value '%s' that can not be parsed.",
                m_isSource ? "Source" : "Target", m_relClass->GetFullName(), CONSTRAINTCLASSNAME_ATTRIBUTE, constraintClassName.c_str());
            return SchemaReadStatus::InvalidECSchemaXml;
            }

        ECSchemaCP resolvedSchema = m_relClass->GetSchema().GetSchemaByAliasP (alias);
        if (nullptr == resolvedSchema)
            {
            if (!schemaContext.ResolveConflicts())
                {
                LOG.errorv("Invalid ECSchemaXML: %s ECRelationshipConstraint on ECRelationship %s contains a %s attribute with the alias '%s' that can not be resolved to a referenced schema.",
                            m_isSource ? "Source" : "Target", m_relClass->GetFullName(), CONSTRAINTCLASSNAME_ATTRIBUTE, alias.c_str());
                return SchemaReadStatus::InvalidECSchemaXml;
                }
            LOG.infov("Invalid ECSchemaXML: %s ECRelationshipConstraint on ECRelationship %s contains a %s attribute with the alias '%s' that can not be resolved to a referenced schema. Constraint will be ignored",
                        m_isSource ? "Source" : "Target", m_relClass->GetFullName(), CONSTRAINTCLASSNAME_ATTRIBUTE, alias.c_str());
            continue;
            }

        ECClassCP constraintClass = resolvedSchema->GetClassCP (className.c_str());
        if (nullptr == constraintClass)
            {
            if (!schemaContext.ResolveConflicts())
                {
                LOG.errorv("Invalid ECSchemaXML: The %s ECRelationshipConstraint on ECRelationship %s contains a %s attribute with the value '%s' that can not be resolved to an ECClass named '%s' in the ECSchema '%s'",
                           m_isSource ? "Source" : "Target", m_relClass->GetFullName(), CONSTRAINTCLASSNAME_ATTRIBUTE, constraintClassName.c_str(), className.c_str(), resolvedSchema->GetName().c_str());
                return SchemaReadStatus::InvalidECSchemaXml;
                }
            LOG.infov("Invalid ECSchemaXML: The %s ECRelationshipConstraint on ECRelationship %s contains a %s attribute with the value '%s' that can not be resolved to an ECClass named '%s' in the ECSchema '%s'. Constraint will be ignored.",
                       m_isSource ? "Source" : "Target", m_relClass->GetFullName(), CONSTRAINTCLASSNAME_ATTRIBUTE, constraintClassName.c_str(), className.c_str(), resolvedSchema->GetName().c_str());
            continue;
            }

        if (!constraintClass->IsEntityClass() && !constraintClass->IsRelationshipClass())
            {
            if (2 == m_relClass->GetSchema().GetOriginalECXmlVersionMajor())
                {
                LOG.debugv("Invalid ECSchemaXML: The %s ECRelationshipConstraint on %s contains a %s attribute with the value '%s' that does not resolve to an ECEntityClass or ECRelationshipClass named '%s' in the ECSchema '%s'.  The constraint class will be ignored.",
                             m_isSource ? "Source" : "Target", m_relClass->GetFullName(), CONSTRAINTCLASSNAME_ATTRIBUTE, constraintClassName.c_str(), className.c_str(), resolvedSchema->GetName().c_str());
                continue;
                }
            else
                {
                LOG.errorv("Invalid ECSchemaXML: The %s ECRelationshipConstraint on %s contains a %s attribute with the value '%s' that does not resolve to an ECEntityClass or ECRelationshipClass named '%s' in the ECSchema '%s'",
                           m_isSource ? "Source" : "Target", m_relClass->GetFullName(), CONSTRAINTCLASSNAME_ATTRIBUTE, constraintClassName.c_str(), className.c_str(), resolvedSchema->GetName().c_str());
                return SchemaReadStatus::InvalidECSchemaXml;
                }
            }

        bool alreadyExists = false;
        for (auto ecClass : m_constraintClasses)
            if (ECClass::ClassesAreEqualByName(ecClass, constraintClass))
                alreadyExists = true;

        if (!alreadyExists)
            m_constraintClasses.push_back(constraintClass);

        for (pugi::xml_node keyNode : constraintClassNode.children(EC_CONSTRAINTKEY_ELEMENT))
            {
            if(keyNode.type() != pugi::xml_node_type::node_element)
                continue;
            if (m_relClass->GetSchema().OriginalECXmlVersionLessThan(ECVersion::V3_1))
                {
                LOG.debugv("Key properties are no longer supported on constraint classes. All key properties have been dropped from the constraint class '%s' on the %s-Constraint of relationship '%s'.",
                                constraintClass->GetName().c_str(), (m_isSource) ? ECXML_SOURCECONSTRAINT_ELEMENT : ECXML_TARGETCONSTRAINT_ELEMENT, m_relClass->GetFullName());
                break;
                }
            else
                {
                LOG.errorv("Invalid ECSchemaXML: The constraint class '%s' on the %s-Constraint of relationship '%s' has key properties. Key properties are no longer supported.",
                            constraintClass->GetName().c_str(), (m_isSource) ? ECXML_SOURCECONSTRAINT_ELEMENT : ECXML_TARGETCONSTRAINT_ELEMENT, m_relClass->GetFullName());
                return SchemaReadStatus::InvalidECSchemaXml;
                }
            }
        }

    return status;
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------+---------------+---------------+---------------+---------------+-------
bool ECRelationshipConstraint::ToJson(BeJsValue outValue)
    {
    outValue.SetEmptyObject();
    outValue[MULTIPLICITY_ATTRIBUTE] = GetMultiplicity().ToString();
    outValue[ROLELABEL_ATTRIBUTE] = GetInvariantRoleLabel();
    outValue[POLYMORPHIC_ATTRIBUTE] = GetIsPolymorphic();

    if (nullptr != m_abstractConstraint)
        outValue[ABSTRACTCONSTRAINT_ATTRIBUTE] = ECJsonUtilities::FormatClassName(*GetAbstractConstraint());

    WriteCustomAttributes(outValue);
    
    auto const& constraintClasses = GetConstraintClasses();
    if (constraintClasses.size() != 0)
        {
        auto constraintClassArr = outValue[ECJSON_CONSTRAINT_CLASSES];
        for (auto const constraintClass : constraintClasses)
            constraintClassArr.appendValue() = ECJsonUtilities::FormatClassName(*constraintClass);
        }

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
SchemaWriteStatus ECRelationshipConstraint::WriteXml (BeXmlWriterR xmlWriter, Utf8CP elementName, ECVersion ecXmlVersion) const
    {
    SchemaWriteStatus status = SchemaWriteStatus::Success;

    xmlWriter.WriteElementStart(elementName);

    if (ecXmlVersion >= ECVersion::V3_1)
        xmlWriter.WriteAttribute(MULTIPLICITY_ATTRIBUTE, m_multiplicity->ToString().c_str());
    else
        xmlWriter.WriteAttribute(CARDINALITY_ATTRIBUTE, SchemaParseUtils::MultiplicityToLegacyString(*m_multiplicity).c_str());

    xmlWriter.WriteAttribute(ROLELABEL_ATTRIBUTE, GetInvariantRoleLabel().c_str());

    xmlWriter.WriteAttribute(POLYMORPHIC_ATTRIBUTE, this->GetIsPolymorphic());

    if (nullptr != m_abstractConstraint && ecXmlVersion >= ECVersion::V3_1)
        {
        Utf8String qualifiedClassName = ECClass::GetQualifiedClassName(m_relClass->GetSchema(), *GetAbstractConstraint());
        xmlWriter.WriteAttribute(ABSTRACTCONSTRAINT_ATTRIBUTE, qualifiedClassName.c_str());
        }

    WriteCustomAttributes(xmlWriter, ecXmlVersion);

    for (const auto &constraint : m_constraintClasses)
        {
        xmlWriter.WriteElementStart(EC_CONSTRAINTCLASS_ELEMENT);
        Utf8String qualifiedClassName = ECClass::GetQualifiedClassName(m_relClass->GetSchema(), *constraint);
        xmlWriter.WriteAttribute(CONSTRAINTCLASSNAME_ATTRIBUTE, qualifiedClassName.c_str());
        xmlWriter.WriteElementEnd();
        }

    xmlWriter.WriteElementEnd();
    return status;
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------+---------------+---------------+---------------+---------------+-------
ECObjectsStatus ECRelationshipConstraint::SetAbstractConstraint(Utf8CP abstractConstraint) {return SetAbstractConstraint(abstractConstraint, m_verify);}

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------+---------------+---------------+---------------+---------------+-------
ECObjectsStatus ECRelationshipConstraint::SetAbstractConstraint(Utf8CP value, bool validate)
    {
    if (Utf8String::IsNullOrEmpty(value))
        return ECObjectsStatus::Error;

    // Parse the potentially qualified class name into an alias and short class name
    Utf8String alias;
    Utf8String className;
    if (ECObjectsStatus::Success != ECClass::ParseClassName(alias, className, value))
        {
        LOG.errorv("Invalid ECSchemaXML: The ECRelationshipConstraint contains an %s attribute with the value '%s' that can not be parsed.",
                    ABSTRACTCONSTRAINT_ATTRIBUTE, value);
        return ECObjectsStatus::Error;
        }

    ECSchemaCP resolvedSchema = m_relClass->GetSchema().GetSchemaByAliasP(alias);
    if (nullptr == resolvedSchema)
        {
        LOG.errorv("Invalid ECSchemaXML: ECRelationshipConstraint contains an %s attribute with the alias '%s' that can not be resolved to a referenced schema.",
                    ABSTRACTCONSTRAINT_ATTRIBUTE, alias.c_str());
        return ECObjectsStatus::SchemaNotFound;
        }

    ECClassCP constraintClass = resolvedSchema->GetClassCP(className.c_str());
    if (nullptr == constraintClass)
        {
        LOG.errorv("Invalid ECSchemaXML: The ECRelationshipConstraint contains an %s attribute with the value '%s' that can not be resolved to an ECClass named '%s' in the ECSchema '%s'",
                    ABSTRACTCONSTRAINT_ATTRIBUTE, value, className.c_str(), resolvedSchema->GetName().c_str());
        return ECObjectsStatus::ClassNotFound;
        }

    if (!constraintClass->IsEntityClass() && !constraintClass->IsRelationshipClass())
        {
        LOG.errorv("Invalid ECSchemaXML: The ECRelationshipConstraint contains an %s attribute with the value '%s' that does not resolve to an ECEntityClass named '%s' in the ECSchema '%s'",
                    ABSTRACTCONSTRAINT_ATTRIBUTE, value, className.c_str(), resolvedSchema->GetName().c_str());
        return ECObjectsStatus::ClassNotFound;
        }

    if (validate)
        return SetAbstractConstraint(*constraintClass);

    m_abstractConstraint = constraintClass;
    return ECObjectsStatus::Success;
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------+---------------+---------------+---------------+---------------+-------
ECObjectsStatus ECRelationshipConstraint::SetAbstractConstraint(ECClassCR abstractConstraint)
    {
    if (m_verify)
        {
        ECObjectsStatus status = ValidateAbstractConstraint(&abstractConstraint);
        if (ECObjectsStatus::Success != status)
            return status;
        }
    else
        m_verified = false;

    m_abstractConstraint = &abstractConstraint;
    return ECObjectsStatus::Success;
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------+---------------+---------------+---------------+---------------+-------
ECObjectsStatus ECRelationshipConstraint::SetAbstractConstraint(ECEntityClassCR abstractConstraint) {return SetAbstractConstraint((ECClassCR)abstractConstraint);}
ECObjectsStatus ECRelationshipConstraint::SetAbstractConstraint(ECRelationshipClassCR abstractConstraint) {return SetAbstractConstraint((ECClassCR)abstractConstraint);}

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------+---------------+---------------+---------------+---------------+-------
ECClassCP const ECRelationshipConstraint::GetAbstractConstraint() const
    {
    if (nullptr != m_abstractConstraint)
        return m_abstractConstraint;

    if (1 == m_constraintClasses.size())
        return m_constraintClasses[0];

    return nullptr;
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------+---------------+---------------+---------------+---------------+-------
ECObjectsStatus ECRelationshipConstraint::AddClass(ECRelationshipClassCR classConstraint) {return AddClass((ECClassCR)classConstraint);}
ECObjectsStatus ECRelationshipConstraint::AddClass(ECEntityClassCR classConstraint) {return AddClass((ECClassCR)classConstraint);}

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------+---------------+---------------+---------------+---------------+-------
ECObjectsStatus ECRelationshipConstraint::AddClass(ECClassCR classConstraint)
    {
    if (m_verify)
        {
        if (m_constraintClasses.size() == 1 && !IsAbstractConstraintDefined())
            return ECObjectsStatus::RelationshipConstraintsNotCompatible;

        ECObjectsStatus validationStatus = ValidateClassConstraint(classConstraint);
        if (validationStatus != ECObjectsStatus::Success)
            return validationStatus;
        }
    else
        m_verified = false;

    if (&(classConstraint.GetSchema()) != &(m_relClass->GetSchema()))
        {
        ECSchemaReferenceListCR referencedSchemas = m_relClass->GetSchema().GetReferencedSchemas();
        ECSchemaReferenceList::const_iterator schemaIterator = referencedSchemas.find(classConstraint.GetSchema().GetSchemaKey());
        if (schemaIterator == referencedSchemas.end())
            return ECObjectsStatus::SchemaNotFound;
        }

    for (auto ecClass : m_constraintClasses)
        {
        if (ECClass::ClassesAreEqualByName(ecClass, &classConstraint))
            return ECObjectsStatus::Success;
        }

    m_constraintClasses.push_back(&classConstraint);

    return ECObjectsStatus::Success;
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------+---------------+---------------+---------------+---------------+-------
ECObjectsStatus ECRelationshipConstraint::RemoveClass(ECClassCR classConstraint)
    {
    for (auto itor = m_constraintClasses.begin(); itor != m_constraintClasses.end(); itor++)
        {
        if (ECClass::ClassesAreEqualByName(*itor, &classConstraint))
            {
            m_constraintClasses.erase(itor);
            return ECObjectsStatus::Success;
            }
        }

    return ECObjectsStatus::ClassNotFound;
    }

bool classCompatibleWithConstraint(ECClassCP constraintClass, ECClassCP testClass, bool isPolymorphic)
    {
    if (nullptr == testClass || nullptr == constraintClass)
        return false;

    if (ECClass::ClassesAreEqualByName(constraintClass, testClass))
        return true;

    if (isPolymorphic)
        {
        ECEntityClassCP entityClass = testClass->GetEntityClassCP();
        if (nullptr != entityClass && entityClass->IsOrAppliesTo(constraintClass->GetEntityClassCP()))
            return true;
        else
            return testClass->Is(constraintClass);
        }
    return false;
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------+---------------+---------------+---------------+---------------+-------
bool ECRelationshipConstraint::SupportsClass(ECClassCR ecClass) const
    {
    if (!ecClass.IsEntityClass() && !ecClass.IsRelationshipClass())
        return false;

    if (classCompatibleWithConstraint(GetAbstractConstraint(), &ecClass, m_isPolymorphic))
        return true;

    for (ECClassCP constraintClass : GetConstraintClasses())
        {
        if (constraintClass->GetName().EqualsI("AnyClass"))
            return true;

        if (classCompatibleWithConstraint(constraintClass, &ecClass, m_isPolymorphic))
            return true;
        }

    return false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus ECRelationshipConstraint::SetIsPolymorphic (Utf8CP isPolymorphic)
    {
    PRECONDITION (nullptr != isPolymorphic, ECObjectsStatus::PreconditionViolated);

    ECObjectsStatus status = SchemaParseUtils::ParseBooleanXmlString (m_isPolymorphic, isPolymorphic);
    if (ECObjectsStatus::Success != status)
        LOG.errorv("Failed to parse the isPolymorphic string '%s' for ECRelationshipConstraint. Expected values are True or False", isPolymorphic);

    return status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus ECRelationshipConstraint::SetMultiplicity (uint32_t& lowerLimit, uint32_t& upperLimit)
    {
    if (lowerLimit == 0 && upperLimit == 1)
        m_multiplicity = &s_zeroOneMultiplicity;
    else if (lowerLimit == 0 && upperLimit == INT_MAX)
        m_multiplicity = &s_zeroManyMultiplicity;
    else if (lowerLimit == 1 && upperLimit == 1)
        m_multiplicity = &s_oneOneMultiplicity;
    else if (lowerLimit == 1 && upperLimit == INT_MAX)
        m_multiplicity = &s_oneManyMultiplicity;
    else
        m_multiplicity = new RelationshipMultiplicity(lowerLimit, upperLimit);

    return ECObjectsStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus ECRelationshipConstraint::SetMultiplicity (RelationshipMultiplicityCR multiplicity)
    {
    uint32_t lowerLimit = multiplicity.GetLowerLimit();
    uint32_t upperLimit = multiplicity.GetUpperLimit();

    if (m_verify)
        {
        ECObjectsStatus validationStatus = ValidateMultiplicityConstraint(lowerLimit, upperLimit);
        if (validationStatus != ECObjectsStatus::Success)
            return validationStatus;
        }
    else
        m_verified = false;

    return SetMultiplicity(lowerLimit, upperLimit);
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------+---------------+---------------+---------------+---------------+-------
ECObjectsStatus ECRelationshipConstraint::SetMultiplicity(Utf8StringCR multiplicity) {return SetMultiplicity(multiplicity.c_str(), m_verify);}

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------+---------------+---------------+---------------+---------------+-------
ECObjectsStatus ECRelationshipConstraint::SetMultiplicityFromLegacyString(Utf8CP multiplicity, bool validate)
    {
    PRECONDITION (nullptr != multiplicity, ECObjectsStatus::PreconditionViolated);
    uint32_t lowerLimit;
    uint32_t upperLimit;
    if (ECObjectsStatus::Success != SchemaParseUtils::ParseLegacyMultiplicityString(lowerLimit, upperLimit, multiplicity))
        {
        LOG.errorv ("Failed to parse the RelationshipMultiplicity string '%s'.", multiplicity);
        return ECObjectsStatus::ParseError;
        }

    if (validate)
        {
        ECObjectsStatus validationStatus = ValidateMultiplicityConstraint(lowerLimit, upperLimit);
        if (validationStatus != ECObjectsStatus::Success)
            return validationStatus;
        }

    return SetMultiplicity(lowerLimit, upperLimit);
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------+---------------+---------------+---------------+---------------+-------
ECObjectsStatus ECRelationshipConstraint::SetMultiplicity(Utf8CP multiplicity, bool validate)
    {
    PRECONDITION (nullptr != multiplicity, ECObjectsStatus::PreconditionViolated);
    uint32_t lowerLimit;
    uint32_t upperLimit;
    if (ECObjectsStatus::Success != SchemaParseUtils::ParseMultiplicityString(lowerLimit, upperLimit, multiplicity))
        {
        LOG.errorv ("Failed to parse the RelationshipMultiplicity string '%s'.", multiplicity);
        return ECObjectsStatus::ParseError;
        }

    if (validate)
        {
        ECObjectsStatus validationStatus = ValidateMultiplicityConstraint(lowerLimit, upperLimit);
        if (validationStatus != ECObjectsStatus::Success)
            return validationStatus;
        }

    return SetMultiplicity(lowerLimit, upperLimit);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus ECRelationshipConstraint::SetCardinality (Utf8CP cardinality)
    {
    PRECONDITION (nullptr != cardinality, ECObjectsStatus::PreconditionViolated);
    uint32_t lowerLimit;
    uint32_t upperLimit;
    ECObjectsStatus status = SchemaParseUtils::ParseCardinalityString(lowerLimit, upperLimit, cardinality);
    if (ECObjectsStatus::Success != status)
        {
        LOG.errorv ("Failed to parse the RelationshipCardinality string '%s'.", cardinality);
        return ECObjectsStatus::ParseError;
        }

    return SetMultiplicity(lowerLimit, upperLimit);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String ECRelationshipConstraint::GetRoleLabel () const
    {
    return GetInvariantRoleLabel();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus ECRelationshipConstraint::SetRoleLabel (Utf8StringCR value)
    {
    if (Utf8String::IsNullOrEmpty(value.c_str()))
        return ECObjectsStatus::Error;
    m_roleLabel = value;
    return ECObjectsStatus::Success;
    }

  /*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus ECRelationshipConstraint::CopyTo(ECRelationshipConstraintR toRelationshipConstraint, bool copyReferences)
    {
    ECObjectsStatus status = ECObjectsStatus::Success;

    if (IsRoleLabelDefined() && ECObjectsStatus::Success != (status = toRelationshipConstraint.SetRoleLabel(GetInvariantRoleLabel().c_str())))
        return status;
    if (ECObjectsStatus::Success != (status = toRelationshipConstraint.SetMultiplicity(GetMultiplicity())))
        return status;
    if (ECObjectsStatus::Success != (status = toRelationshipConstraint.SetIsPolymorphic(GetIsPolymorphic())))
        return status;

    ECSchemaP destSchema = const_cast<ECSchemaP>(toRelationshipConstraint._GetContainerSchema());

    if (IsAbstractConstraintDefined())
        {
        ECClassP targetAbstractConstraint;
        if (ECObjectsStatus::Success == (status = destSchema->GetOrCopyReferencedClassForCopy(GetRelationshipClass(), targetAbstractConstraint, GetAbstractConstraint(), copyReferences)))
            status = toRelationshipConstraint.SetAbstractConstraint(*targetAbstractConstraint);

        if (ECObjectsStatus::Success != status)
            return status;
        }

    for (auto constraintClass : GetConstraintClasses())
        {
        ECClassP targetConstraintClass;
        if (ECObjectsStatus::Success == (status = destSchema->GetOrCopyReferencedClassForCopy(GetRelationshipClass(), targetConstraintClass, constraintClass, copyReferences)))
            if (ECObjectsStatus::Success != (status = toRelationshipConstraint.AddClass(*targetConstraintClass)))
                break;
        }

    if (ECObjectsStatus::Success != status)
        return status;

    return CopyCustomAttributesTo(toRelationshipConstraint, copyReferences);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus ECRelationshipConstraint::GetOrderedRelationshipPropertyName (Utf8String& propertyNameOut)  const
    {
    // see if the custom attribute signifying a Ordered relationship is defined
    IECInstancePtr caInstance = GetCustomAttribute("Bentley_Standard_CustomAttributes", "OrderedRelationshipsConstraint");
    if (caInstance.IsValid())
        {
        ECN::ECValue value;
        Utf8CP propertyName = "OrderIdProperty";
        if (ECObjectsStatus::Success == caInstance->GetValue (value, propertyName))
            {
            propertyNameOut = value.GetUtf8CP();
            return ECObjectsStatus::Success;
            }
        }
    return ECObjectsStatus::Error;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
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
* @bsimethod
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
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus ECRelationshipClass::SetStrength (StrengthType strength)
    {
    if (!ValidateStrengthConstraint(strength, false))
        return ECObjectsStatus::RelationshipConstraintsNotCompatible;

    m_strength = strength;
    return ECObjectsStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus ECRelationshipClass::SetStrength(Utf8CP strength)
    {
    PRECONDITION (nullptr != strength, ECObjectsStatus::PreconditionViolated);

    StrengthType strengthType;
    ECObjectsStatus status = SchemaParseUtils::ParseStrengthType(strengthType, strength);
    if (ECObjectsStatus::Success == status)
        SetStrength(strengthType);
    else if (GetSchema().OriginalECXmlVersionGreaterThan(ECVersion::Latest))
        {
        LOG.debugv("ECRelationshipClass '%s' has an unknown Strength type '%s'. Setting to 'Referencing'", GetFullName(), strength);
        return SetStrength(StrengthType::Referencing); // Default if the ECVersion is greater than the latest known version. Return so error status is not returned.
        }
    else
        LOG.errorv ("Failed to parse the Strength string '%s' for ECRelationshipClass '%s'.", strength, this->GetName().c_str());

    return status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus ECRelationshipClass::SetStrengthDirection (ECRelatedInstanceDirection direction)
    {
    if (!ValidateStrengthDirectionConstraint(direction, false))
        return ECObjectsStatus::RelationshipConstraintsNotCompatible;

    m_strengthDirection = direction;
    return ECObjectsStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus ECRelationshipClass::SetStrengthDirection (Utf8CP directionString)
    {
    PRECONDITION (nullptr != directionString, ECObjectsStatus::PreconditionViolated);

    ECRelatedInstanceDirection direction;
    ECObjectsStatus status = SchemaParseUtils::ParseDirectionString(direction, directionString);
    if (ECObjectsStatus::Success != status)
        LOG.errorv ("Failed to parse the ECRelatedInstanceDirection string '%s' for ECRelationshipClass '%s'.", directionString, this->GetName().c_str());
    else
        SetStrengthDirection (direction);

    return status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
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
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus ECRelationshipClass::GetOrderedRelationshipPropertyName (Utf8String& propertyNameOut, ECRelationshipEnd end) const
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
            propertyNameOut = value.GetUtf8CP();
            return ECObjectsStatus::Success;
            }
        }

    return ECObjectsStatus::Error;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
SchemaWriteStatus ECRelationshipClass::_WriteXml (BeXmlWriterR xmlWriter, ECVersion ecXmlVersion) const
    {
    SchemaWriteStatus   status;
    bmap<Utf8CP, Utf8CP> additionalAttributes;
    additionalAttributes[STRENGTH_ATTRIBUTE] = SchemaParseUtils::StrengthToXmlString(m_strength);
    if (m_strengthDirection != ECRelatedInstanceDirection::Forward)
        { //skip the attribute for "forward" as it is the default value.
        additionalAttributes[STRENGTHDIRECTION_ATTRIBUTE] = SchemaParseUtils::DirectionToXmlString(m_strengthDirection);
        }

    if (SchemaWriteStatus::Success != (status = ECClass::_WriteXml (xmlWriter, ecXmlVersion, ECXML_RELATIONSHIP_CLASS_ELEMENT, &additionalAttributes, false)))
        return status;

    // verify that this really is the current relationship class element // CGM 07/15 - Can't do this with an XmlWriter
    //if (0 != strcmp (classNode->GetName(), EC_RELATIONSHIP_CLASS_ELEMENT))
    //    {
    //    BeAssert (false);
    //    return SchemaWriteStatus::FailedToCreateXml;
    //    }

    m_source.WriteXml (xmlWriter, ECXML_SOURCECONSTRAINT_ELEMENT, ecXmlVersion);
    m_target.WriteXml (xmlWriter, ECXML_TARGETCONSTRAINT_ELEMENT, ecXmlVersion);
    xmlWriter.WriteElementEnd();

    return status;
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------+---------------+---------------+---------------+---------------+-------
bool ECRelationshipClass::_ToJson(BeJsValue outValue, bool standalone, bool includeSchemaVersion, bool includeInheritedProperties) const
    {
    bvector<bpair<Utf8String, Json::Value>> attributes;

    attributes.push_back(bpair<Utf8String, Json::Value>(STRENGTH_ATTRIBUTE, SchemaParseUtils::StrengthToJsonString(GetStrength())));
    attributes.push_back(bpair<Utf8String, Json::Value>(STRENGTHDIRECTION_ATTRIBUTE, SchemaParseUtils::DirectionToJsonString(GetStrengthDirection())));

    Json::Value sourceJson;
    if (!GetSource().ToJson(BeJsValue(sourceJson)))
        return false;
    attributes.push_back(bpair<Utf8String, Json::Value>(ECJSON_SOURCECONSTRAINT_ELEMENT, sourceJson));

    Json::Value targetJson;
    if (!GetTarget().ToJson(BeJsValue(targetJson)))
        return false;
    attributes.push_back(bpair<Utf8String, Json::Value>(ECJSON_TARGETCONSTRAINT_ELEMENT, targetJson));

    return T_Super::_ToJson(outValue, standalone, includeSchemaVersion, includeInheritedProperties, attributes);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
SchemaReadStatus ECRelationshipClass::_ReadXmlAttributes (pugi::xml_node classNode)
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
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
SchemaReadStatus ECRelationshipClass::_ReadXmlContents (pugi::xml_node classNode, ECSchemaReadContextR context, ECSchemaCP conversionSchema, bvector<NavigationECPropertyP>& navigationProperties)
    {
    SchemaReadStatus status = T_Super::_ReadXmlContents (classNode, context, conversionSchema, navigationProperties);
    if (status != SchemaReadStatus::Success)
        return status;

    pugi::xml_node sourceNode = classNode.child(ECXML_SOURCECONSTRAINT_ELEMENT);
    if (sourceNode)
        status = m_source.ReadXml (sourceNode, context);
    if (status != SchemaReadStatus::Success)
        return status;

    pugi::xml_node targetNode = classNode.child (ECXML_TARGETCONSTRAINT_ELEMENT);
    if (targetNode)
        status = m_target.ReadXml (targetNode, context);
    if (status != SchemaReadStatus::Success)
        return status;

    return status;
    }

 /*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus ECRelationshipClass::_AddBaseClass(ECClassCR baseClass, bool insertAtBeginning, bool resolveConflicts, bool validate)
    {
    if (!baseClass.IsRelationshipClass())
        {
        LOG.errorv("Cannot add class '%s' as a base class to '%s' because they are of differing class types", baseClass.GetFullName(), GetFullName());
        return ECObjectsStatus::BaseClassUnacceptable;
        }

    if (validate && ECVersion::V3_1 <= GetSchema().GetECVersion())
        {
        // Get the relationship base class and compare its strength and direction
        ECRelationshipClassCP relationshipBaseClass = baseClass.GetRelationshipClassCP();
        if (!ValidateStrengthConstraint(relationshipBaseClass->GetStrength()))
            {
            LOG.errorv("Strength Constraint Violation: Cannot not add '%s' as a base class to relationship class '%s' because they have different strength types.",
                       relationshipBaseClass->GetFullName(), GetFullName());
            return ECObjectsStatus::RelationshipConstraintsNotCompatible;
            }

        if (!ValidateStrengthDirectionConstraint(relationshipBaseClass->GetStrengthDirection()))
            {
            LOG.errorv("Strength Direction Constraint Violation: Cannot not add '%s' as a base class to relationship class '%s' because they have different strength directions.",
                       relationshipBaseClass->GetFullName(), GetFullName());
            return ECObjectsStatus::RelationshipConstraintsNotCompatible;
            }

        m_verified = false;

        if (m_verify)
            {
            ECObjectsStatus status;
                if (ECObjectsStatus::Success != (status = GetSource().ValidateBaseConstraint(relationshipBaseClass->GetSource())) ||
                    ECObjectsStatus::Success != (status = GetTarget().ValidateBaseConstraint(relationshipBaseClass->GetTarget())))
                return status;

            m_verified = true;
            }
        }

    if (GetSchema().OriginalECXmlVersionLessThan(ECVersion::V3_1))
        {
        if (HasBaseClasses())
            {
            ECClassP originalBase = *GetBaseClasses().begin();
            LOG.debugv("Multiple base classes for relationship classes are not supported.  Replacing base class '%s' with '%s' for RelationshipClass '%s'",
                          originalBase->GetFullName(), baseClass.GetFullName(), GetFullName());
            RemoveBaseClass(*originalBase);
            }
        }

    if (HasBaseClasses())
        return ECObjectsStatus::RelationshipAlreadyHasBaseClass;

    return ECClass::_AddBaseClass(baseClass, insertAtBeginning, resolveConflicts, validate);
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------+---------------+---------------+---------------+---------------+-------
bool ECRelationshipClass::GetIsVerified()
    {
    if (!m_source.m_verified || !m_target.m_verified)
        m_verified = false;

    return m_verified;
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------+---------------+---------------+---------------+---------------+-------
bool ECRelationshipClass::Verify() const
    {
    m_verified = Verify(false);
    return m_verified;
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------+---------------+---------------+---------------+---------------+-------
bool ECRelationshipClass::Verify (bool resolveIssues) const
    {
    bool isValid = true;

    if (!ValidateStrengthConstraint(m_strength))
        isValid = false;

    if (!ValidateStrengthDirectionConstraint(m_strengthDirection))
        isValid = false;

    ECRelationshipConstraintP source = &GetSource();
    if (!source->IsValid(resolveIssues))
        isValid = false;

    ECRelationshipConstraintP target = &GetTarget();
    if (!target->IsValid(resolveIssues))
        isValid = false;

    return isValid;
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
                LOG.errorv("Strength Constraint Violation: ECRelationshipClass '%s' has different Strength, %s, than its base class '%s', %s.",
                            GetFullName(), SchemaParseUtils::StrengthToXmlString(value), relationshipBaseClass->GetFullName(), SchemaParseUtils::StrengthToXmlString(relationshipBaseClass->GetStrength()));
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
                LOG.errorv("Strength Direction Constraint Violation: ECRelationshipClass '%s' has different Strength Direction, %s, than it's base class '%s', %s.",
                            GetFullName(), SchemaParseUtils::DirectionToXmlString(value), relationshipBaseClass->GetFullName(), SchemaParseUtils::DirectionToXmlString(relationshipBaseClass->GetStrengthDirection()));
                return false;
                }
            }
        }

    return (!compareValue || GetStrengthDirection() == value);
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------+---------------+---------------+---------------+---------------+-------
ECObjectsStatus ECRelationshipClass::CreateNavigationProperty(NavigationECPropertyP& ecProperty, Utf8StringCR name, ECRelationshipClassCR relationshipClass, ECRelatedInstanceDirection direction, bool verify)
    {
    ecProperty = new NavigationECProperty(*this);
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

END_BENTLEY_ECOBJECT_NAMESPACE
