/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicApi/ECObjects/StandaloneECInstance.h $
|
|   $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__

#include <ECObjects/ECObjects.h>
/** @cond BENTLEY_SDK_Internal */
#include <ECObjects/ECDBuffer.h>
/** @endcond */

BEGIN_BENTLEY_ECOBJECT_NAMESPACE

#define STANDALONEENABLER_EnablerID         0xEC5E
/** @cond BENTLEY_SDK_Internal */
typedef RefCountedPtr<IECWipRelationshipInstance> IECWipRelationshipInstancePtr;
#define DEFAULT_NUMBITSPERPROPERTY  2

enum PropertyFlagIndex ENUM_UNDERLYING_TYPE(uint8_t)
    {
    PROPERTYFLAGINDEX_IsLoaded = 0,
    PROPERTYFLAGINDEX_IsReadOnly  = 1   // For a *conditionally* read-only property
    };

enum MemoryInstanceUsageBitmask ENUM_UNDERLYING_TYPE(uint32_t)
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
    uint8_t                 numBitsPerProperty;
    uint32_t                numPerPropertyFlagsEntries;
    uint32_t *                perPropertyFlags;
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
    Byte *                  m_data;
    uint32_t                m_bytesAllocated;
    StructInstanceVector*   m_structInstances;
    MemoryECInstanceBase const* m_parentInstance;
    StructValueIdentifier   m_structValueId;
    bool                    m_usingSharedMemory;
    uint16_t                m_usageBitmask;  // currently only used to round trip Partially Loaded and Hidden flags

    IECInstancePtr          GetStructArrayInstance (StructValueIdentifier structValueId) const;
    StructArrayEntry const* GetAddressOfStructArrayEntry (StructValueIdentifier key) const;
    StructValueIdentifier   GetMaxStructValueIdentifier () const;
    Byte*                   GetAddressOfPropertyData () const;
    ECObjectsStatus         RemoveStructStructArrayEntry (StructValueIdentifier structValueId);
    void                    InitializePerPropertyFlags (ClassLayoutCR classLayout, uint8_t numBitsPerProperty);

protected:
    //! The MemoryECInstanceBase will take ownership of the memory
    ECOBJECTS_EXPORT MemoryECInstanceBase (Byte * data, uint32_t size, ClassLayoutCR classLayout, bool allowWritingDirectlyToInstanceMemory, MemoryECInstanceBase const * parentInstance=NULL);
    ECOBJECTS_EXPORT MemoryECInstanceBase (ClassLayoutCR classLayout, uint32_t minimumBufferSize, bool allowWritingDirectlyToInstanceMemory, ECClassCR ecClass, MemoryECInstanceBase const * parentInstance=NULL);

    ECOBJECTS_EXPORT virtual ~MemoryECInstanceBase ();

    ECOBJECTS_EXPORT virtual bool               _IsMemoryInitialized () const;
    ECOBJECTS_EXPORT virtual ECObjectsStatus    _ModifyData (uint32_t offset, void const * newData, uint32_t dataLength);
    ECOBJECTS_EXPORT virtual ECObjectsStatus    _MoveData (uint32_t toOffset, uint32_t fromOffset, uint32_t dataLength);
    ECOBJECTS_EXPORT virtual ECObjectsStatus    _ShrinkAllocation ();
    ECOBJECTS_EXPORT virtual void               _FreeAllocation ();
    ECOBJECTS_EXPORT virtual ECObjectsStatus    _GrowAllocation (uint32_t bytesNeeded);

    ECOBJECTS_EXPORT virtual Byte const *       _GetData () const override;
    ECOBJECTS_EXPORT virtual uint32_t           _GetBytesAllocated () const override;
    ECOBJECTS_EXPORT virtual ECObjectsStatus    _SetStructArrayValueToMemory (ECValueCR v, PropertyLayoutCR propertyLayout, uint32_t index) override;    
    ECOBJECTS_EXPORT virtual ECObjectsStatus    _GetStructArrayValueFromMemory (ECValueR v, PropertyLayoutCR propertyLayout, uint32_t index) const override;
    ECOBJECTS_EXPORT virtual ECObjectsStatus    _RemoveStructArrayElementsFromMemory (PropertyLayoutCR propertyLayout, uint32_t removeIndex, uint32_t removeCount) override;
    ECOBJECTS_EXPORT virtual ECN::PrimitiveType _GetStructArrayPrimitiveType () const {return PRIMITIVETYPE_Integer;}

    ECOBJECTS_EXPORT virtual void               _ClearValues () override;
    ECOBJECTS_EXPORT virtual ECObjectsStatus    _CopyFromBuffer (ECDBufferCR src) override;

                     virtual bool               _AcquireData (bool forWrite) const override { return true; }
                     virtual bool               _ReleaseData() const override { return true; }

    ECOBJECTS_EXPORT virtual ECObjectsStatus    _EvaluateCalculatedProperty (ECValueR evaluatedValue, ECValueCR existingValue, PropertyLayoutCR propLayout) const override;
    ECOBJECTS_EXPORT virtual ECObjectsStatus    _UpdateCalculatedPropertyDependents (ECValueCR calculatedValue, PropertyLayoutCR propLayout) override;
    ECOBJECTS_EXPORT virtual bool               _IsStructValidForArray (IECInstanceCR structInstance, PropertyLayoutCR propLayout) const override;

                     virtual IECInstanceP       _GetAsIECInstance () const = 0;
