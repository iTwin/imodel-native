/*---------------------------------------------------------------------------------**//**
|
|     $Source: src/ECDBuffer.cpp $
|
|   $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include    "ECObjectsPch.h"
#include    <algorithm>
#if defined (_WIN32) // WIP_NONPORT - iostreams not support on Android
#include    <iomanip>
#endif

// SHRINK_TO_FIT will cause space reserved for variable-sized values to be reduced to the minimum upon every set operation.
// SHRINK_TO_FIT is not recommended and is mainly for testing. It it better to "compact" everything at once
#define SHRINK_TO_FIT 1 

using namespace std;

BEGIN_BENTLEY_ECOBJECT_NAMESPACE

enum ECDHeaderOffsets
    {
    ECDOFFSET_FormatVersion         = 0,
    ECDOFFSET_ReadableByVersion     = 1,
    ECDOFFSET_WritableByVersion     = 2,
    ECDOFFSET_HeaderSize            = 3,
    ECDOFFSET_Flags                 = 4
    };

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
void            appendFormattedString (WString& outStream, WCharCP fmtStr, ...)
    {
    WString line;
    va_list argList;
    va_start(argList, fmtStr /* the last fixed argument */);
    line.VSprintf (fmtStr, argList);
    va_end(argList);
    outStream.append (line);
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
        case PRIMITIVETYPE_IGeometry:
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
           
////////////////////////////////////////////////////////////////////////////////////////////////////////////
//  PropertyLayout inline methods
////////////////////////////////////////////////////////////////////////////////////////////////////////////
WCharCP                     PropertyLayout::GetAccessString() const     { return m_accessString.c_str(); }
UInt32                      PropertyLayout::GetParentStructIndex() const{ return m_parentStructIndex; }
UInt32                      PropertyLayout::GetOffset() const           { BeAssert ( ! m_typeDescriptor.IsStruct()); return m_offset; }
UInt32                      PropertyLayout::GetNullflagsOffset() const  { BeAssert ( ! m_typeDescriptor.IsStruct()); return m_nullflagsOffset; }
NullflagsBitmask            PropertyLayout::GetNullflagsBitmask() const { BeAssert ( ! m_typeDescriptor.IsStruct()); return m_nullflagsBitmask; }
ECTypeDescriptor            PropertyLayout::GetTypeDescriptor() const   { return m_typeDescriptor; }
UInt32                      PropertyLayout::GetModifierFlags() const    { return m_modifierFlags; }
UInt32                      PropertyLayout::GetModifierData() const     { return m_modifierData; }    
bool                        PropertyLayout::IsReadOnlyProperty () const {return PROPERTYLAYOUTMODIFIERFLAGS_IsReadOnly == (m_modifierFlags & PROPERTYLAYOUTMODIFIERFLAGS_IsReadOnly);}

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

    wchar_t line[1024];
    BeStringUtilities::Snwprintf (line, _countof(line), L"%-32s %-16s offset=%3i nullflagsOffset=%3i, nullflagsBitmask=0x%08.X", m_accessString.c_str(), typeName.c_str(), m_offset, m_nullflagsOffset, m_nullflagsBitmask);
        
    return line;
    }
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    CaseyMullen     10/09
+---------------+---------------+---------------+---------------+---------------+------*/
bool            PropertyLayout::IsFixedSized () const
    {
    if (m_typeDescriptor.IsStruct())
        return true;

    return ( ( m_typeDescriptor.IsPrimitive() || 
               (m_typeDescriptor.IsPrimitiveArray() && (m_modifierFlags & PROPERTYLAYOUTMODIFIERFLAGS_IsArrayFixedCount))
             ) && PrimitiveTypeIsFixedSize (m_typeDescriptor.GetPrimitiveType()));
    }
      
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   08/12
+---------------+---------------+---------------+---------------+---------------+------*/
bool PropertyLayout::HoldsCalculatedProperty() const
    {
    return 0 != (m_modifierFlags & PROPERTYLAYOUTMODIFIERFLAGS_IsCalculated);
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
        UInt32 fixedCount = m_modifierData;
        return CalculateFixedArrayPropertySize (fixedCount, m_typeDescriptor.GetPrimitiveType());
        }
        
    DEBUG_FAIL("Can not determine size in fixed section for datatype");
    return 0;
    }    

#define FNV1_32_INIT ((UInt32)0x811c9dc5)
#define FNV_32_PRIME ((UInt32)0x01000193)

