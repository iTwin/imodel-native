/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicApi/ECObjects/MemoryInstanceSupport.h $
|
|   $Copyright: (c) 2012 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
/*__PUBLISH_SECTION_START__*/
#pragma once

#include "ECObjects.h"

/*__PUBLISH_SECTION_END__*/
#define N_FINAL_STRING_PROPS_IN_FAKE_CLASS 48
#define PROPERTYLAYOUT_Source_ECPointer L"Source ECPointer"
#define PROPERTYLAYOUT_Target_ECPointer L"Target ECPointer"
/*__PUBLISH_SECTION_START__*/

EC_TYPEDEFS(MemoryInstanceSupport);

BEGIN_BENTLEY_EC_NAMESPACE
    
typedef UInt32 NullflagsBitmask;
typedef UInt16 ClassIndex;
typedef UInt16 SchemaIndex;

/*__PUBLISH_SECTION_END__*/

typedef UInt32 SecondaryOffset;
typedef UInt32 ArrayCount;

#define NULLFLAGS_BITMASK_AllOn     0xFFFFFFFF

/*__PUBLISH_SECTION_START__*/

enum ArrayModifierFlags ENUM_UNDERLYING_TYPE (UInt32)
    {
    PROPERTYLAYOUTMODIFIERFLAGS_None              = 0x00,
    PROPERTYLAYOUTMODIFIERFLAGS_IsArrayFixedCount = 0x01,
    PROPERTYLAYOUTMODIFIERFLAGS_IsReadOnly        = 0x02,
    PROPERTYLAYOUTMODIFIERFLAGS_IsCalculated      = 0x04
    };

/*=================================================================================**//**
* @bsistruct
+===============+===============+===============+===============+===============+======*/      
struct PropertyLayout
    {
/*__PUBLISH_SECTION_END__*/
friend struct ClassLayout;    
private:
    WString                 m_accessString;        
    UInt32                  m_parentStructIndex;
    ECTypeDescriptor        m_typeDescriptor;

    // Using UInt32 instead of size_t below because we will persist this struct in an XAttribute. It will never be very big.
    UInt32              m_offset; //! An offset to either the data holding that property's value (for fixed-size values) or to the offset at which the properties value can be found.
    UInt32              m_modifierFlags; //! Can be used to indicate that a string should be treated as fixed size, with a max length, or that a longer fixed size type should be treated as an optional variable-sized type, or that for a string that only an entry to a StringTable is Stored, or that a default value should be used.
    UInt32              m_modifierData;  //! Data used with the modifier flag, like the length of a fixed-sized string.
    UInt32              m_nullflagsOffset;
    NullflagsBitmask    m_nullflagsBitmask;

  //ECPropertyCP        m_property; // WIP_FUSION: optional? YAGNI?

public:
    PropertyLayout (WCharCP accessString, UInt32 psi, ECTypeDescriptor typeDescriptor, UInt32 offset, UInt32 nullflagsOffset, UInt32 nullflagsBitmask, UInt32 modifierFlags = 0,  UInt32 modifierData = 0) : //, ECPropertyCP property) :
        m_accessString(accessString), m_parentStructIndex (psi), m_typeDescriptor(typeDescriptor), m_offset(offset), m_nullflagsOffset(nullflagsOffset), 
        m_nullflagsBitmask (nullflagsBitmask), m_modifierFlags (modifierFlags), m_modifierData (modifierData) { }; //, m_property(property) {};

/*__PUBLISH_SECTION_START__*/
private:
    PropertyLayout (){}

public:
    ECOBJECTS_EXPORT WCharCP                     GetAccessString() const;
    ECOBJECTS_EXPORT UInt32                      GetParentStructIndex() const;
    ECOBJECTS_EXPORT UInt32                      GetOffset() const;
    ECOBJECTS_EXPORT UInt32                      GetNullflagsOffset() const;
    ECOBJECTS_EXPORT NullflagsBitmask            GetNullflagsBitmask() const;
    ECOBJECTS_EXPORT ECTypeDescriptor            GetTypeDescriptor() const;
    ECOBJECTS_EXPORT UInt32                      GetModifierFlags() const;
    ECOBJECTS_EXPORT UInt32                      GetModifierData() const;
    ECOBJECTS_EXPORT bool                        IsReadOnlyProperty () const;

    ECOBJECTS_EXPORT bool                        SetReadOnlyMask (bool readOnly);
    ECOBJECTS_EXPORT bool                        IsFixedSized() const;
    ECOBJECTS_EXPORT bool                        HoldsCalculatedProperty() const;

    //! Gets the size required for this PropertyValue in the fixed Section of the IECInstance's memory
    //! Variable-sized types will have 4 byte SecondaryOffset stored in the fixed Section.
    ECOBJECTS_EXPORT UInt32                       GetSizeInFixedSection() const;
    
    ECOBJECTS_EXPORT WString                      ToString();
    };

