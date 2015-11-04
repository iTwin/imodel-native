/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicApi/ECObjects/ECDBuffer.h $
|
|   $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
/*__PUBLISH_SECTION_START__*/
/** @cond BENTLEY_SDK_Internal */

#include "ECObjects.h"

/*__PUBLISH_SECTION_END__*/
#define N_FINAL_STRING_PROPS_IN_FAKE_CLASS 48
/*__PUBLISH_SECTION_START__*/

EC_TYPEDEFS(ECDBuffer);

BEGIN_BENTLEY_ECOBJECT_NAMESPACE

typedef uint32_t NullflagsBitmask;
typedef uint16_t ClassIndex;
typedef uint16_t SchemaIndex;
typedef RefCountedPtr<ClassLayout> ClassLayoutPtr;

/*__PUBLISH_SECTION_END__*/

typedef uint32_t SecondaryOffset;
typedef uint32_t ArrayCount;

#define NULLFLAGS_BITMASK_AllOn     0xFFFFFFFF

/*__PUBLISH_SECTION_START__*/

enum ArrayModifierFlags ENUM_UNDERLYING_TYPE (uint32_t)
    {
    PROPERTYLAYOUTMODIFIERFLAGS_None              = 0x00,
    PROPERTYLAYOUTMODIFIERFLAGS_IsArrayFixedCount = 0x01,
    PROPERTYLAYOUTMODIFIERFLAGS_IsReadOnly        = 0x02,
    PROPERTYLAYOUTMODIFIERFLAGS_IsCalculated      = 0x04
    };

