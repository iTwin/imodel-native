/*--------------------------------------------------------------------------------------+
|
|     $Source: ecobjects/native/StandaloneECInstance.cpp $
|
|   $Copyright: (c) 2011 Bentley Systems, Incorporated. All rights reserved. $
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
    m_isInManagedInstance = false;
    m_structInstances = NULL;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Bill.Steinbock                  04/2010
+---------------+---------------+---------------+---------------+---------------+------*/
MemoryECInstanceBase::MemoryECInstanceBase (ClassLayoutCR classLayout, UInt32 minimumBufferSize, bool allowWritingDirectlyToInstanceMemory) :
        MemoryInstanceSupport (allowWritingDirectlyToInstanceMemory),
        m_bytesAllocated(0), m_data(NULL), m_structValueId (0)
    {
    m_isInManagedInstance = false;
    m_structInstances = NULL;

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
    if (freeExisitingData)
        {
        if (m_data)
            {
            free (m_data);
            m_data = NULL;
            }

        // allocate memory that the MemoryECInstanceBase will take ownership of 
        m_data = (byte*)malloc (size);
        if (NULL == m_data)
            {
            DEBUG_EXPECT (false && "unable to allocate memory for instance data");
            return;
            }

        memcpy (m_data, data, size);
        }
    else
        {
        m_data = data;
        }

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
* @bsimethod                                    Bill.Steinbock                  12/2010
+---------------+---------------+---------------+---------------+---------------+------*/
size_t                MemoryECInstanceBase::GetObjectSize () const
    {
    return _GetObjectSize();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Bill.Steinbock                  04/2010
+---------------+---------------+---------------+---------------+---------------+------*/
size_t          MemoryECInstanceBase::CalculateSupportingInstanceDataSize () const
    {
    if (!m_structInstances)
        return 0;

    size_t size = (m_structInstances->size() * sizeof (StructArrayEntry));
    for (size_t i = 0; i<m_structInstances->size(); i++)
        {
        StructArrayEntry const& entry = (*m_structInstances)[i];
        
        MemoryECInstanceBase* mbInstance = entry.structInstance->GetAsMemoryECInstance();
        DEBUG_EXPECT (NULL != mbInstance);
        if (!mbInstance)
            continue;

        size += mbInstance->GetObjectSize ();
        }

    return size;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Bill.Steinbock                  12/2010
+---------------+---------------+---------------+---------------+---------------+------*/
size_t          MemoryECInstanceBase::LoadDataIntoManagedInstance (byte* managedBuffer, size_t sizeOfManagedBuffer) const
    {
    // let each native instance load its own object data
    size_t offset = _LoadObjectDataIntoManagedInstance (managedBuffer);

    // set the m_isInManagedInstance value to true  in managedBuffer
    bool   isInManagedInstance         = true;
    size_t offsetToIsInManagedInstance = (size_t)((byte const* )&m_isInManagedInstance - (byte const* )this); 
    memcpy (managedBuffer+offsetToIsInManagedInstance, &isInManagedInstance, sizeof(isInManagedInstance));

    // store the allocated size of the managed buffer in m_bytesAllocated in managedBuffer
    size_t offsetToAllocatedSize = (size_t)((byte const* )&m_bytesAllocated - (byte const* )this); 
    UInt32 managedBufferSize = (UInt32)sizeOfManagedBuffer;
    memcpy (managedBuffer+offsetToAllocatedSize, &managedBufferSize, sizeof(managedBufferSize));

    // store the number of supporting struct instances in m_structValueId within managedBuffer   - this will need to change if we allow property changes in managed code
    size_t offsetToStructValueId = (size_t)((byte const* )&m_structValueId - (byte const* )this); 
    UInt32 numArrayInstances     = m_structInstances ? (UInt32)m_structInstances->size() : 0;
    memcpy (managedBuffer+offsetToStructValueId, &numArrayInstances, sizeof(numArrayInstances));

    // store the offset to the property data in m_data within managedBuffer
    size_t offsetToPropertyData = (size_t)((byte const* )&m_data - (byte const* )this); 
    memcpy (managedBuffer+offsetToPropertyData, &offset, sizeof(offset));

    // now copy the property data
    size_t currentBytesUsed = (size_t)GetBytesUsed ();
    memcpy (managedBuffer+offset, m_data, currentBytesUsed);

    offset += currentBytesUsed;

    if (!m_structInstances)
        return offset;

    // the current offset is set to the location to store the first StructArrayEntry. Store this offset in location of m_structInstances within managedBuffer
    size_t offsetToFirstStructArrayEntry = (size_t)((byte const* )&m_structInstances - (byte const* )this); 
    memcpy (managedBuffer+offsetToFirstStructArrayEntry, &offset, sizeof(offset));
    
    // all the following pointer to offset "hocus pocus" assumes that sizeof(IECInstancePtr) >= sizeof(offset) so that it is 
    // safe to stuff the offset into the pointer.
    DEBUG_EXPECT (sizeof(IECInstancePtr) >= sizeof(offset));

    // find the offset to the location in managed memory for the first struct instance
    // Note: on 64 bit  sizeof (structValueIdentifier) + sizeof (IECInstancePtr) is not equal to sizeof(StructArrayEntry)
    size_t offsetToStructInstance = offset + (numArrayInstances * sizeof(StructArrayEntry));

    // calculate offset to instance pointer in array entry - on 64 bit we can not just use sizeof(entry.structValueIdentifier)
    StructArrayEntry const& firstEntry = (*m_structInstances)[0];
    size_t offsetToInstancePtr = (byte const* )&firstEntry.structInstance - (byte const* )&firstEntry.structValueIdentifier;

    size_t iecInstanceOffset;
    size_t sizeOfStructInstance;

    // step through the array and store the StructValueIdentifier and the offset to the 
    // concrete struct array instance which is typically a StandaloneECInstance
    for (size_t i = 0; i<numArrayInstances; i++)
        {
        StructArrayEntry const& entry = (*m_structInstances)[i];

        // store the StructValueIdentifier
        memcpy (managedBuffer+offset, &entry.structValueIdentifier, sizeof(entry.structValueIdentifier));

        // At this point the offsetToStructInstance is the offset to the concrete object (most likely a StandAloneECInstance). We need to adjust this to
        // point to the vtable of a IECInstance 
        iecInstanceOffset = offsetToStructInstance + entry.structInstance->GetOffsetToIECInstance();

        // store the offset to the instance
        memcpy (managedBuffer+(offset+offsetToInstancePtr), &iecInstanceOffset, sizeof(iecInstanceOffset)); 
        offset += sizeof (StructArrayEntry);

        // store the struct instance
        MemoryECInstanceBase* mbInstance = entry.structInstance->GetAsMemoryECInstance();
        DEBUG_EXPECT (NULL != mbInstance);
        if (!mbInstance)
            continue;
        sizeOfStructInstance = mbInstance->LoadDataIntoManagedInstance (managedBuffer+offsetToStructInstance, sizeOfManagedBuffer-offsetToStructInstance);
        offsetToStructInstance += sizeOfStructInstance; 
        }

    return  offsetToStructInstance;
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
    if (m_isInManagedInstance)
        return;

    free (m_data); 
    m_data = NULL;

    if (m_structInstances)
        {
        m_structInstances->clear ();
        delete m_structInstances;
        m_structInstances = NULL;
        }
    }
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    CaseyMullen     10/09
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus           MemoryECInstanceBase::_GrowAllocation (UInt32 bytesNeeded)
    {
    DEBUG_EXPECT (!m_isInManagedInstance);
    if (m_isInManagedInstance)
        return ECOBJECTS_STATUS_Error;

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
    if (m_isInManagedInstance)
        {
        byte const* baseAddress = (byte const*)this;
        byte const* dataAddress =  baseAddress + (size_t)m_data;
        return dataAddress;
        }

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
    DEBUG_EXPECT (!m_isInManagedInstance);
    if (m_isInManagedInstance)
        return ECOBJECTS_STATUS_Error;

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
                 
    if (NULL == m_structInstances)
        m_structInstances = new StructInstanceVector ();

    m_structInstances->push_back (StructArrayEntry (m_structValueId, p));
    
    return ECOBJECTS_STATUS_Success; 
    }    
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Bill.Steinbock                  12/2010
+---------------+---------------+---------------+---------------+---------------+------*/
StructArrayEntry const* MemoryECInstanceBase::GetAddressOfStructArrayEntry (StructValueIdentifier key) const
    {
    if (!m_structInstances)
        return NULL;

    StructArrayEntry const* instanceArray = NULL;
    size_t numEntries = 0;

    if (!m_isInManagedInstance)
        {
        numEntries = m_structInstances->size();
        instanceArray = &(*m_structInstances)[0];
        }
    else
        {
        numEntries = m_structValueId;

        byte const* baseAddress = (byte const*)this;
        byte const* arrayAddress =  baseAddress + (size_t)m_structInstances;

        instanceArray = (StructArrayEntry const*)arrayAddress;
        }

    for (size_t i = 0; i<numEntries; i++)
        {
        StructArrayEntry const& entry = instanceArray[i];
        if (entry.structValueIdentifier != key)
            continue;
        return &entry;
        }

    return NULL;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Bill.Steinbock                  12/2010
+---------------+---------------+---------------+---------------+---------------+------*/
IECInstancePtr  MemoryECInstanceBase::GetStructArrayInstance (StructValueIdentifier structValueId) const
    {
    StructArrayEntry const* entry = GetAddressOfStructArrayEntry (structValueId);

    if (!entry)
        return NULL;

    if (!m_isInManagedInstance)
        return entry->structInstance;

    byte const* baseAddress = (byte const*)this;
    size_t offset = (size_t)(entry->structInstance.get());
    byte const* arrayAddress =  baseAddress + offset;

    // since the offset we put us at the vtable of the IECInstance and not to the start of the concrete object 
    // we can cast is directly to an IECInstanceP. See comments in method LoadDataIntoManagedInstance
    IECInstanceP iecInstanceP = (IECInstanceP) const_cast<byte*>(arrayAddress);
    return iecInstanceP;
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

    IECInstancePtr instancePtr = GetStructArrayInstance (structValueId);
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
    DEBUG_EXPECT (!m_isInManagedInstance);
    if (m_isInManagedInstance)
        return;

    if (m_structInstances)
        m_structInstances->clear ();

    InitializeMemory (GetClassLayout(), m_data, m_bytesAllocated);
    }
   
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Bill.Steinbock                  04/2010
+---------------+---------------+---------------+---------------+---------------+------*/
ClassLayoutCR       MemoryECInstanceBase::GetClassLayout () const
    {
    return _GetClassLayout();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Bill.Steinbock                  04/2010
+---------------+---------------+---------------+---------------+---------------+------*/
IECInstancePtr       MemoryECInstanceBase::GetAsIECInstance () const
    {
    return _GetAsIECInstance();
    }

///////////////////////////////////////////////////////////////////////////////////////////
//  StandaloneECInstance
///////////////////////////////////////////////////////////////////////////////////////////
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Bill.Steinbock                  12/2010
+---------------+---------------+---------------+---------------+---------------+------*/
size_t                StandaloneECInstance::_GetOffsetToIECInstance () const
    {
    EC::IECInstanceP iecInstanceP   = (EC::IECInstanceP)this;
    byte const* baseAddressOfIECInstance = (byte const *)iecInstanceP;
    byte const* baseAddressOfConcrete = (byte const *)this;

    return (size_t)(baseAddressOfIECInstance - baseAddressOfConcrete);
    }

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

    //ECObjectsLogger::Log()->tracev (L"StandaloneECInstance at 0x%x is being destructed. It references enabler 0x%x", this, m_sharedWipEnabler);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Dylan.Rush      11/10
+---------------+---------------+---------------+---------------+---------------+------*/ 
StandaloneECInstancePtr         StandaloneECInstance::Duplicate(IECInstanceCR instance)
    {
    ECClassCR              ecClass           = instance.GetClass();
    StandaloneECEnablerPtr standaloneEnabler = instance.GetEnablerR().ObtainStandaloneInstanceEnabler (ecClass.Schema.Name.c_str(), ecClass.Name.c_str());
    if (standaloneEnabler.IsNull())
        return NULL;

    StandaloneECInstancePtr newInstance = standaloneEnabler->CreateInstance();
    ECValueAccessorPairCollectionOptionsPtr options = ECValueAccessorPairCollectionOptions::Create (instance, false);
    ECValueAccessorPairCollection collection (*options);
    for each (ECValueAccessorPair pair in collection)
        {
        newInstance->SetValueUsingAccessor(pair.GetAccessor(), pair.GetValue());
        }

    return newInstance;
    }
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Bill.Steinbock                  12/2010
+---------------+---------------+---------------+---------------+---------------+------*/
size_t                StandaloneECInstance::_GetObjectSize () const
    {
    size_t objectSize = sizeof(*this);
    size_t primaryInstanceDataSize = (size_t)GetBytesUsed();
    size_t supportingInstanceDataSize = CalculateSupportingInstanceDataSize ();

    return objectSize+primaryInstanceDataSize+supportingInstanceDataSize;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Bill.Steinbock                  12/2010
+---------------+---------------+---------------+---------------+---------------+------*/
size_t                StandaloneECInstance::_LoadObjectDataIntoManagedInstance (byte* managedBuffer) const
    {
    size_t size = sizeof(*this);
    memcpy (managedBuffer, this, size);
    return size;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Bill.Steinbock                  12/2010
+---------------+---------------+---------------+---------------+---------------+------*/
IECInstancePtr      StandaloneECInstance::_GetAsIECInstance () const
    {
    return const_cast<StandaloneECInstance*>(this);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Bill.Steinbock                  12/2010
+---------------+---------------+---------------+---------------+---------------+------*/
MemoryECInstanceBase* StandaloneECInstance::_GetAsMemoryECInstance () const
    {
    return const_cast<StandaloneECInstance*>(this);
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
ECObjectsStatus           StandaloneECInstance::_GetValue (ECValueR v, const wchar_t * propertyAccessString, bool useArrayIndex, UInt32 arrayIndex) const
    {
    ClassLayoutCR classLayout = GetClassLayout();

    return GetValueFromMemory (classLayout, v, propertyAccessString, useArrayIndex, arrayIndex);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    05/10
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus           StandaloneECInstance::_GetValue (ECValueR v, UInt32 propertyIndex, bool useArrayIndex, UInt32 arrayIndex) const
    {
    ClassLayoutCR classLayout = GetClassLayout();

    return GetValueFromMemory (classLayout, v, propertyIndex, useArrayIndex, arrayIndex);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    CaseyMullen     09/09
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus           StandaloneECInstance::_SetValue (const wchar_t * propertyAccessString, ECValueCR v, bool useArrayIndex, UInt32 arrayIndex)
    {
    ClassLayoutCR classLayout = GetClassLayout();
    return SetValueToMemory (classLayout, propertyAccessString, v, useArrayIndex, arrayIndex);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    05/10
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus           StandaloneECInstance::_SetValue (UInt32 propertyIndex, ECValueCR v, bool useArrayIndex, UInt32 arrayIndex)
    {
    ClassLayoutCR classLayout = GetClassLayout();
    return SetValueToMemory (classLayout, propertyIndex, v, useArrayIndex, arrayIndex);
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
bwstring        StandaloneECInstance::_ToString (const wchar_t* indent) const
    {
    return InstanceDataToString (indent, GetClassLayout());
    }

///////////////////////////////////////////////////////////////////////////////////////////
//  StandaloneECEnabler
///////////////////////////////////////////////////////////////////////////////////////////
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    CaseyMullen     12/09
+---------------+---------------+---------------+---------------+---------------+------*/    
StandaloneECEnabler::StandaloneECEnabler (ECClassCR ecClass, ClassLayoutCR classLayout, IStandaloneEnablerLocatorR childECEnablerLocator, bool ownsClassLayout) :
    ECEnabler (ecClass, childECEnablerLocator),
    ClassLayoutHolder (classLayout),
    m_ownsClassLayout (ownsClassLayout)
    {
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    11/10
+---------------+---------------+---------------+---------------+---------------+------*/    
StandaloneECEnabler::~StandaloneECEnabler ()
    {
    if (m_ownsClassLayout)
        {
        ClassLayoutP classLayoutP = (ClassLayoutP)&GetClassLayout();
        delete classLayoutP;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    CaseyMullen     12/09
+---------------+---------------+---------------+---------------+---------------+------*/    
StandaloneECEnablerPtr    StandaloneECEnabler::CreateEnabler (ECClassCR ecClass, ClassLayoutCR classLayout, IStandaloneEnablerLocatorR childECEnablerLocator, bool ownsClassLayout)
    {
    return new StandaloneECEnabler (ecClass, classLayout, childECEnablerLocator, ownsClassLayout);
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
ECObjectsStatus           StandaloneECEnabler::_GetPropertyIndex(UInt32& propertyIndex, const wchar_t * propertyAccessString) const
    {
    ClassLayoutCR       classLayout = GetClassLayout();

    return classLayout.GetPropertyIndex (propertyIndex, propertyAccessString);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Dylan.Rush      12/10
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus           StandaloneECEnabler::_GetAccessString(wchar_t const *& accessString, UInt32 propertyIndex) const
    {
    ClassLayoutCR       classLayout = GetClassLayout();
    PropertyLayoutCP    propertyLayout;
    ECObjectsStatus     status = classLayout.GetPropertyLayoutByIndex (propertyLayout, propertyIndex);
    if (ECOBJECTS_STATUS_Success != status)
        return status;
    accessString = propertyLayout->GetAccessString();
    return ECOBJECTS_STATUS_Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Dylan.Rush      12/10
+---------------+---------------+---------------+---------------+---------------+------*/
UInt32                    StandaloneECEnabler::_GetPropertyCount() const
    {
    ClassLayoutCR       classLayout = GetClassLayout();

    return classLayout.GetPropertyCount ();
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
StandaloneECInstancePtr   StandaloneECEnabler::CreateInstance (UInt32 minimumBufferSize) const
    {
    return new StandaloneECInstance (*this, minimumBufferSize);
    }    
    
END_BENTLEY_EC_NAMESPACE