//    ECOBJECTS_EXPORT virtual ECObjectsStatus    _SetCalculatedValueToMemory (ECValueCR v, PropertyLayoutCR propertyLayout, bool useIndex, UInt32 index) const override;

    ECOBJECTS_EXPORT  ECObjectsStatus           SetValueInternal (uint32_t propertyIndex, ECValueCR v, bool useArrayIndex, uint32_t arrayIndex);
public:
    ECOBJECTS_EXPORT  ECObjectsStatus           SetInstancePerPropertyFlagsData (Byte const* perPropertyFlagsDataAddress, int numBitsPerProperty, int numPerPropertyFlagsEntries);

    ECOBJECTS_EXPORT IECInstanceCP              GetAsIECInstance () const;
    ECOBJECTS_EXPORT IECInstanceP               GetAsIECInstanceP();
    ECOBJECTS_EXPORT uint32_t                   GetPerPropertyFlagsSize () const;

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
    ECOBJECTS_EXPORT void                     SetData (const Byte * data, uint32_t size, bool freeExisitingDataAndCreateCopyOfNewData); //The MemoryECInstanceBase will take ownership of the memory

    //! Gets a pointer to the internal data buffer
    //! @return a pointer to the internal data buffer
    ECOBJECTS_EXPORT Byte const *             GetData () const;
    //! Gets the number of bytes used by the internal data buffer
    //! @return  the number of bytes used by the internal data buffer
    ECOBJECTS_EXPORT uint32_t                 GetBytesUsed () const;

//__PUBLISH_SECTION_END__
    // This is only published because published tests use it...
//__PUBLISH_SECTION_START__
    //! Merges the property values of the specified IECInstance into this MemoryECInstanceBase
    //! @param[in]      fromNativeInstance The IECInstance supplying property values to be merged
    //! @return ECOBJECTS_STATUS_Success if property values were successfully merged, otherwise an error status
    ECOBJECTS_EXPORT ECObjectsStatus          MergePropertiesFromInstance (ECN::IECInstanceCR fromNativeInstance);
//__PUBLISH_SECTION_END__
    ECOBJECTS_EXPORT ECObjectsStatus          RemoveStructArrayElements (PropertyLayoutCR propertyLayout, uint32_t removeIndex, uint32_t removeCount);
    ECOBJECTS_EXPORT ECObjectsStatus          IsPerPropertyBitSet (bool& isSet, uint8_t bitIndex, uint32_t propertyIndex) const;
    ECOBJECTS_EXPORT ECObjectsStatus          IsAnyPerPropertyBitSet (bool& isSet, uint8_t bitIndex) const;
    ECOBJECTS_EXPORT ECObjectsStatus          SetPerPropertyBit (uint8_t bitIndex, uint32_t propertyIndex, bool setBit);
    ECOBJECTS_EXPORT ECObjectsStatus          SetBitForAllProperties (uint8_t bitIndex, bool setBit);
    ECOBJECTS_EXPORT ECObjectsStatus          SetIsLoadedBit (uint32_t propertyIndex);

    ECOBJECTS_EXPORT void                     ClearAllPerPropertyFlags ();
    ECOBJECTS_EXPORT uint8_t                  GetNumBitsInPerPropertyFlags ();
    ECOBJECTS_EXPORT MemoryECInstanceBase const *   GetParentInstance () const;

    ECOBJECTS_EXPORT IECInstancePtr           GetStructArrayInstanceByIndex (uint32_t index, StructValueIdentifier& structValueId) const;
    ECOBJECTS_EXPORT ECObjectsStatus          SetStructArrayInstance (MemoryECInstanceBaseR instance, StructValueIdentifier structValueId);

    ECOBJECTS_EXPORT void                     SetUsingSharedMemory ();

    ECOBJECTS_EXPORT Byte const *             GetPerPropertyFlagsData () const;
    ECOBJECTS_EXPORT uint8_t                  GetNumBitsPerProperty () const;
    ECOBJECTS_EXPORT uint32_t                 GetPerPropertyFlagsDataLength () const;
    ECOBJECTS_EXPORT ECObjectsStatus          AddNullArrayElements (uint32_t propIdx, uint32_t insertCount);
    ECOBJECTS_EXPORT uint16_t                 GetUsageBitmask () const;
    ECOBJECTS_EXPORT void                     SetUsageBitmask (uint16_t mask);
    ECOBJECTS_EXPORT void                     SetPartiallyLoaded (bool set);
    ECOBJECTS_EXPORT bool                     IsPartiallyLoaded () const;
    ECOBJECTS_EXPORT bool                     SetHiddenInstance (bool set);
    ECOBJECTS_EXPORT bool                     IsHiddenInstance () const;