/*=================================================================================**//**
* @addtogroup ECObjectsGroup
* @beginGroup
* @bsistruct
+===============+===============+===============+===============+===============+======*/      
struct PropertyLayout
    {
/*__PUBLISH_SECTION_END__*/
friend struct ClassLayout;
private:
    Utf8String              m_accessString;
    uint32_t                m_parentStructIndex;
    ECTypeDescriptor        m_typeDescriptor;

    // Using UInt32 instead of size_t below because we will persist this struct in an XAttribute. It will never be very big.
    //! An offset to either the data holding that property's value (for fixed-size values) or to the offset at which the properties value can be found.
    uint32_t            m_offset;
    //! Indicates some special handling for the ECProperty, e.g. PROPERTYLAYOUTMODIFIERFLAGS_IsReadOnly, PROPERTYLAYOUTMODIFIERFLAGS_IsCalculated, etc.
    uint32_t            m_modifierFlags;
    //! Data used with the modifier flag, like the length of a fixed-sized string.
    uint32_t            m_modifierData;
    uint32_t            m_nullflagsOffset;
    NullflagsBitmask    m_nullflagsBitmask;
public:
    PropertyLayout (Utf8CP accessString, uint32_t psi, ECTypeDescriptor typeDescriptor, uint32_t offset, uint32_t nullflagsOffset, uint32_t nullflagsBitmask, uint32_t modifierFlags = 0,  uint32_t modifierData = 0) : //, ECPropertyCP property) :
        m_accessString(accessString), m_parentStructIndex (psi), m_typeDescriptor(typeDescriptor), m_offset(offset), m_nullflagsOffset(nullflagsOffset),
        m_nullflagsBitmask (nullflagsBitmask), m_modifierFlags (modifierFlags), m_modifierData (modifierData) { }; //, m_property(property) {};

//__PUBLISH_CLASS_VIRTUAL__
/*__PUBLISH_SECTION_START__*/
private:
    PropertyLayout (){}

public:
    ECOBJECTS_EXPORT Utf8CP                      GetAccessString() const;       //!< Returns the access string for retrieving this property
    ECOBJECTS_EXPORT uint32_t                    GetParentStructIndex() const;  //!< Returns the property index of the struct parent of this property. If this is a root property an index of zero is returned.
    ECOBJECTS_EXPORT uint32_t                    GetOffset() const;             //!< Returns the offset to either the data holding this property's value (for fixed-size values) or to the offset at which the property's value can be found
    ECOBJECTS_EXPORT uint32_t                    GetNullflagsOffset() const;    //!< Returns the offset to Null flags bit mask.
    ECOBJECTS_EXPORT NullflagsBitmask            GetNullflagsBitmask() const;   //!< Returns a bit mask that has the a single bit set. It can be used to AND with the bitmask at the offset returned by GetNullflagsOffset() to determine if the property is NULL.
    ECOBJECTS_EXPORT ECTypeDescriptor            GetTypeDescriptor() const;     //!< Returns an ECTypeDescriptor that defines this property

    //! Returns the modifier flags that describe this property, which can indicate
    //! @li that a string should be treated as fixed size
    //! @li that a string should have a max length
    //! @li that a longer fixed size type should be treated as an optional variable-sized type
    //! @li that for a string, only an entry to the string table is stored
    //! @li a default value should be used
    ECOBJECTS_EXPORT uint32_t                    GetModifierFlags() const; 
    ECOBJECTS_EXPORT uint32_t                    GetModifierData() const;       //!< Returns the data used with the modifier flag, like the length of a fixed-sized string.
    ECOBJECTS_EXPORT bool                        IsReadOnlyProperty () const;   //!< Returns whether this is a read-only property

    //! Sets the readonly flag for this property
    //! @param[in] readOnly Flag indicating whether this property is read-only or not
    //! @returns The readOnly status of this property
    ECOBJECTS_EXPORT bool                        SetReadOnlyMask (bool readOnly);
    ECOBJECTS_EXPORT bool                        IsFixedSized() const; //!< Returns whether this is a fixed-size property
    ECOBJECTS_EXPORT bool                        HoldsCalculatedProperty() const; //!< Returns whether this property is actually a Calculated Property

    //! Gets the size required for this PropertyValue in the fixed Section of the IECInstance's memory
    //! Variable-sized types will have 4 byte SecondaryOffset stored in the fixed Section.
    ECOBJECTS_EXPORT uint32_t                     GetSizeInFixedSection() const;
    
    //! Returns a string containing detailed information about this property's characteristics
    //! (including access string, type name, offset, nullflagsOffset, nullflagsBitmask
    ECOBJECTS_EXPORT Utf8String                     ToString();
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
+===============+===============+===============+===============+===============+======*/      
struct ClassLayout : RefCountedBase
    {
/*__PUBLISH_SECTION_END__*/
    friend struct ECDBuffer;
private:
    struct AccessStringIndexPair : bpair<Utf8CP, uint32_t>
        {
        AccessStringIndexPair (Utf8CP accessor, uint32_t index) : bpair<Utf8CP, uint32_t> (accessor, index) { }
        };

    typedef bvector<AccessStringIndexPair>                          IndicesByAccessString;
    typedef bvector<PropertyLayoutP>                                PropertyLayoutVector;
    typedef bmap<uint32_t, bvector<uint32_t> >                          LogicalStructureMap;

    enum State
        {
        AcceptingFixedSizeProperties,
        AcceptingVariableSizeProperties,
        Closed
        };

    // These members are expected to be persisted
    Utf8String              m_className;

    PropertyLayoutVector    m_propertyLayouts;      // This is the primary collection, there is a secondary map for lookup by name, below.
    IndicesByAccessString   m_indicesByAccessString; // Always sorted; maps access strings to indices which can be used to index into PropertyLayoutVector
    LogicalStructureMap     m_logicalStructureMap;

    // These members are transient
    uint32_t                          m_sizeOfFixedSection;
    bool                              m_isRelationshipClass;         // make this a bitmask IsRelationshipClass, OnPartialInstanceLoaded (this should allow roundtrip native-managed-native
    int                               m_propertyIndexOfSourceECPointer;
    int                               m_propertyIndexOfTargetECPointer;
    int                               m_uniqueId;
    mutable CompatibleClassLayoutsMap m_compatibleClassLayouts;
    mutable uint32_t                  m_checkSum;

    void                            CheckForECPointers (Utf8CP accessString);

    // returns an iterator into m_indicesByAccessStrings at which accessString exists (if forCreate=false), or would be inserted (if forCreate=true)
    IndicesByAccessString::const_iterator GetPropertyIndexPosition (Utf8CP accessString, bool forCreate) const;
    uint32_t ComputeCheckSum () const;

    struct  Factory
    {
    friend struct ClassLayout;

    private:
        State           m_state;
        ECClassCR       m_ecClass;
        uint32_t        m_offset;
        uint32_t        m_nullflagsOffset;
        uint32_t        m_nonStructPropertyCount;
        ClassLayoutPtr  m_underConstruction;

        uint32_t    GetParentStructIndex (Utf8CP accessString) const;

        void        AddProperty (Utf8CP accessString, ECTypeDescriptor propertyDescriptor, uint32_t size, uint32_t modifierFlags = 0, uint32_t modifierData = 0);
        void        AddStructProperty (Utf8CP accessString, ECTypeDescriptor propertyDescriptor);
        void        AddFixedSizeProperty (Utf8CP accessString, ECTypeDescriptor propertyDescriptor, bool isReadOnly, bool isCalculated);
        void        AddFixedSizeArrayProperty (Utf8CP accessString, ECTypeDescriptor propertyDescriptor, uint32_t arrayCount, bool isReadOnly, bool isCalculated);
        void        AddVariableSizeProperty (Utf8CP accessString, ECTypeDescriptor propertyDescriptor, bool isReadOnly, bool isCalculated);
        void        AddVariableSizeArrayPropertyWithFixedCount (Utf8CP accessString, ECTypeDescriptor typeDescriptor, uint32_t arrayCount, bool isReadOnly, bool isCalculated);
        void        AddProperties (ECClassCR ecClass, Utf8CP nameRoot, bool addFixedSize);

        Factory (ECClassCR ecClass);
        ClassLayoutPtr DoBuildClassLayout ();
    };

    ClassLayout();
    virtual ~ClassLayout();

    Utf8String              GetShortDescription() const;
    Utf8String              LogicalStructureToString (uint32_t parentStructIndex = 0, uint32_t indentLevel = 0) const;
    BentleyStatus          SetClass (Utf8CP  className);

public:
    Utf8String              GetName() const;
    int                     GetUniqueId() const;
    void                    AddPropertyLayout (Utf8CP accessString, PropertyLayoutR);
    void                    AddToLogicalStructureMap (PropertyLayoutR propertyLayout, uint32_t propertyIndex);
    ECOBJECTS_EXPORT void   InitializeMemoryForInstance(Byte * data, uint32_t bytesAllocated) const;
    ECOBJECTS_EXPORT uint32_t GetSizeOfFixedSection() const;

    ECOBJECTS_EXPORT uint32_t               GetFirstChildPropertyIndex (uint32_t parentIndex) const;
    ECOBJECTS_EXPORT uint32_t               GetNextChildPropertyIndex (uint32_t parentIndex, uint32_t childIndex) const;
    ECOBJECTS_EXPORT ECObjectsStatus        GetAccessStringByIndex (Utf8CP& accessString, uint32_t propertyIndex) const;
    ECOBJECTS_EXPORT ECObjectsStatus        GetPropertyIndices (bvector<uint32_t>& properties, uint32_t parentIndex) const;
    ECOBJECTS_EXPORT bool                   HasChildProperties (uint32_t parentIndex) const;
    ECOBJECTS_EXPORT uint32_t               GetParentPropertyIndex (uint32_t childIndex) const;

    // Returns true if this ClassLayout is equivalent to the other ClassLayout (checks name and checksum)
    ECOBJECTS_EXPORT bool                   Equals (ClassLayoutCR other, bool compareNames = true) const;

    ECOBJECTS_EXPORT void                   AddPropertyDirect (Utf8CP accessString, uint32_t parentStructIndex, ECTypeDescriptor typeDescriptor, uint32_t offset, uint32_t nullflagsOffset, uint32_t nullflagsBitmask, uint32_t modifierFlags, uint32_t modifierData);
    ECOBJECTS_EXPORT ECObjectsStatus        FinishLayout ();
    ECOBJECTS_EXPORT ClassLayoutPtr         Clone (Utf8CP name = nullptr) const;
    ECOBJECTS_EXPORT void                   SetPropertyLayoutModifierData (PropertyLayoutCR layout, uint32_t modifierData);
//__PUBLISH_CLASS_VIRTUAL__
/*__PUBLISH_SECTION_START__*/
private:
    //ClassLayout (){}

public:

    //! Given a class, will create the ClassLayout that manages that class
    //! @param[in] ecClass  The ECClass to build the ClassLayout from
    //! @returns ClassLayout pointer managing this ECClass
    ECOBJECTS_EXPORT static ClassLayoutPtr BuildFromClass (ECClassCR ecClass);

    //! Creates an empty ClassLayout for a class with the given name
    //! @param[in] className    The name of the class that this ClassLayout will define
    ECOBJECTS_EXPORT static ClassLayoutPtr CreateEmpty    (Utf8CP  className);

    //! Returns the name of the ECClass that this ClassLayout manages
    ECOBJECTS_EXPORT Utf8String const & GetECClassName() const; 

    //! Returns the property index of the given relationship end
    //! @param[in] end  The ECRelationshipEnd for which to get the pointer index
    //! @returns The property index of the pointer for the given relationship end
    ECOBJECTS_EXPORT int            GetECPointerIndex (ECRelationshipEnd end) const;
    
    //! Returns the checksum for this ClassLayout
    ECOBJECTS_EXPORT uint32_t        GetChecksum () const;

    //! Returns the number of properties this ClassLayout manages
    ECOBJECTS_EXPORT uint32_t        GetPropertyCount () const;

    //! Returns the number of properties this ClassLayout manages, not counting embedded structs
    ECOBJECTS_EXPORT uint32_t        GetPropertyCountExcludingEmbeddedStructs () const;

    //! Given a property access string, will return the PropertyLayout
    //! @param[out] propertyLayout  Will point to the PropertyLayout if found
    //! @param[in]  accessString    The access string for the desired property
    //! @returns ECObjectsStatus::PropertyNotFound if the property is not found, ECObjectsStatus::Success otherwise
    ECOBJECTS_EXPORT ECObjectsStatus GetPropertyLayout (PropertyLayoutCP & propertyLayout, Utf8CP accessString) const;

    //! Given a property index, will return the PropertyLayout
    //! @param[out] propertyLayout  Will point to the PropertyLayout if found
    //! @param[in]  propertyIndex   The index in the ClassLayout of the desired property
    //! @returns ECObjectsStatus::PropertyNotFound if the property is not found, ECObjectsStatus::Success otherwise
    ECOBJECTS_EXPORT ECObjectsStatus GetPropertyLayoutByIndex (PropertyLayoutCP & propertyLayout, uint32_t propertyIndex) const;

    //! Given a property layout, will return the property index within the ClassLayout
    //! @param[out] propertyIndex   Will contain the index of the given property within the ClassLayout, if found
    //! @param[in]  propertyLayout  The propertyLayout of the property that we want the index for
    //! @returns ECObjectsStatus::PropertyNotFound if the property is not found, ECObjectsStatus::Success otherwise
    ECOBJECTS_EXPORT ECObjectsStatus GetPropertyLayoutIndex (uint32_t& propertyIndex, PropertyLayoutCR propertyLayout) const;

    //! Given an access string, will return the property index within the ClassLayout
    //! @param[out] propertyIndex   Will contain the index of the given property within the ClassLayout, if found
    //! @param[in]  accessString    The access string for the desired property
    //! @returns ECObjectsStatus::PropertyNotFound if the property is not found, ECObjectsStatus::Success otherwise
    ECOBJECTS_EXPORT ECObjectsStatus GetPropertyIndex (uint32_t& propertyIndex, Utf8CP accessString) const;

    //! Given a propertyIndex, will return whether the property is read-only or not
    //! @param[in]  propertyIndex   The index within the ClassLayout of the property to check
    //! @returns true if the property is read-only, false otherwise
    ECOBJECTS_EXPORT bool            IsPropertyReadOnly (uint32_t propertyIndex) const;

    //! Sets the read-only status of a property, given its index within the ClassLayout
    //! @param[in]  propertyIndex   The index within the ClassLayout of the property to set the read-only flag for
    //! @param[in]  readOnly        Flag indicating whether this property is read-only
    //! @returns The value of readOnly if successfully set, otherwise false
    ECOBJECTS_EXPORT bool            SetPropertyReadOnly (uint32_t propertyIndex, bool readOnly) const;

    //! Determines the number of bytes used for property data, so far
    ECOBJECTS_EXPORT uint32_t       CalculateBytesUsed(Byte const * propertyData) const;

    //! Checks the given classLayout to see if it is equal to, or a subset of, this layout
    //! @param[in]  layout  The ClassLayout to test compatibility of
    //! @returns true if the given ClassLayout is equal to or a subset of this layout, false otherwise
    ECOBJECTS_EXPORT bool           IsCompatible(ClassLayoutCR layout) const;

    //! Returns a string containing a description of the class and its properties
    ECOBJECTS_EXPORT Utf8String       ToString() const;
    };

typedef RefCountedPtr<ClassLayout>  ClassLayoutPtr;
typedef bvector<ClassLayoutPtr>     ClassLayoutVector;

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

    ClassLayoutP                            GetClassLayoutP (ClassIndex classIndex) { return const_cast<ClassLayoutP>(GetClassLayout(classIndex)); }

    ECOBJECTS_EXPORT ClassLayoutCP           FindClassLayout (Utf8CP className, ClassIndex* classIndex) const;
//__PUBLISH_CLASS_VIRTUAL__
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
    ECOBJECTS_EXPORT BentleyStatus          AddClassLayout (ClassLayoutR classLayout, ClassIndex classIndex);

    //! Returns the ClassLayout at the given index
    //! @param[in]  classIndex  The index of the desired ClassLayout
    //! @returns A pointer to the requested ClassLayout if the index is valid, NULL otherwise
    ECOBJECTS_EXPORT ClassLayoutCP          GetClassLayout (ClassIndex classIndex) const;

    //! Given a classname, tries to find the corresponding ClassLayout
    //! @param[in]  className   The name of the class to find
    //! @returns A pointer to the corresponding ClassLayout if found, NULL otherwise
    ECOBJECTS_EXPORT ClassLayoutCP          FindClassLayout (Utf8CP className) const;

    //! Given a classname, tries to find the index of the corresponding ClassLayout
    //! @param[out] classIndex  The index of the corresponding ClassLayout, if found
    //! @param[in]  className   The name of the class to find the ClassLayout index of
    //! @returns SUCCESS if the ClassLayout is found, ERROR otherwise
    ECOBJECTS_EXPORT BentleyStatus          FindClassIndex (ClassIndex& classIndex, Utf8CP className) const;

    //! Finds the first available index for adding a class layout
    //! @param[out] classIndex  The first available index in the layout.  This is not necessarily the size of the layout because there can be gaps.
    //! @returns SUCCESS if the index is found, ERROR otherwise
    ECOBJECTS_EXPORT BentleyStatus          FindAvailableClassIndex (ClassIndex& classIndex) const;
   
    //! Returns the max index in the ClassLayout
    //! @remarks This may often correspond to "number of ClassLayouts - 1", but not necessarily, because there can be gaps
    //!          so when you call GetClassLayout (index) you might get NULLs. Even the last one could be NULL.
    //! @remarks NOTE: Check IsEmpty() before GetMaxIndex() to ensure there is at least one ClassLayout, otherwise the return value of GetMaxIndex() is undefined
    ECOBJECTS_EXPORT uint32_t               GetMaxIndex () const;

    //! Returns whether or not this SchemaLayout has any ClassLayouts defined.
    ECOBJECTS_EXPORT bool                   IsEmpty() const;

    //! Creates a new SchemaLayout object
    //! @param[in]  index   The SchemaIndex used to identify this SchemaLayout
    //! @remarks It is up to the calling application to ensure uniqueness for the SchemaIndex.
    ECOBJECTS_EXPORT static SchemaLayoutP   Create (SchemaIndex index);
};

