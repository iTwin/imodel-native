/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicApi/ECObjects/StandaloneInstance.h $
|
|   $Copyright: (c) 2009 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
//__PUBLISH_SECTION_START__
#pragma once

#include <ECObjects\ECObjects.h>

BEGIN_BENTLEY_EC_NAMESPACE
    
//! EC::StandaloneInstance is the native equivalent of a .NET "Heavyweight" ECInstance.
//! It holds the values in memory that it allocates... laid out according to the ClassLayout
//! @see MemoryEnabler, Instance
struct StandaloneInstance : Instance, MemoryInstanceSupport
    {
friend StandaloneInstanceFactory;
private:
    MemoryEnablerPtr m_enabler; 
    std::wstring     m_instanceID;
    
    byte *           m_data;
    UInt32           m_bytesAllocated;
    // WIP_FUSION: Unpublish most/all of this? 
     
public: // These two must be public so that XDataEnabler can get at the guts of Standalone to copy it into and XAttribute     
    ECOBJECTS_EXPORT virtual byte const * GetDataForRead () const;
    ECOBJECTS_EXPORT virtual UInt32       GetBytesAllocated () const;
    
private:
    virtual bool      IsMemoryInitialized () const;
    virtual byte *    GetDataForWrite () const;
    virtual StatusInt ModifyData (UInt32 offset, void const * newData, UInt32 dataLength);    
    virtual UInt32    GetBytesUsed () const;
    virtual void      ShrinkAllocation (UInt32 newAllocation);
    virtual void      FreeAllocation ();
    virtual void      AllocateBytes (UInt32 minimumBytesToAllocate);
    virtual void      GrowAllocation (UInt32 bytesNeeded);        
    
    StandaloneInstance (MemoryEnablerCR enabler, byte * data, UInt32 size);
    
    static StandaloneInstanceP CreateWithNewMemory (MemoryEnablerCR enabler);
    static StandaloneInstanceP CreateFromUninitializedMemory (MemoryEnablerCR enabler, byte * data, UInt32 size);
    static StandaloneInstanceP CreateFromInitializedMemory (MemoryEnablerCR enabler, byte * data, UInt32 size);
        
protected:
    ECOBJECTS_EXPORT virtual EnablerCP       _GetEnabler() const override;
    ECOBJECTS_EXPORT virtual MemoryEnablerCP GetMemoryEnabler() const override;
    
    ECOBJECTS_EXPORT virtual std::wstring    _GetInstanceID() const override;
    ECOBJECTS_EXPORT virtual bool            _IsReadOnly() const override;        
    ECOBJECTS_EXPORT virtual StatusInt       _GetValue (ValueR v, const wchar_t * propertyAccessString, UInt32 nIndices, UInt32 const * indices) const override;
    ECOBJECTS_EXPORT virtual StatusInt       _SetValue (const wchar_t * propertyAccessString, ValueCR v, UInt32 nIndices, UInt32 const * indices) override;      
        
public:
    ECOBJECTS_EXPORT static MemoryEnablerPtr CreateEnabler (ClassCR ecClass);

    // WIP_FUSION: make these private...
    ECOBJECTS_EXPORT StandaloneInstance (MemoryEnablerCR enabler);
    ECOBJECTS_EXPORT StandaloneInstance (ClassCR ecClass); // WIP_FUSION: delete
    
    //! Provides access to the raw data. For internal use only
    ECOBJECTS_EXPORT byte const * PeekData();
    //! Provides access to the raw data. For internal use only
    ECOBJECTS_EXPORT UInt32 PeekDataSize();
    };

//! StandaloneInstanceFactory is used to construct a new @ref StandaloneInstance. 
//! @see StandaloneInstance
//! StandaloneInstanceFactory has more that one way of constructing instances, for various situations.
struct StandaloneInstanceFactory
    {
private:    
    MemoryEnablerR      m_memoryEnabler;
    StandaloneInstanceP m_instanceUnderConstruction;
    UInt32              m_minimumSlack;
    byte *              m_data;
    UInt32              m_size;
    UInt32              m_nBegun;
    UInt32              m_nFinished;
    UInt32              m_nReallocationRequests;
    
public:
    //! @param enabler  The @ref MemoryEnabler of the @ref Class for which the factory will construct instances.
    //! @param slack    The minimum unused space allocated as part of each finished instance. Defaults to 0.
    //!                 Increase it to avoid reallocs when the instance grows due to setting new (larger)
    //!                 values into variable-sized properties.
    //! @param initialBufferSize The initial size for the factory's internal buffer. If it less that the size of the 
    //!                 fixed-length section of the instance, then that size will be used initially.
    //!                 After creation of a few new instances, the buffer will self-adjust to an appropriate size,
    //!                 but you can use this to ensure an appropriate size from the outset.
    ECOBJECTS_EXPORT StandaloneInstanceFactory (MemoryEnablerR enabler, UInt32 slack = 0, UInt32 initialBufferSize = 0);
    
    //! Creates a new @ref StandaloneInstance in an "Under Construction" state. Use with @ref FinishInstance method.
    //! While "under construction" the instance uses a buffer provided by the factory. The factory keeps a 
    //! "high water mark" of memory required by new instances, and after constructing a few, no reallocs
    //! are required when constructing new instances. When FinishInstance is called, the instance gets its own
    //! buffer that will have at least as much "slack" (free space allocated at the end) as was requested
    //! in the constructor of the factory. The slack may prevent future reallocs, or it may just be wasted space.
    //! 
    //! This method of constructing instances is intended for cases when you are constructing multiple 
    //! StandaloneInstances of a given EC::Class, and you do not know the desired final size of each instance's
    //! internal buffer, but want to avoid multiple reallocs in order to get an adequate size.
    //!
    //! For greatest efficiency, you should set the variable-sized values (strings) in the same order that those
    //! properties appear in their EC::Class.
    //!
    //! You can only have one StandaloneInstance "under construction" with a given factory at any given time.
    //!
    //! @code
    //! StandaloneInstanceFactoryP factory = new StandaloneInstanceFactory (*enabler);
    //! StandaloneInstanceP instance = NULL;
    //! factory->BeginInstanceConstruction (instance);
    //! Value v(L"The length of incoming strings can be unpredictable.");
    //! instance->SetValue (L"S", v);
    //! // Set many more property values, looping through some source of data
    //! factory->FinishInstance(instance);
    //! @endcode
    ECOBJECTS_EXPORT BentleyStatus BeginInstanceConstruction (StandaloneInstanceP& instance);
    ECOBJECTS_EXPORT BentleyStatus FinishInstance (StandaloneInstanceP& instance);
    
    //! The number of times that a reallocation of the buffer has be "requested" by the fact that the 
    //! instance under construction had to be reallocated (at least once) while it was "under construction"
    ECOBJECTS_EXPORT UInt32 GetReallocationCount ();
    ECOBJECTS_EXPORT UInt32 GetBegunCount ();
    ECOBJECTS_EXPORT UInt32 GetFinishedCount ();
    //! Creates a new @ref StandaloneInstance
    //StandaloneInstanceP CreateInstanceWithSlack (UInt32 predictedSizeOfVariableLengthData);
    //StandaloneInstanceP CreateInstance ();
    };
    
END_BENTLEY_EC_NAMESPACE