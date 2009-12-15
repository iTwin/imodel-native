/*--------------------------------------------------------------------------------------+
|
|     $Source: ecobjects/native/StandaloneInstance.cpp $
|
|   $Copyright: (c) 2009 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ECObjectsPch.h"

BEGIN_BENTLEY_EC_NAMESPACE

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    CaseyMullen     12/09
+---------------+---------------+---------------+---------------+---------------+------*/
MemoryEnablerPtr StandaloneInstance::CreateEnabler (ClassCR ecClass) 
    {
    return MemoryEnabler::Create (ecClass, 0);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    CaseyMullen     10/09
+---------------+---------------+---------------+---------------+---------------+------*/
StandaloneInstance::StandaloneInstance (ClassCR ecClass) : m_bytesAllocated(0), m_data(NULL) 
    {
    m_enabler = MemoryEnabler::Create (ecClass, 0); // WIP_FUSION: hack! We certainly don't want to create a new one every time... and we prefer to not have to even look it up, again
    AllocateAndInitializeMemory ();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    CaseyMullen     10/09
+---------------+---------------+---------------+---------------+---------------+------*/
StandaloneInstance::StandaloneInstance (MemoryEnablerCR enabler) : 
        m_enabler(const_cast<MemoryEnablerP>(&enabler)),
        m_bytesAllocated(0), m_data(NULL) 
    {
    AllocateAndInitializeMemory ();
    }
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    CaseyMullen     09/09
+---------------+---------------+---------------+---------------+---------------+------*/        
StandaloneInstance::StandaloneInstance (MemoryEnablerCR enabler, byte * data, UInt32 size) :
        m_enabler(const_cast<MemoryEnablerP>(&enabler)),
        m_bytesAllocated(size), m_data(data) 
    {
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    CaseyMullen     09/09
+---------------+---------------+---------------+---------------+---------------+------*/        
StandaloneInstanceP StandaloneInstance::CreateWithNewMemory (MemoryEnablerCR enabler)
    {
    return new StandaloneInstance (enabler);
    }
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    CaseyMullen     09/09
+---------------+---------------+---------------+---------------+---------------+------*/        
StandaloneInstanceP StandaloneInstance::CreateFromUninitializedMemory (MemoryEnablerCR enabler, byte * data, UInt32 size)
    {
    StandaloneInstanceP instance = new StandaloneInstance (enabler, data, size);
    instance->InitializeMemory (data, size);
    return instance;
    }
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    CaseyMullen     09/09
+---------------+---------------+---------------+---------------+---------------+------*/        
StandaloneInstanceP StandaloneInstance::CreateFromInitializedMemory (MemoryEnablerCR enabler, byte * data, UInt32 size)
    {
    return new StandaloneInstance (enabler, data, size);
    }    
      
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    CaseyMullen     09/09
+---------------+---------------+---------------+---------------+---------------+------*/
MemoryEnablerCP StandaloneInstance::GetMemoryEnabler() const
    {
    return m_enabler.get();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    CaseyMullen     10/09
+---------------+---------------+---------------+---------------+---------------+------*/    
EnablerCP       StandaloneInstance::_GetEnabler() const
    {
    return m_enabler.get();
    }
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    CaseyMullen     09/09
+---------------+---------------+---------------+---------------+---------------+------*/
bool            StandaloneInstance::_IsReadOnly() const
    {
    return false;
    }
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    CaseyMullen     10/09
+---------------+---------------+---------------+---------------+---------------+------*/    
std::wstring    StandaloneInstance::_GetInstanceID() const
    {
    if (m_instanceID.size() == 0)
        {
        wchar_t id[1024];
        swprintf(id, sizeof(id)/sizeof(wchar_t), L"%s-0x%X", _GetEnabler()->GetClass()->GetName().c_str(), this);
        StandaloneInstanceP thisNotConst = const_cast<StandaloneInstanceP>(this);
        thisNotConst->m_instanceID = id;        
        }
        
    return m_instanceID;
    }
    
 
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    CaseyMullen     09/09
+---------------+---------------+---------------+---------------+---------------+------*/
bool            StandaloneInstance::IsMemoryInitialized () const
    {
    return m_data != NULL;
    }
        
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    CaseyMullen     09/09
+---------------+---------------+---------------+---------------+---------------+------*/
byte const *    StandaloneInstance::GetDataForRead () const
    {
    return m_data;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    CaseyMullen     09/09
+---------------+---------------+---------------+---------------+---------------+------*/
byte *          StandaloneInstance::GetDataForWrite () const
    {
    return m_data;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    CaseyMullen     09/09
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt       StandaloneInstance::ModifyData (UInt32 offset, void const * newData, UInt32 dataLength)
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
UInt32          StandaloneInstance::GetBytesUsed () const
    {
    if (NULL == m_data)
        return 0;

    return GetMemoryEnabler()->GetClassLayout().GetBytesUsed(m_data);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    CaseyMullen     09/09
+---------------+---------------+---------------+---------------+---------------+------*/
UInt32          StandaloneInstance::GetBytesAllocated () const
    {
    return m_bytesAllocated;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    CaseyMullen     09/09
+---------------+---------------+---------------+---------------+---------------+------*/
void            StandaloneInstance::ShrinkAllocation (UInt32 newAllocation)
    {
    DEBUG_EXPECT (false && "WIP_FUSION: needs implementation");
    } 

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    CaseyMullen     09/09
+---------------+---------------+---------------+---------------+---------------+------*/
void            StandaloneInstance::FreeAllocation ()
    {
    free (m_data); 
    m_data = NULL;
    }
            
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    CaseyMullen     10/09
+---------------+---------------+---------------+---------------+---------------+------*/
void            StandaloneInstance::AllocateBytes (UInt32 minimumBytesToAllocate)
    {
    DEBUG_EXPECT (0 == m_bytesAllocated);
    DEBUG_EXPECT (NULL == m_data);
    
    // WIP_FUSION: add performance counter
    m_data = (byte*)malloc(minimumBytesToAllocate);
    m_bytesAllocated = minimumBytesToAllocate;
    }
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    CaseyMullen     10/09
+---------------+---------------+---------------+---------------+---------------+------*/
void            StandaloneInstance::GrowAllocation (UInt32 bytesNeeded)
    {
    DEBUG_EXPECT (m_bytesAllocated > 0);
    DEBUG_EXPECT (NULL != m_data);
    // WIP_FUSION: add performance counter
            
    UInt32 newSize = 2 * (m_bytesAllocated + bytesNeeded); // Assume the growing trend will continue. The StandaloneInstanceFactory will ensure that the final instances are trimmed down to an appropriate size
    m_data = (byte*)realloc(m_data, newSize); 
    //UInt32 bytesUsed = GetBytesUsed(); x memcpy (data, m_data, bytesUsed);
// WIP_FUSION: Would this be more efficient as a realloc?    
    //free (m_data);
    //m_data = data;
    m_bytesAllocated = newSize;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    CaseyMullen     09/09
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt       StandaloneInstance::_GetValue (ValueR v, const wchar_t * propertyAccessString, UInt32 nIndices, UInt32 const * indices) const
    {
    return GetValueFromMemory (v, propertyAccessString, nIndices, indices);
    }
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    CaseyMullen     09/09
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt       StandaloneInstance::_SetValue (const wchar_t * propertyAccessString, ValueCR v, UInt32 nIndices, UInt32 const * indices)
    {
    StatusInt status = SetValueToMemory (propertyAccessString, v, nIndices, indices);

    return status;
    }
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    CaseyMullen     12/09
+---------------+---------------+---------------+---------------+---------------+------*/
StandaloneInstanceFactory::StandaloneInstanceFactory (MemoryEnablerR enabler, UInt32 slack, UInt32 initialBufferSize) : 
    m_memoryEnabler (enabler), m_nBegun(0), m_nFinished (0), m_nReallocationRequests(0),
    m_instanceUnderConstruction (NULL), m_minimumSlack (slack), m_data (NULL), m_size (0)
    {
    ClassLayout classLayout = m_memoryEnabler.GetClassLayout();
    UInt32 sizeOfFixedSection = classLayout.GetSizeOfFixedSection();
    
    if (initialBufferSize < sizeOfFixedSection)
        initialBufferSize = sizeOfFixedSection;
        
    m_size = initialBufferSize;
    }
 
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    CaseyMullen     12/09
+---------------+---------------+---------------+---------------+---------------+------*/    
BentleyStatus StandaloneInstanceFactory::BeginInstanceConstruction (StandaloneInstanceP& instance)
    {
    if (NULL != m_instanceUnderConstruction)
        {
        // WIP_FUSION: Log message "Programmer Error: Attempted to create a new Instance of %s under construction while there is already one under construction by this factory", m_enabler.GetClass().GetName().c_str());
        PRECONDITION (NULL == m_instanceUnderConstruction && "A given factory can only have one instance under construction at a time", ERROR);
        }

    ++m_nBegun;
    
    if (NULL == m_data)
        {
        m_data = (byte*)malloc (m_size);
        // WIP_FUSION: log it in debug, as a malloc
        }
        
    m_instanceUnderConstruction = StandaloneInstance::CreateFromUninitializedMemory (m_memoryEnabler, m_data, m_size);
    instance = m_instanceUnderConstruction;
    
    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    CaseyMullen     12/09
+---------------+---------------+---------------+---------------+---------------+------*/    
BentleyStatus StandaloneInstanceFactory::FinishInstance (StandaloneInstanceP& instance)
    {
    PRECONDITION (NULL != instance, ERROR);
    if (instance != m_instanceUnderConstruction)
        {
        // WIP_FUSION: Log message "Programmer Error: Attempted to finish a new Instance of %s that is not under construction by this factory!", m_enabler.GetClass().GetName().c_str());
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
                
        m_data = NULL; // The StandaloneInstance realloced our original buffer, and thereby took ownership of it
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
UInt32 StandaloneInstanceFactory::GetReallocationCount ()
    {
    return m_nReallocationRequests;
    }
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    CaseyMullen     12/09
+---------------+---------------+---------------+---------------+---------------+------*/    
UInt32 StandaloneInstanceFactory::GetBegunCount ()
    {
    return m_nBegun;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    CaseyMullen     12/09
+---------------+---------------+---------------+---------------+---------------+------*/    
UInt32 StandaloneInstanceFactory::GetFinishedCount ()
    {
    return m_nFinished;
    }        
    
END_BENTLEY_EC_NAMESPACE