/*__PUBLISH_SECTION_END__*/

//! An internal helper used by ECDBuffer to resize (add/remove elements) array property values
struct      ArrayResizer
    {
    friend struct ECDBuffer;

private:
    ClassLayoutCR           m_classLayout;
    PropertyLayoutCR        m_propertyLayout;
    ECDBufferR  m_instance;

    uint32_t        m_arrayOffset;

    uint32_t        m_resizeIndex;
    uint32_t        m_resizeElementCount;
    uint32_t        m_resizeFixedSectionByteCount; // bytesNeeded

    PrimitiveType   m_elementType;
    bool            m_elementTypeIsFixedSize;
    uint32_t        m_elementSizeInFixedSection;

    // state variables for pre resize
    ArrayCount      m_preAllocatedArrayCount;
    uint32_t        m_preNullFlagBitmasksCount;
    uint32_t        m_preHeaderByteCount;
    uint32_t        m_preFixedSectionByteCount;
    uint32_t        m_preArrayByteCount;

    // state variables for post resize
    ArrayCount      m_postAllocatedArrayCount;
    uint32_t        m_postNullFlagBitmasksCount;
    uint32_t        m_postHeaderByteCount;
    uint32_t        m_postFixedSectionByteCount;
    SecondaryOffset m_postSecondaryOffsetOfResizeIndex;

    // pointers to memory that must be shifted during resize
    Byte const *    m_pResizeIndexPreShift;
    Byte const *    m_pResizeIndexPostShift;

    Byte const *    m_propertyData;

    ArrayResizer (ClassLayoutCR classLayout, PropertyLayoutCR propertyLayout, ECDBufferR instance, uint32_t resizeIndex, uint32_t resizeElementCount);

    ECObjectsStatus       ShiftDataFollowingResizeIndex ();
    ECObjectsStatus       SetSecondaryOffsetsFollowingResizeIndex ();
    ECObjectsStatus       ShiftDataPreceedingResizeIndex ();
    ECObjectsStatus       SetSecondaryOffsetsPreceedingResizeIndex (SecondaryOffset* pSecondaryOffset, uint32_t byteCountToSet);
    ECObjectsStatus       WriteArrayHeader ();

    static ECObjectsStatus    CreateNullArrayElementsAt (ClassLayoutCR classLayout, PropertyLayoutCR propertyLayout, ECDBufferR instance, uint32_t insertIndex, uint32_t insertCount);
    };

