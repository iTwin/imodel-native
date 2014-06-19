/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicApi/ECObjects/StandaloneECInstance.h $
|
|   $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
//__PUBLISH_SECTION_START__
#pragma once

#include <ECObjects/ECObjects.h>
#include <ECObjects/ECDBuffer.h>

EC_TYPEDEFS(StandaloneECEnabler);

BEGIN_BENTLEY_ECOBJECT_NAMESPACE

#define STANDALONEENABLER_EnablerID         0xEC5E
typedef RefCountedPtr<StandaloneECEnabler>  StandaloneECEnablerPtr;
typedef RefCountedPtr<StandaloneECInstance> StandaloneECInstancePtr;


#define DEFAULT_NUMBITSPERPROPERTY  2

enum PropertyFlagIndex : UInt8
    {
    PROPERTYFLAGINDEX_IsLoaded = 0,
    PROPERTYFLAGINDEX_IsReadOnly  = 1   // For a *conditionally* read-only property
    };

enum MemoryInstanceUsageBitmask : UInt32
    {
    MEMORYINSTANCEUSAGE_Empty              = 0x0000,
    MEMORYINSTANCEUSAGE_IsPartiallyLoaded  = 0x0001,
    MEMORYINSTANCEUSAGE_IsHidden           = 0x0002     // currently used only by ECXAInstance
    };

typedef int StructValueIdentifier;

//__PUBLISH_SECTION_END__
struct StructArrayEntry
    {
    StructArrayEntry (StructValueIdentifier structValueId, IECInstancePtr const& instancePtr)
        {
        structValueIdentifier = structValueId;
        structInstance        = instancePtr;
        }

    StructValueIdentifier  structValueIdentifier;
    IECInstancePtr         structInstance;
    };

typedef bvector<StructArrayEntry> StructInstanceVector;
//__PUBLISH_SECTION_START__

struct PerPropertyFlagsHolder
    {
    UInt8                   numBitsPerProperty;
    UInt32                  numPerPropertyFlagsEntries;
    UInt32 *                perPropertyFlags;
    };

/*=================================================================================**//**
* ECN::MemoryECInstanceBase is base class for ECInstances that holds its values in memory that it allocates. 
* The memory is laid out according to the ClassLayout. The ClassLayout must be provided by classes that 
* subclass this class.
* @see IECInstance
* @ingroup ECObjectsGroup
* @bsiclass 
+===============+===============+===============+===============+===============+======*/
struct MemoryECInstanceBase : ECDBuffer
{
//__PUBLISH_SECTION_END__

friend struct IECInstance;
private:
    PerPropertyFlagsHolder  m_perPropertyFlagsHolder;
    byte *                  m_data;
    UInt32                  m_bytesAllocated;
    StructInstanceVector*   m_structInstances;
    MemoryECInstanceBase const* m_parentInstance;
    StructValueIdentifier   m_structValueId;
    bool                    m_usingSharedMemory;
    UInt16                  m_usageBitmask;  // currently only used to round trip Partially Loaded and Hidden flags

    IECInstancePtr          GetStructArrayInstance (StructValueIdentifier structValueId) const;
    StructArrayEntry const* GetAddressOfStructArrayEntry (StructValueIdentifier key) const;
    StructValueIdentifier   GetMaxStructValueIdentifier () const;
    byte*                   GetAddressOfPropertyData () const;
    ECObjectsStatus         RemoveStructStructArrayEntry (StructValueIdentifier structValueId);
    void                    InitializePerPropertyFlags (ClassLayoutCR classLayout, UInt8 numBitsPerProperty);

protected:
    //! The MemoryECInstanceBase will take ownership of the memory
    ECOBJECTS_EXPORT MemoryECInstanceBase (byte * data, UInt32 size, ClassLayoutCR classLayout, bool allowWritingDirectlyToInstanceMemory, MemoryECInstanceBase const * parentInstance=NULL);
    ECOBJECTS_EXPORT MemoryECInstanceBase (ClassLayoutCR classLayout, UInt32 minimumBufferSize, bool allowWritingDirectlyToInstanceMemory, ECClassCR ecClass, MemoryECInstanceBase const * parentInstance=NULL);

