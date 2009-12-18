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
    swprintf (line, sizeof(line)/sizeof(wchar_t), L"%32s %16s offset=%3i nullflagsOffset=%3i, nullflagsBitmask=0x%08.X", m_accessString.c_str(), datatype, m_offset, m_nullflagsOffset, m_nullflagsBitmask);
        
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
ClassLayout::ClassLayout() : m_state(AcceptingFixedSizeProperties), 
                             m_class(NULL),
                             m_classID(0),
                             m_nProperties(0), 
                             m_nullflagsOffset (0),
                             m_offset(sizeof(InstanceFlags)), // The first 32 bits are reserved for flags/future
                             m_sizeOfFixedSection(0)
    {
    
    };
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    CaseyMullen     10/09
+---------------+---------------+---------------+---------------+---------------+------*/
void            ClassLayout::Dump () const
    {
    wprintf (L"ECClassIndex=%i, ECClass.Name=%s\n", m_classID, m_className.c_str());
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
    UInt32 sizeOfFixedSection = GetSizeOfFixedSection();
    memset (data, 0, sizeof(InstanceFlags)); 
    
    UInt32 bitsPerMask = (sizeof(NullflagsBitmask) * 8);
    UInt32 nNullflagsBitmasks = (UInt32)m_propertyLayouts.size() / bitsPerMask;
    if ( (m_propertyLayouts.size() % bitsPerMask > 0))
        ++nNullflagsBitmasks;
    
    for (UInt32 i = 0; i < nNullflagsBitmasks; i++)
        {
        NullflagsBitmask * nullflags = (NullflagsBitmask *)(data + sizeof(InstanceFlags) + i * sizeof (NullflagsBitmask));
        *nullflags = NULLFLAGS_BITMASK_AllOn;
        }
            
    for (UInt32 i = 0; i < m_propertyLayouts.size(); ++i)
        {
        PropertyLayoutCR layout = m_propertyLayouts[i];
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
    if (m_sizeOfFixedSection == 0)
        return 0;

    SecondaryOffset * pLast = (SecondaryOffset*)(data + m_sizeOfFixedSection - sizeof(SecondaryOffset));
    
    // pLast is the last offset, pointing to one byte beyond the used space, so it is equal to the number of bytes used, so far
    return *pLast;
    }
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    CaseyMullen     10/09
+---------------+---------------+---------------+---------------+---------------+------*/
UInt16          ClassLayout::GetClassID() const
    {
    return m_classID;
    } 
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    CaseyMullen     12/09
+---------------+---------------+---------------+---------------+---------------+------*/    
std::wstring    ClassLayout::GetClassName() const
    {
    return m_className;
    }
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    CaseyMullen     10/09
+---------------+---------------+---------------+---------------+---------------+------*/
void            MemoryInstanceSupport::ShiftValueData(ClassLayoutCR classLayout, byte * data, PropertyLayoutCR propertyLayout, Int32 shiftBy)
    {
    DEBUG_EXPECT (0 != shiftBy && "It is a pointless waste of time to shift nothing");
    DEBUG_EXPECT (!propertyLayout.IsFixedSized() && "The propertyLayout should be that of the variable-sized property whose size is increasing");
    DEBUG_EXPECT (classLayout.GetSizeOfFixedSection() > 0 && "The ClassLayout has not been initialized");
    
    SecondaryOffset * pLast = (SecondaryOffset*)(data + classLayout.GetSizeOfFixedSection() - sizeof(SecondaryOffset));
    SecondaryOffset * pAdjusting = (SecondaryOffset*)(data + propertyLayout.GetOffset());
    SecondaryOffset * pCurrent = pAdjusting + 1; // start at the one AFTER the property whose value's size is adjusting
    DEBUG_EXPECT (pCurrent <= pLast);
    DEBUG_EXPECT ((*pCurrent - *pAdjusting + shiftBy) >= 0 && "shiftBy cannot be such that it would cause the adjusting property to shrink to a negative size");
    
    UInt32 bytesToMove = *pLast - *pCurrent;
    if (bytesToMove > 0)
        {
        byte * source = data + *pCurrent;
        byte * destination = source + shiftBy;
        memmove (destination, source, bytesToMove); // WIP_FUSION: not using the IMemoryProvider. Use Modify data
        }

    UInt32 sizeOfSecondaryOffsetsToShift = (UInt32)(((byte*)pLast - (byte*)pCurrent) + sizeof (SecondaryOffset));
    UInt32 nSecondaryOffsetsToShift = sizeOfSecondaryOffsetsToShift / sizeof(SecondaryOffset);
    SecondaryOffset * shiftedSecondaryOffsets = (SecondaryOffset*)alloca (sizeOfSecondaryOffsetsToShift);
    for (UInt32 i = 0; i < nSecondaryOffsetsToShift; i++)
    //while (pCurrent <= pLast) // WIP_FUSION: calculate the size of these to be changed, allocate a buffer on the stack for the new values, then use ModifyData
        {
        shiftedSecondaryOffsets[i] = pCurrent[i] + shiftBy;
        //*pCurrent += shiftBy; // WIP_FUSION: not using the IMemoryProvider
        //++pCurrent;
        }
        
    UInt32 offsetOfCurrent = (UInt32)((byte*)pCurrent - data);
    ModifyData (offsetOfCurrent, shiftedSecondaryOffsets, sizeOfSecondaryOffsetsToShift);
    }
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    CaseyMullen     10/09
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt       ClassLayout::SetClass (ClassCR ecClass, UInt16 classID)
    {
    m_class = &ecClass;
    m_classID = classID;
    m_className = ecClass.GetName(); // WIP_FUSION: remove this redundant information
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

    for (int i = 0; i < N_FINAL_STRING_PROPS_IN_FAKE_CLASS; i++)
        {
        wchar_t propertyName[66];
        swprintf (propertyName, L"Property%i", i);
        AddVariableSizeProperty (propertyName,       EC::DATATYPE_String);
        }
    
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
    
    //wprintf (L"ECClass name=%s\n", ecClass.GetName().c_str());
    //Dump();
    
    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    CaseyMullen     10/09
+---------------+---------------+---------------+---------------+---------------+------*/  
ClassCP         ClassLayout::GetClass () const
    {
    return m_class;
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
        
#ifndef NDEBUG
    DEBUG_EXPECT (m_nProperties == m_propertyLayouts.size());
    if (0 == m_propertyLayouts.size())
        return SUCCESS;
    
    UInt32 bitsPerMask = (sizeof(NullflagsBitmask) * 8);
    UInt32 nNullflagsBitmasks = (UInt32)m_propertyLayouts.size() / bitsPerMask;
    if ( (m_propertyLayouts.size() % bitsPerMask > 0) )
        ++nNullflagsBitmasks;
    
    DEBUG_EXPECT (m_propertyLayouts[0].m_offset == sizeof(InstanceFlags) + nNullflagsBitmasks * sizeof(NullflagsBitmask));
    for (UInt32 i = 0; i < m_propertyLayouts.size(); i++)
        {
        UInt32 expectedNullflagsOffset = (i / bitsPerMask * sizeof(NullflagsBitmask)) + sizeof(InstanceFlags);
        DEBUG_EXPECT (m_propertyLayouts[i].m_nullflagsOffset == expectedNullflagsOffset);
        }     
#endif
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
StatusInt       ClassLayout::AddProperty (wchar_t const * accessString, DataType datatype, size_t size) // WIP_FUSION: use UInt32 instead of size
    {
    UInt32  positionInCurrentNullFlags = m_nProperties % 32;
    NullflagsBitmask  nullflagsBitmask = 0x01 << positionInCurrentNullFlags;
    
    if (0 == positionInCurrentNullFlags)
        {
        // It is time to add a new set of Nullflags
        m_nullflagsOffset = m_nullflagsOffset + sizeof(NullflagsBitmask);
        m_offset += sizeof(NullflagsBitmask);
        for (UInt32 i = 0; i < m_propertyLayouts.size(); i++)
            m_propertyLayouts[i].m_offset += sizeof(NullflagsBitmask); // Offsets of already-added property layouts need to get bumped up
        } 

    PropertyLayout propertyLayout (accessString, datatype, m_offset, m_nullflagsOffset, nullflagsBitmask);

    m_nProperties++;
    m_offset += (UInt32)size;
   
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
UInt32          ClassLayout::GetPropertyCount () const
    {
    return m_nProperties;
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
    assert (propertyIndex < m_propertyLayouts.size());
    if (propertyIndex >= m_propertyLayouts.size())
        return ERROR; // WIP_FUSION PropertyIndexOutOfBounds
        
    propertyLayout = &m_propertyLayouts[propertyIndex];
    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    CaseyMullen     12/09
+---------------+---------------+---------------+---------------+---------------+------*/
MemoryEnablerSupport::MemoryEnablerSupport (ClassCR ecClass, UInt16 classID)
    {
    // FUSION_WIP: sometimes, this will be loaded from the file
    m_classLayout.SetClass(ecClass, classID);
    }
    
MemoryEnablerSupport::MemoryEnablerSupport (ClassLayoutCR classLayout) : m_classLayout (classLayout)
    {
    }    

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    CaseyMullen     12/09
+---------------+---------------+---------------+---------------+---------------+------*/
MemoryEnablerSupport::MemoryEnablerSupport (ClassCR ecClass, UInt16 classID, UInt32 enablerID, std::wstring name) 
    {
    m_classLayout.SetClass(ecClass, classID);
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
    
    SecondaryOffset secondaryOffset = *((SecondaryOffset*)pValue);
    DEBUG_EXPECT (0 != secondaryOffset && "The instance is not initialized!");
    
    pValue = data + secondaryOffset;
    return pValue;
    }
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    CaseyMullen     10/09
+---------------+---------------+---------------+---------------+---------------+------*/
bool            MemoryInstanceSupport::IsNull (PropertyLayoutCR propertyLayout) const
    {
    NullflagsBitmask const * nullflags = (NullflagsBitmask const *)(GetDataForRead() + propertyLayout.GetNullflagsOffset());
    return (0 != (*nullflags & propertyLayout.GetNullflagsBitmask()));
    }
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    CaseyMullen     10/09
+---------------+---------------+---------------+---------------+---------------+------*/
void            MemoryInstanceSupport::SetNull (PropertyLayoutCR propertyLayout, bool isNull)
    {
    NullflagsBitmask mask = propertyLayout.GetNullflagsBitmask();
    
    NullflagsBitmask * nullflags = (NullflagsBitmask *)(GetDataForWrite() + propertyLayout.GetNullflagsOffset());
    if (isNull && 0 == (*nullflags & mask))
        *nullflags |= mask; // turn on the null bit
    else if (!isNull && mask == (*nullflags & mask))
        *nullflags ^= mask; // turn off the null bit
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
    SecondaryOffset const * pSecondaryOffset = (SecondaryOffset const *)(data + offset);

    SecondaryOffset secondaryOffset = *pSecondaryOffset;
    DEBUG_EXPECT (0 != secondaryOffset && "The instance is not initialized!");
    
    return secondaryOffset;
    }    
        
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    CaseyMullen     10/09
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt       MemoryInstanceSupport::EnsureSpaceIsAvailable (ClassLayoutCR classLayout, PropertyLayoutCR propertyLayout, UInt32 bytesNeeded)
    {
    byte const *             data = GetDataForRead();
    SecondaryOffset*         pSecondaryOffset = (SecondaryOffset*)(data + propertyLayout.GetOffset());
    SecondaryOffset*         pNextSecondaryOffset = pSecondaryOffset + 1;
    UInt32 availableBytes = *pNextSecondaryOffset - *pSecondaryOffset;
    
#ifndef SHRINK_TO_FIT    
    if (bytesNeeded <= availableBytes)
        return SUCCESS;
#endif
    UInt32 bytesUsed = classLayout.GetBytesUsed(data);
    Int32 additionalBytesNeeded = bytesNeeded - availableBytes;
    
    if (additionalBytesNeeded <= 0)
        return SUCCESS;
        
    Int32 additionalBytesAvailable = GetBytesAllocated() - bytesUsed;
    
    if (additionalBytesNeeded > additionalBytesAvailable)
        GrowAllocation (additionalBytesNeeded - additionalBytesAvailable);
    
    byte * writeableData = GetDataForWrite();
    DEBUG_EXPECT (bytesUsed == classLayout.GetBytesUsed(writeableData));
    
    ShiftValueData(classLayout, writeableData, propertyLayout, additionalBytesNeeded);

    DEBUG_EXPECT (0 == bytesUsed || (bytesUsed + additionalBytesNeeded == classLayout.GetBytesUsed(writeableData)));
    
    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    CaseyMullen     10/09
+---------------+---------------+---------------+---------------+---------------+------*/
void            MemoryInstanceSupport::AllocateAndInitializeMemory (ClassLayoutCR classLayout)
    {
    DEBUG_EXPECT (!IsMemoryInitialized());
    
    UInt32 bytesUsed = classLayout.GetSizeOfFixedSection();
    
    AllocateBytes (bytesUsed);
    
    UInt32 bytesAllocated = GetBytesAllocated();
    
    byte * data = GetDataForWrite();
    classLayout.InitializeMemoryForInstance (data, bytesAllocated);
    
    // WIP_FUSION: could initialize default values here.
    }
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    CaseyMullen     12/09
+---------------+---------------+---------------+---------------+---------------+------*/
void            MemoryInstanceSupport::InitializeMemory(ClassLayoutCR classLayout, byte * data, UInt32 bytesAllocated) const
    {
    classLayout.InitializeMemoryForInstance (data, bytesAllocated);
    }
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    CaseyMullen     09/09
+---------------+---------------+---------------+---------------+---------------+------*/    
UInt32          MemoryInstanceSupport::GetBytesUsed (ClassLayoutCR classLayout, byte const * data) const
    {
    return classLayout.GetBytesUsed (data);
    }
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    CaseyMullen     09/09
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt       MemoryInstanceSupport::GetValueFromMemory (ValueR v, PropertyLayoutCR propertyLayout, UInt32 nIndices, UInt32 const * indices) const
    {
    if (IsNull(propertyLayout))
        {
        //v.Clear();
        v.SetToNull();
        return SUCCESS;
        }
        
    byte const * pValue = GetAddressOfValue (propertyLayout);
    
    switch (propertyLayout.GetDataType())
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
StatusInt       MemoryInstanceSupport::GetValueFromMemory (ClassLayoutCR classLayout, ValueR v, const wchar_t * propertyAccessString, UInt32 nIndices, UInt32 const * indices) const
    {
    PRECONDITION (NULL != propertyAccessString, ECOBJECTS_STATUS_PreconditionViolated);
    PRECONDITION (Instance::AccessStringAndNIndicesAgree(propertyAccessString, nIndices, true), ECOBJECTS_STATUS_AccessStringDisagreesWithNIndices);
                
    // WIP_FUSION: check null flags first!
    IMemoryProviderCR memoryProvider = *this;

    PropertyLayoutCP propertyLayout = NULL;
    StatusInt status = classLayout.GetPropertyLayout (propertyLayout, propertyAccessString);
    if (SUCCESS != status || NULL == propertyLayout)
        return ERROR; // WIP_FUSION ERROR_PropertyNotFound
        
    return GetValueFromMemory (v, *propertyLayout, nIndices, indices);
    }
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    CaseyMullen     09/09
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt       MemoryInstanceSupport::SetValueToMemory (ClassLayoutCR classLayout, const wchar_t * propertyAccessString, ValueCR v, UInt32 nIndices, UInt32 const * indices)
    {
    PRECONDITION (NULL != propertyAccessString, ECOBJECTS_STATUS_PreconditionViolated);
    PRECONDITION (Instance::AccessStringAndNIndicesAgree(propertyAccessString, nIndices, true), ECOBJECTS_STATUS_AccessStringDisagreesWithNIndices);
                
    IMemoryProviderR memoryProvider = *this;

    PropertyLayoutCP propertyLayout = NULL;
    StatusInt status = classLayout.GetPropertyLayout (propertyLayout, propertyAccessString); // WIP_FUSION: If it only has one error, let it just return null
    if (SUCCESS != status || NULL == propertyLayout)
        return ERROR; // WIP_FUSION ERROR_PropertyNotFound
        
    if (v.IsNull())
        {
        SetNull (*propertyLayout, true);
        return SUCCESS;
        }
        
    SetNull (*propertyLayout, false);
                
    if (propertyLayout->GetDataType() != v.GetDataType())
        return ERROR; // WIP_FUSION ERROR_DataTypeMismatch
    
    UInt32 offset = GetOffsetOfValue (*propertyLayout);
    
    switch (propertyLayout->GetDataType())
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
            
            StatusInt status = EnsureSpaceIsAvailable (classLayout, *propertyLayout, bytesNeeded);
            if (SUCCESS != status)
                return status;
                
            return ModifyData (offset, value, bytesNeeded);
            }
        default:
            {
            assert(false && L"DataType not implemented");
            return ERROR;
            }
        }

    return SUCCESS;
    }        

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    CaseyMullen     10/09
+---------------+---------------+---------------+---------------+---------------+------*/
void            MemoryInstanceSupport::DumpInstanceData (ClassLayoutCR classLayout) const
    {
    byte const * data = GetDataForRead();
    
    wprintf (L"ECClass=%s at address = 0x%8.0x\n", classLayout.GetClassName().c_str(), data);
    InstanceFlags flags = *(InstanceFlags*)data;
    wprintf (L"  [0x%8.0x][%4.d] InstanceFlags = 0x%08.x\n", data, 0, flags);
    
    UInt32 nProperties = classLayout.GetPropertyCount ();
    
    UInt32 bitsPerMask = (sizeof(NullflagsBitmask) * 8);
    UInt32 nNullflagsBitmasks = nProperties / bitsPerMask;
    if ( (nProperties % bitsPerMask) > 0)
        ++nNullflagsBitmasks;
    
    for (UInt32 i = 0; i < nNullflagsBitmasks; i++)
        {
        UInt32 offset = sizeof(InstanceFlags) + i * sizeof(NullflagsBitmask);
        byte const * address = offset + data;
        wprintf (L"  [0x%8.x][%4.d] Nullflags[%d] = 0x%8.x\n", address, offset, i, *(NullflagsBitmask*)(data + offset));
        }
    
    for (UInt32 i = 0; i < nProperties; i++)
        {
        PropertyLayoutCP propertyLayout;
        StatusInt status = classLayout.GetPropertyLayoutByIndex (propertyLayout, i);
        if (SUCCESS != status)
            {
            wprintf (L"Error (%d) returned while getting PropertyLayout #%d", status, i);
            return;
            }

        UInt32 offset = propertyLayout->GetOffset();
        byte const * address = data + offset;
            
        Value v;
        GetValueFromMemory (v, *propertyLayout, 0, NULL);
        std::wstring valueAsString = v.ToString();
           
        if (propertyLayout->IsFixedSized())
            wprintf (L"  [0x%8.x][%4.d] %s = %s\n", address, offset, propertyLayout->GetAccessString(), valueAsString.c_str());
        else
            {
            SecondaryOffset secondaryOffset = *(SecondaryOffset*)address;
            byte const * realAddress = data + secondaryOffset;
            
            wprintf (L"  [0x%8.x][%4.d] -> [0x%8.x][%4.d] %s = %s\n", address, offset, realAddress, secondaryOffset, propertyLayout->GetAccessString(), valueAsString.c_str());
            }
        }
        
    UInt32 offsetOfLast = classLayout.GetSizeOfFixedSection() - sizeof(SecondaryOffset);
    SecondaryOffset * pLast = (SecondaryOffset*)(data + offsetOfLast);
    wprintf (L"  [0x%8.x][%4.d] Offset of TheEnd = %d\n", pLast, offsetOfLast, *pLast);
    }
    
END_BENTLEY_EC_NAMESPACE