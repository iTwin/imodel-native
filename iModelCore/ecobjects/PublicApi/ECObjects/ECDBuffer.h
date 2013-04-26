/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicApi/ECObjects/ECDBuffer.h $
|
|   $Copyright: (c) 2013 Bentley Systems, Incorporated. All rights reserved. $
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

EC_TYPEDEFS(ECDBuffer);

BEGIN_BENTLEY_ECOBJECT_NAMESPACE
    
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
* @ingroup ECObjectsGroup
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
    //! An offset to either the data holding that property's value (for fixed-size values) or to the offset at which the properties value can be found.
    UInt32              m_offset;
    //! Indicates some special handling for the ECProperty, e.g. PROPERTYLAYOUTMODIFIERFLAGS_IsReadOnly, PROPERTYLAYOUTMODIFIERFLAGS_IsCalculated, etc.
    UInt32              m_modifierFlags;
    //! Data used with the modifier flag, like the length of a fixed-sized string.
    UInt32              m_modifierData;
    UInt32              m_nullflagsOffset;
    NullflagsBitmask    m_nullflagsBitmask;
public:
    PropertyLayout (WCharCP accessString, UInt32 psi, ECTypeDescriptor typeDescriptor, UInt32 offset, UInt32 nullflagsOffset, UInt32 nullflagsBitmask, UInt32 modifierFlags = 0,  UInt32 modifierData = 0) : //, ECPropertyCP property) :
        m_accessString(accessString), m_parentStructIndex (psi), m_typeDescriptor(typeDescriptor), m_offset(offset), m_nullflagsOffset(nullflagsOffset), 
        m_nullflagsBitmask (nullflagsBitmask), m_modifierFlags (modifierFlags), m_modifierData (modifierData) { }; //, m_property(property) {};

/*__PUBLISH_SECTION_START__*/
private:
    PropertyLayout (){}