/*__PUBLISH_SECTION_END__*/
struct less_classLayout
{
bool operator()(ClassLayoutCP s1, ClassLayoutCP s2) const;
};

 typedef bmap<ClassLayoutCP, bool, less_classLayout>   CompatibleClassLayoutsMap;

 /*__PUBLISH_SECTION_START__*/
 /*=================================================================================**//**
* @bsistruct
+===============+===============+===============+===============+===============+======*/      
struct ClassLayout
    {
/*__PUBLISH_SECTION_END__*/
    friend struct MemoryInstanceSupport;
private:
    struct AccessStringIndexPair : bpair<WCharCP, UInt32>
        {
        AccessStringIndexPair (WCharCP accessor, UInt32 index) : bpair<WCharCP, UInt32> (accessor, index) { }
        };

    typedef bvector<AccessStringIndexPair>                          IndicesByAccessString;
    typedef bvector<PropertyLayoutP>                                PropertyLayoutVector;
    typedef bmap<UInt32, bvector<UInt32> >                          LogicalStructureMap;
    
    enum State
        {
        AcceptingFixedSizeProperties,
        AcceptingVariableSizeProperties,
        Closed
        };
    
    // These members are expected to be persisted  
    WString                 m_className;
    
    PropertyLayoutVector    m_propertyLayouts;      // This is the primary collection, there is a secondary map for lookup by name, below.    
    IndicesByAccessString   m_indicesByAccessString; // Always sorted; maps access strings to indices which can be used to index into PropertyLayoutVector
    LogicalStructureMap     m_logicalStructureMap;
    
    // These members are transient
    UInt32                            m_sizeOfFixedSection;
    bool                              m_isRelationshipClass;         // make this a bitmask IsRelationshipClass, OnPartialInstanceLoaded (this should allow roundtrip native-managed-native
    int                               m_propertyIndexOfSourceECPointer;
    int                               m_propertyIndexOfTargetECPointer;
    int                               m_uniqueId;
    mutable CompatibleClassLayoutsMap m_compatibleClassLayouts;
    mutable UInt32                    m_checkSum;

    void                            CheckForECPointers (WCharCP accessString);

    // returns an iterator into m_indicesByAccessStrings at which accessString exists (if forCreate=false), or would be inserted (if forCreate=true)
    IndicesByAccessString::const_iterator GetPropertyIndexPosition (WCharCP accessString, bool forCreate) const;
    UInt32  ComputeCheckSum () const;

    struct  Factory
    {
    friend struct ClassLayout;

    private:
        State           m_state;
        ECClassCR       m_ecClass;
        UInt32          m_offset;
        UInt32          m_nullflagsOffset;
        UInt32          m_nonStructPropertyCount;
        ClassLayoutR    m_underConstruction;

        UInt32      GetParentStructIndex (WCharCP accessString) const;

        void        AddProperty (WCharCP accessString, ECTypeDescriptor propertyDescriptor, UInt32 size, UInt32 modifierFlags = 0, UInt32 modifierData = 0);
        void        AddStructProperty (WCharCP accessString, ECTypeDescriptor propertyDescriptor);
        void        AddFixedSizeProperty (WCharCP accessString, ECTypeDescriptor propertyDescriptor, bool isReadOnly, bool isCalculated);
        void        AddFixedSizeArrayProperty (WCharCP accessString, ECTypeDescriptor propertyDescriptor, UInt32 arrayCount, bool isReadOnly);
        void        AddVariableSizeProperty (WCharCP accessString, ECTypeDescriptor propertyDescriptor, bool isReadOnly, bool isCalculated);
        void        AddVariableSizeArrayPropertyWithFixedCount (WCharCP accessString, ECTypeDescriptor typeDescriptor, UInt32 arrayCount, bool isReadOnly);        
        void        AddProperties (ECClassCR ecClass, WCharCP nameRoot, bool addFixedSize);

        Factory (ECClassCR ecClass);
        ClassLayoutP DoBuildClassLayout ();
    };

    ClassLayout();

    WString                GetShortDescription() const;
    WString                LogicalStructureToString (UInt32 parentStructIndex = 0, UInt32 indentLevel = 0) const;
    BentleyStatus          SetClass (WCharCP  className);

public:    
    WString                 GetName() const;
    int                     GetUniqueId() const;
    void                    AddPropertyLayout (WCharCP accessString, PropertyLayoutR);
    void                    AddToLogicalStructureMap (PropertyLayoutR propertyLayout, UInt32 propertyIndex);
    ECOBJECTS_EXPORT void   InitializeMemoryForInstance(byte * data, UInt32 bytesAllocated) const;
    ECOBJECTS_EXPORT UInt32 GetSizeOfFixedSection() const;

    ECOBJECTS_EXPORT UInt32                  GetFirstChildPropertyIndex (UInt32 parentIndex) const;
    ECOBJECTS_EXPORT UInt32                  GetNextChildPropertyIndex (UInt32 parentIndex, UInt32 childIndex) const;
    ECOBJECTS_EXPORT ECObjectsStatus         GetAccessStringByIndex (WCharCP& accessString, UInt32 propertyIndex) const;
    ECOBJECTS_EXPORT ECObjectsStatus         GetPropertyIndices (bvector<UInt32>& properties, UInt32 parentIndex) const;
    ECOBJECTS_EXPORT bool                    HasChildProperties (UInt32 parentIndex) const;

    ECOBJECTS_EXPORT ~ClassLayout();
    ECOBJECTS_EXPORT void            AddPropertyDirect (WCharCP accessString, UInt32 parentStructIndex, ECTypeDescriptor typeDescriptor, UInt32 offset, UInt32 nullflagsOffset, UInt32 nullflagsBitmask);
    ECOBJECTS_EXPORT ECObjectsStatus FinishLayout ();

/*__PUBLISH_SECTION_START__*/
private:
    //ClassLayout (){}

public:
    ECOBJECTS_EXPORT static ClassLayoutP BuildFromClass (ECClassCR ecClass);
    ECOBJECTS_EXPORT static ClassLayoutP CreateEmpty    (WCharCP  className);

    ECOBJECTS_EXPORT WString const & GetECClassName() const;
    ECOBJECTS_EXPORT int            GetECPointerIndex (ECRelationshipEnd end) const;
    
    ECOBJECTS_EXPORT UInt32          GetChecksum () const;
    ECOBJECTS_EXPORT UInt32          GetPropertyCount () const;
    ECOBJECTS_EXPORT UInt32          GetPropertyCountExcludingEmbeddedStructs () const;
    ECOBJECTS_EXPORT ECObjectsStatus GetPropertyLayout (PropertyLayoutCP & propertyLayout, WCharCP accessString) const;
    ECOBJECTS_EXPORT ECObjectsStatus GetPropertyLayoutByIndex (PropertyLayoutCP & propertyLayout, UInt32 propertyIndex) const;
                     ECObjectsStatus GetPropertyLayoutIndex (UInt32& propertyIndex, PropertyLayoutCR propertyLayout) const;
    ECOBJECTS_EXPORT ECObjectsStatus GetPropertyIndex (UInt32& propertyIndex, WCharCP accessString) const;
    ECOBJECTS_EXPORT bool            IsPropertyReadOnly (UInt32 propertyIndex) const;
    ECOBJECTS_EXPORT bool            SetPropertyReadOnly (UInt32 propertyIndex, bool readOnly) const;
    
    //! Determines the number of bytes used, so far
    ECOBJECTS_EXPORT UInt32         CalculateBytesUsed(byte const * data) const;
    ECOBJECTS_EXPORT bool           IsCompatible(ClassLayoutCR layout) const;

    ECOBJECTS_EXPORT WString       ToString() const;
    };

