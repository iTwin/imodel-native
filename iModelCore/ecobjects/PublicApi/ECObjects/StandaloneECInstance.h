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
    
/*=================================================================================**//**
* EC::MemoryECInstanceBase is base class for ECInstances that holds its values in memory that it allocates. 
* The memory is laid out according to the ClassLayout. The ClassLayout must be provided by classes that 
* subclass this class.
* @see ClassLayoutHolder, IECInstance
* @bsiclass 
+===============+===============+===============+===============+===============+======*/
struct MemoryECInstanceBase : MemoryInstanceSupport
    {
private:
    byte *                  m_data;
    UInt32                  m_bytesAllocated;
    bool                    m_isInManagedInstance;
    StructValueIdentifier   m_structValueId;
    StructInstanceVector*   m_structInstances;
    
    IECInstancePtr          GetStructArrayInstance (StructValueIdentifier structValueId) const;
    StructArrayEntry const* GetAddressOfStructArrayEntry (StructValueIdentifier key) const;

protected:
    //! The MemoryECInstanceBase will take ownership of the memory
    ECOBJECTS_EXPORT MemoryECInstanceBase (byte * data, UInt32 size, bool allowWritingDirectlyToInstanceMemory);
    ECOBJECTS_EXPORT MemoryECInstanceBase (ClassLayoutCR classLayout, UInt32 minimumBufferSize, bool allowWritingDirectlyToInstanceMemory);
    ECOBJECTS_EXPORT virtual ~MemoryECInstanceBase ();

    ECOBJECTS_EXPORT virtual bool             _IsMemoryInitialized () const;
    ECOBJECTS_EXPORT virtual ECObjectsStatus  _ModifyData (UInt32 offset, void const * newData, UInt32 dataLength);    
    ECOBJECTS_EXPORT virtual void             _ShrinkAllocation (UInt32 newAllocation);
    ECOBJECTS_EXPORT virtual void             _FreeAllocation ();
    ECOBJECTS_EXPORT virtual ECObjectsStatus  _GrowAllocation (UInt32 bytesNeeded);        

    ECOBJECTS_EXPORT virtual byte const *     _GetData () const override;
    ECOBJECTS_EXPORT virtual UInt32           _GetBytesAllocated () const override;
    ECOBJECTS_EXPORT virtual ECObjectsStatus  _SetStructArrayValueToMemory (ECValueCR v, ClassLayoutCR classLayout, PropertyLayoutCR propertyLayout, UInt32 index) override;    
    ECOBJECTS_EXPORT virtual ECObjectsStatus  _GetStructArrayValueFromMemory (ECValueR v, PropertyLayoutCR propertyLayout, UInt32 index) const override;  

    virtual ClassLayoutCR       _GetClassLayout () const = 0;
    virtual IECInstancePtr      _GetAsIECInstance () const = 0;
    virtual size_t              _GetObjectSize () const = 0;
    virtual size_t              _LoadObjectDataIntoManagedInstance (byte* managedBuffer) const = 0;

public: // These must be public so that ECXInstanceEnabler can get at the guts of StandaloneECInstance to copy it into an XAttribute
    ECOBJECTS_EXPORT void                     SetData (byte * data, UInt32 size, bool freeExisitingData); //The MemoryECInstanceBase will take ownership of the memory

    ECOBJECTS_EXPORT byte const *             GetData () const;
    ECOBJECTS_EXPORT UInt32                   GetBytesUsed () const;
    ECOBJECTS_EXPORT void                     ClearValues ();
    ECOBJECTS_EXPORT ClassLayoutCR            GetClassLayout() const;
    ECOBJECTS_EXPORT IECInstancePtr           GetAsIECInstance () const;
    ECOBJECTS_EXPORT size_t                   GetObjectSize () const;
    ECOBJECTS_EXPORT size_t                   CalculateSupportingInstanceDataSize () const;
    ECOBJECTS_EXPORT size_t                   LoadDataIntoManagedInstance (byte* managedBuffer, size_t sizeOfManagedBuffer) const;
    };

