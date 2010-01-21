/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicApi/ECObjects/MemoryInstanceSupport.h $
|
|   $Copyright: (c) 2010 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
/*__PUBLISH_SECTION_START__*/
#pragma once

#include <ECObjects\ECObjects.h>
#include <hash_map>

#define N_FINAL_STRING_PROPS_IN_FAKE_CLASS 48

EC_TYPEDEFS(MemoryInstanceSupport);

BEGIN_BENTLEY_EC_NAMESPACE
    
typedef UInt16 ClassIndex;
typedef UInt16 SchemaIndex;

typedef UInt32 NullflagsBitmask;
typedef UInt32 InstanceFlags;
typedef UInt32 SecondaryOffset;
typedef UInt32 ArrayCount;

#define NULLFLAGS_BITMASK_AllOn     0xFFFFFFFF

struct  InstanceHeader
    {
    SchemaIndex     m_schemaIndex;
    ClassIndex      m_classIndex;

    InstanceFlags   m_instanceFlags;
    };


/// WIP, not sure this belongs here or that we even need the struct but I'm defining it for now just to experiment with some ideas.
struct ECTypeDescriptor
    {
private:
    ValueKind       m_typeKind;

    union
        {
        ArrayKind       m_arrayKind;
        PrimitiveType   m_primitiveType;
        };  
    ECTypeDescriptor () : m_typeKind ((ValueKind) 0), m_primitiveType ((PrimitiveType) 0) { };

public:
    static ECTypeDescriptor   CreatePrimitiveTypeDescriptor (PrimitiveType primitiveType) 
        { ECTypeDescriptor type; type.m_typeKind = VALUEKIND_Primitive; type.m_primitiveType = primitiveType; return type; }
    static ECTypeDescriptor   CreatePrimitiveArrayTypeDescriptor (PrimitiveType primitiveType) 
        { ECTypeDescriptor type; type.m_typeKind = VALUEKIND_Array; type.m_primitiveType = primitiveType; return type; }
    static ECTypeDescriptor   CreateStructArrayTypeDescriptor () 
        { ECTypeDescriptor type; type.m_typeKind = VALUEKIND_Array; type.m_arrayKind = ARRAYKIND_Struct; return type; }
    static ECTypeDescriptor   CreateStructTypeDescriptor () 
        { ECTypeDescriptor type; type.m_typeKind = VALUEKIND_Struct; type.m_arrayKind = (ArrayKind)0; return type; }

    ECTypeDescriptor (PrimitiveType primitiveType) : m_typeKind (VALUEKIND_Primitive), m_primitiveType (primitiveType) { };

    inline ValueKind        GetTypeKind() const         { return m_typeKind; }
    inline ArrayKind        GetArrayKind() const        { return (ArrayKind)(m_arrayKind & 0xFF); }    
    inline bool             IsPrimitive() const         { return (GetTypeKind() == VALUEKIND_Primitive ); }
    inline bool             IsStruct() const            { return (GetTypeKind() == VALUEKIND_Struct ); }
    inline bool             IsArray() const             { return (GetTypeKind() == VALUEKIND_Array ); }
    inline bool             IsPrimitiveArray() const    { return (GetTypeKind() == VALUEKIND_Array ) && (GetArrayKind() == ARRAYKIND_Primitive); }
    inline bool             IsStructArray() const       { return (GetTypeKind() == VALUEKIND_Array ) && (GetArrayKind() == ARRAYKIND_Struct); }
    inline PrimitiveType    GetPrimitiveType() const    { return m_primitiveType; }
    };

enum ArrayModifierFlags : UInt32
    {
    ARRAYMODIFIERFLAGS_IsFixedSize    = 0x01
    };

