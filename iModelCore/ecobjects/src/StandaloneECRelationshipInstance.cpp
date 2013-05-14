/*--------------------------------------------------------------------------------------+
|
|     $Source: src/StandaloneECRelationshipInstance.cpp $
|
|   $Copyright: (c) 2013 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ECObjectsPch.h"

BEGIN_BENTLEY_ECOBJECT_NAMESPACE

///////////////////////////////////////////////////////////////////////////////////////////
//  StandaloneECRelationshipInstance
///////////////////////////////////////////////////////////////////////////////////////////
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    CaseyMullen     01/10
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
* @bsimethod                                    Bill.Steinbock                  04/2012
+---------------+---------------+---------------+---------------+---------------+------*/
StandaloneECRelationshipInstance::~StandaloneECRelationshipInstance()
    {
    m_relationshipEnabler->Release();

    //LOG.tracev (L"StandaloneECRelationshipInstance at 0x%x is being destructed. It references enabler 0x%x", this, m_sharedWipEnabler);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Bill.Steinbock                  12/2010
+---------------+---------------+---------------+---------------+---------------+------*/
size_t                StandaloneECRelationshipInstance::_GetOffsetToIECInstance () const
    {
    ECN::IECInstanceCP iecInstanceP   = dynamic_cast<ECN::IECInstanceCP>(this);
    byte const* baseAddressOfIECInstance = (byte const *)iecInstanceP;
    byte const* baseAddressOfConcrete = (byte const *)this;

    return (size_t)(baseAddressOfIECInstance - baseAddressOfConcrete);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Bill.Steinbock                  12/2010
+---------------+---------------+---------------+---------------+---------------+------*/
size_t                StandaloneECRelationshipInstance::_GetObjectSize () const
    {
    size_t objectSize = sizeof(*this);
    size_t primaryInstanceDataSize = (size_t)_GetBytesAllocated(); //GetBytesUsed();
    size_t perPropertyDataSize = sizeof(UInt32) * GetPerPropertyFlagsSize();
    size_t supportingInstanceDataSize = 0; // CalculateSupportingInstanceDataSize ();

    return objectSize+primaryInstanceDataSize+perPropertyDataSize+supportingInstanceDataSize;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Bill.Steinbock                  12/2010
+---------------+---------------+---------------+---------------+---------------+------*/
size_t                StandaloneECRelationshipInstance::_LoadObjectDataIntoManagedInstance (byte* managedBuffer) const
    {
    size_t size = sizeof(*this);
    memcpy (managedBuffer, this, size);
    return size;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Bill.Steinbock                  12/2010
+---------------+---------------+---------------+---------------+---------------+------*/
IECInstanceP      StandaloneECRelationshipInstance::_GetAsIECInstance () const
    {
    return const_cast<StandaloneECRelationshipInstance*>(this);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Bill.Steinbock                  12/2010
+---------------+---------------+---------------+---------------+---------------+------*/
MemoryECInstanceBase* StandaloneECRelationshipInstance::_GetAsMemoryECInstance () const
    {
    return const_cast<StandaloneECRelationshipInstance*>(this);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    CaseyMullen     10/09
+---------------+---------------+---------------+---------------+---------------+------*/    
ECEnablerCR         StandaloneECRelationshipInstance::_GetEnabler() const
    {
    return *m_relationshipEnabler;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    CaseyMullen     10/09
+---------------+---------------+---------------+---------------+---------------+------*/    
WString        StandaloneECRelationshipInstance::_GetInstanceId() const
    {
    return m_instanceId;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    05/11
+---------------+---------------+---------------+---------------+---------------+------*/    
ECObjectsStatus StandaloneECRelationshipInstance::_SetInstanceId (WCharCP instanceId)
    {
    m_instanceId = instanceId;
    return ECOBJECTS_STATUS_Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    CaseyMullen     01/10
+---------------+---------------+---------------+---------------+---------------+------*/
ClassLayoutCR       StandaloneECRelationshipInstance::_GetClassLayout () const
    {
    return m_relationshipEnabler->GetClassLayout();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    CaseyMullen     09/09
+---------------+---------------+---------------+---------------+---------------+------*/
bool                StandaloneECRelationshipInstance::_IsReadOnly() const
    {
    return false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    05/10
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus           StandaloneECRelationshipInstance::_GetValue (ECValueR v, UInt32 propertyIndex, bool useArrayIndex, UInt32 arrayIndex) const
    {
    return GetValueFromMemory (v, propertyIndex, useArrayIndex, arrayIndex);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Bill.Steinbock                  08/2012
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus           StandaloneECRelationshipInstance::_GetIsPropertyNull (bool& isNull, UInt32 propertyIndex, bool useArrayIndex, UInt32 arrayIndex) const
    {
    return GetIsNullValueFromMemory (isNull, propertyIndex, useArrayIndex, arrayIndex);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    05/10
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus           StandaloneECRelationshipInstance::_SetValue (UInt32 propertyIndex, ECValueCR v, bool useArrayIndex, UInt32 arrayIndex)
    {
    SetIsLoadedBit (propertyIndex);

    return SetValueToMemory (propertyIndex, v, useArrayIndex, arrayIndex);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   09/12
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus StandaloneECRelationshipInstance::_SetInternalValue (UInt32 propertyIndex, ECValueCR v, bool useArrayIndex, UInt32 arrayIndex)
    {
    return SetValueInternal (propertyIndex, v, useArrayIndex, arrayIndex);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Adam.Klatzkin                   01/2010
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus           StandaloneECRelationshipInstance::_InsertArrayElements (UInt32 propIdx, UInt32 index, UInt32 size)
    {
    ECObjectsStatus status = InsertNullArrayElementsAt (propIdx, index, size);
    
    return status;
    } 
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Adam.Klatzkin                   01/2010
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus           StandaloneECRelationshipInstance::_AddArrayElements (UInt32 propIdx, UInt32 size)
    {
    ECObjectsStatus status = AddNullArrayElementsAt (propIdx, size);
    
    return status;
    }        

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Bill.Steinbock                  07/2011
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus           StandaloneECRelationshipInstance::_RemoveArrayElement (UInt32 propIdx, UInt32 index)
    {
    ECObjectsStatus status = RemoveArrayElementsAt (propIdx, index, 1);
    
    return status;
    } 

 /*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Adam.Klatzkin                   01/2010
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus           StandaloneECRelationshipInstance::_ClearArray (UInt32 propIdx)
    {
    return ECOBJECTS_STATUS_OperationNotSupported;
    }                      

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    CaseyMullen     10/09
+---------------+---------------+---------------+---------------+---------------+------*/    
WString        StandaloneECRelationshipInstance::_ToString (WCharCP indent) const
    {
    return InstanceDataToString (indent);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Bill.Steinbock                  04/2012
+---------------+---------------+---------------+---------------+---------------+------*/
void            StandaloneECRelationshipInstance::_SetSource (IECInstanceP instance) 
    {
    if (NULL == instance)
        return;

    FOR_EACH (ECClassCP source, GetRelationshipClass().GetSource().GetClasses())
        {
        if (source->GetName().EqualsI (L"AnyClass") || instance->GetClass().Is(source))
            {
            m_source = instance;
            return;
            }
        }

    BeAssert(false && "Invalid source instance");
    LOG.warningv (L"Invalid source instance of class '%ls' for relationship class %ls", instance->GetClass().GetName().c_str(), GetRelationshipClass().GetName().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Bill.Steinbock                  04/2012
+---------------+---------------+---------------+---------------+---------------+------*/
IECInstancePtr  StandaloneECRelationshipInstance::_GetSource () const 
    {
    return m_source;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Bill.Steinbock                  04/2012
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus StandaloneECRelationshipInstance::_GetSourceOrderId (Int64& sourceOrderId) const 
    {
    sourceOrderId = m_sourceOrderId;
    return ECOBJECTS_STATUS_Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Bill.Steinbock                  04/2012
+---------------+---------------+---------------+---------------+---------------+------*/
StandaloneECRelationshipEnablerCR  StandaloneECRelationshipInstance::GetRelationshipEnabler() const
    {
    return *m_relationshipEnabler; 
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Bill.Steinbock                  04/2012
+---------------+---------------+---------------+---------------+---------------+------*/
ECRelationshipClassCR                      StandaloneECRelationshipInstance::GetRelationshipClass () const
    {
    return m_relationshipEnabler->GetRelationshipClass ();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Bill.Steinbock                  04/2012
+---------------+---------------+---------------+---------------+---------------+------*/
void            StandaloneECRelationshipInstance::_SetTarget (IECInstanceP instance) 
    {
    if (NULL == instance)
        return;

    FOR_EACH (ECClassCP target, this->GetRelationshipClass().GetTarget().GetClasses())
        {
        if (target->GetName().EqualsI (L"AnyClass") || instance->GetClass().Is(target))
            {
            m_target = instance;
            return;
            }
        }

    BeAssert(false && "Invalid target instance");
    LOG.warningv (L"Invalid target instance of class '%ls' for relationship class %ls", instance->GetClass().GetName().c_str(), GetRelationshipClass().GetName().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Bill.Steinbock                  04/2012
+---------------+---------------+---------------+---------------+---------------+------*/
IECInstancePtr  StandaloneECRelationshipInstance::_GetTarget () const 
    {
    return m_target;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Bill.Steinbock                  04/2012
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus StandaloneECRelationshipInstance::_GetTargetOrderId (Int64& targetOrderId) const 
    {
    targetOrderId = m_targetOrderId;
    return ECOBJECTS_STATUS_Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Bill.Steinbock                  04/2012
+---------------+---------------+---------------+---------------+---------------+------*/
WCharCP     StandaloneECRelationshipInstance::GetName() 
    {
    return m_name.c_str();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Bill.Steinbock                  04/2012
+---------------+---------------+---------------+---------------+---------------+------*/
void     StandaloneECRelationshipInstance::SetName (WCharCP name) 
    {
    if (name)
        m_name = name;
    }


///////////////////////////////////////////////////////////////////////////////////////////////
// StandaloneECRelationshipEnabler
//////////////////////////////////////////////////////////////////////////////////////////////
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Bill.Steinbock                  04/2012
+---------------+---------------+---------------+---------------+---------------+------*/
StandaloneECRelationshipEnabler::StandaloneECRelationshipEnabler (ECRelationshipClassCR ecClass) :
    ECEnabler (ecClass, NULL),
    ClassLayoutHolder (ecClass.GetDefaultStandaloneEnabler()->GetClassLayout()),
    m_relationshipClass (ecClass)
    {
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Bill.Steinbock                  04/2012
+---------------+---------------+---------------+---------------+---------------+------*/
StandaloneECRelationshipEnabler::~StandaloneECRelationshipEnabler ()
    {
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Bill.Steinbock                  04/2012
+---------------+---------------+---------------+---------------+---------------+------*/
WCharCP           StandaloneECRelationshipEnabler::_GetName() const
    {
    return L"Bentley::ECN::StandaloneECEnabler";
    }
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    05/10
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus StandaloneECRelationshipEnabler::_GetPropertyIndex(UInt32& propertyIndex, WCharCP propertyAccessString) const { return GetClassLayout().GetPropertyIndex (propertyIndex, propertyAccessString); }
ECObjectsStatus StandaloneECRelationshipEnabler::_GetAccessString(WCharCP& accessString, UInt32 propertyIndex) const { return GetClassLayout().GetAccessStringByIndex (accessString, propertyIndex); }
UInt32          StandaloneECRelationshipEnabler::_GetFirstPropertyIndex (UInt32 parentIndex) const {  return GetClassLayout().GetFirstChildPropertyIndex (parentIndex); }
UInt32          StandaloneECRelationshipEnabler::_GetNextPropertyIndex (UInt32 parentIndex, UInt32 inputIndex) const { return GetClassLayout().GetNextChildPropertyIndex (parentIndex, inputIndex);  }
ECObjectsStatus StandaloneECRelationshipEnabler::_GetPropertyIndices (bvector<UInt32>& indices, UInt32 parentIndex) const { return GetClassLayout().GetPropertyIndices (indices, parentIndex);  }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Paul.Connelly                   12/11
+---------------+---------------+---------------+---------------+---------------+------*/
bool            StandaloneECRelationshipEnabler::_HasChildProperties (UInt32 parentIndex) const
    {
    return GetClassLayout().HasChildProperties (parentIndex);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Bill.Steinbock                  04/2012
+---------------+---------------+---------------+---------------+---------------+------*/
IECWipRelationshipInstancePtr StandaloneECRelationshipEnabler::_CreateWipRelationshipInstance () const
    {
    // not supported
    return  NULL;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Bill.Steinbock                  04/2012
+---------------+---------------+---------------+---------------+---------------+------*/
ECN::ECRelationshipClassCR     StandaloneECRelationshipEnabler::_GetRelationshipClass()  const
    {
    return m_relationshipClass;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   06/12
+---------------+---------------+---------------+---------------+---------------+------*/
ECEnablerCR StandaloneECRelationshipEnabler::GetECEnabler() const
    {
    return *this;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Bill.Steinbock                  04/2012
+---------------+---------------+---------------+---------------+---------------+------*/
StandaloneECRelationshipEnablerPtr StandaloneECRelationshipEnabler::CreateStandaloneRelationshipEnabler (ECN::ECRelationshipClassCR ecClass)
    {
    return new StandaloneECRelationshipEnabler (ecClass);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Bill.Steinbock                  04/2012
+---------------+---------------+---------------+---------------+---------------+------*/
StandaloneECRelationshipInstancePtr       StandaloneECRelationshipEnabler::CreateRelationshipInstance () const
    {
    return new StandaloneECRelationshipInstance (*this);
    }

END_BENTLEY_ECOBJECT_NAMESPACE
