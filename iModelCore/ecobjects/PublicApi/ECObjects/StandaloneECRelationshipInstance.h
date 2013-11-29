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

#pragma warning(disable:4250)

//=======================================================================================
//! @ingroup ECObjectsGroup
//! StandaloneECRelationshipInstance is used to represent a relationship between
//! two IECInstances 
//=======================================================================================
struct StandaloneECRelationshipInstance : virtual IECRelationshipInstance
//__PUBLISH_SECTION_END__
                                          , StandaloneECInstance
//__PUBLISH_SECTION_START__
    {
//__PUBLISH_SECTION_END__
    friend struct StandaloneECRelationshipEnabler;

private:
    IECInstancePtr                    m_source;
    IECInstancePtr                    m_target;
    StandaloneECRelationshipEnablerCP m_relationshipEnabler; 
    WString                           m_instanceId;

protected:  
    // IECRelationshipInstance
    ECOBJECTS_EXPORT virtual void            _SetSource (IECInstanceP instance);
    ECOBJECTS_EXPORT virtual IECInstancePtr  _GetSource () const;
    ECOBJECTS_EXPORT virtual void            _SetTarget (IECInstanceP instance);
    ECOBJECTS_EXPORT virtual IECInstancePtr  _GetTarget () const;

    // IECInstance
    //virtual WString             _GetInstanceId() const override;
    //virtual ECObjectsStatus     _SetInstanceId(WCharCP id) override;
    //virtual bool                _IsReadOnly() const override;        
    //virtual ECObjectsStatus     _GetValue (ECValueR v, UInt32 propertyIndex, bool useArrayIndex, UInt32 arrayIndex) const override;
    //virtual ECObjectsStatus     _SetValue (UInt32 propertyIndex, ECValueCR v, bool useArrayIndex, UInt32 arrayIndex) override;      
    //virtual ECObjectsStatus     _SetInternalValue (UInt32 propertyIndex, ECValueCR v, bool useArrayIndex, UInt32 arrayIndex) override;
    //virtual ECObjectsStatus     _GetIsPropertyNull (bool& isNull, UInt32 propertyIndex, bool useArrayIndex, UInt32 arrayIndex) const override;

    //virtual ECObjectsStatus     _InsertArrayElements (UInt32 propIdx, UInt32 index, UInt32 size) override;
    //virtual ECObjectsStatus     _AddArrayElements (UInt32 propIdx, UInt32 size) override;
    //virtual ECObjectsStatus     _RemoveArrayElement (UInt32 propIdx, UInt32 index) override;
    //virtual ECObjectsStatus     _ClearArray (UInt32 propIdx) override;    
    //virtual WString             _ToString (WCharCP indent) const override;
    //virtual ClassLayoutCR       _GetClassLayout () const;
    //virtual ECEnablerCR         _GetEnabler() const override;
    //virtual MemoryECInstanceBase* _GetAsMemoryECInstance () const override;
    //virtual size_t              _GetObjectSize () const;
    //virtual size_t              _GetOffsetToIECInstance () const;

    // MemoryECInstanceBase
    ECOBJECTS_EXPORT virtual IECInstanceP            _GetAsIECInstance () const;

    ECOBJECTS_EXPORT StandaloneECRelationshipInstance (StandaloneECRelationshipEnablerR relationshipEnabler);
    ECOBJECTS_EXPORT ~StandaloneECRelationshipInstance();

//__PUBLISH_CLASS_VIRTUAL__
//__PUBLISH_SECTION_START__
public:
    //! Returns the RelationshipEnabler for the RelationshipClass that this RelationshipInstance represents
    ECOBJECTS_EXPORT StandaloneECRelationshipEnablerCR  GetRelationshipEnabler() const;  
    //! Returns the RelationshipClass that this Instance is an instance of
    ECOBJECTS_EXPORT ECRelationshipClassCR              GetRelationshipClass () const;
    };