enum ECDFormatVersion ENUM_UNDERLYING_TYPE(uint8_t)
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

enum ECDFlags ENUM_UNDERLYING_TYPE(uint8_t)
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
    uint8_t         m_formatVersion;
    // The minimum version of ECD format which is read-compatible with the data in this buffer. Always less than or equal to m_formatVersion
    // Code compiled for an ECD format version less than m_readableByVersion should not attempt to read the buffer. All other code can safely read the buffer.
    uint8_t         m_readableByVersion;
    // The minimum version of ECD format which is write-compatible with the data in this buffer. Always less than or equal to m_formatVersion, always greater than or equal to m_readableByVersion.
    // Code compiled for an ECD format version less than m_writableByVersion should not attempt to modify the buffer.
    // All other code can safely modify the buffer, with the caveat that the persisted header is kept intact, aside from modifications to data understood by the compiled ECD format version.
    uint8_t         m_writableByVersion;
    // The size in bytes of this header. Adding this value to the base address of the buffer yields the beginning of the property data.
    uint8_t         m_headerSize;
    // A set of flags. See ECDFlags
    uint8_t         m_flags;
    // Additional data can be appended here in future versions by creating a subclass of ECDHeader_v0
public:
    // Construct header with default string encoding
    ECOBJECTS_EXPORT ECDHeader_v0();
    // Construct header with specified string encoding
    ECOBJECTS_EXPORT ECDHeader_v0 (bool useUtf8Encoding);

    // Read header from persisted ECData. Returns false if no ECDHeader could be extracted.
    ECOBJECTS_EXPORT static bool    ReadHeader (ECDHeader_v0& header, Byte const* data);

    bool                GetFlag (ECDFlags flag) const       { return 0 != (m_flags & flag); }
    void                SetFlag (ECDFlags flag, bool set)   { m_flags = set ? (m_flags | flag) : (m_flags & ~flag); }
    bool                IsReadable() const                  { return ECDFormat_Current >= m_readableByVersion; }
    bool                IsWritable() const                  { return ECDFormat_Current >= m_writableByVersion; }
    uint8_t             GetFormatVersion() const            { return m_formatVersion; }
    uint8_t             GetSize() const                     { return m_headerSize; }
    };
