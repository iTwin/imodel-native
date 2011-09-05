/*--------------------------------------------------------------------------------------+
|
|     $Source: src/ECInstance.cpp $
|
|   $Copyright: (c) 2011 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#include "ECObjectsPch.h"

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
    
    assert (sizeof(IECInstance) == sizeof (RefCountedBase) && L"Increasing the size or memory layout of the base EC::IECInstance will adversely affect subclasses. Think of this as a pure interface... to which you would never be able to add (additional) data, either");
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

    ECObjectsLogger::Log()->debugv (L"%s Live IECInstances: %d, Total Allocs: %d, TotalFrees: %d", prefix, g_currentLive, g_totalAllocs, g_totalFrees);
#ifdef DEBUG_INSTANCE_LEAKS
    FOR_EACH (DebugInstanceLeakMap::value_type leak, g_debugInstanceLeakMap)
        {
        IECInstance* leakedInstance = leak.first;
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
        
        ECObjectsLogger::Log()->errorv (L"Leaked the %dth IECInstance that was allocated: ECClass=%s, InstanceId=%s", 
            orderOfAllocation, className.c_str(), leakedInstance->GetInstanceId().c_str());
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
    WCharCP pointerToBrackets = pointerToBrackets = wcsstr (propertyAccessString, L"[]"); ;
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
ECObjectsStatus     IECInstance::SetValue (WCharCP propertyAccessString, ECValueCR v) { return _SetValue (propertyAccessString, v, false, 0); }
ECObjectsStatus     IECInstance::SetValue (WCharCP propertyAccessString, ECValueCR v, UInt32 arrayIndex) { return _SetValue (propertyAccessString, v, true, arrayIndex); }
ECObjectsStatus     IECInstance::SetValue (UInt32 propertyIndex, ECValueCR v) { return _SetValue (propertyIndex, v, false, 0); }
ECObjectsStatus     IECInstance::SetValue (UInt32 propertyIndex, ECValueCR v, UInt32 arrayIndex) { return _SetValue (propertyIndex, v, true, arrayIndex); }

#define NUM_INDEX_BUFFER_CHARS 63
#define NUM_ACCESSSTRING_BUFFER_CHARS 1023

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

    assert (pos2 != NULL);

    numChars = pos2 - pos1 - 1;

    wcsncpy(indexBuffer, pos1+1, numChars>NUM_INDEX_BUFFER_CHARS?NUM_INDEX_BUFFER_CHARS:numChars);
    indexBuffer[numChars]=0;

    UInt32 indexValue = -1;
    swscanf (indexBuffer, L"%ud", &indexValue);

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
        if(arrayIndex < 0)
            return instance.SetValue(propertyIndex, value);

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
ECObjectsStatus           IECInstance::GetValueUsingAccessor (ECValueR v, ECValueAccessorCR accessor) const
    {
    ECObjectsStatus status            = ECOBJECTS_STATUS_Success;
    IECInstancePtr  currentInstance   = const_cast <IECInstance*> (this);

    for (UInt32 depth = 0; depth < accessor.GetDepth(); depth ++)
        {
        bool compatible = (accessor[depth].enabler == &currentInstance->GetEnabler()); // if same enabler then use property index to set value else use access string

        status = getValueHelper (v, *currentInstance, accessor, depth, compatible);
        if (ECOBJECTS_STATUS_Success != status)
            return status;

        if (v.IsStruct() && accessor[depth].arrayIndex >= 0)
            currentInstance = v.GetStruct();
        }

    return status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Dylan.Rush      11/10
+---------------+---------------+---------------+---------------+---------------+------*/ 
ECObjectsStatus           IECInstance::SetValueUsingAccessor (ECValueAccessorCR accessor, ECValueCR valueToSet)
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

            //Expand array if necessary.
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
            return setValueHelper (*currentInstance, accessor, depth, compatible, valueToSet);

        // if we are not inside an array this is an embedded struct, go into it.
        if (0 > arrayIndex)
            continue;

        ECValue         structPlaceholder;

        // if we get here we are processing an array of structs.  Get the struct's ECInstance so we can use for the next location depth
        status = getValueHelper (structPlaceholder, *currentInstance, accessor, depth, compatible);
        if (ECOBJECTS_STATUS_Success != status)
            return status;

        assert (structPlaceholder.IsStruct() && "Accessor depth is greater than expected.");

        IECInstancePtr newInstance = structPlaceholder.GetStruct();

        // If the struct does not have an instance associated with it then build the instance
        if (newInstance.IsNull())
            {
            EC::ECEnablerR          structEnabler = *(const_cast<EC::ECEnablerP>(&accessor.GetEnabler (depth + 1)));
            ECClassCR               structClass   = accessor.GetEnabler (depth + 1).GetClass();
            StandaloneECEnablerPtr  standaloneEnabler = structEnabler.GetEnablerForStructArrayMember (structClass.GetSchema().GetName().c_str(), structClass.GetName().c_str());

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
static ECClassP GetClassFromReferencedSchemas (ECSchemaCR rootSchema, WCharCP schemaName, WCharCP className)
    {
    FOR_EACH (ECSchemaCP refSchema, rootSchema.GetReferencedSchemas())
        {
        if (!refSchema->GetName().EqualsI (schemaName))
            {
            // look in reference schemas
            ECClassP foundClassP = GetClassFromReferencedSchemas (*refSchema, schemaName, className);
            if (foundClassP)
                return  foundClassP;

            continue;
            }

        ECClassP classP = refSchema->GetClassP (className);
        if (classP)
            return classP;
        }

    return NULL;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Bill.Steinbock                  10/2010
+---------------+---------------+---------------+---------------+---------------+------*/
static ECObjectsStatus setECValueUsingFullAccessString (wchar_t* asBuffer, wchar_t* indexBuffer, ECValueCR v, IECInstanceR instance, WCharCP managedPropertyAccessor)
    {
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

    assert (pos2 != NULL);

    numChars = pos2 - pos1 - 1;

    wcsncpy(indexBuffer, pos1+1, numChars>NUM_INDEX_BUFFER_CHARS?NUM_INDEX_BUFFER_CHARS:numChars);
    indexBuffer[numChars]=0;

    UInt32 indexValue = 0;
    if (1 != swscanf (indexBuffer, L"%ud", &indexValue))
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

            ECPropertyP  prop = ecClass.GetPropertyP (asBuffer);
            if (!prop->GetIsArray())
                return ECOBJECTS_STATUS_Error;

            ArrayECPropertyP arrayProp = dynamic_cast<ArrayECPropertyP>(prop);
            if (!arrayProp)
                return ECOBJECTS_STATUS_Error;

            ECClassCP structClass = arrayProp->GetStructElementType();

            StandaloneECEnablerPtr standaloneEnabler = instance.GetEnablerR().GetEnablerForStructArrayMember (structClass->GetSchema().GetName().c_str(), structClass->GetName().c_str());
            if (standaloneEnabler.IsNull())
                return ECOBJECTS_STATUS_Error;

            ECValue                     arrayEntryVal;

            for (UInt32 i=0; i<numToInsert; i++)
                {
                arrayEntryVal.SetStruct (standaloneEnabler->CreateInstance().get());
                if (SUCCESS != instance.SetValue (asBufferStr.c_str(), arrayEntryVal, size+i))
                    return ECOBJECTS_STATUS_Error;
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

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Dylan.Rush                      1/11
+---------------+---------------+---------------+---------------+---------------+------*/
static ECTypeDescriptor getTypeDescriptor  (IECInstanceCR instance, int propertyIndex)
    {
    //GetTypeDescriptor (int) would be another candidate to add to the ECEnabler API
    ECObjectsStatus status;
    ECEnablerCP enabler = & instance.GetEnabler();
    ClassLayoutHolderCP layoutHolder = dynamic_cast<ClassLayoutHolderCP> (enabler);
    if (NULL == layoutHolder)
        {
        ECTypeDescriptor d;
        return d;
        }
    PropertyLayoutCP propLayout;
    status = layoutHolder->GetClassLayout().GetPropertyLayoutByIndex (propLayout, propertyIndex);
    if (ECOBJECTS_STATUS_Success != status)
        {
        ECTypeDescriptor d;
        return d;
        }
    return propLayout->GetTypeDescriptor();
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

#ifdef NOT_USED
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Dylan.Rush                      1/11
+---------------+---------------+---------------+---------------+---------------+------*/
ValueKind  ECInstanceInteropHelper::GetValueKind  (IECInstanceCR instance, int propertyIndex)
    {
    return getTypeDescriptor (instance, propertyIndex).GetTypeKind();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Dylan.Rush                      1/11
+---------------+---------------+---------------+---------------+---------------+------*/
ArrayKind  ECInstanceInteropHelper::GetArrayKind  (IECInstanceCR instance, int propertyIndex)
    {
    return getTypeDescriptor (instance, propertyIndex).GetArrayKind();
    }

#endif

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Dylan.Rush                      1/11
+---------------+---------------+---------------+---------------+---------------+------*/
PrimitiveType  ECInstanceInteropHelper::GetPrimitiveType  (IECInstanceCR instance, int propertyIndex)
    {
    return getTypeDescriptor (instance, propertyIndex).GetPrimitiveType();
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
bool  ECInstanceInteropHelper::IsStructArray  (IECInstanceCR instance, int propertyIndex)
    {
    return getTypeDescriptor (instance, propertyIndex).IsStructArray();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Dylan.Rush                      1/11
+---------------+---------------+---------------+---------------+---------------+------*/
bool  ECInstanceInteropHelper::IsArray  (IECInstanceCR instance, int propertyIndex)
    {
    return getTypeDescriptor (instance, propertyIndex).IsArray();
    }

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
ECObjectsStatus                 IECInstance::InsertArrayElements (WCharCP propertyAccessString, UInt32 index, UInt32 size, EC::EmbeddedInstanceCallbackP memoryReallocationCallbackP)
    {
    return _InsertArrayElements (propertyAccessString, index, size, memoryReallocationCallbackP);
    } 
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Adam.Klatzkin                   01/2010
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus                 IECInstance::AddArrayElements (WCharCP propertyAccessString, UInt32 size, EC::EmbeddedInstanceCallbackP memoryReallocationCallbackP)
    {
    return _AddArrayElements (propertyAccessString, size, memoryReallocationCallbackP);
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
WString                        IECInstance::ToString (WCharCP indent) const
    {
    return _ToString (indent);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    10/10
+---------------+---------------+---------------+---------------+---------------+------*/
ECInstanceDeserializationContextPtr ECInstanceDeserializationContext::CreateContext (ECSchemaCR schema, IStandaloneEnablerLocaterR standaloneEnablerLocater)
    {
    return new ECInstanceDeserializationContext (&schema, NULL, standaloneEnablerLocater);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    10/10
+---------------+---------------+---------------+---------------+---------------+------*/
ECInstanceDeserializationContextPtr ECInstanceDeserializationContext::CreateContext (ECSchemaDeserializationContextR context, IStandaloneEnablerLocaterR standaloneEnablerLocater)
    {
    return new ECInstanceDeserializationContext (NULL, &context, standaloneEnablerLocater);
    }

END_BENTLEY_EC_NAMESPACE


#include <xmllite.h>
#include <atlbase.h>

BEGIN_BENTLEY_EC_NAMESPACE

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   04/10
+---------------+---------------+---------------+---------------+---------------+------*/
static void                     AppendAccessString (WString& compoundAccessString, WString& baseAccessString, const WString& propertyName)
    {
    compoundAccessString = baseAccessString;
    compoundAccessString.append (propertyName);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   05/10
+---------------+---------------+---------------+---------------+---------------+------*/
static WCharCP                  GetPrimitiveTypeString (PrimitiveType primitiveType)
    {
    switch (primitiveType)
        {
        case PRIMITIVETYPE_Binary:
            return L"binary";

        case PRIMITIVETYPE_Boolean:
            return L"boolean";

        case PRIMITIVETYPE_DateTime:
            return L"dateTime";

        case PRIMITIVETYPE_Double:
            return L"double";

        case PRIMITIVETYPE_Integer:
            return L"int";

        case PRIMITIVETYPE_Long:
            return L"long";

        case PRIMITIVETYPE_Point2D:
            return L"point2d";

        case PRIMITIVETYPE_Point3D:
            return L"point3d";

        case PRIMITIVETYPE_String:
            return L"string";
        }

    assert (false);
    return L"";
    }


typedef std::vector<byte>   T_ByteArray;

static const wchar_t INSTANCEID_ATTRIBUTE[]         = L"instanceID";
static const wchar_t SOURCECLASS_ATTRIBUTE[]        = L"sourceClass";
static const wchar_t SOURCEINSTANCEID_ATTRIBUTE[]   = L"sourceInstanceID";
static const wchar_t TARGETCLASS_ATTRIBUTE[]        = L"targetClass";
static const wchar_t TARGETINSTANCEID_ATTRIBUTE[]   = L"targetInstanceID";
static const wchar_t XMLNS_ATTRIBUTE[]              = L"xmlns";
static const wchar_t XSI_NIL_ATTRIBUTE[]            = L"nil";

// =====================================================================================
// InstanceXMLReader class
// =====================================================================================
struct  InstanceXmlReader
{
private:
    WString                            m_fileName;
    CComPtr <IStream>                   m_stream;
    CComPtr <IXmlReader>                m_xmlReader;
    WString                            m_fullSchemaName;
    ECSchemaCP                          m_schema;
    ECInstanceDeserializationContextR   m_context;


public:
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   05/10
+---------------+---------------+---------------+---------------+---------------+------*/
InstanceXmlReader (ECInstanceDeserializationContextR context, CComPtr <IStream> stream)
    :
    m_context (context), m_stream (stream), m_xmlReader (NULL), m_schema (NULL)
    {
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   05/10
+---------------+---------------+---------------+---------------+---------------+------*/
InstanceXmlReader (ECInstanceDeserializationContextR context, WCharCP fileName)
    :
    m_context (context), m_fileName (fileName), m_stream (NULL), m_xmlReader (NULL), m_schema (NULL)
    {
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   05/10
+---------------+---------------+---------------+---------------+---------------+------*/
InstanceReadStatus   Init ()
    {
    // different constructors set different members, according to the source of the stream and the reader.
    HRESULT     status;
    if (NULL == m_stream)
        {
        if FAILED (status = SHCreateStreamOnFileW (m_fileName.c_str (), STGM_READ, &m_stream))
            return INSTANCE_READ_STATUS_FileNotFound;
        }

    if (NULL == m_xmlReader)
        {
        if (FAILED (status = CreateXmlReader (__uuidof(IXmlReader), (void**) &m_xmlReader, NULL)))
            return INSTANCE_READ_STATUS_CantCreateXmlReader;

        if (FAILED (status= m_xmlReader->SetInput (m_stream)))
            return INSTANCE_READ_STATUS_CantSetStream;
        }
    return INSTANCE_READ_STATUS_Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   05/10
+---------------+---------------+---------------+---------------+---------------+------*/
InstanceReadStatus   TranslateStatus (HRESULT status)
    {
    struct ErrorMap { HRESULT m_hResult; InstanceReadStatus m_ixrStatus; };
    static ErrorMap s_errorMap[] = 
        {
        { WC_E_DECLELEMENT,         INSTANCE_READ_STATUS_BadElement              },
        { WC_E_NAME,                INSTANCE_READ_STATUS_NoElementName           },  
        { WC_E_ELEMENTMATCH,        INSTANCE_READ_STATUS_EndElementDoesntMatch   },
        { WC_E_ENTITYCONTENT,       INSTANCE_READ_STATUS_BadElement              },  
        { WC_E_ROOTELEMENT,         INSTANCE_READ_STATUS_BadElement              },    
        { S_FALSE,                  INSTANCE_READ_STATUS_XmlFileIncomplete       },
        };
    
    for (int iError=0; iError < _countof (s_errorMap); iError++)
        {
        if (status == s_errorMap[iError].m_hResult)
            return s_errorMap[iError].m_ixrStatus;
        }
    return INSTANCE_READ_STATUS_XmlParseError;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   05/10
+---------------+---------------+---------------+---------------+---------------+------*/
InstanceReadStatus   ReadInstance (IECInstancePtr& ecInstance)
    {
    // The Instance XML element starts with a node that has the name of the class of the instance.
    HRESULT         status;
    XmlNodeType     nodeType;
    bool            nonComment = false;
    bool            gotComment = false;

    while (S_OK == (status = m_xmlReader->Read (&nodeType)))
        {
        switch (nodeType) 
            {
            case XmlNodeType_EndElement:
                {
                // we should not hit this.
                assert (false);
                return INSTANCE_READ_STATUS_Success;
                }

            case XmlNodeType_Element:
                {
                ECClassCP       ecClass;

                nonComment = true;
                // the first Element tells us ECClass of the instance.
                InstanceReadStatus       ixrStatus;
                if (INSTANCE_READ_STATUS_Success != (ixrStatus = GetInstance (&ecClass, ecInstance)))
                    return ixrStatus;

                // this reads the property members and consumes the XmlNodeType_EndElement corresponding to this XmlNodeType_Element.
                return ReadInstanceOrStructMembers (*ecClass, ecInstance.get(), NULL);
                }

            case XmlNodeType_Comment:
                {
                gotComment = true;
                break;
                }
            default:
                {
                // we can ignore the other types.
                nonComment = true;
                break;
                }
            }
        }
    if (gotComment && !nonComment)
        return INSTANCE_READ_STATUS_CommentOnly;

    // should not get here.
    return TranslateStatus (status);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    10/10
+---------------+---------------+---------------+---------------+---------------+------*/
ECSchemaCP       GetSchema()
    {
    if (NULL != m_schema)
        return m_schema;

    m_schema = m_context.GetSchemaCP();

//WIP_FUSION: Do we want to check for mismatches between the supplied schema name/version and m_fullSchemaName from the instance

    if (NULL != m_schema)
        return m_schema;

    ECSchemaDeserializationContextPtr schemaContext = m_context.GetSchemaContextPtr();

    if (schemaContext.IsValid())
        {
        WString    schemaName;
        UInt32      versionMajor;
        UInt32      versionMinor;

        if (SUCCESS == ECSchema::ParseSchemaFullName (schemaName, versionMajor, versionMinor, m_fullSchemaName))
            m_schema = ECSchema::LocateSchema (schemaName, versionMajor, versionMinor, *schemaContext);
        }

    return m_schema;    
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   04/10
+---------------+---------------+---------------+---------------+---------------+------*/
InstanceReadStatus   GetInstance (ECClassCP* ecClass, IECInstancePtr& ecInstance)
    {
    *ecClass = NULL;

    // when this method is called, we are positioned at the node that should have a className.
    HRESULT         status;
    if (FAILED (status = m_xmlReader->MoveToElement()))
        return INSTANCE_READ_STATUS_BadElement;

    WCharCP      className;
    if (FAILED (status = m_xmlReader->GetLocalName (&className, NULL)))
        return INSTANCE_READ_STATUS_NoElementName;

    // read the className attributes.
    for (status = m_xmlReader->MoveToFirstAttribute(); S_OK == status; status = m_xmlReader->MoveToNextAttribute())
        {
        // we have an attribute.
        WCharCP      attributeName;
        if (FAILED (status = m_xmlReader->GetLocalName (&attributeName, NULL)))
            return TranslateStatus (status);

        // see if it's the xmlns attribute... that's the schema name.
        if (0 == wcscmp (XMLNS_ATTRIBUTE, attributeName))
            {
            // get the value.
            WCharCP      schemaName;

            if (FAILED (status = m_xmlReader->GetValue (&schemaName, NULL)))
                return TranslateStatus (status);

            m_fullSchemaName.assign (schemaName);
            }
        }

    ECSchemaCP  schema = GetSchema();
    if (NULL == schema)
        {
        ECObjectsLogger::Log()->errorv (L"Failed to locate ECSchema %s", m_fullSchemaName.c_str());
        return INSTANCE_READ_STATUS_ECSchemaNotFound;
        }

    // see if we can find the class from the schema.
    ECClassCP    foundClass;
    if (NULL == (foundClass = schema->GetClassP (className)))
        {
        WString schemaName;
        UInt32  majorVersion;
        UInt32  minorVersion;
        bool checkName = ECOBJECTS_STATUS_Success == ECSchema::ParseSchemaFullName (schemaName, majorVersion, minorVersion, m_fullSchemaName);
        
        ECSchemaReferenceList refList = schema->GetReferencedSchemas();
        ECSchemaReferenceList::const_iterator schemaIterator;
        for (schemaIterator = refList.begin(); schemaIterator != refList.end(); schemaIterator++)
            {
            if (checkName && (*schemaIterator)->GetName() != schemaName)
                continue;
                
            if (NULL != (foundClass = (*schemaIterator)->GetClassP (className)))
                break;
            }
        }
    if (NULL == foundClass)
        {
        ECObjectsLogger::Log()->errorv (L"Failed to find ECClass %s in %s", className, m_fullSchemaName.c_str());
        return INSTANCE_READ_STATUS_ECClassNotFound;
        }

    *ecClass = foundClass;

    // NEEDSWORK: we could first look for an optional enabler supplied via the context
    StandaloneECEnablerPtr      standaloneEnabler  = foundClass->GetDefaultStandaloneEnabler();
                                ecInstance         = standaloneEnabler->CreateInstance().get();

    bool                        needSourceClass    = false;
    bool                        needSourceId       = false;
    bool                        needTargetClass    = false;
    bool                        needTargetId       = false;

    // if relationship, need the attributes.
    IECRelationshipInstance*    relationshipInstance = dynamic_cast <IECRelationshipInstance*> (ecInstance.get());
    if (NULL != relationshipInstance)
        needSourceClass = needSourceId = needTargetClass = needTargetId = true;

    // read the instance attributes.
    for (status = m_xmlReader->MoveToFirstAttribute(); S_OK == status; status = m_xmlReader->MoveToNextAttribute())
        {
        // we have an attribute.
        WCharCP      attributeName;
        if (FAILED (status = m_xmlReader->GetLocalName (&attributeName, NULL)))
            return TranslateStatus (status);

        WCharCP      pQName;
        if (S_OK == m_xmlReader->GetQualifiedName(&pQName, NULL))
            {
            if ((L':' == pQName[5]) && (0 == wcsncmp (XMLNS_ATTRIBUTE, pQName, 5)))
                continue;  // skip xmlns:xxxxx attributes
            }

        // see if it's the instanceId attribute.
        if (0 == wcscmp (INSTANCEID_ATTRIBUTE, attributeName))
            {
            // get the value.
            WCharCP  instanceId;
            if (FAILED (status = m_xmlReader->GetValue (&instanceId, NULL)))
                return TranslateStatus (status);
            ecInstance->SetInstanceId (instanceId);
            }

        else if (0 == wcscmp (SOURCEINSTANCEID_ATTRIBUTE, attributeName))
            {
            // get the value.
            WCharCP  sourceInstanceId;
            if (FAILED (status = m_xmlReader->GetValue (&sourceInstanceId, NULL)))
                return TranslateStatus (status);
#if defined (NEEDSWORK_RELATIONSHIP)
            relationshipInstance->SetSourceInstanceId (sourceInstanceId);
#endif
            }
        else if (0 == wcscmp (SOURCECLASS_ATTRIBUTE, attributeName))
            {
            // get the value.
            WCharCP  sourceClass;
            if (FAILED (status = m_xmlReader->GetValue (&sourceClass, NULL)))
                return TranslateStatus (status);
#if defined (NEEDSWORK_RELATIONSHIP)
            relationshipInstance->SetSourceClass (sourceClass);
#endif
            }
        else if (0 == wcscmp (TARGETINSTANCEID_ATTRIBUTE, attributeName))
            {
            // get the value.
            WCharCP  targetInstanceId;
            if (FAILED (status = m_xmlReader->GetValue (&targetInstanceId, NULL)))
                return TranslateStatus (status);
#if defined (NEEDSWORK_RELATIONSHIP)
            relationshipInstance->SetSourceInstanceId (targetInstanceId);
#endif
            }
        else if (0 == wcscmp (TARGETCLASS_ATTRIBUTE, attributeName))
            {
            // get the value.
            WCharCP  targetClass;
            if (FAILED (status = m_xmlReader->GetValue (&targetClass, NULL)))
                return TranslateStatus (status);
#if defined (NEEDSWORK_RELATIONSHIP)
            relationshipInstance->SetSourceClass (targetClass);
#endif
            }
        else if (0 == wcscmp (XMLNS_ATTRIBUTE, attributeName))
            {
            WCharCP  nameSpace;
            if (FAILED (status = m_xmlReader->GetValue (&nameSpace, NULL)))
                return TranslateStatus (status);
            WCharCP  schemaName = foundClass->GetSchema().GetName().c_str();
            if (0 != wcsncmp (schemaName, nameSpace, wcslen (schemaName)))
                ECObjectsLogger::Log()->warningv(L"ECInstance of (%s) has xmlns (%s) that disagrees with its ECSchemaName (%s)", 
                    foundClass->GetName().c_str(), nameSpace, schemaName);
            }

        else
            {
            ECObjectsLogger::Log()->warningv(L"ECInstance of (%s) has an unrecognized attribute (%s)", 
                    foundClass->GetName().c_str(), attributeName);
            }

        // the namespace should agree with the schema name.
        }

    // IsEmptyElement returns false if the reader is positioned on an attribute node, even if attribute's parent element is empty. 
    // Pay no attention to the return code - if there are no attributes MoveToElement returns S_FALSE.
    m_xmlReader->MoveToElement();

    return INSTANCE_READ_STATUS_Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   04/10
+---------------+---------------+---------------+---------------+---------------+------*/
InstanceReadStatus   ReadInstanceOrStructMembers (ECClassCR ecClass, IECInstanceP ecInstance, WString* baseAccessString)
    {
    // On entry, the reader is positioned in the content of an instance or struct.

    HRESULT         status;

    // if it's an empty node, all members of the instance are NULL.
    if (m_xmlReader->IsEmptyElement())
        return INSTANCE_READ_STATUS_Success;

    XmlNodeType     nodeType;
    while (S_OK == (status = m_xmlReader->Read (&nodeType)))
        {
        switch (nodeType) 
            {
            case XmlNodeType_Element:
                InstanceReadStatus   propertyStatus;
                if (INSTANCE_READ_STATUS_Success != (propertyStatus = ReadProperty (ecClass, ecInstance, baseAccessString)))
                    return propertyStatus;
                break;

            case XmlNodeType_EndElement:
                // we have encountered the end of the class or struct.
                return INSTANCE_READ_STATUS_Success;

            case XmlNodeType_Text:
                {
                // we do not expect there to be any "value" in a struct of class, only property nodes.
                assert (false);
                break;
                }

            default:
                {
                // simply ignore white space, comments, etc.
                break;
                }
            }
        }
    // shouldn't get here.
    assert (false);
    return TranslateStatus (status);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   04/10
+---------------+---------------+---------------+---------------+---------------+------*/
InstanceReadStatus   ReadProperty (ECClassCR ecClass, IECInstanceP ecInstance, WString* baseAccessString)
    {
    // on entry, the reader is positioned at the Element.
    // get the element name, which is the property name.
    HRESULT         status;
    WCharCP        propertyName;
    if (FAILED (status = m_xmlReader->GetLocalName (&propertyName, NULL)))
        return INSTANCE_READ_STATUS_NoElementName;

    // try to find the property in the class.
    ECPropertyP ecProperty;
    if (NULL == (ecProperty = ecClass.GetPropertyP (propertyName)))
        {
        ECObjectsLogger::Log()->warningv (L"No ECProperty '%s' found in ECClass '%s'. Value will be ignored.", propertyName, ecClass.GetName().c_str());
        // couldn't find it, skip the rest of the property.
        return SkipToElementEnd ();
        }

    PrimitiveECPropertyP    primitiveProperty;
    ArrayECPropertyP        arrayProperty;
    StructECPropertyP       structProperty;
    if (NULL != (primitiveProperty = ecProperty->GetAsPrimitiveProperty()))
        return ReadPrimitiveProperty (primitiveProperty, ecInstance, baseAccessString);
                //Above is good, if SkipToElementEnd() is returned from ReadPrimitiveValue.
    else if (NULL != (arrayProperty = ecProperty->GetAsArrayProperty()))
        return ReadArrayProperty (arrayProperty, ecInstance, baseAccessString);
    else if (NULL != (structProperty = ecProperty->GetAsStructProperty()))
        return ReadEmbeddedStructProperty (structProperty, ecInstance, baseAccessString);

    // should be one of those!
    assert (false);
    return INSTANCE_READ_STATUS_BadECProperty;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Bill.Steinbock                  10/2010
+---------------+---------------+---------------+---------------+---------------+------*/
InstanceReadStatus   ReadCustomSerializedStruct (StructECPropertyP structProperty, IECInstanceP ecInstance, WString* baseAccessString, ICustomECStructSerializerP customECStructSerializerP)
    {
    // On entry, the reader is positioned in the content of an instance or struct.
    HRESULT         status;

    // if it's an empty node, all members of the instance are NULL.
    if (m_xmlReader->IsEmptyElement())
        return INSTANCE_READ_STATUS_Success;


    XmlNodeType     nodeType;
    while (S_OK == (status = m_xmlReader->Read (&nodeType)))
        {
        bool positionedAtText = false;
        switch (nodeType) 
            {
            case XmlNodeType_Element:
                // a custom serializer should only allow a single text value.
                assert (false);
                break;

            case XmlNodeType_EndElement:
                // we have encountered the end of the class or struct.
                return INSTANCE_READ_STATUS_Success;

            case XmlNodeType_Text:
                {
                positionedAtText = true;
                break;
                }

            default:
                {
                // simply ignore white space, comments, etc.
                break;
                }
            }

        if (positionedAtText)
            break;
        }

    WCharCP  propertyValueString;
    if (FAILED (status = m_xmlReader->GetValue (&propertyValueString, NULL)))
        return TranslateStatus (status);

    WString    thisAccessString;
    if (NULL != baseAccessString)
        AppendAccessString (thisAccessString, *baseAccessString, structProperty->GetName());
    else
        thisAccessString = structProperty->GetName().c_str();
    thisAccessString.append (L".");

    customECStructSerializerP->LoadStructureFromString (structProperty, *ecInstance, thisAccessString.c_str(), propertyValueString);

    return INSTANCE_READ_STATUS_Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   04/10
+---------------+---------------+---------------+---------------+---------------+------*/
InstanceReadStatus   ReadEmbeddedStructProperty (StructECPropertyP structProperty, IECInstanceP ecInstance, WString* baseAccessString)
    {
    // empty element OK for struct - all members are null.
    if (m_xmlReader->IsEmptyElement())
        return INSTANCE_READ_STATUS_Success;

    WString    thisAccessString;
    if (NULL != baseAccessString)
        AppendAccessString (thisAccessString, *baseAccessString, structProperty->GetName());
    else
        thisAccessString = structProperty->GetName().c_str();
    thisAccessString.append (L".");

    ICustomECStructSerializerP customECStructSerializerP = CustomStructSerializerManager::GetManager().GetCustomSerializer (structProperty, *ecInstance);
    if (customECStructSerializerP)
        return ReadCustomSerializedStruct (structProperty, ecInstance, baseAccessString, customECStructSerializerP);

    return ReadInstanceOrStructMembers (structProperty->GetType(), ecInstance, &thisAccessString);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   04/10
+---------------+---------------+---------------+---------------+---------------+------*/
InstanceReadStatus   ReadPrimitiveProperty (PrimitiveECPropertyP primitiveProperty, IECInstanceP ecInstance, WString* baseAccessString)
    {
    // on entry, we are positioned in the PrimitiveProperty element.
    PrimitiveType                   propertyType = primitiveProperty->GetType();
    InstanceReadStatus   ixrStatus;
    ECValue         ecValue;
    if (INSTANCE_READ_STATUS_Success != (ixrStatus = ReadPrimitiveValue (ecValue, propertyType)))
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

    assert (ECOBJECTS_STATUS_Success == setStatus);

    return INSTANCE_READ_STATUS_Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   04/10
+---------------+---------------+---------------+---------------+---------------+------*/
InstanceReadStatus   ReadArrayProperty (ArrayECPropertyP arrayProperty, IECInstanceP ecInstance, WString* baseAccessString)
    {
    // on entry, the reader is positioned at the element that indicates the start of the array.
    // empty element OK for array - no members.
    if (m_xmlReader->IsEmptyElement())
        return INSTANCE_READ_STATUS_Success;

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

        // read nodes - we expect to find an Element with LocalName matching the primitive type.
        XmlNodeType     nodeType;
        HRESULT         status;
        while (S_OK == (status = m_xmlReader->Read (&nodeType)))
            {
            switch (nodeType)
                {
                case XmlNodeType_Element:
                    {
                    // validate the LocalName against the expected primitiveType.
                    WCharCP        primitiveTypeName;
                    if (FAILED (status = m_xmlReader->GetLocalName (&primitiveTypeName, NULL)))
                        return INSTANCE_READ_STATUS_NoElementName;

                    if (!ValidateArrayPrimitiveType (primitiveTypeName, memberType))
                        {
                        ECObjectsLogger::Log()->warningv(L"Incorrectly formatted array element found in array %ls.  Expected: %ls  Found: %ls",
                                                        accessString, GetPrimitiveTypeString (memberType), primitiveTypeName);
                        //Skip this element to start looking for elements with the correct primitive type.
                        SkipToElementEnd();
                            continue;
                        //By continuing here, we make sure that the bad value is not set.
                        }

                    // now we know the type and we are positioned at the element containing the value.
                    // read it, populating the ECInstance using accessString and arrayIndex.
                    InstanceReadStatus   ixrStatus;
                    ECValue                         ecValue;
                    if (INSTANCE_READ_STATUS_Success != (ixrStatus = ReadPrimitiveValue (ecValue, memberType)))
                        return ixrStatus;
                    if(ecValue.IsUninitialized())
                        {
                        //A malformed value was found.  A warning was shown; just move on.
                        continue;
                        }

                    if ( ! isFixedSizeArray)
                        ecInstance->AddArrayElements (accessString.c_str(), 1);

                    ECObjectsStatus   setStatus;
                    if (ECOBJECTS_STATUS_Success != (setStatus = ecInstance->SetValue (accessString.c_str(), ecValue, index)))
                        {
                        assert (false);
                        return INSTANCE_READ_STATUS_CantSetValue;
                        }

                    // increment the array index.
                    index++;

                    break;
                    }

                case XmlNodeType_Text:
                    {
                    // we don't expend that there is any text in an array element, just child elements.
                    assert (false);
                    break;
                    }

                case XmlNodeType_EndElement:
                    // we have encountered the end of the array.
                    return INSTANCE_READ_STATUS_Success;

                default:
                    {
                    // simply ignore white space, comments, etc.
                    break;
                    }
                }
            }

        // shouldn't get here.
        return TranslateStatus (status);
        }

    else if (ARRAYKIND_Struct == arrayKind)
        {
        ECClassCP   structMemberType = arrayProperty->GetStructElementType();

        // read nodes - we expect to find an Element with LocalName being the class name of structMemberType.
        // For polymorphic arrays, the LocalName might also be the name of a class that has structMemberType as a BaseType.
        XmlNodeType     nodeType;
        HRESULT         status;
        while (S_OK == (status = m_xmlReader->Read (&nodeType)))
            {
            switch (nodeType)
                {
                case XmlNodeType_Element:
                    {
                    // validate the LocalName against the structMemberType
                    WCharCP        structName;
                    if (FAILED (status = m_xmlReader->GetLocalName (&structName, NULL)))
                        return INSTANCE_READ_STATUS_NoElementName;

                    ECClassCP   thisMemberType;
                    if (NULL == (thisMemberType = ValidateArrayStructType (structName, structMemberType)))
                        return INSTANCE_READ_STATUS_BadArrayElement;

                    InstanceReadStatus ixrStatus;
                    if (INSTANCE_READ_STATUS_Success != (ixrStatus = ReadStructArrayMember (*thisMemberType, ecInstance, accessString, index)))
                        return ixrStatus;

                    // increment the array index.
                    index++;

                    break;
                    }

                case XmlNodeType_EndElement:
                    // we have encountered the end of the array.
                    return INSTANCE_READ_STATUS_Success;

                default:
                    {
                    // simply ignore white space, comments, etc.
                    break;
                    }
                }
            }
        // shouldn't get here.
        return TranslateStatus (status);
        }
    
    return INSTANCE_READ_STATUS_Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   04/10
+---------------+---------------+---------------+---------------+---------------+------*/
InstanceReadStatus   ReadStructArrayMember (ECClassCR structClass, IECInstanceP owningInstance, WString& accessString, UInt32 index)
    {
    // On entry, the reader is positioned at the element that starts the struct.
    // we have to create an IECInstance for the array member.
    ClassLayoutP                    classLayout         = ClassLayout::BuildFromClass (structClass, 0, 0);
    StandaloneECEnablerPtr          standaloneEnabler   = StandaloneECEnabler::CreateEnabler (structClass, *classLayout, owningInstance->GetEnablerR(), true);

    // The following way causes an assert in ECPerSchemaCache::LoadSchema processing SetSchemaPAndAddRefToSharedSchemaCache (schemaP) because the schemacache's ptr was set recursively when processing struct arrays
    //StandaloneECEnablerPtr standaloneEnabler = owningInstance->GetEnablerR().ObtainStandaloneInstanceEnabler (structClass.GetSchema().GetName().c_str(), structClass.GetName().c_str());

    if (standaloneEnabler.IsNull())
        return INSTANCE_READ_STATUS_UnableToGetStandaloneEnabler;

    // create the instance.
    IECInstancePtr                  structInstance      = standaloneEnabler->CreateInstance().get();

    InstanceReadStatus   ixrStatus;
    if (INSTANCE_READ_STATUS_Success != (ixrStatus = ReadInstanceOrStructMembers (structClass, structInstance.get(), NULL)))
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
    assert (ECOBJECTS_STATUS_Success == setStatus);

    return INSTANCE_READ_STATUS_Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   04/10
+---------------+---------------+---------------+---------------+---------------+------*/
InstanceReadStatus   ReadPrimitiveValue (ECValueR ecValue, PrimitiveType propertyType)
    {
    // The reader is positioned at the XmlNodeType_Element that holds the value. 
    // We expect to find a Text element with the value, advance until we do.

    HRESULT         status;
    // First check to see if this is set to NULL
    for (status = m_xmlReader->MoveToFirstAttribute(); S_OK == status; status = m_xmlReader->MoveToNextAttribute())
        {
        // we have an attribute.
        WCharCP      qualifiedName;
        if (S_OK == m_xmlReader->GetQualifiedName(&qualifiedName, NULL))
            {
            if ((L':' == qualifiedName[5]) && (0 == wcsncmp (XMLNS_ATTRIBUTE, qualifiedName, 5)))
                continue;  // skip xmlns:xxxxx attributes
            }

        WCharCP      attributeName;
        if (FAILED (status = m_xmlReader->GetLocalName (&attributeName, NULL)))
            return TranslateStatus (status);

        // see if it's the instanceId attribute.
        if (0 == wcscmp (XSI_NIL_ATTRIBUTE, attributeName))
            {
            // get the value.
            WCharCP  isNil;
            if (FAILED (status = m_xmlReader->GetValue (&isNil, NULL)))
                return TranslateStatus (status);

            if ((0 == wcscmp (isNil, L"True")) || (0 == wcscmp (isNil, L"true")) || 
                (0 == wcscmp (isNil, L"TRUE")) || (0 == wcscmp (isNil, L"1")))
                {
                ecValue.SetToNull();
                return INSTANCE_READ_STATUS_Success;
                }
            }
            //m_xmlReader->MoveToElement();
        }

    if (m_xmlReader->IsEmptyElement())
        {
        return INSTANCE_READ_STATUS_Success;
        }

    XmlNodeType     nodeType;
    while (S_OK == (status = m_xmlReader->Read (&nodeType)))
        {
        bool positionedAtText = false;
        switch (nodeType) 
            {
            case XmlNodeType_Element:
                assert (false);
                return INSTANCE_READ_STATUS_UnexpectedElement;

            case XmlNodeType_EndElement:
                // we have encountered the end of the class or struct without getting a value from the element.
                // we will break here to keep the ECValue null.
                return INSTANCE_READ_STATUS_Success;

            case XmlNodeType_Text:
                {
                // we do not expect there to be any "value" in a struct of class, only property nodes.
                positionedAtText = true;
                break;
                }

            default:
                {
                // simply ignore white space, comments, etc.
                break;
                }
            }

        if (positionedAtText)
            break;
        }

    WCharCP  propertyValueString;
    if (FAILED (status = m_xmlReader->GetValue (&propertyValueString, NULL)))
        return TranslateStatus (status);

    switch (propertyType)
        {
        case PRIMITIVETYPE_Binary:
            {
            T_ByteArray                     byteArray;

            if (INSTANCE_READ_STATUS_Success != ConvertStringToByteArray (byteArray, propertyValueString))
                {
                ECObjectsLogger::Log()->warningv(L"Type mismatch in deserialization: \"%ls\" is not Binary", propertyValueString);
                return SkipToElementEnd();
                }
            ecValue.SetBinary (&byteArray.front(), byteArray.size(), true);
            break;
            }

        case PRIMITIVETYPE_Boolean:
            {
            bool boolValue = ((0 == wcscmp (propertyValueString, L"True")) || (0 == wcscmp (propertyValueString, L"true")) || 
                             (0 == wcscmp (propertyValueString, L"TRUE")) || (0 == wcscmp (propertyValueString, L"1")));
            ecValue.SetBoolean (boolValue);
            break;
            }

        case PRIMITIVETYPE_DateTime:
            {
            Int64   ticks;
            if (1 != swscanf (propertyValueString, L"%I64d", &ticks))
                {
                ECObjectsLogger::Log()->warningv(L"Type mismatch in deserialization: \"%ls\" is not DateTime", propertyValueString);
                return SkipToElementEnd();
                }

            ecValue.SetDateTimeTicks (ticks);
            break;
            }

        case PRIMITIVETYPE_Double:
            {
            double  doubleValue;
            if (1 != swscanf (propertyValueString, L"%lg", &doubleValue))
                {
                ECObjectsLogger::Log()->warningv(L"Type mismatch in deserialization: \"%ls\" is not Double", propertyValueString);
                return SkipToElementEnd();
                }
            ecValue.SetDouble (doubleValue);
            break;
            }

        case PRIMITIVETYPE_Integer:
            {
            Int32   intValue;
            if (1 != swscanf (propertyValueString, L"%d", &intValue))
                {
                ECObjectsLogger::Log()->warningv(L"Type mismatch in deserialization: \"%ls\" is not Integer", propertyValueString);
                return SkipToElementEnd();
                }
            ecValue.SetInteger (intValue);
            break;
            }

        case PRIMITIVETYPE_Long:
            {
            Int64   longValue;
            if (1 != swscanf (propertyValueString, L"%I64d", &longValue))
                {
                ECObjectsLogger::Log()->warningv(L"Type mismatch in deserialization: \"%ls\" is not Long", propertyValueString);
                return SkipToElementEnd();
                }
            ecValue.SetLong (longValue);
            break;
            }

        case PRIMITIVETYPE_Point2D:
            {
            DPoint2d point2d;
            if (2 != swscanf (propertyValueString, L"%lg,%lg", &point2d.x, &point2d.y))
                {
                ECObjectsLogger::Log()->warningv(L"Type mismatch in deserialization: \"%ls\" is not Point2D", propertyValueString);
                return SkipToElementEnd();
                }
            ecValue.SetPoint2D (point2d);
            break;
            }

        case PRIMITIVETYPE_Point3D:
            {
            DPoint3d point3d;
            if (3 != swscanf (propertyValueString, L"%lg,%lg,%lg", &point3d.x, &point3d.y, &point3d.z))
                {
                ECObjectsLogger::Log()->warningv(L"Type mismatch in deserialization: \"%ls\" is not Point3D", propertyValueString);
                return SkipToElementEnd();
                }
            ecValue.SetPoint3D (point3d);
            break;
            }

        case PRIMITIVETYPE_String:
            {
            ecValue.SetString (propertyValueString);
            break;
            }

        default:
            {
            assert (false);
            return INSTANCE_READ_STATUS_BadPrimitivePropertyType;
            }
        }

    // we want to find the EndElement for this node.
    while (S_OK == (status = m_xmlReader->Read (&nodeType)))
        {
        switch (nodeType) 
            {
            case XmlNodeType_Element:
                return INSTANCE_READ_STATUS_UnexpectedElement;

            case XmlNodeType_EndElement:
                // we have encountered the end of the class or struct.
                return INSTANCE_READ_STATUS_Success;

            default:
                {
                // simply ignore white space, comments, etc.
                break;
                }
            }
        }

    // shouldn't get here.
    assert (false);
    return TranslateStatus (status);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   04/10
+---------------+---------------+---------------+---------------+---------------+------*/
InstanceReadStatus   ConvertStringToByteArray (T_ByteArray& byteData, WCharCP stringData)
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
bool                            ValidateArrayPrimitiveType (WCharCP typeFound, PrimitiveType expectedType)
    {
    return (0 == wcscmp (typeFound, GetPrimitiveTypeString (expectedType)));
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
        assert (false);
        return NULL;
        }
    if (!classFound->Is (expectedType))
        {
        assert (false);
        return NULL;
        }

    return classFound;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   04/10
+---------------+---------------+---------------+---------------+---------------+------*/
InstanceReadStatus   SkipToElementEnd ()
    {
    // skips from current point to the end of the current element.
    BOOL isEmpty = m_xmlReader->IsEmptyElement();
    if (isEmpty)
        {
        while (S_OK == m_xmlReader->MoveToNextAttribute())
            {
            // just skip it
            }
        return INSTANCE_READ_STATUS_Success;
        }

    m_xmlReader->MoveToElement();
    UINT initialDepth = 0;
    m_xmlReader->GetDepth(&initialDepth);

    HRESULT         status;
    XmlNodeType     nodeType;
    while (S_OK == (status = m_xmlReader->Read (&nodeType)))
        {
        // ignore everything except the end of the element.
        if (XmlNodeType_EndElement == nodeType)
            {
            UINT depth = 0;
            m_xmlReader->GetDepth(&depth);
            if (depth <= initialDepth + 1) // skip nested elements, too
                return INSTANCE_READ_STATUS_Success;
            }
        }

    return INSTANCE_READ_STATUS_BadElement;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   05/10
+---------------+---------------+---------------+---------------+---------------+------*/
UInt32 GetLineNumber ()
    {
    UInt32  lineNumber = 0;
    m_xmlReader->GetLineNumber (&lineNumber);
    return lineNumber;
    }

};

// =====================================================================================
// InstanceXMLWriter class
// =====================================================================================
struct  InstanceXmlWriter
{
private:
    WString                    m_fileName;
    CComPtr <IStream>           m_stream;
    CComPtr <IXmlWriter>        m_xmlWriter;
    CComPtr <IXmlWriterOutput>  m_xmlOutput;
    bool                        m_compacted;

public:
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   05/10
+---------------+---------------+---------------+---------------+---------------+------*/
InstanceXmlWriter (CComPtr <IStream> stream)
    {
    m_stream            = stream;
    m_xmlWriter         = NULL;
    m_compacted         = false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   05/10
+---------------+---------------+---------------+---------------+---------------+------*/
InstanceXmlWriter (WString fileName)
    {
    m_fileName          = fileName;
    m_stream            = NULL;
    m_xmlWriter         = NULL;
    m_compacted         = false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   05/10
+---------------+---------------+---------------+---------------+---------------+------*/
InstanceWriteStatus     Init ()
    {
    // different constructors set different members, according to the source of the stream and the writer.
    HRESULT     status;
    if (NULL == m_stream)
        {
        if FAILED (status = SHCreateStreamOnFileW (m_fileName.c_str (), STGM_CREATE | STGM_WRITE, &m_stream))
            return INSTANCE_WRITE_STATUS_CantCreateStream;
        }

    if (NULL == m_xmlWriter)
        {
        if (FAILED (status = CreateXmlWriter (__uuidof(IXmlWriter), (void**) &m_xmlWriter, NULL)))
            return INSTANCE_WRITE_STATUS_CantCreateXmlWriter;


        if (FAILED (status = CreateXmlWriterOutputWithEncodingName(m_stream, 0, L"utf-16", &m_xmlOutput)))
            return INSTANCE_WRITE_STATUS_CantCreateXmlWriter;

        if (FAILED (status= m_xmlWriter->SetOutput (m_xmlOutput)))
            return INSTANCE_WRITE_STATUS_CantSetStream;

        if (!m_compacted)
            m_xmlWriter->SetProperty (XmlWriterProperty_Indent, true);
        }
    return INSTANCE_WRITE_STATUS_Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   04/10
+---------------+---------------+---------------+---------------+---------------+------*/
InstanceWriteStatus     WriteInstance (IECInstanceCR instance, bool writeStart, bool writeInstanceId)
    {
    ECClassCR               ecClass     = instance.GetClass();
    ECSchemaCR              ecSchema    = ecClass.GetSchema();

    HRESULT status;
    if (writeStart)
        {
        if (S_OK != (status = m_xmlWriter->WriteStartDocument (XmlStandalone_Omit)))
            return TranslateStatus (status);
        }

    // start by writing the name of the class as an element, with the schema name as the namespace.
    size_t size = wcslen(ecSchema.GetName().c_str()) + 8;
    WCharP fullSchemaName = (wchar_t*)malloc(size * sizeof(wchar_t));
    swprintf(fullSchemaName, size, L"%s.%02d.%02d", ecSchema.GetName().c_str(), ecSchema.GetVersionMajor(), ecSchema.GetVersionMinor());
    if (S_OK != (status = m_xmlWriter->WriteStartElement (NULL, ecClass.GetName().c_str(), fullSchemaName)))
        {
        free(fullSchemaName);
        return TranslateStatus (status);
        }
    free(fullSchemaName);

    if (writeInstanceId)
        {
        if (S_OK != (status = m_xmlWriter->WriteAttributeString (NULL, INSTANCEID_ATTRIBUTE, NULL, instance.GetInstanceId().c_str())))
            return TranslateStatus (status);
        }

    WritePropertiesOfClassOrStructArrayMember (ecClass, instance, NULL);

    if (S_OK != (status = m_xmlWriter->WriteEndElement ()))
        return TranslateStatus (status);

    m_xmlWriter->WriteEndDocument ();

    m_xmlWriter->Flush();

    return INSTANCE_WRITE_STATUS_Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   04/10
+---------------+---------------+---------------+---------------+---------------+------*/
InstanceWriteStatus     WritePropertiesOfClassOrStructArrayMember (ECClassCR ecClass, IECInstanceCR ecInstance, WString* baseAccessString)
    {
    CustomStructSerializerManagerR customStructSerializerMgr = CustomStructSerializerManager::GetManager();

    ECPropertyIterableCR    collection  = ecClass.GetProperties (true);
    FOR_EACH (ECPropertyP ecProperty, collection)
        {
        PrimitiveECPropertyP        primitiveProperty;
        ArrayECPropertyP            arrayProperty;
        StructECPropertyP           structProperty;
        InstanceWriteStatus ixwStatus;
            
        if (NULL != (primitiveProperty = ecProperty->GetAsPrimitiveProperty()))
            ixwStatus = WritePrimitiveProperty (*primitiveProperty, ecInstance, baseAccessString);
        else if (NULL != (arrayProperty = ecProperty->GetAsArrayProperty()))
            ixwStatus = WriteArrayProperty (*arrayProperty, ecInstance, baseAccessString);
        else if (NULL != (structProperty = ecProperty->GetAsStructProperty()))
            {
            ICustomECStructSerializerP customECStructSerializerP = customStructSerializerMgr.GetCustomSerializer (structProperty, ecInstance);

            if (customECStructSerializerP)
                {
                ixwStatus = INSTANCE_WRITE_STATUS_BadPrimitivePropertyType;

                HRESULT     status;
                WString xmlString; 

                if (ECOBJECTS_STATUS_Success == customECStructSerializerP->GenerateXmlString (xmlString, structProperty, ecInstance, baseAccessString?baseAccessString->c_str():NULL))
                    {
                    if (S_OK != (status = m_xmlWriter->WriteStartElement (NULL, structProperty->GetName().c_str(), NULL)))
                        return TranslateStatus (status);

                    if (S_OK != (status = m_xmlWriter->WriteChars (xmlString.c_str(), static_cast <UINT> (xmlString.length()))))
                        return TranslateStatus (status);
                    
                    if (S_OK != (status = m_xmlWriter->WriteEndElement ()))
                        return TranslateStatus (status);

                    ixwStatus = INSTANCE_WRITE_STATUS_Success;
                    }
                }
            else
                {
                ixwStatus = WriteEmbeddedStructProperty (*structProperty, ecInstance, baseAccessString);
                }
            }

        if (INSTANCE_WRITE_STATUS_Success != ixwStatus)
            {
            assert (false);
            return ixwStatus;
            }
        }

     return INSTANCE_WRITE_STATUS_Success;
     }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   04/10
+---------------+---------------+---------------+---------------+---------------+------*/
InstanceWriteStatus     WritePrimitiveProperty (PrimitiveECPropertyR primitiveProperty, IECInstanceCR ecInstance, WString* baseAccessString)
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

    HRESULT     status;
    if (S_OK != (status = m_xmlWriter->WriteStartElement (NULL, propertyName.c_str(), NULL)))
        return TranslateStatus (status);

    InstanceWriteStatus     ixwStatus;
    PrimitiveType                   propertyType = primitiveProperty.GetType();
    if (INSTANCE_WRITE_STATUS_Success != (ixwStatus = WritePrimitiveValue (ecValue, propertyType)))
        return ixwStatus;

    if (S_OK != (status = m_xmlWriter->WriteEndElement ()))
        return TranslateStatus (status);

    return INSTANCE_WRITE_STATUS_Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   04/10
+---------------+---------------+---------------+---------------+---------------+------*/
InstanceWriteStatus     WritePrimitiveValue (ECValueCR ecValue, PrimitiveType propertyType)
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
                HRESULT         status;
                if (S_OK != (status = m_xmlWriter->WriteChars (byteString.c_str(), static_cast <UINT> (byteString.length()))))
                    return TranslateStatus (status);
                else
                    return INSTANCE_WRITE_STATUS_Success;
                }
            else
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
            swprintf (outString, L"%I64d", ecValue.GetDateTimeTicks());
            break;
            }

        case PRIMITIVETYPE_Double:
            {
            swprintf (outString, L"%.13g", ecValue.GetDouble());
            break;
            }

        case PRIMITIVETYPE_Integer:
            {
            swprintf (outString, L"%d", ecValue.GetInteger());
            break;
            }

        case PRIMITIVETYPE_Long:
            {
            swprintf (outString, L"%I64d", ecValue.GetLong());
            break;
            }

        case PRIMITIVETYPE_Point2D:
            {
            DPoint2d    point2d = ecValue.GetPoint2D();
            swprintf (outString, L"%.13g,%.13g", point2d.x, point2d.y);
            break;
            }

        case PRIMITIVETYPE_Point3D:
            {
            DPoint3d    point3d = ecValue.GetPoint3D();
            swprintf (outString, L"%.13g,%.13g,%.13g", point3d.x, point3d.y, point3d.z);
            break;
            }

        case PRIMITIVETYPE_String:
            {
            WCharCP  stringValue = ecValue.GetString ();
            HRESULT         status;
            if (S_OK != (status = m_xmlWriter->WriteChars (stringValue, static_cast <UINT> (wcslen (stringValue)))))
                return TranslateStatus (status);
            return INSTANCE_WRITE_STATUS_Success;
            }

        default:
            {
            assert (false);
            return INSTANCE_WRITE_STATUS_BadPrimitivePropertyType;
            }
        }

    HRESULT     status;
    if (S_OK != (status = m_xmlWriter->WriteChars (outString, static_cast <UINT> (wcslen (outString)))))
        return TranslateStatus (status);

    return INSTANCE_WRITE_STATUS_Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   04/10
+---------------+---------------+---------------+---------------+---------------+------*/
InstanceWriteStatus     WriteArrayProperty (ArrayECPropertyR arrayProperty, IECInstanceCR ecInstance, WString* baseAccessString)
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

    // write the start element, which consists of the member name.
    HRESULT     status;

    if (S_OK != (status = m_xmlWriter->WriteStartElement (NULL, arrayProperty.GetName().c_str(), NULL)))
        return TranslateStatus (status);

    InstanceWriteStatus     ixwStatus;
    if (ARRAYKIND_Primitive == arrayKind)
        {
        PrimitiveType   memberType  = arrayProperty.GetPrimitiveElementType();
        WCharCP  typeString  = GetPrimitiveTypeString (memberType);
        for (int index=0; ; index++)
            {
            if (SUCCESS != ecInstance.GetValue (ecValue, accessString.c_str(), index))
                break;

            if (S_OK != (status = m_xmlWriter->WriteStartElement (NULL, typeString, NULL)))
                return TranslateStatus (status);

            // write the primitve value
            if (INSTANCE_WRITE_STATUS_Success != (ixwStatus = WritePrimitiveValue (ecValue, memberType)))
                {
                assert (false);
                return ixwStatus;
                }

            if (S_OK != (status = m_xmlWriter->WriteEndElement ()))
                return TranslateStatus (status);
            }
        }
    else if (ARRAYKIND_Struct == arrayKind)
        {
        ECClassCP   memberClass = arrayProperty.GetStructElementType();
        for (int index=0; ; index++)
            {
            if (SUCCESS != ecInstance.GetValue (ecValue, accessString.c_str(), index))
                break;

            // the XML element tag is the struct type.
            assert (ecValue.IsStruct());

            IECInstancePtr  structInstance = ecValue.GetStruct();
            if (!structInstance.IsValid())
                {
                assert (false);
                break;
                }

            ECClassCR   structClass = structInstance->GetClass();
            assert (structClass.Is (memberClass));

            if (S_OK != (status = m_xmlWriter->WriteStartElement (NULL, structClass.GetName().c_str(), NULL)))
                return TranslateStatus (status);

            WritePropertiesOfClassOrStructArrayMember (structClass, *structInstance.get(), NULL);

            if (S_OK != (status = m_xmlWriter->WriteEndElement ()))
                return TranslateStatus (status);
            }
        }
    else
        {
        // unexpected arrayKind - should never happen.
        assert (false);
        }


    // write the end element for the array as a whole.
    if (S_OK != (status = m_xmlWriter->WriteEndElement ()))
        return TranslateStatus (status);

    return INSTANCE_WRITE_STATUS_Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   05/10
+---------------+---------------+---------------+---------------+---------------+------*/
InstanceWriteStatus     WriteEmbeddedStructProperty (StructECPropertyR structProperty, IECInstanceCR ecInstance, WString* baseAccessString)
    {
    // the tag of the element for an embedded struct is the property name.
    HRESULT     status;
    if (S_OK != (status = m_xmlWriter->WriteStartElement (NULL, structProperty.GetName().c_str(), NULL)))
        return TranslateStatus (status);

    WString    thisAccessString;
    if (NULL != baseAccessString)
        AppendAccessString (thisAccessString, *baseAccessString, structProperty.GetName());
    else
        thisAccessString = structProperty.GetName().c_str();
    thisAccessString.append (L".");

    ECClassCR   structClass = structProperty.GetType();
    WritePropertiesOfClassOrStructArrayMember (structClass, ecInstance, &thisAccessString);

    // write the end element for the array as a whole.
    if (S_OK != (status = m_xmlWriter->WriteEndElement ()))
        return TranslateStatus (status);

    return INSTANCE_WRITE_STATUS_Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   04/10
+---------------+---------------+---------------+---------------+---------------+------*/
WString                        ConvertByteArrayToString (const byte *byteData, size_t numBytes)
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

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   05/10
+---------------+---------------+---------------+---------------+---------------+------*/
InstanceWriteStatus     TranslateStatus (HRESULT status)
    {
#if 0
    struct ErrorMap { HRESULT m_hResult; InstanceWriteStatus m_ixrStatus; };
    // I'm not sure what the possible errors are when writing, so this method does pretty much nothing.
    static ErrorMap s_errorMap[] = 
        {
        { WC_E_DECLELEMENT,         INSTANCE_READ_STATUS_BadElement              },
        { WC_E_NAME,                INSTANCE_READ_STATUS_NoElementName           },  
        { WC_E_ELEMENTMATCH,        INSTANCE_READ_STATUS_EndElementDoesntMatch   },
        { WC_E_ENTITYCONTENT,       INSTANCE_READ_STATUS_BadElement              },      
        { S_FALSE,                  INSTANCE_READ_STATUS_XmlFileIncomplete       },
        };
    
    for (int iError=0; iError < _countof (s_errorMap); iError++)
        {
        if (status == s_errorMap[iError].m_hResult)
            return s_errorMap[iError].m_ixrStatus;
        }
#endif
    return INSTANCE_WRITE_STATUS_XmlWriteError;
    }

};

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   04/10
+---------------+---------------+---------------+---------------+---------------+------*/
InstanceReadStatus   IECInstance::ReadFromXmlFile (IECInstancePtr& ecInstance, WCharCP fileName, ECInstanceDeserializationContextR context)
    {
    InstanceXmlReader reader (context, fileName);

    InstanceReadStatus   status;
    if (INSTANCE_READ_STATUS_Success != (status = reader.Init ()))
        return status;

    return reader.ReadInstance (ecInstance);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Carole.MacDonald                05/2010
+---------------+---------------+---------------+---------------+---------------+------*/
InstanceReadStatus   IECInstance::ReadFromXmlStream (IECInstancePtr& ecInstance, IStreamP stream, ECInstanceDeserializationContextR context)
    {
    InstanceXmlReader reader (context, stream);

    InstanceReadStatus   status;
    if (INSTANCE_READ_STATUS_Success != (status = reader.Init ()))
        return status;

    return reader.ReadInstance (ecInstance);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   03/2011
+---------------+---------------+---------------+---------------+---------------+------*/
static InstanceReadStatus   ReportStatus (InstanceReadStatus status, WCharCP xmlString, IECInstancePtr& ecInstance)
    {
    if (INSTANCE_READ_STATUS_Success != status)
        ECObjectsLogger::Log()->errorv (L"Failed to deserialize instance from XML string. Status %d, string %s", status, xmlString);
    else
        ECObjectsLogger::Log()->tracev (L"Native ECInstance of type %s deserialized from string", ecInstance.IsValid() ? ecInstance->GetClass().GetName() : L"Null");
    return status;
    }
        
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Carole.MacDonald                05/2010
+---------------+---------------+---------------+---------------+---------------+------*/
InstanceReadStatus   IECInstance::ReadFromXmlString (IECInstancePtr& ecInstance, WCharCP xmlString, ECInstanceDeserializationContextR context)
    {
    CComPtr <IStream> stream;
    if (S_OK != ::CreateStreamOnHGlobal(NULL,TRUE,&stream))
        return ReportStatus (INSTANCE_READ_STATUS_CantCreateStream, xmlString, ecInstance);

    LARGE_INTEGER liPos = {0};
    if (S_OK != stream->Seek(liPos, STREAM_SEEK_SET, NULL))
        return ReportStatus (INSTANCE_READ_STATUS_CantCreateStream, xmlString, ecInstance);

    ULARGE_INTEGER uliSize = { 0 };
    stream->SetSize(uliSize);

    ULONG bytesWritten;
    ULONG ulSize = (ULONG) wcslen(xmlString) * sizeof(wchar_t);

    if (S_OK != stream->Write(xmlString, ulSize, &bytesWritten))
        return ReportStatus (INSTANCE_READ_STATUS_CantCreateStream, xmlString, ecInstance);

    if (ulSize != bytesWritten)
        return ReportStatus (INSTANCE_READ_STATUS_CantCreateStream, xmlString, ecInstance);

    if (S_OK != stream->Seek(liPos, STREAM_SEEK_SET, NULL))
        return ReportStatus (INSTANCE_READ_STATUS_CantCreateStream, xmlString, ecInstance);

    InstanceXmlReader reader (context, stream);

    InstanceReadStatus   status;
    if (INSTANCE_READ_STATUS_Success != (status = reader.Init ()))
        return ReportStatus (status, xmlString, ecInstance);

    return ReportStatus (reader.ReadInstance (ecInstance), xmlString, ecInstance);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   04/10
+---------------+---------------+---------------+---------------+---------------+------*/
InstanceWriteStatus     IECInstance::WriteToXmlFile (WCharCP fileName, bool isStandAlone, bool writeInstanceId)
    {
    InstanceXmlWriter writer (fileName);

    InstanceWriteStatus   status;
    if (INSTANCE_WRITE_STATUS_Success != (status = writer.Init ()))
        return status;

    return writer.WriteInstance (*this, isStandAlone, writeInstanceId);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Carole.MacDonald                05/2010
+---------------+---------------+---------------+---------------+---------------+------*/
InstanceWriteStatus     IECInstance::WriteToXmlStream (IStreamP stream, bool isStandAlone, bool writeInstanceId)
    {
    InstanceXmlWriter writer (stream);

    InstanceWriteStatus   status;
    if (INSTANCE_WRITE_STATUS_Success != (status = writer.Init ()))
        return status;

    return writer.WriteInstance (*this, isStandAlone, writeInstanceId);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Carole.MacDonald                06/2010
+---------------+---------------+---------------+---------------+---------------+------*/
InstanceWriteStatus     IECInstance::WriteToXmlString (WString & ecInstanceXml, bool isStandAlone, bool writeInstanceId)
    {
    InstanceWriteStatus   status;

    CComPtr <IStream> stream;
    if (S_OK != ::CreateStreamOnHGlobal(NULL,TRUE,&stream))
        return INSTANCE_WRITE_STATUS_CantCreateStream;

    InstanceXmlWriter writer (stream);
    if (INSTANCE_WRITE_STATUS_Success != (status = writer.Init ()))
        return status;

    if (INSTANCE_WRITE_STATUS_Success != (status = writer.WriteInstance(*this, isStandAlone, writeInstanceId)))
        return status;

    LARGE_INTEGER liPos = {0};
    STATSTG statstg;
    
    ULARGE_INTEGER beginningPos;
    if (S_OK != stream->Seek(liPos, STREAM_SEEK_SET, &beginningPos))
        return INSTANCE_WRITE_STATUS_CantSetStream;

    memset (&statstg, 0, sizeof(statstg));
    if (S_OK != stream->Stat(&statstg, STATFLAG_NONAME))
        return INSTANCE_WRITE_STATUS_CantReadFromStream;
    
    WCharP xml = (WCharP) malloc((statstg.cbSize.LowPart + 1) * sizeof(wchar_t) );
    ULONG bytesRead;
    stream->Read(xml, statstg.cbSize.LowPart * sizeof(wchar_t), &bytesRead);
    xml[bytesRead / sizeof(wchar_t)] = L'\0';
    ecInstanceXml = xml;

    // There is an invisible UNICODE character in the beginning that messes things up
    ecInstanceXml = ecInstanceXml.substr(1);
    free(xml);
    return INSTANCE_WRITE_STATUS_Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Bill.Steinbock                  05/2011
+---------------+---------------+---------------+---------------+---------------+------*/
WCharCP IECInstance::GetInstanceLabelPropertyName () const
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
ECObjectsStatus      IECInstance::_GetDisplayLabel (WString& displayLabel) const
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
ECObjectsStatus      IECInstance::GetDisplayLabel (WString& displayLabel) const
    {
    return  _GetDisplayLabel (displayLabel);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Bill.Steinbock                  05/2011
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus      IECInstance::_SetDisplayLabel (WCharCP displayLabel)    
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
ECObjectsStatus      IECInstance::SetDisplayLabel (WCharCP displayLabel)    
    {
    return  _SetDisplayLabel (displayLabel);
    }

///////////////////////////////////////////////////////////////////////////////////////////////
//   IECRelationshipInstance
///////////////////////////////////////////////////////////////////////////////////////////////
void          IECRelationshipInstance::SetSource (IECInstanceP instance)
    {
    _SetSource (instance);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Bill.Steinbock                  08/2010
+---------------+---------------+---------------+---------------+---------------+------*/
IECInstancePtr                    IECRelationshipInstance::GetSource () const
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
IECInstancePtr                    IECRelationshipInstance::GetTarget () const
    {
    return _GetTarget ();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Bill.Steinbock                  05/2011
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus IECRelationshipInstance::GetSourceOrderId (Int64& sourceOrderId) const
    {
    return _GetSourceOrderId (sourceOrderId);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Bill.Steinbock                  05/2011
+---------------+---------------+---------------+---------------+---------------+------*/
 ECObjectsStatus IECRelationshipInstance::GetTargetOrderId (Int64& targetOrderId) const
    {
    return _GetTargetOrderId (targetOrderId);
    }

///////////////////////////////////////////////////////////////////////////////////////////////
//   IECWipRelationshipInstance
///////////////////////////////////////////////////////////////////////////////////////////////
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Bill.Steinbock                  04/2011
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus  IECWipRelationshipInstance::SetName (WCharCP name)   
    {
    return _SetName (name);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Bill.Steinbock                  04/2011
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus  IECWipRelationshipInstance::SetSourceOrderId (Int64 sourceOrderId)
    {
    return _SetSourceOrderId (sourceOrderId);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Bill.Steinbock                  04/2011
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus IECWipRelationshipInstance::SetTargetOrderId (Int64 targetOrderId)
    {
    return _SetTargetOrderId (targetOrderId);
    }

END_BENTLEY_EC_NAMESPACE