/*=================================================================================**//**
* @bsistruct                                                     CaseyMullen    10/09
+===============+===============+===============+===============+===============+======*/      
struct PropertyLayout
    {
friend ClassLayout;    
private:
    std::wstring            m_accessString;        
    ECTypeDescriptor        m_typeDescriptor;
    
    // Using UInt32 instead of size_t below because we will persist this struct in an XAttribute. It will never be very big.
    UInt32                  m_offset; //! An offset to either the data holding that property’s value (for fixed-size values) or to the offset at which the properties value can be found.
    UInt32                  m_modifierFlags; //! Can be used to indicate that a string should be treated as fixed size, with a max length, or that a longer fixed size type should be treated as an optional variable-sized type, or that for a string that only an entry to a StringTable is Stored, or that a default value should be used.
    UInt32                  m_modifierData;  //! Data used with the modifier flag, like the length of a fixed-sized string.
    UInt32                  m_nullflagsOffset;
    NullflagsBitmask        m_nullflagsBitmask;
  //ECPropertyCP            m_property; // WIP_FUSION: optional? YAGNI?

    // transient data
    int                     m_expectedIndices; // we can calculate this from access string on construction
    
public:
    PropertyLayout (wchar_t const * accessString, ECTypeDescriptor typeDescriptor, UInt32 offset, UInt32 nullflagsOffset, UInt32 nullflagsBitmask, UInt32 modifierFlags = 0, UInt32 modifierData = 0) : //, ECPropertyCP property) :
        m_accessString(accessString), m_typeDescriptor(typeDescriptor), m_offset(offset), m_nullflagsOffset(nullflagsOffset), 
        m_nullflagsBitmask (nullflagsBitmask), m_modifierFlags (modifierFlags), m_modifierData (modifierData) 
        { m_expectedIndices = IECInstance::ParseExpectedNIndices(accessString); }; //, m_property(property) {};

    inline UInt32           GetOffset() const           { return m_offset; }
    inline UInt32           GetNullflagsOffset() const  { return m_nullflagsOffset; }
    inline NullflagsBitmask GetNullflagsBitmask() const { return m_nullflagsBitmask; }
    inline ECTypeDescriptor GetTypeDescriptor() const   { return m_typeDescriptor; }
    inline wchar_t const *  GetAccessString() const     { return m_accessString.c_str(); }
    inline int              GetExpectedIndices() const  { return m_expectedIndices; }
    inline UInt32           GetModifierFlags() const    { return m_modifierFlags; }
    inline UInt32           GetModifierData() const     { return m_modifierData; }
    
    bool                    IsFixedSized() const;
    //! Gets the size required for this PropertyValue in the fixed Section of the IECInstance's memory
    //! Variable-sized types will have 4 byte SecondaryOffset stored in the fixed Section.
    UInt32                  GetSizeInFixedSection() const;
    
    std::wstring            ToString();
    };

#define USE_HASHMAP_IN_CLASSLAYOUT    
/*=================================================================================**//**
* @bsistruct                                                     CaseyMullen    10/09
+===============+===============+===============+===============+===============+======*/      
struct ClassLayout
    {
    friend MemoryInstanceSupport;
private:
    struct StringComparer {bool operator()(wchar_t const * s1, wchar_t const * s2) const   {return wcscmp (s1, s2) < 0;}};
#ifdef USE_HASHMAP_IN_CLASSLAYOUT    
    typedef stdext::hash_map<wchar_t const *, PropertyLayoutCP, stdext::hash_compare<const wchar_t *, StringComparer>>   PropertyLayoutLookup;
#else
    typedef std::map<wchar_t const *, PropertyLayoutCP, StringComparer> PropertyLayoutLookup;
#endif    
    typedef std::vector<PropertyLayout>                                 PropertyLayoutVector;
    
    enum State
        {
        AcceptingFixedSizeProperties,
        AcceptingVariableSizeProperties,
        Closed
        };
    
    // These members are expected to be persisted  
    ClassIndex              m_classIndex; // Unique per some context, e.g. per DgnFile
    std::wstring            m_className;
    UInt32                  m_nProperties;
    
    PropertyLayoutVector    m_propertyLayouts; // This is the primary collection, there is a secondary map for lookup by name, below.
    PropertyLayoutLookup    m_propertyLayoutLookup;
    
    // These members are transient
    SchemaIndex             m_schemaIndex;
    UInt32                  m_nullflagsOffset;
    State                   m_state;
    UInt32                  m_offset;
    UInt32                  m_sizeOfFixedSection;
    
    void                    AddProperties (ECClassCR ecClass, wchar_t const * nameRoot, bool addFixedSize);
    StatusInt               AddProperty (wchar_t const * accessString, ECTypeDescriptor propertyDescriptor, UInt32 size, UInt32 modifierFlags = 0, UInt32 modifierData = 0);
    StatusInt               AddFixedSizeProperty (wchar_t const * accessString, ECTypeDescriptor propertyDescriptor);
    StatusInt               AddFixedSizeArrayProperty (wchar_t const * accessString, ECTypeDescriptor propertyDescriptor, UInt32 arrayCount);
    StatusInt               AddVariableSizeProperty (wchar_t const * accessString, ECTypeDescriptor propertyDescriptor);

    BentleyStatus           SetClass (ECClassCR ecClass, UInt16 classIndex);

    ClassLayout(SchemaIndex schemaIndex);

public:
    ECOBJECTS_EXPORT static ClassLayoutP BuildFromClass (ECClassCR ecClass, ClassIndex classIndex, SchemaIndex schemaIndex);
    ECOBJECTS_EXPORT static ClassLayoutP CreateEmpty    (ECClassCR ecClass, ClassIndex classIndex, SchemaIndex schemaIndex);

    ECOBJECTS_EXPORT std::wstring   GetClassName() const;
    ECOBJECTS_EXPORT ClassIndex     GetClassIndex() const;
    ECOBJECTS_EXPORT SchemaIndex    GetSchemaIndex () const;
    ECOBJECTS_EXPORT UInt32         GetPropertyCount () const;
    ECOBJECTS_EXPORT StatusInt      GetPropertyLayout (PropertyLayoutCP & propertyLayout, wchar_t const * accessString) const;
    ECOBJECTS_EXPORT StatusInt      GetPropertyLayoutByIndex (PropertyLayoutCP & propertyLayout, UInt32 propertyIndex) const;
    // WIP_FUSION add StatusInt      GetPropertyIndex (UInt32& propertyIndex, wchar_t const * accessString);
    
/*__PUBLISH_SECTION_END__*/
    ECOBJECTS_EXPORT void           AddPropertyDirect (wchar_t const * accessString, PrimitiveType primitivetype, UInt32 offset, UInt32 nullflagsOffset, UInt32 nullflagsBitmask);
    ECOBJECTS_EXPORT StatusInt      FinishLayout ();
/*__PUBLISH_SECTION_START__*/

    void                        Dump() const;
    
    void                        InitializeMemoryForInstance(byte * data, UInt32 bytesAllocated) const;
    
    static UInt32               GetFixedPrimitiveValueSize (PrimitiveType primitiveType); // WIP_FUSION: move to ecvalue.h
    UInt32                      GetSizeOfFixedSection() const;
    
    //! Determines the number of bytes used, so far
    ECOBJECTS_EXPORT UInt32     CalculateBytesUsed(byte const * data) const;
    };
    
