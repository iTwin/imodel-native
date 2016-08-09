/*--------------------------------------------------------------------------------------+
|
|     $Source: src/ECInstance.cpp $
|
|   $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ECObjectsPch.h"
#include <ECObjects/ECSchema.h>
#include <GeomSerialization/GeomSerializationApi.h>

DEFINE_KEY_METHOD(ECN::IECRelationshipInstance)

BEGIN_BENTLEY_ECOBJECT_NAMESPACE

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Bill.Steinbock                  09/2010
+---------------+---------------+---------------+---------------+---------------+------*/
CustomStructSerializerManager::CustomStructSerializerManager ()
    {
    // we could add needed serializers here.
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Bill.Steinbock                  09/2010
+---------------+---------------+---------------+---------------+---------------+------*/
CustomStructSerializerManager::~CustomStructSerializerManager ()
    {
    m_serializers.clear();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Bill.Steinbock                  09/2010
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus            CustomStructSerializerManager::AddCustomSerializer (Utf8CP serializerName, ICustomECStructSerializerP serializer)
    {
    if (GetCustomSerializer (serializerName))
        return ERROR;

    m_serializers[Utf8String(serializerName)] = serializer;
    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Bill.Steinbock                  09/2010
+---------------+---------------+---------------+---------------+---------------+------*/
CustomStructSerializerManagerR                   CustomStructSerializerManager::GetManager()
    {
    static CustomStructSerializerManagerP   s_serializerManager = NULL;

    if (NULL == s_serializerManager)
        s_serializerManager = new CustomStructSerializerManager();
        
    return *s_serializerManager;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Bill.Steinbock                  09/2010
+---------------+---------------+---------------+---------------+---------------+------*/
ICustomECStructSerializerP                      CustomStructSerializerManager::GetCustomSerializer (Utf8CP serializerName) const
    {
    if (m_serializers.empty())
        return NULL;

    NameSerializerMap::const_iterator it = m_serializers.find (serializerName);
    if (it == m_serializers.end())
        return NULL;

    return it->second;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Bill.Steinbock                  09/2010
+---------------+---------------+---------------+---------------+---------------+------*/
ICustomECStructSerializerP                      CustomStructSerializerManager::GetCustomSerializer (StructECPropertyP structProperty, IECInstanceCR ecInstance) const
    {
    if (m_serializers.empty())
        return NULL;

    // see if the struct has a custom attribute to custom serialize itself
    IECInstancePtr caInstance = structProperty->GetType().GetCustomAttribute("Bentley_Standard_CustomAttributes", "CustomStructSerializer");
    if (caInstance.IsValid())
        {
        ECValue value;
        if (ECObjectsStatus::Success == caInstance->GetValue (value, "SerializerName"))
            {
            ICustomECStructSerializerP serializerP = GetCustomSerializer (value.GetUtf8CP());
            if (serializerP)
                {
                // let the serializer decide if it wants to process the struct from this type of IECInstance
                if (serializerP->UsesCustomStructXmlString  (structProperty, ecInstance))
                    return serializerP;
                }
            }
        }

    return NULL;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Bill.Steinbock                  11/2010
+---------------+---------------+---------------+---------------+---------------+------*/
bool IECInstance::IsFixedArrayProperty (ECN::IECInstanceR instance, Utf8CP accessString, uint32_t* numFixedEntries)
    {
    ECValue         arrayVal;

    if (ECObjectsStatus::Success != instance.GetValue (arrayVal, accessString))
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
* @bsimethod                                                    CaseyMullen     09/09
+---------------+---------------+---------------+---------------+---------------+------*/
IECInstance::IECInstance()
    {
    BeAssert (sizeof(IECInstance) == sizeof (RefCountedBase) && "Increasing the size or memory layout of the base ECN::IECInstance will adversely affect subclasses. Think of this as a pure interface... to which you would never be able to add (additional) data, either");
    };    

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    CaseyMullen     09/09
+---------------+---------------+---------------+---------------+---------------+------*/
IECInstance::~IECInstance()
    {
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    CaseyMullen    02/10
+---------------+---------------+---------------+---------------+---------------+------*/
bool IsExcluded(Utf8String& className, bvector<Utf8String>& classNamesToExclude)
    {
    for (Utf8String excludedClass: classNamesToExclude)
        {
        if (0 == className.compare (excludedClass))
            return true;
        }

    return false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Carole.MacDonald                05/2012
+---------------+---------------+---------------+---------------+---------------+------*/
IECInstancePtr IECInstance::CreateCopyThroughSerialization()
    {
    Utf8String ecInstanceXml;
    this->WriteToXmlString(ecInstanceXml, true, false);

    ECInstanceReadContextPtr instanceContext = ECInstanceReadContext::CreateContext (GetClass().GetSchema());

    IECInstancePtr deserializedInstance;
    IECInstance::ReadFromXmlString(deserializedInstance, ecInstanceXml.c_str(), *instanceContext);

    return deserializedInstance;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    05/11
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus        IECInstance::_SetInstanceId (Utf8CP id)    { return ECObjectsStatus::OperationNotSupported; }
ECObjectsStatus        IECInstance::SetInstanceId (Utf8CP id)     { return _SetInstanceId(id); }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    CaseyMullen     09/09
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String        IECInstance::GetInstanceId() const
    {
    return _GetInstanceId();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   03/15
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String     IECInstance::GetInstanceIdForSerialization() const
    {
    return _GetInstanceIdForSerialization();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    CaseyMullen     09/09
+---------------+---------------+---------------+---------------+---------------+------*/
ECClassCR       IECInstance::GetClass() const 
    {
    ECEnablerCR enabler = GetEnabler();
        
    return enabler.GetClass();
    }
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Bill.Steinbock                  12/2010
+---------------+---------------+---------------+---------------+---------------+------*/
MemoryECInstanceBase* IECInstance::_GetAsMemoryECInstance () const
    {
    return NULL;    // default to NULL and let real MemoryECInstanceBased classes override this method.
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   11/12
+---------------+---------------+---------------+---------------+---------------+------*/
ECDBuffer* IECInstance::_GetECDBuffer() const
    {
    return NULL;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Bill.Steinbock                  12/2010
+---------------+---------------+---------------+---------------+---------------+------*/
size_t IECInstance::GetOffsetToIECInstance () const
    {
    return _GetOffsetToIECInstance();  
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Dylan.Rush      08/2011
+---------------+---------------+---------------+---------------+---------------+------*/ 
bool                IECInstance::_IsPropertyReadOnly (Utf8CP accessString) const
    {
/*
    if (_IsReadOnly())
        return true;
*/

    uint32_t propertyIndex;
    return ECObjectsStatus::Success != GetEnabler().GetPropertyIndex (propertyIndex, accessString) || IsPropertyReadOnly (propertyIndex);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Dylan.Rush      08/2011
+---------------+---------------+---------------+---------------+---------------+------*/ 
bool                IECInstance::_IsPropertyReadOnly (uint32_t propertyIndex) const
    {
    return /*_IsReadOnly() || */ GetEnabler().IsPropertyReadOnly (propertyIndex);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    CaseyMullen     09/09
+---------------+---------------+---------------+---------------+---------------+------*/   
ECEnablerCR           IECInstance::GetEnabler() const { return _GetEnabler();  }
ECEnablerR            IECInstance::GetEnablerR() const { return *const_cast<ECEnablerP>(&_GetEnabler());  }
bool                  IECInstance::IsReadOnly() const { return _IsReadOnly();  }
ECDBuffer const*            IECInstance::GetECDBuffer() const { return _GetECDBuffer(); }
ECDBuffer*                  IECInstance::GetECDBufferP() { return _GetECDBuffer(); }
MemoryECInstanceBase const* IECInstance::GetAsMemoryECInstance () const {return _GetAsMemoryECInstance();}
MemoryECInstanceBase*       IECInstance::GetAsMemoryECInstanceP () { return _GetAsMemoryECInstance(); }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    CaseyMullen     09/09
+---------------+---------------+---------------+---------------+---------------+------*/   
ECObjectsStatus     IECInstance::GetValue (ECValueR v, Utf8CP propertyAccessString) const 
    {
    uint32_t propertyIndex=0;
    ECObjectsStatus status = GetEnabler().GetPropertyIndex (propertyIndex, propertyAccessString);

    if (ECObjectsStatus::Success != status)
        return status;

    return GetValue (v, propertyIndex, false, 0); }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   10/14
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus     IECInstance::GetValueOrAdhoc (ECValueR v, Utf8CP accessString) const
    {
    auto status = GetValue (v, accessString);
    if (ECObjectsStatus::PropertyNotFound == status)
        {
        for (auto const& containerIndex : AdhocContainerPropertyIndexCollection (GetEnabler()))
            {
            AdhocPropertyQuery adhocs (*this, containerIndex);
            uint32_t propertyIndex;
            if (adhocs.GetPropertyIndex (propertyIndex, accessString))
                status = adhocs.GetValue (v, propertyIndex);
            }
        }

    return status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    CaseyMullen     09/09
+---------------+---------------+---------------+---------------+---------------+------*/   
ECObjectsStatus     IECInstance::GetValue (ECValueR v, Utf8CP propertyAccessString, uint32_t arrayIndex) const 
    {
    uint32_t propertyIndex=0;
    ECObjectsStatus status = GetEnabler().GetPropertyIndex (propertyIndex, propertyAccessString);

    if (ECObjectsStatus::Success != status)
        return status;

    return GetValue (v, propertyIndex, true, arrayIndex); 
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    CaseyMullen     09/09
+---------------+---------------+---------------+---------------+---------------+------*/   
ECObjectsStatus     IECInstance::GetValue (ECValueR v, uint32_t propertyIndex) const 
    { 
    return GetValue (v, propertyIndex, false, 0); 
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    CaseyMullen     09/09
+---------------+---------------+---------------+---------------+---------------+------*/   
ECObjectsStatus     IECInstance::GetValue (ECValueR v, uint32_t propertyIndex, uint32_t arrayIndex) const
    {
    return GetValue (v, propertyIndex, true, arrayIndex); 
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Krischan.Eberle    02/13
//+---------------+---------------+---------------+---------------+---------------+------
ECObjectsStatus     IECInstance::GetValue (ECValueR v, uint32_t propertyIndex, bool useArrayIndex, uint32_t arrayIndex) const 
    {
    ECObjectsStatus stat = _GetValue (v, propertyIndex, useArrayIndex, arrayIndex); 
    if (stat != ECObjectsStatus::Success)
        {
        return stat;
        }

    if (!v.IsDateTime ())
        {
        return stat;
        }

    return SetDateTimeMetadataInECValue (v, propertyIndex);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    CaseyMullen     09/09
+---------------+---------------+---------------+---------------+---------------+------*/   
ECObjectsStatus     IECInstance::SetInternalValue (Utf8CP propertyAccessString, ECValueCR v) 
    {
    uint32_t propertyIndex=0;
    ECObjectsStatus status = GetEnabler().GetPropertyIndex (propertyIndex, propertyAccessString);

    if (ECObjectsStatus::Success != status)
        return status;

    return SetInternalValue (propertyIndex, v); 
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    CaseyMullen     09/09
+---------------+---------------+---------------+---------------+---------------+------*/   
ECObjectsStatus     IECInstance::ChangeValue (Utf8CP propertyAccessString, ECValueCR v) 
    {
    uint32_t propertyIndex=0;
    ECObjectsStatus status = GetEnabler().GetPropertyIndex (propertyIndex, propertyAccessString);

    if (ECObjectsStatus::Success != status)
        return status;

    return ChangeValue (propertyIndex, v);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   10/14
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus IECInstance::ChangeValueOrAdhoc (Utf8CP propertyAccessString, ECValueCR v)
    {
    auto status = ChangeValue (propertyAccessString, v);
    if (ECObjectsStatus::PropertyNotFound == status)
        {
        for (auto const& containerIndex : AdhocContainerPropertyIndexCollection (GetEnabler()))
            {
            AdhocPropertyEdit adhocs (*this, containerIndex);
            uint32_t propertyIndex;
            if (adhocs.GetPropertyIndex (propertyIndex, propertyAccessString))
                {
                bool isReadOnly = true;
                status = adhocs.IsReadOnly (isReadOnly, propertyIndex);
                if (ECObjectsStatus::Success == status)
                    {
                    if (isReadOnly)
                        status = ECObjectsStatus::UnableToSetReadOnlyProperty;
                    else
                        {
                        ECValue curV;
                        if (ECObjectsStatus::Success == adhocs.GetValue (curV, propertyIndex) && curV.Equals (v))
                            status = ECObjectsStatus::PropertyValueMatchesNoChange;
                        else
                            status = adhocs.SetValue (propertyIndex, v);
                        }
                    }
                }
            }
        }

    return status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Bill.Steinbock                  11/2012
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus     IECInstance::SetValue (Utf8CP propertyAccessString, ECValueCR v) 
    {
    ECObjectsStatus status = ChangeValue (propertyAccessString, v);

    if (ECObjectsStatus::PropertyValueMatchesNoChange == status)
        return ECObjectsStatus::Success;

    return status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   10/14
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus IECInstance::SetValueOrAdhoc (Utf8CP propertyAccessString, ECValueCR v)
    {
    auto status = ChangeValueOrAdhoc (propertyAccessString, v);
    if (ECObjectsStatus::PropertyValueMatchesNoChange == status)
        return ECObjectsStatus::Success;

    return status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    CaseyMullen     09/09
+---------------+---------------+---------------+---------------+---------------+------*/   
ECObjectsStatus     IECInstance::SetInternalValue (Utf8CP propertyAccessString, ECValueCR v, uint32_t arrayIndex) 
    {
    uint32_t propertyIndex=0;
    ECObjectsStatus status = GetEnabler().GetPropertyIndex (propertyIndex, propertyAccessString);

    if (ECObjectsStatus::Success != status)
        return status;

    return SetInternalValue (propertyIndex, v, arrayIndex); 
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    CaseyMullen     09/09
+---------------+---------------+---------------+---------------+---------------+------*/   
ECObjectsStatus     IECInstance::ChangeValue (Utf8CP propertyAccessString, ECValueCR v, uint32_t arrayIndex) 
    {
    uint32_t propertyIndex=0;
    ECObjectsStatus status = GetEnabler().GetPropertyIndex (propertyIndex, propertyAccessString);

    if (ECObjectsStatus::Success != status)
        return status;

    return ChangeValue (propertyIndex, v, arrayIndex); 
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    CaseyMullen     09/09
+---------------+---------------+---------------+---------------+---------------+------*/   
ECObjectsStatus     IECInstance::SetValue (Utf8CP propertyAccessString, ECValueCR v, uint32_t arrayIndex) 
    {
    ECObjectsStatus status = ChangeValue (propertyAccessString, v, arrayIndex);

    if (ECObjectsStatus::PropertyValueMatchesNoChange == status)
        return ECObjectsStatus::Success;

    return status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    CaseyMullen     09/09
+---------------+---------------+---------------+---------------+---------------+------*/   
ECObjectsStatus     IECInstance::SetInternalValue (uint32_t propertyIndex, ECValueCR v) 
    {
    auto status = _SetInternalValue (propertyIndex, v, false, 0);

    if (ECObjectsStatus::PropertyValueMatchesNoChange == status)
        status = ECObjectsStatus::Success;

    return status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    CaseyMullen     09/09
+---------------+---------------+---------------+---------------+---------------+------*/   
ECObjectsStatus     IECInstance::SetValue (uint32_t propertyIndex, ECValueCR v) 
    {
    ECObjectsStatus status = ChangeValue (propertyIndex, v);

    if (ECObjectsStatus::PropertyValueMatchesNoChange == status)
        return ECObjectsStatus::Success;

    return  status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Bill.Steinbock                  11/2012
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus     IECInstance::SetInternalValue (uint32_t propertyIndex, ECValueCR v, uint32_t arrayIndex) 
    { 

    return _SetInternalValue (propertyIndex, v, true, arrayIndex); 
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   09/12
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus IECInstance::_SetInternalValue (uint32_t propertyIndex, ECValueCR v, bool useArrayIndex, uint32_t arrayIndex)
    {
    // Default impl; instances that support calculated properties should override
    return _SetValue (propertyIndex, v, useArrayIndex, arrayIndex);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Bill.Steinbock                  11/2012
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus     IECInstance::ChangeValue (uint32_t propertyIndex, ECValueCR v, uint32_t arrayIndex) 
    {
    return ChangeValue (propertyIndex, v, true, arrayIndex);
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                                    CaseyMullen     09/09
//+---------------+---------------+---------------+---------------+---------------+-----
ECObjectsStatus     IECInstance::ChangeValue (uint32_t propertyIndex, ECValueCR v) 
    {
    return ChangeValue (propertyIndex, v, false, 0);
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                 02/2013
//+---------------+---------------+---------------+---------------+---------------+-----
ECObjectsStatus     IECInstance::ChangeValue (uint32_t propertyIndex, ECValueCR v, bool useArrayIndex, uint32_t arrayIndex) 
    {
    if ( ! ChangeValuesAllowed())
        return ECObjectsStatus::UnableToSetReadOnlyInstance;

    bool isNull = false;
    ECObjectsStatus status = GetIsPropertyNull (isNull, propertyIndex, useArrayIndex, arrayIndex);
    if (status != ECObjectsStatus::Success)
        return status;

    if (IsPropertyReadOnly (propertyIndex) && !isNull)
        return ECObjectsStatus::UnableToSetReadOnlyProperty;

    if (v.IsDateTime ())
        {
        status = ValidateDateTimeMetadata (propertyIndex, v);
        if (status != ECObjectsStatus::Success)
            {
            return status;
            }
        }

    return _SetValue (propertyIndex, v, useArrayIndex, arrayIndex); 
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Bill.Steinbock                  11/2012
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus     IECInstance::SetValue (uint32_t propertyIndex, ECValueCR v, uint32_t arrayIndex) 
    {
    ECObjectsStatus status = ChangeValue (propertyIndex, v, arrayIndex);

    if (ECObjectsStatus::PropertyValueMatchesNoChange == status)
        return ECObjectsStatus::Success;

    return  status;
    }

bool                IECInstance::IsPropertyReadOnly (uint32_t propertyIndex) const { return _IsPropertyReadOnly (propertyIndex); }
bool                IECInstance::IsPropertyReadOnly (Utf8CP accessString) const { return _IsPropertyReadOnly (accessString); }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   10/14
+---------------+---------------+---------------+---------------+---------------+------*/
bool IECInstance::IsPropertyOrAdhocReadOnly (Utf8CP accessString) const
    {
    uint32_t propertyIndex;
    auto status = GetEnabler().GetPropertyIndex (propertyIndex, accessString);
    if (ECObjectsStatus::PropertyNotFound == status)
        {
        bool readOnly = true;
        for (auto const& containerIndex : AdhocContainerPropertyIndexCollection (GetEnabler()))
            {
            AdhocPropertyQuery adhocs (*this, containerIndex);
            if (adhocs.GetPropertyIndex (propertyIndex, accessString))
                {
                status = adhocs.IsReadOnly (readOnly, propertyIndex);
                break;
                }
            }

        return ECObjectsStatus::Success != status || readOnly;
        }

    return ECObjectsStatus::Success != status || IsPropertyReadOnly (propertyIndex);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   10/14
+---------------+---------------+---------------+---------------+---------------+------*/

#define NUM_INDEX_BUFFER_CHARS 63
#define NUM_ACCESSSTRING_BUFFER_CHARS 1023

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Dylan.Rush                      1/11
+---------------+---------------+---------------+---------------+---------------+------*/
static ECPropertyP getProperty  (ECClassCR ecClass, Utf8CP accessor, Utf8Char* buffer)
    {
    //Gets the ECProperty for a full native accessor.
    //For example, the full native accessor could be "GrandfatherStruct.ParentStruct.StringMember"
    //In this case, passing this accessor to this function will give you the
    //ECProperty for StringMember.
   
    Utf8CP dotPos = strchr (accessor, '.');
    if (NULL != dotPos)
        {
        size_t dotIndex  = dotPos - accessor;
        buffer[dotIndex] = '\0';

        ECPropertyP prop = ecClass.GetPropertyP (buffer);

        if (NULL == prop)
            return NULL;

        StructECPropertyP structProperty = prop->GetAsStructPropertyP();

        if (NULL == structProperty)
            return NULL;

        return getProperty (structProperty->GetType(), &dotPos[1], &buffer[dotIndex+1]);
        }

    Utf8CP bracketPos = strchr (accessor, '[');
    if (NULL != bracketPos)
        {
        size_t bracketIndex = bracketPos - accessor;
        buffer[bracketIndex] = '\0';
        }
    return ecClass.GetPropertyP (buffer);
    }
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Bill.Steinbock                  10/2010
+---------------+---------------+---------------+---------------+---------------+------*/
static ECObjectsStatus getECValueUsingFullAccessString (Utf8P asBuffer, Utf8P indexBuffer, ECValueR v, IECInstanceCR instance, Utf8CP managedPropertyAccessor)
    {
    // see if access string specifies an array
    Utf8CP pos1 = strchr (managedPropertyAccessor, '[');

    // if not an array then 
    if (NULL == pos1)
        return instance.GetValue (v, managedPropertyAccessor);

    size_t numChars = 0;
    numChars = pos1 - managedPropertyAccessor;
    strncpy(asBuffer, managedPropertyAccessor, numChars>NUM_ACCESSSTRING_BUFFER_CHARS?NUM_ACCESSSTRING_BUFFER_CHARS:numChars);
    asBuffer[numChars]=0;

    // BRACKETS_OKAY: Brackets contain an array index
    Utf8CP pos2 = strchr (pos1+1, ']');

    BeAssert (pos2 != NULL);

    numChars = pos2 - pos1 - 1;

    strncpy(indexBuffer, pos1+1, numChars>NUM_INDEX_BUFFER_CHARS?NUM_INDEX_BUFFER_CHARS:numChars);
    indexBuffer[numChars]=0;

    uint32_t indexValue = -1;
    BE_STRING_UTILITIES_UTF8_SSCANF(indexBuffer, "%ud", &indexValue);

    ECValue         arrayVal;
    ECObjectsStatus status;

    if (ECObjectsStatus::Success != (status = instance.GetValue (arrayVal, asBuffer)))
        return status;

    if (-1 == indexValue)
        {
        //Caller asked for the array itself, not any particular element.
        //Returns a dummy ECValue with only the array info copied.
        ArrayInfo info = arrayVal.GetArrayInfo();
        if(info.IsStructArray())
            v.SetStructArrayInfo(info.GetCount(), info.IsFixedCount());
        else
            v.SetPrimitiveArrayInfo (info.GetElementPrimitiveType(), info.GetCount(), info.IsFixedCount());
        return ECObjectsStatus::Success;
        }

    ArrayInfo arrayInfo = arrayVal.GetArrayInfo();
    uint32_t  size      = arrayInfo.GetCount();

    if (indexValue >= size)
        return ECObjectsStatus::Error;

    if (arrayInfo.IsPrimitiveArray())
        return instance.GetValue (v, asBuffer, indexValue);

    // must be a struct array
    
    if (ECObjectsStatus::Success != (status = instance.GetValue (arrayVal, asBuffer, indexValue)))
        return status;

    // If there is no '.' in the rest of the access string, the caller was requesting the value representing the struct
    // array element itself, not the value of any of its members.
    if ('\0' == pos2[1])
        {
        v.SetStruct(arrayVal.GetStruct().get());
        return ECObjectsStatus::Success;
        }

    IECInstancePtr arrayEntryInstance = arrayVal.GetStruct();

    return getECValueUsingFullAccessString (asBuffer, indexBuffer, v, *arrayEntryInstance, pos2+2); // move to character after "]." in access string.
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Bill.Steinbock                  10/2010
+---------------+---------------+---------------+---------------+---------------+------*/
static ECObjectsStatus getECValueFromInstance (ECValueR v, IECInstanceCR instance, Utf8CP managedPropertyAccessor)
    {
    WString asBufferStr;

    v.Clear();
    Utf8Char asBuffer[NUM_ACCESSSTRING_BUFFER_CHARS+1];
    Utf8Char indexBuffer[NUM_INDEX_BUFFER_CHARS+1];

    return getECValueUsingFullAccessString (asBuffer, indexBuffer, v, instance, managedPropertyAccessor);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Dylan.Rush      11/10
+---------------+---------------+---------------+---------------+---------------+------*/ 
static ECObjectsStatus          getValueHelper (ECValueR value, IECInstanceCR instance, ECValueAccessorCR accessor, uint32_t depth, bool compatible)
    {
    ECValueAccessor::Location const& loc = accessor[depth];
    int arrayIndex = loc.GetArrayIndex();
    if (compatible)
        {
        uint32_t propertyIndex = (uint32_t)loc.GetPropertyIndex();
        if (arrayIndex < 0)
            return instance.GetValue (value, propertyIndex);
        return instance.GetValue (value, propertyIndex,  (uint32_t)arrayIndex);
        }

    Utf8CP accessString = accessor.GetAccessString (depth);
    if (NULL == accessString)
        return ECObjectsStatus::Error;

    if (arrayIndex < 0)
        return instance.GetValue (value, accessString);

    return instance.GetValue (value, accessString, (uint32_t)arrayIndex);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Dylan.Rush      11/10
+---------------+---------------+---------------+---------------+---------------+------*/ 
static ECObjectsStatus          setValueHelper (IECInstanceR instance, ECValueAccessorCR accessor, uint32_t depth, bool compatible, ECValueCR value)
    {
    int arrayIndex = accessor[depth].GetArrayIndex();
    if (compatible)
        {
        uint32_t propertyIndex = (uint32_t)accessor[depth].GetPropertyIndex();

        if(arrayIndex < 0)
            {
            return instance.SetValue(propertyIndex, value);
            }
        return instance.SetValue (propertyIndex, value, (uint32_t)arrayIndex);
        }

    // not the same enabler between accessor and instance so use access string to set value
    Utf8CP accessString = accessor.GetAccessString(depth);
    if (NULL == accessString)
        return ECObjectsStatus::Error;

    if (arrayIndex < 0)
        return instance.SetValue (accessString, value);

    return instance.SetValue (accessString, value, (uint32_t)arrayIndex);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Dylan.Rush      11/10
+---------------+---------------+---------------+---------------+---------------+------*/ 
static ECObjectsStatus          setInternalValueHelper (IECInstanceR instance, ECValueAccessorCR accessor, uint32_t depth, bool compatible, ECValueCR value)
    {
    int arrayIndex = accessor[depth].GetArrayIndex();
    if (compatible)
        {
        uint32_t propertyIndex = (uint32_t)accessor[depth].GetPropertyIndex();

        if(arrayIndex < 0)
            {
            return instance.SetInternalValue(propertyIndex, value);
            }

        return instance.SetInternalValue (propertyIndex, value, (uint32_t)arrayIndex);
        }

    // not the same enabler between accessor and instance so use access string to set value
    Utf8CP accessString = accessor.GetAccessString(depth);
    if (NULL == accessString)
        return ECObjectsStatus::Error;

    if (arrayIndex < 0)
        return instance.SetInternalValue (accessString, value);

    return instance.SetInternalValue (accessString, value, (uint32_t)arrayIndex);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Dylan.Rush      11/10
+---------------+---------------+---------------+---------------+---------------+------*/ 
ECObjectsStatus           IECInstance::GetValueUsingAccessor (ECValueR v, ECValueAccessorCR accessor) const
    {
    if (accessor.IsAdhocProperty())
        {
        // The array index is already pointing to the index of the desired ad-hoc property
        if (1 != accessor.GetDepth() || ECValueAccessor::INDEX_ROOT == accessor[0].GetArrayIndex())
            {
            BeAssert (false);
            return ECObjectsStatus::Error;
            }

        AdhocPropertyQuery adhoc (*this, accessor[0].GetPropertyIndex());
        return adhoc.GetValue (v, accessor[0].GetArrayIndex());
        }

    ECObjectsStatus status            = ECObjectsStatus::Success;
    IECInstancePtr  currentInstance   = const_cast <IECInstance*> (this);
    for (uint32_t depth = 0; depth < accessor.GetDepth(); depth ++)
        {
        v.Clear();
        bool compatible = (accessor[depth].GetEnabler() == &currentInstance->GetEnabler()); // if same enabler then use property index to set value else use access string

        status = getValueHelper (v, *currentInstance, accessor, depth, compatible);
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
* @bsimethod                                                    Dylan.Rush      11/10
+---------------+---------------+---------------+---------------+---------------+------*/ 
ECObjectsStatus           IECInstance::SetInternalValueUsingAccessor (ECValueAccessorCR accessor, ECValueCR valueToSet)
    {
    if (accessor.IsAdhocProperty())
        {
        // The array index is already pointing to the index of the desired ad-hoc property
        if (1 != accessor.GetDepth() || ECValueAccessor::INDEX_ROOT == accessor[0].GetArrayIndex())
            {
            BeAssert (false);
            return ECObjectsStatus::Error;
            }

        AdhocPropertyEdit adhoc (*this, accessor[0].GetPropertyIndex());
        return adhoc.SetValue (accessor[0].GetArrayIndex(), valueToSet);
        }

    ECObjectsStatus status          = ECObjectsStatus::Success;
    IECInstancePtr  currentInstance = this;

    for (uint32_t depth = 0; depth < accessor.GetDepth(); depth++)
        {
        bool compatible = (accessor[depth].GetEnabler() == &currentInstance->GetEnabler()); // if same enabler then use property index to set value else use access string
        int  propertyIndex   = accessor[depth].GetPropertyIndex();
        int  arrayIndex      = accessor[depth].GetArrayIndex();

        if (arrayIndex > -1)
            {
            ECValue         arrayInfoPlaceholder;

            //Get the array value to check its size. Expand array if necessary.
            if (compatible)
                status = currentInstance->GetValue(arrayInfoPlaceholder, (uint32_t)propertyIndex);
            else
                status = currentInstance->GetValue(arrayInfoPlaceholder, accessor.GetAccessString (depth));

            if (ECObjectsStatus::Success != status)
                return status;

            uint32_t arraySize = arrayInfoPlaceholder.GetArrayInfo().GetCount();

            if ((uint32_t)arrayIndex >= arraySize)
                {
                if (arrayInfoPlaceholder.GetArrayInfo().IsFixedCount())
                    return ECObjectsStatus::IndexOutOfRange;

                uint32_t numToInsert = 1 + (uint32_t)arrayIndex - arraySize;

                Utf8CP accessorWithBrackets = accessor.GetAccessString (depth);
                if (NULL == accessorWithBrackets)
                    return ECObjectsStatus::Error;

                status = currentInstance->AddArrayElements (accessorWithBrackets, numToInsert);    
                if (ECObjectsStatus::Success != status)
                    return status;
                }
            }

        // if we are processing the deepest location then set the value
        if (depth == (accessor.GetDepth()-1))
            return setInternalValueHelper (*currentInstance, accessor, depth, compatible, valueToSet);

        // if we are not inside an array this is an embedded struct, go into it.
        if (0 > arrayIndex)
            continue;

        ECValue         structPlaceholder;

        // if we get here we are processing an array of structs.  Get the struct's ECInstance so we can use for the next location depth
        status = getValueHelper (structPlaceholder, *currentInstance, accessor, depth, compatible);
        if (ECObjectsStatus::Success != status)
            return status;

        BeAssert (structPlaceholder.IsStruct() && "Accessor depth is greater than expected.");

        IECInstancePtr newInstance = structPlaceholder.GetStruct();

        // If the struct does not have an instance associated with it then build the instance
        if (newInstance.IsNull())
            {
            ECN::ECEnablerR          structEnabler = *(const_cast<ECN::ECEnablerP>(&accessor.GetEnabler (depth + 1)));
            ECClassCR               structClass   = accessor.GetEnabler (depth + 1).GetClass();
            StandaloneECEnablerPtr  standaloneEnabler = structEnabler.GetEnablerForStructArrayMember (structClass.GetSchema().GetSchemaKey(), structClass.GetName().c_str());

            newInstance = standaloneEnabler->CreateInstance();

            ECValue valueForSettingStructClass;
            valueForSettingStructClass.SetStruct (newInstance.get());

            status = setValueHelper (*currentInstance, accessor, depth, compatible, valueForSettingStructClass);
            if (ECObjectsStatus::Success != status)
                return status;

            // TFS#159623: Cannot use the StandaloneECInstance here...the containing ECInstance may have allocated a different ECInstance!
            // Obtain the struct ECInstance from the containing ECInstance.
            status = getValueHelper (valueForSettingStructClass, *currentInstance, accessor, depth, compatible);
            if (ECObjectsStatus::Success != status || valueForSettingStructClass.IsNull() || !valueForSettingStructClass.IsStruct())
                {
                BeAssert (false);
                return status;
                }

            newInstance = valueForSettingStructClass.GetStruct();
            }

        currentInstance = newInstance;
        } 

    return status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Bill.Steinbock                  11/2011
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus           IECInstance::SetValueUsingAccessor (ECValueAccessorCR accessor, ECValueCR valueToSet)
    {
    if ( ! ChangeValuesAllowed())
        return ECObjectsStatus::UnableToSetReadOnlyInstance;

    return  SetInternalValueUsingAccessor (accessor, valueToSet);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Bill.Steinbock                  08/2012
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus    IECInstance::GetIsPropertyNull (bool& isNull, uint32_t propertyIndex, bool useArrayIndex, uint32_t arrayIndex) const
    {
    return  _GetIsPropertyNull (isNull, propertyIndex, useArrayIndex, arrayIndex);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Paul.Connelly                   03/2013
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus IECInstance::_GetIsPropertyNull (bool& isNull, uint32_t propertyIndex, bool useArrayIndex, uint32_t arrayIndex) const
    {
    // default implementation. ECD-based implementations can be more efficient.
    isNull = true;
    ECValue v;
    ECObjectsStatus status = _GetValue (v, propertyIndex, useArrayIndex, arrayIndex);
    if (ECObjectsStatus::Success == status)
        isNull = v.IsNull();

    return status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Bill.Steinbock                  08/2012
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus     IECInstance::IsPropertyNull (bool& isNull, Utf8CP propertyAccessString) const 
    {
    uint32_t propertyIndex=0;
    ECObjectsStatus status = GetEnabler().GetPropertyIndex (propertyIndex, propertyAccessString);

    if (ECObjectsStatus::Success != status)
        return status;

    return _GetIsPropertyNull (isNull, propertyIndex, false, 0); 
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Bill.Steinbock                  08/2012
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus     IECInstance::IsPropertyNull (bool& isNull, Utf8CP propertyAccessString, uint32_t arrayIndex) const 
    {
    uint32_t propertyIndex=0;
    ECObjectsStatus status = GetEnabler().GetPropertyIndex (propertyIndex, propertyAccessString);

    if (ECObjectsStatus::Success != status)
        return status;

    return _GetIsPropertyNull (isNull, propertyIndex, true, arrayIndex); 
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Bill.Steinbock                  08/2012
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus     IECInstance::IsPropertyNull (bool& isNull, uint32_t propertyIndex) const 
    {
    return _GetIsPropertyNull (isNull, propertyIndex, false, 0); 
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Bill.Steinbock                  08/2012
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus     IECInstance::IsPropertyNull (bool& isNull, uint32_t propertyIndex, uint32_t arrayIndex) const 
    {
    return _GetIsPropertyNull (isNull, propertyIndex, true, arrayIndex); 
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                  02/2013
//+---------------+---------------+---------------+---------------+---------------+------
ECObjectsStatus IECInstance::ValidateDateTimeMetadata (uint32_t propertyIndex, ECValueCR v) const
    {
    if (v.IsDateTime () && !v.IsNull ())
        {
        DateTimeInfo dateTimeInfo;
        const ECObjectsStatus stat = GetDateTimeInfo (dateTimeInfo, propertyIndex);
        if (stat != ECObjectsStatus::Success)
            {
            LOG.error ("Error retrieving the DateTimeInfo custom attribute from the respective ECProperty.");
            return stat;
            }

        if (!v.DateTimeInfoMatches (dateTimeInfo))
            {
            LOG.errorv ("Setting a DateTime ECValue in ECInstance failed. DateTime metadata in ECValue mismatches the DateTimeInfo custom attribute on the respective ECProperty. Actual: %s. Expected: %s.",
                v.DateTimeMetadataToString ().c_str (), dateTimeInfo.ToString ().c_str ());
            return ECObjectsStatus::DataTypeMismatch;
            }
        }

    return ECObjectsStatus::Success;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                  02/2013
//+---------------+---------------+---------------+---------------+---------------+------
ECObjectsStatus IECInstance::SetDateTimeMetadataInECValue (ECValueR v, uint32_t propertyIndex) const
    {
    //only set date time meta data if the value is not null and if the metadata wasn't already set (by impl of _GetValue)
    if (!v.IsNull () && v.IsDateTime () && !v.IsDateTimeMetadataSet ())
        {
        DateTimeInfo caDateTimeMetadata;
        if (GetDateTimeInfo (caDateTimeMetadata, propertyIndex) == ECObjectsStatus::Success)
            {
            //fails if caDateTimeMetadata specified local DateTimeKind which is not supported
            if (SUCCESS != v.SetDateTimeMetadata (caDateTimeMetadata))
                {
                return ECObjectsStatus::DataTypeNotSupported;
                }
            }
        }

    return ECObjectsStatus::Success;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                  02/2013
//+---------------+---------------+---------------+---------------+---------------+------
ECObjectsStatus IECInstance::GetDateTimeInfo (DateTimeInfoR dateTimeInfo, uint32_t propertyIndex) const
    {
    //TODO: Need to profile this. The implementation does look up the access string from the prop index
    //and then parses to access string (to check whether it might refer to a struct member) before
    //actually calling ECClass::GetProperty
    ECPropertyCP ecProperty = GetEnabler ().LookupECProperty (propertyIndex);
    if (ecProperty == NULL)
        {
        return ECObjectsStatus::PropertyNotFound;
        }

    return StandardCustomAttributeHelper::GetDateTimeInfo (dateTimeInfo, *ecProperty);
    }

/////////////////////////////////////////////////////////////////////////////////////////
//  ECInstanceInteropHelper
/////////////////////////////////////////////////////////////////////////////////////////
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Bill.Steinbock                  10/2010
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus                 ECInstanceInteropHelper::GetValue (IECInstanceCR instance, ECValueR value, Utf8CP managedPropertyAccessor)
    {
    return getECValueFromInstance (value, instance, managedPropertyAccessor);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Bill.Steinbock                  03/2010
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus ECInstanceInteropHelper::GetLong (IECInstanceCR instance, int64_t & value, Utf8CP managedPropertyAccessor)
    {
    ECValue v;

    ECObjectsStatus status = getECValueFromInstance (v, instance, managedPropertyAccessor);
    if (status == ECObjectsStatus::Success)
        value = v.GetLong();
    
    return status;
    }    

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    CaseyMullen     09/09
+---------------+---------------+---------------+---------------+---------------+------*/        
ECObjectsStatus ECInstanceInteropHelper::GetInteger (IECInstanceCR instance, int & value, Utf8CP managedPropertyAccessor)
    {
    ECValue v;

    ECObjectsStatus status = getECValueFromInstance (v, instance, managedPropertyAccessor);
    if (status == ECObjectsStatus::Success)
        value = v.GetInteger();
    
    return status;
    }    

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    CaseyMullen     09/09
+---------------+---------------+---------------+---------------+---------------+------*/                
ECObjectsStatus ECInstanceInteropHelper::GetDouble (IECInstanceCR instance, double& value, Utf8CP managedPropertyAccessor)
    {
    ECValue v;

    ECObjectsStatus status = getECValueFromInstance (v, instance, managedPropertyAccessor);
    if (status == ECObjectsStatus::Success)        
        value = v.GetDouble();
        
    return status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    CaseyMullen     09/09
+---------------+---------------+---------------+---------------+---------------+------*/       
ECObjectsStatus ECInstanceInteropHelper::GetString (IECInstanceCR instance, Utf8StringR value, Utf8CP managedPropertyAccessor)
    {
    ECValue v;

    ECObjectsStatus status = getECValueFromInstance (v, instance, managedPropertyAccessor);
    if (status == ECObjectsStatus::Success)
        value = v.GetUtf8CP();

    return status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Bill.Steinbock                  03/2010
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus ECInstanceInteropHelper::GetBoolean (IECInstanceCR instance, bool & value, Utf8CP managedPropertyAccessor)
    {
    ECValue v;

    ECObjectsStatus status = getECValueFromInstance (v, instance, managedPropertyAccessor);
    if (status == ECObjectsStatus::Success)
        value = v.GetBoolean();

    return status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Bill.Steinbock                  03/2010
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus ECInstanceInteropHelper::GetPoint2D (IECInstanceCR instance, DPoint2d & value, Utf8CP managedPropertyAccessor)
    {
    ECValue v;

    ECObjectsStatus status = getECValueFromInstance (v, instance, managedPropertyAccessor);
    if (status == ECObjectsStatus::Success)
        value = v.GetPoint2D();
        
    return status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Bill.Steinbock                  03/2010
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus ECInstanceInteropHelper::GetPoint3D (IECInstanceCR instance, DPoint3d & value, Utf8CP managedPropertyAccessor)
    {
    ECValue v;

    ECObjectsStatus status = getECValueFromInstance (v, instance, managedPropertyAccessor);
    if (status == ECObjectsStatus::Success)
        value = v.GetPoint3D();
        
    return status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Bill.Steinbock                  03/2010
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus ECInstanceInteropHelper::GetDateTime (IECInstanceCR instance, DateTimeR value, Utf8CP managedPropertyAccessor)
    {
    ECValue v;

    ECObjectsStatus status = getECValueFromInstance (v, instance, managedPropertyAccessor);
    if (status == ECObjectsStatus::Success)
        value = v.GetDateTime();
        
    return status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Bill.Steinbock                  03/2010
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus ECInstanceInteropHelper::GetDateTimeTicks (IECInstanceCR instance, int64_t & value, Utf8CP managedPropertyAccessor)
    {
    ECValue v;

    ECObjectsStatus status = getECValueFromInstance (v, instance, managedPropertyAccessor);
    if (status == ECObjectsStatus::Success)
        value = v.GetDateTimeTicks();
        
    return status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Bill.Steinbock                  10/2010
+---------------+---------------+---------------+---------------+---------------+------*/
static ECObjectsStatus setECValueUsingFullAccessString (Utf8Char* asBuffer, Utf8Char* indexBuffer, ECValueCR v, IECInstanceR instance, Utf8CP managedPropertyAccessor)
    {
    // skip all the work if the instance is read only
    if (instance.IsReadOnly())
    if ( ! instance.ChangeValuesAllowed())
        return ECObjectsStatus::UnableToSetReadOnlyInstance;

    // see if access string specifies an array
    Utf8CP pos1 = strchr (managedPropertyAccessor, '[');

    // if not an array then 
    if (NULL == pos1)
        return instance.SetValue (managedPropertyAccessor, v);

    size_t numChars = 0;
    numChars = pos1 - managedPropertyAccessor;
    strncpy(asBuffer, managedPropertyAccessor, numChars>NUM_ACCESSSTRING_BUFFER_CHARS?NUM_ACCESSSTRING_BUFFER_CHARS:numChars);
    asBuffer[numChars]=0;

    Utf8CP pos2 = strchr (pos1+1, ']');

    BeAssert (pos2 != NULL);

    numChars = pos2 - pos1 - 1;

    strncpy(indexBuffer, pos1+1, numChars>NUM_INDEX_BUFFER_CHARS?NUM_INDEX_BUFFER_CHARS:numChars);
    indexBuffer[numChars]=0;

    uint32_t indexValue = 0;
    if (1 != BE_STRING_UTILITIES_UTF8_SSCANF(indexBuffer, "%ud", &indexValue))
        return ECObjectsStatus::Error;

    ECValue         arrayVal;
    ECObjectsStatus status;

    if (ECObjectsStatus::Success != (status = instance.GetValue (arrayVal, asBuffer)))
        return status;

    ArrayInfo arrayInfo = arrayVal.GetArrayInfo();
    uint32_t  size      = arrayInfo.GetCount();

    if (indexValue >= size)
        {
        if (arrayInfo.IsFixedCount())
            return ECObjectsStatus::Error;

        unsigned int numToInsert = (indexValue + 1) - size;
        status =  instance.AddArrayElements (asBuffer, numToInsert);    
        if (ECObjectsStatus::Success != status)
            return status;

        if (arrayInfo.IsStructArray())
            {
            ECClassCR    ecClass     = instance.GetClass();

            Utf8Char buffer [NUM_INDEX_BUFFER_CHARS+1];
            strncpy(buffer, asBuffer, NUM_INDEX_BUFFER_CHARS);
            ECPropertyP prop = getProperty (ecClass, asBuffer, buffer);
            
            if (!prop->GetIsArray())
                return ECObjectsStatus::Error;

            StructArrayECPropertyP arrayProp = dynamic_cast<StructArrayECPropertyP>(prop);
            if (!arrayProp)
                return ECObjectsStatus::Error;

            ECClassCP structClass = arrayProp->GetStructElementType();

            StandaloneECEnablerPtr standaloneEnabler = instance.GetEnablerR().GetEnablerForStructArrayMember (structClass->GetSchema().GetSchemaKey(), structClass->GetName().c_str());
            if (standaloneEnabler.IsNull())
                return ECObjectsStatus::Error;

            ECValue                     arrayEntryVal;

            for (uint32_t i=0; i<numToInsert; i++)
                {
                // only set new struct value if AddArrayElements did not already set it
                if (ECObjectsStatus::Success != instance.GetValue (arrayEntryVal, asBuffer, size+i) || arrayEntryVal.IsNull ())
                    {
                    arrayEntryVal.SetStruct (standaloneEnabler->CreateInstance().get());
                    if (ECObjectsStatus::Success != instance.SetValue (asBuffer, arrayEntryVal, size+i))
                        return ECObjectsStatus::Error;
                    }
                }
            }
        }

    if (arrayInfo.IsPrimitiveArray())
        return instance.SetValue (asBuffer, v, indexValue);

    // must be a struct array
    if (NULL == strchr (pos2, '.'))
        {
        //Caller is attempting to set the value of this struct array element directly.
        return instance.SetValue (asBuffer, v, indexValue);
        }
    instance.GetValue (arrayVal, asBuffer, indexValue);
    IECInstancePtr arrayEntryInstance = arrayVal.GetStruct();

    return setECValueUsingFullAccessString (asBuffer, indexBuffer, v, *arrayEntryInstance, pos2+2); // move to character after "]." in access string.
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Bill.Steinbock                  10/2010
+---------------+---------------+---------------+---------------+---------------+------*/
static ECObjectsStatus setECValueInInstance (ECValueCR v, IECInstanceR instance, Utf8CP managedPropertyAccessor)
    {
    Utf8String asBufferStr;

    Utf8Char asBuffer[NUM_ACCESSSTRING_BUFFER_CHARS+1];
    Utf8Char indexBuffer[NUM_INDEX_BUFFER_CHARS+1];

    return setECValueUsingFullAccessString (asBuffer, indexBuffer, v, instance, managedPropertyAccessor);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Bill.Steinbock                  10/2010
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus ECInstanceInteropHelper::SetValue  (IECInstanceR instance, Utf8CP managedPropertyAccessor, ECValueCR value)
    {
    return setECValueInInstance (value, instance, managedPropertyAccessor);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    CaseyMullen     01/10
+---------------+---------------+---------------+---------------+---------------+------*/       
ECObjectsStatus ECInstanceInteropHelper::SetLongValue (IECInstanceR instance, Utf8CP managedPropertyAccessor, int64_t value)
    {
    ECValue v(value);
    return setECValueInInstance (v, instance, managedPropertyAccessor);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    CaseyMullen     01/10
+---------------+---------------+---------------+---------------+---------------+------*/       
ECObjectsStatus ECInstanceInteropHelper::SetIntegerValue (IECInstanceR instance, Utf8CP managedPropertyAccessor, int value)
    {
    ECValue v(value);
    return setECValueInInstance (v, instance, managedPropertyAccessor);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Bill.Steinbock                  03/2010
+---------------+---------------+---------------+---------------+---------------+------*/       
ECObjectsStatus ECInstanceInteropHelper::SetDoubleValue (IECInstanceR instance, Utf8CP managedPropertyAccessor, double value)
    {
    ECValue v(value);
    return setECValueInInstance (v, instance, managedPropertyAccessor);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    CaseyMullen     01/10
+---------------+---------------+---------------+---------------+---------------+------*/       
ECObjectsStatus ECInstanceInteropHelper::SetStringValue  (IECInstanceR instance, Utf8CP managedPropertyAccessor, Utf8CP value)
    {
    ECValue v(value, false);
    return setECValueInInstance (v, instance, managedPropertyAccessor);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Bill.Steinbock                  03/2010
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus ECInstanceInteropHelper::SetBooleanValue  (IECInstanceR instance, Utf8CP managedPropertyAccessor, bool value)
    {
    ECValue v(value);
    return setECValueInInstance (v, instance, managedPropertyAccessor);
    }
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Bill.Steinbock                  03/2010
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus ECInstanceInteropHelper::SetPoint2DValue  (IECInstanceR instance, Utf8CP managedPropertyAccessor, DPoint2dCR value)
    {
    ECValue v(value);
    return setECValueInInstance (v, instance, managedPropertyAccessor);
    }
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Bill.Steinbock                  03/2010
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus ECInstanceInteropHelper::SetPoint3DValue  (IECInstanceR instance, Utf8CP managedPropertyAccessor, DPoint3dCR value)
    {
    ECValue v(value);
    return setECValueInInstance (v, instance, managedPropertyAccessor);
    }
   
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Bill.Steinbock                  03/2010
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus ECInstanceInteropHelper::SetDateTimeValue (IECInstanceR instance, Utf8CP managedPropertyAccessor, DateTimeCR value)
    {
    ECValue v(value);
    return setECValueInInstance (v, instance, managedPropertyAccessor);
    }
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Bill.Steinbock                  03/2010
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus ECInstanceInteropHelper::SetDateTimeTicks (IECInstanceR instance, Utf8CP managedPropertyAccessor, int64_t value)
    {
    ECValue v;
    v.SetDateTimeTicks (value);
    return setECValueInInstance (v, instance, managedPropertyAccessor);
    }

///////////////////////////////////////////////////////////////////////////////
// Get Using ECValueAccessor
///////////////////////////////////////////////////////////////////////////////
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Bill.Steinbock                  03/2010
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus ECInstanceInteropHelper::GetLong (IECInstanceCR instance, int64_t & value, ECValueAccessorCR accessor)
    {
    ECValue v;

    ECObjectsStatus status = instance.GetValueUsingAccessor (v, accessor);
    if (status == ECObjectsStatus::Success)
        value = v.GetLong();
    
    return status;
    }    

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    CaseyMullen     09/09
+---------------+---------------+---------------+---------------+---------------+------*/        
ECObjectsStatus ECInstanceInteropHelper::GetInteger (IECInstanceCR instance, int & value, ECValueAccessorCR accessor)
    {
    ECValue v;

    ECObjectsStatus status = instance.GetValueUsingAccessor (v, accessor);
    if (status == ECObjectsStatus::Success)
        value = v.GetInteger();
    
    return status;
    }    

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    CaseyMullen     09/09
+---------------+---------------+---------------+---------------+---------------+------*/                
ECObjectsStatus ECInstanceInteropHelper::GetDouble (IECInstanceCR instance, double& value, ECValueAccessorCR accessor)
    {
    ECValue v;

    ECObjectsStatus status = instance.GetValueUsingAccessor (v, accessor);
    if (status == ECObjectsStatus::Success)        
        value = v.GetDouble();
        
    return status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    CaseyMullen     09/09
+---------------+---------------+---------------+---------------+---------------+------*/       
ECObjectsStatus ECInstanceInteropHelper::GetString (IECInstanceCR instance, Utf8StringR value, ECValueAccessorCR accessor)
    {
    ECValue v;

    ECObjectsStatus status = instance.GetValueUsingAccessor (v, accessor);
    if (status == ECObjectsStatus::Success)
        value = v.GetUtf8CP();

    return status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Bill.Steinbock                  03/2010
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus ECInstanceInteropHelper::GetBoolean (IECInstanceCR instance, bool & value, ECValueAccessorCR accessor)
    {
    ECValue v;

    ECObjectsStatus status = instance.GetValueUsingAccessor (v, accessor);
    if (status == ECObjectsStatus::Success)
        value = v.GetBoolean();

    return status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Bill.Steinbock                  03/2010
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus ECInstanceInteropHelper::GetPoint2D (IECInstanceCR instance, DPoint2d & value, ECValueAccessorCR accessor)
    {
    ECValue v;

    ECObjectsStatus status = instance.GetValueUsingAccessor (v, accessor);
    if (status == ECObjectsStatus::Success)
        value = v.GetPoint2D();
        
    return status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Bill.Steinbock                  03/2010
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus ECInstanceInteropHelper::GetPoint3D (IECInstanceCR instance, DPoint3d & value, ECValueAccessorCR accessor)
    {
    ECValue v;

    ECObjectsStatus status = instance.GetValueUsingAccessor (v, accessor);
    if (status == ECObjectsStatus::Success)
        value = v.GetPoint3D();
        
    return status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Bill.Steinbock                  03/2010
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus ECInstanceInteropHelper::GetDateTime (IECInstanceCR instance, DateTimeR value, ECValueAccessorCR accessor)
    {
    ECValue v;

    ECObjectsStatus status = instance.GetValueUsingAccessor (v, accessor);
    if (status == ECObjectsStatus::Success)
        value = v.GetDateTime();
        
    return status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Bill.Steinbock                  03/2010
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus ECInstanceInteropHelper::GetDateTimeTicks (IECInstanceCR instance, int64_t & value, ECValueAccessorCR accessor)
    {
    ECValue v;

    ECObjectsStatus status = instance.GetValueUsingAccessor (v, accessor);
    if (status == ECObjectsStatus::Success)
        value = v.GetDateTimeTicks();
        
    return status;
    }

///////////////////////////////////////////////////////////////////////////////
// Set Using ECValueAccessor
///////////////////////////////////////////////////////////////////////////////
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    CaseyMullen     01/10
+---------------+---------------+---------------+---------------+---------------+------*/       
ECObjectsStatus ECInstanceInteropHelper::SetValue  (IECInstanceR instance, ECValueAccessorCR accessor, ECValueCR value)
    {
    return  instance.SetValueUsingAccessor (accessor, value);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Bill.Steinbock                  06/2011
+---------------+---------------+---------------+---------------+---------------+------*/       
ECObjectsStatus ECInstanceInteropHelper::SetLongValue (IECInstanceR instance, ECValueAccessorCR accessor, int64_t value)
    {
    ECValue v(value);
    return  instance.SetValueUsingAccessor (accessor, v);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Bill.Steinbock                  06/2011
+---------------+---------------+---------------+---------------+---------------+------*/       
ECObjectsStatus ECInstanceInteropHelper::SetIntegerValue (IECInstanceR instance, ECValueAccessorCR accessor, int value)
    {
    ECValue v(value);
    return  instance.SetValueUsingAccessor (accessor, v);
    }
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Bill.Steinbock                  06/2011
+---------------+---------------+---------------+---------------+---------------+------*/       
ECObjectsStatus ECInstanceInteropHelper::SetDoubleValue (IECInstanceR instance, ECValueAccessorCR accessor, double value)
    {
    ECValue v(value);
    return  instance.SetValueUsingAccessor (accessor, v);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Bill.Steinbock                  06/2011
+---------------+---------------+---------------+---------------+---------------+------*/       
ECObjectsStatus ECInstanceInteropHelper::SetStringValue  (IECInstanceR instance, ECValueAccessorCR accessor, Utf8CP value)
    {
    ECValue v(value, false);
    return  instance.SetValueUsingAccessor (accessor, v);
    }
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Bill.Steinbock                  06/2011
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus ECInstanceInteropHelper::SetBooleanValue  (IECInstanceR instance, ECValueAccessorCR accessor, bool value)
    {
    ECValue v(value);
    return  instance.SetValueUsingAccessor (accessor, v);
    }
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Bill.Steinbock                  06/2011
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus ECInstanceInteropHelper::SetPoint2DValue  (IECInstanceR instance, ECValueAccessorCR accessor, DPoint2dCR value)
    {
    ECValue v(value);
    return  instance.SetValueUsingAccessor (accessor, v);
    }
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Bill.Steinbock                  03/2010
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus ECInstanceInteropHelper::SetPoint3DValue  (IECInstanceR instance, ECValueAccessorCR accessor, DPoint3dCR value)
    {
    ECValue v(value);
    return  instance.SetValueUsingAccessor (accessor, v);
    }
   
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Bill.Steinbock                  06/2011
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus ECInstanceInteropHelper::SetDateTimeValue (IECInstanceR instance, ECValueAccessorCR accessor, DateTimeCR value)
    {
    ECValue v(value);
    return  instance.SetValueUsingAccessor (accessor, v);
    }
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Bill.Steinbock                  06/2011
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus ECInstanceInteropHelper::SetDateTimeTicks (IECInstanceR instance, ECValueAccessorCR accessor, int64_t value)
    {
    ECValue v;
    v.SetDateTimeTicks (value);
    return  instance.SetValueUsingAccessor (accessor, v);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Bill.Steinbock                  06/2011
+---------------+---------------+---------------+---------------+---------------+------*/
bool            ECInstanceInteropHelper::IsNull (IECInstanceR instance, ECValueAccessorCR accessor)
    {
    ECValue v;

    ECObjectsStatus status = instance.GetValueUsingAccessor (v, accessor);
    if (status != ECObjectsStatus::Success)
        return true;

    return v.IsNull();
    }

 /*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Bill.Steinbock                  06/2011
+---------------+---------------+---------------+---------------+---------------+------*/
void            ECInstanceInteropHelper::SetToNull (IECInstanceR instance, ECValueAccessorCR accessor)
    {
    ECValue v;
    v.SetToNull();

    instance.SetValueUsingAccessor (accessor, v);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Dylan.Rush                      08/2011
+---------------+---------------+---------------+---------------+---------------+------*/
bool            ECInstanceInteropHelper::IsPropertyReadOnly (IECInstanceCR instance, ECValueAccessorR accessor)
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
        if (structInstance.IsNull ())
            {
            // note: null structs were throwing exceptions in element info dlg
            // I assume that if the struct is null, it is considered read-only
            return true;
            }

        return structInstance->IsPropertyReadOnly (propertyIndex);
        }
    return instance.IsPropertyReadOnly (propertyIndex);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Bill.Steinbock                  09/2011
+---------------+---------------+---------------+---------------+---------------+------*/
ECN::ECEnablerP                  ECInstanceInteropHelper::GetEnablerForStructArrayEntry (IECInstanceR instance, ECValueAccessorR arrayMemberAccessor, SchemaKeyCR schemaKey, Utf8CP className)
    {
    ECN::ECValue v;
    instance.GetValueUsingAccessor (v, arrayMemberAccessor);

    if (!v.IsStruct())
        return NULL;

    if (!v.IsNull())
        {
        ECN::IECInstancePtr structInstance = v.GetStruct();
        return &structInstance->GetEnablerR();
        }

    // if we get here we probably have a fixed size array with NULL entries
    ECN::ECEnablerP structArrayEnabler = const_cast<ECN::ECEnablerP>(arrayMemberAccessor.DeepestLocation().GetEnabler());

    ECN::StandaloneECEnablerPtr standaloneEnabler = structArrayEnabler->GetEnablerForStructArrayMember (schemaKey, className);
    if (standaloneEnabler.IsNull())
        {
        LOG.errorv ("Unable to locate a standalone enabler for class %s", className);
        return NULL;
        }

    return standaloneEnabler.get();
    }

/*---------------------------------------------------------------------------------**//**
* This method is called from derived class so the rootInstance was pinned if necessary
* before calling this method.
* @bsimethod                                    Bill.Steinbock                  06/2011
+---------------+---------------+---------------+---------------+---------------+------*/
/* static */ ECN::IECInstancePtr getParentNativeInstance (ECN::IECInstanceCP rootInstance, ECN::ECValueAccessorCR structValueAccessor)
    {
    // if not a top level property, get the native instance that will contain this struct array
    if (structValueAccessor.GetDepth () > 1)
        {
        ECN::ECValue parentStructValue;

        ECN::ECValueAccessor parentInstanceAccessor (structValueAccessor);
        parentInstanceAccessor.PopLocation ();   // remove one level to get to the parent instance

        rootInstance->GetValueUsingAccessor (parentStructValue, parentInstanceAccessor);
        if (!parentStructValue.IsStruct ())
            return NULL;

        ECN::IECInstancePtr structInstance = parentStructValue.GetStruct ();
        if (structInstance.IsValid())
            return structInstance;

        // we may be processing a member of an embedded struct so we need to check the next level up
        return getParentNativeInstance (rootInstance, parentInstanceAccessor);
        }

    return const_cast<ECN::IECInstanceP>(rootInstance);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Bill.Steinbock                  06/2011
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus  ECInstanceInteropHelper::GetStructArrayEntry (ECN::ECValueAccessorR structArrayEntryValueAccessor, IECInstanceR instance, uint32_t index, ECN::ECValueAccessorCR structArrayValueAccessor, 
                                                             bool createPropertyIfNotFound, Utf8CP wcharAccessString, 
                                                             SchemaKeyCR schemaKey, Utf8CP className)
    {
    ECN::ECEnablerR structArrayEnabler = *(const_cast<ECN::ECEnablerP>(structArrayValueAccessor.DeepestLocationCR().GetEnabler()));
    ECN::StandaloneECEnablerPtr standaloneEnabler = structArrayEnabler.GetEnablerForStructArrayMember (schemaKey, className);
    if (standaloneEnabler.IsNull())
        {
        LOG.errorv ("Unable to locate a standalone enabler for class \" %s \"", className);
        return ECObjectsStatus::EnablerNotFound;
        }

    ECN::ECValue  arrayVal;
    instance.GetValueUsingAccessor (arrayVal, structArrayValueAccessor);

    ArrayInfo   arrayInfo  = arrayVal.GetArrayInfo();
    uint32_t    arrayCount = arrayInfo.GetCount();

    // adjust the ECVAlueAccessor to include the array index
    ECN::ECValueAccessor arrayEntryValueAccessor (structArrayValueAccessor);
    arrayEntryValueAccessor.DeepestLocation ().SetArrayIndex (index);

    if (arrayCount <= index)
        {
        // see if we are allowed to add a new struct array instance
        if (!createPropertyIfNotFound)
            return ECObjectsStatus::Error;

        // only proceed if not read only instance
        if ( ! instance.ChangeValuesAllowed())
            return ECObjectsStatus::UnableToSetReadOnlyInstance;

        ECN::IECInstancePtr parentNativeInstance = getParentNativeInstance (&instance, structArrayValueAccessor);
        if (parentNativeInstance.IsNull())
            {
            LOG.error ("Unable to get native instance when processing ECInstanceInteropHelper::GetStructArrayEntry");
            return ECObjectsStatus::Error;
            }

        ::uint32_t numToInsert = (index + 1) - arrayCount;
        if (ECN::ECObjectsStatus::Success != parentNativeInstance->AddArrayElements (wcharAccessString, numToInsert))
            {
            LOG.errorv("Unable to add array element(s) to native instance - access string \"%s\"", structArrayValueAccessor.GetManagedAccessString().c_str());
            return ECObjectsStatus::UnableToAddStructArrayMember;
            }

        ECN::ECValue  arrayEntryVal;
        for (::uint32_t i=0; i<numToInsert; i++)
            {
            arrayEntryVal.SetStruct (standaloneEnabler->CreateInstance().get());
            if (ECObjectsStatus::Success != parentNativeInstance->SetValue (wcharAccessString, arrayEntryVal, arrayCount+i))
                return ECObjectsStatus::UnableToSetStructArrayMemberInstance;
            }
        }
    else
        {
        // make sure the struct instance is not null
        ECN::ECValue arrayEntryVal;

        instance.GetValueUsingAccessor (arrayEntryVal, arrayEntryValueAccessor);
        if (arrayEntryVal.IsNull())
            {
            // see if we are allowed to add a new strct array instance
            if (!createPropertyIfNotFound)
                return ECObjectsStatus::Error;

            // only proceed if not read only instance
            if (instance.ChangeValuesAllowed())
                return ECObjectsStatus::UnableToSetReadOnlyInstance;

            arrayEntryVal.SetStruct (standaloneEnabler->CreateInstance().get());

            ECN::IECInstancePtr parentNativeInstance = getParentNativeInstance (&instance, structArrayValueAccessor);
            if (parentNativeInstance.IsNull())
                {
                LOG.error ("Unable to get native instance when processing ECInstanceInteropHelper::GetStructArrayEntry");
                return ECObjectsStatus::Error;
                }

            if (ECObjectsStatus::Success != parentNativeInstance->SetValue (wcharAccessString, arrayEntryVal, index))
                return ECObjectsStatus::UnableToSetStructArrayMemberInstance;
            }
        }

    structArrayEntryValueAccessor.Clone (arrayEntryValueAccessor);
    return ECObjectsStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Dylan.Rush                      1/11
+---------------+---------------+---------------+---------------+---------------+------*/
bool  ECInstanceInteropHelper::IsCalculatedECProperty  (IECInstanceCR instance, int propertyIndex)
    {
    Utf8CP accessor;
    if (ECObjectsStatus::Success != instance.GetEnabler().GetAccessString (accessor, (uint32_t) propertyIndex))
        return false;

    ECClassCR ecClass = instance.GetClass();

    Utf8Char buffer [NUM_INDEX_BUFFER_CHARS+1];

    strncpy(buffer, accessor, NUM_INDEX_BUFFER_CHARS);

    ECPropertyP ecProperty = getProperty (ecClass, accessor, buffer);

    if (NULL == ecProperty)
        return false;

    return ecProperty->IsDefined ("Bentley_Standard_CustomAttributes", "CalculatedECPropertySpecification");
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Dylan.Rush                      1/11
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus ECInstanceInteropHelper::SetValueByIndex (IECInstanceR instance, int propertyIndex, int arrayIndex, ECValueCR value)
    {
    if (-1 == arrayIndex)
        return instance.SetValue ((uint32_t) propertyIndex, value);

    ECN::ECValue v;
    instance.GetValue (v, propertyIndex);
    uint32_t count = v.GetArrayInfo().GetCount();
    if ((uint32_t)arrayIndex >= count)
        {
        ECObjectsStatus status;
        uint32_t size = 1 + ((uint32_t)arrayIndex - count);
        Utf8CP accessString;
        status = instance.GetEnabler().GetAccessString (accessString, propertyIndex);
        if (ECObjectsStatus::Success != status)
            return status;
        status = instance.AddArrayElements (accessString, size);
        if (ECN::ECObjectsStatus::Success != status)
            return status;
        }
    return instance.SetValue ((uint32_t) propertyIndex, value, (uint32_t) arrayIndex);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Dylan.Rush                      1/11
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus ECInstanceInteropHelper::GetValueByIndex (ECValueR value, IECInstanceCR instance, int propertyIndex, int arrayIndex)
    {
    if (-1 == arrayIndex)
        return instance.GetValue (value, (uint32_t) propertyIndex);

    ECN::ECValue v;
    instance.GetValue (v, propertyIndex);
    uint32_t count = v.GetArrayInfo().GetCount();
    if ((uint32_t)propertyIndex >= count)
        {
        value.SetToNull ();
        }
    return instance.GetValue (value, (uint32_t) propertyIndex, (uint32_t) arrayIndex);
    }

/*---------------------------------------------------------------------------------**//**
* Expect some accessor ending in an array property. Want the last IECInstance in the
* chain and the property index of the array property.
* ex: "SomeStruct.SomeStructArray[0].SomeArray"
* @bsimethod                                                    Paul.Connelly   08/13
+---------------+---------------+---------------+---------------+---------------+------*/
static ECObjectsStatus  resolveArrayAccessString (IECInstancePtr& resolvedInstance, uint32_t& resolvedPropertyIndex, IECInstanceR rootInstance, Utf8CP fullAccessString)
    {
    ECValueAccessor accessor;
    ECObjectsStatus status = ECValueAccessor::PopulateValueAccessor (accessor, rootInstance, fullAccessString);
    if (ECObjectsStatus::Success != status)
        return status;
    else if (0 == accessor.GetDepth())
        return ECObjectsStatus::PropertyNotFound;

    resolvedInstance = const_cast<IECInstanceP> (&rootInstance);
    resolvedPropertyIndex = 0;

    uint32_t depth = 0;
    for ( ; depth < accessor.GetDepth() - 1; depth++)
        {
        ECValue v;
        bool compatible = (accessor[depth].GetEnabler() == &resolvedInstance->GetEnabler());
        ECObjectsStatus status = getValueHelper (v, *resolvedInstance, accessor, depth, compatible);
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
        Utf8CP accessString = accessor.GetAccessString (depth);
        if (NULL == accessString)
            return ECObjectsStatus::Error;

        ECObjectsStatus status = resolvedInstance->GetEnabler().GetPropertyIndex (resolvedPropertyIndex, accessString);
        if (ECObjectsStatus::Success != status)
            return status;
        }

    ECPropertyCP ecprop = accessor[depth].GetECProperty();
    return NULL != ecprop && ecprop->GetIsArray() ? ECObjectsStatus::Success : ECObjectsStatus::Error;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   08/13
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus ECInstanceInteropHelper::ClearArray (IECInstanceR rootInstance, Utf8CP accessString)
    {
    IECInstancePtr resolvedInstance;
    uint32_t propertyIndex;
    ECObjectsStatus status = resolveArrayAccessString (resolvedInstance, propertyIndex, rootInstance, accessString);
    if (ECObjectsStatus::Success == status)
        status = resolvedInstance->ClearArray (propertyIndex);

    return status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   08/13
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus ECInstanceInteropHelper::RemoveArrayElement (IECInstanceR rootInstance, Utf8CP accessString, uint32_t arrayIndex)
    {
    IECInstancePtr resolvedInstance;
    uint32_t propertyIndex;
    ECObjectsStatus status = resolveArrayAccessString (resolvedInstance, propertyIndex, rootInstance, accessString);
    if (ECObjectsStatus::Success == status)
        status = resolvedInstance->RemoveArrayElement (propertyIndex, arrayIndex);

    return status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   08/13
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus ECInstanceInteropHelper::AddArrayElements (IECInstanceR rootInstance, Utf8CP accessString, uint32_t count, uint32_t atIndex)
    {
    IECInstancePtr resolvedInstance;
    uint32_t propertyIndex;
    ECObjectsStatus status = resolveArrayAccessString (resolvedInstance, propertyIndex, rootInstance, accessString);
    if (ECObjectsStatus::Success == status)
        status = -1 == atIndex ? resolvedInstance->AddArrayElements (propertyIndex, count) : resolvedInstance->InsertArrayElements (propertyIndex, atIndex, count);

    return status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Adam.Klatzkin                   01/2010
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus                 IECInstance::InsertArrayElements (Utf8CP propertyAccessString, uint32_t index, uint32_t size)
    {
    uint32_t propIdx;
    ECObjectsStatus status = GetEnabler().GetPropertyIndex (propIdx, propertyAccessString);
    return ECObjectsStatus::Success == status ? InsertArrayElements (propIdx, index, size) : status;
    } 
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Adam.Klatzkin                   01/2010
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus                 IECInstance::AddArrayElements (Utf8CP propertyAccessString, uint32_t size)
    {
    uint32_t propIdx;
    ECObjectsStatus status = GetEnabler().GetPropertyIndex (propIdx, propertyAccessString);
    return ECObjectsStatus::Success == status ? AddArrayElements (propIdx, size) : status;
    }        

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Adam.Klatzkin                   01/2010
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus                 IECInstance::RemoveArrayElement (Utf8CP propertyAccessString, uint32_t index)
    {
    uint32_t propIdx;
    ECObjectsStatus status = GetEnabler().GetPropertyIndex (propIdx, propertyAccessString);
    return ECObjectsStatus::Success == status ? RemoveArrayElement (propIdx, index) : status;
    } 
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Adam.Klatzkin                   01/2010
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus                 IECInstance::ClearArray (Utf8CP propertyAccessString)
    {
    uint32_t propertyIndex;
    ECObjectsStatus status = GetEnabler().GetPropertyIndex (propertyIndex, propertyAccessString);
    return ECObjectsStatus::Success == status ? ClearArray (propertyIndex) : status;
    }           

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   11/12
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus IECInstance::ClearArray (uint32_t propIdx)                                    { return _ClearArray (propIdx); }
ECObjectsStatus IECInstance::InsertArrayElements (uint32_t propIdx, uint32_t idx, uint32_t size)  { return _InsertArrayElements (propIdx, idx, size); }
ECObjectsStatus IECInstance::AddArrayElements (uint32_t propIdx, uint32_t size)                 { return _AddArrayElements (propIdx, size); }
ECObjectsStatus IECInstance::RemoveArrayElement (uint32_t propIdx, uint32_t idx)                { return _RemoveArrayElement (propIdx, idx); }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    CaseyMullen     09/09
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String                         IECInstance::ToString (Utf8CP indent) const
    {
    return _ToString (indent);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   10/12
+---------------+---------------+---------------+---------------+---------------+------*/
static bool getInstanceLabelPropertyNameFromClass (Utf8StringR propertyName, ECClassCR ecClass, bool& alwaysUseClassLabel)
    {
    IECInstancePtr caInstance = ecClass.GetCustomAttribute ("Bentley_Standard_CustomAttributes", "InstanceLabelSpecification");
    if (caInstance.IsValid())
        {
        ECValue value;
        if (ECObjectsStatus::Success == caInstance->GetValue (value, "PropertyName") && !value.IsNull())
            {
            propertyName = value.GetUtf8CP();
            return true;
            }
        }

    if (ecClass.GetCustomAttribute ("Bentley_Standard_CustomAttributes", "ClassLabelIsInstanceLabel").IsValid())
        {
        alwaysUseClassLabel = true;
        return false;
        }

    for (ECClassCP baseClass: ecClass.GetBaseClasses())
        {
        if (getInstanceLabelPropertyNameFromClass (propertyName, *baseClass, alwaysUseClassLabel))
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
* @bsimethod                                    Bill.Steinbock                  05/2011
+---------------+---------------+---------------+---------------+---------------+------*/
bool                         IECInstance::GetInstanceLabelPropertyName (Utf8StringR propName) const
    {
    ECClassCR ecClass = GetClass();
    bool alwaysUseClassLabel = false;
    if (getInstanceLabelPropertyNameFromClass (propName, ecClass, alwaysUseClassLabel))
        return true;
    else if (alwaysUseClassLabel)
        return false;

    const Utf8CP* standardName = s_standardInstanceLabelPropertyNames;
    while (*standardName)
        {
        if (NULL != ecClass.GetPropertyP (*standardName))
            {
            propName = *standardName;
            return true;
            }

        ++standardName;
        }

    return false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Bill.Steinbock                  05/2011
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus                 IECInstance::_GetDisplayLabel (Utf8String& displayLabel) const
    {
    Utf8String propertyName;
    if (GetInstanceLabelPropertyName (propertyName) && 0 < propertyName.length())   // empty property name => always use class label, don't look for "NAME" or "Name"
        {
        ECN::ECValue ecValue;
        if (ECObjectsStatus::Success == GetValue (ecValue, propertyName.c_str()) && !ecValue.IsNull())
            {
            auto prop = GetClass().GetPropertyP (propertyName.c_str());
            auto adapter = nullptr != prop ? prop->GetTypeAdapter() : nullptr;
            if (nullptr == adapter)
                {
                // Preserve old behavior of converting directly to string, because ECObjects.dll doesn't know how to use type adapters...
            if (ecValue.ConvertToPrimitiveType (PRIMITIVETYPE_String) && !Utf8String::IsNullOrEmpty (ecValue.GetUtf8CP()))
                    {
                displayLabel = ecValue.GetUtf8CP();
                    return ECObjectsStatus::Success;
                    }
                }
            else
                {
                // TFS#244646: Use type adapters to do conversion
                auto context = nullptr != adapter ? IECTypeAdapterContext::Create (*prop, *this) : nullptr;
                if (context.IsValid() && adapter->ConvertToString (displayLabel, ecValue, *context))
                    return ECObjectsStatus::Success;
                }
            }
        }

    // According to documentation in managed ECF, we are supposed to fallback to the class's display label
    displayLabel = GetClass().GetDisplayLabel();
    return ECObjectsStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Bill.Steinbock                  05/2011
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus                 IECInstance::GetDisplayLabel (Utf8String& displayLabel) const
    {
    return  _GetDisplayLabel (displayLabel);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Bill.Steinbock                  05/2011
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus                 IECInstance::_SetDisplayLabel (Utf8CP displayLabel)    
    {
    Utf8String propertyName;
    if (!GetInstanceLabelPropertyName (propertyName))
        return ECObjectsStatus::Error;

    ECN::ECValue ecValue;
    auto prop = GetClass().GetPropertyP (propertyName.c_str());
    auto adapter = nullptr != prop ? prop->GetTypeAdapter() : nullptr;
    if (nullptr == adapter)
        {
        // Preserve old behavior of converting directly to string, because ECObjects.dll doesn't know how to use type adapters...
        ecValue.SetUtf8CP (displayLabel, false);
        }
    else
        {
        // TFS#244646: Use type adapters to do conversion
        auto context = nullptr != adapter ? IECTypeAdapterContext::Create (*prop, *this) : nullptr;
        if (context.IsNull() || !adapter->ConvertFromString (ecValue, displayLabel, *context))
            return ECObjectsStatus::Error;
        }

    return SetValue (propertyName.c_str(), ecValue);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Bill.Steinbock                  05/2011
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus                 IECInstance::SetDisplayLabel (Utf8CP displayLabel)    
    {
    return  _SetDisplayLabel (displayLabel);
    }

///////////////////////////////////////////////////////////////////////////////////////////////
//   IECRelationshipInstance
///////////////////////////////////////////////////////////////////////////////////////////////
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Bill.Steinbock                  08/2010
+---------------+---------------+---------------+---------------+---------------+------*/
void                            IECRelationshipInstance::SetSource (IECInstanceP instance)
    {
    _SetSource (instance);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Bill.Steinbock                  08/2010
+---------------+---------------+---------------+---------------+---------------+------*/
IECInstancePtr                  IECRelationshipInstance::GetSource () const
    {
    return _GetSource ();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Bill.Steinbock                  08/2010
+---------------+---------------+---------------+---------------+---------------+------*/
void                            IECRelationshipInstance::SetTarget (IECInstanceP instance)
    {
    _SetTarget (instance);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Bill.Steinbock                  08/2010
+---------------+---------------+---------------+---------------+---------------+------*/
IECInstancePtr                  IECRelationshipInstance::GetTarget () const
    {
    return _GetTarget ();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Bill.Steinbock                  05/2011
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus                 IECRelationshipInstance::GetSourceOrderId (int64_t& sourceOrderId) const
    {
    return _GetSourceOrderId (sourceOrderId);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Bill.Steinbock                  05/2011
+---------------+---------------+---------------+---------------+---------------+------*/
 ECObjectsStatus                IECRelationshipInstance::GetTargetOrderId (int64_t& targetOrderId) const
    {
    return _GetTargetOrderId (targetOrderId);
    }

///////////////////////////////////////////////////////////////////////////////////////////////
//   IECWipRelationshipInstance
///////////////////////////////////////////////////////////////////////////////////////////////
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Bill.Steinbock                  04/2011
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus                   IECWipRelationshipInstance::SetName (Utf8CP name)   
    {
    return _SetName (name);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Bill.Steinbock                  04/2011
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus                   IECWipRelationshipInstance::SetSourceOrderId (int64_t sourceOrderId)
    {
    return _SetSourceOrderId (sourceOrderId);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Bill.Steinbock                  04/2011
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus                   IECWipRelationshipInstance::SetTargetOrderId (int64_t targetOrderId)
    {
    return _SetTargetOrderId (targetOrderId);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   04/10
+---------------+---------------+---------------+---------------+---------------+------*/
void              convertByteArrayToString (Utf8StringR outString, const Byte *byteData, size_t numBytes)
    {
    static const wchar_t    base64Chars[] = {L"ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/"};

    WString tmpString;
    outString.clear();

    // from each 3 bytes we get 4 output characters, rounded up.
    for (size_t iByte=0; iByte < numBytes; iByte += 3)
        {
        uint32_t    nextThreeBytes = byteData[iByte] | (byteData[iByte+1] << 8) | (byteData[iByte+2] << 16);

        for (size_t jPos=0; jPos < 4; jPos++)
            {
            Byte sixBits = nextThreeBytes & 0x3f;

            if ( (iByte + jPos) < (numBytes + 1) )
                tmpString.append (1, base64Chars[sixBits]);
            else
                tmpString.append (1, L'=');
            
            nextThreeBytes = nextThreeBytes >> 6;
            }
        }
    BeStringUtilities::WCharToUtf8(outString, tmpString.c_str());
    }

typedef bvector<Byte>   T_ByteArray;
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   04/10
+---------------+---------------+---------------+---------------+---------------+------*/
bool   convertStringToByteArray (T_ByteArray& byteData, Utf8CP stringData)
    {
    WString str;
    str.AssignUtf8(stringData);

    // the length of stringData should be a multiple of four.
    size_t  stringLen = wcslen (str.c_str());
    if (0 != (stringLen % 4))
        return false;

    // from each 4 characters we get 3 byte values.
    for (size_t iPos=0; iPos < stringLen; iPos+= 4)
        {
        int32_t nextThreeBytes = 0;
        int     numBytesToPush = 3;
        int     shift;
        int     jPos;
        for (jPos=0, shift=0; jPos < 4; jPos++, shift += 6)
            {
            wchar_t charValue = str[iPos+jPos];
            if ( (charValue >= L'A') && (charValue <= L'Z') )
                nextThreeBytes |= ((charValue - L'A') << shift);
            else if ((charValue >= L'a') && (charValue <= L'z') )
                nextThreeBytes |= ( ((charValue - L'a') + 26) << shift);
            else if ((charValue >= L'0') && (charValue <= L'9') )
                nextThreeBytes |= ( ((charValue - L'0') + 52) << shift);
            else if (charValue == L'+')
                nextThreeBytes |= ( 62 << shift);
            else if (charValue == L'/')
                nextThreeBytes |= ( 63 << shift);
            else if (charValue == L'=')
                {
                // = should only appear in the last two characters of the string.
                if (stringLen - (iPos + jPos) > 2)
                    return false;
                numBytesToPush = jPos-1;
                break;
                }
            else
                {
                return false;
                }
            }

        Byte*   bytes = (Byte*)&nextThreeBytes;
        byteData.push_back (*bytes);
        if (numBytesToPush > 1)
            byteData.push_back (*(bytes+1));
        if (numBytesToPush > 2)
            byteData.push_back (*(bytes+2));
        }

    return true;
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Colin.Kerr                     12/15
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

#define INSTANCEID_ATTRIBUTE         "instanceID"
#define SOURCECLASS_ATTRIBUTE        "sourceClass"
#define SOURCEINSTANCEID_ATTRIBUTE   "sourceInstanceID"
#define TARGETCLASS_ATTRIBUTE        "targetClass"
#define TARGETINSTANCEID_ATTRIBUTE   "targetInstanceID"
#define XMLNS_ATTRIBUTE              "xmlns"
#define XSI_NIL_ATTRIBUTE            "nil"

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   05/10
+---------------+---------------+---------------+---------------+---------------+------*/
static Utf8CP                   GetPrimitiveTypeString (PrimitiveType primitiveType)
    {
    switch (primitiveType)
        {
        case PRIMITIVETYPE_Binary:
            return "binary";

        case PRIMITIVETYPE_Boolean:
            return "boolean";

        case PRIMITIVETYPE_DateTime:
            return "dateTime";

        case PRIMITIVETYPE_Double:
            return "double";

        case PRIMITIVETYPE_Integer:
            return "int";

        case PRIMITIVETYPE_Long:
            return "long";

        case PRIMITIVETYPE_Point2D:
            return "point2d";

        case PRIMITIVETYPE_Point3D:
            return "point3d";

        case PRIMITIVETYPE_String:
            return "string";
        }

    BeAssert (false);
    return "";
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                               
+---------------+---------------+---------------+---------------+---------------+------*/
static BentleyStatus    LogXmlLoadError ()
    {        
    WString     errorString;
    BeXmlDom::GetLastErrorString (errorString);
    LOG.errorv (errorString.c_str());

    return SUCCESS;
    }

// =====================================================================================
// InstanceXMLReader class
// =====================================================================================
struct  InstanceXmlReader
{
private:
    Utf8String              m_fullSchemaName;
    BeXmlNodeR              m_xmlNode;
    ECSchemaCP              m_schema;
    ECInstanceReadContextR  m_context;
    Utf8String              m_className;

public:
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   10/2011
+---------------+---------------+---------------+---------------+---------------+------*/
InstanceXmlReader (ECInstanceReadContextR context, BeXmlNodeR xmlNode)
    : m_context (context), m_schema (NULL), m_xmlNode (xmlNode)
    {
    m_className = m_xmlNode.GetName ();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Basanta.Kharel                  10/2011
+---------------+---------------+---------------+---------------+---------------+------*/
InstanceXmlReader (ECInstanceReadContextR context, BeXmlNodeR xmlNode, Utf8String className)
    : m_context (context), m_schema (NULL), m_xmlNode (xmlNode)
    {
    m_className = className;
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                         Carole.MacDonald       03/15
//+---------------+---------------+---------------+---------------+---------------+----
ECSchemaCP GetSchema(Utf8String schemaName)
    {
    SchemaKey key;
    if (ECObjectsStatus::Success != SchemaKey::ParseSchemaFullName(key, schemaName.c_str()))
        return NULL;

    return m_context.FindSchemaCP(key, SchemaMatchType::LatestMajorCompatible);//Abeesh: Preserving old behavior. Ideally it should be exact 
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   10/2011
+---------------+---------------+---------------+---------------+---------------+------*/
ECSchemaCP       GetSchema()
    {
    if (NULL != m_schema)
        return m_schema;
    
    SchemaKey key;
    if (ECObjectsStatus::Success != SchemaKey::ParseSchemaFullName(key, m_fullSchemaName.c_str()))
        return NULL;
    
    m_schema = m_context.FindSchemaCP(key, SchemaMatchType::LatestMajorCompatible);//Abeesh: Preserving old behavior. Ideally it should be exact 
    return m_schema; 
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                         Carole.MacDonald       03/15
//+---------------+---------------+---------------+---------------+---------------+----
StandaloneECInstancePtr CreateConstraintInstance( Utf8String className, Utf8String instanceId, ECSchemaCP defaultSchema)
    {
    // Classnames might be qualified by a schema name.
    Utf8String constraintSchemaName;
    Utf8String constraintClassName;
    if (ECObjectsStatus::Success != ECClass::ParseClassName (constraintSchemaName, constraintClassName, className))
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

    ECClassCP constraintClass = constraintSchema->GetClassCP (constraintClassName.c_str());
    if (nullptr == constraintClass)
        {
        LOG.errorv("Invalid ECSchemaXML: The ECRelationshipConstraint contains a classname attribute with the value '%s' that can not be resolved to an ECClass named '%s' in the ECSchema '%s'",
            className.c_str(), constraintClassName.c_str(), constraintSchema->GetName().c_str());
        return nullptr;
        }

    auto constraintInstance= constraintClass->GetDefaultStandaloneEnabler()->CreateInstance();
    constraintInstance->SetInstanceId(instanceId.c_str());

    return constraintInstance;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   10/2011
+---------------+---------------+---------------+---------------+---------------+------*/
InstanceReadStatus      ReadInstance (IECInstancePtr& ecInstance)
    {
    // When this is called, m_xmlNode should be a BeXmlNode that is the class name, with a name space corresponding to the schema name.
    InstanceReadStatus      ixrStatus;
    ECClassCP               ecClass;
    if (InstanceReadStatus::Success != (ixrStatus = GetInstance (ecClass, ecInstance)))
        return ixrStatus;

    // this reads the property members and consumes the XmlNodeType_EndElement corresponding to this XmlNodeType_Element.
    return ReadInstanceOrStructMembers (*ecClass, ecInstance.get(), NULL, m_xmlNode);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   10/2011
+---------------+---------------+---------------+---------------+---------------+------*/
InstanceReadStatus      GetInstance (ECClassCP& ecClass, IECInstancePtr& ecInstance)
    {
    ECSchemaCP schema = NULL;
    
    // get the xmlns name, if there is one.
    Utf8CP  schemaName;
    if (NULL != (schemaName = m_xmlNode.GetNamespace()) && 0 != BeStringUtilities::Strnicmp (schemaName, ECXML_URI, strlen(ECXML_URI)))
        {
        m_fullSchemaName = schemaName;
        schema = GetSchema ();
        }
    else
        schema = &(m_context.GetFallBackSchema ());

    if (NULL == schema)
        {
        LOG.errorv ("Failed to locate ECSchema %s", m_fullSchemaName.c_str());
        return InstanceReadStatus::ECSchemaNotFound;
        }

    // see if we can find the class from the schema.
    m_context.ResolveSerializedClassName (m_className, *schema);
    ECClassCP    foundClass;
    if (NULL == (foundClass = schema->GetClassCP (m_className.c_str ())))
        {
        ECSchemaReferenceListCR refList = schema->GetReferencedSchemas();
        SchemaKey key;
        if (ECObjectsStatus::Success == SchemaKey::ParseSchemaFullName (key, m_fullSchemaName.c_str()))
            {
            ECSchemaReferenceList::const_iterator schemaIterator = refList.find (key);
            if (schemaIterator != refList.end())
                foundClass = schemaIterator->second->GetClassCP (m_className.c_str ());
            }
        else
            {
            for (ECSchemaReferenceList::const_iterator schemaIterator = refList.begin(); schemaIterator != refList.end(); schemaIterator++)
                {
                if (NULL != (foundClass = schemaIterator->second->GetClassCP (m_className.c_str ())))
                    break;
                }
            }
        }
    if (NULL == foundClass)
        {
        LOG.errorv ("Failed to find ECClass %s in %s", m_className.c_str (), m_fullSchemaName.c_str ());
        return InstanceReadStatus::ECClassNotFound;
        }

    ecClass = foundClass;

    ecInstance = m_context.CreateStandaloneInstance (*foundClass).get();
    
    Utf8String instanceId;
    if (BEXML_Success == m_xmlNode.GetAttributeStringValue (instanceId, INSTANCEID_ATTRIBUTE))
        {
        ecInstance->SetInstanceId (instanceId.c_str());
        }

    IECRelationshipInstance*    relationshipInstance = dynamic_cast <IECRelationshipInstance*> (ecInstance.get());

    // if relationship, need the attributes used in relationships.
    if (NULL != relationshipInstance)
        {
        // see if we can find the attributes corresponding to the relationship instance ids.
        Utf8String sourceInstanceId;
        if (BEXML_Success != m_xmlNode.GetAttributeStringValue (sourceInstanceId, SOURCEINSTANCEID_ATTRIBUTE))
            LOG.warning ("Source InstanceId not set on serialized relationship instance");

        Utf8String sourceClassName;
        if (BEXML_Success != m_xmlNode.GetAttributeStringValue (sourceClassName, SOURCECLASS_ATTRIBUTE))
            LOG.warning ("Source className not set on serialized relationship instance");

        Utf8String targetInstanceId;
        if (BEXML_Success != m_xmlNode.GetAttributeStringValue (targetInstanceId, TARGETINSTANCEID_ATTRIBUTE))
            LOG.warning ("Target InstanceId not set on serialized relationship instance");

        Utf8String targetClassName;
        if (BEXML_Success != m_xmlNode.GetAttributeStringValue (targetClassName, TARGETCLASS_ATTRIBUTE))
            LOG.warning ("Target className not set on serialized relationship instance");

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
* @bsimethod                                    Barry.Bentley                   10/2011
+---------------+---------------+---------------+---------------+---------------+------*/
InstanceReadStatus   ReadInstanceOrStructMembers (ECClassCR ecClass, IECInstanceP ecInstance, Utf8String* baseAccessString, BeXmlNodeR instanceNode)
    {
    // On entry, the instanceNode is the XML node that contains the children that have propertyValues.
    for (BeXmlNodeP propertyValueNode = instanceNode.GetFirstChild (BEXMLNODE_Element); NULL != propertyValueNode; propertyValueNode = propertyValueNode->GetNextSibling(BEXMLNODE_Element))
        {
        InstanceReadStatus   propertyStatus = ReadPropertyValue (ecClass, ecInstance, baseAccessString, *propertyValueNode);

        // if property not found, ReadPropertyValue warns, so just continue..
        if (InstanceReadStatus::PropertyNotFound == propertyStatus)
            continue;
        else if (InstanceReadStatus::TypeMismatch == propertyStatus)
            continue;
        else if (InstanceReadStatus::Success != propertyStatus)
            return propertyStatus;
        }

    return InstanceReadStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   10/2011
+---------------+---------------+---------------+---------------+---------------+------*/
InstanceReadStatus   ReadPropertyValue (ECClassCR ecClass, IECInstanceP ecInstance, Utf8String* baseAccessString, BeXmlNodeR propertyValueNode)
    {
    // on entry, propertyValueNode is the XML node for the property value.
    Utf8String     propertyName (propertyValueNode.GetName());
    m_context.ResolveSerializedPropertyName (propertyName, ecClass);

    // try to find the property in the class.
    ECPropertyP ecProperty;
    if (NULL == (ecProperty = ecClass.GetPropertyP (propertyName)))
        {
        LOG.errorv("No ECProperty '%s' found in ECClass '%s'. Value will be ignored.", propertyName.c_str(), ecClass.GetName().c_str());
        return InstanceReadStatus::PropertyNotFound;
        }

    PrimitiveECPropertyP    primitiveProperty;
    ArrayECPropertyP        arrayProperty;
    StructECPropertyP       structProperty;
    NavigationECPropertyP   navigationProperty;
    if (NULL != (primitiveProperty = ecProperty->GetAsPrimitivePropertyP()))
        return ReadPrimitivePropertyValue (primitiveProperty, ecInstance, baseAccessString, propertyValueNode);
                //Above is good, if SkipToElementEnd() is returned from ReadPrimitiveValue.
    else if (NULL != (arrayProperty = ecProperty->GetAsArrayPropertyP()))
        return ReadArrayPropertyValue (arrayProperty, ecInstance, baseAccessString, propertyValueNode);
    else if (NULL != (structProperty = ecProperty->GetAsStructPropertyP()))
        return ReadEmbeddedStructPropertyValue(structProperty, ecInstance, baseAccessString, propertyValueNode);
    else if (nullptr != (navigationProperty = ecProperty->GetAsNavigationPropertyP()))
        return ReadNavigationPropertyValue(navigationProperty, ecInstance, baseAccessString, propertyValueNode);

    // should be one of those!
    BeAssert (false);
    return InstanceReadStatus::BadECProperty;
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Colin.Kerr                     12/15
//---------------+---------------+---------------+---------------+---------------+------
InstanceReadStatus  ReadSimplePropertyValue(Utf8StringCR propertyName, PrimitiveType propertyType, Utf8StringP baseAccessString, IECInstanceP ecInstance, BeXmlNodeR propertyValueNode, PrimitiveType serializedType)
    {
    // on entry, propertyValueNode is the xml node for the primitive property value.
    InstanceReadStatus   ixrStatus;
    ECValue              ecValue;
    if (InstanceReadStatus::Success != (ixrStatus = ReadPrimitiveValue(ecValue, propertyType, propertyValueNode, serializedType)))
        return ixrStatus;

    if (ecValue.IsUninitialized())
        {
        //A malformed value was found.  A warning was shown; just move on.
        return InstanceReadStatus::Success;
        }

    ECObjectsStatus setStatus;
    Utf8String accessString;
    CreateAccessString(accessString, baseAccessString, propertyName);
    setStatus = ecInstance->SetInternalValue(accessString.c_str(), ecValue);

    if (ECObjectsStatus::Success != setStatus && ECObjectsStatus::PropertyValueMatchesNoChange != setStatus)
        LOG.warningv("Unable to set value for property %s", propertyName.c_str());

    BeAssert(ECObjectsStatus::Success == setStatus || ECObjectsStatus::PropertyValueMatchesNoChange == setStatus);

    return InstanceReadStatus::Success;
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Colin.Kerr                     01/16
//---------------+---------------+---------------+---------------+---------------+------
InstanceReadStatus ReadPrimitiveArrayValues(IECInstanceP ecInstance, Utf8StringCR accessString, PrimitiveType memberType, PrimitiveType serializedMemberType, bool isFixedSizeArray, BeXmlNodeR propertyValueNode)
    {
    // start the address out as zero.
    uint32_t    index = 0;

    // step through the nodes. Each should be a primitive value type like <int>value</int>
    for (BeXmlNodeP arrayValueNode = propertyValueNode.GetFirstChild(BEXMLNODE_Element); NULL != arrayValueNode; arrayValueNode = arrayValueNode->GetNextSibling(BEXMLNODE_Element))
        {
        if (memberType == serializedMemberType && !ValidateArrayPrimitiveType(arrayValueNode->GetName(), memberType))
            {
            LOG.warningv("Incorrectly formatted array element found in array %s.  Expected: %s  Found: %s",
                         accessString.c_str(), GetPrimitiveTypeString(memberType), arrayValueNode->GetName());
            continue;
            }

        if (!isFixedSizeArray)
            ecInstance->AddArrayElements(accessString.c_str(), 1);

        // read it, populating the ECInstance using accessString and arrayIndex.
        InstanceReadStatus      ixrStatus;
        ECValue                 ecValue;
        if (InstanceReadStatus::Success == (ixrStatus = ReadPrimitiveValue(ecValue, memberType, *arrayValueNode, serializedMemberType)))
            {
            // If we failed to read the value above, the array member will have been allocated but left null.
            // This allows any default value to be applied to it via CalculatedECPropertySpecification, 
            // and is less surprising than the old behavior which would have omitted the member entirely.
            ECObjectsStatus   setStatus = ecInstance->SetInternalValue(accessString.c_str(), ecValue, index);
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
// @bsimethod                                    Colin.Kerr                     12/15
//---------------+---------------+---------------+---------------+---------------+------
InstanceReadStatus  ReadNavigationPropertyValue(NavigationECPropertyP navigationProperty, IECInstanceP ecInstance, Utf8StringP baseAccessString, BeXmlNodeR propertyValueNode)
    {
    // We always input string as the serialized type to handle the case where the instance is loaded in an environment where the type is different than when serialized
    if (navigationProperty->IsMultiple())
        {
        Utf8String accessString;
        CreateAccessString(accessString, baseAccessString, navigationProperty->GetName());
        return ReadPrimitiveArrayValues(ecInstance, accessString, navigationProperty->GetType(), PrimitiveType::PRIMITIVETYPE_String, false, propertyValueNode);
        }
    else
        return ReadSimplePropertyValue(navigationProperty->GetName(), navigationProperty->GetType(), baseAccessString, ecInstance, propertyValueNode, PrimitiveType::PRIMITIVETYPE_String);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   10/2011
+---------------+---------------+---------------+---------------+---------------+------*/
InstanceReadStatus   ReadPrimitivePropertyValue (PrimitiveECPropertyP primitiveProperty, IECInstanceP ecInstance, Utf8String* baseAccessString, BeXmlNodeR propertyValueNode)
    {
    return ReadSimplePropertyValue(primitiveProperty->GetName(), primitiveProperty->GetType(), baseAccessString, ecInstance, propertyValueNode, m_context.GetSerializedPrimitiveType(*primitiveProperty));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   10/2011
+---------------+---------------+---------------+---------------+---------------+------*/
InstanceReadStatus   ReadArrayPropertyValue (ArrayECPropertyP arrayProperty, IECInstanceP ecInstance, Utf8String* baseAccessString, BeXmlNodeR propertyValueNode)
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
        PrimitiveType   memberType = arrayProperty->GetPrimitiveElementType();
        PrimitiveType serializedMemberType = m_context.GetSerializedPrimitiveArrayType(*arrayProperty);

        bool            isFixedSizeArray = false;

        if (arrayProperty->GetMinOccurs() == arrayProperty->GetMaxOccurs())
            isFixedSizeArray = true;

        InstanceReadStatus status = ReadPrimitiveArrayValues(ecInstance, accessString, memberType, serializedMemberType, isFixedSizeArray, propertyValueNode);
        if (InstanceReadStatus::Success != status)
            return status;
        }

    else if (ARRAYKIND_Struct == arrayKind)
        {
        StructArrayECPropertyP structArray = arrayProperty->GetAsStructArrayPropertyP();
        ECClassCP   structMemberType = structArray->GetStructElementType();
        for (BeXmlNodeP arrayValueNode = propertyValueNode.GetFirstChild (BEXMLNODE_Element); NULL != arrayValueNode; arrayValueNode = arrayValueNode->GetNextSibling(BEXMLNODE_Element))
            {
            // the Name of each node element is the class name of structMemberType.
            // For polymorphic arrays, the Name might also be the name of a class that has structMemberType as a BaseType.
            ECClassCP   thisMemberType;
            Utf8String  arrayMemberType (arrayValueNode->GetName());
            m_context.ResolveSerializedClassName (arrayMemberType, structMemberType->GetSchema());
            if (nullptr == (thisMemberType = ValidateArrayStructType (arrayMemberType.c_str (), structMemberType)))
                {
                LOG.warningv ("Incorrect structType found in %s.  Expected: %s  Found: %s",
                    accessString.c_str (), structMemberType->GetName().c_str(), arrayValueNode->GetName ());
                continue;
                }

            InstanceReadStatus ixrStatus;
            if (InstanceReadStatus::Success != (ixrStatus = ReadStructArrayMember (*thisMemberType, ecInstance, accessString, index, *arrayValueNode)))
                return ixrStatus;

            // increment the array index.
            index++;
            }
        }
    
    return InstanceReadStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   10/2011
+---------------+---------------+---------------+---------------+---------------+------*/
InstanceReadStatus   ReadEmbeddedStructPropertyValue (StructECPropertyP structProperty, IECInstanceP ecInstance, Utf8String* baseAccessString, BeXmlNodeR propertyValueNode)
    {
    // empty element OK for struct - all members are null.
    Utf8String    thisAccessString;
    CreateAccessString(thisAccessString, baseAccessString, structProperty->GetName());
    thisAccessString.append (".");

    ICustomECStructSerializerP customECStructSerializerP = CustomStructSerializerManager::GetManager().GetCustomSerializer (structProperty, *ecInstance);
    if (customECStructSerializerP)
        return ReadCustomSerializedStruct (structProperty, ecInstance, baseAccessString, customECStructSerializerP, propertyValueNode);

    return ReadInstanceOrStructMembers (structProperty->GetType(), ecInstance, &thisAccessString, propertyValueNode);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Bill.Steinbock                  10/2010
+---------------+---------------+---------------+---------------+---------------+------*/
InstanceReadStatus   ReadCustomSerializedStruct (StructECPropertyP structProperty, IECInstanceP ecInstance, Utf8String* baseAccessString, ICustomECStructSerializerP customECStructSerializerP, BeXmlNodeR propertyValueNode)
    {
    Utf8String propertyValueString;

    // empty?
    if (BEXML_Success != propertyValueNode.GetContent (propertyValueString))
        return InstanceReadStatus::Success;

    Utf8String    thisAccessString;
    CreateAccessString(thisAccessString, baseAccessString, structProperty->GetName());
    thisAccessString.append (".");

    customECStructSerializerP->LoadStructureFromString (structProperty, *ecInstance, thisAccessString.c_str(), propertyValueString.c_str());

    return InstanceReadStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   10/2011
+---------------+---------------+---------------+---------------+---------------+------*/
#if defined (_MSC_VER)
    #pragma warning(disable:4189) // setStatus unused if NDEBUG set.
#endif // defined (_MSC_VER)

InstanceReadStatus   ReadStructArrayMember (ECClassCR structClass, IECInstanceP owningInstance, Utf8String& accessString, uint32_t index, BeXmlNodeR arrayMemberValue)
    {
    // On entry, arrayMemberValue is an XML Node for the element that starts the struct.

    // Create an IECInstance for the array member.
    IECInstancePtr      structInstance = m_context.CreateStandaloneInstance (structClass).get();

    InstanceReadStatus   ixrStatus;
    if (InstanceReadStatus::Success != (ixrStatus = ReadInstanceOrStructMembers (structClass, structInstance.get(), NULL, arrayMemberValue)))
        return ixrStatus;

    // every StructArrayMember is a new ECInstance, 
    // set the value 
    ECValue structValue;
    structValue.SetStruct (structInstance.get());

    if (!IECInstance::IsFixedArrayProperty (*owningInstance, accessString.c_str()))
        {
        // add the value to the array.
        owningInstance->AddArrayElements (accessString.c_str(), 1);
        }

    ECObjectsStatus setStatus = owningInstance->SetInternalValue (accessString.c_str(), structValue, index);
    if (ECObjectsStatus::Success != setStatus)
        BeAssert (ECObjectsStatus::Success == setStatus);

    return InstanceReadStatus::Success;
    }

#if defined (_MSC_VER)
    #pragma warning(default:4189)
#endif // defined (_MSC_VER)

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   10/2011
+---------------+---------------+---------------+---------------+---------------+------*/
InstanceReadStatus   ReadPrimitiveValue (ECValueR ecValue, PrimitiveType propertyType, BeXmlNodeR primitiveValueNode, PrimitiveType serializedType)
    {
    // If we fail to read the property value for some reason, return it as null
    ecValue.SetToNull();
    ecValue.SetPrimitiveType (propertyType);

    // On entry primitiveValueNode is the XML node that holds the value. 
    // First check to see if the value is set to NULL
    bool         nullValue;
    if (BEXML_Success == primitiveValueNode.GetAttributeBooleanValue (nullValue, XSI_NIL_ATTRIBUTE))
        if (true == nullValue)
            return InstanceReadStatus::Success;

    // If we're able to determine that the serialized type differs from the ECProperty's type, use that information to convert to correct type
    if (serializedType != propertyType)
        {
        Utf8String propertyValueString;
        if (BEXML_Success == primitiveValueNode.GetContent (propertyValueString))
            {
            ecValue.SetUtf8CP (propertyValueString.c_str(), false);
            if (ecValue.ConvertToPrimitiveType (serializedType) && ecValue.ConvertToPrimitiveType (propertyType))
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
            Utf8String     propertyValueString;
            if (BEXML_Success != primitiveValueNode.GetContent (propertyValueString))
                return InstanceReadStatus::Success;

            if (!convertStringToByteArray (byteArray, propertyValueString.c_str ()))
                {
                LOG.errorv("Type mismatch in deserialization: \"%s\" is not Binary", propertyValueString.c_str ());
                return InstanceReadStatus::TypeMismatch;
                }
            if (!byteArray.empty())
                ecValue.SetBinary (&byteArray.front(), byteArray.size(), true);
            else
                ecValue.SetBinary (NULL, 0, true);
            break;
            }

        case PRIMITIVETYPE_IGeometry:
            {
            T_ByteArray                     byteArray;
            // try to read the actual value.
            Utf8String     propertyValueString;
            if (BEXML_Success != primitiveValueNode.GetContent (propertyValueString))
                return InstanceReadStatus::Success;

            // It is possible that this came in as serialized text Xml, and not binary.  Let's try to get it as xmlText
            if (!convertStringToByteArray (byteArray, propertyValueString.c_str ()))
                {
                if (NULL != primitiveValueNode.GetFirstChild())
                    {
                    Utf8String xmlString;
                    primitiveValueNode.GetFirstChild()->GetXmlString(xmlString);
                    bvector<IGeometryPtr> geoms;
                    bmap<OrderedIGeometryPtr, BeExtendedData> extendedData;
                    if (BeXmlCGStreamReader::TryParse(xmlString.c_str(), geoms, extendedData, 0))
                        {
                        ecValue.SetIGeometry(*geoms[0]);
                        break;
                        }
                    }
                else
                    {
                    bvector<IGeometryPtr> geoms;
                    bmap<OrderedIGeometryPtr, BeExtendedData> extendedData;
                    if (BeXmlCGStreamReader::TryParse(Utf8String(propertyValueString.c_str()).c_str(), geoms, extendedData, 0))
                        {
                        ecValue.SetIGeometry(*geoms[0]);
                        break;
                        }
                    }
                LOG.errorv("Type mismatch in deserialization: \"%s\" is not Binary", propertyValueString.c_str ());
                return InstanceReadStatus::TypeMismatch;
                }
            ecValue.SetIGeometry(&byteArray.front(), byteArray.size(), true);
            break;
            }
        case PRIMITIVETYPE_Boolean:
            {
            bool boolValue;
            BeXmlStatus status = primitiveValueNode.GetContentBooleanValue (boolValue);
            if (BEXML_Success != status)
                {
                if (BEXML_ContentWrongType == status)
                    return InstanceReadStatus::TypeMismatch;
                if (BEXML_NullNodeValue == status || BEXML_NodeNotFound == status)
                    ecValue.SetToNull ();
                return InstanceReadStatus::Success;
                }

            ecValue.SetBoolean (boolValue);
            break;
            }

        case PRIMITIVETYPE_DateTime:
            {
            int64_t ticks;
            BeXmlStatus status = primitiveValueNode.GetContentInt64Value (ticks);
            if (BEXML_Success != status)
                {
                if (BEXML_ContentWrongType == status)
                    return InstanceReadStatus::TypeMismatch;
                if (BEXML_NullNodeValue == status || BEXML_NodeNotFound == status)
                    ecValue.SetToNull ();
                return InstanceReadStatus::Success;
                }

            ecValue.SetDateTimeTicks (ticks);
            break;
            }

        case PRIMITIVETYPE_Double:
            {
            double  doubleValue;
            BeXmlStatus status = primitiveValueNode.GetContentDoubleValue (doubleValue);
            if (BEXML_Success != status)
                {
                if (BEXML_ContentWrongType == status)
                    return InstanceReadStatus::TypeMismatch;
                if (BEXML_NullNodeValue == status || BEXML_NodeNotFound == status)
                    ecValue.SetToNull ();
                return InstanceReadStatus::Success;
                }

            ecValue.SetDouble (doubleValue);
            break;
            }

        case PRIMITIVETYPE_Integer:
            {
            int32_t intValue;
            BeXmlStatus status = primitiveValueNode.GetContentInt32Value (intValue);
            if (BEXML_Success != status)
                {
                if (BEXML_ContentWrongType == status)
                    return InstanceReadStatus::TypeMismatch;
                if (BEXML_NullNodeValue == status || BEXML_NodeNotFound == status)
                    ecValue.SetToNull ();
                return InstanceReadStatus::Success;
                }
            ecValue.SetInteger (intValue);
            break;
            }

        case PRIMITIVETYPE_Long:
            {
            int64_t longValue;
            BeXmlStatus status = primitiveValueNode.GetContentInt64Value (longValue);
            if (BEXML_Success != status)
                {
                if (BEXML_ContentWrongType == status)
                    return InstanceReadStatus::TypeMismatch;
                if (BEXML_NullNodeValue == status || BEXML_NodeNotFound == status)
                    ecValue.SetToNull ();
                return InstanceReadStatus::Success;
                }

            ecValue.SetLong (longValue);
            break;
            }

        case PRIMITIVETYPE_Point2D:
            {
            double x, y;
            BeXmlStatus status = primitiveValueNode.GetContentDPoint2dValue (x, y);
            if (BEXML_Success != status)
                {
                if (BEXML_ContentWrongType == status)
                    return InstanceReadStatus::TypeMismatch;
                if (BEXML_NullNodeValue == status || BEXML_NodeNotFound == status)
                    ecValue.SetToNull ();
                return InstanceReadStatus::Success;
                }

            DPoint2d point2d;
            point2d.x = x;
            point2d.y = y;
            ecValue.SetPoint2D (point2d);
            break;
            }

        case PRIMITIVETYPE_Point3D:
            {
            double x, y, z;
            BeXmlStatus status = primitiveValueNode.GetContentDPoint3dValue (x, y, z);
            if (BEXML_Success != status)
                {
                if (BEXML_ContentWrongType == status)
                    return InstanceReadStatus::TypeMismatch;
                if (BEXML_NullNodeValue == status || BEXML_NodeNotFound == status)
                    ecValue.SetToNull ();
                return InstanceReadStatus::Success;
                }

            DPoint3d point3d;
            point3d.x = x;
            point3d.y = y;
            point3d.z = z;
            ecValue.SetPoint3D (point3d);
            break;
            }

        case PRIMITIVETYPE_String:
            {
            Utf8String     propertyValueString;
            BeXmlStatus status = primitiveValueNode.GetContent (propertyValueString);
            if (BEXML_Success != status)
                return InstanceReadStatus::Success;
            ecValue.SetUtf8CP (propertyValueString.c_str ());
            break;
            }

        default:
            {
            BeAssert (false);
            return InstanceReadStatus::BadPrimitivePropertyType;
            }
        }
    
    return InstanceReadStatus::Success;
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   04/10
+---------------+---------------+---------------+---------------+---------------+------*/
bool                            ValidateArrayPrimitiveType (Utf8CP typeFound, PrimitiveType expectedType)
    {
    return (0 == strcmp (typeFound, GetPrimitiveTypeString (expectedType)));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   04/10
+---------------+---------------+---------------+---------------+---------------+------*/
ECClassCP                       ValidateArrayStructType (Utf8CP typeFound, ECClassCP expectedType)
    {
    // the common case is that they're all of the expected ECClass.
    if (0 == strcmp (typeFound, expectedType->GetName().c_str()))
        return expectedType;

    ECSchemaCP  schema = GetSchema();
    if (NULL == schema)
        return NULL;

    // typeFound must resolve to an ECClass that is either expectedType or a class that has expectedType as a Base GetClass().
    ECClassCP    classFound;
    if (NULL == (classFound = schema->GetClassCP (typeFound)) || !classFound->Is (expectedType))
        return NULL;

    return classFound;
    }
};

/*---------------------------------------------------------------------------------**//**
* In old unit schema some ecclass where treated as both struct and custom attribute
* EC 3.0 doesnot allow this and it deserializes those classes to struct by default.
* this class creates a customattribute for the same name with "Attr" suffix and loads 
* the customattributes from xmlnode.
* @bsistruct                                                    Basanta.Kharel   12/2015
+---------------+---------------+---------------+---------------+---------------+------*/
struct NamedAttributeDeserializer : ICustomAttributeDeserializer
    {
    private:
        Utf8String m_oldAttributeClassName;
        Utf8String m_newClassName;

        bool CreateCustomAttribute (ECSchemaReadContextR schemaContext, Utf8StringCR existingStructClassName, Utf8StringCR schemaName)
            {
            ECSchemaPtr schema = GetSchema (schemaName, schemaContext);
            if (schema.IsNull ())
                return false;

            ECClassCP structClass = schema->GetClassCP (existingStructClassName.c_str ());
            if (!structClass)
                {
                LOG.errorv ("Failed to inject customattribute class: \"%s\" to schema by copying struct class: \"%s\" which doesnot exist", m_newClassName.c_str(), existingStructClassName.c_str());
                BeAssert (false);
                return false;
                }

            ECCustomAttributeClassP attributeClass = NULL;
            if (ECObjectsStatus::Success != schema->CreateCustomAttributeClass (attributeClass, m_newClassName))
                return false;

            for (ECPropertyCP sourceProp : structClass->GetProperties (false))
                {
                ECPropertyP destProperty;
                attributeClass->CopyProperty (destProperty, sourceProp, true);
                }
            return true;
            }

        Utf8String GetSchemaName (BeXmlNodeR xmlNode)
            {
            // get the xmlns name, if there is one.
            Utf8CP  schemaName;
            Utf8String fullSchemaName = "";
            if (NULL != (schemaName = xmlNode.GetNamespace ()) && 0 != BeStringUtilities::Strnicmp (schemaName, ECXML_URI, strlen (ECXML_URI)))
                {
                fullSchemaName = schemaName;
                }
            return fullSchemaName;
            }

        bool ClassExists (Utf8StringCR className, Utf8StringCR schemaName, ECSchemaReadContextR schemaContext)
            {
            ECSchemaPtr schema = GetSchema (schemaName, schemaContext);
            if (!schema.IsValid())
                return false;
            ECClassCP ecClass = schema->GetClassCP (className.c_str ());
            if (ecClass)
                return true;
            return false;
            }

        ECSchemaPtr GetSchema (Utf8String schemaName, ECSchemaReadContextR context)
            {
            SchemaKey key;
            if (ECObjectsStatus::Success != SchemaKey::ParseSchemaFullName (key, schemaName.c_str ()))
                return NULL;

            return context.LocateSchema (key, SchemaMatchType::LatestMajorCompatible);//Abeesh: Preserving old behavior. Ideally it should be exact 
            }
        
    public:

        NamedAttributeDeserializer (Utf8String oldAttributeClassName, Utf8String newClassName)
            :m_oldAttributeClassName (oldAttributeClassName), m_newClassName (newClassName)
            {
            }

        InstanceReadStatus LoadCustomAttributeFromString (IECInstancePtr& ecInstance, BeXmlNodeR xmlNode, ECInstanceReadContextR context, ECSchemaReadContextR schemaContext, IECCustomAttributeContainerR customAttributeContainer)
            {
            Utf8String schemaName = GetSchemaName (xmlNode);

            bool attributeExists = true;
            if (!ClassExists (m_newClassName, schemaName, schemaContext))
                attributeExists = CreateCustomAttribute (schemaContext, m_oldAttributeClassName, schemaName);

            if (!attributeExists)
                return InstanceReadStatus::BadElement;

            InstanceXmlReader   reader (context, xmlNode, m_newClassName);
            return reader.ReadInstance (ecInstance);
            }
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Basanta.Kharel                 12/2015
+---------------+---------------+---------------+---------------+---------------+------*/
CustomAttributeDeserializerManager::CustomAttributeDeserializerManager ()
    {
    // we could add needed deserializers here.
    m_deserializers["UnitSpecification"] = new NamedAttributeDeserializer ("UnitSpecification", "UnitSpecificationAttr");
    m_deserializers["DisplayUnitSpecification"] = new NamedAttributeDeserializer ("DisplayUnitSpecification", "DisplayUnitSpecificationAttr");
    }

/*---------------------------------------------------------------------------------**//**
@bsimethod                                    Basanta.Kharel                 12/2015
+---------------+---------------+---------------+---------------+---------------+------*/
CustomAttributeDeserializerManager::~CustomAttributeDeserializerManager ()
    {
    m_deserializers.clear();
    }

/*---------------------------------------------------------------------------------**//**
@bsimethod                                    Basanta.Kharel                 12/2015
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus            CustomAttributeDeserializerManager::AddCustomDeserializer (Utf8CP deserializerName, ICustomAttributeDeserializerP deserializer)
    {
    if (GetCustomDeserializer (deserializerName))
        return ERROR;

    m_deserializers[Utf8String(deserializerName)] = deserializer;
    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
@bsimethod                                    Basanta.Kharel                 12/2015
+---------------+---------------+---------------+---------------+---------------+------*/
CustomAttributeDeserializerManagerR                   CustomAttributeDeserializerManager::GetManager ()
    {
    static CustomAttributeDeserializerManagerP   s_deserializerManager = NULL;

    if (NULL == s_deserializerManager)
        s_deserializerManager = new CustomAttributeDeserializerManager ();
        
    return *s_deserializerManager;
    }

/*---------------------------------------------------------------------------------**//**
@bsimethod                                    Basanta.Kharel                 12/2015
+---------------+---------------+---------------+---------------+---------------+------*/
ICustomAttributeDeserializerP                      CustomAttributeDeserializerManager::GetCustomDeserializer (Utf8CP deserializerName) const
    {
    if (m_deserializers.empty())
        return NULL;

    AttributeDeserializerMap::const_iterator it = m_deserializers.find (deserializerName);
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
* @bsimethod                                    Barry.Bentley                   05/10
+---------------+---------------+---------------+---------------+---------------+------*/
InstanceXmlWriter (BeXmlWriter *writer)
    : m_xmlWriter (writer)
    {
    writer->SetIndentation(4);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   10/11
+---------------+---------------+---------------+---------------+---------------+------*/
InstanceWriteStatus     WriteInstance (IECInstanceCR ecInstance, bool writeInstanceId, Utf8CP className)
    {
    ECClassCR   ecClass         = ecInstance.GetClass();
    ECSchemaCR  ecSchema        = ecClass.GetSchema();
    Utf8String  fullSchemaName = ecSchema.GetLegacyFullSchemaName();

    m_xmlWriter->WriteElementStart(className, fullSchemaName.c_str());

    auto relationshipInstance = dynamic_cast<IECRelationshipInstanceCP> (&ecInstance);
    // if relationship, need the attributes used in relationships.
    if (NULL != relationshipInstance)
        {
        if (!relationshipInstance->GetSource().IsValid())
            return InstanceWriteStatus::XmlWriteError;

        Utf8String sourceClassName;
        if (0 != relationshipInstance->GetSource()->GetClass().GetSchema().GetLegacyFullSchemaName().CompareTo(fullSchemaName))
            sourceClassName.Sprintf("%s:%s", relationshipInstance->GetSource()->GetClass().GetSchema().GetLegacyFullSchemaName().c_str(), relationshipInstance->GetSource()->GetClass().GetName().c_str());
        else
            sourceClassName.Sprintf("%s", relationshipInstance->GetSource()->GetClass().GetName().c_str());
        m_xmlWriter->WriteAttribute(SOURCEINSTANCEID_ATTRIBUTE, relationshipInstance->GetSource()->GetInstanceId().c_str());
        m_xmlWriter->WriteAttribute(SOURCECLASS_ATTRIBUTE, sourceClassName.c_str());

        if (!relationshipInstance->GetTarget().IsValid())
            return InstanceWriteStatus::XmlWriteError;

        Utf8String targetClassName;
        if (0 != relationshipInstance->GetTarget()->GetClass().GetSchema().GetLegacyFullSchemaName().CompareTo(fullSchemaName))
            targetClassName.Sprintf("%s:%s", relationshipInstance->GetTarget()->GetClass().GetSchema().GetLegacyFullSchemaName().c_str(), relationshipInstance->GetTarget()->GetClass().GetName().c_str());
        else
            targetClassName.Sprintf("%s", relationshipInstance->GetTarget()->GetClass().GetName().c_str());
        m_xmlWriter->WriteAttribute(TARGETINSTANCEID_ATTRIBUTE, relationshipInstance->GetTarget()->GetInstanceId().c_str());
        m_xmlWriter->WriteAttribute(TARGETCLASS_ATTRIBUTE, targetClassName.c_str());
        }

    if (writeInstanceId)
        m_xmlWriter->WriteAttribute(INSTANCEID_ATTRIBUTE, ecInstance.GetInstanceIdForSerialization().c_str());

    InstanceWriteStatus status = WritePropertyValuesOfClassOrStructArrayMember (ecClass, ecInstance, NULL);
    if (status != InstanceWriteStatus::Success)
        return status;
    m_xmlWriter->WriteElementEnd();
    return InstanceWriteStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Prasanna.Prakash                03/16
+---------------+---------------+---------------+---------------+---------------+------*/
InstanceWriteStatus     WriteInstance(IECInstanceCR ecInstance, bool writeInstanceId)
    {
    Utf8CP className = ecInstance.GetClass().GetName().c_str();
    
    return WriteInstance(ecInstance, writeInstanceId, className);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   04/10
+---------------+---------------+---------------+---------------+---------------+------*/
InstanceWriteStatus     WritePropertyValuesOfClassOrStructArrayMember (ECClassCR ecClass, IECInstanceCR ecInstance, Utf8String* baseAccessString)
    {
    CustomStructSerializerManagerR customStructSerializerMgr = CustomStructSerializerManager::GetManager();

    ECPropertyIterableCR    collection  = ecClass.GetProperties (true);
    for (ECPropertyP ecProperty: collection)
        {
        PrimitiveECPropertyP    primitiveProperty;
        ArrayECPropertyP        arrayProperty;
        StructECPropertyP       structProperty;
        NavigationECPropertyP   navProperty;
        InstanceWriteStatus     ixwStatus = InstanceWriteStatus::BadPrimitivePropertyType;
            
        if (NULL != (primitiveProperty = ecProperty->GetAsPrimitivePropertyP()))
            ixwStatus = WritePrimitivePropertyValue (*primitiveProperty, ecInstance, baseAccessString);
        else if (NULL != (arrayProperty = ecProperty->GetAsArrayPropertyP()))
            ixwStatus = WriteArrayPropertyValue (*arrayProperty, ecInstance, baseAccessString);
        else if (NULL != (structProperty = ecProperty->GetAsStructPropertyP()))
            {
            if (ecInstance.SaveOnlyLoadedPropertiesToXml())
                {
                // if the above flag is set then the instance sets "IsLoaded" flags for loaded properties and that "IsLoaded" flag is set in the ECValue for the property
                Utf8String    accessString;
                CreateAccessString(accessString, baseAccessString, structProperty->GetName());

                // no members, don't write anything.
                ECValue         ecValue;
                if (ECObjectsStatus::Success != ecInstance.GetValue (ecValue, accessString.c_str()) || !ecValue.IsLoaded())
                    continue;
                }

            ICustomECStructSerializerP customECStructSerializerP;
            if (NULL != (customECStructSerializerP = customStructSerializerMgr.GetCustomSerializer (structProperty, ecInstance)))
                {
                Utf8String     xmlString; 
                if (ECObjectsStatus::Success != customECStructSerializerP->GenerateXmlString (xmlString, structProperty, ecInstance, baseAccessString?baseAccessString->c_str():NULL))
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
                ixwStatus = WriteEmbeddedStructPropertyValue (*structProperty, ecInstance, baseAccessString);
                }
            }
        else if (nullptr != (navProperty = ecProperty->GetAsNavigationPropertyP()))
            {
            ixwStatus = WriteNavigationPropertyValue(*navProperty, ecInstance, baseAccessString);
            }

        if (InstanceWriteStatus::Success != ixwStatus)
            {
            BeAssert (false);
            return ixwStatus;
            }
        }

     return InstanceWriteStatus::Success;
     }

InstanceWriteStatus  WritePrimitiveArray(IECInstanceCR ecInstance, Utf8StringCR accessString, uint32_t nElements, PrimitiveType memberType)
    {
    ECValue         ecValue;
    InstanceWriteStatus     status;
    Utf8CP          typeString = GetPrimitiveTypeString(memberType);
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
// @bsimethod                                    Colin.Kerr                     12/15
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
        // No members, don't write anything
        uint32_t nElements = ecValue.GetArrayInfo().GetCount();
        if (0 == nElements)
            return InstanceWriteStatus::Success;
        
        m_xmlWriter->WriteElementStart(propertyName.c_str());
        WritePrimitiveArray(ecInstance, accessString, nElements, navigationProperty.GetType());
        m_xmlWriter->WriteElementEnd();
        }
    else
        {
        m_xmlWriter->WriteElementStart(propertyName.c_str());
        WritePrimitiveValue(ecValue, navigationProperty.GetType());
        m_xmlWriter->WriteElementEnd();
        }

    return InstanceWriteStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   04/10
+---------------+---------------+---------------+---------------+---------------+------*/
InstanceWriteStatus     WritePrimitivePropertyValue (PrimitiveECPropertyR primitiveProperty, IECInstanceCR ecInstance, Utf8String* baseAccessString)
    {
    ECObjectsStatus     getStatus;
    ECValue             ecValue;

    Utf8StringCR    propertyName = primitiveProperty.GetName();
    Utf8String      accessString;
    CreateAccessString(accessString, baseAccessString, propertyName);
    getStatus = ecInstance.GetValue(ecValue, accessString.c_str());

    // couldn't get, or NULL value, write nothing.
    if ( (ECObjectsStatus::Success != getStatus) || ecValue.IsNull() )
        return InstanceWriteStatus::Success;

    m_xmlWriter->WriteElementStart(propertyName.c_str());
    PrimitiveType           propertyType = primitiveProperty.GetType();

    InstanceWriteStatus status = WritePrimitiveValue (ecValue, propertyType);
    if (status != InstanceWriteStatus::Success)
        return status;
    m_xmlWriter->WriteElementEnd();
    return status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   04/10
+---------------+---------------+---------------+---------------+---------------+------*/
InstanceWriteStatus     WritePrimitiveValue (ECValueCR ecValue, PrimitiveType propertyType)
    {
    char     outString[512];

    // write the content according to type.
    switch (propertyType)
        {
        case PRIMITIVETYPE_Binary:
            {
            size_t      numBytes;
            const Byte* byteData; 
            if (NULL != (byteData = ecValue.GetBinary (numBytes)))
                {
                Utf8String    byteString;
                convertByteArrayToString (byteString, byteData, numBytes);
                m_xmlWriter->WriteRaw(byteString.c_str());
                }
            return InstanceWriteStatus::Success;
            break;
            }

        case PRIMITIVETYPE_IGeometry:
            {
            bmap<OrderedIGeometryPtr, BeExtendedData> extendedData;
            Utf8String beCgXml;
            BeXmlCGWriter::Write(beCgXml, *(ecValue.GetIGeometry()), &extendedData);
            m_xmlWriter->WriteRaw(beCgXml.c_str());
            //strcpy(outString, beCgXml.c_str());
            return InstanceWriteStatus::Success;
            break;
            }

        case PRIMITIVETYPE_Boolean:
            {
            strcpy (outString, ecValue.GetBoolean () ? "True" : "False");
            break;
            }

        case PRIMITIVETYPE_DateTime:
            {
            BeStringUtilities::Snprintf (outString, "%lld", ecValue.GetDateTimeTicks());
            break;
            }

        case PRIMITIVETYPE_Double:
            {
            BeStringUtilities::Snprintf (outString, "%.17g", ecValue.GetDouble());
            break;
            }

        case PRIMITIVETYPE_Integer:
            {
            BeStringUtilities::Snprintf (outString, "%d", ecValue.GetInteger());
            break;
            }

        case PRIMITIVETYPE_Long:
            {
            BeStringUtilities::Snprintf (outString, "%lld", ecValue.GetLong());
            break;
            }

        case PRIMITIVETYPE_Point2D:
            {
            DPoint2d    point2d = ecValue.GetPoint2D();
            BeStringUtilities::Snprintf (outString, "%.17g,%.17g", point2d.x, point2d.y);
            break;
            }

        case PRIMITIVETYPE_Point3D:
            {
            DPoint3d    point3d = ecValue.GetPoint3D();
            BeStringUtilities::Snprintf (outString, "%.17g,%.17g,%.17g", point3d.x, point3d.y, point3d.z);
            break;
            }

        case PRIMITIVETYPE_String:
            {
            m_xmlWriter->WriteText (ecValue.GetUtf8CP());
            return InstanceWriteStatus::Success;
            }

        default:
            {
            BeAssert (false);
            return InstanceWriteStatus::BadPrimitivePropertyType;
            }
        }

    m_xmlWriter->WriteRaw (outString);
    return InstanceWriteStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   04/10
+---------------+---------------+---------------+---------------+---------------+------*/
#if defined (_MSC_VER)
    #pragma warning(disable:4189) // memberClass unused if NDEBUG set.
#endif // defined (_MSC_VER)

InstanceWriteStatus     WriteArrayPropertyValue (ArrayECPropertyR arrayProperty, IECInstanceCR ecInstance, Utf8String* baseAccessString)
    {
    ArrayKind       arrayKind = arrayProperty.GetKind();

    Utf8String    accessString;
    CreateAccessString(accessString, baseAccessString, arrayProperty.GetName());


    // no members, don't write anything.
    ECValue         ecValue;
    if (ECObjectsStatus::Success != ecInstance.GetValue (ecValue, accessString.c_str()) || ecValue.IsNull() || ecValue.GetArrayInfo().GetCount() == 0)
        return InstanceWriteStatus::Success;

    uint32_t nElements = ecValue.GetArrayInfo().GetCount();

    if (BEXML_Success != m_xmlWriter->WriteElementStart(arrayProperty.GetName().c_str()))
        return InstanceWriteStatus::XmlWriteError;

    if (ARRAYKIND_Primitive == arrayKind)
        {
        InstanceWriteStatus status = WritePrimitiveArray(ecInstance, accessString, nElements, arrayProperty.GetPrimitiveElementType());
        if (InstanceWriteStatus::Success != status)
            return status;
        }
    else if (ARRAYKIND_Struct == arrayKind)
        {
        for (uint32_t index=0; index < nElements ; index++)
            {
            if (ECObjectsStatus::Success != ecInstance.GetValue (ecValue, accessString.c_str(), index))
                break;

            // the XML element tag is the struct type.
            BeAssert (ecValue.IsStruct());

            IECInstancePtr  structInstance = ecValue.GetStruct();
            if (!structInstance.IsValid())
                {
                // ###TODO: It is valid to have null struct array instances....
                BeAssert (false);
                break;
                }

            ECClassCR   structClass = structInstance->GetClass();
            BeAssert (structClass.Is (arrayProperty.GetAsStructArrayProperty()->GetStructElementType()));

            m_xmlWriter->WriteElementStart (Utf8String(structClass.GetName().c_str()).c_str());

            InstanceWriteStatus iwxStatus;
            if (InstanceWriteStatus::Success != (iwxStatus = WritePropertyValuesOfClassOrStructArrayMember (structClass, *structInstance.get(), NULL)))
                {
                BeAssert (false);
                return iwxStatus;
                }
            m_xmlWriter->WriteElementEnd();
            }
        }
    else
        {
        // unexpected arrayKind - should never happen.
        BeAssert (false);
        }
    m_xmlWriter->WriteElementEnd();
    return InstanceWriteStatus::Success;
    }

#if defined (_MSC_VER)
    #pragma warning(default:4189)
#endif // defined (_MSC_VER)

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   05/10
+---------------+---------------+---------------+---------------+---------------+------*/
InstanceWriteStatus     WriteEmbeddedStructPropertyValue (StructECPropertyR structProperty, IECInstanceCR ecInstance, Utf8String* baseAccessString)
    {
    // the tag of the element for an embedded struct is the property name.
    m_xmlWriter->WriteElementStart(structProperty.GetName().c_str());

    Utf8String    thisAccessString;
    CreateAccessString(thisAccessString, baseAccessString, structProperty.GetName());
    thisAccessString.append (".");

    ECClassCR   structClass = structProperty.GetType();
    WritePropertyValuesOfClassOrStructArrayMember (structClass, ecInstance, &thisAccessString);

    m_xmlWriter->WriteElementEnd();
    return InstanceWriteStatus::Success;
    }

};


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   10/2011
+---------------+---------------+---------------+---------------+---------------+------*/
InstanceReadStatus   IECInstance::ReadFromXmlFile (IECInstancePtr& ecInstance, WCharCP ecInstanceFile, ECInstanceReadContextR context)
    {
    ecInstance = NULL;

    BeXmlStatus xmlStatus;
    BeXmlDomPtr xmlDom = BeXmlDom::CreateAndReadFromFile (xmlStatus, ecInstanceFile);
    if ((xmlStatus != BEXML_Success) || !xmlDom.IsValid())
        {
        BeAssert (false);
        LogXmlLoadError ();
        return InstanceReadStatus::XmlParseError;
        }
    return ReadFromBeXmlDom (ecInstance, *xmlDom.get(), context);
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                12/2012
//+---------------+---------------+---------------+---------------+---------------+------
InstanceReadStatus   IECInstance::ReadFromXmlString (IECInstancePtr& ecInstance, Utf8CP ecInstanceXml, ECInstanceReadContextR context)
    {
    ecInstance = NULL;
    BeXmlStatus xmlStatus;
    BeXmlDomPtr xmlDom = BeXmlDom::CreateAndReadFromString (xmlStatus, ecInstanceXml);
    if (!xmlDom.IsValid())
        {
        BeAssert (false);
        LogXmlLoadError ();
        return InstanceReadStatus::XmlParseError;
        }
    return ReadFromBeXmlDom (ecInstance, *xmlDom.get(), context);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Carole.MacDonald                05/2010
+---------------+---------------+---------------+---------------+---------------+------*/
InstanceReadStatus   IECInstance::ReadFromXmlString (IECInstancePtr& ecInstance, WCharCP ecInstanceXml, ECInstanceReadContextR context)
    {
    ecInstance = NULL;

    BeXmlStatus xmlStatus;
    BeXmlDomPtr xmlDom = BeXmlDom::CreateAndReadFromString (xmlStatus, ecInstanceXml, wcslen (ecInstanceXml));

    if (!xmlDom.IsValid())
        {
        BeAssert (false);
        LogXmlLoadError ();
        return InstanceReadStatus::XmlParseError;
        }

    return ReadFromBeXmlDom (ecInstance, *xmlDom.get(), context);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   10/2011
+---------------+---------------+---------------+---------------+---------------+------*/
InstanceReadStatus  IECInstance::ReadFromBeXmlDom (IECInstancePtr& ecInstance, BeXmlDomR xmlDom, ECInstanceReadContextR context)
    {
    ecInstance = NULL;

    BeXmlNodeP      instanceNode;
    if ( (BEXML_Success != xmlDom.SelectNode (instanceNode, "/", NULL, BeXmlDom::NODE_BIAS_First)) || (NULL == instanceNode) )
        {
        BeAssert (false);
        LOG.errorv ("Invalid ECInstanceXML: Missing a top-level instance node");
        return InstanceReadStatus::BadElement;
        }

    if (BEXMLNODE_Document == instanceNode->GetType())
        instanceNode = instanceNode->GetFirstChild (BEXMLNODE_Element);

    if (NULL == instanceNode)
        {
        BeAssert (false);
        LOG.errorv ("Invalid ECInstanceXML: Missing a top-level instance node");
        return InstanceReadStatus::BadElement;
        }

    return ReadFromBeXmlNode (ecInstance, *instanceNode, context);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Carole.MacDonald                05/2010
+---------------+---------------+---------------+---------------+---------------+------*/
InstanceReadStatus   IECInstance::ReadFromBeXmlNode (IECInstancePtr& ecInstance, BeXmlNodeR instanceNode, ECInstanceReadContextR context)
    {
    InstanceXmlReader   reader (context, instanceNode);
    return reader.ReadInstance (ecInstance);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   04/10
+---------------+---------------+---------------+---------------+---------------+------*/
InstanceWriteStatus     IECInstance::WriteToXmlFile (WCharCP fileName, bool writeInstanceId, bool utf16)
    {
    BeXmlWriterPtr xmlWriter = BeXmlWriter::CreateFileWriter(fileName);
    InstanceXmlWriter   instanceWriter (xmlWriter.get());

    xmlWriter->WriteDocumentStart(XML_CHAR_ENCODING_UTF8);
    return instanceWriter.WriteInstance (*this, writeInstanceId);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    Paul.Connelly   12/12
//+---------------+---------------+---------------+---------------+---------------+------
template<typename T_STR> InstanceWriteStatus writeInstanceToXmlString (T_STR& ecInstanceXml, bool isStandAlone, bool writeInstanceId, IECInstanceR instance)
    {
    ecInstanceXml.clear();
    BeXmlWriterPtr xmlWriter = BeXmlWriter::Create();        
    InstanceXmlWriter   instanceWriter (xmlWriter.get());
    if (isStandAlone)
        xmlWriter->WriteDocumentStart(XML_CHAR_ENCODING_UTF8);

    InstanceWriteStatus status;
    if (InstanceWriteStatus::Success != (status = instanceWriter.WriteInstance (instance, writeInstanceId)))
        return status;

    xmlWriter->ToString (ecInstanceXml);
    return InstanceWriteStatus::Success;
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                12/2012
//+---------------+---------------+---------------+---------------+---------------+------
InstanceWriteStatus     IECInstance::WriteToXmlString (Utf8String & ecInstanceXml, bool isStandAlone, bool writeInstanceId)
    {
    return writeInstanceToXmlString (ecInstanceXml, isStandAlone, writeInstanceId, *this);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Carole.MacDonald                06/2010
+---------------+---------------+---------------+---------------+---------------+------*/
InstanceWriteStatus     IECInstance::WriteToXmlString (WString & ecInstanceXml, bool isStandAlone, bool writeInstanceId)
    {
    return writeInstanceToXmlString (ecInstanceXml, isStandAlone, writeInstanceId, *this);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Prasanna.Prakash                03/2016
+---------------+---------------+---------------+---------------+---------------+------*/
InstanceWriteStatus     IECInstance::WriteToBeXmlNode (BeXmlWriterR xmlWriter)
    {
    Utf8CP className = this->GetClass().GetName().c_str();
    
    return WriteToBeXmlNode(xmlWriter, className);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   10/2011
+---------------+---------------+---------------+---------------+---------------+------*/
InstanceWriteStatus     IECInstance::WriteToBeXmlNode(BeXmlWriterR xmlWriter, Utf8CP className)
    {
    InstanceXmlWriter instanceWriter(&xmlWriter);

    return instanceWriter.WriteInstance(*this, false, className);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   09/13
+---------------+---------------+---------------+---------------+---------------+------*/
InstanceWriteStatus IECInstance::WriteToBeXmlDom (BeXmlWriterR xmlWriter, bool writeInstanceId)
    {
    InstanceXmlWriter writer (&xmlWriter);
    return writer.WriteInstance (*this, writeInstanceId);
    }

#if defined (NEEDSWORK_LIBXML) // WIP_NONPORT
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Carole.MacDonald                05/2010
+---------------+---------------+---------------+---------------+---------------+------*/
InstanceWriteStatus     IECInstance::WriteToXmlStream (IStreamP stream, bool isCompleteXmlDocument, bool writeInstanceId, bool utf16)
    {
    InstanceXmlWriter writer (stream);

    InstanceWriteStatus   status;
    if (InstanceWriteStatus::Success != (status = writer.Init (utf16)))
        return status;

    return writer.WriteInstance (*this, isStandAlone, writeInstanceId);
    }
#endif //defined (NEEDSWORK_XML)

#ifdef DGN_IMPORTER_REORG_WIP
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Bill.Steinbock                  05/2013
+---------------+---------------+---------------+---------------+---------------+------*/
bool IECInstance::SaveOnlyLoadedPropertiesToXml() const
    {
    return _SaveOnlyLoadedPropertiesToXml();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   11/12
+---------------+---------------+---------------+---------------+---------------+------*/
DgnPlatform::DgnECInstance const* IECInstance::AsDgnECInstanceCP() const
    {
    return _GetAsDgnECInstance();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   11/12
+---------------+---------------+---------------+---------------+---------------+------*/
DgnPlatform::DgnECInstance* IECInstance::AsDgnECInstanceP()
    {
    return const_cast<DgnPlatform::DgnECInstance*> (_GetAsDgnECInstance());
    }
#endif

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Bill.Steinbock                  05/2013
+---------------+---------------+---------------+---------------+---------------+------*/
bool IECInstance::SaveOnlyLoadedPropertiesToXml() const
    {
    return _SaveOnlyLoadedPropertiesToXml();
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  03/2012
+---------------+---------------+---------------+---------------+---------------+------*/
ECSchemaCP ECInstanceReadContext::FindSchemaCP(SchemaKeyCR key, SchemaMatchType matchType) const
    {
    ECSchemaCP schema = _FindSchemaCP(key, matchType);
    if (NULL != schema)
        return schema;

    return &m_fallBackSchema;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   09/13
+---------------+---------------+---------------+---------------+---------------+------*/
bool IECSchemaRemapper::ResolvePropertyName (Utf8StringR name, ECClassCR ecClass) const        { return _ResolvePropertyName (name, ecClass); }
bool IECSchemaRemapper::ResolveClassName (Utf8StringR name, ECSchemaCR schema) const           { return _ResolveClassName (name, schema); }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   06/14
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus IECInstanceInterface::GetInstanceValue (ECValueR v, Utf8CP accessor) const { return _GetInstanceValue (v, accessor); }
ECClassCP IECInstanceInterface::GetInstanceClass() const                                    { return _GetInstanceClass(); }
IECInstanceCP IECInstanceInterface::ObtainECInstance() const                                { return _ObtainECInstance(); }
Utf8String IECInstanceInterface::GetInstanceId() const                                         { return _GetInstanceId(); }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   06/14
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus ECInstanceInterface::_GetInstanceValue (ECValueR v, Utf8CP accessor) const
    {
    return ECInstanceInteropHelper::GetValue (m_instance, v, accessor);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   06/14
+---------------+---------------+---------------+---------------+---------------+------*/
ECClassCP ECInstanceInterface::_GetInstanceClass() const
    {
    return &m_instance.GetClass();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   06/14
+---------------+---------------+---------------+---------------+---------------+------*/
IECInstanceCP ECInstanceInterface::_ObtainECInstance() const
    {
    return &m_instance;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   07/14
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String ECInstanceInterface::_GetInstanceId() const
    {
    return m_instance.GetInstanceId();
    }

END_BENTLEY_ECOBJECT_NAMESPACE