//__PUBLISH_SECTION_START__
};

/** @endcond */

struct StandaloneECEnabler;
typedef RefCountedPtr<StandaloneECEnabler>  StandaloneECEnablerPtr;
struct StandaloneECInstance;
typedef RefCountedPtr<StandaloneECInstance>  StandaloneECInstancePtr;

//=======================================================================================
//! ECN::StandaloneECInstance is an implementation of IECInstance which is not tied
//! to a specified persistence store and which holds the values in memory that it allocates,
//! laid out according to the ClassLayout.
//! @see IECInstance
//! @ingroup ECObjectsGroup
//=======================================================================================
struct StandaloneECInstance : IECInstance
//__PUBLISH_SECTION_END__
                            , MemoryECInstanceBase
//__PUBLISH_SECTION_START__
    {
//__PUBLISH_SECTION_END__
friend struct StandaloneECEnabler;
private:
    Utf8String              m_instanceId;
    StandaloneECEnablerPtr  m_sharedWipEnabler;
    bool                    m_isSupportingInstance;
    ECSchemaPtr             m_boundSchema;

    //! The StandaloneECInstance will take ownership of the memory
    StandaloneECInstance (StandaloneECEnablerR enabler, Byte * data, uint32_t size);

protected:
    ECOBJECTS_EXPORT StandaloneECInstance (StandaloneECEnablerR enabler, uint32_t minimumBufferSize);
    ECOBJECTS_EXPORT ~StandaloneECInstance ();

    // IECInstance
    ECOBJECTS_EXPORT virtual Utf8String          _GetInstanceId() const override;
    ECOBJECTS_EXPORT virtual ECObjectsStatus     _SetInstanceId(Utf8CP id) override;
    ECOBJECTS_EXPORT virtual bool                _IsReadOnly() const override;
    ECOBJECTS_EXPORT virtual ECObjectsStatus     _GetValue (ECValueR v, uint32_t propertyIndex, bool useArrayIndex, uint32_t arrayIndex) const override;
    ECOBJECTS_EXPORT virtual ECObjectsStatus     _SetValue (uint32_t propertyIndex, ECValueCR v, bool useArrayIndex, uint32_t arrayIndex) override;
    ECOBJECTS_EXPORT virtual ECObjectsStatus     _SetInternalValue (uint32_t propertyIndex, ECValueCR v, bool useArrayIndex, uint32_t arrayIndex) override;
    ECOBJECTS_EXPORT virtual ECObjectsStatus     _InsertArrayElements (uint32_t propIdx, uint32_t index, uint32_t size) override;
    ECOBJECTS_EXPORT virtual ECObjectsStatus     _AddArrayElements (uint32_t propIdx, uint32_t size) override;
    ECOBJECTS_EXPORT virtual ECObjectsStatus     _RemoveArrayElement (uint32_t propIdx, uint32_t index) override;
    ECOBJECTS_EXPORT virtual ECObjectsStatus     _ClearArray (uint32_t propIdx) override;    
    ECOBJECTS_EXPORT virtual Utf8String          _ToString (Utf8CP indent) const override;
    ECOBJECTS_EXPORT virtual ClassLayoutCR       _GetClassLayout () const;
    ECOBJECTS_EXPORT virtual ECEnablerCR         _GetEnabler() const override;
    ECOBJECTS_EXPORT virtual MemoryECInstanceBase* _GetAsMemoryECInstance () const override;
    ECOBJECTS_EXPORT virtual ECDBuffer*          _GetECDBuffer() const override;
    ECOBJECTS_EXPORT virtual size_t              _GetOffsetToIECInstance () const;

    // MemoryECInstanceBase
    ECOBJECTS_EXPORT virtual IECInstanceP        _GetAsIECInstance () const override;

    ECOBJECTS_EXPORT virtual ECObjectsStatus     _GetIsPropertyNull (bool& isNull, uint32_t propertyIndex, bool useArrayIndex, uint32_t arrayIndex) const override;

public:
    // We use this as an optimization for setting struct array members.
    // If the StandaloneECInstance is not currently part of a struct array, we don't need to make a deep copy when adding it as a supporting instance
                             bool                   IsSupportingInstance() const { return m_isSupportingInstance; }
                             void                   SetIsSupportingInstance()    { m_isSupportingInstance = true; }

    // For cases in which we want the lifetime of the ECSchema bound to that of the IECInstance.
    ECOBJECTS_EXPORT         void                   BindSchema();
//__PUBLISH_CLASS_VIRTUAL__
//__PUBLISH_SECTION_START__
public:
    //! Creates an in-memory duplicate of an instance, making deep copies of its ECValues.
    //! @param[in]  instance    The instance to be duplicated.
    //! @return     The in-memory duplicated instance.
    ECOBJECTS_EXPORT static StandaloneECInstancePtr Duplicate(IECInstanceCR instance);
    };

