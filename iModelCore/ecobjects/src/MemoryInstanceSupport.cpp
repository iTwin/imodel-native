/*--------------------------------------------------------------------------------------+
|
|     $Source: ecobjects/native/MemoryInstanceSupport.cpp $
|
|   $Copyright: (c) 2009 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ECObjectsPch.h"

// SHRINK_TO_FIT will cause space reserved for variable-sized values to be reduced to the minimum upon every set operation.
// SHRINK_TO_FIT is not recommended and is mainly for testing. It it better to "compact" everything at once
#define SHRINK_TO_FIT 1 

BEGIN_BENTLEY_EC_NAMESPACE

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    CaseyMullen     10/09
+---------------+---------------+---------------+---------------+---------------+------*/
wchar_t const * GetDatatypeName (DataType datatype) // WIP_FUSION: move to Value.h or nearby
    {
    switch (datatype)
        {
        // WIP_FUSION: names should match the .NET ECObjects names
        case DATATYPE_Integer32:
            return L"integer32";
        case DATATYPE_String:
            return L"string";
        case DATATYPE_Double:
            return L"double";
        case DATATYPE_Long64:
            return L"long64";            
        default:
            return L"UNKNOWN!";
        }
    }
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    CaseyMullen     10/09
+---------------+---------------+---------------+---------------+---------------+------*/
std::wstring    PropertyLayout::ToString ()
    {
    wchar_t const * datatype = GetDatatypeName (m_dataType);
    
    wchar_t line[1024];
    swprintf (line, sizeof(line)/sizeof(wchar_t), L"%32s %16s offset=%3i nullflagsOffset=%3i, nullflagsBitmask=0x%04X", m_accessString.c_str(), datatype, m_offset, m_nullflagsOffset, m_nullflagsBitmask);
        
    return line;
    }
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    CaseyMullen     10/09
+---------------+---------------+---------------+---------------+---------------+------*/
bool            PropertyLayout::IsFixedSized () const
    {
    // WIP_FUSION: when we have m_modifiers, they will determine if it is fixed size... could have fixed size string or variable int (added at end)
    switch (m_dataType)
        {
        case DATATYPE_Integer32:
        case DATATYPE_Long64:
        case DATATYPE_Double:
            return true;
        case DATATYPE_String:
            return false;
        default:
            DEBUG_FAIL("Unknown datatype encountered");
            return false;
        }
    }
      
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    CaseyMullen     10/09
+---------------+---------------+---------------+---------------+---------------+------*/
UInt32          PropertyLayout::GetSizeInFixedSection () const
    {
    if (!IsFixedSized())
        return sizeof(SecondaryOffset);
        
    return ClassLayout::GetPropertyValueSize(m_dataType);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    CaseyMullen     10/09
+---------------+---------------+---------------+---------------+---------------+------*/
void            ClassLayout::Dump () const
    {
    wprintf (L"ECClass perFileId=%i\n", m_perFileClassID);
    for each (PropertyLayout layout in m_propertyLayouts)
        {
        wprintf (layout.ToString().c_str());
        wprintf (L"\n");
        }
    }
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    CaseyMullen     10/09
+---------------+---------------+---------------+---------------+---------------+------*/    
UInt32          ClassLayout::GetSizeOfFixedSection() const
    {
    if (m_sizeOfFixedSection != 0)
        return m_sizeOfFixedSection;
        
    PropertyLayoutCR lastPropertyLayout = m_propertyLayouts[m_propertyLayouts.size() - 1];
    
    UInt32  size = (UInt32)lastPropertyLayout.GetOffset() + lastPropertyLayout.GetSizeInFixedSection();
    
    if (!lastPropertyLayout.IsFixedSized())
        size += sizeof(SecondaryOffset); // There is one additional SecondaryOffset tracking the end of the last variable-sized value
    
    const_cast<ClassLayout*>(this)->m_sizeOfFixedSection = size;
    
    return m_sizeOfFixedSection;
    }
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    CaseyMullen     10/09
+---------------+---------------+---------------+---------------+---------------+------*/     
void            ClassLayout::InitializeMemoryForInstance(byte * data, UInt32 bytesAllocated) const
    {
    memset (data, 0, bytesAllocated);
    
    UInt32 sizeOfFixedSection = GetSizeOfFixedSection();
    
    UInt32 currentNullflagsOffset = 23; // Not a real offset... but guaranteed to differ from the first real offset
    for (UInt32 i = 0; i < m_propertyLayouts.size(); ++i)
        {
        PropertyLayoutCR layout = m_propertyLayouts[i];
        if (currentNullflagsOffset != layout.GetNullflagsOffset())
            {
            currentNullflagsOffset = layout.GetNullflagsOffset();
            NullflagsBitmask * nullflags = (NullflagsBitmask *)(data + currentNullflagsOffset);
            *nullflags = NULLFLAGS_BITMASK_AllOn;
            }
            
        if (!layout.IsFixedSized())
            {
            DEBUG_EXPECT (layout.GetOffset() % 4 == 0 && "We expect secondaryOffsets to be aligned on a 4 byte boundary");
            
            SecondaryOffset* secondaryOffset = (SecondaryOffset*)(data + layout.GetOffset());
            *secondaryOffset = sizeOfFixedSection;
            
            bool isLastLayout = (i + 1 == m_propertyLayouts.size());
            if (isLastLayout)
                {
                ++secondaryOffset; // A SecondaryOffset beyond the last one for a property value, holding the end of the variable-sized section
                *secondaryOffset = sizeOfFixedSection;
                }
            }
        }
    }
  
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    CaseyMullen     10/09
+---------------+---------------+---------------+---------------+---------------+------*/
UInt32          ClassLayout::GetBytesUsed(byte const * data) const
    {
    DEBUG_EXPECT (m_sizeOfFixedSection > 0 && "The ClassLayout has not been initialized");
    SecondaryOffset * pLast = (SecondaryOffset*)(data + m_sizeOfFixedSection - sizeof(SecondaryOffset));
    
    // pLast is the last offset, pointing to one byte beyond the used space, so it is equal to the number of bytes used, so far
    return *pLast;
    }
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    CaseyMullen     10/09
+---------------+---------------+---------------+---------------+---------------+------*/
void            ClassLayout::ShiftValueData(byte * data, PropertyLayoutCR propertyLayout, Int32 shiftBy) const
    {
    DEBUG_EXPECT (!propertyLayout.IsFixedSized() && "The propertyLayout should be that of the variable-sized property whose size is increasing");
    DEBUG_EXPECT (m_sizeOfFixedSection > 0 && "The ClassLayout has not been initialized");
    
    SecondaryOffset * pLast = (SecondaryOffset*)(data + m_sizeOfFixedSection - sizeof(SecondaryOffset));
    SecondaryOffset * pAdjusting = (SecondaryOffset*)(data + propertyLayout.GetOffset());
    SecondaryOffset * pCurrent = pAdjusting + 1; // start at the one AFTER the property whose value's size is adjusting
    DEBUG_EXPECT (pCurrent <= pLast);
    DEBUG_EXPECT ((*pCurrent - *pAdjusting + shiftBy) >= 0 && "shiftBy cannot be such that it would cause the adjusting property to shrink to a negative size");
    
    UInt32 bytesToMove = *pLast - *pCurrent;
    if (bytesToMove > 0)
        {
        byte * source = data + *pCurrent;
        byte * destination = source + shiftBy;
        memmove (destination, source, bytesToMove);
        }

    while (pCurrent <= pLast)
        {
        *pCurrent += shiftBy;
        ++pCurrent;
        }
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    CaseyMullen     10/09
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt       ClassLayout::SetClass (ClassCR ecClass)
    {
    // WIP_FUSION: iterate through the EC::Properties of the EC::Class and build the layout
    
    // WIP_FUSION: fake it til we make it
    /* Fake a layout for
    
        struct ManufacturerInfo
            {
            std::wstring    m_name;
            int             m_accountNo;
            };    
    
        int                             m_A;
        int                             m_AA;
        ::Int64                         m_C;
        std::wstring                    m_B;
        std::wstring                    m_S;
        int[]                           m_arrayOfInts;
        ManufacturerInfo                m_manufacturer;
        std::vector<std::wstring>       m_aliases;
        std::vector<ManufacturerInfo>   m_arrayOfStructs;
    */
    
    AddFixedSizeProperty (L"A",                      EC::DATATYPE_Integer32);
    AddFixedSizeProperty (L"AA",                     EC::DATATYPE_Integer32);
    AddFixedSizeProperty (L"C",                      EC::DATATYPE_Long64);
    AddFixedSizeProperty (L"D",                      EC::DATATYPE_Double);
    AddFixedSizeProperty (L"Manufacturer.AccountNo", EC::DATATYPE_Integer32);
    
    AddVariableSizeProperty (L"B",                   EC::DATATYPE_String);
    AddVariableSizeProperty (L"S",                   EC::DATATYPE_String);
    AddVariableSizeProperty (L"Manufacturer.Name",   EC::DATATYPE_String);
    
    //AddArrayProperty (L"ArrayOfInts",   EC::DATATYPE_String);
    /* WIP_FUSION: deal with array later.
    Arrays of structs will get sliced so that they appear to be arrays of a single value type.
    For example, an array of Manufacturer would show up as three PropertyLayout objects:
    ArrayOfManufacturers[]           // This only holds the count. It is fixed size. The count is the max count of any of the value arrays, below
    ArrayOfManufacturers[].Name      // This only holds "Name" values. It is variable sized unless the array has a hard upper limit.
    ArrayOfManufacturers[].AccountNo // This only holds "AccountNo" values. It is variable sized unless the array has a hard upper limit.
    
    For a given array of fixed-sized values, the memory layout will be: count, nullflags, element, element, element
    ...and we have random access to the elements.
    
    For an array of variable-sized values, we follow the same pattern as the overall StandaloneInstance
    and have a fixed-sized section (an array of SecondaryOffsets) followed by a variable-sized section holding the actual values.
    For efficiency, when adding array elements, you really, really want to reserve them in advance, otherwise every
    new element increases the size of the fixed-sized section, forcing a shift of the entire variable-sized section.
    
    Even though we "slice" arrays of structs, when we add a new 'struct value' to the array, we really need to keep all 
    of the 'slices' in synch, e.g. we can't increase the length of ArrayOfManufacturers[].Name without also increasing
    the length of ArrayOfManufacturers[].AccountNo. Hmmm... possibly they should all share the same count.
    
    After 32 elements we have to insert another 4 bytes of nullflags, which also complicates the calculation of offsets
    and the shifting of values in the case of arrays of variable-sized values.
    
    Multidimensional arrays:
    ArrayOfManufacturers[].Aliases[]
    For this 2D array, we essentially have an array (ArrayOfManufacturers) where every element is an array of strings
    (Aliases). In the outer array, we lookup the right array of Aliases. Within that array of Aliases, layout is 
    identical to a normal array of strings.
    
    The tricky part is that when the nested Aliases array needs more memory, it must propagate a memory request back to
    its outer array, which may in turn need to request more space for its "value", which may need to request more memory
    from the "instance", which may have to realloc and move the whole shebang, in order to accomplish it.
    
    Need to figure out how to really handle this recursively in a consistent way....
    
    */
    
    FinishLayout ();
    
    //wprintf (L"ECClass name=%s\n", ecClass.GetName());
    //Dump();
    
    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    CaseyMullen     10/09
+---------------+---------------+---------------+---------------+---------------+------*/  
StatusInt       ClassLayout::FinishLayout ()
    {
    m_state = Closed;
    for (UInt32 i = 0; i < m_propertyLayouts.size(); i++)
        {
        PropertyLayoutCP propertyLayout = &m_propertyLayouts[i];
        m_propertyLayoutLookup[propertyLayout->GetAccessString()] = propertyLayout;
        }
        
    return SUCCESS;
    }
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    CaseyMullen     10/09
+---------------+---------------+---------------+---------------+---------------+------*/    
UInt32          ClassLayout::GetPropertyValueSize (DataType datatype) // WIP_FUSION: Move to ECValue.h
    {
    switch (datatype)
        {
        case EC::DATATYPE_Integer32:
            return sizeof(Int32);
        case EC::DATATYPE_Long64:
            return sizeof(Int64);
        case EC::DATATYPE_Double:
            return sizeof(double);
        default:
            DEBUG_FAIL("Most datatypes have not yet been implemented... or perhaps you have passed in a variable-sized type.");
            return 0;
        }
    }
  
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    CaseyMullen     10/09
+---------------+---------------+---------------+---------------+---------------+------*/    
StatusInt       ClassLayout::AddProperty (wchar_t const * accessString, DataType datatype, size_t size)
    {
    UInt32  positionInCurrentNullFlags = m_nProperties % 32;
    NullflagsBitmask  nullflagsBitmask = 0x01 << positionInCurrentNullFlags;
    
    PropertyLayout propertyLayout (accessString, datatype, m_offset, m_nullflagsOffset, nullflagsBitmask);
    
    m_nProperties++;
    m_offset += (UInt32)size;
   
    if ((positionInCurrentNullFlags + 1) == 32)
        {
        // It is time to bump up the nullflags offset
        m_nullflagsOffset = m_offset;
        m_offset += sizeof(NullflagsBitmask);
        }
        
    m_propertyLayouts.push_back(propertyLayout);
    
    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    CaseyMullen     10/09
+---------------+---------------+---------------+---------------+---------------+------*/    
StatusInt       ClassLayout::AddFixedSizeProperty (wchar_t const * accessString, DataType datatype)
    {
    if (m_state != AcceptingFixedSizeProperties)
        return ERROR; // ClassLayoutNotAcceptingFixedSizeProperties
    
    size_t size = GetPropertyValueSize (datatype);
    
    return AddProperty (accessString, datatype, size);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    CaseyMullen     10/09
+---------------+---------------+---------------+---------------+---------------+------*/ 
StatusInt       ClassLayout::AddVariableSizeProperty (wchar_t const * accessString, DataType datatype)
    {
    if (m_state == AcceptingFixedSizeProperties)
        m_state = AcceptingVariableSizeProperties;
    else if (m_state != AcceptingVariableSizeProperties)
        return ERROR; // ClassLayoutNotAcceptingVariableSizeProperties  
        
    size_t size = sizeof(SecondaryOffset); // the offset will just point to this secondary offset

    return AddProperty (accessString, datatype, size);
    }  

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    CaseyMullen     10/09
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt       ClassLayout::GetPropertyLayout (PropertyLayoutCP & propertyLayout, wchar_t const * accessString) const
    {
    PropertyLayoutLookup::const_iterator it = m_propertyLayoutLookup.find(accessString);
    
    if (it == m_propertyLayoutLookup.end())
        {
        return ECOBJECTS_STATUS_PropertyNotFound;
        }
    else
        {
        propertyLayout = it->second;
        return SUCCESS;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    CaseyMullen     10/09
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt       ClassLayout::GetPropertyLayoutByIndex (PropertyLayoutCP & propertyLayout, UInt32 propertyIndex) const
    {
    ECAssert (propertyIndex < m_propertyLayouts.size());
    if (propertyIndex >= m_propertyLayouts.size())
        return ERROR; // WIP_FUSION PropertyIndexOutOfBounds
        
    propertyLayout = &m_propertyLayouts[propertyIndex];
    return SUCCESS;
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    CaseyMullen     10/09
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt       MemoryEnabler::CreateInstance (InstanceP& instance, ClassCR ecClass, wchar_t const * instanceId) const
    {
    instance = new StandaloneInstance (ecClass);
    
    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    CaseyMullen     10/09
+---------------+---------------+---------------+---------------+---------------+------*/
ClassLayoutCR   MemoryInstanceSupport::GetClassLayout() const
    {
    DEBUG_EXPECT (NULL != GetMemoryEnabler());
    return GetMemoryEnabler()->GetClassLayout();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    CaseyMullen     10/09
+---------------+---------------+---------------+---------------+---------------+------*/
byte const *    MemoryInstanceSupport::GetAddressOfValue (PropertyLayoutCR propertyLayout) const
    {
    UInt32 offset = propertyLayout.GetOffset();
    
    byte const * data = GetDataForRead();
    byte const * pValue = data + offset;
    
    if (propertyLayout.IsFixedSized())
        return pValue;
    
    SecondaryOffset secondaryOffset = (SecondaryOffset)(*pValue);
    DEBUG_EXPECT (0 != secondaryOffset && "The instance is not initialized!");
    
    pValue = data + secondaryOffset;
    return pValue;
    }
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    CaseyMullen     10/09
+---------------+---------------+---------------+---------------+---------------+------*/
UInt32          MemoryInstanceSupport::GetOffsetOfValue (PropertyLayoutCR propertyLayout) const
    {
    UInt32 offset = propertyLayout.GetOffset();
    
    if (propertyLayout.IsFixedSized())
        return offset;

    byte const * data = GetDataForRead();
    byte const * pValue = data + offset;
        
    SecondaryOffset secondaryOffset = (SecondaryOffset)(*pValue);
    DEBUG_EXPECT (0 != secondaryOffset && "The instance is not initialized!");
    
    return secondaryOffset;
    }    
        
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    CaseyMullen     10/09
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt       MemoryInstanceSupport::EnsureSpaceIsAvailable (PropertyLayoutCR propertyLayout, UInt32 bytesNeeded)
    {
    SecondaryOffset* pSecondaryOffset = (SecondaryOffset*)(GetDataForRead() + propertyLayout.GetOffset());
    SecondaryOffset* pNextSecondaryOffset = pSecondaryOffset + 1;
    UInt32 availableBytes = *pNextSecondaryOffset - *pSecondaryOffset;
    
#ifndef SHRINK_TO_FIT    
    if (bytesNeeded <= availableBytes)
        return SUCCESS;
#endif
    
    Int32 additionalBytesNeeded = bytesNeeded - availableBytes;
    Int32 additionalBytesAvailable = GetBytesAllocated() - GetBytesUsed();
    
    if (additionalBytesNeeded > additionalBytesAvailable)
        GrowAllocation (additionalBytesNeeded);
    
    AdjustBytesUsed(additionalBytesNeeded);
    
    ClassLayoutCR classLayout = GetClassLayout();
    classLayout.ShiftValueData(GetDataForWrite(), propertyLayout, additionalBytesNeeded);
    
    return SUCCESS;
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    CaseyMullen     10/09
+---------------+---------------+---------------+---------------+---------------+------*/
void            MemoryInstanceSupport::InitializeInstanceMemory ()
    {
    DEBUG_EXPECT (!IsMemoryInitialized());
    
    ClassLayoutCR classLayout = GetClassLayout();
        
    DEBUG_EXPECT (0 == GetBytesUsed());    
    UInt32 bytesUsed = classLayout.GetSizeOfFixedSection();
    
    AllocateBytes (bytesUsed + 1024); // AllocateBytes must preceed AdjustBytesUsed because AllocateBytes may be initializing a new XAttribute
    AdjustBytesUsed (bytesUsed);
    
    UInt32 bytesAllocated = GetBytesAllocated();
    DEBUG_EXPECT (bytesAllocated >= bytesUsed + 1024);
    
    byte * data = GetDataForWrite();
    classLayout.InitializeMemoryForInstance (data, bytesAllocated);
    
    // WIP_FUSION: could initialize default values here.
    }
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    CaseyMullen     09/09
+---------------+---------------+---------------+---------------+---------------+------*/
UInt32          MemoryInstanceSupport::GetBytesUsedFromInstanceMemory(byte const * data) const
    {
    return GetClassLayout().GetBytesUsed (data);
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    CaseyMullen     09/09
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt       MemoryInstanceSupport::GetValueFromMemory (ValueR v, const wchar_t * propertyAccessString, UInt32 nIndices, UInt32 const * indices) const
    {
    PRECONDITION (NULL != propertyAccessString, ECOBJECTS_STATUS_PreconditionViolated);
    PRECONDITION (Instance::AccessStringAndNIndicesAgree(propertyAccessString, nIndices, true), ECOBJECTS_STATUS_AccessStringDisagreesWithNIndices);
                
    // WIP_FUSION: check null flags first!
    IMemoryProviderCR memoryProvider = *this;

    PropertyLayoutCP layout = NULL;
    ClassLayoutCR classLayout = GetClassLayout();
    StatusInt status = classLayout.GetPropertyLayout (layout, propertyAccessString);
    if (SUCCESS != status || NULL == layout)
        return ERROR; // WIP_FUSION ERROR_PropertyNotFound
        
    byte const * pValue = GetAddressOfValue (*layout);
    
    switch (layout->GetDataType())
        {
        case DATATYPE_Integer32:
            {
            Int32 value;
            memcpy (&value, pValue, sizeof(value));
            v.SetInteger (value);
            return SUCCESS;
            }
        case DATATYPE_Long64:
            {
            Int64 value;
            memcpy (&value, pValue, sizeof(value));
            v.SetLong (value);
            return SUCCESS;
            }            
        case DATATYPE_Double:
            {
            double value;
            memcpy (&value, pValue, sizeof(value));
            v.SetDouble (value);
            return SUCCESS;
            }            
        case DATATYPE_String:
            {
            wchar_t * pString = (wchar_t *)pValue;
            v.SetString (pString, false); // WIP_FUSION: We are passing false for "makeDuplicateCopy" to avoid the allocation 
                                          // and copying... but how do make the caller aware of this? When do they need 
                                          // to be aware. The wchar_t* they get back would get invalidated if the 
                                          // XAttribute or other IMemoryProvider got reallocated, or the string got moved.
                                          // The caller must immediately use (e.g. marshal or copy) the returned value.
                                          // Optionally, the caller could ask the EC::Value to make a duplicate? 
            return SUCCESS;            
            }
        }
    
    POSTCONDITION (false && "datatype not implemented", ERROR);
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    CaseyMullen     09/09
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt       MemoryInstanceSupport::SetValueToMemory (const wchar_t * propertyAccessString, ValueCR v, UInt32 nIndices, UInt32 const * indices)
    {
    PRECONDITION (NULL != propertyAccessString, ECOBJECTS_STATUS_PreconditionViolated);
    PRECONDITION (Instance::AccessStringAndNIndicesAgree(propertyAccessString, nIndices, true), ECOBJECTS_STATUS_AccessStringDisagreesWithNIndices);
                
    IMemoryProviderR memoryProvider = *this;

    PropertyLayoutCP layout = NULL;
    ClassLayoutCR classLayout = GetClassLayout();
    StatusInt status = classLayout.GetPropertyLayout (layout, propertyAccessString); // WIP_FUSION: If it only has one error, let it just return null
    if (SUCCESS != status || NULL == layout)
        return ERROR; // WIP_FUSION ERROR_PropertyNotFound
        
    if (layout->GetDataType() != v.GetDataType())
        return ERROR; // WIP_FUSION ERROR_DataTypeMismatch
    
    UInt32 offset = GetOffsetOfValue (*layout);
    
    switch (layout->GetDataType())
        {
        case DATATYPE_Integer32:
            {
            Int32 value = v.GetInteger();
            return ModifyData (offset, &value, sizeof(value));
            }
        case DATATYPE_Long64:
            {
            Int64 value = v.GetLong();
            return ModifyData (offset, &value, sizeof(value));
            }
        case DATATYPE_Double:
            {
            double value = v.GetDouble();
            return ModifyData (offset, &value, sizeof(value));
            }       
        case DATATYPE_String:
            {
            wchar_t const * value = v.GetString();
            UInt32 bytesNeeded = (UInt32)(sizeof(wchar_t) * (wcslen(value) + 1));
            
            StatusInt status = EnsureSpaceIsAvailable (*layout, bytesNeeded);
            if (SUCCESS != status)
                return status;
                
            return ModifyData (offset, value, bytesNeeded);
            }
        default:
            {
            ECAssert(false && L"DataType not implemented");
            return ERROR;
            }
        }

    return SUCCESS;
    }        


END_BENTLEY_EC_NAMESPACE