typedef bvector<ClassLayoutCP>  ClassLayoutVector;

/*=================================================================================**//**
* @bsistruct
+===============+===============+===============+===============+===============+======*/      
struct SchemaLayout
{
/*__PUBLISH_SECTION_END__*/
private:
    SchemaIndex             m_schemaIndex;
    ClassLayoutVector       m_classLayouts;

public:
    SchemaLayout(SchemaIndex index) : m_schemaIndex(index) {}

/*__PUBLISH_SECTION_START__*/
private:
    SchemaLayout (){}

public:
    ECOBJECTS_EXPORT SchemaIndex            GetSchemaIndex() const;
    ECOBJECTS_EXPORT BentleyStatus          AddClassLayout (ClassLayoutCR, ClassIndex);
    ECOBJECTS_EXPORT ClassLayoutCP          GetClassLayout (ClassIndex classIndex);
    ECOBJECTS_EXPORT ClassLayoutCP          FindClassLayout (WCharCP className);
    ECOBJECTS_EXPORT BentleyStatus          FindClassIndex (ClassIndex& classIndex, WCharCP className) const;
    ECOBJECTS_EXPORT BentleyStatus          FindAvailableClassIndex (ClassIndex&);
    // This may often correspond to "number of ClassLayouts - 1", but not necessarily, because there can be gaps
    // so when you call GetClassLayout (index) you might get NULLs. Even the last one could be NULL.
    ECOBJECTS_EXPORT UInt32                 GetMaxIndex ();
    ECOBJECTS_EXPORT static SchemaLayoutP   Create (SchemaIndex index);
};

