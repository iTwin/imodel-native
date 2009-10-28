/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicApi/ECObjects/MemoryLayout.h $
|
|   $Copyright: (c) 2009 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
//__PUBLISH_SECTION_START__
#pragma once

#include <ECObjects\ECObjects.h>

BEGIN_BENTLEY_EC_NAMESPACE
    
typedef UInt32 NullflagsBitmask;
typedef UInt32 SecondaryOffset;

struct MemoryEnabler;
typedef RefCountedPtr<MemoryEnabler> MemoryEnablerPtr;

#define NULLFLAGS_BITMASK_AllOn     0xFFFFFFFF
#define MEMORYENABLER_EnablerID     0x00EC3E30 // WIP_FUSION: get a real id

/*=================================================================================**//**
* @bsistruct                                                     CaseyMullen    10/09
+===============+===============+===============+===============+===============+======*/      
struct PropertyLayout
    {
private:
    std::wstring        m_accessString;
    DataType            m_dataType;
    
    // WIP_FUSION: Using UInt32 instead of size_t below because we anticipate persisting this struct in an XAttribute
    UInt32              m_offset; //! An offset to either the data holding that property’s value (for fixed-size values) or to the offset at which the properties value can be found.
  //UInt32              m_modifierFlags //! Can be used to indicate that a string should be treated as fixed size, with a max length, or that a longer fixed size type should be treated as an optional variable-sized type, or that for a string that only an entry to a StringTable is Stored, or that a default value should be used.
  //UInt32              m_modifierData  //! Data used with the modifier flag, like the length of a fixed-sized string.
    UInt32              m_nullflagsOffset;
    UInt32              m_nullflagsBitmask;
  //PropertyCP          m_property; // WIP_FUSION: optional? YAGNI?
    
public:
    PropertyLayout (wchar_t const * accessString, DataType dataType, UInt32 offset, UInt32 nullflagsOffset, UInt32 nullflagsBitmask) : //, PropertyCP property) :
        m_accessString(accessString), m_dataType(dataType), m_offset(offset), m_nullflagsOffset(nullflagsOffset), 
        m_nullflagsBitmask (nullflagsBitmask) {}; //, m_property(property) {};

    inline UInt32           GetOffset() const           { return m_offset; }
    inline UInt32           GetNullflagsOffset() const  { return m_nullflagsOffset; }
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
    friend MemoryEnabler;

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
    UInt32                  m_perFileClassID;
    State                   m_state;
    UInt32                  m_nProperties;
    
    UInt32                  m_offset;
    UInt32                  m_nullflagsOffset;

    PropertyLayoutVector    m_propertyLayouts; // This is the primary collection, there is a secondary map for lookup by name, below.
    PropertyLayoutLookup    m_propertyLayoutLookup;
    
    // These members are transient
    UInt32                  m_sizeOfFixedSection;
    
    StatusInt               AddProperty (wchar_t const * accessString, DataType datatype, size_t size);
    StatusInt               AddFixedSizeProperty (wchar_t const * accessString, DataType datatype);
    StatusInt               AddVariableSizeProperty (wchar_t const * accessString, DataType datatype);
    StatusInt               FinishLayout ();
    
    UInt32                  GetSizeOfFixedSection() const;
    void                    InitializeMemoryForInstance(byte * data, UInt32 bytesAllocated) const;
    
    //! Shifts the values' data and adjusts SecondaryOffsets for all variable-sized property values 
    //! AFTER the given one, to make room for additional bytes needed for the property value of the given PropertyLayout
    //! or to "compact" to reclaim unused space.
    //! @param data                     Start of the data of the MemoryInstance
    //! @param propertyLayout           PropertyLayout of the variable-sized property whose size is increasing
    //! @param shiftBy    Positive or negative! Memory will be moved and SecondaryOffsets will be adjusted by this amount
    void                    ShiftValueData(byte * data, PropertyLayoutCR propertyLayout, Int32 shiftBy) const;    
    
public:
    ClassLayout() : m_state(AcceptingFixedSizeProperties), 
                    m_perFileClassID(0), 
                    m_nProperties(0), 
                    m_nullflagsOffset (0),
                    m_offset(sizeof(NullflagsBitmask)),
                    m_sizeOfFixedSection(0) {};
    
    StatusInt               SetClass (ClassCR ecClass);
    StatusInt               GetPropertyLayout (PropertyLayoutCP & propertyLayout, wchar_t const * accessString) const;
    StatusInt               GetPropertyLayoutByIndex (PropertyLayoutCP & propertyLayout, UInt32 propertyIndex) const;
    // WIP_FUSION add StatusInt GetPropertyIndex (UInt32& propertyIndex, wchar_t const * accessString);
    void                    Dump() const;
    
    static UInt32           GetPropertyValueSize (DataType datatype); // WIP_FUSION: move to ecvalue.h

    };
         
struct ClassLayoutRegistry
    {
    std::map<UInt32, ClassLayoutCP>  m_classLayouts;
    };         
    
