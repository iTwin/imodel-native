/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicApi/ECObjects/MemoryInstanceSupport.h $
|
|   $Copyright: (c) 2010 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
//__PUBLISH_SECTION_START__
#pragma once

#include <ECObjects\ECObjects.h>

#define N_FINAL_STRING_PROPS_IN_FAKE_CLASS 48

EC_TYPEDEFS(MemoryInstanceSupport);

BEGIN_BENTLEY_EC_NAMESPACE
    
typedef UInt32 NullflagsBitmask;
typedef UInt32 InstanceFlags;
typedef UInt32 SecondaryOffset;

#define NULLFLAGS_BITMASK_AllOn     0xFFFFFFFF

/*=================================================================================**//**
* @bsistruct                                                     CaseyMullen    10/09
+===============+===============+===============+===============+===============+======*/      
struct PropertyLayout
    {
friend ClassLayout;    
private:
    std::wstring        m_accessString;
    DataType            m_dataType;
    
    // Using UInt32 instead of size_t below because we will persist this struct in an XAttribute. It will never be very big.
    UInt32              m_offset; //! An offset to either the data holding that property’s value (for fixed-size values) or to the offset at which the properties value can be found.
  //UInt32              m_modifierFlags //! Can be used to indicate that a string should be treated as fixed size, with a max length, or that a longer fixed size type should be treated as an optional variable-sized type, or that for a string that only an entry to a StringTable is Stored, or that a default value should be used.
  //UInt32              m_modifierData  //! Data used with the modifier flag, like the length of a fixed-sized string.
    UInt32              m_nullflagsOffset;
    NullflagsBitmask    m_nullflagsBitmask;
  //PropertyCP          m_property; // WIP_FUSION: optional? YAGNI?
    
public:
    PropertyLayout (wchar_t const * accessString, DataType dataType, UInt32 offset, UInt32 nullflagsOffset, UInt32 nullflagsBitmask) : //, PropertyCP property) :
        m_accessString(accessString), m_dataType(dataType), m_offset(offset), m_nullflagsOffset(nullflagsOffset), 
        m_nullflagsBitmask (nullflagsBitmask) {}; //, m_property(property) {};

    inline UInt32           GetOffset() const           { return m_offset; }
    inline UInt32           GetNullflagsOffset() const  { return m_nullflagsOffset; }
    inline NullflagsBitmask GetNullflagsBitmask() const { return m_nullflagsBitmask; }
    inline DataType         GetDataType() const         { return m_dataType; }
    inline wchar_t const *  GetAccessString() const     { return m_accessString.c_str(); }
    
    bool                    IsFixedSized() const;
    //! Gets the size required for this PropertyValue in the fixed Section of the Instance's memory
    //! Variable-sized types will have 4 byte SecondaryOffset stored in the fixed Section.
    UInt32                  GetSizeInFixedSection() const;
    
    std::wstring ToString();
    };
    
/*=================================================================================**//**
* @bsistruct                                                     CaseyMullen    10/09
+===============+===============+===============+===============+===============+======*/      
struct ClassLayout
    {
    friend MemoryInstanceSupport;
private:
    struct StringComparer {bool operator()(wchar_t const * s1, wchar_t const * s2) const   {return wcscmp (s1, s2) < 0;}};
    typedef std::map<wchar_t const *, PropertyLayoutCP, StringComparer> PropertyLayoutLookup;
    typedef std::vector<PropertyLayout>                                 PropertyLayoutVector;
    
    enum State
        {
        AcceptingFixedSizeProperties,
        AcceptingVariableSizeProperties,
        Closed
        };
    
    // These members are expected to be persisted  
    UInt16                  m_classIndex; // Unique per some context, e.g. per DgnFile
    std::wstring            m_className; // WIP_FUSION: remove this redundant information and just use m_class. But we still need to persist the name.
    UInt32                  m_nProperties;
    
    PropertyLayoutVector    m_propertyLayouts; // This is the primary collection, there is a secondary map for lookup by name, below.
    PropertyLayoutLookup    m_propertyLayoutLookup;
    
    // These members are transient
    ClassCP                 m_class;
    UInt32                  m_nullflagsOffset;
    State                   m_state;
    UInt32                  m_offset;
    UInt32                  m_sizeOfFixedSection;
    
    StatusInt               AddProperty (wchar_t const * accessString, DataType datatype, size_t size);
    StatusInt               AddFixedSizeProperty (wchar_t const * accessString, DataType datatype);
    StatusInt               AddVariableSizeProperty (wchar_t const * accessString, DataType datatype);
    StatusInt               FinishLayout ();
    std::wstring            GetClassName() const;

public:
    ECOBJECTS_EXPORT ClassLayout();
    ECOBJECTS_EXPORT StatusInt SetClass (ClassCR ecClass, UInt16 classIndex);
    ECOBJECTS_EXPORT ClassCR   GetClass () const;
    UInt16                  GetClassIndex() const;
    UInt32                  GetPropertyCount () const;
    StatusInt               GetPropertyLayout (PropertyLayoutCP & propertyLayout, wchar_t const * accessString) const;
    StatusInt               GetPropertyLayoutByIndex (PropertyLayoutCP & propertyLayout, UInt32 propertyIndex) const;
    // WIP_FUSION add StatusInt GetPropertyIndex (UInt32& propertyIndex, wchar_t const * accessString);
    
    void                    Dump() const;
    
    void                    InitializeMemoryForInstance(byte * data, UInt32 bytesAllocated) const;
    
    static UInt32           GetPropertyValueSize (DataType datatype); // WIP_FUSION: move to ecvalue.h
    UInt32                  GetSizeOfFixedSection() const;
    
    //! Determines the number of bytes used, so far
    ECOBJECTS_EXPORT UInt32 CalculateBytesUsed(byte const * data) const;
    };
    
