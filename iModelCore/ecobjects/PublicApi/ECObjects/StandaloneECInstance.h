/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicApi/ECObjects/StandaloneECInstance.h $
|
|   $Copyright: (c) 2011 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
//__PUBLISH_SECTION_START__
#pragma once

#include <ECObjects\ECObjects.h>
#include <ECObjects\MemoryInstanceSupport.h>

EC_TYPEDEFS(StandaloneECEnabler);

BEGIN_BENTLEY_EC_NAMESPACE

#define STANDALONEENABLER_EnablerID         0xEC5E
typedef RefCountedPtr<StandaloneECEnabler>  StandaloneECEnablerPtr;
typedef RefCountedPtr<StandaloneECInstance> StandaloneECInstancePtr;
typedef RefCountedPtr<IECWipRelationshipInstance> IECWipRelationshipInstancePtr;

typedef int StructValueIdentifier;

struct StructArrayEntry
    {
    StructArrayEntry (StructValueIdentifier structValueId, IECInstancePtr& instancePtr)
        {
        structValueIdentifier = structValueId;
        structInstance        = instancePtr;
        }

    StructValueIdentifier  structValueIdentifier;
    IECInstancePtr         structInstance;
    };

typedef bvector<StructArrayEntry> StructInstanceVector;

union PerPropertyFlagsUnion
{
    UInt32 *  address;
    Int64     offset; 
};

union InstanceDataUnion
{
    byte *  address;
    Int64   offset; 
};

union SupportingInstanceUnion
{
    StructInstanceVector *  vectorP;
    Int64                   offset; 
};

struct PerPropertyFlagsHolder
    {
    UInt8                   numBitsPerProperty;
    UInt32                  numPerPropertyFlagsEntries;
    PerPropertyFlagsUnion   perPropertyFlags;
    };


/*=================================================================================**//**
* EC::MemoryECInstanceBase is base class for ECInstances that holds its values in memory that it allocates. 
* The memory is laid out according to the ClassLayout. The ClassLayout must be provided by classes that 
* subclass this class.
* @see ClassLayoutHolder, IECInstance
* @bsiclass 
+===============+===============+===============+===============+===============+======*/
struct MemoryECInstanceBase : MemoryInstanceSupport
{
friend struct IECInstance;

private:
    PerPropertyFlagsHolder  m_perPropertyFlagsHolder;
    InstanceDataUnion       m_data;
    UInt32                  m_bytesAllocated;
    bool                    m_isInManagedInstance;
    StructValueIdentifier   m_structValueId;
    SupportingInstanceUnion m_structInstances;
    bool                    m_usingSharedMemory;

//__PUBLISH_SECTION_END__
    IECInstancePtr          GetStructArrayInstance (StructValueIdentifier structValueId) const;
    StructArrayEntry const* GetAddressOfStructArrayEntry (StructValueIdentifier key) const;
    byte*                   GetAddressOfPropertyData () const;
    //void                    UpdateStructArrayOffsets (byte const* gapAddress, bool& updateOffset, bool& updateOffsetToEnd, size_t resizeAmount);
    void                    UpdateStructArrayOffsets (byte const* gapAddress, size_t resizeAmount);
    ECObjectsStatus         RemoveStructStructArrayEntry (StructValueIdentifier structValueId, EC::EmbeddedInstanceCallbackP memoryReallocationCallbackP);
    void                    RemoveGapFromStructArrayEntries (byte const* gapAddress, size_t resizeAmount);
    void                    WalkSupportingStructs (WStringR completeString, WCharCP prefix) const;
    void                    InitializePerPropertyFlags (ClassLayoutCR classLayout, UInt8 numBitsPerProperty);
 
 //__PUBLISH_SECTION_START__

protected:
    //! The MemoryECInstanceBase will take ownership of the memory
    ECOBJECTS_EXPORT MemoryECInstanceBase (byte * data, UInt32 size, ClassLayoutCR classLayout, bool allowWritingDirectlyToInstanceMemory);
    ECOBJECTS_EXPORT MemoryECInstanceBase (ClassLayoutCR classLayout, UInt32 minimumBufferSize, bool allowWritingDirectlyToInstanceMemory);
    ECOBJECTS_EXPORT virtual ~MemoryECInstanceBase ();

