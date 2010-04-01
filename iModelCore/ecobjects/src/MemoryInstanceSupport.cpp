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
        return CalculateFixedArrayPropertySize (fixedCount, m_typeDescriptor.GetPrimitiveType());
        }
        
    DEBUG_FAIL("Can not determine size in fixed section for datatype");
    return 0;
    }    

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    CaseyMullen     10/09
+---------------+---------------+---------------+---------------+---------------+------*/
ClassLayout::ClassLayout(SchemaIndex schemaIndex) : 
    m_schemaIndex (schemaIndex),
    m_isPersisted(false),
    m_classIndex(0),
    m_sizeOfFixedSection(0), 
    m_isRelationshipClass(false), m_propertyIndexOfSourceECPointer(-1), m_propertyIndexOfTargetECPointer(-1)
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
    memset (data, 0, sizeof(InstanceHeader)); 
    
    InstanceHeader& header = *(InstanceHeader*) data;
    header.m_schemaIndex   = m_schemaIndex;
    header.m_classIndex    = m_classIndex;
    header.m_instanceFlags = 0;

    UInt32 nNullflagsBitmasks = CalculateNumberNullFlagsBitmasks (GetPropertyCount());
    InitializeNullFlags ((NullflagsBitmask *)(data + sizeof (InstanceHeader)), nNullflagsBitmasks);
            
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
        
#ifdef BOOLEAN_CAUSES_PROBLEMS
        DEBUG_EXPECT (layout.GetOffset() % 4 == 0 && "We expect secondaryOffsets to be aligned on a 4 byte boundary");
