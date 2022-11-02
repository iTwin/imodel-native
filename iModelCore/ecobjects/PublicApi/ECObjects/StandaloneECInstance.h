/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once

#include <ECObjects/ECObjects.h>
#include <ECObjects/ECDBuffer.h>

BEGIN_BENTLEY_ECOBJECT_NAMESPACE

#define STANDALONEENABLER_EnablerID         0xEC5E
typedef RefCountedPtr<IECWipRelationshipInstance> IECWipRelationshipInstancePtr;
#define DEFAULT_NUMBITSPERPROPERTY  2

// @addtogroup ECObjectsGroup
// @beginGroup

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
    uint8_t                 numBitsPerProperty;
    uint32_t                numPerPropertyFlagsEntries;
    uint32_t *                perPropertyFlags;
    };

struct InstanceXmlReader;
/*=================================================================================**//**
* ECN::MemoryECInstanceBase is base class for ECInstances that holds its values in memory that it allocates.
* The memory is laid out according to the ClassLayout. The ClassLayout must be provided by classes that
* subclass this class.
* @see IECInstance
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct MemoryECInstanceBase : ECDBuffer
{
friend struct IECInstance;
friend struct InstanceXmlReader;
private:
    PerPropertyFlagsHolder  m_perPropertyFlagsHolder;
    Byte *                  m_data;
    uint32_t                m_bytesAllocated;
    StructInstanceVector*   m_structInstances;
    MemoryECInstanceBase const* m_parentInstance;
    StructValueIdentifier   m_structValueId;
    bool                    m_usingSharedMemory;
    bool                    m_allowWritingDirectlyToInstanceMemory;
    bool                    m_allPropertiesCalculated;
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

    ECOBJECTS_EXPORT ECObjectsStatus    _SetIsHidden (bool set) override;
    ECOBJECTS_EXPORT bool               _IsHidden () const override;
    ECOBJECTS_EXPORT bool               _IsMemoryInitialized () const override;
    ECOBJECTS_EXPORT ECObjectsStatus    _ModifyData (uint32_t offset, void const * newData, uint32_t dataLength) override;
    ECOBJECTS_EXPORT ECObjectsStatus    _MoveData (uint32_t toOffset, uint32_t fromOffset, uint32_t dataLength) override;
    ECOBJECTS_EXPORT ECObjectsStatus    _ShrinkAllocation () override;
    ECOBJECTS_EXPORT void               _FreeAllocation () override;
    ECOBJECTS_EXPORT ECObjectsStatus    _GrowAllocation (uint32_t bytesNeeded) override;

    ECOBJECTS_EXPORT Byte const *       _GetData () const override;
    ECOBJECTS_EXPORT uint32_t           _GetBytesAllocated () const override;
    ECOBJECTS_EXPORT ECObjectsStatus    _SetStructArrayValueToMemory (ECValueCR v, PropertyLayoutCR propertyLayout, uint32_t index) override;    
    ECOBJECTS_EXPORT ECObjectsStatus    _GetStructArrayValueFromMemory (ECValueR v, PropertyLayoutCR propertyLayout, uint32_t index) const override;
    ECOBJECTS_EXPORT ECObjectsStatus    _RemoveStructArrayElementsFromMemory (PropertyLayoutCR propertyLayout, uint32_t removeIndex, uint32_t removeCount) override;
    ECOBJECTS_EXPORT ECN::PrimitiveType _GetStructArrayPrimitiveType () const override {return PRIMITIVETYPE_Integer;}

    ECOBJECTS_EXPORT void               _ClearValues () override;
    ECOBJECTS_EXPORT ECObjectsStatus    _CopyFromBuffer (ECDBufferCR src) override;

                     bool               _AcquireData(bool forWrite) const override {return true;}
                     bool               _ReleaseData() const override {return true;}

    ECOBJECTS_EXPORT ECObjectsStatus    _EvaluateCalculatedProperty (ECValueR evaluatedValue, ECValueCR existingValue, PropertyLayoutCR propLayout) const override;
    ECOBJECTS_EXPORT ECObjectsStatus    _UpdateCalculatedPropertyDependents (ECValueCR calculatedValue, PropertyLayoutCR propLayout) override;
    ECOBJECTS_EXPORT bool               _IsStructValidForArray (IECInstanceCR structInstance, PropertyLayoutCR propLayout) const override;

                     virtual IECInstanceP       _GetAsIECInstance () const override = 0;
//    ECOBJECTS_EXPORT virtual ECObjectsStatus    _SetCalculatedValueToMemory (ECValueCR v, PropertyLayoutCR propertyLayout, bool useIndex, UInt32 index) const override;

    bool _AllowWritingDirectlyToInstanceMemory() const override {return m_allowWritingDirectlyToInstanceMemory;}
    bool _AreAllPropertiesCalculated() const override {return m_allPropertiesCalculated;}
    void _SetAllPropertiesCalculated(bool allCalculated) override {m_allPropertiesCalculated = allCalculated;}

    ECOBJECTS_EXPORT  ECObjectsStatus           SetValueInternal (uint32_t propertyIndex, ECValueCR v, bool useArrayIndex, uint32_t arrayIndex);
public:
    ECOBJECTS_EXPORT  ECObjectsStatus           SetInstancePerPropertyFlagsData (Byte const* perPropertyFlagsDataAddress, int numBitsPerProperty, int numPerPropertyFlagsEntries);

    ECOBJECTS_EXPORT IECInstanceCP              GetAsIECInstance () const;
    ECOBJECTS_EXPORT IECInstanceP               GetAsIECInstanceP();
    ECOBJECTS_EXPORT uint32_t                   GetPerPropertyFlagsSize () const;

public:
   // These must be public so that ECXInstanceEnabler can get at the guts of StandaloneECInstance to copy it into an XAttribute

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

    //! Merges the property values of the specified IECInstance into this MemoryECInstanceBase
    //! @param[in]      fromNativeInstance The IECInstance supplying property values to be merged
    //! @return ECObjectsStatus::Success if property values were successfully merged, otherwise an error status
    ECOBJECTS_EXPORT ECObjectsStatus          MergePropertiesFromInstance (ECN::IECInstanceCR fromNativeInstance);
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
};


struct StandaloneECEnabler;
typedef RefCountedPtr<StandaloneECEnabler>  StandaloneECEnablerPtr;
struct StandaloneECInstance;
typedef RefCountedPtr<StandaloneECInstance>  StandaloneECInstancePtr;

//=======================================================================================
//! ECN::StandaloneECInstance is an implementation of IECInstance which is not tied
//! to a specified persistence store and which holds the values in memory that it allocates,
//! laid out according to the ClassLayout.
//! @see IECInstance
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE StandaloneECInstance : IECInstance, MemoryECInstanceBase
    {
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
    ECOBJECTS_EXPORT Utf8String          _GetInstanceId() const override;
    ECOBJECTS_EXPORT ECObjectsStatus     _SetInstanceId(Utf8CP id) override;
    ECOBJECTS_EXPORT bool                _IsReadOnly() const override;
    ECOBJECTS_EXPORT ECObjectsStatus     _GetValue (ECValueR v, uint32_t propertyIndex, bool useArrayIndex, uint32_t arrayIndex) const override;
    ECOBJECTS_EXPORT ECObjectsStatus     _SetValue (uint32_t propertyIndex, ECValueCR v, bool useArrayIndex, uint32_t arrayIndex) override;
    ECOBJECTS_EXPORT ECObjectsStatus     _SetInternalValue (uint32_t propertyIndex, ECValueCR v, bool useArrayIndex, uint32_t arrayIndex) override;
    ECOBJECTS_EXPORT ECObjectsStatus     _InsertArrayElements (uint32_t propIdx, uint32_t index, uint32_t size) override;
    ECOBJECTS_EXPORT ECObjectsStatus     _AddArrayElements (uint32_t propIdx, uint32_t size) override;
    ECOBJECTS_EXPORT ECObjectsStatus     _RemoveArrayElement (uint32_t propIdx, uint32_t index) override;
    ECOBJECTS_EXPORT ECObjectsStatus     _ClearArray (uint32_t propIdx) override;    
    ECOBJECTS_EXPORT Utf8String          _ToString (Utf8CP indent) const override;
    ECOBJECTS_EXPORT ClassLayoutCR       _GetClassLayout () const override;
    ECOBJECTS_EXPORT ECEnablerCR         _GetEnabler() const override;
    ECOBJECTS_EXPORT MemoryECInstanceBase* _GetAsMemoryECInstance () const override;
    ECOBJECTS_EXPORT ECDBuffer*          _GetECDBuffer() const override;
    ECOBJECTS_EXPORT size_t              _GetOffsetToIECInstance () const override;
    ECOBJECTS_EXPORT ECObjectsStatus _GetIsPropertyNull(bool& isNull, uint32_t propertyIndex, bool useArrayIndex, uint32_t arrayIndex) const override;
    ECObjectsStatus _GetShouldSerializeProperty(bool& serialize, uint32_t propertyIndex, bool useArrayIndex, uint32_t arrayIndex) const override 
        {return GetShouldSerializeProperty(serialize, propertyIndex, useArrayIndex, arrayIndex);}

    // MemoryECInstanceBase
    ECOBJECTS_EXPORT IECInstanceP        _GetAsIECInstance () const override;

public:
    // We use this as an optimization for setting struct array members.
    // If the StandaloneECInstance is not currently part of a struct array, we don't need to make a deep copy when adding it as a supporting instance
                             bool                   IsSupportingInstance() const { return m_isSupportingInstance; }
                             void                   SetIsSupportingInstance()    { m_isSupportingInstance = true; }

    // For cases in which we want the lifetime of the ECSchema bound to that of the IECInstance.
    ECOBJECTS_EXPORT         void                   BindSchema();
public:
    //! Creates an in-memory duplicate of an instance, making deep copies of its ECValues.
    //! @param[in]  instance    The instance to be duplicated.
    //! @return     The in-memory duplicated instance.
    ECOBJECTS_EXPORT static StandaloneECInstancePtr Duplicate(IECInstanceCR instance);
    };

//=======================================================================================
//! IECWipRelationshipInstance is used to set the name and order properties for an
//! ECRelationship.
//=======================================================================================
struct IECWipRelationshipInstance : StandaloneECInstance
    {
    protected:
        ECOBJECTS_EXPORT IECWipRelationshipInstance (StandaloneECEnablerR enabler) : StandaloneECInstance (enabler, 0){}

        virtual BentleyStatus  _SetName (Utf8CP name) = 0;
        virtual BentleyStatus  _SetSourceOrderId (int64_t sourceOrderId) = 0;
        virtual BentleyStatus  _SetTargetOrderId (int64_t targetOrderId) = 0;

    public:
        ECOBJECTS_EXPORT BentleyStatus  SetName (Utf8CP name);
        ECOBJECTS_EXPORT BentleyStatus  SetSourceOrderId (int64_t sourceOrderId);
        ECOBJECTS_EXPORT BentleyStatus  SetTargetOrderId (int64_t targetOrderId);
    };

//=======================================================================================
//! ECEnabler for standalone ECInstances (IECInstances not tied to a specific persistent store)
//! @see StandaloneECInstance
//=======================================================================================
struct StandaloneECEnabler : public ECEnabler
   {
private:
    ClassLayoutPtr          m_classLayout;

protected:
    StandaloneECEnabler (ECClassCR ecClass, ClassLayoutR classLayout, IStandaloneEnablerLocaterP structStandaloneEnablerLocater);
    virtual ~StandaloneECEnabler();

    Utf8CP                      _GetName() const override;
    ECObjectsStatus             _GetPropertyIndex (uint32_t& propertyIndex, Utf8CP propertyAccessString) const override;
    ECObjectsStatus             _GetAccessString  (Utf8CP& propertyAccessString, uint32_t propertyIndex) const override;
    uint32_t                    _GetFirstPropertyIndex (uint32_t parentIndex) const override;
    uint32_t                    _GetNextPropertyIndex  (uint32_t parentIndex, uint32_t inputIndex) const override;
    bool                        _HasChildProperties (uint32_t parentIndex) const override;
    uint32_t                    _GetParentPropertyIndex (uint32_t childIndex) const override;
    ECObjectsStatus             _GetPropertyIndices (bvector<uint32_t>& indices, uint32_t parentIndex) const override;
    bool                        _IsPropertyReadOnly (uint32_t propertyIndex) const override;

    uint32_t _GetExcessiveRefCountThreshold() const override {return 0x7fffffff;} 
public:
    //! if structStandaloneEnablerLocater is NULL, we'll use GetDefaultStandaloneEnabler for embedded structs
    //! Creates a StandaloneECEnabler for the specified ECClass
    //! @param[in]      ecClass                        The ECClass for which to create the enabler
    //! @param[in]      classLayout                    The ClassLayout associated with the specified ECClass
    //! @param[in]      structStandaloneEnablerLocater An object capable of locating StandaloneECEnablers for struct properties. If NULL, ECClass::GetDefaultStandaloneEnabler() will be used.
    //! @return A StandaloneECEnabler, or nullptr if the enabler could not be created
    ECOBJECTS_EXPORT static StandaloneECEnablerPtr  CreateEnabler (ECClassCR ecClass, ClassLayoutR classLayout, IStandaloneEnablerLocaterP structStandaloneEnablerLocater);
    ECOBJECTS_EXPORT StandaloneECInstanceP          CreateSharedInstance (Byte * data, uint32_t size);

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

/** @endGroup */
END_BENTLEY_ECOBJECT_NAMESPACE