    ECOBJECTS_EXPORT virtual bool             _IsMemoryInitialized () const;
    ECOBJECTS_EXPORT virtual ECObjectsStatus  _ModifyData (UInt32 offset, void const * newData, UInt32 dataLength);    
    ECOBJECTS_EXPORT virtual void             _ShrinkAllocation (UInt32 newAllocation);
    ECOBJECTS_EXPORT virtual void             _FreeAllocation ();
    ECOBJECTS_EXPORT virtual ECObjectsStatus  _GrowAllocation (UInt32 bytesNeeded, EmbeddedInstanceCallbackP memoryCallback);        

    ECOBJECTS_EXPORT virtual byte const *     _GetData () const override;
    ECOBJECTS_EXPORT virtual UInt32           _GetBytesAllocated () const override;
    ECOBJECTS_EXPORT virtual ECObjectsStatus  _SetStructArrayValueToMemory (ECValueCR v, ClassLayoutCR classLayout, PropertyLayoutCR propertyLayout, UInt32 index) override;    
    ECOBJECTS_EXPORT virtual ECObjectsStatus  _GetStructArrayValueFromMemory (ECValueR v, PropertyLayoutCR propertyLayout, UInt32 index) const override;  
    ECOBJECTS_EXPORT virtual ECObjectsStatus  _RemoveStructArrayElementsFromMemory (ClassLayoutCR classLayout, PropertyLayoutCR propertyLayout, UInt32 removeIndex, UInt32 removeCount, EmbeddedInstanceCallbackP memoryCallback) override;

    virtual ClassLayoutCR       _GetClassLayout () const = 0;
    virtual IECInstancePtr      _GetAsIECInstance () const = 0;
    virtual size_t              _GetObjectSize () const = 0;
    virtual size_t              _LoadObjectDataIntoManagedInstance (byte* managedBuffer) const = 0;
                             
public: // These must be public so that ECXInstanceEnabler can get at the guts of StandaloneECInstance to copy it into an XAttribute
    ECOBJECTS_EXPORT void                     SetData (byte * data, UInt32 size, bool freeExisitingData); //The MemoryECInstanceBase will take ownership of the memory

    ECOBJECTS_EXPORT byte const *             GetData () const;
    ECOBJECTS_EXPORT UInt32                   GetBytesUsed () const;
    ECOBJECTS_EXPORT UInt32                   GetPerPropertyFlagsSize () const;
    ECOBJECTS_EXPORT void                     ClearValues ();
    ECOBJECTS_EXPORT ClassLayoutCR            GetClassLayout() const;
    ECOBJECTS_EXPORT IECInstancePtr           GetAsIECInstance () const;
    ECOBJECTS_EXPORT size_t                   GetObjectSize () const;
    ECOBJECTS_EXPORT size_t                   CalculateSupportingInstanceDataSize () const;
    ECOBJECTS_EXPORT size_t                   LoadDataIntoManagedInstance (byte* managedBuffer, size_t sizeOfManagedBuffer) const;
    ECOBJECTS_EXPORT void                     FixupStructArrayOffsets (int offsetToGap, size_t resizeAmount, bool removingGap);
    ECOBJECTS_EXPORT ECObjectsStatus          RemoveStructArrayElements (ClassLayoutCR classLayout, PropertyLayoutCR propertyLayout, UInt32 removeIndex, UInt32 removeCount, EmbeddedInstanceCallbackP memoryCallback);
    ECOBJECTS_EXPORT ECObjectsStatus          IsPerPropertyBitSet (bool& isSet, UInt8 bitIndex, UInt32 propertyIndex) const;
    ECOBJECTS_EXPORT ECObjectsStatus          IsAnyPerPropertyBitSet (bool& isSet, UInt8 bitIndex) const;
    ECOBJECTS_EXPORT ECObjectsStatus          SetPerPropertyBit (UInt8 bitIndex, UInt32 propertyIndex, bool setBit);
    ECOBJECTS_EXPORT void                     ClearAllPerPropertyFlags ();
    ECOBJECTS_EXPORT UInt8                    GetNumBitsInPerPropertyFlags ();

