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
        m_standaloneEnabler(const_cast<StandaloneECEnablerP>(&enabler)), // WIP_FUSION: can we get rid of the const cast?
        m_bytesAllocated(size), m_data(data) 
    {
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    CaseyMullen     09/09
+---------------+---------------+---------------+---------------+---------------+------*/        
void                StandaloneECInstance::ClearValues ()
    {
    InitializeMemory (m_standaloneEnabler->GetClassLayout(), m_data, m_bytesAllocated);
    }
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    CaseyMullen     09/09
+---------------+---------------+---------------+---------------+---------------+------*/        
StandaloneECInstanceP StandaloneECInstance::CreateFromUninitializedMemory (StandaloneECEnablerCR enabler, byte * data, UInt32 size)
    {
    StandaloneECInstanceP instance = new StandaloneECInstance (enabler, data, size);
    instance->ClearValues();
    
    return instance;
    }
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    CaseyMullen     09/09
+---------------+---------------+---------------+---------------+---------------+------*/        
StandaloneECInstanceP StandaloneECInstance::CreateFromInitializedMemory (StandaloneECEnablerCR enabler, byte * data, UInt32 size)
    {
    return new StandaloneECInstance (enabler, data, size);
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
ECEnablerCR           StandaloneECInstance::_GetEnabler() const
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
byte const *         StandaloneECInstance::GetDataForRead () const
    {
    return _GetDataForRead();
    }
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    CaseyMullen     01/10
+---------------+---------------+---------------+---------------+---------------+------*/
ClassLayoutCR        StandaloneECInstance::GetClassLayout () const
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
            
    UInt32 newSize = 2 * (m_bytesAllocated + bytesNeeded); // Assume the growing trend will continue. The StandaloneECInstanceFactory will ensure that the final instances are trimmed down to an appropriate size
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
StandaloneECInstanceFactory::StandaloneECInstanceFactory (ClassLayoutCR classLayout, UInt32 slack, UInt32 initialBufferSize) : 
    m_nBegun(0), m_nFinished (0), m_nReallocationRequests(0),
    m_instanceUnderConstruction (NULL), m_minimumSlack (slack), m_data (NULL), m_size (0),
    m_standaloneEnabler (*(new StandaloneECEnabler (classLayout)))
    {
    UInt32 sizeOfFixedSection = classLayout.GetSizeOfFixedSection();
    
    if (initialBufferSize < sizeOfFixedSection)
        initialBufferSize = sizeOfFixedSection;
        
    m_size = initialBufferSize;
    }
 
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    CaseyMullen     12/09
+---------------+---------------+---------------+---------------+---------------+------*/    
BentleyStatus   StandaloneECInstanceFactory::BeginConstruction (StandaloneECInstanceP& instance)
    {
    PRECONDITION (NULL == instance && "The StandaloneECInstance passed to BeginConstruction must be NULL", ERROR);
    if (NULL != m_instanceUnderConstruction)
        {
        // WIP_FUSION: Log message "Programmer Error: Attempted to create a new IECInstance of %s under construction while there is already one under construction by this factory", m_enabler.GetClass().GetName().c_str());
        PRECONDITION (NULL == m_instanceUnderConstruction && "A given factory can only have one instance under construction at a time", ERROR);
        }

    ++m_nBegun;
    
    if (NULL == m_data)
        {
        m_data = (byte*)malloc (m_size);
        // WIP_FUSION: log it in debug, as a malloc
        }
        
    m_instanceUnderConstruction = StandaloneECInstance::CreateFromUninitializedMemory (m_standaloneEnabler, m_data, m_size);
    instance = m_instanceUnderConstruction;
    
    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    CaseyMullen     12/09
+---------------+---------------+---------------+---------------+---------------+------*/    
BentleyStatus   StandaloneECInstanceFactory::FinishConstruction (StandaloneECInstanceP& instance)
    {
    PRECONDITION (NULL != instance, ERROR);
    if (instance != m_instanceUnderConstruction)
        {
        // WIP_FUSION: Log message "Programmer Error: Attempted to finish a new IECInstance of %s that is not under construction by this factory!", m_enabler.GetClass().GetName().c_str());
        PRECONDITION (instance == m_instanceUnderConstruction && "Attempted to finish an instance that is not under construction by this factory!", ERROR);
        }

    ++m_nFinished;
    m_instanceUnderConstruction = NULL;

    if (instance->m_bytesAllocated > m_size)
        {
        // It was reallocated, set a new high-water mark
        DEBUG_EXPECT (instance->GetBytesUsed() > m_size && "The under-construction instance should not have been realloced unless it needed more memory.");
        
        ++m_nReallocationRequests;
        m_size = instance->m_bytesAllocated;
                
        m_data = NULL; // The StandaloneECInstance realloced our original buffer, and thereby took ownership of it
        // We don't malloc the new buffer at this time... maybe this is the last time we'll ever be called
        return SUCCESS;
        }
    
    UInt32 bytesUsed = instance->GetBytesUsed();
    
    DEBUG_EXPECT (bytesUsed <= m_size);
    
    UInt32 newSize = bytesUsed + m_minimumSlack;
    
    byte * newData = (byte*)malloc (newSize);
    memcpy (newData, instance->m_data, bytesUsed);
    instance->m_bytesAllocated = newSize;    
    instance->m_data = newData; // Swap in a new buffer that is just the right size
    
    return SUCCESS;
    }    

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    CaseyMullen     12/09
+---------------+---------------+---------------+---------------+---------------+------*/    
BentleyStatus   StandaloneECInstanceFactory::CancelConstruction (StandaloneECInstanceP& instance)
    {
    FinishConstruction (instance); // Not the most efficient thing to do, but the logic is much simpler this way.
    delete instance;
    
    instance = NULL;
    
    return SUCCESS;
    }
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    CaseyMullen     12/09
+---------------+---------------+---------------+---------------+---------------+------*/    
UInt32          StandaloneECInstanceFactory::GetReallocationCount ()
    {
    return m_nReallocationRequests;
    }
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    CaseyMullen     12/09
+---------------+---------------+---------------+---------------+---------------+------*/    
UInt32          StandaloneECInstanceFactory::GetBegunCount ()
    {
    return m_nBegun;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    CaseyMullen     12/09
+---------------+---------------+---------------+---------------+---------------+------*/    
UInt32          StandaloneECInstanceFactory::GetFinishedCount ()
    {
    return m_nFinished;
    }        
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    CaseyMullen     12/09
+---------------+---------------+---------------+---------------+---------------+------*/    
StandaloneECEnabler::StandaloneECEnabler (ClassLayoutCR classLayout) :
    ECEnabler (classLayout.GetClass()),
    ClassLayoutHolder (classLayout)
    {
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    CaseyMullen     12/09
+---------------+---------------+---------------+---------------+---------------+------*/    
StandaloneECEnablerPtr StandaloneECEnabler::CreateEnabler (ClassLayoutCR classLayout)
    {
    return new StandaloneECEnabler (classLayout);
    }
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    CaseyMullen     12/09
+---------------+---------------+---------------+---------------+---------------+------*/
wchar_t const * StandaloneECEnabler::_GetName() const
    {
    return L"Bentley::EC::StandaloneECEnabler";
    }
        
END_BENTLEY_EC_NAMESPACE