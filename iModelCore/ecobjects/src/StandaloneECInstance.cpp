/*--------------------------------------------------------------------------------------+
|
|     $Source: src/StandaloneECInstance.cpp $
|
|   $Copyright: (c) 2012 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ECObjectsPch.h"

BEGIN_BENTLEY_EC_NAMESPACE

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
    m_data = data;
    m_structInstances = NULL;
    m_usingSharedMemory = false;
    m_parentInstance = parentInstance;

    InitializePerPropertyFlags (classLayout, DEFAULT_NUMBITSPERPROPERTY);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Bill.Steinbock                  04/2010
+---------------+---------------+---------------+---------------+---------------+------*/
MemoryECInstanceBase::MemoryECInstanceBase (ClassLayoutCR classLayout, UInt32 minimumBufferSize, bool allowWritingDirectlyToInstanceMemory, MemoryECInstanceBase const* parentInstance) :
        MemoryInstanceSupport (allowWritingDirectlyToInstanceMemory),
        m_bytesAllocated(0)
    {
    m_structInstances = NULL;
    m_data = NULL;
    m_usingSharedMemory = false;

    m_parentInstance = parentInstance;

#if defined (_WIN32) // WIP_NONPORT
    UInt32 size = MAX (minimumBufferSize, classLayout.GetSizeOfFixedSection());
#elif defined (__unix__)
    // *** NEEDS WORK: When you stop including Windows.h, you can use this for both platforms:
    UInt32 size = std::max (minimumBufferSize, classLayout.GetSizeOfFixedSection());
#endif    
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
        return status;
                 
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
    if (m_structInstances)
        m_structInstances->clear ();

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
        status = GetPrimitiveValueFromMemory (v, propertyLayout, true, removeIndex+i);
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
ECObjectsStatus      MemoryECInstanceBase::AddNullArrayElements (WCharCP propertyAccessString, UInt32 insertCount)
    {
    return AddNullArrayElementsAt (GetClassLayout(), propertyAccessString, insertCount);
    }
	
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Paul.Connelly                   10/11
+---------------+---------------+---------------+---------------+---------------+------*/
MemoryECInstanceBase const* MemoryECInstanceBase::GetParentInstance () const
    {
    return m_parentInstance;
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
ECObjectsStatus           StandaloneECInstance::_InsertArrayElements (WCharCP propertyAccessString, UInt32 index, UInt32 size)
    {
    ClassLayoutCR classLayout = GetClassLayout();
    ECObjectsStatus status = InsertNullArrayElementsAt (classLayout, propertyAccessString, index, size);
    
    return status;
    } 
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Adam.Klatzkin                   01/2010
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus           StandaloneECInstance::_AddArrayElements (WCharCP propertyAccessString, UInt32 size)
    {
    ClassLayoutCR classLayout = GetClassLayout();    
    ECObjectsStatus status = AddNullArrayElementsAt (classLayout, propertyAccessString, size);
    
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