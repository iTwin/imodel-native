/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicApi/ECObjects/StandaloneECInstance.h $
|
|   $Copyright: (c) 2010 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
//__PUBLISH_SECTION_START__
#pragma once

#include <ECObjects\ECObjects.h>

EC_TYPEDEFS(StandaloneECEnabler);

BEGIN_BENTLEY_EC_NAMESPACE

#define STANDALONEENABLER_EnablerID     0x00EC3E30 // WIP_FUSION: get a real id
typedef RefCountedPtr<StandaloneECEnabler> StandaloneECEnablerPtr;
    
//! EC::StandaloneECInstance is the native equivalent of a .NET "Heavyweight" ECInstance.
//! It holds the values in memory that it allocates... laid out according to the ClassLayout
//! @see ClassLayoutHolder, IECInstance
struct StandaloneECInstance : IECInstance, MemoryInstanceSupport
    {
friend StandaloneECInstanceFactory;
private:
    StandaloneECEnablerP m_standaloneEnabler; 
    std::wstring     m_instanceId;
    
    byte *           m_data;
    UInt32           m_bytesAllocated;
     
private:
    virtual bool      _IsMemoryInitialized () const;
    virtual byte *    _GetDataForWrite () const;
    virtual StatusInt _ModifyData (UInt32 offset, void const * newData, UInt32 dataLength);    
    virtual void      _ShrinkAllocation (UInt32 newAllocation);
    virtual void      _FreeAllocation ();
    virtual StatusInt _GrowAllocation (UInt32 bytesNeeded);        
    
    StandaloneECInstance (StandaloneECEnablerCR enabler, byte * data, UInt32 size);
    
    //static StandaloneECInstanceP CreateWithNewMemory (StandaloneECEnablerCR enabler);
    static StandaloneECInstanceP CreateFromUninitializedMemory (StandaloneECEnablerCR enabler, byte * data, UInt32 size);
    static StandaloneECInstanceP CreateFromInitializedMemory (StandaloneECEnablerCR enabler, byte * data, UInt32 size);
        
protected:
    virtual ECEnablerCR     _GetEnabler() const override;
    
    virtual std::wstring    _GetInstanceId() const override;
    virtual bool            _IsReadOnly() const override;        
    virtual StatusInt       _GetValue (ECValueR v, const wchar_t * propertyAccessString, UInt32 nIndices, UInt32 const * indices) const override;
    virtual StatusInt       _SetValue (const wchar_t * propertyAccessString, ECValueCR v, UInt32 nIndices, UInt32 const * indices) override;      
    virtual void            _Dump () const;
    virtual void            _Free () override;
    virtual byte const *    _GetDataForRead () const;
    virtual UInt32          _GetBytesAllocated () const;
    
public: // These must be public so that ECXDataEnabler can get at the guts of StandaloneECInstance to copy it into an XAttribute     
    ECOBJECTS_EXPORT byte const *         GetDataForRead () const;
    ECOBJECTS_EXPORT UInt32               GetBytesUsed () const;
    ECOBJECTS_EXPORT void                 ClearValues ();
    ECOBJECTS_EXPORT ClassLayoutCR        GetClassLayout() const;
    };

