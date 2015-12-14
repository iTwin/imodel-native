/*--------------------------------------------------------------------------------------+
|
|     $Source: src/StandaloneECInstance.cpp $
|
|   $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ECObjectsPch.h"

BEGIN_BENTLEY_ECOBJECT_NAMESPACE

///////////////////////////////////////////////////////////////////////////////////////////
//  MemoryECInstanceBase
///////////////////////////////////////////////////////////////////////////////////////////
const uint32_t BITS_PER_FLAGSBITMASK = (sizeof(uint32_t) * 8);
const uint32_t BITS_TO_SHIFT_FOR_FLAGSBITMASK = 5;            // bitToCheck >> BITS_TO_SHIFT_FOR_FLAGSBITMASK equiv. to bitToCheck / BITS_PER_FLAGSBITMASK

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Bill.Steinbock                  04/2010
+---------------+---------------+---------------+---------------+---------------+------*/
MemoryECInstanceBase::MemoryECInstanceBase (Byte * data, uint32_t size, ClassLayoutCR classLayout, bool allowWritingDirectlyToInstanceMemory, MemoryECInstanceBase const* parentInstance) :
        ECDBuffer (allowWritingDirectlyToInstanceMemory),
        m_bytesAllocated(size)
    {
    m_data = data;
    m_structInstances = NULL;
    m_usingSharedMemory = false;
    m_parentInstance = parentInstance;
    m_usageBitmask = 0;

    InitializePerPropertyFlags (classLayout, DEFAULT_NUMBITSPERPROPERTY);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Bill.Steinbock                  04/2010
+---------------+---------------+---------------+---------------+---------------+------*/
MemoryECInstanceBase::MemoryECInstanceBase (ClassLayoutCR classLayout, uint32_t minimumBufferSize, bool allowWritingDirectlyToInstanceMemory, ECClassCR ecClass, MemoryECInstanceBase const* parentInstance) :
        ECDBuffer (allowWritingDirectlyToInstanceMemory),
        m_bytesAllocated(0)
    {
    m_structInstances = NULL;
    m_data = NULL;
    m_usingSharedMemory = false;
    m_usageBitmask = 0;
    m_parentInstance = parentInstance;

    uint32_t size = std::max (minimumBufferSize, CalculateInitialAllocation (classLayout));
    m_data = (Byte*)malloc (size);
    m_bytesAllocated = size;

    InitializeMemory (classLayout, m_data, m_bytesAllocated, ecClass.IsDefined ("PersistStringsAsUtf8"));
    
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
void    MemoryECInstanceBase::InitializePerPropertyFlags (ClassLayoutCR classLayout, uint8_t numBitsPerPropertyFlag)
    {
    // max numBitsPerPropertyFlag = 16
    m_perPropertyFlagsHolder.numBitsPerProperty = (numBitsPerPropertyFlag > 16) ? 16 : numBitsPerPropertyFlag;

    uint32_t numProperties = classLayout.GetPropertyCount ();
    if (numBitsPerPropertyFlag > 0 && numProperties > 0)
        {
        m_perPropertyFlagsHolder.numPerPropertyFlagsEntries = ((numProperties * m_perPropertyFlagsHolder.numBitsPerProperty)+BITS_PER_FLAGSBITMASK) / BITS_PER_FLAGSBITMASK;
        m_perPropertyFlagsHolder.perPropertyFlags = (uint32_t*)calloc(m_perPropertyFlagsHolder.numPerPropertyFlagsEntries, sizeof(uint32_t));
        }
    else
        {
        m_perPropertyFlagsHolder.numPerPropertyFlagsEntries = 0;
        m_perPropertyFlagsHolder.perPropertyFlags = NULL;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Bill.Steinbock                  09/2011
+---------------+---------------+---------------+---------------+---------------+------*/
uint8_t                  MemoryECInstanceBase::GetNumBitsInPerPropertyFlags ()
    {
    return m_perPropertyFlagsHolder.numBitsPerProperty;
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Bill.Steinbock                  09/2011
+---------------+---------------+---------------+---------------+---------------+------*/
uint32_t   MemoryECInstanceBase::GetPerPropertyFlagsSize () const
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
void MemoryECInstanceBase::SetData (const Byte * data, uint32_t size, bool freeExisitingDataAndCreateCopyOfNewData) //The MemoryECInstanceBase will take ownership of the memory
    {
    if (freeExisitingDataAndCreateCopyOfNewData)
        {
        if (m_data)
            {
            free (m_data);
            m_data = NULL;
            }

        // allocate memory that the MemoryECInstanceBase will take ownership of 
        m_data = (Byte*)malloc (size);
        if (NULL == m_data)
            {
            DEBUG_EXPECT (false && "unable to allocate memory for instance data");
            return;
            }

        memcpy (m_data, data, size);
        }
    else
        {
        m_data = const_cast<Byte *>(data);
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
* @bsimethod                                    Bill.Steinbock                  09/2011
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus          MemoryECInstanceBase::IsPerPropertyBitSet (bool& isSet, uint8_t bitIndex, uint32_t propertyIndex) const
    {
    if (bitIndex >= m_perPropertyFlagsHolder.numBitsPerProperty)
        return ECObjectsStatus::InvalidIndexForPerPropertyFlag;

    if (propertyIndex >= GetClassLayout().GetPropertyCount ())
        return ECObjectsStatus::IndexOutOfRange;

    uint32_t* addressOfPerPropertyFlags = m_perPropertyFlagsHolder.perPropertyFlags;

    if (!addressOfPerPropertyFlags)
        return ECObjectsStatus::Error;

    uint32_t bitToCheck = (propertyIndex * m_perPropertyFlagsHolder.numBitsPerProperty) + bitIndex;
    uint32_t offset = bitToCheck >> BITS_TO_SHIFT_FOR_FLAGSBITMASK;
    uint32_t bit    = bitToCheck % BITS_PER_FLAGSBITMASK;
    uint32_t bitValue = 1 << bit;

    isSet = (bitValue == (addressOfPerPropertyFlags[offset] & bitValue));

    return ECObjectsStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Paul.Connelly                   10/11
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus         MemoryECInstanceBase::IsAnyPerPropertyBitSet (bool& isSet, uint8_t bitIndex) const
    {
    static const ::uint32_t   s_maskFor2Bits[2] = { 0x55555555, 0xAAAAAAAA };
    
    if (2 > m_perPropertyFlagsHolder.numBitsPerProperty)
        {
        DEBUG_FAIL ("PerPropertyFlagsHolder presently supports maximum 2 bits");
        return ECObjectsStatus::Error;
        }
    else if (bitIndex >= m_perPropertyFlagsHolder.numBitsPerProperty)
        return ECObjectsStatus::InvalidIndexForPerPropertyFlag;

    ::uint32_t mask = m_perPropertyFlagsHolder.numBitsPerProperty == 2 ? s_maskFor2Bits[(int)bitIndex] : 0xFFFFFFFF;

    uint32_t* addressOfPerPropertyFlags = m_perPropertyFlagsHolder.perPropertyFlags;
    if (NULL == addressOfPerPropertyFlags)
        return ECObjectsStatus::Error;

    isSet = false;
    for (uint32_t i = 0; i < m_perPropertyFlagsHolder.numPerPropertyFlagsEntries; i++)
        {
        if (0 != (mask & (addressOfPerPropertyFlags[i])))
            {
            isSet = true;
            break;
            }
        }

    return ECObjectsStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
* Set IsLoaded bit for specified property. If the property is a member of a struct
* then as set IsLoaded bit on the parent struct property.
* @bsimethod                                    Bill.Steinbock                  09/2011
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus          MemoryECInstanceBase::SetIsLoadedBit (uint32_t propertyIndex)
    {
    ECObjectsStatus status = SetPerPropertyBit ((uint8_t) PROPERTYFLAGINDEX_IsLoaded, propertyIndex, true);

    PropertyLayoutCP propertyLayout = NULL;

    if (ECObjectsStatus::Success == GetClassLayout().GetPropertyLayoutByIndex (propertyLayout, propertyIndex))
        {
        uint32_t parentIndex = propertyLayout->GetParentStructIndex();
        if (0 != parentIndex)
            {
            status = SetIsLoadedBit (parentIndex);
            }
        }

    return status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Bill.Steinbock                  09/2011
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus          MemoryECInstanceBase::SetPerPropertyBit (uint8_t bitIndex, uint32_t propertyIndex, bool setBit)
    {
    if (bitIndex >= m_perPropertyFlagsHolder.numBitsPerProperty)
        return ECObjectsStatus::InvalidIndexForPerPropertyFlag;

    if (propertyIndex >= GetClassLayout().GetPropertyCount ())
        return ECObjectsStatus::IndexOutOfRange;

    uint32_t* addressOfPerPropertyFlags = m_perPropertyFlagsHolder.perPropertyFlags;

    if (!addressOfPerPropertyFlags)
        return ECObjectsStatus::InvalidIndexForPerPropertyFlag;

    uint32_t bitToSet = (propertyIndex * m_perPropertyFlagsHolder.numBitsPerProperty) + bitIndex;
    uint32_t offset = bitToSet >> BITS_TO_SHIFT_FOR_FLAGSBITMASK;
    uint32_t bit    = bitToSet % BITS_PER_FLAGSBITMASK;
    uint32_t bitValue = 1 << bit;

    if (setBit)
        addressOfPerPropertyFlags[offset] |= bitValue;
    else
        addressOfPerPropertyFlags[offset] &= ~bitValue;

    return ECObjectsStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Bill.Steinbock                  09/2011
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus    MemoryECInstanceBase::SetBitForAllProperties (uint8_t bitIndex, bool setBit)
    {
    ECObjectsStatus status;
    int numProperties = GetClassLayout().GetPropertyCount ();

    for (uint32_t propertyIndex=0; propertyIndex<(uint32_t)numProperties; propertyIndex++)
        {
        status = SetPerPropertyBit (bitIndex, propertyIndex, setBit);
        if (ECObjectsStatus::Success != status)
            return status;
        }
    
    return ECObjectsStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Bill.Steinbock                  09/2011
+---------------+---------------+---------------+---------------+---------------+------*/
void    MemoryECInstanceBase::ClearAllPerPropertyFlags ()
    {
    uint32_t* addressOfPerPropertyFlags = m_perPropertyFlagsHolder.perPropertyFlags;
    if (!addressOfPerPropertyFlags)
        return;

    if (addressOfPerPropertyFlags)
        memset (addressOfPerPropertyFlags, 0, m_perPropertyFlagsHolder.numPerPropertyFlagsEntries*sizeof(uint32_t));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    CaseyMullen     09/09
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus           MemoryECInstanceBase::_ModifyData (uint32_t offset, void const * newData, uint32_t dataLength)
    {
    PRECONDITION (NULL != m_data, ECObjectsStatus::PreconditionViolated);
    PRECONDITION (offset + dataLength <= m_bytesAllocated, ECObjectsStatus::MemoryBoundsOverrun);

    Byte * dest = GetAddressOfPropertyData() + offset;
    memcpy (dest, newData, dataLength);
    
    return ECObjectsStatus::Success;
    }
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   10/12
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus MemoryECInstanceBase::_MoveData (uint32_t toOffset, uint32_t fromOffset, uint32_t dataLength)
    {
    PRECONDITION (NULL != m_data, ECObjectsStatus::PreconditionViolated);
    PRECONDITION (toOffset + dataLength <= m_bytesAllocated, ECObjectsStatus::MemoryBoundsOverrun);

    Byte* data = GetAddressOfPropertyData();
    memmove (data+toOffset, data+fromOffset, dataLength);

    return ECObjectsStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    CaseyMullen     09/09
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus                MemoryECInstanceBase::_ShrinkAllocation ()
    {
    uint32_t newAllocation = GetBytesUsed();
    if (0 == newAllocation)
        _FreeAllocation();
    else if (newAllocation != _GetBytesAllocated())
        {
        Byte* reallocedData = (Byte*)realloc(m_data, newAllocation);
        if (NULL == reallocedData)
            {
            BeAssert (false);
            return ECObjectsStatus::UnableToAllocateMemory;
            }

        m_data = reallocedData;
        m_bytesAllocated = newAllocation;
        }

    return ECObjectsStatus::Success;
    } 
//
////*---------------------------------------------------------------------------------**//**
////* @bsimethod                                                    Paul.Connelly   06/14
////+---------------+---------------+---------------+---------------+---------------+------*/
//ECObjectsStatus MemoryECInstanceBase::_SetCalculatedValueToMemory (ECValueCR v, PropertyLayoutCR propertyLayout, bool useIndex, UInt32 index) const
//    {
//    // If we're using pinned managed memory buffer, we can't resize it...we'll just re-evaluate the calculated property next time its value is requested.
//    return !m_usingSharedMemory ? ECDBuffer::_SetCalculatedValueToMemory (v, propertyLayout, useIndex, index) : ECObjectsStatus::Success;
//    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    CaseyMullen     09/09
+---------------+---------------+---------------+---------------+---------------+------*/
void                MemoryECInstanceBase::_FreeAllocation ()
    {
    if (!m_usingSharedMemory)
        {
        if (m_data)
            free (m_data); 

        if (m_perPropertyFlagsHolder.perPropertyFlags)
            {
            free (m_perPropertyFlagsHolder.perPropertyFlags); 
            m_perPropertyFlagsHolder.perPropertyFlags = NULL;
            }
        }

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
ECObjectsStatus           MemoryECInstanceBase::_GrowAllocation (uint32_t bytesNeeded)
    {
    DEBUG_EXPECT (m_bytesAllocated > 0);
    DEBUG_EXPECT (NULL != m_data);
        
    uint32_t newSize = 2 * (m_bytesAllocated + bytesNeeded); // Assume the growing trend will continue.

    Byte * reallocedData = (Byte*)realloc(m_data, newSize);
    DEBUG_EXPECT (NULL != reallocedData);
    if (NULL == reallocedData)
        return ECObjectsStatus::UnableToAllocateMemory;
    
    m_data = reallocedData; 
    m_bytesAllocated = newSize;

    return ECObjectsStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    CaseyMullen     09/09
+---------------+---------------+---------------+---------------+---------------+------*/
Byte const *        MemoryECInstanceBase::_GetData () const
    {
    return m_data;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    CaseyMullen     09/09
+---------------+---------------+---------------+---------------+---------------+------*/
uint32_t            MemoryECInstanceBase::_GetBytesAllocated () const
    {
    return m_bytesAllocated;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Adam.Klatzkin                   02/2010
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus     MemoryECInstanceBase::_SetStructArrayValueToMemory (ECValueCR v, PropertyLayoutCR propertyLayout, uint32_t index)
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
        // Avoid making a copy if we have a Standalone that's not already part of somebody else's struct array
        IECInstancePtr pFrom = v.GetStruct();
        StandaloneECInstancePtr pTo = dynamic_cast<StandaloneECInstance*> (pFrom.get());
        if (pTo.IsNull() || pTo->IsSupportingInstance())
            {
            pTo = pFrom->GetEnabler().GetClass().GetDefaultStandaloneEnabler()->CreateInstance();
            ECObjectsStatus copyStatus = pTo->CopyValues (*pFrom);
            if (ECObjectsStatus::Success != copyStatus)
                return copyStatus;
            }

        // We now own it as a supporting instance
        pTo->SetIsSupportingInstance();
        p = pTo.get();
        structValueIdValue.SetInteger (structValueId);
        }

    ECObjectsStatus status = SetPrimitiveValueToMemory (structValueIdValue, propertyLayout, true, index);      
    if (status != ECObjectsStatus::Success)
        {
        // This useless return value again....
        if (ECObjectsStatus::PropertyValueMatchesNoChange != status)
            return status;
        }
                 
    if (NULL == m_structInstances)
        m_structInstances = new StructInstanceVector ();

    m_structInstances->push_back (StructArrayEntry (structValueId, p));

    return ECObjectsStatus::Success; 
    }    
  
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Paul.Connelly                   10/11
+---------------+---------------+---------------+---------------+---------------+------*/
StructValueIdentifier MemoryECInstanceBase::GetMaxStructValueIdentifier () const
    {
    if (!m_structInstances)
        return 0;

    size_t numEntries =  m_structInstances->size();
    StructArrayEntry const* instanceArray = &(*m_structInstances)[0];

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
    if (!m_structInstances)
        return NULL;

    StructArrayEntry const* instanceArray = &(*m_structInstances)[0];
    size_t numEntries = m_structInstances->size();

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
ECObjectsStatus           MemoryECInstanceBase::SetStructArrayInstance (MemoryECInstanceBaseR instance, StructValueIdentifier structValueId)
    {
    if (NULL == m_structInstances)
        m_structInstances = new StructInstanceVector ();

    IECInstancePtr instancePtr = instance.GetAsIECInstanceP();
    m_structInstances->push_back (StructArrayEntry (structValueId, instancePtr));
    if (m_structValueId < structValueId)
        m_structValueId = structValueId;

    return ECObjectsStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Bill.Steinbock                  11/2011
+---------------+---------------+---------------+---------------+---------------+------*/
IECInstancePtr           MemoryECInstanceBase::GetStructArrayInstanceByIndex (uint32_t index, StructValueIdentifier& structValueId) const
    {
    if (NULL == m_structInstances)  // no struct instances exist
        return NULL;

    if (index >= m_structInstances->size())
        return NULL;

    StructArrayEntry& arrayEntry = (*m_structInstances)[index];
    structValueId = arrayEntry.structValueIdentifier;
    return arrayEntry.structInstance.get();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Bill.Steinbock                  12/2010
+---------------+---------------+---------------+---------------+---------------+------*/
IECInstancePtr  MemoryECInstanceBase::GetStructArrayInstance (StructValueIdentifier structValueId) const
    {
    StructArrayEntry const* entry = GetAddressOfStructArrayEntry (structValueId);
    return entry->structInstance;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Adam.Klatzkin                   02/2010
+---------------+---------------+---------------+---------------+---------------+------*/    
ECObjectsStatus           MemoryECInstanceBase::_GetStructArrayValueFromMemory (ECValueR v, PropertyLayoutCR propertyLayout, uint32_t index) const
    {
    ECValue structValueIdValue;
    ECObjectsStatus status = GetPrimitiveValueFromMemory (structValueIdValue, propertyLayout, true, index);      
    if (status != ECObjectsStatus::Success)
        return status;
        
    // A structValueId of 0 means the instance is null
    if (structValueIdValue.IsNull() || 0 == structValueIdValue.GetInteger())
        {
        v.SetStruct(NULL);
        return ECObjectsStatus::Success;
        }        
                    
    StructValueIdentifier structValueId = structValueIdValue.GetInteger();

    IECInstancePtr instancePtr = GetStructArrayInstance (structValueId);
    v.SetStruct (instancePtr.get());
    
    return ECObjectsStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    CaseyMullen     01/10
+---------------+---------------+---------------+---------------+---------------+------*/
Byte const *        MemoryECInstanceBase::GetData () const
    {
    return _GetData();
    }
    
/*---------------------------------------------------------------------------------**//**
* Get the address of the data, this is &m_data if not in embedded in a managed
* instance, otherwise m_data is an offset and the address is calculated.
* @bsimethod                                    Bill.Steinbock                  06/2011
+---------------+---------------+---------------+---------------+---------------+------*/
Byte*        MemoryECInstanceBase::GetAddressOfPropertyData () const
    {
    return const_cast<Byte*>(_GetData());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    CaseyMullen     09/09
+---------------+---------------+---------------+---------------+---------------+------*/
uint32_t            MemoryECInstanceBase::GetBytesUsed () const
    {
    if (NULL == m_data)
        return 0;

    return CalculateBytesUsed ();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    CaseyMullen     09/09
+---------------+---------------+---------------+---------------+---------------+------*/        
void                MemoryECInstanceBase::_ClearValues ()
    {
    if (m_structInstances)
        m_structInstances->clear ();

    InitializeMemory (GetClassLayout(), GetAddressOfPropertyData(), m_bytesAllocated);

    ClearAllPerPropertyFlags ();
    }
   
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Bill.Steinbock                  07/2011
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus MemoryECInstanceBase::RemoveStructStructArrayEntry (StructValueIdentifier structValueId)
    {
    if (!m_structInstances)
        return ECObjectsStatus::Error;

    StructInstanceVector::iterator iter;
    for (iter = m_structInstances->begin(); iter != m_structInstances->end(); iter++)
        {
        if (structValueId == (*iter).structValueIdentifier)
            {
            m_structInstances->erase(iter);
            return ECObjectsStatus::Success;
            }
        }

    return ECObjectsStatus::Error;
    }
                                  
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Bill.Steinbock                  07/2011
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus          MemoryECInstanceBase::RemoveStructArrayElements (PropertyLayoutCR propertyLayout, uint32_t removeIndex, uint32_t removeCount)
    {
    return  _RemoveStructArrayElementsFromMemory (propertyLayout, removeIndex, removeCount);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Bill.Steinbock                  07/2011
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus MemoryECInstanceBase::_RemoveStructArrayElementsFromMemory (PropertyLayoutCR propertyLayout, uint32_t removeIndex, uint32_t removeCount)
    {
    ECValue         v;
    ECObjectsStatus status;

    for (uint32_t i = 0; i<removeCount; i++)
        {
        status = GetPrimitiveValueFromMemory (v, propertyLayout, true, removeIndex+i);
        if (status != ECObjectsStatus::Success)
            return status;

        // get struct value id from ecValue
        StructValueIdentifier structValueId = v.GetInteger ();    

        status = RemoveStructStructArrayEntry (structValueId);       
        if (status != ECObjectsStatus::Success)
            return status;
       }   

    return RemoveArrayElementsFromMemory (propertyLayout, removeIndex, removeCount);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Bill.Steinbock                  12/2011
+---------------+---------------+---------------+---------------+---------------+------*/
Byte const *             MemoryECInstanceBase::GetPerPropertyFlagsData () const
    {
    return (Byte const *)m_perPropertyFlagsHolder.perPropertyFlags;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Bill.Steinbock                  12/2011
+---------------+---------------+---------------+---------------+---------------+------*/
uint8_t                  MemoryECInstanceBase::GetNumBitsPerProperty () const
    {
    return m_perPropertyFlagsHolder.numBitsPerProperty;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Bill.Steinbock                  12/2011
+---------------+---------------+---------------+---------------+---------------+------*/
uint32_t                 MemoryECInstanceBase::GetPerPropertyFlagsDataLength () const
    {
    return m_perPropertyFlagsHolder.numPerPropertyFlagsEntries;
    }

 /*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Bill.Steinbock                  01/2012
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus      MemoryECInstanceBase::AddNullArrayElements (uint32_t propIdx, uint32_t insertCount)
    {
    return AddNullArrayElementsAt (propIdx, insertCount);
    }
        
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Paul.Connelly                   10/11
+---------------+---------------+---------------+---------------+---------------+------*/
MemoryECInstanceBase const* MemoryECInstanceBase::GetParentInstance () const
    {
    return m_parentInstance;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Bill.Steinbock                  08/2012
+---------------+---------------+---------------+---------------+---------------+------*/
static void     mergeProperties (IECInstanceR target, ECValuesCollectionCR source, bool sourceIsMemoryBased)
    {
    for (ECValuesCollection::const_iterator it=source.begin(); it != source.end(); ++it)
        {
        ECPropertyValue const& prop  = *it;
        ECValueCR              value = prop.GetValue();

        if (sourceIsMemoryBased)
            {
            if (!value.IsLoaded())
                continue;
            }

        if (prop.HasChildValues())
            {
            auto ecprop = prop.GetValueAccessor().GetECProperty();
            if (nullptr != ecprop && ecprop->GetIsArray())
                {
                // TFS#128233: overwrite the entire array, adjusting size if necessary.
                ECValue targetArray;
                if (prop.GetValue().IsArray() && ECObjectsStatus::Success == target.GetValueUsingAccessor (targetArray, prop.GetValueAccessor()))
                    {
                    uint32_t nArrayElements = prop.GetValue().GetArrayInfo().GetCount();
                    if (nArrayElements != targetArray.GetArrayInfo().GetCount())
                        {
                        auto accessString = prop.GetValueAccessor().GetManagedAccessString();
                        if (ECObjectsStatus::Success == ECInstanceInteropHelper::ClearArray (target, accessString.c_str()))
                            ECInstanceInteropHelper::AddArrayElements (target, accessString.c_str(), nArrayElements);
                        }
                    }
                }

            mergeProperties (target, *prop.GetChildValues(), sourceIsMemoryBased);
            continue;
            }

        if (value.IsNull() && (!sourceIsMemoryBased || !value.IsLoaded()) )  // if value contains a loaded null then set the value
            continue;

        target.SetInternalValueUsingAccessor (prop.GetValueAccessor(), value);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Bill.Steinbock                  08/2012
+---------------+---------------+---------------+---------------+---------------+------*/
uint16_t        MemoryECInstanceBase::GetUsageBitmask () const
    {
    return m_usageBitmask;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Bill.Steinbock                  08/2012
+---------------+---------------+---------------+---------------+---------------+------*/
void            MemoryECInstanceBase::SetUsageBitmask (uint16_t mask)
    {
    m_usageBitmask =  mask;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Bill.Steinbock                  08/2012
+---------------+---------------+---------------+---------------+---------------+------*/
void            MemoryECInstanceBase::SetPartiallyLoaded (bool set)
    {
    if (set)
        m_usageBitmask = m_usageBitmask | (uint16_t)MEMORYINSTANCEUSAGE_IsPartiallyLoaded;
    else 
        m_usageBitmask = m_usageBitmask & (~(uint16_t)MEMORYINSTANCEUSAGE_IsPartiallyLoaded); 
    }

/*---------------------------------------------------------------------------------**//**
* Partially loaded means that not all available instance properties have been load
* from persistent storage due to some type of loading filter that was limiting
* the property values returned.
* @bsimethod                                    Bill.Steinbock                  08/2012
+---------------+---------------+---------------+---------------+---------------+------*/
bool           MemoryECInstanceBase::IsPartiallyLoaded () const
    {
    return  0 != (m_usageBitmask & (uint16_t)MEMORYINSTANCEUSAGE_IsPartiallyLoaded); 
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Bill.Steinbock                  08/2012
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus MemoryECInstanceBase::_SetIsHidden (bool set)
    {
    if (set)
        m_usageBitmask = m_usageBitmask | (uint16_t)MEMORYINSTANCEUSAGE_IsHidden;
    else 
        m_usageBitmask = m_usageBitmask & (~(uint16_t)MEMORYINSTANCEUSAGE_IsHidden);

    return ECObjectsStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
* Currently only used by ECXA Instance serialization/deserialization
* @bsimethod                                    Bill.Steinbock                  08/2012
+---------------+---------------+---------------+---------------+---------------+------*/
bool           MemoryECInstanceBase::_IsHidden () const
    {
    return  0 != (m_usageBitmask & (uint16_t)MEMORYINSTANCEUSAGE_IsHidden); 
    }

/*---------------------------------------------------------------------------------**//**
* This is used when copying properties from partial instance for instance update processing
* @bsimethod                                    Bill.Steinbock                  08/2012
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus MemoryECInstanceBase::MergePropertiesFromInstance
(
ECN::IECInstanceCR     fromNativeInstance
)
    {
    IECInstancePtr  thisAsIECInstance = GetAsIECInstanceP ();

    if (!thisAsIECInstance->GetClass().GetName().Equals (fromNativeInstance.GetClass().GetName().c_str()))
        return ECObjectsStatus::Error;

    bool sourceIsMemoryBased = (NULL != fromNativeInstance.GetAsMemoryECInstance ());

    ECValuesCollectionPtr   properties = ECValuesCollection::Create (fromNativeInstance);
    mergeProperties (*thisAsIECInstance, *properties, sourceIsMemoryBased);

    return ECObjectsStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
* Used when populating per property flags from managed instance
* @bsimethod                                    Bill.Steinbock                  08/2012
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus          MemoryECInstanceBase::SetInstancePerPropertyFlagsData (Byte const* perPropertyFlagsDataAddress, int numBitsPerProperty, int numPerPropertyFlagsEntries)
    {
    if (numBitsPerProperty != m_perPropertyFlagsHolder.numBitsPerProperty)
        return ECObjectsStatus::Error;

    if (numPerPropertyFlagsEntries !=  m_perPropertyFlagsHolder.numPerPropertyFlagsEntries)
        return ECObjectsStatus::Error;

    if (0 == numPerPropertyFlagsEntries)
        return ECObjectsStatus::Success;

    memcpy (m_perPropertyFlagsHolder.perPropertyFlags, perPropertyFlagsDataAddress, m_perPropertyFlagsHolder.numPerPropertyFlagsEntries * sizeof(uint32_t));
    return ECObjectsStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   09/12
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus MemoryECInstanceBase::SetValueInternal (uint32_t propertyIndex, ECValueCR v, bool useArrayIndex, uint32_t arrayIndex)
    {
    PropertyLayoutCP propLayout;
    ECObjectsStatus status;
    if (ECObjectsStatus::Success == (status = GetClassLayout().GetPropertyLayoutByIndex (propLayout, propertyIndex)))
        {
        SetIsLoadedBit (propertyIndex);
        status = SetInternalValueToMemory (*propLayout, v, useArrayIndex, arrayIndex);
        }

    return status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Bill.Steinbock                  11/2011
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus MemoryECInstanceBase::_CopyFromBuffer (ECDBufferCR src)
    {
    MemoryECInstanceBase const* fromMemoryInstance = dynamic_cast<MemoryECInstanceBase const*> (&src);
    if (NULL != fromMemoryInstance && GetClassLayout().Equals (fromMemoryInstance->GetClassLayout()))
        {
        SetUsageBitmask (fromMemoryInstance->GetUsageBitmask());
        memcpy (m_perPropertyFlagsHolder.perPropertyFlags, fromMemoryInstance->GetPerPropertyFlagsData(), m_perPropertyFlagsHolder.numPerPropertyFlagsEntries * sizeof(uint32_t));
        }

    return CopyPropertiesFromBuffer (src);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   05/13
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus MemoryECInstanceBase::_EvaluateCalculatedProperty (ECValueR calcedValue, ECValueCR existingValue, PropertyLayoutCR propLayout) const
    {
    IECInstanceCR thisInstance = *GetAsIECInstance();
    CalculatedPropertySpecificationCP spec = LookupCalculatedPropertySpecification (thisInstance, propLayout);
    if (NULL != spec)
        {
        uint32_t propIdx;
        if (ECObjectsStatus::Success == GetClassLayout().GetPropertyLayoutIndex (propIdx, propLayout))
            (const_cast<MemoryECInstanceBase&> (*this)).SetIsLoadedBit (propIdx);

        return spec->Evaluate (calcedValue, existingValue, thisInstance, propLayout.GetAccessString());
        }
    else
        return ECObjectsStatus::Error;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   05/13
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus MemoryECInstanceBase::_UpdateCalculatedPropertyDependents (ECValueCR calcedValue, PropertyLayoutCR propLayout)
    {
    IECInstanceR thisInstance = *GetAsIECInstanceP();
    CalculatedPropertySpecificationCP spec = LookupCalculatedPropertySpecification (thisInstance, propLayout);
    return NULL != spec ? spec->UpdateDependentProperties (calcedValue, thisInstance) : ECObjectsStatus::Error;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   05/13
+---------------+---------------+---------------+---------------+---------------+------*/
bool MemoryECInstanceBase::_IsStructValidForArray (IECInstanceCR structInstance, PropertyLayoutCR propLayout) const
    {
    uint32_t propIdx;
    if (ECObjectsStatus::Success == GetClassLayout().GetPropertyLayoutIndex (propIdx, propLayout))
        {
        ECPropertyCP ecprop = GetAsIECInstance()->GetEnabler().LookupECProperty (propIdx);
        StructArrayECPropertyCP arrayProp = ecprop ? ecprop->GetAsStructArrayProperty() : NULL;
        if (NULL != arrayProp)
            return structInstance.GetEnabler().GetClass().Is (arrayProp->GetStructElementType());
        }

    return false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   05/13
+---------------+---------------+---------------+---------------+---------------+------*/
IECInstanceP MemoryECInstanceBase::GetAsIECInstanceP()                                 { return _GetAsIECInstance(); }
IECInstanceCP MemoryECInstanceBase::GetAsIECInstance() const                           { return _GetAsIECInstance(); }

///////////////////////////////////////////////////////////////////////////////////////////
//  StandaloneECInstance
///////////////////////////////////////////////////////////////////////////////////////////
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Bill.Steinbock                  12/2010
+---------------+---------------+---------------+---------------+---------------+------*/
size_t                StandaloneECInstance::_GetOffsetToIECInstance () const
    {
    ECN::IECInstanceCP iecInstanceP   = dynamic_cast<ECN::IECInstanceCP>(this);
    Byte const* baseAddressOfIECInstance = (Byte const *)iecInstanceP;
    Byte const* baseAddressOfConcrete = (Byte const *)this;

    return (size_t)(baseAddressOfIECInstance - baseAddressOfConcrete);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    CaseyMullen     09/09
+---------------+---------------+---------------+---------------+---------------+------*/        
StandaloneECInstance::StandaloneECInstance (StandaloneECEnablerR enabler, Byte * data, uint32_t size) :
        MemoryECInstanceBase (data, size, enabler.GetClassLayout(), true),
        m_sharedWipEnabler(&enabler), m_isSupportingInstance (false)
    {
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    CaseyMullen     01/10
+---------------+---------------+---------------+---------------+---------------+------*/        
StandaloneECInstance::StandaloneECInstance (StandaloneECEnablerR enabler, uint32_t minimumBufferSize) :
        MemoryECInstanceBase (enabler.GetClassLayout(), minimumBufferSize, true, enabler.GetClass()),
        m_sharedWipEnabler(&enabler), m_isSupportingInstance (false)
    {
    }
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    CaseyMullen     01/10
+---------------+---------------+---------------+---------------+---------------+------*/        
StandaloneECInstance::~StandaloneECInstance ()
    {
    //LOG.tracev (L"StandaloneECInstance at 0x%x is being destructed. It references enabler 0x%x", this, m_sharedWipEnabler);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Dylan.Rush      11/10
+---------------+---------------+---------------+---------------+---------------+------*/ 
StandaloneECInstancePtr         StandaloneECInstance::Duplicate(IECInstanceCR instance)
    {
    ECClassCR              ecClass           = instance.GetClass();
    StandaloneECEnablerPtr standaloneEnabler = instance.GetEnablerR().GetEnablerForStructArrayMember (ecClass.GetSchema().GetSchemaKey(), ecClass.GetName().c_str());
    if (standaloneEnabler.IsNull())
        return NULL;

    StandaloneECInstancePtr newInstance = standaloneEnabler->CreateInstance();
    if (ECObjectsStatus::Success != newInstance->CopyValues (instance))
        return NULL;

    return newInstance;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Bill.Steinbock                  12/2010
+---------------+---------------+---------------+---------------+---------------+------*/
IECInstanceP      StandaloneECInstance::_GetAsIECInstance () const
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
* @bsimethod                                                    Paul.Connelly   11/12
+---------------+---------------+---------------+---------------+---------------+------*/
ECDBuffer* StandaloneECInstance::_GetECDBuffer() const
    {
    return const_cast<StandaloneECInstance*> (this);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    CaseyMullen     10/09
+---------------+---------------+---------------+---------------+---------------+------*/    
ECEnablerCR         StandaloneECInstance::_GetEnabler() const
    {
    BeAssert (m_sharedWipEnabler.IsValid());
    return *m_sharedWipEnabler;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    CaseyMullen     10/09
+---------------+---------------+---------------+---------------+---------------+------*/    
Utf8String        StandaloneECInstance::_GetInstanceId() const
    {
    return m_instanceId;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    05/11
+---------------+---------------+---------------+---------------+---------------+------*/    
ECObjectsStatus StandaloneECInstance::_SetInstanceId (Utf8CP instanceId)
    {
    m_instanceId = instanceId;
    return ECObjectsStatus::Success;
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
* @bsimethod                                                    JoshSchifter    05/10
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus           StandaloneECInstance::_GetValue (ECValueR v, uint32_t propertyIndex, bool useArrayIndex, uint32_t arrayIndex) const
    {
    v.Clear ();

    ECObjectsStatus status = GetValueFromMemory (v, propertyIndex, useArrayIndex, arrayIndex);
    if (ECObjectsStatus::Success == status)
        {
        bool isSet = false;
        if (ECObjectsStatus::Success == MemoryECInstanceBase::IsPerPropertyBitSet (isSet, (uint8_t) PROPERTYFLAGINDEX_IsLoaded, propertyIndex)) 
            v.SetIsLoaded (isSet);
        }

    return status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    CaseyMullen     09/09
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus           StandaloneECInstance::_GetIsPropertyNull (bool& isNull, uint32_t propertyIndex, bool useArrayIndex, uint32_t arrayIndex) const
    {
    return GetIsNullValueFromMemory (isNull, propertyIndex, useArrayIndex, arrayIndex);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    05/10
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus           StandaloneECInstance::_SetValue (uint32_t propertyIndex, ECValueCR v, bool useArrayIndex, uint32_t arrayIndex)
    {
    SetIsLoadedBit (propertyIndex);

    return SetValueToMemory (propertyIndex, v, useArrayIndex, arrayIndex);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   09/12
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus StandaloneECInstance::_SetInternalValue (uint32_t propertyIndex, ECValueCR v, bool useArrayIndex, uint32_t arrayIndex)
    {
    SetIsLoadedBit (propertyIndex);
    return SetValueInternal (propertyIndex, v, useArrayIndex, arrayIndex);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Adam.Klatzkin                   01/2010
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus           StandaloneECInstance::_InsertArrayElements (uint32_t propIdx, uint32_t index, uint32_t size)
    {
    SetIsLoadedBit (propIdx);
    ECObjectsStatus status = InsertNullArrayElementsAt (propIdx, index, size);
    
    return status;
    } 
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   07/14
+---------------+---------------+---------------+---------------+---------------+------*/
void StandaloneECInstance::BindSchema()
    {
    m_boundSchema = const_cast<ECSchemaP>(&GetClass().GetSchema());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Adam.Klatzkin                   01/2010
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus           StandaloneECInstance::_AddArrayElements (uint32_t propIdx, uint32_t size)
    {
    SetIsLoadedBit (propIdx);
    ECObjectsStatus status = AddNullArrayElementsAt (propIdx, size);
    
    return status;
    }        

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Bill.Steinbock                  07/2011
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus           StandaloneECInstance::_RemoveArrayElement (uint32_t propIdx, uint32_t index)
    {
    return RemoveArrayElementsAt (propIdx, index, 1);
    } 

 /*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Adam.Klatzkin                   01/2010
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus           StandaloneECInstance::_ClearArray (uint32_t propIdx)
    {
    PropertyLayoutCP pPropertyLayout = NULL;
    ECObjectsStatus status = GetClassLayout().GetPropertyLayoutByIndex (pPropertyLayout, propIdx);
    if (ECObjectsStatus::Success != status || NULL == pPropertyLayout)
        return ECObjectsStatus::PropertyNotFound;

    uint32_t arrayCount = GetReservedArrayCount (*pPropertyLayout);
    if (arrayCount > 0)
        {
        status =  RemoveArrayElements (*pPropertyLayout, 0, arrayCount);

        if (ECObjectsStatus::Success == status)
            SetPerPropertyBit ((uint8_t) PROPERTYFLAGINDEX_IsLoaded, propIdx, false);

        return  status;
        }

    return ECObjectsStatus::Success;
    }                      

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    CaseyMullen     10/09
+---------------+---------------+---------------+---------------+---------------+------*/    
Utf8String        StandaloneECInstance::_ToString (Utf8CP indent) const
    {
    return InstanceDataToString (indent);
    }

///////////////////////////////////////////////////////////////////////////////////////////
//  StandaloneECEnabler
///////////////////////////////////////////////////////////////////////////////////////////
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    CaseyMullen     12/09
+---------------+---------------+---------------+---------------+---------------+------*/    
StandaloneECEnabler::StandaloneECEnabler (ECClassCR ecClass, ClassLayoutR classLayout, IStandaloneEnablerLocaterP structStandaloneEnablerLocater) :
    ECEnabler (ecClass, structStandaloneEnablerLocater),
    m_classLayout (&classLayout)
    {
    BeAssert (NULL != &ecClass);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    11/10
+---------------+---------------+---------------+---------------+---------------+------*/    
StandaloneECEnabler::~StandaloneECEnabler ()
    {
    //
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    CaseyMullen     12/09
+---------------+---------------+---------------+---------------+---------------+------*/    
StandaloneECEnablerPtr    StandaloneECEnabler::CreateEnabler (ECClassCR ecClass, ClassLayoutR classLayout, IStandaloneEnablerLocaterP structStandaloneEnablerLocater)
    {
    return new StandaloneECEnabler (ecClass, classLayout, structStandaloneEnablerLocater);
    }
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    CaseyMullen     12/09
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8CP           StandaloneECEnabler::_GetName() const
    {
    return "Bentley::ECN::StandaloneECEnabler";
    }
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    05/10
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus StandaloneECEnabler::_GetPropertyIndex(uint32_t& propertyIndex, Utf8CP propertyAccessString) const { return GetClassLayout().GetPropertyIndex (propertyIndex, propertyAccessString); }
ECObjectsStatus StandaloneECEnabler::_GetAccessString(Utf8CP& accessString, uint32_t propertyIndex) const { return GetClassLayout().GetAccessStringByIndex (accessString, propertyIndex); }
uint32_t        StandaloneECEnabler::_GetFirstPropertyIndex (uint32_t parentIndex) const {  return GetClassLayout().GetFirstChildPropertyIndex (parentIndex); }
uint32_t        StandaloneECEnabler::_GetNextPropertyIndex (uint32_t parentIndex, uint32_t inputIndex) const { return GetClassLayout().GetNextChildPropertyIndex (parentIndex, inputIndex);  }
ECObjectsStatus StandaloneECEnabler::_GetPropertyIndices (bvector<uint32_t>& indices, uint32_t parentIndex) const { return GetClassLayout().GetPropertyIndices (indices, parentIndex);  }
ClassLayoutCR   StandaloneECEnabler::GetClassLayout() const { return *m_classLayout; }
ClassLayoutR    StandaloneECEnabler::GetClassLayout() { return *m_classLayout; }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Paul.Connelly                   12/11
+---------------+---------------+---------------+---------------+---------------+------*/
bool            StandaloneECEnabler::_HasChildProperties (uint32_t parentIndex) const
    {
    return GetClassLayout().HasChildProperties (parentIndex);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   10/14
+---------------+---------------+---------------+---------------+---------------+------*/
uint32_t StandaloneECEnabler::_GetParentPropertyIndex (uint32_t childIndex) const
    {
    return GetClassLayout().GetParentPropertyIndex (childIndex);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   08/12
+---------------+---------------+---------------+---------------+---------------+------*/
bool StandaloneECEnabler::_IsPropertyReadOnly (uint32_t propertyIndex) const
    {
    return GetClassLayout().IsPropertyReadOnly (propertyIndex);
    }
 
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    CaseyMullen     09/09
+---------------+---------------+---------------+---------------+---------------+------*/        
StandaloneECInstanceP   StandaloneECEnabler::CreateSharedInstance (Byte * data, uint32_t size)
    {
    StandaloneECInstanceP instance = new StandaloneECInstance (*this, data, size);
    instance->SetUsingSharedMemory ();

    return instance;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    CaseyMullen     09/09
+---------------+---------------+---------------+---------------+---------------+------*/
StandaloneECInstancePtr   StandaloneECEnabler::CreateInstance (uint32_t minimumBufferSize)
    {
    StandaloneECInstancePtr instance = new StandaloneECInstance (*this, minimumBufferSize);
    return instance;
    }    
    
END_BENTLEY_ECOBJECT_NAMESPACE
