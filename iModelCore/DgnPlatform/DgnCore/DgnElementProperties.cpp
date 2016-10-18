/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnCore/DgnElementProperties.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
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
Utf8String ElementECInstanceAdapter::GetPropName(uint32_t index) const
    {
    PropertyLayoutCP propLayout;
    if (ECObjectsStatus::Success != GetClassLayout().GetPropertyLayoutByIndex(propLayout, index))
        {
        BeAssert(false);
        return "";
        }
    return propLayout->GetAccessString();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson      10/16
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus ElementECInstanceAdapter::_GetValue (ECValueR v, uint32_t propertyIndex, bool useArrayIndex, uint32_t arrayIndex) const 
    {
    DgnElement::PropertyArrayIndex ai(useArrayIndex, arrayIndex);
    auto stat = m_element.GetPropertyValue(v, GetPropName(propertyIndex).c_str(), ai);
    return toECObjectsStatus(stat);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson      10/16
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus ElementECInstanceAdapter::_SetValue (uint32_t propertyIndex, ECValueCR v, bool useArrayIndex, uint32_t arrayIndex)
    {
    if (m_readOnly)
        {
        BeAssert(false);
        return ECObjectsStatus::UnableToSetReadOnlyInstance;
        }
    DgnElement::PropertyArrayIndex ai(useArrayIndex, arrayIndex);
    auto stat = m_element.SetPropertyValue(GetPropName(propertyIndex).c_str(), v, ai);
    return toECObjectsStatus(stat);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson      10/16
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus ElementECInstanceAdapter::_InsertArrayElements (uint32_t propertyIndex, uint32_t index, uint32_t size)
    {
    if (m_readOnly)
        {
        BeAssert(false);
        return ECObjectsStatus::UnableToSetReadOnlyInstance;
        }
    // Only auto-handled properties can be arrays
    ElementAutoHandledPropertiesECInstanceAdapter autoHandledProps(m_element);
    return autoHandledProps._InsertArrayElements(propertyIndex, index, size);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson      10/16
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus ElementECInstanceAdapter::_AddArrayElements (uint32_t propertyIndex, uint32_t size)
    {
    if (m_readOnly)
        {
        BeAssert(false);
        return ECObjectsStatus::UnableToSetReadOnlyInstance;
        }
    // Only auto-handled properties can be arrays
    ElementAutoHandledPropertiesECInstanceAdapter autoHandledProps(m_element);
    return autoHandledProps.AddArrayElements(propertyIndex, size);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson      10/16
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus ElementECInstanceAdapter::_RemoveArrayElement (uint32_t propertyIndex, uint32_t index)
    {
    if (m_readOnly)
        {
        BeAssert(false);
        return ECObjectsStatus::UnableToSetReadOnlyInstance;
        }
    // Only auto-handled properties can be arrays
    ElementAutoHandledPropertiesECInstanceAdapter autoHandledProps(m_element);
    return autoHandledProps._RemoveArrayElement(propertyIndex, index);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson      10/16
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus ElementECInstanceAdapter::_ClearArray (uint32_t propIdx)
    {
    if (m_readOnly)
        {
        BeAssert(false);
        return ECObjectsStatus::UnableToSetReadOnlyInstance;
        }
    // Only auto-handled properties can be arrays
    ElementAutoHandledPropertiesECInstanceAdapter autoHandledProps(m_element);
    return autoHandledProps._ClearArray(propIdx);
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
        s_ignoreList->insert(BIS_ELEMENT_PROP_ModelId);
        s_ignoreList->insert(BIS_ELEMENT_PROP_CodeAuthorityId);
        s_ignoreList->insert(BIS_ELEMENT_PROP_CodeNamespace);
        s_ignoreList->insert(BIS_ELEMENT_PROP_CodeValue);
        });

    return s_ignoreList->find(propName) != s_ignoreList->end();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson      10/16
+---------------+---------------+---------------+---------------+---------------+------*/
bool DgnElement::SetPropertyFilter::_ExcludeProperty(ECPropertyValueCR propValue) const
    {
    if ((0 != (m_ignore & Null)) && propValue.GetValue().IsNull())
        return true;

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
        if (propName.Equals(GEOM3_InSpatialIndex))
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
            ECPropertyCP ecProp = prop.GetValueAccessor().GetECProperty();
            ECObjectsStatus ecStatus;
            if (NULL == ecProp)
                {
                BeAssert(false);
                continue;
                }
            BeAssert(ecProp->GetIsPrimitive() || ecProp->GetIsNavigation());
            if (ECObjectsStatus::Success != (ecStatus = SetInternalValueUsingAccessor (prop.GetValueAccessor(), prop.GetValue())))
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

    ElementECInstanceAdapter ecOther(other);
    ECValuesCollection otherProperties(ecOther);
    for (auto const& otherProp : otherProperties)
        {
        if (filter._ExcludeProperty(otherProp))
            continue;

        auto const& propName = otherProp.GetValueAccessor().GetECProperty()->GetName();

        if (propName.Equals("UserProperties")) // _GetPropertyValue does not work for user props
            {
            if (nullptr == m_userProperties)
                LoadUserProperties();
            if (nullptr == other.m_userProperties)
                other.LoadUserProperties();
            if (!m_userProperties->ToString().Equals(other.m_userProperties->ToString()))
                return false;
            }

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

        str.append(propertyValue.GetValueAccessor().GetDebugAccessString().c_str());
        str.append("=");
        str.append(propertyValue.GetValue().ToString().c_str());
        str.append(comma);
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
DgnElementPtr DgnElements::CreateElement(DgnDbStatus* inStat, ECN::IECInstanceCR properties)
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

    return handler->_CreateNewElement(inStat, GetDgnDb(), properties);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson      07/16
+---------------+---------------+---------------+---------------+---------------+------*/
DgnElement::CreateParams DgnElement::InitCreateParamsFromECInstance(DgnDbStatus* inStat, DgnDbR db, ECN::IECInstanceCR properties)
    {
    DgnDbStatus ALLOW_NULL_OUTPUT(stat, inStat);

    DgnModelId mid;
        {
        ECN::ECValue v;
        if (ECN::ECObjectsStatus::Success != properties.GetValue(v, BIS_ELEMENT_PROP_ModelId) || v.IsNull())
            {
            stat = DgnDbStatus::BadArg;
            return CreateParams(db, DgnModelId(), DgnClassId());
            }
        mid = DgnModelId((uint64_t)v.GetLong());
        if (!mid.IsValid())
            {
            stat = DgnDbStatus::BadArg;
            return CreateParams(db, DgnModelId(), DgnClassId());
            }
        }

    DgnClassId classId(properties.GetClass().GetId().GetValue());

    DgnCode code;
        //! The authority ID must be non-null and identify a valid authority.
        //! The namespace may not be null, but may be a blank string.
        //! The value may be null if and only if the namespace is blank, signifying that the authority
        //! assigns no special meaning to the object's code.
        //! The value may not be an empty string.
        {
        ECN::ECValue v;
        if (ECN::ECObjectsStatus::Success != properties.GetValue(v, BIS_ELEMENT_PROP_CodeAuthorityId) || v.IsNull())
            {
            stat = DgnDbStatus::BadArg;
            return CreateParams(db, DgnModelId(), classId);
            }
        DgnAuthorityId id((uint64_t) v.GetLong());

        if (ECN::ECObjectsStatus::Success != properties.GetValue(v, BIS_ELEMENT_PROP_CodeNamespace) || v.IsNull())
            {
            stat = DgnDbStatus::BadArg;
            return CreateParams(db, DgnModelId(), classId);
            }
        Utf8String codeName(v.GetUtf8CP());

        if (ECN::ECObjectsStatus::Success != properties.GetValue(v, BIS_ELEMENT_PROP_CodeValue) || (v.IsNull() && !Utf8String::IsNullOrEmpty(codeName.c_str())) ||
            (!v.IsNull() && 0 == strlen(v.GetUtf8CP())))
            {
            stat = DgnDbStatus::BadArg;
            return CreateParams(db, DgnModelId(), classId);
            }

        code.From(id, v.GetUtf8CP(), codeName);
        }

    DgnElement::CreateParams params(db, mid, classId, code);

    auto ecinstanceid = properties.GetInstanceId();                 // Note that ECInstanceId is not a normal property and will not be returned by the property collection below
    if (!ecinstanceid.empty())
        {
        uint64_t idvalue;
        if (BSISUCCESS != BeStringUtilities::ParseUInt64(idvalue, ecinstanceid.c_str()))
            {
            stat = DgnDbStatus::BadArg;
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

//---------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald            09/2016
//---------------+---------------+---------------+---------------+---------------+-------
DgnElement::CreateParams dgn_ElementHandler::Element::_InitCreateParams(DgnDbStatus* inStat, DgnDbR db, ECN::IECInstanceCR properties)
    {
    return DgnElement::InitCreateParamsFromECInstance(inStat, db, properties);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson      07/16
+---------------+---------------+---------------+---------------+---------------+------*/
DgnElementPtr dgn_ElementHandler::Element::_CreateNewElement(DgnDbStatus* inStat, DgnDbR db, ECN::IECInstanceCR properties)
    {
    DgnDbStatus ALLOW_NULL_OUTPUT(stat, inStat);
    auto params = _InitCreateParams(inStat, db, properties);
    if (!params.IsValid())
        return nullptr;
    auto ele = _CreateInstance(params);
    if (nullptr == ele)
        {
        BeAssert(false && "when would a handler fail to construct an element?");
        return nullptr;
        }
    DgnElement::SetPropertyFilter filter(DgnElement::SetPropertyFilter::Ignore::WriteOnlyNullBootstrapping);
    stat = ele->_SetPropertyValues(properties, filter);
    return (DgnDbStatus::Success == stat)? ele: nullptr;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      10/16
+---------------+---------------+---------------+---------------+---------------+------*/
ElementAutoHandledPropertiesECInstanceAdapter::ElementAutoHandledPropertiesECInstanceAdapter(DgnElement const& el) 
    : m_element(const_cast<DgnElement&>(el))
    {
    AddRef(); // protect against somebody else doing AddRef + Release and deleting this

    m_eclass = m_element.GetElementClass();
    m_layout = &m_eclass->GetDefaultStandaloneEnabler()->GetClassLayout();

    if (DgnElement::PropState::Unknown == m_element.m_flags.m_propState)
        LoadProperties();

    if (DgnElement::PropState::NotFound == m_element.m_flags.m_propState)
        {
        m_eclass = nullptr; // This object cannot be used to access properties on this element
        BeAssert(!IsValid());
        BeAssert(nullptr == m_element.m_ecPropertyData);
        return; // This element has no auto-handled properties
        }

    BeAssert(IsValid());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson      10/16
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus ElementAutoHandledPropertiesECInstanceAdapter::LoadProperties()
    {
    BeAssert(IsValid());
    BeAssert(DgnElement::PropState::Unknown == m_element.m_flags.m_propState);
    BeAssert(nullptr == m_element.m_ecPropertyData);
    
    ECSqlClassInfo& classInfo = m_element.GetDgnDb().Elements().FindClassInfo(m_element); // Note: This "Find" method will create a ClassInfo if necessary
    if (classInfo.GetSelectEcPropsECSql().empty())
        {
        Utf8String props;
        Utf8CP comma = "";
        bvector<ECN::ECPropertyCP> autoHandledProperties;
        for (auto prop : AutoHandledPropertiesCollection(*m_eclass, m_element.GetDgnDb(), ECSqlClassParams::StatementType::Select, false))
            {
            Utf8StringCR propName = prop->GetName();
            props.append(comma).append("[").append(propName).append("]");
            comma = ",";
            }

        if (props.empty())
            {
            m_element.m_flags.m_propState = DgnElement::PropState::NotFound;
            return BSIERROR;
            }

        classInfo.SetSelectEcPropsECSql(Utf8PrintfString("SELECT %s FROM %s WHERE ECInstanceId=? ECSQLOPTIONS NoECClassIdFilter", 
                                                             props.c_str(), m_eclass->GetECSqlName().c_str()));
        }

    // *********************************************************************************************************
    // From this point on, we must return BSISUCCESS. 
    // The ECClass says it has auto-handled properties. They may be NULL or not yet defined, but they are there.
    // *********************************************************************************************************

    m_element.m_flags.m_propState = DgnElement::PropState::InBuffer;

    auto stmt = m_element.GetDgnDb().GetPreparedECSqlStatement(classInfo.GetSelectEcPropsECSql().c_str());

    auto const& classLayout = _GetClassLayout();
    m_element.m_ecPropertyDataSize = CalculateInitialAllocation(classLayout);
    m_element.m_ecPropertyData = (Byte*)bentleyAllocator_malloc(m_element.m_ecPropertyDataSize);
    InitializeMemory(classLayout, m_element.m_ecPropertyData, m_element.m_ecPropertyDataSize, true);

    stmt->BindId(1, m_element.GetElementId());
    if (BE_SQLITE_ROW != stmt->Step())
        {
        return BSISUCCESS; // element is not persistent => all props are null at this point
        }

    ECInstanceECSqlSelectAdapter adapter(*stmt);
    adapter.SetInstanceData(*this, true);

    return BSISUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      10/16
+---------------+---------------+---------------+---------------+---------------+------*/
#ifdef WIP_PROPERTIES // *** ECObjects should be doing this
bool ElementAutoHandledPropertiesECInstanceAdapter::IsValidValue(ECN::ECPropertyCR prop, ECN::ECValueCR value)
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
* @bsimethod                                                    Sam.Wilson      10/16
+---------------+---------------+---------------+---------------+---------------+------*/
bool ElementAutoHandledPropertiesECInstanceAdapter::IsValidForStatementType(ECN::ECPropertyCR prop, ECSqlClassParams::StatementType stypeNeeded)
    {
    BeAssert(IsValid());
    // *** WIP_AUTO_HANDLED_PROPERTIES -- must somehow cache this kind of metadata

    auto propertyStatementType = m_element.GetDgnDb().Schemas().GetECClass(BIS_ECSCHEMA_NAME, "AutoHandledProperty");
    auto stypeCA = prop.GetCustomAttribute(*propertyStatementType);
    if (!stypeCA.IsValid())
        return true;

    ECN::ECValue stypeValue;
    stypeCA->GetValue(stypeValue, "StatementTypes");

    return 0 != ((uint32_t)stypeNeeded & stypeValue.GetInteger());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson      10/16
+---------------+---------------+---------------+---------------+---------------+------*/
BeSQLite::EC::ECInstanceUpdater* ElementAutoHandledPropertiesECInstanceAdapter::GetUpdater()
    {
    BeAssert(IsValid());

    DgnClassId eclassid(m_eclass->GetId().GetValue());

    auto& updaterCache = m_element.GetDgnDb().Elements().m_updaterCache;
    auto iupdater = updaterCache.find(eclassid);
    if (iupdater != updaterCache.end())
        return iupdater->second;

    bvector<ECN::ECPropertyCP> autoHandledProperties;
    for (auto prop : AutoHandledPropertiesCollection(*m_eclass, m_element.GetDgnDb(), ECSqlClassParams::StatementType::InsertUpdate, false))
        {
        autoHandledProperties.push_back(prop);
        }

    if (autoHandledProperties.empty())
        return updaterCache[eclassid] = nullptr;

    return updaterCache[eclassid] = new EC::ECInstanceUpdater(m_element.GetDgnDb(), *m_eclass, autoHandledProperties);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson      10/16
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnElements::ClearUpdaterCache()
    {
    for (auto& upd : m_updaterCache)
        {
        if (upd.second)
            delete upd.second;
        }
    m_updaterCache.clear();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson      10/16
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus ElementAutoHandledPropertiesECInstanceAdapter::UpdateProperties()
    {
    BeAssert(IsValid());
    BeAssert(DgnElement::PropState::Dirty == m_element.m_flags.m_propState);
    BeAssert(nullptr != m_element.m_ecPropertyData);

    m_element.m_flags.m_propState = DgnElement::PropState::InBuffer;

    ECInstanceUpdater* updater = GetUpdater();
    if (nullptr == updater)
        {
        BeAssert(false);
        return DgnDbStatus::WrongClass;
        }

    return (BSISUCCESS == updater->Update(*this))? DgnDbStatus::Success: DgnDbStatus::WriteError;
    }

/*---------------------------------------------------------------------------------**//**
* Adapted from MemoryECBaseInstance
* @bsimethod                                                    Sam.Wilson      10/16
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus ElementAutoHandledPropertiesECInstanceAdapter::_CopyFromBuffer (ECDBufferCR src)
    {
    //DgnElement const* fromMemoryInstance = dynamic_cast<DgnElement const*> (&src);
    //if (NULL != fromMemoryInstance && GetClassLayout().Equals (fromMemoryInstance->GetClassLayout()))
    //    {
    //    SetUsageBitmask (fromMemoryInstance->GetUsageBitmask());
    //    memcpy (m_perPropertyFlagsHolder.perPropertyFlags, fromMemoryInstance->GetPerPropertyFlagsData(), m_perPropertyFlagsHolder.numPerPropertyFlagsEntries * sizeof(uint32_t));
    //    }

    return CopyPropertiesFromBuffer (src);
    }

/*---------------------------------------------------------------------------------**//**
* Adapted from MemoryECBaseInstance
* @bsimethod                                                    Sam.Wilson      10/16
+---------------+---------------+---------------+---------------+---------------+------*/
uint32_t ElementAutoHandledPropertiesECInstanceAdapter::GetBytesUsed () const
    {
    if (NULL == m_element.m_ecPropertyData)
        return 0;

    return CalculateBytesUsed ();
    }

/*---------------------------------------------------------------------------------**//**
* Adapted from MemoryECBaseInstance
* @bsimethod                                                    Sam.Wilson      10/16
+---------------+---------------+---------------+---------------+---------------+------*/        
void ElementAutoHandledPropertiesECInstanceAdapter::_ClearValues ()
    {
    //if (m_structInstances)
    //    m_structInstances->clear ();

    InitializeMemory (GetClassLayout(), m_element.m_ecPropertyData, m_element.m_ecPropertyDataSize);

    //ClearAllPerPropertyFlags ();
    }
   
/*---------------------------------------------------------------------------------**//**
* Adapted from MemoryECBaseInstance
* @bsimethod                                                    Sam.Wilson      10/16
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus ElementAutoHandledPropertiesECInstanceAdapter::_ModifyData (uint32_t offset, void const * newData, uint32_t dataLength)
    {
    PRECONDITION (NULL != m_element.m_ecPropertyData, ECObjectsStatus::PreconditionViolated);
    PRECONDITION (offset + dataLength <= m_element.m_ecPropertyDataSize, ECObjectsStatus::MemoryBoundsOverrun);

    Byte * dest = m_element.m_ecPropertyData + offset;
    memcpy (dest, newData, dataLength);
    
    return ECObjectsStatus::Success;
    }
    
/*---------------------------------------------------------------------------------**//**
* Adapted from MemoryECBaseInstance
* @bsimethod                                                    Sam.Wilson      10/16
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus ElementAutoHandledPropertiesECInstanceAdapter::_MoveData (uint32_t toOffset, uint32_t fromOffset, uint32_t dataLength)
    {
    PRECONDITION (NULL != m_element.m_ecPropertyData, ECObjectsStatus::PreconditionViolated);
    PRECONDITION (toOffset + dataLength <= m_element.m_ecPropertyDataSize, ECObjectsStatus::MemoryBoundsOverrun);

    Byte* data = m_element.m_ecPropertyData;
    memmove (data+toOffset, data+fromOffset, dataLength);

    return ECObjectsStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
* Adapted from MemoryECBaseInstance
* @bsimethod                                                    Sam.Wilson      10/16
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus ElementAutoHandledPropertiesECInstanceAdapter::_ShrinkAllocation ()
    {
    uint32_t newAllocation = GetBytesUsed();
    if (0 == newAllocation)
        _FreeAllocation();
    else if (newAllocation != _GetBytesAllocated())
        {
        Byte* reallocedData = (Byte*)bentleyAllocator_realloc(m_element.m_ecPropertyData, newAllocation);
        if (NULL == reallocedData)
            {
            BeAssert (false);
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
void ElementAutoHandledPropertiesECInstanceAdapter::_FreeAllocation ()
    {
    //if (!m_usingSharedMemory)
    //    {
        if (m_element.m_ecPropertyData)
            bentleyAllocator_free(m_element.m_ecPropertyData);

        //if (m_perPropertyFlagsHolder.perPropertyFlags)
        //    {
        //    free (m_perPropertyFlagsHolder.perPropertyFlags); 
        //    m_perPropertyFlagsHolder.perPropertyFlags = NULL;
        //    }
    //    }

    m_element.m_ecPropertyData = NULL;

    //if (m_structInstances)
    //    {
    //    m_structInstances->clear ();
    //    delete m_structInstances;
    //    m_structInstances = NULL;
    //    }
    }

/*---------------------------------------------------------------------------------**//**
* Adapted from MemoryECBaseInstance
* @bsimethod                                                    Sam.Wilson      10/16
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus ElementAutoHandledPropertiesECInstanceAdapter::_GrowAllocation (uint32_t bytesNeeded)
    {
    DEBUG_EXPECT (m_element.m_ecPropertyDataSize > 0);
    DEBUG_EXPECT (NULL != m_element.m_ecPropertyData);
        
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
ECObjectsStatus ElementAutoHandledPropertiesECInstanceAdapter::_ClearArray (uint32_t propIdx)
    {
    PropertyLayoutCP pPropertyLayout = NULL;
    ECObjectsStatus status = GetClassLayout().GetPropertyLayoutByIndex (pPropertyLayout, propIdx);
    if (ECObjectsStatus::Success != status || NULL == pPropertyLayout)
        return ECObjectsStatus::PropertyNotFound;

    uint32_t arrayCount = GetReservedArrayCount (*pPropertyLayout);
    if (arrayCount > 0)
        {
        RemoveArrayElements (*pPropertyLayout, 0, arrayCount);
        }

    return ECObjectsStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson      10/16
+---------------+---------------+---------------+---------------+---------------+------*/
ElementECPropertyAccessor::ElementECPropertyAccessor(DgnElement const& el, Utf8CP accessString) :
    ElementAutoHandledPropertiesECInstanceAdapter(el)
    {
    if (ECObjectsStatus::Success != m_layout->GetPropertyIndex(m_propIdx, accessString))
        {
        m_propIdx = UINT32_MAX;     // This object cannot be used to access this property
        BeAssert(!IsValid());
        return;
        }
    Init(m_propIdx);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson      10/16
+---------------+---------------+---------------+---------------+---------------+------*/
ElementECPropertyAccessor::ElementECPropertyAccessor(DgnElement const& el, uint32_t propIdx) :
    ElementAutoHandledPropertiesECInstanceAdapter(el)
    {
    Init(propIdx);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson      10/16
+---------------+---------------+---------------+---------------+---------------+------*/
void ElementECPropertyAccessor::Init(uint32_t propIdx)
    {
    m_propIdx = propIdx;
    m_isCustomHandled = false;

    if (!ElementAutoHandledPropertiesECInstanceAdapter::IsValid())
        return;

    m_classInfo = &m_element.GetDgnDb().Elements().FindClassInfo(m_element);

    m_isCustomHandled = m_classInfo->IsCustomHandledProperty(propIdx);

    BeAssert(IsValid());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      10/16
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus ElementECPropertyAccessor::SetAutoHandledPropertyValue(ECValueCR value, DgnElement::PropertyArrayIndex const& arrayIdx)
    {
    if (!IsValid() || m_isCustomHandled)
        {
        BeAssert(false);
        return DgnDbStatus::BadArg;
        }

#ifdef WIP_PROPERTIES // *** ECObjects should be doing this
    if (!IsValidValue(*m_ecprop, value))
        return DgnDbStatus::BadArg;
#endif

#ifdef WIP_PROPERTIES // *** Need ECPropertyCP
    if (!IsValidForStatementType(*m_ecprop, m_element.GetElementId().IsValid() ? 
                                 ECSqlClassParams::StatementType::Update : ECSqlClassParams::StatementType::Insert))
        return DgnDbStatus::ReadOnly;
#endif

    auto status = SetValueToMemory(m_propIdx, value, arrayIdx.m_hasIndex, arrayIdx.m_index);
        
    if ((ECObjectsStatus::Success != status) && (ECObjectsStatus::PropertyValueMatchesNoChange != status))
        return DgnDbStatus::BadArg; // probably a type mismatch

    m_element.m_flags.m_propState = DgnElement::PropState::Dirty;

    return DgnDbStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      10/16
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus ElementECPropertyAccessor::GetAutoHandledPropertyValue(ECN::ECValueR value, DgnElement::PropertyArrayIndex const& arrayIdx)
    {
    if (!IsValid() || m_isCustomHandled)
        {
        BeAssert(false);
        return DgnDbStatus::BadArg;
        }

    if (ECN::ECObjectsStatus::Success != GetValueFromMemory(value, m_propIdx, arrayIdx.m_hasIndex, arrayIdx.m_index))
        return DgnDbStatus::BadRequest;
    
    return DgnDbStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      08/16
+---------------+---------------+---------------+---------------+---------------+------*/
DateTime DgnElement::GetPropertyValueDateTime(Utf8CP propertyName, PropertyArrayIndex const& arrayIdx) const
    {
    ECN::ECValue value;
    DgnDbStatus status = GetPropertyValue(value, propertyName, arrayIdx);
    BeAssert(DgnDbStatus::Success == status);
    UNUSED_VARIABLE(status);
    return value.IsNull() ? DateTime() : value.GetDateTime();
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      08/16
+---------------+---------------+---------------+---------------+---------------+------*/
DPoint3d DgnElement::GetPropertyValueDPoint3d(Utf8CP propertyName, PropertyArrayIndex const& arrayIdx) const
    {
    ECN::ECValue value;
    DgnDbStatus status = GetPropertyValue(value, propertyName, arrayIdx);
    BeAssert(DgnDbStatus::Success == status);
    UNUSED_VARIABLE(status);
    return value.IsNull() ? DPoint3d::From(0,0,0) : value.GetPoint3d();
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      08/16
+---------------+---------------+---------------+---------------+---------------+------*/
DPoint2d DgnElement::GetPropertyValueDPoint2d(Utf8CP propertyName, PropertyArrayIndex const& arrayIdx) const
    {
    ECN::ECValue value;
    DgnDbStatus status = GetPropertyValue(value, propertyName, arrayIdx);
    BeAssert(DgnDbStatus::Success == status);
    UNUSED_VARIABLE(status);
    return value.IsNull() ? DPoint2d::From(0,0) : value.GetPoint2d();
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Shaun.Sewall                    08/16
+---------------+---------------+---------------+---------------+---------------+------*/
bool DgnElement::GetPropertyValueBoolean(Utf8CP propertyName, PropertyArrayIndex const& arrayIdx) const
    {
    ECN::ECValue value;
    DgnDbStatus status = GetPropertyValue(value, propertyName, arrayIdx);
    BeAssert(DgnDbStatus::Success == status);
    UNUSED_VARIABLE(status);
    return value.IsNull() ? false : value.GetBoolean();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Shaun.Sewall                    08/16
+---------------+---------------+---------------+---------------+---------------+------*/
double DgnElement::GetPropertyValueDouble(Utf8CP propertyName, PropertyArrayIndex const& arrayIdx) const
    {
    ECN::ECValue value;
    DgnDbStatus status = GetPropertyValue(value, propertyName, arrayIdx);
    BeAssert(DgnDbStatus::Success == status);
    UNUSED_VARIABLE(status);
    return value.IsNull() ? 0.0 : value.GetDouble();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Shaun.Sewall                    08/16
+---------------+---------------+---------------+---------------+---------------+------*/
int32_t DgnElement::GetPropertyValueInt32(Utf8CP propertyName, PropertyArrayIndex const& arrayIdx) const
    {
    ECN::ECValue value;
    DgnDbStatus status = GetPropertyValue(value, propertyName, arrayIdx);
    BeAssert(DgnDbStatus::Success == status);
    UNUSED_VARIABLE(status);
    return value.IsNull() ? 0 : value.GetInteger();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Shaun.Sewall                    08/16
+---------------+---------------+---------------+---------------+---------------+------*/
uint64_t DgnElement::GetPropertyValueUInt64(Utf8CP propertyName, PropertyArrayIndex const& arrayIdx) const
    {
    ECN::ECValue value;
    DgnDbStatus status = GetPropertyValue(value, propertyName, arrayIdx);
    BeAssert(DgnDbStatus::Success == status);
    UNUSED_VARIABLE(status);
    return value.IsNull() ? 0 : static_cast<uint64_t>(value.GetLong());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Shaun.Sewall                    08/16
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String DgnElement::GetPropertyValueString(Utf8CP propertyName, PropertyArrayIndex const& arrayIdx) const
    {
    ECN::ECValue value;
    DgnDbStatus status = GetPropertyValue(value, propertyName, arrayIdx);
    BeAssert(DgnDbStatus::Success == status);
    UNUSED_VARIABLE(status);
    return value.GetUtf8CP();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      08/16
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus DgnElement::SetPropertyValue(Utf8CP propertyName, DateTimeCR value, PropertyArrayIndex const& arrayIdx)
    {
    DgnDbStatus status = SetPropertyValue(propertyName, ECValue(value), arrayIdx);
    BeAssert(DgnDbStatus::Success == status);
    return status;
    }
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      08/16
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus DgnElement::SetPropertyValue(Utf8CP propertyName, DPoint3dCR pt, PropertyArrayIndex const& arrayIdx)
    {
    DgnDbStatus status = SetPropertyValue(propertyName, ECValue(pt), arrayIdx);
    BeAssert(DgnDbStatus::Success == status);
    return status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      08/16
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus DgnElement::SetPropertyValue(Utf8CP propertyName, DPoint2dCR pt, PropertyArrayIndex const& arrayIdx)
    {
    DgnDbStatus status = SetPropertyValue(propertyName, ECValue(pt), arrayIdx);
    BeAssert(DgnDbStatus::Success == status);
    return status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Shaun.Sewall                    08/16
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus DgnElement::SetPropertyValue(Utf8CP propertyName, bool value, PropertyArrayIndex const& arrayIdx)
    {
    DgnDbStatus status = SetPropertyValue(propertyName, ECValue(value), arrayIdx);
    BeAssert(DgnDbStatus::Success == status);
    return status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Shaun.Sewall                    08/16
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus DgnElement::SetPropertyValue(Utf8CP propertyName, double value, PropertyArrayIndex const& arrayIdx)
    {
    DgnDbStatus status = SetPropertyValue(propertyName, ECValue(value), arrayIdx);
    BeAssert(DgnDbStatus::Success == status);
    return status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Shaun.Sewall                    08/16
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus DgnElement::SetPropertyValue(Utf8CP propertyName, int32_t value, PropertyArrayIndex const& arrayIdx)
    {
    DgnDbStatus status = SetPropertyValue(propertyName, ECValue(value), arrayIdx);
    BeAssert(DgnDbStatus::Success == status);
    return status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Shaun.Sewall                    08/16
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus DgnElement::SetPropertyValue(Utf8CP propertyName, BeInt64Id id, PropertyArrayIndex const& arrayIdx)
    {
    ECValue value(id.GetValueUnchecked());
    if (!id.IsValid())
        value.SetToNull();

    DgnDbStatus status = SetPropertyValue(propertyName, value, arrayIdx);
    BeAssert(DgnDbStatus::Success == status);
    return status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Shaun.Sewall                    08/16
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus DgnElement::SetPropertyValue(Utf8CP propertyName, Utf8CP value, PropertyArrayIndex const& arrayIdx)
    {
    DgnDbStatus status = SetPropertyValue(propertyName, ECValue(value, false), arrayIdx);
    BeAssert(DgnDbStatus::Success == status);
    return status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson      06/16
+---------------+---------------+---------------+---------------+---------------+------*/
YawPitchRollAngles DgnElement::GetPropertyValueYpr(Utf8CP yawName, Utf8CP pitchName, Utf8CP rollName) const
    {
    YawPitchRollAngles angles;
    angles.SetYaw  (AngleInDegrees::FromDegrees(GetPropertyValueDouble(yawName)));
    angles.SetPitch(AngleInDegrees::FromDegrees(GetPropertyValueDouble(pitchName)));
    angles.SetRoll (AngleInDegrees::FromDegrees(GetPropertyValueDouble(rollName)));
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
DgnDbStatus DgnElement::_InsertPropertyArrayItems (uint32_t propertyIndex, uint32_t index, uint32_t size)
    {
    // Only auto-handled properties can be arrays
    ElementAutoHandledPropertiesECInstanceAdapter ecPropAccess(*this);
    if (!ecPropAccess.IsValid())
        return DgnDbStatus::BadArg;
    return toDgnDbWriteStatus(ecPropAccess._InsertArrayElements(propertyIndex, index, size));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson      06/16
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus DgnElement::_AddPropertyArrayItems (uint32_t propertyIndex, uint32_t size)
    {
    // Only auto-handled properties can be arrays
    ElementAutoHandledPropertiesECInstanceAdapter ecPropAccess(*this);
    if (!ecPropAccess.IsValid())
        return DgnDbStatus::BadArg;
    return toDgnDbWriteStatus(ecPropAccess._AddArrayElements(propertyIndex, size));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson      06/16
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus DgnElement::_RemovePropertyArrayItem (uint32_t propertyIndex, uint32_t index)
    {
    // Only auto-handled properties can be arrays
    ElementAutoHandledPropertiesECInstanceAdapter ecPropAccess(*this);
    if (!ecPropAccess.IsValid())
        return DgnDbStatus::BadArg;
    return toDgnDbWriteStatus(ecPropAccess._RemoveArrayElement(propertyIndex, index));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson      06/16
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus DgnElement::_ClearPropertyArray (uint32_t propertyIndex)
    {
    // Only auto-handled properties can be arrays
    ElementAutoHandledPropertiesECInstanceAdapter ecPropAccess(*this);
    if (!ecPropAccess.IsValid())
        return DgnDbStatus::BadArg;
    return toDgnDbWriteStatus(ecPropAccess._ClearArray(propertyIndex));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson      06/16
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus DgnElement::GetPropertyIndex(uint32_t& index, Utf8CP accessString)
    {
    auto& classLayout = GetElementClass()->GetDefaultStandaloneEnabler()->GetClassLayout();
    return toDgnDbWriteStatus(classLayout.GetPropertyIndex(index, accessString));
    }

DgnDbStatus DgnElement::SetPropertyValues(ECN::IECInstanceCR instance, SetPropertyFilter const& filter) 
    {return _SetPropertyValues(instance, filter);}

DgnDbStatus DgnElement::InsertPropertyArrayItems(uint32_t propertyIndex, uint32_t index, uint32_t size)
    {return _InsertPropertyArrayItems(propertyIndex, index, size);}

DgnDbStatus DgnElement::RemovePropertyArrayItem(uint32_t propertyIndex, uint32_t index)
    {return _RemovePropertyArrayItem(propertyIndex, index);}

DgnDbStatus DgnElement::AddPropertyArrayItems(uint32_t propertyIndex, uint32_t size)
    {return _AddPropertyArrayItems(propertyIndex, size);}

DgnDbStatus DgnElement::ClearPropertyArray(uint32_t propertyIndex)
    {return _ClearPropertyArray(propertyIndex);}

DgnDbStatus DgnElement::GetPropertyValue(ECN::ECValueR value, Utf8CP name, PropertyArrayIndex aidx) const 
    {return _GetPropertyValue(value, name, aidx);}

DgnDbStatus DgnElement::SetPropertyValue(Utf8CP name, ECN::ECValueCR value, PropertyArrayIndex aidx)
    {return _SetPropertyValue(name, value, aidx);}

ClassLayoutHelper::ClassLayoutHelper(DgnDbCR db, Utf8CP schemaName, Utf8CP className)
    {
    m_layout = &db.Schemas().GetECClass(schemaName, className)->GetDefaultStandaloneEnabler()->GetClassLayout();
    }

ClassLayoutHelper::ClassLayoutHelper(ECClassCR eclass)
    {
    m_layout = &eclass.GetDefaultStandaloneEnabler()->GetClassLayout();
    }

uint32_t ClassLayoutHelper::GetPropertyIndex(Utf8CP propName)
    {
    uint32_t propIdx;
    auto status = m_layout->GetPropertyIndex(propIdx, propName);
    BeAssert(ECObjectsStatus::Success == status);
    return propIdx;
    }

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

bpair<ECSqlClassInfo::T_ElementPropGet,ECSqlClassInfo::T_ElementPropSet> const* ECSqlClassInfo::GetPropertyAccessors(uint32_t propIdx) const 
    {
    auto i = m_propertyAccessors.find(propIdx);
    return (i != m_propertyAccessors.end())? &i->second: nullptr;
    }