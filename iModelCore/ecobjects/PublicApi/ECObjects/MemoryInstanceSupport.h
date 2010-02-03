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
#define PROPERTYLAYOUT_Source_ECPointer L"Source ECPointer"
#define PROPERTYLAYOUT_Target_ECPointer L"Target ECPointer"
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

enum ArrayModifierFlags : UInt32
    {
    ARRAYMODIFIERFLAGS_IsFixedCount    = 0x01
    };

/*=================================================================================**//**
* @bsistruct
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
* @bsistruct
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
    bool                    m_isPersisted;
    UInt32                  m_nullflagsOffset;
    State                   m_state;
    UInt32                  m_offset;
    UInt32                  m_sizeOfFixedSection;
    bool                    m_isRelationshipClass;
    int                     m_propertyIndexOfSourceECPointer;
    int                     m_propertyIndexOfTargetECPointer;
    
    void                    AddProperties (ECClassCR ecClass, wchar_t const * nameRoot, bool addFixedSize);
    StatusInt               AddProperty (wchar_t const * accessString, ECTypeDescriptor typeDescriptor, UInt32 size, UInt32 modifierFlags = 0, UInt32 modifierData = 0);
    StatusInt               AddFixedSizeProperty (wchar_t const * accessString, ECTypeDescriptor typeDescriptor);
    StatusInt               AddFixedSizeArrayProperty (wchar_t const * accessString, ECTypeDescriptor typeDescriptor, UInt32 arrayCount);
    StatusInt               AddVariableSizeProperty (wchar_t const * accessString, ECTypeDescriptor typeDescriptor);
    StatusInt               AddVariableSizeArrayPropertyWithFixedCount (wchar_t const * accessString, ECTypeDescriptor typeDescriptor, UInt32 arrayCount);

    BentleyStatus           SetClass (wchar_t const *  className, UInt16 classIndex);

    ClassLayout(SchemaIndex schemaIndex);

public:
    ECOBJECTS_EXPORT static ClassLayoutP BuildFromClass (ECClassCR ecClass, ClassIndex classIndex, SchemaIndex schemaIndex);
    ECOBJECTS_EXPORT static ClassLayoutP CreateEmpty    (wchar_t const *  className, ClassIndex classIndex, SchemaIndex schemaIndex);

    ECOBJECTS_EXPORT std::wstring   GetClassName() const;
    // These members are only meaningful in the context of a consumer like DgnHandlers.dll that actually handles persistence of ClassLayouts
    ECOBJECTS_EXPORT ClassIndex     GetClassIndex() const;
    ECOBJECTS_EXPORT SchemaIndex    GetSchemaIndex () const;
    ECOBJECTS_EXPORT bool           IsPersisted () const;
    ECOBJECTS_EXPORT void           SetIsPersisted (bool isPersisted) const;
    ECOBJECTS_EXPORT int            GetSourceECPointerIndex () const;
    ECOBJECTS_EXPORT int            GetTargetECPointerIndex () const;
    
    ECOBJECTS_EXPORT UInt32         GetPropertyCount () const;
    ECOBJECTS_EXPORT StatusInt      GetPropertyLayout (PropertyLayoutCP & propertyLayout, wchar_t const * accessString) const;
    ECOBJECTS_EXPORT StatusInt      GetPropertyLayoutByIndex (PropertyLayoutCP & propertyLayout, UInt32 propertyIndex) const;
    // WIP_FUSION add StatusInt      GetPropertyIndex (UInt32& propertyIndex, wchar_t const * accessString);
    
/*__PUBLISH_SECTION_END__*/
    ECOBJECTS_EXPORT void           AddPropertyDirect (wchar_t const * accessString, PrimitiveType primitivetype, UInt32 offset, UInt32 nullflagsOffset, UInt32 nullflagsBitmask);
    ECOBJECTS_EXPORT StatusInt      FinishLayout ();
