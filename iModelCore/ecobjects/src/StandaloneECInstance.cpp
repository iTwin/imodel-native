/*--------------------------------------------------------------------------------------+
|
|     $Source: ecobjects/native/StandaloneECInstance.cpp $
|
|   $Copyright: (c) 2010 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ECObjectsPch.h"

BEGIN_BENTLEY_EC_NAMESPACE

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    CaseyMullen     09/09
+---------------+---------------+---------------+---------------+---------------+------*/        
StandaloneECInstance::StandaloneECInstance (StandaloneECEnablerCR enabler, byte * data, UInt32 size) :
        MemoryInstanceSupport (true),
        m_standaloneEnabler(const_cast<StandaloneECEnablerP>(&enabler)), // WIP_FUSION: can we get rid of the const cast?
        m_bytesAllocated(size), m_data(data) 
    {
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    CaseyMullen     01/10
+---------------+---------------+---------------+---------------+---------------+------*/        
StandaloneECInstance::StandaloneECInstance (StandaloneECEnablerCR enabler, UInt32 minimumBufferSize) :
        MemoryInstanceSupport (true),
        m_standaloneEnabler(const_cast<StandaloneECEnablerP>(&enabler)), // WIP_FUSION: can we get rid of the const cast?
        m_bytesAllocated(0), m_data(NULL) 
    {
    UInt32 size = max (minimumBufferSize, enabler.GetClassLayout().GetSizeOfFixedSection());
    m_data = (byte*)malloc (size);
    m_bytesAllocated = size;
    ClearValues();
    }
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    CaseyMullen     09/09
+---------------+---------------+---------------+---------------+---------------+------*/        
void                StandaloneECInstance::ClearValues ()
    {
    InitializeMemory (m_standaloneEnabler->GetClassLayout(), m_data, m_bytesAllocated);
    }
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    CaseyMullen     12/09
+---------------+---------------+---------------+---------------+---------------+------*/    
void                StandaloneECInstance::_Free ()
    { 
    delete this;
    };
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    CaseyMullen     10/09
+---------------+---------------+---------------+---------------+---------------+------*/    
void                StandaloneECInstance::_Dump() const
    {
    return DumpInstanceData (m_standaloneEnabler->GetClassLayout());
    }
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    CaseyMullen     10/09
+---------------+---------------+---------------+---------------+---------------+------*/    
ECEnablerCR         StandaloneECInstance::_GetEnabler() const
    {
    return *m_standaloneEnabler;
    }
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    CaseyMullen     09/09
+---------------+---------------+---------------+---------------+---------------+------*/
bool                StandaloneECInstance::_IsReadOnly() const
    {
    return false;
    }
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    CaseyMullen     10/09
+---------------+---------------+---------------+---------------+---------------+------*/    
std::wstring        StandaloneECInstance::_GetInstanceId() const
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
* @bsimethod                                                    CaseyMullen     09/09
+---------------+---------------+---------------+---------------+---------------+------*/
bool                StandaloneECInstance::_IsMemoryInitialized () const
    {
    return m_data != NULL;
    }
        
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    CaseyMullen     09/09
+---------------+---------------+---------------+---------------+---------------+------*/
byte const *        StandaloneECInstance::_GetDataForRead () const
    {
    return m_data;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    CaseyMullen     09/09
+---------------+---------------+---------------+---------------+---------------+------*/
byte *              StandaloneECInstance::_GetDataForWrite () const
    {
    return m_data;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    CaseyMullen     09/09
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt           StandaloneECInstance::_ModifyData (UInt32 offset, void const * newData, UInt32 dataLength)
    {
    PRECONDITION (NULL != m_data, ERROR);
    PRECONDITION (offset + dataLength <= m_bytesAllocated, ERROR); //WIP_FUSION ERROR_MemoryBoundsOverrun
    byte * dest = m_data + offset;
    memcpy (dest, newData, dataLength);
    
    return SUCCESS;
    }
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    CaseyMullen     09/09
+---------------+---------------+---------------+---------------+---------------+------*/
UInt32              StandaloneECInstance::GetBytesUsed () const
    {
    if (NULL == m_data)
        return 0;

    return m_standaloneEnabler->GetClassLayout().CalculateBytesUsed(m_data);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    CaseyMullen     01/10
+---------------+---------------+---------------+---------------+---------------+------*/
byte const *        StandaloneECInstance::GetDataForRead () const
    {
    return _GetDataForRead();
    }
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    CaseyMullen     01/10
+---------------+---------------+---------------+---------------+---------------+------*/
ClassLayoutCR       StandaloneECInstance::GetClassLayout () const
    {
    return m_standaloneEnabler->GetClassLayout();
    }
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    CaseyMullen     09/09
+---------------+---------------+---------------+---------------+---------------+------*/
UInt32              StandaloneECInstance::_GetBytesAllocated () const
    {
    return m_bytesAllocated;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    CaseyMullen     09/09
+---------------+---------------+---------------+---------------+---------------+------*/
void                StandaloneECInstance::_ShrinkAllocation (UInt32 newAllocation)
    {
    DEBUG_EXPECT (false && "WIP_FUSION: needs implementation");
    } 

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    CaseyMullen     09/09
+---------------+---------------+---------------+---------------+---------------+------*/
void                StandaloneECInstance::_FreeAllocation ()
    {
    free (m_data); 
    m_data = NULL;
    }
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    CaseyMullen     10/09
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt           StandaloneECInstance::_GrowAllocation (UInt32 bytesNeeded)
    {
    DEBUG_EXPECT (m_bytesAllocated > 0);
    DEBUG_EXPECT (NULL != m_data);
    // WIP_FUSION: add performance counter
            
    UInt32 newSize = 2 * (m_bytesAllocated + bytesNeeded); // Assume the growing trend will continue.
    byte * reallocedData = (byte*)realloc(m_data, newSize);
    DEBUG_EXPECT (NULL != reallocedData);
    if (NULL == reallocedData)
        return ERROR;
        
    m_data = reallocedData;    
    m_bytesAllocated = newSize;
    
    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    CaseyMullen     09/09
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt           StandaloneECInstance::_GetValue (ECValueR v, const wchar_t * propertyAccessString, UInt32 nIndices, UInt32 const * indices) const
    {
    ClassLayoutCR classLayout = m_standaloneEnabler->GetClassLayout();
    
    return GetValueFromMemory (classLayout, v, propertyAccessString, nIndices, indices);
    }
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    CaseyMullen     09/09
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt           StandaloneECInstance::_SetValue (const wchar_t * propertyAccessString, ECValueCR v, UInt32 nIndices, UInt32 const * indices)
    {
    ClassLayoutCR classLayout = m_standaloneEnabler->GetClassLayout();
    StatusInt status = SetValueToMemory (classLayout, propertyAccessString, v, nIndices, indices);

    return status;
    }
    
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
StandaloneECEnablerPtr StandaloneECEnabler::CreateEnabler (ECClassCR ecClass, ClassLayoutCR classLayout)
    {
    return new StandaloneECEnabler (ecClass, classLayout);
    }
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    CaseyMullen     12/09
+---------------+---------------+---------------+---------------+---------------+------*/
wchar_t const *         StandaloneECEnabler::_GetName() const
    {
    return L"Bentley::EC::StandaloneECEnabler";
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
StandaloneECInstanceP   StandaloneECEnabler::CreateInstance (UInt32 minimumBufferSize)
    {
    return new StandaloneECInstance (*this, minimumBufferSize);
    }    
    
END_BENTLEY_EC_NAMESPACE