    ECOBJECTS_EXPORT IECInstancePtr           GetStructArrayInstanceByIndex (UInt32 index, StructValueIdentifier& structValueId) const;
    ECOBJECTS_EXPORT ECObjectsStatus          SetStructArrayInstance (MemoryECInstanceBaseCR instance, StructValueIdentifier structValueId);
    ECOBJECTS_EXPORT void                     SetUsingSharedMemory ();
};

/*=================================================================================**//**
//! @ingroup ECObjectsGroup
* EC::StandaloneECInstance is the native equivalent of a .NET "Heavyweight" ECInstance.
* It holds the values in memory that it allocates... laid out according to the ClassLayout
* @see ClassLayoutHolder, IECInstance
* @bsiclass 
+===============+===============+===============+===============+===============+======*/
struct StandaloneECInstance : MemoryECInstanceBase, IECInstance
    {
friend struct StandaloneECEnabler;
private:
    WString              m_instanceId;
    StandaloneECEnablerP m_sharedWipEnabler; 

    //! The StandaloneECInstance will take ownership of the memory
    StandaloneECInstance (StandaloneECEnablerCR enabler, byte * data, UInt32 size);
    
protected:  
    ECOBJECTS_EXPORT StandaloneECInstance (StandaloneECEnablerCR enabler, UInt32 minimumBufferSize);
    ECOBJECTS_EXPORT ~StandaloneECInstance ();

    // IECInstance
    ECOBJECTS_EXPORT virtual WString             _GetInstanceId() const override;
    ECOBJECTS_EXPORT virtual ECObjectsStatus     _SetInstanceId(WCharCP id) override;
    ECOBJECTS_EXPORT virtual bool                _IsReadOnly() const override;        
    ECOBJECTS_EXPORT virtual ECObjectsStatus     _GetValue (ECValueR v, WCharCP propertyAccessString, bool useArrayIndex, UInt32 arrayIndex) const override;
    ECOBJECTS_EXPORT virtual ECObjectsStatus     _GetValue (ECValueR v, UInt32 propertyIndex, bool useArrayIndex, UInt32 arrayIndex) const override;
    ECOBJECTS_EXPORT virtual ECObjectsStatus     _SetValue (WCharCP propertyAccessString, ECValueCR v, bool useArrayIndex, UInt32 arrayIndex) override;      
    ECOBJECTS_EXPORT virtual ECObjectsStatus     _SetValue (UInt32 propertyIndex, ECValueCR v, bool useArrayIndex, UInt32 arrayIndex) override;      

    ECOBJECTS_EXPORT virtual ECObjectsStatus     _InsertArrayElements (WCharCP propertyAccessString, UInt32 index, UInt32 size, EC::EmbeddedInstanceCallbackP memoryReallocationCallbackP) override;
    ECOBJECTS_EXPORT virtual ECObjectsStatus     _AddArrayElements (WCharCP propertyAccessString, UInt32 size, EC::EmbeddedInstanceCallbackP memoryReallocationCallbackP) override;
    ECOBJECTS_EXPORT virtual ECObjectsStatus     _RemoveArrayElement (WCharCP propertyAccessString, UInt32 index) override;
    ECOBJECTS_EXPORT virtual ECObjectsStatus     _ClearArray (WCharCP propertyAccessString) override;    
    ECOBJECTS_EXPORT virtual WString             _ToString (WCharCP indent) const override;
    ECOBJECTS_EXPORT virtual ClassLayoutCR       _GetClassLayout () const;
    ECOBJECTS_EXPORT virtual ECEnablerCR         _GetEnabler() const override;
    ECOBJECTS_EXPORT virtual MemoryECInstanceBase* _GetAsMemoryECInstance () const override;
    ECOBJECTS_EXPORT virtual size_t              _GetObjectSize () const;
    ECOBJECTS_EXPORT virtual size_t              _GetOffsetToIECInstance () const;

    // MemoryECInstanceBase
    ECOBJECTS_EXPORT virtual IECInstancePtr      _GetAsIECInstance () const;
    ECOBJECTS_EXPORT virtual size_t              _LoadObjectDataIntoManagedInstance (byte* managedBuffer) const;

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
        ECOBJECTS_EXPORT IECWipRelationshipInstance (StandaloneECEnablerCR enabler) : StandaloneECInstance (enabler, 0){}

        ECOBJECTS_EXPORT virtual BentleyStatus  _SetName (WCharCP name) = 0;
        ECOBJECTS_EXPORT virtual BentleyStatus  _SetSourceOrderId (Int64 sourceOrderId) = 0;
        ECOBJECTS_EXPORT virtual BentleyStatus  _SetTargetOrderId (Int64 targetOrderId) = 0;

    public:
        ECOBJECTS_EXPORT BentleyStatus  SetName (WCharCP name);
        ECOBJECTS_EXPORT BentleyStatus  SetSourceOrderId (Int64 sourceOrderId);
        ECOBJECTS_EXPORT BentleyStatus  SetTargetOrderId (Int64 targetOrderId);
    };