/*__PUBLISH_SECTION_END__*/

//=======================================================================================    
//! Holds a ClassLayoutCR and provides a public method by which to access it.
//! Used by StandaloneECEnabler and ECXInstanceEnabler
//=======================================================================================    
struct ClassLayoutHolder
    {
private:
    ClassLayoutCR                   m_classLayout;
        
protected:
    ECOBJECTS_EXPORT                ClassLayoutHolder (ClassLayoutCR classLayout);

public:    
    ECOBJECTS_EXPORT ClassLayoutCR  GetClassLayout() const;
    };

//! An internal helper used by MemoryInstanceSupport to resize (add/remove elements) array property values
struct      ArrayResizer
    {
    friend struct MemoryInstanceSupport;
    
private:
    ClassLayoutCR           m_classLayout;
    PropertyLayoutCR        m_propertyLayout;
    MemoryInstanceSupportR  m_instance;
    
    UInt32          m_arrayOffset;
    
    UInt32          m_resizeIndex;
    UInt32          m_resizeElementCount; 
    UInt32          m_resizeFixedSectionByteCount; // bytesNeeded
        
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
        
    ECObjectsStatus       ShiftDataFollowingResizeIndex ();
    ECObjectsStatus       SetSecondaryOffsetsFollowingResizeIndex ();
    ECObjectsStatus       ShiftDataPreceedingResizeIndex ();
    ECObjectsStatus       SetSecondaryOffsetsPreceedingResizeIndex (SecondaryOffset* pSecondaryOffset, UInt32 byteCountToSet);    
    ECObjectsStatus       WriteArrayHeader ();
        
    static ECObjectsStatus    CreateNullArrayElementsAt (ClassLayoutCR classLayout, PropertyLayoutCR propertyLayout, MemoryInstanceSupportR instance, UInt32 insertIndex, UInt32 insertCount);
    };