//! Holds a ClassLayoutCR and provides a public method by which to access it.
//! Used by StandaloneInstanceEnabler and XDataEnabler
struct ClassLayoutHolder
    {
private:
    ClassLayoutCR            m_classLayout;
        
protected:
    ECOBJECTS_EXPORT ClassLayoutHolder (ClassLayoutCR classLayout);

public:    
    ECOBJECTS_EXPORT ClassLayoutCR    GetClassLayout() const;
    };


//! Base class for EC::Instance implementations that get/set values from a block of memory, 
//! e.g. StandaloneInstance and XDataInstance
struct MemoryInstanceSupport
    {
private:    
    byte const *                GetAddressOfValue (PropertyLayoutCR propertyLayout) const;
    UInt32                      GetOffsetOfValue (PropertyLayoutCR propertyLayout) const;
    bool                        IsValueNull (PropertyLayoutCR propertyLayout) const;
    void                        SetValueNull (PropertyLayoutCR propertyLayout, bool isNull);
    
    //! Shifts the values' data and adjusts SecondaryOffsets for all variable-sized property values 
    //! AFTER the given one, to make room for additional bytes needed for the property value of the given PropertyLayout
    //! or to "compact" to reclaim unused space.
    //! @param data           Start of the data of the MemoryInstanceSupport
    //! @param bytesAllocated How much memory is allocated for the data
    //! @param propertyLayout PropertyLayout of the variable-sized property whose size is increasing
    //! @param shiftBy        Positive or negative! Memory will be moved and SecondaryOffsets will be adjusted by this amount
    StatusInt                   ShiftValueData(ClassLayoutCR classLayout, byte * data, UInt32 bytesAllocated, PropertyLayoutCR propertyLayout, Int32 shiftBy);
        
    StatusInt                   EnsureSpaceIsAvailable (ClassLayoutCR classLayout, PropertyLayoutCR propertyLayout, UInt32 bytesNeeded);
         
protected:
    ECOBJECTS_EXPORT void       InitializeMemory(ClassLayoutCR classLayout, byte * data, UInt32 bytesAllocated) const;
    //ECOBJECTS_EXPORT UInt32     GetBytesUsed (ClassLayoutCR classLayout, byte const * data) const;
    ECOBJECTS_EXPORT StatusInt  GetValueFromMemory (ValueR v, PropertyLayoutCR propertyLayout,      UInt32 nIndices, UInt32 const * indices) const;
    ECOBJECTS_EXPORT StatusInt  GetValueFromMemory (ClassLayoutCR classLayout, ValueR v, const wchar_t * propertyAccessString, UInt32 nIndices, UInt32 const * indices) const;
    ECOBJECTS_EXPORT StatusInt  SetValueToMemory (ClassLayoutCR classLayout, const wchar_t * propertyAccessString, ValueCR v,  UInt32 nIndices, UInt32 const * indices);      
    ECOBJECTS_EXPORT void       DumpInstanceData (ClassLayoutCR classLayout) const;
    
    virtual bool         _IsMemoryInitialized () const = 0;    
    //! Get a pointer to the first byte of the data    
    virtual byte const * _GetDataForRead () const = 0;
    virtual byte *       _GetDataForWrite () const = 0;
    virtual StatusInt    _ModifyData (UInt32 offset, void const * newData, UInt32 dataLength) = 0;
    virtual UInt32       _GetBytesAllocated () const = 0;
        
    //! Reallocates memory for the Instance and copies the old Instance data into the new memory
    //! You might get more memory than used asked for, but you won't get less
    //! @param additionalBytesNeeded  Additional bytes of memory needed above current allocation
    virtual StatusInt    _GrowAllocation (UInt32 additionalBytesNeeded) = 0;
    
    //! Reallocates memory for the Instance and copies the old Instance data into the new memory
    //! This is not guaranteed to do anything or to change to precisely the allocation you request
    //! but it will be at least as large as you request.
    //! @param newAllocation  Additional bytes of memory needed above current allocation    
    virtual void         _ShrinkAllocation (UInt32 newAllocation) = 0;
    
    //! Free any allocated memory
    virtual void         _FreeAllocation () = 0;
    };

END_BENTLEY_EC_NAMESPACE