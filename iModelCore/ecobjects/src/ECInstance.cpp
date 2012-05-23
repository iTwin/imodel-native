/*--------------------------------------------------------------------------------------+
|
|     $Source: src/ECInstance.cpp $
|
|   $Copyright: (c) 2012 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ECObjectsPch.h"
#include <ECObjects/ecschema.h>

BEGIN_BENTLEY_EC_NAMESPACE
    
static UInt32 g_totalAllocs = 0;
static UInt32 g_totalFrees  = 0;
static UInt32 g_currentLive = 0;

//#define DEBUG_INSTANCE_LEAKS
#ifdef DEBUG_INSTANCE_LEAKS
typedef std::map<IECInstance*, UInt32> DebugInstanceLeakMap;
DebugInstanceLeakMap    g_debugInstanceLeakMap;
#endif

//WIP_FUSION:  This should use EC::LeakDetector  (see ecschema.cpp)

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
bool IECInstance::IsFixedArrayProperty (EC::IECInstanceR instance, WCharCP accessString)
    {
    ECValue         arrayVal;

    if (ECOBJECTS_STATUS_Success != instance.GetValue (arrayVal, accessString))
        return false;

    ArrayInfo info = arrayVal.GetArrayInfo();
    return info.IsFixedCount();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    CaseyMullen     09/09
+---------------+---------------+---------------+---------------+---------------+------*/
IECInstance::IECInstance()
    {
    g_totalAllocs++;
    g_currentLive++;
#ifdef DEBUG_INSTANCE_LEAKS
    g_debugInstanceLeakMap[this] = g_totalAllocs; // record this so we know if it was the 1st, 2nd allocation
#endif

    //size_t sizeofInstance = sizeof(IECInstance);
    //size_t sizeofVoid = sizeof (void*);
    
    BeAssert (sizeof(IECInstance) == sizeof (RefCountedBase) && L"Increasing the size or memory layout of the base EC::IECInstance will adversely affect subclasses. Think of this as a pure interface... to which you would never be able to add (additional) data, either");
    };    

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    CaseyMullen     09/09
+---------------+---------------+---------------+---------------+---------------+------*/
IECInstance::~IECInstance()
    {
#ifdef DEBUG_INSTANCE_LEAKS
    g_debugInstanceLeakMap.erase(this);
#endif

    g_totalFrees++;
    g_currentLive--;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    01/10
+---------------+---------------+---------------+---------------+---------------+------*/
void IECInstance::Debug_DumpAllocationStats(WCharCP prefix)
    {
    if (!prefix)
        prefix = L"";

    ECObjectsLogger::Log()->debugv (L"%ls Live IECInstances: %d, Total Allocs: %d, TotalFrees: %d", prefix, g_currentLive, g_totalAllocs, g_totalFrees);
#ifdef DEBUG_INSTANCE_LEAKS
    FOR_EACH (DebugInstanceLeakMap::value_type leak, g_debugInstanceLeakMap)
        {
        //IECInstance* leakedInstance = leak.first;
        UInt32    orderOfAllocation = leak.second;
        ECObjectsLogger::Log()->debugv (L"Leaked the %dth IECInstance that was allocated.", orderOfAllocation);
        //leakedInstance->Dump();
        }
#endif
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
* @bsimethod                                                    CaseyMullen    02/10
+---------------+---------------+---------------+---------------+---------------+------*/
void IECInstance::Debug_ReportLeaks(bvector<WString>& classNamesToExclude)
    {
#ifdef DEBUG_INSTANCE_LEAKS
    FOR_EACH (DebugInstanceLeakMap::value_type leak, g_debugInstanceLeakMap)
        {
        IECInstance* leakedInstance = leak.first;
        UInt32    orderOfAllocation = leak.second;
        
        WString className = leakedInstance->GetClass().GetName();
        if (IsExcluded (className, classNamesToExclude))
            continue;
        WString schemaName = leakedInstance->GetClass().GetSchema().GetName();
        ECObjectsLogger::Log()->errorv (L"Leaked the %dth IECInstance that was allocated: ECSchema=%ls, ECClass=%ls, InstanceId=%ls", 
            orderOfAllocation, schemaName.c_str(), className.c_str(), leakedInstance->GetInstanceId().c_str());
        //leakedInstance->Dump();
        }
#endif
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    01/10
+---------------+---------------+---------------+---------------+---------------+------*/
void IECInstance::Debug_GetAllocationStats(int* currentLive, int* totalAllocs, int* totalFrees)
    {
    if (currentLive)
        *currentLive = g_currentLive;

    if (totalAllocs)
        *totalAllocs = g_totalAllocs;

    if (totalFrees)
        *totalFrees  = g_totalFrees;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    01/10
+---------------+---------------+---------------+---------------+---------------+------*/
void IECInstance::Debug_ResetAllocationStats()
    {
    g_totalAllocs = g_totalFrees = g_currentLive = 0;
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
* @bsimethod                                                    CaseyMullen     09/09
+---------------+---------------+---------------+---------------+---------------+------*/    
int IECInstance::ParseExpectedNIndices (WCharCP propertyAccessString)
    {
    WCharCP pointerToBrackets = wcsstr (propertyAccessString, L"[]");
    int nBrackets = 0;
    while (NULL != pointerToBrackets)
        {
        nBrackets++;
        pointerToBrackets += 2; // skip past the brackets
        pointerToBrackets = wcsstr (pointerToBrackets, L"[]"); ;
        }   
    
    return nBrackets;
    }
        
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Bill.Steinbock                  12/2010
+---------------+---------------+---------------+---------------+---------------+------*/
MemoryECInstanceBase* IECInstance::_GetAsMemoryECInstance () const
    {
    return NULL;    // default to NULL and let real MemoryECInstanceBased classes override this method.
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

    // For array properties, the convention has been to use them with
    // empty brackets at the end: PropertyName[]
    // As ECProperties, they do not have any brackets: PropertyName
    WString unBracketedAccessString = accessString;
    unBracketedAccessString.Trim (L"[]");

    ECPropertyP ecProperty = GetClass().GetPropertyP (unBracketedAccessString.c_str());
    if (ecProperty)
        return ecProperty->GetIsReadOnly();

    ECObjectsLogger::Log()->errorv (L"IECInstance: Attempted to check if property '%ls' is read only; could not find property in class '%ls'", unBracketedAccessString.c_str(), GetClass().GetName().c_str());
    return false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Dylan.Rush      08/2011
+---------------+---------------+---------------+---------------+---------------+------*/ 
bool                IECInstance::_IsPropertyReadOnly (UInt32 propertyIndex) const
    {
    if (_IsReadOnly())
        return true;

    WCharCP accessString;
    ECObjectsStatus status = GetEnabler().GetAccessString  (accessString, propertyIndex);
    if (ECOBJECTS_STATUS_Success != status)
        {
        ECObjectsLogger::Log()->errorv (L"IECInstance: Attempted to check if property with index '%d' is read only; could not resolve property name from enabler '%ls'", propertyIndex, GetEnabler().GetName());
        return false;
        }
    return IECInstance::_IsPropertyReadOnly (accessString);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    CaseyMullen     09/09
+---------------+---------------+---------------+---------------+---------------+------*/   
ECEnablerCR           IECInstance::GetEnabler() const { return _GetEnabler();  }
ECEnablerR            IECInstance::GetEnablerR() const { return *const_cast<ECEnablerP>(&_GetEnabler());  }
bool                  IECInstance::IsReadOnly() const { return _IsReadOnly();  }
MemoryECInstanceBase* IECInstance::GetAsMemoryECInstance () const {return _GetAsMemoryECInstance();}

ECObjectsStatus     IECInstance::GetValue (ECValueR v, WCharCP propertyAccessString) const { return _GetValue (v, propertyAccessString, false, 0); }
ECObjectsStatus     IECInstance::GetValue (ECValueR v, WCharCP propertyAccessString, UInt32 arrayIndex) const { return _GetValue (v, propertyAccessString, true, arrayIndex); }
ECObjectsStatus     IECInstance::GetValue (ECValueR v, UInt32 propertyIndex) const { return _GetValue (v, propertyIndex, false, 0); }
ECObjectsStatus     IECInstance::GetValue (ECValueR v, UInt32 propertyIndex, UInt32 arrayIndex) const { return _GetValue (v, propertyIndex, true, arrayIndex); }

ECObjectsStatus     IECInstance::SetInternalValue (WCharCP propertyAccessString, ECValueCR v) 
    {
    return _SetValue (propertyAccessString, v, false, 0); 
    }

ECObjectsStatus     IECInstance::SetValue (WCharCP propertyAccessString, ECValueCR v) 
    {
    if (IsReadOnly())
        return ECOBJECTS_STATUS_UnableToSetReadOnlyInstance;
    return _SetValue (propertyAccessString, v, false, 0); 
    }

ECObjectsStatus     IECInstance::SetInternalValue (WCharCP propertyAccessString, ECValueCR v, UInt32 arrayIndex) 
    {
    return _SetValue (propertyAccessString, v, true, arrayIndex); 
    }

ECObjectsStatus     IECInstance::SetValue (WCharCP propertyAccessString, ECValueCR v, UInt32 arrayIndex) 
    {
    if (IsReadOnly())
        return ECOBJECTS_STATUS_UnableToSetReadOnlyInstance;

    return _SetValue (propertyAccessString, v, true, arrayIndex); 
    }

ECObjectsStatus     IECInstance::SetInternalValue (UInt32 propertyIndex, ECValueCR v) 
    {
    return _SetValue (propertyIndex, v, false, 0); 
    }

ECObjectsStatus     IECInstance::SetValue (UInt32 propertyIndex, ECValueCR v) 
    {
    if (IsReadOnly())
        return ECOBJECTS_STATUS_UnableToSetReadOnlyInstance;

    return _SetValue (propertyIndex, v, false, 0); 
    }

ECObjectsStatus     IECInstance::SetInternalValue (UInt32 propertyIndex, ECValueCR v, UInt32 arrayIndex) 
    { 
    return _SetValue (propertyIndex, v, true, arrayIndex); 
    }

ECObjectsStatus     IECInstance::SetValue (UInt32 propertyIndex, ECValueCR v, UInt32 arrayIndex) 
    { 
    if (IsReadOnly())
        return ECOBJECTS_STATUS_UnableToSetReadOnlyInstance;

    return _SetValue (propertyIndex, v, true, arrayIndex); 
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
    //WIP_FUSION this leaves the [] appended at the end of arrays.
   
    WCharCP dotPos = wcschr (accessor, L'.');
    if (NULL != dotPos)
        {
        size_t dotIndex  = dotPos - accessor;
        buffer[dotIndex] = '\0';

        ECPropertyP prop = ecClass.GetPropertyP (buffer);

        if (NULL == prop)
            return NULL;

        StructECPropertyP structProperty = prop->GetAsStructProperty();

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

    WCharCP pos2 = wcschr (pos1+1, L']');

    BeAssert (pos2 != NULL);

    numChars = pos2 - pos1 - 1;

    wcsncpy(indexBuffer, pos1+1, numChars>NUM_INDEX_BUFFER_CHARS?NUM_INDEX_BUFFER_CHARS:numChars);
    indexBuffer[numChars]=0;

    UInt32 indexValue = -1;
    BeStringUtilities::Swscanf (indexBuffer, L"%ud", &indexValue);

    ECValue         arrayVal;
    ECObjectsStatus status;

    WString asBufferStr = asBuffer;
    asBufferStr.append (L"[]");

    if (ECOBJECTS_STATUS_Success != (status = instance.GetValue (arrayVal, asBufferStr.c_str())))
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
        return instance.GetValue (v, asBufferStr.c_str(), indexValue);

    // must be a struct array
    
    if (ECOBJECTS_STATUS_Success != (status = instance.GetValue (arrayVal, asBufferStr.c_str(), indexValue)))
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
    int arrayIndex = accessor[depth].arrayIndex;
    if (compatible)
        {
        UInt32 propertyIndex = (UInt32)accessor[depth].propertyIndex;
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
    int arrayIndex = accessor[depth].arrayIndex;
    if (compatible)
        {
        UInt32 propertyIndex = (UInt32)accessor[depth].propertyIndex;

#ifdef NOT_YET  // Casey decided that the ReadOnlyProperty setting was for GUI use only
        bool readonlyProperty = instance.IsPropertyReadOnly (propertyIndex);
#endif

        if(arrayIndex < 0)
            {
#ifdef NOT_YET  // Casey decided that the ReadOnlyProperty setting was for GUI use only
            if (readonlyProperty && !instance.IsNullValue(propertyIndex))
                return ECOBJECTS_STATUS_UnableToSetReadOnlyProperty;
#endif
            return instance.SetValue(propertyIndex, value);
            }
#ifdef NOT_YET  // Casey decided that the ReadOnlyProperty setting was for GUI use only
        if (readonlyProperty && !instance.IsNullValue (propertyIndex, (UInt32)arrayIndex))
            return ECOBJECTS_STATUS_UnableToSetReadOnlyProperty;
#endif
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
    int arrayIndex = accessor[depth].arrayIndex;
    if (compatible)
        {
        UInt32 propertyIndex = (UInt32)accessor[depth].propertyIndex;

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
        bool compatible = (accessor[depth].enabler == &currentInstance->GetEnabler()); // if same enabler then use property index to set value else use access string

        status = getValueHelper (v, *currentInstance, accessor, depth, compatible);
        if (ECOBJECTS_STATUS_Success != status)
            {
            // if we're accessing a property of an embedded struct, we expect GetValue() to return a null struct - so continue
            if (v.IsStruct () && v.IsNull () && ECValueAccessor::INDEX_ROOT == accessor[depth].arrayIndex)
                continue;
            else
                return status;
            }

        if (v.IsStruct() && accessor[depth].arrayIndex >= 0)
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
        bool compatible = (accessor[depth].enabler == &currentInstance->GetEnabler()); // if same enabler then use property index to set value else use access string
        int  propertyIndex   = accessor[depth].propertyIndex;
        int  arrayIndex      = accessor[depth].arrayIndex;

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
            EC::ECEnablerR          structEnabler = *(const_cast<EC::ECEnablerP>(&accessor.GetEnabler (depth + 1)));
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
ECObjectsStatus ECInstanceInteropHelper::GetString (IECInstanceCR instance, WCharCP & value, WCharCP managedPropertyAccessor)
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

    WString asBufferStr = asBuffer;
    asBufferStr.append (L"[]");

    if (ECOBJECTS_STATUS_Success != (status = instance.GetValue (arrayVal, asBufferStr.c_str())))
        return status;

    ArrayInfo arrayInfo = arrayVal.GetArrayInfo();
    UInt32    size      = arrayInfo.GetCount();

    if (indexValue >= size)
        {
        if (arrayInfo.IsFixedCount())
            return ECOBJECTS_STATUS_Error;

        UInt numToInsert = (indexValue + 1) - size;
        status =  instance.AddArrayElements (asBufferStr.c_str(), numToInsert);    
        if (ECOBJECTS_STATUS_Success != status)
            return status;

        if (arrayInfo.IsStructArray())
            {
            ECClassCR    ecClass     = instance.GetClass();

            //ECPropertyP  prop = ecClass.GetPropertyP (asBuffer);
            wchar_t buffer [NUM_INDEX_BUFFER_CHARS+1];
            wcsncpy(buffer, asBufferStr.c_str(), NUM_INDEX_BUFFER_CHARS);
            ECPropertyP prop = getProperty (ecClass, asBufferStr.c_str(), buffer);
            
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
                if (ECOBJECTS_STATUS_Success != instance.GetValue (arrayEntryVal, asBufferStr.c_str (), size+i) || arrayEntryVal.IsNull ())
                    {
                    arrayEntryVal.SetStruct (standaloneEnabler->CreateInstance().get());
                    if (SUCCESS != instance.SetValue (asBufferStr.c_str(), arrayEntryVal, size+i))
                        return ECOBJECTS_STATUS_Error;
                    }
                }
            }
        }

    if (arrayInfo.IsPrimitiveArray())
        return instance.SetValue (asBufferStr.c_str(), v, indexValue);

    // must be a struct array
    if (NULL == wcschr (pos2, L'.'))
        {
        //Caller is attempting to set the value of this struct array element directly.
        return instance.SetValue (asBufferStr.c_str(), v, indexValue);
        }
    instance.GetValue (arrayVal, asBufferStr.c_str(), indexValue);
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
ECObjectsStatus ECInstanceInteropHelper::GetString (IECInstanceCR instance, WCharCP & value, ECValueAccessorCR accessor)
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
    UInt32 propertyIndex = accessor.DeepestLocation().propertyIndex;
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
EC::ECEnablerP                  ECInstanceInteropHelper::GetEnablerForStructArrayEntry (IECInstanceR instance, ECValueAccessorR arrayMemberAccessor, SchemaKeyCR schemaKey, WCharCP className)
    {
    EC::ECValue v;
    instance.GetValueUsingAccessor (v, arrayMemberAccessor);

    if (!v.IsStruct())
        return NULL;

    if (!v.IsNull())
        {
        EC::IECInstancePtr structInstance = v.GetStruct();
        return &structInstance->GetEnablerR();
        }

    // if we get here we probably have a fixed size array with NULL entries
    EC::ECEnablerP structArrayEnabler = const_cast<EC::ECEnablerP>(arrayMemberAccessor.DeepestLocation().enabler);

    EC::StandaloneECEnablerPtr standaloneEnabler = structArrayEnabler->GetEnablerForStructArrayMember (schemaKey, className);
    if (standaloneEnabler.IsNull())
        {
        ECObjectsLogger::Log()->errorv (L"Unable to locate a standalone enabler for class %ls", className);
        return NULL;
        }

    return standaloneEnabler.get();
    }

/*---------------------------------------------------------------------------------**//**
* This method is called from derived class so the rootInstance was pinned if necessary
* before calling this method.
* @bsimethod                                    Bill.Steinbock                  06/2011
+---------------+---------------+---------------+---------------+---------------+------*/
/* static */ EC::IECInstancePtr getParentNativeInstance (EC::IECInstanceCP rootInstance, EC::ECValueAccessorCR structValueAccessor)
    {
    // if not a top level property, get the native instance that will contain this struct array
    if (structValueAccessor.GetDepth () > 1)
        {
        EC::ECValue parentStructValue;

        EC::ECValueAccessor parentInstanceAccessor (structValueAccessor);
        parentInstanceAccessor.PopLocation ();   // remove one level to get to the parent instance

        rootInstance->GetValueUsingAccessor (parentStructValue, parentInstanceAccessor);
        if (!parentStructValue.IsStruct ())
            return NULL;

        EC::IECInstancePtr structInstance = parentStructValue.GetStruct ();
        if (structInstance.IsValid())
            return structInstance;

        // we may be processing a member of an embedded struct so we need to check the next level up
        return getParentNativeInstance (rootInstance, parentInstanceAccessor);
        }

    return const_cast<EC::IECInstanceP>(rootInstance);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Bill.Steinbock                  06/2011
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus  ECInstanceInteropHelper::GetStructArrayEntry (EC::ECValueAccessorR structArrayEntryValueAccessor, IECInstanceR instance, UInt32 index, EC::ECValueAccessorCR structArrayValueAccessor, 
                                                             bool createPropertyIfNotFound, WCharCP wcharAccessString, 
                                                             SchemaKeyCR schemaKey, WCharCP className)
    {
    EC::ECEnablerR structArrayEnabler = *(const_cast<EC::ECEnablerP>(structArrayValueAccessor.DeepestLocationCR().enabler));
    EC::StandaloneECEnablerPtr standaloneEnabler = structArrayEnabler.GetEnablerForStructArrayMember (schemaKey, className);
    if (standaloneEnabler.IsNull())
        {
        ECObjectsLogger::Log()->errorv (L"Unable to locate a standalone enabler for class \" %ls \"", className);
        return ECOBJECTS_STATUS_EnablerNotFound;
        }

    EC::ECValue  arrayVal;
    instance.GetValueUsingAccessor (arrayVal, structArrayValueAccessor);

    ArrayInfo   arrayInfo  = arrayVal.GetArrayInfo();
    UInt32      arrayCount = arrayInfo.GetCount();

    // adjust the ECVAlueAccessor to include the array index
    EC::ECValueAccessor arrayEntryValueAccessor (structArrayValueAccessor);
    arrayEntryValueAccessor.DeepestLocation ().arrayIndex = index;

    if (arrayCount <= index)
        {
        // see if we are allowed to add a new strct array instance
        if (!createPropertyIfNotFound)
            return ECOBJECTS_STATUS_Error;

        // only proceed if not read only instance
        if (instance.IsReadOnly())
            return ECOBJECTS_STATUS_UnableToSetReadOnlyInstance;

        EC::IECInstancePtr parentNativeInstance = getParentNativeInstance (&instance, structArrayValueAccessor);
        if (parentNativeInstance.IsNull())
            {
            ECObjectsLogger::Log()->error (L"Unable to get native instance when processing ECInstanceInteropHelper::GetStructArrayEntry");
            return ECOBJECTS_STATUS_Error;
            }

        ::UInt32 numToInsert = (index + 1) - arrayCount;
        if (EC::ECOBJECTS_STATUS_Success != parentNativeInstance->AddArrayElements (wcharAccessString, numToInsert))
            {
            ECObjectsLogger::Log()->errorv(L"Unable to add array element(s) to native instance - access string \"%ls\"", structArrayValueAccessor.GetManagedAccessString().c_str());
            return ECOBJECTS_STATUS_UnableToAddStructArrayMember;
            }

        EC::ECValue  arrayEntryVal;
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
        EC::ECValue arrayEntryVal;

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

            EC::IECInstancePtr parentNativeInstance = getParentNativeInstance (&instance, structArrayValueAccessor);
            if (parentNativeInstance.IsNull())
                {
                ECObjectsLogger::Log()->error (L"Unable to get native instance when processing ECInstanceInteropHelper::GetStructArrayEntry");
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
bool            ECInstanceInteropHelper::GetNextInteropProperty 
(
int& propertyIndex, 
int& structNameLength, 
WCharCP& accessor,
IECInstanceCR instance, 
int prefix,
bool includeNulls,
bool firstRunInStruct
)
    {
    int maxPropertyIndex = instance.GetEnabler().GetPropertyCount();
    bool thisValueIsNull = false;
    do
        {
        if (propertyIndex >= maxPropertyIndex)
            return false;

        WCharCP currentAccessString;
        WCharCP nextAccessString;

        if (prefix > 0)
            {
            DEBUG_EXPECT (propertyIndex > -1);
            if (ECOBJECTS_STATUS_Success != instance.GetEnabler().GetAccessString (currentAccessString, propertyIndex))
                return false;
            }

        //If the caller is a struct (prefix > 0) continue to the next struct.  Else just get next access string and 
        //continue.
        if (firstRunInStruct)
            {
            accessor = currentAccessString;
            }
        else
            {
            do {
                if (++propertyIndex >= maxPropertyIndex)
                    return false;
                if (ECOBJECTS_STATUS_Success != instance.GetEnabler().GetAccessString (nextAccessString, propertyIndex))
                    return false;
                } while (0 != prefix && 0 != wcsncmp (currentAccessString, nextAccessString, (size_t) prefix));
            accessor = nextAccessString;
            }

        WCharCP dotPos = wcschr (accessor + prefix + 1, L'.');
        if (NULL == dotPos)
            structNameLength = -1;
        else
            structNameLength = (int) (dotPos - accessor);

        if ( ! includeNulls)
            {
            if (-1 == structNameLength)
                {
                ECValue v;
                instance.GetValue (v, propertyIndex);
                thisValueIsNull = v.IsNull();
                }
            else 
                thisValueIsNull = false;
            }
        } while ( ! includeNulls && thisValueIsNull);
    return true;
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
int   ECInstanceInteropHelper::FirstIndexOfStruct  (IECInstanceCR instance, WCharCP structName)
    {
    int maxPropertyIndex = instance.GetEnabler().GetPropertyCount();
    size_t structNameLength = wcslen (structName);
    int propertyIndex = -1;

    WCharCP currentAccessString;
    do {
        propertyIndex ++;
        if (propertyIndex == maxPropertyIndex)
            return -1;
        if (ECOBJECTS_STATUS_Success != instance.GetEnabler().GetAccessString (currentAccessString, propertyIndex))
            return -1;
        } while (0 != wcsncmp (currentAccessString, structName, structNameLength));
    return propertyIndex;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Dylan.Rush                      1/11
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus ECInstanceInteropHelper::SetValueByIndex (IECInstanceR instance, int propertyIndex, int arrayIndex, ECValueCR value)
    {
    if (-1 == arrayIndex)
        return instance.SetValue ((UInt32) propertyIndex, value);

    EC::ECValue v;
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
        if (EC::ECOBJECTS_STATUS_Success != status)
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

    EC::ECValue v;
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
    return _InsertArrayElements (propertyAccessString, index, size);
    } 
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Adam.Klatzkin                   01/2010
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus                 IECInstance::AddArrayElements (WCharCP propertyAccessString, UInt32 size)
    {
    return _AddArrayElements (propertyAccessString, size);
    }        

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Adam.Klatzkin                   01/2010
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus                 IECInstance::RemoveArrayElement (WCharCP propertyAccessString, UInt32 index)
    {
    return _RemoveArrayElement (propertyAccessString, index);
    } 
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Adam.Klatzkin                   01/2010
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus                 IECInstance::ClearArray (WCharCP propertyAccessString)
    {
    return _ClearArray (propertyAccessString);
    }           
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    CaseyMullen     09/09
+---------------+---------------+---------------+---------------+---------------+------*/
WString                         IECInstance::ToString (WCharCP indent) const
    {
    return _ToString (indent);
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Bill.Steinbock                  05/2011
+---------------+---------------+---------------+---------------+---------------+------*/
WCharCP                         IECInstance::GetInstanceLabelPropertyName () const
    {
    ECClassCR ecClass = GetClass(); 

        // see if the struct has a custom attribute to custom serialize itself
    IECInstancePtr caInstance = ecClass.GetCustomAttribute(L"InstanceLabelSpecification");
    if (caInstance.IsValid())
        {
        ECValue value;
        if (SUCCESS == caInstance->GetValue (value, L"PropertyName") && !value.IsNull())
            {
            return value.GetString();
            }
        }

    if (NULL != ecClass.GetPropertyP (L"DisplayLabel"))
        return L"DisplayLabel";

    if (NULL != ecClass.GetPropertyP (L"DISPLAYLABEL"))
        return L"DISPLAYLABEL";

     if (NULL != ecClass.GetPropertyP (L"displaylabel"))
        return L"displaylabel";
   
    if (NULL != ecClass.GetPropertyP (L"Name"))
        return L"Name";

    if (NULL != ecClass.GetPropertyP (L"NAME"))
        return L"NAME";

    if (NULL != ecClass.GetPropertyP (L"name"))
        return L"name";

    return NULL;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Bill.Steinbock                  05/2011
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus                 IECInstance::_GetDisplayLabel (WString& displayLabel) const
    {
    WCharCP propertyName = GetInstanceLabelPropertyName ();
    if (NULL == propertyName)
        return ECOBJECTS_STATUS_Error;

    EC::ECValue ecValue;
    if (SUCCESS == GetValue (ecValue, propertyName) && !ecValue.IsNull())
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
    WCharCP propertyName = GetInstanceLabelPropertyName ();
    if (NULL == propertyName)
        return ECOBJECTS_STATUS_Error;

    EC::ECValue ecValue;
    ecValue.SetString (displayLabel);

    if (SUCCESS == SetValue (propertyName, ecValue))
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
static WString              ConvertByteArrayToString (const byte *byteData, size_t numBytes)
    {
    static const wchar_t    base64Chars[] = {L"ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/"};

    if (0 == numBytes)
        return L"";

    // from each 3 bytes we get 4 output characters, rounded up.
    WString    outString;
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

    return outString;
    }

typedef std::vector<byte>   T_ByteArray;
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   04/10
+---------------+---------------+---------------+---------------+---------------+------*/
static InstanceReadStatus   ConvertStringToByteArray (T_ByteArray& byteData, WCharCP stringData)
    {
    // the length of stringData should be a muttiple of four.
    size_t  stringLen = wcslen (stringData);
    if (0 != (stringLen % 4))
        return INSTANCE_READ_STATUS_BadBinaryData;

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
                    return INSTANCE_READ_STATUS_BadBinaryData;
                numBytesToPush = jPos-1;
                break;
                }
            else
                {
                return INSTANCE_READ_STATUS_BadBinaryData;
                }
            }

        byte*   bytes = (byte*)&nextThreeBytes;
        byteData.push_back (*bytes);
        if (numBytesToPush > 1)
            byteData.push_back (*(bytes+1));
        if (numBytesToPush > 2)
            byteData.push_back (*(bytes+2));
        }

    return INSTANCE_READ_STATUS_Success;
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
    ECObjectsLogger::Log()->errorv (errorString.c_str());

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

    // get the xmlns name, if there is one.
    Utf8CP  schemaName;
    if (NULL != (schemaName = m_xmlNode.GetNamespace()))
        m_fullSchemaName.AssignUtf8 (schemaName);

    ECSchemaCP  schema = GetSchema();
    if (NULL == schema)
        {
        ECObjectsLogger::Log()->errorv (L"Failed to locate ECSchema %ls", m_fullSchemaName.c_str());
        return INSTANCE_READ_STATUS_ECSchemaNotFound;
        }

    // see if we can find the class from the schema.
    ECClassCP    foundClass;
    if (NULL == (foundClass = schema->GetClassP (className.c_str())))
        {
        ECSchemaReferenceListCR refList = schema->GetReferencedSchemas();
        SchemaKey key;
        if (ECOBJECTS_STATUS_Success == SchemaKey::ParseSchemaFullName (key, m_fullSchemaName.c_str()))
            {
            ECSchemaReferenceList::const_iterator schemaIterator = refList.find (key);
            if (schemaIterator != refList.end())
                foundClass = schemaIterator->second->GetClassP (className.c_str());
            }
        else
            {
            for (ECSchemaReferenceList::const_iterator schemaIterator = refList.begin(); schemaIterator != refList.end(); schemaIterator++)
                {
                if (NULL != (foundClass = schemaIterator->second->GetClassP (className.c_str())))
                    break;
                }
            }
        }
    if (NULL == foundClass)
        {
        ECObjectsLogger::Log()->errorv (L"Failed to find ECClass %ls in %ls", className.c_str(), m_fullSchemaName.c_str());
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
        ECObjectsLogger::Log()->warningv (L"No ECProperty '%ls' found in ECClass '%ls'. Value will be ignored.", propertyName.c_str(), ecClass.GetName().c_str());
        return INSTANCE_READ_STATUS_PropertyNotFound;
        }

    PrimitiveECPropertyP    primitiveProperty;
    ArrayECPropertyP        arrayProperty;
    StructECPropertyP       structProperty;
    if (NULL != (primitiveProperty = ecProperty->GetAsPrimitiveProperty()))
        return ReadPrimitivePropertyValue (primitiveProperty, ecInstance, baseAccessString, propertyValueNode);
                //Above is good, if SkipToElementEnd() is returned from ReadPrimitiveValue.
    else if (NULL != (arrayProperty = ecProperty->GetAsArrayProperty()))
        return ReadArrayPropertyValue (arrayProperty, ecInstance, baseAccessString, propertyValueNode);
    else if (NULL != (structProperty = ecProperty->GetAsStructProperty()))
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
    if (INSTANCE_READ_STATUS_Success != (ixrStatus = ReadPrimitiveValue (ecValue, propertyType, propertyValueNode)))
        return ixrStatus;

    if(ecValue.IsUninitialized())
        {
        //A malformed value was found.  A warning was shown; just move on.
        return INSTANCE_READ_STATUS_Success;
        }

    ECObjectsStatus setStatus;
    if (NULL == baseAccessString)
        {
        setStatus = ecInstance->SetValue (primitiveProperty->GetName().c_str(), ecValue);

        if (ECOBJECTS_STATUS_Success != setStatus)
            ECObjectsLogger::Log()->warningv(L"Unable to set value for property %ls", primitiveProperty->GetName().c_str());
        }
    else
        {
        WString compoundAccessString;
        AppendAccessString (compoundAccessString, *baseAccessString, primitiveProperty->GetName());
        setStatus = ecInstance->SetValue (compoundAccessString.c_str(), ecValue);

        if (ECOBJECTS_STATUS_Success != setStatus)
            ECObjectsLogger::Log()->warningv(L"Unable to set value for property %ls", compoundAccessString.c_str());
        }

    BeAssert (ECOBJECTS_STATUS_Success == setStatus);

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

    accessString.append (L"[]");

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
            if (!ValidateArrayPrimitiveType (arrayValueNode->GetName(), memberType))
                {
                ECObjectsLogger::Log()->warningv(L"Incorrectly formatted array element found in array %ls.  Expected: %hs  Found: %hs", accessString.c_str(), GetPrimitiveTypeString (memberType), arrayValueNode->GetName());
                continue;
                }

            // read it, populating the ECInstance using accessString and arrayIndex.
            InstanceReadStatus      ixrStatus;
            ECValue                 ecValue;
            if (INSTANCE_READ_STATUS_Success != (ixrStatus = ReadPrimitiveValue (ecValue, memberType, *arrayValueNode)))
                continue;

            if ( !isFixedSizeArray)
                ecInstance->AddArrayElements (accessString.c_str(), 1);

            ECObjectsStatus   setStatus = ecInstance->SetValue (accessString.c_str(), ecValue, index);
            if (ECOBJECTS_STATUS_Success != setStatus && ECOBJECTS_STATUS_PropertyValueMatchesNoChange != setStatus)   
                {
                BeAssert (false);
                return INSTANCE_READ_STATUS_CantSetValue;
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
                return INSTANCE_READ_STATUS_BadArrayElement;


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

    ECObjectsStatus setStatus = owningInstance->SetValue (accessString.c_str(), structValue, index);
    if (ECOBJECTS_STATUS_Success != setStatus)
        BeAssert (ECOBJECTS_STATUS_Success == setStatus);

    return INSTANCE_READ_STATUS_Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   10/2011
+---------------+---------------+---------------+---------------+---------------+------*/
InstanceReadStatus   ReadPrimitiveValue (ECValueR ecValue, PrimitiveType propertyType, BeXmlNodeR primitiveValueNode)
    {
    // On entry primitiveValueNode is the XML node that holds the value. 
    // First check to see if the value is set to NULL
    WString         nullValue;
    if (BEXML_Success == primitiveValueNode.GetAttributeStringValue (nullValue, XSI_NIL_ATTRIBUTE))
        {
        if ( (0 == BeStringUtilities::Wcsicmp (nullValue.c_str(), L"true")) || (0 == wcscmp (nullValue.c_str(), L"1")) )
            {
            ecValue.SetToNull();
            return INSTANCE_READ_STATUS_Success;
            }
        }

    // try to read the actual value.
    WString     propertyValueString;
    if (BEXML_Success != primitiveValueNode.GetContent (propertyValueString))
        return INSTANCE_READ_STATUS_Success;

    // an empty string should not be parsed.
    if (0 == propertyValueString.length())
        {
        // set to an empty string. This matches what we did in the managed ECObjects.
        if (PRIMITIVETYPE_String == propertyType)
            ecValue.SetString (L"");

        return INSTANCE_READ_STATUS_Success;
        }

    WCharCP     propertyValueWChar = propertyValueString.c_str();
    switch (propertyType)
        {
        case PRIMITIVETYPE_Binary:
            {
            T_ByteArray                     byteArray;

            if (INSTANCE_READ_STATUS_Success != ConvertStringToByteArray (byteArray, propertyValueWChar))
                {
                ECObjectsLogger::Log()->warningv(L"Type mismatch in deserialization: \"%ls\" is not Binary", propertyValueWChar);
                return INSTANCE_READ_STATUS_TypeMismatch;
                }
            ecValue.SetBinary (&byteArray.front(), byteArray.size(), true);
            break;
            }

        case PRIMITIVETYPE_Boolean:
            {
            bool boolValue = ( (0 == BeStringUtilities::Wcsicmp (propertyValueWChar, L"true"))|| (0 == wcscmp (propertyValueWChar, L"1")) );
            ecValue.SetBoolean (boolValue);
            break;
            }

        case PRIMITIVETYPE_DateTime:
            {
            Int64   ticks;
            if (1 != BeStringUtilities::Swscanf (propertyValueWChar, L"%I64d", &ticks))
                {
                ECObjectsLogger::Log()->warningv(L"Type mismatch in deserialization: \"%ls\" is not DateTime", propertyValueWChar);
                return INSTANCE_READ_STATUS_TypeMismatch;
                }

            ecValue.SetDateTimeTicks (ticks);
            break;
            }

        case PRIMITIVETYPE_Double:
            {
            double  doubleValue;
            if (1 != BeStringUtilities::Swscanf (propertyValueWChar, L"%lg", &doubleValue))
                {
                ECObjectsLogger::Log()->warningv(L"Type mismatch in deserialization: \"%ls\" is not Double", propertyValueWChar);
                return INSTANCE_READ_STATUS_TypeMismatch;
                }
            ecValue.SetDouble (doubleValue);
            break;
            }

        case PRIMITIVETYPE_Integer:
            {
            Int32   intValue;
            if (1 != BeStringUtilities::Swscanf (propertyValueWChar, L"%d", &intValue))
                {
                ECObjectsLogger::Log()->warningv(L"Type mismatch in deserialization: \"%ls\" is not Integer", propertyValueWChar);
                return INSTANCE_READ_STATUS_TypeMismatch;
                }
            ecValue.SetInteger (intValue);
            break;
            }

        case PRIMITIVETYPE_Long:
            {
            Int64   longValue;
            if (1 != BeStringUtilities::Swscanf (propertyValueWChar, L"%I64d", &longValue))
                {
                ECObjectsLogger::Log()->warningv(L"Type mismatch in deserialization: \"%ls\" is not Long", propertyValueWChar);
                return INSTANCE_READ_STATUS_TypeMismatch;
                }
            ecValue.SetLong (longValue);
            break;
            }

        case PRIMITIVETYPE_Point2D:
            {
            DPoint2d point2d;
            if (2 != BeStringUtilities::Swscanf (propertyValueWChar, L"%lg,%lg", &point2d.x, &point2d.y))
                {
                ECObjectsLogger::Log()->warningv(L"Type mismatch in deserialization: \"%ls\" is not Point2D", propertyValueWChar);
                return INSTANCE_READ_STATUS_TypeMismatch;
                }
            ecValue.SetPoint2D (point2d);
            break;
            }

        case PRIMITIVETYPE_Point3D:
            {
            DPoint3d point3d;
            if (3 != BeStringUtilities::Swscanf (propertyValueWChar, L"%lg,%lg,%lg", &point3d.x, &point3d.y, &point3d.z))
                {
                ECObjectsLogger::Log()->warningv(L"Type mismatch in deserialization: \"%ls\" is not Point3D", propertyValueWChar);
                return INSTANCE_READ_STATUS_TypeMismatch;
                }
            ecValue.SetPoint3D (point3d);
            break;
            }

        case PRIMITIVETYPE_String:
            {
            ecValue.SetString (propertyValueWChar);
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
    if (NULL == (classFound = schema->GetClassP (typeFound)))
        {
        BeAssert (false);
        return NULL;
        }
    if (!classFound->Is (expectedType))
        {
        BeAssert (false);
        return NULL;
        }

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
            
        if (NULL != (primitiveProperty = ecProperty->GetAsPrimitiveProperty()))
            ixwStatus = WritePrimitivePropertyValue (*primitiveProperty, ecInstance, baseAccessString, rootNode);
        else if (NULL != (arrayProperty = ecProperty->GetAsArrayProperty()))
            ixwStatus = WriteArrayPropertyValue (*arrayProperty, ecInstance, baseAccessString, rootNode);
        else if (NULL != (structProperty = ecProperty->GetAsStructProperty()))
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
    wchar_t     outString[512];

    // write the content according to type.
    switch (propertyType)
        {
        case PRIMITIVETYPE_Binary:
            {
            size_t      numBytes;
            const byte* byteData; 
            if (NULL != (byteData = ecValue.GetBinary (numBytes)))
                {
                WString    byteString = ConvertByteArrayToString (byteData, numBytes);
                propertyValueNode.SetContentFast (byteString.c_str());
                }
            return INSTANCE_WRITE_STATUS_Success;
            break;
            }

        case PRIMITIVETYPE_Boolean:
            {
            wcscpy (outString, ecValue.GetBoolean () ? L"True" : L"False");
            break;
            }

        case PRIMITIVETYPE_DateTime:
            {
            BeStringUtilities::Snwprintf (outString, L"%I64d", ecValue.GetDateTimeTicks());
            break;
            }

        case PRIMITIVETYPE_Double:
            {
            BeStringUtilities::Snwprintf (outString, L"%.13g", ecValue.GetDouble());
            break;
            }

        case PRIMITIVETYPE_Integer:
            {
            BeStringUtilities::Snwprintf (outString, L"%d", ecValue.GetInteger());
            break;
            }

        case PRIMITIVETYPE_Long:
            {
            BeStringUtilities::Snwprintf (outString, L"%I64d", ecValue.GetLong());
            break;
            }

        case PRIMITIVETYPE_Point2D:
            {
            DPoint2d    point2d = ecValue.GetPoint2D();
            BeStringUtilities::Snwprintf (outString, L"%.13g,%.13g", point2d.x, point2d.y);
            break;
            }

        case PRIMITIVETYPE_Point3D:
            {
            DPoint3d    point3d = ecValue.GetPoint3D();
            BeStringUtilities::Snwprintf (outString, L"%.13g,%.13g,%.13g", point3d.x, point3d.y, point3d.z);
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
InstanceWriteStatus     WriteArrayPropertyValue (ArrayECPropertyR arrayProperty, IECInstanceCR ecInstance, WString* baseAccessString, BeXmlNodeR propertyValueNode)
    {
    ArrayKind       arrayKind = arrayProperty.GetKind();

    WString    accessString;
    if (NULL == baseAccessString)
        accessString = arrayProperty.GetName();    
    else
        AppendAccessString (accessString, *baseAccessString, arrayProperty.GetName());

    accessString.append (L"[]");

    // no members, don't write anything.
    ECValue         ecValue;
    if (SUCCESS != ecInstance.GetValue (ecValue, accessString.c_str(), 0) || ecValue.IsNull())
        return INSTANCE_WRITE_STATUS_Success;

    BeXmlNodeP  arrayNode = propertyValueNode.AddEmptyElement (arrayProperty.GetName().c_str());

    InstanceWriteStatus     ixwStatus;
    if (ARRAYKIND_Primitive == arrayKind)
        {
        PrimitiveType   memberType  = arrayProperty.GetPrimitiveElementType();
        Utf8CP          typeString  = GetPrimitiveTypeString (memberType);
        for (int index=0; ; index++)
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
        for (int index=0; ; index++)
            {
            if (SUCCESS != ecInstance.GetValue (ecValue, accessString.c_str(), index))
                break;

            // the XML element tag is the struct type.
            BeAssert (ecValue.IsStruct());

            IECInstancePtr  structInstance = ecValue.GetStruct();
            if (!structInstance.IsValid())
                {
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
        ECObjectsLogger::Log()->errorv (L"Invalid ECInstanceXML: Missing a top-level instance node");
        return INSTANCE_READ_STATUS_BadElement;
        }

    if (BEXMLNODE_Document == instanceNode->GetType())
        instanceNode = instanceNode->GetFirstChild (BEXMLNODE_Element);

    if (NULL == instanceNode)
        {
        BeAssert (false);
        ECObjectsLogger::Log()->errorv (L"Invalid ECInstanceXML: Missing a top-level instance node");
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
InstanceWriteStatus     IECInstance::WriteToXmlFile (WCharCP fileName, bool isStandAlone, bool writeInstanceId, bool utf16)
    {
    BeXmlDomPtr xmlDom = BeXmlDom::CreateEmpty();        

    InstanceXmlWriter   instanceWriter (*xmlDom.get(), NULL);

    InstanceWriteStatus status;
    if (INSTANCE_WRITE_STATUS_Success != (status = instanceWriter.WriteInstance (*this, writeInstanceId)))
        return status;

    return (BEXML_Success == xmlDom->ToFile (fileName, BeXmlDom::TO_STRING_OPTION_Indent, utf16 ? BeXmlDom::FILE_ENCODING_Utf16 : BeXmlDom::FILE_ENCODING_Utf8)) 
            ? INSTANCE_WRITE_STATUS_Success : INSTANCE_WRITE_STATUS_FailedToWriteFile;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Carole.MacDonald                06/2010
+---------------+---------------+---------------+---------------+---------------+------*/
InstanceWriteStatus     IECInstance::WriteToXmlString (WString & ecInstanceXml, bool isStandAlone, bool writeInstanceId)
    {
    ecInstanceXml.clear();

    BeXmlDomPtr xmlDom = BeXmlDom::CreateEmpty();        

    InstanceXmlWriter   instanceWriter (*xmlDom.get(), NULL);

    InstanceWriteStatus status;
    if (INSTANCE_WRITE_STATUS_Success != (status = instanceWriter.WriteInstance (*this, writeInstanceId)))
        return status;

    UInt64  opts = BeXmlDom::TO_STRING_OPTION_OmitByteOrderMark;

    if ( ! isStandAlone)
        opts |= BeXmlDom::TO_STRING_OPTION_OmitXmlDeclaration;

    xmlDom->ToString (ecInstanceXml, (BeXmlDom::ToStringOption) opts);

    return INSTANCE_WRITE_STATUS_Success;
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
* @bsimethod                                    Abeesh.Basheer                  03/2012
+---------------+---------------+---------------+---------------+---------------+------*/
ECSchemaCP ECInstanceReadContext::FindSchemaCP(SchemaKeyCR key, SchemaMatchType matchType) const
    {
    ECSchemaCP schema = _FindSchemaCP(key, matchType);
    if (NULL != schema)
        return schema;

    return &m_fallBackSchema;
    }
END_BENTLEY_EC_NAMESPACE