/*__PUBLISH_SECTION_START__*/

    void                            Dump() const;
    
    void                            InitializeMemoryForInstance(byte * data, UInt32 bytesAllocated) const;
    
    static UInt32                   GetFixedPrimitiveValueSize (PrimitiveType primitiveType); // WIP_FUSION: move to ecvalue.h
    UInt32                          GetSizeOfFixedSection() const;
    
    //! Determines the number of bytes used, so far
    ECOBJECTS_EXPORT UInt32         CalculateBytesUsed(byte const * data) const;
    };

typedef std::vector<ClassLayoutCP>  ClassLayoutVector;

/*=================================================================================**//**
* @bsistruct
+===============+===============+===============+===============+===============+======*/      
struct SchemaLayout
{
private:
    SchemaIndex             m_schemaIndex;
    ClassLayoutVector       m_classLayouts;

public:
    ECOBJECTS_EXPORT SchemaLayout(SchemaIndex index) : m_schemaIndex(index) {}

    // NEEDSWORK: remove these
    //ECOBJECTS_EXPORT SchemaLayout() : m_schemaIndex(0) {}
    //ECOBJECTS_EXPORT void SetSchemaIndex(SchemaIndex i) {m_schemaIndex = i;}


    ECOBJECTS_EXPORT SchemaIndex            GetSchemaIndex() const { return m_schemaIndex; }
    ECOBJECTS_EXPORT BentleyStatus          AddClassLayout (ClassLayoutCR, ClassIndex, bool isPersistent);
    ECOBJECTS_EXPORT ClassLayoutCP          GetClassLayout (ClassIndex classIndex);
    ECOBJECTS_EXPORT ClassLayoutCP          FindClassLayout (wchar_t const * className);
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

/*__PUBLISH_SECTION_END__*/    
//! An internal helper used by MemoryInstanceSupport to resize (add/remove elements) array property values
struct      ArrayResizer
    {
    friend MemoryInstanceSupport;
    
private:
    ClassLayoutCR           m_classLayout;
    PropertyLayoutCR        m_propertyLayout;
    MemoryInstanceSupportR  m_instance;
    
    UInt32          m_arrayOffset;
    
    UInt32          m_resizeIndex;
    UInt32          m_resizeElementCount;
    UInt32          m_resizeByteCount; // bytesNeeded
        
    PrimitiveType   m_elementType;
    bool            m_elementTypeIsFixedSize;
    UInt32          m_elementSizeInFixedSection;
    
    // state variables for pre resize
    ArrayCount      m_preAllocatedArrayCount;
    UInt32          m_preNullFlagBitmasksCount;
    UInt32          m_preHeaderByteCount;
    UInt32          m_preFixedSectionByteCount;
    UInt32          m_preArrayByteCount;    
    
    // state variables for post resize
    ArrayCount      m_postAllocatedArrayCount;
    UInt32          m_postNullFlagBitmasksCount;
    UInt32          m_postHeaderByteCount;
    UInt32          m_postFixedSectionByteCount;    
    SecondaryOffset m_postSecondaryOffsetOfResizeIndex;
    
    // pointers to memory that must be shifted during resize
    byte const *    m_pResizeIndexPreShift;
    byte const *    m_pResizeIndexPostShift;    
    
    byte const *    m_data;

    ArrayResizer (ClassLayoutCR classLayout, PropertyLayoutCR propertyLayout, MemoryInstanceSupportR instance, UInt32 resizeIndex, UInt32 resizeElementCount);
        
    StatusInt       ShiftDataFollowingResizeIndex ();
    StatusInt       SetSecondaryOffsetsFollowingResizeIndex ();
    StatusInt       ShiftDataPreceedingResizeIndex ();
    StatusInt       SetSecondaryOffsetsPreceedingResizeIndex (SecondaryOffset* pSecondaryOffset, UInt32 byteCountToSet);    
    StatusInt       WriteArrayHeader ();
        