public:
    ECOBJECTS_EXPORT WCharCP                     GetAccessString() const;       //!< Returns the access string for retrieving this property
    ECOBJECTS_EXPORT UInt32                      GetParentStructIndex() const;  //!< Returns the property index of the struct parent of this property. If this is a root property an index of zero is returned.
    ECOBJECTS_EXPORT UInt32                      GetOffset() const;             //!< Returns the offset to either the data holding this property's value (for fixed-size values) or to the offset at which the property's value can be found
    ECOBJECTS_EXPORT UInt32                      GetNullflagsOffset() const;    //!< Returns the offset to Null flags bit mask.
    ECOBJECTS_EXPORT NullflagsBitmask            GetNullflagsBitmask() const;   //!< Returns a bit mask that has the a single bit set. It can be used to AND with the bitmask at the offset returned by GetNullflagsOffset() to determine if the property is NULL.
    ECOBJECTS_EXPORT ECTypeDescriptor            GetTypeDescriptor() const;     //!< Returns an ECTypeDescriptor that defines this property

    //! Returns the modifier flags that describe this property, which can indicate
    //! @li that a string should be treated as fixed size
    //! @li that a string should have a max length
    //! @li that a longer fixed size type should be treated as an optional variable-sized type
    //! @li that for a string, only an entry to the string table is stored
    //! @li a default value should be used
    ECOBJECTS_EXPORT UInt32                      GetModifierFlags() const; 
    ECOBJECTS_EXPORT UInt32                      GetModifierData() const;       //!< Returns the data used with the modifier flag, like the length of a fixed-sized string.
    ECOBJECTS_EXPORT bool                        IsReadOnlyProperty () const;   //!< Returns whether this is a read-only property

    //! Sets the readonly flag for this property
    //! @param[in] readOnly Flag indicating whether this property is read-only or not
    //! @returns The readOnly status of this property
    ECOBJECTS_EXPORT bool                        SetReadOnlyMask (bool readOnly);
    ECOBJECTS_EXPORT bool                        IsFixedSized() const; //!< Returns whether this is a fixed-size property
    ECOBJECTS_EXPORT bool                        HoldsCalculatedProperty() const; //!< Returns whether this property is actually a Calculated Property

    //! Gets the size required for this PropertyValue in the fixed Section of the IECInstance's memory
    //! Variable-sized types will have 4 byte SecondaryOffset stored in the fixed Section.
    ECOBJECTS_EXPORT UInt32                       GetSizeInFixedSection() const;
    
    //! Returns a string containing detailed information about this property's characteristics
    //! (including access string, type name, offset, nullflagsOffset, nullflagsBitmask
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
* Responsible for managing the layout of the portion of an ECD buffer storing property
* values.
* @ingroup ECObjectsGroup
+===============+===============+===============+===============+===============+======*/      
struct ClassLayout
    {
/*__PUBLISH_SECTION_END__*/
    friend struct ECDBuffer;
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
        void        AddFixedSizeArrayProperty (WCharCP accessString, ECTypeDescriptor propertyDescriptor, UInt32 arrayCount, bool isReadOnly, bool isCalculated);
        void        AddVariableSizeProperty (WCharCP accessString, ECTypeDescriptor propertyDescriptor, bool isReadOnly, bool isCalculated);
        void        AddVariableSizeArrayPropertyWithFixedCount (WCharCP accessString, ECTypeDescriptor typeDescriptor, UInt32 arrayCount, bool isReadOnly, bool isCalculated);
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

    // Returns true if this ClassLayout is equivalent to the other ClassLayout (checks name and checksum)
    ECOBJECTS_EXPORT bool                    Equals (ClassLayoutCR other) const;

    ECOBJECTS_EXPORT ~ClassLayout();
    ECOBJECTS_EXPORT void            AddPropertyDirect (WCharCP accessString, UInt32 parentStructIndex, ECTypeDescriptor typeDescriptor, UInt32 offset, UInt32 nullflagsOffset, UInt32 nullflagsBitmask);
    ECOBJECTS_EXPORT ECObjectsStatus FinishLayout ();
/*__PUBLISH_SECTION_START__*/
private:
    //ClassLayout (){}

public:

    //! Given a class, will create the ClassLayout that manages that class
    //! @param[in] ecClass  The ECClass to build the ClassLayout from
    //! @returns ClassLayout pointer managing this ECClass
    ECOBJECTS_EXPORT static ClassLayoutP BuildFromClass (ECClassCR ecClass);

    //! Creates an empty ClassLayout for a class with the given name
    //! @param[in] className    The name of the class that this ClassLayout will define
    ECOBJECTS_EXPORT static ClassLayoutP CreateEmpty    (WCharCP  className);

    //! Returns the name of the ECClass that this ClassLayout manages
    ECOBJECTS_EXPORT WString const & GetECClassName() const; 

    //! Returns the property index of the given relationship end
    //! @param[in] end  The ECRelationshipEnd for which to get the pointer index
    //! @returns The property index of the pointer for the given relationship end
    ECOBJECTS_EXPORT int            GetECPointerIndex (ECRelationshipEnd end) const;
    
    //! Returns the checksum for this ClassLayout
    ECOBJECTS_EXPORT UInt32          GetChecksum () const;

    //! Returns the number of properties this ClassLayout manages
    ECOBJECTS_EXPORT UInt32          GetPropertyCount () const;

    //! Returns the number of properties this ClassLayout manages, not counting embedded structs
    ECOBJECTS_EXPORT UInt32          GetPropertyCountExcludingEmbeddedStructs () const;

    //! Given a property access string, will return the PropertyLayout
    //! @param[out] propertyLayout  Will point to the PropertyLayout if found
    //! @param[in]  accessString    The access string for the desired property
    //! @returns ECOBJECTS_STATUS_PropertyNotFound if the property is not found, ECOBJECTS_STATUS_Success otherwise
    ECOBJECTS_EXPORT ECObjectsStatus GetPropertyLayout (PropertyLayoutCP & propertyLayout, WCharCP accessString) const;

    //! Given a property index, will return the PropertyLayout
    //! @param[out] propertyLayout  Will point to the PropertyLayout if found
    //! @param[in]  propertyIndex   The index in the ClassLayout of the desired property
    //! @returns ECOBJECTS_STATUS_PropertyNotFound if the property is not found, ECOBJECTS_STATUS_Success otherwise
    ECOBJECTS_EXPORT ECObjectsStatus GetPropertyLayoutByIndex (PropertyLayoutCP & propertyLayout, UInt32 propertyIndex) const;

    //! Given a property layout, will return the property index within the ClassLayout
    //! @param[out] propertyIndex   Will contain the index of the given property within the ClassLayout, if found
    //! @param[in]  propertyLayout  The propertyLayout of the property that we want the index for
    //! @returns ECOBJECTS_STATUS_PropertyNotFound if the property is not found, ECOBJECTS_STATUS_Success otherwise
    ECOBJECTS_EXPORT ECObjectsStatus GetPropertyLayoutIndex (UInt32& propertyIndex, PropertyLayoutCR propertyLayout) const;

    //! Given an access string, will return the property index within the ClassLayout
    //! @param[out] propertyIndex   Will contain the index of the given property within the ClassLayout, if found
    //! @param[in]  accessString    The access string for the desired property
    //! @returns ECOBJECTS_STATUS_PropertyNotFound if the property is not found, ECOBJECTS_STATUS_Success otherwise
    ECOBJECTS_EXPORT ECObjectsStatus GetPropertyIndex (UInt32& propertyIndex, WCharCP accessString) const;

    //! Given a propertyIndex, will return whether the property is read-only or not
    //! @param[in]  propertyIndex   The index within the ClassLayout of the property to check
    //! @returns true if the property is read-only, false otherwise
    ECOBJECTS_EXPORT bool            IsPropertyReadOnly (UInt32 propertyIndex) const;

    //! Sets the read-only status of a property, given its index within the ClassLayout
    //! @param[in]  propertyIndex   The index within the ClassLayout of the property to set the read-only flag for
    //! @param[in]  readOnly        Flag indicating whether this property is read-only
    //! @returns The value of readOnly if successfully set, otherwise false
    ECOBJECTS_EXPORT bool            SetPropertyReadOnly (UInt32 propertyIndex, bool readOnly) const;
    
    //! Determines the number of bytes used for property data, so far
    ECOBJECTS_EXPORT UInt32         CalculateBytesUsed(byte const * propertyData) const;

    //! Checks the given classLayout to see if it is equal to, or a subset of, this layout
    //! @param[in]  layout  The ClassLayout to test compatibility of
    //! @returns true if the given ClassLayout is equal to or a subset of this layout, false otherwise
    ECOBJECTS_EXPORT bool           IsCompatible(ClassLayoutCR layout) const;

    //! Returns a string containing a description of the class and its properties
    ECOBJECTS_EXPORT WString       ToString() const;
    };

