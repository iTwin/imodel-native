P*--------------------------------------------------------------------------------------+
|
|     $Source: src/StandaloneECInstance.cpp $
|
|   $Copyright: (c) 2012 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ECObjectsPch.h"

BEGIN_BENTLEY_ECOBJECT_NAMESPACE

///////////////////////////////////////////////////////////////////////////////////////////
//  MemoryECInstanceBase
///////////////////////////////////////////////////////////////////////////////////////////
const UInt32 BITS_PER_FLAGSBITMASK = (sizeof(UInt32) * 8);
const UInt32 BITS_TO_SHIFT_FOR_FLAGSBITMASK = 5;            // bitToCheck >> BITS_TO_SHIFT_FOR_FLAGSBITMASK equiv. to bitToCheck / BITS_PER_FLAGSBITMASK

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Bill.Steinbock                  04/2010
+---------------+---------------+---------------+---------------+---------------+------*/
MemoryECInstanceBase::MemoryECInstanceBase (byte * data, UInt32 size, ClassLayoutCR classLayout, bool allowWritingDirectlyToInstanceMemory, MemoryECInstanceBase const* parentInstance) :
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
MemoryECInstanceBase::MemoryECInstanceBase (ClassLayoutCR classLayout, UInt32 minimumBufferSize, bool allowWritingDirectlyToInstanceMemory, MemoryECInstanceBase const* parentInstance) :
        ECDBuffer (allowWritingDirectlyToInstanceMemory),
        m_bytesAllocated(0)
    {
    m_structInstances = NULL;
    m_data = NULL;
    m_usingSharedMemory = false;
    m_usageBitmask = 0;
    m_parentInstance = parentInstance;

    UInt32 size = std::max (minimumBufferSize, CalculateInitialAllocation (classLayout));
    m_data = (byte*)malloc (size);
    m_bytesAllocated = size;

    InitializeMemory (classLayout, m_data, m_bytesAllocated);
    
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
        m_perPropertyFlagsHolder.perPropertyFlags = (UInt32*)calloc(m_perPropertyFlagsHolder.numPerPropertyFlagsEntries, sizeof(UInt32));
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
void MemoryECInstanceBase::SetData (const byte * data, UInt32 size, bool freeExisitingData) //The MemoryECInstanceBase will take ownership of the memory
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
        m_data = const_cast<byte *>(data);
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
ECObjectsStatus          MemoryECInstanceBase::IsPerPropertyBitSet (bool& isSet, UInt8 bitIndex, UInt32 propertyIndex) const
    {
    if (bitIndex >= m_perPropertyFlagsHolder.numBitsPerProperty)
        return ECOBJECTS_STATUS_InvalidIndexForPerPropertyFlag;

    if (propertyIndex >= GetClassLayout().GetPropertyCount ())
        return ECOBJECTS_STATUS_IndexOutOfRange;

    UInt32* addressOfPerPropertyFlags = m_perPropertyFlagsHolder.perPropertyFlags;

    if (!addressOfPerPropertyFlags)
        return ECOBJECTS_STATUS_Error;

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
    
    if (2 > m_perPropertyFlagsHolder.numBitsPerProperty)
        {
        DEBUG_FAIL ("PerPropertyFlagsHolder presently supports maximum 2 bits");
        return ECOBJECTS_STATUS_Error;
        }
    else if (bitIndex >= m_perPropertyFlagsHolder.numBitsPerProperty)
        return ECOBJECTS_STATUS_InvalidIndexForPerPropertyFlag;

    ::UInt32 mask = m_perPropertyFlagsHolder.numBitsPerProperty == 2 ? s_maskFor2Bits[(int)bitIndex] : 0xFFFFFFFF;

    UInt32* addressOfPerPropertyFlags = m_perPropertyFlagsHolder.perPropertyFlags;
    if (NULL == addressOfPerPropertyFlags)
        return ECOBJECTS_STATUS_Error;

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
* Set IsLoaded bit for specified property. If the property is a member of a struct
* then as set IsLoaded bit on the parent struct property.
* @bsimethod                                    Bill.Steinbock                  09/2011
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus          MemoryECInstanceBase::SetIsLoadedBit (UInt32 propertyIndex)
    {
    ECObjectsStatus status = SetPerPropertyBit ((UInt8) PROPERTYFLAGINDEX_IsLoaded, propertyIndex, true);

    PropertyLayoutCP propertyLayout = NULL;

    if (ECOBJECTS_STATUS_Success == GetClassLayout().GetPropertyLayoutByIndex (propertyLayout, propertyIndex))
        {
        UInt32 parentIndex = propertyLayout->GetParentStructIndex();
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
ECObjectsStatus          MemoryECInstanceBase::SetPerPropertyBit (UInt8 bitIndex, UInt32 propertyIndex, bool setBit)
    {
    if (bitIndex >= m_perPropertyFlagsHolder.numBitsPerProperty)
        return ECOBJECTS_STATUS_InvalidIndexForPerPropertyFlag;

    if (propertyIndex >= GetClassLayout().GetPropertyCount ())
        return ECOBJECTS_STATUS_IndexOutOfRange;

    UInt32* addressOfPerPropertyFlags = m_perPropertyFlagsHolder.perPropertyFlags;

    if (!addressOfPerPropertyFlags)
        return ECOBJECTS_STATUS_InvalidIndexForPerPropertyFlag;

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
    UInt32* addressOfPerPropertyFlags = m_perPropertyFlagsHolder.perPropertyFlags;
    if (!addressOfPerPropertyFlags)
        return;

    if (addressOfPerPropertyFlags)
        memset (addressOfPerPropertyFlags, 0, m_perPropertyFlagsHolder.numPerPropertyFlagsEntries*sizeof(UInt32));
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
* @bsimethod                                                    Paul.Connelly   10/12
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus MemoryECInstanceBase::_MoveData (UInt32 toOffset, UInt32 fromOffset, UInt32 dataLength)
    {
    PRECONDITION (NULL != m_data, ECOBJECTS_STATUS_PreconditionViolated);
    PRECONDITION (toOffset + dataLength <= m_bytesAllocated, ECOBJECTS_STATUS_MemoryBoundsOverrun);

    byte* data = GetAddressOfPropertyData();
    memmove (data+toOffset, data+fromOffset, dataLength);

    return ECOBJECTS_STATUS_Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    CaseyMullen     09/09
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus                MemoryECInstanceBase::_ShrinkAllocation ()
    {
    UInt32 newAllocation = GetBytesUsed();
    if (0 == newAllocation)
        _FreeAllocation();
    else if (newAllocation != _GetBytesAllocated())
        {
        byte* reallocedData = (byte*)realloc(m_data, newAllocation);
        if (NULL == reallocedData)
            {
            BeAssert (false);
            return ECOBJECTS_STATUS_UnableToAllocateMemory;
            }

        m_data = reallocedData;
        m_bytesAllocated = newAllocation;
        }

    return ECOBJECTS_STATUS_Success;
    } 

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
ECObjectsStatus           MemoryECInstanceBase::_GrowAllocation (UInt32 bytesNeeded)
    {
    DEBUG_EXPECT (m_bytesAllocated > 0);
    DEBUG_EXPECT (NULL != m_data);
        
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

    ECObjectsStatus status = SetPrimitiveValueToMemory (structValueIdValue, classLayout, propertyLayout, true, index);      
    if (status != ECOBJECTS_STATUS_Success)
        {
        // This useless return value again....
        if (ECOBJECTS_STATUS_PropertyValueMatchesNoChange != status)
            return status;
        }
                 
    if (NULL == m_structInstances)
        m_structInstances = new StructInstanceVector ();

    m_structInstances->push_back (StructArrayEntry (structValueId, p));

    return ECOBJECTS_STATUS_Success; 
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
ECObjectsStatus           MemoryECInstanceBase::SetStructArrayInstance (MemoryECInstanceBaseCR instance, StructValueIdentifier structValueId)
    {
    if (NULL == m_structInstances)
        m_structInstances = new StructInstanceVector ();

    IECInstancePtr instancePtr = instance.GetAsIECInstance();
    m_structInstances->push_back (StructArrayEntry (structValueId, instancePtr));
    if (m_structValueId < structValueId)
        m_structValueId = structValueId;

    return ECOBJECTS_STATUS_Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Bill.Steinbock                  11/2011
+---------------+---------------+---------------+---------------+---------------+------*/
IECInstancePtr           MemoryECInstanceBase::GetStructArrayInstanceByIndex (UInt32 index, StructValueIdentifier& structValueId) const
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
ECObjectsStatus           MemoryECInstanceBase::_GetStructArrayValueFromMemory (ECValueR v, PropertyLayoutCR propertyLayout, UInt32 index) const
    {
    ECValue structValueIdValue;
    ECObjectsStatus status = GetPrimitiveValueFromMemory (structValueIdValue, GetClassLayout(), propertyLayout, true, index);      
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

    return CalculateBytesUsed (GetClassLayout());
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
        return ECOBJECTS_STATUS_Error;

    StructInstanceVector::iterator iter;
    for (iter = m_structInstances->begin(); iter != m_structInstances->end(); iter++)
        {
        if (structValueId == (*iter).structValueIdentifier)
            {
            m_structInstances->erase(iter);
            return ECOBJECTS_STATUS_Success;
            }
        }

    return ECOBJECTS_STATUS_Error;
    }
                                  
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Bill.Steinbock                  07/2011
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus          MemoryECInstanceBase::RemoveStructArrayElements (ClassLayoutCR classLayout, PropertyLayoutCR propertyLayout, UInt32 removeIndex, UInt32 removeCount)
    {
    return  _RemoveStructArrayElementsFromMemory (classLayout, propertyLayout, removeIndex, removeCount);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Bill.Steinbock                  07/2011
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus MemoryECInstanceBase::_RemoveStructArrayElementsFromMemory (ClassLayoutCR classLayout, PropertyLayoutCR propertyLayout, UInt32 removeIndex, UInt32 removeCount)
    {
    ECValue         v;
    ECObjectsStatus status;

    for (UInt32 i = 0; i<removeCount; i++)
        {
        status = GetPrimitiveValueFromMemory (v, GetClassLayout(), propertyLayout, true, removeIndex+i);
        if (status != ECOBJECTS_STATUS_Success)
            return status;

        // get struct value id from ecValue
        StructValueIdentifier structValueId = v.GetInteger ();    

        status = RemoveStructStructArrayEntry (structValueId);       
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
    return (byte const *)m_perPropertyFlagsHolder.perPropertyFlags;
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
* @bsimethod                                    Bill.Steinbock                  01/2012
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus      MemoryECInstanceBase::AddNullArrayElements (UInt32 propIdx, UInt32 insertCount)
    {
    return AddNullArrayElementsAt (GetClassLayout(), propIdx, insertCount);
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
UInt16          MemoryECInstanceBase::GetUsageBitmask () const
    {
    return m_usageBitmask;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Bill.Steinbock                  08/2012
+---------------+---------------+---------------+---------------+---------------+------*/
void            MemoryECInstanceBase::SetUsageBitmask (UInt16 mask)
    {
    m_usageBitmask =  mask;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Bill.Steinbock                  08/2012
+---------------+---------------+---------------+---------------+---------------+------*/
void            MemoryECInstanceBase::SetPartiallyLoaded (bool set)
    {
    if (set)
        m_usageBitmask = m_usageBitmask | (UInt16)MEMORYINSTANCEUSAGE_IsPartiallyLoaded;
    else 
        m_usageBitmask = m_usageBitmask & (~(UInt16)MEMORYINSTANCEUSAGE_IsPartiallyLoaded); 
    }

/*---------------------------------------------------------------------------------**//**
* Partially loaded means that not all available instance properties have been load
* from persistent storage due to some type of loading filter that was limiting
* the property values returned.
* @bsimethod                                    Bill.Steinbock                  08/2012
+---------------+---------------+---------------+---------------+---------------+------*/
bool           MemoryECInstanceBase::IsPartiallyLoaded ()
    {
    return  0 != (m_usageBitmask & (UInt16)MEMORYINSTANCEUSAGE_IsPartiallyLoaded); 
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Bill.Steinbock                  08/2012
+---------------+---------------+---------------+---------------+---------------+------*/
bool            MemoryECInstanceBase::SetHiddenInstance (bool set)
    {
    bool returnVal = IsHiddenInstance();

    if (set)
        m_usageBitmask = m_usageBitmask | (UInt16)MEMORYINSTANCEUSAGE_IsHidden;
    else 
        m_usageBitmask = m_usageBitmask & (~(UInt16)MEMORYINSTANCEUSAGE_IsHidden);

    return returnVal;
    }

/*---------------------------------------------------------------------------------**//**
* Currently only used by ECXA Instance serialization/deserialization
* @bsimethod                                    Bill.Steinbock                  08/2012
+---------------+---------------+---------------+---------------+---------------+------*/
bool           MemoryECInstanceBase::IsHiddenInstance ()
    {
    return  0 != (m_usageBitmask & (UInt16)MEMORYINSTANCEUSAGE_IsHidden); 
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
    IECInstancePtr  thisAsIECInstance = GetAsIECInstance ();

    if (!thisAsIECInstance->GetClass().GetName().Equals (fromNativeInstance.GetClass().GetName().c_str()))
        return ECOBJECTS_STATUS_Error;

    bool sourceIsMemoryBased = (NULL != fromNativeInstance.GetAsMemoryECInstance ());

    ECValuesCollectionPtr   properties = ECValuesCollection::Create (fromNativeInstance);
    mergeProperties (*thisAsIECInstance, *properties, sourceIsMemoryBased);

    return ECOBJECTS_STATUS_Success;
    }

/*---------------------------------------------------------------------------------**//**
* Used when populating per property flags from managed instance
* @bsimethod                                    Bill.Steinbock                  08/2012
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus          MemoryECInstanceBase::SetInstancePerPropertyFlagsData (byte const* perPropertyFlagsDataAddress, int numBitsPerProperty, int numPerPropertyFlagsEntries)
    {
    if (numBitsPerProperty != m_perPropertyFlagsHolder.numBitsPerProperty)
        return ECOBJECTS_STATUS_Error;

    if (numPerPropertyFlagsEntries !=  m_perPropertyFlagsHolder.numPerPropertyFlagsEntries)
        return ECOBJECTS_STATUS_Error;

    if (0 == numPerPropertyFlagsEntries)
        return ECOBJECTS_STATUS_Success;

    memcpy (m_perPropertyFlagsHolder.perPropertyFlags, perPropertyFlagsDataAddress, m_perPropertyFlagsHolder.numPerPropertyFlagsEntries * sizeof(UInt32));
    return ECOBJECTS_STATUS_Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   09/12
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus MemoryECInstanceBase::SetValueInternal (UInt32 propertyIndex, ECValueCR v, bool useArrayIndex, UInt32 arrayIndex)
    {
    PropertyLayoutCP propLayout;
    ECObjectsStatus status;
    if (ECOBJECTS_STATUS_Success == (status = GetClassLayout().GetPropertyLayoutByIndex (propLayout, propertyIndex)))
        {
        SetIsLoadedBit (propertyIndex);
        status = SetInternalValueToMemory (GetClassLayout(), *propLayout, v, useArrayIndex, arrayIndex);
        }

    return status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Bill.Steinbock                  11/2011
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus MemoryECInstanceBase::_CopyInstanceProperties
(
ECN::IECInstanceCR     fromNativeInstance
)
    {
    MemoryECInstanceBase* fromMemoryInstance = fromNativeInstance.GetAsMemoryECInstance();
    if (NULL != fromMemoryInstance && GetClassLayout().Equals (fromMemoryInstance->GetClassLayout()))
        {
        SetUsageBitmask (fromMemoryInstance->GetUsageBitmask());
        memcpy (m_perPropertyFlagsHolder.perPropertyFlags, fromMemoryInstance->GetPerPropertyFlagsData(), m_perPropertyFlagsHolder.numPerPropertyFlagsEntries * sizeof(UInt32));
        }

    return CopyInstancePropertiesToBuffer (fromNativeInstance);
    }

///////////////////////////////////////////////////////////////////////////////////////////
//  StandaloneECInstance
///////////////////////////////////////////////////////////////////////////////////////////
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Bill.Steinbock                  12/2010
+---------------+---------------+---------------+---------------+---------------+------*/
size_t                StandaloneECInstance::_GetOffsetToIECInstance () const
    {
    ECN::IECInstanceCP iecInstanceP   = dynamic_cast<ECN::IECInstanceCP>(this);
    byte const* baseAddressOfIECInstance = (byte const *)iecInstanceP;
    byte const* baseAddressOfConcrete = (byte const *)this;

    return (size_t)(baseAddressOfIECInstance - baseAddressOfConcrete);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    CaseyMullen     09/09
+---------------+---------------+---------------+---------------+---------------+------*/        
StandaloneECInstance::StandaloneECInstance (StandaloneECEnablerR enabler, byte * data, UInt32 size) :
        MemoryECInstanceBase (data, size, enabler.GetClassLayout(), true),
        m_sharedWipEnabler(&enabler)
    {
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    CaseyMullen     01/10
+---------------+---------------+---------------+---------------+---------------+------*/        
StandaloneECInstance::StandaloneECInstance (StandaloneECEnablerR enabler, UInt32 minimumBufferSize) :
        MemoryECInstanceBase (enabler.GetClassLayout(), minimumBufferSize, true),
        m_sharedWipEnabler(&enabler)
    {
    }
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    CaseyMullen     01/10
+---------------+---------------+---------------+---------------+---------------+------*/        
StandaloneECInstance::~StandaloneECInstance ()
    {
    //ECObjectsLogger::Log()->tracev (L"StandaloneECInstance at 0x%x is being destructed. It references enabler 0x%x", this, m_sharedWipEnabler);
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
    if (ECOBJECTS_STATUS_Success != newInstance->CopyInstanceProperties (instance))
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
* @bsimethod                                                    JoshSchifter    05/10
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus           StandaloneECInstance::_GetValue (ECValueR v, UInt32 propertyIndex, bool useArrayIndex, UInt32 arrayIndex) const
    {
    v.Clear ();

    ClassLayoutCR classLayout = GetClassLayout();

    ECObjectsStatus status = GetValueFromMemory (v, classLayout, propertyIndex, useArrayIndex, arrayIndex);
    if (ECOBJECTS_STATUS_Success == status)
        {
        bool isSet = false;
        if (ECOBJECTS_STATUS_Success == MemoryECInstanceBase::IsPerPropertyBitSet (isSet, (UInt8) PROPERTYFLAGINDEX_IsLoaded, propertyIndex)) 
            v.SetIsLoaded (isSet);
        }

    return status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    CaseyMullen     09/09
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus           StandaloneECInstance::_GetIsPropertyNull (bool& isNull, UInt32 propertyIndex, bool useArrayIndex, UInt32 arrayIndex) const
    {
    ClassLayoutCR classLayout = GetClassLayout();

    return GetIsNullValueFromMemory (classLayout, isNull, propertyIndex, useArrayIndex, arrayIndex);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    05/10
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus           StandaloneECInstance::_SetValue (UInt32 propertyIndex, ECValueCR v, bool useArrayIndex, UInt32 arrayIndex)
    {
    ClassLayoutCR classLayout = GetClassLayout();

    SetIsLoadedBit (propertyIndex);

    return SetValueToMemory (classLayout, propertyIndex, v, useArrayIndex, arrayIndex);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   09/12
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus StandaloneECInstance::_SetInternalValue (UInt32 propertyIndex, ECValueCR v, bool useArrayIndex, UInt32 arrayIndex)
    {
    SetIsLoadedBit (propertyIndex);
    return SetValueInternal (propertyIndex, v, useArrayIndex, arrayIndex);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Adam.Klatzkin                   01/2010
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus           StandaloneECInstance::_InsertArrayElements (UInt32 propIdx, UInt32 index, UInt32 size)
    {
    ClassLayoutCR classLayout = GetClassLayout();
    ECObjectsStatus status = InsertNullArrayElementsAt (classLayout, propIdx, index, size);
    
    return status;
    } 
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Adam.Klatzkin                   01/2010
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus           StandaloneECInstance::_AddArrayElements (UInt32 propIdx, UInt32 size)
    {
    ClassLayoutCR classLayout = GetClassLayout();    
    ECObjectsStatus status = AddNullArrayElementsAt (classLayout, propIdx, size);
    
    return status;
    }        

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Bill.Steinbock                  07/2011
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus           StandaloneECInstance::_RemoveArrayElement (UInt32 propIdx, UInt32 index)
    {
    ClassLayoutCR classLayout = GetClassLayout();    
    return RemoveArrayElementsAt (classLayout, propIdx, index, 1);
    } 

 /*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Adam.Klatzkin                   01/2010
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus           StandaloneECInstance::_ClearArray (UInt32 propIdx)
    {
    ClassLayoutCR classLayout = GetClassLayout();    
    PropertyLayoutCP pPropertyLayout = NULL;
    ECObjectsStatus status = classLayout.GetPropertyLayoutByIndex (pPropertyLayout, propIdx);
    if (SUCCESS != status || NULL == pPropertyLayout)
        return ECOBJECTS_STATUS_PropertyNotFound;

    UInt32 arrayCount = GetReservedArrayCount (*pPropertyLayout);
    if (arrayCount > 0)
        {
        status =  RemoveArrayElements (classLayout, *pPropertyLayout, 0, arrayCount);

        if (ECOBJECTS_STATUS_Success == status)
            SetPerPropertyBit ((UInt8) PROPERTYFLAGINDEX_IsLoaded, propIdx, false);

        return  status;
        }

    return ECOBJECTS_STATUS_Success;
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
    BeAssert (NULL != &ecClass);
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
    return L"Bentley::ECN::StandaloneECEnabler";
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
* @bsimethod                                                    Paul.Connelly   08/12
+---------------+---------------+---------------+---------------+---------------+------*/
bool StandaloneECEnabler::_IsPropertyReadOnly (UInt32 propertyIndex) const
    {
    return GetClassLayout().IsPropertyReadOnly (propertyIndex);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    CaseyMullen     09/09
+---------------+---------------+---------------+---------------+---------------+------*/        
StandaloneECInstancePtr   StandaloneECEnabler::CreateInstance (UInt32 minimumBufferSize)
    {
    return new StandaloneECInstance (*this, minimumBufferSize);
    }    
    
END_BENTLEY_ECOBJECT_NAMESPACE
