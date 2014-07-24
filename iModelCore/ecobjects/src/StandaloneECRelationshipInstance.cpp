/*--------------------------------------------------------------------------------------+
|
|     $Source: src/StandaloneECRelationshipInstance.cpp $
|
|   $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
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

    for (ECClassCP source: GetRelationshipClass().GetSource().GetClasses())
        {
        if (instance->GetClass().Is(source))
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

    for (ECClassCP target: this->GetRelationshipClass().GetTarget().GetClasses())
        {
        if (instance->GetClass().Is(target))
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
* @bsimethod                                    Sylvain.Pucci                 12/2013
+---------------+---------------+---------------+---------------+---------------+------*/
OrderIdEntries& StandaloneECRelationshipInstance::OrderIdEntries()
    {
    return m_orderIdEntries;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sylvain.Pucci                 12/2013
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus StandaloneECRelationshipInstance::_GetSourceOrderId (Int64& sourceOrderId) const
    {
    sourceOrderId = m_sourceOrderId;
    return ECOBJECTS_STATUS_Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sylvain.Pucci                 12/2013
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus StandaloneECRelationshipInstance::_GetTargetOrderId (Int64& targetOrderId) const
    {
    targetOrderId = m_targetOrderId;
    return ECOBJECTS_STATUS_Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sylvain.Pucci                 12/2013
+---------------+---------------+---------------+---------------+---------------+------*/        
OrderIdEntries::OrderIdEntries () 
    {
    Clear();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sylvain.Pucci                 12/2013
+---------------+---------------+---------------+---------------+---------------+------*/        
ECObjectsStatus OrderIdEntries::Clear () 
    {
    m_sourceOrderId = 0;
    m_targetOrderId = 0; 
    m_isSourceOrderIdDefined = false;
    m_isTargetOrderIdDefined = false;
    m_sourceNextOrderId = 0;
    m_isSourceNextOrderIdDefined = false;
    m_targetNextOrderId = 0;
    m_isTargetNextOrderIdDefined = false;
    return ECOBJECTS_STATUS_Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sylvain.Pucci                 12/2013
+---------------+---------------+---------------+---------------+---------------+------*/
bool OrderIdEntries::TryGetSourceOrderId (Int64& sourceOrderId) const
    {
    sourceOrderId = 0;
     if (!m_isSourceOrderIdDefined)
         return false;
    sourceOrderId = m_sourceOrderId;
    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sylvain.Pucci                 12/2013
+---------------+---------------+---------------+---------------+---------------+------*/
bool OrderIdEntries::TryGetTargetOrderId (Int64& targetOrderId) const
    {
    targetOrderId = 0;
     if (!m_isTargetOrderIdDefined)
         return false;
    targetOrderId = m_targetOrderId;
    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sylvain.Pucci                 12/2013
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus OrderIdEntries::SetSourceOrderId (Int64 sourceOrderId)
    {
    m_isSourceNextOrderIdDefined = false;
    m_isSourceOrderIdDefined = true;
    m_sourceOrderId = sourceOrderId;
    return ECOBJECTS_STATUS_Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sylvain.Pucci                 12/2013
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus OrderIdEntries::SetTargetOrderId (Int64 targetOrderId)
    {
    m_isTargetNextOrderIdDefined = false;
    m_isTargetOrderIdDefined = true;
    m_targetOrderId = targetOrderId;
    return ECOBJECTS_STATUS_Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sylvain.Pucci                 12/2013
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus StandaloneECRelationshipInstance::SetSourceOrderId (Int64 sourceOrderId)
    {
    m_sourceOrderId = sourceOrderId;
    return ECOBJECTS_STATUS_Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sylvain.Pucci                 12/2013
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus StandaloneECRelationshipInstance::SetTargetOrderId (Int64 targetOrderId)
    {
    m_targetOrderId = targetOrderId;
    return ECOBJECTS_STATUS_Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sylvain.Pucci                  11/2013
+---------------+---------------+---------------+---------------+---------------+------*/
 ECObjectsStatus OrderIdEntries::SetSourceNextOrderId (Int64 sourceOrderId)
     {
     m_isSourceOrderIdDefined = false;
     m_isSourceNextOrderIdDefined = true;
     m_sourceNextOrderId = sourceOrderId;
     return ECOBJECTS_STATUS_Success;
     }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sylvain.Pucci                  11/2013
+---------------+---------------+---------------+---------------+---------------+------*/
 ECObjectsStatus OrderIdEntries::SetTargetNextOrderId (Int64 sourceOrderId)
     {
     m_isSourceOrderIdDefined = false;
     m_isTargetNextOrderIdDefined = true;
     m_targetNextOrderId = sourceOrderId;
     return ECOBJECTS_STATUS_Success;
     }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sylvain.Pucci                  11/2013
+---------------+---------------+---------------+---------------+---------------+------*/
 bool OrderIdEntries::TryGetSourceNextOrderId (Int64& orderId) const
     {
     if (!m_isSourceNextOrderIdDefined)
         return false;
     orderId = m_sourceNextOrderId;
     return true;
     }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sylvain.Pucci                  11/2013
+---------------+---------------+---------------+---------------+---------------+------*/
 bool OrderIdEntries::TryGetTargetNextOrderId (Int64& orderId) const
     {
     if (!m_isTargetNextOrderIdDefined)
         return false;
     orderId = m_targetNextOrderId;
     return true;
     }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sylvain.Pucci                  07/2014
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus StandaloneECRelationshipInstance::SetSourceAssociatedString (WCharCP propertiesString) 
    {
    if (!propertiesString)
        return ECOBJECTS_STATUS_Error;
    m_sourceAssociatedString = propertiesString;
    return ECOBJECTS_STATUS_Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sylvain.Pucci                  07/2014
+---------------+---------------+---------------+---------------+---------------+------*/
WCharCP     StandaloneECRelationshipInstance::GetSourceAssociatedString() const
    {
    return m_sourceAssociatedString.c_str();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sylvain.Pucci                  07/2014
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus StandaloneECRelationshipInstance::SetTargetAssociatedString (WCharCP propertiesString) 
    {
    if (!propertiesString)
        return ECOBJECTS_STATUS_Error;
    m_targetAssociatedString = propertiesString;
    return ECOBJECTS_STATUS_Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sylvain.Pucci                  07/2014
+---------------+---------------+---------------+---------------+---------------+------*/
WCharCP     StandaloneECRelationshipInstance::GetTargetAssociatedString() const
    {
    return m_targetAssociatedString.c_str();
    }

///////////////////////////////////////////////////////////////////////////////////////////////
// StandaloneECRelationshipEnabler
//////////////////////////////////////////////////////////////////////////////////////////////
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Bill.Steinbock                  04/2012
+---------------+---------------+---------------+---------------+---------------+------*/
StandaloneECRelationshipEnabler::StandaloneECRelationshipEnabler (ECRelationshipClassCR ecClass, ClassLayoutR classLayout, IStandaloneEnablerLocaterP structStandaloneEnablerLocater) :
    StandaloneECEnabler (ecClass, classLayout, structStandaloneEnablerLocater),
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
StandaloneECRelationshipInstancePtr StandaloneECRelationshipEnabler::_CreateWipRelationshipInstance () const
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
    return new StandaloneECRelationshipEnabler (ecClass, ecClass.GetDefaultStandaloneEnabler()->GetClassLayout(), NULL);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Bill.Steinbock                  04/2012
+---------------+---------------+---------------+---------------+---------------+------*/
StandaloneECRelationshipEnablerPtr StandaloneECRelationshipEnabler::CreateStandaloneRelationshipEnabler (ECN::ECRelationshipClassCR ecClass, ClassLayoutR classLayout, IStandaloneEnablerLocaterP structStandaloneEnablerLocater)
    {
    return new StandaloneECRelationshipEnabler (ecClass, classLayout, structStandaloneEnablerLocater);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Bill.Steinbock                  04/2012
+---------------+---------------+---------------+---------------+---------------+------*/
StandaloneECRelationshipInstancePtr       StandaloneECRelationshipEnabler::CreateRelationshipInstance ()
    {
    return new StandaloneECRelationshipInstance (*this);
    }

END_BENTLEY_ECOBJECT_NAMESPACE