    ECOBJECTS_EXPORT virtual ~MemoryECInstanceBase ();

    ECOBJECTS_EXPORT virtual bool               _IsMemoryInitialized () const;
    ECOBJECTS_EXPORT virtual ECObjectsStatus    _ModifyData (UInt32 offset, void const * newData, UInt32 dataLength);    
    ECOBJECTS_EXPORT virtual ECObjectsStatus    _MoveData (UInt32 toOffset, UInt32 fromOffset, UInt32 dataLength);
    ECOBJECTS_EXPORT virtual ECObjectsStatus    _ShrinkAllocation ();
    ECOBJECTS_EXPORT virtual void               _FreeAllocation ();
    ECOBJECTS_EXPORT virtual ECObjectsStatus    _GrowAllocation (UInt32 bytesNeeded);        

    ECOBJECTS_EXPORT virtual byte const *       _GetData () const override;
    ECOBJECTS_EXPORT virtual UInt32             _GetBytesAllocated () const override;
    ECOBJECTS_EXPORT virtual ECObjectsStatus    _SetStructArrayValueToMemory (ECValueCR v, PropertyLayoutCR propertyLayout, UInt32 index) override;    
    ECOBJECTS_EXPORT virtual ECObjectsStatus    _GetStructArrayValueFromMemory (ECValueR v, PropertyLayoutCR propertyLayout, UInt32 index) const override;  
    ECOBJECTS_EXPORT virtual ECObjectsStatus    _RemoveStructArrayElementsFromMemory (PropertyLayoutCR propertyLayout, UInt32 removeIndex, UInt32 removeCount) override;
    ECOBJECTS_EXPORT virtual ECN::PrimitiveType _GetStructArrayPrimitiveType () const {return PRIMITIVETYPE_Integer;}
   
    ECOBJECTS_EXPORT virtual void               _ClearValues () override;
    ECOBJECTS_EXPORT virtual ECObjectsStatus    _CopyFromBuffer (ECDBufferCR src) override;

                     virtual bool               _AcquireData (bool forWrite) const override { return true; }
                     virtual bool               _ReleaseData() const override { return true; }

    ECOBJECTS_EXPORT virtual ECObjectsStatus    _EvaluateCalculatedProperty (ECValueR evaluatedValue, ECValueCR existingValue, PropertyLayoutCR propLayout) const override;
    ECOBJECTS_EXPORT virtual ECObjectsStatus    _UpdateCalculatedPropertyDependents (ECValueCR calculatedValue, PropertyLayoutCR propLayout) override;
    ECOBJECTS_EXPORT virtual bool               _IsStructValidForArray (IECInstanceCR structInstance, PropertyLayoutCR propLayout) const override;

                     virtual IECInstanceP       _GetAsIECInstance () const = 0;

    ECOBJECTS_EXPORT  ECObjectsStatus           SetValueInternal (UInt32 propertyIndex, ECValueCR v, bool useArrayIndex, UInt32 arrayIndex);
public:
    ECOBJECTS_EXPORT  ECObjectsStatus           SetInstancePerPropertyFlagsData (byte const* perPropertyFlagsDataAddress, int numBitsPerProperty, int numPerPropertyFlagsEntries);

    ECOBJECTS_EXPORT IECInstanceCP              GetAsIECInstance () const;
    ECOBJECTS_EXPORT IECInstanceP               GetAsIECInstanceP();
    ECOBJECTS_EXPORT UInt32                     GetPerPropertyFlagsSize () const;

//__PUBLISH_CLASS_VIRTUAL__
//__PUBLISH_SECTION_START__

public:
//__PUBLISH_SECTION_END__
   // These must be public so that ECXInstanceEnabler can get at the guts of StandaloneECInstance to copy it into an XAttribute
//__PUBLISH_SECTION_START__