//=======================================================================================
//! @ingroup ECObjectsGroup
//! ECEnabler for Standalone ECInstances (IECInstances not tied to a specific persistent store)
//=======================================================================================
struct StandaloneECEnabler : public ClassLayoutHolder, public ECEnabler
    {
private:
    bool    m_ownsClassLayout;

    StandaloneECEnabler (ECClassCR ecClass, ClassLayoutCR classLayout, IStandaloneEnablerLocaterP structStandaloneEnablerLocater, bool ownsClassLayout);
    virtual ~StandaloneECEnabler();

protected:
    virtual WCharCP                     _GetName() const override;
    virtual ECObjectsStatus             _GetPropertyIndex (UInt32& propertyIndex, WCharCP propertyAccessString) const override;
    virtual ECObjectsStatus             _GetAccessString  (WCharCP& propertyAccessString, UInt32 propertyIndex) const override;
    virtual UInt32                      _GetPropertyCount () const override;
    virtual UInt32                      _GetFirstPropertyIndex (UInt32 parentIndex) const override;
    virtual UInt32                      _GetNextPropertyIndex  (UInt32 parentIndex, UInt32 inputIndex) const override;
    virtual ECObjectsStatus             _GetPropertyIndices (bvector<UInt32>& indices, UInt32 parentIndex) const override;

public: 
    //! if structStandaloneEnablerLocater is NULL, we'll use GetDefaultStandaloneEnabler for embedded structs
    ECOBJECTS_EXPORT static StandaloneECEnablerPtr CreateEnabler (ECClassCR ecClass, ClassLayoutCR classLayout, IStandaloneEnablerLocaterP structStandaloneEnablerLocater, bool ownsClassLayout);
    ECOBJECTS_EXPORT StandaloneECInstancePtr       CreateInstance (UInt32 minimumInitialSize = 0) const;
    //ECOBJECTS_EXPORT StandaloneECInstanceP         CreateInstanceFromUninitializedMemory (byte * data, UInt32 size);

    //! Used to construct from another memory source like ECXData. The caller is claiming that the memory
    //! has been properly initialized with the classLayout that was passed in
    //ECOBJECTS_EXPORT StandaloneECInstanceP         CreateInstanceFromInitializedMemory (ClassLayoutCR classLayout, byte * data, UInt32 size);
    
    ECOBJECTS_EXPORT StandaloneECInstanceP         CreateSharedInstance (byte * data, UInt32 size);
    };
END_BENTLEY_EC_NAMESPACE