/*=================================================================================**//**
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
    ECOBJECTS_EXPORT virtual WString            _GetInstanceId() const override;
    ECOBJECTS_EXPORT virtual bool                _IsReadOnly() const override;        
    ECOBJECTS_EXPORT virtual ECObjectsStatus     _GetValue (ECValueR v, WCharCP propertyAccessString, bool useArrayIndex, UInt32 arrayIndex) const override;
    ECOBJECTS_EXPORT virtual ECObjectsStatus     _GetValue (ECValueR v, UInt32 propertyIndex, bool useArrayIndex, UInt32 arrayIndex) const override;
    ECOBJECTS_EXPORT virtual ECObjectsStatus     _SetValue (WCharCP propertyAccessString, ECValueCR v, bool useArrayIndex, UInt32 arrayIndex) override;      
    ECOBJECTS_EXPORT virtual ECObjectsStatus     _SetValue (UInt32 propertyIndex, ECValueCR v, bool useArrayIndex, UInt32 arrayIndex) override;      

    ECOBJECTS_EXPORT virtual ECObjectsStatus     _InsertArrayElements (WCharCP propertyAccessString, UInt32 index, UInt32 size) override;
    ECOBJECTS_EXPORT virtual ECObjectsStatus     _AddArrayElements (WCharCP propertyAccessString, UInt32 size) override;
    ECOBJECTS_EXPORT virtual ECObjectsStatus     _RemoveArrayElement (WCharCP propertyAccessString, UInt32 index) override;
    ECOBJECTS_EXPORT virtual ECObjectsStatus     _ClearArray (WCharCP propertyAccessString) override;    
    ECOBJECTS_EXPORT virtual WString            _ToString (WCharCP indent) const override;
    ECOBJECTS_EXPORT virtual ClassLayoutCR       _GetClassLayout () const;
    ECOBJECTS_EXPORT virtual ECEnablerCR         _GetEnabler() const override;
    ECOBJECTS_EXPORT virtual MemoryECInstanceBase* _GetAsMemoryECInstance () const override;
    ECOBJECTS_EXPORT virtual size_t                _GetObjectSize () const;
    ECOBJECTS_EXPORT virtual size_t                _GetOffsetToIECInstance () const;

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
//! ECEnabler for Standalone ECInstances
//=======================================================================================
struct StandaloneECEnabler : public ClassLayoutHolder, public ECEnabler
    {
private:
    bool    m_ownsClassLayout;

    StandaloneECEnabler (ECClassCR ecClass, ClassLayoutCR classLayout, IStandaloneEnablerLocaterR childECEnablerLocater, bool ownsClassLayout);
    virtual ~StandaloneECEnabler();

protected:
    virtual WCharCP                  _GetName() const override;
    virtual ECObjectsStatus             _GetPropertyIndex (UInt32& propertyIndex, WCharCP propertyAccessString) const override;
    virtual ECObjectsStatus             _GetAccessString  (WCharCP& propertyAccessString, UInt32 propertyIndex) const override;
    virtual UInt32                      _GetPropertyCount () const override;
    virtual UInt32                      _GetFirstPropertyIndex (UInt32 parentIndex) const override;
    virtual UInt32                      _GetNextPropertyIndex  (UInt32 parentIndex, UInt32 inputIndex) const override;

public: 
    ECOBJECTS_EXPORT static StandaloneECEnablerPtr CreateEnabler (ECClassCR ecClass, ClassLayoutCR classLayout, IStandaloneEnablerLocaterR childECEnablerLocater, bool ownsClassLayout);
    ECOBJECTS_EXPORT StandaloneECInstancePtr       CreateInstance (UInt32 minimumInitialSize = 0) const;
    //ECOBJECTS_EXPORT StandaloneECInstanceP         CreateInstanceFromUninitializedMemory (byte * data, UInt32 size);
    //! Used to construct from another memory source like ECXData. The caller is claiming that the memory
    //! has been properly initialized with the classLayout that was passed in
    //ECOBJECTS_EXPORT StandaloneECInstanceP         CreateInstanceFromInitializedMemory (ClassLayoutCR classLayout, byte * data, UInt32 size);
    };
END_BENTLEY_EC_NAMESPACE

