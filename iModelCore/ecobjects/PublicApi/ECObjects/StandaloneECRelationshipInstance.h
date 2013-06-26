/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicApi/ECObjects/StandaloneECRelationshipInstance.h $
|
|   $Copyright: (c) 2013 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
//__PUBLISH_SECTION_START__
#pragma once

#include <ECObjects/ECObjects.h>
#include <ECObjects/ECDBuffer.h>

EC_TYPEDEFS(StandaloneECRelationshipEnabler);
EC_TYPEDEFS(StandaloneECRelationshipInstance);

BEGIN_BENTLEY_ECOBJECT_NAMESPACE

typedef RefCountedPtr<StandaloneECRelationshipEnabler>  StandaloneECRelationshipEnablerPtr;
typedef RefCountedPtr<StandaloneECRelationshipInstance> StandaloneECRelationshipInstancePtr;

//=======================================================================================
//! @ingroup ECObjectsGroup
//! StandaloneECRelationshipInstance is used to represent a relationship between
//! two IECInstances 
//=======================================================================================
struct StandaloneECRelationshipInstance : IECRelationshipInstance
//__PUBLISH_SECTION_END__
                                          , MemoryECInstanceBase
//__PUBLISH_SECTION_START__
    {
//__PUBLISH_SECTION_END__
    friend struct StandaloneECRelationshipEnabler;

private:
    IECInstancePtr                    m_source;
    IECInstancePtr                    m_target;
    Int64                             m_sourceOrderId;
    Int64                             m_targetOrderId;
    WString                           m_name;
    StandaloneECRelationshipEnablerCP m_relationshipEnabler; 
    WString                           m_instanceId;

    StandaloneECRelationshipInstance (StandaloneECRelationshipEnablerCR relationshipEnabler);
    ~StandaloneECRelationshipInstance();

protected:  
    // IECRelationshipInstance
    virtual void            _SetSource (IECInstanceP instance);
    virtual IECInstancePtr  _GetSource () const;
    virtual ECObjectsStatus _GetSourceOrderId (Int64& sourceOrderId) const;
    virtual void            _SetTarget (IECInstanceP instance);
    virtual IECInstancePtr  _GetTarget () const;
    virtual ECObjectsStatus _GetTargetOrderId (Int64& targetOrderId) const;

    // IECInstance
    virtual WString             _GetInstanceId() const override;
    virtual ECObjectsStatus     _SetInstanceId(WCharCP id) override;
    virtual bool                _IsReadOnly() const override;        
    virtual ECObjectsStatus     _GetValue (ECValueR v, UInt32 propertyIndex, bool useArrayIndex, UInt32 arrayIndex) const override;
    virtual ECObjectsStatus     _SetValue (UInt32 propertyIndex, ECValueCR v, bool useArrayIndex, UInt32 arrayIndex) override;      
    virtual ECObjectsStatus     _SetInternalValue (UInt32 propertyIndex, ECValueCR v, bool useArrayIndex, UInt32 arrayIndex) override;
    virtual ECObjectsStatus     _GetIsPropertyNull (bool& isNull, UInt32 propertyIndex, bool useArrayIndex, UInt32 arrayIndex) const override;

    virtual ECObjectsStatus     _InsertArrayElements (UInt32 propIdx, UInt32 index, UInt32 size) override;
    virtual ECObjectsStatus     _AddArrayElements (UInt32 propIdx, UInt32 size) override;
    virtual ECObjectsStatus     _RemoveArrayElement (UInt32 propIdx, UInt32 index) override;
    virtual ECObjectsStatus     _ClearArray (UInt32 propIdx) override;    
    virtual WString             _ToString (WCharCP indent) const override;
    virtual ClassLayoutCR       _GetClassLayout () const;
    virtual ECEnablerCR         _GetEnabler() const override;
    virtual MemoryECInstanceBase* _GetAsMemoryECInstance () const override;
    virtual size_t              _GetObjectSize () const;
    virtual size_t              _GetOffsetToIECInstance () const;

    // MemoryECInstanceBase
    virtual IECInstanceP        _GetAsIECInstance () const;
    virtual size_t              _LoadObjectDataIntoManagedInstance (byte* managedBuffer) const;

//__PUBLISH_CLASS_VIRTUAL__
//__PUBLISH_SECTION_START__
public:
    //! Returns the RelationshipEnabler for the RelationshipClass that this RelationshipInstance represents
    ECOBJECTS_EXPORT StandaloneECRelationshipEnablerCR  GetRelationshipEnabler() const;  
    //! Returns the RelationshipClass that this Instance is an instance of
    ECOBJECTS_EXPORT ECRelationshipClassCR              GetRelationshipClass () const;
    //! Gets the name of this RelationshipInstance
    ECOBJECTS_EXPORT WCharCP                            GetName(); 
    //! Sets the name of this RelationshipInstance
    ECOBJECTS_EXPORT void                               SetName(WCharCP name); 
    };

//=======================================================================================
//! @ingroup ECObjectsGroup
//! ECEnabler for Standalone ECRelationshipInstances (IECInstances not tied to a specific persistent store)
//=======================================================================================
struct StandaloneECRelationshipEnabler : public IECRelationshipEnabler, public ECEnabler
   {
//__PUBLISH_SECTION_END__
private:
    ECN::ECRelationshipClassCR      m_relationshipClass;

    StandaloneECRelationshipEnabler (ECN::ECRelationshipClassCR ecClass);
    ~StandaloneECRelationshipEnabler ();

protected:
    virtual WCharCP                     _GetName() const override;
    virtual ECObjectsStatus             _GetPropertyIndex (UInt32& propertyIndex, WCharCP propertyAccessString) const override;
    virtual ECObjectsStatus             _GetAccessString  (WCharCP& propertyAccessString, UInt32 propertyIndex) const override;
    virtual UInt32                      _GetFirstPropertyIndex (UInt32 parentIndex) const override;
    virtual UInt32                      _GetNextPropertyIndex  (UInt32 parentIndex, UInt32 inputIndex) const override;
    virtual bool                        _HasChildProperties (UInt32 parentIndex) const override;
    virtual ECObjectsStatus             _GetPropertyIndices (bvector<UInt32>& indices, UInt32 parentIndex) const override;

    virtual IECWipRelationshipInstancePtr _CreateWipRelationshipInstance () const;
    virtual ECN::ECRelationshipClassCR    _GetRelationshipClass() const;

public:
//__PUBLISH_CLASS_VIRTUAL__
//__PUBLISH_SECTION_START__
public:
    //! Given an ECRelationshipClass, will create an enabler for that class
    //! @param[in]  ecClass The relationship class which to create an enabler for
    //! @returns The StandaloneECRelationshipEnabler for the given class.
    ECOBJECTS_EXPORT static StandaloneECRelationshipEnablerPtr  CreateStandaloneRelationshipEnabler (ECN::ECRelationshipClassCR ecClass);
    //! Creates an instance of the supported ECRelationshipClass
    ECOBJECTS_EXPORT StandaloneECRelationshipInstancePtr        CreateRelationshipInstance () const;
    //! Returns this enabler as a base ECEnabler
    ECOBJECTS_EXPORT ECEnablerCR                                GetECEnabler() const;
    //! Get the ClassLayout for this enabler
    ECOBJECTS_EXPORT ClassLayoutCR                              GetClassLayout() const;
    };

END_BENTLEY_ECOBJECT_NAMESPACE

