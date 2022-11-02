/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "ECObjectsPch.h"

BEGIN_BENTLEY_ECOBJECT_NAMESPACE

///////////////////////////////////////////////////////////////////////////////////////////
//  StandaloneECRelationshipInstance
///////////////////////////////////////////////////////////////////////////////////////////
/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/        
StandaloneECRelationshipInstance::StandaloneECRelationshipInstance (StandaloneECRelationshipEnablerCR relationshipEnabler) :
        MemoryECInstanceBase (relationshipEnabler.GetClassLayout(), 0, true, relationshipEnabler.GetClass())
    {
    m_relationshipEnabler = &relationshipEnabler;
    m_relationshipEnabler->AddRef();   // make sure relationship enabler stays around
    m_target = NULL;
    m_source = NULL;
    m_sourceOrderId = 0;
    m_targetOrderId = 0; 
    }
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
StandaloneECRelationshipInstance::~StandaloneECRelationshipInstance()
    {
    m_relationshipEnabler->Release();

    //LOG.tracev (L"StandaloneECRelationshipInstance at 0x%x is being destructed. It references enabler 0x%x", this, m_sharedWipEnabler);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
size_t                StandaloneECRelationshipInstance::_GetOffsetToIECInstance () const
    {
    ECN::IECInstanceCP iecInstanceP   = dynamic_cast<ECN::IECInstanceCP>(this);
    Byte const* baseAddressOfIECInstance = (Byte const *)iecInstanceP;
    Byte const* baseAddressOfConcrete = (Byte const *)this;
    return (size_t)(baseAddressOfIECInstance - baseAddressOfConcrete);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
size_t                StandaloneECRelationshipInstance::_GetObjectSize () const
    {
    size_t objectSize = sizeof(*this);
    size_t primaryInstanceDataSize = (size_t)_GetBytesAllocated(); //GetBytesUsed();
    size_t perPropertyDataSize = sizeof(uint32_t) * GetPerPropertyFlagsSize();
    size_t supportingInstanceDataSize = 0; // CalculateSupportingInstanceDataSize ();

    return objectSize+primaryInstanceDataSize+perPropertyDataSize+supportingInstanceDataSize;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
IECInstanceP      StandaloneECRelationshipInstance::_GetAsIECInstance () const
    {
    return const_cast<StandaloneECRelationshipInstance*>(this);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
MemoryECInstanceBase* StandaloneECRelationshipInstance::_GetAsMemoryECInstance () const
    {
    return const_cast<StandaloneECRelationshipInstance*>(this);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/    
ECEnablerCR         StandaloneECRelationshipInstance::_GetEnabler() const
    {
    return *m_relationshipEnabler;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/    
Utf8String        StandaloneECRelationshipInstance::_GetInstanceId() const
    {
    return m_instanceId;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/    
ECObjectsStatus StandaloneECRelationshipInstance::_SetInstanceId (Utf8CP instanceId)
    {
    m_instanceId = instanceId;
    return ECObjectsStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ClassLayoutCR       StandaloneECRelationshipInstance::_GetClassLayout () const
    {
    return m_relationshipEnabler->GetClassLayout();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool                StandaloneECRelationshipInstance::_IsReadOnly() const
    {
    return false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus           StandaloneECRelationshipInstance::_GetValue (ECValueR v, uint32_t propertyIndex, bool useArrayIndex, uint32_t arrayIndex) const
    {
    return GetValueFromMemory (v, propertyIndex, useArrayIndex, arrayIndex);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus           StandaloneECRelationshipInstance::_GetIsPropertyNull (bool& isNull, uint32_t propertyIndex, bool useArrayIndex, uint32_t arrayIndex) const
    {
    return GetIsNullValueFromMemory (isNull, propertyIndex, useArrayIndex, arrayIndex);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus           StandaloneECRelationshipInstance::_SetValue (uint32_t propertyIndex, ECValueCR v, bool useArrayIndex, uint32_t arrayIndex)
    {
    SetIsLoadedBit (propertyIndex);

    return SetValueToMemory (propertyIndex, v, useArrayIndex, arrayIndex);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus StandaloneECRelationshipInstance::_SetInternalValue (uint32_t propertyIndex, ECValueCR v, bool useArrayIndex, uint32_t arrayIndex)
    {
    return SetValueInternal (propertyIndex, v, useArrayIndex, arrayIndex);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus           StandaloneECRelationshipInstance::_InsertArrayElements (uint32_t propIdx, uint32_t index, uint32_t size)
    {
    ECObjectsStatus status = InsertNullArrayElementsAt (propIdx, index, size);
    
    return status;
    } 
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus           StandaloneECRelationshipInstance::_AddArrayElements (uint32_t propIdx, uint32_t size)
    {
    ECObjectsStatus status = AddNullArrayElementsAt (propIdx, size);
    
    return status;
    }        

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus           StandaloneECRelationshipInstance::_RemoveArrayElement (uint32_t propIdx, uint32_t index)
    {
    ECObjectsStatus status = RemoveArrayElementsAt (propIdx, index, 1);
    
    return status;
    } 

 /*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus           StandaloneECRelationshipInstance::_ClearArray (uint32_t propIdx)
    {
    return ECObjectsStatus::OperationNotSupported;
    }                      

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/    
Utf8String        StandaloneECRelationshipInstance::_ToString (Utf8CP indent) const
    {
    return InstanceDataToString (indent);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void            StandaloneECRelationshipInstance::_SetSource (IECInstanceP instance) 
    {
    if (NULL == instance)
        return;

    if (GetRelationshipClass().GetSource().SupportsClass(instance->GetClass()))
        {
        m_source = instance;
        return;
        }

    BeAssert(false && "Invalid source instance");
    LOG.warningv ("Invalid source instance of class '%s' for relationship class %s", instance->GetClass().GetName().c_str(), GetRelationshipClass().GetName().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
IECInstancePtr  StandaloneECRelationshipInstance::_GetSource () const 
    {
    return m_source;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus StandaloneECRelationshipInstance::_GetSourceOrderId (int64_t& sourceOrderId) const 
    {
    sourceOrderId = m_sourceOrderId;
    return ECObjectsStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
StandaloneECRelationshipEnablerCR  StandaloneECRelationshipInstance::GetRelationshipEnabler() const
    {
    return *m_relationshipEnabler; 
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ECRelationshipClassCR                      StandaloneECRelationshipInstance::GetRelationshipClass () const
    {
    return m_relationshipEnabler->GetRelationshipClass ();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void            StandaloneECRelationshipInstance::_SetTarget (IECInstanceP instance) 
    {
    if (NULL == instance)
        return;

    if (GetRelationshipClass().GetTarget().SupportsClass(instance->GetClass()))
        {
        m_target = instance;
        return;
        }

    BeAssert(false && "Invalid target instance");
    LOG.warningv ("Invalid target instance of class '%s' for relationship class %s", instance->GetClass().GetName().c_str(), GetRelationshipClass().GetName().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
IECInstancePtr  StandaloneECRelationshipInstance::_GetTarget () const 
    {
    return m_target;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus StandaloneECRelationshipInstance::_GetTargetOrderId (int64_t& targetOrderId) const 
    {
    targetOrderId = m_targetOrderId;
    return ECObjectsStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8CP     StandaloneECRelationshipInstance::GetName() 
    {
    return m_name.c_str();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void     StandaloneECRelationshipInstance::SetName (Utf8CP name) 
    {
    if (name)
        m_name = name;
    }


///////////////////////////////////////////////////////////////////////////////////////////////
// StandaloneECRelationshipEnabler
//////////////////////////////////////////////////////////////////////////////////////////////
/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
StandaloneECRelationshipEnabler::StandaloneECRelationshipEnabler (ECRelationshipClassCR ecClass, ClassLayoutR classLayout, IStandaloneEnablerLocaterP structStandaloneEnablerLocater) :
    StandaloneECEnabler (ecClass, classLayout, structStandaloneEnablerLocater),
    m_relationshipClass (ecClass)
    {
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ClassLayoutCR StandaloneECRelationshipEnabler::GetClassLayout() const
    {
    return GetClass().GetDefaultStandaloneEnabler()->GetClassLayout();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
StandaloneECRelationshipEnabler::~StandaloneECRelationshipEnabler ()
    {
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
IECWipRelationshipInstancePtr StandaloneECRelationshipEnabler::_CreateWipRelationshipInstance () const
    {
    // not supported
    return  NULL;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ECN::ECRelationshipClassCR     StandaloneECRelationshipEnabler::_GetRelationshipClass()  const
    {
    return m_relationshipClass;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ECEnablerCR StandaloneECRelationshipEnabler::GetECEnabler() const
    {
    return *this;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
StandaloneECRelationshipEnablerPtr StandaloneECRelationshipEnabler::CreateStandaloneRelationshipEnabler (ECN::ECRelationshipClassCR ecClass)
    {
    return new StandaloneECRelationshipEnabler (ecClass, ecClass.GetDefaultStandaloneEnabler()->GetClassLayout(), NULL);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
StandaloneECRelationshipEnablerPtr StandaloneECRelationshipEnabler::CreateStandaloneRelationshipEnabler (ECN::ECRelationshipClassCR ecClass, ClassLayoutR classLayout, IStandaloneEnablerLocaterP structStandaloneEnablerLocater)
    {
    return new StandaloneECRelationshipEnabler (ecClass, classLayout, structStandaloneEnablerLocater);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
StandaloneECRelationshipInstancePtr StandaloneECRelationshipEnabler::CreateRelationshipInstance() const
    {
    if (ECClassModifier::Abstract == GetClass().GetClassModifier())
        {
        LOG.errorv("A StandaloneECRelationshipInstance could not be created for class %s because it is an abstract class.",
                   GetClass().GetFullName());
        return nullptr;
        }

    return new StandaloneECRelationshipInstance (*this);
    }

END_BENTLEY_ECOBJECT_NAMESPACE
