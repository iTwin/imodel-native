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
* @bsimethod                                                    CaseyMullen     10/09
+---------------+---------------+---------------+---------------+---------------+------*/
StandaloneInstance::StandaloneInstance (ClassCR ecClass) : m_bytesAllocated(0), m_bytesUsed(0), m_data(NULL) 
    {
    m_enabler = MemoryEnabler::Create (ecClass); // WIP_FUSION: hack! We certainly don't want to create a new one every time... and we prefer to not have to even look it up, again
    
    wchar_t id[256];
    swprintf(id, sizeof(id)/sizeof(wchar_t), L"%s-0x%X", ecClass.GetName(), this);
    m_instanceID = id;
    
    InitializeInstanceMemory ();
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
    return m_instanceID;
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
            
    byte * data = (byte*)malloc(m_bytesAllocated + bytesNeeded); 
    memcpy (data, m_data, m_bytesAllocated);
    
    free (m_data);
    m_data = data;
    m_bytesAllocated += bytesNeeded;
    }

END_BENTLEY_EC_NAMESPACE