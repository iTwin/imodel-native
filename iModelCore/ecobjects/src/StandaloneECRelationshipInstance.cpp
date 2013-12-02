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
StandaloneECRelationshipInstance::StandaloneECRelationshipInstance (StandaloneECRelationshipEnablerR relationshipEnabler) :
        StandaloneECInstance (relationshipEnabler, 0)
    {
    m_relationshipEnabler = &relationshipEnabler;
    m_relationshipEnabler->AddRef();   // make sure relationship enabler stays around
    m_target = NULL;
    m_source = NULL;
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
IECInstanceP      StandaloneECRelationshipInstance::_GetAsIECInstance () const
    {
    return const_cast<StandaloneECRelationshipInstance*>(this);
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

    FOR_EACH (ECClassCP target, this->GetRelationshipClass().GetTarget().GetClasses())
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
ECObjectsStatus StandaloneECRelationshipInstance::_SetSourceOrderId (Int64 sourceOrderId)
    {
    m_sourceOrderId = sourceOrderId;
    return ECOBJECTS_STATUS_Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sylvain.Pucci                 12/2013
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus StandaloneECRelationshipInstance::_SetTargetOrderId (Int64 targetOrderId)
    {
    m_targetOrderId = targetOrderId;
    return ECOBJECTS_STATUS_Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sylvain.Pucci                 12/2013
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus StandaloneECRelationshipInstance::SetSourceOrderId (Int64 sourceOrderId)
    {
    return _SetSourceOrderId (sourceOrderId);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sylvain.Pucci                 12/2013
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus StandaloneECRelationshipInstance::SetTargetOrderId (Int64 targetOrderId)
    {
    return _SetTargetOrderId (targetOrderId);
    }

///////////////////////////////////////////////////////////////////////////////////////////////
//   IECWipRelationshipInstance
///////////////////////////////////////////////////////////////////////////////////////////////

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sylvain.Pucci                  11/2013
+---------------+---------------+---------------+---------------+---------------+------*/
 IECWipRelationshipInstance::IECWipRelationshipInstance (StandaloneECRelationshipEnablerR relationshipEnabler) 
     : StandaloneECRelationshipInstance (relationshipEnabler)
     {
     m_isSourceOrderIdDefined = false;
     m_isTargetOrderIdDefined = false;
     m_sourceNextOrderId = 0;
     m_isSourceNextOrderIdDefined = false;
     m_targetNextOrderId = 0;
     m_isTargetNextOrderIdDefined = false;
     }
 
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sylvain.Pucci                  11/2013
+---------------+---------------+---------------+---------------+---------------+------*/
 ECObjectsStatus IECWipRelationshipInstance::_SetSourceOrderId (Int64 sourceOrderId)
     {
     m_isSourceOrderIdDefined = true;
     return StandaloneECRelationshipInstance::_SetSourceOrderId (sourceOrderId);
     }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sylvain.Pucci                  11/2013
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus IECWipRelationshipInstance::_SetTargetOrderId (Int64 targetOrderId)
     {
     m_isTargetOrderIdDefined = true;
     return StandaloneECRelationshipInstance::_SetTargetOrderId (targetOrderId);
     }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sylvain.Pucci                  11/2013
+---------------+---------------+---------------+---------------+---------------+------*/
 ECObjectsStatus IECWipRelationshipInstance::SetSourceNextOrderId (Int64 sourceOrderId)
     {
     m_sourceNextOrderId = sourceOrderId;
     m_isSourceNextOrderIdDefined = true;
     return ECOBJECTS_STATUS_Success;
     }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sylvain.Pucci                  11/2013
+---------------+---------------+---------------+---------------+---------------+------*/
 ECObjectsStatus IECWipRelationshipInstance::SetTargetNextOrderId (Int64 sourceOrderId)
     {
     m_targetNextOrderId = sourceOrderId;
     m_isTargetNextOrderIdDefined = true;
     return ECOBJECTS_STATUS_Success;
     }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sylvain.Pucci                  11/2013
+---------------+---------------+---------------+---------------+---------------+------*/
 ECObjectsStatus IECWipRelationshipInstance::_GetSourceOrderId (Int64& orderId) const
     {
     if (!m_isSourceOrderIdDefined)
         return ECOBJECTS_STATUS_Error;
     return StandaloneECRelationshipInstance::_GetSourceOrderId (orderId);
     }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sylvain.Pucci                  11/2013
+---------------+---------------+---------------+---------------+---------------+------*/
 ECObjectsStatus IECWipRelationshipInstance::_GetTargetOrderId (Int64& orderId) const
     {
     if (!m_isTargetOrderIdDefined)
         return ECOBJECTS_STATUS_Error;
     return StandaloneECRelationshipInstance::_GetSourceOrderId (orderId);
     }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sylvain.Pucci                  11/2013
+---------------+---------------+---------------+---------------+---------------+------*/
 ECObjectsStatus IECWipRelationshipInstance::GetSourceNextOrderId (Int64& orderId) const
     {
     if (!m_isSourceNextOrderIdDefined)
         return ECOBJECTS_STATUS_Error;
     orderId = m_sourceNextOrderId;
     return ECOBJECTS_STATUS_Success;
     }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sylvain.Pucci                  11/2013
+---------------+---------------+---------------+---------------+---------------+------*/
 ECObjectsStatus IECWipRelationshipInstance::GetTargetNextOrderId (Int64& orderId) const
     {
     if (!m_isTargetNextOrderIdDefined)
         return ECOBJECTS_STATUS_Error;
     orderId = m_targetNextOrderId;
     return ECOBJECTS_STATUS_Success;
     }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sylvain.Pucci                  11/2013
+---------------+---------------+---------------+---------------+---------------+------*/
 bool IECWipRelationshipInstance::IsSourceOrderIdDefined ()
     {
     return m_isSourceOrderIdDefined;
     }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sylvain.Pucci                  11/2013
+---------------+---------------+---------------+---------------+---------------+------*/
 bool IECWipRelationshipInstance::IsTargetOrderIdDefined ()
     {
     return m_isTargetOrderIdDefined;
     }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sylvain.Pucci                  11/2013
+---------------+---------------+---------------+---------------+---------------+------*/
 bool IECWipRelationshipInstance::IsSourceNextOrderIdDefined ()
     {
     return m_isSourceNextOrderIdDefined;
     }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sylvain.Pucci                  11/2013
+---------------+---------------+---------------+---------------+---------------+------*/
 bool IECWipRelationshipInstance::IsTargetNextOrderIdDefined ()
     {
     return m_isTargetNextOrderIdDefined;
     }

 /*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Bill.Steinbock                  04/2012
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus IECWipRelationshipInstance::SetPropertiesString (WCharCP propertiesString) 
    {
    if (!propertiesString)
        return ECOBJECTS_STATUS_Error;
    m_propertiesString = propertiesString;
    return ECOBJECTS_STATUS_Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Bill.Steinbock                  04/2012
+---------------+---------------+---------------+---------------+---------------+------*/
WCharCP     IECWipRelationshipInstance::GetPropertiesString() const
    {
    if (m_propertiesString.empty())
        return NULL;
    return m_propertiesString.c_str();
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
* @bsimethod                                                    Paul.Connelly   06/13
+---------------+---------------+---------------+---------------+---------------+------*/
//ClassLayoutCR StandaloneECRelationshipEnabler::GetClassLayout() const
//    {
//    return GetClass().GetDefaultStandaloneEnabler()->GetClassLayout();
//    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Bill.Steinbock                  04/2012
+---------------+---------------+---------------+---------------+---------------+------*/
StandaloneECRelationshipEnabler::~StandaloneECRelationshipEnabler ()
    {
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