/*---------------------------------------------------------------------------------**//**
*
*  perform a 32 bit Fowler/Noll/Vo hash on a buffer
*
* based on  http://www.isthe.com/chongo/src/fnv/hash_32.c
*
* input:
*	buf	- start of buffer to hash
*	len	- length of buffer in octets
*	hval	- previous hash value or 0 if first call
*
* returns:
*	32 bit hash as a static hash type
*
*
*
* @bsimethod                                    Bill.Steinbock                  12/2013
+---------------+---------------+---------------+---------------+---------------+------*/
static UInt32 ComputeFnvHashForString (WCharCP buf, UInt32 hashValue)
    {
    if (NULL == buf || 0==*buf)
        return hashValue;

    size_t len = wcslen(buf) * sizeof (WChar);

    unsigned char *bp = (unsigned char *)buf;	/* start of buffer */
    unsigned char *be = bp + len;		/* beyond end of buffer */

    /*
    * FNV-1 hash each octet in the buffer
    */
    while (bp < be) 
        {
        /* multiply by the 32 bit FNV magic prime mod 2^32 */
        hashValue *= FNV_32_PRIME;

        /* xor the bottom with the current octet */
        hashValue ^= (UInt32)*bp++;
        }

    /* return our new hash value */
    return hashValue;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Bill.Steinbock                  01/2012
+---------------+---------------+---------------+---------------+---------------+------*/
static UInt32 ComputeFnvHashForUInt32 (UInt32 buf, UInt32 hashValue)
    {
    unsigned char *bp = (unsigned char *)&buf;	/* start of buffer */
    unsigned char *be = bp + sizeof (buf);	/* beyond end of buffer */

    /*
    * FNV-1 hash each octet in the buffer
    */
    while (bp < be) 
        {
        /* multiply by the 32 bit FNV magic prime mod 2^32 */
        hashValue *= FNV_32_PRIME;

        /* xor the bottom with the current octet */
        hashValue ^= (UInt32)*bp++;
        }

    /* return our new hash value */
    return hashValue;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Bill.Steinbock                  01/2012
+---------------+---------------+---------------+---------------+---------------+------*/
UInt32  ClassLayout::ComputeCheckSum () const
    {
    UInt32 checkSum = FNV1_32_INIT;
    checkSum = ComputeFnvHashForUInt32 ((UInt32)m_propertyLayouts.size(), checkSum);

    for (PropertyLayoutP propertyP: m_propertyLayouts)
        {
        checkSum = ComputeFnvHashForUInt32 ((UInt32)propertyP->GetParentStructIndex(), checkSum);
        checkSum = ComputeFnvHashForUInt32 ((UInt32)propertyP->GetModifierFlags(), checkSum);
        checkSum = ComputeFnvHashForUInt32 ((UInt32)propertyP->GetModifierData(), checkSum);

        ECN::ECTypeDescriptor typeDescr = propertyP->GetTypeDescriptor();
        checkSum = ComputeFnvHashForUInt32 ((UInt32)typeDescr.GetTypeKind(), checkSum);
        checkSum = ComputeFnvHashForUInt32 ((UInt32)typeDescr.GetPrimitiveType(), checkSum);  // since we are in a union Primitive Type includes Array Type

        if (!propertyP->GetTypeDescriptor().IsStruct())
            {
            checkSum = ComputeFnvHashForUInt32 ((UInt32)propertyP->GetOffset(), checkSum);
            checkSum = ComputeFnvHashForUInt32 ((UInt32)propertyP->GetNullflagsOffset(), checkSum);
           }

        checkSum = ComputeFnvHashForString (propertyP->GetAccessString(), checkSum);
        }

    return checkSum;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Bill.Steinbock                  01/2012
+---------------+---------------+---------------+---------------+---------------+------*/
UInt32          ClassLayout::GetChecksum () const
    {
    if (0 == m_checkSum)
        m_checkSum = ComputeCheckSum ();

    return  m_checkSum;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   10/12
+---------------+---------------+---------------+---------------+---------------+------*/
bool            ClassLayout::Equals (ClassLayoutCR other) const
    {
    if (this == &other)
        return true;
    else
        return this->GetChecksum() == other.GetChecksum() && this->m_className.Equals (other.m_className);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    CaseyMullen     10/09
+---------------+---------------+---------------+---------------+---------------+------*/
ClassLayout::ClassLayout()
    :
    m_sizeOfFixedSection(0), 
    m_isRelationshipClass(false), m_propertyIndexOfSourceECPointer(-1), m_propertyIndexOfTargetECPointer(-1)
    {
    m_uniqueId = randomValue();
    m_checkSum = 0;
    };
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    11/10
+---------------+---------------+---------------+---------------+---------------+------*/
ClassLayout::~ClassLayout()
    {
    for (PropertyLayoutP layout: m_propertyLayouts)
        delete layout;

    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    11/10
+---------------+---------------+---------------+---------------+---------------+------*/
WString        ClassLayout::GetShortDescription () const
    {
    wchar_t line[1024];
    BeStringUtilities::Snwprintf (line, _countof(line), L"ClassLayout for ECClass.GetName()=%ls", m_className.c_str());

    return line;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    11/10
+---------------+---------------+---------------+---------------+---------------+------*/
WString        ClassLayout::GetName () const
    {
    return GetShortDescription();
    }

static WString  getIndentedFmt (int indentLevel)
    {
    return WPrintfString (L"%%-%dd", 12 - 3 * indentLevel);
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
    WString oss;

    for (UInt32 propIndex: it->second)
        {
        for (UInt32 i = 0; i < indentLevel; i++)
            oss += indentStr;

        oss += WPrintfString(getIndentedFmt(indentLevel).c_str(), propIndex);

        PropertyLayoutCP    propertyLayout = NULL;
        ECObjectsStatus     status = GetPropertyLayoutByIndex (propertyLayout, propIndex);

        if (SUCCESS != status || NULL == propertyLayout)
            {
            oss += L"  *** ERROR finding PropertyLayout ***\n";
            continue;
            }

        WCharCP  accessString = propertyLayout->GetAccessString();
        oss += WPrintfString (L"%-40ls  Parent: %-10d\n", accessString, propertyLayout->GetParentStructIndex());

        if (propertyLayout->GetTypeDescriptor().IsStruct())
            oss += LogicalStructureToString (propIndex, indentLevel+1); 
        }
    
    return oss;
    }    

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    CaseyMullen     10/09
+---------------+---------------+---------------+---------------+---------------+------*/
WString        ClassLayout::ToString () const
    {
    WString oss;

    oss += GetShortDescription() += L"\n";

    UInt32 propIndex = 0;

    for (PropertyLayoutP layout: m_propertyLayouts)
        {
        oss += WPrintfString(L"%-4d", propIndex);
        oss += layout->ToString();
        oss += L"\n";

        propIndex++;
        }

    oss += L"Logical Structure:\n";
    oss += LogicalStructureToString ();

    return oss;
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

    UInt32  nonStructPropCount = GetPropertyCountExcludingEmbeddedStructs();

    if (0 == nonStructPropCount)
        return;
        
    UInt32 nNullflagsBitmasks = CalculateNumberNullFlagsBitmasks (nonStructPropCount);
    InitializeNullFlags ((NullflagsBitmask *)data, nNullflagsBitmasks);
            
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

    if (0 != BeStringUtilities::Wcsicmp (GetECClassName().c_str(), classLayout.GetECClassName().c_str()))
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
UInt32          ClassLayout::CalculateBytesUsed(byte const * propertyData) const
    {
    // handle case when no properties are defined
    if (0 == m_propertyLayouts.size())
        return m_sizeOfFixedSection;
        
    PropertyLayoutCP lastPropertyLayout = m_propertyLayouts[m_propertyLayouts.size() - 1];
    if (lastPropertyLayout->IsFixedSized())
        return m_sizeOfFixedSection;

    SecondaryOffset * pLast = (SecondaryOffset*)(propertyData + m_sizeOfFixedSection - sizeof(SecondaryOffset));
    
    // pLast is the last offset, pointing to one byte beyond the used space, so it is equal to the number of bytes used, so far
    return *pLast;
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
        ECObjectsStatus status = m_underConstruction->GetPropertyIndex (parentStructIndex, parentAccessString.c_str());
        if (ECOBJECTS_STATUS_Success != status)
            BeAssert (false);
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
    m_underConstruction->AddPropertyLayout (accessString, *propertyLayout);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    CaseyMullen     10/09
+---------------+---------------+---------------+---------------+---------------+------*/    
void            ClassLayout::Factory::AddProperty (WCharCP accessString, ECTypeDescriptor typeDescriptor, UInt32 size, UInt32 modifierFlags, UInt32 modifierData)
    {
    UInt32  positionInCurrentNullFlags = m_nonStructPropertyCount % /*32*/ BITS_PER_NULLFLAGSBITMASK;
    NullflagsBitmask  nullflagsBitmask = 0x01 << positionInCurrentNullFlags;
    
    if (0 == positionInCurrentNullFlags && 0 != m_nonStructPropertyCount)
        {
        // It is time to add a new set of Nullflags
        m_nullflagsOffset += sizeof(NullflagsBitmask);
        m_offset          += sizeof(NullflagsBitmask);
        for (UInt32 i = 0; i < m_underConstruction->GetPropertyCount(); i++)
            {
            if ( ! m_underConstruction->m_propertyLayouts[i]->m_typeDescriptor.IsStruct())
                m_underConstruction->m_propertyLayouts[i]->m_offset += sizeof(NullflagsBitmask); // Offsets of already-added property layouts need to get bumped up
            }
        } 

    UInt32          parentStructIndex = GetParentStructIndex(accessString);
    PropertyLayoutP propertyLayout = new PropertyLayout (accessString, parentStructIndex, typeDescriptor, m_offset, m_nullflagsOffset, nullflagsBitmask, modifierFlags, modifierData);
    m_underConstruction->AddPropertyLayout (accessString, *propertyLayout); 

    m_offset += (UInt32)size;
    m_nonStructPropertyCount++;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    CaseyMullen     10/09
+---------------+---------------+---------------+---------------+---------------+------*/    
void            ClassLayout::Factory::AddFixedSizeProperty (WCharCP accessString, ECTypeDescriptor typeDescriptor, bool isReadOnly, bool isCalculated)
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

    if (isCalculated)
        modifierFlags |= PROPERTYLAYOUTMODIFIERFLAGS_IsCalculated;

    AddProperty (accessString, typeDescriptor, size, modifierFlags);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Adam.Klatzkin                   01/2010
+---------------+---------------+---------------+---------------+---------------+------*/ 
void            ClassLayout::Factory::AddFixedSizeArrayProperty (WCharCP accessString, ECTypeDescriptor typeDescriptor, UInt32 arrayCount, bool isReadOnly, bool isCalculated)
    {
    if (!EXPECTED_CONDITION (m_state == AcceptingFixedSizeProperties)) // ClassLayoutNotAcceptingFixedSizeProperties    
        return;
    
    UInt32 size = CalculateFixedArrayPropertySize (arrayCount, typeDescriptor.GetPrimitiveType());

    UInt32  modifierFlags = PROPERTYLAYOUTMODIFIERFLAGS_IsArrayFixedCount;
    if (isReadOnly)
        modifierFlags |= PROPERTYLAYOUTMODIFIERFLAGS_IsReadOnly; 

    if (isCalculated)
        modifierFlags |= PROPERTYLAYOUTMODIFIERFLAGS_IsCalculated;
   
    AddProperty (accessString, typeDescriptor, size, modifierFlags, arrayCount);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    CaseyMullen     10/09
+---------------+---------------+---------------+---------------+---------------+------*/ 
void            ClassLayout::Factory::AddVariableSizeProperty (WCharCP accessString, ECTypeDescriptor typeDescriptor, bool isReadOnly, bool isCalculated)
    {
    if (m_state == AcceptingFixedSizeProperties)
        m_state = AcceptingVariableSizeProperties;

    if (!EXPECTED_CONDITION (m_state == AcceptingVariableSizeProperties)) // ClassLayoutNotAcceptingVariableSizeProperties    
        return;
        
    UInt32 size = sizeof(SecondaryOffset); // the offset will just point to this secondary offset

    UInt32  modifierFlags = 0;
    if (isReadOnly)
        modifierFlags |= PROPERTYLAYOUTMODIFIERFLAGS_IsReadOnly; 

    if (isCalculated)
        modifierFlags |= PROPERTYLAYOUTMODIFIERFLAGS_IsCalculated;

    AddProperty (accessString, typeDescriptor, size, modifierFlags);
    }  

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Adam.Klatzkin                   01/2010
+---------------+---------------+---------------+---------------+---------------+------*/ 
void            ClassLayout::Factory::AddVariableSizeArrayPropertyWithFixedCount (WCharCP accessString, ECTypeDescriptor typeDescriptor, UInt32 arrayCount, bool isReadOnly, bool isCalculated)
    {
    if (m_state == AcceptingFixedSizeProperties)
        m_state = AcceptingVariableSizeProperties;

    if (!EXPECTED_CONDITION (m_state == AcceptingVariableSizeProperties)) // ClassLayoutNotAcceptingVariableSizeProperties    
        return;
        
    UInt32 size = sizeof(SecondaryOffset); // the offset will just point to this secondary offset
    UInt32 modifierFlags = PROPERTYLAYOUTMODIFIERFLAGS_IsArrayFixedCount;
    if (isReadOnly)
        modifierFlags |= PROPERTYLAYOUTMODIFIERFLAGS_IsReadOnly; 

    if (isCalculated)
        modifierFlags |= PROPERTYLAYOUTMODIFIERFLAGS_IsCalculated;

    AddProperty (accessString, typeDescriptor, size, modifierFlags, arrayCount);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    12/09
+---------------+---------------+---------------+---------------+---------------+------*/
void            ClassLayout::Factory::AddProperties (ECClassCR ecClass, WCharCP nameRoot, bool addingFixedSizeProps)
    {
    if (addingFixedSizeProps)
        AddStructProperty (NULL == nameRoot ? L"" : nameRoot, ECTypeDescriptor::CreateStructTypeDescriptor());

    for (ECPropertyP property: ecClass.GetProperties())
        {
        WString    propName = property->GetName();
        
        if (NULL != nameRoot)
            propName = WString (nameRoot).append (L".") + propName;

        if (property->GetIsPrimitive())
            {
            PrimitiveECPropertyP  primitiveProp = property->GetAsPrimitivePropertyP();
            PrimitiveType         primitiveType = primitiveProp->GetType();

            bool isFixedSize = PrimitiveTypeIsFixedSize(primitiveType);

            if (addingFixedSizeProps && isFixedSize)
                AddFixedSizeProperty (propName.c_str(), primitiveType, property->GetIsReadOnly(), primitiveProp->IsCalculated());
            else if ( ! addingFixedSizeProps && ! isFixedSize)
                AddVariableSizeProperty (propName.c_str(), primitiveType, property->GetIsReadOnly(), primitiveProp->IsCalculated());
            }
        else if (property->GetIsStruct())
            {
            StructECPropertyP  structProp = property->GetAsStructPropertyP();
            ECClassCR          nestedClass = structProp->GetType();
            
            AddProperties (nestedClass, propName.c_str(), addingFixedSizeProps);
            }
        else if (property->GetIsArray())
            {
            ArrayECPropertyP  arrayProp = property->GetAsArrayPropertyP();
            ArrayKind arrayKind = arrayProp->GetKind();
            if (arrayKind == ARRAYKIND_Primitive)
                {
                bool isFixedArrayCount = (arrayProp->GetMinOccurs() == arrayProp->GetMaxOccurs());
                bool isFixedPropertySize = isFixedArrayCount && PrimitiveTypeIsFixedSize (arrayProp->GetPrimitiveElementType());
                
                if (addingFixedSizeProps && isFixedPropertySize)
                    AddFixedSizeArrayProperty (propName.c_str(), ECTypeDescriptor::CreatePrimitiveArrayTypeDescriptor (arrayProp->GetPrimitiveElementType()), arrayProp->GetMinOccurs(), property->GetIsReadOnly(), arrayProp->IsCalculated());
                else if (!addingFixedSizeProps && !isFixedPropertySize)
                    {
                    if (isFixedArrayCount)
                        AddVariableSizeArrayPropertyWithFixedCount (propName.c_str(), ECTypeDescriptor::CreatePrimitiveArrayTypeDescriptor (arrayProp->GetPrimitiveElementType()), arrayProp->GetMinOccurs(), property->GetIsReadOnly(), arrayProp->IsCalculated());
                    else
                        AddVariableSizeProperty (propName.c_str(), ECTypeDescriptor::CreatePrimitiveArrayTypeDescriptor (arrayProp->GetPrimitiveElementType()), property->GetIsReadOnly(), arrayProp->IsCalculated());
                    }
                }
            else if ((arrayKind == ARRAYKIND_Struct) && (!addingFixedSizeProps))
                {
                bool isFixedArrayCount = (arrayProp->GetMinOccurs() == arrayProp->GetMaxOccurs());
                if (isFixedArrayCount)
                    AddVariableSizeArrayPropertyWithFixedCount (propName.c_str(), ECTypeDescriptor::CreateStructArrayTypeDescriptor(), arrayProp->GetMinOccurs(), property->GetIsReadOnly(), arrayProp->IsCalculated());
                else
                    AddVariableSizeProperty (propName.c_str(), ECTypeDescriptor::CreateStructArrayTypeDescriptor(), property->GetIsReadOnly(), arrayProp->IsCalculated()); 
                }
            }
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    01/10
+---------------+---------------+---------------+---------------+---------------+------*/
ClassLayoutPtr    ClassLayout::Factory::DoBuildClassLayout ()
    { //ECLogger::Log->debugv (L"Building ClassLayout for ECClass %ls", m_ecClass->GetName().c_str());
    m_underConstruction->m_isRelationshipClass = (dynamic_cast<ECRelationshipClassCP>(&m_ecClass) != NULL);

    // Iterate through the ECProperties of the ECClass and build the layout
    AddProperties (m_ecClass, NULL, true);
    AddProperties (m_ecClass, NULL, false);

    if (m_underConstruction->m_isRelationshipClass)
        {
        AddVariableSizeProperty (PROPERTYLAYOUT_Source_ECPointer, PRIMITIVETYPE_Binary, false, false);
        AddVariableSizeProperty (PROPERTYLAYOUT_Target_ECPointer, PRIMITIVETYPE_Binary, false, false);
        }

    m_underConstruction->FinishLayout ();
    m_underConstruction->m_checkSum = m_underConstruction->ComputeCheckSum ();

    return m_underConstruction;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    02/10
+---------------+---------------+---------------+---------------+---------------+------*/
ClassLayout::Factory::Factory (ECClassCR ecClass)
    : 
    m_ecClass (ecClass),
    m_state   (AcceptingFixedSizeProperties),
    m_offset  (sizeof(NullflagsBitmask)),
    m_nullflagsOffset (0),
    m_nonStructPropertyCount (0),
    m_underConstruction (ClassLayout::CreateEmpty (m_ecClass.GetName().c_str()))
    {
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    01/10
+---------------+---------------+---------------+---------------+---------------+------*/
ClassLayoutPtr    ClassLayout::BuildFromClass (ECClassCR ecClass)
    {
    Factory     factory (ecClass);

    return factory.DoBuildClassLayout ();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    01/10
+---------------+---------------+---------------+---------------+---------------+------*/
ClassLayoutPtr    ClassLayout::CreateEmpty (WCharCP className)
    {
    ClassLayoutP classLayout = new ClassLayout();

    classLayout->SetClass (className);

    return classLayout;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    CaseyMullen     10/09
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   ClassLayout::SetClass (WCharCP className)
    {
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
        m_sizeOfFixedSection = 0;
        }
    else
        {
        UInt32    size = lastPropertyLayout->GetOffset() + lastPropertyLayout->GetSizeInFixedSection();

        if ( ! lastPropertyLayout->IsFixedSized())
            size += sizeof(SecondaryOffset); // There is one additional SecondaryOffset tracking the end of the last variable-sized value

        m_sizeOfFixedSection = size;
        }

#ifndef NDEBUG
    DEBUG_EXPECT (m_indicesByAccessString.size() == m_propertyLayouts.size());

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
                DEBUG_EXPECT (m_propertyLayouts[i]->m_offset == nNullflagsBitmasks * sizeof(NullflagsBitmask));
            else
                DEBUG_EXPECT (m_propertyLayouts[i]->m_offset > prevNonStructOffset);
            }

        // Check the NullFlagsOffsets
        UInt32 expectedNullflagsOffset = 0;

        if ( ! isStruct)
            expectedNullflagsOffset = (nonStructPropIndex / BITS_PER_NULLFLAGSBITMASK * sizeof(NullflagsBitmask));

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
* @bsimethod                                                    Paul.Connelly   01/12
+---------------+---------------+---------------+---------------+---------------+------*/ 
ClassLayout::IndicesByAccessString::const_iterator ClassLayout::GetPropertyIndexPosition (WCharCP accessString, bool forCreate) const
    {
    // this vector is always sorted, so we can do binary search
    IndicesByAccessString::const_iterator begin   = m_indicesByAccessString.begin(),
                                    end           = m_indicesByAccessString.end(),
                                    it;
    size_t count = m_indicesByAccessString.size(), step;
    while (count > 0)
        {
        it = begin;
        step = count >> 1;
        it += step;
        int cmp = wcscmp (it->first, accessString);
        if (cmp == 0)           // easy out, avoiding redundant wcscmp() below
            {
            if (forCreate)
                DEBUG_FAIL ("Attempting to add a PropertyLayout with a duplicate name");
            else
                return it;
            }
        else if (cmp < 0)
            {
            begin = ++it;
            count -= step+1;
            }
        else
            count = step;
        }
    it = begin;

    if (forCreate)
        {
        if (it != end)
            DEBUG_EXPECT (0 != wcscmp (accessString, it->first) && "Cannot create a PropertyLayout with the same access string as an existing layout");
        return it;
        }
    else
        return (it != end && 0 == wcscmp (accessString, it->first)) ? it : end;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    CaseyMullen     10/09
+---------------+---------------+---------------+---------------+---------------+------*/    
void            ClassLayout::AddPropertyLayout (WCharCP accessString, PropertyLayoutR propertyLayout)
    {
    m_propertyLayouts.push_back(&propertyLayout);

    // if we knew no new PropertyLayouts could be added after construction, we could delay sorting the list until FinishLayout()
    // since AddPropertyLayout() is public we can't know that, so we need to insert in sorted order
    UInt32 index = (UInt32)(m_propertyLayouts.size() - 1);
    WCharCP plAccessString = propertyLayout.GetAccessString();
    AccessStringIndexPair newPair (plAccessString, index);
    IndicesByAccessString::iterator begin = m_indicesByAccessString.begin();
    IndicesByAccessString::iterator insertPos = begin + (GetPropertyIndexPosition (plAccessString, true) - begin);    // because constness...
    m_indicesByAccessString.insert (insertPos, newPair);

    AddToLogicalStructureMap (propertyLayout, index);
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
UInt32           nullflagsBitmask,
UInt32           modifierFlags,
UInt32           modifierData
)
    {
    PropertyLayoutP propertyLayout = new PropertyLayout (accessString, parentStructIndex, typeDescriptor, offset, nullflagsOffset, nullflagsBitmask, modifierFlags, modifierData);

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
    BeAssert (propertyIndex < m_propertyLayouts.size());
    if (propertyIndex >= m_propertyLayouts.size())
        return true; 
        
    return m_propertyLayouts[propertyIndex]->IsReadOnlyProperty (); 
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Bill.Steinbock                  09/2011
+---------------+---------------+---------------+---------------+---------------+------*/
bool            ClassLayout::SetPropertyReadOnly (UInt32 propertyIndex,  bool readOnly) const
    {
    BeAssert (propertyIndex < m_propertyLayouts.size());
    if (propertyIndex >= m_propertyLayouts.size())
        return false; 
        
    return m_propertyLayouts[propertyIndex]->SetReadOnlyMask (readOnly); 
    }    

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    CaseyMullen     10/09
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus       ClassLayout::GetPropertyLayout (PropertyLayoutCP & propertyLayout, WCharCP accessString) const
    {
    IndicesByAccessString::const_iterator pos = GetPropertyIndexPosition (accessString, false);

    if (pos == m_indicesByAccessString.end())
        return ECOBJECTS_STATUS_PropertyNotFound;
    else
        {
        propertyLayout = m_propertyLayouts[pos->second];
        return ECOBJECTS_STATUS_Success;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    05/10
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus     ClassLayout::GetPropertyIndex (UInt32& propertyIndex, WCharCP accessString) const
    {
    IndicesByAccessString::const_iterator pos = GetPropertyIndexPosition (accessString, false);
    if (pos == m_indicesByAccessString.end())
        return ECOBJECTS_STATUS_PropertyNotFound;

    propertyIndex = pos->second;
    return ECOBJECTS_STATUS_Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    CaseyMullen     10/09
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus       ClassLayout::GetPropertyLayoutByIndex (PropertyLayoutCP & propertyLayout, UInt32 propertyIndex) const
    {
    BeAssert (propertyIndex < m_propertyLayouts.size());
    if (propertyIndex >= m_propertyLayouts.size())
        return ECOBJECTS_STATUS_IndexOutOfRange; 
        
    propertyLayout = m_propertyLayouts[propertyIndex];
    return ECOBJECTS_STATUS_Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   08/12
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus ClassLayout::GetPropertyLayoutIndex (UInt32& propertyIndex, PropertyLayoutCR propertyLayout) const
    {
    for (size_t i = 0; i < m_propertyLayouts.size(); i++)
        if (m_propertyLayouts[i] == &propertyLayout)
            {
            propertyIndex = (UInt32)i;
            return ECOBJECTS_STATUS_Success;
            }

    return ECOBJECTS_STATUS_PropertyNotFound;
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
* @bsimethod                                                    Paul.Connelly   02/12
+---------------+---------------+---------------+---------------+---------------+------*/
bool ClassLayout::HasChildProperties (UInt32 parentIndex) const
    {
    PropertyLayoutCP propertyLayout;
    return ECOBJECTS_STATUS_Success == GetPropertyLayoutByIndex (propertyLayout, parentIndex) && propertyLayout->GetTypeDescriptor().IsStruct();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    01/10
+---------------+---------------+---------------+---------------+---------------+------*/
UInt32  ClassLayout::GetFirstChildPropertyIndex (UInt32 parentIndex) const
    {
    // Find the parent in the map
    bmap<UInt32, bvector<UInt32> >::const_iterator mapIterator = m_logicalStructureMap.find (parentIndex);

    if ( ! EXPECTED_CONDITION (m_logicalStructureMap.end() != mapIterator))
        return 0;

    // Return the first member of the parent's childList
    bvector<UInt32> const& childIndexList = mapIterator->second;

    return !childIndexList.empty() ? *childIndexList.begin() : 0;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Bill.Steinbock                  09/2011
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus ClassLayout::GetPropertyIndices (bvector<UInt32>& properties, UInt32 parentIndex) const
    {
    bmap<UInt32, bvector<UInt32> >::const_iterator mapIterator = m_logicalStructureMap.find (parentIndex);
    if ( ! EXPECTED_CONDITION (m_logicalStructureMap.end() != mapIterator))
        return ECOBJECTS_STATUS_Error;

    for (UInt32 propIndex: mapIterator->second)
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
    bmap<UInt32, bvector<UInt32> >::const_iterator mapIterator = m_logicalStructureMap.find (parentIndex);

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
* @bsimethod                                    Bill.Steinbock                  01/2012
+---------------+---------------+---------------+---------------+---------------+------*/
/*static*/ SchemaLayoutP   SchemaLayout::Create (SchemaIndex index)
    {
    return new  SchemaLayout (index);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Bill.Steinbock                  01/2012
+---------------+---------------+---------------+---------------+---------------+------*/
SchemaIndex      SchemaLayout::GetSchemaIndex() const 
    {
    return m_schemaIndex; 
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    01/10
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   SchemaLayout::AddClassLayout (ClassLayoutR classLayout, ClassIndex classIndex)
    {
    if (m_classLayouts.size() <= classIndex)
        m_classLayouts.resize (20 + classIndex, NULL); 

    BeAssert (m_classLayouts[classIndex].IsNull() && "ClassIndex is already in use");

    m_classLayouts[classIndex] = &classLayout;
    
    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    01/10
+---------------+---------------+---------------+---------------+---------------+------*/
ClassLayoutCP   SchemaLayout::GetClassLayout (ClassIndex classIndex) const
    {
    if (m_classLayouts.size() <= classIndex)
        return NULL;

    return m_classLayouts[classIndex].get();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    01/10
+---------------+---------------+---------------+---------------+---------------+------*/
ClassLayoutCP   SchemaLayout::FindClassLayout (WCharCP className) const
    {
    for (ClassLayoutPtr const& classLayout: m_classLayouts)
        {
        if (classLayout.IsNull())
            continue;

        if (0 == wcscmp (classLayout->GetECClassName().c_str(), className))
            return classLayout.get();
        }

    return NULL;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   11/13
+---------------+---------------+---------------+---------------+---------------+------*/
ClassLayoutCP SchemaLayout::FindClassLayout (WCharCP className, ClassIndex* classIndex) const
    {
    for (size_t i = 0; i < m_classLayouts.size(); i++)
        {
        ClassLayoutCP layout = m_classLayouts[i].get();
        if (NULL != layout && 0 == wcscmp (layout->GetECClassName().c_str(), className))
            {
            if (NULL != classIndex)
                *classIndex = (ClassIndex)i;

            return layout;
            }
        }

    return NULL;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   07/12
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   SchemaLayout::FindClassIndex (ClassIndex& classIndex, WCharCP className) const
    {
    for (size_t i = 0; i < m_classLayouts.size(); i++)
        {
        if (NULL != m_classLayouts[i].get() && 0 == BeStringUtilities::Wcsicmp (m_classLayouts[i]->GetECClassName().c_str(), className))
            {
            classIndex = (ClassIndex)i;
            return SUCCESS;
            }
        }

    return ERROR;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    CaseyMullen    02/10
+---------------+---------------+---------------+---------------+---------------+------*/
UInt32          SchemaLayout::GetMaxIndex() const
    {
    return (UInt32)m_classLayouts.size() - 1;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   02/13
+---------------+---------------+---------------+---------------+---------------+------*/
bool            SchemaLayout::IsEmpty() const
    {
    return m_classLayouts.empty();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    01/10
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   SchemaLayout::FindAvailableClassIndex(ClassIndex& classIndex) const
    {
    ClassLayoutPtr nullVal = NULL;
    ClassLayoutVector::const_iterator iter = std::find (m_classLayouts.begin(), m_classLayouts.end(), nullVal);

    size_t firstNullIndex = iter - m_classLayouts.begin();

    if (USHRT_MAX > firstNullIndex)
        { 
        classIndex = (UInt16) firstNullIndex;
        return SUCCESS;
        }

    // The max size for classIndex is 0xffff, but if we reach that limit,
    // most likely something else has gone wrong.
    BeAssert(false);
    return ERROR;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Adam.Klatzkin                   01/2010
+---------------+---------------+---------------+---------------+---------------+------*/    
UInt32          ECDBuffer::GetPropertyValueSize (PropertyLayoutCR propertyLayout) const
    {
    if (propertyLayout.IsFixedSized())
        {
        return propertyLayout.GetSizeInFixedSection();
        }
    else
        {    
        UInt32 offset = propertyLayout.GetOffset();
        SecondaryOffset const * pSecondaryOffset = (SecondaryOffset const *)(GetPropertyData() + offset);
        
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
UInt32          ECDBuffer::GetPropertyValueSize (PropertyLayoutCR propertyLayout, UInt32 index) const
    {
    if (IsArrayOfFixedSizeElements (propertyLayout))
        {
        return ECValue::GetFixedPrimitiveValueSize (propertyLayout.GetTypeDescriptor().GetPrimitiveType());
        }
    else if (PRIMITIVETYPE_Integer == GetStructArrayPrimitiveType () && propertyLayout.GetTypeDescriptor().IsStructArray())
        {
        return ECValue::GetFixedPrimitiveValueSize (PRIMITIVETYPE_Integer);
        }
    else
        {                            
        UInt32 arrayOffset = GetOffsetOfPropertyValue (propertyLayout);   
        byte const *             data = GetPropertyData();
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
* @bsimethod                                                    Paul.Connelly   11/12
+---------------+---------------+---------------+---------------+---------------+------*/
byte const* ECDBuffer::GetPropertyData() const
    {
    byte const* data = _GetData();
    return NULL != data ? data + GetOffsetToPropertyData() : NULL;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    AdamKlatzkin    01/10
+---------------+---------------+---------------+---------------+---------------+------*/
ArrayCount      ECDBuffer::GetReservedArrayCount (PropertyLayoutCR propertyLayout) const
    {
    if (propertyLayout.GetModifierFlags() & PROPERTYLAYOUTMODIFIERFLAGS_IsArrayFixedCount)
        return propertyLayout.GetModifierData();
    else
        return GetAllocatedArrayCount (propertyLayout);
    }    
        
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    AdamKlatzkin    01/10
+---------------+---------------+---------------+---------------+---------------+------*/
ArrayCount      ECDBuffer::GetAllocatedArrayCount (PropertyLayoutCR propertyLayout) const
    {
    if (propertyLayout.IsFixedSized())
        {
        return propertyLayout.GetModifierData();
        }
    else
        {
        SecondaryOffset* pSecondaryOffset = (SecondaryOffset*)(GetPropertyData() + propertyLayout.GetOffset());
        SecondaryOffset* pNextOffset = pSecondaryOffset + 1;        
        
        SecondaryOffset arrayOffset = *pSecondaryOffset;
        if ((arrayOffset == 0) || (*pNextOffset == 0) || (arrayOffset == *pNextOffset))
            return 0;

        byte const * pCount = GetPropertyData() + arrayOffset;
        return *((ArrayCount*)pCount);
        }
    }    

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    CaseyMullen     10/09
+---------------+---------------+---------------+---------------+---------------+------*/
byte const *    ECDBuffer::GetAddressOfPropertyValue (PropertyLayoutCR propertyLayout) const
    {
    return GetPropertyData() + GetOffsetOfPropertyValue (propertyLayout);
    }
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    CaseyMullen     10/09
+---------------+---------------+---------------+---------------+---------------+------*/
byte const *    ECDBuffer::GetAddressOfPropertyValue (PropertyLayoutCR propertyLayout, UInt32 index) const
    {
    UInt32 arrayOffset = GetOffsetOfPropertyValue (propertyLayout);   
    return GetPropertyData() + GetOffsetOfArrayIndexValue (arrayOffset, propertyLayout, index);
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
bool            ECDBuffer::IsPropertyValueNull (PropertyLayoutCR propertyLayout, bool useIndex, UInt32 index) const
    {
    if (propertyLayout.GetTypeDescriptor().IsStruct())
        return true;    // embedded structs always null
    else if (!useIndex && propertyLayout.GetTypeDescriptor().IsArray())
        return false;   // arrays are never null

    UInt32 nullflagsOffset;
    UInt32 nullflagsBitmask;
    byte const * data = GetPropertyData();
    if (useIndex)
        {
        // see if we have an UninitializedFixedCountArray
        if ((propertyLayout.GetModifierFlags() & PROPERTYLAYOUTMODIFIERFLAGS_IsArrayFixedCount) && (GetAllocatedArrayCount (propertyLayout) == 0))    
            return true;

        PrepareToAccessNullFlags (nullflagsOffset, nullflagsBitmask, data, propertyLayout, index); 
        }
    else
        PrepareToAccessNullFlags (nullflagsOffset, nullflagsBitmask, data, propertyLayout);

    NullflagsBitmask const * nullflags = (NullflagsBitmask const *)(data + nullflagsOffset);
    return (0 != (*nullflags & nullflagsBitmask));    
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    CaseyMullen     10/09
+---------------+---------------+---------------+---------------+---------------+------*/
void            ECDBuffer::SetPropertyValueNull (PropertyLayoutCR propertyLayout, bool useIndex, UInt32 index, bool isNull)
    {  
    UInt32 nullflagsOffset;
    UInt32 nullflagsBitmask;
    byte const * data = GetPropertyData();
    if (useIndex)
        PrepareToAccessNullFlags (nullflagsOffset, nullflagsBitmask, data, propertyLayout, index);   
    else
        PrepareToAccessNullFlags (nullflagsOffset, nullflagsBitmask, data, propertyLayout);   
    
    NullflagsBitmask* nullflagsP = (NullflagsBitmask*)(data + nullflagsOffset);
    NullflagsBitmask nullflags = *nullflagsP;
    if (isNull && 0 == (nullflags & nullflagsBitmask))
        nullflags |= nullflagsBitmask;
    else if (!isNull && nullflagsBitmask == (nullflags & nullflagsBitmask))
        nullflags ^= nullflagsBitmask;

    ModifyData (nullflagsP, nullflags);
    }    

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    CaseyMullen     10/09
+---------------+---------------+---------------+---------------+---------------+------*/
UInt32          ECDBuffer::GetOffsetOfPropertyValue (PropertyLayoutCR propertyLayout, bool useIndex, UInt32 index) const
    {
    UInt32 offset = propertyLayout.GetOffset();
    
    if (!propertyLayout.IsFixedSized())
        {
        SecondaryOffset const * pSecondaryOffset = (SecondaryOffset const *)(GetPropertyData() + offset);
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
UInt32          ECDBuffer::GetOffsetOfArrayIndex (UInt32 arrayOffset, PropertyLayoutCR propertyLayout, UInt32 index) const
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
    else if (PRIMITIVETYPE_Integer == GetStructArrayPrimitiveType () && propertyLayout.GetTypeDescriptor().IsStructArray())
        primaryOffset += (index * ECValue::GetFixedPrimitiveValueSize (ECN::PRIMITIVETYPE_Integer));
    else
        primaryOffset += index * sizeof (SecondaryOffset);

    return primaryOffset;
    }  

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Adam.Klatzkin                   01/2010
+---------------+---------------+---------------+---------------+---------------+------*/
UInt32          ECDBuffer::GetOffsetOfArrayIndexValue (UInt32 arrayOffset, PropertyLayoutCR propertyLayout, UInt32 index) const
    {    
    UInt32 arrayIndexOffset = GetOffsetOfArrayIndex (arrayOffset, propertyLayout, index);

    if (IsArrayOfFixedSizeElements (propertyLayout) || (PRIMITIVETYPE_Integer == GetStructArrayPrimitiveType () && propertyLayout.GetTypeDescriptor().IsStructArray()))
        return arrayIndexOffset;

    byte const * data = GetPropertyData();
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
ECObjectsStatus       ECDBuffer::EnsureSpaceIsAvailable (UInt32& offset, PropertyLayoutCR propertyLayout, UInt32 bytesNeeded)
    {
    byte const *             data = GetPropertyData();
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
        ModifyData (pNextSecondaryOffset, *pSecondaryOffset); // As long as we have zeros, it as if the last non-zero one were the value to use whereever there is a zero... 
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
        
    return GrowPropertyValue (propertyLayout, additionalBytesNeeded);
    }        
        
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Bill.Steinbock                  07/2011
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus ECDBuffer::RemoveArrayElementsFromMemory (PropertyLayoutCR propertyLayout, UInt32 removeIndex, UInt32 removeCount)
    {
    ClassLayoutCR classLayout = GetClassLayout();
    ECObjectsStatus status = ECOBJECTS_STATUS_Error;

    bool isFixedCount = (propertyLayout.GetModifierFlags() & PROPERTYLAYOUTMODIFIERFLAGS_IsArrayFixedCount);
    PRECONDITION (!isFixedCount && propertyLayout.GetTypeDescriptor().IsArray() && "A variable size array property is required to remove an array entry", ECOBJECTS_STATUS_PreconditionViolated);
    
    PRECONDITION (removeCount > 0, ECOBJECTS_STATUS_IndexOutOfRange)        
   
    UInt32       bytesAllocated = _GetBytesAllocated();    
    byte *       currentData    = (byte*)_GetData();

    // since we can not use memmove on XAttribute memory, copy the memory move it around and then use _ModifyData
    // copy the entire instance into allocated memory
    ScopedArray<byte> scoped((size_t)bytesAllocated, currentData);
    byte*   _data = scoped.GetData();

    byte*   propertyData = _data + GetOffsetToPropertyData();
    UInt32  propertyBytesAllocated = bytesAllocated - GetOffsetToPropertyData();

    bool    hasFixedSizedElements = IsArrayOfFixedSizeElements (propertyLayout) || ((PRIMITIVETYPE_Integer == GetStructArrayPrimitiveType () && propertyLayout.GetTypeDescriptor().IsStructArray()));
    UInt32  arrayOffset           = GetOffsetOfPropertyValue (propertyLayout);   
    byte   *arrayAddress          = propertyData + arrayOffset;

    // modify from bottom up
    SecondaryOffset  beginIndexValueOffset = GetOffsetOfArrayIndexValue (arrayOffset, propertyLayout, removeIndex);
    byte *           destination = propertyData + beginIndexValueOffset;
    byte *           source;
    UInt32           bytesToMove;
    UInt32           totalBytesAdjusted=0;
    ArrayCount       preArrayCount = GetAllocatedArrayCount (propertyLayout);
    ArrayCount       postArrayCount = preArrayCount - removeCount;
    SecondaryOffset* pThisProperty = (SecondaryOffset*)(propertyData + propertyLayout.GetOffset());
    SecondaryOffset* pNextProperty = pThisProperty + 1;

    if ((removeIndex + removeCount) >= preArrayCount) // removing entries an end of array
        {
        source = propertyData + *pNextProperty;
        bytesToMove = propertyBytesAllocated - *pNextProperty;
        }
    else   // removing entries at beginning or in middle of array
        {
        SecondaryOffset endIndexValueOffset = GetOffsetOfArrayIndexValue (arrayOffset, propertyLayout, removeIndex+removeCount);
        source = propertyData + endIndexValueOffset;

        bytesToMove = propertyBytesAllocated - endIndexValueOffset;
        }

    if (bytesToMove < 0)
        return ECOBJECTS_STATUS_Error;

    // remove the array values - bytesToMove may be equal to zero if allocated size has no padding and we are deleting the last member of the array.
    if (bytesToMove > 0)
        memmove (destination, source, bytesToMove);

    totalBytesAdjusted += (UInt32)(source - destination);

    UInt32           preNullFlagBitmasksCount = CalculateNumberNullFlagsBitmasks (preArrayCount);   
    UInt32           postNullFlagBitmasksCount = CalculateNumberNullFlagsBitmasks (postArrayCount);
    UInt32           nullFlagsDelta = 0;
    SecondaryOffset* firstArrayEntryOffset = (SecondaryOffset *)(propertyData + GetOffsetOfArrayIndex (arrayOffset, propertyLayout, 0));

    if (!hasFixedSizedElements)
        {
        // remove secondary offsets to array entry values
        SecondaryOffset  beginIndexValueOffset = GetOffsetOfArrayIndex (arrayOffset, propertyLayout, removeIndex);
        byte *           destination = propertyData + beginIndexValueOffset;  
        UInt32           offsetDelta = removeCount * sizeof(SecondaryOffset);

        source      = destination + offsetDelta;    
        bytesToMove = propertyBytesAllocated - (beginIndexValueOffset+offsetDelta);

        if (bytesToMove > 0)
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

        bytesToMove = propertyBytesAllocated - (arrayOffset + sizeof(ArrayCount) + (postNullFlagBitmasksCount*sizeof(NullflagsBitmask)));

        memmove (destination, source, bytesToMove);
        totalBytesAdjusted += (UInt32)(source - destination);
        }

    // Update the array count
    memcpy (arrayAddress, &postArrayCount, sizeof(ArrayCount));
   
    SecondaryOffset * pLast = (SecondaryOffset*)(propertyData + classLayout.GetSizeOfFixedSection() - sizeof(SecondaryOffset));

    // adjust the offsets in the fixed section beyond the one we just modified
    while (pNextProperty <= pLast)
        {
        if (0 != *pNextProperty)
            *pNextProperty -= totalBytesAdjusted;

        pNextProperty++;
        }

    // replace all property data for the instance
    status = _ModifyData (0, _data, bytesAllocated);

    if (ECOBJECTS_STATUS_Success == status)
        _HandleArrayResize (&propertyLayout, removeIndex, -1 * removeCount);

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
ECObjectsStatus ECDBuffer::RemoveArrayElements (PropertyLayoutCR propertyLayout, UInt32 removeIndex, UInt32 removeCount)
    {
    ECTypeDescriptor typeDescriptor = propertyLayout.GetTypeDescriptor();
    PRECONDITION (typeDescriptor.IsArray() && 
        "Can not set the value to memory at an array index using the specified property layout because it is not an array datatype", 
        ECOBJECTS_STATUS_PreconditionViolated);  
                        
    if (removeIndex >= GetReservedArrayCount (propertyLayout))
        return ECOBJECTS_STATUS_IndexOutOfRange;  


#ifdef DEBUGGING_ARRAYENTRY_REMOVAL 
    ECObjectsStatus status = ECOBJECTS_STATUS_Error;
    WString preDeleteLayout = InstanceDataToString (L"", GetClassLayout());
    if (preDeleteLayout.empty())
        return status;
#endif

    if (typeDescriptor.IsPrimitiveArray())
        return RemoveArrayElementsFromMemory (propertyLayout, removeIndex, removeCount);
    else if (typeDescriptor.IsStructArray())    
        {
        ECObjectsStatus result = _RemoveStructArrayElementsFromMemory (propertyLayout, removeIndex, removeCount);       
        if (ECOBJECTS_STATUS_Success == result)
            _HandleArrayResize (&propertyLayout, removeIndex, -1 * removeCount);
        return result;
        }

    POSTCONDITION (false && "Can not set the value to memory using the specified property layout because it is an unsupported datatype", ECOBJECTS_STATUS_DataTypeNotSupported);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   11/12
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus ECDBuffer::ClearArrayElementsFromMemory (UInt32 propertyIndex)
    {
    PropertyLayoutCP pPropertyLayout = NULL;
    ClassLayoutCR classLayout = GetClassLayout();
    
    ECObjectsStatus status = classLayout.GetPropertyLayoutByIndex (pPropertyLayout, propertyIndex);
    if (ECOBJECTS_STATUS_Success == status)
        {
        UInt32 arrayCount = GetReservedArrayCount (*pPropertyLayout);
        if (0 < arrayCount)
            status = RemoveArrayElements (*pPropertyLayout, 0, arrayCount);
        }

    return status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Bill.Steinbock                  07/2011
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus ECDBuffer::RemoveArrayElementsAt (UInt32 propIdx, UInt32 removeIndex, UInt32 removeCount)
    {        
    ClassLayoutCR classLayout = GetClassLayout();
    PropertyLayoutCP pPropertyLayout = NULL;
    ECObjectsStatus status = classLayout.GetPropertyLayoutByIndex (pPropertyLayout, propIdx);
    if (SUCCESS != status || NULL == pPropertyLayout)
        return ECOBJECTS_STATUS_PropertyNotFound;

    return RemoveArrayElements (*pPropertyLayout, removeIndex, removeCount);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Adam.Klatzkin                   01/2010
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus ECDBuffer::InsertNullArrayElementsAt (UInt32 propIdx, UInt32 insertIndex, UInt32 insertCount)
    {        
    ClassLayoutCR classLayout = GetClassLayout();
    PropertyLayoutCP pPropertyLayout = NULL;
    ECObjectsStatus status = classLayout.GetPropertyLayoutByIndex (pPropertyLayout, propIdx);
    if (SUCCESS != status || NULL == pPropertyLayout)
        return ECOBJECTS_STATUS_PropertyNotFound;
            
    PropertyLayoutCR propertyLayout = *pPropertyLayout;
    bool isFixedCount = (propertyLayout.GetModifierFlags() & PROPERTYLAYOUTMODIFIERFLAGS_IsArrayFixedCount);
    PRECONDITION (!isFixedCount && propertyLayout.GetTypeDescriptor().IsArray() && "A variable size array property is required to grow an array", ECOBJECTS_STATUS_UnableToResizeFixedSizedArray);
    
    PRECONDITION (insertCount > 0, ECOBJECTS_STATUS_IndexOutOfRange)        
    
    status = ArrayResizer::CreateNullArrayElementsAt (classLayout, propertyLayout, *this, insertIndex, insertCount);
    if (ECOBJECTS_STATUS_Success == status)
        _HandleArrayResize (pPropertyLayout, insertIndex, insertCount);

    return status;
    }
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Adam.Klatzkin                   01/2010
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus       ECDBuffer::AddNullArrayElementsAt (UInt32 propIdx, UInt32 count)
    {        
    if (0 == count)
        return ECOBJECTS_STATUS_Success;    // it's a no-op. Used to be treated as error, but it's bound to happen.

    ClassLayoutCR classLayout = GetClassLayout();
    PropertyLayoutCP pPropertyLayout = NULL;
    ECObjectsStatus status = classLayout.GetPropertyLayoutByIndex (pPropertyLayout, propIdx);
    if (ECOBJECTS_STATUS_Success != status || NULL == pPropertyLayout)
        return ECOBJECTS_STATUS_PropertyNotFound; 
            
    PropertyLayoutCR propertyLayout = *pPropertyLayout;
    bool isFixedCount = (propertyLayout.GetModifierFlags() & PROPERTYLAYOUTMODIFIERFLAGS_IsArrayFixedCount);
    PRECONDITION (!isFixedCount && propertyLayout.GetTypeDescriptor().IsArray() && "A variable size array property is required to grow an array", ECOBJECTS_STATUS_UnableToResizeFixedSizedArray);
    
    ArrayCount index = GetAllocatedArrayCount (propertyLayout);
    status = ArrayResizer::CreateNullArrayElementsAt (classLayout, propertyLayout, *this, index, count);
    if (ECOBJECTS_STATUS_Success == status)
        _HandleArrayResize (pPropertyLayout, index, count);

    return status;
    }         

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Adam.Klatzkin                   01/2010
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus       ECDBuffer::EnsureSpaceIsAvailableForArrayIndexValue (PropertyLayoutCR propertyLayout, UInt32 arrayIndex, UInt32 bytesNeeded)
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
    
    UInt32 endOfValueDataPreGrow = *((SecondaryOffset*)(GetPropertyData() + propertyLayout.GetOffset()) + 1);
    ECObjectsStatus status = GrowPropertyValue (propertyLayout, additionalBytesNeeded);

    if (ECOBJECTS_STATUS_Success != status)
        return status;

    if (arrayIndex < arrayCount - 1)
        return ShiftArrayIndexValueData(propertyLayout, arrayIndex, arrayCount, endOfValueDataPreGrow, additionalBytesNeeded);
    else
        return status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   11/12
+---------------+---------------+---------------+---------------+---------------+------*/
UInt32              ECDBuffer::CalculateBytesUsed () const
    {
    UInt32 offset = GetOffsetToPropertyData();
    return offset + GetClassLayout().CalculateBytesUsed (_GetData() + offset);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Adam.Klatzkin                   01/2010
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus       ECDBuffer::GrowPropertyValue (PropertyLayoutCR propertyLayout, UInt32 additionalBytesNeeded)
    {
    ClassLayoutCR classLayout = GetClassLayout();
    byte const * data = GetPropertyData();
    UInt32 bytesUsed = classLayout.CalculateBytesUsed(data) + GetOffsetToPropertyData();
        
    UInt32 bytesAllocated = _GetBytesAllocated();    
    UInt32 additionalBytesAvailable = bytesAllocated - bytesUsed;
    
    ECObjectsStatus status = ECOBJECTS_STATUS_Success;
    if (additionalBytesNeeded > additionalBytesAvailable)
        {
        UInt32 growBy = additionalBytesNeeded - additionalBytesAvailable;
        status = _GrowAllocation (growBy);
        UInt32 newBytesAllocated = _GetBytesAllocated();
        DEBUG_EXPECT (newBytesAllocated >= bytesAllocated + growBy);
        bytesAllocated = newBytesAllocated;
        }
    
    if (ECOBJECTS_STATUS_Success != status)
        return status;
        
    byte * writeableData = (byte *)GetPropertyData();
    DEBUG_EXPECT (bytesUsed == classLayout.CalculateBytesUsed(writeableData) + GetOffsetToPropertyData());
    
    status = ShiftValueData(writeableData, bytesAllocated, propertyLayout, additionalBytesNeeded);

    DEBUG_EXPECT (0 == bytesUsed || (bytesUsed + additionalBytesNeeded == GetOffsetToPropertyData() + classLayout.CalculateBytesUsed(writeableData)));
    
    return status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   08/12
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus     ECDBuffer::Compress()
    {
    return _IsMemoryInitialized() ? _ShrinkAllocation() : ECOBJECTS_STATUS_Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    CaseyMullen     01/10
+---------------+---------------+---------------+---------------+---------------+------*/
ECDBuffer::ECDBuffer (bool allowWritingDirectlyToInstanceMemory) :
    m_allowWritingDirectlyToInstanceMemory (allowWritingDirectlyToInstanceMemory)
    {
    }
        
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    CaseyMullen     10/09
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus       ECDBuffer::ShiftValueData(byte * propertyData, UInt32 bytesAllocated, PropertyLayoutCR propertyLayout, Int32 shiftBy)
    {
    ClassLayoutCR classLayout = GetClassLayout();

    DEBUG_EXPECT (0 != shiftBy && "It is a pointless waste of time to shift nothing");
    DEBUG_EXPECT (!propertyLayout.IsFixedSized() && "The propertyLayout should be that of the variable-sized property whose size is increasing");
    DEBUG_EXPECT (classLayout.GetSizeOfFixedSection() > 0 && "The ClassLayout has not been initialized");
    
    SecondaryOffset * pLast = (SecondaryOffset*)(propertyData + classLayout.GetSizeOfFixedSection() - sizeof(SecondaryOffset));
    SecondaryOffset * pAdjusting = (SecondaryOffset*)(propertyData + propertyLayout.GetOffset());
    SecondaryOffset * pCurrent = pAdjusting + 1; // start at the one AFTER the property whose value's size is adjusting
    DEBUG_EXPECT (pCurrent <= pLast);
    DEBUG_EXPECT ((*pCurrent - *pAdjusting + shiftBy) >= 0 && "shiftBy cannot be such that it would cause the adjusting property to shrink to a negative size");
    
    UInt32 bytesToMove = *pLast - *pCurrent;
    if (bytesToMove > 0)
        {
        byte * source = propertyData + *pCurrent;
        byte * destination = source + shiftBy;
        
        DEBUG_EXPECT (destination + bytesToMove <= propertyData + bytesAllocated && "Attempted to move memory beyond the end of the allocated XAttribute.");
        if (destination + bytesToMove > propertyData + bytesAllocated)
            return ECOBJECTS_STATUS_IndexOutOfRange;
            
        MoveData (destination, source, bytesToMove);
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

    ScopedArray<SecondaryOffset> shiftedSecondaryOffsetArray (nSecondaryOffsetsToShift);
    SecondaryOffset* shiftedSecondaryOffsets = shiftedSecondaryOffsetArray.GetData();

    memset (shiftedSecondaryOffsets, 0, sizeOfSecondaryOffsetsToShift);
    for (UInt32 i = 0; i < nSecondaryOffsetsToShift - 1 && 0 != pCurrent[i]; i++) // stop when we hit a zero
        shiftedSecondaryOffsets[i] = pCurrent[i] + shiftBy;

    shiftedSecondaryOffsets[nSecondaryOffsetsToShift - 1] = pCurrent[nSecondaryOffsetsToShift - 1] + shiftBy; // always do the last one
    
    // UInt32 offsetOfCurrent = (UInt32)((byte*)pCurrent - propertyData);
    // return _ModifyData (offsetOfCurrent, shiftedSecondaryOffsets, sizeOfSecondaryOffsetsToShift);
    return ModifyData ((byte*)pCurrent, shiftedSecondaryOffsets, sizeOfSecondaryOffsetsToShift);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Adam.Klatzkin                   01/2010
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus       ECDBuffer::ShiftArrayIndexValueData(PropertyLayoutCR propertyLayout, UInt32 arrayIndex, UInt32 arrayCount, UInt32 endOfValueDataOffset, Int32 shiftBy)
    {
    DEBUG_EXPECT (0 != shiftBy && "It is a pointless waste of time to shift nothing");
    DEBUG_EXPECT (arrayIndex < arrayCount - 1 && "It is a pointless waste of time to shift nothing");

    byte * data = (byte*)GetPropertyData();
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
            
        MoveData (destination, source, bytesToMove);
        }

    // Shift all secondaryOffsets for indices following the one that just got larger
    UInt32 nSecondaryOffsetsToShift = arrayCount - (arrayIndex + 1);
    UInt32 sizeOfSecondaryOffsetsToShift = nSecondaryOffsetsToShift * sizeof (SecondaryOffset);    

    ScopedArray<SecondaryOffset> shiftedSecondaryOffsets (nSecondaryOffsetsToShift);
    SecondaryOffset * pCurrent = (SecondaryOffset*)(data + GetOffsetOfArrayIndex (arrayOffset, propertyLayout, arrayIndex + 1));
    for (UInt32 i = 0; i < nSecondaryOffsetsToShift; i++)
        shiftedSecondaryOffsets.GetData()[i] = pCurrent[i] + shiftBy;
        
    return ModifyData ((byte const*)pCurrent, shiftedSecondaryOffsets.GetData(), sizeOfSecondaryOffsetsToShift);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   05/13
+---------------+---------------+---------------+---------------+---------------+------*/
void            ECDBuffer::InitializeMemory(ClassLayoutCR classLayout, byte* data, UInt32 bytesAllocated, bool forceUtf8)
    {
    ECDHeader hdr (forceUtf8 ? true : StringEncoding_Utf8 == GetDefaultStringEncoding());
    memcpy (data, &hdr, hdr.GetSize());

    classLayout.InitializeMemoryForInstance (data + hdr.GetSize(), bytesAllocated - hdr.GetSize());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Bill.Steinbock                  01/2012
+---------------+---------------+---------------+---------------+---------------+------*/
ECN::PrimitiveType         ECDBuffer::GetStructArrayPrimitiveType () const
    {
    return _GetStructArrayPrimitiveType();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void ECDBuffer::ClearValues()                                               { _ClearValues(); }
ECObjectsStatus ECDBuffer::CopyFromBuffer (ECDBufferCR source)    { return _CopyFromBuffer (source); }
ClassLayoutCR ECDBuffer::GetClassLayout() const                             { return _GetClassLayout(); }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    03/11
+---------------+---------------+---------------+---------------+---------------+------*/ 
static ECObjectsStatus     duplicateProperties (IECInstanceR target, ECValuesCollectionCR source)
    {
    ECObjectsStatus status = ECOBJECTS_STATUS_Success;
    for (ECValuesCollection::const_iterator it=source.begin(); it != source.end(); ++it)
        {
        ECPropertyValue const& prop = *it;
        if (prop.HasChildValues())
            {
            // Stupid PropertyValueMatchesNoChange return value is stupid.
            if (SUCCESS != (status = duplicateProperties (target, *prop.GetChildValues())))
                if (ECOBJECTS_STATUS_PropertyValueMatchesNoChange != status)
                    return status;

            continue;
            }
        else 
            {
            ECPropertyCP ecProp = prop.GetValueAccessor().GetECProperty();
            if (NULL != ecProp && ecProp->GetIsPrimitive() && SUCCESS != (status = target.SetInternalValueUsingAccessor (prop.GetValueAccessor(), prop.GetValue())))
                if (ECOBJECTS_STATUS_PropertyValueMatchesNoChange != status)
                    return status;
            }
        }

    return ECOBJECTS_STATUS_PropertyValueMatchesNoChange == status ? ECOBJECTS_STATUS_Success : status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Abeesh.Basheer                  04/2013
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus IECInstance::CopyValues(ECN::IECInstanceCR source)
    {
    if (!GetClass().GetName().Equals (source.GetClass().GetName()))
        return ECOBJECTS_STATUS_Error;

    // If both are ECD-based, try copying the buffers first - it's cheaper.
    ECDBuffer* thisBuffer = this->GetECDBufferP();
    ECDBuffer const* srcBuffer;
    if (NULL != thisBuffer && NULL != (srcBuffer = source.GetECDBuffer()) && ECOBJECTS_STATUS_Success == thisBuffer->CopyFromBuffer (*srcBuffer))
        return ECOBJECTS_STATUS_Success;
    
    // Not ECD-based, or incompatible layouts. Copy property-by-property
    ECValuesCollectionPtr srcValues = ECValuesCollection::Create (source);
    return duplicateProperties (*this, *srcValues);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   05/13
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus ECDBuffer::GetStructArrayValueFromMemory (ECValueR v, PropertyLayoutCR propertyLayout, UInt32 index, int * structValueIdP) const
    {
    ECObjectsStatus status = _GetStructArrayValueFromMemory (v, propertyLayout, index);
    if (NULL != structValueIdP && ECOBJECTS_STATUS_Success == status)
        {
        ECValue idVal;
        status = GetPrimitiveValueFromMemory (idVal, propertyLayout, true, index);
        if (ECOBJECTS_STATUS_Success == status)
            {
            if (idVal.IsInteger() && !idVal.IsNull())
                *structValueIdP = idVal.GetInteger();
            else
                status = ECOBJECTS_STATUS_Error;
            }
        }

    return status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   11/12
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus ECDBuffer::CopyPropertiesFromBuffer (ECDBufferCR srcBuffer)
    {
    ClassLayoutCR classLayout = GetClassLayout();
    if (!classLayout.Equals (srcBuffer.GetClassLayout()))
        return ECOBJECTS_STATUS_OperationNotSupported;

    // Ensure source instance's data is accessible. This is ugly but necessary e.g. if source instance is an ECXDInstance - it may need to acquire its XAttribute
    ScopedDataAccessor scopedDataAccessor (srcBuffer);
    if (!scopedDataAccessor.IsValid())
        return ECOBJECTS_STATUS_Error;

    // Make sure we have enough room for the data
    UInt32 bytesAvailable = _GetBytesAllocated();
    UInt32 bytesNeeded = srcBuffer.CalculateBytesUsed();
    if (bytesAvailable < bytesNeeded && ECOBJECTS_STATUS_Success != _GrowAllocation (bytesNeeded - bytesAvailable))
            return ECOBJECTS_STATUS_UnableToAllocateMemory;

    // copy ecd buffer
    if (ECOBJECTS_STATUS_Success != ModifyData (_GetData(), srcBuffer._GetData(), bytesNeeded))
        return ECOBJECTS_STATUS_Error;

    // copy struct instances, updating their identifiers if necessary
    ECObjectsStatus status = ECOBJECTS_STATUS_Success;
    PropertyLayoutCP propLayout;
    UInt32 nProperties = classLayout.GetPropertyCount();
    for (UInt32 propIdx = 0; propIdx < nProperties; propIdx++)
        {
        if (ECOBJECTS_STATUS_Success != classLayout.GetPropertyLayoutByIndex (propLayout, propIdx) || !propLayout->GetTypeDescriptor().IsStructArray())
            continue;

        ArrayCount nEntries = srcBuffer.GetReservedArrayCount (*propLayout);
        for (ArrayCount arrayIdx = 0; arrayIdx < nEntries; arrayIdx++)
            {
            ECValue srcStructVal;
            if (ECOBJECTS_STATUS_Success == srcBuffer._GetStructArrayValueFromMemory (srcStructVal, *propLayout, arrayIdx) && !srcStructVal.IsNull())
                {
                // The ECDBuffer we copied from source to 'this' contains struct ID values relevant only to source buffer
                // So clear out the struct ID entry in 'this' first.
                // WIP_FUSION: Assumption that struct IDs are always integers and 0 == null struct
                //  There seems to be no reason not to enforce that.
                ECValue nullStructIdValue (0);
                status = SetPrimitiveValueToMemory (nullStructIdValue, *propLayout, true, arrayIdx);
                if (ECOBJECTS_STATUS_Success != status && ECOBJECTS_STATUS_PropertyValueMatchesNoChange == status)  // useless redundant return value...
                    break;

                if (ECOBJECTS_STATUS_Success != (status = _SetStructArrayValueToMemory (srcStructVal, *propLayout, arrayIdx)))
                    {
                    // This useless return value is a constant pain in the...
                    if (ECOBJECTS_STATUS_PropertyValueMatchesNoChange == status)
                        status = ECOBJECTS_STATUS_Success;
                    else
                        break;
                    }
                }
            }

        if (ECOBJECTS_STATUS_Success != status)
            break;
        }

    return status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    CaseyMullen     12/09
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus       ECDBuffer::GetPrimitiveValueFromMemory (ECValueR v, PropertyLayoutCR propertyLayout, bool useIndex, UInt32 index) const
    {
    ECTypeDescriptor typeDescriptor = propertyLayout.GetTypeDescriptor();

    DEBUG_EXPECT (typeDescriptor.IsArray() == useIndex);   

    ECObjectsStatus status = ECOBJECTS_STATUS_Success;
    bool isInUninitializedFixedCountArray = ((useIndex) && (propertyLayout.GetModifierFlags() & PROPERTYLAYOUTMODIFIERFLAGS_IsArrayFixedCount) && (GetAllocatedArrayCount (propertyLayout) == 0));    
    if (isInUninitializedFixedCountArray || IsPropertyValueNull(propertyLayout, useIndex, index))
        {
        if (typeDescriptor.IsStructArray())
            v.SetPrimitiveType (GetStructArrayPrimitiveType ());
        else
            v.SetPrimitiveType (typeDescriptor.GetPrimitiveType());

        v.SetToNull();
        if (!propertyLayout.HoldsCalculatedProperty())
            return ECOBJECTS_STATUS_Success;
        }    
    else
        {
        byte const * pValue;
        if (useIndex)
            pValue = GetAddressOfPropertyValue (propertyLayout, index);
        else
            pValue = GetAddressOfPropertyValue (propertyLayout);

        PrimitiveType primitiveType;
        if (typeDescriptor.IsPrimitive() || (typeDescriptor.IsPrimitiveArray()))
            primitiveType = typeDescriptor.GetPrimitiveType();
        else
            primitiveType = GetStructArrayPrimitiveType ();
        
        switch (primitiveType)       
            {
            case PRIMITIVETYPE_Integer:
                {
                Int32 value;
                memcpy (&value, pValue, sizeof(value));
                v.SetInteger (value);
                break;
                }
            case PRIMITIVETYPE_Long:
                {
                Int64 value;
                memcpy (&value, pValue, sizeof(value));
                v.SetLong (value);
                break;
                }            
            case PRIMITIVETYPE_Double:
                {
                double value;
                memcpy (&value, pValue, sizeof(value));
                v.SetDouble (value);
                break;
                }
            case PRIMITIVETYPE_Binary:
            case PRIMITIVETYPE_IGeometry:
                {
                UInt32 size;
                if (useIndex)
                    size = GetPropertyValueSize (propertyLayout, index);
                else
                    size = GetPropertyValueSize (propertyLayout);

                UInt32 const* actualSize   = (UInt32 const*)pValue;
                byte const*   actualBuffer = pValue+sizeof(UInt32);

                v.SetBinary (actualBuffer, *actualSize);
                break;
                }  
            case PRIMITIVETYPE_Boolean:
                {
                bool value;
                memcpy (&value, pValue, sizeof(value));
                v.SetBoolean (value);
                break;
                } 
            case PRIMITIVETYPE_Point2D:
                {
                DPoint2d value;
                memcpy (&value, pValue, sizeof(value));
                v.SetPoint2D (value);
                break;
                }       
            case PRIMITIVETYPE_Point3D:
                {
                DPoint3d value;
                memcpy (&value, pValue, sizeof(value));
                v.SetPoint3D (value);
                break;
                }       
            case PRIMITIVETYPE_DateTime:
                {
                Int64 value;
                memcpy (&value, pValue, sizeof(value));
                v.SetDateTimeTicks (value);
                break;
                } 
            case PRIMITIVETYPE_String:
                {
                // Note: we could avoid a string copy by returning a pointer directly to the string in this instance's memory buffer,
                // but the pointer will become invalid as soon as the instance does.
                // Since there are situations in which the returned ECValue outlasts the instance (e.g. evaluating ECExpressions), and the caller
                // cannot know his ECValue is about to evaporate, we have to make the copy.
                // The exception is if the caller passed us an ECValue with a flag explicitly requesting we avoid making the copy.
                bool makeACopy = !v.AllowsPointersIntoInstanceMemory();
                if (StringEncoding_Utf16 == GetStringEncoding())
                    v.SetUtf16CP ((Utf16CP)pValue, makeACopy);
                else
                    v.SetUtf8CP ((Utf8CP)pValue, makeACopy);
                break;            
                }
            default:
                BeAssert (false && "datatype not implemented");
                return ECOBJECTS_STATUS_DataTypeNotSupported;
            }
        }

    if (ECOBJECTS_STATUS_Success == status && propertyLayout.HoldsCalculatedProperty())
        status = EvaluateCalculatedProperty (propertyLayout, v, useIndex, index);

    return status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Adam.Klatzkin                   01/2010
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus       ECDBuffer::GetValueFromMemory (ECValueR v, PropertyLayoutCR propertyLayout) const
    {
    ECTypeDescriptor typeDescriptor = propertyLayout.GetTypeDescriptor();
    if (typeDescriptor.IsPrimitive())
        {
        return GetPrimitiveValueFromMemory (v, propertyLayout, false, 0);
        }
    if (typeDescriptor.IsStruct())
        {
        // Note that it is not possible to retrieve an embedded struct as an atomic ECValue - you must access its property values individually.
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
ECObjectsStatus       ECDBuffer::GetValueFromMemory (ECValueR v, PropertyLayoutCR propertyLayout, UInt32 index) const
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
ECObjectsStatus       ECDBuffer::GetValueFromMemory (ECValueR v, WCharCP propertyAccessString, bool useIndex, UInt32 index) const
    {
    PRECONDITION (NULL != propertyAccessString, ECOBJECTS_STATUS_PreconditionViolated);
                
    PropertyLayoutCP propertyLayout = NULL;
    ECObjectsStatus status = GetClassLayout().GetPropertyLayout (propertyLayout, propertyAccessString);
    if (ECOBJECTS_STATUS_Success != status || NULL == propertyLayout)
        return ECOBJECTS_STATUS_PropertyNotFound;

    if (useIndex)
        return GetValueFromMemory (v, *propertyLayout, index);
    else
        return GetValueFromMemory (v, *propertyLayout);
    }    

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Bill.Steinbock                  08/2012
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus       ECDBuffer::GetIsNullValueFromMemory (bool& isNull, UInt32 propertyIndex, bool useIndex, UInt32 index) const
    {
    PropertyLayoutCP propertyLayout = NULL;
    ECObjectsStatus status = GetClassLayout().GetPropertyLayoutByIndex (propertyLayout, propertyIndex);
    if (ECOBJECTS_STATUS_Success != status || NULL == propertyLayout)
        return ECOBJECTS_STATUS_PropertyNotFound;    

    if (useIndex)
        {
        if (!propertyLayout->GetTypeDescriptor().IsArray())
            return ECOBJECTS_STATUS_PropertyNotFound;    

        if (index >= GetReservedArrayCount (*propertyLayout))
            return ECOBJECTS_STATUS_IndexOutOfRange;
        }

    isNull = !propertyLayout->HoldsCalculatedProperty() && IsPropertyValueNull (*propertyLayout, useIndex, index);
    return ECOBJECTS_STATUS_Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    05/10
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus  ECDBuffer::GetValueFromMemory (ECValueR v, UInt32 propertyIndex, bool useArrayIndex, UInt32 arrayIndex) const
    {
    PropertyLayoutCP propertyLayout = NULL;
    ECObjectsStatus status = GetClassLayout().GetPropertyLayoutByIndex (propertyLayout, propertyIndex);
    if (ECOBJECTS_STATUS_Success != status || NULL == propertyLayout)
        return ECOBJECTS_STATUS_PropertyNotFound;    

    if (useArrayIndex)
        return GetValueFromMemory (v, *propertyLayout, arrayIndex);
    else
        return GetValueFromMemory (v, *propertyLayout);
    }    

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   08/12
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus         ECDBuffer::SetPrimitiveValueToMemory (ECValueCR v, PropertyLayoutCR propertyLayout, bool useIndex, UInt32 index)
    {
    return SetPrimitiveValueToMemory (v, propertyLayout, useIndex, index, false);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    CaseyMullen     09/09
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus       ECDBuffer::SetPrimitiveValueToMemory (ECValueCR v, PropertyLayoutCR propertyLayout, bool useIndex, UInt32 index, bool alreadyCalculated)
    {
    // When we GetPrimitiveValueFromMemory(), we have already calculated its value and we want to set that value to memory, in which case 'alreadyCalculated' will be false
    // Otherwise, we first need to apply the new calculated property value to its dependent properties
    bool  isOriginalValueNull = IsPropertyValueNull (propertyLayout, useIndex, index);

    if (!alreadyCalculated && propertyLayout.HoldsCalculatedProperty())
        {
        ECObjectsStatus calcStatus = _UpdateCalculatedPropertyDependents (v, propertyLayout);
        switch (calcStatus)
            {
        case ECOBJECTS_STATUS_Success:
            break;
        case ECOBJECTS_STATUS_UnableToSetReadOnlyProperty:
            // It is okay to set the read-only value once
            if (isOriginalValueNull)
                break;
            else
                return calcStatus;
        default:
            return calcStatus;
            }
        }

    bool isInUninitializedFixedCountArray = ((useIndex) && (propertyLayout.GetModifierFlags() & PROPERTYLAYOUTMODIFIERFLAGS_IsArrayFixedCount) && (GetAllocatedArrayCount (propertyLayout) == 0));
            
    if (v.IsNull())
        {
        if (!isInUninitializedFixedCountArray)
            SetPropertyValueNull (propertyLayout, useIndex, index, true);

        if (isOriginalValueNull)
            return ECOBJECTS_STATUS_PropertyValueMatchesNoChange;

        return ECOBJECTS_STATUS_Success;
        } 

    if (isInUninitializedFixedCountArray)
        {
        ArrayResizer::CreateNullArrayElementsAt (GetClassLayout(), propertyLayout, *this, 0, GetReservedArrayCount (propertyLayout));
        }   

    UInt32 offset = GetOffsetOfPropertyValue (propertyLayout, useIndex, index);

#ifdef EC_TRACE_MEMORY 
    wprintf (L"SetValue %ls of 0x%x at offset=%d to %ls.\n", propertyAccessString, this, offset, v.ToString().c_str());
#endif    
    ECTypeDescriptor typeDescriptor = propertyLayout.GetTypeDescriptor();
    PrimitiveType primitiveType;
    if (typeDescriptor.IsPrimitive() || (typeDescriptor.IsPrimitiveArray()))
        primitiveType = typeDescriptor.GetPrimitiveType();
    else
        primitiveType = GetStructArrayPrimitiveType ();

    ECObjectsStatus result = ECOBJECTS_STATUS_Error;

    switch (primitiveType)
        {
        case PRIMITIVETYPE_Integer:
            {
            if (!v.IsInteger ())
                return ECOBJECTS_STATUS_DataTypeMismatch;

            Int32 value = v.GetInteger();

            UInt32* valueP = (UInt32*)(GetPropertyData() + offset);
            if (!isOriginalValueNull && *valueP == value)
                return ECOBJECTS_STATUS_PropertyValueMatchesNoChange;

            result = ModifyData (valueP, value);
            }
            break;
        case PRIMITIVETYPE_Long:
            {
            if (!v.IsLong ())
                return ECOBJECTS_STATUS_DataTypeMismatch;

            Int64 value = v.GetLong();
            byte const* valueP = GetPropertyData() + offset;
            if (!isOriginalValueNull && 0 == memcmp (valueP, &value, sizeof(value)))
                return ECOBJECTS_STATUS_PropertyValueMatchesNoChange;

            result = ModifyData (valueP, &value, sizeof (value));
            }
            break;
        case PRIMITIVETYPE_Double:
            {
            if (!v.IsDouble ())
                return ECOBJECTS_STATUS_DataTypeMismatch;

            double value = v.GetDouble();
            byte const* valueP = GetPropertyData() + offset;
            if (!isOriginalValueNull && 0 == memcmp (valueP, &value, sizeof(value)))
                return ECOBJECTS_STATUS_PropertyValueMatchesNoChange;

            result = ModifyData (valueP, &value, sizeof(value));
            }       
            break;
        case PRIMITIVETYPE_String:
            {
            if (!v.IsString ())
                return ECOBJECTS_STATUS_DataTypeMismatch;

            void const* value;
            UInt32 bytesNeeded;
            if (StringEncoding_Utf16 == GetStringEncoding())
                {
                value = v.GetUtf16CP();
                bytesNeeded = (1 + (UInt32)BeStringUtilities::Utf16Len (v.GetUtf16CP())) * sizeof(Utf16Char);
                }
            else
                {
                value = v.GetUtf8CP();
                bytesNeeded = 1 + (UInt32)strlen (v.GetUtf8CP());
                }

            UInt32 currentSize;
            if (useIndex)
                currentSize = GetPropertyValueSize (propertyLayout, index);
            else
                currentSize = GetPropertyValueSize (propertyLayout);

            if (!isOriginalValueNull && currentSize>=bytesNeeded && 0 == memcmp (GetPropertyData() + offset, value, bytesNeeded))
                return ECOBJECTS_STATUS_PropertyValueMatchesNoChange;

            ECObjectsStatus status;

            if (useIndex)
                status = EnsureSpaceIsAvailableForArrayIndexValue (propertyLayout, index, bytesNeeded);
            else
                status = EnsureSpaceIsAvailable (offset, propertyLayout, bytesNeeded);

            if (ECOBJECTS_STATUS_Success != status)
                return status;

            result = ModifyData (GetPropertyData() + offset, value, bytesNeeded);
            }
            break;
        case PRIMITIVETYPE_IGeometry:
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

            if (!isOriginalValueNull && currentSize>=totalSize && 0 == memcmp (GetPropertyData() + offset, dataBuffer, bytesNeeded))
                return ECOBJECTS_STATUS_PropertyValueMatchesNoChange;

            ECObjectsStatus status;
            if (useIndex)
                status = EnsureSpaceIsAvailableForArrayIndexValue (propertyLayout, index, bytesNeeded);
            else
                status = EnsureSpaceIsAvailable (offset, propertyLayout, bytesNeeded);

            if (ECOBJECTS_STATUS_Success != status)
                return status;
                
            // WIP_FUSION We need to figure out the story for value size.  It is legitamate to have a non-null binary value that is
            // 0 bytes in length.  Currently, we do not track that length anywhere.  How do we capture this in the case that the binary value was previously set to some
            // value > 0 in length and we are not auto compressing property values.
            if (bytesNeeded == 0)
                return ECOBJECTS_STATUS_Success;
            
            result = ModifyData (GetPropertyData() + offset, dataBuffer, bytesNeeded);
            free (dataBuffer);
            }
            break;
        case PRIMITIVETYPE_Boolean:
            {
            if (!v.IsBoolean ())
                return ECOBJECTS_STATUS_DataTypeMismatch;

            bool value = v.GetBoolean();
            byte const* valueP = GetPropertyData() + offset;
            if (!isOriginalValueNull && 0 == memcmp (valueP, &value, sizeof(value)))
                return ECOBJECTS_STATUS_PropertyValueMatchesNoChange;

            result = ModifyData (valueP, &value, sizeof(value));
            }       
            break;
        case PRIMITIVETYPE_Point2D:
            {
            if (!v.IsPoint2D ())
                return ECOBJECTS_STATUS_DataTypeMismatch;

            DPoint2d value = v.GetPoint2D();
            byte const* valueP = GetPropertyData() + offset;
            if (!isOriginalValueNull && 0 == memcmp (valueP, &value, sizeof(value)))
                return ECOBJECTS_STATUS_PropertyValueMatchesNoChange;

            result = ModifyData (valueP, &value, sizeof(value));
            }       
            break;
        case PRIMITIVETYPE_Point3D:
            {
            if (!v.IsPoint3D ())
                return ECOBJECTS_STATUS_DataTypeMismatch;

            DPoint3d value = v.GetPoint3D();
            byte const* valueP = GetPropertyData() + offset;
            if (!isOriginalValueNull && 0 == memcmp (valueP, &value, sizeof(value)))
                return ECOBJECTS_STATUS_PropertyValueMatchesNoChange;

            result = ModifyData (valueP, &value, sizeof(value));
            } 
            break;
        case PRIMITIVETYPE_DateTime:      // stored as long
            {
            if (!v.IsDateTime ())
                return ECOBJECTS_STATUS_DataTypeMismatch;

            Int64 value = v.GetDateTimeTicks();
            byte const* valueP = GetPropertyData() + offset;
            if (!isOriginalValueNull && 0 == memcmp (valueP, &value, sizeof(value)))
                return ECOBJECTS_STATUS_PropertyValueMatchesNoChange;

            result = ModifyData (valueP, &value, sizeof (value));
            }
            break;
        }

    if (ECOBJECTS_STATUS_Success == result)
        {
        SetPropertyValueNull (propertyLayout, useIndex, index, false);
        return result; 
        }

    POSTCONDITION (false && "datatype not implemented", ECOBJECTS_STATUS_DataTypeNotSupported);
    }                
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Adam.Klatzkin                   01/2010
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus       ECDBuffer::SetValueToMemory (ECValueCR v, PropertyLayoutCR propertyLayout)
    {
    ECTypeDescriptor typeDescriptor = propertyLayout.GetTypeDescriptor();
    PRECONDITION (!typeDescriptor.IsStruct() && "Cannot set an embedded struct value directly. Set the individual struct members instead.", ECOBJECTS_STATUS_DataTypeNotSupported);

    if (typeDescriptor.IsPrimitive())
        return SetPrimitiveValueToMemory (v, propertyLayout, false, 0);

    POSTCONDITION (false && "Can not set the value to memory using the specified property layout because it is an unsupported datatype", ECOBJECTS_STATUS_DataTypeNotSupported);
    }      
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Adam.Klatzkin                   01/2010
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus       ECDBuffer::SetValueToMemory (ECValueCR v, PropertyLayoutCR propertyLayout, UInt32 index)
    {   
    ECTypeDescriptor typeDescriptor = propertyLayout.GetTypeDescriptor();
    PRECONDITION (typeDescriptor.IsArray() && 
        "Can not set the value to memory at an array index using the specified property layout because it is not an array datatype", 
        ECOBJECTS_STATUS_PreconditionViolated);  
                        
    PRECONDITION (!typeDescriptor.IsStruct() && "Cannot set an embedded struct value directly. Set the individual struct members instead.", ECOBJECTS_STATUS_DataTypeNotSupported);

    if (index >= GetReservedArrayCount (propertyLayout))
        return ECOBJECTS_STATUS_IndexOutOfRange;

    if (typeDescriptor.IsPrimitiveArray())
        return SetPrimitiveValueToMemory (v, propertyLayout, true, index);
    else if (typeDescriptor.IsStructArray() && (v.IsNull() || v.IsStruct()))
        {
        if (v.GetStruct().IsValid() && !_IsStructValidForArray (*v.GetStruct(), propertyLayout))
            return ECOBJECTS_STATUS_UnableToSetStructArrayMemberInstance;

        return _SetStructArrayValueToMemory (v, propertyLayout, index);       
        }

    POSTCONDITION (false && "Can not set the value to memory using the specified property layout because it is an unsupported datatype", ECOBJECTS_STATUS_DataTypeNotSupported);
    }     
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   09/12
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus ECDBuffer::SetInternalValueToMemory (PropertyLayoutCR propertyLayout, ECValueCR v, bool useIndex, UInt32 index)
    {
    if (!useIndex && propertyLayout.GetTypeDescriptor().IsPrimitive())
        {
        // It may have a calculated property specification - make sure we don't try to evaluate the specification or apply value to dependent properties
        return SetPrimitiveValueToMemory (v, propertyLayout, false, 0, true);
        }
    else
        return useIndex ? SetValueToMemory (v, propertyLayout, index) : SetValueToMemory (v, propertyLayout);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    CaseyMullen     09/09
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus       ECDBuffer::SetValueToMemory (WCharCP propertyAccessString, ECValueCR v, bool useIndex, UInt32 index)
    {
    PRECONDITION (NULL != propertyAccessString, ECOBJECTS_STATUS_PreconditionViolated);
                
    PropertyLayoutCP propertyLayout = NULL;
    ECObjectsStatus status = GetClassLayout().GetPropertyLayout (propertyLayout, propertyAccessString);
    if (ECOBJECTS_STATUS_Success != status || NULL == propertyLayout)
        return ECOBJECTS_STATUS_PropertyNotFound;       

    if (useIndex)
        return SetValueToMemory (v, *propertyLayout, index);
    else
        return SetValueToMemory (v, *propertyLayout);
    }     

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    05/10
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus       ECDBuffer::SetValueToMemory (UInt32 propertyIndex, ECValueCR v, bool useArrayIndex, UInt32 arrayIndex)
    {
    PropertyLayoutCP propertyLayout = NULL;
    ECObjectsStatus status = GetClassLayout().GetPropertyLayoutByIndex (propertyLayout, propertyIndex);
    if (ECOBJECTS_STATUS_Success != status || NULL == propertyLayout)
        return ECOBJECTS_STATUS_PropertyNotFound;   

    if (useArrayIndex)
        return SetValueToMemory (v, *propertyLayout, arrayIndex);
    else
        return SetValueToMemory (v, *propertyLayout);
    }     

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   08/12
+---------------+---------------+---------------+---------------+---------------+------*/
CalculatedPropertySpecificationCP ECDBuffer::LookupCalculatedPropertySpecification (IECInstanceCR instance, PropertyLayoutCR propLayout) const
    {
    ClassLayoutCR classLayout = GetClassLayout();
    UInt32 propertyIndex;
    ECPropertyCP ecprop;
    if (ECOBJECTS_STATUS_Success == classLayout.GetPropertyLayoutIndex (propertyIndex, propLayout) && NULL != (ecprop = instance.GetEnabler().LookupECProperty (propertyIndex)))
        {
        PrimitiveECPropertyCP primProp;
        ArrayECPropertyCP arrayProp;
        if (NULL != (primProp = ecprop->GetAsPrimitiveProperty()))
            return primProp->GetCalculatedPropertySpecification();
        else if (NULL != (arrayProp = ecprop->GetAsArrayProperty()))
            return arrayProp->GetCalculatedPropertySpecification();
        }

    return NULL;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   08/12
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus  ECDBuffer::EvaluateCalculatedProperty (PropertyLayoutCR propLayout, ECValueR existingValue, bool useArrayIndex, UInt32 arrayIndex) const
    {
    ECValue updatedValue;
    ECObjectsStatus evalStatus = _EvaluateCalculatedProperty (updatedValue, existingValue, propLayout);
    
    if (ECOBJECTS_STATUS_Success != evalStatus || updatedValue.Equals (existingValue))
        return evalStatus;

    // ###TODO: I don't like this cast. Calculated properties require that we modify the instance in order to store the calculated value
    ECDBuffer& memInst = const_cast<ECDBuffer&> (*this);
    evalStatus = memInst.SetPrimitiveValueToMemory (updatedValue, propLayout, useArrayIndex, arrayIndex, true);
    if (ECOBJECTS_STATUS_Success == evalStatus)
        existingValue = updatedValue;

    return evalStatus;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   09/12
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus ECDBuffer::ModifyData (byte const* data, void const* newData, size_t len)
    {
    if (m_allowWritingDirectlyToInstanceMemory)
        {
        memcpy (const_cast<byte*> (data), newData, len);
        return ECOBJECTS_STATUS_Success;
        }
    else
        return _ModifyData ((UInt32)(data - _GetData()), newData, (UInt32)len);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   09/12
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus ECDBuffer::ModifyData (UInt32 const* data, UInt32 newData)
    {
    if (m_allowWritingDirectlyToInstanceMemory)
        {
        *const_cast<UInt32*> (data) = newData;
        return ECOBJECTS_STATUS_Success;
        }
    else
        return _ModifyData ((UInt32)((byte const*)data - _GetData()), &newData, sizeof(newData));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   10/12
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus ECDBuffer::MoveData (byte* to, byte const* from, size_t len)
    {
    if (m_allowWritingDirectlyToInstanceMemory)
        {
        memmove (to, from, len);
        return ECOBJECTS_STATUS_Success;
        }
    else
        {
        byte const* data = _GetData();
        return _MoveData ((UInt32)(to - data), (UInt32)(from - data), (UInt32)len);
        }
    }

#if defined(_WIN32)
static ECDBuffer::StringEncoding s_preferredEncoding = ECDBuffer::StringEncoding_Utf16;
#else
static ECDBuffer::StringEncoding s_preferredEncoding = ECDBuffer::StringEncoding_Utf8;
#endif

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   11/12
+---------------+---------------+---------------+---------------+---------------+------*/
ECDBuffer::StringEncoding ECDBuffer::GetStringEncoding() const
    {
    UInt8 flags = UInt8(*(_GetData() + ECDOFFSET_Flags));
    return 0 != (flags & ECDFLAG_Utf8Encoding) ? StringEncoding_Utf8 : StringEncoding_Utf16;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   11/12
+---------------+---------------+---------------+---------------+---------------+------*/
ECDBuffer::StringEncoding ECDBuffer::GetDefaultStringEncoding ()                { return s_preferredEncoding; }
void ECDBuffer::SetDefaultStringEncoding (StringEncoding def)                   { s_preferredEncoding = def; }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    CaseyMullen     10/09
+---------------+---------------+---------------+---------------+---------------+------*/
WString        ECDBuffer::InstanceDataToString (WCharCP indent) const
    {
    static const bool s_skipDump = false;
    static int s_dumpCount = 0;             // MT: For debugging purposes only...
    s_dumpCount++;
    
    WString oss;

    appendFormattedString (oss, L"%ls================== ECInstance Dump #%d =============================\n", indent, s_dumpCount);
    if (s_skipDump)
        return oss;
  
    byte const * data = GetPropertyData();

    ClassLayoutCR classLayout = GetClassLayout();
    appendFormattedString (oss, L"%lsECClass=%ls at address = 0x%0x\n", indent, classLayout.GetECClassName().c_str(), data);
    UInt32 nProperties = classLayout.GetPropertyCount ();
    UInt32 nNullflagsBitmasks = CalculateNumberNullFlagsBitmasks (nProperties);
    
    for (UInt32 i = 0; i < nNullflagsBitmasks; i++)
        {
        UInt32 offset = GetECDHeaderCP()->GetSize() + i * sizeof(NullflagsBitmask);
        byte const * address = offset + data;
        appendFormattedString (oss, L"%ls  [0x%x][%4.d] Nullflags[%d] = 0x%x\n", indent, address, offset, i, *(NullflagsBitmask*)(data + offset));
        }
    
    for (UInt32 i = 0; i < nProperties; i++)
        {
        PropertyLayoutCP propertyLayout;
        ECObjectsStatus status = classLayout.GetPropertyLayoutByIndex (propertyLayout, i);
        if (ECOBJECTS_STATUS_Success != status)
            {
            appendFormattedString (oss, L"%lsError (%d) returned while getting PropertyLayout #%d", indent, status, i);
            return oss;
            }

        if (propertyLayout->GetTypeDescriptor().IsStruct())
            {
            WCharCP accessStringP = propertyLayout->GetAccessString();
            // skip outputting "Struct" for struct that represents the root instance 
            if (NULL != accessStringP && 0 != *accessStringP)
                appendFormattedString (oss, L"%ls  Struct %ls\n", indent, propertyLayout->GetAccessString());

            continue;
            }

        UInt32 offset = propertyLayout->GetOffset();
        byte const * address = data + offset;
            
        ECValue v;
        GetValueFromMemory (v, *propertyLayout);
        WString valueAsString = v.ToString();
           
        if (propertyLayout->IsFixedSized())            
            appendFormattedString (oss, L"%ls  [0x%x][%4.d] %ls = %ls\n", indent, address, offset, propertyLayout->GetAccessString(), valueAsString.c_str());
        else
            {
            SecondaryOffset secondaryOffset = *(SecondaryOffset*)address;
            byte const * realAddress = data + secondaryOffset;
            
            appendFormattedString (oss, L"%ls  [0x%x][%4.d] -> [0x%x][%4.d] %ls = %ls\n", indent, address, offset, realAddress, secondaryOffset, propertyLayout->GetAccessString(), valueAsString.c_str());
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

                        appendFormattedString (oss, L"%ls  [0x%x][%4.d] Nullflags[%d] = 0x%x\n", indent, bitAddress, nullflagsOffset, i, *(NullflagsBitmask*)(bitAddress));
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
                        BeStringUtilities::Snwprintf(temp, 1024, L"Error (%d) returned while obtaining array index value", status, i);
                        valueAsString = WString (temp);
                        }

                    if (IsArrayOfFixedSizeElements (*propertyLayout))
                        appendFormattedString (oss, L"%ls      [0x%x][%4.d] %d = %ls\n", indent, address, offset, i, valueAsString.c_str());
                    else
                        {
                        SecondaryOffset secondaryOffset = GetOffsetOfArrayIndexValue (GetOffsetOfPropertyValue (*propertyLayout), *propertyLayout, i);
                        byte const * realAddress = data + secondaryOffset;
                        
                        appendFormattedString (oss, L"%ls      [0x%x][%4.d] -> [0x%x][%4.d] %d = %ls\n", indent, address, offset, realAddress, secondaryOffset, i, valueAsString.c_str());                    
                        }     
                    if ((ECOBJECTS_STATUS_Success == status) && (!v.IsNull()) && (v.IsStruct()))
                        {
                        WString structIndent(indent);
                        structIndent.append (L"      ");

                        WString structString = v.GetStruct()->ToString(structIndent.c_str());
                        oss += structString;

                        appendFormattedString (oss, L"%ls=================== END Struct Instance ===========================\n", structIndent);
                        }         
                    }
                }
            }
        }
        
    if (1 == nProperties)
        {
        // We have one PropertyLayout, which is an empty root struct, signifying an empty ECClass
        DEBUG_EXPECT (0 == classLayout.GetSizeOfFixedSection());
        appendFormattedString (oss, L"Class has no properties\n");
        }
    else
        {
        UInt32 offsetOfLast = classLayout.GetSizeOfFixedSection() - sizeof(SecondaryOffset);
        SecondaryOffset * pLast = (SecondaryOffset*)(data + offsetOfLast);
        appendFormattedString (oss, L"%ls  [0x%x][%4.d] Offset of TheEnd = %d\n", indent, pLast, offsetOfLast, *pLast);
        }

    return oss;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Adam.Klatzkin                   02/2010
+---------------+---------------+---------------+---------------+---------------+------*/
ArrayResizer::ArrayResizer (ClassLayoutCR classLayout, PropertyLayoutCR propertyLayout, ECDBufferR instance, UInt32 resizeIndex, UInt32 resizeElementCount) : m_classLayout (classLayout),
        m_propertyLayout (propertyLayout), m_instance (instance), m_resizeIndex (resizeIndex), m_resizeElementCount (resizeElementCount)    
    {                
    m_preAllocatedArrayCount = m_instance.GetAllocatedArrayCount (propertyLayout);    
            
    ECTypeDescriptor typeDescriptor = propertyLayout.GetTypeDescriptor();
    if (typeDescriptor.IsPrimitiveArray())
        m_elementType = typeDescriptor.GetPrimitiveType();
    else
        m_elementType = instance.GetStructArrayPrimitiveType ();

    m_elementTypeIsFixedSize = IsArrayOfFixedSizeElements (propertyLayout) || (PRIMITIVETYPE_Integer == instance.GetStructArrayPrimitiveType () && propertyLayout.GetTypeDescriptor().IsStructArray());
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
    m_pResizeIndexPreShift = m_propertyData + m_arrayOffset + offsetOfResizePoint;
    m_pResizeIndexPostShift = m_pResizeIndexPreShift + m_resizeFixedSectionByteCount;        
    #ifndef NDEBUG
    UInt32 arrayByteCountAfterGrow = m_instance.GetPropertyValueSize (m_propertyLayout);
    byte const * pNextProperty = m_propertyData + m_arrayOffset + arrayByteCountAfterGrow;
    DEBUG_EXPECT (pNextProperty == m_propertyData + *((SecondaryOffset*)(m_propertyData + m_propertyLayout.GetOffset()) + 1));
    #endif    
    if (m_preArrayByteCount > offsetOfResizePoint)
        {        
        UInt32 byteCountToShift = m_preArrayByteCount - offsetOfResizePoint;        
        DEBUG_EXPECT (m_pResizeIndexPostShift + byteCountToShift <= pNextProperty); 
        m_instance.MoveData ((byte*)m_pResizeIndexPostShift, m_pResizeIndexPreShift, byteCountToShift);
        }
    
    return SetSecondaryOffsetsFollowingResizeIndex();        
    }

/*---------------------------------------------------------------------------------**//**
* @bsistruct                                                    Paul.Connelly   09/12
+---------------+---------------+---------------+---------------+---------------+------*/
struct ScopedWriteBuffer
    {
private:
    ScopedArray<byte>       m_scopedArray;
    byte*                   m_data;

    ScopedWriteBuffer (ScopedWriteBuffer const&);
    ScopedWriteBuffer const& operator= (ScopedWriteBuffer const&);
public:
    ScopedWriteBuffer (UInt32 size, bool allowWritingDirectlyToMemory, void* directMemory)
        : m_scopedArray (allowWritingDirectlyToMemory ? 1 : size),
        m_data (allowWritingDirectlyToMemory ? (byte*)directMemory : m_scopedArray.GetData()) { }

    byte*       GetData()   { return m_data; }
    };

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
    ScopedWriteBuffer writeBuffer (insertedSecondaryOffsetByteCount + (nSecondaryOffsetsShifted * sizeof (SecondaryOffset)), m_instance.m_allowWritingDirectlyToInstanceMemory, pSecondaryOffset);
    pWriteBuffer = writeBuffer.GetData();
    if (!m_instance.m_allowWritingDirectlyToInstanceMemory)
        sizeOfWriteBuffer = insertedSecondaryOffsetByteCount + (nSecondaryOffsetsShifted * sizeof (SecondaryOffset));
        
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
        byte const* modifyP = m_pResizeIndexPostShift - insertedSecondaryOffsetByteCount;
        status = m_instance.ModifyData (modifyP, pWriteBuffer, sizeOfWriteBuffer);            
        //DEBUG_EXPECT (m_propertyData + modifyOffset + sizeOfWriteBuffer <= m_propertyData + m_arrayOffset + m_instance.GetPropertyValueSize (m_propertyLayout));
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
    
    byte const * pShiftFrom = m_propertyData + m_arrayOffset + m_preHeaderByteCount;
    byte * pShiftTo = (byte*)(m_propertyData + m_arrayOffset + m_postHeaderByteCount);
    UInt32 byteCountToShift = (UInt32)(m_pResizeIndexPreShift - pShiftFrom);                
    
    // shift all the elements in the fixed section preceding the insert point if we needed to grow the nullflags bitmask            
    if ((pShiftTo != pShiftFrom))
        {
        m_instance.MoveData (pShiftTo, pShiftFrom, byteCountToShift);
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
    
    ScopedWriteBuffer writeBuffer (byteCountToSet, m_instance.m_allowWritingDirectlyToInstanceMemory, pSecondaryOffset);
    byte * pWriteBuffer = writeBuffer.GetData();
     
    // update shifted secondary offsets        
    SecondaryOffset* pSecondaryOffsetWriteBuffer = (SecondaryOffset*)pWriteBuffer;

    for (UInt32 i = 0; i < m_resizeIndex ;i++)
        {
        pSecondaryOffsetWriteBuffer[i] = pSecondaryOffset[i] + m_resizeFixedSectionByteCount;
        DEBUG_EXPECT (pSecondaryOffsetWriteBuffer[i] <= m_postSecondaryOffsetOfResizeIndex);
        }
        
    if (!m_instance.m_allowWritingDirectlyToInstanceMemory)
        {
        status = m_instance.ModifyData ((byte*)pSecondaryOffset, pWriteBuffer, byteCountToSet);
        //DEBUG_EXPECT (modifyOffset >= m_postHeaderByteCount);
        //DEBUG_EXPECT (m_propertyData + modifyOffset + byteCountToSet <= m_pResizeIndexPostShift);
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
    // write the new array header (updated count & null flags)      
    ECObjectsStatus status = ECOBJECTS_STATUS_Success;
    ScopedWriteBuffer writeBuffer (m_postHeaderByteCount, m_instance.m_allowWritingDirectlyToInstanceMemory, (byte*)(m_propertyData + m_arrayOffset));
    byte * pWriteBuffer = writeBuffer.GetData();
    if (!m_instance.m_allowWritingDirectlyToInstanceMemory)
        memcpy (pWriteBuffer, m_propertyData + m_arrayOffset, m_preHeaderByteCount);

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
        NullflagsBitmask newNullFlags = *pNullflagsCurrent;
        if (isNull && 0 == (*pNullflagsCurrent & nullflagsBitmask))
            newNullFlags |= nullflagsBitmask; // turn on the null bit
        else if (!isNull && nullflagsBitmask == (*pNullflagsCurrent & nullflagsBitmask))
            newNullFlags ^= nullflagsBitmask; // turn off the null bit

        m_instance.ModifyData (pNullflagsCurrent, newNullFlags);
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
        status = m_instance.ModifyData (m_propertyData + m_arrayOffset, pWriteBuffer, m_postHeaderByteCount);
        if (ECOBJECTS_STATUS_Success != status)
            return status;        
        }                       
        
    return status;
    }
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Adam.Klatzkin                   01/2010
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus       ArrayResizer::CreateNullArrayElementsAt (ClassLayoutCR classLayout, PropertyLayoutCR propertyLayout, ECDBufferR instance, UInt32 insertIndex, UInt32 insertCount)
    {                        
    ArrayResizer resizer (classLayout, propertyLayout, instance, insertIndex, insertCount);
    PRECONDITION (resizer.m_resizeIndex <= resizer.m_preAllocatedArrayCount, ECOBJECTS_STATUS_IndexOutOfRange);        
    
    ECObjectsStatus status = instance.EnsureSpaceIsAvailable (resizer.m_arrayOffset, propertyLayout, resizer.m_preArrayByteCount + resizer.m_resizeFixedSectionByteCount);
    if (ECOBJECTS_STATUS_Success != status)
        return status;       
        
    resizer.m_propertyData = resizer.m_instance.GetPropertyData();
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

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   11/12
+---------------+---------------+---------------+---------------+---------------+------*/
ECDHeader_v0::ECDHeader_v0()
  : m_formatVersion(ECDFormat_Current), m_readableByVersion(ECDFormat_MinimumReadable), m_writableByVersion(ECDFormat_MinimumWritable), m_headerSize(sizeof(ECDHeader)), m_flags(0)
    {
    if (ECDBuffer::StringEncoding_Utf8 == ECDBuffer::GetDefaultStringEncoding())
        SetFlag (ECDFLAG_Utf8Encoding, true);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   05/13
+---------------+---------------+---------------+---------------+---------------+------*/
ECDHeader_v0::ECDHeader_v0 (bool useUtf8Encoding)
  : m_formatVersion(ECDFormat_Current), m_readableByVersion(ECDFormat_MinimumReadable), m_writableByVersion(ECDFormat_MinimumWritable), m_headerSize(sizeof(ECDHeader)), m_flags(0)
    {
    if (useUtf8Encoding)
        SetFlag (ECDFLAG_Utf8Encoding, true);
    }
   
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   11/12
+---------------+---------------+---------------+---------------+---------------+------*/
bool ECDHeader_v0::ReadHeader (ECDHeader_v0& hdrOut, byte const* data)
    {
    // Note this returns the version of the header with which the code was compiled, truncating any data added to the struct in a future version.
    // This is correct - older code has no way to interpret the additional data anyway.
    // The original header remains intact in the ECD buffer's data.
    ECDHeader_v0* hdr = (ECDHeader_v0*)data;
    if (ECDFormat_Invalid == hdr->m_formatVersion)
        {
        // When would this actually happen?
        return false;
        }

    // Note we use sizeof(ECDHeader), not hdr->GetSize(), because the persisted header might be from a newer ECD format version and be larger than ECDHeader can hold
    memcpy (&hdrOut, hdr, sizeof(ECDHeader));
    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   11/12
+---------------+---------------+---------------+---------------+---------------+------*/
ECDHeader const* ECDBuffer::GetECDHeaderCP() const
    {
    byte const* data = _GetData();
#ifndef NDEBUG
    ECDHeader hdr;
    if (!ECDHeader::ReadHeader (hdr, data))
        {
        BeAssert (false);
        return NULL;
        }
#endif

    return (ECDHeader const*)data;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   11/12
+---------------+---------------+---------------+---------------+---------------+------*/
UInt32 ECDBuffer::GetOffsetToPropertyData() const
    {
    UInt8* hdrSizeP = (UInt8*)(_GetData() + ECDOFFSET_HeaderSize);
    return *hdrSizeP;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   11/12
+---------------+---------------+---------------+---------------+---------------+------*/
UInt32 ECDBuffer::CalculateInitialAllocation (ClassLayoutCR classLayout)
    {
    return sizeof(ECDHeader) + classLayout.GetSizeOfFixedSection();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   11/12
+---------------+---------------+---------------+---------------+---------------+------*/
bool ECDBuffer::IsCompatibleVersion (ECDHeader* header, byte const* data, bool requireWritable)
    {
    ECDHeader localHeader;
    if (NULL == header)
        header = &localHeader;

    if (!ECDHeader::ReadHeader (*header, data))
        return false;
    else if (requireWritable && !header->IsWritable())
        return false;
    else
        return header->IsReadable();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   04/13
+---------------+---------------+---------------+---------------+---------------+------*/
size_t ECDBuffer::GetBufferSize() const
    {
    ScopedDataAccessor scopedDataAccessor (*this);
    if (scopedDataAccessor.IsValid())
        return (size_t)CalculateBytesUsed();
    else
        return 0;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   04/13
+---------------+---------------+---------------+---------------+---------------+------*/
void ECDBuffer::GetBufferData (byte* dest) const
    {
    ScopedDataAccessor scopedDataAccessor (*this);
    if (!scopedDataAccessor.IsValid())
        { BeAssert (false); return; }

    size_t nBytes = GetBufferSize();
    memcpy (dest, _GetData(), nBytes);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   04/13
+---------------+---------------+---------------+---------------+---------------+------*/
bool ECDBuffer::IsEmpty() const
    {
    ScopedDataAccessor dataAccessor (*this);
    if (!dataAccessor.IsValid())
        { BeAssert (false); return false; }

    ClassLayoutCR classLayout = GetClassLayout();

    // A few notes about this method:
    // 1. We'd like to be able to test if classLayout.GetSizeOfFixedSection() != classLayout.CalculateBytesUsed() => variable-sized properties exist => some non-null properties exist
    //    But that's not true, because when setting a variable-sized property to null we don't deallocate its storage.
    // 2. We'd like to be able to test all our null flags bitmasks against 0xFFFFFFFF => all properties are null
    //    This doesn't work because we don't un-set null flags for array properties even when adding elements to them - they are always set.
    // So we are left having to simply check each property individually. Oh well.

    // structs are always null
    // primitive properties use null flags to indicate null-ness
    // array properties' null flags are always "on". check array count.
    for (PropertyLayout const* propLayout: classLayout.m_propertyLayouts)
        {
        if (propLayout->GetTypeDescriptor().IsPrimitive())
            {
            if (!IsPropertyValueNull (*propLayout, false, 0))
                return false;
            }
        else if (propLayout->GetTypeDescriptor().IsArray())
            {
            if (propLayout->IsFixedSized())
                {
                // ECD does not currently support fixed-sized arrays - they always are allocated in the variable-sized portion of the buffer.
                // If it does in the future, we need to check each element to determine if it's null - the space will be allocated in fixed-size portion of buffer.
                BeAssert (false);
                return false;
                }
            else if (0 < GetReservedArrayCount (*propLayout))
                {
                // a non-empty array, even if it contains only null elements, is still a non-empty array => buffer is non-empty
                return false;
                }
            }
        }

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   05/13
+---------------+---------------+---------------+---------------+---------------+------*/
bool ECDBuffer::EvaluateAllCalculatedProperties()
    {
    ScopedDataAccessor scopedDataAccessor (*this);
    if (!scopedDataAccessor.IsValid())
        { BeAssert (false); return false; }

    for (PropertyLayout const* propLayout: GetClassLayout().m_propertyLayouts)
        {
        if (propLayout->HoldsCalculatedProperty())
            {
            ECValue v;
            if (ECOBJECTS_STATUS_Success == GetValueFromMemory (v, *propLayout) && v.IsArray())
                {
                // an array of calculated primitive values
                UInt32 arrayCount = v.GetArrayInfo().GetCount();
                for (UInt32 i = 0; i < arrayCount; i++)
                    GetValueFromMemory (v, *propLayout, i);
                }
            }
        else if (propLayout->GetTypeDescriptor().IsStructArray())
            {
            ECValue v;
            if (ECOBJECTS_STATUS_Success == GetValueFromMemory (v, *propLayout))
                {
                UInt32 arrayCount = v.GetArrayInfo().GetCount();
                for (UInt32 i = 0; i < arrayCount; i++)
                    {
                    if (ECOBJECTS_STATUS_Success == GetValueFromMemory (v, *propLayout, i) && !v.IsNull() && NULL != v.GetStruct()->GetECDBuffer())
                        v.GetStruct()->GetECDBufferP()->EvaluateAllCalculatedProperties();
                    }
                }
            }
        }

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   05/13
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus ECDBuffer::CopyDataBuffer (ECDBufferCR src, bool allowClassLayoutConversion)
    {
    // Do we really support this? Currently everybody uses integers for this...
    if (GetStructArrayPrimitiveType() != src.GetStructArrayPrimitiveType())
        return ECOBJECTS_STATUS_OperationNotSupported;

    // Note this method does not do anything about per-property flags, etc, which are relevant to MemoryECInstanceBase-derived classes...
    ScopedDataAccessor srcAccessor (src), dstAccessor (*this, true);
    if (!srcAccessor.IsValid() || !dstAccessor.IsValid())
        return ECOBJECTS_STATUS_Error;

    ECObjectsStatus status = ECOBJECTS_STATUS_Success;
    ClearValues();

    ClassLayoutCR srcLayout = src.GetClassLayout();
    ClassLayoutCR dstLayout = GetClassLayout();
    if (srcLayout.Equals (dstLayout))
        {
        // we can copy directly
        UInt32 bytesAvailable = _GetBytesAllocated();
        UInt32 bytesNeeded = src.CalculateBytesUsed();
        if (bytesNeeded <= bytesAvailable || ECOBJECTS_STATUS_Success == (status = _GrowAllocation (bytesNeeded - bytesAvailable)))
            status = ModifyData (_GetData(), src._GetData(), bytesNeeded);
        }
    else if (!allowClassLayoutConversion)
        status = ECOBJECTS_STATUS_ECClassNotSupported;
    else
        {
        // The ClassLayouts differ - we need to set properties one-by-one
        // We may need to convert primitive property values from one type to another, skip properties not present in one buffer or the other, etc
        ECValue v;
        v.SetAllowsPointersIntoInstanceMemory (true);
        for (PropertyLayoutCP srcPropLayout: srcLayout.m_propertyLayouts)
            {
            PropertyLayoutCP dstPropLayout;
            if (ECOBJECTS_STATUS_Success == dstLayout.GetPropertyLayout (dstPropLayout, srcPropLayout->GetAccessString()))
                {
                ECTypeDescriptor dstType = dstPropLayout->GetTypeDescriptor(),
                                 srcType = srcPropLayout->GetTypeDescriptor();

                if (dstType.GetTypeKind() != srcType.GetTypeKind())
                    {
                    BeAssert (false && "ECDBuffer::CopyDataBuffer() skipping property with mismatched types...this may be a programmer error");
                    continue;
                    }
                else if (dstType.IsStruct())
                    continue;   // embedded structs always null
                else if (dstType.IsPrimitive())
                    {
                    if (ECOBJECTS_STATUS_Success == src.GetPrimitiveValueFromMemory (v, *srcPropLayout, false, 0) && v.ConvertToPrimitiveType (dstType.GetPrimitiveType()))
                        SetPrimitiveValueToMemory (v, *dstPropLayout, false, 0, true /* set calculated property directly */);
                    }
                else // array
                    {
                    // ###TODO? If the primitive types match, we could conceivably copy the entire array as one big chunk of memory, which would save us some behind-the-scenes
                    // memory swapping/allocation for arrays of variable-sized properties (principally strings). Worth it?

                    // Note for struct arrays this copies the struct value identifiers directly, does not copy or create struct array instances or fix up identifiers
                    if (ECOBJECTS_STATUS_Success == src.GetValueFromMemory (v, *srcPropLayout) && v.IsArray() && v.GetArrayInfo().GetCount() > 0)
                        {
                        UInt32 propertyIndex;
                        if (ECOBJECTS_STATUS_Success != dstLayout.GetPropertyLayoutIndex (propertyIndex, *dstPropLayout))
                            { BeAssert(false); continue; }

                        UInt32 count = v.GetArrayInfo().GetCount();
                        status = AddNullArrayElementsAt (propertyIndex, count);
                        if (ECOBJECTS_STATUS_Success != status)
                            return status;

                        for (UInt32 i = 0; i < count; i++)
                            {
                            if (ECOBJECTS_STATUS_Success == src.GetPrimitiveValueFromMemory (v, *srcPropLayout, true, i) && v.ConvertToPrimitiveType (dstType.GetPrimitiveType()))
                                SetPrimitiveValueToMemory (v, *dstPropLayout, true, i, true);
                            }
                        }
                    }
                }
            }
        }

    return status;
    }

END_BENTLEY_ECOBJECT_NAMESPACE