typedef bvector<ClassLayoutCP>  ClassLayoutVector;

/*=================================================================================**//**
* @ingroup ECObjectsGroup
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

    //! Gets the SchemaIndex of this particular SchemaLayout.
    ECOBJECTS_EXPORT SchemaIndex            GetSchemaIndex() const;

    //! Adds the ClassLayout at the given index
    //! @param[in]  classLayout The ClassLayout to add
    //! @param[in]  classIndex  The index where to add the ClassLayout
    //! @returns SUCCESS
    ECOBJECTS_EXPORT BentleyStatus          AddClassLayout (ClassLayoutCR classLayout, ClassIndex classIndex);

    //! Returns the ClassLayout at the given index
    //! @param[in]  classIndex  The index of the desired ClassLayout
    //! @returns A pointer to the requested ClassLayout if the index is valid, NULL otherwise
    ECOBJECTS_EXPORT ClassLayoutCP          GetClassLayout (ClassIndex classIndex);

    //! Given a classname, tries to find the corresponding ClassLayout
    //! @param[in]  className   The name of the class to find
    //! @returns A pointer to the corresponding ClassLayout if found, NULL otherwise
    ECOBJECTS_EXPORT ClassLayoutCP          FindClassLayout (WCharCP className);

    //! Given a classname, tries to find the index of the corresponding ClassLayout
    //! @param[out] classIndex  The index of the corresponding ClassLayout, if found
    //! @param[in]  className   The name of the class to find the ClassLayout index of
    //! @returns SUCCESS if the ClassLayout is found, ERROR otherwise
    ECOBJECTS_EXPORT BentleyStatus          FindClassIndex (ClassIndex& classIndex, WCharCP className) const;

    //! Finds the first available index for adding a class layout
    //! @param[out] classIndex  The first available index in the layout.  This is not necessarily the size of the layout because there can be gaps.
    //! @returns SUCCESS if the index is found, ERROR otherwise
    ECOBJECTS_EXPORT BentleyStatus          FindAvailableClassIndex (ClassIndex& classIndex);
   
    //! Returns the max index in the ClassLayout
    //! @remarks This may often correspond to "number of ClassLayouts - 1", but not necessarily, because there can be gaps
    //!          so when you call GetClassLayout (index) you might get NULLs. Even the last one could be NULL.
    //! @remarks NOTE: Check IsEmpty() before GetMaxIndex() to ensure there is at least one ClassLayout, otherwise the return value of GetMaxIndex() is undefined
    ECOBJECTS_EXPORT UInt32                 GetMaxIndex ();

    //! Returns whether or not this SchemaLayout has any ClassLayouts defined.
    ECOBJECTS_EXPORT bool                   IsEmpty() const;

    //! Creates a new SchemaLayout object
    //! @param[in]  index   The SchemaIndex used to identify this SchemaLayout
    //! @remarks It is up to the calling application to ensure uniqueness for the SchemaIndex.
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

//! An internal helper used by ECDBuffer to resize (add/remove elements) array property values
struct      ArrayResizer
    {
    friend struct ECDBuffer;

private:
    ClassLayoutCR           m_classLayout;
    PropertyLayoutCR        m_propertyLayout;
    ECDBufferR  m_instance;

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

    byte const *    m_propertyData;

    ArrayResizer (ClassLayoutCR classLayout, PropertyLayoutCR propertyLayout, ECDBufferR instance, UInt32 resizeIndex, UInt32 resizeElementCount);

    ECObjectsStatus       ShiftDataFollowingResizeIndex ();
    ECObjectsStatus       SetSecondaryOffsetsFollowingResizeIndex ();
    ECObjectsStatus       ShiftDataPreceedingResizeIndex ();
    ECObjectsStatus       SetSecondaryOffsetsPreceedingResizeIndex (SecondaryOffset* pSecondaryOffset, UInt32 byteCountToSet);
    ECObjectsStatus       WriteArrayHeader ();

    static ECObjectsStatus    CreateNullArrayElementsAt (ClassLayoutCR classLayout, PropertyLayoutCR propertyLayout, ECDBufferR instance, UInt32 insertIndex, UInt32 insertCount);
    };

enum ECDFormatVersion ENUM_UNDERLYING_TYPE(UInt8)
    {
    // Invalid or unknown version, possibly result of corrupted ECD buffer
    ECDFormat_Invalid   = 0xFF,

    // First version of ECD format to support versioning
    ECDFormat_v0        = 0,

    // Newer versions go here. Document all changes made to the format for each new version

    // The ECD format version with which this code was compiled. Should always match the maximum value of ECDFormatVersion
    ECDFormat_Current   = ECDFormat_v0,

    // The minimum ECD format version capable of reading data persisted with ECDFormat_Current.
    // Any code compiled with a version less than this cannot correctly interpret ECData saved with ECDFormat_Current.
    ECDFormat_MinimumReadable = ECDFormat_v0,

    // The minimum ECD format version capable of modifying data persisted with ECDFormat_Current.
    // Any code compiled with a version less than this cannot safely update ECData saved with ECDFormat_Current.
    ECDFormat_MinimumWritable = ECDFormat_v0,
    };

enum ECDFlags ENUM_UNDERLYING_TYPE(UInt8)
    {
    // Encoding used for all strings in this buffer. If not set, encoding is Utf16
    ECDFLAG_Utf8Encoding            = 1 << 0,

    // Future version of ECD format may add additional flags here.
    };

/*---------------------------------------------------------------------------------**//**
* This header is prepended to every ECDBuffer. Its purpose is to allow existing code to
* react gracefully to future changes to the ECD format.
* @bsistruct                                                    Paul.Connelly   11/12
+---------------+---------------+---------------+---------------+---------------+------*/
#pragma pack(push, 1)
struct ECDHeader_v0
    {
protected:
    // The version of ECD format used by this buffer.
    UInt8           m_formatVersion;
    // The minimum version of ECD format which is read-compatible with the data in this buffer. Always less than or equal to m_formatVersion
    // Code compiled for an ECD format version less than m_readableByVersion should not attempt to read the buffer. All other code can safely read the buffer.
    UInt8           m_readableByVersion;
    // The minimum version of ECD format which is write-compatible with the data in this buffer. Always less than or equal to m_formatVersion, always greater than or equal to m_readableByVersion.
    // Code compiled for an ECD format version less than m_writableByVersion should not attempt to modify the buffer.
    // All other code can safely modify the buffer, with the caveat that the persisted header is kept intact, aside from modifications to data understood by the compiled ECD format version.
    UInt8           m_writableByVersion;
    // The size in bytes of this header. Adding this value to the base address of the buffer yields the beginning of the property data.
    UInt8           m_headerSize;
    // A set of flags. See ECDFlags
    UInt8           m_flags;
    // Additional data can be appended here in future versions by creating a subclass of ECDHeader_v0
public:
    // Construct header with default values and size
    ECOBJECTS_EXPORT ECDHeader_v0();
    // Read header from persisted ECData. Returns false if no ECDHeader could be extracted.
    ECOBJECTS_EXPORT static bool    ReadHeader (ECDHeader_v0& header, byte const* data);

    bool                GetFlag (ECDFlags flag) const       { return 0 != (m_flags & flag); }
    void                SetFlag (ECDFlags flag, bool set)   { m_flags = set ? (m_flags | flag) : (m_flags & ~flag); }
    bool                IsReadable() const                  { return ECDFormat_Current >= m_readableByVersion; }
    bool                IsWritable() const                  { return ECDFormat_Current >= m_writableByVersion; }
    UInt8               GetFormatVersion() const            { return m_formatVersion; }
    UInt8               GetSize() const                     { return m_headerSize; }
    };