/** @cond BENTLEY_SDK_Internal */
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

        ECOBJECTS_EXPORT virtual BentleyStatus  _SetName (Utf8CP name) = 0;
        ECOBJECTS_EXPORT virtual BentleyStatus  _SetSourceOrderId (int64_t sourceOrderId) = 0;
        ECOBJECTS_EXPORT virtual BentleyStatus  _SetTargetOrderId (int64_t targetOrderId) = 0;

//__PUBLISH_CLASS_VIRTUAL__
//__PUBLISH_SECTION_START__
    public:
        ECOBJECTS_EXPORT BentleyStatus  SetName (Utf8CP name);
        ECOBJECTS_EXPORT BentleyStatus  SetSourceOrderId (int64_t sourceOrderId);
        ECOBJECTS_EXPORT BentleyStatus  SetTargetOrderId (int64_t targetOrderId);
    };
/** @endcond */

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

    virtual Utf8CP                      _GetName() const override;
    virtual ECObjectsStatus             _GetPropertyIndex (uint32_t& propertyIndex, Utf8CP propertyAccessString) const override;
    virtual ECObjectsStatus             _GetAccessString  (Utf8CP& propertyAccessString, uint32_t propertyIndex) const override;
    virtual uint32_t                    _GetFirstPropertyIndex (uint32_t parentIndex) const override;
    virtual uint32_t                    _GetNextPropertyIndex  (uint32_t parentIndex, uint32_t inputIndex) const override;
    virtual bool                        _HasChildProperties (uint32_t parentIndex) const override;
    virtual uint32_t                    _GetParentPropertyIndex (uint32_t childIndex) const override;
    virtual ECObjectsStatus             _GetPropertyIndices (bvector<uint32_t>& indices, uint32_t parentIndex) const override;
    virtual bool                        _IsPropertyReadOnly (uint32_t propertyIndex) const override;
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
    ECOBJECTS_EXPORT StandaloneECInstanceP          CreateSharedInstance (Byte * data, uint32_t size);
//__PUBLISH_SECTION_START__

    //! Creates a StandaloneECInstance
    //! @param[in]      minimumInitialSize The number of bytes to allocate in the buffer. Typically useful only when preparing to copy values from another StandaloneECInstance with a known allocation.
    //! @return A StandaloneECInstance of the ECClass associated with this ECEnabler.
    ECOBJECTS_EXPORT StandaloneECInstancePtr        CreateInstance (uint32_t minimumInitialSize = 0);
    //! Gets the ClassLayout associated with this enabler
    //! @return the ClassLayout associated with this enabler
    ECOBJECTS_EXPORT ClassLayoutCR                  GetClassLayout() const;

    //! Gets the ClassLayout associated with this enabler
    //! @return the ClassLayout associated with this enabler
    ECOBJECTS_EXPORT ClassLayoutR                   GetClassLayout();
    };
END_BENTLEY_ECOBJECT_NAMESPACE

//#pragma make_public (ECN::StandaloneECEnabler)
//#pragma make_public (ECN::StandaloneECInstance)
//#pragma make_public (ECN::MemoryECInstanceBase)

