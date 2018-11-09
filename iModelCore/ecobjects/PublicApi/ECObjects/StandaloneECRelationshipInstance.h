/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicApi/ECObjects/StandaloneECRelationshipInstance.h $
|
|   $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__

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
    , MemoryECInstanceBase
    {
    friend struct StandaloneECRelationshipEnabler;

private:
    IECInstancePtr                    m_source;
    IECInstancePtr                    m_target;
    int64_t                           m_sourceOrderId;
    int64_t                           m_targetOrderId;
    Utf8String                        m_name;
    StandaloneECRelationshipEnablerCP m_relationshipEnabler;
    Utf8String                        m_instanceId;

    StandaloneECRelationshipInstance (StandaloneECRelationshipEnablerCR relationshipEnabler);
    ~StandaloneECRelationshipInstance();

protected:
    // IECRelationshipInstance
    ECOBJECTS_EXPORT void            _SetSource (IECInstanceP instance) override;
    ECOBJECTS_EXPORT IECInstancePtr  _GetSource () const override;
    ECOBJECTS_EXPORT void            _SetTarget (IECInstanceP instance) override;
    ECOBJECTS_EXPORT IECInstancePtr  _GetTarget () const override;
    ECOBJECTS_EXPORT ECObjectsStatus _GetSourceOrderId (int64_t& sourceOrderId) const override;
    ECOBJECTS_EXPORT ECObjectsStatus _GetTargetOrderId (int64_t& targetOrderId) const override;
    // IECInstance
    ECOBJECTS_EXPORT Utf8String          _GetInstanceId () const override;
    ECOBJECTS_EXPORT ECObjectsStatus     _SetInstanceId (Utf8CP id) override;
    ECOBJECTS_EXPORT bool                _IsReadOnly () const override;
    ECOBJECTS_EXPORT ECObjectsStatus     _GetValue (ECValueR v, uint32_t propertyIndex, bool useArrayIndex, uint32_t arrayIndex) const override;
    ECOBJECTS_EXPORT ECObjectsStatus     _SetValue (uint32_t propertyIndex, ECValueCR v, bool useArrayIndex, uint32_t arrayIndex) override;
    ECOBJECTS_EXPORT ECObjectsStatus     _SetInternalValue (uint32_t propertyIndex, ECValueCR v, bool useArrayIndex, uint32_t arrayIndex) override;
    ECOBJECTS_EXPORT ECObjectsStatus     _GetIsPropertyNull (bool& isNull, uint32_t propertyIndex, bool useArrayIndex, uint32_t arrayIndex) const override;
    ECObjectsStatus _GetShouldSerializeProperty(bool& serialize, uint32_t propertyIndex, bool useArrayIndex, uint32_t arrayIndex) const override 
        {return GetShouldSerializeProperty(serialize, propertyIndex, useArrayIndex, arrayIndex);}

    ECOBJECTS_EXPORT ECObjectsStatus     _InsertArrayElements (uint32_t propIdx, uint32_t index, uint32_t size) override;
    ECOBJECTS_EXPORT ECObjectsStatus     _AddArrayElements (uint32_t propIdx, uint32_t size) override;
    ECOBJECTS_EXPORT ECObjectsStatus     _RemoveArrayElement (uint32_t propIdx, uint32_t index) override;
    ECOBJECTS_EXPORT ECObjectsStatus     _ClearArray (uint32_t propIdx) override;
    ECOBJECTS_EXPORT Utf8String          _ToString (Utf8CP indent) const override;
    ECOBJECTS_EXPORT ClassLayoutCR       _GetClassLayout () const override;
    ECOBJECTS_EXPORT ECEnablerCR         _GetEnabler () const override;
    ECOBJECTS_EXPORT MemoryECInstanceBase* _GetAsMemoryECInstance () const override;
    ECOBJECTS_EXPORT virtual size_t _GetObjectSize () const;
    ECOBJECTS_EXPORT size_t _GetOffsetToIECInstance () const override;
    // MemoryECInstanceBase
    ECOBJECTS_EXPORT IECInstanceP            _GetAsIECInstance () const override;

public:
    //! Returns the RelationshipEnabler for the RelationshipClass that this RelationshipInstance represents
    ECOBJECTS_EXPORT StandaloneECRelationshipEnablerCR  GetRelationshipEnabler() const;  
    //! Returns the RelationshipClass that this Instance is an instance of
    ECOBJECTS_EXPORT ECRelationshipClassCR              GetRelationshipClass () const;
    //! Gets the name of this RelationshipInstance
    ECOBJECTS_EXPORT Utf8CP                             GetName(); 
    //! Sets the name of this RelationshipInstance
    ECOBJECTS_EXPORT void                               SetName(Utf8CP name); 
    };

//=======================================================================================
//! ECEnabler for Standalone ECRelationshipInstances (IECInstances not tied to a specific persistent store)
//=======================================================================================
struct StandaloneECRelationshipEnabler : public IECRelationshipEnabler, public StandaloneECEnabler
   {
private:
    ECN::ECRelationshipClassCR      m_relationshipClass;

    StandaloneECRelationshipEnabler (ECN::ECRelationshipClassCR ecClass, ClassLayoutR classLayout, IStandaloneEnablerLocaterP structStandaloneEnablerLocater);
    ECOBJECTS_EXPORT ~StandaloneECRelationshipEnabler ();

protected:
    virtual IECWipRelationshipInstancePtr _CreateWipRelationshipInstance () const;
    virtual ECN::ECRelationshipClassCR    _GetRelationshipClass() const;

public:
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