    static StatusInt    CreateNullArrayElementsAt (ClassLayoutCR classLayout, PropertyLayoutCR propertyLayout, MemoryInstanceSupportR instance, UInt32 insertIndex, UInt32 insertCount);
    
    };
/*__PUBLISH_SECTION_START__*/    

//! Base class for EC::IECInstance implementations that get/set values from a block of memory, 
//! e.g. StandaloneECInstance and ECXDataInstance
struct MemoryInstanceSupport
    {
/*__PUBLISH_SECTION_END__*/    
    friend  ArrayResizer;
/*__PUBLISH_SECTION_START__*/    
    
private:    
    bool                        m_allowWritingDirectlyToInstanceMemory;
	   
    //! Returns the offset of the property value relative to the start of the instance data.
    //! If nIndices is > 0 then the offset of the array element value is returned.
    UInt32              GetOffsetOfPropertyValue (PropertyLayoutCR propertyLayout, UInt32 nIndices = 0, UInt32 const * indices = NULL) const;
    //! Returns the size in bytes of the property value
    UInt32              GetPropertyValueSize (PropertyLayoutCR propertyLayout) const;
	//! Returns the address of the property value 
	//! If nIndices is > 0 then the address of the array element value is returned
    byte const *        GetAddressOfPropertyValue (PropertyLayoutCR propertyLayout, UInt32 nIndices = 0, UInt32 const * indices = NULL) const;    
    //! Returns the offset of the specified array index relative to the start of the instance data.
    //! Note that this offset does not necessarily contain the index value.  If the element type is fixed it contains the value but if it is a variable
    //! size type then the index contains a secondary offset.  If you need the offset of the actual element value then use GetOffsetOfArrayIndexValue
    //! This method does not do any parameter checking.  It is the responsibility of the caller to ensure the property is an array and the index is in
    //! a valid range.
    UInt32              GetOffsetOfArrayIndex (UInt32 arrayOffset, PropertyLayoutCR propertyLayout, UInt32 index) const;
    //! Returns the offset of the specified array index value relative to the start of the instance data.
    //! This method does not do any parameter checking.  It is the responsibility of the caller to ensure the property is an array and the index is in
    //! a valid range.
    UInt32              GetOffsetOfArrayIndexValue (UInt32 arrayOffset, PropertyLayoutCR propertyLayout, UInt32 index) const;
    //! Returns true if the property value is null; otherwise false
    //! If nIndices is > 0 then the null check is based on the array element at the specified index
    bool                IsPropertyValueNull (PropertyLayoutCR propertyLayout, UInt32 nIndices, UInt32 const * indices) const;
    //! Sets the null bit of the specified property to the value indicated by isNull
    //! If nIndices is > 0 then the null bit is set for the array element at the specified index    
    void                SetPropertyValueNull (PropertyLayoutCR propertyLayout, UInt32 nIndices, UInt32 const * indices, bool isNull);    
    //! Returns the number of elements in the specfieid array that are currently reserved but not necessarily allocated.
    //! This is important when an array has a minimum size but has not yet been initialized.  We delay initializing the memory for the minimum # of elements until
    //! the first value is set.  If an array does not have a minimum element count then GetReservedArrayCount will always equal GetAllocatedArrayCount
    //! This value is always >= the value returned by GetAllocatedArrayCount.
    //! This is the value used to set the count on an ArrayInfo value object that will be returned to a caller via GetValueFromMemory.  It is an implementation detail
    //! of memory based instances as to whether or not the physical memory to back that array count has actually been allocated.
    ArrayCount          GetReservedArrayCount (PropertyLayoutCR propertyLayout) const;
    //! Returns the number of elements in the specfieid array that are currently allocated in the instance data memory block.
    //! See the description of GetReservedArrayCount for explanation about the differences between the two.
    ArrayCount          GetAllocatedArrayCount (PropertyLayoutCR propertyLayout) const;
    //! Obtains the current primitive value for the specified property
    //! If nIndices is > 0 then the primitive value for the array element at the specified index is obtained.
    StatusInt           GetPrimitiveValueFromMemory (ECValueR v, PropertyLayoutCR propertyLayout, UInt32 nIndices, UInt32 const * indices) const;
    //! Sets the primitive value for the specified property
    //! If nIndices is > 0 then the primitive value for the array element at the specified index is set.
    StatusInt           SetPrimitiveValueToMemory   (ECValueCR v, ClassLayoutCR classLayout, PropertyLayoutCR propertyLayout, UInt32 nIndices, UInt32 const * indices);
    