/*=================================================================================**//**
* @bsistruct
+===============+===============+===============+===============+===============+======*/      
struct SchemaLayoutEntry
    {
    bool                m_persistent;
    ClassLayoutCR       m_classLayout;

    SchemaLayoutEntry (ClassLayoutCR l, bool p) : m_classLayout(l), m_persistent(p) { }
    };

typedef std::vector<SchemaLayoutEntry*>  SchemaLayoutEntryArray;

/*=================================================================================**//**
* @bsistruct
+===============+===============+===============+===============+===============+======*/      
struct SchemaLayout
{
private:
    SchemaIndex             m_schemaIndex;
    SchemaLayoutEntryArray  m_entries;

public:
    ECOBJECTS_EXPORT SchemaLayout(SchemaIndex index) : m_schemaIndex(index) {}

    // NEEDSWORK: remove these
    ECOBJECTS_EXPORT SchemaLayout() : m_schemaIndex(0) {}
    ECOBJECTS_EXPORT void SetSchemaIndex(SchemaIndex i) {m_schemaIndex = i;}


    ECOBJECTS_EXPORT SchemaIndex            GetSchemaIndex() const { return m_schemaIndex; }
    ECOBJECTS_EXPORT BentleyStatus          AddClassLayout (ClassLayoutCR, ClassIndex, bool isPersistent);
    ECOBJECTS_EXPORT SchemaLayoutEntry*     GetEntry (ClassIndex classIndex);
    ECOBJECTS_EXPORT SchemaLayoutEntry*     FindEntry (wchar_t const * className);
    ECOBJECTS_EXPORT BentleyStatus          FindAvailableClassIndex (ClassIndex&);
};

//! Holds a ClassLayoutCR and provides a public method by which to access it.
//! Used by StandaloneECEnabler and ECXDataEnabler
struct ClassLayoutHolder
    {
private:
    ClassLayoutCR                   m_classLayout;
        
protected:
    ECOBJECTS_EXPORT                ClassLayoutHolder (ClassLayoutCR classLayout);

public:    
    ECOBJECTS_EXPORT ClassLayoutCR  GetClassLayout() const;
    };

