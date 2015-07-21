/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicApi/ECObjects/StandaloneECRelationshipInstance.h $
|
|   $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__
/** @cond BENTLEY_SDK_Internal */

#include <ECObjects/ECObjects.h>
#include <ECObjects/ECDBuffer.h>

EC_TYPEDEFS(StandaloneECRelationshipEnabler);
EC_TYPEDEFS(StandaloneECRelationshipInstance);

BEGIN_BENTLEY_ECOBJECT_NAMESPACE

typedef RefCountedPtr<StandaloneECRelationshipEnabler>  StandaloneECRelationshipEnablerPtr;
typedef RefCountedPtr<StandaloneECRelationshipInstance> StandaloneECRelationshipInstancePtr;

//=======================================================================================
//! @addtogroup ECObjectsGroup
//! @beginGroup
//! StandaloneECRelationshipInstance is used to represent a relationship between
//! two IECInstances
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE StandaloneECRelationshipInstance : IECRelationshipInstance
    //__PUBLISH_SECTION_END__
    , MemoryECInstanceBase
    //__PUBLISH_SECTION_START__
    {
    //__PUBLISH_SECTION_END__
    friend struct StandaloneECRelationshipEnabler;

private:
    IECInstancePtr                    m_source;
    IECInstancePtr                    m_target;
    int64_t                           m_sourceOrderId;
    int64_t                           m_targetOrderId;
    WString                           m_name;
    StandaloneECRelationshipEnablerCP m_relationshipEnabler;
    WString                           m_instanceId;

    StandaloneECRelationshipInstance (StandaloneECRelationshipEnablerCR relationshipEnabler);
    ~StandaloneECRelationshipInstance();

protected:
    // IECRelationshipInstance
    ECOBJECTS_EXPORT virtual void            _SetSource (IECInstanceP instance);
    ECOBJECTS_EXPORT virtual IECInstancePtr  _GetSource () const;
    ECOBJECTS_EXPORT virtual void            _SetTarget (IECInstanceP instance);
    ECOBJECTS_EXPORT virtual IECInstancePtr  _GetTarget () const;
    ECOBJECTS_EXPORT virtual ECObjectsStatus _GetSourceOrderId (int64_t& sourceOrderId) const override;
    ECOBJECTS_EXPORT virtual ECObjectsStatus _GetTargetOrderId (int64_t& targetOrderId) const override;
    // IECInstance
    ECOBJECTS_EXPORT virtual WString             _GetInstanceId () const override;
    ECOBJECTS_EXPORT virtual ECObjectsStatus     _SetInstanceId (WCharCP id) override;
    ECOBJECTS_EXPORT virtual bool                _IsReadOnly () const override;
    ECOBJECTS_EXPORT virtual ECObjectsStatus     _GetValue (ECValueR v, uint32_t propertyIndex, bool useArrayIndex, uint32_t arrayIndex) const override;
    ECOBJECTS_EXPORT virtual ECObjectsStatus     _SetValue (uint32_t propertyIndex, ECValueCR v, bool useArrayIndex, uint32_t arrayIndex) override;
    ECOBJECTS_EXPORT virtual ECObjectsStatus     _SetInternalValue (uint32_t propertyIndex, ECValueCR v, bool useArrayIndex, uint32_t arrayIndex) override;
    ECOBJECTS_EXPORT virtual ECObjectsStatus     _GetIsPropertyNull (bool& isNull, uint32_t propertyIndex, bool useArrayIndex, uint32_t arrayIndex) const override;

    ECOBJECTS_EXPORT virtual ECObjectsStatus     _InsertArrayElements (uint32_t propIdx, uint32_t index, uint32_t size) override;
    ECOBJECTS_EXPORT virtual ECObjectsStatus     _AddArrayElements (uint32_t propIdx, uint32_t size) override;
    ECOBJECTS_EXPORT virtual ECObjectsStatus     _RemoveArrayElement (uint32_t propIdx, uint32_t index) override;
    ECOBJECTS_EXPORT virtual ECObjectsStatus     _ClearArray (uint32_t propIdx) override;
    ECOBJECTS_EXPORT virtual WString             _ToString (WCharCP indent) const override;
    ECOBJECTS_EXPORT virtual ClassLayoutCR       _GetClassLayout () const;
    ECOBJECTS_EXPORT virtual ECEnablerCR         _GetEnabler () const override;
    ECOBJECTS_EXPORT virtual MemoryECInstanceBase* _GetAsMemoryECInstance () const override;
    ECOBJECTS_EXPORT virtual size_t              _GetObjectSize () const;
    ECOBJECTS_EXPORT virtual size_t              _GetOffsetToIECInstance () const;
    // MemoryECInstanceBase
    ECOBJECTS_EXPORT virtual IECInstanceP            _GetAsIECInstance () const;

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
    //! @param[in]  structStandaloneEnablerLocater
    //! @returns The StandaloneECRelationshipEnabler for the given class.
    ECOBJECTS_EXPORT static StandaloneECRelationshipEnablerPtr  CreateStandaloneRelationshipEnabler (ECN::ECRelationshipClassCR ecClass, ClassLayoutR classLayout, IStandaloneEnablerLocaterP structStandaloneEnablerLocater);
    //! Creates an instance of the supported ECRelationshipClass
    ECOBJECTS_EXPORT StandaloneECRelationshipInstancePtr        CreateRelationshipInstance () const;
    //! Returns this enabler as a base ECEnabler
    ECOBJECTS_EXPORT ECEnablerCR                                GetECEnabler() const;
    //! Get the ClassLayout for this enabler
    ECOBJECTS_EXPORT ClassLayoutCR                              GetClassLayout() const;
    };
/** @endGroup */
//#pragma warning(default:4250)

END_BENTLEY_ECOBJECT_NAMESPACE

/** @endcond */