#pragma pack(pop)

typedef ECDHeader_v0 ECDHeader;

/*__PUBLISH_SECTION_START__*/
//=======================================================================================
//! Base class for ECN::IECInstance implementations that get/set values from a block of memory,
//! e.g. StandaloneECInstance and ECXInstance
//! @bsiclass
//=======================================================================================
struct ECDBuffer
    {
    friend  struct ArrayResizer;
    friend  struct ECDBufferScope;
private:
    mutable bool        m_allowWritingDirectlyToInstanceMemory;
    mutable bool        m_allPropertiesCalculated;

//__PUBLISH_CLASS_VIRTUAL__
/*__PUBLISH_SECTION_END__*/

    uint32_t            GetOffsetToPropertyData() const;

    //! Returns the offset of the property value relative to the start of the instance data.
    //! If useIndex is true then the offset of the array element value at the specified index is returned.
    uint32_t            GetOffsetOfPropertyValue (PropertyLayoutCR propertyLayout, bool useIndex = false, uint32_t index = 0) const;

    //! Returns the size in bytes of the property value
    uint32_t            GetPropertyValueSize (PropertyLayoutCR propertyLayout) const;
    //! Returns the size in bytes of the array element value at the specified index
    uint32_t            GetPropertyValueSize (PropertyLayoutCR propertyLayout, uint32_t index) const;

    //! Returns the address of the property value
    Byte const *        GetAddressOfPropertyValue (PropertyLayoutCR propertyLayout) const;
    //! Returns the address of the array element value at the specified index
    Byte const *        GetAddressOfPropertyValue (PropertyLayoutCR propertyLayout, uint32_t index) const;

    //! Returns the offset of the specified array index relative to the start of the instance data.
    //! Note that this offset does not necessarily contain the index value.  If the element type is fixed it contains the value but if it is a variable
    //! size type then the index contains a secondary offset.  If you need the offset of the actual element value then use GetOffsetOfArrayIndexValue
    //! This method does not do any parameter checking.  It is the responsibility of the caller to ensure the property is an array and the index is in
    //! a valid range.
    uint32_t            GetOffsetOfArrayIndex (uint32_t arrayOffset, PropertyLayoutCR propertyLayout, uint32_t index) const;

    //! Returns the offset of the specified array index value relative to the start of the instance data.
    //! This method does not do any parameter checking.  It is the responsibility of the caller to ensure the property is an array and the index is in
    //! a valid range.
    uint32_t            GetOffsetOfArrayIndexValue (uint32_t arrayOffset, PropertyLayoutCR propertyLayout, uint32_t index) const;

    //! Returns true if the property value is null; otherwise false
    //! If nIndices is > 0 then the null check is based on the array element at the specified index
    bool                IsPropertyValueNull (PropertyLayoutCR propertyLayout, bool useIndex = false, uint32_t index = 0) const;

    //! Sets the null bit of the specified property to the value indicated by isNull
    //! If nIndices is > 0 then the null bit is set for the array element at the specified index
    void                SetPropertyValueNull (PropertyLayoutCR propertyLayout, bool useIndex, uint32_t index, bool isNull);

    //! Shifts the values' data and adjusts SecondaryOffsets for all variable-sized property values
    //! AFTER the given one, to make room for additional bytes needed for the property value of the given PropertyLayout
    //! or to "compact" to reclaim unused space.
    //! @param data           Start of the data of the ECDBuffer
    //! @param bytesAllocated How much memory is allocated for the data
    //! @param propertyLayout PropertyLayout of the variable-sized property whose size is increasing
    //! @param shiftBy        Positive or negative! Memory will be moved and SecondaryOffsets will be adjusted by this amount
    ECObjectsStatus                   ShiftValueData(Byte * data, uint32_t bytesAllocated, PropertyLayoutCR propertyLayout, int32_t shiftBy);
    ECObjectsStatus                   ShiftArrayIndexValueData(PropertyLayoutCR propertyLayout, uint32_t arrayIndex, uint32_t arrayCount,  uint32_t endOfValueDataOffset, int32_t shiftBy);

    ECObjectsStatus                   EnsureSpaceIsAvailable (uint32_t& offset, PropertyLayoutCR propertyLayout, uint32_t bytesNeeded);
    ECObjectsStatus                   EnsureSpaceIsAvailableForArrayIndexValue (PropertyLayoutCR propertyLayout, uint32_t arrayIndex, uint32_t bytesNeeded);
    ECObjectsStatus                   GrowPropertyValue (PropertyLayoutCR propertyLayout, uint32_t additionalbytesNeeded);
    // Updates the calculated value in memory and returns the updated value in existingValue
    ECObjectsStatus                   EvaluateCalculatedProperty (PropertyLayoutCR propertyLayout, ECValueR existingValue, bool useArrayIndex, uint32_t arrayIndex) const;

    ECObjectsStatus                   SetPrimitiveValueToMemory (ECValueCR v, PropertyLayoutCR propertyLayout, bool useIndex, uint32_t index, bool alreadyCalculated);

    ECObjectsStatus                   ModifyData (Byte const* data, void const* newData, size_t dataLength);
    ECObjectsStatus                   ModifyData (uint32_t const* data, uint32_t newData);
    ECObjectsStatus                   MoveData (Byte* to, Byte const* from, size_t dataLength);

    // These methods are for internal use by ECDBuffer only. They should be used within a single scope and *must* be paired. ScopedDataAccessor helps ensure this.
    friend struct ScopedDataAccessor;
    virtual bool                      _AcquireData (bool forWrite) const = 0;
    virtual bool                      _ReleaseData() const = 0;
protected:
    ECOBJECTS_EXPORT ECDHeader const*       GetECDHeaderCP() const;

    //! Returns the number of elements in the specified array that are currently allocated in the instance data memory block.
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
    ECOBJECTS_EXPORT ECObjectsStatus  GetPrimitiveValueFromMemory (ECValueR v, PropertyLayoutCR propertyLayout, bool useIndex = false, uint32_t index = 0) const;    
    //! Sets the primitive value for the specified property
    //! If nIndices is > 0 then the primitive value for the array element at the specified index is set.
    //! This is protected because implementors of _SetStructArrayArrayValueToMemory should call it to store a binary primitive value that can be retrieved
    //! when _GetStructArrayValueFromMemory is invoked as a means to locate the externalized struct array value.
    ECOBJECTS_EXPORT ECObjectsStatus  SetPrimitiveValueToMemory   (ECValueCR v, PropertyLayoutCR propertyLayout, bool useIndex = false, uint32_t index = 0);    
    ECOBJECTS_EXPORT ECObjectsStatus  GetValueFromMemory (ECValueR v, PropertyLayoutCR propertyLayout) const;
    ECOBJECTS_EXPORT ECObjectsStatus  GetValueFromMemory (ECValueR v, PropertyLayoutCR propertyLayout, uint32_t index) const;    
    ECOBJECTS_EXPORT ECObjectsStatus  GetValueFromMemory (ECValueR v, uint32_t propertyIndex, bool useArrayIndex = false, uint32_t arrayIndex = 0) const;
    ECOBJECTS_EXPORT ECObjectsStatus  SetValueToMemory (ECValueCR v, PropertyLayoutCR propertyLayout);          
    ECOBJECTS_EXPORT ECObjectsStatus  SetValueToMemory (ECValueCR v, PropertyLayoutCR propertyLayout, uint32_t index);              
    ECOBJECTS_EXPORT ECObjectsStatus  SetInternalValueToMemory (PropertyLayoutCR propertyLayout, ECValueCR v, bool useIndex = false, uint32_t index = 0);
    ECOBJECTS_EXPORT ECObjectsStatus  SetValueToMemory (uint32_t propertyIndex, ECValueCR v, bool useArrayIndex = false, uint32_t arrayIndex = 0);      
    ECOBJECTS_EXPORT ECObjectsStatus  InsertNullArrayElementsAt (uint32_t propIdx, uint32_t insertIndex, uint32_t insertCount);
    ECOBJECTS_EXPORT ECObjectsStatus  AddNullArrayElementsAt (uint32_t propIdx, uint32_t insertCount);
    ECOBJECTS_EXPORT ECObjectsStatus  RemoveArrayElementsFromMemory (PropertyLayoutCR propertyLayout, uint32_t removeIndex, uint32_t removeCount);
    ECOBJECTS_EXPORT ECObjectsStatus  RemoveArrayElementsAt (uint32_t propIdx, uint32_t removeIndex, uint32_t removeCount);
    ECOBJECTS_EXPORT ECObjectsStatus  ClearArrayElementsFromMemory (uint32_t propIdx);
    ECOBJECTS_EXPORT Utf8String       InstanceDataToString (Utf8CP indent) const;
    ECOBJECTS_EXPORT ECObjectsStatus  GetIsNullValueFromMemory (bool& isNull, uint32_t propertyIndex, bool useIndex, uint32_t index) const;

    ECOBJECTS_EXPORT ECObjectsStatus  CopyPropertiesFromBuffer (ECDBufferCR src);

    virtual ~ECDBuffer () {}

    //! Sets the in-memory value of the array index of the specified property to be the struct value as held by v
    //! Since struct arrays support polymorphic values, we do not support storing the full struct value in the data section of the instance.  It must be externalized, therefore
    //! it is implementation specific and left to the implementation of the instance to store the value.  Before returning, the implementation should call into SetPrimitiveValueToMemory
    //! with a binary token that will actually be stored with the instance at the array index value that can then be used to locate the externalized struct value.
    //! Note that top-level struct properties will automatically be stored in the data section of the instance.  It is only struct array values that must be stored
    //! externally.
    //! If isInitializing is true, we are in the process of copying ECD memory buffer from another ECDBuffer, in which case this buffer's struct value ID entries are not valid.
    virtual ECObjectsStatus         _SetStructArrayValueToMemory (ECValueCR v, PropertyLayoutCR propertyLayout, uint32_t index) = 0;
    virtual ECObjectsStatus         _GetStructArrayValueFromMemory (ECValueR v, PropertyLayoutCR propertyLayout, uint32_t index) const = 0;
    virtual ECN::PrimitiveType      _GetStructArrayPrimitiveType () const = 0;

    virtual ECObjectsStatus         _RemoveStructArrayElementsFromMemory (PropertyLayoutCR propertyLayout, uint32_t removeIndex, uint32_t removeCount) = 0;
    virtual bool                    _IsStructValidForArray (IECInstanceCR structInstance, PropertyLayoutCR propLayout) const = 0;

    //! Invoked after a variable-sized array is successfully resized, to allow derived classes to adjust their internal state in response if necessary.
    virtual void                    _HandleArrayResize (PropertyLayoutCP propertyLayout, uint32_t atIndex, int32_t countDelta) { };

    virtual bool                   _IsMemoryInitialized () const = 0;

    //! Get a pointer to the first byte of the ECDBuffer's data. This points to the first byte of the ECDHeader
    virtual Byte const *          _GetData () const = 0;
    virtual ECObjectsStatus       _ModifyData (uint32_t offset, void const * newData, uint32_t dataLength) = 0;
    virtual ECObjectsStatus       _MoveData (uint32_t toOffset, uint32_t fromOffset, uint32_t dataLength) = 0;
    virtual uint32_t              _GetBytesAllocated () const = 0;

    //! Reallocates memory for the IECInstance and copies the old IECInstance data into the new memory
    //! You might get more memory than used asked for, but you won't get less
    //! @param additionalBytesNeeded  Additional bytes of memory needed above current allocation
    virtual ECObjectsStatus      _GrowAllocation (uint32_t additionalBytesNeeded) = 0;

    //! Shrinks the allocated IECInstance data to be as small as possible
    virtual ECObjectsStatus    _ShrinkAllocation () = 0;

    //! Free any allocated memory
    virtual void                _FreeAllocation () = 0;

    virtual void                _SetPerPropertyFlag (PropertyLayoutCR propertyLayout, bool useIndex, uint32_t index, int flagIndex, bool enable) {};

    virtual void                _ClearValues() = 0;
    virtual ECObjectsStatus     _CopyFromBuffer (ECDBufferCR source) = 0;

    virtual ClassLayoutCR       _GetClassLayout() const = 0;

    virtual ECObjectsStatus     _EvaluateCalculatedProperty (ECValueR evaluatedValue, ECValueCR existingValue, PropertyLayoutCR propLayout) const = 0;
    virtual ECObjectsStatus     _UpdateCalculatedPropertyDependents (ECValueCR calculatedValue, PropertyLayoutCR propLayout) = 0;
    ECOBJECTS_EXPORT virtual ECObjectsStatus    _SetCalculatedValueToMemory (ECValueCR v, PropertyLayoutCR propertyLayout, bool useIndex, uint32_t index) const;

    virtual bool                _IsPersistentlyReadOnly() const { return false; }
    virtual ECObjectsStatus     _SetIsPersistentlyReadOnly (bool readOnly) { return ECObjectsStatus::OperationNotSupported; }
    virtual bool                _IsHidden() const { return false; }
    virtual ECObjectsStatus     _SetIsHidden (bool hidden) { return ECObjectsStatus::OperationNotSupported; }

    // Helper for implementations of calculated property methods above
    ECOBJECTS_EXPORT CalculatedPropertySpecificationCP LookupCalculatedPropertySpecification (IECInstanceCR thisAsIECInstance, PropertyLayoutCR propLayout) const;

    // Get a pointer to the first byte of the ECDBuffer's property data. This is the first byte beyond the ECDHeader.
    ECOBJECTS_EXPORT Byte const*    GetPropertyData() const;

public:
    ECOBJECTS_EXPORT ECObjectsStatus  GetValueFromMemory (ECValueR v, Utf8CP propertyAccessString, bool useIndex = false, uint32_t index = 0) const;
    ECOBJECTS_EXPORT ECObjectsStatus  SetValueToMemory (Utf8CP propertyAccessString, ECValueCR v,  bool useIndex = false, uint32_t index = 0);      
    //! Returns the number of bytes which must be allocated to store the header + the fixed portion of the property data, using ECDFormat_Current
    ECOBJECTS_EXPORT uint32_t               CalculateBytesUsed () const;
    ECOBJECTS_EXPORT ECObjectsStatus        GetStructArrayValueFromMemory (ECValueR v, PropertyLayoutCR propertyLayout, uint32_t index, int* structValueIdentifier = NULL) const;

    //! Returns the number of elements in the specfieid array that are currently reserved but not necessarily allocated.
    //! This is important when an array has a minimum size but has not yet been initialized.  We delay initializing the memory for the minimum # of elements until
    //! the first value is set.  If an array does not have a minimum element count then GetReservedArrayCount will always equal GetAllocatedArrayCount
    //! This value is always >= the value returned by GetAllocatedArrayCount.
    //! This is the value used to set the count on an ArrayInfo value object that will be returned to a caller via GetValueFromMemory.  It is an implementation detail
    //! of memory based instances as to whether or not the physical memory to back that array count has actually been allocated.
    ECOBJECTS_EXPORT ArrayCount             GetReservedArrayCount (PropertyLayoutCR propertyLayout) const;

    // Copies the data from the specified ECDBuffer into this ECDBuffer, converting to match this buffer's ClassLayout if necessary.
    // Does not copy struct array instances, only their identifiers. In general CopyFromBuffer() should be used instead.
    ECOBJECTS_EXPORT ECObjectsStatus        CopyDataBuffer (ECDBufferCR src, bool allowClassLayoutConversion);
    ECOBJECTS_EXPORT ClassLayoutCR          GetClassLayout () const;
    ECOBJECTS_EXPORT ECObjectsStatus        RemoveArrayElements (PropertyLayoutCR propertyLayout, uint32_t removeIndex, uint32_t removeCount);
   
    ECOBJECTS_EXPORT void                   SetPerPropertyFlag (PropertyLayoutCR propertyLayout, bool useIndex, uint32_t index, int flagIndex, bool enable);
    ECOBJECTS_EXPORT ECN::PrimitiveType     GetStructArrayPrimitiveType () const;

    // Compress the memory storing the data to as small a size as possible
    ECOBJECTS_EXPORT ECObjectsStatus        Compress();
    // Calculate how many bytes are required for an empty buffer using the specified classLayout
    ECOBJECTS_EXPORT static uint32_t        CalculateInitialAllocation (ClassLayoutCR classLayout);
    // Initialize a block of memory for a new ECDBuffer. If forceUtf8 is true, all strings in the buffer will be stored as Utf8; otherwise the default string encoding will be used.
    ECOBJECTS_EXPORT static void            InitializeMemory(ClassLayoutCR classLayout, Byte* data, uint32_t bytesAllocated, bool forceUtf8 = false);

    //! Given a persisted ECD buffer, returns true if the data is compatible with ECDFormat_Current.
    //! This method must be called before attempting to instantiate an ECD-based instance from a block of persistent memory.
    //! If it returns false, the calling method must not attempt to instantiate the instance.
    //! @param[out] header          Optional; if non-null, will contain a copy of the persistent ECDHeader
    //! @param[in]  data            The ECD-formatted block of memory
    //! @param[in]  requireWritable If true, the method will return false if the block of memory is not write-compatible with ECDFormat_Current.
    //! @return     true if the memory is read-compatible (if requireWritable=false) or write-compatible(if requireWritable=true) with ECDFormat_Current
    ECOBJECTS_EXPORT static bool            IsCompatibleVersion (ECDHeader* header, Byte const* data, bool requireWritable = false);

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
    ECOBJECTS_EXPORT void                   GetBufferData (Byte* dest) const;

    // Evaluates all calculated property values in the buffer. Recurses into struct array members. Does not re-evaluate default value specifications if value has been overridden
    ECOBJECTS_EXPORT bool                   EvaluateAllCalculatedProperties ();
    ECOBJECTS_EXPORT bool                   EvaluateAllCalculatedProperties (bool includeDefaultValueSpecifications);

    ECOBJECTS_EXPORT bool                   IsPersistentlyReadOnly() const;
    ECOBJECTS_EXPORT ECObjectsStatus        SetIsPersistentlyReadOnly (bool readOnly);
    ECOBJECTS_EXPORT bool                   IsHidden() const;
    ECOBJECTS_EXPORT ECObjectsStatus        SetIsHidden (bool hidden);
/*__PUBLISH_SECTION_START__*/  
public:
    //! Returns true if the buffer is empty (all values are null and all arrays are empty)
    ECOBJECTS_EXPORT bool                   IsEmpty() const;
    //! Sets all values to null
    ECOBJECTS_EXPORT void                   ClearValues();
    //! Attempts to copy property values from source buffer. Expects source to have a compatible class layout.
    //! @param[in] source The ECDBuffer to copy values from
    ECOBJECTS_EXPORT ECObjectsStatus        CopyFromBuffer (ECDBufferCR source);
    };

