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
struct StandaloneECRelationshipInstance : IECRelationshipInstance
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
    Int64                             m_sourceOrderId;        
    Int64                             m_targetOrderId;        

protected:  
    // IECRelationshipInstance
    ECOBJECTS_EXPORT virtual void            _SetSource (IECInstanceP instance);
    ECOBJECTS_EXPORT virtual IECInstancePtr  _GetSource () const;
    ECOBJECTS_EXPORT virtual void            _SetTarget (IECInstanceP instance);
    ECOBJECTS_EXPORT virtual IECInstancePtr  _GetTarget () const;
    ECOBJECTS_EXPORT virtual ECObjectsStatus _GetSourceOrderId (Int64& sourceOrderId) const override;
    ECOBJECTS_EXPORT virtual ECObjectsStatus _GetTargetOrderId (Int64& targetOrderId) const override;
    ECOBJECTS_EXPORT virtual ECObjectsStatus _SetSourceOrderId (Int64 sourceOrderId);
    ECOBJECTS_EXPORT virtual ECObjectsStatus _SetTargetOrderId (Int64 targetOrderId);

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
    //! Sets the source order Id.
    //! @param[in] sourceOrderId Contains the orderId to set for the source instance
    ECOBJECTS_EXPORT ECObjectsStatus                    SetSourceOrderId (Int64 sourceOrderId);
    //! Sets the target order Id.
    //! @param[in] targetOrderId Contains the orderId to set for the target instance
    ECOBJECTS_EXPORT ECObjectsStatus                    SetTargetOrderId (Int64 targetOrderId);
    };

//=======================================================================================
//! @ingroup ECObjectsGroup
//! IECWipRelationshipInstance is an infoprmation holder used to represent a relationship between
//! two IECInstances when passed to relationship persistence operation.
//=======================================================================================
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
        ECOBJECTS_EXPORT         IECWipRelationshipInstance (StandaloneECRelationshipEnablerR relationshipEnabler);
        ECOBJECTS_EXPORT virtual ECObjectsStatus _GetSourceOrderId (Int64& sourceOrderId) const override;
        ECOBJECTS_EXPORT virtual ECObjectsStatus _GetTargetOrderId (Int64& targetOrderId) const override;
        ECOBJECTS_EXPORT virtual ECObjectsStatus _SetSourceOrderId (Int64 sourceOrderId);
        ECOBJECTS_EXPORT virtual ECObjectsStatus _SetTargetOrderId (Int64 sourceOrderId);
        
    public:

        //! Gets the property string of this RelationshipInstance
        ECOBJECTS_EXPORT WCharCP            GetPropertiesString() const; 
        //! Sets the property string of this RelationshipInstance
        ECOBJECTS_EXPORT ECObjectsStatus    SetPropertiesString (WCharCP propertiesString); 

        //! Sets the orderId of the next source instance in case of ordered relationship.
        //! This is set by the client of a persistence provider and allows the provider 
        //! determining the orderid for this relationship.
        ECOBJECTS_EXPORT ECObjectsStatus    SetSourceNextOrderId (Int64 sourceOrderId);
        //! Sets the orderId of the next target instance in case of ordered relationship.
        //! This is set by the client of a persistence provider and allows the provider 
        //! determining the orderid for this relationship.
        ECOBJECTS_EXPORT ECObjectsStatus    SetTargetNextOrderId (Int64 sourceOrderId);
        //! Gets the orderId of the next source instance in case of ordered relationship.
        //! This is used by the persistence provider to get the value assigned by the client 
        //! and compute the orderId of this relationship.
        ECOBJECTS_EXPORT ECObjectsStatus    GetSourceNextOrderId (Int64& orderId) const;
        //! Gets the orderId of the next target instance in case of ordered relationship.
        //! This is used by the persistence provider to get the value assigned by the client 
        //! and compute the orderId of this relationship.
        ECOBJECTS_EXPORT ECObjectsStatus    GetTargetNextOrderId (Int64& orderId) const;
        //! Gets whether the source orderId is defined.
        ECOBJECTS_EXPORT bool               IsSourceOrderIdDefined ();
        //! Gets whether the target orderId is defined.
        ECOBJECTS_EXPORT bool               IsTargetOrderIdDefined ();
        //! Gets whether the next source orderId is defined.
        ECOBJECTS_EXPORT bool               IsSourceNextOrderIdDefined ();
        //! Gets whether the next target orderId is defined.
        ECOBJECTS_EXPORT bool               IsTargetNextOrderIdDefined ();
    };

//=======================================================================================
//! @ingroup ECObjectsGroup
//! ECEnabler for Standalone ECRelationshipInstances (IECInstances not tied to a specific persistent store)
//=======================================================================================
struct StandaloneECRelationshipEnabler : public IECRelationshipEnabler
//__PUBLISH_SECTION_END__
    , public StandaloneECEnabler
//__PUBLISH_SECTION_START__
   {
//__PUBLISH_SECTION_END__
private:
    ECN::ECRelationshipClassCR      m_relationshipClass;

    StandaloneECRelationshipEnabler (ECN::ECRelationshipClassCR ecClass, ClassLayoutR classLayout, IStandaloneEnablerLocaterP structStandaloneEnablerLocater);
    ECOBJECTS_EXPORT ~StandaloneECRelationshipEnabler ();

protected:
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
    //! Given an ECRelationshipClass, will create an enabler for that class
    //! @param[in]  ecClass The relationship class which to create an enabler for
    //! @param[in]  classLayout for the ecClass
    //! @param[in]  enablerLocater
    //! @returns The StandaloneECRelationshipEnabler for the given class.
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