    //! It is use to set ECD buffer for instance.
    //! @remarks This api should not be used in normal circumstances instead use CreateSharedInstance() to create instance from existing buffer.
    //! @param[in]  data  pointer to ECD buffer
    //! @param[in]  size  size of ECD buffer.
    //! @param[in]  freeExisitingDataAndCreateCopyOfNewData if True it frees existing memory and allocate new memory of specified by 'size' and copy user specified data buffer into it. If False it will simply assign user provide buffer to instance without freeing any existing memory.
    ECOBJECTS_EXPORT void                     SetData (const byte * data, UInt32 size, bool freeExisitingDataAndCreateCopyOfNewData); //The MemoryECInstanceBase will take ownership of the memory

    //! Gets a pointer to the internal data buffer
    //! @return a pointer to the internal data buffer
    ECOBJECTS_EXPORT byte const *             GetData () const;
    //! Gets the number of bytes used by the internal data buffer
    //! @return  the number of bytes used by the internal data buffer
    ECOBJECTS_EXPORT UInt32                   GetBytesUsed () const;

//__PUBLISH_SECTION_END__
    // This is only published because published tests use it...
//__PUBLISH_SECTION_START__
    //! Merges the property values of the specified IECInstance into this MemoryECInstanceBase
    //! @param[in]      fromNativeInstance The IECInstance supplying property values to be merged
    //! @return ECOBJECTS_STATUS_Success if property values were successfully merged, otherwise an error status
    ECOBJECTS_EXPORT ECObjectsStatus          MergePropertiesFromInstance (ECN::IECInstanceCR fromNativeInstance);
//__PUBLISH_SECTION_END__
    ECOBJECTS_EXPORT ECObjectsStatus          RemoveStructArrayElements (PropertyLayoutCR propertyLayout, UInt32 removeIndex, UInt32 removeCount);
    ECOBJECTS_EXPORT ECObjectsStatus          IsPerPropertyBitSet (bool& isSet, UInt8 bitIndex, UInt32 propertyIndex) const;
    ECOBJECTS_EXPORT ECObjectsStatus          IsAnyPerPropertyBitSet (bool& isSet, UInt8 bitIndex) const;
    ECOBJECTS_EXPORT ECObjectsStatus          SetPerPropertyBit (UInt8 bitIndex, UInt32 propertyIndex, bool setBit);
    ECOBJECTS_EXPORT ECObjectsStatus          SetBitForAllProperties (UInt8 bitIndex, bool setBit);
    ECOBJECTS_EXPORT ECObjectsStatus          SetIsLoadedBit (UInt32 propertyIndex);

    ECOBJECTS_EXPORT void                     ClearAllPerPropertyFlags ();
    ECOBJECTS_EXPORT UInt8                    GetNumBitsInPerPropertyFlags ();
    ECOBJECTS_EXPORT MemoryECInstanceBase const *   GetParentInstance () const;

    ECOBJECTS_EXPORT IECInstancePtr           GetStructArrayInstanceByIndex (UInt32 index, StructValueIdentifier& structValueId) const;
    ECOBJECTS_EXPORT ECObjectsStatus          SetStructArrayInstance (MemoryECInstanceBaseR instance, StructValueIdentifier structValueId);

    ECOBJECTS_EXPORT void                     SetUsingSharedMemory ();

    ECOBJECTS_EXPORT byte const *             GetPerPropertyFlagsData () const;
    ECOBJECTS_EXPORT UInt8                    GetNumBitsPerProperty () const;
    ECOBJECTS_EXPORT UInt32                   GetPerPropertyFlagsDataLength () const;
    ECOBJECTS_EXPORT ECObjectsStatus          AddNullArrayElements (UInt32 propIdx, UInt32 insertCount);
    ECOBJECTS_EXPORT UInt16                   GetUsageBitmask () const;
    ECOBJECTS_EXPORT void                     SetUsageBitmask (UInt16 mask);
    ECOBJECTS_EXPORT void                     SetPartiallyLoaded (bool set);
    ECOBJECTS_EXPORT bool                     IsPartiallyLoaded () const;
    ECOBJECTS_EXPORT bool                     SetHiddenInstance (bool set);
    ECOBJECTS_EXPORT bool                     IsHiddenInstance () const;
//__PUBLISH_SECTION_START__
};

