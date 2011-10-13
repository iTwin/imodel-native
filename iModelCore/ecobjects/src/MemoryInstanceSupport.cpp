
|
|     $Source: src/MemoryInstanceSupport.cpp $
|
|   $Copyright: (c) 2011 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include    "ECObjectsPch.h"
#include    <algorithm>
#include    <iomanip>

// SHRINK_TO_FIT will cause space reserved for variable-sized values to be reduced to the minimum upon every set operation.
// SHRINK_TO_FIT is not recommended and is mainly for testing. It it better to "compact" everything at once
#define SHRINK_TO_FIT 1 

using namespace std;

BEGIN_BENTLEY_EC_NAMESPACE

const UInt32 BITS_PER_NULLFLAGSBITMASK = (sizeof(NullflagsBitmask) * 8);

/*---------------------------------------------------------------------------------**//**
* used in compatible class layout map
* @bsimethod                                    Bill.Steinbock                  10/2011
+---------------+---------------+---------------+---------------+---------------+------*/
bool less_classLayout::operator()(ClassLayoutCP s1, ClassLayoutCP s2) const
    {
    return (s1->GetUniqueId() < s2->GetUniqueId());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Bill.Steinbock                  10/2011
+---------------+---------------+---------------+---------------+---------------+------*/
static int      randomValue  ()
    {
    static const int     low  = 0x00000001;
    static const int     high = 0x7fffffff;

    // includes low, excludes high.
    int     randomNum = rand();
    int     range = high - low;
    return  low + (int) ( (double) range * (double) randomNum / (double) (RAND_MAX + 1));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    08/10
+---------------+---------------+---------------+---------------+---------------+------*/
void            appendFormattedString (wostringstream& outStream, WCharCP fmtStr, ...)
    {
    wchar_t line[1024];

    va_list argList;
    va_start(argList, fmtStr /* the last fixed argument */);
    vswprintf(line, _countof(line), fmtStr, argList);
    va_end(argList);

    // in case we use up the local buffer, truncate with a newline
    swprintf (&line[_countof(line) - 2], 2, L"\n");;

    outStream << line;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    12/09
+---------------+---------------+---------------+---------------+---------------+------*/
static bool     PrimitiveTypeIsFixedSize (PrimitiveType primitiveType)
    {
    switch (primitiveType)
        {
        case PRIMITIVETYPE_Integer:
        case PRIMITIVETYPE_Long:
        case PRIMITIVETYPE_Double:
        case PRIMITIVETYPE_Boolean:
        case PRIMITIVETYPE_Point2D:
        case PRIMITIVETYPE_Point3D:
        case PRIMITIVETYPE_DateTime: 
            return true;
        case PRIMITIVETYPE_String:
        case PRIMITIVETYPE_Binary:
            return false;
        default:
            DEBUG_FAIL("Unsupported data type");
            return false;
        }
    }
    
/*---------------------------------------------------------------------------------**//**
* internal helper method - no error checking.  Presumes caller has already determined
* that propertyLayout is an array.
* @bsimethod                                    Adam.Klatzkin                   02/2010
+---------------+---------------+---------------+---------------+---------------+------*/    
static inline bool      IsArrayOfFixedSizeElements (PropertyLayoutCR propertyLayout)
    {
    ECTypeDescriptor typedescriptor = propertyLayout.GetTypeDescriptor();
    
    if ((typedescriptor.IsPrimitiveArray()) && (PrimitiveTypeIsFixedSize (typedescriptor.GetPrimitiveType())))
        return true;
    else
        return false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    AdamKlatzkin    01/10
+---------------+---------------+---------------+---------------+---------------+------*/
static inline UInt32    CalculateNumberNullFlagsBitmasks (UInt32 numberOfItems)
    {
    UInt32 nNullflagsBitmasks = numberOfItems / BITS_PER_NULLFLAGSBITMASK;
    if ( (numberOfItems % BITS_PER_NULLFLAGSBITMASK > 0) )
        ++nNullflagsBitmasks;
    return nNullflagsBitmasks;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    AdamKlatzkin    01/10
+---------------+---------------+---------------+---------------+---------------+------*/
static inline void      InitializeNullFlags (NullflagsBitmask * nullFlagsStart, UInt32 numberOfGroups)
    {
    NullflagsBitmask* nullflagsLocation = nullFlagsStart;
    for (UInt32 i = 0; i < numberOfGroups; i++, nullflagsLocation++)
        *nullflagsLocation = NULLFLAGS_BITMASK_AllOn;
    }

/*---------------------------------------------------------------------------------**//**
* Fixed array properties (arrays that are entirely stored in the fixed section of an instance)
* must have both a fixed element count and contain fixed size elements.  This method will calculate
* size (in bytes) required to store such an array in a the instance fixed section.  It does not validate
* that the conditions are met for storing the array there.  That exercise is left to the caller.
* @bsimethod                                                    AdamKlatzkin    01/10
+---------------+---------------+---------------+---------------+---------------+------*/
static inline UInt32    CalculateFixedArrayPropertySize (UInt32 fixedCount, PrimitiveType primitiveType)
    {
    return (CalculateNumberNullFlagsBitmasks (fixedCount) * sizeof (NullflagsBitmask)) + 
        (fixedCount *ECValue::GetFixedPrimitiveValueSize(primitiveType));
    }  
           
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Bill.Steinbock                  09/2011
+---------------+---------------+---------------+---------------+---------------+------*/
bool    PropertyLayout::SetReadOnlyMask (bool readOnly)
    {
    bool outVal = PROPERTYLAYOUTMODIFIERFLAGS_IsReadOnly == (m_modifierFlags & PROPERTYLAYOUTMODIFIERFLAGS_IsReadOnly);

    if (readOnly)
        m_modifierFlags |= PROPERTYLAYOUTMODIFIERFLAGS_IsReadOnly;
    else
        m_modifierFlags &= ~PROPERTYLAYOUTMODIFIERFLAGS_IsReadOnly;

    return outVal;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    CaseyMullen     10/09
+---------------+---------------+---------------+---------------+---------------+------*/
WString    PropertyLayout::ToString ()
    {    
    WString typeName;
    if ((m_typeDescriptor.IsPrimitive()) || (m_typeDescriptor.IsPrimitiveArray()))
        typeName = ECXml::GetPrimitiveTypeName (m_typeDescriptor.GetPrimitiveType());
    else
        typeName = L"struct";

    if (m_typeDescriptor.IsArray())
        typeName.append(L"[]");
    
    wchar_t line[1024];
    swprintf (line, _countof(line), L"%-32s %-16s offset=%3i nullflagsOffset=%3i, nullflagsBitmask=0x%08.X", m_accessString.c_str(), typeName.c_str(), m_offset, m_nullflagsOffset, m_nullflagsBitmask);
        
    return line;
    }
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    CaseyMullen     10/09
+---------------+---------------+---------------+---------------+---------------+------*/
bool            PropertyLayout::IsFixedSized () const
    {
    // WIP_FUSION: when we have m_modifiers, they will determine if it is fixed size... could have fixed size string or variable int (added at end)
    if (m_typeDescriptor.IsStruct())
        return true;

    return ( ( m_typeDescriptor.IsPrimitive() || 
               (m_typeDescriptor.IsPrimitiveArray() && (m_modifierFlags & PROPERTYLAYOUTMODIFIERFLAGS_IsArrayFixedCount))
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
        return ECValue::GetFixedPrimitiveValueSize(m_typeDescriptor.GetPrimitiveType());
    else if (m_typeDescriptor.IsPrimitiveArray())
        {
        UInt32 fixedCount = m_modifierData; // WIP_FUSION for now assume modifier data holds the count but I'm not sure if that is the right place for this.
        return CalculateFixedArrayPropertySize (fixedCount, m_typeDescriptor.GetPrimitiveType());
        }
        
    DEBUG_FAIL("Can not determine size in fixed section for datatype");
    return 0;
    }    

#define DEBUG_CLASSLAYOUT_LEAKS
#ifdef DEBUG_CLASSLAYOUT_LEAKS
LeakDetector<ClassLayout> g_classLayoutLeakDetector (L"ClassLayout", L"ClassLayouts", true);
#else
LeakDetector<ClassLayout> g_classLayoutLeakDetector (L"ClassLayout", L"ClassLayouts", false);
#endif

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    CaseyMullen     10/09
+---------------+---------------+---------------+---------------+---------------+------*/
ClassLayout::ClassLayout(SchemaIndex schemaIndex, bool hideFromLeakDetection)
    :
    m_hideFromLeakDetection(hideFromLeakDetection),
    m_schemaIndex (schemaIndex),
    m_classIndex(0),
    m_sizeOfFixedSection(0), 
    m_isRelationshipClass(false), m_propertyIndexOfSourceECPointer(-1), m_propertyIndexOfTargetECPointer(-1)
    {
    m_uniqueId = randomValue();

    if ( ! m_hideFromLeakDetection)
        g_classLayoutLeakDetector.ObjectCreated(*this);
    };
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    11/10
+---------------+---------------+---------------+---------------+---------------+------*/
ClassLayout::~ClassLayout()
    {
    FOR_EACH (PropertyLayoutP layout, m_propertyLayouts)
        delete layout;

    if ( ! m_hideFromLeakDetection)
        g_classLayoutLeakDetector.ObjectDestroyed(*this);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    09/10
+---------------+---------------+---------------+---------------+---------------+------*/
ILeakDetector&  ClassLayout::Debug_GetLeakDetector() { return g_classLayoutLeakDetector; }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    11/10
+---------------+---------------+---------------+---------------+---------------+------*/
WString        ClassLayout::GetShortDescription () const
    {
    wchar_t line[1024];
    swprintf (line, _countof(line), L"ClassLayout for ECClassIndex=%i, ECClass.GetName()=%s", m_classIndex, m_className.c_str());

    return line;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    11/10
+---------------+---------------+---------------+---------------+---------------+------*/
WString        ClassLayout::GetName () const
    {
    return GetShortDescription();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    01/11
+---------------+---------------+---------------+---------------+---------------+------*/
WString        ClassLayout::LogicalStructureToString (UInt32 parentStuctIndex, UInt32 indentLevel) const
    {
    LogicalStructureMap::const_iterator it = m_logicalStructureMap.find(parentStuctIndex);

    if ( ! EXPECTED_CONDITION (it != m_logicalStructureMap.end()))
        return L"";

    WCharCP indentStr = L"|--";
    wostringstream oss;
    oss << setiosflags (ios::left);

    FOR_EACH (UInt32 propIndex, it->second)
        {
        for (UInt32 i = 0; i < indentLevel; i++)
            oss << indentStr;

        oss << setw(12 - 3 * indentLevel) << propIndex;

        PropertyLayoutCP    propertyLayout = NULL;
        ECObjectsStatus     status = GetPropertyLayoutByIndex (propertyLayout, propIndex);

        if (SUCCESS != status || NULL == propertyLayout)
            {
            oss << L"  *** ERROR finding PropertyLayout ***" << endl;
            continue;
            }

        WCharCP  accessString = propertyLayout->GetAccessString();
        oss << setw(40) << accessString << setw(10) << L"  Parent: " << propertyLayout->GetParentStructIndex() << endl;

        if (propertyLayout->GetTypeDescriptor().IsStruct())
            oss << LogicalStructureToString (propIndex, indentLevel+1); 
        }
    
    return oss.str().c_str();
    }    

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    CaseyMullen     10/09
+---------------+---------------+---------------+---------------+---------------+------*/
WString        ClassLayout::ToString () const
    {
    wostringstream oss;

    oss << GetShortDescription() << endl;

    UInt32 propIndex = 0;

    FOR_EACH (PropertyLayoutP layout, m_propertyLayouts)
        {
        oss << setiosflags( ios::left ) << setw(4) << propIndex;
        oss << layout->ToString();
        oss << endl;

        propIndex++;
        }

    oss << L"Logical Structure:" << endl;
    oss << LogicalStructureToString ();

    return oss.str().c_str();
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
    memset (data, 0, sizeof(InstanceHeader)); 
    
    InstanceHeader& header = *(InstanceHeader*) data;
    header.m_schemaIndex   = m_schemaIndex;
    header.m_classIndex    = m_classIndex;
    header.m_instanceFlags = 0;

    UInt32  nonStructPropCount = GetPropertyCountExcludingEmbeddedStructs();

    if (0 == nonStructPropCount)
        return;
        
    UInt32 nNullflagsBitmasks = CalculateNumberNullFlagsBitmasks (nonStructPropCount);
    InitializeNullFlags ((NullflagsBitmask *)(data + sizeof (InstanceHeader)), nNullflagsBitmasks);
            
    bool isFirstVariableSizedProperty = true;
    for (UInt32 i = 0; i < m_propertyLayouts.size(); ++i)
        {
        PropertyLayoutCP layout = m_propertyLayouts[i];
        if (layout->IsFixedSized())
            {
            if (layout->GetTypeDescriptor().IsArray())
                {
                nNullflagsBitmasks = CalculateNumberNullFlagsBitmasks (layout->GetModifierData());
                InitializeNullFlags ((NullflagsBitmask *)(data + layout->GetOffset()), nNullflagsBitmasks);
                }            
            continue;
            }
        
#ifdef BOOLEAN_CAUSES_PROBLEMS
        DEBUG_EXPECT (layout.GetOffset() % 4 == 0 && "We expect secondaryOffsets to be aligned on a 4 byte boundary");
#endif
            
        SecondaryOffset* secondaryOffset = (SecondaryOffset*)(data + layout->GetOffset());

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
* Checks testLayout layout to see if equal to or a  subset of classlayout
* @bsimethod                                    Bill.Steinbock                  10/2011
+---------------+---------------+---------------+---------------+---------------+------*/
static bool            areLayoutsCompatible (ClassLayoutCR testLayout, ClassLayoutCR classLayout) 
    {
    UInt32 nProperties = testLayout.GetPropertyCount ();

    // see if every property in testLayout can be found in classLayout and is the same type
    for (UInt32 i = 0; i < nProperties; i++)
        {
        PropertyLayoutCP testPropertyLayout;
        ECObjectsStatus status = testLayout.GetPropertyLayoutByIndex (testPropertyLayout, i);
        if (ECOBJECTS_STATUS_Success != status)
            return false;

        PropertyLayoutCP propertyLayout;
        status = classLayout.GetPropertyLayout (propertyLayout, testPropertyLayout->GetAccessString());
        if (ECOBJECTS_STATUS_Success != status)
            return false;

        if (testPropertyLayout->GetTypeDescriptor().GetTypeKind() != propertyLayout->GetTypeDescriptor().GetTypeKind())
            return false;

        if (testPropertyLayout->GetTypeDescriptor().IsStructArray() != propertyLayout->GetTypeDescriptor().IsStructArray())
            return false;

        if (testPropertyLayout->GetTypeDescriptor().IsPrimitiveArray())
            {
            if (!propertyLayout->GetTypeDescriptor().IsPrimitiveArray())
                return false;

            if (testPropertyLayout->GetTypeDescriptor().GetPrimitiveType() != propertyLayout->GetTypeDescriptor().GetPrimitiveType())
                return false;
            }

        if (testPropertyLayout->GetTypeDescriptor().IsPrimitive())
            {
            if (!propertyLayout->GetTypeDescriptor().IsPrimitive())
                return false;

            if (testPropertyLayout->GetTypeDescriptor().GetPrimitiveType() != propertyLayout->GetTypeDescriptor().GetPrimitiveType())
                return false;
            }
        }

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Bill.Steinbock                  03/2010
+---------------+---------------+---------------+---------------+---------------+------*/
bool            ClassLayout::IsCompatible (ClassLayoutCR classLayout) const
    {
    if (this == &classLayout)
        return true;

    CompatibleClassLayoutsMap::const_iterator compatibeLayoutIter = m_compatibleClassLayouts.find(&classLayout);
    if (m_compatibleClassLayouts.end() != compatibeLayoutIter)
        return compatibeLayoutIter->second;

    if (0 != _wcsicmp (GetECClassName().c_str(), classLayout.GetECClassName().c_str()))
        return false;

    bool isCompatible = areLayoutsCompatible (*this, classLayout);
    m_compatibleClassLayouts[&classLayout] = isCompatible;

    return isCompatible;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Bill.Steinbock                  10/2011
+---------------+---------------+---------------+---------------+---------------+------*/
int                    ClassLayout:: GetUniqueId() const
    {
    return m_uniqueId;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    CaseyMullen     10/09
+---------------+---------------+---------------+---------------+---------------+------*/
UInt32          ClassLayout::CalculateBytesUsed(byte const * data) const
    {
    DEBUG_EXPECT (0 != m_sizeOfFixedSection); 
    
    // handle case when no properties are defined
    if (0 == m_propertyLayouts.size())
        return m_sizeOfFixedSection;
        
    PropertyLayoutCP lastPropertyLayout = m_propertyLayouts[m_propertyLayouts.size() - 1];  // WIP_FUSION: if _GetBytesUsed shows up in profiler, we may want to add bool m_hasVariableSizedValues;
    if (lastPropertyLayout->IsFixedSized())
        return m_sizeOfFixedSection;

    SecondaryOffset * pLast = (SecondaryOffset*)(data + m_sizeOfFixedSection - sizeof(SecondaryOffset));
    
    // pLast is the last offset, pointing to one byte beyond the used space, so it is equal to the number of bytes used, so far
    return *pLast;
    }
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    01/10
+---------------+---------------+---------------+---------------+---------------+------*/
SchemaIndex     ClassLayout::GetSchemaIndex() const
    {
    return m_schemaIndex;
    } 

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    CaseyMullen     10/09
+---------------+---------------+---------------+---------------+---------------+------*/
ClassIndex      ClassLayout::GetClassIndex() const
    {
    return m_classIndex;
    } 
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    CaseyMullen     12/09
+---------------+---------------+---------------+---------------+---------------+------*/    
WString const &  ClassLayout::GetECClassName() const
    {
    return m_className;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    CaseyMullen    01/10
+---------------+---------------+---------------+---------------+---------------+------*/
int   ClassLayout::GetECPointerIndex (ECRelationshipEnd end) const
    {
    if (end == ECRelationshipEnd_Source)
        return m_propertyIndexOfSourceECPointer;
    else
        return m_propertyIndexOfTargetECPointer;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    01/11
+---------------+---------------+---------------+---------------+---------------+------*/    
UInt32          ClassLayout::Factory::GetParentStructIndex (WCharCP accessString) const
    {
    // The access string will contain a '.' only if the property is inside an embedded struct.
    UInt32          parentStructIndex = 0;
    WCharCP  pLastDot = wcsrchr (accessString, L'.');

    if (NULL != pLastDot)
        {
        WString         parentAccessString (accessString, pLastDot - accessString);
        ECObjectsStatus status = m_underConstruction.GetPropertyIndex (parentStructIndex, parentAccessString.c_str());

        assert (SUCCESS == status);
        }
    
    return parentStructIndex;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    CaseyMullen     10/09
+---------------+---------------+---------------+---------------+---------------+------*/    
void            ClassLayout::Factory::AddStructProperty (WCharCP accessString, ECTypeDescriptor typeDescriptor)
    {
    if (!EXPECTED_CONDITION (m_state == AcceptingFixedSizeProperties)) // ClassLayoutNotAcceptingFixedSizeProperties    
        return;
    
    if (!EXPECTED_CONDITION (typeDescriptor.IsStruct()))
        return;

    UInt32  parentStructIndex = GetParentStructIndex (accessString);

    // A struct PropertyLayout is just a placeholder.  Don't call AddProperty.
    PropertyLayoutP propertyLayout = new PropertyLayout(accessString, parentStructIndex, typeDescriptor, 0, 0, 0, 0, 0);
    m_underConstruction.AddPropertyLayout (accessString, *propertyLayout);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    CaseyMullen     10/09
+---------------+---------------+---------------+---------------+---------------+------*/    
void            ClassLayout::Factory::AddProperty (WCharCP accessString, ECTypeDescriptor typeDescriptor, UInt32 size, UInt32 modifierFlags, UInt32 modifierData)
    {
    UInt32  positionInCurrentNullFlags = m_nonStructPropertyCount % 32;
    NullflagsBitmask  nullflagsBitmask = 0x01 << positionInCurrentNullFlags;
    
    if (0 == positionInCurrentNullFlags && 0 != m_nonStructPropertyCount)
        {
        // It is time to add a new set of Nullflags
        m_nullflagsOffset += sizeof(NullflagsBitmask);
        m_offset          += sizeof(NullflagsBitmask);
        for (UInt32 i = 0; i < m_underConstruction.GetPropertyCount(); i++)
            {
            if ( ! m_underConstruction.m_propertyLayouts[i]->m_typeDescriptor.IsStruct())
                m_underConstruction.m_propertyLayouts[i]->m_offset += sizeof(NullflagsBitmask); // Offsets of already-added property layouts need to get bumped up
            }
        } 

    // WIP_FUSION, for now all accessors of array property layouts are stored with the brackets appended.  This means all access to array values through an
    // IECInstance must include the brackets.  If you want to obtain an array element value then you specify an index.  If you want to obtain an array info value
    // then you do not specify an index.  I'd like to consider an update to this so if an access string does not include the [] then we always return the ArrayInfo value.
    WString tempAccessString = accessString;
    if (typeDescriptor.IsArray())
        tempAccessString.append (L"[]");

    UInt32          parentStructIndex = GetParentStructIndex(accessString);
    PropertyLayoutP propertyLayout = new PropertyLayout (tempAccessString.c_str(), parentStructIndex, typeDescriptor, m_offset, m_nullflagsOffset, nullflagsBitmask, modifierFlags, modifierData);
    m_underConstruction.AddPropertyLayout (accessString, *propertyLayout); 

    m_offset += (UInt32)size;
    m_nonStructPropertyCount++;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    CaseyMullen     10/09
+---------------+---------------+---------------+---------------+---------------+------*/    
void            ClassLayout::Factory::AddFixedSizeProperty (WCharCP accessString, ECTypeDescriptor typeDescriptor, bool isReadOnly)
    {
    if (!EXPECTED_CONDITION (m_state == AcceptingFixedSizeProperties)) // ClassLayoutNotAcceptingFixedSizeProperties    
        return;
    
    if (!typeDescriptor.IsPrimitive())
        {
        DEBUG_FAIL ("We currently only support fixed sized properties for primitive types");
        return;
        }
    
    UInt32 size = ECValue::GetFixedPrimitiveValueSize (typeDescriptor.GetPrimitiveType());
 
    UInt32  modifierFlags = 0;
    if (isReadOnly)
        modifierFlags |= PROPERTYLAYOUTMODIFIERFLAGS_IsReadOnly; 

    AddProperty (accessString, typeDescriptor, size, modifierFlags);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Adam.Klatzkin                   01/2010
+---------------+---------------+---------------+---------------+---------------+------*/ 
void            ClassLayout::Factory::AddFixedSizeArrayProperty (WCharCP accessString, ECTypeDescriptor typeDescriptor, UInt32 arrayCount, bool isReadOnly)
    {
    if (!EXPECTED_CONDITION (m_state == AcceptingFixedSizeProperties)) // ClassLayoutNotAcceptingFixedSizeProperties    
        return;
    
    UInt32 size = CalculateFixedArrayPropertySize (arrayCount, typeDescriptor.GetPrimitiveType());

    UInt32  modifierFlags = PROPERTYLAYOUTMODIFIERFLAGS_IsArrayFixedCount;
    if (isReadOnly)
        modifierFlags |= PROPERTYLAYOUTMODIFIERFLAGS_IsReadOnly; 
   
    AddProperty (accessString, typeDescriptor, size, modifierFlags, arrayCount);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    CaseyMullen     10/09
+---------------+---------------+---------------+---------------+---------------+------*/ 
void            ClassLayout::Factory::AddVariableSizeProperty (WCharCP accessString, ECTypeDescriptor typeDescriptor, bool isReadOnly)
    {
    if (m_state == AcceptingFixedSizeProperties)
        m_state = AcceptingVariableSizeProperties;

    if (!EXPECTED_CONDITION (m_state == AcceptingVariableSizeProperties)) // ClassLayoutNotAcceptingVariableSizeProperties    
        return;
        
    UInt32 size = sizeof(SecondaryOffset); // the offset will just point to this secondary offset

    UInt32  modifierFlags = 0;
    if (isReadOnly)
        modifierFlags |= PROPERTYLAYOUTMODIFIERFLAGS_IsReadOnly; 

    AddProperty (accessString, typeDescriptor, size, modifierFlags);
    }  

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Adam.Klatzkin                   01/2010
+---------------+---------------+---------------+---------------+---------------+------*/ 
void            ClassLayout::Factory::AddVariableSizeArrayPropertyWithFixedCount (WCharCP accessString, ECTypeDescriptor typeDescriptor, UInt32 arrayCount, bool isReadOnly)
    {
    if (m_state == AcceptingFixedSizeProperties)
        m_state = AcceptingVariableSizeProperties;

    if (!EXPECTED_CONDITION (m_state == AcceptingVariableSizeProperties)) // ClassLayoutNotAcceptingVariableSizeProperties    
        return;
        
    UInt32 size = sizeof(SecondaryOffset); // the offset will just point to this secondary offset
    UInt32 modifierFlags = PROPERTYLAYOUTMODIFIERFLAGS_IsArrayFixedCount;
    if (isReadOnly)
        modifierFlags |= PROPERTYLAYOUTMODIFIERFLAGS_IsReadOnly; 

    AddProperty (accessString, typeDescriptor, size, modifierFlags, arrayCount);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    12/09
+---------------+---------------+---------------+---------------+---------------+------*/
void            ClassLayout::Factory::AddProperties (ECClassCR ecClass, WCharCP nameRoot, bool addingFixedSizeProps)
    {
    if (addingFixedSizeProps)
        AddStructProperty (NULL == nameRoot ? L"" : nameRoot, ECTypeDescriptor::CreateStructTypeDescriptor());

    FOR_EACH (ECPropertyP property, ecClass.GetProperties())
        {
        WString    propName = property->GetName();
        
        if (NULL != nameRoot)
            propName = WString (nameRoot).append (L".") + propName;

        if (property->GetIsPrimitive())
            {
            PrimitiveECPropertyP  primitiveProp = property->GetAsPrimitiveProperty();
            PrimitiveType         primitiveType = primitiveProp->GetType();

            bool isFixedSize = PrimitiveTypeIsFixedSize(primitiveType);

            if (addingFixedSizeProps && isFixedSize)
                AddFixedSizeProperty (propName.c_str(), primitiveType, property->GetIsReadOnly());
            else if ( ! addingFixedSizeProps && ! isFixedSize)
                AddVariableSizeProperty (propName.c_str(), primitiveType, property->GetIsReadOnly());
            }
        else if (property->GetIsStruct())
            {
            StructECPropertyP  structProp = property->GetAsStructProperty();
            ECClassCR          nestedClass = structProp->GetType();
            
            AddProperties (nestedClass, propName.c_str(), addingFixedSizeProps);
            }
        else if (property->GetIsArray())
            {
            ArrayECPropertyP  arrayProp = property->GetAsArrayProperty();
            ArrayKind arrayKind = arrayProp->GetKind();
            if (arrayKind == ARRAYKIND_Primitive)
                {
                bool isFixedArrayCount = (arrayProp->GetMinOccurs() == arrayProp->GetMaxOccurs());
                bool isFixedPropertySize = isFixedArrayCount && PrimitiveTypeIsFixedSize (arrayProp->GetPrimitiveElementType());
                
                if (addingFixedSizeProps && isFixedPropertySize)
                    AddFixedSizeArrayProperty (propName.c_str(), ECTypeDescriptor::CreatePrimitiveArrayTypeDescriptor (arrayProp->GetPrimitiveElementType()), arrayProp->GetMinOccurs(), property->GetIsReadOnly());
                else if (!addingFixedSizeProps && !isFixedPropertySize)
                    {
                    if (isFixedArrayCount)
                        AddVariableSizeArrayPropertyWithFixedCount (propName.c_str(), ECTypeDescriptor::CreatePrimitiveArrayTypeDescriptor (arrayProp->GetPrimitiveElementType()), arrayProp->GetMinOccurs(), property->GetIsReadOnly());
                    else
                        AddVariableSizeProperty (propName.c_str(), ECTypeDescriptor::CreatePrimitiveArrayTypeDescriptor (arrayProp->GetPrimitiveElementType()), property->GetIsReadOnly());
                    }
                }
            else if ((arrayKind == ARRAYKIND_Struct) && (!addingFixedSizeProps))
                {
                bool isFixedArrayCount = (arrayProp->GetMinOccurs() == arrayProp->GetMaxOccurs());
                if (isFixedArrayCount)
                    AddVariableSizeArrayPropertyWithFixedCount (propName.c_str(), ECTypeDescriptor::CreateStructArrayTypeDescriptor(), arrayProp->GetMinOccurs(), property->GetIsReadOnly());
                else
                    AddVariableSizeProperty (propName.c_str(), ECTypeDescriptor::CreateStructArrayTypeDescriptor(), property->GetIsReadOnly());                
                }
            }
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    01/10
+---------------+---------------+---------------+---------------+---------------+------*/
ClassLayoutP    ClassLayout::Factory::DoBuildClassLayout ()
    { //ECLogger::Log->debugv (L"Building ClassLayout for ECClass %s", m_ecClass->GetName().c_str());
    m_underConstruction.m_isRelationshipClass = (dynamic_cast<ECRelationshipClassCP>(&m_ecClass) != NULL);

    // Iterate through the ECProperties of the ECClass and build the layout
    AddProperties (m_ecClass, NULL, true);
    AddProperties (m_ecClass, NULL, false);

    if (m_underConstruction.m_isRelationshipClass)
        {
        AddVariableSizeProperty (PROPERTYLAYOUT_Source_ECPointer, PRIMITIVETYPE_Binary, false);
        AddVariableSizeProperty (PROPERTYLAYOUT_Target_ECPointer, PRIMITIVETYPE_Binary, false);
        }

    m_underConstruction.FinishLayout ();
    
    return &m_underConstruction;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    02/10
+---------------+---------------+---------------+---------------+---------------+------*/
ClassLayout::Factory::Factory (ECClassCR ecClass, ClassIndex classIndex, SchemaIndex schemaIndex, bool hideFromLeakDetection)
    : 
    m_ecClass (ecClass),
    m_state   (AcceptingFixedSizeProperties),
    m_offset  (sizeof(InstanceHeader) + sizeof(NullflagsBitmask)),
    m_nullflagsOffset (sizeof(InstanceHeader)),
    m_nonStructPropertyCount (0),
    m_underConstruction (*ClassLayout::CreateEmpty (m_ecClass.GetName().c_str(), classIndex, schemaIndex, hideFromLeakDetection))
    {
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    01/10
+---------------+---------------+---------------+---------------+---------------+------*/
ClassLayoutP    ClassLayout::BuildFromClass (ECClassCR ecClass, ClassIndex classIndex, SchemaIndex schemaIndex, bool hideFromLeakDetection)
    {
    Factory     factory (ecClass, classIndex, schemaIndex, hideFromLeakDetection);

    return factory.DoBuildClassLayout ();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    01/10
+---------------+---------------+---------------+---------------+---------------+------*/
ClassLayoutP    ClassLayout::CreateEmpty (WCharCP className, ClassIndex classIndex, SchemaIndex schemaIndex, bool hideFromLeakDetection)
    {
    ClassLayoutP classLayout = new ClassLayout(schemaIndex, hideFromLeakDetection);

    classLayout->SetClass (className, classIndex);

    return classLayout;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    CaseyMullen     10/09
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   ClassLayout::SetClass (WCharCP className, UInt16 classIndex)
    {
    m_classIndex = classIndex;
    m_className  = className;

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    CaseyMullen     10/09
+---------------+---------------+---------------+---------------+---------------+------*/  
ECObjectsStatus       ClassLayout::FinishLayout ()
    {
    // Calculate size of fixed section    
    PropertyLayoutCP lastPropertyLayout = NULL;

    for (PropertyLayoutVector::const_reverse_iterator it = m_propertyLayouts.rbegin(); it != m_propertyLayouts.rend( ); it++)
        {
        PropertyLayoutCP candidate = *it;

        // Find the last property layout that uses space in the fixed section.
        // For now, that is anything that is not a struct since variable size
        // properties store their secondary offsets in the fixed section.
        if ( ! candidate->GetTypeDescriptor().IsStruct())
            {
            lastPropertyLayout = candidate;
            break;
            }
        }

    if (NULL == lastPropertyLayout)
        {
        // Either there are no properties or all the properties are structs.
        m_sizeOfFixedSection = sizeof (InstanceHeader);
        }
    else
        {
        UInt32    size = lastPropertyLayout->GetOffset() + lastPropertyLayout->GetSizeInFixedSection();

        if ( ! lastPropertyLayout->IsFixedSized())
            size += sizeof(SecondaryOffset); // There is one additional SecondaryOffset tracking the end of the last variable-sized value

        m_sizeOfFixedSection = size;
        }

#ifndef NDEBUG
    DEBUG_EXPECT (m_propertyLayoutMap.size() == m_propertyLayouts.size());

    UInt32  nNullflagsBitmasks  = CalculateNumberNullFlagsBitmasks (GetPropertyCountExcludingEmbeddedStructs());
    UInt32  nonStructPropIndex  = 0;
    UInt32  prevNonStructOffset = 0;
    for (UInt32 i = 0; i < m_propertyLayouts.size(); i++)
        {
        bool isStruct = m_propertyLayouts[i]->m_typeDescriptor.IsStruct();

        // Check the offsets
        if (isStruct)
            {
            DEBUG_EXPECT (m_propertyLayouts[i]->m_offset == 0);
            }
        else
            {
            if (0 == prevNonStructOffset)
                DEBUG_EXPECT (m_propertyLayouts[i]->m_offset == sizeof(InstanceHeader) + nNullflagsBitmasks * sizeof(NullflagsBitmask));
            else
                DEBUG_EXPECT (m_propertyLayouts[i]->m_offset > prevNonStructOffset);
            }

        // Check the NullFlagsOffsets
        UInt32 expectedNullflagsOffset = 0;

        if ( ! isStruct)
            expectedNullflagsOffset = (nonStructPropIndex / BITS_PER_NULLFLAGSBITMASK * sizeof(NullflagsBitmask)) + sizeof(InstanceHeader);

        DEBUG_EXPECT (m_propertyLayouts[i]->m_nullflagsOffset == expectedNullflagsOffset);

        if ( ! isStruct)
            {
            nonStructPropIndex++;
            prevNonStructOffset = m_propertyLayouts[i]->m_offset;
            }
        }     
#endif
    return ECOBJECTS_STATUS_Success;
    }
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    CaseyMullen    01/10
+---------------+---------------+---------------+---------------+---------------+------*/     
void            ClassLayout::CheckForECPointers (WCharCP accessString)
    {
    // Remember indices of source and target ECPointers for fast lookup
    if (0 == wcscmp (PROPERTYLAYOUT_Source_ECPointer, accessString))
        {
        m_isRelationshipClass = true;
        m_propertyIndexOfSourceECPointer = GetPropertyCount() - 1;
        }
        
    if (0 == wcscmp (PROPERTYLAYOUT_Target_ECPointer, accessString))
        {
        m_isRelationshipClass = true;
        m_propertyIndexOfTargetECPointer = GetPropertyCount() - 1;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    01/11
+---------------+---------------+---------------+---------------+---------------+------*/    
void            ClassLayout::AddToLogicalStructureMap (PropertyLayoutR propertyLayout, UInt32 propertyIndex)
    {
    if (propertyIndex != 0)
        {
        UInt32  parentStuctIndex = propertyLayout.GetParentStructIndex();
        LogicalStructureMap::iterator it = m_logicalStructureMap.find(parentStuctIndex);

        if ( ! EXPECTED_CONDITION (it != m_logicalStructureMap.end()))
            return;

        it->second.push_back (propertyIndex);
        }

    if (propertyLayout.GetTypeDescriptor().IsStruct())
        m_logicalStructureMap[propertyIndex] = bvector<UInt32>();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    CaseyMullen     10/09
+---------------+---------------+---------------+---------------+---------------+------*/    
void            ClassLayout::AddPropertyLayout (WCharCP accessString, PropertyLayoutR propertyLayout)
    {
    m_propertyLayouts.push_back(&propertyLayout);
    m_propertyLayoutMap[propertyLayout.GetAccessString()] = m_propertyLayouts.back();

    AddToLogicalStructureMap (propertyLayout, (UInt32) m_propertyLayouts.size() - 1);
    CheckForECPointers (accessString);
    }

/*---------------------------------------------------------------------------------**//**
* Called by SchemaLayoutElementHandler's BuildClassLayout() when deserializing ClassLayouts
* @bsimethod                                                    JoshSchifter    01/10
+---------------+---------------+---------------+---------------+---------------+------*/     
void            ClassLayout::AddPropertyDirect
(
WCharCP  accessString,
UInt32           parentStructIndex,
ECTypeDescriptor typeDescriptor,
UInt32           offset,
UInt32           nullflagsOffset,
UInt32           nullflagsBitmask
)
    {
    PropertyLayoutP propertyLayout = new PropertyLayout (accessString, parentStructIndex, typeDescriptor, offset, nullflagsOffset, nullflagsBitmask);

    AddPropertyLayout (accessString, *propertyLayout); 
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    CaseyMullen     10/09
+---------------+---------------+---------------+---------------+---------------+------*/
UInt32          ClassLayout::GetPropertyCount () const
    {
    return (UInt32) m_propertyLayouts.size();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    01/11
+---------------+---------------+---------------+---------------+---------------+------*/
UInt32          ClassLayout::GetPropertyCountExcludingEmbeddedStructs () const
    {
    UInt32      nonStructPropertyCount = 0;

    for (UInt32 i = 0; i < m_propertyLayouts.size(); i++)
        {
        if ( ! m_propertyLayouts[i]->m_typeDescriptor.IsStruct())
            nonStructPropertyCount++;
        }

    return nonStructPropertyCount;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Bill.Steinbock                  09/2011
+---------------+---------------+---------------+---------------+---------------+------*/
bool            ClassLayout::IsPropertyReadOnly (UInt32 propertyIndex) const
    {
    assert (propertyIndex < m_propertyLayouts.size());
    if (propertyIndex >= m_propertyLayouts.size())
        return true; 
        
    return m_propertyLayouts[propertyIndex]->IsReadOnlyProperty (); 
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Bill.Steinbock                  09/2011
+---------------+---------------+---------------+---------------+---------------+------*/
bool            ClassLayout::SetPropertyReadOnly (UInt32 propertyIndex,  bool readOnly) const
    {
    assert (propertyIndex < m_propertyLayouts.size());
    if (propertyIndex >= m_propertyLayouts.size())
        return false; 
        
    return m_propertyLayouts[propertyIndex]->SetReadOnlyMask (readOnly); 
    }    

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    CaseyMullen     10/09
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus       ClassLayout::GetPropertyLayout (PropertyLayoutCP & propertyLayout, WCharCP accessString) const
    {
    PropertyLayoutMap::const_iterator it = m_propertyLayoutMap.find(accessString);
    
    if (it == m_propertyLayoutMap.end())
        {
        return ECOBJECTS_STATUS_PropertyNotFound;
        }
    else
        {
        propertyLayout = it->second;
        return ECOBJECTS_STATUS_Success;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    05/10
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus     ClassLayout::GetPropertyIndex (UInt32& propertyIndex, WCharCP accessString) const
    {
    PropertyLayoutCP    propertyLayout;

    ECObjectsStatus status = GetPropertyLayout (propertyLayout, accessString);
    if (ECOBJECTS_STATUS_Success != status)
        return status;

    for (UInt32 i = 0; i < m_propertyLayouts.size(); i++)
        {
        PropertyLayoutCP candidate = m_propertyLayouts[i];

        if (propertyLayout == candidate)
            {
            propertyIndex = i;
            return ECOBJECTS_STATUS_Success;
            }
        }

    assert (false && "Property present in map but not in vector"); 
    return ECOBJECTS_STATUS_Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    CaseyMullen     10/09
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus       ClassLayout::GetPropertyLayoutByIndex (PropertyLayoutCP & propertyLayout, UInt32 propertyIndex) const
    {
    assert (propertyIndex < m_propertyLayouts.size());
    if (propertyIndex >= m_propertyLayouts.size())
        return ECOBJECTS_STATUS_IndexOutOfRange; 
        
    propertyLayout = m_propertyLayouts[propertyIndex];
    return ECOBJECTS_STATUS_Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Dylan.Rush      12/10
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus         ClassLayout::GetAccessStringByIndex(WCharCP& accessString, UInt32 propertyIndex) const
    {
    PropertyLayoutCP    propertyLayout;
    ECObjectsStatus     status = GetPropertyLayoutByIndex (propertyLayout, propertyIndex);

    if (ECOBJECTS_STATUS_Success != status)
        return status;

    accessString = propertyLayout->GetAccessString();

    return ECOBJECTS_STATUS_Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    01/10
+---------------+---------------+---------------+---------------+---------------+------*/
UInt32  ClassLayout::GetFirstChildPropertyIndex (UInt32 parentIndex) const
    {
    // Find the parent in the map
    bmap<UInt32, bvector<UInt32>>::const_iterator mapIterator = m_logicalStructureMap.find (parentIndex);

    if ( ! EXPECTED_CONDITION (m_logicalStructureMap.end() != mapIterator))
        return 0;

    // Return the first member of the parent's childList
    bvector<UInt32> const& childIndexList = mapIterator->second;

    return *childIndexList.begin();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Bill.Steinbock                  09/2011
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus ClassLayout::GetPropertyIndices (bvector<UInt32>& properties, UInt32 parentIndex) const
    {
    bmap<UInt32, bvector<UInt32>>::const_iterator mapIterator = m_logicalStructureMap.find (parentIndex);
    if ( ! EXPECTED_CONDITION (m_logicalStructureMap.end() != mapIterator))
        return ECOBJECTS_STATUS_Error;

    FOR_EACH (UInt32 propIndex, mapIterator->second)
        properties.push_back(propIndex);

    if (properties.size() > 0)
        return ECOBJECTS_STATUS_Success;

    return ECOBJECTS_STATUS_Error;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    01/10
+---------------+---------------+---------------+---------------+---------------+------*/
UInt32  ClassLayout::GetNextChildPropertyIndex (UInt32 parentIndex, UInt32 childIndex) const
    {
    // Find the parent in the map
    bmap<UInt32, bvector<UInt32>>::const_iterator mapIterator = m_logicalStructureMap.find (parentIndex);

    if ( ! EXPECTED_CONDITION (m_logicalStructureMap.end() != mapIterator))
        return 0;

    // Find the child in the parent's childList
    bvector<UInt32> const& childIndexList = mapIterator->second;

    bvector<UInt32>::const_iterator it = std::find (childIndexList.begin(), childIndexList.end(), childIndex);
    if ( ! EXPECTED_CONDITION (childIndexList.end() != it))
        return 0;

    // Get the next entry in the childList.
    if (childIndexList.end() != ++it)
        return *it;

    return 0;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    01/10
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   SchemaLayout::AddClassLayout (ClassLayoutCR classLayout, ClassIndex classIndex)
    {
    if (m_classLayouts.size() <= classIndex)
        m_classLayouts.resize (20 + classIndex, NULL); 

    assert (NULL == m_classLayouts[classIndex] && "ClassIndex is already in use");

    m_classLayouts[classIndex] = &classLayout;
    
    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    01/10
+---------------+---------------+---------------+---------------+---------------+------*/
ClassLayoutCP   SchemaLayout::GetClassLayout (ClassIndex classIndex)
    {
    if (m_classLayouts.size() <= classIndex)
        return NULL;

    return m_classLayouts[classIndex];
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    01/10
+---------------+---------------+---------------+---------------+---------------+------*/
ClassLayoutCP   SchemaLayout::FindClassLayout (WCharCP className)
    {
    FOR_EACH (ClassLayoutCP classLayout, m_classLayouts)
        {
        if (NULL == classLayout)
            continue;

        if (0 == _wcsicmp (classLayout->GetECClassName().c_str(), className))
            return classLayout;
        }

    return NULL;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    CaseyMullen    02/10
+---------------+---------------+---------------+---------------+---------------+------*/
UInt32          SchemaLayout::GetMaxIndex()
    {
    return (UInt32)m_classLayouts.size() - 1;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    01/10
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   SchemaLayout::FindAvailableClassIndex(ClassIndex& classIndex)
    {
    ClassLayoutP nullVal = NULL;
    ClassLayoutVector::iterator iter = std::find (m_classLayouts.begin(), m_classLayouts.end(), nullVal);

    size_t firstNullIndex = iter - m_classLayouts.begin();

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
ClassLayoutCR   ClassLayoutHolder::GetClassLayout() const 
    {
    return m_classLayout;
    }    
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Adam.Klatzkin                   01/2010
+---------------+---------------+---------------+---------------+---------------+------*/    
UInt32          MemoryInstanceSupport::GetPropertyValueSize (PropertyLayoutCR propertyLayout) const
    {
    if (propertyLayout.IsFixedSized())
        {
        return propertyLayout.GetSizeInFixedSection();
        }
    else
        {    
        UInt32 offset = propertyLayout.GetOffset();
        SecondaryOffset const * pSecondaryOffset = (SecondaryOffset const *)(_GetData() + offset);
        
        SecondaryOffset secondaryOffset = *pSecondaryOffset;
        if (0 == secondaryOffset)
            return 0;

        // get offset, in the fixed-sized section, to the next variable size property
        SecondaryOffset nextSecondaryOffset = *(pSecondaryOffset + 1);
        if (0 == nextSecondaryOffset)
            return 0;

        UInt32 size = nextSecondaryOffset - secondaryOffset;
        return size;
        }
    }    
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Adam.Klatzkin                   01/2010
+---------------+---------------+---------------+---------------+---------------+------*/    
UInt32          MemoryInstanceSupport::GetPropertyValueSize (PropertyLayoutCR propertyLayout, UInt32 index) const
    {
    if (IsArrayOfFixedSizeElements (propertyLayout))
        {
        return ECValue::GetFixedPrimitiveValueSize (propertyLayout.GetTypeDescriptor().GetPrimitiveType());
        }
    else
        {                            
        UInt32 arrayOffset = GetOffsetOfPropertyValue (propertyLayout);   
        byte const *             data = _GetData();
        SecondaryOffset* pIndexValueOffset = (SecondaryOffset*)(data + GetOffsetOfArrayIndex (arrayOffset, propertyLayout, index));    
        SecondaryOffset indexValueOffset = arrayOffset + *pIndexValueOffset;    
        if (0 == indexValueOffset)        
            return 0;
        UInt32 arrayCount = GetAllocatedArrayCount (propertyLayout);
        SecondaryOffset* pNextPropertyValueOffset = (SecondaryOffset*)(data + propertyLayout.GetOffset()) + 1;
        SecondaryOffset nextIndexValueOffset;
        if (index == arrayCount - 1) 
            nextIndexValueOffset = *pNextPropertyValueOffset;
        else 
            nextIndexValueOffset = arrayOffset + *(pIndexValueOffset + 1);            
        if (0 == nextIndexValueOffset)
            return 0;
            
        UInt32 size = nextIndexValueOffset - indexValueOffset;        
        return size;
        }
    }        

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    01/10
+---------------+---------------+---------------+---------------+---------------+------*/
InstanceHeader const& MemoryInstanceSupport::PeekInstanceHeader (void const* data)
    {
    return * ((InstanceHeader const*) data);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    AdamKlatzkin    01/10
+---------------+---------------+---------------+---------------+---------------+------*/
ArrayCount      MemoryInstanceSupport::GetReservedArrayCount (PropertyLayoutCR propertyLayout) const
    {
    if (propertyLayout.GetModifierFlags() & PROPERTYLAYOUTMODIFIERFLAGS_IsArrayFixedCount)
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
        {
        return propertyLayout.GetModifierData();
        }
    else
        {
        SecondaryOffset* pSecondaryOffset = (SecondaryOffset*)(_GetData() + propertyLayout.GetOffset());
        SecondaryOffset* pNextOffset = pSecondaryOffset + 1;        
        
        SecondaryOffset arrayOffset = *pSecondaryOffset;
        if ((arrayOffset == 0) || (*pNextOffset == 0) || (arrayOffset == *pNextOffset))
            return 0;

        byte const * pCount = _GetData() + arrayOffset;
        return *((ArrayCount*)pCount);
        }
    }    

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    CaseyMullen     10/09
+---------------+---------------+---------------+---------------+---------------+------*/
byte const *    MemoryInstanceSupport::GetAddressOfPropertyValue (PropertyLayoutCR propertyLayout) const
    {
    return _GetData() + GetOffsetOfPropertyValue (propertyLayout);
    }
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    CaseyMullen     10/09
+---------------+---------------+---------------+---------------+---------------+------*/
byte const *    MemoryInstanceSupport::GetAddressOfPropertyValue (PropertyLayoutCR propertyLayout, UInt32 index) const
    {
    UInt32 arrayOffset = GetOffsetOfPropertyValue (propertyLayout);   
    return _GetData() + GetOffsetOfArrayIndexValue (arrayOffset, propertyLayout, index);
    }    

/*---------------------------------------------------------------------------------**//**
* internal helper - no parameter checking
* Determines the offset & bitmask necessary to access the null flag for a specified property
*
* @param[OUT]   nullFlagsOffset     The offset of the null flags group.  This offset is relative to the
*                                   beginning of the instance data.
* @param[OUT]   nullflagsBitmask    The bitmask that should be used to access the null flag bit in the group at nullflagsOffset.
* @param[IN]    propertyLayout      The propertyLayout to obtain nullflags lookup information for
* @bsimethod                                    Adam.Klatzkin                   01/2010
+---------------+---------------+---------------+---------------+---------------+------*/
static void     PrepareToAccessNullFlags 
(
UInt32&             nullflagsOffset, 
UInt32&             nullflagsBitmask, 
byte const *        data, 
PropertyLayoutCR    propertyLayout
)
    {
    nullflagsOffset = propertyLayout.GetNullflagsOffset();
    nullflagsBitmask = propertyLayout.GetNullflagsBitmask();
    }    
    
/*---------------------------------------------------------------------------------**//**
* internal helper - no parameter checking
* Determines the offset & bitmask necessary to access the null flag for a specified index of an element in an array proeprty.
*
* @param[OUT]   nullFlagsOffset     The offset of the null flags group.  This offset is relative to the
*                                   beginning of the instance data.
* @param[OUT]   nullflagsBitmask    The bitmask that should be used to access the null flag bit in the group at nullflagsOffset.
* @param[IN]    propertyLayout      The propertyLayout to obtain nullflags lookup information for
* @param[IN]    index               The index number (starting at 0) of the array element to access null flags for.  It is expected
*                                   that the caller has validated this parameter is within a valid range (< ArrayCount).
* @bsimethod                                    Adam.Klatzkin                   01/2010
+---------------+---------------+---------------+---------------+---------------+------*/
static void     PrepareToAccessNullFlags 
(
UInt32&             nullflagsOffset, 
UInt32&             nullflagsBitmask, 
byte const *        data, 
PropertyLayoutCR    propertyLayout, 
UInt32              index
)
    {
    if (propertyLayout.IsFixedSized())
        nullflagsOffset = propertyLayout.GetOffset();
    else
        nullflagsOffset = *((SecondaryOffset*)(data + propertyLayout.GetOffset())) + sizeof (ArrayCount);

    nullflagsOffset += (index / BITS_PER_NULLFLAGSBITMASK * sizeof(NullflagsBitmask));
    nullflagsBitmask = 0x01 << (index % BITS_PER_NULLFLAGSBITMASK);
    }       
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    CaseyMullen     10/09
+---------------+---------------+---------------+---------------+---------------+------*/
bool            MemoryInstanceSupport::IsPropertyValueNull (PropertyLayoutCR propertyLayout, bool useIndex, UInt32 index) const
    {
    UInt32 nullflagsOffset;
    UInt32 nullflagsBitmask;
    byte const * data = _GetData();
    if (useIndex)
        PrepareToAccessNullFlags (nullflagsOffset, nullflagsBitmask, data, propertyLayout, index);    
    else
        PrepareToAccessNullFlags (nullflagsOffset, nullflagsBitmask, data, propertyLayout);

    NullflagsBitmask const * nullflags = (NullflagsBitmask const *)(data + nullflagsOffset);
    return (0 != (*nullflags & nullflagsBitmask));    
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    CaseyMullen     10/09
+---------------+---------------+---------------+---------------+---------------+------*/
void            MemoryInstanceSupport::SetPropertyValueNull (PropertyLayoutCR propertyLayout, bool useIndex, UInt32 index, bool isNull)
    {  
    UInt32 nullflagsOffset;
    UInt32 nullflagsBitmask;
    byte const * data = _GetData();
    if (useIndex)
        PrepareToAccessNullFlags (nullflagsOffset, nullflagsBitmask, data, propertyLayout, index);   
    else
        PrepareToAccessNullFlags (nullflagsOffset, nullflagsBitmask, data, propertyLayout);   
    
    NullflagsBitmask * nullflags = (NullflagsBitmask *)(data + nullflagsOffset);
    if (isNull && 0 == (*nullflags & nullflagsBitmask))
        *nullflags |= nullflagsBitmask; // turn on the null bit
    else if (!isNull && nullflagsBitmask == (*nullflags & nullflagsBitmask))
        *nullflags ^= nullflagsBitmask; // turn off the null bit // WIP_FUSION: Needs to use ModifyData
    }    

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    CaseyMullen     10/09
+---------------+---------------+---------------+---------------+---------------+------*/
UInt32          MemoryInstanceSupport::GetOffsetOfPropertyValue (PropertyLayoutCR propertyLayout, bool useIndex, UInt32 index) const
    {
    UInt32 offset = propertyLayout.GetOffset();
    
    if (!propertyLayout.IsFixedSized())
        {
        SecondaryOffset const * pSecondaryOffset = (SecondaryOffset const *)(_GetData() + offset);
        offset = *pSecondaryOffset;
        }
        
    if (!useIndex)
        return offset;
    else
        return GetOffsetOfArrayIndexValue (offset, propertyLayout, index);
    }       
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Adam.Klatzkin                   01/2010
+---------------+---------------+---------------+---------------+---------------+------*/
UInt32          MemoryInstanceSupport::GetOffsetOfArrayIndex (UInt32 arrayOffset, PropertyLayoutCR propertyLayout, UInt32 index) const
    {    
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
        primaryOffset += (index * ECValue::GetFixedPrimitiveValueSize (propertyLayout.GetTypeDescriptor().GetPrimitiveType()));
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
    
    byte const * data = _GetData();
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
ECObjectsStatus       MemoryInstanceSupport::EnsureSpaceIsAvailable (UInt32& offset, ClassLayoutCR classLayout, PropertyLayoutCR propertyLayout, UInt32 bytesNeeded, EmbeddedInstanceCallbackP callbackP)
    {
    byte const *             data = _GetData();
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
        return ECOBJECTS_STATUS_Success;
        
    return GrowPropertyValue (classLayout, propertyLayout, additionalBytesNeeded, callbackP);
    }        
        
#define DEBUGGING_ARRAYENTRY_REMOVAL   

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Bill.Steinbock                  07/2011
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus MemoryInstanceSupport::RemoveArrayElementsFromMemory (ClassLayoutCR classLayout, PropertyLayoutCR propertyLayout, UInt32 removeIndex, UInt32 removeCount)
    {
    ECObjectsStatus status = ECOBJECTS_STATUS_Error;

    bool isFixedCount = (propertyLayout.GetModifierFlags() & PROPERTYLAYOUTMODIFIERFLAGS_IsArrayFixedCount);
    PRECONDITION (!isFixedCount && propertyLayout.GetTypeDescriptor().IsArray() && "A variable size array property is required to remove an array entry", ECOBJECTS_STATUS_PreconditionViolated);
    
    PRECONDITION (removeCount > 0, ECOBJECTS_STATUS_IndexOutOfRange)        
   
    UInt32       bytesAllocated = _GetBytesAllocated();    
    byte *       currentData    = (byte*)_GetData();

    // since we can not use memmove on XAttribute memory, copy the memory move it around and then use _ModifyData
    // copy the entire instance into allocated memory
    ScopedArray<byte> scoped((size_t)bytesAllocated);
    byte*   data = scoped.GetData();
    memcpy (data, currentData, (size_t)bytesAllocated);

    bool    hasFixedSizedElements = IsArrayOfFixedSizeElements (propertyLayout);
    UInt32  arrayOffset           = GetOffsetOfPropertyValue (propertyLayout);   
    byte   *arrayAddress          = data + arrayOffset;

    // modify from bottom up
    SecondaryOffset  beginIndexValueOffset = GetOffsetOfArrayIndexValue (arrayOffset, propertyLayout, removeIndex);
    byte *           destination = data + beginIndexValueOffset;
    byte *           source;
    UInt32           bytesToMove;
    UInt32           totalBytesAdjusted=0;
    ArrayCount       preArrayCount = GetAllocatedArrayCount (propertyLayout);
    ArrayCount       postArrayCount = preArrayCount - removeCount;
    SecondaryOffset* pThisProperty = (SecondaryOffset*)(data + propertyLayout.GetOffset());
    SecondaryOffset* pNextProperty = pThisProperty + 1;

    if ((removeIndex + removeCount) >= preArrayCount) // removing entries an end of array
        {
        source = data + *pNextProperty;
        bytesToMove = bytesAllocated - *pNextProperty;
        }
    else   // removing entries at beginning or in middle of array
        {
        SecondaryOffset endIndexValueOffset = GetOffsetOfArrayIndexValue (arrayOffset, propertyLayout, removeIndex+removeCount);
        source = data + endIndexValueOffset;

        bytesToMove = bytesAllocated - endIndexValueOffset;
        }

    if (bytesToMove <= 0)
        return ECOBJECTS_STATUS_Error;

    // remove the array values
    memmove (destination, source, bytesToMove);
    totalBytesAdjusted += (UInt32)(source - destination);

    UInt32           preNullFlagBitmasksCount = CalculateNumberNullFlagsBitmasks (preArrayCount);   
    UInt32           postNullFlagBitmasksCount = CalculateNumberNullFlagsBitmasks (postArrayCount);
    UInt32           nullFlagsDelta = 0;
    SecondaryOffset* firstArrayEntryOffset = (SecondaryOffset *)(data + GetOffsetOfArrayIndex (arrayOffset, propertyLayout, 0));

    if (!hasFixedSizedElements)
        {
        // remove secondary offsets to array entry values
        SecondaryOffset  beginIndexValueOffset = GetOffsetOfArrayIndex (arrayOffset, propertyLayout, removeIndex);
        byte *           destination = data + beginIndexValueOffset;  
        UInt32           offsetDelta = removeCount * sizeof(SecondaryOffset);

        source      = destination + offsetDelta;    
        bytesToMove = bytesAllocated - (beginIndexValueOffset+offsetDelta);

        memmove (destination, source, bytesToMove);
        totalBytesAdjusted += (UInt32)(source - destination);

        if (preNullFlagBitmasksCount != postNullFlagBitmasksCount)
            nullFlagsDelta = ((preNullFlagBitmasksCount-postNullFlagBitmasksCount)*sizeof(NullflagsBitmask));

        // now adjust the offsets to the entries in the array
        for (UInt32 iOffset=0; iOffset<postArrayCount; iOffset++)
            {
            SecondaryOffset* pCurrOffset = firstArrayEntryOffset+iOffset;
            if (iOffset < removeIndex)
                *pCurrOffset -=  (offsetDelta + nullFlagsDelta);             // adjust by removed SecondaryOffsets
            else
                *pCurrOffset -=  (totalBytesAdjusted + nullFlagsDelta);      // adjust by removed values and SecondaryOffsets
            }
        }
           
    // update the null flags
    NullflagsBitmask* preNullflagsStart = (NullflagsBitmask*)(arrayAddress + sizeof (ArrayCount));
    NullflagsBitmask* pNullflagsCurrent;

    // update/shift the null flags
    UInt32 firstMoveIndex = removeIndex+removeCount;

    UInt32 group;
    UInt32 bit;
    UInt32 nullflagsBitmask;

    if (firstMoveIndex >= preArrayCount)  // removed last entries
        {
        for (UInt32 iRemove = removeIndex; iRemove < removeIndex+removeCount; iRemove++)
            {
            group = iRemove / BITS_PER_NULLFLAGSBITMASK;
            bit = iRemove % BITS_PER_NULLFLAGSBITMASK;
            nullflagsBitmask = 0x01 << bit;
            pNullflagsCurrent = preNullflagsStart + group;
            *pNullflagsCurrent |= nullflagsBitmask; // turn on the null bit
            }
        }
    else // removing entries from middle of array
        {
        UInt32 oldgroup;
        UInt32 oldbit;
        UInt32 oldnullflagsBitmask;
        NullflagsBitmask* pOldNullflagsCurrent;
        NullflagsBitmask* pNullflagsCurrent;
        bool isNull;

        for (UInt32 iRemove = removeIndex; iRemove < postArrayCount; iRemove++)
            {
            oldgroup = (iRemove+removeCount) / BITS_PER_NULLFLAGSBITMASK;
            oldbit = (iRemove+removeCount) % BITS_PER_NULLFLAGSBITMASK;
            oldnullflagsBitmask = 0x01 << oldbit;
            pOldNullflagsCurrent = preNullflagsStart + oldgroup;

            group = iRemove / BITS_PER_NULLFLAGSBITMASK;
            bit = iRemove % BITS_PER_NULLFLAGSBITMASK;
            nullflagsBitmask = 0x01 << bit; 
            pNullflagsCurrent = preNullflagsStart + group;
            isNull = (nullflagsBitmask == (*pNullflagsCurrent & nullflagsBitmask)); 

            if (isNull && 0 == (*pOldNullflagsCurrent & oldnullflagsBitmask))
                *pNullflagsCurrent ^= nullflagsBitmask; // turn off the null bit 
            else if (!isNull && oldnullflagsBitmask == (*pOldNullflagsCurrent & oldnullflagsBitmask))
                *pNullflagsCurrent |= nullflagsBitmask; // turn on the null bit
            }
        }

    if (preNullFlagBitmasksCount != postNullFlagBitmasksCount)
        {
        // now actually remove any unneeded NullFlagBitmask
        destination = arrayAddress + sizeof(ArrayCount) + (postNullFlagBitmasksCount*sizeof(NullflagsBitmask));
        source = (byte*)firstArrayEntryOffset;

        bytesToMove = bytesAllocated - (arrayOffset + sizeof(ArrayCount) + (postNullFlagBitmasksCount*sizeof(NullflagsBitmask)));

        memmove (destination, source, bytesToMove);
        totalBytesAdjusted += (UInt32)(source - destination);
        }

    // Update the array count
    memcpy (arrayAddress, &postArrayCount, sizeof(ArrayCount));
   
    SecondaryOffset * pLast = (SecondaryOffset*)(data + classLayout.GetSizeOfFixedSection() - sizeof(SecondaryOffset));

    // adjust the offsets in the fixed section beyond the one we just modified
    while (pNextProperty <= pLast)
        {
        *pNextProperty -= totalBytesAdjusted;
        pNextProperty++;
        }

    // replace all property data for the instance
    status = _ModifyData (0, data, bytesAllocated);

#ifdef DEBUGGING_ARRAYENTRY_REMOVAL           
    WString postDeleteLayout = InstanceDataToString (L"", classLayout);
    if (postDeleteLayout.empty())
        return status;
#endif

    return status;
    }
             
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Bill.Steinbock                  07/2011
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus MemoryInstanceSupport::RemoveArrayElements (ClassLayoutCR classLayout, PropertyLayoutCR propertyLayout, UInt32 removeIndex, UInt32 removeCount, EC::EmbeddedInstanceCallbackP memoryReallocationCallbackP)
    {
    ECTypeDescriptor typeDescriptor = propertyLayout.GetTypeDescriptor();
    PRECONDITION (typeDescriptor.IsArray() && 
        "Can not set the value to memory at an array index using the specified property layout because it is not an array datatype", 
        ECOBJECTS_STATUS_PreconditionViolated);  
                        
    if (removeIndex >= GetReservedArrayCount (propertyLayout))
        return ECOBJECTS_STATUS_IndexOutOfRange;  


#ifdef DEBUGGING_ARRAYENTRY_REMOVAL 
    ECObjectsStatus status = ECOBJECTS_STATUS_Error;
    WString preDeleteLayout = InstanceDataToString (L"", classLayout);
    if (preDeleteLayout.empty())
        return status;
#endif

    if (typeDescriptor.IsPrimitiveArray())
        return RemoveArrayElementsFromMemory (classLayout, propertyLayout, removeIndex, removeCount);
    else if (typeDescriptor.IsStructArray())              
        return _RemoveStructArrayElementsFromMemory (classLayout, propertyLayout, removeIndex, removeCount, memoryReallocationCallbackP);       

    POSTCONDITION (false && "Can not set the value to memory using the specified property layout because it is an unsupported datatype", ECOBJECTS_STATUS_DataTypeNotSupported);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Bill.Steinbock                  07/2011
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus MemoryInstanceSupport::RemoveArrayElementsAt (ClassLayoutCR classLayout, WCharCP propertyAccessString, UInt32 removeIndex, UInt32 removeCount)
    {        
    PRECONDITION (NULL != propertyAccessString, ECOBJECTS_STATUS_PreconditionViolated);
       
    PropertyLayoutCP pPropertyLayout = NULL;
    ECObjectsStatus status = classLayout.GetPropertyLayout (pPropertyLayout, propertyAccessString);
    if (SUCCESS != status || NULL == pPropertyLayout)
        return ECOBJECTS_STATUS_PropertyNotFound;

    return RemoveArrayElements (classLayout, *pPropertyLayout, removeIndex, removeCount);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Adam.Klatzkin                   01/2010
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus MemoryInstanceSupport::InsertNullArrayElementsAt (ClassLayoutCR classLayout, WCharCP propertyAccessString, UInt32 insertIndex, UInt32 insertCount, EC::EmbeddedInstanceCallbackP memoryReallocationCallbackP)
    {        
    PRECONDITION (NULL != propertyAccessString, ECOBJECTS_STATUS_PreconditionViolated);
                
    PropertyLayoutCP pPropertyLayout = NULL;
    ECObjectsStatus status = classLayout.GetPropertyLayout (pPropertyLayout, propertyAccessString);
    if (SUCCESS != status || NULL == pPropertyLayout)
        return ECOBJECTS_STATUS_PropertyNotFound;
            
    PropertyLayoutCR propertyLayout = *pPropertyLayout;
    // WIP_FUSION improve error codes
    bool isFixedCount = (propertyLayout.GetModifierFlags() & PROPERTYLAYOUTMODIFIERFLAGS_IsArrayFixedCount);
    PRECONDITION (!isFixedCount && propertyLayout.GetTypeDescriptor().IsArray() && "A variable size array property is required to grow an array", ECOBJECTS_STATUS_PreconditionViolated);
    
    PRECONDITION (insertCount > 0, ECOBJECTS_STATUS_IndexOutOfRange)        
    
    return ArrayResizer::CreateNullArrayElementsAt (classLayout, propertyLayout, *this, insertIndex, insertCount, memoryReallocationCallbackP);
    }
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Adam.Klatzkin                   01/2010
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus       MemoryInstanceSupport::AddNullArrayElementsAt (ClassLayoutCR classLayout, WCharCP propertyAccessString, UInt32 count, EC::EmbeddedInstanceCallbackP memoryReallocationCallbackP)
    {        
    PRECONDITION (NULL != propertyAccessString, ECOBJECTS_STATUS_PreconditionViolated);
                
    PropertyLayoutCP pPropertyLayout = NULL;
    ECObjectsStatus status = classLayout.GetPropertyLayout (pPropertyLayout, propertyAccessString);
    if (ECOBJECTS_STATUS_Success != status || NULL == pPropertyLayout)
        return ECOBJECTS_STATUS_PropertyNotFound; 
            
    PropertyLayoutCR propertyLayout = *pPropertyLayout;
    // WIP_FUSION improve error codes
    bool isFixedCount = (propertyLayout.GetModifierFlags() & PROPERTYLAYOUTMODIFIERFLAGS_IsArrayFixedCount);
    PRECONDITION (!isFixedCount && propertyLayout.GetTypeDescriptor().IsArray() && "A variable size array property is required to grow an array", ECOBJECTS_STATUS_PreconditionViolated);
    
    PRECONDITION (count > 0, ECOBJECTS_STATUS_IndexOutOfRange)        
    
    return ArrayResizer::CreateNullArrayElementsAt (classLayout, propertyLayout, *this, GetAllocatedArrayCount (propertyLayout), count, memoryReallocationCallbackP);
    }         

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Adam.Klatzkin                   01/2010
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus       MemoryInstanceSupport::EnsureSpaceIsAvailableForArrayIndexValue (ClassLayoutCR classLayout, PropertyLayoutCR propertyLayout, UInt32 arrayIndex, UInt32 bytesNeeded, EmbeddedInstanceCallbackP callbackP)
    {    
    UInt32 availableBytes = GetPropertyValueSize (propertyLayout, arrayIndex);
    
#ifndef SHRINK_TO_FIT    
    if (bytesNeeded <= availableBytes)
        return SUCCESS;
#endif    
    Int32 additionalBytesNeeded = bytesNeeded - availableBytes;
    
    if (additionalBytesNeeded <= 0)
        return ECOBJECTS_STATUS_Success;
    
    UInt32 arrayCount = GetAllocatedArrayCount (propertyLayout);
    
    UInt32 endOfValueDataPreGrow = *((SecondaryOffset*)(_GetData() + propertyLayout.GetOffset()) + 1);
    ECObjectsStatus status = GrowPropertyValue (classLayout, propertyLayout, additionalBytesNeeded, callbackP);

    if (ECOBJECTS_STATUS_Success != status)
        return status;

    if (arrayIndex < arrayCount - 1)
        return ShiftArrayIndexValueData(propertyLayout, arrayIndex, arrayCount, endOfValueDataPreGrow, additionalBytesNeeded);
    else
        return status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Adam.Klatzkin                   01/2010
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus       MemoryInstanceSupport::GrowPropertyValue (ClassLayoutCR classLayout, PropertyLayoutCR propertyLayout, UInt32 additionalBytesNeeded, EmbeddedInstanceCallbackP callbackP)
    {
    byte const * data = _GetData();
    UInt32 bytesUsed = classLayout.CalculateBytesUsed(data);
        
    UInt32 bytesAllocated = _GetBytesAllocated();    
    UInt32 additionalBytesAvailable = bytesAllocated - bytesUsed;
    
    ECObjectsStatus status = ECOBJECTS_STATUS_Success;
    if (additionalBytesNeeded > additionalBytesAvailable)
        {
        UInt32 growBy = additionalBytesNeeded - additionalBytesAvailable;
        status = _GrowAllocation (growBy, callbackP);
        UInt32 newBytesAllocated = _GetBytesAllocated();
        DEBUG_EXPECT (newBytesAllocated >= bytesAllocated + growBy);
        bytesAllocated = newBytesAllocated;
        }
    
    if (ECOBJECTS_STATUS_Success != status)
        return status;
        
    byte * writeableData = (byte *)_GetData();
    UInt32 revisedBytesUsed = classLayout.CalculateBytesUsed(writeableData);
    DEBUG_EXPECT (bytesUsed == revisedBytesUsed);
    
    status = ShiftValueData(classLayout, writeableData, bytesAllocated, propertyLayout, additionalBytesNeeded);

    DEBUG_EXPECT (0 == bytesUsed || (bytesUsed + additionalBytesNeeded == classLayout.CalculateBytesUsed(writeableData)));
    
    return status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    CaseyMullen     01/10
+---------------+---------------+---------------+---------------+---------------+------*/
MemoryInstanceSupport::MemoryInstanceSupport (bool allowWritingDirectlyToInstanceMemory) :
    m_allowWritingDirectlyToInstanceMemory (allowWritingDirectlyToInstanceMemory) 
    {
    }
        
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    CaseyMullen     10/09
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus       MemoryInstanceSupport::ShiftValueData(ClassLayoutCR classLayout, byte * data, UInt32 bytesAllocated, PropertyLayoutCR propertyLayout, Int32 shiftBy)
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
            return ECOBJECTS_STATUS_IndexOutOfRange;
            
        memmove (destination, source, bytesToMove); // WIP_FUSION: Use Modify data, instead. Need method from Keith. (D-60516)
        }

    // Shift all secondaryOffsets for variable-sized property values that follow the one that just got larger
    UInt32 sizeOfSecondaryOffsetsToShift = (UInt32)(((byte*)pLast - (byte*)pCurrent) + sizeof (SecondaryOffset));
    if (m_allowWritingDirectlyToInstanceMemory)
        {
        for (SecondaryOffset * pSecondaryOffset = pCurrent; 
             pSecondaryOffset < pLast && 0 != *pSecondaryOffset;  // stop when we hit a zero
             ++pSecondaryOffset)
            *pSecondaryOffset += shiftBy; 
        
        *pLast += shiftBy; // always do the last one
        return ECOBJECTS_STATUS_Success;
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
ECObjectsStatus       MemoryInstanceSupport::ShiftArrayIndexValueData(PropertyLayoutCR propertyLayout, UInt32 arrayIndex, UInt32 arrayCount, UInt32 endOfValueDataOffset, Int32 shiftBy)
    {
    DEBUG_EXPECT (0 != shiftBy && "It is a pointless waste of time to shift nothing");
    DEBUG_EXPECT (arrayIndex < arrayCount - 1 && "It is a pointless waste of time to shift nothing");

    byte * data = (byte*)_GetData();
    SecondaryOffset* pNextProperty = (SecondaryOffset*)(data + propertyLayout.GetOffset()) + 1;
    UInt32 arrayOffset = GetOffsetOfPropertyValue (propertyLayout);   
    SecondaryOffset indexValueOffset = GetOffsetOfArrayIndexValue (arrayOffset, propertyLayout, arrayIndex + 1); // start at the one AFTER the index whose value's size is adjusting
    
    UInt32 bytesToMove = endOfValueDataOffset - indexValueOffset;
    if (bytesToMove > 0)
        {
        byte * source = data + indexValueOffset;
        byte * destination = source + shiftBy;
        
        DEBUG_EXPECT (destination + bytesToMove <= data + *pNextProperty && "Attempted to move memory beyond the end of the reserved memory for this property.");
        if (destination + bytesToMove > data + *pNextProperty)
            return ECOBJECTS_STATUS_IndexOutOfRange;
            
        memmove (destination, source, bytesToMove); // WIP_FUSION: Use Modify data, instead  (D-60516)
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
ECObjectsStatus       MemoryInstanceSupport::GetPrimitiveValueFromMemory (ECValueR v, PropertyLayoutCR propertyLayout, bool useIndex, UInt32 index) const
    {
    DEBUG_EXPECT (propertyLayout.GetTypeDescriptor().IsArray() == useIndex);   

    bool isInUninitializedFixedCountArray = ((useIndex) && (propertyLayout.GetModifierFlags() & PROPERTYLAYOUTMODIFIERFLAGS_IsArrayFixedCount) && (GetAllocatedArrayCount (propertyLayout) == 0));    
    if (isInUninitializedFixedCountArray || (IsPropertyValueNull(propertyLayout, useIndex, index)))
        {
        v.SetPrimitiveType (propertyLayout.GetTypeDescriptor().GetPrimitiveType());
        v.SetToNull();
        return ECOBJECTS_STATUS_Success;
        }    

    byte const * pValue;
    if (useIndex)
        pValue = GetAddressOfPropertyValue (propertyLayout, index);
    else
        pValue = GetAddressOfPropertyValue (propertyLayout);

    ECTypeDescriptor typeDescriptor = propertyLayout.GetTypeDescriptor();
    PrimitiveType primitiveType;
    if (typeDescriptor.IsPrimitive() || (typeDescriptor.IsPrimitiveArray()))
        primitiveType = typeDescriptor.GetPrimitiveType();
    else
        primitiveType = PRIMITIVETYPE_Binary;
        
    switch (primitiveType)       
        {
        case PRIMITIVETYPE_Integer:
            {
            Int32 value;
            memcpy (&value, pValue, sizeof(value));
            v.SetInteger (value);
            return ECOBJECTS_STATUS_Success;
            }
        case PRIMITIVETYPE_Long:
            {
            Int64 value;
            memcpy (&value, pValue, sizeof(value));
            v.SetLong (value);
            return ECOBJECTS_STATUS_Success;
            }            
        case PRIMITIVETYPE_Double:
            {
            double value;
            memcpy (&value, pValue, sizeof(value));
            v.SetDouble (value);
            return ECOBJECTS_STATUS_Success;
            }
        case PRIMITIVETYPE_Binary:
            {
            UInt32 size;
            if (useIndex)
                size = GetPropertyValueSize (propertyLayout, index);
            else
                size = GetPropertyValueSize (propertyLayout);

            UInt32 const* actualSize   = (UInt32 const*)pValue;
            byte const*   actualBuffer = pValue+sizeof(UInt32);

            v.SetBinary (actualBuffer, *actualSize);
            return ECOBJECTS_STATUS_Success;
            }  
        case PRIMITIVETYPE_Boolean:
            {
            bool value;
            memcpy (&value, pValue, sizeof(value));
            v.SetBoolean (value);
            return ECOBJECTS_STATUS_Success;
            } 
        case PRIMITIVETYPE_Point2D:
            {
            DPoint2d value;
            memcpy (&value, pValue, sizeof(value));
            v.SetPoint2D (value);
            return ECOBJECTS_STATUS_Success;
            }       
        case PRIMITIVETYPE_Point3D:
            {
            DPoint3d value;
            memcpy (&value, pValue, sizeof(value));
            v.SetPoint3D (value);
            return ECOBJECTS_STATUS_Success;
            }       
        case PRIMITIVETYPE_DateTime:
            {
            Int64 value;
            memcpy (&value, pValue, sizeof(value));
            v.SetDateTimeTicks (value);
            return ECOBJECTS_STATUS_Success;
            } 
        case PRIMITIVETYPE_String:
            {
            WCharP pString = (WCharP)pValue;
            v.SetString (pString, false); // WIP_FUSION: We are passing false for "makeDuplicateCopy" to avoid the allocation 
                                          // and copying... but how do make the caller aware of this? When do they need 
                                          // to be aware. The wchar_t* they get back would get invalidated if the 
                                          // XAttribute or other IMemoryProvider got reallocated, or the string got moved.
                                          // The caller must immediately use (e.g. marshal or copy) the returned value.
                                          // Optionally, the caller could ask the EC::ECValue to make a duplicate? 
            return ECOBJECTS_STATUS_Success;            
            }
        }

    POSTCONDITION (false && "datatype not implemented", ECOBJECTS_STATUS_DataTypeNotSupported);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Adam.Klatzkin                   01/2010
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus       MemoryInstanceSupport::GetValueFromMemory (ECValueR v, PropertyLayoutCR propertyLayout) const
    {
    ECTypeDescriptor typeDescriptor = propertyLayout.GetTypeDescriptor();
    if (typeDescriptor.IsPrimitive())
        {
        return GetPrimitiveValueFromMemory (v, propertyLayout, false, 0);
        }
    if (typeDescriptor.IsStruct())
        {
        // WIP_FUSION: Currently this value just serves as a placeholder, but its basically useless.
        v.SetStruct (NULL);
        return ECOBJECTS_STATUS_Success;
        }
    else if (typeDescriptor.IsArray())
        {                
        UInt32 arrayCount = GetReservedArrayCount (propertyLayout);  
        bool isFixedArrayCount = propertyLayout.GetModifierFlags() & PROPERTYLAYOUTMODIFIERFLAGS_IsArrayFixedCount;                            
        if (typeDescriptor.IsPrimitiveArray())
            return v.SetPrimitiveArrayInfo (typeDescriptor.GetPrimitiveType(), arrayCount, isFixedArrayCount);
        else if (typeDescriptor.IsStructArray())
            return v.SetStructArrayInfo (arrayCount, isFixedArrayCount);
        }
        
    POSTCONDITION (false && "Can not obtain value from memory using the specified property layout because it is an unsupported datatype", ECOBJECTS_STATUS_DataTypeNotSupported);        
    }
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Adam.Klatzkin                   01/2010
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus       MemoryInstanceSupport::GetValueFromMemory (ECValueR v, PropertyLayoutCR propertyLayout, UInt32 index) const
    {
    ECTypeDescriptor typeDescriptor = propertyLayout.GetTypeDescriptor();
    PRECONDITION (typeDescriptor.IsArray() && 
        "Can not obtain value from memory at an array index using the specified property layout because it is not an array datatype", 
        ECOBJECTS_STATUS_PreconditionViolated);    
           
    UInt32 arrayCount = GetReservedArrayCount (propertyLayout);                      
    if (index >= arrayCount)
        return ECOBJECTS_STATUS_IndexOutOfRange;             

    if (typeDescriptor.IsPrimitiveArray())
        return GetPrimitiveValueFromMemory (v, propertyLayout, true, index);
    else if (typeDescriptor.IsStructArray())
        return _GetStructArrayValueFromMemory (v, propertyLayout, index);       
        
    POSTCONDITION (false && "Can not obtain value from memory using the specified property layout because it is an unsupported datatype", ECOBJECTS_STATUS_DataTypeNotSupported);        
    }    
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    CaseyMullen     09/09
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus       MemoryInstanceSupport::GetValueFromMemory (ClassLayoutCR classLayout, ECValueR v, WCharCP propertyAccessString, bool useIndex, UInt32 index) const
    {
    PRECONDITION (NULL != propertyAccessString, ECOBJECTS_STATUS_PreconditionViolated);
                
    PropertyLayoutCP propertyLayout = NULL;
    ECObjectsStatus status = classLayout.GetPropertyLayout (propertyLayout, propertyAccessString);
    if (ECOBJECTS_STATUS_Success != status || NULL == propertyLayout)
        return ECOBJECTS_STATUS_PropertyNotFound;

    if (useIndex)
        return GetValueFromMemory (v, *propertyLayout, index);
    else
        return GetValueFromMemory (v, *propertyLayout);
    }    

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    05/10
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus  MemoryInstanceSupport::GetValueFromMemory (ClassLayoutCR classLayout, ECValueR v, UInt32 propertyIndex, bool useArrayIndex, UInt32 arrayIndex) const
    {
    PropertyLayoutCP propertyLayout = NULL;
    ECObjectsStatus status = classLayout.GetPropertyLayoutByIndex (propertyLayout, propertyIndex);
    if (ECOBJECTS_STATUS_Success != status || NULL == propertyLayout)
        return ECOBJECTS_STATUS_PropertyNotFound;    

    if (useArrayIndex)
        return GetValueFromMemory (v, *propertyLayout, arrayIndex);
    else
        return GetValueFromMemory (v, *propertyLayout);
    }    

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    CaseyMullen     09/09
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus       MemoryInstanceSupport::SetPrimitiveValueToMemory (ECValueCR v, ClassLayoutCR classLayout, PropertyLayoutCR propertyLayout, bool useIndex, UInt32 index)
    {        
    bool isInUninitializedFixedCountArray = ((useIndex) && (propertyLayout.GetModifierFlags() & PROPERTYLAYOUTMODIFIERFLAGS_IsArrayFixedCount) && (GetAllocatedArrayCount (propertyLayout) == 0));
            
    bool  isOriginalValueNull = IsPropertyValueNull (propertyLayout, useIndex, index);

    if (v.IsNull())
        {
        if (!isInUninitializedFixedCountArray)
            SetPropertyValueNull (propertyLayout, useIndex, index, true);

        if (isOriginalValueNull)
            return ECOBJECTS_STATUS_PropertyValueMatchesNoChange;

        return ECOBJECTS_STATUS_Success;
        } 
#ifdef REMOVE // Casey decided that the ReadOnlyProperty setting was for GUI use only and that it should not be enforced at the API level  9/11
    else
        {
        // to match ECF, if the property is marked as read only then we only allow setting the value if the current value is NULL
        if (!isOriginalValueNull && propertyLayout.IsReadOnlyProperty())
            return ECOBJECTS_STATUS_UnableToSetReadOnlyProperty;
        }
#endif

    if (isInUninitializedFixedCountArray)
        {
        ArrayResizer::CreateNullArrayElementsAt (classLayout, propertyLayout, *this, 0, GetReservedArrayCount (propertyLayout));
        }   

    SetPropertyValueNull (propertyLayout, useIndex, index, false);            
    
    UInt32 offset = GetOffsetOfPropertyValue (propertyLayout, useIndex, index);

#ifdef EC_TRACE_MEMORY 
    wprintf (L"SetValue %s of 0x%x at offset=%d to %s.\n", propertyAccessString, this, offset, v.ToString().c_str());
#endif    
    ECTypeDescriptor typeDescriptor = propertyLayout.GetTypeDescriptor();
    PrimitiveType primitiveType;
    if (typeDescriptor.IsPrimitive() || (typeDescriptor.IsPrimitiveArray()))
        primitiveType = typeDescriptor.GetPrimitiveType();
    else
        primitiveType = PRIMITIVETYPE_Binary;
        
    switch (primitiveType)
        {
        case PRIMITIVETYPE_Integer:
            {
            if (!v.IsInteger ())
                return ECOBJECTS_STATUS_DataTypeMismatch;

            Int32 value = v.GetInteger();

            if (!isOriginalValueNull && 0 == memcmp (_GetData() + offset, &value, sizeof(value)))
                return ECOBJECTS_STATUS_PropertyValueMatchesNoChange;

            // WIP_FUSION: would it speed things up to poke directly when m_allowWritingDirectlyToInstanceMemory is true?
            return _ModifyData (offset, &value, sizeof(value));
            }
        case PRIMITIVETYPE_Long:
            {
            if (!v.IsLong ())
                return ECOBJECTS_STATUS_DataTypeMismatch;

            Int64 value = v.GetLong();

            if (!isOriginalValueNull && 0 == memcmp (_GetData() + offset, &value, sizeof(value)))
                return ECOBJECTS_STATUS_PropertyValueMatchesNoChange;

            return _ModifyData (offset, &value, sizeof(value));
            }
        case PRIMITIVETYPE_Double:
            {
            if (!v.IsDouble ())
                return ECOBJECTS_STATUS_DataTypeMismatch;

            double value = v.GetDouble();

            if (!isOriginalValueNull && 0 == memcmp (_GetData() + offset, &value, sizeof(value)))
                return ECOBJECTS_STATUS_PropertyValueMatchesNoChange;

            return _ModifyData (offset, &value, sizeof(value));
            }       
        case PRIMITIVETYPE_String:
            {
            if (!v.IsString ())
                return ECOBJECTS_STATUS_DataTypeMismatch;

            WCharCP value = v.GetString();
            UInt32 bytesNeeded = (UInt32)(sizeof(wchar_t) * (wcslen(value) + 1)); // WIP_FUSION: what if the caller could tell us the size?

            UInt32 currentSize;
            if (useIndex)
                currentSize = GetPropertyValueSize (propertyLayout, index);
            else
                currentSize = GetPropertyValueSize (propertyLayout);

            if (!isOriginalValueNull && currentSize>=bytesNeeded && 0 == memcmp (_GetData() + offset, value, bytesNeeded))
                return ECOBJECTS_STATUS_PropertyValueMatchesNoChange;

            ECObjectsStatus status;

            if (useIndex)
                status = EnsureSpaceIsAvailableForArrayIndexValue (classLayout, propertyLayout, index, bytesNeeded, v.GetMemoryCallback());
            else
                status = EnsureSpaceIsAvailable (offset, classLayout, propertyLayout, bytesNeeded, v.GetMemoryCallback());
            if (ECOBJECTS_STATUS_Success != status)
                return status;

            // WIP_FUSION: would it speed things up to poke directly when m_allowWritingDirectlyToInstanceMemory is true?
            return _ModifyData (offset, value, bytesNeeded);
            }

        case PRIMITIVETYPE_Binary:
            {
            if (!v.IsBinary ())
                return ECOBJECTS_STATUS_DataTypeMismatch;

            UInt32 currentSize;
            if (useIndex)
                currentSize = GetPropertyValueSize (propertyLayout, index);
            else
                currentSize = GetPropertyValueSize (propertyLayout);

            size_t size;
            byte const * data = v.GetBinary (size);
            size_t totalSize = size + sizeof(UInt32);
            UInt32 propertySize = (UInt32)size;

            // set up a buffer that will hold both the size and the data
            byte* dataBuffer = (byte*)calloc (totalSize, sizeof(byte));
            memcpy (dataBuffer, &propertySize, sizeof(UInt32));
            memcpy (dataBuffer+sizeof(UInt32), data, size);

            UInt32 bytesNeeded = (UInt32)totalSize;

            if (!isOriginalValueNull && currentSize>=totalSize && 0 == memcmp (_GetData() + offset, dataBuffer, bytesNeeded))
                return ECOBJECTS_STATUS_PropertyValueMatchesNoChange;

            ECObjectsStatus status;
            if (useIndex)
                status = EnsureSpaceIsAvailableForArrayIndexValue (classLayout, propertyLayout, index, bytesNeeded, v.GetMemoryCallback());
            else
                status = EnsureSpaceIsAvailable (offset, classLayout, propertyLayout, bytesNeeded, v.GetMemoryCallback());
            if (ECOBJECTS_STATUS_Success != status)
                return status;
                
            // WIP_FUSION We need to figure out the story for value size.  It is legitamate to have a non-null binary value that is
            // 0 bytes in length.  Currently, we do not track that length anywhere.  How do we capture this in the case that the binary value was previously set to some
            // value > 0 in length and we are not auto compressing property values.
            if (bytesNeeded == 0)
                return ECOBJECTS_STATUS_Success;
            
            // WIP_FUSION: would it speed things up to poke directly when m_allowWritingDirectlyToInstanceMemory is true?    
            ECObjectsStatus result = _ModifyData (offset, dataBuffer, bytesNeeded);
            free (dataBuffer);
            return result;
            }
        case PRIMITIVETYPE_Boolean:
            {
            if (!v.IsBoolean ())
                return ECOBJECTS_STATUS_DataTypeMismatch;

            bool value = v.GetBoolean();

            if (!isOriginalValueNull && 0 == memcmp (_GetData() + offset, &value, sizeof(value)))
                return ECOBJECTS_STATUS_PropertyValueMatchesNoChange;

            return _ModifyData (offset, &value, sizeof(value));
            }       
        case PRIMITIVETYPE_Point2D:
            {
            if (!v.IsPoint2D ())
                return ECOBJECTS_STATUS_DataTypeMismatch;

            DPoint2d value = v.GetPoint2D();

            if (!isOriginalValueNull && 0 == memcmp (_GetData() + offset, &value, sizeof(value)))
                return ECOBJECTS_STATUS_PropertyValueMatchesNoChange;

            return _ModifyData (offset, &value, sizeof(value));
            }       
        case PRIMITIVETYPE_Point3D:
            {
            if (!v.IsPoint3D ())
                return ECOBJECTS_STATUS_DataTypeMismatch;

            DPoint3d value = v.GetPoint3D();

            if (!isOriginalValueNull && 0 == memcmp (_GetData() + offset, &value, sizeof(value)))
                return ECOBJECTS_STATUS_PropertyValueMatchesNoChange;

            return _ModifyData (offset, &value, sizeof(value));
            } 
        case PRIMITIVETYPE_DateTime:      // stored as long
            {
            if (!v.IsDateTime ())
                return ECOBJECTS_STATUS_DataTypeMismatch;

            Int64 value = v.GetDateTimeTicks();

            if (!isOriginalValueNull && 0 == memcmp (_GetData() + offset, &value, sizeof(value)))
                return ECOBJECTS_STATUS_PropertyValueMatchesNoChange;

            return _ModifyData (offset, &value, sizeof(value));
            }
        }

    POSTCONDITION (false && "datatype not implemented", ECOBJECTS_STATUS_DataTypeNotSupported);
    }                
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Adam.Klatzkin                   01/2010
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus       MemoryInstanceSupport::SetValueToMemory (ECValueCR v, ClassLayoutCR classLayout, PropertyLayoutCR propertyLayout)
    {
    ECTypeDescriptor typeDescriptor = propertyLayout.GetTypeDescriptor();
    if (typeDescriptor.IsPrimitive())
        return SetPrimitiveValueToMemory (v, classLayout, propertyLayout, false, 0);

    POSTCONDITION (false && "Can not set the value to memory using the specified property layout because it is an unsupported datatype", ECOBJECTS_STATUS_DataTypeNotSupported);
    }      
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Adam.Klatzkin                   01/2010
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus       MemoryInstanceSupport::SetValueToMemory (ECValueCR v, ClassLayoutCR classLayout, PropertyLayoutCR propertyLayout, UInt32 index)
    {   
    ECTypeDescriptor typeDescriptor = propertyLayout.GetTypeDescriptor();
    PRECONDITION (typeDescriptor.IsArray() && 
        "Can not set the value to memory at an array index using the specified property layout because it is not an array datatype", 
        ECOBJECTS_STATUS_PreconditionViolated);  
                        
    if (index >= GetReservedArrayCount (propertyLayout))
        return ECOBJECTS_STATUS_IndexOutOfRange;

    if (typeDescriptor.IsPrimitiveArray())
        return SetPrimitiveValueToMemory (v, classLayout, propertyLayout, true, index);
    else if (typeDescriptor.IsStructArray()) // WIP_FUSION ensure that the struct is valid to set as an index in this array.              
        return _SetStructArrayValueToMemory (v, classLayout, propertyLayout, index);       

    POSTCONDITION (false && "Can not set the value to memory using the specified property layout because it is an unsupported datatype", ECOBJECTS_STATUS_DataTypeNotSupported);
    }     
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    CaseyMullen     09/09
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus       MemoryInstanceSupport::SetValueToMemory (ClassLayoutCR classLayout, WCharCP propertyAccessString, ECValueCR v, bool useIndex, UInt32 index)
    {
    PRECONDITION (NULL != propertyAccessString, ECOBJECTS_STATUS_PreconditionViolated);
                
    PropertyLayoutCP propertyLayout = NULL;
    ECObjectsStatus status = classLayout.GetPropertyLayout (propertyLayout, propertyAccessString);
    if (ECOBJECTS_STATUS_Success != status || NULL == propertyLayout)
        return ECOBJECTS_STATUS_PropertyNotFound;       

    if (useIndex)
        return SetValueToMemory (v, classLayout, *propertyLayout, index);
    else
        return SetValueToMemory (v, classLayout, *propertyLayout);
    }     

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    05/10
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus       MemoryInstanceSupport::SetValueToMemory (ClassLayoutCR classLayout, UInt32 propertyIndex, ECValueCR v, bool useArrayIndex, UInt32 arrayIndex)
    {
    PropertyLayoutCP propertyLayout = NULL;
    ECObjectsStatus status = classLayout.GetPropertyLayoutByIndex (propertyLayout, propertyIndex);
    if (ECOBJECTS_STATUS_Success != status || NULL == propertyLayout)
        return ECOBJECTS_STATUS_PropertyNotFound;   

    if (useArrayIndex)
        return SetValueToMemory (v, classLayout, *propertyLayout, arrayIndex);
    else
        return SetValueToMemory (v, classLayout, *propertyLayout);
    }     

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    CaseyMullen     10/09
+---------------+---------------+---------------+---------------+---------------+------*/
WString        MemoryInstanceSupport::InstanceDataToString (WCharCP indent, ClassLayoutCR classLayout) const
    {
    static bool s_skipDump = false;
    static int s_dumpCount = 0;
    s_dumpCount++;
    
    wostringstream oss;

    appendFormattedString (oss, L"%s================== ECInstance Dump #%d =============================\n", indent, s_dumpCount);
    if (s_skipDump)
        return oss.str().c_str();
  
    byte const * data = _GetData();

    appendFormattedString (oss, L"%sECClass=%s at address = 0x%0x\n", indent, classLayout.GetECClassName().c_str(), data);
    InstanceHeader& header = *(InstanceHeader*)data;

    appendFormattedString (oss, L"%s  [0x%0x][%4.d] SchemaIndex = %d\n", indent,        &header.m_schemaIndex,  (byte*)&header.m_schemaIndex   - data, header.m_schemaIndex);
    appendFormattedString (oss, L"%s  [0x%0x][%4.d] ClassIndex  = %d\n", indent,        &header.m_classIndex,   (byte*)&header.m_classIndex    - data, header.m_classIndex);
    appendFormattedString (oss, L"%s  [0x%0x][%4.d] InstanceFlags = 0x%08.x\n", indent, &header.m_instanceFlags,(byte*)&header.m_instanceFlags - data, header.m_instanceFlags);
    
    UInt32 nProperties = classLayout.GetPropertyCount ();
    UInt32 nNullflagsBitmasks = CalculateNumberNullFlagsBitmasks (nProperties);
    
    for (UInt32 i = 0; i < nNullflagsBitmasks; i++)
        {
        UInt32 offset = sizeof(InstanceHeader) + i * sizeof(NullflagsBitmask);
        byte const * address = offset + data;
        appendFormattedString (oss, L"%s  [0x%x][%4.d] Nullflags[%d] = 0x%x\n", indent, address, offset, i, *(NullflagsBitmask*)(data + offset));
        }
    
    for (UInt32 i = 0; i < nProperties; i++)
        {
        PropertyLayoutCP propertyLayout;
        ECObjectsStatus status = classLayout.GetPropertyLayoutByIndex (propertyLayout, i);
        if (ECOBJECTS_STATUS_Success != status)
            {
            appendFormattedString (oss, L"%sError (%d) returned while getting PropertyLayout #%d", indent, status, i);
            return oss.str().c_str();
            }

        if (propertyLayout->GetTypeDescriptor().IsStruct())
            {
            appendFormattedString (oss, L"%s  Struct %s\n", indent, propertyLayout->GetAccessString());
            continue;
            }

        UInt32 offset = propertyLayout->GetOffset();
        byte const * address = data + offset;
            
        ECValue v;
        GetValueFromMemory (v, *propertyLayout);
        WString valueAsString = v.ToString();
           
        if (propertyLayout->IsFixedSized())            
            appendFormattedString (oss, L"%s  [0x%x][%4.d] %s = %s\n", indent, address, offset, propertyLayout->GetAccessString(), valueAsString.c_str());
        else
            {
            SecondaryOffset secondaryOffset = *(SecondaryOffset*)address;
            byte const * realAddress = data + secondaryOffset;
            
            appendFormattedString (oss, L"%s  [0x%x][%4.d] -> [0x%x][%4.d] %s = %s\n", indent, address, offset, realAddress, secondaryOffset, propertyLayout->GetAccessString(), valueAsString.c_str());
            }
            
        if (propertyLayout->GetTypeDescriptor().IsArray())
            {
            UInt32 count = GetAllocatedArrayCount (*propertyLayout);
            if (count != GetReservedArrayCount (*propertyLayout))
                appendFormattedString (oss, L"      array has not yet been initialized\n");
            else
                {  
                if (count > 0)
                    {
                    UInt32 nullflagsOffset;
                    UInt32 nNullflagsBitmasks = CalculateNumberNullFlagsBitmasks (count);

                    if (propertyLayout->IsFixedSized())
                        nullflagsOffset = propertyLayout->GetOffset();
                    else
                        nullflagsOffset = *((SecondaryOffset*)(data + propertyLayout->GetOffset())) + sizeof (ArrayCount);

                    for (UInt32 i = 0; i < nNullflagsBitmasks; i++)
                        {
                        byte const * bitAddress = nullflagsOffset + data;

                        appendFormattedString (oss, L"%s  [0x%x][%4.d] Nullflags[%d] = 0x%x\n", indent, bitAddress, nullflagsOffset, i, *(NullflagsBitmask*)(bitAddress));
                        nullflagsOffset += sizeof(NullflagsBitmask);
                        }
                    }

                for (UInt32 i = 0; i < count; i++)
                    {                
                    offset = GetOffsetOfArrayIndex (GetOffsetOfPropertyValue (*propertyLayout), *propertyLayout, i);
                    address = data + offset;
                    status = GetValueFromMemory (v, *propertyLayout, i);
                    if (ECOBJECTS_STATUS_Success == status)            
                        valueAsString = v.ToString();                
                    else
                        {
                        wchar_t temp[1024];
                        swprintf(temp, 1024, L"Error (%d) returned while obtaining array index value", status, i);
                        valueAsString = WString (temp);
                        }

                    if (IsArrayOfFixedSizeElements (*propertyLayout))
                        appendFormattedString (oss, L"%s      [0x%x][%4.d] %d = %s\n", indent, address, offset, i, valueAsString.c_str());
                    else
                        {
                        SecondaryOffset secondaryOffset = GetOffsetOfArrayIndexValue (GetOffsetOfPropertyValue (*propertyLayout), *propertyLayout, i);
                        byte const * realAddress = data + secondaryOffset;
                        
                        appendFormattedString (oss, L"%s      [0x%x][%4.d] -> [0x%x][%4.d] %d = %s\n", indent, address, offset, realAddress, secondaryOffset, i, valueAsString.c_str());                    
                        }     
                    if ((ECOBJECTS_STATUS_Success == status) && (!v.IsNull()) && (v.IsStruct()))
                        {
                        WString structIndent(indent);
                        structIndent.append (L"      ");

                        WString structString = v.GetStruct()->ToString(structIndent.c_str());
                        oss << structString;

                        appendFormattedString (oss, L"%s=================== END Struct Instance ===========================\n", structIndent);
                        }         
                    }
                }
            }
        }
        
    UInt32 offsetOfLast = classLayout.GetSizeOfFixedSection() - sizeof(SecondaryOffset);
    SecondaryOffset * pLast = (SecondaryOffset*)(data + offsetOfLast);
    appendFormattedString (oss, L"%s  [0x%x][%4.d] Offset of TheEnd = %d\n", indent, pLast, offsetOfLast, *pLast);

    return oss.str().c_str();
    }
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Adam.Klatzkin                   02/2010
+---------------+---------------+---------------+---------------+---------------+------*/
ArrayResizer::ArrayResizer (ClassLayoutCR classLayout, PropertyLayoutCR propertyLayout, MemoryInstanceSupportR instance, UInt32 resizeIndex, UInt32 resizeElementCount) : m_classLayout (classLayout),
        m_propertyLayout (propertyLayout), m_instance (instance), m_resizeIndex (resizeIndex), m_resizeElementCount (resizeElementCount)    
    {                
    m_preAllocatedArrayCount = m_instance.GetAllocatedArrayCount (propertyLayout);    
            
    ECTypeDescriptor typeDescriptor = propertyLayout.GetTypeDescriptor();
    if (typeDescriptor.IsPrimitiveArray())
        m_elementType = typeDescriptor.GetPrimitiveType();
    else
        m_elementType = PRIMITIVETYPE_Binary;
    m_elementTypeIsFixedSize = IsArrayOfFixedSizeElements (propertyLayout);
    if (m_elementTypeIsFixedSize)
        m_elementSizeInFixedSection = ECValue::GetFixedPrimitiveValueSize (m_elementType);
    else
        m_elementSizeInFixedSection = sizeof (SecondaryOffset);
    
    m_preNullFlagBitmasksCount = CalculateNumberNullFlagsBitmasks (m_preAllocatedArrayCount);    
    m_preHeaderByteCount = (m_preAllocatedArrayCount == 0) ? 0 : sizeof (ArrayCount) + (m_preNullFlagBitmasksCount * sizeof (NullflagsBitmask));
    m_preFixedSectionByteCount = m_preHeaderByteCount + (m_preAllocatedArrayCount * m_elementSizeInFixedSection);
    m_preArrayByteCount = instance.GetPropertyValueSize (propertyLayout);
    
    m_postAllocatedArrayCount = m_preAllocatedArrayCount + m_resizeElementCount;
    m_postNullFlagBitmasksCount = CalculateNumberNullFlagsBitmasks (m_postAllocatedArrayCount);
    m_postHeaderByteCount = sizeof (ArrayCount) + (m_postNullFlagBitmasksCount * sizeof (NullflagsBitmask));
    m_postFixedSectionByteCount = m_postHeaderByteCount + (m_postAllocatedArrayCount * m_elementSizeInFixedSection);
    m_resizeFixedSectionByteCount = m_postFixedSectionByteCount - m_preFixedSectionByteCount;
    }    
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Adam.Klatzkin                   02/2010
+---------------+---------------+---------------+---------------+---------------+------*/    
ECObjectsStatus            ArrayResizer::ShiftDataFollowingResizeIndex ()
    {
    // shift all data (fixed & variable) to the right of the element where the resize should occur
    UInt32 offsetOfResizePoint = m_preHeaderByteCount + (m_resizeIndex * m_elementSizeInFixedSection);         
    m_pResizeIndexPreShift = m_data + m_arrayOffset + offsetOfResizePoint;
    m_pResizeIndexPostShift = m_pResizeIndexPreShift + m_resizeFixedSectionByteCount;        
    #ifndef NDEBUG
    UInt32 arrayByteCountAfterGrow = m_instance.GetPropertyValueSize (m_propertyLayout);
    byte const * pNextProperty = m_data + m_arrayOffset + arrayByteCountAfterGrow;
    DEBUG_EXPECT (pNextProperty == m_data + *((SecondaryOffset*)(m_data + m_propertyLayout.GetOffset()) + 1));
    #endif    
    if (m_preArrayByteCount > offsetOfResizePoint)
        {        
        UInt32 byteCountToShift = m_preArrayByteCount - offsetOfResizePoint;        
        DEBUG_EXPECT (m_pResizeIndexPostShift + byteCountToShift <= pNextProperty); 
        memmove ((byte*)m_pResizeIndexPostShift, m_pResizeIndexPreShift, byteCountToShift); // WIP_FUSION .. use _MoveData, waiting on Keith (D-60516)
        }
    
    return SetSecondaryOffsetsFollowingResizeIndex();        
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Adam.Klatzkin                   02/2010
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus            ArrayResizer::SetSecondaryOffsetsFollowingResizeIndex ()
    {
    // initialize the inserted secondary offsets and update all shifted secondary offsets         
    ECObjectsStatus status = ECOBJECTS_STATUS_Success;
    
    if (m_elementTypeIsFixedSize)
        return status;  // There are no secondary offsets to update
        
    byte * pWriteBuffer;              
    UInt32 sizeOfWriteBuffer = 0;                
    UInt32 nSecondaryOffsetsShifted;        
    if (m_preAllocatedArrayCount > m_resizeIndex)
        {
        nSecondaryOffsetsShifted = m_preAllocatedArrayCount - m_resizeIndex;
        m_postSecondaryOffsetOfResizeIndex = *((SecondaryOffset*)m_pResizeIndexPostShift) + m_resizeFixedSectionByteCount;
        }
    else
        {
        nSecondaryOffsetsShifted = 0;
        m_postSecondaryOffsetOfResizeIndex = m_instance.GetPropertyValueSize (m_propertyLayout); // this is a relative index, not absolute
        }
    DEBUG_EXPECT (m_postSecondaryOffsetOfResizeIndex <= m_instance.GetPropertyValueSize (m_propertyLayout));
     
    UInt32 insertedSecondaryOffsetByteCount = m_resizeElementCount * m_elementSizeInFixedSection;       
    SecondaryOffset* pSecondaryOffset = (SecondaryOffset*)(m_pResizeIndexPostShift - insertedSecondaryOffsetByteCount);
    if (m_instance.m_allowWritingDirectlyToInstanceMemory)
        pWriteBuffer = (byte*)pSecondaryOffset;
    else
        {
        sizeOfWriteBuffer = insertedSecondaryOffsetByteCount + (nSecondaryOffsetsShifted * sizeof (SecondaryOffset));
        pWriteBuffer = (byte*)alloca (sizeOfWriteBuffer);  
        }        
        
    // initialize inserted secondary offsets
    SecondaryOffset* pSecondaryOffsetWriteBuffer = (SecondaryOffset*)pWriteBuffer;
    for (UInt32 i = 0; i < m_resizeElementCount ; i++)
        pSecondaryOffsetWriteBuffer[i] = m_postSecondaryOffsetOfResizeIndex;
    
    // update shifted secondary offsets        
    for (UInt32 i = m_resizeElementCount; i < nSecondaryOffsetsShifted + m_resizeElementCount ;i++)
        {
        pSecondaryOffsetWriteBuffer[i] = pSecondaryOffset[i] + m_resizeFixedSectionByteCount;
        DEBUG_EXPECT (pSecondaryOffsetWriteBuffer[i] <= m_instance.GetPropertyValueSize (m_propertyLayout));
        }
        
    if (!m_instance.m_allowWritingDirectlyToInstanceMemory)
        {
        UInt32 modifyOffset = (UInt32)(m_pResizeIndexPostShift - m_data) - insertedSecondaryOffsetByteCount;
        status = m_instance._ModifyData (modifyOffset, pWriteBuffer, sizeOfWriteBuffer);            
        DEBUG_EXPECT (m_data + modifyOffset + sizeOfWriteBuffer <= m_data + m_arrayOffset + m_instance.GetPropertyValueSize (m_propertyLayout));
        if (ECOBJECTS_STATUS_Success != status)
            return status;        
        }            
        
    return status;    
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Adam.Klatzkin                   02/2010
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus            ArrayResizer::ShiftDataPreceedingResizeIndex ()    
    {    
    ECObjectsStatus status = ECOBJECTS_STATUS_Success;
    if (m_resizeIndex == 0)
        return status;
    
    byte const * pShiftFrom = m_data + m_arrayOffset + m_preHeaderByteCount;
    byte * pShiftTo = (byte*)(m_data + m_arrayOffset + m_postHeaderByteCount);
    UInt32 byteCountToShift = (UInt32)(m_pResizeIndexPreShift - pShiftFrom);                
    
    // shift all the elements in the fixed section preceding the insert point if we needed to grow the nullflags bitmask            
    if ((pShiftTo != pShiftFrom))
        {
        memmove (pShiftTo, pShiftFrom, byteCountToShift); // WIP_FUSION .. use _MoveData, waiting on Keith (D-60516)
        DEBUG_EXPECT (pShiftTo + byteCountToShift <= m_pResizeIndexPostShift); 
        }
        
    return SetSecondaryOffsetsPreceedingResizeIndex((SecondaryOffset*)pShiftTo, byteCountToShift);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Adam.Klatzkin                   02/2010
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus            ArrayResizer::SetSecondaryOffsetsPreceedingResizeIndex (SecondaryOffset* pSecondaryOffset, UInt32 byteCountToSet)            
    {
    // update all secondary offsets preceeding the insertion point since the size of the fixed section has changed and therefore all variable data has moved
    ECObjectsStatus status = ECOBJECTS_STATUS_Success;
    
    if (m_elementTypeIsFixedSize)
        return status;
    
    byte * pWriteBuffer;              
    UInt32 sizeOfWriteBuffer = 0;           
    if (m_instance.m_allowWritingDirectlyToInstanceMemory)
        pWriteBuffer = (byte*)pSecondaryOffset;
    else
        {
        sizeOfWriteBuffer = byteCountToSet;
        pWriteBuffer = (byte*)alloca (sizeOfWriteBuffer);  
        }        
                    
    // update shifted secondary offsets        
    SecondaryOffset* pSecondaryOffsetWriteBuffer = (SecondaryOffset*)pWriteBuffer;

    for (UInt32 i = 0; i < m_resizeIndex ;i++)
        {
        pSecondaryOffsetWriteBuffer[i] = pSecondaryOffset[i] + m_resizeFixedSectionByteCount;
        DEBUG_EXPECT (pSecondaryOffsetWriteBuffer[i] <= m_postSecondaryOffsetOfResizeIndex);
        }
        
    if (!m_instance.m_allowWritingDirectlyToInstanceMemory)
        {
        UInt32 modifyOffset = (UInt32)((byte*)pSecondaryOffset - m_data);
        status = m_instance._ModifyData (modifyOffset, pWriteBuffer, byteCountToSet);
        DEBUG_EXPECT (modifyOffset >= m_postHeaderByteCount);
        DEBUG_EXPECT (m_data + modifyOffset + byteCountToSet <= m_pResizeIndexPostShift);
        if (ECOBJECTS_STATUS_Success != status)
            return status;        
        }               
        
    return status;     
    }      

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Adam.Klatzkin                   02/2010
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus            ArrayResizer::WriteArrayHeader ()                
    {
    ECObjectsStatus status = ECOBJECTS_STATUS_Success;
    byte * pWriteBuffer;              
    UInt32 sizeOfWriteBuffer = 0;       
        
    // write the new array header (updated count & null flags)      
    if (m_instance.m_allowWritingDirectlyToInstanceMemory)
        pWriteBuffer = (byte*)(m_data + m_arrayOffset);
    else
        {  
        pWriteBuffer = (byte*)alloca (m_postHeaderByteCount);     
        sizeOfWriteBuffer = m_postHeaderByteCount;
        memcpy (pWriteBuffer, m_data + m_arrayOffset, m_preHeaderByteCount); 
        }        
    *((UInt32*)pWriteBuffer) = m_postAllocatedArrayCount;
    NullflagsBitmask* pNullflagsStart = (NullflagsBitmask*)(pWriteBuffer + sizeof (ArrayCount));
    NullflagsBitmask* pNullflagsCurrent;
    NullflagsBitmask  nullflagsBitmask;
    if (m_postNullFlagBitmasksCount > m_preNullFlagBitmasksCount)
        InitializeNullFlags (pNullflagsStart + m_preNullFlagBitmasksCount, m_postNullFlagBitmasksCount - m_preNullFlagBitmasksCount);
        
    // shift all the nullflags bits right    
    bool isNull;
    UInt32 numberShifted;
    if (m_preAllocatedArrayCount > m_resizeIndex)
        numberShifted = m_preAllocatedArrayCount - m_resizeIndex;
    else
        numberShifted = 0;
    for (UInt32 i = 1 ; i <= numberShifted ; i++)
        {
        pNullflagsCurrent = pNullflagsStart + ((m_preAllocatedArrayCount - i) / BITS_PER_NULLFLAGSBITMASK);
        nullflagsBitmask = 0x01 << ((m_preAllocatedArrayCount - i) % BITS_PER_NULLFLAGSBITMASK);
        isNull = (0 != (*pNullflagsCurrent & nullflagsBitmask));    
        
        pNullflagsCurrent = pNullflagsStart + ((m_postAllocatedArrayCount - i) / BITS_PER_NULLFLAGSBITMASK);
        nullflagsBitmask = 0x01 << ((m_postAllocatedArrayCount - i) % BITS_PER_NULLFLAGSBITMASK);        
        if (isNull && 0 == (*pNullflagsCurrent & nullflagsBitmask))
            *pNullflagsCurrent |= nullflagsBitmask; // turn on the null bit
        else if (!isNull && nullflagsBitmask == (*pNullflagsCurrent & nullflagsBitmask))
            *pNullflagsCurrent ^= nullflagsBitmask; // turn off the null bit // WIP_FUSION: Needs to use ModifyData   
        }
        
    // initilize inserted nullflags bits
    for (UInt32 i=0;i < m_resizeElementCount ; i++)
        {
        pNullflagsCurrent = pNullflagsStart + ((m_resizeIndex + i) / BITS_PER_NULLFLAGSBITMASK);
        nullflagsBitmask = 0x01 << ((m_resizeIndex + i) % BITS_PER_NULLFLAGSBITMASK);
        if (0 == (*pNullflagsCurrent & nullflagsBitmask))
            *pNullflagsCurrent |= nullflagsBitmask; // turn on the null bit        
        }
    if (!m_instance.m_allowWritingDirectlyToInstanceMemory)
        {
        status = m_instance._ModifyData (m_arrayOffset, pWriteBuffer, m_postHeaderByteCount);
        if (ECOBJECTS_STATUS_Success != status)
            return status;        
        }                       
        
    return status;
    }
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Adam.Klatzkin                   01/2010
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus       ArrayResizer::CreateNullArrayElementsAt (ClassLayoutCR classLayout, PropertyLayoutCR propertyLayout, MemoryInstanceSupportR instance, UInt32 insertIndex, UInt32 insertCount, EC::EmbeddedInstanceCallbackP memoryReallocationCallbackP)
    {                        
    ArrayResizer resizer (classLayout, propertyLayout, instance, insertIndex, insertCount);
    PRECONDITION (resizer.m_resizeIndex <= resizer.m_preAllocatedArrayCount, ECOBJECTS_STATUS_IndexOutOfRange);        
    
    ECObjectsStatus status = instance.EnsureSpaceIsAvailable (resizer.m_arrayOffset, classLayout, propertyLayout, resizer.m_preArrayByteCount + resizer.m_resizeFixedSectionByteCount, memoryReallocationCallbackP);
    if (ECOBJECTS_STATUS_Success != status)
        return status;       
        
    resizer.m_data = resizer.m_instance._GetData();
    status = resizer.ShiftDataFollowingResizeIndex();        
    if (ECOBJECTS_STATUS_Success != status)
        return status;
        
    status = resizer.ShiftDataPreceedingResizeIndex();        
    if (ECOBJECTS_STATUS_Success != status)
        return status;        
    
    status = resizer.WriteArrayHeader();

    return status;    
    // WIP_FUSION how do we deal with an error that occurs during the insert "transaction" after some data has already been moved/modified but we haven't finished?
    }    

END_BENTLEY_EC_NAMESPACE