//! Implemented by an EC::Instance that can cooperate with the EC::MemoryEnabler 
//! To allocate and access memory in which to store values
//! @see MemoryEnabler, Instance
struct IMemoryProvider
    {
    friend MemoryEnabler;
    
protected:    
    //! Get a pointer to the first byte of the data    
    virtual byte *  GetData () const = 0;
    virtual UInt32  GetBytesUsed () const = 0;
    virtual void    AdjustBytesUsed (Int32 adjustment) = 0;
    virtual UInt32  GetBytesAllocated () const = 0;
    //! Allocates memory for the Instance. The memory does not need to be initialized in any way.
    //! @param minimumBytesToAllocate
    virtual void    AllocateBytes (UInt32 minimumBytesToAllocate) = 0;
        
    //! Reallocates memory for the Instance and copies the old Instance data into the new memory
    //! You might get more memory than used asked for, but you won't get less
    //! @param additionalBytesNeeded  Additional bytes of memory needed above current allocation
    virtual void    GrowAllocation (UInt32 additionalBytesNeeded) = 0;
    
    //! Reallocates memory for the Instance and copies the old Instance data into the new memory
    //! This is not guaranteed to do anything or to change to precisely the allocation you request
    //! but it will be at least as large as you request.
    //! @param newAllocation  Additional bytes of memory needed above current allocation    
    virtual void    ShrinkAllocation (UInt32 newAllocation) = 0;
    
    //! Free any allocated memory
    virtual void    FreeAllocation () = 0;
    };
    
//! EC::MemoryEnabler can be used with any EC::Instance that implements IMemoryLayoutSupport
struct MemoryEnabler : public Enabler, public IGetValue, public ISetValue,
                       public ICreateInstance //wip: also implement public IArrayManipulator
    {
private:
    ClassLayout             m_classLayout;
    
    byte *                  GetAddressOfValue (IMemoryProvider const & memoryProvider, PropertyLayoutCR propertyLayout) const;
    StatusInt               EnsureSpaceIsAvailable (IMemoryProvider & memoryProvider, PropertyLayoutCR propertyLayout, UInt32 bytesNeeded) const;
        
    MemoryEnabler (ClassCR ecClass) : Enabler (ecClass, MEMORYENABLER_EnablerID, L"Bentley::EC::MemoryEnabler")
        {
        Initialize();
        m_classLayout.SetClass(ecClass);
        }
        
protected:
    ECOBJECTS_EXPORT MemoryEnabler (ClassCR ecClass, UInt32 enablerID, std::wstring name) : Enabler (ecClass, enablerID, name) 
        {
        Initialize();
        m_classLayout.SetClass(ecClass);
        }        
public: 

    static MemoryEnablerPtr             Create(ClassCR ecClass)
        {
        return new MemoryEnabler (ecClass);    
        };
        
    ECOBJECTS_EXPORT void               InitializeInstanceMemory (IMemoryProvider& memoryProvider) const;
        
    ECOBJECTS_EXPORT virtual StatusInt  GetValue (ValueR v, InstanceCR instance, wchar_t const * propertyAccessString,
                                UInt32 nIndices = 0, UInt32 const * indices = NULL) const override;
    ECOBJECTS_EXPORT virtual StatusInt  SetValue (InstanceR instance, wchar_t const * propertyAccessString, ValueCR v,
                                UInt32 nIndices = 0, UInt32 const * indices = NULL) const override;
                                
    ECOBJECTS_EXPORT virtual StatusInt  CreateInstance (InstanceP& instance, ClassCR ecClass, wchar_t const * instanceId) const override;
    
    ClassLayoutCR   GetClassLayout() const { return m_classLayout; }
    };
        
//! EC::MemoryInstance is the native equivalent of a .NET "Heavyweight" ECInstance.
//! It holds the values in memory that it allocates... laid out according to the ClassLayout
//! @see MemoryEnabler, Instance
struct MemoryInstance : Instance, IMemoryProvider
    {
    friend MemoryEnabler;

private:
    std::wstring    m_instanceID;
    
    byte *          m_data;
    UInt32          m_bytesUsed;            
    UInt32          m_bytesAllocated;
     
    virtual byte *  GetData () const                    { return m_data; };
    virtual UInt32  GetBytesUsed () const               { return m_bytesUsed; };
    virtual void    AdjustBytesUsed (Int32 adjustment)  { m_bytesUsed += adjustment; };
    virtual void    SetBytesUsed (UInt32 nBytes)        { m_bytesUsed = nBytes; };
    virtual UInt32  GetBytesAllocated () const          { return m_bytesAllocated; };
    virtual void    ShrinkAllocation (UInt32 newAllocation) {}; // WIP_FUSION: needs implementation
    virtual void    FreeAllocation ()                   { free (m_data); m_data = NULL; };
    virtual void    AllocateBytes (UInt32 minimumBytesToAllocate);
    virtual void    GrowAllocation (UInt32 bytesNeeded);        
    
    static EnablerCR GetEnablerForClass (ClassCR ecClass);
    
protected:
    ECOBJECTS_EXPORT virtual std::wstring _GetInstanceID() const override;
        
public:
    MemoryEnablerCR GetMemoryEnabler() const;
    ECOBJECTS_EXPORT MemoryInstance (ClassCR ecClass);
    };

END_BENTLEY_EC_NAMESPACE