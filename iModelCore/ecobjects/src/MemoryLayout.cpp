/*--------------------------------------------------------------------------------------+
|
|     $Source: ecobjects/native/MemoryLayout.cpp $
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
    
    For an array of variable-sized values, we follow the same pattern as the overall MemoryInstance
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
    
    wprintf (L"ECClass name=%s\n", ecClass.GetName());
    Dump();
    
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
MemoryInstance::MemoryInstance (ClassCR ecClass) : Instance (GetEnablerForClass(ecClass)), 
                                                   m_bytesAllocated(0), m_bytesUsed(0), m_data(NULL) 
    {
    wchar_t id[256];
    swprintf(id, sizeof(id)/sizeof(wchar_t), L"%s-0x%X", ecClass.GetName(), this);
    m_instanceID = id;
    
    MemoryEnablerCP memoryEnabler = (MemoryEnablerCP)GetEnabler();
    DEBUG_EXPECT (NULL != dynamic_cast<MemoryEnablerCP>(GetEnabler()));
    
    memoryEnabler->InitializeInstanceMemory (*this);
    }
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    CaseyMullen     10/09
+---------------+---------------+---------------+---------------+---------------+------*/
MemoryEnablerCR MemoryInstance::GetMemoryEnabler() const
    {
    EnablerCP enabler = GetEnabler();
    DEBUG_EXPECT (NULL != enabler);
    return *(MemoryEnablerCP)enabler;
    }
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    CaseyMullen     10/09
+---------------+---------------+---------------+---------------+---------------+------*/
EnablerCR       MemoryInstance::GetEnablerForClass (ClassCR ecClass)
    {
    MemoryEnablerPtr enabler = MemoryEnabler::Create (ecClass); // WIP_FUSION: hack! We certainly don't want to create a new one every time... and we prefer to not have to even look it up, again
    enabler->AddRef(); // WIP_FUSION: hack... 
    return *enabler;
    }    
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    CaseyMullen     10/09
+---------------+---------------+---------------+---------------+---------------+------*/    
std::wstring    MemoryInstance::_GetInstanceID() const
    {
    return m_instanceID;
    }
       
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    CaseyMullen     10/09
+---------------+---------------+---------------+---------------+---------------+------*/
void            MemoryInstance::AllocateBytes (UInt32 minimumBytesToAllocate)
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
void            MemoryInstance::GrowAllocation (UInt32 bytesNeeded)
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
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    CaseyMullen     10/09
+---------------+---------------+---------------+---------------+---------------+------*/
byte *          MemoryEnabler::GetAddressOfValue (IMemoryProvider const & memoryProvider, PropertyLayoutCR propertyLayout) const
    {
    size_t offset = propertyLayout.GetOffset();
        
    byte * data = memoryProvider.GetData();
    byte * pValue = data + offset;
    
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
StatusInt       MemoryEnabler::EnsureSpaceIsAvailable (IMemoryProvider & memoryProvider, PropertyLayoutCR propertyLayout, UInt32 bytesNeeded) const
    {
    SecondaryOffset* pSecondaryOffset = (SecondaryOffset*)(memoryProvider.GetData() + propertyLayout.GetOffset());
    SecondaryOffset* pNextSecondaryOffset = pSecondaryOffset + 1;
    UInt32 availableBytes = *pNextSecondaryOffset - *pSecondaryOffset;
    
#ifndef SHRINK_TO_FIT    
    if (bytesNeeded <= availableBytes)
        return SUCCESS;
#endif
    
    Int32 additionalBytesNeeded = bytesNeeded - availableBytes;
    Int32 additionalBytesAvailable = memoryProvider.GetBytesAllocated() - memoryProvider.GetBytesUsed();
    
    if (additionalBytesNeeded > additionalBytesAvailable)
        memoryProvider.GrowAllocation (additionalBytesNeeded);
    
    memoryProvider.AdjustBytesUsed(additionalBytesNeeded);
    
    ClassLayoutCR classLayout = GetClassLayout();
    classLayout.ShiftValueData(memoryProvider.GetData(), propertyLayout, additionalBytesNeeded);
    
    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    CaseyMullen     10/09
+---------------+---------------+---------------+---------------+---------------+------*/
void            MemoryEnabler::InitializeInstanceMemory (IMemoryProvider& memoryProvider) const
    {
    DEBUG_EXPECT (NULL == memoryProvider.GetData());
    
    ClassLayoutCR classLayout = GetClassLayout();
        
    DEBUG_EXPECT (0 == memoryProvider.GetBytesUsed());    
    UInt32 bytesUsed = classLayout.GetSizeOfFixedSection();
    
    memoryProvider.AdjustBytesUsed (bytesUsed);
    memoryProvider.AllocateBytes (bytesUsed + 1024);
    
    UInt32 bytesAllocated = memoryProvider.GetBytesAllocated();
    DEBUG_EXPECT (bytesAllocated >= bytesUsed + 1024);
    
    byte * data = memoryProvider.GetData();
    classLayout.InitializeMemoryForInstance (data, bytesAllocated);
    
    // WIP_FUSION: could initialize default values here.
    }
 
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    CaseyMullen     10/09
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt       MemoryEnabler::GetValue (ValueR v, InstanceCR instance, wchar_t const * propertyAccessString,
                            UInt32 nIndices, UInt32 const * indices) const
    {
    // WIP_FUSION: attempting an unsafe cast here caused problems. How expensive is this dynamic cast? Is there any way to avoid it? The ECInstance could cache it's value? Do it once and remember the offset from "this"?
    IMemoryProvider const* memoryProvider = dynamic_cast<IMemoryProvider const*>(&instance);
    PRECONDITION (NULL != memoryProvider && "Programmer Error: only use Instance implementing IMemoryProvider with the MemoryEnabler", ECOBJECTS_STATUS_PreconditionViolated);

    // WIP_FUSION: check null flags first!

    PropertyLayoutCP layout = NULL;
    StatusInt status = m_classLayout.GetPropertyLayout (layout, propertyAccessString);
    if (SUCCESS != status || NULL == layout)
        return ERROR; // WIP_FUSION ERROR_PropertyNotFound
        
    byte * pValue = GetAddressOfValue (*memoryProvider, *layout);
    
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
            v.SetString (pString);
            return SUCCESS;            
            }
        }
    
    POSTCONDITION (false && "datatype not implemented", ERROR);
    }
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    CaseyMullen     10/09
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt       MemoryEnabler::SetValue (InstanceR instance, wchar_t const * propertyAccessString, ValueCR v,
                                   UInt32 nIndices, UInt32 const * indices) const
    {
    IMemoryProvider* memoryProvider = dynamic_cast<IMemoryProvider*>(&instance);
    PRECONDITION (NULL != memoryProvider && "Programmer Error: only use Instance implementing IMemoryProvider with the MemoryEnabler", ECOBJECTS_STATUS_PreconditionViolated);

    PropertyLayoutCP layout = NULL;
    StatusInt status = m_classLayout.GetPropertyLayout (layout, propertyAccessString); // WIP_FUSION: If it only has one error, let it just return null
    if (SUCCESS != status || NULL == layout)
        return ERROR; // WIP_FUSION ERROR_PropertyNotFound
        
    if (layout->GetDataType() != v.GetDataType())
        return ERROR; // WIP_FUSION ERROR_DataTypeMismatch
    
    byte * pValue = GetAddressOfValue (*memoryProvider, *layout);
    
    switch (layout->GetDataType())
        {
        case DATATYPE_Integer32:
            {
            Int32 value = v.GetInteger();
            memcpy (pValue, &value, sizeof(value));
            return SUCCESS;
            }
        case DATATYPE_Long64:
            {
            Int64 value = v.GetLong();
            memcpy (pValue, &value, sizeof(value));
            return SUCCESS;
            }
        case DATATYPE_Double:
            {
            double value = v.GetDouble();
            memcpy (pValue, &value, sizeof(value));
            return SUCCESS;
            }       
        case DATATYPE_String:
            {
            wchar_t const * value = v.GetString();
            UInt32 bytesNeeded = (UInt32)(sizeof(wchar_t) * (wcslen(value) + 1));
            
            StatusInt status = EnsureSpaceIsAvailable (*memoryProvider, *layout, bytesNeeded);
            if (SUCCESS != status)
                return status;
                
            wchar_t * pString = (wchar_t *)pValue;
            memcpy (pString, value, bytesNeeded);

            return SUCCESS;            
            }
        default:
            {
            ECAssert(false && L"DataType not implemented");
            return ERROR;
            }
        }

    return SUCCESS;
    }        

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    CaseyMullen     10/09
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt       MemoryEnabler::CreateInstance (InstanceP& instance, ClassCR ecClass, wchar_t const * instanceId) const
    {
    instance = new MemoryInstance (ecClass);
    
    return SUCCESS;
    }

END_BENTLEY_EC_NAMESPACE