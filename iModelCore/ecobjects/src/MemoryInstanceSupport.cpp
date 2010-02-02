/*--------------------------------------------------------------------------------------+
|
|     $Source: ecobjects/native/MemoryInstanceSupport.cpp $
|
|   $Copyright: (c) 2010 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ECObjectsPch.h"
#include    <algorithm>

// SHRINK_TO_FIT will cause space reserved for variable-sized values to be reduced to the minimum upon every set operation.
// SHRINK_TO_FIT is not recommended and is mainly for testing. It it better to "compact" everything at once
#define SHRINK_TO_FIT 1 

BEGIN_BENTLEY_EC_NAMESPACE

const UInt32 BITS_PER_NULLFLAGSBITMASK = (sizeof(NullflagsBitmask) * 8);

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    12/09
+---------------+---------------+---------------+---------------+---------------+------*/
bool            PrimitiveTypeIsFixedSize (PrimitiveType primitiveType)
    {
    switch (primitiveType)
        {
        case PRIMITIVETYPE_Integer:
        case PRIMITIVETYPE_Long:
        case PRIMITIVETYPE_Double:
            return true;
        case PRIMITIVETYPE_String:
            return false;
        default:
            DEBUG_FAIL("Unsupported data type");
            return false;
        }
    }
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Adam.Klatzkin                   02/2010
+---------------+---------------+---------------+---------------+---------------+------*/    
bool        IsArrayOfFixedSizeElements (PropertyLayoutCR propertyLayout)
    {
    if (PrimitiveTypeIsFixedSize (propertyLayout.GetTypeDescriptor().GetPrimitiveType()))
        return true;
    else
        return false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    AdamKlatzkin    01/10
+---------------+---------------+---------------+---------------+---------------+------*/
inline UInt32   CalculateNumberNullFlagsBitmasks (UInt32 numberOfItems)
    {
    UInt32 nNullflagsBitmasks = numberOfItems / BITS_PER_NULLFLAGSBITMASK;
    if ( (numberOfItems % BITS_PER_NULLFLAGSBITMASK > 0) )
        ++nNullflagsBitmasks;
    return nNullflagsBitmasks;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    AdamKlatzkin    01/10
+---------------+---------------+---------------+---------------+---------------+------*/
inline void     InitializeNullFlags (NullflagsBitmask * nullFlagsStart, UInt32 numberOfGroups)
    {
    NullflagsBitmask* nullflagsLocation = nullFlagsStart;
    for (UInt32 i = 0; i < numberOfGroups; i++, nullflagsLocation++)
        *nullflagsLocation = NULLFLAGS_BITMASK_AllOn;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    AdamKlatzkin    01/10
+---------------+---------------+---------------+---------------+---------------+------*/
inline UInt32   CalculateFixedArraySize (UInt32 fixedCount, PrimitiveType primitiveType)
    {
    return (CalculateNumberNullFlagsBitmasks (fixedCount) * sizeof (NullflagsBitmask)) + 
        (fixedCount *ClassLayout::GetFixedPrimitiveValueSize(primitiveType));
    }  
            
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    CaseyMullen     10/09
+---------------+---------------+---------------+---------------+---------------+------*/
std::wstring    PropertyLayout::ToString ()
    {    
    std::wstring typeName;
    if ((m_typeDescriptor.IsPrimitive()) || (m_typeDescriptor.IsPrimitiveArray()))
        typeName = ECXml::GetPrimitiveTypeName (m_typeDescriptor.GetPrimitiveType());
    else
        typeName = L"struct";

    if (m_typeDescriptor.IsArray())
        typeName += L"[]";
    
    wchar_t line[1024];
    swprintf (line, sizeof(line)/sizeof(wchar_t), L"%32s %16s offset=%3i nullflagsOffset=%3i, nullflagsBitmask=0x%08.X", m_accessString.c_str(), typeName.c_str(), m_offset, m_nullflagsOffset, m_nullflagsBitmask);
        
    return line;
    }
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    CaseyMullen     10/09
+---------------+---------------+---------------+---------------+---------------+------*/
bool            PropertyLayout::IsFixedSized () const
    {
    // WIP_FUSION: when we have m_modifiers, they will determine if it is fixed size... could have fixed size string or variable int (added at end)
    return ( ( m_typeDescriptor.IsPrimitive() || 
               (m_typeDescriptor.IsPrimitiveArray() && (m_modifierFlags & ARRAYMODIFIERFLAGS_IsFixedCount))
             ) && PrimitiveTypeIsFixedSize (m_typeDescriptor.GetPrimitiveType()));
    }
      
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    CaseyMullen     10/09
+---------------+---------------+---------------+---------------+---------------+------*/
UInt32          PropertyLayout::GetSizeInFixedSection () const
    {
    if (!IsFixedSized())
        return sizeof(SecondaryOffset);

    if (m_typeDescriptor.IsPrimitive())        
        return ClassLayout::GetFixedPrimitiveValueSize(m_typeDescriptor.GetPrimitiveType());
    else if (m_typeDescriptor.IsPrimitiveArray())
        {
        UInt32 fixedCount = m_modifierData; // WIP_FUSION for now assume modifier data holds the count but I'm not sure if that is the right place for this.
        return CalculateFixedArraySize (fixedCount, m_typeDescriptor.GetPrimitiveType());
        }
        
    DEBUG_FAIL("Can not determine size in fixed section for datatype");
    return 0;
    }    

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    CaseyMullen     10/09
+---------------+---------------+---------------+---------------+---------------+------*/
ClassLayout::ClassLayout(SchemaLayoutCR schemaLayout) 
    : 
    m_schemaLayout (schemaLayout),
    m_state(AcceptingFixedSizeProperties), 
    m_class(NULL),
    m_classIndex(0),
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
    ILogger *logger = Logger::GetLogger();
    logger->tracev (L"ECClassIndex=%i, ECClass.Name=%s\n", m_classIndex, m_className.c_str());
    for each (PropertyLayout layout in m_propertyLayouts)
        {
        logger->tracev (layout.ToString().c_str());
        logger->tracev (L"\n");
        }
    }
        
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    CaseyMullen     10/09
+---------------+---------------+---------------+---------------+---------------+------*/    
UInt32          ClassLayout::GetSizeOfFixedSection() const
    {
    return m_sizeOfFixedSection;
    }
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    CaseyMullen     10/09
+---------------+---------------+---------------+---------------+---------------+------*/     
void            ClassLayout::InitializeMemoryForInstance(byte * data, UInt32 bytesAllocated) const
    {
    UInt32 sizeOfFixedSection = GetSizeOfFixedSection();
    memset (data, 0, sizeof(InstanceFlags)); 
    
    UInt32 nNullflagsBitmasks = CalculateNumberNullFlagsBitmasks ((UInt32)m_propertyLayouts.size());
    InitializeNullFlags ((NullflagsBitmask *)(data + sizeof (InstanceFlags)), nNullflagsBitmasks);
            
    bool isFirstVariableSizedProperty = true;
    for (UInt32 i = 0; i < m_propertyLayouts.size(); ++i)
        {
        PropertyLayoutCR layout = m_propertyLayouts[i];
        if (layout.IsFixedSized())
            {
            if (layout.GetTypeDescriptor().IsArray())
                {
                nNullflagsBitmasks = CalculateNumberNullFlagsBitmasks (layout.GetModifierData());
                InitializeNullFlags ((NullflagsBitmask *)(data + layout.GetOffset()), nNullflagsBitmasks);
                }            
            continue;
            }
        
        DEBUG_EXPECT (layout.GetOffset() % 4 == 0 && "We expect secondaryOffsets to be aligned on a 4 byte boundary");
            
        SecondaryOffset* secondaryOffset = (SecondaryOffset*)(data + layout.GetOffset());

        if (isFirstVariableSizedProperty)
            {
            *secondaryOffset = sizeOfFixedSection;
            isFirstVariableSizedProperty = false;
            }
        else
            *secondaryOffset = 0; // Keep all but the first one zero until used
        
        bool isLastLayout = (i + 1 == m_propertyLayouts.size());
        if (isLastLayout)
            {
            ++secondaryOffset; // A SecondaryOffset beyond the last one for a property value, holding the end of the variable-sized section
            *secondaryOffset = sizeOfFixedSection;
            }
        }
    }
  
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    CaseyMullen     10/09
+---------------+---------------+---------------+---------------+---------------+------*/
UInt32          ClassLayout::CalculateBytesUsed(byte const * data) const
    {
    DEBUG_EXPECT (0 != m_sizeOfFixedSection);
    if (m_sizeOfFixedSection == 0)//GetSizeOfFixedSection
        return 0;
        
    PropertyLayoutCR lastPropertyLayout = m_propertyLayouts[m_propertyLayouts.size() - 1];  // WIP_FUSION: if _GetBytesUsed shows up in profiler, we may want to add bool m_hasVariableSizedValues;
    if (lastPropertyLayout.IsFixedSized())
        return m_sizeOfFixedSection;

    SecondaryOffset * pLast = (SecondaryOffset*)(data + m_sizeOfFixedSection - sizeof(SecondaryOffset));
    
    // pLast is the last offset, pointing to one byte beyond the used space, so it is equal to the number of bytes used, so far
    return *pLast;
    }
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    CaseyMullen     10/09
+---------------+---------------+---------------+---------------+---------------+------*/
UInt16          ClassLayout::GetClassIndex() const
    {
    return m_classIndex;
    } 
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    CaseyMullen     12/09
+---------------+---------------+---------------+---------------+---------------+------*/    
std::wstring    ClassLayout::GetClassName() const
    {
    return m_className;
    }
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    12/09
+---------------+---------------+---------------+---------------+---------------+------*/
void            ClassLayout::AddProperties (ECClassCR ecClass, wchar_t const * nameRoot, bool addingFixedSizeProps)
    {
    for each (ECPropertyP property in ecClass.GetProperties())
        {
        std::wstring    propName = property->GetName();
        
        if (NULL != nameRoot)
            propName = std::wstring (nameRoot) + L"." + propName;

        if (property->GetIsPrimitive())
            {
            PrimitiveECPropertyP  primitiveProp = property->GetAsPrimitiveProperty();
            PrimitiveType       primitiveType = primitiveProp->GetType();

            bool isFixedSize = PrimitiveTypeIsFixedSize(primitiveType);

            if (addingFixedSizeProps && isFixedSize)
                AddFixedSizeProperty (propName.c_str(), primitiveType);
            else
            if ( ! addingFixedSizeProps && ! isFixedSize)
                AddVariableSizeProperty (propName.c_str(), primitiveType);
            }
        else
        if (property->GetIsStruct())
            {
            StructECPropertyP  structProp = property->GetAsStructProperty();
            ECClassCR          nestedClass = structProp->GetType();
            
            AddProperties (nestedClass, propName.c_str(), addingFixedSizeProps);
            }
        else
        if (property->GetIsArray())
            {            
            ArrayECPropertyP  arrayProp = property->GetAsArrayProperty();            
            ArrayKind arrayKind = arrayProp->GetKind();
            if (arrayKind == ARRAYKIND_Primitive)
                {
                bool isFixedArrayCount = (arrayProp->GetMinOccurs() == arrayProp->GetMaxOccurs());
                bool isFixedPropertySize = isFixedArrayCount && PrimitiveTypeIsFixedSize (arrayProp->GetPrimitiveElementType());
                
                if (addingFixedSizeProps && isFixedPropertySize)
                    AddFixedSizeArrayProperty (propName.c_str(), ECTypeDescriptor::CreatePrimitiveArrayTypeDescriptor (arrayProp->GetPrimitiveElementType()), arrayProp->GetMinOccurs());
                else if (!addingFixedSizeProps && !isFixedPropertySize)
                    {
                    if (isFixedArrayCount)
                        AddVariableSizeArrayPropertyWithFixedCount (propName.c_str(), ECTypeDescriptor::CreatePrimitiveArrayTypeDescriptor (arrayProp->GetPrimitiveElementType()), arrayProp->GetMinOccurs());
                    else
                        AddVariableSizeProperty (propName.c_str(), ECTypeDescriptor::CreatePrimitiveArrayTypeDescriptor (arrayProp->GetPrimitiveElementType()));
                    }   
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

            For an array of variable-sized values, we follow the same pattern as the overall StandaloneECInstance
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

            }
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    01/10
+---------------+---------------+---------------+---------------+---------------+------*/
ClassLayoutP    ClassLayout::CreateEmpty (ECClassCR ecClass, ClassIndex classIndex, SchemaLayoutCR schemaLayout)
    {
    ClassLayoutP classLayout = new ClassLayout(schemaLayout);

    classLayout->SetClass (ecClass, classIndex);

    return classLayout;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    01/10
+---------------+---------------+---------------+---------------+---------------+------*/
ClassLayoutP    ClassLayout::BuildFromClass (ECClassCR ecClass, ClassIndex classIndex, SchemaLayoutCR schemaLayout)
    {
    ClassLayoutP classLayout = CreateEmpty (ecClass, classIndex, schemaLayout);

    // Iterate through the EC::Properties of the EC::Class and build the layout
    classLayout->AddProperties (ecClass, NULL, true);
    classLayout->AddProperties (ecClass, NULL, false);

    classLayout->FinishLayout ();
    
    //wprintf (L"ECClass name=%s\n", ecClass.GetName().c_str());
    //Dump();

    return classLayout;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    CaseyMullen     10/09
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   ClassLayout::SetClass (ECClassCR ecClass, UInt16 classIndex)
    {
    m_class      = &ecClass;
    m_classIndex = classIndex;
    m_className  = ecClass.GetName(); // WIP_FUSION: remove this redundant information

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    CaseyMullen     10/09
+---------------+---------------+---------------+---------------+---------------+------*/  
ECClassCR       ClassLayout::GetClass () const
    {
    return *m_class;
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
        
    // Calculate size of fixed section    
    if (0 >= m_propertyLayouts.size())
        {
        m_sizeOfFixedSection = 0;
        }
    else
        {
        PropertyLayoutCR lastPropertyLayout = m_propertyLayouts[m_propertyLayouts.size() - 1];

        UInt32  size = (UInt32)lastPropertyLayout.GetOffset() + lastPropertyLayout.GetSizeInFixedSection();

        if (!lastPropertyLayout.IsFixedSized())
            size += sizeof(SecondaryOffset); // There is one additional SecondaryOffset tracking the end of the last variable-sized value

        m_sizeOfFixedSection = size;
        }
            
#ifndef NDEBUG
    DEBUG_EXPECT (m_nProperties == m_propertyLayouts.size());
    if (0 == m_propertyLayouts.size())
        return SUCCESS;
    
    UInt32 nNullflagsBitmasks = CalculateNumberNullFlagsBitmasks ((UInt32)m_propertyLayouts.size());
    
    DEBUG_EXPECT (m_propertyLayouts[0].m_offset == sizeof(InstanceFlags) + nNullflagsBitmasks * sizeof(NullflagsBitmask));
    for (UInt32 i = 0; i < m_propertyLayouts.size(); i++)
        {
        UInt32 expectedNullflagsOffset = (i / BITS_PER_NULLFLAGSBITMASK * sizeof(NullflagsBitmask)) + sizeof(InstanceFlags);
        DEBUG_EXPECT (m_propertyLayouts[i].m_nullflagsOffset == expectedNullflagsOffset);
        }     
#endif
    return SUCCESS;
    }
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    CaseyMullen     10/09
+---------------+---------------+---------------+---------------+---------------+------*/    
UInt32          ClassLayout::GetFixedPrimitiveValueSize (PrimitiveType primitivetype) // WIP_FUSION: Move to ECValue.h
    {
    switch (primitivetype)
        {
        case EC::PRIMITIVETYPE_Integer:
            return sizeof(Int32);
        case EC::PRIMITIVETYPE_Long:
            return sizeof(Int64);
        case EC::PRIMITIVETYPE_Double:
            return sizeof(double);
        default:
            DEBUG_FAIL("Most datatypes have not yet been implemented... or perhaps you have passed in a variable-sized type.");
            return 0;
        }
    }
  
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    CaseyMullen     10/09
+---------------+---------------+---------------+---------------+---------------+------*/    
StatusInt       ClassLayout::AddProperty (wchar_t const * accessString, ECTypeDescriptor typeDescriptor, UInt32 size, UInt32 modifierFlags, UInt32 modifierData)
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

    // WIP_FUSION, for now all accessors of array property layouts are stored with the brackets appended.  This means all access to array values through an
    // IECInstance must include the brackets.  If you want to obtain an array element value then you specify an index.  If you want to obtain an array info value
    // then you do not specify an index.  I'd like to consider an update to this so if an access string does not include the [] then we always return the ArrayInfo value.
    std::wstring tempAccessString = accessString;
    if (typeDescriptor.IsArray())
        tempAccessString += L"[]";            

    PropertyLayout propertyLayout (tempAccessString.c_str(), typeDescriptor, m_offset, m_nullflagsOffset, nullflagsBitmask, modifierFlags, modifierData);

    m_nProperties++;
    m_offset += (UInt32)size;
   
    m_propertyLayouts.push_back(propertyLayout);
    
    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    CaseyMullen     10/09
+---------------+---------------+---------------+---------------+---------------+------*/    
StatusInt       ClassLayout::AddFixedSizeProperty (wchar_t const * accessString, ECTypeDescriptor typeDescriptor)
    {
    PRECONDITION (m_state == AcceptingFixedSizeProperties, ERROR); // ClassLayoutNotAcceptingFixedSizeProperties    
    
    if (!typeDescriptor.IsPrimitive())
        {
        DEBUG_FAIL ("We currently only support fixed sized properties for primitive types");
        return ERROR;
        }
    
    UInt32 size = GetFixedPrimitiveValueSize (typeDescriptor.GetPrimitiveType());
    
    return AddProperty (accessString, typeDescriptor, size);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Adam.Klatzkin                   01/2010
+---------------+---------------+---------------+---------------+---------------+------*/ 
StatusInt       ClassLayout::AddFixedSizeArrayProperty (wchar_t const * accessString, ECTypeDescriptor typeDescriptor, UInt32 arrayCount)
    {
    PRECONDITION (m_state == AcceptingFixedSizeProperties, ERROR); // ClassLayoutNotAcceptingFixedSizeProperties    
    
    UInt32 size = CalculateFixedArraySize (arrayCount, typeDescriptor.GetPrimitiveType());
    
    return AddProperty (accessString, typeDescriptor, size, ARRAYMODIFIERFLAGS_IsFixedCount, arrayCount);
    }
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Adam.Klatzkin                   01/2010
+---------------+---------------+---------------+---------------+---------------+------*/ 
StatusInt       ClassLayout::AddVariableSizeArrayPropertyWithFixedCount (wchar_t const * accessString, ECTypeDescriptor typeDescriptor, UInt32 arrayCount)
    {
    if (m_state == AcceptingFixedSizeProperties)
        m_state = AcceptingVariableSizeProperties;
    else if (m_state != AcceptingVariableSizeProperties)
        return ERROR; // ClassLayoutNotAcceptingVariableSizeProperties  
        
    UInt32 size = sizeof(SecondaryOffset); // the offset will just point to this secondary offset
    
    return AddProperty (accessString, typeDescriptor, size, ARRAYMODIFIERFLAGS_IsFixedCount, arrayCount);
    }    

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    CaseyMullen     10/09
+---------------+---------------+---------------+---------------+---------------+------*/ 
StatusInt       ClassLayout::AddVariableSizeProperty (wchar_t const * accessString, ECTypeDescriptor typeDescriptor)
    {
    if (m_state == AcceptingFixedSizeProperties)
        m_state = AcceptingVariableSizeProperties;
    else if (m_state != AcceptingVariableSizeProperties)
        return ERROR; // ClassLayoutNotAcceptingVariableSizeProperties  
        
    UInt32 size = sizeof(SecondaryOffset); // the offset will just point to this secondary offset

    return AddProperty (accessString, typeDescriptor, size);
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
* @bsimethod                                                    JoshSchifter    01/10
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   SchemaLayout::AddClassLayout (ClassLayoutCR classLayout, ClassIndex classIndex, bool isPersistent)
    {
    if (m_entries.size() <= classIndex)
        m_entries.resize (20 + classIndex);

    assert (NULL == m_entries[classIndex] && "Class Index is already in use");

    m_entries[classIndex] = new SchemaLayoutEntry (classLayout, isPersistent);

    return ERROR;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    01/10
+---------------+---------------+---------------+---------------+---------------+------*/
SchemaLayoutEntry*  SchemaLayout::GetEntry (ClassIndex classIndex)
    {
    if (m_entries.size() <= classIndex)
        return NULL;

    return m_entries[classIndex];
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    01/10
+---------------+---------------+---------------+---------------+---------------+------*/
SchemaLayoutEntry*  SchemaLayout::FindEntry (ECClassCR ecClass)
    {
    for each (SchemaLayoutEntry* entry in m_entries)
        {
        if (NULL == entry)
            continue;

        if (&entry->m_classLayout.GetClass() == &ecClass)
            return entry;
        }

    return NULL;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    01/10
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   SchemaLayout::FindAvailableClassIndex(ClassIndex& classIndex)
    {
    SchemaLayoutEntry* nullVal = NULL;
    SchemaLayoutEntryArray::iterator iter = std::find (m_entries.begin(), m_entries.end(), nullVal);

    size_t firstNullIndex = iter - m_entries.begin();

    if (USHRT_MAX > firstNullIndex)
        { 
        classIndex = (UInt16) firstNullIndex;
        return SUCCESS;
        }

    // The max size for classIndex is 0xffff, but if we reach that limit,
    // most likely something else has gone wrong.
    assert(false);
    return ERROR;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    CaseyMullen     12/09
+---------------+---------------+---------------+---------------+---------------+------*/
ClassLayoutHolder::ClassLayoutHolder (ClassLayoutCR classLayout) : m_classLayout (classLayout)
    {
    }    

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    CaseyMullen     12/09
+---------------+---------------+---------------+---------------+---------------+------*/
ClassLayoutCR    ClassLayoutHolder::GetClassLayout() const 
    {
    return m_classLayout;
    }
    
static int s_shiftSecondaryOffsetsInPlace = false; 
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Adam.Klatzkin                   01/2010
+---------------+---------------+---------------+---------------+---------------+------*/    
UInt32          MemoryInstanceSupport::GetPropertyValueSize (PropertyLayoutCR propertyLayout) const
    {
    if (propertyLayout.IsFixedSized())
        return propertyLayout.GetSizeInFixedSection();
    else
        {
        SecondaryOffset* pSecondaryOffset = (SecondaryOffset*)(_GetDataForRead() + propertyLayout.GetOffset());
        SecondaryOffset* pNextOffset = pSecondaryOffset + 1;        
        return *pNextOffset - *pSecondaryOffset;        
        }
    }    

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    AdamKlatzkin    01/10
+---------------+---------------+---------------+---------------+---------------+------*/
UInt32          MemoryInstanceSupport::GetOffsetOfArray (PropertyLayoutCR propertyLayout) const
    {
    if (propertyLayout.IsFixedSized())
        {
        return propertyLayout.GetOffset();
        }
    else
        {
        SecondaryOffset* pSecondaryOffset = (SecondaryOffset*)(_GetDataForRead() + propertyLayout.GetOffset());        
        return *pSecondaryOffset;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    AdamKlatzkin    01/10
+---------------+---------------+---------------+---------------+---------------+------*/
ArrayCount      MemoryInstanceSupport::GetReservedArrayCount (PropertyLayoutCR propertyLayout) const
    {
    if (propertyLayout.GetModifierFlags() & ARRAYMODIFIERFLAGS_IsFixedCount)
        return propertyLayout.GetModifierData();
    else
        return GetAllocatedArrayCount (propertyLayout);
    }    
        
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    AdamKlatzkin    01/10
+---------------+---------------+---------------+---------------+---------------+------*/
ArrayCount      MemoryInstanceSupport::GetAllocatedArrayCount (PropertyLayoutCR propertyLayout) const
    {
    if (propertyLayout.IsFixedSized())
        return propertyLayout.GetModifierData();
    else
        {
        SecondaryOffset* pSecondaryOffset = (SecondaryOffset*)(_GetDataForRead() + propertyLayout.GetOffset());
        SecondaryOffset* pNextOffset = pSecondaryOffset + 1;        
        
        SecondaryOffset arrayOffset = *pSecondaryOffset;
        if ((arrayOffset == 0) || (*pNextOffset == 0) || (arrayOffset == *pNextOffset))
            return 0;

        byte const * pCount = _GetDataForRead() + arrayOffset;
        return *((ArrayCount*)pCount);
        }
    }    

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    CaseyMullen     10/09
+---------------+---------------+---------------+---------------+---------------+------*/
byte const *    MemoryInstanceSupport::GetAddressOfPrimitiveValue (PropertyLayoutCR propertyLayout, UInt32 nIndices, UInt32 const * indices) const
    {
    if (nIndices == 0)
        return _GetDataForRead() + GetOffsetOfPropertyValue (propertyLayout);
    else
        {
        UInt32 arrayOffset = GetOffsetOfArray (propertyLayout);   
        return _GetDataForRead() + GetOffsetOfArrayIndexValue (arrayOffset, propertyLayout, *indices);
        }
    } 

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    CaseyMullen     10/09
+---------------+---------------+---------------+---------------+---------------+------*/
UInt32          MemoryInstanceSupport::GetOffsetOfPrimitiveValue (PropertyLayoutCR propertyLayout, UInt32 nIndices, UInt32 const * indices) const
    {
    if (nIndices == 0)
        return GetOffsetOfPropertyValue (propertyLayout);
    else
        {
        UInt32 arrayOffset = GetOffsetOfArray (propertyLayout);   
        return GetOffsetOfArrayIndexValue (arrayOffset, propertyLayout, *indices);
        }
    } 

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Adam.Klatzkin                   01/2010
+---------------+---------------+---------------+---------------+---------------+------*/
void   MemoryInstanceSupport::PrepareToAccessNullFlags (UInt32& nullflagsOffset, UInt32& nullflagsBitmask, PropertyLayoutCR propertyLayout, UInt32 nIndices, UInt32 const * indices) const
    {
    // Since this is a private method and we are concerned about performance we do not do any parameter checking here.  nIndices must be 0 or 1
    // and we assume if it is 1 that we are obtaining the value from a primitive array.  We also assume that count has already been validated against 
    // the specified index.   
    if (nIndices == 0)
        {
        nullflagsOffset = propertyLayout.GetNullflagsOffset();
        nullflagsBitmask = propertyLayout.GetNullflagsBitmask();
        }
    else if (nIndices == 1)
        {
        if (propertyLayout.IsFixedSized())
            nullflagsOffset = propertyLayout.GetOffset();
        else
            nullflagsOffset = *((SecondaryOffset*)(_GetDataForRead() + propertyLayout.GetOffset())) + sizeof (ArrayCount);

        nullflagsOffset += (*indices / BITS_PER_NULLFLAGSBITMASK * sizeof(NullflagsBitmask));
        nullflagsBitmask = 0x01 << (*indices % BITS_PER_NULLFLAGSBITMASK);
        }
    }
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    CaseyMullen     10/09
+---------------+---------------+---------------+---------------+---------------+------*/
bool            MemoryInstanceSupport::IsPropertyValueNull (PropertyLayoutCR propertyLayout,  UInt32 nIndices, UInt32 const * indices) const
    {
    UInt32 nullflagsOffset;
    UInt32 nullflagsBitmask;
    PrepareToAccessNullFlags (nullflagsOffset, nullflagsBitmask, propertyLayout, nIndices, indices);

    NullflagsBitmask const * nullflags = (NullflagsBitmask const *)(_GetDataForRead() + nullflagsOffset);
    return (0 != (*nullflags & nullflagsBitmask));    
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    CaseyMullen     10/09
+---------------+---------------+---------------+---------------+---------------+------*/
void            MemoryInstanceSupport::SetPropertyValueNull (PropertyLayoutCR propertyLayout,  UInt32 nIndices, UInt32 const * indices, bool isNull)
    {  
    UInt32 nullflagsOffset;
    UInt32 nullflagsBitmask;
    PrepareToAccessNullFlags (nullflagsOffset, nullflagsBitmask, propertyLayout, nIndices, indices);   
    
    NullflagsBitmask * nullflags = (NullflagsBitmask *)(_GetDataForWrite() + nullflagsOffset);
    if (isNull && 0 == (*nullflags & nullflagsBitmask))
        *nullflags |= nullflagsBitmask; // turn on the null bit
    else if (!isNull && nullflagsBitmask == (*nullflags & nullflagsBitmask))
        *nullflags ^= nullflagsBitmask; // turn off the null bit // WIP_FUSION: Needs to use ModifyData
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    CaseyMullen     10/09
+---------------+---------------+---------------+---------------+---------------+------*/
UInt32          MemoryInstanceSupport::GetOffsetOfPropertyValue (PropertyLayoutCR propertyLayout) const
    {
    UInt32 offset = propertyLayout.GetOffset();
    
    if (propertyLayout.IsFixedSized())
        return offset;

    byte const * data = _GetDataForRead();
    SecondaryOffset const * pSecondaryOffset = (SecondaryOffset const *)(data + offset);

    SecondaryOffset secondaryOffset = *pSecondaryOffset;
    
    return secondaryOffset;
    }       
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Adam.Klatzkin                   01/2010
+---------------+---------------+---------------+---------------+---------------+------*/
UInt32          MemoryInstanceSupport::GetOffsetOfArrayIndex (UInt32 arrayOffset, PropertyLayoutCR propertyLayout, UInt32 index) const
    {    
    // Since this is a private method and we are concerned about performance we do not do any release build parameter checking here.  
    // We assume that count has already been validated against the specified index.    
    DEBUG_EXPECT (propertyLayout.GetTypeDescriptor().IsArray());
    
    ArrayCount count = GetAllocatedArrayCount (propertyLayout);
    DEBUG_EXPECT (count > index);
    
    UInt32 primaryOffset;
    if (propertyLayout.IsFixedSized())
        primaryOffset = arrayOffset;
    else
        primaryOffset = arrayOffset + sizeof (ArrayCount);

    primaryOffset += (CalculateNumberNullFlagsBitmasks (count) * sizeof (NullflagsBitmask));

    if (IsArrayOfFixedSizeElements (propertyLayout))
        primaryOffset += (index * ClassLayout::GetFixedPrimitiveValueSize (propertyLayout.GetTypeDescriptor().GetPrimitiveType()));
    else
        primaryOffset += index * sizeof (SecondaryOffset);

    return primaryOffset;
    }  

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Adam.Klatzkin                   01/2010
+---------------+---------------+---------------+---------------+---------------+------*/
UInt32          MemoryInstanceSupport::GetOffsetOfArrayIndexValue (UInt32 arrayOffset, PropertyLayoutCR propertyLayout, UInt32 index) const
    {    
    UInt32 arrayIndexOffset = GetOffsetOfArrayIndex (arrayOffset, propertyLayout, index);

    if (IsArrayOfFixedSizeElements (propertyLayout))
        return arrayIndexOffset;
    
    byte const * data = _GetDataForRead();
    SecondaryOffset const * pSecondaryOffset = (SecondaryOffset const *)(data + arrayIndexOffset);

    SecondaryOffset secondaryOffset = *pSecondaryOffset;
    DEBUG_EXPECT (0 != secondaryOffset && "The instance is not initialized!");
    
    // The stored secondary offset of an array element is relative to the beginning of the array.  We expect this method to return
    // an offset to the array element that is relative to the start of the instance data.
    return arrayOffset + secondaryOffset;
    }  
        
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    CaseyMullen     10/09
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt       MemoryInstanceSupport::EnsureSpaceIsAvailable (UInt32& offset, ClassLayoutCR classLayout, PropertyLayoutCR propertyLayout, UInt32 bytesNeeded)
    {
    byte const *             data = _GetDataForRead();
    SecondaryOffset*         pSecondaryOffset = (SecondaryOffset*)(data + propertyLayout.GetOffset());
    SecondaryOffset*         pNextSecondaryOffset = pSecondaryOffset + 1;
    
    if (0 == *pSecondaryOffset)
        {
        SecondaryOffset* pPriorOffset;
        for (pPriorOffset = pSecondaryOffset - 1; 0 == *pPriorOffset; --pPriorOffset)
            ; // We have found the first non-zero secondaryOffset before this one
            
        for (SecondaryOffset* pOffsetToBackfill = pPriorOffset + 1; pOffsetToBackfill <= pSecondaryOffset; ++pOffsetToBackfill)
            *pOffsetToBackfill = *pPriorOffset; // Backfill zeros. We can only have zero SecondaryOffsets after a property value that has been set.
        }

    UInt32 availableBytes = 0;
    if (0 == *pNextSecondaryOffset)
        *pNextSecondaryOffset = *pSecondaryOffset; // WIP_FUSION: Use ModifyData // As long as we have zeros, it as if the last non-zero one were the value to use whereever there is a zero... 
    else        
        availableBytes = *pNextSecondaryOffset - *pSecondaryOffset;

    offset = *pSecondaryOffset;
    
#ifndef SHRINK_TO_FIT    
    if (bytesNeeded <= availableBytes)
        return SUCCESS;
#endif
    
    Int32 additionalBytesNeeded = bytesNeeded - availableBytes;
    
    if (additionalBytesNeeded <= 0)
        return SUCCESS;
        
    return GrowPropertyValue (classLayout, propertyLayout, additionalBytesNeeded);
    }        
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Adam.Klatzkin                   01/2010
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt       MemoryInstanceSupport::InsertNullArrayElementsAt (ClassLayoutCR classLayout, const wchar_t * propertyAccessString, UInt32 insertIndex, UInt32 insertCount)
    {        
    PRECONDITION (NULL != propertyAccessString, ECOBJECTS_STATUS_PreconditionViolated);
                
    PropertyLayoutCP pPropertyLayout = NULL;
    StatusInt status = classLayout.GetPropertyLayout (pPropertyLayout, propertyAccessString);
    if (SUCCESS != status || NULL == pPropertyLayout)
        return ERROR; // WIP_FUSION ERROR_PropertyNotFound        
            
    PropertyLayoutCR propertyLayout = *pPropertyLayout;
    // WIP_FUSION improve error codes
    bool isFixedCount = (propertyLayout.GetModifierFlags() & ARRAYMODIFIERFLAGS_IsFixedCount);
    PRECONDITION (!isFixedCount && propertyLayout.GetTypeDescriptor().IsArray() && "A variable size array property is required to grow an array", ERROR);
    
    PRECONDITION (insertCount > 0, ERROR)        
    
    return CreateNullArrayElementsAt (classLayout, propertyLayout, insertIndex, insertCount);
    }
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Adam.Klatzkin                   01/2010
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt       MemoryInstanceSupport::AddNullArrayElementsAt (ClassLayoutCR classLayout, const wchar_t * propertyAccessString, UInt32 count)
    {        
    PRECONDITION (NULL != propertyAccessString, ECOBJECTS_STATUS_PreconditionViolated);
                
    PropertyLayoutCP pPropertyLayout = NULL;
    StatusInt status = classLayout.GetPropertyLayout (pPropertyLayout, propertyAccessString);
    if (SUCCESS != status || NULL == pPropertyLayout)
        return ERROR; // WIP_FUSION ERROR_PropertyNotFound        
            
    PropertyLayoutCR propertyLayout = *pPropertyLayout;
    // WIP_FUSION improve error codes
    bool isFixedCount = (propertyLayout.GetModifierFlags() & ARRAYMODIFIERFLAGS_IsFixedCount);
    PRECONDITION (!isFixedCount && propertyLayout.GetTypeDescriptor().IsArray() && "A variable size array property is required to grow an array", ERROR);
    
    PRECONDITION (count > 0, ERROR)        
    
    return CreateNullArrayElementsAt (classLayout, propertyLayout, GetAllocatedArrayCount (propertyLayout), count);
    }    
    
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Adam.Klatzkin                   01/2010
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt       MemoryInstanceSupport::CreateNullArrayElementsAt (ClassLayoutCR classLayout, PropertyLayoutCR propertyLayout, UInt32 insertIndex, UInt32 insertCount)
    {                
    UInt32 arrayCount = GetAllocatedArrayCount (propertyLayout);    
    PRECONDITION (insertIndex <= arrayCount, ERROR);
        
    // we only need to grow the fixed section of the array
    UInt32 elementSizeInFixedSection;
    PrimitiveType elementType = propertyLayout.GetTypeDescriptor().GetPrimitiveType();
    bool hasFixedSizeElements = IsArrayOfFixedSizeElements (propertyLayout);
    if (hasFixedSizeElements)
        elementSizeInFixedSection = ClassLayout::GetFixedPrimitiveValueSize (elementType);
    else
        elementSizeInFixedSection = sizeof (SecondaryOffset);
    
    UInt32 currentNullFlagBitmasksCount = CalculateNumberNullFlagsBitmasks (arrayCount);    
    UInt32 currentHeaderByteCount = (arrayCount == 0) ? 0 : sizeof (ArrayCount) + (currentNullFlagBitmasksCount * sizeof (NullflagsBitmask));
    UInt32 currentFixedSectionByteCount = currentHeaderByteCount + (arrayCount * elementSizeInFixedSection);
    UInt32 currentArrayByteCount = GetPropertyValueSize (propertyLayout);
    
    UInt32 newArrayCount = arrayCount + insertCount;
    UInt32 newNullFlagBitmasksCount = CalculateNumberNullFlagsBitmasks (newArrayCount);
    UInt32 newHeaderByteCount = sizeof (ArrayCount) + (newNullFlagBitmasksCount * sizeof (NullflagsBitmask));
    UInt32 newFixedSectionByteCount = newHeaderByteCount + (newArrayCount * elementSizeInFixedSection);
    
    UInt32 bytesNeeded = newFixedSectionByteCount - currentFixedSectionByteCount;
    
    UInt32 arrayOffset;
    StatusInt status = EnsureSpaceIsAvailable (arrayOffset, classLayout, propertyLayout, currentArrayByteCount + bytesNeeded);
    if (SUCCESS != status)
        return status;       
        
    byte * pWriteBuffer;              
    UInt32 sizeOfWriteBuffer = 0;
    
    // shift all data (fixed & variable) to the right of the inserted element 
    byte const *             data = _GetDataForRead(); // need to get this value again since EnsureSpace may have re allocated    
    UInt32 offsetOfInsertionPoint = currentHeaderByteCount + (insertIndex * elementSizeInFixedSection);         
    byte const * pInsertFrom = data + arrayOffset + offsetOfInsertionPoint;
    byte const * pInsertTo = pInsertFrom + bytesNeeded;        
    #ifndef NDEBUG
    UInt32 arrayByteCountAfterGrow = GetPropertyValueSize (propertyLayout);
    byte const * pNextProperty = data + arrayOffset + arrayByteCountAfterGrow;
    DEBUG_EXPECT (pNextProperty == data + *((SecondaryOffset*)(data + propertyLayout.GetOffset()) + 1));
    #endif    
    if (currentArrayByteCount > offsetOfInsertionPoint)
        {        
        UInt32 byteCountToShift = currentArrayByteCount - offsetOfInsertionPoint;        
        DEBUG_EXPECT (pInsertTo + byteCountToShift <= pNextProperty); 
        memmove ((byte*)pInsertTo, pInsertFrom, byteCountToShift); // WIP_FUSION .. use _MoveData, waiting on Keith
        }
           
    // initialize the inserted secondary offsets and update all shifted secondary offsets         
    SecondaryOffset insertedSecondaryOffset;
    if (!hasFixedSizeElements)
        {
        UInt32 nSecondaryOffsetsShifted;        
        if (arrayCount > insertIndex)
            {
            nSecondaryOffsetsShifted = arrayCount - insertIndex;
            insertedSecondaryOffset = *((SecondaryOffset*)pInsertTo) + bytesNeeded;
            }
        else
            {
            nSecondaryOffsetsShifted = 0;
            insertedSecondaryOffset = GetPropertyValueSize (propertyLayout); // this is a relative index, not absolute
            }
        DEBUG_EXPECT (insertedSecondaryOffset <= arrayByteCountAfterGrow);
         
        UInt32 insertedSecondaryOffsetByteCount = insertCount * elementSizeInFixedSection;       
        SecondaryOffset* pSecondaryOffset = (SecondaryOffset*)(pInsertTo - insertedSecondaryOffsetByteCount);
        if (s_shiftSecondaryOffsetsInPlace)        
            pWriteBuffer = (byte*)pSecondaryOffset;
        else
            {
            sizeOfWriteBuffer = insertedSecondaryOffsetByteCount + (nSecondaryOffsetsShifted * sizeof (SecondaryOffset));
            pWriteBuffer = (byte*)alloca (sizeOfWriteBuffer);  
            }        
            
        // initialize inserted secondary offsets
        SecondaryOffset* pSecondaryOffsetWriteBuffer = (SecondaryOffset*)pWriteBuffer;
        for (UInt32 i = 0; i < insertCount ; i++)
            pSecondaryOffsetWriteBuffer[i] = insertedSecondaryOffset;
        
        // update shifted secondary offsets        
        for (UInt32 i = insertCount; i < nSecondaryOffsetsShifted + insertCount ;i++)
            {
            pSecondaryOffsetWriteBuffer[i] = pSecondaryOffset[i] + bytesNeeded;
            DEBUG_EXPECT (pSecondaryOffsetWriteBuffer[i] <=arrayByteCountAfterGrow);
            }
            
        if (!s_shiftSecondaryOffsetsInPlace)
            {
            UInt32 modifyOffset = (UInt32)(pInsertTo - data) - insertedSecondaryOffsetByteCount;
            status = _ModifyData (modifyOffset, pWriteBuffer, sizeOfWriteBuffer);            
            DEBUG_EXPECT (data + modifyOffset + sizeOfWriteBuffer <= pNextProperty);
            if (SUCCESS != status)
                return status;        
            }                
        }

                
    if (insertIndex > 0)
        {
        byte const *             data = _GetDataForRead(); // need to get this value again since EnsureSpace may have re allocated            
        byte const * pShiftFrom = data + arrayOffset + currentHeaderByteCount;
        byte * pShiftTo = (byte*)(data + arrayOffset + newHeaderByteCount);
        UInt32 byteCountToShift = (UInt32)(pInsertFrom - pShiftFrom);                
        
        // shift all the elements in the fixed section preceding the insert point if we needed to grow the nullflags bitmask            
        if ((pShiftTo != pShiftFrom))
            {
            memmove (pShiftTo, pShiftFrom, byteCountToShift); // WIP_FUSION .. use _MoveData, waiting on Keith
            DEBUG_EXPECT (pShiftTo + byteCountToShift <= pInsertTo); 
            }
            
        // update all secondary offsets preceeding the insertion point since the size of the fixed section has changed and therefore all variable data has moved
        if (!hasFixedSizeElements)
            {                    
            if (s_shiftSecondaryOffsetsInPlace)        
                pWriteBuffer = pShiftTo;
            else if (sizeOfWriteBuffer < byteCountToShift)
                {
                sizeOfWriteBuffer = byteCountToShift;
                pWriteBuffer = (byte*)alloca (sizeOfWriteBuffer);  
                }        
                            
            // update shifted secondary offsets        
            SecondaryOffset* pSecondaryOffset = (SecondaryOffset*)pShiftTo;
            SecondaryOffset* pSecondaryOffsetWriteBuffer = (SecondaryOffset*)pWriteBuffer;

            for (UInt32 i = 0; i < insertIndex ;i++)
                {
                pSecondaryOffsetWriteBuffer[i] = pSecondaryOffset[i] + bytesNeeded;
                DEBUG_EXPECT (pSecondaryOffsetWriteBuffer[i] <= insertedSecondaryOffset);
                }
                
            if (!s_shiftSecondaryOffsetsInPlace)
                {
                UInt32 modifyOffset = (UInt32)(pShiftTo - data);
                status = _ModifyData (modifyOffset, pWriteBuffer, byteCountToShift);
                DEBUG_EXPECT (modifyOffset >= newHeaderByteCount);
                DEBUG_EXPECT (data + modifyOffset + byteCountToShift <= pInsertTo);
                if (SUCCESS != status)
                    return status;        
                }                    
            }      
        }
            
    // write the new array header (updated count & null flags)      
    if (s_shiftSecondaryOffsetsInPlace)
        pWriteBuffer = (byte*)(data + arrayOffset);
    else
        {
        if (newHeaderByteCount > sizeOfWriteBuffer)
            {        
            pWriteBuffer = (byte*)alloca (newHeaderByteCount);     
            sizeOfWriteBuffer = newHeaderByteCount;
            }
        memcpy (pWriteBuffer, data + arrayOffset, currentHeaderByteCount); 
        }        
    *((UInt32*)pWriteBuffer) = newArrayCount;
    NullflagsBitmask* pNullflagsStart = (NullflagsBitmask*)(pWriteBuffer + sizeof (ArrayCount));
    NullflagsBitmask* pNullflagsCurrent;
    NullflagsBitmask  nullflagsBitmask;
    if (newNullFlagBitmasksCount > currentNullFlagBitmasksCount)
        InitializeNullFlags (pNullflagsStart + currentNullFlagBitmasksCount, newNullFlagBitmasksCount - currentNullFlagBitmasksCount);
    // shift all the nullflags bits right    
    bool isNull;
    UInt32 numberShifted;
    if (arrayCount > insertIndex)
        numberShifted = arrayCount - insertIndex;
    else
        numberShifted = 0;
    for (UInt32 i = 1 ; i <= numberShifted ; i++)
        {
        pNullflagsCurrent = pNullflagsStart + ((arrayCount - i) / BITS_PER_NULLFLAGSBITMASK);
        nullflagsBitmask = 0x01 << ((arrayCount - i) % BITS_PER_NULLFLAGSBITMASK);
        isNull = (0 != (*pNullflagsCurrent & nullflagsBitmask));    
        
        pNullflagsCurrent = pNullflagsStart + ((newArrayCount - i) / BITS_PER_NULLFLAGSBITMASK);
        nullflagsBitmask = 0x01 << ((newArrayCount - i) % BITS_PER_NULLFLAGSBITMASK);        
        if (isNull && 0 == (*pNullflagsCurrent & nullflagsBitmask))
            *pNullflagsCurrent |= nullflagsBitmask; // turn on the null bit
        else if (!isNull && nullflagsBitmask == (*pNullflagsCurrent & nullflagsBitmask))
            *pNullflagsCurrent ^= nullflagsBitmask; // turn off the null bit // WIP_FUSION: Needs to use ModifyData   
        }
    // initilize inserted nullflags bits
    for (UInt32 i=0;i < insertCount ; i++)
        {
        pNullflagsCurrent = pNullflagsStart + ((insertIndex + i) / BITS_PER_NULLFLAGSBITMASK);
        nullflagsBitmask = 0x01 << ((insertIndex + i) % BITS_PER_NULLFLAGSBITMASK);
        if (0 == (*pNullflagsCurrent & nullflagsBitmask))
            *pNullflagsCurrent |= nullflagsBitmask; // turn on the null bit        
        }
    if (!s_shiftSecondaryOffsetsInPlace)
        {
        status = _ModifyData (arrayOffset, pWriteBuffer, newHeaderByteCount);
        if (SUCCESS != status)
            return status;        
        }                   
    
    return SUCCESS;
    
    // NEEDSWORK .. Need to review and be certain that _GetDataForRead/Write is being used correctly.  Any time we enter a code path where we may do a write, we need
    // to ensure
    
    // AFTER talking with Casey
    // I can get rid of _GetDataForRead/Write & just create _GetData
    // be very careful with Ensure invocations or antyhing that may realloc the buffer because previous ptr returned from _GetData will now be invalid
    // when inserting elements in the array I will need to do 2 shifts but I should not have to shift the same data twice.  As long as I pad the shift of the
    // first set of data with the delta needed for the second shift of data.    
    
    // WIP_FUSION how do we deal with an error that occurs during the insert "transaction" after some data has already been moved/modified but we haven't finished?
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Adam.Klatzkin                   01/2010
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt       MemoryInstanceSupport::EnsureSpaceIsAvailableForArrayIndexValue (ClassLayoutCR classLayout, PropertyLayoutCR propertyLayout, UInt32 arrayIndex, UInt32 bytesNeeded)
    {    
    UInt32 arrayOffset = GetOffsetOfArray (propertyLayout);   
    byte const *             data = _GetDataForRead();
    SecondaryOffset* pIndexValueOffset = (SecondaryOffset*)(data + GetOffsetOfArrayIndex (arrayOffset, propertyLayout, arrayIndex));    
    SecondaryOffset indexValueOffset = arrayOffset + *pIndexValueOffset;    
    UInt32 arrayCount = GetAllocatedArrayCount (propertyLayout);
    SecondaryOffset* pNextPropertyValueOffset = (SecondaryOffset*)(_GetDataForRead() + propertyLayout.GetOffset()) + 1;
    SecondaryOffset nextIndexValueOffset;
    if (arrayIndex == arrayCount - 1) 
        nextIndexValueOffset = *pNextPropertyValueOffset;
    else 
        nextIndexValueOffset = arrayOffset + *(pIndexValueOffset + 1);
    UInt32 availableBytes = nextIndexValueOffset - indexValueOffset;
    
#ifndef SHRINK_TO_FIT    
    if (bytesNeeded <= availableBytes)
        return SUCCESS;
#endif    
    Int32 additionalBytesNeeded = bytesNeeded - availableBytes;
    
    if (additionalBytesNeeded <= 0)
        return SUCCESS;
    
    UInt32 endOfValueDataPreGrow = *pNextPropertyValueOffset;
    StatusInt status = GrowPropertyValue (classLayout, propertyLayout, additionalBytesNeeded);

    if (SUCCESS != status)
        return status;

    if (arrayIndex < arrayCount - 1)
        return ShiftArrayIndexValueData(propertyLayout, arrayIndex, arrayCount, endOfValueDataPreGrow, additionalBytesNeeded);
    else
        return status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Adam.Klatzkin                   01/2010
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt       MemoryInstanceSupport::GrowPropertyValue (ClassLayoutCR classLayout, PropertyLayoutCR propertyLayout, UInt32 additionalBytesNeeded)
    {
    byte const * data = _GetDataForWrite();
    UInt32 bytesUsed = classLayout.CalculateBytesUsed(data);
        
    UInt32 bytesAllocated = _GetBytesAllocated();    
    UInt32 additionalBytesAvailable = bytesAllocated - bytesUsed;
    
    StatusInt status = SUCCESS;
    UInt32 growBy = additionalBytesNeeded - additionalBytesAvailable;
    if (additionalBytesNeeded > additionalBytesAvailable)
        {
        status = _GrowAllocation (growBy);
        UInt32 newBytesAllocated = _GetBytesAllocated();
        DEBUG_EXPECT (newBytesAllocated >= bytesAllocated + growBy);
        bytesAllocated = newBytesAllocated;
        }
    
    if (SUCCESS != status)
        return status;
        
    byte * writeableData = _GetDataForWrite();
    DEBUG_EXPECT (bytesUsed == classLayout.CalculateBytesUsed(writeableData));
    
    status = ShiftValueData(classLayout, writeableData, bytesAllocated, propertyLayout, additionalBytesNeeded);

    DEBUG_EXPECT (0 == bytesUsed || (bytesUsed + additionalBytesNeeded == classLayout.CalculateBytesUsed(writeableData)));
    
    return status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    CaseyMullen     10/09
+---------------+---------------+---------------+---------------+---------------+------*/
void            MemoryInstanceSupport::SetShiftSecondaryOffsetsInPlace (bool inPlace)
    {
    s_shiftSecondaryOffsetsInPlace = inPlace;
    }
        
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    CaseyMullen     10/09
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt       MemoryInstanceSupport::ShiftValueData(ClassLayoutCR classLayout, byte * data, UInt32 bytesAllocated, PropertyLayoutCR propertyLayout, Int32 shiftBy)
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
        
        DEBUG_EXPECT (destination + bytesToMove <= data + bytesAllocated && "Attempted to move memory beyond the end of the allocated XAttribute.");
        if (destination + bytesToMove > data + bytesAllocated)
            return ERROR;
            
        memmove (destination, source, bytesToMove); // WIP_FUSION: Use Modify data, instead. Need method from Keith.
        }

    // Shift all secondaryOffsets for variable-sized property values that follow the one that just got larger
    UInt32 sizeOfSecondaryOffsetsToShift = (UInt32)(((byte*)pLast - (byte*)pCurrent) + sizeof (SecondaryOffset));
    if (s_shiftSecondaryOffsetsInPlace)
        {
        for (SecondaryOffset * pSecondaryOffset = pCurrent; 
             pSecondaryOffset < pLast && 0 != *pSecondaryOffset;  // stop when we hit a zero
             ++pSecondaryOffset)
            *pSecondaryOffset += shiftBy; 
        
        *pLast += shiftBy; // always do the last one
        return SUCCESS;
        }
        
    UInt32 nSecondaryOffsetsToShift = sizeOfSecondaryOffsetsToShift / sizeof(SecondaryOffset);

    SecondaryOffset * shiftedSecondaryOffsets = (SecondaryOffset*)alloca (sizeOfSecondaryOffsetsToShift);
    memset (shiftedSecondaryOffsets, 0, sizeOfSecondaryOffsetsToShift);
    for (UInt32 i = 0; i < nSecondaryOffsetsToShift - 1 && 0 != pCurrent[i]; i++) // stop when we hit a zero
        shiftedSecondaryOffsets[i] = pCurrent[i] + shiftBy;
        
    shiftedSecondaryOffsets[nSecondaryOffsetsToShift - 1] = pCurrent[nSecondaryOffsetsToShift - 1] + shiftBy; // always do the last one
    
    UInt32 offsetOfCurrent = (UInt32)((byte*)pCurrent - data);
    return _ModifyData (offsetOfCurrent, shiftedSecondaryOffsets, sizeOfSecondaryOffsetsToShift);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Adam.Klatzkin                   01/2010
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt       MemoryInstanceSupport::ShiftArrayIndexValueData(PropertyLayoutCR propertyLayout, UInt32 arrayIndex, UInt32 arrayCount, UInt32 endOfValueDataOffset, Int32 shiftBy)
    {
    DEBUG_EXPECT (0 != shiftBy && "It is a pointless waste of time to shift nothing");
    DEBUG_EXPECT (arrayIndex < arrayCount - 1 && "It is a pointless waste of time to shift nothing");

    byte * data = _GetDataForWrite();
    SecondaryOffset* pNextProperty = (SecondaryOffset*)(data + propertyLayout.GetOffset()) + 1;
    UInt32 arrayOffset = GetOffsetOfArray (propertyLayout);   
    SecondaryOffset indexValueOffset = GetOffsetOfArrayIndexValue (arrayOffset, propertyLayout, arrayIndex + 1); // start at the one AFTER the index whose value's size is adjusting
    
    UInt32 bytesToMove = endOfValueDataOffset - indexValueOffset;
    if (bytesToMove > 0)
        {
        byte * source = data + indexValueOffset;
        byte * destination = source + shiftBy;
        
        DEBUG_EXPECT (destination + bytesToMove <= data + *pNextProperty && "Attempted to move memory beyond the end of the reserved memory for this property.");
        if (destination + bytesToMove > data + *pNextProperty)
            return ERROR;
            
        memmove (destination, source, bytesToMove); // WIP_FUSION: Use Modify data, instead
        }

    // Shift all secondaryOffsets for indices following the one that just got larger
    UInt32 nSecondaryOffsetsToShift = arrayCount - (arrayIndex + 1);
    UInt32 sizeOfSecondaryOffsetsToShift = nSecondaryOffsetsToShift * sizeof (SecondaryOffset);    
    SecondaryOffset * shiftedSecondaryOffsets = (SecondaryOffset*)alloca (sizeOfSecondaryOffsetsToShift);
    SecondaryOffset * pCurrent = (SecondaryOffset*)(data + GetOffsetOfArrayIndex (arrayOffset, propertyLayout, arrayIndex + 1));
    for (UInt32 i = 0; i < nSecondaryOffsetsToShift; i++)
        shiftedSecondaryOffsets[i] = pCurrent[i] + shiftBy;
        
    UInt32 offsetOfCurrent = (UInt32)((byte*)pCurrent - data);
    return _ModifyData (offsetOfCurrent, shiftedSecondaryOffsets, sizeOfSecondaryOffsetsToShift);
    }
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    CaseyMullen     12/09
+---------------+---------------+---------------+---------------+---------------+------*/
void            MemoryInstanceSupport::InitializeMemory(ClassLayoutCR classLayout, byte * data, UInt32 bytesAllocated) const
    {
    classLayout.InitializeMemoryForInstance (data, bytesAllocated);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    CaseyMullen     12/09
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt       MemoryInstanceSupport::GetPrimitiveValueFromMemory (ECValueR v, PropertyLayoutCR propertyLayout, UInt32 nIndices, UInt32 const * indices) const
    {
    DEBUG_EXPECT (propertyLayout.GetExpectedIndices() == nIndices);   

    bool isInUninitializedFixedCountArray = ((nIndices > 0) && (propertyLayout.GetModifierFlags() & ARRAYMODIFIERFLAGS_IsFixedCount) && (GetAllocatedArrayCount (propertyLayout) == 0));    
    if (isInUninitializedFixedCountArray || (IsPropertyValueNull(propertyLayout, nIndices, indices)))
        {
        v.SetPrimitiveType (propertyLayout.GetTypeDescriptor().GetPrimitiveType());
        v.SetToNull();
        return SUCCESS;
        }    

    byte const * pValue = GetAddressOfPrimitiveValue (propertyLayout, nIndices, indices);

    ECTypeDescriptor typeDescriptor = propertyLayout.GetTypeDescriptor();    
    switch (typeDescriptor.GetPrimitiveType())
        {
        case PRIMITIVETYPE_Integer:
            {
            Int32 value;
            memcpy (&value, pValue, sizeof(value));
            v.SetInteger (value);
            return SUCCESS;
            }
        case PRIMITIVETYPE_Long:
            {
            Int64 value;
            memcpy (&value, pValue, sizeof(value));
            v.SetLong (value);
            return SUCCESS;
            }            
        case PRIMITIVETYPE_Double:
            {
            double value;
            memcpy (&value, pValue, sizeof(value));
            v.SetDouble (value);
            return SUCCESS;
            }            
        case PRIMITIVETYPE_String:
            {
            wchar_t * pString = (wchar_t *)pValue;
            v.SetString (pString, false); // WIP_FUSION: We are passing false for "makeDuplicateCopy" to avoid the allocation 
                                          // and copying... but how do make the caller aware of this? When do they need 
                                          // to be aware. The wchar_t* they get back would get invalidated if the 
                                          // XAttribute or other IMemoryProvider got reallocated, or the string got moved.
                                          // The caller must immediately use (e.g. marshal or copy) the returned value.
                                          // Optionally, the caller could ask the EC::ECValue to make a duplicate? 
            return SUCCESS;            
            }
        }

    POSTCONDITION (false && "datatype not implemented", ERROR);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Adam.Klatzkin                   01/2010
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt       MemoryInstanceSupport::GetValueFromMemory (ECValueR v, PropertyLayoutCR propertyLayout, UInt32 nIndices, UInt32 const * indices) const
    {
    // AZK To obtain a value from memory for a memory based PropertyLayout relative to this instance, the value must be embedded in the memory block contained by this instance.
    // Therefore nIndices must be <= 1.  If it is greater then 1 then the property accessor would be to some property nested within an array of structs.  Due to
    // polymorphic struct arrays we do not store struct array values in the same memory block
    PRECONDITION (nIndices <= 1, ERROR);

    ECTypeDescriptor typeDescriptor = propertyLayout.GetTypeDescriptor();
    if (typeDescriptor.IsPrimitive())
        {
        if (!EXPECTED_CONDITION (nIndices == 0))        
            return ERROR;
        return GetPrimitiveValueFromMemory (v, propertyLayout, nIndices, indices);
        }
    else if (typeDescriptor.IsPrimitiveArray())
        {                
        UInt32 arrayCount = GetReservedArrayCount (propertyLayout);
        
        if (nIndices == 0)
            {    
            bool isFixedArrayCount = propertyLayout.GetModifierFlags() & ARRAYMODIFIERFLAGS_IsFixedCount;                            
            v.SetPrimitiveArrayInfo (typeDescriptor.GetPrimitiveType(), arrayCount, isFixedArrayCount);
            return SUCCESS;
            }
        else
            {                        
            if (*indices >= arrayCount)
                return ERROR; // WIP_FUSION ERROR_InvalidIndex                

            return GetPrimitiveValueFromMemory (v, propertyLayout, nIndices, indices);
            }
        }

    POSTCONDITION (false && "Can not obtain value from memory using the specified property layout because it is an unsupported datatype", ERROR);        
    }
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    CaseyMullen     09/09
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt       MemoryInstanceSupport::GetValueFromMemory (ClassLayoutCR classLayout, ECValueR v, const wchar_t * propertyAccessString, UInt32 nIndices, UInt32 const * indices) const
    {
    PRECONDITION (NULL != propertyAccessString, ECOBJECTS_STATUS_PreconditionViolated);
    //PRECONDITION (IECInstance::AccessStringAndNIndicesAgree(propertyAccessString, nIndices, true), ECOBJECTS_STATUS_AccessStringDisagreesWithNIndices);
                
    PropertyLayoutCP propertyLayout = NULL;
    StatusInt status = classLayout.GetPropertyLayout (propertyLayout, propertyAccessString);
    if (SUCCESS != status || NULL == propertyLayout)
        return ERROR; // WIP_FUSION ERROR_PropertyNotFound        

    return GetValueFromMemory (v, *propertyLayout, nIndices, indices);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    CaseyMullen     09/09
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt       MemoryInstanceSupport::SetPrimitiveValueToMemory (ECValueCR v, ClassLayoutCR classLayout, PropertyLayoutCR propertyLayout, UInt32 nIndices, UInt32 const * indices)
    {        
    bool isInUninitializedFixedCountArray = ((nIndices > 0) && (propertyLayout.GetModifierFlags() & ARRAYMODIFIERFLAGS_IsFixedCount) && (GetAllocatedArrayCount (propertyLayout) == 0));
            
    if (v.IsNull())
        {
        if (!isInUninitializedFixedCountArray)
            SetPropertyValueNull (propertyLayout, nIndices, indices, true);
        return SUCCESS;
        }   
             
    if (isInUninitializedFixedCountArray)
        {
        CreateNullArrayElementsAt (classLayout, propertyLayout, 0, GetReservedArrayCount (propertyLayout));
        }        
    SetPropertyValueNull (propertyLayout, nIndices, indices, false);            
    
    UInt32 offset = GetOffsetOfPrimitiveValue (propertyLayout, nIndices, indices);
#ifdef EC_TRACE_MEMORY 
    wprintf (L"SetValue %s of 0x%x at offset=%d to %s.\n", propertyAccessString, this, offset, v.ToString().c_str());
#endif    
    switch (propertyLayout.GetTypeDescriptor().GetPrimitiveType())
        {
        case PRIMITIVETYPE_Integer:
            {
            Int32 value = v.GetInteger();
            return _ModifyData (offset, &value, sizeof(value));
            }
        case PRIMITIVETYPE_Long:
            {
            Int64 value = v.GetLong();
            return _ModifyData (offset, &value, sizeof(value));
            }
        case PRIMITIVETYPE_Double:
            {
            double value = v.GetDouble();
            return _ModifyData (offset, &value, sizeof(value));
            }       
        case PRIMITIVETYPE_String:
            {
            wchar_t const * value = v.GetString();
            UInt32 bytesNeeded = (UInt32)(sizeof(wchar_t) * (wcslen(value) + 1)); // WIP_FUSION: what if the caller could tell us the size?
            
            StatusInt status;
            if (1 == nIndices)
                status = EnsureSpaceIsAvailableForArrayIndexValue (classLayout, propertyLayout, *indices, bytesNeeded);
            else
                status = EnsureSpaceIsAvailable (offset, classLayout, propertyLayout, bytesNeeded);
            if (SUCCESS != status)
                return status;
                
            return _ModifyData (offset, value, bytesNeeded);
            }
        }

    POSTCONDITION (false && "datatype not implemented", ERROR);
    }        
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Adam.Klatzkin                   01/2010
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt       MemoryInstanceSupport::SetValueToMemory (ClassLayoutCR classLayout, const wchar_t * propertyAccessString, ECValueCR v, UInt32 nIndices, UInt32 const * indices)
    {
    PRECONDITION (NULL != propertyAccessString, ECOBJECTS_STATUS_PreconditionViolated);
    // AZK To obtain a value from memory for a memory based PropertyLayout relative to this instance, the value must be embedded in the memory block contained by this instance.
    // Therefore nIndices must be <= 1.  If it is greater then 1 then the property accessor would be to some property nested within an array of structs.  Due to
    // polymorphic struct arrays we do not store struct array values in the same memory block
    PRECONDITION (nIndices <= 1, ERROR);

    PRECONDITION (NULL != propertyAccessString, ECOBJECTS_STATUS_PreconditionViolated);
    //PRECONDITION (IECInstance::AccessStringAndNIndicesAgree(propertyAccessString, nIndices, true), ECOBJECTS_STATUS_AccessStringDisagreesWithNIndices);
                
    PropertyLayoutCP propertyLayout = NULL;
    StatusInt status = classLayout.GetPropertyLayout (propertyLayout, propertyAccessString);
    if (SUCCESS != status || NULL == propertyLayout)
        return ERROR; // WIP_FUSION ERROR_PropertyNotFound        

    ECTypeDescriptor typeDescriptor = propertyLayout->GetTypeDescriptor();
    if (typeDescriptor.IsPrimitive())
        {
        if (!EXPECTED_CONDITION (nIndices == 0))        
            return ERROR;
        return SetPrimitiveValueToMemory (v, classLayout, *propertyLayout, nIndices, indices);
        }
    else if (typeDescriptor.IsPrimitiveArray())
        {
        if (nIndices == 1)
            {            
            if (*indices >= GetReservedArrayCount (*propertyLayout))
                return ERROR; // WIP_FUSION ERROR_InvalidIndex

            return SetPrimitiveValueToMemory (v, classLayout, *propertyLayout, nIndices, indices);
            }
        }

    POSTCONDITION (false && "Can not set the value to memory using the specified property layout because it is an unsupported datatype", ERROR);
    }  

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    CaseyMullen     10/09
+---------------+---------------+---------------+---------------+---------------+------*/
void            MemoryInstanceSupport::DumpInstanceData (ClassLayoutCR classLayout) const
    {
    static bool s_skipDump = false;
    static int s_dumpCount = 0;
    s_dumpCount++;
    
    byte const * data = _GetDataForRead();
    Bentley::NativeLogging::ILogger *logger = Logger::GetLogger();
    
    logger->tracev (L"======================= Dump #%d ===================================\n", s_dumpCount);
    if (s_skipDump)
        return;
  
    logger->tracev (L"ECClass=%s at address = 0x%0x\n", classLayout.GetClass().GetName().c_str(), data);
    InstanceFlags flags = *(InstanceFlags*)data;
    logger->tracev (L"  [0x%0x][%4.d] InstanceFlags = 0x%08.x\n", data, 0, flags);
    
    UInt32 nProperties = classLayout.GetPropertyCount ();
    
    UInt32 nNullflagsBitmasks = CalculateNumberNullFlagsBitmasks (nProperties);
    
    for (UInt32 i = 0; i < nNullflagsBitmasks; i++)
        {
        UInt32 offset = sizeof(InstanceFlags) + i * sizeof(NullflagsBitmask);
        byte const * address = offset + data;
        logger->tracev (L"  [0x%x][%4.d] Nullflags[%d] = 0x%x\n", address, offset, i, *(NullflagsBitmask*)(data + offset));
        }
    
    for (UInt32 i = 0; i < nProperties; i++)
        {
        PropertyLayoutCP propertyLayout;
        StatusInt status = classLayout.GetPropertyLayoutByIndex (propertyLayout, i);
        if (SUCCESS != status)
            {
            logger->tracev (L"Error (%d) returned while getting PropertyLayout #%d", status, i);
            return;
            }

        UInt32 offset = propertyLayout->GetOffset();
        byte const * address = data + offset;
            
        ECValue v;
        GetValueFromMemory (v, *propertyLayout, 0, NULL);
        std::wstring valueAsString = v.ToString();
           
        if (propertyLayout->IsFixedSized())            
            logger->tracev (L"  [0x%x][%4.d] %s = %s\n", address, offset, propertyLayout->GetAccessString(), valueAsString.c_str());
        else
            {
            SecondaryOffset secondaryOffset = *(SecondaryOffset*)address;
            byte const * realAddress = data + secondaryOffset;
            
            logger->tracev (L"  [0x%x][%4.d] -> [0x%x][%4.d] %s = %s\n", address, offset, realAddress, secondaryOffset, propertyLayout->GetAccessString(), valueAsString.c_str());
            }
            
        if (propertyLayout->GetTypeDescriptor().IsArray())
            {
            UInt32 count = GetAllocatedArrayCount (*propertyLayout);
            if (count != GetReservedArrayCount (*propertyLayout))
                logger->tracev (L"      array has not yet been initialized\n");
            else
                {            
                for (UInt32 i = 0; i < count; i++)
                    {                
                    offset = GetOffsetOfArrayIndex (GetOffsetOfArray (*propertyLayout), *propertyLayout, i);
                    address = data + offset;
                    GetValueFromMemory (v, *propertyLayout, 1, &i);
                    valueAsString = v.ToString();                
                    if (IsArrayOfFixedSizeElements (*propertyLayout))
                        logger->tracev (L"      [0x%x][%4.d] %d = %s\n", address, offset, i, valueAsString.c_str());
                    else
                        {
                        SecondaryOffset secondaryOffset = GetOffsetOfArrayIndexValue (GetOffsetOfArray (*propertyLayout), *propertyLayout, i);
                        byte const * realAddress = data + secondaryOffset;
                        
                        logger->tracev (L"      [0x%x][%4.d] -> [0x%x][%4.d] %d = %s\n", address, offset, realAddress, secondaryOffset, i, valueAsString.c_str());                    
                        }              
                    }
                }
            }
        }
        
    UInt32 offsetOfLast = classLayout.GetSizeOfFixedSection() - sizeof(SecondaryOffset);
    SecondaryOffset * pLast = (SecondaryOffset*)(data + offsetOfLast);
    logger->tracev (L"  [0x%x][%4.d] Offset of TheEnd = %d\n", pLast, offsetOfLast, *pLast);
    }
    
END_BENTLEY_EC_NAMESPACE