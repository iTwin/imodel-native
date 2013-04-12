/*--------------------------------------------------------------------------------------+
|
|     $Source: src/ECInstance.cpp $
|
|   $Copyright: (c) 2013 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ECObjectsPch.h"
#include <ECObjects/ecschema.h>

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
BentleyStatus            CustomStructSerializerManager::AddCustomSerializer (WCharCP serializerName, ICustomECStructSerializerP serializer)
    {
    if (GetCustomSerializer (serializerName))
        return ERROR;

    m_serializers[WString(serializerName)] = serializer;
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
ICustomECStructSerializerP                      CustomStructSerializerManager::GetCustomSerializer (WCharCP serializerName) const
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
    IECInstancePtr caInstance = structProperty->GetType().GetCustomAttribute(L"CustomStructSerializer");
    if (caInstance.IsValid())
        {
        ECValue value;
        if (SUCCESS == caInstance->GetValue (value, L"SerializerName"))
            {
            ICustomECStructSerializerP serializerP = GetCustomSerializer (value.GetString());
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
bool IECInstance::IsFixedArrayProperty (ECN::IECInstanceR instance, WCharCP accessString, UInt32* numFixedEntries)
    {
    ECValue         arrayVal;

    if (ECOBJECTS_STATUS_Success != instance.GetValue (arrayVal, accessString))
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
    BeAssert (sizeof(IECInstance) == sizeof (RefCountedBase) && L"Increasing the size or memory layout of the base ECN::IECInstance will adversely affect subclasses. Think of this as a pure interface... to which you would never be able to add (additional) data, either");
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
bool IsExcluded(WString& className, bvector<WString>& classNamesToExclude)
    {
    FOR_EACH (WString excludedClass, classNamesToExclude)
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
    WString ecInstanceXml;
    this->WriteToXmlString(ecInstanceXml, true, false);

    ECInstanceReadContextPtr instanceContext = ECInstanceReadContext::CreateContext (GetClass().GetSchema());

    IECInstancePtr deserializedInstance;
    IECInstance::ReadFromXmlString(deserializedInstance, ecInstanceXml.c_str(), *instanceContext);

    return deserializedInstance;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    05/11
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus        IECInstance::_SetInstanceId (WCharCP id)    { return ECOBJECTS_STATUS_OperationNotSupported; }
ECObjectsStatus        IECInstance::SetInstanceId (WCharCP id)     { return _SetInstanceId(id); }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    CaseyMullen     09/09
+---------------+---------------+---------------+---------------+---------------+------*/
WString        IECInstance::GetInstanceId() const
    {
    return _GetInstanceId();
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
bool                IECInstance::_IsPropertyReadOnly (WCharCP accessString) const
    {
    if (_IsReadOnly())
        return true;
    
    UInt32 propertyIndex;
    return ECOBJECTS_STATUS_Success != GetEnabler().GetPropertyIndex (propertyIndex, accessString) || IsPropertyReadOnly (propertyIndex);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Dylan.Rush      08/2011
+---------------+---------------+---------------+---------------+---------------+------*/ 
bool                IECInstance::_IsPropertyReadOnly (UInt32 propertyIndex) const
    {
    return _IsReadOnly() || GetEnabler().IsPropertyReadOnly (propertyIndex);
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
ECObjectsStatus     IECInstance::GetValue (ECValueR v, WCharCP propertyAccessString) const 
    {
    UInt32 propertyIndex=0;
    ECObjectsStatus status = GetEnabler().GetPropertyIndex (propertyIndex, propertyAccessString);

    if (ECOBJECTS_STATUS_Success != status)
        return status;

    return _GetValue (v, propertyIndex, false, 0); 
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    CaseyMullen     09/09
+---------------+---------------+---------------+---------------+---------------+------*/   
ECObjectsStatus     IECInstance::GetValue (ECValueR v, WCharCP propertyAccessString, UInt32 arrayIndex) const 
    {
    UInt32 propertyIndex=0;
    ECObjectsStatus status = GetEnabler().GetPropertyIndex (propertyIndex, propertyAccessString);

    if (ECOBJECTS_STATUS_Success != status)
        return status;

    return _GetValue (v, propertyIndex, true, arrayIndex); 
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    CaseyMullen     09/09
+---------------+---------------+---------------+---------------+---------------+------*/   
ECObjectsStatus     IECInstance::GetValue (ECValueR v, UInt32 propertyIndex) const { return _GetValue (v, propertyIndex, false, 0); }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    CaseyMullen     09/09
+---------------+---------------+---------------+---------------+---------------+------*/   
ECObjectsStatus     IECInstance::GetValue (ECValueR v, UInt32 propertyIndex, UInt32 arrayIndex) const { return _GetValue (v, propertyIndex, true, arrayIndex); }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    CaseyMullen     09/09
+---------------+---------------+---------------+---------------+---------------+------*/   
ECObjectsStatus     IECInstance::SetInternalValue (WCharCP propertyAccessString, ECValueCR v) 
    {
    UInt32 propertyIndex=0;
    ECObjectsStatus status = GetEnabler().GetPropertyIndex (propertyIndex, propertyAccessString);

    if (ECOBJECTS_STATUS_Success != status)
        return status;

    return SetInternalValue (propertyIndex, v); 
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    CaseyMullen     09/09
+---------------+---------------+---------------+---------------+---------------+------*/   
ECObjectsStatus     IECInstance::ChangeValue (WCharCP propertyAccessString, ECValueCR v) 
    {
    UInt32 propertyIndex=0;
    ECObjectsStatus status = GetEnabler().GetPropertyIndex (propertyIndex, propertyAccessString);

    if (ECOBJECTS_STATUS_Success != status)
        return status;

    return ChangeValue (propertyIndex, v); 
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Bill.Steinbock                  11/2012
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus     IECInstance::SetValue (WCharCP propertyAccessString, ECValueCR v) 
    {
    ECObjectsStatus status = ChangeValue (propertyAccessString, v);

    if (ECOBJECTS_STATUS_PropertyValueMatchesNoChange == status)
        return ECOBJECTS_STATUS_Success;

    return status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    CaseyMullen     09/09
+---------------+---------------+---------------+---------------+---------------+------*/   
ECObjectsStatus     IECInstance::SetInternalValue (WCharCP propertyAccessString, ECValueCR v, UInt32 arrayIndex) 
    {
    UInt32 propertyIndex=0;
    ECObjectsStatus status = GetEnabler().GetPropertyIndex (propertyIndex, propertyAccessString);

    if (ECOBJECTS_STATUS_Success != status)
        return status;

    return SetInternalValue (propertyIndex, v, arrayIndex); 
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    CaseyMullen     09/09
+---------------+---------------+---------------+---------------+---------------+------*/   
ECObjectsStatus     IECInstance::ChangeValue (WCharCP propertyAccessString, ECValueCR v, UInt32 arrayIndex) 
    {
    UInt32 propertyIndex=0;
    ECObjectsStatus status = GetEnabler().GetPropertyIndex (propertyIndex, propertyAccessString);

    if (ECOBJECTS_STATUS_Success != status)
        return status;

    return ChangeValue (propertyIndex, v, arrayIndex); 
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    CaseyMullen     09/09
+---------------+---------------+---------------+---------------+---------------+------*/   
ECObjectsStatus     IECInstance::SetValue (WCharCP propertyAccessString, ECValueCR v, UInt32 arrayIndex) 
    {
    ECObjectsStatus status = ChangeValue (propertyAccessString, v, arrayIndex);

    if (ECOBJECTS_STATUS_PropertyValueMatchesNoChange == status)
        return ECOBJECTS_STATUS_Success;

    return status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    CaseyMullen     09/09
+---------------+---------------+---------------+---------------+---------------+------*/   
ECObjectsStatus     IECInstance::SetInternalValue (UInt32 propertyIndex, ECValueCR v) 
    {
    return _SetInternalValue (propertyIndex, v, false, 0); 
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    CaseyMullen     09/09
+---------------+---------------+---------------+---------------+---------------+------*/   
ECObjectsStatus     IECInstance::ChangeValue (UInt32 propertyIndex, ECValueCR v) 
    {
    if (IsReadOnly())
        return ECOBJECTS_STATUS_UnableToSetReadOnlyInstance;

    bool isNull = false;
    if (GetIsPropertyNull (isNull, propertyIndex, false, 0))
        return ECOBJECTS_STATUS_UnableToQueryForNullPropertyFlag;

    if (IsPropertyReadOnly (propertyIndex) && !isNull)
        return ECOBJECTS_STATUS_UnableToSetReadOnlyProperty;

    return _SetValue (propertyIndex, v, false, 0); 
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    CaseyMullen     09/09
+---------------+---------------+---------------+---------------+---------------+------*/   
ECObjectsStatus     IECInstance::SetValue (UInt32 propertyIndex, ECValueCR v) 
    {
    ECObjectsStatus status = ChangeValue (propertyIndex, v);

    if (ECOBJECTS_STATUS_PropertyValueMatchesNoChange == status)
        return ECOBJECTS_STATUS_Success;

    return  status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Bill.Steinbock                  11/2012
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus     IECInstance::SetInternalValue (UInt32 propertyIndex, ECValueCR v, UInt32 arrayIndex) 
    { 
    return _SetInternalValue (propertyIndex, v, true, arrayIndex); 
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   09/12
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus IECInstance::_SetInternalValue (UInt32 propertyIndex, ECValueCR v, bool useArrayIndex, UInt32 arrayIndex)
    {
    // Default impl; instances that support calculated properties should override
    return _SetValue (propertyIndex, v, useArrayIndex, arrayIndex);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Bill.Steinbock                  11/2012
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus     IECInstance::ChangeValue (UInt32 propertyIndex, ECValueCR v, UInt32 arrayIndex) 
    {
    if (IsReadOnly())
        return ECOBJECTS_STATUS_UnableToSetReadOnlyInstance;

    bool isNull = false;
    ECObjectsStatus status = GetIsPropertyNull (isNull, propertyIndex, true, arrayIndex);
    if (status != ECOBJECTS_STATUS_Success)
        return status;

    if (IsPropertyReadOnly (propertyIndex) && !isNull)
        return ECOBJECTS_STATUS_UnableToSetReadOnlyProperty;

    return _SetValue (propertyIndex, v, true, arrayIndex); 
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Bill.Steinbock                  11/2012
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus     IECInstance::SetValue (UInt32 propertyIndex, ECValueCR v, UInt32 arrayIndex) 
    {
    ECObjectsStatus status = ChangeValue (propertyIndex, v, arrayIndex);

    if (ECOBJECTS_STATUS_PropertyValueMatchesNoChange == status)
        return ECOBJECTS_STATUS_Success;

    return  status;
    }

bool                IECInstance::IsPropertyReadOnly (UInt32 propertyIndex) const { return _IsPropertyReadOnly (propertyIndex); }
bool                IECInstance::IsPropertyReadOnly (WCharCP accessString) const { return _IsPropertyReadOnly (accessString); }

#define NUM_INDEX_BUFFER_CHARS 63
#define NUM_ACCESSSTRING_BUFFER_CHARS 1023

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Dylan.Rush                      1/11
+---------------+---------------+---------------+---------------+---------------+------*/
static ECPropertyP getProperty  (ECClassCR ecClass, WCharCP accessor, wchar_t* buffer)
    {
    //Gets the ECProperty for a full native accessor.
    //For example, the full native accessor could be "GrandfatherStruct.ParentStruct.StringMember"
    //In this case, passing this accessor to this function will give you the
    //ECProperty for StringMember.
   
    WCharCP dotPos = wcschr (accessor, L'.');
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

    WCharCP bracketPos = wcschr (accessor, L'[');
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
static ECObjectsStatus getECValueUsingFullAccessString (wchar_t* asBuffer, wchar_t* indexBuffer, ECValueR v, IECInstanceCR instance, WCharCP managedPropertyAccessor)
    {
    // see if access string specifies an array
    WCharCP pos1 = wcschr (managedPropertyAccessor, L'[');

    // if not an array then 
    if (NULL == pos1)
        return instance.GetValue (v, managedPropertyAccessor);

    size_t numChars = 0;
    numChars = pos1 - managedPropertyAccessor;
    wcsncpy(asBuffer, managedPropertyAccessor, numChars>NUM_ACCESSSTRING_BUFFER_CHARS?NUM_ACCESSSTRING_BUFFER_CHARS:numChars);
    asBuffer[numChars]=0;

    // BRACKETS_OKAY: Brackets contain an array index
    WCharCP pos2 = wcschr (pos1+1, L']');

    BeAssert (pos2 != NULL);

    numChars = pos2 - pos1 - 1;

    wcsncpy(indexBuffer, pos1+1, numChars>NUM_INDEX_BUFFER_CHARS?NUM_INDEX_BUFFER_CHARS:numChars);
    indexBuffer[numChars]=0;

    UInt32 indexValue = -1;
    BeStringUtilities::Swscanf (indexBuffer, L"%ud", &indexValue);

    ECValue         arrayVal;
    ECObjectsStatus status;

    if (ECOBJECTS_STATUS_Success != (status = instance.GetValue (arrayVal, asBuffer)))
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
        return ECOBJECTS_STATUS_Success;
        }

    ArrayInfo arrayInfo = arrayVal.GetArrayInfo();
    UInt32    size      = arrayInfo.GetCount();

    if (indexValue >= size)
        return ECOBJECTS_STATUS_Error;

    if (arrayInfo.IsPrimitiveArray())
        return instance.GetValue (v, asBuffer, indexValue);

    // must be a struct array
    
    if (ECOBJECTS_STATUS_Success != (status = instance.GetValue (arrayVal, asBuffer, indexValue)))
        return status;

    // If there is no '.' in the rest of the access string, the caller was requesting the value representing the struct
    // array element itself, not the value of any of its members.
    if ('\0' == pos2[1])
        {
        v.SetStruct(arrayVal.GetStruct().get());
        return ECOBJECTS_STATUS_Success;
        }

    IECInstancePtr arrayEntryInstance = arrayVal.GetStruct();

    return getECValueUsingFullAccessString (asBuffer, indexBuffer, v, *arrayEntryInstance, pos2+2); // move to character after "]." in access string.
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Bill.Steinbock                  10/2010
+---------------+---------------+---------------+---------------+---------------+------*/
static ECObjectsStatus getECValueFromInstance (ECValueR v, IECInstanceCR instance, WCharCP managedPropertyAccessor)
    {
    WString asBufferStr;

    v.Clear();
    wchar_t asBuffer[NUM_ACCESSSTRING_BUFFER_CHARS+1];
    wchar_t indexBuffer[NUM_INDEX_BUFFER_CHARS+1];

    return getECValueUsingFullAccessString (asBuffer, indexBuffer, v, instance, managedPropertyAccessor);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Dylan.Rush      11/10
+---------------+---------------+---------------+---------------+---------------+------*/ 
static ECObjectsStatus          getValueHelper (ECValueR value, IECInstanceCR instance, ECValueAccessorCR accessor, UInt32 depth, bool compatible)
    {
    ECValueAccessor::Location const& loc = accessor[depth];
    int arrayIndex = loc.GetArrayIndex();
    if (compatible)
        {
        UInt32 propertyIndex = (UInt32)loc.GetPropertyIndex();
        if (arrayIndex < 0)
            return instance.GetValue (value, propertyIndex);
        return instance.GetValue (value, propertyIndex,  (UInt32)arrayIndex);
        }

    WCharCP accessString = accessor.GetAccessString (depth);
    if (NULL == accessString)
        return ECOBJECTS_STATUS_Error;

    if (arrayIndex < 0)
        return instance.GetValue (value, accessString);

    return instance.GetValue (value, accessString, (UInt32)arrayIndex);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Dylan.Rush      11/10
+---------------+---------------+---------------+---------------+---------------+------*/ 
static ECObjectsStatus          setValueHelper (IECInstanceR instance, ECValueAccessorCR accessor, UInt32 depth, bool compatible, ECValueCR value)
    {
    int arrayIndex = accessor[depth].GetArrayIndex();
    if (compatible)
        {
        UInt32 propertyIndex = (UInt32)accessor[depth].GetPropertyIndex();

        if(arrayIndex < 0)
            {
            return instance.SetValue(propertyIndex, value);
            }
        return instance.SetValue (propertyIndex, value, (UInt32)arrayIndex);
        }

    // not the same enabler between accessor and instance so use access string to set value
    WCharCP accessString = accessor.GetAccessString(depth);
    if (NULL == accessString)
        return ECOBJECTS_STATUS_Error;

    if (arrayIndex < 0)
        return instance.SetValue (accessString, value);

    return instance.SetValue (accessString, value, (UInt32)arrayIndex);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Dylan.Rush      11/10
+---------------+---------------+---------------+---------------+---------------+------*/ 
static ECObjectsStatus          setInternalValueHelper (IECInstanceR instance, ECValueAccessorCR accessor, UInt32 depth, bool compatible, ECValueCR value)
    {
    int arrayIndex = accessor[depth].GetArrayIndex();
    if (compatible)
        {
        UInt32 propertyIndex = (UInt32)accessor[depth].GetPropertyIndex();

        if(arrayIndex < 0)
            {
            return instance.SetInternalValue(propertyIndex, value);
            }

        return instance.SetInternalValue (propertyIndex, value, (UInt32)arrayIndex);
        }

    // not the same enabler between accessor and instance so use access string to set value
    WCharCP accessString = accessor.GetAccessString(depth);
    if (NULL == accessString)
        return ECOBJECTS_STATUS_Error;

    if (arrayIndex < 0)
        return instance.SetInternalValue (accessString, value);

    return instance.SetInternalValue (accessString, value, (UInt32)arrayIndex);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Dylan.Rush      11/10
+---------------+---------------+---------------+---------------+---------------+------*/ 
ECObjectsStatus           IECInstance::GetValueUsingAccessor (ECValueR v, ECValueAccessorCR accessor) const
    {
    ECObjectsStatus status            = ECOBJECTS_STATUS_Success;
    IECInstancePtr  currentInstance   = const_cast <IECInstance*> (this);

    for (UInt32 depth = 0; depth < accessor.GetDepth(); depth ++)
        {
        v.Clear();
        bool compatible = (accessor[depth].GetEnabler() == &currentInstance->GetEnabler()); // if same enabler then use property index to set value else use access string

        status = getValueHelper (v, *currentInstance, accessor, depth, compatible);
        if (ECOBJECTS_STATUS_Success != status)
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
    ECObjectsStatus status          = ECOBJECTS_STATUS_Success;
    IECInstancePtr  currentInstance = this;

    for (UInt32 depth = 0; depth < accessor.GetDepth(); depth++)
        {
        bool compatible = (accessor[depth].GetEnabler() == &currentInstance->GetEnabler()); // if same enabler then use property index to set value else use access string
        int  propertyIndex   = accessor[depth].GetPropertyIndex();
        int  arrayIndex      = accessor[depth].GetArrayIndex();

        if (arrayIndex > -1)
            {
            ECValue         arrayInfoPlaceholder;

            //Get the array value to check its size. Expand array if necessary.
            if (compatible)
                currentInstance->GetValue(arrayInfoPlaceholder, (UInt32)propertyIndex);
            else
                currentInstance->GetValue(arrayInfoPlaceholder, accessor.GetAccessString (depth));

            if (ECOBJECTS_STATUS_Success != status)
                return status;

            UInt32 arraySize = arrayInfoPlaceholder.GetArrayInfo().GetCount();

            if ((UInt32)arrayIndex >= arraySize)
                {
                if (arrayInfoPlaceholder.GetArrayInfo().IsFixedCount())
                    return ECOBJECTS_STATUS_IndexOutOfRange;

                UInt32 numToInsert = 1 + (UInt32)arrayIndex - arraySize;

                WCharCP accessorWithBrackets = accessor.GetAccessString (depth);
                if (NULL == accessorWithBrackets)
                    return ECOBJECTS_STATUS_Error;

                status = currentInstance->AddArrayElements (accessorWithBrackets, numToInsert);    
                if (ECOBJECTS_STATUS_Success != status)
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
        if (ECOBJECTS_STATUS_Success != status)
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
            if (ECOBJECTS_STATUS_Success != status)
                return status;
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
    if (IsReadOnly())
        return ECOBJECTS_STATUS_UnableToSetReadOnlyInstance;

    return  SetInternalValueUsingAccessor (accessor, valueToSet);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Bill.Steinbock                  08/2012
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus    IECInstance::GetIsPropertyNull (bool& isNull, UInt32 propertyIndex, bool useArrayIndex, UInt32 arrayIndex) const
    {
    return  _GetIsPropertyNull (isNull, propertyIndex, useArrayIndex, arrayIndex);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Paul.Connelly                   03/2013
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus IECInstance::_GetIsPropertyNull (bool& isNull, UInt32 propertyIndex, bool useArrayIndex, UInt32 arrayIndex) const
    {
    // default implementation. ECD-based implementations can be more efficient.
    isNull = true;
    ECValue v;
    ECObjectsStatus status = _GetValue (v, propertyIndex, useArrayIndex, arrayIndex);
    if (ECOBJECTS_STATUS_Success == status)
        isNull = v.IsNull();

    return status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Bill.Steinbock                  08/2012
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus     IECInstance::IsPropertyNull (bool& isNull, WCharCP propertyAccessString) const 
    {
    UInt32 propertyIndex=0;
    ECObjectsStatus status = GetEnabler().GetPropertyIndex (propertyIndex, propertyAccessString);

    if (ECOBJECTS_STATUS_Success != status)
        return status;

    return _GetIsPropertyNull (isNull, propertyIndex, false, 0); 
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Bill.Steinbock                  08/2012
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus     IECInstance::IsPropertyNull (bool& isNull, WCharCP propertyAccessString, UInt32 arrayIndex) const 
    {
    UInt32 propertyIndex=0;
    ECObjectsStatus status = GetEnabler().GetPropertyIndex (propertyIndex, propertyAccessString);

    if (ECOBJECTS_STATUS_Success != status)
        return status;

    return _GetIsPropertyNull (isNull, propertyIndex, true, arrayIndex); 
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Bill.Steinbock                  08/2012
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus     IECInstance::IsPropertyNull (bool& isNull, UInt32 propertyIndex) const 
    {
    return _GetIsPropertyNull (isNull, propertyIndex, false, 0); 
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Bill.Steinbock                  08/2012
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus     IECInstance::IsPropertyNull (bool& isNull, UInt32 propertyIndex, UInt32 arrayIndex) const 
    {
    return _GetIsPropertyNull (isNull, propertyIndex, true, arrayIndex); 
    }

/////////////////////////////////////////////////////////////////////////////////////////
//  ECInstanceInteropHelper
/////////////////////////////////////////////////////////////////////////////////////////
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Bill.Steinbock                  10/2010
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus                 ECInstanceInteropHelper::GetValue (IECInstanceCR instance, ECValueR value, WCharCP managedPropertyAccessor)
    {
    return getECValueFromInstance (value, instance, managedPropertyAccessor);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Bill.Steinbock                  03/2010
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus ECInstanceInteropHelper::GetLong (IECInstanceCR instance, Int64 & value, WCharCP managedPropertyAccessor)
    {
    ECValue v;

    ECObjectsStatus status = getECValueFromInstance (v, instance, managedPropertyAccessor);
    if (status == ECOBJECTS_STATUS_Success)
        value = v.GetLong();
    
    return status;
    }    

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    CaseyMullen     09/09
+---------------+---------------+---------------+---------------+---------------+------*/        
ECObjectsStatus ECInstanceInteropHelper::GetInteger (IECInstanceCR instance, int & value, WCharCP managedPropertyAccessor)
    {
    ECValue v;

    ECObjectsStatus status = getECValueFromInstance (v, instance, managedPropertyAccessor);
    if (status == ECOBJECTS_STATUS_Success)
        value = v.GetInteger();
    
    return status;
    }    

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    CaseyMullen     09/09
+---------------+---------------+---------------+---------------+---------------+------*/                
ECObjectsStatus ECInstanceInteropHelper::GetDouble (IECInstanceCR instance, double& value, WCharCP managedPropertyAccessor)
    {
    ECValue v;

    ECObjectsStatus status = getECValueFromInstance (v, instance, managedPropertyAccessor);
    if (status == ECOBJECTS_STATUS_Success)        
        value = v.GetDouble();
        
    return status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    CaseyMullen     09/09
+---------------+---------------+---------------+---------------+---------------+------*/       
ECObjectsStatus ECInstanceInteropHelper::GetString (IECInstanceCR instance, WStringR value, WCharCP managedPropertyAccessor)
    {
    ECValue v;

    ECObjectsStatus status = getECValueFromInstance (v, instance, managedPropertyAccessor);
    if (status == ECOBJECTS_STATUS_Success)
        value = v.GetString();

    return status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Bill.Steinbock                  03/2010
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus ECInstanceInteropHelper::GetBoolean (IECInstanceCR instance, bool & value, WCharCP managedPropertyAccessor)
    {
    ECValue v;

    ECObjectsStatus status = getECValueFromInstance (v, instance, managedPropertyAccessor);
    if (status == ECOBJECTS_STATUS_Success)
        value = v.GetBoolean();

    return status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Bill.Steinbock                  03/2010
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus ECInstanceInteropHelper::GetPoint2D (IECInstanceCR instance, DPoint2d & value, WCharCP managedPropertyAccessor)
    {
    ECValue v;

    ECObjectsStatus status = getECValueFromInstance (v, instance, managedPropertyAccessor);
    if (status == ECOBJECTS_STATUS_Success)
        value = v.GetPoint2D();
        
    return status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Bill.Steinbock                  03/2010
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus ECInstanceInteropHelper::GetPoint3D (IECInstanceCR instance, DPoint3d & value, WCharCP managedPropertyAccessor)
    {
    ECValue v;

    ECObjectsStatus status = getECValueFromInstance (v, instance, managedPropertyAccessor);
    if (status == ECOBJECTS_STATUS_Success)
        value = v.GetPoint3D();
        
    return status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Bill.Steinbock                  03/2010
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus ECInstanceInteropHelper::GetDateTime (IECInstanceCR instance, SystemTime & value, WCharCP managedPropertyAccessor)
    {
    ECValue v;

    ECObjectsStatus status = getECValueFromInstance (v, instance, managedPropertyAccessor);
    if (status == ECOBJECTS_STATUS_Success)
        value = v.GetDateTime();
        
    return status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Bill.Steinbock                  03/2010
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus ECInstanceInteropHelper::GetDateTimeTicks (IECInstanceCR instance, Int64 & value, WCharCP managedPropertyAccessor)
    {
    ECValue v;

    ECObjectsStatus status = getECValueFromInstance (v, instance, managedPropertyAccessor);
    if (status == ECOBJECTS_STATUS_Success)
        value = v.GetDateTimeTicks();
        
    return status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Bill.Steinbock                  10/2010
+---------------+---------------+---------------+---------------+---------------+------*/
static ECObjectsStatus setECValueUsingFullAccessString (wchar_t* asBuffer, wchar_t* indexBuffer, ECValueCR v, IECInstanceR instance, WCharCP managedPropertyAccessor)
    {
    // skip all the work if the instance is read only
    if (instance.IsReadOnly())
        return ECOBJECTS_STATUS_UnableToSetReadOnlyInstance;

    // see if access string specifies an array
    WCharCP pos1 = wcschr (managedPropertyAccessor, L'[');

    // if not an array then 
    if (NULL == pos1)
        return instance.SetValue (managedPropertyAccessor, v);

    size_t numChars = 0;
    numChars = pos1 - managedPropertyAccessor;
    wcsncpy(asBuffer, managedPropertyAccessor, numChars>NUM_ACCESSSTRING_BUFFER_CHARS?NUM_ACCESSSTRING_BUFFER_CHARS:numChars);
    asBuffer[numChars]=0;

    WCharCP pos2 = wcschr (pos1+1, L']');

    BeAssert (pos2 != NULL);

    numChars = pos2 - pos1 - 1;

    wcsncpy(indexBuffer, pos1+1, numChars>NUM_INDEX_BUFFER_CHARS?NUM_INDEX_BUFFER_CHARS:numChars);
    indexBuffer[numChars]=0;

    UInt32 indexValue = 0;
    if (1 != BeStringUtilities::Swscanf (indexBuffer, L"%ud", &indexValue))
        return ECOBJECTS_STATUS_Error;

    ECValue         arrayVal;
    ECObjectsStatus status;

    if (ECOBJECTS_STATUS_Success != (status = instance.GetValue (arrayVal, asBuffer)))
        return status;

    ArrayInfo arrayInfo = arrayVal.GetArrayInfo();
    UInt32    size      = arrayInfo.GetCount();

    if (indexValue >= size)
        {
        if (arrayInfo.IsFixedCount())
            return ECOBJECTS_STATUS_Error;

        UInt numToInsert = (indexValue + 1) - size;
        status =  instance.AddArrayElements (asBuffer, numToInsert);    
        if (ECOBJECTS_STATUS_Success != status)
            return status;

        if (arrayInfo.IsStructArray())
            {
            ECClassCR    ecClass     = instance.GetClass();

            wchar_t buffer [NUM_INDEX_BUFFER_CHARS+1];
            wcsncpy(buffer, asBuffer, NUM_INDEX_BUFFER_CHARS);
            ECPropertyP prop = getProperty (ecClass, asBuffer, buffer);
            
            if (!prop->GetIsArray())
                return ECOBJECTS_STATUS_Error;

            ArrayECPropertyP arrayProp = dynamic_cast<ArrayECPropertyP>(prop);
            if (!arrayProp)
                return ECOBJECTS_STATUS_Error;

            ECClassCP structClass = arrayProp->GetStructElementType();

            StandaloneECEnablerPtr standaloneEnabler = instance.GetEnablerR().GetEnablerForStructArrayMember (structClass->GetSchema().GetSchemaKey(), structClass->GetName().c_str());
            if (standaloneEnabler.IsNull())
                return ECOBJECTS_STATUS_Error;

            ECValue                     arrayEntryVal;

            for (UInt32 i=0; i<numToInsert; i++)
                {
                // only set new struct value if AddArrayElements did not already set it
                if (ECOBJECTS_STATUS_Success != instance.GetValue (arrayEntryVal, asBuffer, size+i) || arrayEntryVal.IsNull ())
                    {
                    arrayEntryVal.SetStruct (standaloneEnabler->CreateInstance().get());
                    if (SUCCESS != instance.SetValue (asBuffer, arrayEntryVal, size+i))
                        return ECOBJECTS_STATUS_Error;
                    }
                }
            }
        }

    if (arrayInfo.IsPrimitiveArray())
        return instance.SetValue (asBuffer, v, indexValue);

    // must be a struct array
    if (NULL == wcschr (pos2, L'.'))
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
static ECObjectsStatus setECValueInInstance (ECValueCR v, IECInstanceR instance, WCharCP managedPropertyAccessor)
    {
    WString asBufferStr;

    wchar_t asBuffer[NUM_ACCESSSTRING_BUFFER_CHARS+1];
    wchar_t indexBuffer[NUM_INDEX_BUFFER_CHARS+1];

    return setECValueUsingFullAccessString (asBuffer, indexBuffer, v, instance, managedPropertyAccessor);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Bill.Steinbock                  10/2010
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus ECInstanceInteropHelper::SetValue  (IECInstanceR instance, WCharCP managedPropertyAccessor, ECValueCR value)
    {
    return setECValueInInstance (value, instance, managedPropertyAccessor);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    CaseyMullen     01/10
+---------------+---------------+---------------+---------------+---------------+------*/       
ECObjectsStatus ECInstanceInteropHelper::SetLongValue (IECInstanceR instance, WCharCP managedPropertyAccessor, Int64 value)
    {
    ECValue v(value);
    return setECValueInInstance (v, instance, managedPropertyAccessor);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    CaseyMullen     01/10
+---------------+---------------+---------------+---------------+---------------+------*/       
ECObjectsStatus ECInstanceInteropHelper::SetIntegerValue (IECInstanceR instance, WCharCP managedPropertyAccessor, int value)
    {
    ECValue v(value);
    return setECValueInInstance (v, instance, managedPropertyAccessor);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Bill.Steinbock                  03/2010
+---------------+---------------+---------------+---------------+---------------+------*/       
ECObjectsStatus ECInstanceInteropHelper::SetDoubleValue (IECInstanceR instance, WCharCP managedPropertyAccessor, double value)
    {
    ECValue v(value);
    return setECValueInInstance (v, instance, managedPropertyAccessor);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    CaseyMullen     01/10
+---------------+---------------+---------------+---------------+---------------+------*/       
ECObjectsStatus ECInstanceInteropHelper::SetStringValue  (IECInstanceR instance, WCharCP managedPropertyAccessor, WCharCP value)
    {
    ECValue v(value, false);
    return setECValueInInstance (v, instance, managedPropertyAccessor);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Bill.Steinbock                  03/2010
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus ECInstanceInteropHelper::SetBooleanValue  (IECInstanceR instance, WCharCP managedPropertyAccessor, bool value)
    {
    ECValue v(value);
    return setECValueInInstance (v, instance, managedPropertyAccessor);
    }
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Bill.Steinbock                  03/2010
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus ECInstanceInteropHelper::SetPoint2DValue  (IECInstanceR instance, WCharCP managedPropertyAccessor, DPoint2dCR value)
    {
    ECValue v(value);
    return setECValueInInstance (v, instance, managedPropertyAccessor);
    }
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Bill.Steinbock                  03/2010
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus ECInstanceInteropHelper::SetPoint3DValue  (IECInstanceR instance, WCharCP managedPropertyAccessor, DPoint3dCR value)
    {
    ECValue v(value);
    return setECValueInInstance (v, instance, managedPropertyAccessor);
    }
   
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Bill.Steinbock                  03/2010
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus ECInstanceInteropHelper::SetDateTimeValue (IECInstanceR instance, WCharCP managedPropertyAccessor, SystemTime& value)
    {
    ECValue v(value);
    return setECValueInInstance (v, instance, managedPropertyAccessor);
    }
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Bill.Steinbock                  03/2010
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus ECInstanceInteropHelper::SetDateTimeTicks (IECInstanceR instance, WCharCP managedPropertyAccessor, Int64 value)
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
ECObjectsStatus ECInstanceInteropHelper::GetLong (IECInstanceCR instance, Int64 & value, ECValueAccessorCR accessor)
    {
    ECValue v;

    ECObjectsStatus status = instance.GetValueUsingAccessor (v, accessor);
    if (status == ECOBJECTS_STATUS_Success)
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
    if (status == ECOBJECTS_STATUS_Success)
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
    if (status == ECOBJECTS_STATUS_Success)        
        value = v.GetDouble();
        
    return status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    CaseyMullen     09/09
+---------------+---------------+---------------+---------------+---------------+------*/       
ECObjectsStatus ECInstanceInteropHelper::GetString (IECInstanceCR instance, WStringR value, ECValueAccessorCR accessor)
    {
    ECValue v;

    ECObjectsStatus status = instance.GetValueUsingAccessor (v, accessor);
    if (status == ECOBJECTS_STATUS_Success)
        value = v.GetString();

    return status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Bill.Steinbock                  03/2010
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus ECInstanceInteropHelper::GetBoolean (IECInstanceCR instance, bool & value, ECValueAccessorCR accessor)
    {
    ECValue v;

    ECObjectsStatus status = instance.GetValueUsingAccessor (v, accessor);
    if (status == ECOBJECTS_STATUS_Success)
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
    if (status == ECOBJECTS_STATUS_Success)
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
    if (status == ECOBJECTS_STATUS_Success)
        value = v.GetPoint3D();
        
    return status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Bill.Steinbock                  03/2010
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus ECInstanceInteropHelper::GetDateTime (IECInstanceCR instance, SystemTime & value, ECValueAccessorCR accessor)
    {
    ECValue v;

    ECObjectsStatus status = instance.GetValueUsingAccessor (v, accessor);
    if (status == ECOBJECTS_STATUS_Success)
        value = v.GetDateTime();
        
    return status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Bill.Steinbock                  03/2010
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus ECInstanceInteropHelper::GetDateTimeTicks (IECInstanceCR instance, Int64 & value, ECValueAccessorCR accessor)
    {
    ECValue v;

    ECObjectsStatus status = instance.GetValueUsingAccessor (v, accessor);
    if (status == ECOBJECTS_STATUS_Success)
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
ECObjectsStatus ECInstanceInteropHelper::SetLongValue (IECInstanceR instance, ECValueAccessorCR accessor, Int64 value)
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
ECObjectsStatus ECInstanceInteropHelper::SetStringValue  (IECInstanceR instance, ECValueAccessorCR accessor, WCharCP value)
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
ECObjectsStatus ECInstanceInteropHelper::SetDateTimeValue (IECInstanceR instance, ECValueAccessorCR accessor, SystemTime& value)
    {
    ECValue v(value);
    return  instance.SetValueUsingAccessor (accessor, v);
    }
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Bill.Steinbock                  06/2011
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus ECInstanceInteropHelper::SetDateTimeTicks (IECInstanceR instance, ECValueAccessorCR accessor, Int64 value)
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
    if (status != ECOBJECTS_STATUS_Success)
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
    UInt32 propertyIndex = accessor.DeepestLocation().GetPropertyIndex();
    if (1 < accessor.GetDepth())
        {
        ECValue v;
        ECValueAccessor newAccessor(accessor);
        newAccessor.PopLocation();
        status = instance.GetValueUsingAccessor(v, newAccessor);
        if (ECOBJECTS_STATUS_Success != status)
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
ECN::ECEnablerP                  ECInstanceInteropHelper::GetEnablerForStructArrayEntry (IECInstanceR instance, ECValueAccessorR arrayMemberAccessor, SchemaKeyCR schemaKey, WCharCP className)
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
        LOG.errorv (L"Unable to locate a standalone enabler for class %ls", className);
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
ECObjectsStatus  ECInstanceInteropHelper::GetStructArrayEntry (ECN::ECValueAccessorR structArrayEntryValueAccessor, IECInstanceR instance, UInt32 index, ECN::ECValueAccessorCR structArrayValueAccessor, 
                                                             bool createPropertyIfNotFound, WCharCP wcharAccessString, 
                                                             SchemaKeyCR schemaKey, WCharCP className)
    {
    ECN::ECEnablerR structArrayEnabler = *(const_cast<ECN::ECEnablerP>(structArrayValueAccessor.DeepestLocationCR().GetEnabler()));
    ECN::StandaloneECEnablerPtr standaloneEnabler = structArrayEnabler.GetEnablerForStructArrayMember (schemaKey, className);
    if (standaloneEnabler.IsNull())
        {
        LOG.errorv (L"Unable to locate a standalone enabler for class \" %ls \"", className);
        return ECOBJECTS_STATUS_EnablerNotFound;
        }

    ECN::ECValue  arrayVal;
    instance.GetValueUsingAccessor (arrayVal, structArrayValueAccessor);

    ArrayInfo   arrayInfo  = arrayVal.GetArrayInfo();
    UInt32      arrayCount = arrayInfo.GetCount();

    // adjust the ECVAlueAccessor to include the array index
    ECN::ECValueAccessor arrayEntryValueAccessor (structArrayValueAccessor);
    arrayEntryValueAccessor.DeepestLocation ().SetArrayIndex (index);

    if (arrayCount <= index)
        {
        // see if we are allowed to add a new strct array instance
        if (!createPropertyIfNotFound)
            return ECOBJECTS_STATUS_Error;

        // only proceed if not read only instance
        if (instance.IsReadOnly())
            return ECOBJECTS_STATUS_UnableToSetReadOnlyInstance;

        ECN::IECInstancePtr parentNativeInstance = getParentNativeInstance (&instance, structArrayValueAccessor);
        if (parentNativeInstance.IsNull())
            {
            LOG.error (L"Unable to get native instance when processing ECInstanceInteropHelper::GetStructArrayEntry");
            return ECOBJECTS_STATUS_Error;
            }

        ::UInt32 numToInsert = (index + 1) - arrayCount;
        if (ECN::ECOBJECTS_STATUS_Success != parentNativeInstance->AddArrayElements (wcharAccessString, numToInsert))
            {
            LOG.errorv(L"Unable to add array element(s) to native instance - access string \"%ls\"", structArrayValueAccessor.GetManagedAccessString().c_str());
            return ECOBJECTS_STATUS_UnableToAddStructArrayMember;
            }

        ECN::ECValue  arrayEntryVal;
        for (::UInt32 i=0; i<numToInsert; i++)
            {
            arrayEntryVal.SetStruct (standaloneEnabler->CreateInstance().get());
            if (SUCCESS != parentNativeInstance->SetValue (wcharAccessString, arrayEntryVal, arrayCount+i))
                return ECOBJECTS_STATUS_UnableToSetStructArrayMemberInstance;
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
                return ECOBJECTS_STATUS_Error;

            // only proceed if not read only instance
            if (instance.IsReadOnly())
                return ECOBJECTS_STATUS_UnableToSetReadOnlyInstance;

            arrayEntryVal.SetStruct (standaloneEnabler->CreateInstance().get());

            ECN::IECInstancePtr parentNativeInstance = getParentNativeInstance (&instance, structArrayValueAccessor);
            if (parentNativeInstance.IsNull())
                {
                LOG.error (L"Unable to get native instance when processing ECInstanceInteropHelper::GetStructArrayEntry");
                return ECOBJECTS_STATUS_Error;
                }

            if (SUCCESS != parentNativeInstance->SetValue (wcharAccessString, arrayEntryVal, index))
                return ECOBJECTS_STATUS_UnableToSetStructArrayMemberInstance;
            }
        }

    structArrayEntryValueAccessor.Clone (arrayEntryValueAccessor);
    return ECOBJECTS_STATUS_Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Dylan.Rush                      1/11
+---------------+---------------+---------------+---------------+---------------+------*/
bool  ECInstanceInteropHelper::IsCalculatedECProperty  (IECInstanceCR instance, int propertyIndex)
    {
    WCharCP accessor;
    if (ECOBJECTS_STATUS_Success != instance.GetEnabler().GetAccessString (accessor, (UInt32) propertyIndex))
        return false;

    ECClassCR ecClass = instance.GetClass();

    wchar_t buffer [NUM_INDEX_BUFFER_CHARS+1];

    wcsncpy(buffer, accessor, NUM_INDEX_BUFFER_CHARS);

    ECPropertyP ecProperty = getProperty (ecClass, accessor, buffer);

    if (NULL == ecProperty)
        return false;

    return ecProperty->IsDefined (L"CalculatedECPropertySpecification");
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Dylan.Rush                      1/11
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus ECInstanceInteropHelper::SetValueByIndex (IECInstanceR instance, int propertyIndex, int arrayIndex, ECValueCR value)
    {
    if (-1 == arrayIndex)
        return instance.SetValue ((UInt32) propertyIndex, value);

    ECN::ECValue v;
    instance.GetValue (v, propertyIndex);
    UInt32 count = v.GetArrayInfo().GetCount();
    if ((UInt32)arrayIndex >= count)
        {
        ECObjectsStatus status;
        UInt32 size = 1 + ((UInt32)arrayIndex - count);
        WCharCP accessString;
        status = instance.GetEnabler().GetAccessString (accessString, propertyIndex);
        if (ECOBJECTS_STATUS_Success != status)
            return status;
        status = instance.AddArrayElements (accessString, size);
        if (ECN::ECOBJECTS_STATUS_Success != status)
            return status;
        }
    return instance.SetValue ((UInt32) propertyIndex, value, (UInt32) arrayIndex);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Dylan.Rush                      1/11
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus ECInstanceInteropHelper::GetValueByIndex (ECValueR value, IECInstanceCR instance, int propertyIndex, int arrayIndex)
    {
    if (-1 == arrayIndex)
        return instance.GetValue (value, (UInt32) propertyIndex);

    ECN::ECValue v;
    instance.GetValue (v, propertyIndex);
    UInt32 count = v.GetArrayInfo().GetCount();
    if ((UInt32)propertyIndex >= count)
        {
        value.SetToNull ();
        }
    return instance.GetValue (value, (UInt32) propertyIndex, (UInt32) arrayIndex);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Adam.Klatzkin                   01/2010
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus                 IECInstance::InsertArrayElements (WCharCP propertyAccessString, UInt32 index, UInt32 size)
    {
    UInt32 propIdx;
    ECObjectsStatus status = GetEnabler().GetPropertyIndex (propIdx, propertyAccessString);
    return ECOBJECTS_STATUS_Success == status ? InsertArrayElements (propIdx, index, size) : status;
    } 
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Adam.Klatzkin                   01/2010
+---------------+---------------+---------------+---------------+---------------+------*/
EC_INLINE ECObjectsStatus                 IECInstance::AddArrayElements (WCharCP propertyAccessString, UInt32 size)
    {
    UInt32 propIdx;
    ECObjectsStatus status = GetEnabler().GetPropertyIndex (propIdx, propertyAccessString);
    return ECOBJECTS_STATUS_Success == status ? AddArrayElements (propIdx, size) : status;
    }        

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Adam.Klatzkin                   01/2010
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus                 IECInstance::RemoveArrayElement (WCharCP propertyAccessString, UInt32 index)
    {
    UInt32 propIdx;
    ECObjectsStatus status = GetEnabler().GetPropertyIndex (propIdx, propertyAccessString);
    return ECOBJECTS_STATUS_Success == status ? RemoveArrayElement (propIdx, index) : status;
    } 
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Adam.Klatzkin                   01/2010
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus                 IECInstance::ClearArray (WCharCP propertyAccessString)
    {
    UInt32 propertyIndex;
    ECObjectsStatus status = GetEnabler().GetPropertyIndex (propertyIndex, propertyAccessString);
    return ECOBJECTS_STATUS_Success == status ? ClearArray (propertyIndex) : status;
    }           

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   11/12
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus IECInstance::ClearArray (UInt32 propIdx)                                    { return _ClearArray (propIdx); }
ECObjectsStatus IECInstance::InsertArrayElements (UInt32 propIdx, UInt32 idx, UInt32 size)  { return _InsertArrayElements (propIdx, idx, size); }
ECObjectsStatus IECInstance::AddArrayElements (UInt32 propIdx, UInt32 size)                 { return _AddArrayElements (propIdx, size); }
ECObjectsStatus IECInstance::RemoveArrayElement (UInt32 propIdx, UInt32 idx)                { return _RemoveArrayElement (propIdx, idx); }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    CaseyMullen     09/09
+---------------+---------------+---------------+---------------+---------------+------*/
WString                         IECInstance::ToString (WCharCP indent) const
    {
    return _ToString (indent);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   10/12
+---------------+---------------+---------------+---------------+---------------+------*/
static bool getInstanceLabelPropertyNameFromClass (WStringR propertyName, ECClassCR ecClass)
    {
    IECInstancePtr caInstance = ecClass.GetCustomAttribute (L"InstanceLabelSpecification");
    if (caInstance.IsValid())
        {
        ECValue value;
        if (SUCCESS == caInstance->GetValue (value, L"PropertyName") && !value.IsNull())
            {
            propertyName = value.GetString();
            return true;
            }
        }

    FOR_EACH (ECClassCP baseClass, ecClass.GetBaseClasses())
        {
        if (getInstanceLabelPropertyNameFromClass (propertyName, *baseClass))
            return true;
        }

    return false;
    }

// ordered by precedence
static const WCharCP s_standardInstanceLabelPropertyNames[] =
    {
    L"DisplayLabel", L"DISPLAYLABEL", L"displaylabel", L"Name", L"NAME", L"name", NULL
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Bill.Steinbock                  05/2011
+---------------+---------------+---------------+---------------+---------------+------*/
bool                         IECInstance::GetInstanceLabelPropertyName (WStringR propName) const
    {
    ECClassCR ecClass = GetClass();
    if (getInstanceLabelPropertyNameFromClass (propName, ecClass))
        return true;

    const WCharCP* standardName = s_standardInstanceLabelPropertyNames;
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
ECObjectsStatus                 IECInstance::_GetDisplayLabel (WString& displayLabel) const
    {
    WString propertyName;
    if (!GetInstanceLabelPropertyName (propertyName))
        return ECOBJECTS_STATUS_Error;

    ECN::ECValue ecValue;
    if (SUCCESS == GetValue (ecValue, propertyName.c_str()) && !ecValue.IsNull())
        {
        displayLabel = ecValue.GetString();
        return ECOBJECTS_STATUS_Success;
        }

    return  ECOBJECTS_STATUS_Error;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Bill.Steinbock                  05/2011
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus                 IECInstance::GetDisplayLabel (WString& displayLabel) const
    {
    return  _GetDisplayLabel (displayLabel);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Bill.Steinbock                  05/2011
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus                 IECInstance::_SetDisplayLabel (WCharCP displayLabel)    
    {
    WString propertyName;
    if (!GetInstanceLabelPropertyName (propertyName))
        return ECOBJECTS_STATUS_Error;

    ECN::ECValue ecValue;
    ecValue.SetString (displayLabel, false);

    if (SUCCESS == SetValue (propertyName.c_str(), ecValue))
        return ECOBJECTS_STATUS_Success;

    return  ECOBJECTS_STATUS_Error;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Bill.Steinbock                  05/2011
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus                 IECInstance::SetDisplayLabel (WCharCP displayLabel)    
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
ECObjectsStatus                 IECRelationshipInstance::GetSourceOrderId (Int64& sourceOrderId) const
    {
    return _GetSourceOrderId (sourceOrderId);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Bill.Steinbock                  05/2011
+---------------+---------------+---------------+---------------+---------------+------*/
 ECObjectsStatus                IECRelationshipInstance::GetTargetOrderId (Int64& targetOrderId) const
    {
    return _GetTargetOrderId (targetOrderId);
    }

///////////////////////////////////////////////////////////////////////////////////////////////
//   IECWipRelationshipInstance
///////////////////////////////////////////////////////////////////////////////////////////////
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Bill.Steinbock                  04/2011
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus                   IECWipRelationshipInstance::SetName (WCharCP name)   
    {
    return _SetName (name);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Bill.Steinbock                  04/2011
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus                   IECWipRelationshipInstance::SetSourceOrderId (Int64 sourceOrderId)
    {
    return _SetSourceOrderId (sourceOrderId);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Bill.Steinbock                  04/2011
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus                   IECWipRelationshipInstance::SetTargetOrderId (Int64 targetOrderId)
    {
    return _SetTargetOrderId (targetOrderId);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   04/10
+---------------+---------------+---------------+---------------+---------------+------*/
void              convertByteArrayToString (WStringR outString, const byte *byteData, size_t numBytes)
    {
    static const wchar_t    base64Chars[] = {L"ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/"};

    outString.clear();

    // from each 3 bytes we get 4 output characters, rounded up.
    for (size_t iByte=0; iByte < numBytes; iByte += 3)
        {
        UInt32      nextThreeBytes = byteData[iByte] | (byteData[iByte+1] << 8) | (byteData[iByte+2] << 16);

        for (size_t jPos=0; jPos < 4; jPos++)
            {
            byte    sixBits = nextThreeBytes & 0x3f;

            if ( (iByte + jPos) < (numBytes + 1) )
                outString.append (1, base64Chars[sixBits]);
            else
                outString.append (1, L'=');
            
            nextThreeBytes = nextThreeBytes >> 6;
            }
        }
    }

typedef bvector<byte>   T_ByteArray;
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   04/10
+---------------+---------------+---------------+---------------+---------------+------*/
bool   convertStringToByteArray (T_ByteArray& byteData, WCharCP stringData)
    {
    // the length of stringData should be a muttiple of four.
    size_t  stringLen = wcslen (stringData);
    if (0 != (stringLen % 4))
        return false;

    // from each 4 characters we get 3 byte values.
    for (size_t iPos=0; iPos < stringLen; iPos+= 4)
        {
        Int32   nextThreeBytes = 0;
        int     numBytesToPush = 3;
        int     shift;
        int     jPos;
        for (jPos=0, shift=0; jPos < 4; jPos++, shift += 6)
            {
            wchar_t charValue = stringData[iPos+jPos];
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

        byte*   bytes = (byte*)&nextThreeBytes;
        byteData.push_back (*bytes);
        if (numBytesToPush > 1)
            byteData.push_back (*(bytes+1));
        if (numBytesToPush > 2)
            byteData.push_back (*(bytes+2));
        }

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   04/10
+---------------+---------------+---------------+---------------+---------------+------*/
static void                 AppendAccessString (WString& compoundAccessString, WString& baseAccessString, const WString& propertyName)
    {
    compoundAccessString = baseAccessString;
    compoundAccessString.append (propertyName);
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
    WString                 m_fullSchemaName;
    BeXmlNodeR              m_xmlNode;
    ECSchemaCP              m_schema;
    ECInstanceReadContextR  m_context;

public:
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   10/2011
+---------------+---------------+---------------+---------------+---------------+------*/
InstanceXmlReader (ECInstanceReadContextR context, BeXmlNodeR xmlNode)
    : m_context (context), m_schema (NULL), m_xmlNode (xmlNode)
    {
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   10/2011
+---------------+---------------+---------------+---------------+---------------+------*/
ECSchemaCP       GetSchema()
    {
    if (NULL != m_schema)
        return m_schema;
    
    SchemaKey key;
    if (ECOBJECTS_STATUS_Success != SchemaKey::ParseSchemaFullName(key, m_fullSchemaName.c_str()))
        return NULL;
    
    m_schema = m_context.FindSchemaCP(key, SCHEMAMATCHTYPE_LatestCompatible);//Abeesh: Preserving old behavior. Ideally it should be exact 
    return m_schema; 
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   10/2011
+---------------+---------------+---------------+---------------+---------------+------*/
InstanceReadStatus      ReadInstance (IECInstancePtr& ecInstance)
    {
    // When this is called, m_xmlNode should be a BeXmlNode that is the class name, with a name space corresponding to the schema name.
    InstanceReadStatus      ixrStatus;
    ECClassCP               ecClass;
    if (INSTANCE_READ_STATUS_Success != (ixrStatus = GetInstance (ecClass, ecInstance)))
        return ixrStatus;

    // this reads the property members and consumes the XmlNodeType_EndElement corresponding to this XmlNodeType_Element.
    return ReadInstanceOrStructMembers (*ecClass, ecInstance.get(), NULL, m_xmlNode);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   10/2011
+---------------+---------------+---------------+---------------+---------------+------*/
InstanceReadStatus      GetInstance (ECClassCP& ecClass, IECInstancePtr& ecInstance)
    {
    // get the node name
    WString className (m_xmlNode.GetName(), true);

    ECSchemaCP schema = NULL;
    
    // get the xmlns name, if there is one.
    Utf8CP  schemaName;
    if (NULL != (schemaName = m_xmlNode.GetNamespace()) && 0 != BeStringUtilities::Stricmp (schemaName, ECXML_URI_2_0))
        {
        m_fullSchemaName.AssignUtf8 (schemaName);
        schema = GetSchema ();
        }
    else
        schema = &(m_context.GetFallBackSchema ());

    if (NULL == schema)
        {
        LOG.errorv (L"Failed to locate ECSchema %ls", m_fullSchemaName.c_str());
        return INSTANCE_READ_STATUS_ECSchemaNotFound;
        }

    // see if we can find the class from the schema.
    ECClassCP    foundClass;
    if (NULL == (foundClass = schema->GetClassCP (className.c_str())))
        {
        ECSchemaReferenceListCR refList = schema->GetReferencedSchemas();
        SchemaKey key;
        if (ECOBJECTS_STATUS_Success == SchemaKey::ParseSchemaFullName (key, m_fullSchemaName.c_str()))
            {
            ECSchemaReferenceList::const_iterator schemaIterator = refList.find (key);
            if (schemaIterator != refList.end())
                foundClass = schemaIterator->second->GetClassCP (className.c_str());
            }
        else
            {
            for (ECSchemaReferenceList::const_iterator schemaIterator = refList.begin(); schemaIterator != refList.end(); schemaIterator++)
                {
                if (NULL != (foundClass = schemaIterator->second->GetClassCP (className.c_str())))
                    break;
                }
            }
        }
    if (NULL == foundClass)
        {
        LOG.errorv (L"Failed to find ECClass %ls in %ls", className.c_str(), m_fullSchemaName.c_str());
        return INSTANCE_READ_STATUS_ECClassNotFound;
        }

    ecClass = foundClass;

    ecInstance = m_context.CreateStandaloneInstance (*foundClass).get();
    
    WString instanceId;
    if (BEXML_Success == m_xmlNode.GetAttributeStringValue (instanceId, INSTANCEID_ATTRIBUTE))
        {
        ecInstance->SetInstanceId (instanceId.c_str());
        }

    IECRelationshipInstance*    relationshipInstance = dynamic_cast <IECRelationshipInstance*> (ecInstance.get());

    // if relationship, need the attributes used in relationships.
    if (NULL != relationshipInstance)
        {
        // see if we can find the attributes corresponding to the relationship instance ids.
        WString relationshipClassName;
        if (BEXML_Success == m_xmlNode.GetAttributeStringValue (instanceId, SOURCEINSTANCEID_ATTRIBUTE))
            {
#if defined (NEEDSWORK_RELATIONSHIP)
            relationshipInstance->SetSourceInstanceId (sourceInstanceId);
#endif
            }

        if (BEXML_Success == m_xmlNode.GetAttributeStringValue (relationshipClassName, SOURCECLASS_ATTRIBUTE))
            {
#if defined (NEEDSWORK_RELATIONSHIP)
            relationshipInstance->SetSourceClass (sourceClass);
#endif
            }

        if (BEXML_Success == m_xmlNode.GetAttributeStringValue (instanceId, TARGETINSTANCEID_ATTRIBUTE))
            {
#if defined (NEEDSWORK_RELATIONSHIP)
            relationshipInstance->SetTargetInstanceId (sourceInstanceId);
#endif
            }

        if (BEXML_Success == m_xmlNode.GetAttributeStringValue (relationshipClassName, TARGETCLASS_ATTRIBUTE))
            {
#if defined (NEEDSWORK_RELATIONSHIP)
            relationshipInstance->SetTargetClass (sourceClass);
#endif
            }
        }

    return INSTANCE_READ_STATUS_Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   10/2011
+---------------+---------------+---------------+---------------+---------------+------*/
InstanceReadStatus   ReadInstanceOrStructMembers (ECClassCR ecClass, IECInstanceP ecInstance, WString* baseAccessString, BeXmlNodeR instanceNode)
    {
    // On entry, the instanceNode is the XML node that contains the children that have propertyValues.
    for (BeXmlNodeP propertyValueNode = instanceNode.GetFirstChild (BEXMLNODE_Element); NULL != propertyValueNode; propertyValueNode = propertyValueNode->GetNextSibling(BEXMLNODE_Element))
        {
        InstanceReadStatus   propertyStatus = ReadPropertyValue (ecClass, ecInstance, baseAccessString, *propertyValueNode);

        // if property not found, ReadPropertyValue warns, so just continue..
        if (INSTANCE_READ_STATUS_PropertyNotFound == propertyStatus)
            continue;
        else if (INSTANCE_READ_STATUS_TypeMismatch == propertyStatus)
            continue;
        else if (INSTANCE_READ_STATUS_Success != propertyStatus)
            return propertyStatus;
        }

    return INSTANCE_READ_STATUS_Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   10/2011
+---------------+---------------+---------------+---------------+---------------+------*/
InstanceReadStatus   ReadPropertyValue (ECClassCR ecClass, IECInstanceP ecInstance, WString* baseAccessString, BeXmlNodeR propertyValueNode)
    {
    // on entry, propertyValueNode is the XML node for the property value.
    WString     propertyName (propertyValueNode.GetName(), true);

    // try to find the property in the class.
    ECPropertyP ecProperty;
    if (NULL == (ecProperty = ecClass.GetPropertyP (propertyName)))
        {
        LOG.warningv (L"No ECProperty '%ls' found in ECClass '%ls'. Value will be ignored.", propertyName.c_str(), ecClass.GetName().c_str());
        return INSTANCE_READ_STATUS_PropertyNotFound;
        }

    PrimitiveECPropertyP    primitiveProperty;
    ArrayECPropertyP        arrayProperty;
    StructECPropertyP       structProperty;
    if (NULL != (primitiveProperty = ecProperty->GetAsPrimitivePropertyP()))
        return ReadPrimitivePropertyValue (primitiveProperty, ecInstance, baseAccessString, propertyValueNode);
                //Above is good, if SkipToElementEnd() is returned from ReadPrimitiveValue.
    else if (NULL != (arrayProperty = ecProperty->GetAsArrayPropertyP()))
        return ReadArrayPropertyValue (arrayProperty, ecInstance, baseAccessString, propertyValueNode);
    else if (NULL != (structProperty = ecProperty->GetAsStructPropertyP()))
        return ReadEmbeddedStructPropertyValue (structProperty, ecInstance, baseAccessString, propertyValueNode);

    // should be one of those!
    BeAssert (false);
    return INSTANCE_READ_STATUS_BadECProperty;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   10/2011
+---------------+---------------+---------------+---------------+---------------+------*/
InstanceReadStatus   ReadPrimitivePropertyValue (PrimitiveECPropertyP primitiveProperty, IECInstanceP ecInstance, WString* baseAccessString, BeXmlNodeR propertyValueNode)
    {
    // on entry, propertyValueNode is the xml node for the primitive property value.
    PrimitiveType        propertyType = primitiveProperty->GetType();
    InstanceReadStatus   ixrStatus;
    ECValue              ecValue;
    if (INSTANCE_READ_STATUS_Success != (ixrStatus = ReadPrimitiveValue (ecValue, propertyType, propertyValueNode, m_context.GetSerializedPrimitiveType (*primitiveProperty))))
        return ixrStatus;

    if(ecValue.IsUninitialized())
        {
        //A malformed value was found.  A warning was shown; just move on.
        return INSTANCE_READ_STATUS_Success;
        }

    ECObjectsStatus setStatus;
    if (NULL == baseAccessString)
        {
        setStatus = ecInstance->SetInternalValue (primitiveProperty->GetName().c_str(), ecValue);

        if (ECOBJECTS_STATUS_Success != setStatus && ECOBJECTS_STATUS_PropertyValueMatchesNoChange != setStatus)
            LOG.warningv(L"Unable to set value for property %ls", primitiveProperty->GetName().c_str());
        }
    else
        {
        WString compoundAccessString;
        AppendAccessString (compoundAccessString, *baseAccessString, primitiveProperty->GetName());
        setStatus = ecInstance->SetInternalValue (compoundAccessString.c_str(), ecValue);

        if (ECOBJECTS_STATUS_Success != setStatus && ECOBJECTS_STATUS_PropertyValueMatchesNoChange != setStatus)
            LOG.warningv(L"Unable to set value for property %ls", compoundAccessString.c_str());
        }

    BeAssert (ECOBJECTS_STATUS_Success == setStatus || ECOBJECTS_STATUS_PropertyValueMatchesNoChange == setStatus);

    return INSTANCE_READ_STATUS_Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   10/2011
+---------------+---------------+---------------+---------------+---------------+------*/
InstanceReadStatus   ReadArrayPropertyValue (ArrayECPropertyP arrayProperty, IECInstanceP ecInstance, WString* baseAccessString, BeXmlNodeR propertyValueNode)
    {
    // on entry, propertyValueNode is the xml node for the primitive property value.
    WString    accessString;
    if (NULL == baseAccessString)
        accessString = arrayProperty->GetName();    
    else
        AppendAccessString (accessString, *baseAccessString, arrayProperty->GetName());

    // start the address out as zero.
    UInt32      index = 0;

    // we have to find out what type the array is.
    ArrayKind   arrayKind = arrayProperty->GetKind();
    if (ARRAYKIND_Primitive == arrayKind)
        {
        PrimitiveType   memberType = arrayProperty->GetPrimitiveElementType();

        bool            isFixedSizeArray = false;

        if (arrayProperty->GetMinOccurs() == arrayProperty->GetMaxOccurs())
            isFixedSizeArray = true; //PrimitiveTypeIsFixedSize (memberType);

        // step through the nodes. Each should be a primitive value type like <int>value</int>
        for (BeXmlNodeP arrayValueNode = propertyValueNode.GetFirstChild (BEXMLNODE_Element); NULL != arrayValueNode; arrayValueNode = arrayValueNode->GetNextSibling(BEXMLNODE_Element))
            {
            PrimitiveType serializedMemberType = m_context.GetSerializedPrimitiveArrayType (*arrayProperty);
            if (memberType == serializedMemberType && !ValidateArrayPrimitiveType (arrayValueNode->GetName(), memberType))
                {
                LOG.warningv(L"Incorrectly formatted array element found in array %ls.  Expected: %hs  Found: %hs", accessString.c_str(), GetPrimitiveTypeString (memberType), arrayValueNode->GetName());
                continue;
                }

            if ( !isFixedSizeArray)
                ecInstance->AddArrayElements (accessString.c_str(), 1);

            // read it, populating the ECInstance using accessString and arrayIndex.
            InstanceReadStatus      ixrStatus;
            ECValue                 ecValue;
            if (INSTANCE_READ_STATUS_Success == (ixrStatus = ReadPrimitiveValue (ecValue, memberType, *arrayValueNode, serializedMemberType)))
                {
                // If we failed to read the value above, the array member will have been allocated but left null.
                // This allows any default value to be applied to it via CalculatedECPropertySpecification, 
                // and is less surprising than the old behavior which would have omitted the member entirely.
                ECObjectsStatus   setStatus = ecInstance->SetInternalValue (accessString.c_str(), ecValue, index);
                if (ECOBJECTS_STATUS_Success != setStatus && ECOBJECTS_STATUS_PropertyValueMatchesNoChange != setStatus)   
                    {
                    BeAssert (false);
                    return INSTANCE_READ_STATUS_CantSetValue;
                    }
                }

            // increment the array index.
            index++;
            }
        }

    else if (ARRAYKIND_Struct == arrayKind)
        {
        ECClassCP   structMemberType = arrayProperty->GetStructElementType();
        for (BeXmlNodeP arrayValueNode = propertyValueNode.GetFirstChild (BEXMLNODE_Element); NULL != arrayValueNode; arrayValueNode = arrayValueNode->GetNextSibling(BEXMLNODE_Element))
            {
            // the Name of each node element is the class name of structMemberType.
            // For polymorphic arrays, the Name might also be the name of a class that has structMemberType as a BaseType.
            ECClassCP   thisMemberType;
            WString     arrayMemberType (arrayValueNode->GetName(), true);
            if (NULL == (thisMemberType = ValidateArrayStructType (arrayMemberType.c_str(), structMemberType)))
                continue;


            InstanceReadStatus ixrStatus;
            if (INSTANCE_READ_STATUS_Success != (ixrStatus = ReadStructArrayMember (*thisMemberType, ecInstance, accessString, index, *arrayValueNode)))
                return ixrStatus;

            // increment the array index.
            index++;

            }
        }
    
    return INSTANCE_READ_STATUS_Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   10/2011
+---------------+---------------+---------------+---------------+---------------+------*/
InstanceReadStatus   ReadEmbeddedStructPropertyValue (StructECPropertyP structProperty, IECInstanceP ecInstance, WString* baseAccessString, BeXmlNodeR propertyValueNode)
    {
    // empty element OK for struct - all members are null.
    WString    thisAccessString;
    if (NULL != baseAccessString)
        AppendAccessString (thisAccessString, *baseAccessString, structProperty->GetName());
    else
        thisAccessString = structProperty->GetName().c_str();
    thisAccessString.append (L".");

    ICustomECStructSerializerP customECStructSerializerP = CustomStructSerializerManager::GetManager().GetCustomSerializer (structProperty, *ecInstance);
    if (customECStructSerializerP)
        return ReadCustomSerializedStruct (structProperty, ecInstance, baseAccessString, customECStructSerializerP, propertyValueNode);

    return ReadInstanceOrStructMembers (structProperty->GetType(), ecInstance, &thisAccessString, propertyValueNode);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Bill.Steinbock                  10/2010
+---------------+---------------+---------------+---------------+---------------+------*/
InstanceReadStatus   ReadCustomSerializedStruct (StructECPropertyP structProperty, IECInstanceP ecInstance, WString* baseAccessString, ICustomECStructSerializerP customECStructSerializerP, BeXmlNodeR propertyValueNode)
    {
    WString propertyValueString;

    // empty?
    if (BEXML_Success != propertyValueNode.GetContent (propertyValueString))
        return INSTANCE_READ_STATUS_Success;

    WString    thisAccessString;
    if (NULL != baseAccessString)
        AppendAccessString (thisAccessString, *baseAccessString, structProperty->GetName());
    else
        thisAccessString = structProperty->GetName().c_str();
    thisAccessString.append (L".");

    customECStructSerializerP->LoadStructureFromString (structProperty, *ecInstance, thisAccessString.c_str(), propertyValueString.c_str());

    return INSTANCE_READ_STATUS_Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   10/2011
+---------------+---------------+---------------+---------------+---------------+------*/
#pragma warning(disable:4189) // setStatus unused if NDEBUG set.
InstanceReadStatus   ReadStructArrayMember (ECClassCR structClass, IECInstanceP owningInstance, WString& accessString, UInt32 index, BeXmlNodeR arrayMemberValue)
    {
    // On entry, arrayMemberValue is an XML Node for the element that starts the struct.

    // Create an IECInstance for the array member.
    IECInstancePtr      structInstance = m_context.CreateStandaloneInstance (structClass).get();

    InstanceReadStatus   ixrStatus;
    if (INSTANCE_READ_STATUS_Success != (ixrStatus = ReadInstanceOrStructMembers (structClass, structInstance.get(), NULL, arrayMemberValue)))
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
    if (ECOBJECTS_STATUS_Success != setStatus)
        BeAssert (ECOBJECTS_STATUS_Success == setStatus);

    return INSTANCE_READ_STATUS_Success;
    }
#pragma warning(default:4189)

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
            return INSTANCE_READ_STATUS_Success;

    // If we're able to determine that the serialized type differs from the ECProperty's type, use that information to convert to correct type
    if (serializedType != propertyType)
        {
        WString propertyValueString;
        if (BEXML_Success == primitiveValueNode.GetContent (propertyValueString))
            {
            ecValue.SetString (propertyValueString.c_str(), false);
            if (ecValue.ConvertToPrimitiveType (serializedType) && ecValue.ConvertToPrimitiveType (propertyType))
                return INSTANCE_READ_STATUS_Success;
            else if (PRIMITIVETYPE_Integer == propertyType && PRIMITIVETYPE_Long == serializedType)
                {
                // Code below will give us INT_MAX if serialized integer out of range of Int32.
                // We don't want that when converting primitive types, and it's not really helpful to users, but not sure if anyone depends on it
                // So only circumventing it for this special case when we know the serialized type
                return INSTANCE_READ_STATUS_TypeMismatch;
                }
            }
        }

    switch (propertyType)
        {
        case PRIMITIVETYPE_Binary:
            {
            T_ByteArray                     byteArray;

            // try to read the actual value.
            WString     propertyValueString;
            if (BEXML_Success != primitiveValueNode.GetContent (propertyValueString))
                return INSTANCE_READ_STATUS_Success;

            if (!convertStringToByteArray (byteArray, propertyValueString.c_str ()))
                {
                LOG.warningv(L"Type mismatch in deserialization: \"%ls\" is not Binary", propertyValueString.c_str ());
                return INSTANCE_READ_STATUS_TypeMismatch;
                }
            ecValue.SetBinary (&byteArray.front(), byteArray.size(), true);
            break;
            }

        case PRIMITIVETYPE_Boolean:
            {
            bool boolValue;
            BeXmlStatus status = primitiveValueNode.GetContentBooleanValue (boolValue);
            if (BEXML_Success != status)
                {
                if (BEXML_ContentWrongType == status)
                    return INSTANCE_READ_STATUS_TypeMismatch;
                if (BEXML_NullNodeValue == status || BEXML_NodeNotFound == status)
                    ecValue.SetToNull ();
                return INSTANCE_READ_STATUS_Success;
                }

            ecValue.SetBoolean (boolValue);
            break;
            }

        case PRIMITIVETYPE_DateTime:
            {
            Int64   ticks;
            BeXmlStatus status = primitiveValueNode.GetContentInt64Value (ticks);
            if (BEXML_Success != status)
                {
                if (BEXML_ContentWrongType == status)
                    return INSTANCE_READ_STATUS_TypeMismatch;
                if (BEXML_NullNodeValue == status || BEXML_NodeNotFound == status)
                    ecValue.SetToNull ();
                return INSTANCE_READ_STATUS_Success;
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
                    return INSTANCE_READ_STATUS_TypeMismatch;
                if (BEXML_NullNodeValue == status || BEXML_NodeNotFound == status)
                    ecValue.SetToNull ();
                return INSTANCE_READ_STATUS_Success;
                }

            ecValue.SetDouble (doubleValue);
            break;
            }

        case PRIMITIVETYPE_Integer:
            {
            Int32   intValue;
            BeXmlStatus status = primitiveValueNode.GetContentInt32Value (intValue);
            if (BEXML_Success != status)
                {
                if (BEXML_ContentWrongType == status)
                    return INSTANCE_READ_STATUS_TypeMismatch;
                if (BEXML_NullNodeValue == status || BEXML_NodeNotFound == status)
                    ecValue.SetToNull ();
                return INSTANCE_READ_STATUS_Success;
                }
            ecValue.SetInteger (intValue);
            break;
            }

        case PRIMITIVETYPE_Long:
            {
            Int64   longValue;
            BeXmlStatus status = primitiveValueNode.GetContentInt64Value (longValue);
            if (BEXML_Success != status)
                {
                if (BEXML_ContentWrongType == status)
                    return INSTANCE_READ_STATUS_TypeMismatch;
                if (BEXML_NullNodeValue == status || BEXML_NodeNotFound == status)
                    ecValue.SetToNull ();
                return INSTANCE_READ_STATUS_Success;
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
                    return INSTANCE_READ_STATUS_TypeMismatch;
                if (BEXML_NullNodeValue == status || BEXML_NodeNotFound == status)
                    ecValue.SetToNull ();
                return INSTANCE_READ_STATUS_Success;
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
                    return INSTANCE_READ_STATUS_TypeMismatch;
                if (BEXML_NullNodeValue == status || BEXML_NodeNotFound == status)
                    ecValue.SetToNull ();
                return INSTANCE_READ_STATUS_Success;
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
            WString     propertyValueString;
            BeXmlStatus status = primitiveValueNode.GetContent (propertyValueString);
            if (BEXML_Success != status)
                return INSTANCE_READ_STATUS_Success;
            ecValue.SetString (propertyValueString.c_str ());
            break;
            }

        default:
            {
            BeAssert (false);
            return INSTANCE_READ_STATUS_BadPrimitivePropertyType;
            }
        }
    
    return INSTANCE_READ_STATUS_Success;
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
ECClassCP                       ValidateArrayStructType (WCharCP typeFound, ECClassCP expectedType)
    {
    // the common case is that they're all of the expected ECClass.
    if (0 == wcscmp (typeFound, expectedType->GetName().c_str()))
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



// =====================================================================================
// InstanceXMLWriter class
// =====================================================================================
struct  InstanceXmlWriter
{
private:
    BeXmlDomR       m_xmlDom;
    BeXmlNodeP      m_rootNode;


public:
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   05/10
+---------------+---------------+---------------+---------------+---------------+------*/
InstanceXmlWriter (BeXmlDomR xmlDom, BeXmlNodeP rootNode) 
    : m_xmlDom (xmlDom), m_rootNode (rootNode)
    {
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   10/11
+---------------+---------------+---------------+---------------+---------------+------*/
InstanceWriteStatus     WriteInstance (IECInstanceCR ecInstance, bool writeInstanceId)
    {
    ECClassCR   ecClass         = ecInstance.GetClass();
    ECSchemaCR  ecSchema        = ecClass.GetSchema();
    WString     className       = ecClass.GetName();
    WString     fullSchemaName;

    fullSchemaName.Sprintf (L"%ls.%02d.%02d", ecSchema.GetName().c_str(), ecSchema.GetVersionMajor(), ecSchema.GetVersionMinor());
    Utf8String  utf8ClassName (className.c_str());
    BeXmlNodeP  instanceNode = m_xmlDom.AddNewElement (utf8ClassName.c_str(), NULL, m_rootNode);
    instanceNode->AddAttributeStringValue (XMLNS_ATTRIBUTE, fullSchemaName.c_str());

    if (writeInstanceId)
        instanceNode->AddAttributeStringValue (INSTANCEID_ATTRIBUTE, ecInstance.GetInstanceId().c_str());

    return WritePropertyValuesOfClassOrStructArrayMember (ecClass, ecInstance, NULL, *instanceNode);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   04/10
+---------------+---------------+---------------+---------------+---------------+------*/
InstanceWriteStatus     WritePropertyValuesOfClassOrStructArrayMember (ECClassCR ecClass, IECInstanceCR ecInstance, WString* baseAccessString, BeXmlNodeR rootNode)
    {
    CustomStructSerializerManagerR customStructSerializerMgr = CustomStructSerializerManager::GetManager();

    ECPropertyIterableCR    collection  = ecClass.GetProperties (true);
    FOR_EACH (ECPropertyP ecProperty, collection)
        {
        PrimitiveECPropertyP    primitiveProperty;
        ArrayECPropertyP        arrayProperty;
        StructECPropertyP       structProperty;
        InstanceWriteStatus     ixwStatus;
            
        if (NULL != (primitiveProperty = ecProperty->GetAsPrimitivePropertyP()))
            ixwStatus = WritePrimitivePropertyValue (*primitiveProperty, ecInstance, baseAccessString, rootNode);
        else if (NULL != (arrayProperty = ecProperty->GetAsArrayPropertyP()))
            ixwStatus = WriteArrayPropertyValue (*arrayProperty, ecInstance, baseAccessString, rootNode);
        else if (NULL != (structProperty = ecProperty->GetAsStructPropertyP()))
            {
            ICustomECStructSerializerP customECStructSerializerP;
            if (NULL != (customECStructSerializerP = customStructSerializerMgr.GetCustomSerializer (structProperty, ecInstance)))
                {
                WString     xmlString; 
                if (ECOBJECTS_STATUS_Success != customECStructSerializerP->GenerateXmlString (xmlString, structProperty, ecInstance, baseAccessString?baseAccessString->c_str():NULL))
                    ixwStatus = INSTANCE_WRITE_STATUS_BadPrimitivePropertyType;
                else
                    {
                    rootNode.SetContent (xmlString.c_str());
                    ixwStatus = INSTANCE_WRITE_STATUS_Success;
                    }
                }
            else
                {
                ixwStatus = WriteEmbeddedStructPropertyValue (*structProperty, ecInstance, baseAccessString, rootNode);
                }
            }

        if (INSTANCE_WRITE_STATUS_Success != ixwStatus)
            {
            BeAssert (false);
            return ixwStatus;
            }
        }

     return INSTANCE_WRITE_STATUS_Success;
     }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   04/10
+---------------+---------------+---------------+---------------+---------------+------*/
InstanceWriteStatus     WritePrimitivePropertyValue (PrimitiveECPropertyR primitiveProperty, IECInstanceCR ecInstance, WString* baseAccessString, BeXmlNodeR rootNode)
    {
    ECObjectsStatus     getStatus;
    ECValue             ecValue;
    WStringCR propertyName = primitiveProperty.GetName();

    if (NULL == baseAccessString)
        {
        getStatus = ecInstance.GetValue (ecValue, propertyName.c_str());
        }
    else
        {
        WString compoundAccessString;
        AppendAccessString (compoundAccessString, *baseAccessString, propertyName);
        getStatus = ecInstance.GetValue (ecValue, compoundAccessString.c_str());
        }

    // couldn't get, or NULL value, write nothing.
    if ( (ECOBJECTS_STATUS_Success != getStatus) || ecValue.IsNull() )
        return INSTANCE_WRITE_STATUS_Success;

    BeXmlNodeP              childNode    = rootNode.AddEmptyElement (propertyName.c_str());
    PrimitiveType           propertyType = primitiveProperty.GetType();

    return WritePrimitiveValue (ecValue, propertyType, *childNode);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   04/10
+---------------+---------------+---------------+---------------+---------------+------*/
InstanceWriteStatus     WritePrimitiveValue (ECValueCR ecValue, PrimitiveType propertyType, BeXmlNodeR propertyValueNode)
    {
    char     outString[512];

    // write the content according to type.
    switch (propertyType)
        {
        case PRIMITIVETYPE_Binary:
            {
            size_t      numBytes;
            const byte* byteData; 
            if (NULL != (byteData = ecValue.GetBinary (numBytes)))
                {
                WString    byteString;
                convertByteArrayToString (byteString, byteData, numBytes);
                propertyValueNode.SetContentFast (byteString.c_str());
                }
            return INSTANCE_WRITE_STATUS_Success;
            break;
            }

        case PRIMITIVETYPE_Boolean:
            {
            strcpy (outString, ecValue.GetBoolean () ? "True" : "False");
            break;
            }

        case PRIMITIVETYPE_DateTime:
            {
            BeStringUtilities::Snprintf (outString, "%I64d", ecValue.GetDateTimeTicks());
            break;
            }

        case PRIMITIVETYPE_Double:
            {
            BeStringUtilities::Snprintf (outString, "%.13g", ecValue.GetDouble());
            break;
            }

        case PRIMITIVETYPE_Integer:
            {
            BeStringUtilities::Snprintf (outString, "%d", ecValue.GetInteger());
            break;
            }

        case PRIMITIVETYPE_Long:
            {
            BeStringUtilities::Snprintf (outString, "%I64d", ecValue.GetLong());
            break;
            }

        case PRIMITIVETYPE_Point2D:
            {
            DPoint2d    point2d = ecValue.GetPoint2D();
            BeStringUtilities::Snprintf (outString, "%.13g,%.13g", point2d.x, point2d.y);
            break;
            }

        case PRIMITIVETYPE_Point3D:
            {
            DPoint3d    point3d = ecValue.GetPoint3D();
            BeStringUtilities::Snprintf (outString, "%.13g,%.13g,%.13g", point3d.x, point3d.y, point3d.z);
            break;
            }

        case PRIMITIVETYPE_String:
            {
            // we cant use SetContentFast here because we have no control over the content.
            propertyValueNode.SetContent (ecValue.GetString());
            return INSTANCE_WRITE_STATUS_Success;
            }

        default:
            {
            BeAssert (false);
            return INSTANCE_WRITE_STATUS_BadPrimitivePropertyType;
            }
        }

    propertyValueNode.SetContentFast (outString);
    return INSTANCE_WRITE_STATUS_Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   04/10
+---------------+---------------+---------------+---------------+---------------+------*/
#pragma warning(disable:4189) // memberClass unused if NDEBUG set.
InstanceWriteStatus     WriteArrayPropertyValue (ArrayECPropertyR arrayProperty, IECInstanceCR ecInstance, WString* baseAccessString, BeXmlNodeR propertyValueNode)
    {
    ArrayKind       arrayKind = arrayProperty.GetKind();

    WString    accessString;
    if (NULL == baseAccessString)
        accessString = arrayProperty.GetName();    
    else
        AppendAccessString (accessString, *baseAccessString, arrayProperty.GetName());

    // no members, don't write anything.
    ECValue         ecValue;
    if (SUCCESS != ecInstance.GetValue (ecValue, accessString.c_str()) || ecValue.IsNull() || ecValue.GetArrayInfo().GetCount() == 0)
        return INSTANCE_WRITE_STATUS_Success;

    UInt32 nElements = ecValue.GetArrayInfo().GetCount();

    BeXmlNodeP  arrayNode = propertyValueNode.AddEmptyElement (arrayProperty.GetName().c_str());

    InstanceWriteStatus     ixwStatus;
    if (ARRAYKIND_Primitive == arrayKind)
        {
        PrimitiveType   memberType  = arrayProperty.GetPrimitiveElementType();
        Utf8CP          typeString  = GetPrimitiveTypeString (memberType);
        for (UInt32 index=0; index < nElements ; index++)
            {
            if (SUCCESS != ecInstance.GetValue (ecValue, accessString.c_str(), index))
                break;

            BeXmlNodeP  memberNode = arrayNode->AddEmptyElement (typeString);

            // write the primitve value
            if (INSTANCE_WRITE_STATUS_Success != (ixwStatus = WritePrimitiveValue (ecValue, memberType, *memberNode)))
                {
                BeAssert (false);
                return ixwStatus;
                }
            }
        }
    else if (ARRAYKIND_Struct == arrayKind)
        {
        for (UInt32 index=0; index < nElements ; index++)
            {
            if (SUCCESS != ecInstance.GetValue (ecValue, accessString.c_str(), index))
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
            BeAssert (structClass.Is (arrayProperty.GetStructElementType()));

            BeXmlNodeP memberNode = arrayNode->AddEmptyElement (structClass.GetName().c_str());

            InstanceWriteStatus iwxStatus;
            if (INSTANCE_WRITE_STATUS_Success != (iwxStatus = WritePropertyValuesOfClassOrStructArrayMember (structClass, *structInstance.get(), NULL, *memberNode)))
                {
                BeAssert (false);
                return iwxStatus;
                }
            }
        }
    else
        {
        // unexpected arrayKind - should never happen.
        BeAssert (false);
        }

    return INSTANCE_WRITE_STATUS_Success;
    }
#pragma warning(default:4189)

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   05/10
+---------------+---------------+---------------+---------------+---------------+------*/
InstanceWriteStatus     WriteEmbeddedStructPropertyValue (StructECPropertyR structProperty, IECInstanceCR ecInstance, WString* baseAccessString, BeXmlNodeR propertyValueNode)
    {
    // the tag of the element for an embedded struct is the property name.
    BeXmlNodeP structNode = propertyValueNode.AddEmptyElement (structProperty.GetName().c_str());

    WString    thisAccessString;
    if (NULL != baseAccessString)
        AppendAccessString (thisAccessString, *baseAccessString, structProperty.GetName());
    else
        thisAccessString = structProperty.GetName().c_str();
    thisAccessString.append (L".");

    ECClassCR   structClass = structProperty.GetType();
    WritePropertyValuesOfClassOrStructArrayMember (structClass, ecInstance, &thisAccessString, *structNode);

    return INSTANCE_WRITE_STATUS_Success;
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
        return INSTANCE_READ_STATUS_XmlParseError;
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
    BeXmlDomPtr xmlDom = BeXmlDom::CreateAndReadFromString (xmlStatus, ecInstanceXml, wcslen (ecInstanceXml) * sizeof(WChar));

    if (!xmlDom.IsValid())
        {
        BeAssert (false);
        LogXmlLoadError ();
        return INSTANCE_READ_STATUS_XmlParseError;
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
    BeXmlDomPtr xmlDom = BeXmlDom::CreateAndReadFromString (xmlStatus, ecInstanceXml, strlen (ecInstanceXml) * sizeof(Utf8Char));
    if (!xmlDom.IsValid())
        {
        BeAssert (false);
        LogXmlLoadError ();
        return INSTANCE_READ_STATUS_XmlParseError;
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
        LOG.errorv (L"Invalid ECInstanceXML: Missing a top-level instance node");
        return INSTANCE_READ_STATUS_BadElement;
        }

    if (BEXMLNODE_Document == instanceNode->GetType())
        instanceNode = instanceNode->GetFirstChild (BEXMLNODE_Element);

    if (NULL == instanceNode)
        {
        BeAssert (false);
        LOG.errorv (L"Invalid ECInstanceXML: Missing a top-level instance node");
        return INSTANCE_READ_STATUS_BadElement;
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
    BeXmlDomPtr xmlDom = BeXmlDom::CreateEmpty();        

    InstanceXmlWriter   instanceWriter (*xmlDom.get(), NULL);

    InstanceWriteStatus status;
    if (INSTANCE_WRITE_STATUS_Success != (status = instanceWriter.WriteInstance (*this, writeInstanceId)))
        return status;

    return (BEXML_Success == xmlDom->ToFile (fileName, (BeXmlDom::ToStringOption)(BeXmlDom::TO_STRING_OPTION_Indent | BeXmlDom::TO_STRING_OPTION_Formatted), utf16 ? BeXmlDom::FILE_ENCODING_Utf16 : BeXmlDom::FILE_ENCODING_Utf8)) 
            ? INSTANCE_WRITE_STATUS_Success : INSTANCE_WRITE_STATUS_FailedToWriteFile;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   12/12
+---------------+---------------+---------------+---------------+---------------+------*/
template<typename T_STR> InstanceWriteStatus writeInstanceToXmlString (T_STR& ecInstanceXml, bool isStandAlone, bool writeInstanceId, IECInstanceR instance)
    {
    ecInstanceXml.clear();
    BeXmlDomPtr xmlDom = BeXmlDom::CreateEmpty();        
    InstanceXmlWriter   instanceWriter (*xmlDom.get(), NULL);
    InstanceWriteStatus status;
    if (INSTANCE_WRITE_STATUS_Success != (status = instanceWriter.WriteInstance (instance, writeInstanceId)))
        return status;

    UInt64  opts = BeXmlDom::TO_STRING_OPTION_OmitByteOrderMark;
    if ( ! isStandAlone)
        opts |= BeXmlDom::TO_STRING_OPTION_OmitXmlDeclaration;

    xmlDom->ToString (ecInstanceXml, (BeXmlDom::ToStringOption) opts);
    return INSTANCE_WRITE_STATUS_Success;
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
* @bsimethod                                    Barry.Bentley                   10/2011
+---------------+---------------+---------------+---------------+---------------+------*/
InstanceWriteStatus     IECInstance::WriteToBeXmlNode (BeXmlNodeR node)
    {
    InstanceXmlWriter instanceWriter (*node.GetDom(), &node);

    return instanceWriter.WriteInstance (*this, false);
    }


#if defined (NEEDSWORK_LIBXML) // WIP_NONPORT
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Carole.MacDonald                05/2010
+---------------+---------------+---------------+---------------+---------------+------*/
InstanceWriteStatus     IECInstance::WriteToXmlStream (IStreamP stream, bool isCompleteXmlDocument, bool writeInstanceId, bool utf16)
    {
    InstanceXmlWriter writer (stream);

    InstanceWriteStatus   status;
    if (INSTANCE_WRITE_STATUS_Success != (status = writer.Init (utf16)))
        return status;

    return writer.WriteInstance (*this, isStandAlone, writeInstanceId);
    }
#endif //defined (NEEDSWORK_XML)

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   11/12
+---------------+---------------+---------------+---------------+---------------+------*/
Bentley::DgnPlatform::DgnECInstance const* IECInstance::AsDgnECInstanceCP() const
    {
    return _GetAsDgnECInstance();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   11/12
+---------------+---------------+---------------+---------------+---------------+------*/
Bentley::DgnPlatform::DgnECInstance* IECInstance::AsDgnECInstanceP()
    {
    return const_cast<Bentley::DgnPlatform::DgnECInstance*> (_GetAsDgnECInstance());
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
* @bsimethod                                                    Paul.Connelly   02/13
+---------------+---------------+---------------+---------------+---------------+------*/
static bool initializeDefaultValues (IECInstanceR instance, ECClassCR ecClass, WCharCP accessStringPrefix)
    {
    ECValue defaultValue;
    IECInstancePtr custAttr;
    bool setAllPropertiesSuccessfully = true;
    FOR_EACH (ECPropertyCP ecprop, ecClass.GetProperties(true))
        {
        PrimitiveECPropertyCP primProp = ecprop->GetAsPrimitiveProperty();
        StructECPropertyCP structProp;

        if (NULL != primProp && (custAttr = primProp->GetCustomAttribute (L"UserDefinedPropertySpecification")).IsValid()
            && ECOBJECTS_STATUS_Success == custAttr->GetValue (defaultValue, L"DefaultValue") && defaultValue.ConvertToPrimitiveType (primProp->GetType())
           )
            {
            WString localAccessString;
            if (NULL != accessStringPrefix)
                {
                localAccessString.append (accessStringPrefix);
                localAccessString.append (1, '.');
                localAccessString.append (primProp->GetName());
                }

            if (ECOBJECTS_STATUS_Success != instance.SetValue (!localAccessString.empty() ? localAccessString.c_str() : primProp->GetName().c_str(), defaultValue))
                setAllPropertiesSuccessfully = false;
            }
        else if (NULL != (structProp = ecprop->GetAsStructProperty()))
            {
            WString localAccessString;
            if (NULL != accessStringPrefix)
                {
                localAccessString.append (accessStringPrefix);
                localAccessString.append (1, '.');
                localAccessString.append (structProp->GetName());
                }

            localAccessString.append (ecprop->GetName());
            if (!initializeDefaultValues (instance, structProp->GetType(), !localAccessString.empty() ? localAccessString.c_str() : structProp->GetName().c_str()))
                setAllPropertiesSuccessfully = false;
            }
        }

    return setAllPropertiesSuccessfully;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   01/13
+---------------+---------------+---------------+---------------+---------------+------*/
bool IECInstance::InitializeDefaultValues()
    {
    if (!GetClass().GetSchema().IsDefined (L"IsUserDefinedSchema"))
        return true;
    else
        return initializeDefaultValues (*this, GetClass(), NULL);
    }

END_BENTLEY_ECOBJECT_NAMESPACE
