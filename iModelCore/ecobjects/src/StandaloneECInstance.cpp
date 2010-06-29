/*--------------------------------------------------------------------------------------+
|
|     $Source: ecobjects/native/StandaloneECInstance.cpp $
|
|   $Copyright: (c) 2010 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ECObjectsPch.h"

BEGIN_BENTLEY_EC_NAMESPACE

///////////////////////////////////////////////////////////////////////////////////////////
//  MemoryECInstanceBase
///////////////////////////////////////////////////////////////////////////////////////////
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Bill.Steinbock                  04/2010
+---------------+---------------+---------------+---------------+---------------+------*/
MemoryECInstanceBase::MemoryECInstanceBase (byte * data, UInt32 size, bool allowWritingDirectlyToInstanceMemory) :
        MemoryInstanceSupport (allowWritingDirectlyToInstanceMemory),
        m_bytesAllocated(size), m_data(data), m_structValueId (0)
    {
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Bill.Steinbock                  04/2010
+---------------+---------------+---------------+---------------+---------------+------*/
MemoryECInstanceBase::MemoryECInstanceBase (ClassLayoutCR classLayout, UInt32 minimumBufferSize, bool allowWritingDirectlyToInstanceMemory) :
        MemoryInstanceSupport (allowWritingDirectlyToInstanceMemory),
        m_bytesAllocated(0), m_data(NULL), m_structValueId (0)
    {
    UInt32 size = max (minimumBufferSize, classLayout.GetSizeOfFixedSection());
    m_data = (byte*)malloc (size);
    m_bytesAllocated = size;

    InitializeMemory (classLayout, m_data, m_bytesAllocated);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Bill.Steinbock                  04/2010
+---------------+---------------+---------------+---------------+---------------+------*/
MemoryECInstanceBase::~MemoryECInstanceBase ()
    {
    _FreeAllocation();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Bill.Steinbock                  04/2010
+---------------+---------------+---------------+---------------+---------------+------*/
void MemoryECInstanceBase::SetData (byte * data, UInt32 size, bool freeExisitingData) //The MemoryECInstanceBase will take ownership of the memory
    {
    if (freeExisitingData && m_data)
        free (m_data);

    m_data = data;
    m_bytesAllocated = size;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    CaseyMullen     09/09
+---------------+---------------+---------------+---------------+---------------+------*/
bool                MemoryECInstanceBase::_IsMemoryInitialized () const
    {
    return m_data != NULL;
    }
        
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    CaseyMullen     09/09
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus           MemoryECInstanceBase::_ModifyData (UInt32 offset, void const * newData, UInt32 dataLength)
    {
    PRECONDITION (NULL != m_data, ECOBJECTS_STATUS_PreconditionViolated);
    PRECONDITION (offset + dataLength <= m_bytesAllocated, ECOBJECTS_STATUS_MemoryBoundsOverrun);
    byte * dest = m_data + offset;
    memcpy (dest, newData, dataLength);
    
    return ECOBJECTS_STATUS_Success;
    }
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    CaseyMullen     09/09
+---------------+---------------+---------------+---------------+---------------+------*/
void                MemoryECInstanceBase::_ShrinkAllocation (UInt32 newAllocation)
    {
    DEBUG_EXPECT (false && "WIP_FUSION: needs implementation");
    } 

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    CaseyMullen     09/09
+---------------+---------------+---------------+---------------+---------------+------*/
void                MemoryECInstanceBase::_FreeAllocation ()
    {
    free (m_data); 
    m_data = NULL;
    }
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    CaseyMullen     10/09
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus           MemoryECInstanceBase::_GrowAllocation (UInt32 bytesNeeded)
    {
    DEBUG_EXPECT (m_bytesAllocated > 0);
    DEBUG_EXPECT (NULL != m_data);
    // WIP_FUSION: add performance counter
            
    UInt32 newSize = 2 * (m_bytesAllocated + bytesNeeded); // Assume the growing trend will continue.
    byte * reallocedData = (byte*)realloc(m_data, newSize);
    DEBUG_EXPECT (NULL != reallocedData);
    if (NULL == reallocedData)
        return ECOBJECTS_STATUS_UnableToAllocateMemory;
        
    m_data = reallocedData;    
    m_bytesAllocated = newSize;
    
    return ECOBJECTS_STATUS_Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    CaseyMullen     09/09
+---------------+---------------+---------------+---------------+---------------+------*/
byte const *        MemoryECInstanceBase::_GetData () const
    {
    return m_data;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    CaseyMullen     09/09
+---------------+---------------+---------------+---------------+---------------+------*/
UInt32              MemoryECInstanceBase::_GetBytesAllocated () const
    {
    return m_bytesAllocated;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Adam.Klatzkin                   02/2010
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus     MemoryECInstanceBase::_SetStructArrayValueToMemory (ECValueCR v, ClassLayoutCR classLayout, PropertyLayoutCR propertyLayout, UInt32 index)
    {
    IECInstancePtr p;
    ECValue binaryValue (PRIMITIVETYPE_Binary);
    m_structValueId++;
    if (v.IsNull())
        {
        p = NULL;
        binaryValue.SetToNull();
        }
    else
        {
        p = v.GetStruct();    
        binaryValue.SetBinary ((const byte *)&(m_structValueId), sizeof (StructValueIdentifier));
        }
        
    ECObjectsStatus status = SetPrimitiveValueToMemory (binaryValue, classLayout, propertyLayout, true, index);      
    if (status != ECOBJECTS_STATUS_Success)
        return status;
                    
    m_structValueMap[m_structValueId] = p;
    
    return ECOBJECTS_STATUS_Success; 
    }    
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Adam.Klatzkin                   02/2010
+---------------+---------------+---------------+---------------+---------------+------*/    
ECObjectsStatus           MemoryECInstanceBase::_GetStructArrayValueFromMemory (ECValueR v, PropertyLayoutCR propertyLayout, UInt32 index) const
    {
    ECValue binaryValue;
    ECObjectsStatus status = GetPrimitiveValueFromMemory (binaryValue, propertyLayout, true, index);      
    if (status != ECOBJECTS_STATUS_Success)
        return status;
        
    if (binaryValue.IsNull())
        {
        v.SetStruct(NULL);
        return ECOBJECTS_STATUS_Success;
        }        
                    
    size_t size;
    StructValueIdentifier structValueId = *(StructValueIdentifier*)binaryValue.GetBinary (size);    

    // WIP_FUSION - is there realy no better way to do this?  something like m_structValueMap[x];
    IECInstancePtr instancePtr = NULL;
    bmap<StructValueIdentifier, IECInstancePtr>::const_iterator  instanceIterator;
    instanceIterator = m_structValueMap.find (structValueId);    
    if ( instanceIterator != m_structValueMap.end() )
        instancePtr = instanceIterator->second;
       
    v.SetStruct (instancePtr.get());
    
    return ECOBJECTS_STATUS_Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    CaseyMullen     01/10
+---------------+---------------+---------------+---------------+---------------+------*/
byte const *        MemoryECInstanceBase::GetData () const
    {
    return _GetData();
    }
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    CaseyMullen     09/09
+---------------+---------------+---------------+---------------+---------------+------*/
UInt32              MemoryECInstanceBase::GetBytesUsed () const
    {
    if (NULL == m_data)
        return 0;

    return GetClassLayout().CalculateBytesUsed(m_data);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    CaseyMullen     09/09
+---------------+---------------+---------------+---------------+---------------+------*/        
void                MemoryECInstanceBase::ClearValues ()
    {
    InitializeMemory (GetClassLayout(), m_data, m_bytesAllocated);
    }
   
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Bill.Steinbock                  04/2010
+---------------+---------------+---------------+---------------+---------------+------*/
ClassLayoutCR       MemoryECInstanceBase::GetClassLayout () const
    {
    return _GetClassLayout();
    }

///////////////////////////////////////////////////////////////////////////////////////////
//  StandaloneECInstance
///////////////////////////////////////////////////////////////////////////////////////////
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    CaseyMullen     09/09
+---------------+---------------+---------------+---------------+---------------+------*/        
StandaloneECInstance::StandaloneECInstance (StandaloneECEnablerCR enabler, byte * data, UInt32 size) :
        MemoryECInstanceBase (data, size, true),
        m_sharedWipEnabler(const_cast<StandaloneECEnablerP>(&enabler)) // WIP_FUSION: can we get rid of the const cast?
    {
    m_sharedWipEnabler->AddRef ();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    CaseyMullen     01/10
+---------------+---------------+---------------+---------------+---------------+------*/        
StandaloneECInstance::StandaloneECInstance (StandaloneECEnablerCR enabler, UInt32 minimumBufferSize) :
        MemoryECInstanceBase (enabler.GetClassLayout(), minimumBufferSize, true),
        m_sharedWipEnabler(const_cast<StandaloneECEnablerP>(&enabler)) // WIP_FUSION: can we get rid of the const cast?
    {
    m_sharedWipEnabler->AddRef ();
    }
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    CaseyMullen     01/10
+---------------+---------------+---------------+---------------+---------------+------*/        
StandaloneECInstance::~StandaloneECInstance ()
    {
    m_sharedWipEnabler->Release ();

    //Logger::GetLogger()->tracev (L"StandaloneECInstance at 0x%x is being destructed. It references enabler 0x%x", this, m_sharedWipEnabler);
    }
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    CaseyMullen     10/09
+---------------+---------------+---------------+---------------+---------------+------*/    
ECEnablerCR         StandaloneECInstance::_GetEnabler() const
    {
    return *m_sharedWipEnabler;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    CaseyMullen     10/09
+---------------+---------------+---------------+---------------+---------------+------*/    
bwstring        StandaloneECInstance::_GetInstanceId() const
    {
    if (m_instanceId.size() == 0)
        {
        wchar_t id[1024];
        swprintf(id, sizeof(id)/sizeof(wchar_t), L"%s-0x%X", _GetEnabler().GetClass().GetName().c_str(), this);
        StandaloneECInstanceP thisNotConst = const_cast<StandaloneECInstanceP>(this);
        thisNotConst->m_instanceId = id;        
        }

    return m_instanceId;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    CaseyMullen     01/10
+---------------+---------------+---------------+---------------+---------------+------*/
ClassLayoutCR       StandaloneECInstance::_GetClassLayout () const
    {
    return m_sharedWipEnabler->GetClassLayout();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    CaseyMullen     09/09
+---------------+---------------+---------------+---------------+---------------+------*/
bool                StandaloneECInstance::_IsReadOnly() const
    {
    return false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    CaseyMullen     09/09
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt           StandaloneECInstance::_GetValue (ECValueR v, const wchar_t * propertyAccessString, bool useArrayIndex, UInt32 arrayIndex) const
    {
    ClassLayoutCR classLayout = GetClassLayout();

    return GetValueFromMemory (classLayout, v, propertyAccessString, useArrayIndex, arrayIndex);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    05/10
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt           StandaloneECInstance::_GetValue (ECValueR v, UInt32 propertyIndex, bool useArrayIndex, UInt32 arrayIndex) const
    {
    ClassLayoutCR classLayout = GetClassLayout();

    return GetValueFromMemory (classLayout, v, propertyIndex, useArrayIndex, arrayIndex);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    CaseyMullen     09/09
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt           StandaloneECInstance::_SetValue (const wchar_t * propertyAccessString, ECValueCR v, bool useArrayIndex, UInt32 arrayIndex)
    {
    ClassLayoutCR classLayout = GetClassLayout();
    StatusInt status = SetValueToMemory (classLayout, propertyAccessString, v, useArrayIndex, arrayIndex);

    return status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    05/10
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt           StandaloneECInstance::_SetValue (UInt32 propertyIndex, ECValueCR v, bool useArrayIndex, UInt32 arrayIndex)
    {
    ClassLayoutCR classLayout = GetClassLayout();
    StatusInt status = SetValueToMemory (classLayout, propertyIndex, v, useArrayIndex, arrayIndex);

    return status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Adam.Klatzkin                   01/2010
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus           StandaloneECInstance::_InsertArrayElements (const wchar_t * propertyAccessString, UInt32 index, UInt32 size)
    {
    ClassLayoutCR classLayout = GetClassLayout();
    ECObjectsStatus status = InsertNullArrayElementsAt (classLayout, propertyAccessString, index, size);
    
    return status;
    } 
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Adam.Klatzkin                   01/2010
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus           StandaloneECInstance::_AddArrayElements (const wchar_t * propertyAccessString, UInt32 size)
    {
    ClassLayoutCR classLayout = GetClassLayout();    
    ECObjectsStatus status = AddNullArrayElementsAt (classLayout, propertyAccessString, size);
    
    return status;
    }        

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Adam.Klatzkin                   01/2010
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus           StandaloneECInstance::_RemoveArrayElement (const wchar_t * propertyAccessString, UInt32 index)
    {
    return ECOBJECTS_STATUS_OperationNotSupported;
    } 

 /*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Adam.Klatzkin                   01/2010
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus           StandaloneECInstance::_ClearArray (const wchar_t * propertyAccessString)
    {
    return ECOBJECTS_STATUS_OperationNotSupported;
    }                      

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    CaseyMullen     10/09
+---------------+---------------+---------------+---------------+---------------+------*/    
void                StandaloneECInstance::_Dump() const
    {
    return DumpInstanceData (GetClassLayout());
    }

///////////////////////////////////////////////////////////////////////////////////////////
//  StandaloneECEnabler
///////////////////////////////////////////////////////////////////////////////////////////
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    CaseyMullen     12/09
+---------------+---------------+---------------+---------------+---------------+------*/    
StandaloneECEnabler::StandaloneECEnabler (ECClassCR ecClass, ClassLayoutCR classLayout) :
    ECEnabler (ecClass),
    ClassLayoutHolder (classLayout)
    {
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    CaseyMullen     12/09
+---------------+---------------+---------------+---------------+---------------+------*/    
StandaloneECEnablerPtr    StandaloneECEnabler::CreateEnabler (ECClassCR ecClass, ClassLayoutCR classLayout)
    {
    return new StandaloneECEnabler (ecClass, classLayout);
    }
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    CaseyMullen     12/09
+---------------+---------------+---------------+---------------+---------------+------*/
wchar_t const *           StandaloneECEnabler::_GetName() const
    {
    return L"Bentley::EC::StandaloneECEnabler";
    }
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    05/10
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt           StandaloneECEnabler::_GetPropertyIndex(UInt32& propertyIndex, const wchar_t * propertyAccessString) const
    {
    ClassLayoutCR       classLayout = GetClassLayout();

    return classLayout.GetPropertyIndex (propertyIndex, propertyAccessString);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    CaseyMullen     09/09
+---------------+---------------+---------------+---------------+---------------+------*/        
/*StandaloneECInstanceP   StandaloneECEnabler::CreateInstanceFromUninitializedMemory (byte * data, UInt32 size)
    {
    StandaloneECInstanceP instance = new StandaloneECInstance (*this, data, size);
    instance->ClearValues();
    
    return instance;
    }*/
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    CaseyMullen     09/09
+---------------+---------------+---------------+---------------+---------------+------*/        
StandaloneECInstancePtr   StandaloneECEnabler::CreateInstance (UInt32 minimumBufferSize)
    {
    return new StandaloneECInstance (*this, minimumBufferSize);
    }    
    
END_BENTLEY_EC_NAMESPACE