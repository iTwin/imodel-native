/*--------------------------------------------------------------------------------------+
|
|     $Source: src/StandaloneECInstance.cpp $
|
|   $Copyright: (c) 2012 Bentley Systems, Incorporated. All rights reserved. $
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
const UInt32 BITS_PER_FLAGSBITMASK = (sizeof(UInt32) * 8);
const UInt32 BITS_TO_SHIFT_FOR_FLAGSBITMASK = 5;            // bitToCheck >> BITS_TO_SHIFT_FOR_FLAGSBITMASK equiv. to bitToCheck / BITS_PER_FLAGSBITMASK

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Bill.Steinbock                  04/2010
+---------------+---------------+---------------+---------------+---------------+------*/
MemoryECInstanceBase::MemoryECInstanceBase (byte * data, UInt32 size, ClassLayoutCR classLayout, bool allowWritingDirectlyToInstanceMemory, MemoryECInstanceBase const* parentInstance) :
        MemoryInstanceSupport (allowWritingDirectlyToInstanceMemory),
        m_bytesAllocated(size)
    {
    m_data.address = data;
    m_isInManagedInstance = false;
    m_structInstances.vectorP = NULL;
    m_usingSharedMemory = false;
    m_parentInstance.parentInstance = parentInstance;

    InitializePerPropertyFlags (classLayout, DEFAULT_NUMBITSPERPROPERTY);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Bill.Steinbock                  04/2010
+---------------+---------------+---------------+---------------+---------------+------*/
MemoryECInstanceBase::MemoryECInstanceBase (ClassLayoutCR classLayout, UInt32 minimumBufferSize, bool allowWritingDirectlyToInstanceMemory, MemoryECInstanceBase const* parentInstance) :
        MemoryInstanceSupport (allowWritingDirectlyToInstanceMemory),
        m_bytesAllocated(0)
    {
    m_isInManagedInstance = false;
    m_structInstances.vectorP = NULL;
    m_data.address = NULL;
    m_usingSharedMemory = false;

    m_parentInstance.parentInstance = parentInstance;

#if defined (_WIN32) // WIP_NONPORT
    UInt32 size = MAX (minimumBufferSize, classLayout.GetSizeOfFixedSection());
#elif defined (__unix__)
    // *** NEEDS WORK: When you stop including Windows.h, you can use this for both platforms:
    UInt32 size = std::max (minimumBufferSize, classLayout.GetSizeOfFixedSection());
#endif    
    m_data.address = (byte*)malloc (size);
    m_bytesAllocated = size;

    InitializeMemory (classLayout, m_data.address, m_bytesAllocated);
    
    InitializePerPropertyFlags (classLayout, DEFAULT_NUMBITSPERPROPERTY);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Bill.Steinbock                  11/2011
+---------------+---------------+---------------+---------------+---------------+------*/
void    MemoryECInstanceBase::SetUsingSharedMemory () 
    {
    m_usingSharedMemory = true;
    }
	
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Bill.Steinbock                  09/2011
+---------------+---------------+---------------+---------------+---------------+------*/
void    MemoryECInstanceBase::InitializePerPropertyFlags (ClassLayoutCR classLayout, UInt8 numBitsPerPropertyFlag)
    {
    // max numBitsPerPropertyFlag = 16
    m_perPropertyFlagsHolder.numBitsPerProperty = (numBitsPerPropertyFlag > 16) ? 16 : numBitsPerPropertyFlag;

    UInt32 numProperties = classLayout.GetPropertyCount ();
    if (numBitsPerPropertyFlag > 0 && numProperties > 0)
        {
        m_perPropertyFlagsHolder.numPerPropertyFlagsEntries = ((numProperties * m_perPropertyFlagsHolder.numBitsPerProperty)+BITS_PER_FLAGSBITMASK) / BITS_PER_FLAGSBITMASK;
        m_perPropertyFlagsHolder.perPropertyFlags.address = (UInt32*)calloc(m_perPropertyFlagsHolder.numPerPropertyFlagsEntries, sizeof(UInt32));
        }
    else
        {
        m_perPropertyFlagsHolder.numPerPropertyFlagsEntries = 0;
        m_perPropertyFlagsHolder.perPropertyFlags.address = NULL;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Bill.Steinbock                  09/2011
+---------------+---------------+---------------+---------------+---------------+------*/
UInt8                    MemoryECInstanceBase::GetNumBitsInPerPropertyFlags ()
    {
    return m_perPropertyFlagsHolder.numBitsPerProperty;
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Bill.Steinbock                  09/2011
+---------------+---------------+---------------+---------------+---------------+------*/
UInt32     MemoryECInstanceBase::GetPerPropertyFlagsSize () const
    {
    return     m_perPropertyFlagsHolder.numPerPropertyFlagsEntries;
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
        if (m_data.address)
            {
            free (m_data.address);
            m_data.address = NULL;
            }

        // allocate memory that the MemoryECInstanceBase will take ownership of 
        m_data.address = (byte*)malloc (size);
        if (NULL == m_data.address)
            {
            DEBUG_EXPECT (false && "unable to allocate memory for instance data");
            return;
            }

        memcpy (m_data.address, data, size);
        }
    else
        {
        m_data.address = data;
        }

    m_bytesAllocated = size;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    CaseyMullen     09/09
+---------------+---------------+---------------+---------------+---------------+------*/
bool                MemoryECInstanceBase::_IsMemoryInitialized () const
    {
    return m_data.address != NULL;
    }
     
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Bill.Steinbock                  12/2010
+---------------+---------------+---------------+---------------+---------------+------*/
size_t                MemoryECInstanceBase::GetObjectSize () const
    {
    return _GetObjectSize();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Paul.Connelly                   10/11
+---------------+---------------+---------------+---------------+---------------+------*/
size_t                MemoryECInstanceBase::GetBaseObjectSize () const
    {
    return _GetBaseObjectSize();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Bill.Steinbock                  09/2011
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus          MemoryECInstanceBase::IsPerPropertyBitSet (bool& isSet, UInt8 bitIndex, UInt32 propertyIndex) const
    {
    if (bitIndex >= m_perPropertyFlagsHolder.numBitsPerProperty)
        return ECOBJECTS_STATUS_InvalidIndexForPerPropertyFlag;

    if (propertyIndex >= GetClassLayout().GetPropertyCount ())
        return ECOBJECTS_STATUS_IndexOutOfRange;

    UInt32* addressOfPerPropertyFlags = m_perPropertyFlagsHolder.perPropertyFlags.address;

    if (!addressOfPerPropertyFlags)
        return ECOBJECTS_STATUS_Error;

    if (m_isInManagedInstance)
        {
        byte const* thisAddress   = (byte const*)this;
        addressOfPerPropertyFlags =  (UInt32*) (thisAddress + m_perPropertyFlagsHolder.perPropertyFlags.offset);
        }

    UInt32 bitToCheck = (propertyIndex * m_perPropertyFlagsHolder.numBitsPerProperty) + bitIndex;
    UInt32 offset = bitToCheck >> BITS_TO_SHIFT_FOR_FLAGSBITMASK;
    UInt32 bit    = bitToCheck % BITS_PER_FLAGSBITMASK;
    UInt32 bitValue = 1 << bit;

    isSet = (bitValue == (addressOfPerPropertyFlags[offset] & bitValue));

    return ECOBJECTS_STATUS_Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Paul.Connelly                   10/11
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus         MemoryECInstanceBase::IsAnyPerPropertyBitSet (bool& isSet, UInt8 bitIndex) const
    {
    static const ::UInt32     s_maskFor2Bits[2] = { 0x55555555, 0xAAAAAAAA };
    
    if (2 >= m_perPropertyFlagsHolder.numBitsPerProperty)
        {
        DEBUG_FAIL ("PerPropertyFlagsHolder presently supports maximum 2 bits");
        return ECOBJECTS_STATUS_Error;
        }
    else if (bitIndex >= m_perPropertyFlagsHolder.numBitsPerProperty)
        return ECOBJECTS_STATUS_InvalidIndexForPerPropertyFlag;

    ::UInt32 mask = m_perPropertyFlagsHolder.numBitsPerProperty == 2 ? s_maskFor2Bits[(int)bitIndex] : 0xFFFFFFFF;

    UInt32* addressOfPerPropertyFlags = m_perPropertyFlagsHolder.perPropertyFlags.address;
    if (NULL == addressOfPerPropertyFlags)
        return ECOBJECTS_STATUS_Error;

    if (m_isInManagedInstance)
        {
        byte const* thisAddress = (byte const*)this;
        addressOfPerPropertyFlags = (UInt32*)(thisAddress + m_perPropertyFlagsHolder.perPropertyFlags.offset);
        }

    isSet = false;
    for (UInt32 i = 0; i < m_perPropertyFlagsHolder.numPerPropertyFlagsEntries; i++)
        {
        if (0 != (mask & (addressOfPerPropertyFlags[i])))
            {
            isSet = true;
            break;
            }
        }

    return ECOBJECTS_STATUS_Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Bill.Steinbock                  09/2011
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus          MemoryECInstanceBase::SetPerPropertyBit (UInt8 bitIndex, UInt32 propertyIndex, bool setBit)
    {
    if (bitIndex >= m_perPropertyFlagsHolder.numBitsPerProperty)
        return ECOBJECTS_STATUS_InvalidIndexForPerPropertyFlag;

    if (propertyIndex >= GetClassLayout().GetPropertyCount ())
        return ECOBJECTS_STATUS_IndexOutOfRange;

    UInt32* addressOfPerPropertyFlags = m_perPropertyFlagsHolder.perPropertyFlags.address;

    if (!addressOfPerPropertyFlags)
        return ECOBJECTS_STATUS_InvalidIndexForPerPropertyFlag;

    if (m_isInManagedInstance)
        {
        byte const* thisAddress   = (byte const*)this;
        addressOfPerPropertyFlags =  (UInt32*) (thisAddress + m_perPropertyFlagsHolder.perPropertyFlags.offset);
        }

    UInt32 bitToSet = (propertyIndex * m_perPropertyFlagsHolder.numBitsPerProperty) + bitIndex;
    UInt32 offset = bitToSet >> BITS_TO_SHIFT_FOR_FLAGSBITMASK;
    UInt32 bit    = bitToSet % BITS_PER_FLAGSBITMASK;
    UInt32 bitValue = 1 << bit;

    if (setBit)
        addressOfPerPropertyFlags[offset] |= bitValue;
    else
        addressOfPerPropertyFlags[offset] &= ~bitValue;

    return ECOBJECTS_STATUS_Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Bill.Steinbock                  09/2011
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus    MemoryECInstanceBase::SetBitForAllProperties (UInt8 bitIndex, bool setBit)
    {
    ECObjectsStatus status;
    int numProperties = GetClassLayout().GetPropertyCount ();

    for (UInt32 propertyIndex=0; propertyIndex<(UInt32)numProperties; propertyIndex++)
        {
        status = SetPerPropertyBit (bitIndex, propertyIndex, setBit);
        if (ECOBJECTS_STATUS_Success != status)
            return status;
        }
    
    return ECOBJECTS_STATUS_Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Bill.Steinbock                  09/2011
+---------------+---------------+---------------+---------------+---------------+------*/
void    MemoryECInstanceBase::ClearAllPerPropertyFlags ()
    {
    UInt32* addressOfPerPropertyFlags = m_perPropertyFlagsHolder.perPropertyFlags.address;
    if (!addressOfPerPropertyFlags)
        return;

    if (m_isInManagedInstance)
        {
        byte const* thisAddress   = (byte const*)this;
        addressOfPerPropertyFlags =  (UInt32*) (thisAddress + m_perPropertyFlagsHolder.perPropertyFlags.offset);
        }

    if (addressOfPerPropertyFlags)
        memset (addressOfPerPropertyFlags, 0, m_perPropertyFlagsHolder.numPerPropertyFlagsEntries*sizeof(UInt32));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Bill.Steinbock                  04/2010
+---------------+---------------+---------------+---------------+---------------+------*/
size_t          MemoryECInstanceBase::CalculateSupportingInstanceDataSize () const
    {
    size_t size = sizeof(UInt32); // number of StructArrayEntry members 
    size += sizeof(size_t); // offset to end of supporting instances

    if (m_structInstances.vectorP)
        {
        if (m_isInManagedInstance)
            {
            byte const* thisAddress       = (byte const*)this;
            byte const* arrayCountAddress =  thisAddress + m_structInstances.offset;
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
            size += (m_structInstances.vectorP->size() * sizeof (StructArrayEntry));
            for (size_t i = 0; i<m_structInstances.vectorP->size(); i++)
                {
                StructArrayEntry const& entry = (*m_structInstances.vectorP)[i];
        
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
    size_t offset = (size_t)_LoadObjectDataIntoManagedInstance (managedBuffer);
    DEBUG_EXPECT (_GetBaseObjectSize () == offset);

    Int64 globalOffset;

    // set the m_isInManagedInstance value to true  in managedBuffer
    bool   isInManagedInstance         = true;
    size_t offsetToIsInManagedInstance = (size_t)((byte const* )&m_isInManagedInstance - (byte const* )this); 
    memcpy (managedBuffer+offsetToIsInManagedInstance, &isInManagedInstance, sizeof(isInManagedInstance));

    // store the offset to the perPropertyFlags in m_perPropertyFlags.address within managedBuffer
    size_t offsetToPropertyFlags = (size_t)((byte const* )&m_perPropertyFlagsHolder.perPropertyFlags.address - (byte const* )this); 
    globalOffset = (Int64)offset;
    memcpy (managedBuffer+offsetToPropertyFlags, &globalOffset, sizeof(globalOffset));

    // now copy the perPropertyFlags data
    size_t currentBytesUsed = m_perPropertyFlagsHolder.numPerPropertyFlagsEntries * sizeof(UInt32);
    memcpy (managedBuffer+offset, m_perPropertyFlagsHolder.perPropertyFlags.address, currentBytesUsed);

    offset += currentBytesUsed;

    // store the offset to the property data in m_data.address within managedBuffer
    size_t offsetToPropertyData = (size_t)((byte const* )&m_data.address - (byte const* )this); 
    globalOffset = (Int64)offset;
    memcpy (managedBuffer+offsetToPropertyData, &globalOffset, sizeof(globalOffset));

    // now copy the property data
    currentBytesUsed = (size_t)m_bytesAllocated;
    memcpy (managedBuffer+offset, m_data.address, currentBytesUsed);

    offset += currentBytesUsed;

    // ensure parentInstance is zeroed out - if we are loading this instance into a parent its parent is responsible for setting the offset
    Int64 parentOffset = 0;
    Int64 offsetToParentInstance = (Int64)((byte const*)&m_parentInstance.offset - (byte const*)this);
    memcpy (managedBuffer + offsetToParentInstance, &parentOffset, sizeof (parentOffset));

    // store the current offset in m_structInstances.vectorP - this points to the begining of the StructEntryArray data
    // number of entries
    // offset to end of supporting instances
    // struct entry[0]....struct entry[n]
    size_t offsetToStructArrayVector = (size_t)((byte const* )&m_structInstances.vectorP - (byte const* )this); 
    globalOffset = (Int64)offset;
    memcpy (managedBuffer+offsetToStructArrayVector, &globalOffset, sizeof(globalOffset));

    // store the number of supporting struct instances
    UInt32 numArrayInstances     = m_structInstances.vectorP ? (UInt32)m_structInstances.vectorP->size() : 0;
    memcpy (managedBuffer+offset, &numArrayInstances, sizeof(numArrayInstances));

    offset += sizeof(numArrayInstances);
    
    Int64 offsetToEndOfSupportingStructInstance = offset;   // we will update this after writing supporting instances
    offset += sizeof(size_t); 

    // go ahead and set this in case there are no supporting instances
    memcpy (managedBuffer+offsetToEndOfSupportingStructInstance, &offset, sizeof(offset));

    if (!m_structInstances.vectorP || 0 == numArrayInstances)
        return offset;
    
    // all the following pointer to offset "hocus pocus" assumes that sizeof(IECInstancePtr) >= sizeof(offset) so that it is 
    // safe to stuff the offset into the pointer.
    DEBUG_EXPECT (sizeof(IECInstancePtr) >= sizeof(offset));

    // calculate offset to instance pointer in array entry - on 64 bit we can not just use sizeof(entry.structValueIdentifier)
    StructArrayEntry const& firstEntry = (*m_structInstances.vectorP)[0];
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
        StructArrayEntry const& entry = (*m_structInstances.vectorP)[i];

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
    PRECONDITION (NULL != m_data.address, ECOBJECTS_STATUS_PreconditionViolated);
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

    if (!m_usingSharedMemory)
        {
        if (m_data.address)
            free (m_data.address); 

        if (m_perPropertyFlagsHolder.perPropertyFlags.address)
            {
            free (m_perPropertyFlagsHolder.perPropertyFlags.address); 
            m_perPropertyFlagsHolder.perPropertyFlags.address = NULL;
            }
        }

    m_data.address = NULL;

    if (m_structInstances.vectorP)
        {
        m_structInstances.vectorP->clear ();
        delete m_structInstances.vectorP;
        m_structInstances.vectorP = NULL;
        }
    }
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Bill.Steinbock                  07/2011
+---------------+---------------+---------------+---------------+---------------+------*/
void    MemoryECInstanceBase::WalkSupportingStructs (WStringR completeString, WCharCP prefix) const
    {
    if (!m_isInManagedInstance)
        return;

    StructArrayEntry* instanceArray = NULL;

    byte const* thisAddress       = (byte const*)this;
    byte const* arrayCountAddress =  thisAddress + m_structInstances.offset;
    byte const* arrayAddress      =  arrayCountAddress + sizeof(UInt32) + sizeof(size_t);
    size_t      numEntries        = *(UInt32 const*)arrayCountAddress;

    instanceArray = (StructArrayEntry*)(const_cast<byte*>(arrayAddress));
    size_t* endOffsetP            = (size_t*)(arrayCountAddress + sizeof(UInt32));

    wchar_t tmpDataString[512];
    BeStringUtilities::Snwprintf (tmpDataString, _countof(tmpDataString), L"%s0x%x Parent instance Address \n%s0x%x Struct Array Count  [%ld] \n%s0x%x Offset to End       [%ld] => 0x%x\n", 
                                                                           prefix, thisAddress, 
                                                                           prefix, arrayCountAddress, numEntries,
                                                                           prefix, endOffsetP, *endOffsetP, thisAddress+*endOffsetP);
    completeString.append (tmpDataString);

    if (0 == numEntries)
        return;

    for (size_t i = 0; i<numEntries; i++)
        {
        StructArrayEntry* entry = &instanceArray[i];

        IECInstanceP iecInstanceP =  getEmbeddedSupportingStructInstance (entry);
        MemoryECInstanceBase* mbInstance = iecInstanceP->GetAsMemoryECInstance();
        if (mbInstance)
            {
            BeStringUtilities::Snwprintf (tmpDataString, _countof(tmpDataString), L"%s    ================== StructArrayEntry [%ld] ==================\n", prefix, i);
            completeString.append (tmpDataString);

            size_t offset = (size_t)(entry->structInstance.get());
            
            BeStringUtilities::Snwprintf (tmpDataString, _countof(tmpDataString), L"%s    0x%x structValueIdentifier [%ld]\n", 
                                                                                    prefix, entry, entry->structValueIdentifier);
            completeString.append (tmpDataString);

            BeStringUtilities::Snwprintf (tmpDataString, _countof(tmpDataString), L"%s    0x%x instanceOffset=%ld  => mbInstance Address = 0x%x  class=%s\n", 
                                                                                   prefix, entry->structInstance.get(), offset, mbInstance, iecInstanceP->GetClass().GetName().c_str());
            completeString.append (tmpDataString);

            WString nextPreFix = prefix;
            nextPreFix.append (L"        ");

            mbInstance->WalkSupportingStructs (completeString, nextPreFix.c_str()); 

            BeStringUtilities::Snwprintf (tmpDataString, _countof(tmpDataString), L"%s    ================== End StructArrayEntry [%ld] ==================\n", prefix, i);
            completeString.append (tmpDataString);
            }
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Bill.Steinbock                  07/2011
+---------------+---------------+---------------+---------------+---------------+------*/
void    MemoryECInstanceBase::UpdateStructArrayOffsets (byte const* gapAddress, size_t resizeAmount)
    {
    StructArrayEntry* instanceArray = NULL;

    byte*   thisAddress       = (byte*)this;
    byte*   arrayCountAddress =  thisAddress + m_structInstances.offset;
    byte*   arrayAddress      =  arrayCountAddress + sizeof(UInt32) + sizeof(size_t);
    size_t* offsetToEndAddress = (size_t*)(arrayCountAddress + sizeof(UInt32));

    thisAddress = GetAddressOfInstanceFromAddressOfPropertyData ();
    byte *  endAddress = thisAddress + *offsetToEndAddress;

    // if this instance or supporting struct instance have changed size then update the offset to the end of the instance.
    if (gapAddress > thisAddress && gapAddress <= endAddress)
        *offsetToEndAddress += resizeAmount;
    else
        return;   // no changes within this instance

    // see if there are supporting struct instances to process
    size_t  numEntries  = *(UInt32*)arrayCountAddress;
    if (0 == numEntries)
        return;

    size_t  newOffset;
    bool    updateOffset       = false;

    // calculate address of parent instance so we can update offsets
    Int64 parentBaseAddress = (Int64) GetAddressOfInstanceFromAddressOfPropertyData ();

    // find the entry that has changed size and update any subsequent StructArrayEntry offsets
    instanceArray = (StructArrayEntry*)(const_cast<byte*>(arrayAddress));
    for (size_t i = 0; i<numEntries; i++)
        {
        StructArrayEntry* entry                 = &instanceArray[i];
        byte*             baseAddress           = (byte*)entry;

        if (0 != *baseAddress)
            {
            // update the instance's offset to parent instance
            byte* currentInstanceAddress = ((byte*)entry) + (size_t)(entry->structInstance.get ());
            IECInstanceP iecInstanceP = (IECInstanceP) const_cast<byte*>(currentInstanceAddress);
            MemoryECInstanceBase* mbInstance = iecInstanceP->GetAsMemoryECInstance ();
            if (NULL != mbInstance && 0 != mbInstance->m_parentInstance.offset)
                mbInstance->m_parentInstance.offset = parentBaseAddress - (Int64)mbInstance;
            }

        if (updateOffset)
            {
            size_t   currentInstanceOffset = (size_t)(entry->structInstance.get());

            newOffset = currentInstanceOffset + resizeAmount;
            memcpy (&entry->structInstance, &newOffset, sizeof(newOffset));
            
            continue;
            }

        if (0 == *baseAddress) // check to see if this is a new StructArrayEntry that has not been populated yet.
            {
            updateOffset = true;
            continue;
            }
        else
            {
            size_t  currentInstanceOffset = (size_t)(entry->structInstance.get());
            byte*   currentInstanceAddress = ((byte*)entry) + currentInstanceOffset;
            byte*   nextInstanceAddress    = endAddress;
            if (i < (numEntries - 1))
                {
                StructArrayEntry*   nextEntry          = &instanceArray[i+1];
                size_t              nextInstanceOffset = (size_t)(nextEntry->structInstance.get());
                nextInstanceAddress                    = (byte*)nextEntry + nextInstanceOffset;
                }

            if (gapAddress > currentInstanceAddress && gapAddress <= nextInstanceAddress)
                {
                updateOffset = true;

                // The change in size may be in a supporting structArrayInstance

                // since the offset will put us at the vtable of the IECInstance and not to the start of the concrete object 
                // we can cast is directly to an IECInstanceP. See comments in method LoadDataIntoManagedInstance
                IECInstanceP iecInstanceP = (IECInstanceP) const_cast<byte*>(currentInstanceAddress);

                MemoryECInstanceBase* mbInstance = iecInstanceP->GetAsMemoryECInstance();
                if (mbInstance)
                    mbInstance->UpdateStructArrayOffsets (gapAddress, resizeAmount); 

                continue;
                }
            }
        }
    }

/*---------------------------------------------------------------------------------**//**
* This is only used when removing support struct instances from native instances
* embedded in managed instances
* @bsimethod                                    Bill.Steinbock                  07/2011
+---------------+---------------+---------------+---------------+---------------+------*/
void    MemoryECInstanceBase::RemoveGapFromStructArrayEntries (byte const* gapAddress, size_t resizeAmount)
    {
    StructArrayEntry* instanceArray = NULL;

    byte*   thisAddress       = (byte*)this;
    byte*   arrayCountAddress =  thisAddress + m_structInstances.offset;
    byte*   arrayAddress      =  arrayCountAddress + sizeof(UInt32) + sizeof(size_t);
    size_t* offsetToEndAddress = (size_t*)(arrayCountAddress + sizeof(UInt32));
    byte*   endAddress        =  thisAddress + *offsetToEndAddress;
    size_t  entrySize         =  sizeof(StructArrayEntry);
    size_t  instanceSize      =  resizeAmount - entrySize;

    // if this instance or supporting struct instance have changed size then update the offset to the end of the instance.
    if (gapAddress > thisAddress && gapAddress <= endAddress)
        *offsetToEndAddress -= resizeAmount;
    else
        return;   // no changes within this instance

    // see if there are supporting struct instances to process
    size_t  numEntries  = *(UInt32*)arrayCountAddress;
    if (0 == numEntries)
        return;

    size_t  newOffset;
    bool    updateOffset       = false;

    // calculate address of parent instance so we can update offsets
    Int64 parentBaseAddress = (Int64) GetAddressOfInstanceFromAddressOfPropertyData ();

    // find the entry that has changed size and update any subsequent StructArrayEntry offsets
    instanceArray = (StructArrayEntry*)(const_cast<byte*>(arrayAddress));
    for (size_t i = 0; i<numEntries; i++)
        {
        StructArrayEntry* entry                 = &instanceArray[i];
        byte*             baseAddress           = (byte*)entry;
        size_t            currentInstanceOffset = (size_t)(entry->structInstance.get());

        if (0 != *baseAddress)
            {
            // update the instance's offset to parent instance
            byte* currentInstanceAddress = ((byte*)entry) + (size_t)(entry->structInstance.get ());
            IECInstanceP iecInstanceP = (IECInstanceP) const_cast<byte*>(currentInstanceAddress);
            MemoryECInstanceBase* mbInstance = iecInstanceP->GetAsMemoryECInstance ();
            if (NULL != mbInstance && 0 != mbInstance->m_parentInstance.offset)
                mbInstance->m_parentInstance.offset = parentBaseAddress - (Int64)mbInstance;
            }

        if (updateOffset)
            {
            newOffset = currentInstanceOffset - resizeAmount;
            memcpy (&entry->structInstance, &newOffset, sizeof(newOffset));

            continue;
            }

        if (baseAddress == gapAddress)  // found removed entry
            {
            // update all offset before this one by the size of the removed StructArrayEntry
            for (size_t j = 0; j < i; j++)
                {
                StructArrayEntry* entry                 = &instanceArray[j];
                size_t            currentInstanceOffset = (size_t)(entry->structInstance.get());

                newOffset = currentInstanceOffset - entrySize;
                memcpy (&entry->structInstance, &newOffset, sizeof(newOffset));
                }

            // update this and all subsequent sibling offsets by the size of the instance that was removed
            resizeAmount = instanceSize;

            updateOffset = true;
            newOffset = currentInstanceOffset - resizeAmount;
            memcpy (&entry->structInstance, &newOffset, sizeof(newOffset));
            continue;
            }

        // if we get here we are still looking for the instance that was resized
        byte*   currentInstanceAddress = (byte*)entry + currentInstanceOffset;
        byte*   nextInstanceAddress    = endAddress;
        if (i < (numEntries - 1))
            {
            StructArrayEntry*   nextEntry          = &instanceArray[i+1];
            size_t              nextInstanceOffset = (size_t)(nextEntry->structInstance.get());
            nextInstanceAddress                    = (byte*)nextEntry + nextInstanceOffset;
            }

        if (gapAddress > currentInstanceAddress && gapAddress <= nextInstanceAddress)
            {
            updateOffset = true;

            // since the offset will put us at the vtable of the IECInstance and not to the start of the concrete object 
            // we can cast is directly to an IECInstanceP. See comments in method LoadDataIntoManagedInstance
            IECInstanceP iecInstanceP = (IECInstanceP)currentInstanceAddress;

            MemoryECInstanceBase* mbInstance = iecInstanceP->GetAsMemoryECInstance();
            if (mbInstance)
                mbInstance->RemoveGapFromStructArrayEntries (gapAddress, resizeAmount); 

            continue;
            }
        }

    // if we get here we must have deleted the last struct array entry so just update the offsets by the size of StructArrayEntry
    if (!updateOffset)
        {
        for (size_t j = 0; j < numEntries; j++)
            {
            StructArrayEntry* entry                 = &instanceArray[j];
            size_t            currentInstanceOffset = (size_t)(entry->structInstance.get());

            newOffset = currentInstanceOffset - entrySize;
            memcpy (&entry->structInstance, &newOffset, sizeof(newOffset));
            }
        }
    }

/*---------------------------------------------------------------------------------**//**
* This should only be called for root instance. This method locates the instance that has been 
* modified. Once located, all subsequent sibling instance offsets must be updated to
* account for the growth/reduction of the property data.
* @bsimethod                                    Bill.Steinbock                  06/2011
+---------------+---------------+---------------+---------------+---------------+------*/
void    MemoryECInstanceBase::FixupStructArrayOffsets (int offsetBeyondGap, size_t resizeAmount, bool removingGaps)
    {
    if (!m_isInManagedInstance)
        return;

    byte const* gapAddress = (byte const*)this + offsetBeyondGap;

    if (!removingGaps)
        UpdateStructArrayOffsets (gapAddress, resizeAmount);
    else 
        RemoveGapFromStructArrayEntries (gapAddress, resizeAmount);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    CaseyMullen     10/09
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus           MemoryECInstanceBase::_GrowAllocation (UInt32 bytesNeeded, EmbeddedInstanceCallbackP memoryCallback)
    {
    DEBUG_EXPECT (m_bytesAllocated > 0);
    DEBUG_EXPECT (NULL != m_data.address);
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

        // adjust offset stored in m_structInstances.vectorP by the amount the propertydata grew - this way when we copy the instance data 
        // into the new buffer in managed code the offset is properly set
        Int64 saveOffsetToStructArrayVector = m_structInstances.offset;
        Int64 offsetToStructArrayVector = m_structInstances.offset + data.gapSize;
        m_structInstances.offset = offsetToStructArrayVector;

        // Note the above only works if the managed buffer has not already been reallocated; otherwise it will copy from the reallocated instance, not 'this'
        // so we must also update struct instances offset in reallocated instance if one exists...
        MemoryECInstanceBase* currentInstance = (MemoryECInstanceBase*) GetAddressOfInstanceFromAddressOfPropertyData ();
        if (this != currentInstance)
            {
            Int64 deltaFromDataToStructs = offsetToStructArrayVector - m_data.offset;
            Int64 deltaFromBaseToData = _GetBaseObjectSize () + m_perPropertyFlagsHolder.numPerPropertyFlagsEntries*sizeof(UInt32);
            currentInstance->m_structInstances.offset = deltaFromBaseToData + deltaFromDataToStructs;

            // for the same reason, must update m_bytesAllocated before we invoke the callback in order for it to be correctly copied
            currentInstance->m_bytesAllocated = m_bytesAllocated + (UInt32)data.gapSize;
            }

        // update the byte allocated so it will be correct in the copied instance.
        m_bytesAllocated += (UInt32)data.gapSize;

        // the callback will make a copy of the original instance and insert gaps where specified
        if (0 == memoryCallback (&data))
            {
            // hocus pocus - set offsets in this object to point to data in newly allocated object
            Int64 deltaOffset = data.newDataAddress - dataAddress;   // delta between memory locations for propertyData

            Int64 offsetToPropertyData = m_data.offset + deltaOffset; 
            m_data.offset = offsetToPropertyData;

            // adjust offset stored in m_structInstances.vectorP by the delta between the old and new data addresses.
            offsetToStructArrayVector = m_structInstances.offset + deltaOffset;
            m_structInstances.offset = offsetToStructArrayVector;

            // adjust offset to perPropertyFlags by delta, otherwise changes to flags after reallocation will not affect the resized instance
            m_perPropertyFlagsHolder.perPropertyFlags.offset += deltaOffset;

            // adjust offset to perPropertyFlags by delta, otherwise changes to flags after reallocation will not affect the resized instance
            m_perPropertyFlagsHolder.perPropertyFlags.offset += deltaOffset;

            return ECOBJECTS_STATUS_Success;
            }
        else
            {
            // reset values if managed callback was unsuccessful
            m_bytesAllocated -= (UInt32)data.gapSize;

            m_structInstances.offset = saveOffsetToStructArrayVector;
            return ECOBJECTS_STATUS_Error;
            }
        }
    else
        {
        if (m_isInManagedInstance)
            return ECOBJECTS_STATUS_Error;

        UInt32 newSize = 2 * (m_bytesAllocated + bytesNeeded); // Assume the growing trend will continue.

        byte * reallocedData = (byte*)realloc(m_data.address, newSize);
        DEBUG_EXPECT (NULL != reallocedData);
        if (NULL == reallocedData)
            return ECOBJECTS_STATUS_UnableToAllocateMemory;
        
        m_data.address = reallocedData; 

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
        byte const* dataAddress =  baseAddress + (size_t)m_data.offset;     // m_data.address need to be signed
        return dataAddress;
        }

    return m_data.address;
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
    ECValue structValueIdValue (PRIMITIVETYPE_Integer);
    StructValueIdentifier structValueId = GetMaxStructValueIdentifier () + 1;

    DEBUG_EXPECT (NULL == GetAddressOfStructArrayEntry (structValueId));

    if (v.IsNull())
        {
        p = NULL;
        structValueIdValue.SetInteger (0);
        }
    else
        {
        p = v.GetStruct();    
        structValueIdValue.SetInteger (structValueId);
        }
   
    size_t offsetToData;
    Int64 offsetToArrayCount;
    size_t bytesAllocated;

    if (m_isInManagedInstance)
        {
        offsetToData = (size_t)m_data.offset;
        offsetToArrayCount = m_structInstances.offset;
        bytesAllocated = (size_t)m_bytesAllocated;

        // set up memory allocation callback in case memory needs to grow to set the structValueId value
        structValueIdValue.SetMemoryCallback(v.GetMemoryCallback());
        }

    ECObjectsStatus status = SetPrimitiveValueToMemory (structValueIdValue, classLayout, propertyLayout, true, index);      
    if (status != ECOBJECTS_STATUS_Success)
        return status;
                 
    if (m_isInManagedInstance)
        {
        EmbeddedInstanceCallbackP  memoryCallback   = v.GetMemoryCallback();
        MemoryECInstanceBaseP      mbStructInstance = p->GetAsMemoryECInstance();

        if (NULL == mbStructInstance)
            return ECOBJECTS_STATUS_Error;

        size_t offsetFromConcreteToIECInstance = p->GetOffsetToIECInstance();

        //size_t neededSize = mbStructInstance->GetObjectSize ();
        //neededSize += sizeof (StructArrayEntry);

        //get the gap location for new StructArrayEntry and for new Instance
        byte* dataAddress = const_cast<byte*>(_GetData ());

        byte*   baseAddress            =  dataAddress - offsetToData;  // need to base it on dataAddress since call to SetPrimitiveValueToMemory may have reallocated the buffer
        byte*   arrayCountAddress      =  baseAddress + offsetToArrayCount + (m_bytesAllocated-bytesAllocated);
        UInt32* arrayCountP            = (UInt32*)arrayCountAddress;
        size_t* endOffsetP             = (size_t*)(arrayCountAddress + sizeof(UInt32));
        byte*   arrayAddress           =  arrayCountAddress + sizeof(UInt32) + sizeof(size_t);
        UInt32  arrayCount             = *arrayCountP;
        byte*   entryAddress           =  arrayAddress + sizeof(StructArrayEntry)*arrayCount;

        // if we're in a managed instance, can't assume that (dataAddress - offsetToData) points to the instance containing the *current* buffer, need to calculate its address
        byte* structInstanceAddress = GetAddressOfInstanceFromAddressOfPropertyData () + *endOffsetP;

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

            Int64 offsetToPropertyData = m_data.offset + deltaOffset; 
            m_data.offset = offsetToPropertyData;

            // adjust offset stored in m_structInstances.vectorP by the delta between the old and new data addresses.
            Int64 offsetToStructArrayVector = m_structInstances.offset + (Int64)deltaOffset;
            m_structInstances.offset = offsetToStructArrayVector;

            // now actually populate the gaps in the reallocated buffer with the new StructArrayEntry and the new Struct Instance
            size_t            offsetToNewEntry          = entryAddress - dataAddress;     // offset relative to m_data.address
            byte*             newEntryAddress           = data.newDataAddress + offsetToNewEntry;
            StructArrayEntry* newEntry                  = (StructArrayEntry*)newEntryAddress;

            size_t            deltaToNewStructInstance  = (structInstanceAddress - dataAddress) + data.gapSize; // offset relative to m_data.address
            size_t            newStructInstanceOffset   = (data.newDataAddress + deltaToNewStructInstance) - newEntryAddress;
            byte*             newStructInstanceLocation = newEntryAddress + newStructInstanceOffset;
                                                                         
            // set the new struct array entry structValueIdentifier into the reallocated buffer
            newEntry->structValueIdentifier = structValueId;

            // set the new struct array entry instance offset into the reallocated buffer
            newStructInstanceOffset += offsetFromConcreteToIECInstance;
            memcpy (&newEntry->structInstance, &newStructInstanceOffset, sizeof (newStructInstanceOffset));

            // load the new struct instance data into the reallocated buffer
            mbStructInstance->LoadDataIntoManagedInstance (newStructInstanceLocation, data.instanceGapSize);

            // adjust offset to perPropertyFlags by delta, otherwise changes to flags after reallocation will not affect the resized instance
            m_perPropertyFlagsHolder.perPropertyFlags.offset += deltaOffset;

            // set offset to parent instance in new struct instance data
            mbStructInstance = (MemoryECInstanceBase*)newStructInstanceLocation;
            Int64 parentBaseAddress = (Int64) GetAddressOfInstanceFromAddressOfPropertyData ();
            mbStructInstance->m_parentInstance.offset = parentBaseAddress - (Int64)newStructInstanceLocation;

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
        if (NULL == m_structInstances.vectorP)
            m_structInstances.vectorP = new StructInstanceVector ();

        m_structInstances.vectorP->push_back (StructArrayEntry (structValueId, p));
        }

    return ECOBJECTS_STATUS_Success; 
    }    
  
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Paul.Connelly                   10/11
+---------------+---------------+---------------+---------------+---------------+------*/
StructValueIdentifier MemoryECInstanceBase::GetMaxStructValueIdentifier () const
    {
    if (!m_structInstances.vectorP)
        return 0;

    StructArrayEntry const* instanceArray = NULL;
    size_t numEntries = 0;

    if (!m_isInManagedInstance)
        {
        numEntries = m_structInstances.vectorP->size();
        instanceArray = &(*m_structInstances.vectorP)[0];
        }
    else
        {
        byte const* baseAddress = (byte const*)this;
        byte const* arrayCountAddress = baseAddress + m_structInstances.offset;
        byte const* arrayAddress = arrayCountAddress + sizeof(UInt32) + sizeof(size_t);

        UInt32 const* arrayCount = (UInt32 const*)arrayCountAddress;
        numEntries = *arrayCount;
        instanceArray = (StructArrayEntry const*)arrayAddress;
        }

    // we cannot simply use the size of the array because structs may have been removed at some point - so we must walk the array and find the highest ID
    StructValueIdentifier maxId = 0;
    for (size_t i = 0; i < numEntries; i++)
        {
        StructArrayEntry const& entry = instanceArray[i];
        if (entry.structValueIdentifier > maxId)
            maxId = entry.structValueIdentifier;
        }

    return maxId;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Bill.Steinbock                  12/2010
+---------------+---------------+---------------+---------------+---------------+------*/
StructArrayEntry const* MemoryECInstanceBase::GetAddressOfStructArrayEntry (StructValueIdentifier key) const
    {
    if (!m_structInstances.vectorP)
        return NULL;

    StructArrayEntry const* instanceArray = NULL;
    size_t numEntries = 0;

    if (!m_isInManagedInstance)
        {
        numEntries = m_structInstances.vectorP->size();
        instanceArray = &(*m_structInstances.vectorP)[0];
        }
    else
        {
        byte const* baseAddress = (byte const*)this;
        byte const* arrayCountAddress =  baseAddress + m_structInstances.offset;
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
* @bsimethod                                    Bill.Steinbock                  11/2011
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus           MemoryECInstanceBase::SetStructArrayInstance (MemoryECInstanceBaseCR instance, StructValueIdentifier structValueId)
    {
    if (m_isInManagedInstance)  // this check should eventually go away unless we keep support for embedding
        return ECOBJECTS_STATUS_Error;

    if (NULL == m_structInstances.vectorP)
        m_structInstances.vectorP = new StructInstanceVector ();

    IECInstancePtr instancePtr = instance.GetAsIECInstance();
    m_structInstances.vectorP->push_back (StructArrayEntry (structValueId, instancePtr));
    if (m_structValueId < structValueId)
        m_structValueId = structValueId;

    return ECOBJECTS_STATUS_Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Bill.Steinbock                  11/2011
+---------------+---------------+---------------+---------------+---------------+------*/
IECInstancePtr           MemoryECInstanceBase::GetStructArrayInstanceByIndex (UInt32 index, StructValueIdentifier& structValueId) const
    {
    if (m_isInManagedInstance)  // this check should eventually go away unless we keep support for embedding
        return NULL;

    if (NULL == m_structInstances.vectorP)  // no struct instances exist
        return NULL;

    if (index >= m_structInstances.vectorP->size())
        return NULL;

    StructArrayEntry& arrayEntry = (*m_structInstances.vectorP)[index];
    structValueId = arrayEntry.structValueIdentifier;
    return arrayEntry.structInstance.get();
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
    ECValue structValueIdValue;
    ECObjectsStatus status = GetPrimitiveValueFromMemory (structValueIdValue, propertyLayout, true, index);      
    if (status != ECOBJECTS_STATUS_Success)
        return status;
        
    // A structValueId of 0 means the instance is null
    if (structValueIdValue.IsNull() || 0 == structValueIdValue.GetInteger())
        {
        v.SetStruct(NULL);
        return ECOBJECTS_STATUS_Success;
        }        
                    
    StructValueIdentifier structValueId = structValueIdValue.GetInteger();

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
* Get the address of the data, this is &m_data.address if not in embedded in a managed
* instance, otherwise m_data.address is an offset and the address is calculated.
* @bsimethod                                    Bill.Steinbock                  06/2011
+---------------+---------------+---------------+---------------+---------------+------*/
byte*        MemoryECInstanceBase::GetAddressOfPropertyData () const
    {
    return const_cast<byte*>(_GetData());
    }

/*---------------------------------------------------------------------------------**//**
* When embedded in a managed instance, get the base address of the instance which holds
* the property data. This may be different than the 'this' pointer if reallocation
* has occurred.
* @bsimethod                                                    Paul.Connelly   10/11
+---------------+---------------+---------------+---------------+---------------+------*/
byte*       MemoryECInstanceBase::GetAddressOfInstanceFromAddressOfPropertyData () const
    {
    byte*   dataAddress = GetAddressOfPropertyData ();
    Int64   offsetToData = GetBaseObjectSize () + (m_perPropertyFlagsHolder.numPerPropertyFlagsEntries * sizeof (UInt32));
    return dataAddress - offsetToData;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    CaseyMullen     09/09
+---------------+---------------+---------------+---------------+---------------+------*/
UInt32              MemoryECInstanceBase::GetBytesUsed () const
    {
    if (NULL == m_data.address)
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

    if (m_structInstances.vectorP)
        m_structInstances.vectorP->clear ();

    InitializeMemory (GetClassLayout(), GetAddressOfPropertyData(), m_bytesAllocated);

    ClearAllPerPropertyFlags ();
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

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Bill.Steinbock                  07/2011
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus MemoryECInstanceBase::RemoveStructStructArrayEntry (StructValueIdentifier structValueId, EC::EmbeddedInstanceCallbackP memoryReallocationCallbackP)
    {
    if (!m_structInstances.vectorP)
        return ECOBJECTS_STATUS_Error;

    if (!m_isInManagedInstance)
        {
        StructInstanceVector::iterator iter;
        for (iter = m_structInstances.vectorP->begin(); iter != m_structInstances.vectorP->end(); iter++)
            {
            if (structValueId == (*iter).structValueIdentifier)
                {
                m_structInstances.vectorP->erase(iter);
                return ECOBJECTS_STATUS_Success;
                }
            }

        return ECOBJECTS_STATUS_Error;
        }

#ifdef DEBUGGING_STRUCTDELETE
    WString beforeDeleteString;
    WalkSupportingStructs (beforeDeleteString, L"");

    WString postDeleteLayout = InstanceDataToString (L"", GetClassLayout());
    if (postDeleteLayout.empty())
        return ECOBJECTS_STATUS_Error;
#endif

    StructArrayEntry* instanceArray = NULL;
    size_t numEntries = 0;

    byte const* baseAddress = (byte const*)this;
    byte const* arrayCountAddress =  baseAddress + m_structInstances.offset;
    byte const* arrayAddress      =  arrayCountAddress + sizeof(UInt32) + sizeof(size_t);  // array count + offset to end of supporting struct instances

    UInt32* arrayCountP = (UInt32*)arrayCountAddress;

    numEntries = *arrayCountP;
    instanceArray = (StructArrayEntry*)arrayAddress;

    byte* dataAddress = const_cast<byte*>(_GetData ());

    MemoryCallbackData data;
    data.dataAddress = dataAddress;
    data.useFlags    =  USE_FLAG_REMOVEGAPS; 

    IECInstancePtr structInstance = NULL;

    for (size_t i = 0; i<numEntries; i++)
        {
        StructArrayEntry& entry = instanceArray[i];
        if (entry.structValueIdentifier != structValueId)
            continue;

        structInstance = getEmbeddedSupportingStructInstance(&entry);
        MemoryECInstanceBaseP mbStructInstance = structInstance->GetAsMemoryECInstance();

        size_t offsetFromConcreteToIECInstance = structInstance->GetOffsetToIECInstance();

        data.gapAddress         = (byte*)&entry;
        data.gapSize            = sizeof (StructArrayEntry);
        data.instanceGapSize    = mbStructInstance->GetObjectSize ();
        data.instanceGapAddress = (byte*)structInstance.get() - offsetFromConcreteToIECInstance;

        break;
        }

    // update the array count, the end offset address, and the offset to the struct instances before inserting the 
    // gaps for the data. This way the values are properly set in the new managed byte buffer holding the resized 
    // instance. We will reset the values if the resizing is unsuccessful.

    // update the array count    
    *arrayCountP = (UInt32)numEntries-1;

    // Note: endOffsetP that points just beyond the last supporting struct instance will be updated 
    //       in the memory resize callback call to FixupStructArrayOffsets

    if (0 == memoryReallocationCallbackP (&data))
        {
        // hocus pocus - set offsets in this object to point to data in newly allocated object
        Int64 deltaOffset = data.newDataAddress - dataAddress;   // delta between memory locations for propertyData

        Int64 offsetToPropertyData = m_data.offset + deltaOffset;  // WIP: need to fix this .... for 32 bit builds this needs to be a Int32
        m_data.offset = offsetToPropertyData;

        // adjust offset stored in m_structInstances.vectorP by the delta between the old and new data addresses.
        Int64 offsetToStructArrayVector = m_structInstances.offset + deltaOffset;
        m_structInstances.offset = offsetToStructArrayVector;

        return ECOBJECTS_STATUS_Success;
        }
    else
        {
        // error occurred reset values
        *arrayCountP = (UInt32)numEntries;

        return ECOBJECTS_STATUS_Error;
        }
    
#ifdef DEBUGGING_STRUCTDELETE
    WString afterDeleteString;
    WalkSupportingStructs (afterDeleteString, L"");
#endif

    return ECOBJECTS_STATUS_Error;
    }
                                  
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Bill.Steinbock                  07/2011
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus          MemoryECInstanceBase::RemoveStructArrayElements (ClassLayoutCR classLayout, PropertyLayoutCR propertyLayout, UInt32 removeIndex, UInt32 removeCount, EC::EmbeddedInstanceCallbackP memoryReallocationCallbackP)
    {
    return  _RemoveStructArrayElementsFromMemory (classLayout, propertyLayout, removeIndex, removeCount, memoryReallocationCallbackP);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Bill.Steinbock                  07/2011
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus MemoryECInstanceBase::_RemoveStructArrayElementsFromMemory (ClassLayoutCR classLayout, PropertyLayoutCR propertyLayout, UInt32 removeIndex, UInt32 removeCount, EC::EmbeddedInstanceCallbackP memoryReallocationCallbackP)
    {
    ECValue         v;
    ECObjectsStatus status;

    for (UInt32 i = 0; i<removeCount; i++)
        {
        status = GetPrimitiveValueFromMemory (v, propertyLayout, true, removeIndex+i);
        if (status != ECOBJECTS_STATUS_Success)
            return status;

        // get struct value id from ecValue
        StructValueIdentifier structValueId = v.GetInteger ();    

        status = RemoveStructStructArrayEntry (structValueId, memoryReallocationCallbackP);       
        if (status != ECOBJECTS_STATUS_Success)
            return status;
       }   

    return RemoveArrayElementsFromMemory (classLayout, propertyLayout, removeIndex, removeCount);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Bill.Steinbock                  12/2011
+---------------+---------------+---------------+---------------+---------------+------*/
byte const *             MemoryECInstanceBase::GetPerPropertyFlagsData () const
    {
    return (byte const *)m_perPropertyFlagsHolder.perPropertyFlags.address;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Bill.Steinbock                  12/2011
+---------------+---------------+---------------+---------------+---------------+------*/
UInt8                    MemoryECInstanceBase::GetNumBitsPerProperty () const
    {
    return m_perPropertyFlagsHolder.numBitsPerProperty;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Bill.Steinbock                  12/2011
+---------------+---------------+---------------+---------------+---------------+------*/
UInt32                   MemoryECInstanceBase::GetPerPropertyFlagsDataLength () const
    {
    return m_perPropertyFlagsHolder.numPerPropertyFlagsEntries;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Paul.Connelly                   10/11
+---------------+---------------+---------------+---------------+---------------+------*/
MemoryECInstanceBase const* MemoryECInstanceBase::GetParentInstance () const
    {
    if (!m_isInManagedInstance)
        return m_parentInstance.parentInstance;
    else if (0 != m_parentInstance.offset)
        {
        byte const * baseAddress = (byte const *)this;
        return (MemoryECInstanceBase const *)(baseAddress + m_parentInstance.offset);
        }
    else
        return NULL;
    }

///////////////////////////////////////////////////////////////////////////////////////////
//  StandaloneECInstance
///////////////////////////////////////////////////////////////////////////////////////////
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Bill.Steinbock                  12/2010
+---------------+---------------+---------------+---------------+---------------+------*/
size_t                StandaloneECInstance::_GetOffsetToIECInstance () const
    {
    EC::IECInstanceCP iecInstanceP   = dynamic_cast<EC::IECInstanceCP>(this);
    byte const* baseAddressOfIECInstance = (byte const *)iecInstanceP;
    byte const* baseAddressOfConcrete = (byte const *)this;

    return (size_t)(baseAddressOfIECInstance - baseAddressOfConcrete);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    CaseyMullen     09/09
+---------------+---------------+---------------+---------------+---------------+------*/        
StandaloneECInstance::StandaloneECInstance (StandaloneECEnablerCR enabler, byte * data, UInt32 size) :
        MemoryECInstanceBase (data, size, enabler.GetClassLayout(), true),
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
    size_t perPropertyDataSize = sizeof(UInt32) * GetPerPropertyFlagsSize();
    size_t supportingInstanceDataSize = CalculateSupportingInstanceDataSize ();

    return objectSize+primaryInstanceDataSize+perPropertyDataSize+supportingInstanceDataSize;
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
* @bsimethod                                    Paul.Connelly                   10/11
+---------------+---------------+---------------+---------------+---------------+------*/
size_t                StandaloneECInstance::_GetBaseObjectSize () const
    {
    return sizeof (*this);
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

    PRECONDITION (NULL != propertyAccessString, ECOBJECTS_STATUS_PreconditionViolated);

    UInt32 propertyIndex = 0;
    if (ECOBJECTS_STATUS_Success != GetEnabler().GetPropertyIndex(propertyIndex, propertyAccessString))
        return ECOBJECTS_STATUS_PropertyNotFound;

    return _SetValue (propertyIndex, v, useArrayIndex, arrayIndex);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    05/10
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus           StandaloneECInstance::_SetValue (UInt32 propertyIndex, ECValueCR v, bool useArrayIndex, UInt32 arrayIndex)
    {
    ClassLayoutCR classLayout = GetClassLayout();

    SetPerPropertyBit ((UInt8) PROPERTYFLAGINDEX_IsLoaded, propertyIndex, true);

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
* @bsimethod                                    Bill.Steinbock                  07/2011
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus           StandaloneECInstance::_RemoveArrayElement (WCharCP propertyAccessString, UInt32 index)
    {
    ClassLayoutCR classLayout = GetClassLayout();    
    ECObjectsStatus status = RemoveArrayElementsAt (classLayout, propertyAccessString, index, 1);
    
    return status;
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
StandaloneECEnabler::StandaloneECEnabler (ECClassCR ecClass, ClassLayoutCR classLayout, IStandaloneEnablerLocaterP structStandaloneEnablerLocater, bool ownsClassLayout) :
    ECEnabler (ecClass, structStandaloneEnablerLocater),
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
StandaloneECEnablerPtr    StandaloneECEnabler::CreateEnabler (ECClassCR ecClass, ClassLayoutCR classLayout, IStandaloneEnablerLocaterP structStandaloneEnablerLocater, bool ownsClassLayout)
    {
    return new StandaloneECEnabler (ecClass, classLayout, structStandaloneEnablerLocater, ownsClassLayout);
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
ECObjectsStatus StandaloneECEnabler::_GetPropertyIndices (bvector<UInt32>& indices, UInt32 parentIndex) const { return GetClassLayout().GetPropertyIndices (indices, parentIndex);  }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Paul.Connelly                   12/11
+---------------+---------------+---------------+---------------+---------------+------*/
bool            StandaloneECEnabler::_HasChildProperties (UInt32 parentIndex) const
    {
    return GetClassLayout().HasChildProperties (parentIndex);
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
StandaloneECInstanceP   StandaloneECEnabler::CreateSharedInstance (byte * data, UInt32 size)
    {
    StandaloneECInstanceP instance = new StandaloneECInstance (*this, data, size);
    instance->SetUsingSharedMemory ();

    return instance;
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    CaseyMullen     09/09
+---------------+---------------+---------------+---------------+---------------+------*/        
StandaloneECInstancePtr   StandaloneECEnabler::CreateInstance (UInt32 minimumBufferSize) const
    {
    return new StandaloneECInstance (*this, minimumBufferSize);
    }    
    
END_BENTLEY_EC_NAMESPACE