#pragma pack(pop)

typedef ECDHeader_v0 ECDHeader;

/*__PUBLISH_SECTION_START__*/
//=======================================================================================
//! Base class for ECN::IECInstance implementations that get/set values from a block of memory,
//! e.g. StandaloneECInstance and ECXInstance
//! @ingroup ECObjectsGroup
//! @bsiclass
//=======================================================================================
struct ECDBuffer
    {
    friend  struct ArrayResizer;
private:
    mutable bool        m_allowWritingDirectlyToInstanceMemory;

//__PUBLISH_CLASS_VIRTUAL__
/*__PUBLISH_SECTION_END__*/

    UInt32              GetOffsetToPropertyData() const;

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

    //! Shifts the values' data and adjusts SecondaryOffsets for all variable-sized property values
    //! AFTER the given one, to make room for additional bytes needed for the property value of the given PropertyLayout
    //! or to "compact" to reclaim unused space.
    //! @param data           Start of the data of the ECDBuffer
    //! @param bytesAllocated How much memory is allocated for the data
    //! @param propertyLayout PropertyLayout of the variable-sized property whose size is increasing
    //! @param shiftBy        Positive or negative! Memory will be moved and SecondaryOffsets will be adjusted by this amount
    ECObjectsStatus                   ShiftValueData(byte * data, UInt32 bytesAllocated, PropertyLayoutCR propertyLayout, Int32 shiftBy);
    ECObjectsStatus                   ShiftArrayIndexValueData(PropertyLayoutCR propertyLayout, UInt32 arrayIndex, UInt32 arrayCount,  UInt32 endOfValueDataOffset, Int32 shiftBy);
        
    ECObjectsStatus                   EnsureSpaceIsAvailable (UInt32& offset, PropertyLayoutCR propertyLayout, UInt32 bytesNeeded);
    ECObjectsStatus                   EnsureSpaceIsAvailableForArrayIndexValue (PropertyLayoutCR propertyLayout, UInt32 arrayIndex, UInt32 bytesNeeded);
    ECObjectsStatus                   GrowPropertyValue (PropertyLayoutCR propertyLayout, UInt32 additionalbytesNeeded);
    // Updates the calculated value in memory and returns the updated value in existingValue
    ECObjectsStatus                   EvaluateCalculatedProperty (PropertyLayoutCR propertyLayout, ECValueR existingValue, bool useArrayIndex, UInt32 arrayIndex) const;
    // Updates the dependent properties of the calculated property
    ECObjectsStatus                   SetCalculatedProperty (ECValueCR v, PropertyLayoutCR propertyLayout);
    ECObjectsStatus                   SetPrimitiveValueToMemory (ECValueCR v, PropertyLayoutCR propertyLayout, bool useIndex, UInt32 index, bool alreadyCalculated);

    ECObjectsStatus                   ModifyData (byte const* data, void const* newData, size_t dataLength);
    ECObjectsStatus                   ModifyData (UInt32 const* data, UInt32 newData);
    ECObjectsStatus                   MoveData (byte* to, byte const* from, size_t dataLength); 

    // These methods are for internal use by ECDBuffer only. They should be used within a single scope and *must* be paired. ScopedDataAccessor helps ensure this.
    friend struct ScopedDataAccessor;
    virtual bool                      _AcquireData() const = 0;
    virtual bool                      _ReleaseData() const = 0;
protected:
    //! Returns the number of bytes which must be allocated to store the header + the fixed portion of the property data, using ECDFormat_Current
    ECOBJECTS_EXPORT UInt32                 CalculateBytesUsed () const;
    ECOBJECTS_EXPORT ECDHeader const*       GetECDHeaderCP() const;

    //! Returns the number of elements in the specfieid array that are currently reserved but not necessarily allocated.
    //! This is important when an array has a minimum size but has not yet been initialized.  We delay initializing the memory for the minimum # of elements until
    //! the first value is set.  If an array does not have a minimum element count then GetReservedArrayCount will always equal GetAllocatedArrayCount
    //! This value is always >= the value returned by GetAllocatedArrayCount.
    //! This is the value used to set the count on an ArrayInfo value object that will be returned to a caller via GetValueFromMemory.  It is an implementation detail
    //! of memory based instances as to whether or not the physical memory to back that array count has actually been allocated.
    ECOBJECTS_EXPORT ArrayCount          GetReservedArrayCount (PropertyLayoutCR propertyLayout) const;

    //! Returns the number of elements in the specfieid array that are currently allocated in the instance data memory block.
    //! See the description of GetReservedArrayCount for explanation about the differences between the two.
    ECOBJECTS_EXPORT ArrayCount          GetAllocatedArrayCount (PropertyLayoutCR propertyLayout) const;


    //! Constructor used by subclasses
    //! @param allowWritingDirectlyToInstanceMemory     If true, ECDBuffer is allowed to memset, memmove, and poke at the
    //!                                                 memory directly, e.g. for StandaloneECIntance.
    //!                                                 If false, all modifications must happen through _ModifyData, e.g. for ECXData.
    ECOBJECTS_EXPORT            ECDBuffer (bool allowWritingDirectlyToInstanceMemory);
                     void       SetInstanceMemoryWritable (bool writable) const { m_allowWritingDirectlyToInstanceMemory = writable; }

    //! Obtains the current primitive value for the specified property
    //! If nIndices is > 0 then the primitive value for the array element at the specified index is obtained.
    //! This is protected because implementors of _GetStructArrayArrayValueFromMemory should call it to obtain the binary primitive value that they stored
    //! when _SetStructArrayValueToMemory was invoked as a means to locate the externalized struct array value.
    ECOBJECTS_EXPORT ECObjectsStatus  GetPrimitiveValueFromMemory (ECValueR v, PropertyLayoutCR propertyLayout, bool useIndex = false, UInt32 index = 0) const;    
    //! Sets the primitive value for the specified property
    //! If nIndices is > 0 then the primitive value for the array element at the specified index is set.
    //! This is protected because implementors of _SetStructArrayArrayValueToMemory should call it to store a binary primitive value that can be retrieved
    //! when _GetStructArrayValueFromMemory is invoked as a means to locate the externalized struct array value.
    ECOBJECTS_EXPORT ECObjectsStatus  SetPrimitiveValueToMemory   (ECValueCR v, PropertyLayoutCR propertyLayout, bool useIndex = false, UInt32 index = 0);    
    ECOBJECTS_EXPORT ECObjectsStatus  GetValueFromMemory (ECValueR v, PropertyLayoutCR propertyLayout) const;
    ECOBJECTS_EXPORT ECObjectsStatus  GetValueFromMemory (ECValueR v, PropertyLayoutCR propertyLayout, UInt32 index) const;    
    ECOBJECTS_EXPORT ECObjectsStatus  GetValueFromMemory (ECValueR v, WCharCP propertyAccessString, bool useIndex = false, UInt32 index = 0) const;
    ECOBJECTS_EXPORT ECObjectsStatus  GetValueFromMemory (ECValueR v, UInt32 propertyIndex, bool useArrayIndex = false, UInt32 arrayIndex = 0) const;
    ECOBJECTS_EXPORT ECObjectsStatus  SetValueToMemory (ECValueCR v, PropertyLayoutCR propertyLayout);          
    ECOBJECTS_EXPORT ECObjectsStatus  SetValueToMemory (ECValueCR v, PropertyLayoutCR propertyLayout, UInt32 index);              
    ECOBJECTS_EXPORT ECObjectsStatus  SetInternalValueToMemory (PropertyLayoutCR propertyLayout, ECValueCR v, bool useIndex = false, UInt32 index = 0);
    ECOBJECTS_EXPORT ECObjectsStatus  SetValueToMemory (WCharCP propertyAccessString, ECValueCR v,  bool useIndex = false, UInt32 index = 0);      
    ECOBJECTS_EXPORT ECObjectsStatus  SetValueToMemory (UInt32 propertyIndex, ECValueCR v, bool useArrayIndex = false, UInt32 arrayIndex = 0);      
    ECOBJECTS_EXPORT ECObjectsStatus  InsertNullArrayElementsAt (UInt32 propIdx, UInt32 insertIndex, UInt32 insertCount);
    ECOBJECTS_EXPORT ECObjectsStatus  AddNullArrayElementsAt (UInt32 propIdx, UInt32 insertCount);
    ECOBJECTS_EXPORT ECObjectsStatus  RemoveArrayElementsFromMemory (PropertyLayoutCR propertyLayout, UInt32 removeIndex, UInt32 removeCount);
    ECOBJECTS_EXPORT ECObjectsStatus  RemoveArrayElementsAt (UInt32 propIdx, UInt32 removeIndex, UInt32 removeCount);
    ECOBJECTS_EXPORT ECObjectsStatus  ClearArrayElementsFromMemory (UInt32 propIdx);
    ECOBJECTS_EXPORT WString          InstanceDataToString (WCharCP indent) const;
    ECOBJECTS_EXPORT ECObjectsStatus  GetIsNullValueFromMemory (bool& isNull, UInt32 propertyIndex, bool useIndex, UInt32 index) const;

    ECOBJECTS_EXPORT ECObjectsStatus  CopyInstancePropertiesToBuffer (IECInstanceCR srcInstance);

    virtual ~ECDBuffer () {}

    //! Sets the in-memory value of the array index of the specified property to be the struct value as held by v
    //! Since struct arrays support polymorphic values, we do not support storing the full struct value in the data section of the instance.  It must be externalized, therefore
    //! it is implementation specific and left to the implementation of the instance to store the value.  Before returning, the implementation should call into SetPrimitiveValueToMemory
    //! with a binary token that will actually be stored with the instance at the array index value that can then be used to locate the externalized struct value.
    //! Note that top-level struct properties will automatically be stored in the data section of the instance.  It is only struct array values that must be stored
    //! externally.
    //! If isInitializing is true, we are in the process of copying ECD memory buffer from another ECDBuffer, in which case this buffer's struct value ID entries are not valid.
    virtual ECObjectsStatus           _SetStructArrayValueToMemory (ECValueCR v, PropertyLayoutCR propertyLayout, UInt32 index) = 0;
    virtual ECObjectsStatus           _GetStructArrayValueFromMemory (ECValueR v, PropertyLayoutCR propertyLayout, UInt32 index) const = 0;
    virtual ECN::PrimitiveType         _GetStructArrayPrimitiveType () const = 0;

    virtual ECObjectsStatus           _RemoveStructArrayElementsFromMemory (PropertyLayoutCR propertyLayout, UInt32 removeIndex, UInt32 removeCount) = 0;

    //! Invoked after a variable-sized array is successfully resized, to allow derived classes to adjust their internal state in response if necessary.
    virtual void                      _HandleArrayResize (PropertyLayoutCP propertyLayout, UInt32 atIndex, Int32 countDelta) { };

    virtual bool                _IsMemoryInitialized () const = 0;    
    
    //! Get a pointer to the first byte of the ECDBuffer's data. This points to the first byte of the ECDHeader    
    virtual byte const *        _GetData () const = 0;
    virtual ECObjectsStatus     _ModifyData (UInt32 offset, void const * newData, UInt32 dataLength) = 0;
    virtual ECObjectsStatus     _MoveData (UInt32 toOffset, UInt32 fromOffset, UInt32 dataLength) = 0;
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

    virtual void                _ClearValues() = 0;
    virtual ECObjectsStatus     _CopyInstanceProperties (IECInstanceCR source) = 0;

    virtual IECInstanceP        _GetAsIECInstance () const = 0;
 
    virtual ClassLayoutCR       _GetClassLayout() const = 0;

    // Get a pointer to the first byte of the ECDBuffer's property data. This is the first byte beyond the ECDHeader.
    ECOBJECTS_EXPORT byte const*    GetPropertyData() const;
public:
    ECOBJECTS_EXPORT ClassLayoutCR          GetClassLayout () const;
    ECOBJECTS_EXPORT IECInstanceCP          GetAsIECInstance () const;
    ECOBJECTS_EXPORT IECInstanceP           GetAsIECInstanceP();
    ECOBJECTS_EXPORT ECObjectsStatus        RemoveArrayElements (PropertyLayoutCR propertyLayout, UInt32 removeIndex, UInt32 removeCount);
    ECOBJECTS_EXPORT void                   SetPerPropertyFlag (PropertyLayoutCR propertyLayout, bool useIndex, UInt32 index, int flagIndex, bool enable);
    ECOBJECTS_EXPORT ECN::PrimitiveType     GetStructArrayPrimitiveType () const;

    // Compress the memory storing the data to as small a size as possible
    ECOBJECTS_EXPORT ECObjectsStatus        Compress();
    // Calculate how many bytes are required for an empty buffer using the specified classLayout
    ECOBJECTS_EXPORT static UInt32          CalculateInitialAllocation (ClassLayoutCR classLayout);
    // Initialize a block of memory for a new ECDBuffer.
    ECOBJECTS_EXPORT static void            InitializeMemory(ClassLayoutCR classLayout, byte * data, UInt32 bytesAllocated);

    //! Given a persisted ECD buffer, returns true if the data is compatible with ECDFormat_Current.
    //! This method must be called before attempting to instantiate an ECD-based instance from a block of persistent memory.
    //! If it returns false, the calling method must not attempt to instantiate the instance.
    //! @param[out] header          Optional; if non-null, will contain a copy of the persistent ECDHeader
    //! @param[in]  data            The ECD-formatted block of memory
    //! @param[in]  requireWritable If true, the method will return false if the block of memory is not write-compatible with ECDFormat_Current.
    //! @return     true if the memory is read-compatible (if requireWritable=false) or write-compatible(if requireWritable=true) with ECDFormat_Current
    ECOBJECTS_EXPORT static bool            IsCompatibleVersion (ECDHeader* header, byte const* data, bool requireWritable = false);

    // Encoding used by all strings in the buffer
    enum StringEncoding
        {
        StringEncoding_Utf8,
        StringEncoding_Utf16
        };

    // Get the encoding used by strings in this buffer
    ECOBJECTS_EXPORT StringEncoding         GetStringEncoding() const;
    // Returns a platform-dependent preferred encoding for strings, or the encoding set by a call to SetDefaultStringEncoding.
    // This is the encoding that will be used when creating new in-memory ECDBuffers.
    ECOBJECTS_EXPORT static StringEncoding  GetDefaultStringEncoding();
    // Override the platform-dependent preferred encoding used when creating new in-memory ECDBuffers.
    ECOBJECTS_EXPORT static void            SetDefaultStringEncoding (StringEncoding defaultEncoding);

    // Get the size in bytes of the ECD memory buffer (including the header).
    ECOBJECTS_EXPORT size_t                 GetBufferSize() const;
    // Copies the ECD memory buffer (including the header) to the specified block of memory. Caller is responsible for allocating a block of memory of sufficient size.
    // (Call GetBufferSize() to determine the required size of the allocation).
    ECOBJECTS_EXPORT void                   GetBufferData (byte* dest) const;
/*__PUBLISH_SECTION_START__*/  
public:
    //! Returns true if the buffer is empty (all values are null and all arrays are empty)
    ECOBJECTS_EXPORT bool                   IsEmpty() const;
    //! Sets all values to null
    ECOBJECTS_EXPORT void                   ClearValues();
    //! Copies property values from source instance
    //! @param[in] sourceInstance   The instance from which to copy property values
    ECOBJECTS_EXPORT ECObjectsStatus        CopyInstanceProperties (IECInstanceCR sourceInstance);
    };   

END_BENTLEY_ECOBJECT_NAMESPACE
