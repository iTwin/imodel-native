/*--------------------------------------------------------------------------------------+
|
|     $Source: src/ECClass.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#include "ECObjectsPch.h"

BEGIN_BENTLEY_ECOBJECT_NAMESPACE

static RelationshipMultiplicity s_zeroOneMultiplicity(0, 1);
static RelationshipMultiplicity s_zeroManyMultiplicity(0, UINT_MAX);
static RelationshipMultiplicity s_oneOneMultiplicity(1, 1);
static RelationshipMultiplicity s_oneManyMultiplicity(1, UINT_MAX);

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

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------+---------------+---------------+---------------+---------------+-------
ECClass::ECClass (ECSchemaCR schema)
    : m_schema(schema), m_modifier(ECClassModifier::None), m_xmlComments(), m_contentXmlComments(), m_propertyCountCached(false), m_propertyCount(0)
    {};

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------+---------------+---------------+---------------+---------------+-------
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

    m_defaultStandaloneEnabler = nullptr;
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------+---------------+---------------+---------------+---------------+-------
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

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------+---------------+---------------+---------------+---------------+-------
ECObjectsStatus ECClass::SetName (Utf8StringCR name)
    {
    if (!m_validatedName.SetValidName(name.c_str(), m_schema.OriginalECXmlVersionLessThan(ECVersion::V3_1)))
        return ECObjectsStatus::InvalidName;

    m_fullName = GetSchema().GetName() + ":" + GetName();
    
    return ECObjectsStatus::Success;
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------+---------------+---------------+---------------+---------------+-------
Utf8StringCR ECClass::GetDescription () const
    {
    return GetSchema().GetLocalizedStrings().GetClassDescription(this, m_description);
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------+---------------+---------------+---------------+---------------+-------
Utf8StringCR ECClass::GetDisplayLabel () const
    {
    return GetSchema().GetLocalizedStrings().GetClassDisplayLabel(this, GetInvariantDisplayLabel());
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
ECObjectsStatus ECClass::RenameConflictProperty(ECPropertyP prop, bool renameDerivedProperties, ECPropertyP& renamedProperty)
    {
    Utf8String originalName = prop->GetName();

    Utf8String newName;
    FindUniquePropertyName(newName, prop->GetClass().GetSchema().GetAlias().c_str(), prop->GetName().c_str());
    ECObjectsStatus status = RenameConflictProperty(prop, renameDerivedProperties, renamedProperty, newName);
    if (ECObjectsStatus::Success != status)
        LOG.errorv("Failed to rename property %s:%s to %s", prop->GetClass().GetFullName(), originalName.c_str(), newName.c_str());
    else
        LOG.warningv("The property %s:%s was renamed to %s", prop->GetClass().GetFullName(), originalName.c_str(), newName.c_str());

    return status;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            02/2016
//---------------+---------------+---------------+---------------+---------------+-------
ECObjectsStatus ECClass::RenameConflictProperty(ECPropertyP prop, bool renameDerivedProperties, ECPropertyP& renamedProperty, Utf8String newName)
    {
    Utf8String originalName = prop->GetName();

    PropertyMap::iterator iter = m_propertyMap.find(prop->GetName().c_str());
    if (iter == m_propertyMap.end())
        return ECObjectsStatus::PropertyNotFound;
    ECPropertyP thisProp = iter->second; // Since the property that is passed in might come from a base class, we need the actual pointer of the property from this class in order to search the propertyList for it

    ECObjectsStatus status;
    if (ECObjectsStatus::Success != (status = CopyProperty(renamedProperty, prop, newName.c_str(), true, false)))
        {
        delete renamedProperty;
        return status;
        }

    iter = m_propertyMap.find(prop->GetName().c_str());
    m_propertyMap.erase(iter);
    auto iter2 = std::find(m_propertyList.begin(), m_propertyList.end(), thisProp);
    if (iter2 != m_propertyList.end())
        m_propertyList.erase(iter2);
    InvalidateDefaultStandaloneEnabler();

    status = AddProperty(renamedProperty, newName);
    if (ECObjectsStatus::Success != status)
        {
        delete renamedProperty;
        return status;
        }

    LOG.infov("Renamed conflict property %s:%s to %s\n", GetFullName(), prop->GetName().c_str(), newName.c_str());

    // If newProperty was successfully added we need to add a CustomAttribute. To help identify the property when doing instance data conversion.
    AddPropertyMapping(originalName.c_str(), newName.c_str());

    if (renameDerivedProperties)
        RenameDerivedProperties(newName);

    return ECObjectsStatus::Success;
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------+---------------+---------------+---------------+---------------+-------
void ECClass::RenameDerivedProperties(Utf8String newName)
    {
    for (ECClassP derivedClass : m_derivedClasses)
        {
        ECPropertyP fromDerived = derivedClass->GetPropertyP(newName.c_str(), false);
        if (nullptr != fromDerived && !fromDerived->GetName().Equals(newName))
            {
            ECPropertyP renamedProperty = nullptr;
            derivedClass->RenameConflictProperty(fromDerived, true, renamedProperty, newName);
            }
        for (ECClassP derivedClassBaseClass : derivedClass->GetBaseClasses())
            {
            ECPropertyP fromBaseDerived = derivedClassBaseClass->GetPropertyP(newName.c_str(), true);
            if (nullptr == fromBaseDerived || fromBaseDerived->GetName().Equals(newName))
                continue;
            ECPropertyP renamedProperty = nullptr;
            derivedClassBaseClass->RenameConflictProperty(fromBaseDerived, true, renamedProperty, newName);
            }
        derivedClass->RenameDerivedProperties(newName);
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Caleb.Shafer    01/2017
//---------------+---------------+---------------+---------------+---------------+-------
void ECClass::AddPropertyMapping(Utf8CP originalName, Utf8CP newName)
    {
    LOG.debugv("Attempting to add RenamedPropertiesMapping custom attribute to ECClass '%s' for Property '%s'.", GetFullName(), newName);

    IECInstancePtr renameInstance = GetCustomAttributeLocal("RenamedPropertiesMapping");
    if (!renameInstance.IsValid())
        renameInstance = ConversionCustomAttributeHelper::CreateCustomAttributeInstance("RenamedPropertiesMapping");
    if (!renameInstance.IsValid())
        {
        LOG.warningv("Failed to create 'RenamedPropertiesMapping' custom attribute for ECClass '%s'", GetFullName());
        return;
        }

    ECValue v;
    renameInstance->GetValue(v, "PropertyMapping");
    Utf8String remapping("");
    if (!v.IsNull())
        remapping = Utf8String(v.GetUtf8CP()).append(";");

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

    LOG.debugv("Successfully added RenamedPropertiesMapping custom attribute to ECClass '%s'", GetFullName());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   03/13
+---------------+---------------+---------------+---------------+---------------+------*/
ECPropertyP ECClass::GetPropertyByIndex (uint32_t index) const
    {
    if (index >= (uint32_t)m_propertyList.size())
        return nullptr;

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

    m_propertyCountCached = false;
    m_propertyCount = 0;
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
            LOG.warningv("Case-collision between %s:%s and %s:%s", baseProperty.GetClass().GetFullName(), baseProperty.GetName().c_str(), GetFullName(), derivedProperty->GetName().c_str());
            ECPropertyP newProperty = nullptr;
            if (ECObjectsStatus::Success != RenameConflictProperty(derivedProperty, true, newProperty))
                return status;
            }

        // TFS#246533: Silly multiple inheritance scenarios...does derived property already have a different base property? Does the new property
        // have priority over that one based on the order of base class declarations?
        else if (nullptr == derivedProperty->GetBaseProperty() || GetBaseClassPropertyP (baseProperty.GetName().c_str()) == &baseProperty)
            {
            Utf8String errMsg;
            if (ECObjectsStatus::Success == (status = CanPropertyBeOverridden(baseProperty, *derivedProperty, errMsg)))
                derivedProperty->SetBaseProperty(&baseProperty);
            else if (ECObjectsStatus::DataTypeMismatch == status && resolveConflicts)
                {
                if (!Utf8String::IsNullOrEmpty(errMsg.c_str()))
                    LOG.warning(errMsg.c_str());
                ECPropertyP newProperty = nullptr;
                if (ECObjectsStatus::Success != RenameConflictProperty(derivedProperty, true, newProperty))
                    return status;
                }
            else if (ECObjectsStatus::DataTypeMismatch == status)
                LOG.error(errMsg.c_str());
            }
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
        Utf8String errMsg;
        ECObjectsStatus status = CanPropertyBeOverridden(*baseProperty, *pProperty, errMsg);
        if (ECObjectsStatus::Success != status)
            {
            if (ECObjectsStatus::DataTypeMismatch == status && resolveConflicts)
                {
                Utf8String originalName = pProperty->GetName();

                Utf8String newName;
                FindUniquePropertyName(newName, pProperty->GetClass().GetSchema().GetAlias().c_str(), pProperty->GetName().c_str());
                pProperty->SetName(newName);

                // If newProperty was successfully added we need to add a CustomAttribute. To help identify the property when doing instance data conversion.
                AddPropertyMapping(originalName.c_str(), newName.c_str());
                }
            else
                {
                if (!Utf8String::IsNullOrEmpty(errMsg.c_str()))
                    LOG.error(errMsg.c_str());
                return status;
                }
            }
        else if (!baseProperty->GetName().Equals(pProperty->GetName()))
            {
            if (resolveConflicts)
                pProperty->SetName(baseProperty->GetName()); // Does not need the Renamed CustomAttribute since the GetPropertyP is a case-insensitive search.
            else
                {
                LOG.errorv("Case-collision between %s:%s and %s:%s", baseProperty->GetClass().GetFullName(), baseProperty->GetName().c_str(), GetFullName(), pProperty->GetName().c_str());
                return ECObjectsStatus::CaseCollision;
                }
            }
        else if (resolveConflicts && pProperty->IsSame(*baseProperty))
            {
            LOG.infov("%s already has a base primitive property %s of the same name.  As no differences were noted, new property will not be added.", pProperty->GetClass().GetFullName(), pProperty->GetName().c_str());
            return ECObjectsStatus::Success;
            }

        pProperty->SetBaseProperty(baseProperty);
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
ECObjectsStatus ECClass::CopyProperty(ECPropertyP& destProperty, ECPropertyCP sourceProperty, bool copyCustomAttributes)
    {
    if (nullptr == sourceProperty)
        return ECObjectsStatus::NullPointerValue;

    return CopyProperty(destProperty, sourceProperty, sourceProperty->GetName().c_str(), copyCustomAttributes);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            02/2016
//---------------+---------------+---------------+---------------+---------------+-------
ECObjectsStatus ECClass::CopyProperty(ECPropertyP& destProperty, ECPropertyCP sourceProperty, Utf8CP destPropertyName, bool copyCustomAttributes, bool andAddProperty, bool copyReferences)
    {
    if (nullptr == sourceProperty)
        return ECObjectsStatus::NullPointerValue;
    if (sourceProperty->GetIsPrimitive())
        {
        PrimitiveECPropertyP destPrimitive;
        PrimitiveECPropertyCP sourcePrimitive = sourceProperty->GetAsPrimitiveProperty();
        destPrimitive = new PrimitiveECProperty(*this);
        ECEnumerationCP enumeration = sourcePrimitive->GetEnumeration();
        if (nullptr != enumeration) 
            {
            if (enumeration->GetSchema().GetSchemaKey() == sourceProperty->GetClass().GetSchema().GetSchemaKey())
                {
                ECEnumerationP destEnum = this->GetSchemaR().GetEnumerationP(enumeration->GetName().c_str());
                if (nullptr == destEnum)
                    {
                    if (copyReferences)
                        {
                        auto status = this->GetSchemaR().CopyEnumeration(destEnum, *enumeration);
                        if (ECObjectsStatus::Success != status && ECObjectsStatus::NamedItemAlreadyExists != status)
                            return status;
                        destPrimitive->SetType(*destEnum);
                        }
                    else
                        {
                        if (!ECSchema::IsSchemaReferenced(*GetContainerSchema(), enumeration->GetSchema()))
                            GetContainerSchema()->AddReferencedSchema(const_cast<ECSchemaR>(enumeration->GetSchema()));
                        destPrimitive->SetType(*enumeration);
                        }
                    }
                else
                    destPrimitive->SetType(*destEnum);
                }
            else
                destPrimitive->SetType(*enumeration);
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

        destProperty = destPrimitive;
        }
    else if (sourceProperty->GetIsStructArray())
        {
        StructArrayECPropertyP destArray;
        StructArrayECPropertyCP sourceArray = sourceProperty->GetAsStructArrayProperty();
        destArray = new StructArrayECProperty(*this);
        ECStructClassCR structElementType = sourceArray->GetStructElementType();
        if (structElementType.GetSchema().GetSchemaKey() == sourceProperty->GetClass().GetSchema().GetSchemaKey())
            {
            ECClassP destClass = this->GetSchemaR().GetClassP(structElementType.GetName().c_str());
            if (nullptr == destClass)
                {
                if (copyReferences)
                    {
                    auto status = this->GetSchemaR().CopyClass(destClass, structElementType, structElementType.GetName(), copyReferences);
                    if (ECObjectsStatus::Success != status && ECObjectsStatus::NamedItemAlreadyExists != status)
                        return status;
                    destArray->SetStructElementType(*destClass->GetStructClassCP());
                    }
                else
                    {
                    if (!ECSchema::IsSchemaReferenced(*GetContainerSchema(), sourceArray->GetClass().GetSchema()))
                        GetContainerSchema()->AddReferencedSchema(const_cast<ECStructClassR>(structElementType).GetSchemaR());

                    destArray->SetStructElementType(structElementType);
                    }
                }
            else
                destArray->SetStructElementType(*destClass->GetStructClassCP());
            }
        else
            destArray->SetStructElementType(structElementType);

        destArray->SetMaxOccurs(sourceArray->GetStoredMaxOccurs());
        destArray->SetMinOccurs(sourceArray->GetMinOccurs());

        destProperty = destArray;
        }
    else if (sourceProperty->GetIsPrimitiveArray())
        {
        PrimitiveArrayECPropertyP destArray;
        PrimitiveArrayECPropertyCP sourceArray = sourceProperty->GetAsPrimitiveArrayProperty();
        destArray = new PrimitiveArrayECProperty(*this);
        ECEnumerationCP enumeration = sourceArray->GetEnumeration();
        if (nullptr != enumeration)
            {
            if (enumeration->GetSchema().GetSchemaKey() == sourceProperty->GetClass().GetSchema().GetSchemaKey())
                {
                ECEnumerationP destEnum = this->GetSchemaR().GetEnumerationP(enumeration->GetName().c_str());
                if (nullptr == destEnum)
                    {
                    if (copyReferences)
                        {
                        auto status = this->GetSchemaR().CopyEnumeration(destEnum, *enumeration);
                        if (ECObjectsStatus::Success != status && ECObjectsStatus::NamedItemAlreadyExists != status)
                            return status;
                        destArray->SetType(*destEnum);
                        }
                    else
                        {
                        if (!ECSchema::IsSchemaReferenced(*GetContainerSchema(), enumeration->GetSchema()))
                            GetContainerSchema()->AddReferencedSchema(const_cast<ECSchemaR>(enumeration->GetSchema()));
                        destArray->SetType(*enumeration);
                        }
                    }
                else
                    destArray->SetType(*destEnum);
                }
            else
                destArray->SetType(*enumeration);
            }
        else
            destArray->SetPrimitiveElementType(sourceArray->GetPrimitiveElementType());

        if (sourceArray->IsMinimumValueDefined()) {
            ECValue valueToCopy;
            sourceArray->GetMinimumValue(valueToCopy);
            destArray->SetMinimumValue(valueToCopy);
        }

        if (sourceArray->IsMaximumValueDefined()) {
            ECValue valueToCopy;
            sourceArray->GetMaximumValue(valueToCopy);
            destArray->SetMaximumValue(valueToCopy);
        }

        if (sourceArray->IsMinimumLengthDefined())
            destArray->SetMinimumLength(sourceArray->GetMinimumLength());
        if (sourceArray->IsMaximumLengthDefined())
            destArray->SetMaximumLength(sourceArray->GetMaximumLength());

        if (sourceArray->IsExtendedTypeDefinedLocally())
            destArray->SetExtendedTypeName(sourceArray->GetExtendedTypeName().c_str());

        destArray->SetMaxOccurs(sourceArray->GetStoredMaxOccurs());
        destArray->SetMinOccurs(sourceArray->GetMinOccurs());

        destProperty = destArray;
        }
    else if (sourceProperty->GetIsStruct())
        {
        StructECPropertyP destStruct;
        StructECPropertyCP sourceStruct = sourceProperty->GetAsStructProperty();
        destStruct = new StructECProperty (*this);
        ECStructClassCR sourceType = sourceStruct->GetType();
        if (sourceType.GetSchema().GetSchemaKey() == sourceProperty->GetClass().GetSchema().GetSchemaKey())
            {
            ECClassP destClass = this->GetSchemaR().GetClassP(sourceType.GetName().c_str());
            if (nullptr == destClass)
                {
                if (copyReferences)
                    {
                    auto status = this->GetSchemaR().CopyClass(destClass, sourceType, sourceType.GetName(), copyReferences);
                    if (ECObjectsStatus::Success != status && ECObjectsStatus::NamedItemAlreadyExists != status)
                        return status;
                    destStruct->SetType(*destClass->GetStructClassCP());
                    }
                else
                    {
                    if (!ECSchema::IsSchemaReferenced(*GetContainerSchema(), sourceStruct->GetClass().GetSchema()))
                        GetContainerSchema()->AddReferencedSchema(const_cast<ECStructClassR>(sourceType).GetSchemaR());
                    destStruct->SetType(sourceType);
                    }
                }
            else
                destStruct->SetType(*destClass->GetStructClassCP());
            }
        else
            destStruct->SetType(sourceType);
        destProperty = destStruct;
        }
    else if (sourceProperty->GetIsNavigation())
        {
        NavigationECPropertyP destNav;
        NavigationECPropertyCP sourceNav = sourceProperty->GetAsNavigationProperty();
        destNav = new NavigationECProperty(*this);

        ECRelationshipClassCP sourceRelClass = sourceNav->GetRelationshipClass();
        if (sourceRelClass->GetSchema().GetSchemaKey() == sourceProperty->GetClass().GetSchema().GetSchemaKey())
            {
            ECClassP destClass = this->GetSchemaR().GetClassP(sourceRelClass->GetName().c_str());
            if (nullptr == destClass && copyReferences)
                {
                auto status = this->GetSchemaR().CopyClass(destClass, *sourceRelClass, sourceRelClass->GetName(), copyReferences);
                if (ECObjectsStatus::Success != status && ECObjectsStatus::NamedItemAlreadyExists != status)
                    return status;
                destNav->SetRelationshipClass(*destClass->GetRelationshipClassCP(), sourceNav->GetDirection());
                }
            else
                {
                if (!ECSchema::IsSchemaReferenced(*GetContainerSchema(), sourceNav->GetClass().GetSchema()))
                    GetContainerSchema()->AddReferencedSchema(const_cast<ECRelationshipClassP>(sourceRelClass)->GetSchemaR());
                destNav->SetRelationshipClass(*sourceRelClass, sourceNav->GetDirection());
                }
            }
        else
            destNav->SetRelationshipClass(*sourceRelClass, sourceNav->GetDirection());

        destProperty = destNav;
        }

    destProperty->SetDescription(sourceProperty->GetInvariantDescription());
    if (sourceProperty->GetIsDisplayLabelDefined())
        destProperty->SetDisplayLabel(sourceProperty->GetInvariantDisplayLabel());
    destProperty->SetName(sourceProperty->GetName());
    destProperty->SetIsReadOnly(sourceProperty->IsReadOnlyFlagSet());
    destProperty->SetPriority(sourceProperty->GetPriority());

    if (sourceProperty->IsCategoryDefinedLocally())
        {
        PropertyCategoryCP sourcePropCategory = sourceProperty->GetCategory();
        if (sourcePropCategory->GetSchema().GetSchemaKey() == sourceProperty->GetClass().GetSchema().GetSchemaKey())
            {
            PropertyCategoryP destPropCategory = this->GetSchemaR().GetPropertyCategoryP(sourcePropCategory->GetName().c_str());
            if (nullptr == destPropCategory)
                {
                if (copyReferences)
                    {
                    auto status = this->GetSchemaR().CopyPropertyCategory(destPropCategory, *sourcePropCategory);
                    if (ECObjectsStatus::Success != status && ECObjectsStatus::NamedItemAlreadyExists != status)
                        return status;
                    destProperty->SetCategory(destPropCategory);
                    }
                else
                    {
                    if (!ECSchema::IsSchemaReferenced(*GetContainerSchema(), sourcePropCategory->GetSchema()))
                        GetContainerSchema()->AddReferencedSchema(const_cast<ECSchemaR>(sourcePropCategory->GetSchema()));
                    destProperty->SetCategory(sourcePropCategory);
                    }
                }
            else
                destProperty->SetCategory(destPropCategory);
            }
        else
            destProperty->SetCategory(sourcePropCategory);
        }

    if (sourceProperty->IsKindOfQuantityDefinedLocally())
        {
        KindOfQuantityCP sourceKoq = sourceProperty->GetKindOfQuantity();
        if (sourceKoq->GetSchema().GetSchemaKey() == sourceProperty->GetClass().GetSchema().GetSchemaKey())
            {
            KindOfQuantityP destKoq = this->GetSchemaR().GetKindOfQuantityP(sourceKoq->GetName().c_str());
            if (nullptr == destKoq)
                {
                if (copyReferences)
                    {
                    auto status = this->GetSchemaR().CopyKindOfQuantity(destKoq, *sourceKoq);
                    if (ECObjectsStatus::Success != status && ECObjectsStatus::NamedItemAlreadyExists != status)
                        return status;
                    destProperty->SetKindOfQuantity(destKoq);
                    }
                else
                    {
                    if (!ECSchema::IsSchemaReferenced(*GetContainerSchema(), sourceKoq->GetSchema()))
                        GetContainerSchema()->AddReferencedSchema(const_cast<ECSchemaR>(sourceKoq->GetSchema()));
                    destProperty->SetKindOfQuantity(sourceKoq);
                    }
                }
            else
                destProperty->SetKindOfQuantity(destKoq);
            }
        else
            destProperty->SetKindOfQuantity(sourceKoq);
        }

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
ECObjectsStatus ECClass::CopyPropertyForSupplementation(ECPropertyP& destProperty, ECPropertyCP sourceProperty, bool copyCustomAttributes)
    {
    ECObjectsStatus status = CopyProperty(destProperty, sourceProperty, copyCustomAttributes);
    if (ECObjectsStatus::Success == status)
        destProperty->m_forSupplementation = true;

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
* @bsimethod                                                    Paul.Connelly   07/15
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
* @bsimethod                                                    Paul.Connelly   12/12
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
* @bsimethod                                    Carole.MacDonald                11/2011
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
// @bsimethod                                   Caleb.Shafer                    12/2016
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
// @bsimethod                                   Caleb.Shafer                    12/2016
//---------------+---------------+---------------+---------------+---------------+-------
// static
bool ECClass::ConvertPropertyToPrimitveArray(ECClassP ecClass, ECClassCP startingClass, Utf8String propName, bool includeDerivedClasses)
    {
    if (includeDerivedClasses && ecClass->HasDerivedClasses())
        {
        for (ECClassP derivedClass : ecClass->GetDerivedClasses())
            {
            if (ECClass::ClassesAreEqualByName(derivedClass, startingClass))
                continue;

            if (!ConvertPropertyToPrimitveArray(derivedClass, ecClass, propName, includeDerivedClasses))
                return false;
            }
        }

    if (ecClass->HasBaseClasses())
        {
        for (ECClassP baseClass : ecClass->GetBaseClasses())
            {
            if (ECClass::ClassesAreEqualByName(baseClass, startingClass))
                continue;

            if (!ConvertPropertyToPrimitveArray(baseClass, ecClass, propName))
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

    ECObjectsStatus status = primProp->CopyCustomAttributesTo(*newProperty);
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
        ConvertPropertyToPrimitveArray(derivedClass, ecClass, propName, true);

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
 @bsimethod                                     Caleb.Shafer                    12/2016
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
                if (!ConvertPropertyToPrimitveArray(this, this, propName))
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
* @bsimethod                                    Carole.MacDonald                03/2010
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
* @bsimethod                                    Caleb.Shafer                    10/2016
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
* @bsimethod                                    Carole.MacDonald                01/2010
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
// @bsimethod                                   Carole.MacDonald            08/2017
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
* @bsimethod                                    Carole.MacDonald                01/2010
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
* @bsimethod                                                    JoshSchifter    09/10
+---------------+---------------+---------------+---------------+---------------+------*/
void ECClass::RemoveDerivedClass (ECClassCR derivedClass) const
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
void ECClass::RemoveDerivedClasses ()
    {
    for (ECDerivedClassesList::iterator iter = m_derivedClasses.end(); iter != m_derivedClasses.begin(); )
        (*--iter)->RemoveBaseClass (*this);

    m_derivedClasses.clear ();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Carole.MacDonald                02/2010
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
                GetFullName(); // Call to ensure m_fullName is cached.
                ArrayInfo arrayInfo = exceptionsArray.GetArrayInfo();
                for (uint32_t i = 0; i < arrayInfo.GetCount(); ++i)
                    {
                    ECValue exception;
                    if (ECObjectsStatus::Success != notSubClassableCA->GetValue(exception, exceptionsIndex, i) || exception.IsNull())
                        continue;

                    if (m_fullName.EqualsIAscii(exception.GetUtf8CP()))
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
        if (*baseClassIterator == (ECClassP) &baseClass)
            {
            LOG.infov("Cannot add class '%s' as a base class to '%s' because it already exists as a base class", baseClass.GetFullName(), GetFullName());
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
            Utf8String errMsg;
            status = ECClass::CanPropertyBeOverridden(*prop, *thisProperty, errMsg);

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
                    LOG.warningv("Case-collision between %s:%s and %s:%s.  Renaming to %s", prop->GetClass().GetFullName(), prop->GetName().c_str(), GetFullName(), thisProperty->GetName().c_str(), prop->GetName().c_str());
                    ECClassP conflictClass = const_cast<ECClassP> (&thisProperty->GetClass());
                    ECPropertyP renamedProperty = nullptr;
                    if (ECObjectsStatus::Success != conflictClass->RenameConflictProperty(thisProperty, true, renamedProperty, prop->GetName()))
                        return status;
                    }
                }

            if (ECObjectsStatus::Success != status)
                {
                if (ECObjectsStatus::DataTypeMismatch == status && resolveConflicts)
                    {
                    if (!Utf8String::IsNullOrEmpty(errMsg.c_str()))
                        LOG.warning(errMsg.c_str());
                    LOG.warningv("Conflict between %s:%s and %s:%s.  Renaming...", prop->GetClass().GetFullName(), prop->GetName().c_str(), GetFullName(), thisProperty->GetName().c_str(), GetFullName(), thisProperty->GetName().c_str());
                    ECClassP conflictClass = const_cast<ECClassP> (&thisProperty->GetClass());
                    ECPropertyP renamedProperty = nullptr;
                    if (ECObjectsStatus::Success != conflictClass->RenameConflictProperty(thisProperty, true, renamedProperty))
                        return status;
                    }
                else
                    {
                    if (!Utf8String::IsNullOrEmpty(errMsg.c_str()))
                        LOG.error(errMsg.c_str());
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
ECObjectsStatus ECClass::_RemoveBaseClass(ECClassCR baseClass)
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
void ECClass::RemoveBaseClasses()
    {
    bvector<ECClassP> classes;
    for (ECBaseClassesList::iterator iter = m_baseClasses.begin(); iter != m_baseClasses.end(); iter++)
        classes.push_back(*iter);

    for (auto& baseClass : classes)
        RemoveBaseClass(*baseClass);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   10/13
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
* @bsimethod                                    Carole.MacDonald                02/2010
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
* @bsimethod                                    Carole.MacDonald                02/2010
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
* @bsimethod                                    Carole.MacDonald                04/2010
+---------------+---------------+---------------+---------------+---------------+------*/
ECPropertyIterable ECClass::GetProperties() const
    {
    return ECPropertyIterable(*this, true);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Carole.MacDonald                04/2010
+---------------+---------------+---------------+---------------+---------------+------*/
ECPropertyIterable ECClass::GetProperties(bool includeBaseProperties) const
    {
    return ECPropertyIterable(*this, includeBaseProperties);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   06/15
+---------------+---------------+---------------+---------------+---------------+------*/
static bool containsProperty(Utf8CP name, PropertyList const& props)
    {
    return props.end() != std::find_if (props.begin(), props.end(), [&name](ECPropertyP const& prop)
        {
        return prop->GetName().Equals (name);
        });
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus ECClass::GetProperties(bool includeBaseProperties, PropertyList* propertyList) const
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
            if (!containsProperty(baseProp->GetName().c_str(), *propertyList) && !containsProperty(baseProp->GetName().c_str(), inheritedProperties))
                inheritedProperties.push_back (baseProp);
            }
        }

    // inherited properties come before this class's properties
    propertyList->reserve(propertyList->size() + inheritedProperties.size());
    propertyList->insert(propertyList->begin(), inheritedProperties.begin(), inheritedProperties.end());

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

//---------------------------------------------------------------------------------------
// @bsimethod                                 Krischan.Eberle                       10/17
//---------------------------------------------------------------------------------------
bool ECClass::Validate() const { return _Validate(); }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    
+---------------+---------------+---------------+---------------+---------------+------*/
SchemaReadStatus ECClass::_ReadXmlAttributes (BeXmlNodeR classNode)
    {                
    Utf8String value;      // used by the macros.
    if (GetName().length() == 0)
        READ_REQUIRED_XML_ATTRIBUTE (classNode, TYPE_NAME_ATTRIBUTE, this, Name, classNode.GetName())
    
    // OPTIONAL attributes - If these attributes exist they MUST be valid    
    READ_OPTIONAL_XML_ATTRIBUTE (classNode, DESCRIPTION_ATTRIBUTE, this, Description)
    READ_OPTIONAL_XML_ATTRIBUTE (classNode, ECXML_DISPLAY_LABEL_ATTRIBUTE, this, DisplayLabel)

    Utf8String     modifierString;
    BeXmlStatus modifierStatus = classNode.GetAttributeStringValue(modifierString, MODIFIER_ATTRIBUTE);
    if (BEXML_Success == modifierStatus)
        {
        if (ECObjectsStatus::Success != SchemaParseUtils::ParseModifierXmlString(m_modifier, modifierString))
            {
            // Don't fail if the modifier string is unknown with >EC3.x versions. Default is None.
            if (GetSchema().OriginalECXmlVersionGreaterThan(ECVersion::Latest))
                {
                LOG.warningv("Class '%s' has an unknown modifier '%s'. Setting to None.", this->GetFullName(), modifierString.c_str());
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

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------+---------------+---------------+---------------+---------------+-------
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
SchemaReadStatus ECClass::_ReadXmlContents (BeXmlNodeR classNode, ECSchemaReadContextR context, ECSchemaCP conversionSchema, bvector<Utf8String>& droppedAliases, bvector<NavigationECPropertyP>& navigationProperties)
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
        else if (!isSchemaSupplemental && (0 == strcmp (childNodeName, ECXML_BASE_CLASS_ELEMENT)))
            {
            SchemaReadStatus status = _ReadBaseClassFromXml(childNode, context, conversionSchema, droppedAliases);
            if (SchemaReadStatus::Success != status)
                return status;

            if (context.GetPreserveXmlComments())
                {
                _ReadCommentsInSameLine(*childNode, comments);
                Utf8String contentIdentifier = ECXML_BASE_CLASS_ELEMENT;
                m_contentXmlComments[contentIdentifier] = comments;
                }
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
                if (BEXML_Success == childNode->GetAttributeStringValue(typeName, TYPE_NAME_ATTRIBUTE))
                    {
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
        else if (0 == strcmp(childNodeName, ECXML_CUSTOM_ATTRIBUTES_ELEMENT))
            {
            if (context.GetPreserveXmlComments())
                {
                Utf8String contentIdentifier = ECXML_CUSTOM_ATTRIBUTES_ELEMENT;
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
SchemaReadStatus ECClass::_ReadBaseClassFromXml (BeXmlNodeP childNode, ECSchemaReadContextR context, ECSchemaCP conversionSchema, bvector<Utf8String>& droppedAliases)
    {
    Utf8String qualifiedClassName;
    childNode->GetContent (qualifiedClassName);

    // Parse the potentially qualified class name into an alias and short class name
    Utf8String alias;
    Utf8String className;
    if (ECObjectsStatus::Success != ECClass::ParseClassName (alias, className, qualifiedClassName))
        {
        LOG.errorv ("Invalid ECSchemaXML: The ECClass '%s' contains a %s element with the value '%s' that can not be parsed.",  
            GetName().c_str(), ECXML_BASE_CLASS_ELEMENT, qualifiedClassName.c_str());

        return SchemaReadStatus::InvalidECSchemaXml;
        }

    auto found = std::find_if(droppedAliases.begin(), droppedAliases.end(), [alias] (Utf8String dgnv8) ->bool { return dgnv8.EqualsIAscii(alias); });
    if (found != droppedAliases.end())
        return SchemaReadStatus::Success;

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
                LOG.warningv("Invalid ECSchemaXML: The ECClass '%s:%s' (%d) has an invalid base class '%s:%s' (%d) but their types differ.  The base class will not be added.",
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
SchemaWriteStatus ECClass::_WriteXml (BeXmlWriterR xmlWriter, ECVersion ecXmlVersion, Utf8CP elementName, bmap<Utf8CP, Utf8CP>* additionalAttributes, bool doElementEnd) const
    {
    SchemaWriteStatus status = SchemaWriteStatus::Success;
    // No need to check here if comments need to be preserved. If they're not preserved m_xmlComments will be empty
    for (auto comment : m_xmlComments)
        xmlWriter.WriteComment(comment.c_str());

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
        xmlWriter.WriteAttribute(MODIFIER_ATTRIBUTE, SchemaParseUtils::ModifierToXmlString(m_modifier));

    if (nullptr != additionalAttributes)
        {
        for (bmap<Utf8CP, Utf8CP>::iterator iter = additionalAttributes->begin(); iter != additionalAttributes->end(); ++iter)
            xmlWriter.WriteAttribute(iter->first, iter->second);
        }
    
    for (const ECClassP& baseClass: m_baseClasses)
        {
        auto comments = m_contentXmlComments.find(ECXML_BASE_CLASS_ELEMENT);
        if (comments != m_contentXmlComments.end())
            {
            for (auto comment : comments->second)
                xmlWriter.WriteComment(comment.c_str());
            }

        xmlWriter.WriteElementStart(ECXML_BASE_CLASS_ELEMENT);
        xmlWriter.WriteText((ECClass::GetQualifiedClassName(GetSchema(), *baseClass)).c_str());
        xmlWriter.WriteElementEnd();
        }

    auto comments = m_contentXmlComments.find(ECXML_CUSTOM_ATTRIBUTES_ELEMENT);
    if (comments != m_contentXmlComments.end())
        {
        for (auto comment : comments->second)
            xmlWriter.WriteComment(comment.c_str());
        }

    WriteCustomAttributes(xmlWriter, ecXmlVersion);
            
    for (ECPropertyP prop: GetProperties(false))
        { 
        auto comments = m_contentXmlComments.find(prop->GetName());

        if (comments != m_contentXmlComments.end())
            {
            for (auto comment : comments->second)
                xmlWriter.WriteComment(comment.c_str());
            }

        prop->_WriteXml (xmlWriter, ecXmlVersion);
        }

    if (doElementEnd)
        xmlWriter.WriteElementEnd();
    return status;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Victor.Cushman              11/2017
//---------------+---------------+---------------+---------------+---------------+-------
bool ECClass::_ToJson(Json::Value& outValue, bool standalone, bool includeSchemaVersion, bool includeInheritedProperties) const
    {
    return _ToJson(outValue, standalone, includeSchemaVersion, includeInheritedProperties, bvector<bpair<Utf8String, Json::Value>>());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Victor.Cushman              11/2017
//---------------+---------------+---------------+---------------+---------------+-------
bool ECClass::_ToJson(Json::Value& outValue, bool standalone, bool includeSchemaVersion, bool includeInheritedProperties, bvector<bpair<Utf8String, Json::Value>> additionalAttributes) const
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
        outValue[MODIFIER_ATTRIBUTE] = SchemaParseUtils::ModifierToJsonString(modifier);

    if (HasBaseClasses() && !IsEntityClass()) // Entity class assigns base class in its _ToJson method
        {
        auto& baseClasses = GetBaseClasses();
        if (0 != baseClasses.size())
            outValue[ECJSON_BASE_CLASS_ELEMENT] = ECJsonUtilities::FormatClassName(*(baseClasses.at(0)));
        }

    if (GetPropertyCount(includeInheritedProperties))
        {
        auto& propertiesArr = outValue[ECJSON_SCHEMA_ITEM_PROPERTIES_ATTRIBUTE] = Json::Value(Json::ValueType::arrayValue);
        for (const auto& prop : GetProperties(includeInheritedProperties))
            {
            Json::Value propJson(Json::ValueType::objectValue);
            if (!prop->_ToJson(propJson, includeInheritedProperties && !ECClass::ClassesAreEqualByName(this, &prop->GetClass())))
                return false;
            propertiesArr.append(propJson);
            }
        }

    // Mixin attributes are treated different from other custom attributes in that the mixin already specifies that
    // it's a mixin with its schema item type, so there is no reason to duplicate the custom attribute. Rather than
    // handle the logic somewhere up the inheritance hierarchy and overcomplicate everything it's easiest to just
    // handle the specific case here.
    Json::Value customAttributesArr;
    WriteCustomAttributes(customAttributesArr);
    if (IsEntityClass() && GetEntityClassCP()->IsMixin())
        {
        // Remove the CoreCustomAttributes.IsMixin custom attribute.
        for (Json::ArrayIndex i = 0; i < customAttributesArr.size(); ++i)
            {
            if (customAttributesArr[i][ECJsonSystemNames::ClassName()] == "CoreCustomAttributes.IsMixin")
                {
                customAttributesArr.removeIndex(i);
                break;
                }
            }
        }
    if (!customAttributesArr.empty())
        outValue[ECJSON_CUSTOM_ATTRIBUTES_ELEMENT] = customAttributesArr;

    for (auto const& attribute : additionalAttributes)
        outValue[attribute.first] = attribute.second;

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
* @bsimethod                                    Carole.MacDonald                06/2010
+---------------+---------------+---------------+---------------+---------------+------*/
void ECClass::_GetBaseContainers(bvector<IECCustomAttributeContainerP>& returnList) const
    {
    for (ECClassP baseClass: m_baseClasses)
        returnList.push_back(baseClass);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   03/13
+---------------+---------------+---------------+---------------+---------------+------*/
size_t ECClass::GetPropertyCount (bool includeBaseClasses) const
    {
    if (!includeBaseClasses || !HasBaseClasses())
        return m_propertyList.size();

    if (m_propertyCountCached)
        return m_propertyCount;

    PropertyList props;
    GetProperties(true, &props);
    m_propertyCount = (uint16_t)props.size();
    m_propertyCountCached = true;

    return m_propertyCount;
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
// @bsimethod                                   Carole.MacDonald            11/2015
//---------------+---------------+---------------+---------------+---------------+-------
SchemaWriteStatus ECEntityClass::_WriteXml(BeXmlWriterR xmlWriter, ECVersion ecXmlVersion) const
    {
    if (ECVersion::V2_0 == ecXmlVersion)
        return T_Super::_WriteXml(xmlWriter, ecXmlVersion);
    else
        return T_Super::_WriteXml(xmlWriter, ecXmlVersion, ECXML_ENTITYCLASS_ELEMENT, nullptr, true);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Victor.Cushman              11/2017
//---------------+---------------+---------------+---------------+---------------+-------
bool ECEntityClass::_ToJson(Json::Value& outValue, bool standalone, bool includeSchemaVersion, bool includeInheritedProperties) const
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
// @bsimethod                                   Colin.Kerr                  12/2015
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
// @bsimethod                                   Colin.Kerr                  02/2017
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
// @bsimethod                                   Caleb.Shafer                01/2017
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
// @bsimethod                                   Colin.Kerr                  03/2017
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
// @bsimethod                                   Carole.MacDonald            11/2015
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
// @bsimethod                                   Victor.Cushman              11/2017
//---------------+---------------+---------------+---------------+---------------+-------
bool ECCustomAttributeClass::_ToJson(Json::Value & outValue, bool standalone, bool includeSchemaVersion, bool includeInheritedProperties) const
    {
    bvector<bpair<Utf8String, Json::Value>> attributes;
    attributes.push_back(bpair<Utf8String, Json::Value>(CUSTOM_ATTRIBUTE_APPLIES_TO_ATTRIBUTE, SchemaParseUtils::ContainerTypeToString(m_containerType)));
    return T_Super::_ToJson(outValue, standalone, includeSchemaVersion, includeInheritedProperties, attributes);
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
    if (BEXML_Success == classNode.GetAttributeStringValue(appliesTo, CUSTOM_ATTRIBUTE_APPLIES_TO_ATTRIBUTE))
        {
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
// @bsimethod                                   Carole.MacDonald            11/2015
//---------------+---------------+---------------+---------------+---------------+-------
SchemaWriteStatus ECStructClass::_WriteXml(BeXmlWriterR xmlWriter, ECVersion ecXmlVersion) const
    {
    if (ECVersion::V2_0 == ecXmlVersion)
        return T_Super::_WriteXml(xmlWriter, ecXmlVersion);
    else
        return T_Super::_WriteXml(xmlWriter, ecXmlVersion, ECXML_STRUCTCLASS_ELEMENT, nullptr, true);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Carole.MacDonald                04/2010
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
* @bsimethod                                                    Paul.Connelly   03/14
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

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    
* If lhs is equal to rhs,     return 0
* If lhs is greater than rhs, return -1
* If lhs is less than rhs,    return 1
+---------------+---------------+---------------+---------------+---------------+------*/
int RelationshipMultiplicity::Compare(RelationshipMultiplicity const& lhs, RelationshipMultiplicity const& rhs)
    {
    if (lhs.GetLowerLimit() == rhs.GetLowerLimit() && 
        lhs.GetUpperLimit() == rhs.GetUpperLimit())
        return 0;

    return (lhs.GetLowerLimit() > rhs.GetLowerLimit() || rhs.GetUpperLimit() > lhs.GetUpperLimit()) ? 1 : -1;
    }
  
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Carole.MacDonald                03/2010
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String RelationshipMultiplicity::ToString() const
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
RelationshipMultiplicityCR RelationshipMultiplicity::ZeroOne()
    {
    return s_zeroOneMultiplicity;
    }
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Carole.MacDonald                03/2010
+---------------+---------------+---------------+---------------+---------------+------*/
RelationshipMultiplicityCR RelationshipMultiplicity::ZeroMany()
    {
    return s_zeroManyMultiplicity;
    }
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Carole.MacDonald                03/2010
+---------------+---------------+---------------+---------------+---------------+------*/
RelationshipMultiplicityCR RelationshipMultiplicity::OneOne()
    {
    return s_oneOneMultiplicity;
    }
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Carole.MacDonald                03/2010
+---------------+---------------+---------------+---------------+---------------+------*/
RelationshipMultiplicityCR RelationshipMultiplicity::OneMany()
    {
    return s_oneManyMultiplicity;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Caleb.Shafer    07/2017
//---------------+---------------+---------------+---------------+---------------+-------
ECRelationshipConstraint::ECRelationshipConstraint(ECRelationshipClassP relationshipClass, bool isSource, bool verify)
    : m_isSource(isSource), m_verify(verify), m_relClass(relationshipClass), m_multiplicity(&s_zeroOneMultiplicity), m_isPolymorphic(true),
    m_abstractConstraint(nullptr), m_verified(false) {}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Carole.MacDonald                03/2010
+---------------+---------------+---------------+---------------+---------------+------*/
ECRelationshipConstraint::~ECRelationshipConstraint()
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

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            11/2017
//---------------+---------------+---------------+---------------+---------------+-------
Utf8CP ECRelationshipConstraint::_GetContainerName() const
    {
    return Utf8String(Utf8String(m_relClass->GetFullName()) + ":" + GetRoleLabel()).c_str();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Caleb.Shafer    09/2016
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
// @bsimethod                                                    Caleb.Shafer    09/2016
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

    if (RelationshipMultiplicity::Compare(GetMultiplicity(), baseConstraint.GetMultiplicity()) == -1)
        {
        LOG.errorv("Multiplicity Violation: The Multiplicity (%" PRIu32 "..%" PRIu32 ") of the %s-constraint on %s is larger than the Multiplicity of it's base class %s (%" PRIu32 "..%" PRIu32 ")",
                   GetMultiplicity().GetLowerLimit(), GetMultiplicity().GetUpperLimit(), (m_isSource) ? ECXML_SOURCECONSTRAINT_ELEMENT : ECXML_TARGETCONSTRAINT_ELEMENT, m_relClass->GetFullName(),
                   baseConstraint.GetRelationshipClass().GetFullName(), baseConstraint.GetMultiplicity().GetLowerLimit(), baseConstraint.GetMultiplicity().GetUpperLimit());
        return ECObjectsStatus::BaseClassUnacceptable;
        }

    return ECObjectsStatus::Success;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Caleb.Shafer    10/2016
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
// @bsimethod                                                    Caleb.Shafer    10/2016
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
                LOG.infov("Attempting to find a common base class between all constraint classes to use as the abstract constraint...");

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
            LOG.messagev(resolveIssues ? NativeLogging::SEVERITY::LOG_WARNING : NativeLogging::SEVERITY::LOG_ERROR,
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
* @bsimethod                                    Caleb.Shafer                08/2016
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

        // Checks the multiplicity: 0 = equal multiplicity, -1 = leftside is bigger, 1 = rightside is bigger
        // since the left side is the current multiplicity and the right side the base class, it is expected the baseclass is bigger.
        if (RelationshipMultiplicity::Compare(RelationshipMultiplicity(lowerLimit, upperLimit), baseClassConstraint->GetMultiplicity()) == -1)
            {
            resolveIssues &= m_relClass->GetSchema().OriginalECXmlVersionLessThan(ECVersion::V3_0);
            LOG.messagev(resolveIssues? NativeLogging::SEVERITY::LOG_WARNING : NativeLogging::SEVERITY::LOG_ERROR,
                "Multiplicity Violation: The multiplicity (%" PRIu32 "..%" PRIu32 ") of the %s-constraint on %s is larger than the Multiplicity of its base class %s (%" PRIu32 "..%" PRIu32 ")",
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
                LOG.warningv("The Multiplicity of %s's base class, %s, has been changed from (%" PRIu32 "..%" PRIu32 ") to (%" PRIu32 "..%" PRIu32 ") to conform to new relationship constraint rules.",
                             m_relClass->GetFullName(), relationshipBaseClass->GetFullName(),
                             baseClassConstraint->GetMultiplicity().GetLowerLimit(), baseClassConstraint->GetMultiplicity().GetUpperLimit(), lowerLimit, upperLimit);
                baseClassConstraint->SetMultiplicity(lowerLimit, upperLimit);
                }
            }
        }

    return ECObjectsStatus::Success;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Caleb.Shafer    09/2016
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
* @bsimethod                                    Carole.MacDonald                03/2010
+---------------+---------------+---------------+---------------+---------------+------*/
SchemaReadStatus ECRelationshipConstraint::ReadXml (BeXmlNodeR constraintNode, ECSchemaReadContextR schemaContext)
    {
    SchemaReadStatus status = SchemaReadStatus::Success;

    Utf8String value;  // needed for macros.
    if (m_relClass->GetSchema().OriginalECXmlVersionAtLeast(ECVersion::V3_1))
        {
        READ_REQUIRED_XML_ATTRIBUTE(constraintNode, POLYMORPHIC_ATTRIBUTE, this, IsPolymorphic, constraintNode.GetName());
        READ_REQUIRED_XML_ATTRIBUTE(constraintNode, ROLELABEL_ATTRIBUTE, this, RoleLabel, constraintNode.GetName());

        Utf8String abstractConstraint;
        if (BEXML_Success == constraintNode.GetAttributeStringValue(abstractConstraint, ABSTRACTCONSTRAINT_ATTRIBUTE) && Utf8String::IsNullOrEmpty(abstractConstraint.c_str()))
            {
            LOG.errorv("Invalid ECSchemaXML: The ECRelationshipClass, %s, cannot have an empty %s attribute.", m_relClass->GetFullName(), MULTIPLICITY_ATTRIBUTE);
            return SchemaReadStatus::InvalidECSchemaXml;
            }

        if (!Utf8String::IsNullOrEmpty(abstractConstraint.c_str()))
            SetAbstractConstraint(abstractConstraint.c_str(), false);

        Utf8String multiplicity;
        if (BEXML_Success != constraintNode.GetAttributeStringValue(multiplicity, MULTIPLICITY_ATTRIBUTE) || Utf8String::IsNullOrEmpty(multiplicity.c_str()))
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
        READ_OPTIONAL_XML_ATTRIBUTE_IGNORING_SET_ERRORS(constraintNode, POLYMORPHIC_ATTRIBUTE, this, IsPolymorphic);
        READ_OPTIONAL_XML_ATTRIBUTE_IGNORING_SET_ERRORS(constraintNode, ROLELABEL_ATTRIBUTE, this, RoleLabel);
        READ_OPTIONAL_XML_ATTRIBUTE(constraintNode, CARDINALITY_ATTRIBUTE, this, Cardinality);
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
            LOG.errorv("Invalid ECSchemaXML: The %s ECRelationshipConstraint on ECRelationship %s contains a %s attribute with the value '%s' that can not be parsed.",
                m_isSource ? "Source" : "Target", m_relClass->GetFullName(), CONSTRAINTCLASSNAME_ATTRIBUTE, constraintClassName.c_str());
            return SchemaReadStatus::InvalidECSchemaXml;
            }
        
        ECSchemaCP resolvedSchema = m_relClass->GetSchema().GetSchemaByAliasP (alias);
        if (nullptr == resolvedSchema)
            {
            LOG.errorv("Invalid ECSchemaXML: %s ECRelationshipConstraint on ECRelationship %s contains a %s attribute with the alias '%s' that can not be resolved to a referenced schema.",
                m_isSource ? "Source" : "Target", m_relClass->GetFullName(), CONSTRAINTCLASSNAME_ATTRIBUTE, alias.c_str());
            return SchemaReadStatus::InvalidECSchemaXml;
            }

        ECClassCP constraintClass = resolvedSchema->GetClassCP (className.c_str());
        if (nullptr == constraintClass)
            {
            LOG.errorv("Invalid ECSchemaXML: The %s ECRelationshipConstraint on ECRelationship %s contains a %s attribute with the value '%s' that can not be resolved to an ECClass named '%s' in the ECSchema '%s'",
                m_isSource ? "Source" : "Target", m_relClass->GetFullName(), CONSTRAINTCLASSNAME_ATTRIBUTE, constraintClassName.c_str(), className.c_str(), resolvedSchema->GetName().c_str());
            return SchemaReadStatus::InvalidECSchemaXml;
            }
        
        if (!constraintClass->IsEntityClass() && !constraintClass->IsRelationshipClass())
            {
            if (2 == m_relClass->GetSchema().GetOriginalECXmlVersionMajor())
                {
                LOG.warningv("Invalid ECSchemaXML: The %s ECRelationshipConstraint on %s contains a %s attribute with the value '%s' that does not resolve to an ECEntityClass or ECRelationshipClass named '%s' in the ECSchema '%s'.  The constraint class will be ignored.", 
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

        for (BeXmlNodeP keyNode = constraintClassNode->GetFirstChild(); nullptr != keyNode; keyNode = keyNode->GetNextSibling())
            {
            if (0 == strcmp(keyNode->GetName(), EC_CONSTRAINTKEY_ELEMENT))
                {
                if (m_relClass->GetSchema().OriginalECXmlVersionLessThan(ECVersion::V3_1))
                    {
                    LOG.warningv("Key properties are no longer supported on constraint classes. All key properties have been dropped from the constraint class '%s' on the %s-Constraint of relationship '%s'.",
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
        }

    return status;
    }
    
//---------------------------------------------------------------------------------------
// @bsimethod                                   Victor.Cushman              11/2017
//---------------+---------------+---------------+---------------+---------------+-------
bool ECRelationshipConstraint::ToJson(Json::Value& outValue)
    {
    outValue = Json::Value(Json::ValueType::objectValue);
    outValue[MULTIPLICITY_ATTRIBUTE] = GetMultiplicity().ToString();
    outValue[ROLELABEL_ATTRIBUTE] = GetInvariantRoleLabel();
    outValue[POLYMORPHIC_ATTRIBUTE] = GetIsPolymorphic();

    if (nullptr != m_abstractConstraint)
        outValue[ABSTRACTCONSTRAINT_ATTRIBUTE] = ECJsonUtilities::FormatClassName(*GetAbstractConstraint());

    Json::Value customAttributesArr;
    if (!WriteCustomAttributes(customAttributesArr))
        return false;
    if (!customAttributesArr.empty())
        outValue[ECJSON_CUSTOM_ATTRIBUTES_ELEMENT] = customAttributesArr;

    auto const& constraintClasses = GetConstraintClasses();
    if (constraintClasses.size() != 0)
        {
        Json::Value constraintClassArr(Json::ValueType::arrayValue);
        for (auto const constraintClass : constraintClasses)
            constraintClassArr.append(ECJsonUtilities::FormatClassName(*constraintClass));
        outValue[ECJSON_CONSTRAINT_CLASSES] = constraintClassArr;
        }

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Carole.MacDonald                03/2010
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
// @bsimethod                                   Caleb.Shafer                  09/2016
//---------------+---------------+---------------+---------------+---------------+-------
ECObjectsStatus ECRelationshipConstraint::SetAbstractConstraint(Utf8CP abstractConstraint) {return SetAbstractConstraint(abstractConstraint, m_verify);}

//---------------------------------------------------------------------------------------
// @bsimethod                                   Caleb.Shafer                  09/2016
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
// @bsimethod                                   Caleb.Shafer                  06/2017
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
// @bsimethod                                   Caleb.Shafer                  09/2016
//---------------+---------------+---------------+---------------+---------------+-------
ECObjectsStatus ECRelationshipConstraint::SetAbstractConstraint(ECEntityClassCR abstractConstraint) {return SetAbstractConstraint((ECClassCR)abstractConstraint);}
ECObjectsStatus ECRelationshipConstraint::SetAbstractConstraint(ECRelationshipClassCR abstractConstraint) {return SetAbstractConstraint((ECClassCR)abstractConstraint);}

//---------------------------------------------------------------------------------------
// @bsimethod                                   Caleb.Shafer                  09/2016
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
// @bsimethod                                   Caleb.Shafer                  06/2017
//---------------+---------------+---------------+---------------+---------------+-------
ECObjectsStatus ECRelationshipConstraint::AddClass(ECRelationshipClassCR classConstraint) {return AddClass((ECClassCR)classConstraint);}
ECObjectsStatus ECRelationshipConstraint::AddClass(ECEntityClassCR classConstraint) {return AddClass((ECClassCR)classConstraint);}

//---------------------------------------------------------------------------------------
// @bsimethod                                   Caleb.Shafer                  06/2017
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
// @bsimethod                                   Caleb.Shafer                  06/2017
//---------------+---------------+---------------+---------------+---------------+-------
ECObjectsStatus ECRelationshipConstraint::RemoveClass(ECEntityClassCR classConstraint) {return RemoveClass((ECClassCR)classConstraint);}
ECObjectsStatus ECRelationshipConstraint::RemoveClass(ECRelationshipClassCR classConstraint) { return RemoveClass((ECClassCR)classConstraint);}

//---------------------------------------------------------------------------------------
// @bsimethod                                   Caleb.Shafer                  06/2017
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
// @bsimethod                                   Colin.Kerr                  12/2015
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
* @bsimethod                                    Carole.MacDonald                03/2010
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
// @bsimethod                                   Caleb.Shafer                  06/2017
//---------------+---------------+---------------+---------------+---------------+-------
ECObjectsStatus ECRelationshipConstraint::SetMultiplicity(Utf8CP multiplicity) {return SetMultiplicity(multiplicity, m_verify);}

//---------------------------------------------------------------------------------------
// @bsimethod                                   Caleb.Shafer                  08/2016
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
// @bsimethod                                   Victor.Cushman                  12/2017
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
* @bsimethod                                    Carole.MacDonald                03/2010
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
ECObjectsStatus ECRelationshipConstraint::SetRoleLabel (Utf8CP value)
    {
    if (Utf8String::IsNullOrEmpty(value))
        return ECObjectsStatus::Error;
    m_roleLabel = value;
    return ECObjectsStatus::Success;
    }
  
  /*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Carole.MacDonald                05/2012
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
        ECClassCP sourceAbstractConstraint = GetAbstractConstraint();
        if (_GetContainerSchema()->GetSchemaKey() != sourceAbstractConstraint->GetSchema().GetSchemaKey())
            status = toRelationshipConstraint.SetAbstractConstraint(*sourceAbstractConstraint);
        else
            {
            ECClassP destAbstractConstraint = destSchema->GetClassP(sourceAbstractConstraint->GetName().c_str());
            if (nullptr == destAbstractConstraint)
                {
                if (copyReferences)
                    {
                    status = destSchema->CopyClass(destAbstractConstraint, *sourceAbstractConstraint, sourceAbstractConstraint->GetName(), copyReferences);
                    if (ECObjectsStatus::Success != status)
                        return status;
                    status = toRelationshipConstraint.SetAbstractConstraint(*destAbstractConstraint);
                    }
                else
                    {
                    if (!ECSchema::IsSchemaReferenced(*destSchema, sourceAbstractConstraint->GetSchema()))
                        if (ECObjectsStatus::Success != (status = destSchema->AddReferencedSchema(m_relClass->GetSchemaR())))
                            return status;
                    status = toRelationshipConstraint.SetAbstractConstraint(*sourceAbstractConstraint);
                    }
                }
            else
                status = toRelationshipConstraint.SetAbstractConstraint(*destAbstractConstraint);
            }
        
        if (ECObjectsStatus::Success != status)
            return status;    
        }

    for (auto constraintClass : GetConstraintClasses())
        {
        if (_GetContainerSchema()->GetSchemaKey() != constraintClass->GetSchema().GetSchemaKey())
            status = toRelationshipConstraint.AddClass(*constraintClass);
        else
            {
            ECClassP destConstraintClass = destSchema->GetClassP(constraintClass->GetName().c_str());
            if (nullptr == destConstraintClass)
                {
                if (copyReferences)
                    {
                    status = destSchema->CopyClass(destConstraintClass, *constraintClass, constraintClass->GetName(), copyReferences);
                    if (ECObjectsStatus::Success != status)
                        return status;
                    status = toRelationshipConstraint.AddClass(*destConstraintClass);
                    }
                else
                    {
                    if (!ECSchema::IsSchemaReferenced(*destSchema, constraintClass->GetSchema()))
                        if (ECObjectsStatus::Success != (status = destSchema->AddReferencedSchema(m_relClass->GetSchemaR())))
                            return status;
                    status = toRelationshipConstraint.AddClass(*constraintClass);
                    }
                }
            else
                status = toRelationshipConstraint.AddClass(*destConstraintClass);
            }
        
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
ECObjectsStatus ECRelationshipClass::SetStrength(Utf8CP strength)
    {
    PRECONDITION (nullptr != strength, ECObjectsStatus::PreconditionViolated);

    StrengthType strengthType;
    ECObjectsStatus status = SchemaParseUtils::ParseStrengthType(strengthType, strength);
    if (ECObjectsStatus::Success == status)
        SetStrength(strengthType);
    else if (GetSchema().OriginalECXmlVersionGreaterThan(ECVersion::Latest))
        {
        LOG.warningv("ECRelationshipClass '%s' has an unknown Strength type '%s'. Setting to 'Referencing'", GetFullName(), strength);
        return SetStrength(StrengthType::Referencing); // Default if the ECVersion is greater than the latest known version. Return so error status is not returned.
        }
    else
        LOG.errorv ("Failed to parse the Strength string '%s' for ECRelationshipClass '%s'.", strength, this->GetName().c_str());

    return status;
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
SchemaWriteStatus ECRelationshipClass::_WriteXml (BeXmlWriterR xmlWriter, ECVersion ecXmlVersion) const
    {
    SchemaWriteStatus   status;
    bmap<Utf8CP, Utf8CP> additionalAttributes;
    additionalAttributes[STRENGTH_ATTRIBUTE] = SchemaParseUtils::StrengthToString(m_strength);
    if (m_strengthDirection != ECRelatedInstanceDirection::Forward)
        { //skip the attribute for "forward" as it is the default value.
        additionalAttributes[STRENGTHDIRECTION_ATTRIBUTE] = SchemaParseUtils::DirectionToString(m_strengthDirection);
        }

    if (SchemaWriteStatus::Success != (status = ECClass::_WriteXml (xmlWriter, ecXmlVersion, ECXML_RELATIONSHIP_CLASS_ELEMENT, &additionalAttributes, false)))
        return status;
        
    // verify that this really is the current relationship class element // CGM 07/15 - Can't do this with an XmlWriter
    //if (0 != strcmp (classNode->GetName(), EC_RELATIONSHIP_CLASS_ELEMENT))
    //    {
    //    BeAssert (false);
    //    return SchemaWriteStatus::FailedToCreateXml;
    //    }
        
    m_source->WriteXml (xmlWriter, ECXML_SOURCECONSTRAINT_ELEMENT, ecXmlVersion);
    m_target->WriteXml (xmlWriter, ECXML_TARGETCONSTRAINT_ELEMENT, ecXmlVersion);
    xmlWriter.WriteElementEnd();

    return status;
    }
    
//---------------------------------------------------------------------------------------
// @bsimethod                                   Victor.Cushman              11/2017
//---------------+---------------+---------------+---------------+---------------+-------
bool ECRelationshipClass::_ToJson(Json::Value& outValue, bool standalone, bool includeSchemaVersion, bool includeInheritedProperties) const
    {
    bvector<bpair<Utf8String, Json::Value>> attributes;

    attributes.push_back(bpair<Utf8String, Json::Value>(STRENGTH_ATTRIBUTE, SchemaParseUtils::StrengthToString(GetStrength())));
    attributes.push_back(bpair<Utf8String, Json::Value>(STRENGTHDIRECTION_ATTRIBUTE, SchemaParseUtils::DirectionToString(GetStrengthDirection())));

    Json::Value sourceJson;
    if (!GetSource().ToJson(sourceJson))
        return false;
    attributes.push_back(bpair<Utf8String, Json::Value>(ECJSON_SOURCECONSTRAINT_ELEMENT, sourceJson));

    Json::Value targetJson;
    if (!GetTarget().ToJson(targetJson))
        return false;
    attributes.push_back(bpair<Utf8String, Json::Value>(ECJSON_TARGETCONSTRAINT_ELEMENT, targetJson));

    return T_Super::_ToJson(outValue, standalone, includeSchemaVersion, includeInheritedProperties, attributes);
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
SchemaReadStatus ECRelationshipClass::_ReadXmlContents (BeXmlNodeR classNode, ECSchemaReadContextR context, ECSchemaCP conversionSchema, bvector<Utf8String>& droppedAliases, bvector<NavigationECPropertyP>& navigationProperties)
    {
    SchemaReadStatus status = T_Super::_ReadXmlContents (classNode, context, conversionSchema, droppedAliases, navigationProperties);
    if (status != SchemaReadStatus::Success)
        return status;

    BeXmlNodeP sourceNode = classNode.SelectSingleNode (EC_NAMESPACE_PREFIX ":" ECXML_SOURCECONSTRAINT_ELEMENT);
    if (nullptr != sourceNode)
        status = m_source->ReadXml (*sourceNode, context);
    if (status != SchemaReadStatus::Success)
        return status;
    
    BeXmlNodeP  targetNode = classNode.SelectSingleNode (EC_NAMESPACE_PREFIX ":" ECXML_TARGETCONSTRAINT_ELEMENT);
    if (nullptr != targetNode)
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
            LOG.warningv("Multiple base classes for relationship classes are not supported.  Replacing base class '%s' with '%s' for RelationshipClass '%s'",
                          originalBase->GetFullName(), baseClass.GetFullName(), GetFullName());
            RemoveBaseClass(*originalBase);
            }
        }

    if (HasBaseClasses())
        return ECObjectsStatus::RelationshipAlreadyHasBaseClass;

    return ECClass::_AddBaseClass(baseClass, insertAtBeginning, resolveConflicts, validate);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Caleb.Shafer    10/2016
//---------------+---------------+---------------+---------------+---------------+-------
bool ECRelationshipClass::GetIsVerified()
    {
    if (!m_source->m_verified || !m_target->m_verified)
        m_verified = false;

    return m_verified;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Caleb.Shafer    10/2016
//---------------+---------------+---------------+---------------+---------------+-------
bool ECRelationshipClass::Verify() const
    {
    m_verified = Verify(false);
    return m_verified;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Caleb.Shafer    09/2016
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
                            GetFullName(), SchemaParseUtils::StrengthToString(value), relationshipBaseClass->GetFullName(), SchemaParseUtils::StrengthToString(relationshipBaseClass->GetStrength()));
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
                            GetFullName(), SchemaParseUtils::DirectionToString(value), relationshipBaseClass->GetFullName(), SchemaParseUtils::DirectionToString(relationshipBaseClass->GetStrengthDirection()));
                return false;
                }
            }
        }

    return (!compareValue || GetStrengthDirection() == value);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Colin.Kerr                  12/2015
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