//! StandaloneECInstanceFactory is used to construct a new @ref StandaloneECInstance. 
//! @see StandaloneECInstance
//! StandaloneECInstanceFactory has more that one way of constructing instances, for various situations.
struct StandaloneECInstanceFactory
    {
private:    
    StandaloneECEnablerR m_standaloneEnabler;
    StandaloneECInstanceP m_instanceUnderConstruction;
    UInt32              m_minimumSlack;
    byte *              m_data;
    UInt32              m_size;
    UInt32              m_nBegun;
    UInt32              m_nFinished;
    UInt32              m_nReallocationRequests;
    
public:
    //! @param classLayout The ClassLayout that will be used for constructed StandaloneInstances.
    //! @param slack    The minimum unused space allocated as part of each finished instance. Defaults to 0.
    //!                 Increase it to avoid reallocs when the instance grows due to setting new (larger)
    //!                 values into variable-sized properties.
    //! @param initialBufferSize The initial size for the factory's internal buffer. If it less that the size of the 
    //!                 fixed-length section of the instance, then that size will be used initially.
    //!                 After creation of a few new instances, the buffer will self-adjust to an appropriate size,
    //!                 but you can use this to ensure an appropriate size from the outset.
    ECOBJECTS_EXPORT StandaloneECInstanceFactory (ECClassCR ecClass, ClassLayoutCR classLayout, UInt32 slack = 0, UInt32 initialBufferSize = 0);
    
    //! Creates a new @ref StandaloneECInstance in an "Under Construction" state. Use with @ref FinishConstruction method.
    //! While "under construction" the instance uses a buffer provided by the factory. The factory keeps a 
    //! "high water mark" of memory required by new instances, and after constructing a few, no reallocs
    //! are required when constructing new instances. When FinishConstruction is called, the instance gets its own
    //! buffer that will have at least as much "slack" (free space allocated at the end) as was requested
    //! in the constructor of the factory. The slack may prevent future reallocs, or it may just be wasted space.
    //! 
    //! This method of constructing instances is intended for cases when you are constructing multiple 
    //! StandaloneInstances of a given EC::ECClass, and you do not know the desired final size of each instance's
    //! internal buffer, but want to avoid multiple reallocs in order to get an adequate size.
    //!
    //! For greatest efficiency, you should set the variable-sized values (strings) in the same order that those
    //! properties appear in their EC::ECClass.
    //!
    //! You can only have one StandaloneECInstance "under construction" with a given factory at any given time.
    //!
    //! @code
    //! StandaloneECInstanceFactoryP factory = new StandaloneECInstanceFactory (*enabler);
    //! StandaloneECInstanceP instance = NULL;
    //! factory->BeginConstruction (instance);
    //! ECValue v(L"The length of incoming strings can be unpredictable.");
    //! instance->SetValue (L"S", v);
    //! // Set many more property values, looping through some source of data
    //! factory->FinishConstruction(instance);
    //! @endcode
    ECOBJECTS_EXPORT BentleyStatus BeginConstruction (StandaloneECInstanceP& instance);
    ECOBJECTS_EXPORT BentleyStatus FinishConstruction (StandaloneECInstanceP& instance);
    ECOBJECTS_EXPORT BentleyStatus CancelConstruction (StandaloneECInstanceP& instance);
    
    //! The number of times that a reallocation of the buffer has be "requested" by the fact that the 
    //! instance under construction had to be reallocated (at least once) while it was "under construction"
    //! Intended for diagnostics and debugging
    ECOBJECTS_EXPORT UInt32 GetReallocationCount ();
    //! The number of StandaloneInstances that have begun construction
    //! Intended for diagnostics and debugging
    ECOBJECTS_EXPORT UInt32 GetBegunCount ();
    //! The number of StandaloneInstances that have finished construction
    //! Intended for diagnostics and debugging.
    ECOBJECTS_EXPORT UInt32 GetFinishedCount ();
    
    //StandaloneECInstanceP CreateInstanceWithSlack (UInt32 predictedSizeOfVariableLengthData);
    //StandaloneECInstanceP CreateInstance ();
    };

struct StandaloneECEnabler : public ClassLayoutHolder, public ECEnabler
    {
friend StandaloneECInstanceFactory;    
private: 
    StandaloneECEnabler (ECClassCR ecClass, ClassLayoutCR classLayout);
protected:    
    virtual wchar_t const * _GetName() const override;
        
public: 
    ECOBJECTS_EXPORT static StandaloneECEnablerPtr CreateEnabler (ECClassCR ecClass, ClassLayoutCR classLayout);
    };    
END_BENTLEY_EC_NAMESPACE