//=================================================================================
//! ECN::StandaloneECInstance is an implementation of IECInstance which is not tied
//! to a specified persistence store and which holds the values in memory that it allocates,
//! laid out according to the ClassLayout.
//! @see IECInstance
//! @ingroup ECObjectsGroup
//! @bsiclass
//+===============+===============+===============+===============+===============+======
struct StandaloneECInstance : virtual IECInstance
//__PUBLISH_SECTION_END__
                            , MemoryECInstanceBase
//__PUBLISH_SECTION_START__
    {
//__PUBLISH_SECTION_END__
friend struct StandaloneECEnabler;
private:
    WString                 m_instanceId;
    StandaloneECEnablerPtr  m_sharedWipEnabler;
    bool                    m_isSupportingInstance;

    //! The StandaloneECInstance will take ownership of the memory
    StandaloneECInstance (StandaloneECEnablerR enabler, byte * data, UInt32 size);
    
protected:  
    ECOBJECTS_EXPORT StandaloneECInstance (StandaloneECEnablerR enabler, UInt32 minimumBufferSize);
    ECOBJECTS_EXPORT ~StandaloneECInstance ();

    // IECInstance
    ECOBJECTS_EXPORT virtual WString             _GetInstanceId() const override;
    ECOBJECTS_EXPORT virtual ECObjectsStatus     _SetInstanceId(WCharCP id) override;
    ECOBJECTS_EXPORT virtual bool                _IsReadOnly() const override;        
    ECOBJECTS_EXPORT virtual ECObjectsStatus     _GetValue (ECValueR v, UInt32 propertyIndex, bool useArrayIndex, UInt32 arrayIndex) const override;
    ECOBJECTS_EXPORT virtual ECObjectsStatus     _SetValue (UInt32 propertyIndex, ECValueCR v, bool useArrayIndex, UInt32 arrayIndex) override;      
    ECOBJECTS_EXPORT virtual ECObjectsStatus     _SetInternalValue (UInt32 propertyIndex, ECValueCR v, bool useArrayIndex, UInt32 arrayIndex) override;
    ECOBJECTS_EXPORT virtual ECObjectsStatus     _InsertArrayElements (UInt32 propIdx, UInt32 index, UInt32 size) override;
    ECOBJECTS_EXPORT virtual ECObjectsStatus     _AddArrayElements (UInt32 propIdx, UInt32 size) override;
    ECOBJECTS_EXPORT virtual ECObjectsStatus     _RemoveArrayElement (UInt32 propIdx, UInt32 index) override;
    ECOBJECTS_EXPORT virtual ECObjectsStatus     _ClearArray (UInt32 propIdx) override;    
    ECOBJECTS_EXPORT virtual WString             _ToString (WCharCP indent) const override;
    ECOBJECTS_EXPORT virtual ClassLayoutCR       _GetClassLayout () const;
    ECOBJECTS_EXPORT virtual ECEnablerCR         _GetEnabler() const override;
    ECOBJECTS_EXPORT virtual MemoryECInstanceBase* _GetAsMemoryECInstance () const override;
    ECOBJECTS_EXPORT virtual ECDBuffer*          _GetECDBuffer() const override;
    ECOBJECTS_EXPORT virtual size_t              _GetOffsetToIECInstance () const;

    // MemoryECInstanceBase
    ECOBJECTS_EXPORT virtual IECInstanceP        _GetAsIECInstance () const override;

    ECOBJECTS_EXPORT virtual ECObjectsStatus     _GetIsPropertyNull (bool& isNull, UInt32 propertyIndex, bool useArrayIndex, UInt32 arrayIndex) const override;

public:
    // We use this as an optimization for setting struct array members.
    // If the StandaloneECInstance is not currently part of a struct array, we don't need to make a deep copy when adding it as a supporting instance
                             bool                   IsSupportingInstance() const { return m_isSupportingInstance; }
                             void                   SetIsSupportingInstance()    { m_isSupportingInstance = true; }
//__PUBLISH_CLASS_VIRTUAL__
//__PUBLISH_SECTION_START__
public:
    //! Creates an in-memory duplicate of an instance, making deep copies of its ECValues.
    //! @param[in]  instance    The instance to be duplicated.
    //! @return     The in-memory duplicated instance.
    ECOBJECTS_EXPORT static StandaloneECInstancePtr Duplicate(IECInstanceCR instance);
    };