    //! Shifts the values' data and adjusts SecondaryOffsets for all variable-sized property values 
    //! AFTER the given one, to make room for additional bytes needed for the property value of the given PropertyLayout
    //! or to "compact" to reclaim unused space.
    //! @param data           Start of the data of the MemoryInstanceSupport
    //! @param bytesAllocated How much memory is allocated for the data
    //! @param propertyLayout PropertyLayout of the variable-sized property whose size is increasing
    //! @param shiftBy        Positive or negative! Memory will be moved and SecondaryOffsets will be adjusted by this amount
    StatusInt                   ShiftValueData(ClassLayoutCR classLayout, byte * data, UInt32 bytesAllocated, PropertyLayoutCR propertyLayout, Int32 shiftBy);
    StatusInt                   ShiftArrayIndexValueData(PropertyLayoutCR propertyLayout, UInt32 arrayIndex, UInt32 arrayCount,  UInt32 endOfValueDataOffset, Int32 shiftBy);
        
    StatusInt                   EnsureSpaceIsAvailable (UInt32& offset, ClassLayoutCR classLayout, PropertyLayoutCR propertyLayout, UInt32 bytesNeeded);
    StatusInt                   EnsureSpaceIsAvailableForArrayIndexValue (ClassLayoutCR classLayout, PropertyLayoutCR propertyLayout, UInt32 arrayIndex, UInt32 bytesNeeded);
    StatusInt                   GrowPropertyValue (ClassLayoutCR classLayout, PropertyLayoutCR propertyLayout, UInt32 additionalbytesNeeded);           
         
protected:
    //! Constructor used by subclasses
    //! @param allowWritingDirectlyToInstanceMemory     If true, MemoryInstanceSupport is allowed to memset, memmove, and poke at the 
    //!                                                 memory directly, e.g. for StandaloneECIntance.
    //!                                                 If false, all modifications must happen through _ModifyData, e.g. for ECXData.
    ECOBJECTS_EXPORT            MemoryInstanceSupport (bool allowWritingDirectlyToInstanceMemory);
    ECOBJECTS_EXPORT void       InitializeMemory(ClassLayoutCR classLayout, byte * data, UInt32 bytesAllocated) const;
    ECOBJECTS_EXPORT StatusInt  GetValueFromMemory (ECValueR v, PropertyLayoutCR propertyLayout,      UInt32 nIndices, UInt32 const * indices) const;
    ECOBJECTS_EXPORT StatusInt  GetValueFromMemory (ClassLayoutCR classLayout, ECValueR v, const wchar_t * propertyAccessString, UInt32 nIndices, UInt32 const * indices) const;
    ECOBJECTS_EXPORT StatusInt  SetValueToMemory (ClassLayoutCR classLayout, const wchar_t * propertyAccessString, ECValueCR v,  UInt32 nIndices, UInt32 const * indices);      
    ECOBJECTS_EXPORT StatusInt  InsertNullArrayElementsAt (ClassLayoutCR classLayout, const wchar_t * propertyAccessString, UInt32 insertIndex, UInt32 insertCount);
    ECOBJECTS_EXPORT StatusInt  AddNullArrayElementsAt (ClassLayoutCR classLayout, const wchar_t * propertyAccessString, UInt32 insertCount);
    ECOBJECTS_EXPORT void       DumpInstanceData (ClassLayoutCR classLayout) const;
    
    virtual bool                _IsMemoryInitialized () const = 0;    
    //! Get a pointer to the first byte of the data    
    virtual byte const *        _GetData () const = 0;
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
    ECOBJECTS_EXPORT static InstanceHeader const&   PeekInstanceHeader (void const* data);
    };    


END_BENTLEY_EC_NAMESPACE