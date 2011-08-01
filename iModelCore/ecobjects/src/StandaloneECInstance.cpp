/*--------------------------------------------------------------------------------------+
|
|     $Source: src/StandaloneECInstance.cpp $
|
|   $Copyright: (c) 2011 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ECObjectsPch.h"

BEGIN_BENTLEY_EC_NAMESPACE

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Bill.Steinbock                  07/2011
+---------------+---------------+---------------+---------------+---------------+------*/
static IECInstanceP    getEmbeddedSupportingStructInstance (StructArrayEntry const* entry)
    {
    if (!entry)
        return NULL;

    byte const* baseAddress = (byte const*)entry;
    size_t offset = (size_t)(entry->structInstance.get());
    byte const* instanceAddress =  baseAddress + offset;

    // since the offset will put us at the vtable of the IECInstance and not to the start of the concrete object 
    // we can cast is directly to an IECInstanceP. See comments in method LoadDataIntoManagedInstance
    IECInstanceP iecInstanceP = (IECInstanceP) const_cast<byte*>(instanceAddress);
    return iecInstanceP;
    }



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

#if defined (_WIN32) // WIP_NONPORT
    UInt32 size = max (minimumBufferSize, classLayout.GetSizeOfFixedSection());
#elif defined (__unix__)
    // *** NEEDS WORK: When you stop including Windows.h, you can use this for both platforms:
    UInt32 size = std::max (minimumBufferSize, classLayout.GetSizeOfFixedSection());
