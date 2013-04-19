/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicApi/ECObjects/StandaloneECInstance.h $
|
|  $Copyright: (c) 2013 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__
/// @cond BENTLEY_SDK_All

#include <ECObjects/ECObjects.h>
/// @cond BENTLEY_SDK_Desktop
#include <ECObjects/ECDBuffer.h>
/// @endcond BENTLEY_SDK_Desktop

EC_TYPEDEFS(StandaloneECEnabler);

BEGIN_BENTLEY_ECOBJECT_NAMESPACE

#define STANDALONEENABLER_EnablerID         0xEC5E
typedef RefCountedPtr<StandaloneECEnabler>  StandaloneECEnablerPtr;

typedef RefCountedPtr<StandaloneECInstance> StandaloneECInstancePtr;

/// @cond BENTLEY_SDK_Desktop

typedef RefCountedPtr<IECWipRelationshipInstance> IECWipRelationshipInstancePtr;


#define DEFAULT_NUMBITSPERPROPERTY  2

enum PropertyFlagIndex ENUM_UNDERLYING_TYPE(UInt8)
    {
    PROPERTYFLAGINDEX_IsLoaded = 0,
    PROPERTYFLAGINDEX_IsReadOnly  = 1   // For a *conditionally* read-only property
    };

enum MemoryInstanceUsageBitmask ENUM_UNDERLYING_TYPE(UInt32)
    {
    MEMORYINSTANCEUSAGE_Empty              = 0x0000,
    MEMORYINSTANCEUSAGE_IsPartiallyLoaded  = 0x0001,
    MEMORYINSTANCEUSAGE_IsHidden           = 0x0002     // currently used only by ECXAInstance
    };

typedef int StructValueIdentifier;

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
* @see ClassLayoutHolder, IECInstance
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
    ECOBJECTS_EXPORT MemoryECInstanceBase (ClassLayoutCR classLayout, UInt32 minimumBufferSize, bool allowWritingDirectlyToInstanceMemory, MemoryECInstanceBase const * parentInstance=NULL);
    ECOBJECTS_EXPORT virtual ~MemoryECInstanceBase ();

    ECOBJECTS_EXPORT virtual bool             _IsMemoryInitialized () const;
    ECOBJECTS_EXPORT virtual ECObjectsStatus  _ModifyData (UInt32 offset, void const * newData, UInt32 dataLength);
    ECOBJECTS_EXPORT virtual ECObjectsStatus  _MoveData (UInt32 toOffset, UInt32 fromOffset, UInt32 dataLength);
    ECOBJECTS_EXPORT virtual ECObjectsStatus  _ShrinkAllocation ();
    ECOBJECTS_EXPORT virtual void             _FreeAllocation ();
    ECOBJECTS_EXPORT virtual ECObjectsStatus  _GrowAllocation (UInt32 bytesNeeded);

    ECOBJECTS_EXPORT virtual byte const *     _GetData () const override;
    ECOBJECTS_EXPORT virtual UInt32           _GetBytesAllocated () const override;
    ECOBJECTS_EXPORT virtual ECObjectsStatus  _SetStructArrayValueToMemory (ECValueCR v, PropertyLayoutCR propertyLayout, UInt32 index) override;    
    ECOBJECTS_EXPORT virtual ECObjectsStatus  _GetStructArrayValueFromMemory (ECValueR v, PropertyLayoutCR propertyLayout, UInt32 index) const override;
    ECOBJECTS_EXPORT virtual ECObjectsStatus  _RemoveStructArrayElementsFromMemory (PropertyLayoutCR propertyLayout, UInt32 removeIndex, UInt32 removeCount) override;
    ECOBJECTS_EXPORT virtual ECN::PrimitiveType _GetStructArrayPrimitiveType () const {return PRIMITIVETYPE_Integer;}

    ECOBJECTS_EXPORT virtual void             _ClearValues () override;
    ECOBJECTS_EXPORT virtual ECObjectsStatus  _CopyInstanceProperties (ECN::IECInstanceCR fromNativeInstance) override;

                     virtual bool             _AcquireData() const override { return true; }
                     virtual bool             _ReleaseData() const override { return true; }

    ECOBJECTS_EXPORT  ECObjectsStatus          SetValueInternal (UInt32 propertyIndex, ECValueCR v, bool useArrayIndex, UInt32 arrayIndex);
public:
    ECOBJECTS_EXPORT  ECObjectsStatus          SetInstancePerPropertyFlagsData (byte const* perPropertyFlagsDataAddress, int numBitsPerProperty, int numPerPropertyFlagsEntries);

//__PUBLISH_CLASS_VIRTUAL__
//__PUBLISH_SECTION_START__