//! @ingroup DgnECGroup
struct IECWipRelationshipInstance: StandaloneECRelationshipInstance
    {
    private:
        bool        m_isSourceOrderIdDefined;
        bool        m_isTargetOrderIdDefined;
        Int64       m_sourceNextOrderId;
        bool        m_isSourceNextOrderIdDefined;
        Int64       m_targetNextOrderId;
        bool        m_isTargetNextOrderIdDefined;
        WString     m_propertiesString;

    protected:
        ECOBJECTS_EXPORT                            IECWipRelationshipInstance (StandaloneECRelationshipEnablerR relationshipEnabler);
        virtual ECOBJECTS_EXPORT ECObjectsStatus    _SetSourceOrderId (Int64 orderId) = 0;
        virtual ECOBJECTS_EXPORT ECObjectsStatus    _SetTargetOrderId (Int64 orderId) = 0;
        virtual ECOBJECTS_EXPORT ECObjectsStatus    _GetSourceOrderId (Int64& sourceOrderId) const = 0;
        virtual ECOBJECTS_EXPORT ECObjectsStatus    _GetTargetOrderId (Int64& targetOrderId) const = 0;
        
    public:

        //! Sets the source orderId of this RelationshipInstance
        ECOBJECTS_EXPORT ECObjectsStatus    SetSourceOrderId (Int64 sourceOrderId);
        //! Sets the target orderId of this RelationshipInstance
        ECOBJECTS_EXPORT ECObjectsStatus    SetTargetOrderId (Int64 sourceOrderId);
        //! Gets the source orderId of this RelationshipInstance
        ECOBJECTS_EXPORT ECObjectsStatus    GetSourceOrderId (Int64& sourceOrderId) const;
        //! Gets the target orderId of this RelationshipInstance
        ECOBJECTS_EXPORT ECObjectsStatus    GetTargetOrderId (Int64& targetOrderId) const;
        //! Gets the property string of this RelationshipInstance
        ECOBJECTS_EXPORT WCharCP            GetPropertiesString() const; 
        //! Sets the property string of this RelationshipInstance
        ECOBJECTS_EXPORT ECObjectsStatus    SetPropertiesString (WCharCP propertiesString); 

        ECOBJECTS_EXPORT ECObjectsStatus    SetSourceNextOrderId (Int64 sourceOrderId);
        ECOBJECTS_EXPORT ECObjectsStatus    SetTargetNextOrderId (Int64 sourceOrderId);
        ECOBJECTS_EXPORT ECObjectsStatus    GetSourceNextOrderId (Int64& orderId) const;
        ECOBJECTS_EXPORT ECObjectsStatus    GetTargetNextOrderId (Int64& orderId) const;
        ECOBJECTS_EXPORT bool               IsSourceOrderIdDefined ();
        ECOBJECTS_EXPORT bool               IsTargetOrderIdDefined ();
        ECOBJECTS_EXPORT bool               IsSourceNextOrderIdDefined ();
        ECOBJECTS_EXPORT bool               IsTargetNextOrderIdDefined ();
    };

//=======================================================================================
//! @ingroup ECObjectsGroup
//! ECEnabler for Standalone ECRelationshipInstances (IECInstances not tied to a specific persistent store)
//=======================================================================================
struct StandaloneECRelationshipEnabler : public IECRelationshipEnabler, public StandaloneECEnabler
   {
//__PUBLISH_SECTION_END__
private:
    ECN::ECRelationshipClassCR      m_relationshipClass;

    StandaloneECRelationshipEnabler (ECN::ECRelationshipClassCR ecClass, ClassLayoutR classLayout, IStandaloneEnablerLocaterP structStandaloneEnablerLocater);
    ECOBJECTS_EXPORT ~StandaloneECRelationshipEnabler ();

protected:
    //virtual WCharCP                     _GetName() const override;
    //virtual ECObjectsStatus             _GetPropertyIndex (UInt32& propertyIndex, WCharCP propertyAccessString) const override;
    //virtual ECObjectsStatus             _GetAccessString  (WCharCP& propertyAccessString, UInt32 propertyIndex) const override;
    //virtual UInt32                      _GetFirstPropertyIndex (UInt32 parentIndex) const override;
    //virtual UInt32                      _GetNextPropertyIndex  (UInt32 parentIndex, UInt32 inputIndex) const override;
    //virtual bool                        _HasChildProperties (UInt32 parentIndex) const override;
    //virtual ECObjectsStatus             _GetPropertyIndices (bvector<UInt32>& indices, UInt32 parentIndex) const override;

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
    ECOBJECTS_EXPORT static StandaloneECRelationshipEnablerPtr  CreateStandaloneRelationshipEnabler (ECN::ECRelationshipClassCR ecClass, ClassLayoutR classLayout, IStandaloneEnablerLocaterP structStandaloneEnablerLocater);
    //! Creates an instance of the supported ECRelationshipClass
    ECOBJECTS_EXPORT StandaloneECRelationshipInstancePtr        CreateRelationshipInstance ();
    //! Returns this enabler as a base ECEnabler
    ECOBJECTS_EXPORT ECEnablerCR                                GetECEnabler() const;
    ////! Get the ClassLayout for this enabler
    //ECOBJECTS_EXPORT ClassLayoutCR                              GetClassLayout() const;
    };

#pragma warning(default:4250)

END_BENTLEY_ECOBJECT_NAMESPACE

