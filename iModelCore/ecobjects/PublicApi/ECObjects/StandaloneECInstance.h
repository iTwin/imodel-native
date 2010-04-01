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
typedef RefCountedPtr<StandaloneECEnabler>  StandaloneECEnablerPtr;
typedef RefCountedPtr<StandaloneECInstance> StandaloneECInstancePtr;
    
/*=================================================================================**//**
* EC::MemoryECInstance is base class for ECInstances that holds its values in memory that it allocates. 
* The memory is laid out according to the ClassLayout. The ClassLayout must be provided by classes that 
* subclass this class.
* @see ClassLayoutHolder, IECInstance
* @bsiclass 
+===============+===============+===============+===============+===============+======*/
struct MemoryECInstance : IECInstance, MemoryInstanceSupport
    {
private:
    std::wstring         m_instanceId;
    
    byte *               m_data;
    UInt32               m_bytesAllocated;
    
    // WIP_FUSION Is this a good enough implementation for StandaloneECInstace?
    typedef int StructValueIdentifier;
    StructValueIdentifier                             m_structValueId;
    std::map<StructValueIdentifier, IECInstancePtr>   m_structValueMap;
    
private:
    virtual bool      _IsMemoryInitialized () const;
    virtual StatusInt _ModifyData (UInt32 offset, void const * newData, UInt32 dataLength);    
    virtual void      _ShrinkAllocation (UInt32 newAllocation);
    virtual void      _FreeAllocation ();
    virtual StatusInt _GrowAllocation (UInt32 bytesNeeded);        
        
protected:
    //! The MemoryECInstance will take ownership of the memory
    MemoryECInstance (byte * data, UInt32 size, bool allowWritingDirectlyToInstanceMemory);
    MemoryECInstance (ClassLayoutCR classLayout, UInt32 minimumBufferSize, bool allowWritingDirectlyToInstanceMemory);
    ~MemoryECInstance ();

    virtual std::wstring    _GetInstanceId() const override;
    virtual bool            _IsReadOnly() const override;        
    virtual StatusInt       _GetValue (ECValueR v, const wchar_t * propertyAccessString) const override;
    virtual StatusInt       _GetValue (ECValueR v, const wchar_t * propertyAccessString, UInt32 index) const override;
    virtual StatusInt       _SetValue (const wchar_t * propertyAccessString, ECValueCR v) override;      
    virtual StatusInt       _SetValue (const wchar_t * propertyAccessString, ECValueCR v, UInt32 index) override;      
    virtual StatusInt       _InsertArrayElements (const wchar_t * propertyAccessString, UInt32 index, UInt32 size) override;
    virtual StatusInt       _AddArrayElements (const wchar_t * propertyAccessString, UInt32 size) override;
    virtual StatusInt       _RemoveArrayElement (const wchar_t * propertyAccessString, UInt32 index) override;
    virtual StatusInt       _ClearArray (const wchar_t * propertyAccessString) override;    
    virtual void            _Dump () const override;
    virtual byte const *    _GetData () const override;
    virtual UInt32          _GetBytesAllocated () const override;
    virtual StatusInt       _SetStructArrayValueToMemory (ECValueCR v, ClassLayoutCR classLayout, PropertyLayoutCR propertyLayout, UInt32 index) override;    
    virtual StatusInt       _GetStructArrayValueFromMemory (ECValueR v, PropertyLayoutCR propertyLayout, UInt32 index) const override;  

    virtual ClassLayoutCR   _GetClassLayout () const = 0;
    
public: // These must be public so that ECXInstanceEnabler can get at the guts of StandaloneECInstance to copy it into an XAttribute     
    ECOBJECTS_EXPORT byte const *         GetData () const;
    ECOBJECTS_EXPORT UInt32               GetBytesUsed () const;
    ECOBJECTS_EXPORT void                 ClearValues ();
    ECOBJECTS_EXPORT ClassLayoutCR        GetClassLayout() const;
    };

/*=================================================================================**//**
* EC::StandaloneECInstance is the native equivalent of a .NET "Heavyweight" ECInstance.
* It holds the values in memory that it allocates... laid out according to the ClassLayout
* @see ClassLayoutHolder, IECInstance
* @bsiclass 
+===============+===============+===============+===============+===============+======*/
struct StandaloneECInstance : MemoryECInstance
    {
friend StandaloneECEnabler;
private:
    std::wstring         m_instanceId;
    StandaloneECEnablerP m_sharedWipEnabler; 

private:
    //! The StandaloneECInstance will take ownership of the memory
    StandaloneECInstance (StandaloneECEnablerCR enabler, byte * data, UInt32 size);
    StandaloneECInstance (StandaloneECEnablerCR enabler, UInt32 minimumBufferSize);
    ~StandaloneECInstance ();
    
protected:
    virtual ClassLayoutCR   _GetClassLayout () const;
    virtual std::wstring    _GetInstanceId() const override;
    virtual ECEnablerCR     _GetEnabler() const override;
    };

struct StandaloneECEnabler : public ClassLayoutHolder, public ECEnabler
    {
private: 
    StandaloneECEnabler (ECClassCR ecClass, ClassLayoutCR classLayout);

protected:
    virtual wchar_t const * _GetName() const override;
 
public: 
    ECOBJECTS_EXPORT static StandaloneECEnablerPtr CreateEnabler (ECClassCR ecClass, ClassLayoutCR classLayout);
    ECOBJECTS_EXPORT StandaloneECInstancePtr       CreateInstance (UInt32 minimumInitialSize = 0);
    //ECOBJECTS_EXPORT StandaloneECInstanceP         CreateInstanceFromUninitializedMemory (byte * data, UInt32 size);
    //! Used to construct from another memory source like ECXData. The caller is claiming that the memory
    //! has been properly initialized with the classLayout that was passed in
    //ECOBJECTS_EXPORT StandaloneECInstanceP         CreateInstanceFromInitializedMemory (ClassLayoutCR classLayout, byte * data, UInt32 size);
    };    
END_BENTLEY_EC_NAMESPACE