public: // These must be public so that ECXInstanceEnabler can get at the guts of StandaloneECInstance to copy it into an XAttribute

    //! It is use to set ECD buffer for instance.
    //! @remarks This api should not be used in normal circumstances instead use CreateSharedInstance() to create instance from existing buffer.
    //! @param[in]  data  pointer to ECD buffer
    //! @param[in]  size  size of ECD buffer.
    //! @param[in]  freeExisitingDataAndCreateCopyOfNewData if True it frees existing memory and allocate new memory of specified by 'size' and copy user specified data buffer into it. If False it will simply assign user provide buffer to instance without freeing any existing memory.
    ECOBJECTS_EXPORT void                     SetData (const byte * data, UInt32 size, bool freeExisitingDataAndCreateCopyOfNewData); //The MemoryECInstanceBase will take ownership of the memory

    ECOBJECTS_EXPORT byte const *             GetData () const;
    ECOBJECTS_EXPORT UInt32                   GetBytesUsed () const;
    ECOBJECTS_EXPORT UInt32                   GetPerPropertyFlagsSize () const;

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
    ECOBJECTS_EXPORT ECObjectsStatus          MergePropertiesFromInstance (ECN::IECInstanceCR fromNativeInstance);

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
};

/// @endcond BENTLEY_SDK_Desktop

//=================================================================================
//! ECN::StandaloneECInstance is an implementation of IECInstance which is not tied
//! to a specified persistence store and which holds the values in memory that it allocates,
//! laid out according to the ClassLayout.
//! @see IECInstance
//! @ingroup ECObjectsGroup
//! @bsiclass
//+===============+===============+===============+===============+===============+======
struct StandaloneECInstance : IECInstance
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
    ECOBJECTS_EXPORT virtual IECInstanceP        _GetAsIECInstance () const;

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

/// @cond BENTLEY_SDK_Desktop

//=======================================================================================
//! IECWipRelationshipInstance is used to set the name and order properties for an
//! ECRelationship.
//! @ingroup ECObjectsGroup
//=======================================================================================
struct IECWipRelationshipInstance : StandaloneECInstance
    {
//__PUBLISH_SECTION_END__
    protected:
        ECOBJECTS_EXPORT IECWipRelationshipInstance (StandaloneECEnablerR enabler) : StandaloneECInstance (enabler, 0){}

        ECOBJECTS_EXPORT virtual BentleyStatus  _SetName (WCharCP name) = 0;
        ECOBJECTS_EXPORT virtual BentleyStatus  _SetSourceOrderId (Int64 sourceOrderId) = 0;
        ECOBJECTS_EXPORT virtual BentleyStatus  _SetTargetOrderId (Int64 targetOrderId) = 0;

//__PUBLISH_CLASS_VIRTUAL__
//__PUBLISH_SECTION_START__
    public:
        ECOBJECTS_EXPORT BentleyStatus  SetName (WCharCP name);
        ECOBJECTS_EXPORT BentleyStatus  SetSourceOrderId (Int64 sourceOrderId);
        ECOBJECTS_EXPORT BentleyStatus  SetTargetOrderId (Int64 targetOrderId);
    };

/// @endcond BENTLEY_SDK_Desktop

//=======================================================================================
//! ECEnabler for standalone ECInstances (IECInstances not tied to a specific persistent store)
//! @see StandaloneECInstance
//! @ingroup ECObjectsGroup
//=======================================================================================
struct StandaloneECEnabler : public ECEnabler
//__PUBLISH_SECTION_END__
    ,public ClassLayoutHolder
//__PUBLISH_SECTION_START__
   {
//__PUBLISH_SECTION_END__
private:
    bool    m_ownsClassLayout;

    StandaloneECEnabler (ECClassCR ecClass, ClassLayoutCR classLayout, IStandaloneEnablerLocaterP structStandaloneEnablerLocater, bool ownsClassLayout);
    virtual ~StandaloneECEnabler();

protected:
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
    ECOBJECTS_EXPORT static StandaloneECEnablerPtr CreateEnabler (ECClassCR ecClass, ClassLayoutCR classLayout, IStandaloneEnablerLocaterP structStandaloneEnablerLocater, bool ownsClassLayout);
    ECOBJECTS_EXPORT StandaloneECInstanceP         CreateSharedInstance (byte * data, UInt32 size);
    ECOBJECTS_EXPORT StandaloneECInstancePtr       CreateInstance (UInt32 minimumInitialSize = 0);
    };
END_BENTLEY_ECOBJECT_NAMESPACE

#pragma make_public (Bentley::ECN::StandaloneECEnabler)
#pragma make_public (Bentley::ECN::StandaloneECInstance)
#pragma make_public (Bentley::ECN::MemoryECInstanceBase)