#endif
            
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
* @bsimethod                                    Bill.Steinbock                  03/2010
+---------------+---------------+---------------+---------------+---------------+------*/
bool            ClassLayout::IsCompatible (ClassLayoutCR classLayout) const
    {
    if (0 != _wcsicmp (GetECClassName().c_str(), classLayout.GetECClassName().c_str()))
        return false;

    UInt32 nProperties = GetPropertyCount ();
    if (nProperties != classLayout.GetPropertyCount())
        return false;

    for (UInt32 i = 0; i < nProperties; i++)
        {
        PropertyLayoutCP propertyLayout;
        StatusInt status = GetPropertyLayoutByIndex (propertyLayout, i);
        if (SUCCESS != status)
            return false;

        PropertyLayoutCP comparePropertyLayout;
        status = classLayout.GetPropertyLayout (comparePropertyLayout, propertyLayout->GetAccessString());
        if (SUCCESS != status)
            return false;

        if (comparePropertyLayout->GetTypeDescriptor().GetTypeKind() != propertyLayout->GetTypeDescriptor().GetTypeKind())
            return false;

        if (comparePropertyLayout->GetTypeDescriptor().IsStructArray() != propertyLayout->GetTypeDescriptor().IsStructArray())
            return false;

        if (propertyLayout->GetTypeDescriptor().IsPrimitiveArray())
            {
            if (!comparePropertyLayout->GetTypeDescriptor().IsPrimitiveArray())
                return false;

            if (comparePropertyLayout->GetTypeDescriptor().GetPrimitiveType() != propertyLayout->GetTypeDescriptor().GetPrimitiveType())
                return false;
            }

        if (propertyLayout->GetTypeDescriptor().IsPrimitive())
            {
            if (!comparePropertyLayout->GetTypeDescriptor().IsPrimitive())
                return false;

            if (comparePropertyLayout->GetTypeDescriptor().GetPrimitiveType() != propertyLayout->GetTypeDescriptor().GetPrimitiveType())
                return false;
            }
        }

    return true;
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
std::wstring const &  ClassLayout::GetECClassName() const
    {
    return m_className;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    CaseyMullen    01/10
+---------------+---------------+---------------+---------------+---------------+------*/
bool            ClassLayout::IsPersisted() const
    {
    return m_isPersisted;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    CaseyMullen    01/10
+---------------+---------------+---------------+---------------+---------------+------*/
void            ClassLayout::SetIsPersisted (bool isPersisted) const
    {
    const_cast<ClassLayoutP>(this)->m_isPersisted = isPersisted;
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
* @bsimethod                                                    CaseyMullen     10/09
+---------------+---------------+---------------+---------------+---------------+------*/    
void            ClassLayout::Factory::AddProperty (wchar_t const * accessString, ECTypeDescriptor typeDescriptor, UInt32 size, UInt32 modifierFlags, UInt32 modifierData)
    {
    UInt32  positionInCurrentNullFlags = m_underConstruction.GetPropertyCount() % 32;
    NullflagsBitmask  nullflagsBitmask = 0x01 << positionInCurrentNullFlags;
    
    if (0 == positionInCurrentNullFlags && 0 != m_underConstruction.GetPropertyCount())
        {
        // It is time to add a new set of Nullflags
        m_nullflagsOffset += sizeof(NullflagsBitmask);
        m_offset          += sizeof(NullflagsBitmask);
        for (UInt32 i = 0; i < m_underConstruction.GetPropertyCount(); i++)
            m_underConstruction.m_propertyLayouts[i].m_offset += sizeof(NullflagsBitmask); // Offsets of already-added property layouts need to get bumped up
        } 

    // WIP_FUSION, for now all accessors of array property layouts are stored with the brackets appended.  This means all access to array values through an
    // IECInstance must include the brackets.  If you want to obtain an array element value then you specify an index.  If you want to obtain an array info value
    // then you do not specify an index.  I'd like to consider an update to this so if an access string does not include the [] then we always return the ArrayInfo value.
    std::wstring tempAccessString = accessString;
    if (typeDescriptor.IsArray())
        tempAccessString += L"[]";

    PropertyLayout propertyLayout (tempAccessString.c_str(), typeDescriptor, m_offset, m_nullflagsOffset, nullflagsBitmask, modifierFlags, modifierData);
    m_underConstruction.m_propertyLayouts.push_back(propertyLayout);
    m_underConstruction.CheckForECPointers (accessString);

    m_offset += (UInt32)size;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    CaseyMullen     10/09
+---------------+---------------+---------------+---------------+---------------+------*/    
void            ClassLayout::Factory::AddFixedSizeProperty (wchar_t const * accessString, ECTypeDescriptor typeDescriptor)
    {
    if (!EXPECTED_CONDITION (m_state == AcceptingFixedSizeProperties)) // ClassLayoutNotAcceptingFixedSizeProperties    
        return;
    
    if (!typeDescriptor.IsPrimitive())
        {
        DEBUG_FAIL ("We currently only support fixed sized properties for primitive types");
        return;
        }
    
    UInt32 size = GetFixedPrimitiveValueSize (typeDescriptor.GetPrimitiveType());
    
    AddProperty (accessString, typeDescriptor, size);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Adam.Klatzkin                   01/2010
+---------------+---------------+---------------+---------------+---------------+------*/ 
void            ClassLayout::Factory::AddFixedSizeArrayProperty (wchar_t const * accessString, ECTypeDescriptor typeDescriptor, UInt32 arrayCount)
    {
    if (!EXPECTED_CONDITION (m_state == AcceptingFixedSizeProperties)) // ClassLayoutNotAcceptingFixedSizeProperties    
        return;
    
    UInt32 size = CalculateFixedArrayPropertySize (arrayCount, typeDescriptor.GetPrimitiveType());
    
    AddProperty (accessString, typeDescriptor, size, ARRAYMODIFIERFLAGS_IsFixedCount, arrayCount);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    CaseyMullen     10/09
+---------------+---------------+---------------+---------------+---------------+------*/ 
void            ClassLayout::Factory::AddVariableSizeProperty (wchar_t const * accessString, ECTypeDescriptor typeDescriptor)
    {
    if (m_state == AcceptingFixedSizeProperties)
        m_state = AcceptingVariableSizeProperties;

    if (!EXPECTED_CONDITION (m_state == AcceptingVariableSizeProperties)) // ClassLayoutNotAcceptingVariableSizeProperties    
        return;
        
    UInt32 size = sizeof(SecondaryOffset); // the offset will just point to this secondary offset

    AddProperty (accessString, typeDescriptor, size);
    }  

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Adam.Klatzkin                   01/2010
+---------------+---------------+---------------+---------------+---------------+------*/ 
void            ClassLayout::Factory::AddVariableSizeArrayPropertyWithFixedCount (wchar_t const * accessString, ECTypeDescriptor typeDescriptor, UInt32 arrayCount)
    {
    if (m_state == AcceptingFixedSizeProperties)
        m_state = AcceptingVariableSizeProperties;

    if (!EXPECTED_CONDITION (m_state == AcceptingVariableSizeProperties)) // ClassLayoutNotAcceptingVariableSizeProperties    
        return;
        
    UInt32 size = sizeof(SecondaryOffset); // the offset will just point to this secondary offset
    
    AddProperty (accessString, typeDescriptor, size, ARRAYMODIFIERFLAGS_IsFixedCount, arrayCount);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    12/09
+---------------+---------------+---------------+---------------+---------------+------*/
void            ClassLayout::Factory::AddProperties (ECClassCR ecClass, wchar_t const * nameRoot, bool addingFixedSizeProps)
    {
    for each (ECPropertyP property in ecClass.GetProperties())
        {
        std::wstring    propName = property->GetName();
        
        if (NULL != nameRoot)
            propName = std::wstring (nameRoot) + L"." + propName;

        if (property->GetIsPrimitive())
            {
            PrimitiveECPropertyP  primitiveProp = property->GetAsPrimitiveProperty();
            PrimitiveType         primitiveType = primitiveProp->GetType();

            bool isFixedSize = PrimitiveTypeIsFixedSize(primitiveType);

            if (addingFixedSizeProps && isFixedSize)
                AddFixedSizeProperty (propName.c_str(), primitiveType);
            else if ( ! addingFixedSizeProps && ! isFixedSize)
                AddVariableSizeProperty (propName.c_str(), primitiveType);
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
                    AddFixedSizeArrayProperty (propName.c_str(), ECTypeDescriptor::CreatePrimitiveArrayTypeDescriptor (arrayProp->GetPrimitiveElementType()), arrayProp->GetMinOccurs());
                else if (!addingFixedSizeProps && !isFixedPropertySize)
                    {
                    if (isFixedArrayCount)
                        AddVariableSizeArrayPropertyWithFixedCount (propName.c_str(), ECTypeDescriptor::CreatePrimitiveArrayTypeDescriptor (arrayProp->GetPrimitiveElementType()), arrayProp->GetMinOccurs());
                    else
                        AddVariableSizeProperty (propName.c_str(), ECTypeDescriptor::CreatePrimitiveArrayTypeDescriptor (arrayProp->GetPrimitiveElementType()));
                    }
                }
            else if ((arrayKind == ARRAYKIND_Struct) && (!addingFixedSizeProps))
                {
                bool isFixedArrayCount = (arrayProp->GetMinOccurs() == arrayProp->GetMaxOccurs());
                if (isFixedArrayCount)
                    AddVariableSizeArrayPropertyWithFixedCount (propName.c_str(), ECTypeDescriptor::CreateStructArrayTypeDescriptor(), arrayProp->GetMinOccurs());
                else
                    AddVariableSizeProperty (propName.c_str(), ECTypeDescriptor::CreateStructArrayTypeDescriptor());                
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
        AddVariableSizeProperty (PROPERTYLAYOUT_Source_ECPointer, PRIMITIVETYPE_Binary);
        AddVariableSizeProperty (PROPERTYLAYOUT_Target_ECPointer, PRIMITIVETYPE_Binary);
        }

    m_underConstruction.FinishLayout ();
    
    return &m_underConstruction;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    02/10
+---------------+---------------+---------------+---------------+---------------+------*/
ClassLayout::Factory::Factory (ECClassCR ecClass, ClassIndex classIndex, SchemaIndex schemaIndex)
    : 
    m_ecClass (ecClass),
    m_state   (AcceptingFixedSizeProperties),
    m_offset  (sizeof(InstanceHeader) + sizeof(NullflagsBitmask)),
    m_nullflagsOffset (sizeof(InstanceHeader)),
    m_underConstruction (*ClassLayout::CreateEmpty (m_ecClass.GetName().c_str(), classIndex, schemaIndex))
    {
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    01/10
+---------------+---------------+---------------+---------------+---------------+------*/
ClassLayoutP    ClassLayout::BuildFromClass (ECClassCR ecClass, ClassIndex classIndex, SchemaIndex schemaIndex)
    {
    Factory     factory (ecClass, classIndex, schemaIndex);

    return factory.DoBuildClassLayout ();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    01/10
+---------------+---------------+---------------+---------------+---------------+------*/
ClassLayoutP    ClassLayout::CreateEmpty (wchar_t const* className, ClassIndex classIndex, SchemaIndex schemaIndex)
    {
    ClassLayoutP classLayout = new ClassLayout(schemaIndex);

    classLayout->SetClass (className, classIndex);

    return classLayout;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    CaseyMullen     10/09
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   ClassLayout::SetClass (wchar_t const* className, UInt16 classIndex)
    {
    m_classIndex = classIndex;
    m_className  = className;

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    CaseyMullen     10/09
+---------------+---------------+---------------+---------------+---------------+------*/  
StatusInt       ClassLayout::FinishLayout ()
    {
    for (UInt32 i = 0; i < m_propertyLayouts.size(); i++)
        {
        PropertyLayoutCP propertyLayout = &m_propertyLayouts[i];
        m_propertyLayoutMap[propertyLayout->GetAccessString()] = propertyLayout;
        }
        
    // Calculate size of fixed section    
    if (0 == m_propertyLayouts.size())
        {
        m_sizeOfFixedSection = 0;
        return SUCCESS;
        }

    PropertyLayoutCR lastPropertyLayout = m_propertyLayouts[m_propertyLayouts.size() - 1];
    UInt32    size = lastPropertyLayout.GetOffset() + lastPropertyLayout.GetSizeInFixedSection();

    if ( ! lastPropertyLayout.IsFixedSized())
        size += sizeof(SecondaryOffset); // There is one additional SecondaryOffset tracking the end of the last variable-sized value

    m_sizeOfFixedSection = size;

#ifndef NDEBUG
    UInt32 nNullflagsBitmasks = CalculateNumberNullFlagsBitmasks ((UInt32)m_propertyLayouts.size());
    DEBUG_EXPECT (m_propertyLayouts[0].m_offset == sizeof(InstanceHeader) + nNullflagsBitmasks * sizeof(NullflagsBitmask));

    for (UInt32 i = 0; i < m_propertyLayouts.size(); i++)
        {
        UInt32 expectedNullflagsOffset = (i / BITS_PER_NULLFLAGSBITMASK * sizeof(NullflagsBitmask)) + sizeof(InstanceHeader);
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
        case PRIMITIVETYPE_Boolean:
            return sizeof(bool); 
        case PRIMITIVETYPE_Point2D:
            return 2*sizeof(double);
        case PRIMITIVETYPE_Point3D:
            return 3*sizeof(double);
        case PRIMITIVETYPE_DateTime:
            return sizeof(Int64); //ticks
        default:
            DEBUG_FAIL("Most datatypes have not yet been implemented... or perhaps you have passed in a variable-sized type.");
            return 0;
        }
    }
  
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    CaseyMullen    01/10
+---------------+---------------+---------------+---------------+---------------+------*/     
void            ClassLayout::CheckForECPointers (wchar_t const * accessString)
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
* Called by SchemaLayoutElementHandler's BuildClassLayout() when deserializing ClassLayouts
* @bsimethod                                                    JoshSchifter    01/10
+---------------+---------------+---------------+---------------+---------------+------*/     
void            ClassLayout::AddPropertyDirect
(
wchar_t const *  accessString,
ECTypeDescriptor typeDescriptor,
UInt32           offset,
UInt32           nullflagsOffset,
UInt32           nullflagsBitmask
)
    {
    PropertyLayout propertyLayout (accessString, typeDescriptor, offset, nullflagsOffset, nullflagsBitmask);

    m_propertyLayouts.push_back(propertyLayout);
    CheckForECPointers (accessString);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    CaseyMullen     10/09
+---------------+---------------+---------------+---------------+---------------+------*/
UInt32          ClassLayout::GetPropertyCount () const
    {
    return (UInt32) m_propertyLayouts.size();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    CaseyMullen     10/09
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt       ClassLayout::GetPropertyLayout (PropertyLayoutCP & propertyLayout, wchar_t const * accessString) const
    {
    PropertyLayoutMap::const_iterator it = m_propertyLayoutMap.find(accessString);
    
    if (it == m_propertyLayoutMap.end())
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
    if (m_classLayouts.size() <= classIndex)
        m_classLayouts.resize (1 + classIndex, NULL); // WIP_FUSION: Increase the increment to 20, later. 
                                                      // Keep the increment low for now, to ensure that resizing has not ill side effects.

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
ClassLayoutCP   SchemaLayout::FindClassLayout (wchar_t const * className)
    {
    for each (ClassLayoutCP classLayout in m_classLayouts)
        {
        if (NULL == classLayout)
            continue;

        if (0 == wcsicmp (classLayout->GetECClassName().c_str(), className))
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
        return propertyLayout.GetSizeInFixedSection();
    else
        {    
        UInt32 offset = propertyLayout.GetOffset();
        SecondaryOffset const * pSecondaryOffset = (SecondaryOffset const *)(_GetData() + offset);
        
        SecondaryOffset secondaryOffset = *pSecondaryOffset;
        if (0 == secondaryOffset)
            return 0;

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
        return ClassLayout::GetFixedPrimitiveValueSize (propertyLayout.GetTypeDescriptor().GetPrimitiveType());
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
StatusInt       MemoryInstanceSupport::EnsureSpaceIsAvailable (UInt32& offset, ClassLayoutCR classLayout, PropertyLayoutCR propertyLayout, UInt32 bytesNeeded)
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
    
    return ArrayResizer::CreateNullArrayElementsAt (classLayout, propertyLayout, *this, insertIndex, insertCount);
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
    
    return ArrayResizer::CreateNullArrayElementsAt (classLayout, propertyLayout, *this, GetAllocatedArrayCount (propertyLayout), count);
    }         

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Adam.Klatzkin                   01/2010
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt       MemoryInstanceSupport::EnsureSpaceIsAvailableForArrayIndexValue (ClassLayoutCR classLayout, PropertyLayoutCR propertyLayout, UInt32 arrayIndex, UInt32 bytesNeeded)
    {    
    UInt32 availableBytes = GetPropertyValueSize (propertyLayout, arrayIndex);
    
#ifndef SHRINK_TO_FIT    
    if (bytesNeeded <= availableBytes)
        return SUCCESS;
#endif    
    Int32 additionalBytesNeeded = bytesNeeded - availableBytes;
    
    if (additionalBytesNeeded <= 0)
        return SUCCESS;
    
    UInt32 arrayCount = GetAllocatedArrayCount (propertyLayout);
    
    UInt32 endOfValueDataPreGrow = *((SecondaryOffset*)(_GetData() + propertyLayout.GetOffset()) + 1);
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
    byte const * data = _GetData();
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
        
    byte * writeableData = (byte *)_GetData();
    DEBUG_EXPECT (bytesUsed == classLayout.CalculateBytesUsed(writeableData));
    
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
    if (m_allowWritingDirectlyToInstanceMemory)
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
StatusInt       MemoryInstanceSupport::GetPrimitiveValueFromMemory (ECValueR v, PropertyLayoutCR propertyLayout, bool useIndex, UInt32 index) const
    {
    DEBUG_EXPECT (propertyLayout.GetTypeDescriptor().IsArray() == useIndex);   

    bool isInUninitializedFixedCountArray = ((useIndex) && (propertyLayout.GetModifierFlags() & ARRAYMODIFIERFLAGS_IsFixedCount) && (GetAllocatedArrayCount (propertyLayout) == 0));    
    if (isInUninitializedFixedCountArray || (IsPropertyValueNull(propertyLayout, useIndex, index)))
        {
        v.SetPrimitiveType (propertyLayout.GetTypeDescriptor().GetPrimitiveType());
        v.SetToNull();
        return SUCCESS;
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
        case PRIMITIVETYPE_Binary:
            {
            UInt32 size;
            if (useIndex)
                size = GetPropertyValueSize (propertyLayout, index);
            else
                size = GetPropertyValueSize (propertyLayout);
            v.SetBinary (pValue, size);
            return SUCCESS;
            }  
        case PRIMITIVETYPE_Boolean:
            {
            bool value;
            memcpy (&value, pValue, sizeof(value));
            v.SetBoolean (value);
            return SUCCESS;
            } 
        case PRIMITIVETYPE_Point2D:
            {
            DPoint2d value;
            memcpy (&value, pValue, sizeof(value));
            v.SetPoint2D (value);
            return SUCCESS;
            }       
        case PRIMITIVETYPE_Point3D:
            {
            DPoint3d value;
            memcpy (&value, pValue, sizeof(value));
            v.SetPoint3D (value);
            return SUCCESS;
            }       
        case PRIMITIVETYPE_DateTime:
            {
            Int64 value;
            memcpy (&value, pValue, sizeof(value));
            v.SetDateTimeTicks (value);
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
StatusInt       MemoryInstanceSupport::GetValueFromMemory (ECValueR v, PropertyLayoutCR propertyLayout) const
    {
    ECTypeDescriptor typeDescriptor = propertyLayout.GetTypeDescriptor();
    if (typeDescriptor.IsPrimitive())
        return GetPrimitiveValueFromMemory (v, propertyLayout, false, 0);
    else if (typeDescriptor.IsArray())
        {                
        UInt32 arrayCount = GetReservedArrayCount (propertyLayout);  
        bool isFixedArrayCount = propertyLayout.GetModifierFlags() & ARRAYMODIFIERFLAGS_IsFixedCount;                            
        if (typeDescriptor.IsPrimitiveArray())
            return v.SetPrimitiveArrayInfo (typeDescriptor.GetPrimitiveType(), arrayCount, isFixedArrayCount);
        else if (typeDescriptor.IsStructArray())
            return v.SetStructArrayInfo (arrayCount, isFixedArrayCount);
        }
        
    POSTCONDITION (false && "Can not obtain value from memory using the specified property layout because it is an unsupported datatype", ERROR);        
    }
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Adam.Klatzkin                   01/2010
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt       MemoryInstanceSupport::GetValueFromMemory (ECValueR v, PropertyLayoutCR propertyLayout, UInt32 index) const
    {
    ECTypeDescriptor typeDescriptor = propertyLayout.GetTypeDescriptor();
    PRECONDITION (typeDescriptor.IsArray() && 
        "Can not obtain value from memory at an array index using the specified property layout because it is not an array datatype", 
        ECOBJECTS_STATUS_PreconditionViolated);    
           
    UInt32 arrayCount = GetReservedArrayCount (propertyLayout);                      
    if (index >= arrayCount)
        return ERROR; // WIP_FUSION ERROR_InvalidIndex                

    if (typeDescriptor.IsPrimitiveArray())
        return GetPrimitiveValueFromMemory (v, propertyLayout, true, index);
    else if (typeDescriptor.IsStructArray())
        return _GetStructArrayValueFromMemory (v, propertyLayout, index);       
        
    POSTCONDITION (false && "Can not obtain value from memory using the specified property layout because it is an unsupported datatype", ERROR);        
    }    
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    CaseyMullen     09/09
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt       MemoryInstanceSupport::GetValueFromMemory (ClassLayoutCR classLayout, ECValueR v, const wchar_t * propertyAccessString, bool useIndex, UInt32 index) const
    {
    PRECONDITION (NULL != propertyAccessString, ECOBJECTS_STATUS_PreconditionViolated);
                
    PropertyLayoutCP propertyLayout = NULL;
    StatusInt status = classLayout.GetPropertyLayout (propertyLayout, propertyAccessString);
    if (SUCCESS != status || NULL == propertyLayout)
        return ERROR; // WIP_FUSION ERROR_PropertyNotFound        

    if (useIndex)
        return GetValueFromMemory (v, *propertyLayout, index);
    else
        return GetValueFromMemory (v, *propertyLayout);
    }    

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    CaseyMullen     09/09
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt       MemoryInstanceSupport::SetPrimitiveValueToMemory (ECValueCR v, ClassLayoutCR classLayout, PropertyLayoutCR propertyLayout, bool useIndex, UInt32 index)
    {        
    bool isInUninitializedFixedCountArray = ((useIndex) && (propertyLayout.GetModifierFlags() & ARRAYMODIFIERFLAGS_IsFixedCount) && (GetAllocatedArrayCount (propertyLayout) == 0));
            
    if (v.IsNull())
        {
        if (!isInUninitializedFixedCountArray)
            SetPropertyValueNull (propertyLayout, useIndex, index, true);
        return SUCCESS;
        }   
             
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
            Int32 value = v.GetInteger();
            // WIP_FUSION: would it speed things up to poke directly when m_allowWritingDirectlyToInstanceMemory is true?
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
            if (useIndex)
                status = EnsureSpaceIsAvailableForArrayIndexValue (classLayout, propertyLayout, index, bytesNeeded);
            else
                status = EnsureSpaceIsAvailable (offset, classLayout, propertyLayout, bytesNeeded);
            if (SUCCESS != status)
                return status;
                
            // WIP_FUSION: would it speed things up to poke directly when m_allowWritingDirectlyToInstanceMemory is true?
            return _ModifyData (offset, value, bytesNeeded);
            }
        case PRIMITIVETYPE_Binary:
            {
            size_t size;
            byte const * data = v.GetBinary (size);
            UInt32 bytesNeeded = (UInt32)size;

            StatusInt status;
            if (useIndex)
                status = EnsureSpaceIsAvailableForArrayIndexValue (classLayout, propertyLayout, index, bytesNeeded);
            else
                status = EnsureSpaceIsAvailable (offset, classLayout, propertyLayout, bytesNeeded);
            if (SUCCESS != status)
                return status;
                
            // WIP_FUSION We need to figure out the story for value size.  It is legitamate to have a non-null binary value that is
            // 0 bytes in length.  Currently, we do not track that length anywhere.  How do we capture this in the case that the binary value was previously set to some
            // value > 0 in length and we are not auto compressing property values.
            if (bytesNeeded == 0)
                return SUCCESS;
            
            // WIP_FUSION: would it speed things up to poke directly when m_allowWritingDirectlyToInstanceMemory is true?    
            return _ModifyData (offset, data, bytesNeeded);
            }
        case PRIMITIVETYPE_Boolean:
            {
            bool value = v.GetBoolean();
            return _ModifyData (offset, &value, sizeof(value));
            }       
        case PRIMITIVETYPE_Point2D:
            {
            DPoint2d value = v.GetPoint2D();
            return _ModifyData (offset, &value, sizeof(value));
            }       
        case PRIMITIVETYPE_Point3D:
            {
            DPoint3d value = v.GetPoint3D();
            return _ModifyData (offset, &value, sizeof(value));
            } 
        case PRIMITIVETYPE_DateTime:      // stored as long
            {
            Int64 value = v.GetDateTimeTicks();
            return _ModifyData (offset, &value, sizeof(value));
            }
        }

    POSTCONDITION (false && "datatype not implemented", ERROR);
    }                
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Adam.Klatzkin                   01/2010
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt       MemoryInstanceSupport::SetValueToMemory (ECValueCR v, ClassLayoutCR classLayout, PropertyLayoutCR propertyLayout)
    {
    ECTypeDescriptor typeDescriptor = propertyLayout.GetTypeDescriptor();
    if (typeDescriptor.IsPrimitive())
        return SetPrimitiveValueToMemory (v, classLayout, propertyLayout, false, 0);

    POSTCONDITION (false && "Can not set the value to memory using the specified property layout because it is an unsupported datatype", ERROR);
    }      
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Adam.Klatzkin                   01/2010
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt       MemoryInstanceSupport::SetValueToMemory (ECValueCR v, ClassLayoutCR classLayout, PropertyLayoutCR propertyLayout, UInt32 index)
    {   
    ECTypeDescriptor typeDescriptor = propertyLayout.GetTypeDescriptor();
    PRECONDITION (typeDescriptor.IsArray() && 
        "Can not set the value to memory at an array index using the specified property layout because it is not an array datatype", 
        ECOBJECTS_STATUS_PreconditionViolated);  
                        
    if (index >= GetReservedArrayCount (propertyLayout))
        return ERROR; // WIP_FUSION ERROR_InvalidIndex

    if (typeDescriptor.IsPrimitiveArray())
        return SetPrimitiveValueToMemory (v, classLayout, propertyLayout, true, index);
    else if (typeDescriptor.IsStructArray()) // WIP_FUSION ensure that the struct is valid to set as an index in this array.              
        return _SetStructArrayValueToMemory (v, classLayout, propertyLayout, index);       

    POSTCONDITION (false && "Can not set the value to memory using the specified property layout because it is an unsupported datatype", ERROR);
    }     
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    CaseyMullen     09/09
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt       MemoryInstanceSupport::SetValueToMemory (ClassLayoutCR classLayout, const wchar_t * propertyAccessString, ECValueCR v, bool useIndex, UInt32 index)
    {
    PRECONDITION (NULL != propertyAccessString, ECOBJECTS_STATUS_PreconditionViolated);
                
    PropertyLayoutCP propertyLayout = NULL;
    StatusInt status = classLayout.GetPropertyLayout (propertyLayout, propertyAccessString);
    if (SUCCESS != status || NULL == propertyLayout)
        return ERROR; // WIP_FUSION ERROR_PropertyNotFound        

    if (useIndex)
        return SetValueToMemory (v, classLayout, *propertyLayout, index);
    else
        return SetValueToMemory (v, classLayout, *propertyLayout);
    }     

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    CaseyMullen     10/09
+---------------+---------------+---------------+---------------+---------------+------*/
void            MemoryInstanceSupport::DumpInstanceData (ClassLayoutCR classLayout) const
    {
    static bool s_skipDump = false;
    static int s_dumpCount = 0;
    s_dumpCount++;
    
    byte const * data = _GetData();
    Bentley::NativeLogging::ILogger *logger = Logger::GetLogger();
    
    logger->tracev (L"======================= Dump #%d ===================================\n", s_dumpCount);
    if (s_skipDump)
        return;
  
    logger->tracev (L"ECClass=%s at address = 0x%0x\n", classLayout.GetECClassName().c_str(), data);
    InstanceHeader& header = *(InstanceHeader*)data;

    logger->tracev (L"  [0x%0x][%4.d] SchemaIndex = %d\n",        &header.m_schemaIndex,  (byte*)&header.m_schemaIndex   - data, header.m_schemaIndex);
    logger->tracev (L"  [0x%0x][%4.d] ClassIndex  = %d\n",        &header.m_classIndex,   (byte*)&header.m_classIndex    - data, header.m_classIndex);
    logger->tracev (L"  [0x%0x][%4.d] InstanceFlags = 0x%08.x\n", &header.m_instanceFlags,(byte*)&header.m_instanceFlags - data, header.m_instanceFlags);
    
    UInt32 nProperties = classLayout.GetPropertyCount ();
    UInt32 nNullflagsBitmasks = CalculateNumberNullFlagsBitmasks (nProperties);
    
    for (UInt32 i = 0; i < nNullflagsBitmasks; i++)
        {
        UInt32 offset = sizeof(InstanceHeader) + i * sizeof(NullflagsBitmask);
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
        GetValueFromMemory (v, *propertyLayout);
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
                    offset = GetOffsetOfArrayIndex (GetOffsetOfPropertyValue (*propertyLayout), *propertyLayout, i);
                    address = data + offset;
                    status = GetValueFromMemory (v, *propertyLayout, i);
                    if (SUCCESS == status)            
                        valueAsString = v.ToString();                
                    else
                        {
                        wchar_t temp[1024];
                        swprintf(temp, 1024, L"Error (%d) returned while obtaining array index value", status, i);
                        valueAsString = std::wstring (temp);
                        }

                    if (IsArrayOfFixedSizeElements (*propertyLayout))
                        logger->tracev (L"      [0x%x][%4.d] %d = %s\n", address, offset, i, valueAsString.c_str());
                    else
                        {
                        SecondaryOffset secondaryOffset = GetOffsetOfArrayIndexValue (GetOffsetOfPropertyValue (*propertyLayout), *propertyLayout, i);
                        byte const * realAddress = data + secondaryOffset;
                        
                        logger->tracev (L"      [0x%x][%4.d] -> [0x%x][%4.d] %d = %s\n", address, offset, realAddress, secondaryOffset, i, valueAsString.c_str());                    
                        }     
                    if ((SUCCESS == status) && (!v.IsNull()) && (v.IsStruct()))
                        {
                        v.GetStruct()->Dump();
                        logger->tracev (L"=================== END Struct Instance ===========================\n");
                        }         
                    }
                }
            }
        }
        
    UInt32 offsetOfLast = classLayout.GetSizeOfFixedSection() - sizeof(SecondaryOffset);
    SecondaryOffset * pLast = (SecondaryOffset*)(data + offsetOfLast);
    logger->tracev (L"  [0x%x][%4.d] Offset of TheEnd = %d\n", pLast, offsetOfLast, *pLast);
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
        m_elementSizeInFixedSection = ClassLayout::GetFixedPrimitiveValueSize (m_elementType);
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
    
    m_resizeByteCount = m_postFixedSectionByteCount - m_preFixedSectionByteCount;    
    }    

    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Adam.Klatzkin                   02/2010
+---------------+---------------+---------------+---------------+---------------+------*/    
StatusInt            ArrayResizer::ShiftDataFollowingResizeIndex ()
    {
    // shift all data (fixed & variable) to the right of the element where the resixe should occur
    UInt32 offsetOfResizePoint = m_preHeaderByteCount + (m_resizeIndex * m_elementSizeInFixedSection);         
    m_pResizeIndexPreShift = m_data + m_arrayOffset + offsetOfResizePoint;
    m_pResizeIndexPostShift = m_pResizeIndexPreShift + m_resizeByteCount;        
    #ifndef NDEBUG
    UInt32 arrayByteCountAfterGrow = m_instance.GetPropertyValueSize (m_propertyLayout);
    byte const * pNextProperty = m_data + m_arrayOffset + arrayByteCountAfterGrow;
    DEBUG_EXPECT (pNextProperty == m_data + *((SecondaryOffset*)(m_data + m_propertyLayout.GetOffset()) + 1));
    #endif    
    if (m_preArrayByteCount > offsetOfResizePoint)
        {        
        UInt32 byteCountToShift = m_preArrayByteCount - offsetOfResizePoint;        
        DEBUG_EXPECT (m_pResizeIndexPostShift + byteCountToShift <= pNextProperty); 
        memmove ((byte*)m_pResizeIndexPostShift, m_pResizeIndexPreShift, byteCountToShift); // WIP_FUSION .. use _MoveData, waiting on Keith
        }
        
    return SetSecondaryOffsetsFollowingResizeIndex();        
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Adam.Klatzkin                   02/2010
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt            ArrayResizer::SetSecondaryOffsetsFollowingResizeIndex ()
    {
    // initialize the inserted secondary offsets and update all shifted secondary offsets         
    StatusInt status = SUCCESS;
    
    if (m_elementTypeIsFixedSize)
        return status;  // There are no secondary offsets to update
        
    byte * pWriteBuffer;              
    UInt32 sizeOfWriteBuffer = 0;                
    UInt32 nSecondaryOffsetsShifted;        
    if (m_preAllocatedArrayCount > m_resizeIndex)
        {
        nSecondaryOffsetsShifted = m_preAllocatedArrayCount - m_resizeIndex;
        m_postSecondaryOffsetOfResizeIndex = *((SecondaryOffset*)m_pResizeIndexPostShift) + m_resizeByteCount;
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
        pSecondaryOffsetWriteBuffer[i] = pSecondaryOffset[i] + m_resizeByteCount;
        DEBUG_EXPECT (pSecondaryOffsetWriteBuffer[i] <= m_instance.GetPropertyValueSize (m_propertyLayout));
        }
        
    if (!m_instance.m_allowWritingDirectlyToInstanceMemory)
        {
        UInt32 modifyOffset = (UInt32)(m_pResizeIndexPostShift - m_data) - insertedSecondaryOffsetByteCount;
        status = m_instance._ModifyData (modifyOffset, pWriteBuffer, sizeOfWriteBuffer);            
        DEBUG_EXPECT (m_data + modifyOffset + sizeOfWriteBuffer <= m_data + m_arrayOffset + m_instance.GetPropertyValueSize (m_propertyLayout));
        if (SUCCESS != status)
            return status;        
        }            
        
    return status;    
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Adam.Klatzkin                   02/2010
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt            ArrayResizer::ShiftDataPreceedingResizeIndex ()    
    {    
    StatusInt status = SUCCESS;
    if (m_resizeIndex == 0)
        return status;
    
    byte const * pShiftFrom = m_data + m_arrayOffset + m_preHeaderByteCount;
    byte * pShiftTo = (byte*)(m_data + m_arrayOffset + m_postHeaderByteCount);
    UInt32 byteCountToShift = (UInt32)(m_pResizeIndexPreShift - pShiftFrom);                
    
    // shift all the elements in the fixed section preceding the insert point if we needed to grow the nullflags bitmask            
    if ((pShiftTo != pShiftFrom))
        {
        memmove (pShiftTo, pShiftFrom, byteCountToShift); // WIP_FUSION .. use _MoveData, waiting on Keith
        DEBUG_EXPECT (pShiftTo + byteCountToShift <= m_pResizeIndexPostShift); 
        }
        
    return SetSecondaryOffsetsPreceedingResizeIndex((SecondaryOffset*)pShiftTo, byteCountToShift);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Adam.Klatzkin                   02/2010
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt            ArrayResizer::SetSecondaryOffsetsPreceedingResizeIndex (SecondaryOffset* pSecondaryOffset, UInt32 byteCountToSet)            
    {
    // update all secondary offsets preceeding the insertion point since the size of the fixed section has changed and therefore all variable data has moved
    StatusInt status = SUCCESS;
    
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
        pSecondaryOffsetWriteBuffer[i] = pSecondaryOffset[i] + m_resizeByteCount;
        DEBUG_EXPECT (pSecondaryOffsetWriteBuffer[i] <= m_postSecondaryOffsetOfResizeIndex);
        }
        
    if (!m_instance.m_allowWritingDirectlyToInstanceMemory)
        {
        UInt32 modifyOffset = (UInt32)((byte*)pSecondaryOffset - m_data);
        status = m_instance._ModifyData (modifyOffset, pWriteBuffer, byteCountToSet);
        DEBUG_EXPECT (modifyOffset >= m_postHeaderByteCount);
        DEBUG_EXPECT (m_data + modifyOffset + byteCountToSet <= m_pResizeIndexPostShift);
        if (SUCCESS != status)
            return status;        
        }               
        
    return status;     
    }      

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Adam.Klatzkin                   02/2010
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt            ArrayResizer::WriteArrayHeader ()                
    {
    StatusInt status = SUCCESS;
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
        if (SUCCESS != status)
            return status;        
        }                       
        
    return status;
    }
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Adam.Klatzkin                   01/2010
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt       ArrayResizer::CreateNullArrayElementsAt (ClassLayoutCR classLayout, PropertyLayoutCR propertyLayout, MemoryInstanceSupportR instance, UInt32 insertIndex, UInt32 insertCount)
    {                        
    ArrayResizer resizer (classLayout, propertyLayout, instance, insertIndex, insertCount);
    PRECONDITION (resizer.m_resizeIndex <= resizer.m_preAllocatedArrayCount, ERROR);        
    
    StatusInt status = instance.EnsureSpaceIsAvailable (resizer.m_arrayOffset, classLayout, propertyLayout, resizer.m_preArrayByteCount + resizer.m_resizeByteCount);
    if (SUCCESS != status)
        return status;       
        
    resizer.m_data = resizer.m_instance._GetData();
    status = resizer.ShiftDataFollowingResizeIndex();        
    if (SUCCESS != status)
        return status;
        
    status = resizer.ShiftDataPreceedingResizeIndex();        
    if (SUCCESS != status)
        return status;        
    
    status = resizer.WriteArrayHeader();

    return status;    
    // WIP_FUSION how do we deal with an error that occurs during the insert "transaction" after some data has already been moved/modified but we haven't finished?
    }    
    
END_BENTLEY_EC_NAMESPACE