/*__PUBLISH_SECTION_END__*/

/*---------------------------------------------------------------------------------**//**
* "Pins" the internal memory buffer used by the ECDBuffer such that:
*   a) all calculated property values will be exactly once, when scope is constructed; and
*   b) addresses of all property values will not change for lifetime of scope.
* To be used in conjunction with ECValue::SetAllowsPointersIntoInstanceMemory() to ensure
* pointers remain valid for lifetime of scope.
* @bsistruct                                                    Paul.Connelly   07/14
+---------------+---------------+---------------+---------------+---------------+------*/
struct ECDBufferScope
    {
private:
    ECDBufferCP         m_buffer;
    bool                m_initialState;

    void*               operator new (size_t);
    void*               operator new[] (size_t);
public:
    //! Use the default constructor in combination with Init if 
    //! it is not clear at construction time whether the evaluation of calculated
    //! properties should be done or not.
    ECOBJECTS_EXPORT ECDBufferScope ();
    ECOBJECTS_EXPORT explicit ECDBufferScope (ECDBufferCR buffer);
    ECOBJECTS_EXPORT explicit ECDBufferScope (IECInstanceCR instance);
    ECOBJECTS_EXPORT void Init (ECDBufferCP buffer);

    ECOBJECTS_EXPORT ~ECDBufferScope();
    };

/*---------------------------------------------------------------------------------**//**
* Exposed here solely for interop with managed code...
* @bsistruct                                                    Paul.Connelly   11/12
+---------------+---------------+---------------+---------------+---------------+------*/
struct ScopedDataAccessor
    {
private:
    ECDBuffer const*            m_buffer;
public:
    ScopedDataAccessor (ECDBuffer const& buffer, bool forWrite = false) : m_buffer(buffer._AcquireData (forWrite) ? &buffer : NULL) { }
    ~ScopedDataAccessor ()
        {
        if (IsValid())
            m_buffer->_ReleaseData();
        }
    bool            IsValid() const { return NULL != m_buffer; }
    Byte const*     GetData() const { return NULL != m_buffer ? m_buffer->_GetData() : NULL; }
    };

/*__PUBLISH_SECTION_START__*/
/** @endGroup */
END_BENTLEY_ECOBJECT_NAMESPACE

/** @endcond */
