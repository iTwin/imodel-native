/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "ECObjectsPch.h"
#include <ECObjects/ECSchema.h>
#include <GeomSerialization/GeomSerializationApi.h>
#include <Bentley/Base64Utilities.h>

DEFINE_KEY_METHOD(ECN::IECRelationshipInstance)

BEGIN_BENTLEY_ECOBJECT_NAMESPACE

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
CustomStructSerializerManager::CustomStructSerializerManager()
    {
    // we could add needed serializers here.
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
CustomStructSerializerManager::~CustomStructSerializerManager()
    {
    m_serializers.clear();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus            CustomStructSerializerManager::AddCustomSerializer(Utf8CP serializerName, ICustomECStructSerializerP serializer)
    {
    if (GetCustomSerializer(serializerName))
        return ERROR;

    m_serializers[Utf8String(serializerName)] = serializer;
    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
CustomStructSerializerManagerR                   CustomStructSerializerManager::GetManager()
    {
    static CustomStructSerializerManagerP   s_serializerManager = NULL;

    if (NULL == s_serializerManager)
        s_serializerManager = new CustomStructSerializerManager();

    return *s_serializerManager;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ICustomECStructSerializerP                      CustomStructSerializerManager::GetCustomSerializer(Utf8CP serializerName) const
    {
    if (m_serializers.empty())
        return NULL;

    NameSerializerMap::const_iterator it = m_serializers.find(serializerName);
    if (it == m_serializers.end())
        return NULL;

    return it->second;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ICustomECStructSerializerP                      CustomStructSerializerManager::GetCustomSerializer(StructECPropertyP structProperty, IECInstanceCR ecInstance) const
    {
    if (m_serializers.empty())
        return NULL;

    // see if the struct has a custom attribute to custom serialize itself
    IECInstancePtr caInstance = structProperty->GetType().GetCustomAttribute("Bentley_Standard_CustomAttributes", "CustomStructSerializer");
    if (caInstance.IsValid())
        {
        ECValue value;
        if (ECObjectsStatus::Success == caInstance->GetValue(value, "SerializerName"))
            {
            ICustomECStructSerializerP serializerP = GetCustomSerializer(value.GetUtf8CP());
            if (serializerP)
                {
                // let the serializer decide if it wants to process the struct from this type of IECInstance
                if (serializerP->UsesCustomStructXmlString(structProperty, ecInstance))
                    return serializerP;
                }
            }
        }

    return NULL;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool IECInstance::IsFixedArrayProperty(ECN::IECInstanceR instance, Utf8CP accessString, uint32_t* numFixedEntries)
    {
    ECValue         arrayVal;

    if (ECObjectsStatus::Success != instance.GetValue(arrayVal, accessString))
        return false;

    if (!arrayVal.IsArray())
        return false;

    ArrayInfo info = arrayVal.GetArrayInfo();

    if (!info.IsFixedCount())
        return false;

    if (numFixedEntries)
        *numFixedEntries = info.GetCount();

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
IECInstance::IECInstance()
    {
    BeAssert(sizeof(IECInstance) == sizeof(RefCountedBase) && "Increasing the size or memory layout of the base ECN::IECInstance will adversely affect subclasses. Think of this as a pure interface... to which you would never be able to add (additional) data, either");
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
IECInstance::~IECInstance()
    {}

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool IsExcluded(Utf8String& className, bvector<Utf8String>& classNamesToExclude)
    {
    for (Utf8String excludedClass : classNamesToExclude)
        {
        if (0 == className.compare(excludedClass))
            return true;
        }

    return false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
IECInstancePtr IECInstance::CreateCopyThroughSerialization()
    {
    return CreateCopyThroughSerialization(GetClass().GetSchema());
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------+---------------+---------------+---------------+---------------+-------
IECInstancePtr IECInstance::CreateCopyThroughSerialization(ECSchemaCR targetSchema)
    {
    Utf8String ecInstanceXml;

    if (targetSchema.GetECVersion() == ECVersion::V2_0)
        this->WriteToXmlString(ecInstanceXml, true, false);
    else
        this->WriteToXmlStringLatestVersion(ecInstanceXml, true, false);

    ECInstanceReadContextPtr instanceContext = ECInstanceReadContext::CreateContext(targetSchema);

    IECInstancePtr deserializedInstance;
    IECInstance::ReadFromXmlString(deserializedInstance, ecInstanceXml.c_str(), *instanceContext);

    return deserializedInstance;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus        IECInstance::_SetInstanceId(Utf8CP id) { return ECObjectsStatus::OperationNotSupported; }
ECObjectsStatus        IECInstance::SetInstanceId(Utf8CP id) { return _SetInstanceId(id); }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String        IECInstance::GetInstanceId() const
    {
    return _GetInstanceId();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String     IECInstance::GetInstanceIdForSerialization() const
    {
    return _GetInstanceIdForSerialization();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ECClassCR       IECInstance::GetClass() const
    {
    ECEnablerCR enabler = GetEnabler();

    return enabler.GetClass();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
MemoryECInstanceBase* IECInstance::_GetAsMemoryECInstance() const
    {
    return NULL;    // default to NULL and let real MemoryECInstanceBased classes override this method.
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ECDBuffer* IECInstance::_GetECDBuffer() const
    {
    return NULL;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
size_t IECInstance::GetOffsetToIECInstance() const
    {
    return _GetOffsetToIECInstance();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool                IECInstance::_IsPropertyReadOnly(Utf8CP accessString) const
    {
    /*
        if (_IsReadOnly())
            return true;
    */

    uint32_t propertyIndex;
    return ECObjectsStatus::Success != GetEnabler().GetPropertyIndex(propertyIndex, accessString) || IsPropertyReadOnly(propertyIndex);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool                IECInstance::_IsPropertyReadOnly(uint32_t propertyIndex) const
    {
    return /*_IsReadOnly() || */ GetEnabler().IsPropertyReadOnly(propertyIndex);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ECEnablerCR           IECInstance::GetEnabler() const { return _GetEnabler(); }
ECEnablerR            IECInstance::GetEnablerR() const { return *const_cast<ECEnablerP>(&_GetEnabler()); }
bool                  IECInstance::IsReadOnly() const { return _IsReadOnly(); }
ECDBuffer const*            IECInstance::GetECDBuffer() const { return _GetECDBuffer(); }
ECDBuffer*                  IECInstance::GetECDBufferP() { return _GetECDBuffer(); }
MemoryECInstanceBase const* IECInstance::GetAsMemoryECInstance() const { return _GetAsMemoryECInstance(); }
MemoryECInstanceBase*       IECInstance::GetAsMemoryECInstanceP() { return _GetAsMemoryECInstance(); }


//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
ECObjectsStatus IECInstance::GetQuantity(Units::QuantityR q, Utf8CP propertyAccessString, bool useArrayIndex, uint32_t arrayIndex) const
    {
    ECPropertyCP prop = GetEnabler().LookupECProperty(propertyAccessString);
    if (nullptr == prop)
        return ECObjectsStatus::PropertyNotFound;

    KindOfQuantityCP koq = prop->GetKindOfQuantity();
    if (nullptr == koq)
        return ECObjectsStatus::PropertyHasNoKindOfQuantity;

    uint32_t propertyIndex = 0;
    ECObjectsStatus status = GetEnabler().GetPropertyIndex(propertyIndex, propertyAccessString);

    if (ECObjectsStatus::Success != status)
        return status;

    ECValue v;
    if (ECObjectsStatus::Success != (status = GetValue(v, propertyIndex, useArrayIndex, arrayIndex)))
        return status;

    if (v.IsNull())
        {
        q = Units::Quantity(0.0, *koq->GetPersistenceUnit());
        return ECObjectsStatus::PropertyValueNull;
        }

    if (!v.IsDouble())
        {
        LOG.errorv("Could not get the quantity for property '%s' because it is not a double", prop->GetName().c_str());
        return ECObjectsStatus::DataTypeNotSupported;
        }

    q = Units::Quantity(v.GetDouble(), *koq->GetPersistenceUnit());
    return ECObjectsStatus::Success;
    }
//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
ECObjectsStatus IECInstance::GetQuantity(Units::QuantityR q, Utf8CP propertyAccessString, uint32_t arrayIndex) const
    {
    return GetQuantity(q, propertyAccessString, true, arrayIndex);
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
ECObjectsStatus IECInstance::GetQuantity(Units::QuantityR q, Utf8CP propertyAccessString) const
    {
    return GetQuantity(q, propertyAccessString, false, 42);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus     IECInstance::GetValue(ECValueR v, Utf8CP propertyAccessString) const
    {
    uint32_t propertyIndex = 0;
    ECObjectsStatus status = GetEnabler().GetPropertyIndex(propertyIndex, propertyAccessString);

    if (ECObjectsStatus::Success != status)
        return status;

    return GetValue(v, propertyIndex, false, 0);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus     IECInstance::GetValueOrAdHoc(ECValueR v, Utf8CP accessString) const
    {
    auto status = GetValue(v, accessString);
    if (ECObjectsStatus::PropertyNotFound == status)
        {
        for (auto const& containerIndex : AdHocContainerPropertyIndexCollection(GetEnabler()))
            {
            AdHocPropertyQuery adHocs(*this, containerIndex);
            uint32_t propertyIndex;
            if (adHocs.GetPropertyIndex(propertyIndex, accessString))
                status = adHocs.GetValue(v, propertyIndex);
            }
        }

    return status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus     IECInstance::GetValue(ECValueR v, Utf8CP propertyAccessString, uint32_t arrayIndex) const
    {
    uint32_t propertyIndex = 0;
    ECObjectsStatus status = GetEnabler().GetPropertyIndex(propertyIndex, propertyAccessString);

    if (ECObjectsStatus::Success != status)
        return status;

    return GetValue(v, propertyIndex, true, arrayIndex);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus     IECInstance::GetValue(ECValueR v, uint32_t propertyIndex) const
    {
    return GetValue(v, propertyIndex, false, 0);
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus     IECInstance::GetValue(ECValueR v, uint32_t propertyIndex, uint32_t arrayIndex) const
    {
    return GetValue(v, propertyIndex, true, arrayIndex);
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
ECObjectsStatus     IECInstance::GetValue(ECValueR v, uint32_t propertyIndex, bool useArrayIndex, uint32_t arrayIndex) const
    {
    ECObjectsStatus stat = _GetValue(v, propertyIndex, useArrayIndex, arrayIndex);
    if (stat != ECObjectsStatus::Success)
        {
        return stat;
        }

    if (!v.IsDateTime())
        {
        return stat;
        }

    return SetDateTimeMetadataInECValue(v, propertyIndex);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus     IECInstance::SetInternalValue(Utf8CP propertyAccessString, ECValueCR v)
    {
    uint32_t propertyIndex = 0;
    ECObjectsStatus status = GetEnabler().GetPropertyIndex(propertyIndex, propertyAccessString);

    if (ECObjectsStatus::Success != status)
        return status;

    return SetInternalValue(propertyIndex, v);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus     IECInstance::ChangeValue(Utf8CP propertyAccessString, ECValueCR v)
    {
    uint32_t propertyIndex = 0;
    ECObjectsStatus status = GetEnabler().GetPropertyIndex(propertyIndex, propertyAccessString);

    if (ECObjectsStatus::Success != status)
        return status;

    return ChangeValue(propertyIndex, v);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus IECInstance::ChangeValueOrAdHoc(Utf8CP propertyAccessString, ECValueCR v)
    {
    auto status = ChangeValue(propertyAccessString, v);
    if (ECObjectsStatus::PropertyNotFound == status)
        {
        for (auto const& containerIndex : AdHocContainerPropertyIndexCollection(GetEnabler()))
            {
            AdHocPropertyEdit adHocs(*this, containerIndex);
            uint32_t propertyIndex;
            if (adHocs.GetPropertyIndex(propertyIndex, propertyAccessString))
                {
                bool isReadOnly = true;
                status = adHocs.IsReadOnly(isReadOnly, propertyIndex);
                if (ECObjectsStatus::Success == status)
                    {
                    if (isReadOnly)
                        status = ECObjectsStatus::UnableToSetReadOnlyProperty;
                    else
                        {
                        ECValue curV;
                        if (ECObjectsStatus::Success == adHocs.GetValue(curV, propertyIndex) && curV.Equals(v))
                            status = ECObjectsStatus::PropertyValueMatchesNoChange;
                        else
                            status = adHocs.SetValue(propertyIndex, v);
                        }
                    }
                }
            }
        }

    return status;
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
ECObjectsStatus    IECInstance::SetQuantity(Utf8CP propertyAccessString, Units::QuantityCR q, bool useArrayIndex, uint32_t arrayIndex)
    {
    ECPropertyCP prop = GetEnabler().LookupECProperty(propertyAccessString);
    if (nullptr == prop)
        return ECObjectsStatus::PropertyNotFound;

    KindOfQuantityCP koq = prop->GetKindOfQuantity();
    if (nullptr == koq)
        return ECObjectsStatus::PropertyHasNoKindOfQuantity;

    Units::Quantity converted = q.ConvertTo(koq->GetPersistenceUnit());
    if (!converted.IsValid())
        return ECObjectsStatus::KindOfQuantityNotCompatible;

    uint32_t propertyIndex = 0;
    ECObjectsStatus status = GetEnabler().GetPropertyIndex(propertyIndex, propertyAccessString);
    if (ECObjectsStatus::Success != status)
        return status;

    ECValue v(converted.GetMagnitude());
    status = ChangeValue(propertyIndex, v, useArrayIndex, arrayIndex);
    if (ECObjectsStatus::PropertyValueMatchesNoChange == status)
        return ECObjectsStatus::Success;

    return status;
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
ECObjectsStatus    IECInstance::SetQuantity(Utf8CP propertyAccessString, Units::QuantityCR q, uint32_t arrayIndex)
    {
    return SetQuantity(propertyAccessString, q, true, arrayIndex);
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
ECObjectsStatus    IECInstance::SetQuantity(Utf8CP propertyAccessString, Units::QuantityCR q)
    {
    return SetQuantity(propertyAccessString, q, false, 42);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus     IECInstance::SetValue(Utf8CP propertyAccessString, ECValueCR v)
    {
    ECObjectsStatus status = ChangeValue(propertyAccessString, v);

    if (ECObjectsStatus::PropertyValueMatchesNoChange == status)
        return ECObjectsStatus::Success;

    return status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus IECInstance::SetValueOrAdHoc(Utf8CP propertyAccessString, ECValueCR v)
    {
    auto status = ChangeValueOrAdHoc(propertyAccessString, v);
    if (ECObjectsStatus::PropertyValueMatchesNoChange == status)
        return ECObjectsStatus::Success;

    return status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus     IECInstance::SetInternalValue(Utf8CP propertyAccessString, ECValueCR v, uint32_t arrayIndex)
    {
    uint32_t propertyIndex = 0;
    ECObjectsStatus status = GetEnabler().GetPropertyIndex(propertyIndex, propertyAccessString);

    if (ECObjectsStatus::Success != status)
        return status;

    return SetInternalValue(propertyIndex, v, arrayIndex);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus     IECInstance::ChangeValue(Utf8CP propertyAccessString, ECValueCR v, uint32_t arrayIndex)
    {
    uint32_t propertyIndex = 0;
    ECObjectsStatus status = GetEnabler().GetPropertyIndex(propertyIndex, propertyAccessString);

    if (ECObjectsStatus::Success != status)
        return status;

    return ChangeValue(propertyIndex, v, arrayIndex);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus     IECInstance::SetValue(Utf8CP propertyAccessString, ECValueCR v, uint32_t arrayIndex)
    {
    ECObjectsStatus status = ChangeValue(propertyAccessString, v, arrayIndex);

    if (ECObjectsStatus::PropertyValueMatchesNoChange == status)
        return ECObjectsStatus::Success;

    return status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus     IECInstance::SetInternalValue(uint32_t propertyIndex, ECValueCR v)
    {
    auto status = _SetInternalValue(propertyIndex, v, false, 0);

    if (ECObjectsStatus::PropertyValueMatchesNoChange == status)
        status = ECObjectsStatus::Success;

    return status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus     IECInstance::SetValue(uint32_t propertyIndex, ECValueCR v)
    {
    ECObjectsStatus status = ChangeValue(propertyIndex, v);

    if (ECObjectsStatus::PropertyValueMatchesNoChange == status)
        return ECObjectsStatus::Success;

    return  status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus     IECInstance::SetInternalValue(uint32_t propertyIndex, ECValueCR v, uint32_t arrayIndex)
    {

    return _SetInternalValue(propertyIndex, v, true, arrayIndex);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus IECInstance::_SetInternalValue(uint32_t propertyIndex, ECValueCR v, bool useArrayIndex, uint32_t arrayIndex)
    {
    // Default impl; instances that support calculated properties should override
    return _SetValue(propertyIndex, v, useArrayIndex, arrayIndex);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus     IECInstance::ChangeValue(uint32_t propertyIndex, ECValueCR v, uint32_t arrayIndex)
    {
    return ChangeValue(propertyIndex, v, true, arrayIndex);
    }

//--------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+-----
ECObjectsStatus     IECInstance::ChangeValue(uint32_t propertyIndex, ECValueCR v)
    {
    return ChangeValue(propertyIndex, v, false, 0);
    }

//--------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+-----
ECObjectsStatus     IECInstance::ChangeValue(uint32_t propertyIndex, ECValueCR v, bool useArrayIndex, uint32_t arrayIndex)
    {
    if (!ChangeValuesAllowed())
        return ECObjectsStatus::UnableToSetReadOnlyInstance;

    bool isNull = false;
    ECObjectsStatus status = GetIsPropertyNull(isNull, propertyIndex, useArrayIndex, arrayIndex);
    if (status != ECObjectsStatus::Success)
        return status;

    if (IsPropertyReadOnly(propertyIndex) && !isNull)
        return ECObjectsStatus::UnableToSetReadOnlyProperty;

    if (v.IsDateTime())
        {
        status = ValidateDateTimeMetadata(propertyIndex, v);
        if (status != ECObjectsStatus::Success)
            {
            return status;
            }
        }

    if (v.IsNavigation())
        {
        status = ValidateNavigationMetadata(propertyIndex, v);
        if (status != ECObjectsStatus::Success)
            return status;
        }

    return _SetValue(propertyIndex, v, useArrayIndex, arrayIndex);
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus     IECInstance::SetValue(uint32_t propertyIndex, ECValueCR v, uint32_t arrayIndex)
    {
    ECObjectsStatus status = ChangeValue(propertyIndex, v, arrayIndex);

    if (ECObjectsStatus::PropertyValueMatchesNoChange == status)
        return ECObjectsStatus::Success;

    return  status;
    }

bool                IECInstance::IsPropertyReadOnly(uint32_t propertyIndex) const { return _IsPropertyReadOnly(propertyIndex); }
bool                IECInstance::IsPropertyReadOnly(Utf8CP accessString) const { return _IsPropertyReadOnly(accessString); }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool IECInstance::IsPropertyOrAdHocReadOnly(Utf8CP accessString) const
    {
    uint32_t propertyIndex;
    auto status = GetEnabler().GetPropertyIndex(propertyIndex, accessString);
    if (ECObjectsStatus::PropertyNotFound == status)
        {
        bool readOnly = true;
        for (auto const& containerIndex : AdHocContainerPropertyIndexCollection(GetEnabler()))
            {
            AdHocPropertyQuery adHocs(*this, containerIndex);
            if (adHocs.GetPropertyIndex(propertyIndex, accessString))
                {
                status = adHocs.IsReadOnly(readOnly, propertyIndex);
                break;
                }
            }

        return ECObjectsStatus::Success != status || readOnly;
        }

    return ECObjectsStatus::Success != status || IsPropertyReadOnly(propertyIndex);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/

#define NUM_INDEX_BUFFER_CHARS 63
#define NUM_ACCESSSTRING_BUFFER_CHARS 1023

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static ECPropertyP getProperty(ECClassCR ecClass, Utf8CP accessor, Utf8Char* buffer)
    {
    //Gets the ECProperty for a full native accessor.
    //For example, the full native accessor could be "GrandfatherStruct.ParentStruct.StringMember"
    //In this case, passing this accessor to this function will give you the
    //ECProperty for StringMember.

    Utf8CP dotPos = strchr(accessor, '.');
    if (NULL != dotPos)
        {
        size_t dotIndex = dotPos - accessor;
        buffer[dotIndex] = '\0';

        ECPropertyP prop = ecClass.GetPropertyP(buffer);

        if (NULL == prop)
            return NULL;

        StructECPropertyP structProperty = prop->GetAsStructPropertyP();

        if (NULL == structProperty)
            return NULL;

        return getProperty(structProperty->GetType(), &dotPos[1], &buffer[dotIndex + 1]);
        }

    Utf8CP bracketPos = strchr(accessor, '[');
    if (NULL != bracketPos)
        {
        size_t bracketIndex = bracketPos - accessor;
        buffer[bracketIndex] = '\0';
        }
    return ecClass.GetPropertyP(buffer);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static ECObjectsStatus getECValueUsingFullAccessString(Utf8P asBuffer, Utf8P indexBuffer, ECValueR v, IECInstanceCR instance, Utf8CP managedPropertyAccessor)
    {
    // see if access string specifies an array
    Utf8CP pos1 = strchr(managedPropertyAccessor, '[');

    // if not an array then
    if (NULL == pos1)
        return instance.GetValue(v, managedPropertyAccessor);

    size_t numChars = 0;
    numChars = pos1 - managedPropertyAccessor;
PUSH_DISABLE_DEPRECATION_WARNINGS
    strncpy(asBuffer, managedPropertyAccessor, numChars > NUM_ACCESSSTRING_BUFFER_CHARS ? NUM_ACCESSSTRING_BUFFER_CHARS : numChars);
POP_DISABLE_DEPRECATION_WARNINGS
    asBuffer[numChars] = 0;

    // BRACKETS_OKAY: Brackets contain an array index
    Utf8CP pos2 = strchr(pos1 + 1, ']');

    BeAssert(pos2 != NULL);

    numChars = pos2 - pos1 - 1;

PUSH_DISABLE_DEPRECATION_WARNINGS
    strncpy(indexBuffer, pos1 + 1, numChars > NUM_INDEX_BUFFER_CHARS ? NUM_INDEX_BUFFER_CHARS : numChars);
POP_DISABLE_DEPRECATION_WARNINGS
    indexBuffer[numChars] = 0;

    uint32_t indexValue = -1;
    Utf8String::Sscanf_safe(indexBuffer, "%ud", &indexValue);

    ECValue         arrayVal;
    ECObjectsStatus status;

    if (ECObjectsStatus::Success != (status = instance.GetValue(arrayVal, asBuffer)))
        return status;

    if (-1 == indexValue)
        {
        //Caller asked for the array itself, not any particular element.
        //Returns a dummy ECValue with only the array info copied.
        ArrayInfo info = arrayVal.GetArrayInfo();
        if (info.IsStructArray())
            v.SetStructArrayInfo(info.GetCount(), info.IsFixedCount());
        else
            v.SetPrimitiveArrayInfo(info.GetElementPrimitiveType(), info.GetCount(), info.IsFixedCount());
        return ECObjectsStatus::Success;
        }

    ArrayInfo arrayInfo = arrayVal.GetArrayInfo();
    uint32_t  size = arrayInfo.GetCount();

    if (indexValue >= size)
        return ECObjectsStatus::Error;

    if (arrayInfo.IsPrimitiveArray())
        return instance.GetValue(v, asBuffer, indexValue);

    // must be a struct array

    if (ECObjectsStatus::Success != (status = instance.GetValue(arrayVal, asBuffer, indexValue)))
        return status;

    // If there is no '.' in the rest of the access string, the caller was requesting the value representing the struct
    // array element itself, not the value of any of its members.
    if ('\0' == pos2[1])
        {
        v.SetStruct(arrayVal.GetStruct().get());
        return ECObjectsStatus::Success;
        }

    IECInstancePtr arrayEntryInstance = arrayVal.GetStruct();

    return getECValueUsingFullAccessString(asBuffer, indexBuffer, v, *arrayEntryInstance, pos2 + 2); // move to character after "]." in access string.
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static ECObjectsStatus getECValueFromInstance(ECValueR v, IECInstanceCR instance, Utf8CP managedPropertyAccessor)
    {
    WString asBufferStr;

    v.Clear();
    Utf8Char asBuffer[NUM_ACCESSSTRING_BUFFER_CHARS + 1];
    Utf8Char indexBuffer[NUM_INDEX_BUFFER_CHARS + 1];

    return getECValueUsingFullAccessString(asBuffer, indexBuffer, v, instance, managedPropertyAccessor);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static ECObjectsStatus          getValueHelper(ECValueR value, IECInstanceCR instance, ECValueAccessorCR accessor, uint32_t depth, bool compatible)
    {
    ECValueAccessor::Location const& loc = accessor[depth];
    int arrayIndex = loc.GetArrayIndex();
    if (compatible)
        {
        uint32_t propertyIndex = (uint32_t) loc.GetPropertyIndex();
        if (arrayIndex < 0)
            return instance.GetValue(value, propertyIndex);
        return instance.GetValue(value, propertyIndex, (uint32_t) arrayIndex);
        }

    Utf8CP accessString = accessor.GetAccessString(depth);
    if (NULL == accessString)
        return ECObjectsStatus::Error;

    if (arrayIndex < 0)
        return instance.GetValue(value, accessString);

    return instance.GetValue(value, accessString, (uint32_t) arrayIndex);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static ECObjectsStatus          setValueHelper(IECInstanceR instance, ECValueAccessorCR accessor, uint32_t depth, bool compatible, ECValueCR value)
    {
    int arrayIndex = accessor[depth].GetArrayIndex();
    if (compatible)
        {
        uint32_t propertyIndex = (uint32_t) accessor[depth].GetPropertyIndex();

        if (arrayIndex < 0)
            {
            return instance.SetValue(propertyIndex, value);
            }
        return instance.SetValue(propertyIndex, value, (uint32_t) arrayIndex);
        }

    // not the same enabler between accessor and instance so use access string to set value
    Utf8CP accessString = accessor.GetAccessString(depth);
    if (NULL == accessString)
        return ECObjectsStatus::Error;

    if (arrayIndex < 0)
        return instance.SetValue(accessString, value);

    return instance.SetValue(accessString, value, (uint32_t) arrayIndex);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static ECObjectsStatus          setInternalValueHelper(IECInstanceR instance, ECValueAccessorCR accessor, uint32_t depth, bool compatible, ECValueCR value)
    {
    int arrayIndex = accessor[depth].GetArrayIndex();
    if (compatible)
        {
        uint32_t propertyIndex = (uint32_t) accessor[depth].GetPropertyIndex();

        if (arrayIndex < 0)
            {
            return instance.SetInternalValue(propertyIndex, value);
            }

        return instance.SetInternalValue(propertyIndex, value, (uint32_t) arrayIndex);
        }

    // not the same enabler between accessor and instance so use access string to set value
    Utf8CP accessString = accessor.GetAccessString(depth);
    if (NULL == accessString)
        return ECObjectsStatus::Error;

    if (arrayIndex < 0)
        return instance.SetInternalValue(accessString, value);

    return instance.SetInternalValue(accessString, value, (uint32_t) arrayIndex);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus           IECInstance::GetValueUsingAccessor(ECValueR v, ECValueAccessorCR accessor) const
    {
    if (accessor.IsAdHocProperty())
        {
        // The array index is already pointing to the index of the desired ad-hoc property
        if (1 != accessor.GetDepth() || ECValueAccessor::INDEX_ROOT == accessor[0].GetArrayIndex())
            {
            BeAssert(false);
            return ECObjectsStatus::Error;
            }

        AdHocPropertyQuery adHoc(*this, accessor[0].GetPropertyIndex());
        return adHoc.GetValue(v, accessor[0].GetArrayIndex());
        }

    ECObjectsStatus status = ECObjectsStatus::Success;
    IECInstancePtr  currentInstance = const_cast <IECInstance*> (this);
    for (uint32_t depth = 0; depth < accessor.GetDepth(); depth++)
        {
        v.Clear();
        bool compatible = (accessor[depth].GetEnabler() == &currentInstance->GetEnabler()); // if same enabler then use property index to set value else use access string

        status = getValueHelper(v, *currentInstance, accessor, depth, compatible);
        if (ECObjectsStatus::Success != status)
            {
            // if we're accessing a property of an embedded struct, we expect GetValue() to return a null struct - so continue
            ECPropertyCP ecprop = accessor[depth].GetECProperty();
            if (NULL != ecprop && ecprop->GetIsStruct())
                continue;
            else
                return status;
            }

        if (v.IsStruct() && accessor[depth].GetArrayIndex() >= 0)
            currentInstance = v.GetStruct();
        }

    return status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus           IECInstance::SetInternalValueUsingAccessor(ECValueAccessorCR accessor, ECValueCR valueToSet)
    {
    if (accessor.IsAdHocProperty())
        {
        // The array index is already pointing to the index of the desired ad-hoc property
        if (1 != accessor.GetDepth() || ECValueAccessor::INDEX_ROOT == accessor[0].GetArrayIndex())
            {
            BeAssert(false);
            return ECObjectsStatus::Error;
            }

        AdHocPropertyEdit adHoc(*this, accessor[0].GetPropertyIndex());
        return adHoc.SetValue(accessor[0].GetArrayIndex(), valueToSet);
        }

    ECObjectsStatus status = ECObjectsStatus::Success;
    IECInstancePtr  currentInstance = this;

    for (uint32_t depth = 0; depth < accessor.GetDepth(); depth++)
        {
        bool compatible = (accessor[depth].GetEnabler() == &currentInstance->GetEnabler()); // if same enabler then use property index to set value else use access string
        int  propertyIndex = accessor[depth].GetPropertyIndex();
        int  arrayIndex = accessor[depth].GetArrayIndex();

        if (arrayIndex > -1)
            {
            ECValue         arrayInfoPlaceholder;

            //Get the array value to check its size. Expand array if necessary.
            if (compatible)
                status = currentInstance->GetValue(arrayInfoPlaceholder, (uint32_t) propertyIndex);
            else
                status = currentInstance->GetValue(arrayInfoPlaceholder, accessor.GetAccessString(depth));

            if (ECObjectsStatus::Success != status)
                return status;

            uint32_t arraySize = arrayInfoPlaceholder.GetArrayInfo().GetCount();

            if ((uint32_t) arrayIndex >= arraySize)
                {
                if (arrayInfoPlaceholder.GetArrayInfo().IsFixedCount())
                    return ECObjectsStatus::IndexOutOfRange;

                uint32_t numToInsert = 1 + (uint32_t) arrayIndex - arraySize;

                Utf8CP accessorWithBrackets = accessor.GetAccessString(depth);
                if (NULL == accessorWithBrackets)
                    return ECObjectsStatus::Error;

                status = currentInstance->AddArrayElements(accessorWithBrackets, numToInsert);
                if (ECObjectsStatus::Success != status)
                    return status;
                }
            }

        // if we are processing the deepest location then set the value
        if (depth == (accessor.GetDepth() - 1))
            return setInternalValueHelper(*currentInstance, accessor, depth, compatible, valueToSet);

        // if we are not inside an array this is an embedded struct, go into it.
        if (0 > arrayIndex)
            continue;

        ECValue         structPlaceholder;

        // if we get here we are processing an array of structs.  Get the struct's ECInstance so we can use for the next location depth
        status = getValueHelper(structPlaceholder, *currentInstance, accessor, depth, compatible);
        if (ECObjectsStatus::Success != status)
            return status;

        BeAssert(structPlaceholder.IsStruct() && "Accessor depth is greater than expected.");

        IECInstancePtr newInstance = structPlaceholder.GetStruct();

        // If the struct does not have an instance associated with it then build the instance
        if (newInstance.IsNull())
            {
            ECN::ECEnablerR          structEnabler = *(const_cast<ECN::ECEnablerP>(&accessor.GetEnabler(depth + 1)));
            ECClassCR               structClass = accessor.GetEnabler(depth + 1).GetClass();
            StandaloneECEnablerPtr  standaloneEnabler = structEnabler.GetEnablerForStructArrayMember(structClass.GetSchema().GetSchemaKey(), structClass.GetName().c_str());

            newInstance = standaloneEnabler->CreateInstance();

            ECValue valueForSettingStructClass;
            valueForSettingStructClass.SetStruct(newInstance.get());

            status = setValueHelper(*currentInstance, accessor, depth, compatible, valueForSettingStructClass);
            if (ECObjectsStatus::Success != status)
                return status;

            // TFS#159623: Cannot use the StandaloneECInstance here...the containing ECInstance may have allocated a different ECInstance!
            // Obtain the struct ECInstance from the containing ECInstance.
            status = getValueHelper(valueForSettingStructClass, *currentInstance, accessor, depth, compatible);
            if (ECObjectsStatus::Success != status || valueForSettingStructClass.IsNull() || !valueForSettingStructClass.IsStruct())
                {
                BeAssert(false);
                return status;
                }

            newInstance = valueForSettingStructClass.GetStruct();
            }

        currentInstance = newInstance;
        }

    return status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus           IECInstance::SetValueUsingAccessor(ECValueAccessorCR accessor, ECValueCR valueToSet)
    {
    if (!ChangeValuesAllowed())
        return ECObjectsStatus::UnableToSetReadOnlyInstance;

    return  SetInternalValueUsingAccessor(accessor, valueToSet);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus    IECInstance::GetIsPropertyNull(bool& isNull, uint32_t propertyIndex, bool useArrayIndex, uint32_t arrayIndex) const
    {
    return  _GetIsPropertyNull(isNull, propertyIndex, useArrayIndex, arrayIndex);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus IECInstance::_GetIsPropertyNull(bool& isNull, uint32_t propertyIndex, bool useArrayIndex, uint32_t arrayIndex) const
    {
    // default implementation. ECD-based implementations can be more efficient.
    isNull = true;
    ECValue v;
    ECObjectsStatus status = _GetValue(v, propertyIndex, useArrayIndex, arrayIndex);
    if (ECObjectsStatus::Success == status)
        isNull = v.IsNull();

    return status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus     IECInstance::IsPropertyNull(bool& isNull, Utf8CP propertyAccessString) const
    {
    uint32_t propertyIndex = 0;
    ECObjectsStatus status = GetEnabler().GetPropertyIndex(propertyIndex, propertyAccessString);

    if (ECObjectsStatus::Success != status)
        return status;

    return _GetIsPropertyNull(isNull, propertyIndex, false, 0);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus     IECInstance::IsPropertyNull(bool& isNull, Utf8CP propertyAccessString, uint32_t arrayIndex) const
    {
    uint32_t propertyIndex = 0;
    ECObjectsStatus status = GetEnabler().GetPropertyIndex(propertyIndex, propertyAccessString);

    if (ECObjectsStatus::Success != status)
        return status;

    return _GetIsPropertyNull(isNull, propertyIndex, true, arrayIndex);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus     IECInstance::IsPropertyNull(bool& isNull, uint32_t propertyIndex) const
    {
    return _GetIsPropertyNull(isNull, propertyIndex, false, 0);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus     IECInstance::IsPropertyNull(bool& isNull, uint32_t propertyIndex, uint32_t arrayIndex) const
    {
    return _GetIsPropertyNull(isNull, propertyIndex, true, arrayIndex);
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
ECObjectsStatus IECInstance::ValidateDateTimeMetadata(uint32_t propertyIndex, ECValueCR v) const
    {
    if (v.IsDateTime() && !v.IsNull())
        {
        DateTime::Info dateTimeInfo;
        const ECObjectsStatus stat = GetDateTimeInfo(dateTimeInfo, propertyIndex);
        if (stat != ECObjectsStatus::Success)
            {
            LOG.error("Error retrieving the DateTimeInfo custom attribute from the respective ECProperty.");
            return stat;
            }

        if (dateTimeInfo.IsValid() && dateTimeInfo.GetKind() == DateTime::Kind::Local)
            {
            LOG.error("ECProperties with DateTime kind 'Local' not supported.");
            return ECObjectsStatus::DataTypeMismatch;
            }

        if (!v.DateTimeInfoMatches(dateTimeInfo))
            {
            LOG.error("Setting a DateTime ECValue in ECInstance failed. DateTime metadata in ECValue mismatches the DateTimeInfo custom attribute on the respective ECProperty");
            return ECObjectsStatus::DataTypeMismatch;
            }
        }

    return ECObjectsStatus::Success;
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
ECObjectsStatus IECInstance::SetDateTimeMetadataInECValue(ECValueR v, uint32_t propertyIndex) const
    {
    //only set date time meta data if the value is not null and if the metadata wasn't already set (by impl of _GetValue)
    if (!v.IsNull() && v.IsDateTime() && !v.IsDateTimeMetadataSet())
        {
        DateTime::Info caDateTimeMetadata;
        if (GetDateTimeInfo(caDateTimeMetadata, propertyIndex) == ECObjectsStatus::Success)
            {
            //fails if caDateTimeMetadata specified local DateTimeKind which is not supported
            if (SUCCESS != v.SetDateTimeMetadata(caDateTimeMetadata))
                return ECObjectsStatus::DataTypeNotSupported;
            }
        }

    return ECObjectsStatus::Success;
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
ECObjectsStatus IECInstance::GetDateTimeInfo(DateTime::Info& dateTimeInfo, uint32_t propertyIndex) const
    {
    //TODO: Need to profile this. The implementation does look up the access string from the prop index
    //and then parses to access string (to check whether it might refer to a struct member) before
    //actually calling ECClass::GetProperty
    ECPropertyCP ecProperty = GetEnabler().LookupECProperty(propertyIndex);
    if (ecProperty == NULL)
        return ECObjectsStatus::PropertyNotFound;

    return StandardCustomAttributeHelper::GetDateTimeInfo(dateTimeInfo, *ecProperty);
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
ECObjectsStatus IECInstance::ValidateNavigationMetadata(uint32_t propertyIndex, ECValueCR v) const
    {
    if (v.IsNavigation() && !v.IsNull())
        {
        ECRelationshipClassCP relClass = v.GetNavigationInfo().GetRelationshipClass();
        if (nullptr == relClass)
            return ECObjectsStatus::Success;

        ECPropertyCP ecProperty = GetEnabler().LookupECProperty(propertyIndex);
        if (nullptr == ecProperty)
            return ECObjectsStatus::DataTypeMismatch;

        NavigationECPropertyCP navProp = ecProperty->GetAsNavigationProperty();
        if (nullptr == navProp)
            return ECObjectsStatus::DataTypeMismatch;

        if (!relClass->Is(navProp->GetRelationshipClass()))
            return ECObjectsStatus::DataTypeMismatch;

        ECRelationshipConstraintCP constraint;
        if (ECRelatedInstanceDirection::Forward == navProp->GetDirection())
            constraint = &relClass->GetSource();
        else
            constraint = &relClass->GetTarget();

        if (!constraint->SupportsClass(GetEnabler().GetClass()))
            return ECObjectsStatus::DataTypeMismatch;
        }

    return ECObjectsStatus::Success;
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
ECObjectsStatus IECInstance::_GetShouldSerializeProperty(bool& serialize, uint32_t propertyIndex, bool useArrayIndex, uint32_t arrayIndex) const
    {
    // This is the default implementation. Using the ECDbuffer implementation will be more efficient.
    bool isNull;
    ECObjectsStatus status;
    if (ECObjectsStatus::Success != (status = _GetIsPropertyNull(isNull, propertyIndex, useArrayIndex, arrayIndex)))
        return status;

    serialize = !isNull;
    return status;
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
ECObjectsStatus IECInstance::ShouldSerializeProperty(bool& serialize, Utf8CP propertyAccessString) const
    {
    uint32_t propertyIndex = 0;
    ECObjectsStatus status = GetEnabler().GetPropertyIndex(propertyIndex, propertyAccessString);

    if (ECObjectsStatus::Success != status)
        return status;

    return _GetShouldSerializeProperty(serialize, propertyIndex, false, 0);
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
ECObjectsStatus IECInstance::ShouldSerializeProperty(bool& serialize, Utf8CP propertyAccessString, uint32_t arrayIndex) const
    {
    uint32_t propertyIndex = 0;
    ECObjectsStatus status = GetEnabler().GetPropertyIndex(propertyIndex, propertyAccessString);

    if (ECObjectsStatus::Success != status)
        return status;

    return _GetShouldSerializeProperty(serialize, propertyIndex, true, arrayIndex);
    }

/////////////////////////////////////////////////////////////////////////////////////////
//  ECInstanceInteropHelper
/////////////////////////////////////////////////////////////////////////////////////////
/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus                 ECInstanceInteropHelper::GetValue(IECInstanceCR instance, ECValueR value, Utf8CP managedPropertyAccessor)
    {
    return getECValueFromInstance(value, instance, managedPropertyAccessor);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus ECInstanceInteropHelper::GetLong(IECInstanceCR instance, int64_t & value, Utf8CP managedPropertyAccessor)
    {
    ECValue v;

    ECObjectsStatus status = getECValueFromInstance(v, instance, managedPropertyAccessor);
    if (status == ECObjectsStatus::Success)
        value = v.GetLong();

    return status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus ECInstanceInteropHelper::GetInteger(IECInstanceCR instance, int & value, Utf8CP managedPropertyAccessor)
    {
    ECValue v;

    ECObjectsStatus status = getECValueFromInstance(v, instance, managedPropertyAccessor);
    if (status == ECObjectsStatus::Success)
        value = v.GetInteger();

    return status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus ECInstanceInteropHelper::GetDouble(IECInstanceCR instance, double& value, Utf8CP managedPropertyAccessor)
    {
    ECValue v;

    ECObjectsStatus status = getECValueFromInstance(v, instance, managedPropertyAccessor);
    if (status == ECObjectsStatus::Success)
        value = v.GetDouble();

    return status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus ECInstanceInteropHelper::GetString(IECInstanceCR instance, Utf8StringR value, Utf8CP managedPropertyAccessor)
    {
    ECValue v;

    ECObjectsStatus status = getECValueFromInstance(v, instance, managedPropertyAccessor);
    if (status == ECObjectsStatus::Success)
        value = v.GetUtf8CP();

    return status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus ECInstanceInteropHelper::GetBoolean(IECInstanceCR instance, bool & value, Utf8CP managedPropertyAccessor)
    {
    ECValue v;

    ECObjectsStatus status = getECValueFromInstance(v, instance, managedPropertyAccessor);
    if (status == ECObjectsStatus::Success)
        value = v.GetBoolean();

    return status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus ECInstanceInteropHelper::GetPoint2d(IECInstanceCR instance, DPoint2d & value, Utf8CP managedPropertyAccessor)
    {
    ECValue v;

    ECObjectsStatus status = getECValueFromInstance(v, instance, managedPropertyAccessor);
    if (status == ECObjectsStatus::Success)
        value = v.GetPoint2d();

    return status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus ECInstanceInteropHelper::GetPoint3d(IECInstanceCR instance, DPoint3d & value, Utf8CP managedPropertyAccessor)
    {
    ECValue v;

    ECObjectsStatus status = getECValueFromInstance(v, instance, managedPropertyAccessor);
    if (status == ECObjectsStatus::Success)
        value = v.GetPoint3d();

    return status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus ECInstanceInteropHelper::GetDateTime(IECInstanceCR instance, DateTimeR value, Utf8CP managedPropertyAccessor)
    {
    ECValue v;

    ECObjectsStatus status = getECValueFromInstance(v, instance, managedPropertyAccessor);
    if (status == ECObjectsStatus::Success)
        value = v.GetDateTime();

    return status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus ECInstanceInteropHelper::GetDateTimeTicks(IECInstanceCR instance, int64_t & value, Utf8CP managedPropertyAccessor)
    {
    ECValue v;

    ECObjectsStatus status = getECValueFromInstance(v, instance, managedPropertyAccessor);
    if (status == ECObjectsStatus::Success)
        value = v.GetDateTimeTicks();

    return status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static ECObjectsStatus setECValueUsingFullAccessString(Utf8Char* asBuffer, Utf8Char* indexBuffer, ECValueCR v, IECInstanceR instance, Utf8CP managedPropertyAccessor)
    {
    // skip all the work if the instance is read only
    if (instance.IsReadOnly())
        if (!instance.ChangeValuesAllowed())
            return ECObjectsStatus::UnableToSetReadOnlyInstance;

    // see if access string specifies an array
    Utf8CP pos1 = strchr(managedPropertyAccessor, '[');

    // if not an array then
    if (NULL == pos1)
        return instance.SetValue(managedPropertyAccessor, v);

    size_t numChars = 0;
    numChars = pos1 - managedPropertyAccessor;
PUSH_DISABLE_DEPRECATION_WARNINGS
    strncpy(asBuffer, managedPropertyAccessor, numChars > NUM_ACCESSSTRING_BUFFER_CHARS ? NUM_ACCESSSTRING_BUFFER_CHARS : numChars);
    asBuffer[numChars] = 0;

    Utf8CP pos2 = strchr(pos1 + 1, ']');

    BeAssert(pos2 != NULL);

    numChars = pos2 - pos1 - 1;

    strncpy(indexBuffer, pos1 + 1, numChars > NUM_INDEX_BUFFER_CHARS ? NUM_INDEX_BUFFER_CHARS : numChars);
POP_DISABLE_DEPRECATION_WARNINGS
    indexBuffer[numChars] = 0;

    uint32_t indexValue = 0;
    if (1 != Utf8String::Sscanf_safe(indexBuffer, "%ud", &indexValue))
        return ECObjectsStatus::Error;

    ECValue         arrayVal;
    ECObjectsStatus status;

    if (ECObjectsStatus::Success != (status = instance.GetValue(arrayVal, asBuffer)))
        return status;

    ArrayInfo arrayInfo = arrayVal.GetArrayInfo();
    uint32_t  size = arrayInfo.GetCount();

    if (indexValue >= size)
        {
        if (arrayInfo.IsFixedCount())
            return ECObjectsStatus::Error;

        unsigned int numToInsert = (indexValue + 1) - size;
        status = instance.AddArrayElements(asBuffer, numToInsert);
        if (ECObjectsStatus::Success != status)
            return status;

        if (arrayInfo.IsStructArray())
            {
            ECClassCR    ecClass = instance.GetClass();

            Utf8Char buffer[NUM_INDEX_BUFFER_CHARS + 1];
PUSH_DISABLE_DEPRECATION_WARNINGS
            strncpy(buffer, asBuffer, NUM_INDEX_BUFFER_CHARS);
POP_DISABLE_DEPRECATION_WARNINGS
            ECPropertyP prop = getProperty(ecClass, asBuffer, buffer);

            if (!prop->GetIsArray())
                return ECObjectsStatus::Error;

            StructArrayECPropertyP arrayProp = dynamic_cast<StructArrayECPropertyP>(prop);
            if (!arrayProp)
                return ECObjectsStatus::Error;

            ECClassCR structClass = arrayProp->GetStructElementType();

            StandaloneECEnablerPtr standaloneEnabler = instance.GetEnablerR().GetEnablerForStructArrayMember(structClass.GetSchema().GetSchemaKey(), structClass.GetName().c_str());
            if (standaloneEnabler.IsNull())
                return ECObjectsStatus::Error;

            ECValue                     arrayEntryVal;

            for (uint32_t i = 0; i < numToInsert; i++)
                {
                // only set new struct value if AddArrayElements did not already set it
                if (ECObjectsStatus::Success != instance.GetValue(arrayEntryVal, asBuffer, size + i) || arrayEntryVal.IsNull())
                    {
                    arrayEntryVal.SetStruct(standaloneEnabler->CreateInstance().get());
                    if (ECObjectsStatus::Success != instance.SetValue(asBuffer, arrayEntryVal, size + i))
                        return ECObjectsStatus::Error;
                    }
                }
            }
        }

    if (arrayInfo.IsPrimitiveArray())
        return instance.SetValue(asBuffer, v, indexValue);

    // must be a struct array
    if (NULL == strchr(pos2, '.'))
        {
        //Caller is attempting to set the value of this struct array element directly.
        return instance.SetValue(asBuffer, v, indexValue);
        }
    instance.GetValue(arrayVal, asBuffer, indexValue);
    IECInstancePtr arrayEntryInstance = arrayVal.GetStruct();

    return setECValueUsingFullAccessString(asBuffer, indexBuffer, v, *arrayEntryInstance, pos2 + 2); // move to character after "]." in access string.
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static ECObjectsStatus setECValueInInstance(ECValueCR v, IECInstanceR instance, Utf8CP managedPropertyAccessor)
    {
    Utf8String asBufferStr;

    Utf8Char asBuffer[NUM_ACCESSSTRING_BUFFER_CHARS + 1];
    Utf8Char indexBuffer[NUM_INDEX_BUFFER_CHARS + 1];

    return setECValueUsingFullAccessString(asBuffer, indexBuffer, v, instance, managedPropertyAccessor);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus ECInstanceInteropHelper::SetValue(IECInstanceR instance, Utf8CP managedPropertyAccessor, ECValueCR value)
    {
    return setECValueInInstance(value, instance, managedPropertyAccessor);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus ECInstanceInteropHelper::SetLongValue(IECInstanceR instance, Utf8CP managedPropertyAccessor, int64_t value)
    {
    ECValue v(value);
    return setECValueInInstance(v, instance, managedPropertyAccessor);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus ECInstanceInteropHelper::SetIntegerValue(IECInstanceR instance, Utf8CP managedPropertyAccessor, int value)
    {
    ECValue v(value);
    return setECValueInInstance(v, instance, managedPropertyAccessor);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus ECInstanceInteropHelper::SetDoubleValue(IECInstanceR instance, Utf8CP managedPropertyAccessor, double value)
    {
    ECValue v(value);
    return setECValueInInstance(v, instance, managedPropertyAccessor);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus ECInstanceInteropHelper::SetStringValue(IECInstanceR instance, Utf8CP managedPropertyAccessor, Utf8CP value)
    {
    ECValue v(value, false);
    return setECValueInInstance(v, instance, managedPropertyAccessor);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus ECInstanceInteropHelper::SetBooleanValue(IECInstanceR instance, Utf8CP managedPropertyAccessor, bool value)
    {
    ECValue v(value);
    return setECValueInInstance(v, instance, managedPropertyAccessor);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus ECInstanceInteropHelper::SetPoint2dValue(IECInstanceR instance, Utf8CP managedPropertyAccessor, DPoint2dCR value)
    {
    ECValue v(value);
    return setECValueInInstance(v, instance, managedPropertyAccessor);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus ECInstanceInteropHelper::SetPoint3dValue(IECInstanceR instance, Utf8CP managedPropertyAccessor, DPoint3dCR value)
    {
    ECValue v(value);
    return setECValueInInstance(v, instance, managedPropertyAccessor);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus ECInstanceInteropHelper::SetDateTimeValue(IECInstanceR instance, Utf8CP managedPropertyAccessor, DateTimeCR value)
    {
    ECValue v(value);
    return setECValueInInstance(v, instance, managedPropertyAccessor);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus ECInstanceInteropHelper::SetDateTimeTicks(IECInstanceR instance, Utf8CP managedPropertyAccessor, int64_t value)
    {
    ECValue v;
    v.SetDateTimeTicks(value);
    return setECValueInInstance(v, instance, managedPropertyAccessor);
    }

///////////////////////////////////////////////////////////////////////////////
// Get Using ECValueAccessor
///////////////////////////////////////////////////////////////////////////////
/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus ECInstanceInteropHelper::GetLong(IECInstanceCR instance, int64_t & value, ECValueAccessorCR accessor)
    {
    ECValue v;

    ECObjectsStatus status = instance.GetValueUsingAccessor(v, accessor);
    if (status == ECObjectsStatus::Success)
        value = v.GetLong();

    return status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus ECInstanceInteropHelper::GetInteger(IECInstanceCR instance, int & value, ECValueAccessorCR accessor)
    {
    ECValue v;

    ECObjectsStatus status = instance.GetValueUsingAccessor(v, accessor);
    if (status == ECObjectsStatus::Success)
        value = v.GetInteger();

    return status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus ECInstanceInteropHelper::GetDouble(IECInstanceCR instance, double& value, ECValueAccessorCR accessor)
    {
    ECValue v;

    ECObjectsStatus status = instance.GetValueUsingAccessor(v, accessor);
    if (status == ECObjectsStatus::Success)
        value = v.GetDouble();

    return status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus ECInstanceInteropHelper::GetString(IECInstanceCR instance, Utf8StringR value, ECValueAccessorCR accessor)
    {
    ECValue v;

    ECObjectsStatus status = instance.GetValueUsingAccessor(v, accessor);
    if (status == ECObjectsStatus::Success)
        value = v.GetUtf8CP();

    return status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus ECInstanceInteropHelper::GetBoolean(IECInstanceCR instance, bool & value, ECValueAccessorCR accessor)
    {
    ECValue v;

    ECObjectsStatus status = instance.GetValueUsingAccessor(v, accessor);
    if (status == ECObjectsStatus::Success)
        value = v.GetBoolean();

    return status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus ECInstanceInteropHelper::GetPoint2d(IECInstanceCR instance, DPoint2d & value, ECValueAccessorCR accessor)
    {
    ECValue v;

    ECObjectsStatus status = instance.GetValueUsingAccessor(v, accessor);
    if (status == ECObjectsStatus::Success)
        value = v.GetPoint2d();

    return status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus ECInstanceInteropHelper::GetPoint3d(IECInstanceCR instance, DPoint3d & value, ECValueAccessorCR accessor)
    {
    ECValue v;

    ECObjectsStatus status = instance.GetValueUsingAccessor(v, accessor);
    if (status == ECObjectsStatus::Success)
        value = v.GetPoint3d();

    return status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus ECInstanceInteropHelper::GetDateTime(IECInstanceCR instance, DateTimeR value, ECValueAccessorCR accessor)
    {
    ECValue v;

    ECObjectsStatus status = instance.GetValueUsingAccessor(v, accessor);
    if (status == ECObjectsStatus::Success)
        value = v.GetDateTime();

    return status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus ECInstanceInteropHelper::GetDateTimeTicks(IECInstanceCR instance, int64_t & value, ECValueAccessorCR accessor)
    {
    ECValue v;

    ECObjectsStatus status = instance.GetValueUsingAccessor(v, accessor);
    if (status == ECObjectsStatus::Success)
        value = v.GetDateTimeTicks();

    return status;
    }

///////////////////////////////////////////////////////////////////////////////
// Set Using ECValueAccessor
///////////////////////////////////////////////////////////////////////////////
/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus ECInstanceInteropHelper::SetValue(IECInstanceR instance, ECValueAccessorCR accessor, ECValueCR value)
    {
    return  instance.SetValueUsingAccessor(accessor, value);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus ECInstanceInteropHelper::SetLongValue(IECInstanceR instance, ECValueAccessorCR accessor, int64_t value)
    {
    ECValue v(value);
    return  instance.SetValueUsingAccessor(accessor, v);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus ECInstanceInteropHelper::SetIntegerValue(IECInstanceR instance, ECValueAccessorCR accessor, int value)
    {
    ECValue v(value);
    return  instance.SetValueUsingAccessor(accessor, v);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus ECInstanceInteropHelper::SetDoubleValue(IECInstanceR instance, ECValueAccessorCR accessor, double value)
    {
    ECValue v(value);
    return  instance.SetValueUsingAccessor(accessor, v);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus ECInstanceInteropHelper::SetStringValue(IECInstanceR instance, ECValueAccessorCR accessor, Utf8CP value)
    {
    ECValue v(value, false);
    return  instance.SetValueUsingAccessor(accessor, v);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus ECInstanceInteropHelper::SetBooleanValue(IECInstanceR instance, ECValueAccessorCR accessor, bool value)
    {
    ECValue v(value);
    return  instance.SetValueUsingAccessor(accessor, v);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus ECInstanceInteropHelper::SetPoint2dValue(IECInstanceR instance, ECValueAccessorCR accessor, DPoint2dCR value)
    {
    ECValue v(value);
    return  instance.SetValueUsingAccessor(accessor, v);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus ECInstanceInteropHelper::SetPoint3dValue(IECInstanceR instance, ECValueAccessorCR accessor, DPoint3dCR value)
    {
    ECValue v(value);
    return  instance.SetValueUsingAccessor(accessor, v);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus ECInstanceInteropHelper::SetDateTimeValue(IECInstanceR instance, ECValueAccessorCR accessor, DateTimeCR value)
    {
    ECValue v(value);
    return  instance.SetValueUsingAccessor(accessor, v);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus ECInstanceInteropHelper::SetDateTimeTicks(IECInstanceR instance, ECValueAccessorCR accessor, int64_t value)
    {
    ECValue v;
    v.SetDateTimeTicks(value);
    return  instance.SetValueUsingAccessor(accessor, v);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool            ECInstanceInteropHelper::IsNull(IECInstanceR instance, ECValueAccessorCR accessor)
    {
    ECValue v;

    ECObjectsStatus status = instance.GetValueUsingAccessor(v, accessor);
    if (status != ECObjectsStatus::Success)
        return true;

    return v.IsNull();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void            ECInstanceInteropHelper::SetToNull(IECInstanceR instance, ECValueAccessorCR accessor)
    {
    ECValue v;
    v.SetToNull();

    instance.SetValueUsingAccessor(accessor, v);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool            ECInstanceInteropHelper::IsPropertyReadOnly(IECInstanceCR instance, ECValueAccessorR accessor)
    {
    ECObjectsStatus status;
    uint32_t propertyIndex = accessor.DeepestLocation().GetPropertyIndex();
    if (1 < accessor.GetDepth())
        {
        ECValue v;
        ECValueAccessor newAccessor(accessor);
        newAccessor.PopLocation();
        status = instance.GetValueUsingAccessor(v, newAccessor);
        if (ECObjectsStatus::Success != status)
            return false;

        IECInstancePtr structInstance = v.GetStruct();
        if (structInstance.IsNull())
            {
            // note: null structs were throwing exceptions in element info dlg
            // I assume that if the struct is null, it is considered read-only
            return true;
            }

        return structInstance->IsPropertyReadOnly(propertyIndex);
        }
    return instance.IsPropertyReadOnly(propertyIndex);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ECN::ECEnablerP                  ECInstanceInteropHelper::GetEnablerForStructArrayEntry(IECInstanceR instance, ECValueAccessorR arrayMemberAccessor, SchemaKeyCR schemaKey, Utf8CP className)
    {
    ECN::ECValue v;
    instance.GetValueUsingAccessor(v, arrayMemberAccessor);

    if (!v.IsStruct())
        return NULL;

    if (!v.IsNull())
        {
        ECN::IECInstancePtr structInstance = v.GetStruct();
        return &structInstance->GetEnablerR();
        }

    // if we get here we probably have a fixed size array with NULL entries
    ECN::ECEnablerP structArrayEnabler = const_cast<ECN::ECEnablerP>(arrayMemberAccessor.DeepestLocation().GetEnabler());

    ECN::StandaloneECEnablerPtr standaloneEnabler = structArrayEnabler->GetEnablerForStructArrayMember(schemaKey, className);
    if (standaloneEnabler.IsNull())
        {
        LOG.errorv("Unable to locate a standalone enabler for class %s", className);
        return NULL;
        }

    return standaloneEnabler.get();
    }

/*---------------------------------------------------------------------------------**//**
* This method is called from derived class so the rootInstance was pinned if necessary
* before calling this method.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
/* static */ ECN::IECInstancePtr getParentNativeInstance(ECN::IECInstanceCP rootInstance, ECN::ECValueAccessorCR structValueAccessor)
    {
    // if not a top level property, get the native instance that will contain this struct array
    if (structValueAccessor.GetDepth() > 1)
        {
        ECN::ECValue parentStructValue;

        ECN::ECValueAccessor parentInstanceAccessor(structValueAccessor);
        parentInstanceAccessor.PopLocation();   // remove one level to get to the parent instance

        rootInstance->GetValueUsingAccessor(parentStructValue, parentInstanceAccessor);
        if (!parentStructValue.IsStruct())
            return NULL;

        ECN::IECInstancePtr structInstance = parentStructValue.GetStruct();
        if (structInstance.IsValid())
            return structInstance;

        // we may be processing a member of an embedded struct so we need to check the next level up
        return getParentNativeInstance(rootInstance, parentInstanceAccessor);
        }

    return const_cast<ECN::IECInstanceP>(rootInstance);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus  ECInstanceInteropHelper::GetStructArrayEntry(ECN::ECValueAccessorR structArrayEntryValueAccessor, IECInstanceR instance, uint32_t index, ECN::ECValueAccessorCR structArrayValueAccessor,
                                                              bool createPropertyIfNotFound, Utf8CP wcharAccessString,
                                                              SchemaKeyCR schemaKey, Utf8CP className)
    {
    ECN::ECEnablerR structArrayEnabler = *(const_cast<ECN::ECEnablerP>(structArrayValueAccessor.DeepestLocationCR().GetEnabler()));
    ECN::StandaloneECEnablerPtr standaloneEnabler = structArrayEnabler.GetEnablerForStructArrayMember(schemaKey, className);
    if (standaloneEnabler.IsNull())
        {
        LOG.errorv("Unable to locate a standalone enabler for class \" %s \"", className);
        return ECObjectsStatus::EnablerNotFound;
        }

    ECN::ECValue  arrayVal;
    instance.GetValueUsingAccessor(arrayVal, structArrayValueAccessor);

    ArrayInfo   arrayInfo = arrayVal.GetArrayInfo();
    uint32_t    arrayCount = arrayInfo.GetCount();

    // adjust the ECVAlueAccessor to include the array index
    ECN::ECValueAccessor arrayEntryValueAccessor(structArrayValueAccessor);
    arrayEntryValueAccessor.DeepestLocation().SetArrayIndex(index);

    if (arrayCount <= index)
        {
        // see if we are allowed to add a new struct array instance
        if (!createPropertyIfNotFound)
            return ECObjectsStatus::Error;

        // only proceed if not read only instance
        if (!instance.ChangeValuesAllowed())
            return ECObjectsStatus::UnableToSetReadOnlyInstance;

        ECN::IECInstancePtr parentNativeInstance = getParentNativeInstance(&instance, structArrayValueAccessor);
        if (parentNativeInstance.IsNull())
            {
            LOG.error("Unable to get native instance when processing ECInstanceInteropHelper::GetStructArrayEntry");
            return ECObjectsStatus::Error;
            }

        ::uint32_t numToInsert = (index + 1) - arrayCount;
        if (ECN::ECObjectsStatus::Success != parentNativeInstance->AddArrayElements(wcharAccessString, numToInsert))
            {
            LOG.errorv("Unable to add array element(s) to native instance - access string \"%s\"", structArrayValueAccessor.GetManagedAccessString().c_str());
            return ECObjectsStatus::UnableToAddStructArrayMember;
            }

        ECN::ECValue  arrayEntryVal;
        for (::uint32_t i = 0; i < numToInsert; i++)
            {
            arrayEntryVal.SetStruct(standaloneEnabler->CreateInstance().get());
            if (ECObjectsStatus::Success != parentNativeInstance->SetValue(wcharAccessString, arrayEntryVal, arrayCount + i))
                return ECObjectsStatus::UnableToSetStructArrayMemberInstance;
            }
        }
    else
        {
        // make sure the struct instance is not null
        ECN::ECValue arrayEntryVal;

        instance.GetValueUsingAccessor(arrayEntryVal, arrayEntryValueAccessor);
        if (arrayEntryVal.IsNull())
            {
            // see if we are allowed to add a new strct array instance
            if (!createPropertyIfNotFound)
                return ECObjectsStatus::Error;

            // only proceed if not read only instance
            if (instance.ChangeValuesAllowed())
                return ECObjectsStatus::UnableToSetReadOnlyInstance;

            arrayEntryVal.SetStruct(standaloneEnabler->CreateInstance().get());

            ECN::IECInstancePtr parentNativeInstance = getParentNativeInstance(&instance, structArrayValueAccessor);
            if (parentNativeInstance.IsNull())
                {
                LOG.error("Unable to get native instance when processing ECInstanceInteropHelper::GetStructArrayEntry");
                return ECObjectsStatus::Error;
                }

            if (ECObjectsStatus::Success != parentNativeInstance->SetValue(wcharAccessString, arrayEntryVal, index))
                return ECObjectsStatus::UnableToSetStructArrayMemberInstance;
            }
        }

    structArrayEntryValueAccessor.Clone(arrayEntryValueAccessor);
    return ECObjectsStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool  ECInstanceInteropHelper::IsCalculatedECProperty(IECInstanceCR instance, int propertyIndex)
    {
    Utf8CP accessor;
    if (ECObjectsStatus::Success != instance.GetEnabler().GetAccessString(accessor, (uint32_t) propertyIndex))
        return false;

    ECClassCR ecClass = instance.GetClass();

    Utf8Char buffer[NUM_INDEX_BUFFER_CHARS + 1];

PUSH_DISABLE_DEPRECATION_WARNINGS
    strncpy(buffer, accessor, NUM_INDEX_BUFFER_CHARS);
POP_DISABLE_DEPRECATION_WARNINGS

    ECPropertyP ecProperty = getProperty(ecClass, accessor, buffer);

    if (NULL == ecProperty)
        return false;

    return ecProperty->IsDefined("Bentley_Standard_CustomAttributes", "CalculatedECPropertySpecification");
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus ECInstanceInteropHelper::SetValueByIndex(IECInstanceR instance, int propertyIndex, int arrayIndex, ECValueCR value)
    {
    if (-1 == arrayIndex)
        return instance.SetValue((uint32_t) propertyIndex, value);

    ECN::ECValue v;
    instance.GetValue(v, propertyIndex);
    uint32_t count = v.GetArrayInfo().GetCount();
    if ((uint32_t) arrayIndex >= count)
        {
        ECObjectsStatus status;
        uint32_t size = 1 + ((uint32_t) arrayIndex - count);
        Utf8CP accessString;
        status = instance.GetEnabler().GetAccessString(accessString, propertyIndex);
        if (ECObjectsStatus::Success != status)
            return status;
        status = instance.AddArrayElements(accessString, size);
        if (ECN::ECObjectsStatus::Success != status)
            return status;
        }
    return instance.SetValue((uint32_t) propertyIndex, value, (uint32_t) arrayIndex);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus ECInstanceInteropHelper::GetValueByIndex(ECValueR value, IECInstanceCR instance, int propertyIndex, int arrayIndex)
    {
    if (-1 == arrayIndex)
        return instance.GetValue(value, (uint32_t) propertyIndex);

    ECN::ECValue v;
    instance.GetValue(v, propertyIndex);
    uint32_t count = v.GetArrayInfo().GetCount();
    if ((uint32_t) propertyIndex >= count)
        {
        value.SetToNull();
        }
    return instance.GetValue(value, (uint32_t) propertyIndex, (uint32_t) arrayIndex);
    }

/*---------------------------------------------------------------------------------**//**
* Expect some accessor ending in an array property. Want the last IECInstance in the
* chain and the property index of the array property.
* ex: "SomeStruct.SomeStructArray[0].SomeArray"
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static ECObjectsStatus  resolveArrayAccessString(IECInstancePtr& resolvedInstance, uint32_t& resolvedPropertyIndex, IECInstanceR rootInstance, Utf8CP fullAccessString)
    {
    ECValueAccessor accessor;
    ECObjectsStatus status = ECValueAccessor::PopulateValueAccessor(accessor, rootInstance, fullAccessString);
    if (ECObjectsStatus::Success != status)
        return status;
    else if (0 == accessor.GetDepth())
        return ECObjectsStatus::PropertyNotFound;

    resolvedInstance = const_cast<IECInstanceP> (&rootInstance);
    resolvedPropertyIndex = 0;

    uint32_t depth = 0;
    for (; depth < accessor.GetDepth() - 1; depth++)
        {
        ECValue v;
        bool compatible = (accessor[depth].GetEnabler() == &resolvedInstance->GetEnabler());
        status = getValueHelper(v, *resolvedInstance, accessor, depth, compatible);
        if (ECObjectsStatus::Success != status)
            {
            ECPropertyCP ecprop = accessor[depth].GetECProperty();
            if (NULL != ecprop && ecprop->GetIsStruct())
                continue;
            else
                return status;
            }

        if (v.IsStruct() && accessor[depth].GetArrayIndex() >= 0)
            {
            resolvedInstance = v.GetStruct();
            if (resolvedInstance.IsNull())
                return ECObjectsStatus::Error;
            }
        }

    if (accessor[depth].GetArrayIndex() >= 0)
        return ECObjectsStatus::Error;

    if (accessor[depth].GetEnabler() == &resolvedInstance->GetEnabler())
        resolvedPropertyIndex = accessor[depth].GetPropertyIndex();
    else
        {
        Utf8CP accessString = accessor.GetAccessString(depth);
        if (NULL == accessString)
            return ECObjectsStatus::Error;

        status = resolvedInstance->GetEnabler().GetPropertyIndex(resolvedPropertyIndex, accessString);
        if (ECObjectsStatus::Success != status)
            return status;
        }

    ECPropertyCP ecprop = accessor[depth].GetECProperty();
    return NULL != ecprop && ecprop->GetIsArray() ? ECObjectsStatus::Success : ECObjectsStatus::Error;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus ECInstanceInteropHelper::ClearArray(IECInstanceR rootInstance, Utf8CP accessString)
    {
    IECInstancePtr resolvedInstance;
    uint32_t propertyIndex;
    ECObjectsStatus status = resolveArrayAccessString(resolvedInstance, propertyIndex, rootInstance, accessString);
    if (ECObjectsStatus::Success == status)
        status = resolvedInstance->ClearArray(propertyIndex);

    return status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus ECInstanceInteropHelper::RemoveArrayElement(IECInstanceR rootInstance, Utf8CP accessString, uint32_t arrayIndex)
    {
    IECInstancePtr resolvedInstance;
    uint32_t propertyIndex;
    ECObjectsStatus status = resolveArrayAccessString(resolvedInstance, propertyIndex, rootInstance, accessString);
    if (ECObjectsStatus::Success == status)
        status = resolvedInstance->RemoveArrayElement(propertyIndex, arrayIndex);

    return status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus ECInstanceInteropHelper::AddArrayElements(IECInstanceR rootInstance, Utf8CP accessString, uint32_t count, uint32_t atIndex)
    {
    IECInstancePtr resolvedInstance;
    uint32_t propertyIndex;
    ECObjectsStatus status = resolveArrayAccessString(resolvedInstance, propertyIndex, rootInstance, accessString);
    if (ECObjectsStatus::Success == status)
        status = -1 == atIndex ? resolvedInstance->AddArrayElements(propertyIndex, count) : resolvedInstance->InsertArrayElements(propertyIndex, atIndex, count);

    return status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus                 IECInstance::InsertArrayElements(Utf8CP propertyAccessString, uint32_t index, uint32_t size)
    {
    uint32_t propIdx;
    ECObjectsStatus status = GetEnabler().GetPropertyIndex(propIdx, propertyAccessString);
    return ECObjectsStatus::Success == status ? InsertArrayElements(propIdx, index, size) : status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus                 IECInstance::AddArrayElements(Utf8CP propertyAccessString, uint32_t size)
    {
    uint32_t propIdx;
    ECObjectsStatus status = GetEnabler().GetPropertyIndex(propIdx, propertyAccessString);
    return ECObjectsStatus::Success == status ? AddArrayElements(propIdx, size) : status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus                 IECInstance::RemoveArrayElement(Utf8CP propertyAccessString, uint32_t index)
    {
    uint32_t propIdx;
    ECObjectsStatus status = GetEnabler().GetPropertyIndex(propIdx, propertyAccessString);
    return ECObjectsStatus::Success == status ? RemoveArrayElement(propIdx, index) : status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus                 IECInstance::ClearArray(Utf8CP propertyAccessString)
    {
    uint32_t propertyIndex;
    ECObjectsStatus status = GetEnabler().GetPropertyIndex(propertyIndex, propertyAccessString);
    return ECObjectsStatus::Success == status ? ClearArray(propertyIndex) : status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus IECInstance::ClearArray(uint32_t propIdx) { return _ClearArray(propIdx); }
ECObjectsStatus IECInstance::InsertArrayElements(uint32_t propIdx, uint32_t idx, uint32_t size) { return _InsertArrayElements(propIdx, idx, size); }
ECObjectsStatus IECInstance::AddArrayElements(uint32_t propIdx, uint32_t size) { return _AddArrayElements(propIdx, size); }
ECObjectsStatus IECInstance::RemoveArrayElement(uint32_t propIdx, uint32_t idx) { return _RemoveArrayElement(propIdx, idx); }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String                         IECInstance::ToString(Utf8CP indent) const
    {
    return _ToString(indent);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static bool getInstanceLabelPropertyNameFromClass(Utf8StringR propertyName, ECClassCR ecClass, bool& alwaysUseClassLabel)
    {
    IECInstancePtr caInstance = ecClass.GetCustomAttribute("Bentley_Standard_CustomAttributes", "InstanceLabelSpecification");
    if (caInstance.IsValid())
        {
        ECValue value;
        if (ECObjectsStatus::Success == caInstance->GetValue(value, "PropertyName") && !value.IsNull())
            {
            propertyName = value.GetUtf8CP();
            return true;
            }
        }

    if (ecClass.GetCustomAttribute("Bentley_Standard_CustomAttributes", "ClassLabelIsInstanceLabel").IsValid())
        {
        alwaysUseClassLabel = true;
        return false;
        }

    for (ECClassCP baseClass : ecClass.GetBaseClasses())
        {
        if (getInstanceLabelPropertyNameFromClass(propertyName, *baseClass, alwaysUseClassLabel))
            return true;
        else if (alwaysUseClassLabel)
            return false;
        }

    return false;
    }

// ordered by precedence
static const Utf8CP s_standardInstanceLabelPropertyNames[] =
    {
     "Name", "NAME", NULL
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool                         IECInstance::GetInstanceLabelPropertyName(Utf8StringR propName) const
    {
    ECClassCR ecClass = GetClass();
    bool alwaysUseClassLabel = false;
    if (getInstanceLabelPropertyNameFromClass(propName, ecClass, alwaysUseClassLabel))
        return true;
    else if (alwaysUseClassLabel)
        return false;

    const Utf8CP* standardName = s_standardInstanceLabelPropertyNames;
    while (*standardName)
        {
        if (NULL != ecClass.GetPropertyP(*standardName))
            {
            propName = *standardName;
            return true;
            }

        ++standardName;
        }

    return false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus                 IECInstance::_GetDisplayLabel(Utf8String& displayLabel) const
    {
    Utf8String propertyName;
    if (GetInstanceLabelPropertyName(propertyName) && 0 < propertyName.length())   // empty property name => always use class label, don't look for "NAME" or "Name"
        {
        ECN::ECValue ecValue;
        if (ECObjectsStatus::Success == GetValue(ecValue, propertyName.c_str()) && !ecValue.IsNull())
            {
            if (ecValue.ConvertToPrimitiveType(PRIMITIVETYPE_String) && !Utf8String::IsNullOrEmpty(ecValue.GetUtf8CP()))
                {
                displayLabel = ecValue.GetUtf8CP();
                return ECObjectsStatus::Success;
                }
            }
        }

    // According to documentation in managed ECF, we are supposed to fallback to the class's display label
    displayLabel = GetClass().GetDisplayLabel();
    return ECObjectsStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus                 IECInstance::GetDisplayLabel(Utf8String& displayLabel) const
    {
    return  _GetDisplayLabel(displayLabel);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus                 IECInstance::_SetDisplayLabel(Utf8CP displayLabel)
    {
    Utf8String propertyName;
    if (!GetInstanceLabelPropertyName(propertyName))
        return ECObjectsStatus::Error;

    ECN::ECValue ecValue;
    ecValue.SetUtf8CP(displayLabel, false);

    return SetValue(propertyName.c_str(), ecValue);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus                 IECInstance::SetDisplayLabel(Utf8CP displayLabel)
    {
    return  _SetDisplayLabel(displayLabel);
    }

///////////////////////////////////////////////////////////////////////////////////////////////
//   IECRelationshipInstance
///////////////////////////////////////////////////////////////////////////////////////////////
/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void                            IECRelationshipInstance::SetSource(IECInstanceP instance)
    {
    _SetSource(instance);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
IECInstancePtr                  IECRelationshipInstance::GetSource() const
    {
    return _GetSource();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void                            IECRelationshipInstance::SetTarget(IECInstanceP instance)
    {
    _SetTarget(instance);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
IECInstancePtr                  IECRelationshipInstance::GetTarget() const
    {
    return _GetTarget();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus                 IECRelationshipInstance::GetSourceOrderId(int64_t& sourceOrderId) const
    {
    return _GetSourceOrderId(sourceOrderId);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus                IECRelationshipInstance::GetTargetOrderId(int64_t& targetOrderId) const
    {
    return _GetTargetOrderId(targetOrderId);
    }

///////////////////////////////////////////////////////////////////////////////////////////////
//   IECWipRelationshipInstance
///////////////////////////////////////////////////////////////////////////////////////////////
/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus                   IECWipRelationshipInstance::SetName(Utf8CP name)
    {
    return _SetName(name);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus                   IECWipRelationshipInstance::SetSourceOrderId(int64_t sourceOrderId)
    {
    return _SetSourceOrderId(sourceOrderId);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus                   IECWipRelationshipInstance::SetTargetOrderId(int64_t targetOrderId)
    {
    return _SetTargetOrderId(targetOrderId);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void              convertByteArrayToString(Utf8StringR outString, const Byte *byteData, size_t numBytes)
    {
    Base64Utilities::Encode(outString, byteData, numBytes);
    }

typedef bvector<Byte>   T_ByteArray;
/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool   convertStringToByteArray(T_ByteArray& byteData, Utf8CP stringData)
    {
    if (!Base64Utilities::MatchesAlphabet(stringData))
        return false;

    Base64Utilities::Decode(byteData, stringData, strlen(stringData));
    return true;
    }

//--------------------------------------------------------------------------------------
// @bsimethod
//---------------+---------------+---------------+---------------+---------------+------
static void CreateAccessString(Utf8StringR accessString, Utf8StringP baseAccessString, Utf8StringCR propertyName)
    {
    if (nullptr == baseAccessString)
        accessString = propertyName;
    else
        {
        accessString = *baseAccessString;
        accessString.append(propertyName);
        }
    }

// =====================================================================================
// InstanceXMLReader class
// =====================================================================================
struct  InstanceXmlReader
    {
    private:
        Utf8String              m_fullSchemaName;
        pugi::xml_node          m_xmlNode;
        ECSchemaCP              m_schema;
        ECInstanceReadContextR  m_context;
        Utf8String              m_className;

    public:
        /*---------------------------------------------------------------------------------**//**
        * @bsimethod
        +---------------+---------------+---------------+---------------+---------------+------*/
        InstanceXmlReader(ECInstanceReadContextR context, pugi::xml_node xmlNode)
            : m_context(context), m_schema(NULL), m_xmlNode(xmlNode)
            {
            m_className = m_xmlNode.name();
            }

        /*---------------------------------------------------------------------------------**//**
        * @bsimethod
        +---------------+---------------+---------------+---------------+---------------+------*/
        InstanceXmlReader(ECInstanceReadContextR context, pugi::xml_node xmlNode, Utf8String className)
            : m_context(context), m_schema(NULL), m_xmlNode(xmlNode)
            {
            m_className = className;
            }

        //-------------------------------------------------------------------------------------
        // @bsimethod
        //+---------------+---------------+---------------+---------------+---------------+----
        ECSchemaCP GetSchema(Utf8String schemaName)
            {
            SchemaKey key;
            if (ECObjectsStatus::Success != SchemaKey::ParseSchemaFullName(key, schemaName.c_str()))
                return NULL;

            return m_context.FindSchemaCP(key, SchemaMatchType::LatestReadCompatible);//Abeesh: Preserving old behavior. Ideally it should be exact
            }
        /*---------------------------------------------------------------------------------**//**
        * @bsimethod
        +---------------+---------------+---------------+---------------+---------------+------*/
        ECSchemaCP       GetSchema()
            {
            if (NULL != m_schema)
                return m_schema;

            SchemaKey key;
            if (ECObjectsStatus::Success != SchemaKey::ParseSchemaFullName(key, m_fullSchemaName.c_str()))
                return NULL;

            m_schema = m_context.FindSchemaCP(key, SchemaMatchType::LatestReadCompatible);//Abeesh: Preserving old behavior. Ideally it should be exact
            return m_schema;
            }

        //-------------------------------------------------------------------------------------
        // @bsimethod
        //+---------------+---------------+---------------+---------------+---------------+----
        StandaloneECInstancePtr CreateConstraintInstance(Utf8String className, Utf8String instanceId, ECSchemaCP defaultSchema)
            {
            // Classnames might be qualified by a schema name.
            Utf8String constraintSchemaName;
            Utf8String constraintClassName;
            if (ECObjectsStatus::Success != ECClass::ParseClassName(constraintSchemaName, constraintClassName, className))
                {
                LOG.errorv("Invalid ECSchemaXML: The ECRelationshipConstraint contains a classname attribute with the value '%s' that can not be parsed.",
                           className.c_str());
                return nullptr;
                }

            ECSchemaCP constraintSchema = Utf8String::IsNullOrEmpty(constraintSchemaName.c_str()) ? defaultSchema : GetSchema(constraintSchemaName);
            if (nullptr == constraintSchema)
                {
                LOG.errorv("Invalid ECSchemaXML: ECRelationshipConstraint contains a classname attribute with the alias '%s' that can not be resolved to a referenced schema.",
                           constraintSchemaName.c_str());
                return nullptr;
                }

            ECClassCP constraintClass = constraintSchema->GetClassCP(constraintClassName.c_str());
            if (nullptr == constraintClass)
                {
                LOG.errorv("Invalid ECSchemaXML: The ECRelationshipConstraint contains a classname attribute with the value '%s' that can not be resolved to an ECClass named '%s' in the ECSchema '%s'",
                           className.c_str(), constraintClassName.c_str(), constraintSchema->GetName().c_str());
                return nullptr;
                }

            auto constraintInstance = constraintClass->GetDefaultStandaloneEnabler()->CreateInstance();
            constraintInstance->SetInstanceId(instanceId.c_str());

            return constraintInstance;
            }

        /*---------------------------------------------------------------------------------**//**
        * @bsimethod
        +---------------+---------------+---------------+---------------+---------------+------*/
        InstanceReadStatus      ReadInstance(IECInstancePtr& ecInstance)
            {
            // When this is called, m_xmlNode should be a node that is the class name, with a name space corresponding to the schema name.
            InstanceReadStatus      ixrStatus;
            ECClassCP               ecClass;
            if (InstanceReadStatus::Success != (ixrStatus = GetInstance(ecClass, ecInstance)))
                return ixrStatus;

            // this reads the property members and consumes the XmlNodeType_EndElement corresponding to this XmlNodeType_Element.
            return ReadInstanceOrStructMembers(*ecClass, ecInstance.get(), NULL, m_xmlNode);
            }

        /*---------------------------------------------------------------------------------**//**
        * @bsimethod
        +---------------+---------------+---------------+---------------+---------------+------*/
        InstanceReadStatus      GetInstance(ECClassCP& ecClass, IECInstancePtr& ecInstance)
            {
            ECSchemaCP schema = NULL;

            // get the xmlns name, if there is one.
            {
            auto xmlnsAttr = m_xmlNode.attribute("xmlns");
            if (xmlnsAttr && 0 != BeStringUtilities::Strnicmp(xmlnsAttr.as_string(), ECXML_URI, strlen(ECXML_URI)))
                {
                m_fullSchemaName = xmlnsAttr.as_string();
                schema = GetSchema();
                }
            else
                schema = &(m_context.GetFallBackSchema());
            }

            if (NULL == schema)
                {
                LOG.errorv("Failed to locate ECSchema %s", m_fullSchemaName.c_str());
                return InstanceReadStatus::ECSchemaNotFound;
                }

            // see if we can find the class from the schema.
            m_context.ResolveSerializedClassName(m_className, *schema);
            ECClassCP    foundClass;
            if (NULL == (foundClass = schema->GetClassCP(m_className.c_str())))
                {
                ECSchemaReferenceListCR refList = schema->GetReferencedSchemas();
                SchemaKey key;
                if (ECObjectsStatus::Success == SchemaKey::ParseSchemaFullName(key, m_fullSchemaName.c_str()))
                    {
                    ECSchemaReferenceList::const_iterator schemaIterator = refList.find(key);
                    if (schemaIterator != refList.end())
                        foundClass = schemaIterator->second->GetClassCP(m_className.c_str());
                    }
                else
                    {
                    for (ECSchemaReferenceList::const_iterator schemaIterator = refList.begin(); schemaIterator != refList.end(); schemaIterator++)
                        {
                        if (NULL != (foundClass = schemaIterator->second->GetClassCP(m_className.c_str())))
                            break;
                        }
                    }
                }
            if (NULL == foundClass)
                {
                LOG.errorv("Failed to find ECClass '%s' in '%s'", m_className.c_str(), schema->GetFullSchemaName().c_str());
                return InstanceReadStatus::ECClassNotFound;
                }

            ecClass = foundClass;

            ecInstance = m_context.CreateStandaloneInstance(*foundClass).get();

            auto idAttr = m_xmlNode.attribute(ECXML_ECINSTANCE_INSTANCEID_ATTRIBUTE);
            if (idAttr)
                {
                ecInstance->SetInstanceId(idAttr.as_string());
                }

            IECRelationshipInstance*    relationshipInstance = dynamic_cast <IECRelationshipInstance*> (ecInstance.get());

            // if relationship, need the attributes used in relationships.
            if (NULL != relationshipInstance)
                {
                // see if we can find the attributes corresponding to the relationship instance ids.
                auto sourceIdAttr = m_xmlNode.attribute(ECINSTANCE_SOURCEINSTANCEID_ATTRIBUTE);
                Utf8String sourceInstanceId;
                if (sourceIdAttr)
                    sourceInstanceId = sourceIdAttr.as_string();
                else   
                    LOG.warning("Source InstanceId not set on serialized relationship instance");

                auto sourceClassNameAttr = m_xmlNode.attribute(ECINSTANCE_SOURCECLASS_ATTRIBUTE);
                Utf8String sourceClassName;
                if (sourceIdAttr)
                    sourceClassName = sourceClassNameAttr.as_string();
                else   
                    LOG.warning("Source className not set on serialized relationship instance");

                auto targetInstanceIdAttr = m_xmlNode.attribute(ECINSTANCE_TARGETINSTANCEID_ATTRIBUTE);
                Utf8String targetInstanceId;
                if (targetInstanceIdAttr)
                    targetInstanceId = targetInstanceIdAttr.as_string();
                else   
                    LOG.warning("Target InstanceId not set on serialized relationship instance");

                auto targetClassNameAttr = m_xmlNode.attribute(ECINSTANCE_TARGETCLASS_ATTRIBUTE);
                Utf8String targetClassName;
                if (targetClassNameAttr)
                    targetClassName = targetClassNameAttr.as_string();
                else   
                    LOG.warning("Target className not set on serialized relationship instance");

                if (!Utf8String::IsNullOrEmpty(sourceInstanceId.c_str()) && !Utf8String::IsNullOrEmpty(sourceClassName.c_str()))
                    {
                    IECInstancePtr source = CreateConstraintInstance(sourceClassName, sourceInstanceId, schema);
                    if (source.IsValid())
                        relationshipInstance->SetSource(source.get());
                    }
                if (!Utf8String::IsNullOrEmpty(targetInstanceId.c_str()) && !Utf8String::IsNullOrEmpty(targetClassName.c_str()))
                    {
                    IECInstancePtr target = CreateConstraintInstance(targetClassName, targetInstanceId, schema);
                    if (target.IsValid())
                        relationshipInstance->SetTarget(target.get());
                    }
                }

            return InstanceReadStatus::Success;
            }

        /*---------------------------------------------------------------------------------**//**
        * @bsimethod
        +---------------+---------------+---------------+---------------+---------------+------*/
        InstanceReadStatus   ReadInstanceOrStructMembers(ECClassCR ecClass, IECInstanceP ecInstance, Utf8String* baseAccessString, pugi::xml_node instanceNode)
            {
            // On entry, the instanceNode is the XML node that contains the children that have propertyValues.
            for (pugi::xml_node propertyValueNode : instanceNode.children())
                {
                if(propertyValueNode.type() != pugi::xml_node_type::node_element)
                    continue;

                InstanceReadStatus   propertyStatus = ReadPropertyValue(ecClass, ecInstance, baseAccessString, propertyValueNode);

                // if property not found, ReadPropertyValue warns, so just continue..
                if (InstanceReadStatus::PropertyNotFound == propertyStatus)
                    continue;
                else if (InstanceReadStatus::TypeMismatch == propertyStatus || InstanceReadStatus::BadNavigationValue == propertyStatus)
                    continue;
                else if (InstanceReadStatus::Success != propertyStatus)
                    return propertyStatus;
                }

            MemoryECInstanceBaseP memory = ecInstance->GetAsMemoryECInstanceP();
            memory->_SetAllPropertiesCalculated(true);
            return InstanceReadStatus::Success;
            }

        /*---------------------------------------------------------------------------------**//**
        * @bsimethod
        +---------------+---------------+---------------+---------------+---------------+------*/
        InstanceReadStatus   ReadPropertyValue(ECClassCR ecClass, IECInstanceP ecInstance, Utf8String* baseAccessString, pugi::xml_node propertyValueNode)
            {
            // on entry, propertyValueNode is the XML node for the property value.
            Utf8String     propertyName(propertyValueNode.name());
            m_context.ResolveSerializedPropertyName(propertyName, ecClass);

            // try to find the property in the class.
            ECPropertyP ecProperty;
            if (NULL == (ecProperty = ecClass.GetPropertyP(propertyName)))
                {
                LOG.debugv("No ECProperty '%s' found in ECClass '%s'. Value will be ignored.", propertyName.c_str(), ecClass.GetFullName());
                return InstanceReadStatus::PropertyNotFound;
                }

            PrimitiveECPropertyP    primitiveProperty;
            ArrayECPropertyP        arrayProperty;
            StructECPropertyP       structProperty;
            NavigationECPropertyP   navigationProperty;
            if (NULL != (primitiveProperty = ecProperty->GetAsPrimitivePropertyP()))
                return ReadPrimitivePropertyValue(ecClass, primitiveProperty, ecInstance, baseAccessString, propertyValueNode);
            //Above is good, if SkipToElementEnd() is returned from ReadPrimitiveValue.
            else if (NULL != (arrayProperty = ecProperty->GetAsArrayPropertyP()))
                return ReadArrayPropertyValue(ecClass, arrayProperty, ecInstance, baseAccessString, propertyValueNode);
            else if (NULL != (structProperty = ecProperty->GetAsStructPropertyP()))
                return ReadEmbeddedStructPropertyValue(structProperty, ecInstance, baseAccessString, propertyValueNode);
            else if (nullptr != (navigationProperty = ecProperty->GetAsNavigationPropertyP()))
                return ReadNavigationPropertyValue(navigationProperty, ecInstance, baseAccessString, propertyValueNode);

            // should be one of those!
            BeAssert(false);
            return InstanceReadStatus::BadECProperty;
        }

        template <typename PrimitivePropertyType>
        ECUnitCP findOldUnit(ECClassCR ecClass, const PrimitivePropertyType primitiveProperty)
            {
            auto oldUnitName = m_context.GetOldUnitName(ecClass, *primitiveProperty);
            if (Utf8String::IsNullOrEmpty(oldUnitName.c_str()))
                {
                m_context.Issues().ReportV(IssueSeverity::Error, IssueCategory::BusinessProperties, IssueType::ECInstance, 
                    "No old unit name resolved for property '%s.%s'.  Cannot ensure old unit is the same or convertible to new unit.  Skipping value.", ecClass.GetFullName(), primitiveProperty->GetName().c_str());
                return nullptr;
                }

            auto ecUnitName = Units::UnitNameMappings::TryGetECNameFromOldName(oldUnitName.c_str());
            if (Utf8String::IsNullOrEmpty(ecUnitName))
                {
                m_context.Issues().ReportV(IssueSeverity::Error, IssueCategory::BusinessProperties, IssueType::ECInstance,
                    "An ECUnit name could not be found for the old unit '%s' for property '%s.%s'.  Cannot convert value. Skipping value.", ecUnitName, ecClass.GetFullName(), primitiveProperty->GetName().c_str());
                return nullptr;
                }

            auto oldUnit = primitiveProperty->GetKindOfQuantity()->GetSchema().LookupUnit(ecUnitName, true);
            if (!oldUnit)
                {
                m_context.Issues().ReportV(IssueSeverity::Error, IssueCategory::BusinessProperties, IssueType::ECInstance, "Failed to lookup unit '%s' for property '%s.%s'. Cannot convert value. Skipping value.", ecUnitName, ecClass.GetFullName(), primitiveProperty->GetName().c_str());
                return nullptr;
                }

            return oldUnit;
            }

        template <typename PrimitivePropertyType>
        bool convertUnit(ECClassCR ecClass, const PrimitivePropertyType primitiveProperty, const ECUnitCP oldUnit, const bool conversionRequired, const KindOfQuantityCP kindOfQuantity, ECValue &ecValue)
            {
            auto convertedValue = 0.0;
            if (ecValue.GetPrimitiveType() == PrimitiveType::PRIMITIVETYPE_Double)
                {
                if (ecValue.IsNull())
                    return true;
                auto pCode = oldUnit->Convert(convertedValue, ecValue.GetDouble(), kindOfQuantity->GetPersistenceUnit());
                if (Units::UnitsProblemCode::NoProblem != pCode)
                    {
                    m_context.Issues().ReportV(IssueSeverity::Error, IssueCategory::BusinessProperties, IssueType::ECInstance,
                        "Failed to convert value for property '%s.%s' from '%s' to '%s'. Skipping value.", ecClass.GetFullName(), primitiveProperty->GetName().c_str(), oldUnit->GetFullName().c_str(), kindOfQuantity->GetPersistenceUnit()->GetFullName().c_str());
                    return false;
                    }
                ecValue.SetDouble(convertedValue);
                return true;
                }
            else if (ecValue.GetPrimitiveType() == PrimitiveType::PRIMITIVETYPE_String)
                {
                if (!Utf8String::IsNullOrEmpty(ecValue.GetUtf8CP()))
                    {
                    double d;
                    if (1 == Utf8String::Sscanf_safe(ecValue.GetUtf8CP(), "%lg", &d))
                        {
                        auto pCode = oldUnit->Convert(convertedValue, d, kindOfQuantity->GetPersistenceUnit());
                        if (Units::UnitsProblemCode::NoProblem != pCode)
                            {
                            m_context.Issues().ReportV(IssueSeverity::Error, IssueCategory::BusinessProperties, IssueType::ECInstance,
                                "Failed to convert value for property '%s.%s' from '%s' to '%s'. Skipping value.", ecClass.GetFullName(), primitiveProperty->GetName().c_str(), oldUnit->GetName().c_str(), kindOfQuantity->GetPersistenceUnit()->GetName().c_str());
                            return false;
                            }
                        Utf8PrintfString dStr("%lg", convertedValue);
                        ecValue.SetUtf8CP(dStr.c_str());
                        return true;
                        }
                    if (conversionRequired)
                        {
                        m_context.Issues().ReportV(IssueSeverity::Error, IssueCategory::BusinessProperties, IssueType::ECInstance,
                            "Could not convert value for property '%s.%s' from string to double for unit conversion from '%s' to '%s'.  Skipping value.", ecClass.GetFullName(), primitiveProperty->GetName().c_str(), oldUnit->GetFullName().c_str(), kindOfQuantity->GetPersistenceUnit()->GetFullName().c_str());
                        return false;
                        }
                    }
                }
            else if (conversionRequired)
                {
                m_context.Issues().ReportV(IssueSeverity::Error, IssueCategory::BusinessProperties, IssueType::ECInstance,
                    "Unit conversion required for property '%s.%s' of type '%s' but conversion is only supported for doubles and strings that can be converted to double. Skipping value.", ecClass.GetFullName(), primitiveProperty->GetName().c_str(), primitiveProperty->GetTypeName().c_str());
                return false;
                }

            return true;
            }

        //--------------------------------------------------------------------------------------
        // @bsimethod
        //---------------+---------------+---------------+---------------+---------------+------
        InstanceReadStatus ReadPrimitiveArrayValues(ECClassCR ecClass, IECInstanceP ecInstance, Utf8StringCR accessString, PrimitiveArrayECPropertyCP primitiveProperty, PrimitiveType serializedMemberType, bool isFixedSizeArray, pugi::xml_node propertyValueNode)
            {
            // start the address out as zero.
            uint32_t index = 0;

            const auto memberType = primitiveProperty->GetPrimitiveElementType();
            const auto kindOfQuantity = primitiveProperty->GetKindOfQuantity();

            ECUnitCP oldUnit = nullptr;
            auto resolveUnits = false;
            auto conversionRequired = false;

            if (kindOfQuantity && m_context.HasUnitResolver())
                {
                resolveUnits = true;
                oldUnit = findOldUnit<PrimitiveArrayECPropertyCP>(ecClass, primitiveProperty);
                if (!oldUnit)
                    return InstanceReadStatus::Success;

                conversionRequired = !ECUnit::AreEqual(oldUnit, kindOfQuantity->GetPersistenceUnit());
                }

            // step through the nodes. Each should be a primitive value type like <int>value</int>
            for (pugi::xml_node arrayValueNode : propertyValueNode.children())
                {
                if (arrayValueNode.type() != pugi::xml_node_type::node_element)
                    continue;

                if (memberType == serializedMemberType && !ValidateArrayPrimitiveType(arrayValueNode.name(), memberType))
                    {
                    LOG.warningv("Incorrectly formatted array element found in array %s.  Expected: %s  Found: %s",
                                accessString.c_str(), SchemaParseUtils::PrimitiveTypeToString(memberType), arrayValueNode.name());
                    continue;
                    }

                if (!isFixedSizeArray)
                    ecInstance->AddArrayElements(accessString.c_str(), 1);

                // read it, populating the ECInstance using accessString and arrayIndex.
                InstanceReadStatus ixrStatus;
                ECValue ecValue;
                if (InstanceReadStatus::Success == (ixrStatus = ReadPrimitiveValue(ecValue, memberType, arrayValueNode, serializedMemberType)))
                    {
                    if (resolveUnits && !convertUnit<PrimitiveArrayECPropertyCP>(ecClass, primitiveProperty, oldUnit, conversionRequired, kindOfQuantity, ecValue))
                        return InstanceReadStatus::Success;

                    // If we failed to read the value above, the array member will have been allocated but left null.
                    // This allows any default value to be applied to it via CalculatedECPropertySpecification,
                    // and is less surprising than the old behavior which would have omitted the member entirely.
                    ECObjectsStatus setStatus = ecInstance->SetInternalValue(accessString.c_str(), ecValue, index);
                    if (ECObjectsStatus::Success != setStatus && ECObjectsStatus::PropertyValueMatchesNoChange != setStatus)
                        {
                        BeAssert(false);
                        return InstanceReadStatus::CantSetValue;
                        }
                    }

                // increment the array index.
                index++;
                }
            return InstanceReadStatus::Success;
            }

        //--------------------------------------------------------------------------------------
        // @bsimethod
        //---------------+---------------+---------------+---------------+---------------+------
        InstanceReadStatus  ReadNavigationPropertyValue(NavigationECPropertyP navigationProperty, IECInstanceP ecInstance, Utf8StringP baseAccessString, pugi::xml_node propertyValueNode)
            {
            // We always input string as the serialized type to handle the case where the instance is loaded in an environment where the type is different than when serialized
            if (navigationProperty->IsMultiple())
                {
                //WIP
                return InstanceReadStatus::XmlParseError;
                }
            else
                {
                // on entry, propertyValueNode is the xml node for the primitive property value.
                InstanceReadStatus   ixrStatus;
                ECValue              ecValue;
                if (InstanceReadStatus::Success != (ixrStatus = ReadNavigationValue(ecValue, navigationProperty, propertyValueNode, PrimitiveType::PRIMITIVETYPE_String)))
                    return ixrStatus;

                if (ecValue.IsUninitialized())
                    {
                    //A malformed value was found.  A warning was shown; just move on.
                    return InstanceReadStatus::Success;
                    }

                ECObjectsStatus setStatus;
                        Utf8String accessString;
                        CreateAccessString(accessString, baseAccessString, navigationProperty->GetName());
                setStatus = ecInstance->SetInternalValue(accessString.c_str(), ecValue);

                if (ECObjectsStatus::Success != setStatus && ECObjectsStatus::PropertyValueMatchesNoChange != setStatus)
                    LOG.warningv("Unable to set value for property %s", navigationProperty->GetName().c_str());

                BeAssert(ECObjectsStatus::Success == setStatus || ECObjectsStatus::PropertyValueMatchesNoChange == setStatus);

                return InstanceReadStatus::Success;
                }
            }

        /*---------------------------------------------------------------------------------**//**
        * @bsimethod
        +---------------+---------------+---------------+---------------+---------------+------*/
        InstanceReadStatus   ReadPrimitivePropertyValue(ECClassCR ecClass, PrimitiveECPropertyP primitiveProperty, IECInstanceP ecInstance, Utf8String* baseAccessString, pugi::xml_node propertyValueNode)
            {
            // on entry, propertyValueNode is the xml node for the primitive property value.
            InstanceReadStatus   ixrStatus;
            ECValue              ecValue;
            if (InstanceReadStatus::Success != (ixrStatus = ReadPrimitiveValue(ecValue, primitiveProperty->GetType(), propertyValueNode, m_context.GetSerializedPrimitiveType(*primitiveProperty))))
                return ixrStatus;

            if (ecValue.IsUninitialized())
                {
                //A malformed value was found.  A warning was shown; just move on.
                return InstanceReadStatus::Success;
                }

            if (const auto kindOfQuantity = primitiveProperty->GetKindOfQuantity(); kindOfQuantity && m_context.HasUnitResolver())
                {
                auto oldUnit = findOldUnit<PrimitiveECPropertyP>(ecClass, primitiveProperty);
                if (!oldUnit)
                    return InstanceReadStatus::Success;

                const auto conversionRequired = !ECUnit::AreEqual(oldUnit, kindOfQuantity->GetPersistenceUnit());
                if (!convertUnit<PrimitiveECPropertyP>(ecClass, primitiveProperty, oldUnit, conversionRequired, kindOfQuantity, ecValue))
                    return InstanceReadStatus::Success;
                }

            ECObjectsStatus setStatus;
            Utf8String accessString;
            CreateAccessString(accessString, baseAccessString, primitiveProperty->GetName());
            setStatus = ecInstance->SetInternalValue(accessString.c_str(), ecValue);

            if (ECObjectsStatus::Success != setStatus && ECObjectsStatus::PropertyValueMatchesNoChange != setStatus)
                LOG.warningv("Unable to set value for property %s", primitiveProperty->GetName().c_str());

            BeAssert(ECObjectsStatus::Success == setStatus || ECObjectsStatus::PropertyValueMatchesNoChange == setStatus);

            return InstanceReadStatus::Success;
            }

        /*---------------------------------------------------------------------------------**//**
        * @bsimethod
        +---------------+---------------+---------------+---------------+---------------+------*/
        InstanceReadStatus   ReadArrayPropertyValue(ECClassCR ecClass, ArrayECPropertyP arrayProperty, IECInstanceP ecInstance, Utf8String* baseAccessString, pugi::xml_node propertyValueNode)
            {
            // on entry, propertyValueNode is the xml node for the primitive property value.
            Utf8String    accessString;
            CreateAccessString(accessString, baseAccessString, arrayProperty->GetName());

            // start the address out as zero.
            uint32_t    index = 0;

            // we have to find out what type the array is.
            ArrayKind   arrayKind = arrayProperty->GetKind();
            if (ARRAYKIND_Primitive == arrayKind)
                {
                PrimitiveArrayECPropertyCP primitiveArray = arrayProperty->GetAsPrimitiveArrayProperty();
                PrimitiveType serializedMemberType = m_context.GetSerializedPrimitiveArrayType(*primitiveArray);

                bool            isFixedSizeArray = false;

                if (primitiveArray->GetMinOccurs() == primitiveArray->GetMaxOccurs())
                    isFixedSizeArray = true;

                InstanceReadStatus status = ReadPrimitiveArrayValues(ecClass, ecInstance, accessString, primitiveArray, serializedMemberType, isFixedSizeArray, propertyValueNode);
                if (InstanceReadStatus::Success != status)
                    return status;
                }

            else if (ARRAYKIND_Struct == arrayKind)
                {
                StructArrayECPropertyP structArray = arrayProperty->GetAsStructArrayPropertyP();
                ECClassCR   structMemberType = structArray->GetStructElementType();
                for (pugi::xml_node arrayValueNode : propertyValueNode.children())
                    {
                    if(arrayValueNode.type() != pugi::xml_node_type::node_element)
                        continue;
                    // the Name of each node element is the class name of structMemberType.
                    // For polymorphic arrays, the Name might also be the name of a class that has structMemberType as a BaseType.
                    ECClassCP   thisMemberType;
                    Utf8String  arrayMemberType(arrayValueNode.name());
                    m_context.ResolveSerializedClassName(arrayMemberType, structMemberType.GetSchema());
                    if (nullptr == (thisMemberType = ValidateArrayStructType(arrayMemberType.c_str(), &structMemberType)))
                        {
                        LOG.warningv("Incorrect structType found in %s.  Expected: %s  Found: %s",
                                     accessString.c_str(), structMemberType.GetName().c_str(), arrayValueNode.name());
                        continue;
                        }

                    InstanceReadStatus ixrStatus;
                    if (InstanceReadStatus::Success != (ixrStatus = ReadStructArrayMember(*thisMemberType, ecInstance, accessString, index, arrayValueNode)))
                        return ixrStatus;

                    // increment the array index.
                    index++;
                    }
                }

            return InstanceReadStatus::Success;
            }

        /*---------------------------------------------------------------------------------**//**
        * @bsimethod
        +---------------+---------------+---------------+---------------+---------------+------*/
        InstanceReadStatus   ReadEmbeddedStructPropertyValue(StructECPropertyP structProperty, IECInstanceP ecInstance, Utf8String* baseAccessString, pugi::xml_node propertyValueNode)
            {
            // empty element OK for struct - all members are null.
            Utf8String    thisAccessString;
            CreateAccessString(thisAccessString, baseAccessString, structProperty->GetName());
            thisAccessString.append(".");

            ICustomECStructSerializerP customECStructSerializerP = CustomStructSerializerManager::GetManager().GetCustomSerializer(structProperty, *ecInstance);
            if (customECStructSerializerP)
                return ReadCustomSerializedStruct(structProperty, ecInstance, baseAccessString, customECStructSerializerP, propertyValueNode);

            return ReadInstanceOrStructMembers(structProperty->GetType(), ecInstance, &thisAccessString, propertyValueNode);
            }

        /*---------------------------------------------------------------------------------**//**
        * @bsimethod
        +---------------+---------------+---------------+---------------+---------------+------*/
        InstanceReadStatus   ReadCustomSerializedStruct(StructECPropertyP structProperty, IECInstanceP ecInstance, Utf8String* baseAccessString, ICustomECStructSerializerP customECStructSerializerP, pugi::xml_node propertyValueNode)
            {
            Utf8String propertyValueString = propertyValueNode.text().as_string();

            // empty?
            if (!Utf8String::IsNullOrEmpty(propertyValueString.c_str()))
                return InstanceReadStatus::Success;

            Utf8String    thisAccessString;
            CreateAccessString(thisAccessString, baseAccessString, structProperty->GetName());
            thisAccessString.append(".");

            customECStructSerializerP->LoadStructureFromString(structProperty, *ecInstance, thisAccessString.c_str(), propertyValueString.c_str());

            return InstanceReadStatus::Success;
            }

        /*---------------------------------------------------------------------------------**//**
        * @bsimethod
        +---------------+---------------+---------------+---------------+---------------+------*/
#if defined (_MSC_VER)
#pragma warning(disable:4189) // setStatus unused if NDEBUG set.
#endif // defined (_MSC_VER)

        InstanceReadStatus   ReadStructArrayMember(ECClassCR structClass, IECInstanceP owningInstance, Utf8String& accessString, uint32_t index, pugi::xml_node arrayMemberValue)
            {
            // On entry, arrayMemberValue is an XML Node for the element that starts the struct.

            // Create an IECInstance for the array member.
            IECInstancePtr      structInstance = m_context.CreateStandaloneInstance(structClass).get();

            InstanceReadStatus   ixrStatus;
            if (InstanceReadStatus::Success != (ixrStatus = ReadInstanceOrStructMembers(structClass, structInstance.get(), NULL, arrayMemberValue)))
                return ixrStatus;

            // every StructArrayMember is a new ECInstance,
            // set the value
            ECValue structValue;
            structValue.SetStruct(structInstance.get());

            if (!IECInstance::IsFixedArrayProperty(*owningInstance, accessString.c_str()))
                {
                // add the value to the array.
                owningInstance->AddArrayElements(accessString.c_str(), 1);
                }

            ECObjectsStatus setStatus = owningInstance->SetInternalValue(accessString.c_str(), structValue, index);
            if (ECObjectsStatus::Success != setStatus)
                BeAssert(ECObjectsStatus::Success == setStatus);

            return InstanceReadStatus::Success;
            }

#if defined (_MSC_VER)
#pragma warning(default:4189)
#endif // defined (_MSC_VER)

        /*---------------------------------------------------------------------------------**//**
        * @bsimethod
        +---------------+---------------+---------------+---------------+---------------+------*/
        InstanceReadStatus   ReadPrimitiveValue(ECValueR ecValue, PrimitiveType propertyType, pugi::xml_node primitiveValueNode, PrimitiveType serializedType)
            {
            // If we fail to read the property value for some reason, return it as null
            ecValue.SetToNull();
            ecValue.SetPrimitiveType(propertyType);

            // On entry primitiveValueNode is the XML node that holds the value.
            // First check to see if the value is set to NULL
            auto nullValueAttr = primitiveValueNode.attribute(ECINSTANCE_XSI_NIL_ATTRIBUTE);
            if (nullValueAttr)
                if (nullValueAttr.as_bool())
                    return InstanceReadStatus::Success;

            // If we're able to determine that the serialized type differs from the ECProperty's type, use that information to convert to correct type
            if (serializedType != propertyType)
                {
                Utf8String propertyValueString = primitiveValueNode.text().as_string();
                if (!Utf8String::IsNullOrEmpty(propertyValueString.c_str()))
                    {
                    ecValue.SetUtf8CP(propertyValueString.c_str(), false);
                    if (ecValue.ConvertToPrimitiveType(serializedType) && ecValue.ConvertToPrimitiveType(propertyType))
                        return InstanceReadStatus::Success;
                    else if (PRIMITIVETYPE_Integer == propertyType && PRIMITIVETYPE_Long == serializedType)
                        {
                        // Code below will give us INT_MAX if serialized integer out of range of Int32.
                        // We don't want that when converting primitive types, and it's not really helpful to users, but not sure if anyone depends on it
                        // So only circumventing it for this special case when we know the serialized type
                        return InstanceReadStatus::TypeMismatch;
                        }
                    }
                }

            switch (propertyType)
                {
                    case PRIMITIVETYPE_Binary:
                    {
                    T_ByteArray                     byteArray;

                    // try to read the actual value.
                    Utf8String     propertyValueString = primitiveValueNode.text().as_string();
                    if (Utf8String::IsNullOrEmpty(propertyValueString.c_str()))
                        return InstanceReadStatus::Success;

                    if (!convertStringToByteArray(byteArray, propertyValueString.c_str()))
                        {
                        LOG.errorv("Type mismatch in deserialization: \"%s\" is not Binary", propertyValueString.c_str());
                        return InstanceReadStatus::TypeMismatch;
                        }
                    if (!byteArray.empty())
                        ecValue.SetBinary(&byteArray.front(), byteArray.size(), true);
                    else
                        ecValue.SetBinary(NULL, 0, true);
                    break;
                    }

                    case PRIMITIVETYPE_IGeometry:
                    {
                    T_ByteArray                     byteArray;
                    // try to read the actual value.
                    auto     propertyValueText = primitiveValueNode.text();
                    Utf8String     propertyValueString = propertyValueText.as_string();
                    bool nodeHasContent = !primitiveValueNode.empty(); //meaning the node has child-nodes
                    bool nodeHasText = !propertyValueText.empty(); //meaning the node has text-value (this can be true even if nodeHasContent is false)
                    if (!nodeHasContent && !nodeHasText) //neither text nor child nodes
                        return InstanceReadStatus::Success;

                    if(nodeHasText && convertStringToByteArray(byteArray, propertyValueString.c_str()))
                        {
                        if(ecValue.SetIGeometry(&byteArray.front(), byteArray.size(), true) == BentleyStatus::SUCCESS)
                            break;
                        }

                    // It is possible that this came in as serialized text Xml, and not binary.
                    // We save instance xml in iModels for custom attributes. As far as we can tell,
                    // no one has ever intentionally created a custom attribute with IGeometry data in it.
                    // It seems very unlikely that a CA was saved AND that CA used xml but it is possible.
                    // Log an error but return success. It will result in the data being dropped on load but
                    // the iModel would still open fine.
                    LOG.errorv("Type mismatch in deserialization: \"%s\" is not Binary", propertyValueString.c_str());
                    return InstanceReadStatus::Success;
                    }
                    case PRIMITIVETYPE_Boolean:
                    {
                    bool boolValue = false;
                    BePugiXmlValueResult status = BePugiXmlHelper::ContentBooleanValue(primitiveValueNode, boolValue);
                    if (status == BePugiXmlValueResult::Null)
                        ecValue.SetToNull();
                    else if(status == BePugiXmlValueResult::TypeMismatch)
                        return InstanceReadStatus::TypeMismatch;
                    else
                        ecValue.SetBoolean(boolValue);
                    break;
                    }

                    case PRIMITIVETYPE_DateTime:
                    {
                    int64_t ticks;
                    BePugiXmlValueResult status = BePugiXmlHelper::GetContentInt64Value(primitiveValueNode, ticks);
                    if (status == BePugiXmlValueResult::Null)
                        ecValue.SetToNull();
                    else if(status == BePugiXmlValueResult::TypeMismatch)
                        return InstanceReadStatus::TypeMismatch;
                    else
                        ecValue.SetDateTimeTicks(ticks);
                    break;
                    }

                    case PRIMITIVETYPE_Double:
                    {
                    double  doubleValue;
                    BePugiXmlValueResult status = BePugiXmlHelper::GetContentDoubleValue(primitiveValueNode, doubleValue);
                    if (status == BePugiXmlValueResult::Null)
                        ecValue.SetToNull();
                    else if(status == BePugiXmlValueResult::TypeMismatch)
                        return InstanceReadStatus::TypeMismatch;
                    else
                        ecValue.SetDouble(doubleValue);
                    break;
                    }

                    case PRIMITIVETYPE_Integer:
                    {
                    int32_t intValue;
                    BePugiXmlValueResult status = BePugiXmlHelper::GetContentInt32Value(primitiveValueNode, intValue);
                    if (status == BePugiXmlValueResult::Null)
                        ecValue.SetToNull();
                    else if(status == BePugiXmlValueResult::TypeMismatch)
                        return InstanceReadStatus::TypeMismatch;
                    else
                        ecValue.SetInteger(intValue);
                    break;
                    }

                    case PRIMITIVETYPE_Long:
                    {
                    int64_t longValue;
                    BePugiXmlValueResult status = BePugiXmlHelper::GetContentInt64Value(primitiveValueNode, longValue);
                    if (status == BePugiXmlValueResult::Null)
                        ecValue.SetToNull();
                    else if(status == BePugiXmlValueResult::TypeMismatch)
                        return InstanceReadStatus::TypeMismatch;
                    else
                        ecValue.SetLong(longValue);
                    break;
                    }

                    case PRIMITIVETYPE_Point2d:
                    {
                    double x, y;
                    BePugiXmlValueResult status = BePugiXmlHelper::GetContentDPoint2dValue(primitiveValueNode, x, y);
                    if (status == BePugiXmlValueResult::Null)
                        ecValue.SetToNull();
                    else if(status == BePugiXmlValueResult::TypeMismatch)
                        return InstanceReadStatus::TypeMismatch;
                    else
                        {
                        DPoint2d point2d;
                        point2d.x = x;
                        point2d.y = y;
                        ecValue.SetPoint2d(point2d);
                        }
                    break;
                    }

                    case PRIMITIVETYPE_Point3d:
                    {
                    double x, y, z;
                    BePugiXmlValueResult status = BePugiXmlHelper::GetContentDPoint3dValue(primitiveValueNode, x, y, z);
                    if (status == BePugiXmlValueResult::Null)
                        ecValue.SetToNull();
                    else if(status == BePugiXmlValueResult::TypeMismatch)
                        return InstanceReadStatus::TypeMismatch;
                    else
                        {
                        DPoint3d point3d;
                        point3d.x = x;
                        point3d.y = y;
                        point3d.z = z;
                        ecValue.SetPoint3d(point3d);
                        }
                    break;
                    }

                    case PRIMITIVETYPE_String:
                    {
                    if (primitiveValueNode)
                        ecValue.SetUtf8CP(primitiveValueNode.text().as_string());
                    
                    break;
                    }

                    default:
                    {
                    BeAssert(false);
                    return InstanceReadStatus::BadPrimitivePropertyType;
                    }
                }

            return InstanceReadStatus::Success;
            }

        /*---------------------------------------------------------------------------------**//**
        * @bsimethod
        +---------------+---------------+---------------+---------------+---------------+------*/
        InstanceReadStatus   ReadNavigationValue(ECValueR ecValue, NavigationECPropertyP navProperty, pugi::xml_node navigationValueNode, PrimitiveType serializedType)
            {
            PrimitiveType propertyType = navProperty->GetType();

            // If we fail to read the property value for some reason, return it as null
            ecValue.SetToNull();

            // On entry navigationValueNode is the XML node that holds the value.
            // First check to see if the value is set to NULL
            auto nullAttr = navigationValueNode.attribute(ECINSTANCE_XSI_NIL_ATTRIBUTE);
            if (nullAttr)
                if (nullAttr.as_bool(false))
                    return InstanceReadStatus::Success;

            bvector<pugi::xml_node> valueNodes;

            bool isFirst = true;
            Utf8String firstNavValueName;
            Utf8String secondNavValueName;
            for(pugi::xml_node navValueNode : navigationValueNode.children())
            {
            if(navValueNode.type() != pugi::xml_node_type::node_element)
                continue;

            if(isFirst)
                {
                valueNodes.push_back(navValueNode); 
                isFirst = false;
                firstNavValueName = navValueNode.name();
                continue;
                }

            secondNavValueName = navValueNode.name();

            // Invalid if the same value is defined twice or if the neither value is an Id
            if (firstNavValueName.Equals(secondNavValueName) || (!firstNavValueName.StartsWith(ECINSTANCE_ID_ATTRIBUTE) && !secondNavValueName.StartsWith(ECINSTANCE_ID_ATTRIBUTE)))
                return InstanceReadStatus::BadNavigationValue;

            valueNodes.push_back(navValueNode);
            break;
            }

            if (valueNodes.size() == 1 && !firstNavValueName.StartsWith(ECINSTANCE_ID_ATTRIBUTE))
                return InstanceReadStatus::BadNavigationValue;

            ECClassId relClassId;
            ECRelationshipClassCP relClass = nullptr;
            BeInt64Id id;
            for (pugi::xml_node valueNode : valueNodes)
                {
                Utf8String valueNodeName(valueNode.name());

                if (0 == valueNodeName.CompareTo(ECINSTANCE_RELATIONSHIPID_ATTTRIBUTE))
                    {
                    auto valueNodeText = valueNode.text();
                    if (valueNodeText.empty())
                        return InstanceReadStatus::BadNavigationValue;

                    if (BentleyStatus::SUCCESS != ECClassId::FromString(relClassId, valueNodeText.as_string()))
                        return InstanceReadStatus::BadNavigationValue;
                    }
                else if (0 == valueNodeName.CompareTo(ECINSTANCE_RELATIONSHIPNAME_ATTRIBUTE))
                    {
                    Utf8String relClassName = valueNode.text().as_string();

                    Utf8String alias;
                    Utf8String className;
                    if (ECObjectsStatus::Success != ECClass::ParseClassName(alias, className, relClassName))
                        return InstanceReadStatus::BadNavigationValue;

                    ECSchemaCP resolvedSchema = navProperty->GetClass().GetSchema().GetSchemaByAliasP(alias);
                    if (nullptr == resolvedSchema)
                        return InstanceReadStatus::BadNavigationValue;

                    ECClassCP resolvedClass = resolvedSchema->GetClassCP(className.c_str());
                    if (nullptr == resolvedClass)
                        return InstanceReadStatus::BadNavigationValue;

                    relClass = resolvedClass->GetRelationshipClassCP();
                    if (nullptr == relClass)
                        return InstanceReadStatus::BadNavigationValue;
                    }
                 else if (valueNodeName.StartsWith(ECINSTANCE_ID_ATTRIBUTE))
                     {
                     size_t primitivePos = valueNodeName.find(":");
                     Utf8String primitiveType(valueNodeName.substr(primitivePos + 1, valueNodeName.size() - primitivePos));
                     if (!ValidateArrayPrimitiveType(primitiveType.c_str(), propertyType))
                         return InstanceReadStatus::BadNavigationValue;

                     if (PrimitiveType::PRIMITIVETYPE_Long == propertyType)
                         {
                         int64_t longValue;
                         auto status = BePugiXmlHelper::GetContentInt64Value(valueNode, longValue);
                         if (status != BePugiXmlValueResult::Success)
                             {
                             if (BePugiXmlValueResult::TypeMismatch == status)
                                 return InstanceReadStatus::TypeMismatch;
                             if (BePugiXmlValueResult::Null == status)
                                 ecValue.SetToNull();
                             return InstanceReadStatus::Success;
                             }

                         id = BeInt64Id(longValue);
                         }
                     }
                }

            if (!id.IsValid())
                return InstanceReadStatus::BadNavigationValue;

            if (relClassId.IsValid())
                ecValue.SetNavigationInfo(id, relClassId);
            else
                ecValue.SetNavigationInfo(id, relClass);

            return InstanceReadStatus::Success;
            }

        /*---------------------------------------------------------------------------------**//**
        * @bsimethod
        +---------------+---------------+---------------+---------------+---------------+------*/
        bool                            ValidateArrayPrimitiveType(Utf8CP typeFound, PrimitiveType expectedType)
            {
            return (0 == strcmp(typeFound, SchemaParseUtils::PrimitiveTypeToString(expectedType)));
            }

        /*---------------------------------------------------------------------------------**//**
        * @bsimethod
        +---------------+---------------+---------------+---------------+---------------+------*/
        ECClassCP                       ValidateArrayStructType(Utf8CP typeFound, ECClassCP expectedType)
            {
            // the common case is that they're all of the expected ECClass.
            if (0 == strcmp(typeFound, expectedType->GetName().c_str()))
                return expectedType;

            ECSchemaCP  schema = GetSchema();
            if (NULL == schema)
                return NULL;

            // typeFound must resolve to an ECClass that is either expectedType or a class that has expectedType as a Base GetClass().
            ECClassCP    classFound;
            if (NULL == (classFound = schema->GetClassCP(typeFound)) || !classFound->Is(expectedType))
                return NULL;

            return classFound;
            }
    };

/*---------------------------------------------------------------------------------**//**
* In old unit schema some ecclass where treated as both struct and custom attribute
* EC 3.0 doesnot allow this and it deserializes those classes to struct by default.
* this class creates a customattribute for the same name with "Attr" suffix and loads
* the customattributes from xmlnode.
* @bsistruct
+---------------+---------------+---------------+---------------+---------------+------*/
struct NamedAttributeDeserializer : ICustomAttributeDeserializer
    {
    private:
        Utf8String m_oldAttributeClassName;
        Utf8String m_newClassName;

        bool CreateCustomAttribute(ECSchemaReadContextR schemaContext, Utf8StringCR existingStructClassName, Utf8StringCR schemaName)
            {
            ECSchemaPtr schema = GetSchema(schemaName, schemaContext);
            if (schema.IsNull())
                return false;

            ECClassCP attributeClass = nullptr;
            return CustomAttributeDeserializerManager::CreateAttrClassVersion(schema.get(), existingStructClassName.c_str(), m_newClassName.c_str(), attributeClass);
            }

        Utf8String GetSchemaName(pugi::xml_node xmlNode)
            {
            // get the xmlns name, if there is one.
            Utf8CP  schemaName = xmlNode.attribute("xmlns").as_string();
            Utf8String fullSchemaName = "";
            if (!Utf8String::IsNullOrEmpty(schemaName) && 0 != BeStringUtilities::Strnicmp(schemaName, ECXML_URI, strlen(ECXML_URI)))
                {
                fullSchemaName = schemaName;
                }
            return fullSchemaName;
            }

        bool ClassExists(Utf8StringCR className, Utf8StringCR schemaName, ECSchemaReadContextR schemaContext)
            {
            ECSchemaPtr schema = GetSchema(schemaName, schemaContext);
            if (!schema.IsValid())
                return false;
            ECClassCP ecClass = schema->GetClassCP(className.c_str());
            if (ecClass)
                return true;
            return false;
            }

        ECSchemaPtr GetSchema(Utf8String schemaName, ECSchemaReadContextR context)
            {
            SchemaKey key;
            if (ECObjectsStatus::Success != SchemaKey::ParseSchemaFullName(key, schemaName.c_str()))
                return NULL;

            return context.LocateSchema(key, SchemaMatchType::LatestReadCompatible);//Abeesh: Preserving old behavior. Ideally it should be exact
            }

    public:

        NamedAttributeDeserializer(Utf8String oldAttributeClassName, Utf8String newClassName)
            :m_oldAttributeClassName(oldAttributeClassName), m_newClassName(newClassName)
            {}

        InstanceReadStatus LoadCustomAttributeFromString(IECInstancePtr& ecInstance, pugi::xml_node xmlNode, ECInstanceReadContextR context, ECSchemaReadContextR schemaContext, IECCustomAttributeContainerR customAttributeContainer)
            {
            Utf8String schemaName = GetSchemaName(xmlNode);

            bool attributeExists = true;
            if (!ClassExists(m_newClassName, schemaName, schemaContext))
                attributeExists = CreateCustomAttribute(schemaContext, m_oldAttributeClassName, schemaName);

            if (!attributeExists)
                return InstanceReadStatus::BadElement;

            InstanceXmlReader   reader(context, xmlNode, m_newClassName);
            return reader.ReadInstance(ecInstance);
            }

    };

bool CustomAttributeDeserializerManager::CreateAttrClassVersion(ECSchemaP schema, Utf8CP existingClassName, Utf8CP newClassName, ECClassCP& attributeClass)
    {
    ECClassCP structClass = schema->GetClassCP(existingClassName);
    if (!structClass)
        {
        LOG.errorv("Failed to inject customattribute class: \"%s\" to schema by copying struct class: \"%s\" which does not exist", newClassName, existingClassName);
        BeAssert(false);
        return false;
        }

    ECCustomAttributeClassP newClass;
    if (ECObjectsStatus::Success != schema->CreateCustomAttributeClass(newClass, newClassName))
        {
        attributeClass = schema->GetClassCP(newClassName);
        return nullptr != attributeClass;
        }

    for (ECPropertyCP sourceProp : structClass->GetProperties(false))
        {
        ECPropertyP destProperty;
        newClass->CopyProperty(destProperty, sourceProp, true);
        }

    attributeClass = newClass;
    return true;
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
CustomAttributeDeserializerManager::CustomAttributeDeserializerManager()
    {
    // we could add needed deserializers here.
    m_deserializers["UnitSpecification"] = new NamedAttributeDeserializer("UnitSpecification", "UnitSpecificationAttr");
    m_deserializers["DisplayUnitSpecification"] = new NamedAttributeDeserializer("DisplayUnitSpecification", "DisplayUnitSpecificationAttr");
    }

/*---------------------------------------------------------------------------------**//**
@bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
CustomAttributeDeserializerManager::~CustomAttributeDeserializerManager()
    {
    m_deserializers.clear();
    }

/*---------------------------------------------------------------------------------**//**
@bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus            CustomAttributeDeserializerManager::AddCustomDeserializer(Utf8CP deserializerName, ICustomAttributeDeserializerP deserializer)
    {
    if (GetCustomDeserializer(deserializerName))
        return ERROR;

    m_deserializers[Utf8String(deserializerName)] = deserializer;
    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
@bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
CustomAttributeDeserializerManagerR                   CustomAttributeDeserializerManager::GetManager()
    {
    static CustomAttributeDeserializerManagerP   s_deserializerManager = NULL;

    if (NULL == s_deserializerManager)
        s_deserializerManager = new CustomAttributeDeserializerManager();

    return *s_deserializerManager;
    }

/*---------------------------------------------------------------------------------**//**
@bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ICustomAttributeDeserializerP                      CustomAttributeDeserializerManager::GetCustomDeserializer(Utf8CP deserializerName) const
    {
    if (m_deserializers.empty())
        return NULL;

    AttributeDeserializerMap::const_iterator it = m_deserializers.find(deserializerName);
    if (it == m_deserializers.end())
        return NULL;

    return it->second;
    }

// =====================================================================================
// InstanceXMLWriter class
// =====================================================================================
struct  InstanceXmlWriter
    {
    private:
        BeXmlWriter*     m_xmlWriter;


    public:
        /*---------------------------------------------------------------------------------**//**
        * @bsimethod
        +---------------+---------------+---------------+---------------+---------------+------*/
        InstanceXmlWriter(BeXmlWriter *writer)
            : m_xmlWriter(writer)
            {
            writer->SetIndentation(4);
            }

        /*---------------------------------------------------------------------------------**//**
        * @bsimethod
        +---------------+---------------+---------------+---------------+---------------+------*/
        InstanceWriteStatus     WriteInstance(IECInstanceCR ecInstance, bool writeInstanceId, Utf8CP className)
            {
            ECClassCR   ecClass = ecInstance.GetClass();
            ECSchemaCR  ecSchema = ecClass.GetSchema();
            Utf8String  fullSchemaName = ecSchema.GetLegacyFullSchemaName();

            m_xmlWriter->WriteElementStart(className, fullSchemaName.c_str());

            auto relationshipInstance = dynamic_cast<IECRelationshipInstanceCP> (&ecInstance);
            // if relationship, need the attributes used in relationships.
            if (NULL != relationshipInstance)
                {
                if (!relationshipInstance->GetSource().IsValid())
                    return InstanceWriteStatus::XmlWriteError;

                Utf8String sourceClassName;
                Utf8String sourceFullSchemaName = relationshipInstance->GetSource()->GetClass().GetSchema().GetLegacyFullSchemaName();
                if (0 != sourceFullSchemaName.CompareTo(fullSchemaName))
                    sourceClassName.Sprintf("%s:%s", sourceFullSchemaName.c_str(), relationshipInstance->GetSource()->GetClass().GetName().c_str());
                else
                    sourceClassName.Sprintf("%s", relationshipInstance->GetSource()->GetClass().GetName().c_str());
                m_xmlWriter->WriteAttribute(ECINSTANCE_SOURCEINSTANCEID_ATTRIBUTE, relationshipInstance->GetSource()->GetInstanceId().c_str());
                m_xmlWriter->WriteAttribute(ECINSTANCE_SOURCECLASS_ATTRIBUTE, sourceClassName.c_str());

                if (!relationshipInstance->GetTarget().IsValid())
                    return InstanceWriteStatus::XmlWriteError;

                Utf8String targetClassName;
                Utf8String targetFullSchemaName = relationshipInstance->GetTarget()->GetClass().GetSchema().GetLegacyFullSchemaName();
                if (0 != targetFullSchemaName.CompareTo(fullSchemaName))
                    targetClassName.Sprintf("%s:%s", targetFullSchemaName.c_str(), relationshipInstance->GetTarget()->GetClass().GetName().c_str());
                else
                    targetClassName.Sprintf("%s", relationshipInstance->GetTarget()->GetClass().GetName().c_str());
                m_xmlWriter->WriteAttribute(ECINSTANCE_TARGETINSTANCEID_ATTRIBUTE, relationshipInstance->GetTarget()->GetInstanceId().c_str());
                m_xmlWriter->WriteAttribute(ECINSTANCE_TARGETCLASS_ATTRIBUTE, targetClassName.c_str());
                }

            if (writeInstanceId)
                m_xmlWriter->WriteAttribute(ECXML_ECINSTANCE_INSTANCEID_ATTRIBUTE, ecInstance.GetInstanceIdForSerialization().c_str());

            InstanceWriteStatus status = WritePropertyValuesOfClassOrStructArrayMember(ecClass, ecInstance, NULL);
            if (status != InstanceWriteStatus::Success)
                return status;
            m_xmlWriter->WriteElementEnd();
            return InstanceWriteStatus::Success;
            }

        /*---------------------------------------------------------------------------------**//**
        * @bsimethod
        +---------------+---------------+---------------+---------------+---------------+------*/
        InstanceWriteStatus     WriteInstanceLatestVersion(IECInstanceCR ecInstance, bool writeInstanceId, Utf8CP className)
            {
            ECClassCR   ecClass = ecInstance.GetClass();
            ECSchemaCR  ecSchema = ecClass.GetSchema();
            Utf8String  fullSchemaName = ecSchema.GetFullSchemaName();

            m_xmlWriter->WriteElementStart(className, fullSchemaName.c_str());

            auto relationshipInstance = dynamic_cast<IECRelationshipInstanceCP> (&ecInstance);
            // if relationship, need the attributes used in relationships.
            if (nullptr != relationshipInstance)
                {
                if (!relationshipInstance->GetSource().IsValid())
                    return InstanceWriteStatus::XmlWriteError;

                Utf8String sourceClassName;
                if (0 != relationshipInstance->GetSource()->GetClass().GetSchema().GetFullSchemaName().CompareTo(fullSchemaName))
                    sourceClassName.Sprintf("%s:%s", relationshipInstance->GetSource()->GetClass().GetSchema().GetFullSchemaName().c_str(), relationshipInstance->GetSource()->GetClass().GetName().c_str());
                else
                    sourceClassName.Sprintf("%s", relationshipInstance->GetSource()->GetClass().GetName().c_str());
                m_xmlWriter->WriteAttribute(ECINSTANCE_SOURCEINSTANCEID_ATTRIBUTE, relationshipInstance->GetSource()->GetInstanceId().c_str());
                m_xmlWriter->WriteAttribute(ECINSTANCE_SOURCECLASS_ATTRIBUTE, sourceClassName.c_str());

                if (!relationshipInstance->GetTarget().IsValid())
                    return InstanceWriteStatus::XmlWriteError;

                Utf8String targetClassName;
                if (0 != relationshipInstance->GetTarget()->GetClass().GetSchema().GetFullSchemaName().CompareTo(fullSchemaName))
                    targetClassName.Sprintf("%s:%s", relationshipInstance->GetTarget()->GetClass().GetSchema().GetFullSchemaName().c_str(), relationshipInstance->GetTarget()->GetClass().GetName().c_str());
                else
                    targetClassName.Sprintf("%s", relationshipInstance->GetTarget()->GetClass().GetName().c_str());
                m_xmlWriter->WriteAttribute(ECINSTANCE_TARGETINSTANCEID_ATTRIBUTE, relationshipInstance->GetTarget()->GetInstanceId().c_str());
                m_xmlWriter->WriteAttribute(ECINSTANCE_TARGETCLASS_ATTRIBUTE, targetClassName.c_str());
                }

            if (writeInstanceId)
                m_xmlWriter->WriteAttribute(ECXML_ECINSTANCE_INSTANCEID_ATTRIBUTE, ecInstance.GetInstanceIdForSerialization().c_str());

            InstanceWriteStatus status = WritePropertyValuesOfClassOrStructArrayMember(ecClass, ecInstance, NULL);
            if (status != InstanceWriteStatus::Success)
                return status;
            m_xmlWriter->WriteElementEnd();
            return InstanceWriteStatus::Success;
            }

        /*---------------------------------------------------------------------------------**//**
        * @bsimethod
        +---------------+---------------+---------------+---------------+---------------+------*/
        InstanceWriteStatus     WriteInstance(IECInstanceCR ecInstance, bool writeInstanceId)
            {
            Utf8CP className = ecInstance.GetClass().GetName().c_str();

            return WriteInstance(ecInstance, writeInstanceId, className);
            }

        //---------------------------------------------------------------------------------------
        // @bsimethod
        //---------------------------------------------------------------------------------------
        InstanceWriteStatus     WriteInstanceLatestVersion(IECInstanceCR ecInstance, bool writeInstanceId)
            {
            Utf8CP className = ecInstance.GetClass().GetName().c_str();

            return WriteInstanceLatestVersion(ecInstance, writeInstanceId, className);
            }

        /*---------------------------------------------------------------------------------**//**
        * @bsimethod
        +---------------+---------------+---------------+---------------+---------------+------*/
        InstanceWriteStatus     WritePropertyValuesOfClassOrStructArrayMember(ECClassCR ecClass, IECInstanceCR ecInstance, Utf8String* baseAccessString)
            {
            CustomStructSerializerManagerR customStructSerializerMgr = CustomStructSerializerManager::GetManager();

            ECPropertyIterableCR    collection = ecClass.GetProperties(true);
            for (ECPropertyP ecProperty : collection)
                {
                PrimitiveECPropertyP    primitiveProperty;
                ArrayECPropertyP        arrayProperty;
                StructECPropertyP       structProperty;
                NavigationECPropertyP   navProperty;
                InstanceWriteStatus     ixwStatus = InstanceWriteStatus::BadPrimitivePropertyType;

                if (NULL != (primitiveProperty = ecProperty->GetAsPrimitivePropertyP()))
                    ixwStatus = WritePrimitivePropertyValue(*primitiveProperty, ecInstance, baseAccessString);
                else if (NULL != (arrayProperty = ecProperty->GetAsArrayPropertyP()))
                    ixwStatus = WriteArrayPropertyValue(*arrayProperty, ecInstance, baseAccessString);
                else if (NULL != (structProperty = ecProperty->GetAsStructPropertyP()))
                    {
                    if (ecInstance.SaveOnlyLoadedPropertiesToXml())
                        {
                        // if the above flag is set then the instance sets "IsLoaded" flags for loaded properties and that "IsLoaded" flag is set in the ECValue for the property
                        Utf8String    accessString;
                        CreateAccessString(accessString, baseAccessString, structProperty->GetName());

                        // no members, don't write anything.
                        ECValue         ecValue;
                        if (ECObjectsStatus::Success != ecInstance.GetValue(ecValue, accessString.c_str()) || !ecValue.IsLoaded())
                            continue;
                        }

                    ICustomECStructSerializerP customECStructSerializerP;
                    if (NULL != (customECStructSerializerP = customStructSerializerMgr.GetCustomSerializer(structProperty, ecInstance)))
                        {
                        Utf8String     xmlString;
                        if (ECObjectsStatus::Success != customECStructSerializerP->GenerateXmlString(xmlString, structProperty, ecInstance, baseAccessString ? baseAccessString->c_str() : NULL))
                            ixwStatus = InstanceWriteStatus::BadPrimitivePropertyType;
                        else
                            {
                            // the tag of the element for an embedded struct is the property name.
                            m_xmlWriter->WriteElementStart(Utf8String(structProperty->GetName().c_str()).c_str());
                            m_xmlWriter->WriteText(xmlString.c_str());
                            ixwStatus = InstanceWriteStatus::Success;
                            m_xmlWriter->WriteElementEnd();
                            }
                        }
                    else
                        {
                        ixwStatus = WriteEmbeddedStructPropertyValue(*structProperty, ecInstance, baseAccessString);
                        }
                    }
                else if (nullptr != (navProperty = ecProperty->GetAsNavigationPropertyP()))
                    {
                    ixwStatus = WriteNavigationPropertyValue(*navProperty, ecInstance, baseAccessString);
                    }

                if (InstanceWriteStatus::Success != ixwStatus)
                    {
                    BeAssert(false);
                    return ixwStatus;
                    }
                }

            return InstanceWriteStatus::Success;
            }

        InstanceWriteStatus  WritePrimitiveArray(IECInstanceCR ecInstance, Utf8StringCR accessString, uint32_t nElements, PrimitiveType memberType)
            {
            ECValue         ecValue;
            InstanceWriteStatus     status;
            Utf8CP          typeString = SchemaParseUtils::PrimitiveTypeToString(memberType);
            for (uint32_t index = 0; index < nElements; index++)
                {
                if (ECObjectsStatus::Success != ecInstance.GetValue(ecValue, accessString.c_str(), index))
                    break;

                if (BEXML_Success != m_xmlWriter->WriteElementStart(typeString))
                    return InstanceWriteStatus::XmlWriteError;

                // write the primitive value
                if (InstanceWriteStatus::Success != (status = WritePrimitiveValue(ecValue, memberType)))
                    {
                    BeAssert(false);
                    return status;
                    }
                m_xmlWriter->WriteElementEnd();
                }
            return InstanceWriteStatus::Success;
            }

        //--------------------------------------------------------------------------------------
        // @bsimethod
        //---------------+---------------+---------------+---------------+---------------+------
        InstanceWriteStatus     WriteNavigationPropertyValue(NavigationECPropertyR navigationProperty, IECInstanceCR ecInstance, Utf8StringP baseAccessString)
            {
            ECObjectsStatus getStatus;
            ECValue         ecValue;
            Utf8StringCR    propertyName = navigationProperty.GetName();
            Utf8String      accessString;
            CreateAccessString(accessString, baseAccessString, propertyName);
            getStatus = ecInstance.GetValue(ecValue, accessString.c_str());

            // couldn't get, or NULL value, write nothing.
            if ((ECObjectsStatus::Success != getStatus) || ecValue.IsNull())
                return InstanceWriteStatus::Success;

            if (navigationProperty.IsMultiple())
                {
                // Not Supported yet
                return InstanceWriteStatus::XmlWriteError;
                }
            else
                {
                m_xmlWriter->WriteElementStart(propertyName.c_str());

                Utf8String typeString (ECINSTANCE_ID_ATTRIBUTE);
                typeString = typeString.append(":");
                typeString = typeString.append(SchemaParseUtils::PrimitiveTypeToString(navigationProperty.GetType()));

                if (BEXML_Success != m_xmlWriter->WriteElementStart(typeString.c_str()))
                    return InstanceWriteStatus::XmlWriteError;
                char outString[512];

                if (PrimitiveType::PRIMITIVETYPE_Long == navigationProperty.GetType())
                    BeStringUtilities::Snprintf(outString, "%lld", ecValue.GetNavigationInfo().GetId<BeInt64Id>().GetValueUnchecked());

                m_xmlWriter->WriteRaw(outString);
                m_xmlWriter->WriteElementEnd();

                // Write either the relationship class name or class id
                if (nullptr != ecValue.GetNavigationInfo().GetRelationshipClass())
                    {
                    if (BEXML_Success != m_xmlWriter->WriteElementStart(ECINSTANCE_RELATIONSHIPNAME_ATTRIBUTE))
                        return InstanceWriteStatus::XmlWriteError;
                    Utf8String className = ECClass::GetQualifiedClassName(ecValue.GetNavigationInfo().GetRelationshipClass()->GetSchema(), *ecValue.GetNavigationInfo().GetRelationshipClass());
                    m_xmlWriter->WriteRaw(className.c_str());
                    m_xmlWriter->WriteElementEnd(); // End of class name
                    }
                else if (ecValue.GetNavigationInfo().GetRelationshipClassId().IsValid())
                    {
                    if (BEXML_Success != m_xmlWriter->WriteElementStart(ECINSTANCE_RELATIONSHIPID_ATTTRIBUTE))
                        return InstanceWriteStatus::XmlWriteError;

                    char classId[512];
                    BeStringUtilities::Snprintf(classId, "%lld", ecValue.GetNavigationInfo().GetRelationshipClassId().GetValueUnchecked());
                    m_xmlWriter->WriteRaw(classId);
                    m_xmlWriter->WriteElementEnd(); // End of class id
                    }

                m_xmlWriter->WriteElementEnd(); // End of Nav Value
                }

            return InstanceWriteStatus::Success;
            }

        /*---------------------------------------------------------------------------------**//**
        * @bsimethod
        +---------------+---------------+---------------+---------------+---------------+------*/
        InstanceWriteStatus     WritePrimitivePropertyValue(PrimitiveECPropertyR primitiveProperty, IECInstanceCR ecInstance, Utf8String* baseAccessString)
            {
            ECObjectsStatus     getStatus;
            ECValue             ecValue;

            Utf8StringCR    propertyName = primitiveProperty.GetName();
            Utf8String      accessString;
            CreateAccessString(accessString, baseAccessString, propertyName);
            getStatus = ecInstance.GetValue(ecValue, accessString.c_str());

            // couldn't get, or NULL value, write nothing.
            if ((ECObjectsStatus::Success != getStatus) || ecValue.IsNull())
                return InstanceWriteStatus::Success;

            m_xmlWriter->WriteElementStart(propertyName.c_str());
            PrimitiveType           propertyType = primitiveProperty.GetType();

            InstanceWriteStatus status = WritePrimitiveValue(ecValue, propertyType);
            if (status != InstanceWriteStatus::Success)
                return status;
            m_xmlWriter->WriteElementEnd();
            return status;
            }

        /*---------------------------------------------------------------------------------**//**
        * @bsimethod
        +---------------+---------------+---------------+---------------+---------------+------*/
        InstanceWriteStatus     WritePrimitiveValue(ECValueCR ecValue, PrimitiveType propertyType)
            {
            char     outString[512];

            // write the content according to type.
            switch (propertyType)
                {
                    case PRIMITIVETYPE_IGeometry:
                    case PRIMITIVETYPE_Binary:
                    {
                    size_t      numBytes;
                    const Byte* byteData;
                    if (NULL != (byteData = ecValue.GetBinary(numBytes)))
                        {
                        Utf8String    byteString;
                        convertByteArrayToString(byteString, byteData, numBytes);
                        m_xmlWriter->WriteRaw(byteString.c_str());
                        }
                    return InstanceWriteStatus::Success;
                    break;
                    }

                    case PRIMITIVETYPE_Boolean:
                    {
                    strcpy(outString, ecValue.GetBoolean() ? "True" : "False");
                    break;
                    }

                    case PRIMITIVETYPE_DateTime:
                    {
                    BeStringUtilities::Snprintf(outString, "%lld", ecValue.GetDateTimeTicks());
                    break;
                    }

                    case PRIMITIVETYPE_Double:
                    {
                    BeStringUtilities::Snprintf(outString, "%.17g", ecValue.GetDouble());
                    break;
                    }

                    case PRIMITIVETYPE_Integer:
                    {
                    BeStringUtilities::Snprintf(outString, "%d", ecValue.GetInteger());
                    break;
                    }

                    case PRIMITIVETYPE_Long:
                    {
                    BeStringUtilities::Snprintf(outString, "%lld", ecValue.GetLong());
                    break;
                    }

                    case PRIMITIVETYPE_Point2d:
                    {
                    DPoint2d    point2d = ecValue.GetPoint2d();
                    BeStringUtilities::Snprintf(outString, "%.17g,%.17g", point2d.x, point2d.y);
                    break;
                    }

                    case PRIMITIVETYPE_Point3d:
                    {
                    DPoint3d    point3d = ecValue.GetPoint3d();
                    BeStringUtilities::Snprintf(outString, "%.17g,%.17g,%.17g", point3d.x, point3d.y, point3d.z);
                    break;
                    }

                    case PRIMITIVETYPE_String:
                    {
                    m_xmlWriter->WriteText(ecValue.GetUtf8CP());
                    return InstanceWriteStatus::Success;
                    }

                    default:
                    {
                    BeAssert(false);
                    return InstanceWriteStatus::BadPrimitivePropertyType;
                    }
                }

            m_xmlWriter->WriteRaw(outString);
            return InstanceWriteStatus::Success;
            }

        /*---------------------------------------------------------------------------------**//**
        * @bsimethod
        +---------------+---------------+---------------+---------------+---------------+------*/
#if defined (_MSC_VER)
#pragma warning(disable:4189) // memberClass unused if NDEBUG set.
#endif // defined (_MSC_VER)

        InstanceWriteStatus     WriteArrayPropertyValue(ArrayECPropertyR arrayProperty, IECInstanceCR ecInstance, Utf8String* baseAccessString)
            {
            ArrayKind       arrayKind = arrayProperty.GetKind();

            Utf8String    accessString;
            CreateAccessString(accessString, baseAccessString, arrayProperty.GetName());


            // no members, don't write anything.
            ECValue         ecValue;
            if (ECObjectsStatus::Success != ecInstance.GetValue(ecValue, accessString.c_str()) || ecValue.IsNull() || ecValue.GetArrayInfo().GetCount() == 0)
                return InstanceWriteStatus::Success;

            uint32_t nElements = ecValue.GetArrayInfo().GetCount();

            if (BEXML_Success != m_xmlWriter->WriteElementStart(arrayProperty.GetName().c_str()))
                return InstanceWriteStatus::XmlWriteError;

            if (ARRAYKIND_Primitive == arrayKind)
                {
                InstanceWriteStatus status = WritePrimitiveArray(ecInstance, accessString, nElements, arrayProperty.GetAsPrimitiveArrayProperty()->GetPrimitiveElementType());
                if (InstanceWriteStatus::Success != status)
                    return status;
                }
            else if (ARRAYKIND_Struct == arrayKind)
                {
                for (uint32_t index = 0; index < nElements; index++)
                    {
                    if (ECObjectsStatus::Success != ecInstance.GetValue(ecValue, accessString.c_str(), index))
                        break;

                    // the XML element tag is the struct type.
                    BeAssert(ecValue.IsStruct());

                    IECInstancePtr  structInstance = ecValue.GetStruct();
                    if (!structInstance.IsValid())
                        {
                        // ###TODO: It is valid to have null struct array instances....
                        BeAssert(false);
                        break;
                        }

                    ECClassCR   structClass = structInstance->GetClass();
                    BeAssert(structClass.Is(&arrayProperty.GetAsStructArrayProperty()->GetStructElementType()));

                    m_xmlWriter->WriteElementStart(Utf8String(structClass.GetName().c_str()).c_str());

                    InstanceWriteStatus iwxStatus;
                    if (InstanceWriteStatus::Success != (iwxStatus = WritePropertyValuesOfClassOrStructArrayMember(structClass, *structInstance.get(), NULL)))
                        {
                        BeAssert(false);
                        return iwxStatus;
                        }
                    m_xmlWriter->WriteElementEnd();
                    }
                }
            else
                {
                // unexpected arrayKind - should never happen.
                BeAssert(false);
                }
            m_xmlWriter->WriteElementEnd();
            return InstanceWriteStatus::Success;
            }

#if defined (_MSC_VER)
#pragma warning(default:4189)
#endif // defined (_MSC_VER)

        /*---------------------------------------------------------------------------------**//**
        * @bsimethod
        +---------------+---------------+---------------+---------------+---------------+------*/
        InstanceWriteStatus     WriteEmbeddedStructPropertyValue(StructECPropertyR structProperty, IECInstanceCR ecInstance, Utf8String* baseAccessString)
            {
            // the tag of the element for an embedded struct is the property name.
            m_xmlWriter->WriteElementStart(structProperty.GetName().c_str());

            Utf8String    thisAccessString;
            CreateAccessString(thisAccessString, baseAccessString, structProperty.GetName());
            thisAccessString.append(".");

            ECClassCR   structClass = structProperty.GetType();
            WritePropertyValuesOfClassOrStructArrayMember(structClass, ecInstance, &thisAccessString);

            m_xmlWriter->WriteElementEnd();
            return InstanceWriteStatus::Success;
            }

    };


/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
InstanceReadStatus   IECInstance::ReadFromXmlFile(IECInstancePtr& ecInstance, WCharCP ecInstanceFile, ECInstanceReadContextR context)
    {
    ecInstance = NULL;

    pugi::xml_document xmldoc;
    pugi::xml_parse_result status = xmldoc.load_file(ecInstanceFile);

    if (!status)
        {
        BeAssert(false);
        LOG.errorv("Failed to read ECInstance from XML File %ls: %s", ecInstanceFile, status.description());
        return InstanceReadStatus::XmlParseError;
        }
    return ReadFromBeXmlDom(ecInstance, xmldoc, context);
    }

//--------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
InstanceReadStatus   IECInstance::ReadFromXmlString(IECInstancePtr& ecInstance, Utf8CP ecInstanceXml, ECInstanceReadContextR context)
    {
    ecInstance = NULL;
    
    pugi::xml_document xmldoc;
    pugi::xml_parse_result status = xmldoc.load_string(ecInstanceXml);

    if (!status)
        {
        BeAssert(false);
        LOG.errorv("Failed to read ECInstance from XML %s", status.description());
        return InstanceReadStatus::XmlParseError;
        }
    return ReadFromBeXmlDom(ecInstance, xmldoc, context);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
InstanceReadStatus   IECInstance::ReadFromXmlString(IECInstancePtr& ecInstance, WCharCP ecInstanceXml, ECInstanceReadContextR context)
    {
    ecInstance = NULL;

    pugi::xml_document xmldoc;
    Utf8String instanceXmlUtf8;
    if(BeStringUtilities::WCharToUtf8(instanceXmlUtf8, ecInstanceXml) != BentleyStatus::SUCCESS)
        {
        BeAssert(false);
        LOG.errorv("Failed to convert instance xml from wchar to char (%ls)", ecInstanceXml);
        return InstanceReadStatus::XmlParseError;
        }
    pugi::xml_parse_result status = xmldoc.load_string(instanceXmlUtf8.c_str());

    if (!status)
        {
        BeAssert(false);
        LOG.errorv("Failed to read ECInstance from XML %s", status.description());
        return InstanceReadStatus::XmlParseError;
        }

    return ReadFromBeXmlDom(ecInstance, xmldoc, context);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
InstanceReadStatus  IECInstance::ReadFromBeXmlDom(IECInstancePtr& ecInstance, pugi::xml_document& xmlDom, ECInstanceReadContextR context)
    {
    ecInstance = NULL;

    pugi::xml_node      instanceNode = xmlDom.document_element();
    if (!instanceNode)
        {
        BeAssert(false);
        LOG.errorv("Invalid ECInstanceXML: Missing a top-level instance node");
        return InstanceReadStatus::BadElement;
        }

    return ReadFromBeXmlNode(ecInstance, instanceNode, context);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
InstanceReadStatus   IECInstance::ReadFromBeXmlNode(IECInstancePtr& ecInstance, pugi::xml_node instanceNode, ECInstanceReadContextR context)
    {
    InstanceXmlReader   reader(context, instanceNode);
    return reader.ReadInstance(ecInstance);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
InstanceWriteStatus     IECInstance::WriteToXmlFile(WCharCP fileName, bool writeInstanceId, bool utf16)
    {
    BeXmlWriterPtr xmlWriter = BeXmlWriter::CreateFileWriter(fileName);
    InstanceXmlWriter   instanceWriter(xmlWriter.get());

    xmlWriter->WriteDocumentStart(XML_CHAR_ENCODING_UTF8);
    return instanceWriter.WriteInstance(*this, writeInstanceId);
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
template<typename T_STR> InstanceWriteStatus writeInstanceToXmlString(T_STR& ecInstanceXml, bool isStandAlone, bool writeInstanceId, IECInstanceR instance, bool useLatestXml)
    {
    ecInstanceXml.clear();
    BeXmlWriterPtr xmlWriter = BeXmlWriter::Create();
    InstanceXmlWriter   instanceWriter(xmlWriter.get());
    if (isStandAlone)
        xmlWriter->WriteDocumentStart(XML_CHAR_ENCODING_UTF8);

    InstanceWriteStatus status;
    if (useLatestXml)
        {
        if (InstanceWriteStatus::Success != (status = instanceWriter.WriteInstanceLatestVersion(instance, writeInstanceId)))
            return status;
        }
    else if (InstanceWriteStatus::Success != (status = instanceWriter.WriteInstance(instance, writeInstanceId)))
        return status;

    xmlWriter->ToString(ecInstanceXml);
    return InstanceWriteStatus::Success;
    }

//-------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
InstanceWriteStatus     IECInstance::WriteToXmlString(Utf8String & ecInstanceXml, bool isStandAlone, bool writeInstanceId)
    {
    return writeInstanceToXmlString(ecInstanceXml, isStandAlone, writeInstanceId, *this, false);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
InstanceWriteStatus     IECInstance::WriteToXmlString(WString & ecInstanceXml, bool isStandAlone, bool writeInstanceId)
    {
    return writeInstanceToXmlString(ecInstanceXml, isStandAlone, writeInstanceId, *this, false);
    }

//-------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
InstanceWriteStatus     IECInstance::WriteToXmlStringLatestVersion(Utf8String & ecInstanceXml, bool isStandAlone, bool writeInstanceId)
    {
    return writeInstanceToXmlString(ecInstanceXml, isStandAlone, writeInstanceId, *this, true);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
InstanceWriteStatus     IECInstance::WriteToXmlStringLatestVersion(WString & ecInstanceXml, bool isStandAlone, bool writeInstanceId)
    {
    return writeInstanceToXmlString(ecInstanceXml, isStandAlone, writeInstanceId, *this, true);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
InstanceWriteStatus     IECInstance::WriteToBeXmlNode(BeXmlWriterR xmlWriter)
    {
    Utf8CP className = this->GetClass().GetName().c_str();

    return WriteToBeXmlNode(xmlWriter, className);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
InstanceWriteStatus     IECInstance::WriteToBeXmlNode(BeXmlWriterR xmlWriter, Utf8CP className)
    {
    InstanceXmlWriter instanceWriter(&xmlWriter);

    return instanceWriter.WriteInstance(*this, false, className);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
InstanceWriteStatus     IECInstance::WriteToBeXmlNodeLatestVersion(BeXmlWriterR xmlWriter, Utf8CP className)
    {
    InstanceXmlWriter instanceWriter(&xmlWriter);

    return instanceWriter.WriteInstanceLatestVersion(*this, false, className);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
InstanceWriteStatus IECInstance::WriteToBeXmlDom(BeXmlWriterR xmlWriter, bool writeInstanceId)
    {
    InstanceXmlWriter writer(&xmlWriter);
    return writer.WriteInstance(*this, writeInstanceId);
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool IECInstance::SaveOnlyLoadedPropertiesToXml() const
    {
    return _SaveOnlyLoadedPropertiesToXml();
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ECSchemaCP ECInstanceReadContext::FindSchemaCP(SchemaKeyCR key, SchemaMatchType matchType) const
    {
    ECSchemaCP schema = _FindSchemaCP(key, matchType);
    if (NULL != schema)
        return schema;

    return &m_fallBackSchema;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool IECSchemaRemapper::ResolvePropertyName(Utf8StringR name, ECClassCR ecClass) const { return _ResolvePropertyName(name, ecClass); }
bool IECSchemaRemapper::ResolveClassName(Utf8StringR name, ECSchemaCR schema) const { return _ResolveClassName(name, schema); }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus IECInstanceInterface::GetInstanceValue(ECValueR v, Utf8CP accessor) const { return _GetInstanceValue(v, accessor); }
ECClassCP IECInstanceInterface::GetInstanceClass() const { return _GetInstanceClass(); }
IECInstanceCP IECInstanceInterface::ObtainECInstance() const { return _ObtainECInstance(); }
Utf8String IECInstanceInterface::GetInstanceId() const { return _GetInstanceId(); }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus ECInstanceInterface::_GetInstanceValue(ECValueR v, Utf8CP accessor) const
    {
    return ECInstanceInteropHelper::GetValue(m_instance, v, accessor);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ECClassCP ECInstanceInterface::_GetInstanceClass() const
    {
    return &m_instance.GetClass();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
IECInstanceCP ECInstanceInterface::_ObtainECInstance() const
    {
    return &m_instance;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String ECInstanceInterface::_GetInstanceId() const
    {
    return m_instance.GetInstanceId();
    }

END_BENTLEY_ECOBJECT_NAMESPACE