//! Base class for EC::IECInstance implementations that get/set values from a block of memory, 
//! e.g. StandaloneECInstance and ECXDataInstance
struct MemoryInstanceSupport
    {
private:    
    byte const *                GetAddressOfPrimitiveValue (PropertyLayoutCR propertyLayout, UInt32 nIndices, UInt32 const * indices) const;
    UInt32                      GetOffsetOfPropertyValue (PropertyLayoutCR propertyLayout) const;
    UInt32                      GetOffsetOfPrimitiveValue (PropertyLayoutCR propertyLayout, UInt32 nIndices, UInt32 const * indices) const;
    UInt32                      GetOffsetOfArrayIndex (PropertyLayoutCR propertyLayout, UInt32 index) const;
    UInt32                      GetOffsetOfArrayIndexValue (PropertyLayoutCR propertyLayout, UInt32 index) const;
    bool                        IsPrimitiveValueNull (PropertyLayoutCR propertyLayout, UInt32 nIndices, UInt32 const * indices) const;
    void                        SetPrimitiveValueNull (PropertyLayoutCR propertyLayout, UInt32 nIndices, UInt32 const * indices, bool isNull);

    UInt32                      GetArrayOffsetAndCount (UInt32& arrayOffset, PropertyLayoutCR propertyLayout) const;
    StatusInt                   GetPrimitiveValueFromMemory (ECValueR v, PropertyLayoutCR propertyLayout, UInt32 nIndices, UInt32 const * indices) const;
    StatusInt                   SetPrimitiveValueToMemory   (ECValueCR v, ClassLayoutCR classLayout, PropertyLayoutCR propertyLayout, UInt32 nIndices, UInt32 const * indices);
    
    //! Shifts the values' data and adjusts SecondaryOffsets for all variable-sized property values 
    //! AFTER the given one, to make room for additional bytes needed for the property value of the given PropertyLayout
    //! or to "compact" to reclaim unused space.
    //! @param data           Start of the data of the MemoryInstanceSupport
    //! @param bytesAllocated How much memory is allocated for the data
    //! @param propertyLayout PropertyLayout of the variable-sized property whose size is increasing
    //! @param shiftBy        Positive or negative! Memory will be moved and SecondaryOffsets will be adjusted by this amount
    StatusInt                   ShiftValueData(ClassLayoutCR classLayout, byte * data, UInt32 bytesAllocated, PropertyLayoutCR propertyLayout, Int32 shiftBy);
    StatusInt                   ShiftArrayIndexValueData(PropertyLayoutCR propertyLayout, UInt32 arrayIndex, UInt32 arrayCount, Int32 shiftBy);
        
    StatusInt                   EnsureSpaceIsAvailable (UInt32& offset, ClassLayoutCR classLayout, PropertyLayoutCR propertyLayout, UInt32 bytesNeeded);
    StatusInt                   EnsureSpaceIsAvailableForArrayIndexValue (ClassLayoutCR classLayout, PropertyLayoutCR propertyLayout, UInt32 arrayIndex, UInt32 bytesNeeded);
    StatusInt                   GrowPropertyValue (ClassLayoutCR classLayout, PropertyLayoutCR propertyLayout, UInt32 additionalbytesNeeded);
         
protected:
    ECOBJECTS_EXPORT void       InitializeMemory(ClassLayoutCR classLayout, byte * data, UInt32 bytesAllocated) const;
    ECOBJECTS_EXPORT StatusInt  GetValueFromMemory (ECValueR v, PropertyLayoutCR propertyLayout,      UInt32 nIndices, UInt32 const * indices) const;
    ECOBJECTS_EXPORT StatusInt  GetValueFromMemory (ClassLayoutCR classLayout, ECValueR v, const wchar_t * propertyAccessString, UInt32 nIndices, UInt32 const * indices) const;
    ECOBJECTS_EXPORT StatusInt  SetValueToMemory (ClassLayoutCR classLayout, const wchar_t * propertyAccessString, ECValueCR v,  UInt32 nIndices, UInt32 const * indices);      
    ECOBJECTS_EXPORT void       DumpInstanceData (ClassLayoutCR classLayout) const;
    
    virtual bool                _IsMemoryInitialized () const = 0;    
    //! Get a pointer to the first byte of the data    
    virtual byte const *        _GetDataForRead () const = 0;
    virtual byte *              _GetDataForWrite () const = 0;
    virtual StatusInt           _ModifyData (UInt32 offset, void const * newData, UInt32 dataLength) = 0;
    virtual UInt32              _GetBytesAllocated () const = 0;
        
    //! Reallocates memory for the IECInstance and copies the old IECInstance data into the new memory
    //! You might get more memory than used asked for, but you won't get less
    //! @param additionalBytesNeeded  Additional bytes of memory needed above current allocation
    virtual StatusInt           _GrowAllocation (UInt32 additionalBytesNeeded) = 0;
    
    //! Reallocates memory for the IECInstance and copies the old IECInstance data into the new memory
    //! This is not guaranteed to do anything or to change to precisely the allocation you request
    //! but it will be at least as large as you request.
    //! @param newAllocation  Additional bytes of memory needed above current allocation    
    virtual void                _ShrinkAllocation (UInt32 newAllocation) = 0;
    
    //! Free any allocated memory
    virtual void                _FreeAllocation () = 0;

public:
    ECOBJECTS_EXPORT static void                    SetShiftSecondaryOffsetsInPlace (bool inPlace);
    ECOBJECTS_EXPORT static InstanceHeader const&   PeekInstanceHeader (void const* data);
    };


END_BENTLEY_EC_NAMESPACE