//=======================================================================================
//! ECEnabler for standalone ECInstances (IECInstances not tied to a specific persistent store)
//! @see StandaloneECInstance
//! @ingroup ECObjectsGroup
//=======================================================================================
struct StandaloneECEnabler : public ECEnabler
   {
//__PUBLISH_SECTION_END__
private:
    ClassLayoutPtr          m_classLayout;

protected:
    StandaloneECEnabler (ECClassCR ecClass, ClassLayoutR classLayout, IStandaloneEnablerLocaterP structStandaloneEnablerLocater);
    virtual ~StandaloneECEnabler();

    virtual WCharCP                     _GetName() const override;
    virtual ECObjectsStatus             _GetPropertyIndex (UInt32& propertyIndex, WCharCP propertyAccessString) const override;
    virtual ECObjectsStatus             _GetAccessString  (WCharCP& propertyAccessString, UInt32 propertyIndex) const override;
    virtual UInt32                      _GetFirstPropertyIndex (UInt32 parentIndex) const override;
    virtual UInt32                      _GetNextPropertyIndex  (UInt32 parentIndex, UInt32 inputIndex) const override;
    virtual bool                        _HasChildProperties (UInt32 parentIndex) const override;
    virtual ECObjectsStatus             _GetPropertyIndices (bvector<UInt32>& indices, UInt32 parentIndex) const override;
    virtual bool                        _IsPropertyReadOnly (UInt32 propertyIndex) const override;
//__PUBLISH_CLASS_VIRTUAL__
//__PUBLISH_SECTION_START__
public: 
    //! if structStandaloneEnablerLocater is NULL, we'll use GetDefaultStandaloneEnabler for embedded structs
    //! Creates a StandaloneECEnabler for the specified ECClass
    //! @param[in]      ecClass                        The ECClass for which to create the enabler
    //! @param[in]      classLayout                    The ClassLayout associated with the specified ECClass
    //! @param[in]      structStandaloneEnablerLocater An object capable of locating StandaloneECEnablers for struct properties. If NULL, ECClass::GetDefaultStandaloneEnabler() will be used.
    //! @return A StandaloneECEnabler, or nullptr if the enabler could not be created
    ECOBJECTS_EXPORT static StandaloneECEnablerPtr  CreateEnabler (ECClassCR ecClass, ClassLayoutR classLayout, IStandaloneEnablerLocaterP structStandaloneEnablerLocater);
//__PUBLISH_SECTION_END__
    ECOBJECTS_EXPORT StandaloneECInstanceP          CreateSharedInstance (byte * data, UInt32 size);
//__PUBLISH_SECTION_START__

    //! Creates a StandaloneECInstance
    //! @param[in]      minimumInitialSize The number of bytes to allocate in the buffer. Typically useful only when preparing to copy values from another StandaloneECInstance with a known allocation.
    //! @return A StandaloneECInstance of the ECClass associated with this ECEnabler.
    ECOBJECTS_EXPORT StandaloneECInstancePtr        CreateInstance (UInt32 minimumInitialSize = 0);
    //! Gets the ClassLayout associated with this enabler
    //! @return the ClassLayout associated with this enabler
    ECOBJECTS_EXPORT ClassLayoutCR                  GetClassLayout() const;

    //! Gets the ClassLayout associated with this enabler
    //! @return the ClassLayout associated with this enabler
    ECOBJECTS_EXPORT ClassLayoutR                   GetClassLayout();
    };
END_BENTLEY_ECOBJECT_NAMESPACE

//__PUBLISH_SECTION_END__
#pragma make_public (Bentley::ECN::StandaloneECEnabler)
#pragma make_public (Bentley::ECN::StandaloneECInstance)
#pragma make_public (Bentley::ECN::MemoryECInstanceBase)