/*__PUBLISH_SECTION_START__*/  
//=======================================================================================    
//! Base class for EC::IECInstance implementations that get/set values from a block of memory, 
//! e.g. StandaloneECInstance and ECXInstance
//=======================================================================================    
struct MemoryInstanceSupport
    {
    friend  struct ArrayResizer;
/*__PUBLISH_SECTION_END__*/    
    
/*__PUBLISH_SECTION_START__*/  
private:    
    bool                        m_allowWritingDirectlyToInstanceMemory;
//__PUBLISH_CLASS_VIRTUAL__
/*__PUBLISH_SECTION_END__*/    

    //! Returns the offset of the property value relative to the start of the instance data.
    //! If useIndex is true then the offset of the array element value at the specified index is returned.
    UInt32              GetOffsetOfPropertyValue (PropertyLayoutCR propertyLayout, bool useIndex = false, UInt32 index = 0) const;
    
    //! Returns the size in bytes of the property value
    UInt32              GetPropertyValueSize (PropertyLayoutCR propertyLayout) const;   
    //! Returns the size in bytes of the array element value at the specified index
    UInt32              GetPropertyValueSize (PropertyLayoutCR propertyLayout, UInt32 index) const;    
    
    //! Returns the address of the property value     
    byte const *        GetAddressOfPropertyValue (PropertyLayoutCR propertyLayout) const;    
    //! Returns the address of the array element value at the specified index
    byte const *        GetAddressOfPropertyValue (PropertyLayoutCR propertyLayout, UInt32 index) const;        
    
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
    bool                IsPropertyValueNull (PropertyLayoutCR propertyLayout, bool useIndex = false, UInt32 index = 0) const;
    
    //! Sets the null bit of the specified property to the value indicated by isNull
    //! If nIndices is > 0 then the null bit is set for the array element at the specified index    
    void                SetPropertyValueNull (PropertyLayoutCR propertyLayout, bool useIndex, UInt32 index, bool isNull);    

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
    
    //! Shifts the values' data and adjusts SecondaryOffsets for all variable-sized property values 
    //! AFTER the given one, to make room for additional bytes needed for the property value of the given PropertyLayout
    //! or to "compact" to reclaim unused space.
    //! @param data           Start of the data of the MemoryInstanceSupport
    //! @param bytesAllocated How much memory is allocated for the data
    //! @param propertyLayout PropertyLayout of the variable-sized property whose size is increasing
    //! @param shiftBy        Positive or negative! Memory will be moved and SecondaryOffsets will be adjusted by this amount
    ECObjectsStatus                   ShiftValueData(ClassLayoutCR classLayout, byte * data, UInt32 bytesAllocated, PropertyLayoutCR propertyLayout, Int32 shiftBy);
    ECObjectsStatus                   ShiftArrayIndexValueData(PropertyLayoutCR propertyLayout, UInt32 arrayIndex, UInt32 arrayCount,  UInt32 endOfValueDataOffset, Int32 shiftBy);
        
    ECObjectsStatus                   EnsureSpaceIsAvailable (UInt32& offset, ClassLayoutCR classLayout, PropertyLayoutCR propertyLayout, UInt32 bytesNeeded);
    ECObjectsStatus                   EnsureSpaceIsAvailableForArrayIndexValue (ClassLayoutCR classLayout, PropertyLayoutCR propertyLayout, UInt32 arrayIndex, UInt32 bytesNeeded);
    ECObjectsStatus                   GrowPropertyValue (ClassLayoutCR classLayout, PropertyLayoutCR propertyLayout, UInt32 additionalbytesNeeded);
    // Updates the calculated value in memory and returns the updated value in existingValue
    ECObjectsStatus                   EvaluateCalculatedProperty (ClassLayoutCR classLayout, PropertyLayoutCR propertyLayout, ECValueR existingValue) const;
    // Updates the dependent properties of the calculated property
    ECObjectsStatus                   SetCalculatedProperty (ECValueCR v, ClassLayoutCR classLayout, PropertyLayoutCR propertyLayout);
    ECObjectsStatus                   SetPrimitiveValueToMemory (ECValueCR v, ClassLayoutCR classLayout, PropertyLayoutCR propertyLayout, bool useIndex, UInt32 index, bool alreadyCalculated);
         
protected:
    //! Constructor used by subclasses
    //! @param allowWritingDirectlyToInstanceMemory     If true, MemoryInstanceSupport is allowed to memset, memmove, and poke at the 
    //!                                                 memory directly, e.g. for StandaloneECIntance.
    //!                                                 If false, all modifications must happen through _ModifyData, e.g. for ECXData.
    ECOBJECTS_EXPORT            MemoryInstanceSupport (bool allowWritingDirectlyToInstanceMemory);
    ECOBJECTS_EXPORT void       InitializeMemory(ClassLayoutCR classLayout, byte * data, UInt32 bytesAllocated) const;
    //! Obtains the current primitive value for the specified property
    //! If nIndices is > 0 then the primitive value for the array element at the specified index is obtained.
    //! This is protected because implementors of _GetStructArrayArrayValueFromMemory should call it to obtain the binary primitive value that they stored
    //! when _SetStructArrayValueToMemory was invoked as a means to locate the externalized struct array value.
    ECOBJECTS_EXPORT ECObjectsStatus  GetPrimitiveValueFromMemory (ECValueR v, ClassLayoutCR classLayout, PropertyLayoutCR propertyLayout, bool useIndex = false, UInt32 index = 0) const;    
    //! Sets the primitive value for the specified property
    //! If nIndices is > 0 then the primitive value for the array element at the specified index is set.
    //! This is protected because implementors of _SetStructArrayArrayValueToMemory should call it to store a binary primitive value that can be retrieved
    //! when _GetStructArrayValueFromMemory is invoked as a means to locate the externalized struct array value.
    ECOBJECTS_EXPORT ECObjectsStatus  SetPrimitiveValueToMemory   (ECValueCR v, ClassLayoutCR classLayout, PropertyLayoutCR propertyLayout, bool useIndex = false, UInt32 index = 0);    
    ECOBJECTS_EXPORT ECObjectsStatus  GetValueFromMemory (ECValueR v, ClassLayoutCR classLayout, PropertyLayoutCR propertyLayout) const;
    ECOBJECTS_EXPORT ECObjectsStatus  GetValueFromMemory (ECValueR v, ClassLayoutCR classLayout, PropertyLayoutCR propertyLayout, UInt32 index) const;    
    ECOBJECTS_EXPORT ECObjectsStatus  GetValueFromMemory (ECValueR v, ClassLayoutCR classLayout, WCharCP propertyAccessString, bool useIndex = false, UInt32 index = 0) const;
    ECOBJECTS_EXPORT ECObjectsStatus  GetValueFromMemory (ECValueR v, ClassLayoutCR classLayout, UInt32 propertyIndex, bool useArrayIndex = false, UInt32 arrayIndex = 0) const;
    ECOBJECTS_EXPORT ECObjectsStatus  SetValueToMemory (ECValueCR v, ClassLayoutCR classLayout, PropertyLayoutCR propertyLayout);          
    ECOBJECTS_EXPORT ECObjectsStatus  SetValueToMemory (ECValueCR v, ClassLayoutCR classLayout, PropertyLayoutCR propertyLayout, UInt32 index);              
    ECOBJECTS_EXPORT ECObjectsStatus  SetValueToMemory (ClassLayoutCR classLayout, WCharCP propertyAccessString, ECValueCR v,  bool useIndex = false, UInt32 index = 0);      
    ECOBJECTS_EXPORT ECObjectsStatus  SetValueToMemory (ClassLayoutCR classLayout, UInt32 propertyIndex, ECValueCR v, bool useArrayIndex = false, UInt32 arrayIndex = 0);      
    ECOBJECTS_EXPORT ECObjectsStatus  InsertNullArrayElementsAt (ClassLayoutCR classLayout, WCharCP propertyAccessString, UInt32 insertIndex, UInt32 insertCount);
    ECOBJECTS_EXPORT ECObjectsStatus  AddNullArrayElementsAt (ClassLayoutCR classLayout, WCharCP propertyAccessString, UInt32 insertCount);
    ECOBJECTS_EXPORT ECObjectsStatus  RemoveArrayElementsFromMemory (ClassLayoutCR classLayout, PropertyLayoutCR propertyLayout, UInt32 removeIndex, UInt32 removeCount);
    ECOBJECTS_EXPORT ECObjectsStatus  RemoveArrayElementsAt (ClassLayoutCR classLayout, WCharCP propertyAccessString, UInt32 removeIndex, UInt32 removeCount);
    ECOBJECTS_EXPORT WString          InstanceDataToString (WCharCP indent, ClassLayoutCR classLayout) const;
    ECOBJECTS_EXPORT ECObjectsStatus  GetIsNullValueFromMemory (ClassLayoutCR classLayout, bool& isNull, UInt32 propertyIndex, bool useIndex, UInt32 index) const;

    virtual ~MemoryInstanceSupport () {}

    //! Sets the in-memory value of the array index of the specified property to be the struct value as held by v
    //! Since struct arrays support polymorphic values, we do not support storing the full struct value in the data section of the instance.  It must be externalized, therefore
    //! it is implementation specific and left to the implementation of the instance to store the value.  Before returning, the implementation should call into SetPrimitiveValueToMemory
    //! with a binary token that will actually be stored with the instance at the array index value that can then be used to locate the externalized struct value.
    //! Note that top-level struct properties will automatically be stored in the data section of the instance.  It is only struct array values that must be stored
    //! externally.
    virtual ECObjectsStatus           _SetStructArrayValueToMemory (ECValueCR v, ClassLayoutCR classLayout, PropertyLayoutCR propertyLayout, UInt32 index) = 0;
    virtual ECObjectsStatus           _GetStructArrayValueFromMemory (ECValueR v, PropertyLayoutCR propertyLayout, UInt32 index) const = 0;
    virtual EC::PrimitiveType         _GetStructArrayPrimitiveType () const = 0;

    virtual ECObjectsStatus           _RemoveStructArrayElementsFromMemory (ClassLayoutCR classLayout, PropertyLayoutCR propertyLayout, UInt32 removeIndex, UInt32 removeCount) = 0;

    //! Invoked after a variable-sized array is successfully resized, to allow derived classes to adjust their internal state in response if necessary.
    virtual void                      _HandleArrayResize (PropertyLayoutCP propertyLayout, UInt32 atIndex, Int32 countDelta) { };

    virtual bool                _IsMemoryInitialized () const = 0;    
    
    //! Get a pointer to the first byte of the data    
    virtual byte const *        _GetData () const = 0;
    virtual ECObjectsStatus     _ModifyData (UInt32 offset, void const * newData, UInt32 dataLength) = 0;
    virtual UInt32              _GetBytesAllocated () const = 0;
        
    //! Reallocates memory for the IECInstance and copies the old IECInstance data into the new memory
    //! You might get more memory than used asked for, but you won't get less
    //! @param additionalBytesNeeded  Additional bytes of memory needed above current allocation
    virtual ECObjectsStatus    _GrowAllocation (UInt32 additionalBytesNeeded) = 0;
    
    //! Shrinks the allocated IECInstance data to be as small as possible
    virtual ECObjectsStatus    _ShrinkAllocation () = 0;
    
    //! Free any allocated memory
    virtual void                _FreeAllocation () = 0;
    
    virtual void                _SetPerPropertyFlag (PropertyLayoutCR propertyLayout, bool useIndex, UInt32 index, int flagIndex, bool enable) {};

public:
    ECOBJECTS_EXPORT ECObjectsStatus        RemoveArrayElements (ClassLayoutCR classLayout, PropertyLayoutCR propertyLayout, UInt32 removeIndex, UInt32 removeCount);
   
    ECOBJECTS_EXPORT void                   SetPerPropertyFlag (PropertyLayoutCR propertyLayout, bool useIndex, UInt32 index, int flagIndex, bool enable);

    ECOBJECTS_EXPORT EC::PrimitiveType      GetStructArrayPrimitiveType () const;

    // Compress the memory storing the data to as small a size as possible
    ECOBJECTS_EXPORT ECObjectsStatus        Compress();

/*__PUBLISH_SECTION_START__*/  
    };   

END_BENTLEY_EC_NAMESPACE
