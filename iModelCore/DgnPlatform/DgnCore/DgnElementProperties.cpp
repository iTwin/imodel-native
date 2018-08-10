/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnCore/DgnElementProperties.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <DgnPlatformInternal.h>
#include "BisCoreNames.h"
#include "ElementECInstanceAdapter.h"

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson      10/16
+---------------+---------------+---------------+---------------+---------------+------*/
static ECObjectsStatus toECObjectsStatus(DgnDbStatus status)
    {
    switch (status)
        {
        case DgnDbStatus::Success:
            return ECObjectsStatus::Success;
        }

    return ECObjectsStatus::Error;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson      10/16
+---------------+---------------+---------------+---------------+---------------+------*/
static DgnDbStatus toDgnDbWriteStatus(ECObjectsStatus status)
    {
    switch (status)
        {
        case ECObjectsStatus::PropertyValueMatchesNoChange:
        case ECObjectsStatus::Success: 
            return DgnDbStatus::Success;
        }

    return DgnDbStatus::WriteError;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson      10/16
+---------------+---------------+---------------+---------------+---------------+------*/
ElementECInstanceAdapter::ElementECInstanceAdapter(DgnElementR el) 
    : m_element(el), m_eclass(el.GetElementClass()), m_readOnly(false)
    {
    AddRef(); // protect against somebody else doing AddRef + Release and deleting this
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson      10/16
+---------------+---------------+---------------+---------------+---------------+------*/
ElementECInstanceAdapter::ElementECInstanceAdapter(DgnElementCR el) 
    : m_element(const_cast<DgnElement&>(el)), m_eclass(el.GetElementClass()), m_readOnly(true)
    {
    AddRef(); // protect against somebody else doing AddRef + Release and deleting this
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson      10/16
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus ElementECInstanceAdapter::_GetValue(ECValueR v, uint32_t propertyIndex, bool useArrayIndex, uint32_t arrayIndex) const 
    {
    PropertyArrayIndex ai(useArrayIndex, arrayIndex);
    auto stat = m_element.GetPropertyValue(v, propertyIndex, ai);
    return toECObjectsStatus(stat);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson      10/16
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus ElementECInstanceAdapter::_SetValue(uint32_t propertyIndex, ECValueCR v, bool useArrayIndex, uint32_t arrayIndex)
    {
    if (m_readOnly)
        {
        BeAssert(false);
        return ECObjectsStatus::UnableToSetReadOnlyInstance;
        }
    PropertyArrayIndex ai(useArrayIndex, arrayIndex);
    auto stat = m_element.SetPropertyValue(propertyIndex, v, ai);
    return toECObjectsStatus(stat);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson      10/16
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus ElementECInstanceAdapter::_InsertArrayElements(uint32_t propertyIndex, uint32_t index, uint32_t size)
    {
    if (m_readOnly)
        {
        BeAssert(false);
        return ECObjectsStatus::UnableToSetReadOnlyInstance;
        }
    // Only auto-handled properties can be arrays
    ElementAutoHandledPropertiesECInstanceAdapter autoHandledProps(m_element, true);
    return autoHandledProps.InsertArrayElements(propertyIndex, index, size);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson      10/16
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus ElementECInstanceAdapter::_AddArrayElements(uint32_t propertyIndex, uint32_t size)
    {
    if (m_readOnly)
        {
        BeAssert(false);
        return ECObjectsStatus::UnableToSetReadOnlyInstance;
        }
    // Only auto-handled properties can be arrays
    ElementAutoHandledPropertiesECInstanceAdapter autoHandledProps(m_element, true);
    return autoHandledProps.AddArrayElements(propertyIndex, size);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson      10/16
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus ElementECInstanceAdapter::_RemoveArrayElement(uint32_t propertyIndex, uint32_t index)
    {
    if (m_readOnly)
        {
        BeAssert(false);
        return ECObjectsStatus::UnableToSetReadOnlyInstance;
        }
    // Only auto-handled properties can be arrays
    ElementAutoHandledPropertiesECInstanceAdapter autoHandledProps(m_element, true);
    return autoHandledProps.RemoveArrayElement(propertyIndex, index);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson      10/16
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus ElementECInstanceAdapter::_ClearArray(uint32_t propIdx)
    {
    if (m_readOnly)
        {
        BeAssert(false);
        return ECObjectsStatus::UnableToSetReadOnlyInstance;
        }
    // Only auto-handled properties can be arrays
    ElementAutoHandledPropertiesECInstanceAdapter autoHandledProps(m_element, true);
    return autoHandledProps.ClearArray(propIdx);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      08/16
+---------------+---------------+---------------+---------------+---------------+------*/
static bool isWriteOnlyProperty(Utf8StringCR propName)
    {
    static std::once_flag s_ignoreListOnceFlag;
    static bset<Utf8String>* s_ignoreList;
    std::call_once(s_ignoreListOnceFlag, []()
        {
        s_ignoreList = new bset<Utf8String>();
        s_ignoreList->insert("LastMod");
        });
    return s_ignoreList->find(propName) != s_ignoreList->end();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson      10/16
+---------------+---------------+---------------+---------------+---------------+------*/
bool DgnElement::SetPropertyFilter::IsBootStrappingProperty(Utf8StringCR propName)
    {
    static std::once_flag s_ignoreListOnceFlag;
    static bset<Utf8String>* s_ignoreList;
    std::call_once(s_ignoreListOnceFlag, []()
        {
        s_ignoreList = new bset<Utf8String>();
        s_ignoreList->insert("Id");
        s_ignoreList->insert(BIS_ELEMENT_PROP_ECInstanceId);
        s_ignoreList->insert(BIS_ELEMENT_PROP_Model);
        s_ignoreList->insert(BIS_ELEMENT_PROP_CodeSpec);
        s_ignoreList->insert(BIS_ELEMENT_PROP_CodeScope);
        s_ignoreList->insert(BIS_ELEMENT_PROP_CodeValue);
        });

    return s_ignoreList->find(propName) != s_ignoreList->end();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson      10/16
+---------------+---------------+---------------+---------------+---------------+------*/
bool DgnElement::SetPropertyFilter::_ExcludeProperty(ECPropertyValueCR propValue) const
    {
    if ((0 != (m_ignore & Null)))
        {
        auto const& value = propValue.GetValue();
        if (value.IsNull() && !value.IsStruct() && !value.IsArray())
            return true;
        }

    ECValueAccessorCR accessor = propValue.GetValueAccessor();

    if (1 == accessor.GetDepth())
        {
        //if ((0 != (m_ignore & CustomHandled)) && m_element.IsCustomHandledProperty(*accessor.GetECProperty()))
        //    return true;

        auto const& propName = accessor.GetECProperty()->GetName();

        if ((0 != (m_ignore & Bootstrapping)) && IsBootStrappingProperty(propName))
            return true;

        if ((0 != (m_ignore & WriteOnly)) && isWriteOnlyProperty(propName))
            return true;

        // You cannot set any of the Geometry properties via the property API
        if (propName.Equals(GeometricElement3d::prop_InSpatialIndex()))
            return true;
        }

    if (!m_ignoreList.empty())
        {
        if (m_ignoreList.find(accessor.GetAccessString()) != m_ignoreList.end())
            return true;
        }

    return false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson      10/16
+---------------+---------------+---------------+---------------+---------------+------*/
bool DgnElement::ComparePropertyFilter::_ExcludeProperty(ECPropertyValueCR propValue) const
    {
    ECValueAccessorCR accessor = propValue.GetValueAccessor();

    if (0 == accessor.GetDepth())
        {
        auto const& propName = accessor.GetECProperty()->GetName();

        if ((0 != (m_ignore & WriteOnly)) && isWriteOnlyProperty(propName))
            return true;
        }

    if (!m_ignoreList.empty())
        {
        if (m_ignoreList.find(accessor.GetAccessString()) != m_ignoreList.end())
            return true;
        }

    return false;
    }

/*---------------------------------------------------------------------------------**//**
* Adapted from the the property-by-property copying logic of IECInstance::CopyValues.
* NB: We must call DgnElement::_SetPropertyValue. We must not use the ECDBuffer copying shortcut.
* @bsimethod                                                    Sam.Wilson      10/16
+---------------+---------------+---------------+---------------+---------------+------*/ 
DgnDbStatus ElementECInstanceAdapter::CopyPropertiesFrom(ECValuesCollectionCR source, DgnElement::SetPropertyFilter const& filter)
    {
    for (ECValuesCollection::const_iterator it=source.begin(); it != source.end(); ++it)
        {
        ECPropertyValue const& prop = *it;

        if (filter._ExcludeProperty(prop))
            continue;

        if (prop.HasChildValues())
            {
            DgnDbStatus status;
            if (DgnDbStatus::Success != (status = CopyPropertiesFrom(*prop.GetChildValues(), filter)))
                {
                if (!filter._IgnoreErrors() && DgnDbStatus::Success != status)
                    return status;
                }
            }
        else 
            {
            ECObjectsStatus ecStatus;
            if (ECObjectsStatus::Success != (ecStatus = SetInternalValueUsingAccessor(prop.GetValueAccessor(), prop.GetValue())))
                {
                if (!filter._IgnoreErrors() && ECObjectsStatus::PropertyValueMatchesNoChange != ecStatus)
                    {
                    BeDataAssert(false);
                    return toDgnDbWriteStatus(ecStatus);
                    }
                }
            }
        }

    return DgnDbStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson      10/16
+---------------+---------------+---------------+---------------+---------------+------*/
bool ElementECInstanceAdapter::ArePropertiesEqualTo(ECValuesCollectionCR expected)
    {
    for (ECPropertyValueCR expectedPropertyValue : expected)
        {
        ECValueAccessorCR valueAccessor = expectedPropertyValue.GetValueAccessor();
        
        if (expectedPropertyValue.HasChildValues())
            {
            if (!ArePropertiesEqualTo(*expectedPropertyValue.GetChildValues()))
                return false;

            continue;
            }

        ECValue actualValue;
        ECObjectsStatus status = GetValueUsingAccessor(actualValue, valueAccessor);
        if (status != ECObjectsStatus::Success)
            return expectedPropertyValue.GetValue().IsNull();

        if (!expectedPropertyValue.GetValue().Equals(actualValue))
            return false;
        }

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      08/16
+---------------+---------------+---------------+---------------+---------------+------*/
bool DgnElement::_EqualProperty(ECN::ECPropertyValueCR expected, DgnElementCR other) const
    {
    if (expected.HasChildValues())
        {
        ElementECInstanceAdapter ecThis(const_cast<DgnElement&>(*this));
        return ecThis.ArePropertiesEqualTo(*expected.GetChildValues());
        }

    ECN::ECValue value;
    auto const& propName = expected.GetValueAccessor().GetECProperty()->GetName();
    if (DgnDbStatus::Success != GetPropertyValue(value, propName.c_str()))
        return expected.GetValue().IsNull();

    return value.Equals(expected.GetValue());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      08/16
+---------------+---------------+---------------+---------------+---------------+------*/
bool DgnElement::_Equals(DgnElementCR other, DgnElement::ComparePropertyFilter const& filter) const
    {
    if (&other == this)
        return true;

    auto ecclass = GetElementClass();
    if (ecclass != other.GetElementClass())
        return false;

    // Note that ECInstanceId is not a normal property and will not be returned by the property collection below
    if (!filter._ExcludeElementId())
        {
        if (GetElementId() != other.GetElementId())
            return false;
        }

    if (ToJsonPropString() != other.ToJsonPropString())
        return false;

    ElementECInstanceAdapter ecOther(other);
    ECValuesCollection otherProperties(ecOther);
    for (auto const& otherProp : otherProperties)
        {
        if (filter._ExcludeProperty(otherProp))
            continue;

        if (!_EqualProperty(otherProp, other))
            return false;
        }

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      10/16
+---------------+---------------+---------------+---------------+---------------+------*/
bool DgnElement::Equals(DgnElementCR source, ComparePropertyFilter const& filter) const
    {
    return _Equals(source, filter);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      08/16
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnElement::_Dump(Utf8StringR str, ComparePropertyFilter const& filter) const
    {
    auto ecclass = GetElementClass();
    str.append(ecclass->GetName().c_str());
    str.append(Utf8PrintfString(" %lld {", GetElementId().GetValueUnchecked()).c_str());
    Utf8CP comma = "";
    ElementECInstanceAdapter ecThis(*this);
    ECValuesCollection propertyValues(ecThis);
    for (ECPropertyValueCR propertyValue : propertyValues)
        {
        if (filter._ExcludeProperty(propertyValue))
            continue;

        str.append(comma);
        str.append(propertyValue.GetValueAccessor().GetDebugAccessString().c_str());
        str.append("=");
        str.append(propertyValue.GetValue().ToString().c_str());
        comma = ", ";
        }
    str.append("}\n");
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      07/15
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnElement::Dump(Utf8StringR str, ComparePropertyFilter const& filter) const {_Dump(str,filter);}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson      07/16
+---------------+---------------+---------------+---------------+---------------+------*/
DgnElementPtr DgnElements::CreateElement(ECN::IECInstanceCR properties, DgnDbStatus* inStat) const
    {
    return CreateElement(properties, false, inStat);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            10/2017
//---------------+---------------+---------------+---------------+---------------+-------
DgnElementPtr DgnElements::CreateElement(ECN::IECInstanceCR properties, bool ignoreUnknownProperties, DgnDbStatus* inStat) const
    {
    DgnDbStatus ALLOW_NULL_OUTPUT(stat, inStat);

    DgnClassId classId(properties.GetClass().GetId().GetValue());
    auto handler = dgn_ElementHandler::Element::FindHandler(GetDgnDb(), classId);
    if (nullptr == handler)
        {
        BeAssert(false);
        stat = DgnDbStatus::MissingHandler;
        return nullptr;
        }

    return handler->_CreateNewElement(GetDgnDb(), properties, ignoreUnknownProperties, inStat);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson      07/16
+---------------+---------------+---------------+---------------+---------------+------*/
DgnElement::CreateParams DgnElement::InitCreateParamsFromECInstance(DgnDbR db, ECN::IECInstanceCR properties, DgnDbStatus* inStat)
    {
    DgnDbStatus ALLOW_NULL_OUTPUT(stat, inStat);

    DgnModelId modelId;
        {
        ECN::ECValue v;
        if (ECN::ECObjectsStatus::Success != properties.GetValue(v, BIS_ELEMENT_PROP_Model) || v.IsNull())
            {
            stat = DgnDbStatus::BadModel;
            return CreateParams(db, DgnModelId(), DgnClassId());
            }
        modelId = v.GetNavigationInfo().GetId<DgnModelId>();
        if (!modelId.IsValid())
            {
            stat = DgnDbStatus::BadModel;
            return CreateParams(db, DgnModelId(), DgnClassId());
            }
        }

    DgnClassId classId(properties.GetClass().GetId().GetValue());

    DgnCode code;
        {
        ECN::ECValue v;
        if (ECN::ECObjectsStatus::Success != properties.GetValue(v, BIS_ELEMENT_PROP_CodeSpec) || v.IsNull())
            {
            stat = DgnDbStatus::MissingId;
            return CreateParams(db, DgnModelId(), classId);
            }
        CodeSpecId codeSpecId = v.GetNavigationInfo().GetId<CodeSpecId>();

        if (ECN::ECObjectsStatus::Success != properties.GetValue(v, BIS_ELEMENT_PROP_CodeScope) || v.IsNull())
            {
            stat = DgnDbStatus::MissingId;
            return CreateParams(db, DgnModelId(), classId);
            }
        DgnElementId codeScopeElementId = v.GetNavigationInfo().GetId<DgnElementId>();

        if (ECN::ECObjectsStatus::Success != properties.GetValue(v, BIS_ELEMENT_PROP_CodeValue) || (!v.IsNull() && 0 == strlen(v.GetUtf8CP())))
            {
            stat = DgnDbStatus::InvalidName;
            return CreateParams(db, DgnModelId(), classId);
            }

        code = DgnCode(codeSpecId, codeScopeElementId, v.GetUtf8CP());
        }

    DgnElement::CreateParams params(db, modelId, classId, code);

    auto ecinstanceid = properties.GetInstanceId();                 // Note that ECInstanceId is not a normal property and will not be returned by the property collection below
    if (!ecinstanceid.empty())
        {
        uint64_t idvalue;
        if (BSISUCCESS != BeStringUtilities::ParseUInt64(idvalue, ecinstanceid.c_str()))
            {
            stat = DgnDbStatus::InvalidId;
            return CreateParams(db, DgnModelId(), DgnClassId());
            }
        params.SetElementId(DgnElementId(idvalue));
        }

    return params;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson      07/16
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus DgnElement::_SetPropertyValues(ECN::IECInstanceCR source, SetPropertyFilter const& filter)
    {
    ElementECInstanceAdapter ecThis(*this);
    ECValuesCollection srcValues(source);
    return ecThis.CopyPropertiesFrom(srcValues, filter);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson      07/16
+---------------+---------------+---------------+---------------+---------------+------*/
DgnElementPtr dgn_ElementHandler::Element::_CreateNewElement(DgnDbR db, ECN::IECInstanceCR properties, bool ignoreErrors, DgnDbStatus* inStat)
    {
    auto params = DgnElement::InitCreateParamsFromECInstance(db, properties, inStat);
    if (!params.IsValid())
        return nullptr;

    DgnDbStatus ALLOW_NULL_OUTPUT(stat, inStat);
    auto ele = _CreateInstance(params);
    if (nullptr == ele)
        {
        BeAssert(false && "when would a handler fail to construct an element?");
        return nullptr;
        }
    DgnElement::SetPropertyFilter filter(DgnElement::SetPropertyFilter::Ignore::WriteOnlyNullBootstrapping, ignoreErrors);
    stat = ele->_SetPropertyValues(properties, filter);

    return (DgnDbStatus::Success == stat) ? ele : nullptr;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      10/16
+---------------+---------------+---------------+---------------+---------------+------*/
void ElementAutoHandledPropertiesECInstanceAdapter::Init(bool loadProperties, size_t initialAllocation) 
    {
    AddRef(); // TRICKY: protect against somebody else doing AddRef + Release and deleting this

    if ((0 != initialAllocation) && (0 == m_element.m_ecPropertyDataSize))
        AllocateBuffer(initialAllocation);

    if ((DgnElement::PropState::Unknown == m_element.m_flags.m_propState) && loadProperties)
        LoadProperties();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      10/16
+---------------+---------------+---------------+---------------+---------------+------*/
ElementAutoHandledPropertiesECInstanceAdapter::ElementAutoHandledPropertiesECInstanceAdapter(DgnElement const& el, bool loadProperties, size_t initialAllocation) 
    : m_element(const_cast<DgnElement&>(el))
    {
    m_eclass = m_element.GetElementClass();
    m_layout = &m_eclass->GetDefaultStandaloneEnabler()->GetClassLayout();
    Init(loadProperties, initialAllocation);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      10/16
+---------------+---------------+---------------+---------------+---------------+------*/
ElementAutoHandledPropertiesECInstanceAdapter::ElementAutoHandledPropertiesECInstanceAdapter(DgnElement const& el, ECClassCR ecls, ECN::ClassLayoutCR layout, bool loadProperties, size_t initialAllocation) 
    : m_element(const_cast<DgnElement&>(el)), m_eclass(&ecls), m_layout(&layout)
    {
    Init(loadProperties, initialAllocation);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      10/16
+---------------+---------------+---------------+---------------+---------------+------*/
void ElementAutoHandledPropertiesECInstanceAdapter::SetDirty()
    {
    m_element.m_flags.m_propState = DgnElement::PropState::Dirty;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson      10/16
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus ElementAutoHandledPropertiesECInstanceAdapter::LoadProperties()
    {
    // Thread-safety - Since properties are lazy-loaded, we must do the loading all within a mutex
    BeMutexHolder _v_(m_element.GetDgnDb().Elements().GetMutex());

    if (DgnElement::PropState::Unknown != m_element.m_flags.m_propState)
        return BSISUCCESS;

    //  We need to load the auto-handled properties into memory
    BeAssert(IsValid());
    BeAssert(DgnElement::PropState::Unknown == m_element.m_flags.m_propState);
    
    ECSqlClassInfo& classInfo = m_element.GetDgnDb().Elements().FindClassInfo(m_element); // Note: This "Find" method will create a ClassInfo if necessary
    auto const& selectProps = m_element.GetDgnDb().Elements().GetSelectEcPropsECSql(classInfo, *m_eclass);
    if (selectProps.empty())
        {
        m_element.m_flags.m_propState = DgnElement::PropState::NotFound;
        return BSIERROR;
        }

    // *********************************************************************************************************
    // From this point on, we must return BSISUCCESS. 
    // The ECClass says it has auto-handled properties. They may be NULL or not yet defined, but they are there.
    // *********************************************************************************************************

    if (nullptr == m_element.m_ecPropertyData)
        {
        BeAssert(0 == m_element.m_ecPropertyDataSize);
        AllocateBuffer(CalculateInitialAllocation(_GetClassLayout()));
        }

    if (!m_element.GetElementId().IsValid())
        {
        // If the element is certainly not persistent, then there's obviously no point in trying to "load" existing properties.
		// Preparing the statement and doing the step is significantly time-consuming, so this optimization has a big payoff in 
		// workflows such as bulk import.
        // Note that we must be sure that that buffer is allocated (in the step above) before returning early!
        m_element.m_flags.m_propState = DgnElement::PropState::InBuffer;
        return BSISUCCESS; // element is not persistent => all props are null at this point
        }

    CachedECSqlStatementPtr stmt = m_element.GetDgnDb().GetPreparedECSqlStatement(selectProps.c_str());
    if (stmt == nullptr)
        return BSIERROR;

    stmt->BindId(1, m_element.GetElementId());
    if (BE_SQLITE_ROW != stmt->Step())
        {
        m_element.m_flags.m_propState = DgnElement::PropState::InBuffer;
        return BSISUCCESS; // element is not persistent => all props are null at this point
        }

    ECInstanceECSqlSelectAdapter adapter(*stmt); // This adapter extracts the values from the statement and sets them into the ECDBuffer
    if (BSISUCCESS != adapter.GetInstance(*this))
        return BSIERROR;

    m_element.m_flags.m_propState = DgnElement::PropState::InBuffer; // initial state should InBuffer, not Dirty

    return BSISUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      10/16
+---------------+---------------+---------------+---------------+---------------+------*/
void ElementAutoHandledPropertiesECInstanceAdapter::AllocateBuffer(size_t size)
    {
    BeAssert(nullptr == m_element.m_ecPropertyData);
    BeAssert(0 == m_element.m_ecPropertyDataSize);

    m_element.m_ecPropertyDataSize = static_cast<uint32_t>(size);
    m_element.m_ecPropertyData = (Byte*)bentleyAllocator_malloc(m_element.m_ecPropertyDataSize);
    InitializeMemory(_GetClassLayout(), m_element.m_ecPropertyData, m_element.m_ecPropertyDataSize, true);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      10/16
+---------------+---------------+---------------+---------------+---------------+------*/
#ifdef WIP_PROPERTIES // *** ECObjects should be doing this
static bool isValidValue(ECN::ECPropertyCR prop, ECN::ECValueCR value)
    {
    BeAssert(IsValid());
    if (value.IsNull())
        {
        ECN::ECDbPropertyMap propertyMap;
        // *** WIP_AUTO_HANDLED_PROPERTIES -- must somehow cache this kind of metadata
        if (ECN::ECDbMapCustomAttributeHelper::TryGetPropertyMap(propertyMap, prop))
            {
            bool isNullable;
            if (ECN::ECObjectsStatus::Success == propertyMap.TryGetIsNullable(isNullable) && !isNullable)
                return false;
            }
        }

    // *** TBD: do range validation
    return true;
    }
#endif

/*---------------------------------------------------------------------------------**//**
* Adapted from MemoryECBaseInstance
* @bsimethod                                    Caleb.Shafer                      01/17
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus ElementAutoHandledPropertiesECInstanceAdapter::_SetStructArrayValueToMemory(ECN::ECValueCR v, ECN::PropertyLayoutCR propertyLayout, uint32_t index)
    {
    IECInstancePtr p;
    ECValue structValueIdValue(PRIMITIVETYPE_Integer);
    int32_t structValueId = GetMaxStructValueIdentifier() + 1;

    DEBUG_EXPECT(nullptr == GetAddressOfStructArrayEntry(structValueId));

    if (v.IsNull())
        {
        p = nullptr;
        structValueIdValue.SetInteger(0);
        }
    else
        {
        // Avoid making a copy if we have a Standalone that's not already part of somebody else's struct array
        IECInstancePtr pFrom = v.GetStruct();
        StandaloneECInstancePtr pTo = dynamic_cast<StandaloneECInstance*> (pFrom.get());
        if (pTo.IsNull() || pTo->IsSupportingInstance())
            {
            pTo = pFrom->GetEnabler().GetClass().GetDefaultStandaloneEnabler()->CreateInstance();
            ECObjectsStatus copyStatus = pTo->CopyValues(*pFrom);
            if (ECObjectsStatus::Success != copyStatus)
                return copyStatus;
            }

        // We now own it as a supporting instance
        pTo->SetIsSupportingInstance();
        p = pTo.get();
        structValueIdValue.SetInteger(structValueId);
        }

    ECObjectsStatus status = SetPrimitiveValueToMemory(structValueIdValue, propertyLayout, true, index);
    if (status != ECObjectsStatus::Success)
        {
        // This useless return value again....
        if (ECObjectsStatus::PropertyValueMatchesNoChange != status)
            return status;
        }

    if (nullptr == m_element.m_structInstances)
        m_element.m_structInstances = new ECN::StructInstanceVector();

    m_element.m_structInstances->push_back(ECN::StructArrayEntry(structValueId, p));

    return ECObjectsStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
* Adapted from MemoryECBaseInstance
* @bsimethod                                    Caleb.Shafer                      01/17
+---------------+---------------+---------------+---------------+---------------+------*/
ECN::StructValueIdentifier ElementAutoHandledPropertiesECInstanceAdapter::GetMaxStructValueIdentifier () const
    {
    if (nullptr == m_element.m_structInstances)
        return 0;

    size_t numEntries =  m_element.m_structInstances->size();
    StructArrayEntry const* instanceArray = &(*m_element.m_structInstances)[0];

    // we cannot simply use the size of the array because structs may have been removed at some point - so we must walk the array and find the highest ID
    ECN::StructValueIdentifier maxId = 0;
    for (size_t i = 0; i < numEntries; i++)
        {
        StructArrayEntry const& entry = instanceArray[i];
        if (entry.structValueIdentifier > maxId)
            maxId = entry.structValueIdentifier;
        }

    return maxId;
    }

/*---------------------------------------------------------------------------------**//**
* Adapted from MemoryECBaseInstance
* @bsimethod                                    Caleb.Shafer                      01/17
+---------------+---------------+---------------+---------------+---------------+------*/
StructArrayEntry const* ElementAutoHandledPropertiesECInstanceAdapter::GetAddressOfStructArrayEntry (StructValueIdentifier key) const
    {
    if (nullptr == m_element.m_structInstances)
        return nullptr;

    StructArrayEntry const* instanceArray = &(*m_element.m_structInstances)[0];
    for (size_t i = 0; i<m_element.m_structInstances->size(); i++)
        {
        StructArrayEntry const& entry = instanceArray[i];
        if (entry.structValueIdentifier != key)
            continue;
        return &entry;
        }

    return nullptr;
    }

/*---------------------------------------------------------------------------------**//**
* Adapted from MemoryECBaseInstance
* @bsimethod                                    Caleb.Shafer                      01/17
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus ElementAutoHandledPropertiesECInstanceAdapter::_GetStructArrayValueFromMemory(ECN::ECValueR v, ECN::PropertyLayoutCR propertyLayout, uint32_t index) const
    {
    ECValue structInstanceIdValue;
    ECObjectsStatus status = GetPrimitiveValueFromMemory(structInstanceIdValue, propertyLayout, true, index);
    if (ECObjectsStatus::Success != status)
        return status;

    // A structValueIdx of 0 means the instance is null
    if (structInstanceIdValue.IsNull() || 0 == structInstanceIdValue.GetInteger())
        {
        v.SetStruct(nullptr);
        return ECObjectsStatus::Success;
        }

    ECN::StructValueIdentifier structInstanceId = structInstanceIdValue.GetInteger();
    ECN::StructArrayEntry const* entry = GetAddressOfStructArrayEntry(structInstanceId);

    IECInstancePtr instancePtr = entry->structInstance;
    v.SetStruct(instancePtr.get());

    return ECObjectsStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
* Adapted from MemoryECBaseInstance
* @bsimethod                                    Caleb.Shafer                      01/17
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus ElementAutoHandledPropertiesECInstanceAdapter::RemoveStructArrayEntry(StructValueIdentifier structValueId)
    {
    if (nullptr == m_element.m_structInstances)
        return ECObjectsStatus::Error;

    ECN::StructInstanceVector::iterator iter;
    for (iter = m_element.m_structInstances->begin(); iter != m_element.m_structInstances->end(); iter++)
        {
        if (structValueId == (*iter).structValueIdentifier)
            {
            m_element.m_structInstances->erase(iter);
            return ECObjectsStatus::Success;
            }
        }

    return ECObjectsStatus::Error;
    }

/*---------------------------------------------------------------------------------**//**
* Adapted from MemoryECBaseInstance
* @bsimethod                                    Caleb.Shafer                      01/17
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus ElementAutoHandledPropertiesECInstanceAdapter::_RemoveStructArrayElementsFromMemory(ECN::PropertyLayoutCR propertyLayout, uint32_t removeIndex, uint32_t removeCount)
    {
    if (nullptr == m_element.m_structInstances)
        return ECObjectsStatus::Error;

    for (uint32_t i = 0; i < removeCount; i++)
        {
        ECValue v;
        ECObjectsStatus status = GetPrimitiveValueFromMemory(v, propertyLayout, true, removeIndex + i);
        if (ECObjectsStatus::Success != status)
            return status;

        ECN::StructValueIdentifier structValueId = v.GetInteger();
        status = RemoveStructArrayEntry(structValueId);
        if (ECObjectsStatus::Success != status)
            return status;
        }

    return RemoveArrayElementsFromMemory(propertyLayout, removeIndex, removeCount);
    }

/*---------------------------------------------------------------------------------**//**
* Adapted from MemoryECBaseInstance
* @bsimethod                                    Caleb.Shafer                      01/17
+---------------+---------------+---------------+---------------+---------------+------*/
bool ElementAutoHandledPropertiesECInstanceAdapter::_IsStructValidForArray(ECN::IECInstanceCR structInstance, ECN::PropertyLayoutCR propLayout) const
    {
    uint32_t propIdx;
    if (ECObjectsStatus::Success == m_layout->GetPropertyLayoutIndex(propIdx, propLayout))
        {
        ECPropertyCP ecprop = _GetAsIECInstance()->GetEnabler().LookupECProperty(propIdx);
        StructArrayECPropertyCP arrayProp = (nullptr != ecprop) ? ecprop->GetAsStructArrayProperty() : nullptr;
        if (nullptr != arrayProp)
            return structInstance.GetEnabler().GetClass().Is(&arrayProp->GetStructElementType());
        }

    return false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson      12/16
+---------------+---------------+---------------+---------------+---------------+------*/
ECInstanceUpdaterCache::ECInstanceUpdaterCache()
    {
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson      12/16
+---------------+---------------+---------------+---------------+---------------+------*/
ECInstanceUpdaterCache::~ECInstanceUpdaterCache()
    {
    Clear();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson      12/16
+---------------+---------------+---------------+---------------+---------------+------*/
void ECInstanceUpdaterCache::Clear()
    {
    for (auto& entry : m_updaters)
        {
        if (nullptr != entry.second)
            delete entry.second;
        }
    m_updaters.clear();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson      12/16
+---------------+---------------+---------------+---------------+---------------+------*/
BeSQLite::EC::ECInstanceUpdater* ECInstanceUpdaterCache::GetUpdater(DgnDbR db, ECN::ECClassCR eclass)
    {
    BeMutexHolder _v_(db.Elements().GetMutex());

    DgnClassId eclassId(eclass.GetId().GetValue());
  
    auto iupdater = m_updaters.find(eclassId);
    if (iupdater != m_updaters.end())
        return iupdater->second;

    bvector<ECN::ECPropertyCP> propertiesToBind;
    _GetPropertiesToBind(propertiesToBind, db, eclass);

    if (propertiesToBind.empty())
        return m_updaters[eclassId] = nullptr;

    return m_updaters[eclassId] = new EC::ECInstanceUpdater(db, eclass, db.GetECCrudWriteToken(), propertiesToBind, "NoECClassIdFilter ReadonlyPropertiesAreUpdatable");
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson      10/16
+---------------+---------------+---------------+---------------+---------------+------*/
BeSQLite::EC::ECInstanceUpdater* ElementAutoHandledPropertiesECInstanceAdapter::GetUpdater()
    {
    BeAssert(IsValid());
    return m_element.GetDgnDb().Elements().m_updaterCache.GetUpdater(m_element.GetDgnDb(), *m_eclass);
    }
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson      12/16
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnElements::AutoHandledPropertyUpdaterCache::_GetPropertiesToBind(bvector<ECN::ECPropertyCP>& autoHandledProperties, DgnDbR db, ECClassCR eclass)
    {
    for (auto prop : AutoHandledPropertiesCollection(eclass, db, ECSqlClassParams::StatementType::InsertUpdate, false))
        {
        autoHandledProperties.push_back(prop);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson      10/16
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnElements::ClearECCaches()
    {
    BeMutexHolder _v_v(m_mutex);
    m_stmts.Empty();
    m_classInfos.clear();
    m_updaterCache.Clear();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Bill.Steinbock                  12/2010
+---------------+---------------+---------------+---------------+---------------+------*/
IECInstanceP ElementAutoHandledPropertiesECInstanceAdapter::_GetAsIECInstance() const
    {
    return const_cast<ElementAutoHandledPropertiesECInstanceAdapter*>(this);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            11/2016
//---------------+---------------+---------------+---------------+---------------+-------
ECObjectsStatus ElementAutoHandledPropertiesECInstanceAdapter::_UpdateCalculatedPropertyDependents(ECValueCR calcedValue, PropertyLayoutCR propLayout)
    {
    IECInstanceR thisInstance = *_GetAsIECInstance();
    CalculatedPropertySpecificationCP spec = LookupCalculatedPropertySpecification(thisInstance, propLayout);
    return NULL != spec ? spec->UpdateDependentProperties(calcedValue, thisInstance) : ECObjectsStatus::Error;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson      10/16
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus ElementAutoHandledPropertiesECInstanceAdapter::UpdateProperties()
    {
    BeAssert(IsValid());
    BeAssert(nullptr != m_element.m_ecPropertyData);
    BeAssert(DgnElement::PropState::Dirty == m_element.m_flags.m_propState);

    ECInstanceUpdater* updater = GetUpdater();
    if (nullptr == updater)
        {
        BeAssert(false);
        return DgnDbStatus::WrongClass;
        }

    if (BE_SQLITE_OK != updater->Update(*this))
        return DgnDbStatus::WriteError;

    m_element.m_flags.m_propState = DgnElement::PropState::InBuffer;
    return DgnDbStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
* Adapted from MemoryECBaseInstance
* @bsimethod                                                    Sam.Wilson      10/16
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus ElementAutoHandledPropertiesECInstanceAdapter::_CopyFromBuffer(ECDBufferCR src)
    {
    //DgnElement const* fromMemoryInstance = dynamic_cast<DgnElement const*> (&src);
    //if (NULL != fromMemoryInstance && GetClassLayout().Equals (fromMemoryInstance->GetClassLayout()))
    //    {
    //    SetUsageBitmask (fromMemoryInstance->GetUsageBitmask());
    //    memcpy (m_perPropertyFlagsHolder.perPropertyFlags, fromMemoryInstance->GetPerPropertyFlagsData(), m_perPropertyFlagsHolder.numPerPropertyFlagsEntries * sizeof(uint32_t));
    //    }

    return CopyPropertiesFromBuffer(src);
    }

/*---------------------------------------------------------------------------------**//**
* Adapted from MemoryECBaseInstance
* @bsimethod                                                    Sam.Wilson      10/16
+---------------+---------------+---------------+---------------+---------------+------*/
uint32_t ElementAutoHandledPropertiesECInstanceAdapter::GetBytesUsed() const
    {
    if (NULL == m_element.m_ecPropertyData)
        return 0;

    return CalculateBytesUsed();
    }

/*---------------------------------------------------------------------------------**//**
* Adapted from MemoryECBaseInstance
* @bsimethod                                                    Sam.Wilson      10/16
+---------------+---------------+---------------+---------------+---------------+------*/        
void ElementAutoHandledPropertiesECInstanceAdapter::_ClearValues()
    {
    if (nullptr != m_element.m_structInstances)
        m_element.m_structInstances->clear ();

    InitializeMemory(GetClassLayout(), m_element.m_ecPropertyData, m_element.m_ecPropertyDataSize);

    //ClearAllPerPropertyFlags ();
    }
   
/*---------------------------------------------------------------------------------**//**
* Adapted from MemoryECBaseInstance
* @bsimethod                                                    Sam.Wilson      10/16
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus ElementAutoHandledPropertiesECInstanceAdapter::_ModifyData(uint32_t offset, void const * newData, uint32_t dataLength)
    {
    PRECONDITION (NULL != m_element.m_ecPropertyData, ECObjectsStatus::PreconditionViolated);
    PRECONDITION (offset + dataLength <= m_element.m_ecPropertyDataSize, ECObjectsStatus::MemoryBoundsOverrun);

    Byte * dest = m_element.m_ecPropertyData + offset;
    memcpy(dest, newData, dataLength);

    SetDirty();
    
    return ECObjectsStatus::Success;
    }
    
/*---------------------------------------------------------------------------------**//**
* Adapted from MemoryECBaseInstance
* @bsimethod                                                    Sam.Wilson      10/16
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus ElementAutoHandledPropertiesECInstanceAdapter::_MoveData(uint32_t toOffset, uint32_t fromOffset, uint32_t dataLength)
    {
    PRECONDITION (NULL != m_element.m_ecPropertyData, ECObjectsStatus::PreconditionViolated);
    PRECONDITION (toOffset + dataLength <= m_element.m_ecPropertyDataSize, ECObjectsStatus::MemoryBoundsOverrun);

    Byte* data = m_element.m_ecPropertyData;
    memmove(data+toOffset, data+fromOffset, dataLength);

    SetDirty();

    return ECObjectsStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
* Adapted from MemoryECBaseInstance
* @bsimethod                                                    Sam.Wilson      10/16
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus ElementAutoHandledPropertiesECInstanceAdapter::_ShrinkAllocation()
    {
    uint32_t newAllocation = GetBytesUsed();
    if (0 == newAllocation)
        _FreeAllocation();
    else if (newAllocation != _GetBytesAllocated())
        {
        Byte* reallocedData = (Byte*)bentleyAllocator_realloc(m_element.m_ecPropertyData, newAllocation);
        if (NULL == reallocedData)
            {
            BeAssert(false);
            return ECObjectsStatus::UnableToAllocateMemory;
            }

        m_element.m_ecPropertyData = reallocedData;
        m_element.m_ecPropertyDataSize = newAllocation;
        }

    return ECObjectsStatus::Success;
    } 

/*---------------------------------------------------------------------------------**//**
* Adapted from MemoryECBaseInstance
* @bsimethod                                                    Sam.Wilson      10/16
+---------------+---------------+---------------+---------------+---------------+------*/
void ElementAutoHandledPropertiesECInstanceAdapter::_FreeAllocation()
    {
    //if (!m_usingSharedMemory)
    //    {
        if (nullptr != m_element.m_ecPropertyData)
            {
            bentleyAllocator_free(m_element.m_ecPropertyData);

            m_element.m_ecPropertyData = nullptr;
            m_element.m_ecPropertyDataSize = 0;
            }

        //if (m_perPropertyFlagsHolder.perPropertyFlags)
        //    {
        //    free (m_perPropertyFlagsHolder.perPropertyFlags); 
        //    m_perPropertyFlagsHolder.perPropertyFlags = NULL;
        //    }

        BeAssert(nullptr == m_element.m_ecPropertyData);
        BeAssert(0 == m_element.m_ecPropertyDataSize);

    //    }

    if (nullptr != m_element.m_structInstances)
        {
        m_element.m_structInstances->clear();
        delete m_element.m_structInstances;
        m_element.m_structInstances = nullptr;
        }
    }

/*---------------------------------------------------------------------------------**//**
* Adapted from MemoryECBaseInstance
* @bsimethod                                                    Sam.Wilson      10/16
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus ElementAutoHandledPropertiesECInstanceAdapter::_GrowAllocation(uint32_t bytesNeeded)
    {
    uint32_t newSize = 2 * (m_element.m_ecPropertyDataSize + bytesNeeded); // Assume the growing trend will continue.

    Byte * reallocedData = (Byte*)bentleyAllocator_realloc(m_element.m_ecPropertyData, newSize);
    DEBUG_EXPECT (NULL != reallocedData);
    if (NULL == reallocedData)
        return ECObjectsStatus::UnableToAllocateMemory;
    
    m_element.m_ecPropertyData = reallocedData;
    m_element.m_ecPropertyDataSize = newSize;

    return ECObjectsStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
* Adapted from MemoryECBaseInstance
* @bsimethod                                                    Sam.Wilson      10/16
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus ElementAutoHandledPropertiesECInstanceAdapter::_ClearArray(uint32_t propIdx)
    {
    PropertyLayoutCP pPropertyLayout = NULL;
    ECObjectsStatus status = GetClassLayout().GetPropertyLayoutByIndex(pPropertyLayout, propIdx);
    if (ECObjectsStatus::Success != status || NULL == pPropertyLayout)
        return ECObjectsStatus::PropertyNotFound;

    uint32_t arrayCount = GetReservedArrayCount(*pPropertyLayout);
    if (arrayCount > 0)
        {
        RemoveArrayElements(*pPropertyLayout, 0, arrayCount);
        }

    SetDirty(); 

    return ECObjectsStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson      10/16
+---------------+---------------+---------------+---------------+---------------+------*/
ElementECPropertyAccessor::ElementECPropertyAccessor(DgnElementCR el, Utf8CP accessString)
    : m_element(const_cast<DgnElementR>(el)), m_readOnly(true)
    {
    Init(0, accessString);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson      10/16
+---------------+---------------+---------------+---------------+---------------+------*/
ElementECPropertyAccessor::ElementECPropertyAccessor(DgnElementCR el, uint32_t propIdx)
    : m_element(const_cast<DgnElementR>(el)), m_readOnly(true)
    {
    Init(propIdx, nullptr);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson      10/16
+---------------+---------------+---------------+---------------+---------------+------*/
ElementECPropertyAccessor::ElementECPropertyAccessor(DgnElementR el, Utf8CP accessString)
    : m_element(el), m_readOnly(false)
    {
    Init(0, accessString);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson      10/16
+---------------+---------------+---------------+---------------+---------------+------*/
ElementECPropertyAccessor::ElementECPropertyAccessor(DgnElementR el, uint32_t propIdx)
    : m_element(el), m_readOnly(false)
    {
    Init(propIdx, nullptr);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson      10/16
+---------------+---------------+---------------+---------------+---------------+------*/
void ElementECPropertyAccessor::Init(uint32_t propIdx, Utf8CP accessString)
    {
    m_eclass = m_element.GetElementClass();
    m_layout = &m_eclass->GetDefaultStandaloneEnabler()->GetClassLayout();

    if (nullptr == accessString)
        m_propIdx = propIdx;
    else
        {
        if (ECObjectsStatus::Success != m_layout->GetPropertyIndex(m_propIdx, accessString))
            {
            m_propIdx = 0;
            m_isPropertyIndexValid = false;
            m_classInfo = nullptr;
            m_accessors = nullptr;
            BeAssert(!IsValid());
            return;
            }
        }

    m_isPropertyIndexValid = true;

    // Additional information about the custom and auto-handled properties on this class
    // *** WIP_PROPERTIES - I wish I could cache this data on the ECClass object itself, instead of having to do this extra lookup
    m_classInfo = &m_element.GetDgnDb().Elements().FindClassInfo(m_element);
    m_accessors = m_classInfo->GetPropertyAccessors(m_propIdx);
    } 

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      10/16
+---------------+---------------+---------------+---------------+---------------+------*/
ECN::PropertyLayoutCP ElementECPropertyAccessor::GetPropertyLayout() const
    {
    ECN::PropertyLayoutCP playout;
    m_layout->GetPropertyLayoutByIndex(playout, m_propIdx);
    return playout;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      10/16
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus ElementECPropertyAccessor::SetAutoHandledPropertyValue(ECValueCR value, PropertyArrayIndex const& arrayIndex)
    {
    if (!IsValid() || (nullptr != m_accessors))
        {
        BeAssert(false);
        return DgnDbStatus::BadArg;
        }
    auto validator = m_classInfo->GetPropertyValidator(m_propIdx);
    if (nullptr != validator)
        {
        auto status = (*validator)(m_element, value);
        if (DgnDbStatus::Success != status)
            return status;
        }

#ifdef WIP_PROPERTIES // *** ECObjects should be doing this
    if (!isValidValue(*m_ecprop, value))
        return DgnDbStatus::BadArg;
#endif

    auto stypeNeeded = m_element.GetElementId().IsValid()? ECSqlClassParams::StatementType::Update : ECSqlClassParams::StatementType::Insert;
    if (0 == ((uint32_t)stypeNeeded & m_classInfo->GetAutoPropertyStatementType(m_propIdx)))
        return DgnDbStatus::ReadOnly;

    ElementAutoHandledPropertiesECInstanceAdapter ecdbuffer(m_element, *m_eclass, *m_layout, true);
    if (!ecdbuffer.IsValid())
        {
        BeAssert(false);
        return DgnDbStatus::NotFound;
        }

    PropertyLayoutCP propertyLayout = NULL;
    ECObjectsStatus status = m_layout->GetPropertyLayoutByIndex(propertyLayout, m_propIdx);
    if (ECObjectsStatus::Success != status || NULL == propertyLayout)
        return toDgnDbWriteStatus(status);
    status = ecdbuffer.SetInternalValueToMemory(*propertyLayout, value, arrayIndex.m_hasIndex, arrayIndex.m_index);
        
    if (ECObjectsStatus::PropertyValueMatchesNoChange == status)
        return DgnDbStatus::Success;

    BeAssert((ECObjectsStatus::Success != status) || (DgnElement::PropState::Dirty == m_element.m_flags.m_propState));
    return toDgnDbWriteStatus(status);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      10/16
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus ElementECPropertyAccessor::GetAutoHandledPropertyValue(ECN::ECValueR value, PropertyArrayIndex const& arrayIndex) const
    {
    if (!IsValid() || (nullptr != m_accessors))
        {
        BeAssert(false);
        return DgnDbStatus::BadArg;
        }
    ElementAutoHandledPropertiesECInstanceAdapter ecdbuffer(m_element, *m_eclass, *m_layout, true);
    if (!ecdbuffer.IsValid())
        {
        BeAssert(false);
        return DgnDbStatus::NotFound;
        }

    auto status = ecdbuffer.GetValueFromMemory(value, m_propIdx, arrayIndex.m_hasIndex, arrayIndex.m_index);
    return toDgnDbWriteStatus(status);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      08/16
+---------------+---------------+---------------+---------------+---------------+------*/
DateTime DgnElement::GetPropertyValueDateTime(Utf8CP propertyName, PropertyArrayIndex const& arrayIndex) const
    {
    ECN::ECValue value;
    DgnDbStatus status = GetPropertyValue(value, propertyName, arrayIndex);
    BeAssert(DgnDbStatus::Success == status);
    UNUSED_VARIABLE(status);
    return value.IsNull() ? DateTime() : value.GetDateTime();
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      08/16
+---------------+---------------+---------------+---------------+---------------+------*/
DPoint3d DgnElement::GetPropertyValueDPoint3d(Utf8CP propertyName, PropertyArrayIndex const& arrayIndex) const
    {
    ECN::ECValue value;
    DgnDbStatus status = GetPropertyValue(value, propertyName, arrayIndex);
    BeAssert(DgnDbStatus::Success == status);
    UNUSED_VARIABLE(status);
    return value.IsNull() ? DPoint3d::From(0,0,0) : value.GetPoint3d();
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      08/16
+---------------+---------------+---------------+---------------+---------------+------*/
DPoint2d DgnElement::GetPropertyValueDPoint2d(Utf8CP propertyName, PropertyArrayIndex const& arrayIndex) const
    {
    ECN::ECValue value;
    DgnDbStatus status = GetPropertyValue(value, propertyName, arrayIndex);
    BeAssert(DgnDbStatus::Success == status);
    UNUSED_VARIABLE(status);
    return value.IsNull() ? DPoint2d::From(0,0) : value.GetPoint2d();
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Shaun.Sewall                    08/16
+---------------+---------------+---------------+---------------+---------------+------*/
bool DgnElement::GetPropertyValueBoolean(Utf8CP propertyName, PropertyArrayIndex const& arrayIndex) const
    {
    ECN::ECValue value;
    DgnDbStatus status = GetPropertyValue(value, propertyName, arrayIndex);
    BeAssert(DgnDbStatus::Success == status);
    UNUSED_VARIABLE(status);
    return value.IsNull() ? false : value.GetBoolean();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Shaun.Sewall                    08/16
+---------------+---------------+---------------+---------------+---------------+------*/
double DgnElement::GetPropertyValueDouble(Utf8CP propertyName, PropertyArrayIndex const& arrayIndex) const
    {
    ECN::ECValue value;
    DgnDbStatus status = GetPropertyValue(value, propertyName, arrayIndex);
    BeAssert(DgnDbStatus::Success == status);
    UNUSED_VARIABLE(status);
    return value.IsNull() ? 0.0 : value.GetDouble();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Shaun.Sewall                    08/16
+---------------+---------------+---------------+---------------+---------------+------*/
int32_t DgnElement::GetPropertyValueInt32(Utf8CP propertyName, PropertyArrayIndex const& arrayIndex) const
    {
    ECN::ECValue value;
    DgnDbStatus status = GetPropertyValue(value, propertyName, arrayIndex);
    BeAssert(DgnDbStatus::Success == status);
    UNUSED_VARIABLE(status);
    return value.IsNull() ? 0 : value.GetInteger();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Shaun.Sewall                    08/16
+---------------+---------------+---------------+---------------+---------------+------*/
uint64_t DgnElement::GetPropertyValueUInt64(Utf8CP propertyName, PropertyArrayIndex const& arrayIndex) const
    {
    ECN::ECValue value;
    DgnDbStatus status = GetPropertyValue(value, propertyName, arrayIndex);
    BeAssert(DgnDbStatus::Success == status);
    UNUSED_VARIABLE(status);
    return value.IsNull() ? 0 : static_cast<uint64_t>(value.GetLong());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Shaun.Sewall                    08/16
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String DgnElement::GetPropertyValueString(Utf8CP propertyName, PropertyArrayIndex const& arrayIndex) const
    {
    ECN::ECValue value;
    DgnDbStatus status = GetPropertyValue(value, propertyName, arrayIndex);
    BeAssert(DgnDbStatus::Success == status);
    UNUSED_VARIABLE(status);
    return value.GetUtf8CP();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Shaun.Sewall                    08/16
+---------------+---------------+---------------+---------------+---------------+------*/
BeGuid DgnElement::GetPropertyValueGuid(Utf8CP propertyName, PropertyArrayIndex const& arrayIndex) const
    {
    ECN::ECValue value;
    DgnDbStatus status = GetPropertyValue(value, propertyName, arrayIndex);
    BeAssert(DgnDbStatus::Success == status);
    UNUSED_VARIABLE(status);

    if (!value.IsBinary())
        return BeGuid();

    size_t binarySize;
    const Byte* binary = value.GetBinary(binarySize);
    if (!binary || (binarySize != sizeof(BeGuid)))
        return BeGuid();

    BeGuid guid; 
    memcpy(&guid, binary, sizeof(guid)); 
    return guid;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Shaun.Sewall                    11/16
+---------------+---------------+---------------+---------------+---------------+------*/
DgnElement::NavigationPropertyInfo DgnElement::GetNavigationPropertyInfo(Utf8CP propertyName) const
    {
    ECN::ECValue value;
    DgnDbStatus status = GetPropertyValue(value, propertyName);
    BeAssert(DgnDbStatus::Success == status);
    UNUSED_VARIABLE(status);
    return value.IsNull() ? NavigationPropertyInfo() : NavigationPropertyInfo(value.GetNavigationInfo().GetId<BeInt64Id>(), value.GetNavigationInfo().GetRelationshipClassId());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      08/16
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus DgnElement::SetPropertyValue(Utf8CP propertyName, DateTimeCR value, PropertyArrayIndex const& arrayIndex)
    {
    DgnDbStatus status = SetPropertyValue(propertyName, ECValue(value), arrayIndex);
    BeAssert(DgnDbStatus::Success == status);
    return status;
    }
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      08/16
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus DgnElement::SetPropertyValue(Utf8CP propertyName, DPoint3dCR pt, PropertyArrayIndex const& arrayIndex)
    {
    DgnDbStatus status = SetPropertyValue(propertyName, ECValue(pt), arrayIndex);
    BeAssert(DgnDbStatus::Success == status);
    return status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      08/16
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus DgnElement::SetPropertyValue(Utf8CP propertyName, DPoint2dCR pt, PropertyArrayIndex const& arrayIndex)
    {
    DgnDbStatus status = SetPropertyValue(propertyName, ECValue(pt), arrayIndex);
    BeAssert(DgnDbStatus::Success == status);
    return status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Shaun.Sewall                    08/16
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus DgnElement::SetPropertyValue(Utf8CP propertyName, bool value, PropertyArrayIndex const& arrayIndex)
    {
    DgnDbStatus status = SetPropertyValue(propertyName, ECValue(value), arrayIndex);
    BeAssert(DgnDbStatus::Success == status);
    return status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Shaun.Sewall                    08/16
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus DgnElement::SetPropertyValue(Utf8CP propertyName, double value, PropertyArrayIndex const& arrayIndex)
    {
    DgnDbStatus status = SetPropertyValue(propertyName, ECValue(value), arrayIndex);
    BeAssert(DgnDbStatus::Success == status);
    return status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Shaun.Sewall                    08/16
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus DgnElement::SetPropertyValue(Utf8CP propertyName, int32_t value, PropertyArrayIndex const& arrayIndex)
    {
    DgnDbStatus status = SetPropertyValue(propertyName, ECValue(value), arrayIndex);
    BeAssert(DgnDbStatus::Success == status);
    return status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Shaun.Sewall                    08/16
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus DgnElement::SetPropertyValue(Utf8CP propertyName, int64_t value, PropertyArrayIndex const& arrayIndex)
    {
    DgnDbStatus status = SetPropertyValue(propertyName, ECValue(value), arrayIndex);
    BeAssert(DgnDbStatus::Success == status);
    return status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Shaun.Sewall                    08/16
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus DgnElement::SetPropertyValue(Utf8CP propertyName, BeSQLite::EC::ECInstanceId id, DgnClassId relClassId)
    {
    ECValue value;
    if (id.IsValid())
        relClassId.IsValid() ? value.SetNavigationInfo(id, (ECN::ECClassId)(relClassId.GetValue())) : value.SetNavigationInfo(id);
    else
        value.SetToNull();

    DgnDbStatus status = SetPropertyValue(propertyName, value);
    BeAssert(DgnDbStatus::Success == status);
    return status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Shaun.Sewall                    08/16
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus DgnElement::SetPropertyValue(Utf8CP propertyName, Utf8CP value, PropertyArrayIndex const& arrayIndex)
    {
    DgnDbStatus status = SetPropertyValue(propertyName, ECValue(value, false), arrayIndex);
    BeAssert(DgnDbStatus::Success == status);
    return status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson      06/16
+---------------+---------------+---------------+---------------+---------------+------*/
YawPitchRollAngles DgnElement::GetPropertyValueYpr(Utf8CP yawName, Utf8CP pitchName, Utf8CP rollName) const
    {
    YawPitchRollAngles angles;
    angles.SetYaw(AngleInDegrees::FromDegrees(GetPropertyValueDouble(yawName)));
    angles.SetPitch(AngleInDegrees::FromDegrees(GetPropertyValueDouble(pitchName)));
    angles.SetRoll(AngleInDegrees::FromDegrees(GetPropertyValueDouble(rollName)));
    return angles;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson      06/16
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus DgnElement::SetPropertyValueYpr(YawPitchRollAnglesCR angles, Utf8CP yawName, Utf8CP pitchName, Utf8CP rollName)
    {
    DgnDbStatus status;
    if (DgnDbStatus::Success != (status = SetPropertyValue(yawName, angles.GetYaw().Degrees())))
        return status;
    if (DgnDbStatus::Success != (status = SetPropertyValue(pitchName, angles.GetPitch().Degrees())))
        return status;
    if (DgnDbStatus::Success != (status = SetPropertyValue(rollName, angles.GetRoll().Degrees())))
        return status;
    return DgnDbStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson      06/16
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus DgnElement::_InsertPropertyArrayItems(uint32_t propertyIndex, uint32_t index, uint32_t size)
    {
    // Only auto-handled properties can be arrays
    ElementAutoHandledPropertiesECInstanceAdapter ecPropAccess(*this, true);
    if (!ecPropAccess.IsValid())
        return DgnDbStatus::BadArg;
    return toDgnDbWriteStatus(ecPropAccess.InsertArrayElements(propertyIndex, index, size));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson      06/16
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus DgnElement::_AddPropertyArrayItems(uint32_t propertyIndex, uint32_t size)
    {
    // Only auto-handled properties can be arrays
    ElementAutoHandledPropertiesECInstanceAdapter ecPropAccess(*this, true);
    if (!ecPropAccess.IsValid())
        return DgnDbStatus::BadArg;
    return toDgnDbWriteStatus(ecPropAccess.AddArrayElements(propertyIndex, size));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson      06/16
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus DgnElement::_RemovePropertyArrayItem(uint32_t propertyIndex, uint32_t index)
    {
    // Only auto-handled properties can be arrays
    ElementAutoHandledPropertiesECInstanceAdapter ecPropAccess(*this, true);
    if (!ecPropAccess.IsValid())
        return DgnDbStatus::BadArg;
    return toDgnDbWriteStatus(ecPropAccess.RemoveArrayElement(propertyIndex, index));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson      06/16
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus DgnElement::_ClearPropertyArray(uint32_t propertyIndex)
    {
    // Only auto-handled properties can be arrays
    ElementAutoHandledPropertiesECInstanceAdapter ecPropAccess(*this, true);
    if (!ecPropAccess.IsValid())
        return DgnDbStatus::BadArg;
    return toDgnDbWriteStatus(ecPropAccess.ClearArray(propertyIndex));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson      06/16
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus DgnElement::GetPropertyIndex(uint32_t& index, Utf8CP accessString)
    {
    auto& classLayout = GetElementClass()->GetDefaultStandaloneEnabler()->GetClassLayout();
    return toDgnDbWriteStatus(classLayout.GetPropertyIndex(index, accessString));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      10/16
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus DgnElement::SetPropertyValues(ECN::IECInstanceCR instance, SetPropertyFilter const& filter) 
    {return _SetPropertyValues(instance, filter);}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      10/16
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus DgnElement::InsertPropertyArrayItems(uint32_t propertyIndex, uint32_t index, uint32_t size)
    {return _InsertPropertyArrayItems(propertyIndex, index, size);}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      10/16
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus DgnElement::RemovePropertyArrayItem(uint32_t propertyIndex, uint32_t index)
    {return _RemovePropertyArrayItem(propertyIndex, index);}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      10/16
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus DgnElement::AddPropertyArrayItems(uint32_t propertyIndex, uint32_t size)
    {return _AddPropertyArrayItems(propertyIndex, size);}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      10/16
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus DgnElement::ClearPropertyArray(uint32_t propertyIndex)
    {return _ClearPropertyArray(propertyIndex);}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      10/16
+---------------+---------------+---------------+---------------+---------------+------*/
void ECSqlClassInfo::RegisterPropertyAccessors(ECN::ClassLayout const& layout, Utf8CP propName, T_ElementPropGet getFunc, T_ElementPropSet setFunc)
    {
    uint32_t propIdx;
    auto status = layout.GetPropertyIndex(propIdx, propName);
    if (ECN::ECObjectsStatus::Success != status)
        {
        BeAssert(false);
        return;
        }
    m_propertyAccessors[propIdx] = make_bpair(getFunc, setFunc);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      10/16
+---------------+---------------+---------------+---------------+---------------+------*/
bpair<ECSqlClassInfo::T_ElementPropGet,ECSqlClassInfo::T_ElementPropSet> const* ECSqlClassInfo::GetPropertyAccessors(uint32_t propIdx) const 
    {
    auto i = m_propertyAccessors.find(propIdx);
    return (i != m_propertyAccessors.end())? &i->second: nullptr;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      10/16
+---------------+---------------+---------------+---------------+---------------+------*/
void ECSqlClassInfo::RegisterPropertyValidator(ECN::ClassLayout const& layout, Utf8CP propName, T_ElementPropValidator func)
    {
    uint32_t propIdx;
    auto status = layout.GetPropertyIndex(propIdx, propName);
    if (ECN::ECObjectsStatus::Success != status)
        {
        BeAssert(false);
        return;
        }
    m_autoPropertyValidators[propIdx] = func;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      10/16
+---------------+---------------+---------------+---------------+---------------+------*/
ECSqlClassInfo::T_ElementPropValidator const* ECSqlClassInfo::GetPropertyValidator(uint32_t propIdx) const 
    {
    auto i = m_autoPropertyValidators.find(propIdx);
    return (i != m_autoPropertyValidators.end())? &i->second: nullptr;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      10/16
+---------------+---------------+---------------+---------------+---------------+------*/
uint32_t ECSqlClassInfo::GetAutoPropertyStatementType(uint32_t propIdx)
    {
    auto i = m_autoPropertyStatementType.find(propIdx);
    return (m_autoPropertyStatementType.end() == i)? (uint32_t)ECSqlClassParams::StatementType::All: i->second;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      10/16
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8CP ElementECPropertyAccessor::GetAccessString() const 
    {
    PropertyLayoutCP layout;
    return ECObjectsStatus::Success == m_layout->GetPropertyLayoutByIndex(layout, m_propIdx)? layout->GetAccessString(): "";
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      02/16
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus ElementECPropertyAccessor::GetPropertyValue(ECN::ECValueR value, PropertyArrayIndex const& arrayIndex) const
    {
    if (nullptr == m_accessors)
        return GetAutoHandledPropertyValue(value, arrayIndex);

    return m_accessors->first(value, m_element);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      02/16
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus ElementECPropertyAccessor::SetPropertyValue(ECN::ECValueCR value, PropertyArrayIndex const& arrayIndex)
    {
    if (m_readOnly)
        {
        BeAssert(false);
        return DgnDbStatus::ReadOnly;
        }

    if (nullptr == m_accessors)
        return SetAutoHandledPropertyValue(value, arrayIndex);

    return m_accessors->second(m_element, value);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      02/16
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnElement::RemapAutoHandledNavigationproperties(DgnImportContext& importer)
    {
    for (auto prop : AutoHandledPropertiesCollection(*GetElementClass(), GetDgnDb(), ECSqlClassParams::StatementType::All, false))
        {
        if (!prop->GetIsNavigation())
            continue;

        ECValue v;
        GetPropertyValue(v, prop->GetName().c_str());
        if (v.IsNull())
            continue;

        DgnElementId id((uint64_t)v.GetLong());
        id = importer.FindElementId(id);
        if (id.IsValid())
            SetPropertyValue(prop->GetName().c_str(), v);
        }
    }