#endif    
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
    size_t size = sizeof(UInt32); // number of StructArrayEntry members 
    size += sizeof(size_t); // offset to end of supporting instances

    if (m_structInstances)
        {
        if (m_isInManagedInstance)
            {
            byte const* thisAddress       = (byte const*)this;
            byte const* arrayCountAddress =  thisAddress + (size_t)m_structInstances;
            byte const* arrayAddress      =  arrayCountAddress + sizeof(UInt32) + sizeof(size_t);
            size_t      numEntries        = *(UInt32 const*)arrayCountAddress;

            size += (numEntries * sizeof (StructArrayEntry));

            StructArrayEntry* instanceArray = (StructArrayEntry*)(const_cast<byte*>(arrayAddress));

            for (size_t i = 0; i<numEntries; i++)
                {
                StructArrayEntry* entry       = &instanceArray[i];
                byte*             baseAddress = (byte*)entry;

                size_t  offset = (size_t)(entry->structInstance.get());

                byte const* instanceAddress = baseAddress + offset;

                // since the offset will put us at the vtable of the IECInstance and not to the start of the concrete object 
                // we can cast is directly to an IECInstanceP. See comments in method LoadDataIntoManagedInstance
                IECInstanceP iecInstanceP = (IECInstanceP) const_cast<byte*>(instanceAddress);

                MemoryECInstanceBase* mbInstance = iecInstanceP->GetAsMemoryECInstance();
                DEBUG_EXPECT (NULL != mbInstance);
                if (!mbInstance)
                    continue;

                size += mbInstance->GetObjectSize ();
                }
            }
        else
            {
            size += (m_structInstances->size() * sizeof (StructArrayEntry));
            for (size_t i = 0; i<m_structInstances->size(); i++)
                {
                StructArrayEntry const& entry = (*m_structInstances)[i];
        
                MemoryECInstanceBase* mbInstance = entry.structInstance->GetAsMemoryECInstance();
                DEBUG_EXPECT (NULL != mbInstance);
                if (!mbInstance)
                    continue;

                size += mbInstance->GetObjectSize ();
                }
            }
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

    // store the offset to the property data in m_data within managedBuffer
    size_t offsetToPropertyData = (size_t)((byte const* )&m_data - (byte const* )this); 
    memcpy (managedBuffer+offsetToPropertyData, &offset, sizeof(offset));

    // now copy the property data
    size_t currentBytesUsed = (size_t)m_bytesAllocated; //GetBytesUsed ();
    memcpy (managedBuffer+offset, m_data, currentBytesUsed);

    offset += currentBytesUsed;

    // store the current offset in m_structInstances - this points to the begining of the StructEntryArray data
    // number of entries
    // offset to end of supporting instances
    // struct entry[0]....struct entry[n]
    size_t offsetToStructArrayVector = (size_t)((byte const* )&m_structInstances - (byte const* )this); 
    memcpy (managedBuffer+offsetToStructArrayVector, &offset, sizeof(offset));

    // store the number of supporting struct instances
    UInt32 numArrayInstances     = m_structInstances ? (UInt32)m_structInstances->size() : 0;
    memcpy (managedBuffer+offset, &numArrayInstances, sizeof(numArrayInstances));

    offset += sizeof(numArrayInstances);
    
    size_t offsetToEndOfSupportingStructInstance = offset;   // we will update this after writing supporting instances
    offset += sizeof(size_t); 

    // go ahead and set this in case there are no supporting instances
    memcpy (managedBuffer+offsetToEndOfSupportingStructInstance, &offset, sizeof(offset));

    if (!m_structInstances || 0 == numArrayInstances)
        return offset;
    
    // all the following pointer to offset "hocus pocus" assumes that sizeof(IECInstancePtr) >= sizeof(offset) so that it is 
    // safe to stuff the offset into the pointer.
    DEBUG_EXPECT (sizeof(IECInstancePtr) >= sizeof(offset));

    // calculate offset to instance pointer in array entry - on 64 bit we can not just use sizeof(entry.structValueIdentifier)
    StructArrayEntry const& firstEntry = (*m_structInstances)[0];
    size_t offsetToInstancePtr = (byte const* )&firstEntry.structInstance - (byte const* )&firstEntry.structValueIdentifier;

    // calculate relative offset from address of entry to first struct instance
    // Note: on 64 bit  sizeof (structValueIdentifier) + sizeof (IECInstancePtr) is not equal to sizeof(StructArrayEntry)
    size_t relativeOffsetToStructInstance = (numArrayInstances * sizeof(StructArrayEntry));  
    size_t totalOffsetToInstanceEnd       = offset + relativeOffsetToStructInstance; 

    size_t iecInstanceOffset;
    size_t sizeOfStructInstance;

    // step through the array and store the StructValueIdentifier and the offset to the 
    // concrete struct array instance which is typically a StandaloneECInstance
    for (size_t i = 0; i<numArrayInstances; i++)
        {
        StructArrayEntry const& entry = (*m_structInstances)[i];

        // store the StructValueIdentifier
        memcpy (managedBuffer+offset, &entry.structValueIdentifier, sizeof(entry.structValueIdentifier));

        // At this point the offsetToStructInstance is the offset to the concrete object (most likely a StandaloneECInstance). We need to adjust this to
        // point to the vtable of a IECInstance 
        iecInstanceOffset = relativeOffsetToStructInstance + entry.structInstance->GetOffsetToIECInstance();

        // store the offset to the instance
        memcpy (managedBuffer+(offset+offsetToInstancePtr), &iecInstanceOffset, sizeof(iecInstanceOffset)); 
        offset += sizeof (StructArrayEntry);

        // store the struct instance
        MemoryECInstanceBase* mbInstance = entry.structInstance->GetAsMemoryECInstance();
        DEBUG_EXPECT (NULL != mbInstance);
        if (!mbInstance)
            continue;
        sizeOfStructInstance = mbInstance->LoadDataIntoManagedInstance (managedBuffer+totalOffsetToInstanceEnd, sizeOfManagedBuffer-totalOffsetToInstanceEnd);

        totalOffsetToInstanceEnd       += sizeOfStructInstance; 
        relativeOffsetToStructInstance += sizeOfStructInstance; 
        relativeOffsetToStructInstance -= sizeof (StructArrayEntry); 
        }

    memcpy (managedBuffer+offsetToEndOfSupportingStructInstance, &totalOffsetToInstanceEnd, sizeof(totalOffsetToInstanceEnd));

    return  totalOffsetToInstanceEnd;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    CaseyMullen     09/09
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus           MemoryECInstanceBase::_ModifyData (UInt32 offset, void const * newData, UInt32 dataLength)
    {
    PRECONDITION (NULL != m_data, ECOBJECTS_STATUS_PreconditionViolated);
    PRECONDITION (offset + dataLength <= m_bytesAllocated, ECOBJECTS_STATUS_MemoryBoundsOverrun);

    byte * dest = GetAddressOfPropertyData() + offset;
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
    
#ifdef NOT_USED
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Bill.Steinbock                  07/2011
+---------------+---------------+---------------+---------------+---------------+------*/
void    MemoryECInstanceBase::WalkSupportingStructs ()
    {
    StructArrayEntry* instanceArray = NULL;

    byte const* thisAddress       = (byte const*)this;
    byte const* arrayCountAddress =  thisAddress + (size_t)m_structInstances;
    byte const* arrayAddress      =  arrayCountAddress + sizeof(UInt32) + sizeof(size_t);
    size_t      numEntries        = *(UInt32 const*)arrayCountAddress;

    instanceArray = (StructArrayEntry*)(const_cast<byte*>(arrayAddress));

    for (size_t i = 0; i<numEntries; i++)
        {
        StructArrayEntry* entry = &instanceArray[i];

        IECInstanceP iecInstanceP =  getEmbeddedSupportingStructInstance (entry);
        MemoryECInstanceBase* mbInstance = iecInstanceP->GetAsMemoryECInstance();
        if (mbInstance)
            mbInstance->WalkSupportingStructs (); 
        }
    }
#endif

/*---------------------------------------------------------------------------------**//**
* update OffsetToEnd as we move forward, update m_structInstances offset as we back
* out of the recursion.
* @bsimethod                                    Bill.Steinbock                  07/2011
+---------------+---------------+---------------+---------------+---------------+------*/
void    MemoryECInstanceBase::UpdateStructArrayOffsets (byte const* gapAddress, bool& updateOffset, bool& updateOffsetToEnd, size_t resizeAmount)
    {
    StructArrayEntry* instanceArray = NULL;

    byte const* thisAddress       = (byte const*)this;
    byte const* arrayCountAddress =  thisAddress + (size_t)m_structInstances;
    byte const* arrayAddress      =  arrayCountAddress + sizeof(UInt32) + sizeof(size_t);
    size_t      numEntries        = *(UInt32 const*)arrayCountAddress;
    size_t      newOffset;

    if (updateOffsetToEnd)
        {
        size_t*  offsetToEndAddress = (size_t*)(const_cast<byte*>(arrayCountAddress) + sizeof(UInt32));

        *offsetToEndAddress += resizeAmount;
        }

    instanceArray = (StructArrayEntry*)(const_cast<byte*>(arrayAddress));
    for (size_t i = 0; i<numEntries; i++)
        {
        StructArrayEntry* entry       = &instanceArray[i];
        byte*             baseAddress = (byte*)entry;

        // check to see if this is a new StructArrayEntry that has not been populated yet.
        if (0 == *baseAddress)
            return;

        size_t  offset = (size_t)(entry->structInstance.get());

        if (updateOffset)
            {
            newOffset = offset + resizeAmount;
            memcpy (&entry->structInstance, &newOffset, sizeof(newOffset));
            continue;
            }

        if (baseAddress > gapAddress)
            {
            updateOffset = true;
            updateOffsetToEnd = false;
            return;   // we don't need to adjust any offsets at this level in the heirarchy
            }

        // see if any child instances grew
        byte const* instanceAddress = baseAddress + offset;

        // since the offset will put us at the vtable of the IECInstance and not to the start of the concrete object 
        // we can cast is directly to an IECInstanceP. See comments in method LoadDataIntoManagedInstance
        IECInstanceP iecInstanceP = (IECInstanceP) const_cast<byte*>(instanceAddress);

        MemoryECInstanceBase* mbInstance = iecInstanceP->GetAsMemoryECInstance();
        if (mbInstance)
            mbInstance->UpdateStructArrayOffsets (gapAddress, updateOffset, updateOffsetToEnd, resizeAmount); 
        }
    }

/*---------------------------------------------------------------------------------**//**
* This should only be called for root instance. This method builds a tree structure on
* struct array instance offsets. Once built it locates the instance that has been 
* modified. Once located, all subsequent sibling instance offsets must be updated to
* account for the growth of the property data.
* @bsimethod                                    Bill.Steinbock                  06/2011
+---------------+---------------+---------------+---------------+---------------+------*/
void    MemoryECInstanceBase::FixupStructArrayOffsets (int offsetBeyondGap, size_t resizeAmount)
    {
    if (!m_isInManagedInstance)
        return;

    byte const* gapAddress = (byte const*)this + offsetBeyondGap;
    bool   updateOffset      = false;
    bool   updateOffsetToEnd = true;    // if a supporting instance grew then we must adjust the offset used to position new supporting instances

    UpdateStructArrayOffsets (gapAddress, updateOffset, updateOffsetToEnd, resizeAmount);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    CaseyMullen     10/09
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus           MemoryECInstanceBase::_GrowAllocation (UInt32 bytesNeeded, EmbeddedInstanceCallbackP memoryCallback)
    {
    DEBUG_EXPECT (m_bytesAllocated > 0);
    DEBUG_EXPECT (NULL != m_data);
    // WIP_FUSION: add performance counter
            
    if (memoryCallback)
        {
        if (!m_isInManagedInstance)
            return ECOBJECTS_STATUS_Error;

        byte* dataAddress = const_cast<byte*>(_GetData ());

        MemoryCallbackData data;
        data.gapSize = (size_t)(bytesNeeded * 2); 
        data.dataAddress = dataAddress;
        data.gapAddress = dataAddress + m_bytesAllocated;

        // adjust offset stored in m_structInstances by the amount the propertydata grew - this way when we copy the instance data 
        // into the new buffer in managed code the offset is properly set
        Int64 saveOffsetToStructArrayVector = (Int64)m_structInstances;
        Int64 offsetToStructArrayVector = (Int64)m_structInstances + data.gapSize;
        memcpy (&m_structInstances, &offsetToStructArrayVector, sizeof(byte*));

        // update the byte allocated so it will be correct in the copied instance.
        m_bytesAllocated += (UInt32)data.gapSize;

        if (0 == memoryCallback (&data))
            {
            // hocus pocus - set offsets in this object to point to data in newly allocated object
            Int64 deltaOffset = data.newDataAddress - dataAddress;   // delta between memory locations for propertyData

            Int64 offsetToPropertyData = (Int64)m_data + deltaOffset; 
            memcpy (&m_data, &offsetToPropertyData, sizeof(byte*));

            // adjust offset stored in m_structInstances by the delta between the old and new data addresses.
            Int64 offsetToStructArrayVector = (Int64)m_structInstances + (Int64)deltaOffset;
            memcpy (&m_structInstances, &offsetToStructArrayVector, sizeof(byte*));

            return ECOBJECTS_STATUS_Success;
            }
        else
            {
            // reset values if managed callback was unsuccessful
            m_bytesAllocated -= (UInt32)data.gapSize;

            memcpy (&m_structInstances, &saveOffsetToStructArrayVector, sizeof(byte*));
            return ECOBJECTS_STATUS_Error;
            }
        }
    else
        {
        if (m_isInManagedInstance)
            return ECOBJECTS_STATUS_Error;

        UInt32 newSize = 2 * (m_bytesAllocated + bytesNeeded); // Assume the growing trend will continue.

        byte * reallocedData = (byte*)realloc(m_data, newSize);
        DEBUG_EXPECT (NULL != reallocedData);
        if (NULL == reallocedData)
            return ECOBJECTS_STATUS_UnableToAllocateMemory;
        
        m_data = reallocedData; 

        m_bytesAllocated = newSize;
        }
    
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
   
    size_t offsetToData;
    size_t offsetToArrayCount;
    size_t bytesAllocated;

    if (m_isInManagedInstance)
        {
        offsetToData = (size_t)m_data;
        offsetToArrayCount = (size_t)m_structInstances;
        bytesAllocated = (size_t)m_bytesAllocated;

        // set up memory allocation callback in case memory needs to grow to set the structValueId value
        binaryValue.SetMemoryCallback(v.GetMemoryCallback());
        }

    ECObjectsStatus status = SetPrimitiveValueToMemory (binaryValue, classLayout, propertyLayout, true, index);      
    if (status != ECOBJECTS_STATUS_Success)
        return status;
                 
    if (m_isInManagedInstance)
        {
        EmbeddedInstanceCallbackP  memoryCallback   = v.GetMemoryCallback();
        MemoryECInstanceBaseP      mbStructInstance = p->GetAsMemoryECInstance();

        if (NULL == mbStructInstance)
            return ECOBJECTS_STATUS_Error;

        size_t offsetFromConcreteToIECInstance = p->GetOffsetToIECInstance();

        size_t neededSize = mbStructInstance->GetObjectSize ();
        neededSize += sizeof (StructArrayEntry);

        //get the gap location for new StructArrayEntry and for new Instance
        byte* dataAddress = const_cast<byte*>(_GetData ());

        byte*   baseAddress            =  dataAddress - offsetToData;  // need to base it on dataAddress since call to SetPrimitiveValueToMemory may have reallocated the buffer
        byte*   arrayCountAddress      =  baseAddress + offsetToArrayCount + (m_bytesAllocated-bytesAllocated);
        UInt32* arrayCountP            = (UInt32*)arrayCountAddress;
        size_t* endOffsetP             = (size_t*)(arrayCountAddress + sizeof(UInt32));
        byte*   arrayAddress           =  arrayCountAddress + sizeof(UInt32) + sizeof(size_t);
        UInt32  arrayCount             = *arrayCountP;
        byte*   entryAddress           =  arrayAddress + sizeof(StructArrayEntry)*arrayCount;
        byte*   structInstanceAddress  =  baseAddress + *endOffsetP;

        MemoryCallbackData data;
        data.dataAddress        = dataAddress;
        data.gapSize            = sizeof (StructArrayEntry);
        data.gapAddress         = entryAddress;
        data.instanceGapSize    = mbStructInstance->GetObjectSize ();
        data.instanceGapAddress = structInstanceAddress;

        // update the array count, the end offset address, and the offset to the struct instances before inserting the 
        // gaps for the data. This way the values are properly set in the new managed byte buffer holding the resized 
        // instance. We will reset the values if the resizing is unsuccessful.

        // update the array count    
        *arrayCountP = arrayCount+1;

        // Note: endOffsetP that points just beyond the last supporting struct instance will be updated 
        //       in the memory resize callback call to FixupStructArrayOffsets

        // update existing struct array offsets.
        size_t             newOffset;
        StructArrayEntry*  entryArray   = (StructArrayEntry*)arrayAddress;
        StructArrayEntry*  currentEntry;

        for (UInt32 i = 0; i<arrayCount; i++)
            {
            currentEntry = &entryArray[i];
            newOffset    = (size_t)(currentEntry->structInstance.get());
            newOffset    += data.gapSize;
            memcpy (&currentEntry->structInstance, &newOffset, sizeof(newOffset));
            }

        if (0 == memoryCallback (&data))
            {
            // hocus pocus - set offsets in this object to point to data in newly allocated object
            Int64 deltaOffset = data.newDataAddress - dataAddress;   // delta between memory locations for propertyData

            Int64 offsetToPropertyData = (Int64)m_data + deltaOffset; 
            memcpy (&m_data, &offsetToPropertyData, sizeof(byte*));

            // adjust offset stored in m_structInstances by the delta between the old and new data addresses.
            Int64 offsetToStructArrayVector = (Int64)m_structInstances + (Int64)deltaOffset;
            memcpy (&m_structInstances, &offsetToStructArrayVector, sizeof(byte*));

            // now actually populate the gaps in the reallocated buffer with the new StructArrayEntry and the new Struct Instance
            size_t            offsetToNewEntry          = entryAddress - dataAddress;     // offset relative to m_data
            byte*             newEntryAddress           = data.newDataAddress + offsetToNewEntry;
            StructArrayEntry* newEntry                  = (StructArrayEntry*)newEntryAddress;

            size_t            deltaToNewStructInstance  = (structInstanceAddress - dataAddress) + data.gapSize; // offset relative to m_data
            size_t            newStructInstanceOffset   = (data.newDataAddress + deltaToNewStructInstance) - newEntryAddress;
            byte*             newStructInstanceLocation = newEntryAddress + newStructInstanceOffset;
                                                                         
            // set the new struct array entry structValueIdentifier into the reallocated buffer
            newEntry->structValueIdentifier = m_structValueId;

            // set the new struct array entry instance offset into the reallocated buffer
            newStructInstanceOffset += offsetFromConcreteToIECInstance;
            memcpy (&newEntry->structInstance, &newStructInstanceOffset, sizeof (newStructInstanceOffset));

            // load the new struct instance data into the reallocated buffer
            mbStructInstance->LoadDataIntoManagedInstance (newStructInstanceLocation, data.instanceGapSize);

            return ECOBJECTS_STATUS_Success;
            }
        else
            {
            // error occurred reset values
            *arrayCountP = arrayCount;

            for (UInt32 i = 0; i<arrayCount; i++)
                {
                currentEntry = &entryArray[i];
                newOffset    = (size_t)(currentEntry->structInstance.get());
                newOffset    -=  data.gapSize;
                memcpy (&currentEntry->structInstance, &newOffset, sizeof(newOffset));
                }

            return ECOBJECTS_STATUS_Error;
            }
        }
    else
        {
        if (NULL == m_structInstances)
            m_structInstances = new StructInstanceVector ();

        m_structInstances->push_back (StructArrayEntry (m_structValueId, p));
        }

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
        byte const* baseAddress = (byte const*)this;
        byte const* arrayCountAddress =  baseAddress + (size_t)m_structInstances;
        byte const* arrayAddress      =  arrayCountAddress + sizeof(UInt32) + sizeof(size_t);  // array count + offset to end of supporting struct instances

        UInt32 const* arrayCount = (UInt32 const*)arrayCountAddress;
        numEntries = *arrayCount;
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
    if (!m_isInManagedInstance)
        return entry->structInstance;

    return getEmbeddedSupportingStructInstance(entry);
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
* Get the address of the data, this is &m_data if not in embedded in a managed
* instance, otherwise m_data is an offset and the address is calculated.
* @bsimethod                                    Bill.Steinbock                  06/2011
+---------------+---------------+---------------+---------------+---------------+------*/
byte*        MemoryECInstanceBase::GetAddressOfPropertyData () const
    {
    return const_cast<byte*>(_GetData());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    CaseyMullen     09/09
+---------------+---------------+---------------+---------------+---------------+------*/
UInt32              MemoryECInstanceBase::GetBytesUsed () const
    {
    if (NULL == m_data)
        return 0;

    return GetClassLayout().CalculateBytesUsed(_GetData());
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

    InitializeMemory (GetClassLayout(), GetAddressOfPropertyData(), m_bytesAllocated);
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
* @bsimethod                                                    JoshSchifter    03/11
+---------------+---------------+---------------+---------------+---------------+------*/ 
static void     duplicateProperties (IECInstanceR target, ECValuesCollectionCR source)
    {
    for (ECValuesCollection::const_iterator it=source.begin(); it != source.end(); ++it)
        {
        ECPropertyValue const& prop = *it;
        if (prop.HasChildValues())
            {
            duplicateProperties (target, *prop.GetChildValues());
            continue;
            }

        target.SetValueUsingAccessor (prop.GetValueAccessor(), prop.GetValue());
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Dylan.Rush      11/10
+---------------+---------------+---------------+---------------+---------------+------*/ 
StandaloneECInstancePtr         StandaloneECInstance::Duplicate(IECInstanceCR instance)
    {
    ECClassCR              ecClass           = instance.GetClass();
    StandaloneECEnablerPtr standaloneEnabler = instance.GetEnablerR().GetEnablerForStructArrayMember (ecClass.GetSchema().GetName().c_str(), ecClass.GetName().c_str());
    if (standaloneEnabler.IsNull())
        return NULL;

    StandaloneECInstancePtr newInstance = standaloneEnabler->CreateInstance();

    ECValuesCollectionPtr   properties = ECValuesCollection::Create (instance);
    duplicateProperties (*newInstance, *properties);

    return newInstance;
    }
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Bill.Steinbock                  12/2010
+---------------+---------------+---------------+---------------+---------------+------*/
size_t                StandaloneECInstance::_GetObjectSize () const
    {
    size_t objectSize = sizeof(*this);
    size_t primaryInstanceDataSize = (size_t)_GetBytesAllocated(); //GetBytesUsed();
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
WString        StandaloneECInstance::_GetInstanceId() const
    {
    return m_instanceId;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    05/11
+---------------+---------------+---------------+---------------+---------------+------*/    
ECObjectsStatus StandaloneECInstance::_SetInstanceId (WCharCP instanceId)
    {
    m_instanceId = instanceId;
    return ECOBJECTS_STATUS_Success;
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
ECObjectsStatus           StandaloneECInstance::_GetValue (ECValueR v, WCharCP propertyAccessString, bool useArrayIndex, UInt32 arrayIndex) const
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
ECObjectsStatus           StandaloneECInstance::_SetValue (WCharCP propertyAccessString, ECValueCR v, bool useArrayIndex, UInt32 arrayIndex)
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
ECObjectsStatus           StandaloneECInstance::_InsertArrayElements (WCharCP propertyAccessString, UInt32 index, UInt32 size, EC::EmbeddedInstanceCallbackP memoryReallocationCallbackP)
    {
    ClassLayoutCR classLayout = GetClassLayout();
    ECObjectsStatus status = InsertNullArrayElementsAt (classLayout, propertyAccessString, index, size, memoryReallocationCallbackP);
    
    return status;
    } 
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Adam.Klatzkin                   01/2010
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus           StandaloneECInstance::_AddArrayElements (WCharCP propertyAccessString, UInt32 size, EC::EmbeddedInstanceCallbackP memoryReallocationCallbackP)
    {
    ClassLayoutCR classLayout = GetClassLayout();    
    ECObjectsStatus status = AddNullArrayElementsAt (classLayout, propertyAccessString, size, memoryReallocationCallbackP);
    
    return status;
    }        

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Adam.Klatzkin                   01/2010
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus           StandaloneECInstance::_RemoveArrayElement (WCharCP propertyAccessString, UInt32 index)
    {
    return ECOBJECTS_STATUS_OperationNotSupported;
    } 

 /*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Adam.Klatzkin                   01/2010
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus           StandaloneECInstance::_ClearArray (WCharCP propertyAccessString)
    {
    return ECOBJECTS_STATUS_OperationNotSupported;
    }                      

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    CaseyMullen     10/09
+---------------+---------------+---------------+---------------+---------------+------*/    
WString        StandaloneECInstance::_ToString (WCharCP indent) const
    {
    return InstanceDataToString (indent, GetClassLayout());
    }

///////////////////////////////////////////////////////////////////////////////////////////
//  StandaloneECEnabler
///////////////////////////////////////////////////////////////////////////////////////////
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    CaseyMullen     12/09
+---------------+---------------+---------------+---------------+---------------+------*/    
StandaloneECEnabler::StandaloneECEnabler (ECClassCR ecClass, ClassLayoutCR classLayout, IStandaloneEnablerLocaterR childECEnablerLocater, bool ownsClassLayout) :
    ECEnabler (ecClass, childECEnablerLocater),
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
StandaloneECEnablerPtr    StandaloneECEnabler::CreateEnabler (ECClassCR ecClass, ClassLayoutCR classLayout, IStandaloneEnablerLocaterR childECEnablerLocater, bool ownsClassLayout)
    {
    return new StandaloneECEnabler (ecClass, classLayout, childECEnablerLocater, ownsClassLayout);
    }
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    CaseyMullen     12/09
+---------------+---------------+---------------+---------------+---------------+------*/
WCharCP           StandaloneECEnabler::_GetName() const
    {
    return L"Bentley::EC::StandaloneECEnabler";
    }
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    05/10
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus StandaloneECEnabler::_GetPropertyIndex(UInt32& propertyIndex, WCharCP propertyAccessString) const { return GetClassLayout().GetPropertyIndex (propertyIndex, propertyAccessString); }
ECObjectsStatus StandaloneECEnabler::_GetAccessString(WCharCP& accessString, UInt32 propertyIndex) const { return GetClassLayout().GetAccessStringByIndex (accessString, propertyIndex); }
UInt32          StandaloneECEnabler::_GetPropertyCount() const    { return GetClassLayout().GetPropertyCount (); }
UInt32          StandaloneECEnabler::_GetFirstPropertyIndex (UInt32 parentIndex) const {  return GetClassLayout().GetFirstChildPropertyIndex (parentIndex); }
UInt32          StandaloneECEnabler::_GetNextPropertyIndex (UInt32 parentIndex, UInt32 inputIndex) const { return GetClassLayout().GetNextChildPropertyIndex (parentIndex, inputIndex);  }

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