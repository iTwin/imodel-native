/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnHandlers/DgnEC/ECXDRelationship.cpp $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include    <DgnPlatformInternal.h>

USING_NAMESPACE_BENTLEY_DGNPLATFORM
using namespace Bentley::DgnPlatform::LoggingHelpers;

USING_NAMESPACE_EC
USING_NAMESPACE_BENTLEY_LOGGING
BEGIN_BENTLEY_DGNPLATFORM_NAMESPACE

using namespace std;

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    CaseyMullen     01/10
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt               SetECPointerToNull (StandaloneECInstanceR wipInstance, ECRelationshipEnd end)
    {
    ECValue nullValue;

    return wipInstance.SetValue (wipInstance.GetClassLayout().GetECPointerIndex (end), nullValue);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    CaseyMullen     01/10
+---------------+---------------+---------------+---------------+---------------+------*/
void                    SerializeECPointer (DataExternalizer& stream, DgnElementECInstanceCR instance, ElementRefP hostElementRef)
    {
    LocalKey localKey = instance.GetLocalKey();

    if (hostElementRef != instance.GetElementRef())
        {
        stream.put (localKey.m_providerId);
        stream.put (localKey.m_localId);
        PersistentElementPath pepA (&instance.GetDgnModel(), instance.GetElementRef());
        pepA.Store (&stream);
        return;
        }

    // Handle special-cases for "compression"
    // 0 bytes: providerId=ECXData, XAttributeId=1 (most common)
    // 2 bytes: providerId=ECXData, XAttributeId is as specified (assuming it can fit in 16 bits)
    // 4 bytes: providerId=ECXData, XAttributeId is as specified
    // 8 bytes: other ProviderId + LocalId
    // All of the above imply that it is on "this" element
    // For cases where we need a real PEP, we always have to store both 16 bit providerId and 32 bit localId.

    if (PROVIDERID_ECXData != localKey.m_providerId)
        {
        stream.put (localKey.m_providerId);
        stream.put (localKey.m_localId);
        // Will be 6 bytes
        return;
        }

    // ProviderId will be assumed to be PROVIDERID_ECXData
    if (1 == localKey.m_localId)
        return; // will be 0 bytes

    if (localKey.m_localId <= 0xFFFF)
        {
        stream.put ((UInt16)localKey.m_localId);
        // Will be 2 bytes
        return; 
        }
    else
        {
        stream.put ((UInt32)localKey.m_localId);
        // Will be 4 bytes
        return; 
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    CaseyMullen     01/10
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt               DeserializeECPointer (DgnECPointer& ecPointer, DataInternalizer& stream)
    {
    if (stream.getSize() > 6)
        {
        stream.get (&ecPointer.m_localKey.m_providerId);
        stream.get (&ecPointer.m_localKey.m_localId);
        StatusInt status = ecPointer.m_pep.Load (stream);

        BeAssert (SUCCESS == status && "Failed to load PEP from ECPointer");
        return status;
        }

    switch (stream.getSize())
        {
        case 0:
            ecPointer.m_localKey.m_localId = 1;
            ecPointer.m_localKey.m_providerId = PROVIDERID_ECXData;
            return SUCCESS;
        case 2:
            UInt16 localId;
            stream.get (&localId);
            ecPointer.m_localKey.m_localId = localId;
            ecPointer.m_localKey.m_providerId = PROVIDERID_ECXData;
            return SUCCESS;
        case 4:
            stream.get (&ecPointer.m_localKey.m_localId);
            ecPointer.m_localKey.m_providerId = PROVIDERID_ECXData;
            return SUCCESS;
        case 6:
            stream.get (&ecPointer.m_localKey.m_providerId);
            stream.get (&ecPointer.m_localKey.m_localId);
            return SUCCESS;
        default:
            BeAssert (false && "unrecognized DgnECPointer in stream");
            return ERROR;
        }
    }    
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    CaseyMullen     01/10
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt               SetECPointerValue (StandaloneECInstanceR wipInstance, ECRelationshipEnd end, DgnElementECInstanceCR instance, ElementRefP hostElementRef)
    {
    ECValue binaryValue;
    DgnECPointer::SetECPointerValue (binaryValue, instance, hostElementRef);
    
    StatusInt status = wipInstance.SetValue (wipInstance.GetClassLayout().GetECPointerIndex (end), binaryValue);
    BeAssert (SUCCESS == status);

#if just_checking
    ECValue v;
    status = wipInstance.GetValue (v, wipInstance.GetClassLayout().GetECPointerIndex (end));
    
    DgnECPointer outPointer;
    GetECPointerFromValue (outPointer, v);
#endif
    return status;
    }
        
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    CaseyMullen     01/10
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt               DgnECPointer::InitializeFromValue (ECValueR binaryValue, DgnModelR homeDgnModel, ElementRefP hostElementRef)
    {
    BeAssert (binaryValue.IsBinary());

    m_homeDgnModel = &homeDgnModel;
    m_hostElementRef = hostElementRef;

    size_t size;
    const byte* data = binaryValue.GetBinary (size);
    
    DataInternalizer stream (data, size);
    return DeserializeECPointer (*this, stream);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    CaseyMullen     02/10
+---------------+---------------+---------------+---------------+---------------+------*/
bool                    DgnECPointer::IsHost ()
    {
    return m_pep.IsEmpty();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    CaseyMullen     02/10
+---------------+---------------+---------------+---------------+---------------+------*/
ElementRefP             DgnECPointer::GetElementRef ()
    {
    if (NULL != m_elementRef)
        return m_elementRef;

    if (IsHost())
        {
        m_elementRef = m_hostElementRef;
        }
    else
        {
        ElementHandle element = m_pep.EvaluateElement (m_homeDgnModel);
        m_elementRef = element.GetElementRef();
        }

    return m_elementRef;
    }        

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    CaseyMullen     01/10
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt               DgnECPointer::SetECPointerValue (ECValueR binaryValue, DgnElementECInstanceCR instance, ElementRefP hostElementRef)
    {
    DataExternalizer stream;
    SerializeECPointer (stream, instance, hostElementRef);
    return binaryValue.SetBinary (stream.getBuf(), stream.getBytesWritten(), true);
    }    

//=======================================================================================
// @bsistruct                                            Casey.Mullen           02/10
//=======================================================================================
struct PEPEvaluator : PersistentElementPath::PathProcessor
    {
    DgnModelP   m_model;
    ElementRefP m_elementRef;
    ElementId   m_elementId;
    bool        m_isDeleted;

    PEPEvaluator () : m_elementId(0), m_elementRef(NULL), m_model(NULL), m_isDeleted(false)
        {
        }

    virtual void OnElementId (ElementId elementId, ElementRefP ref, DgnModelP model) override
        {
        m_elementRef = ref;
        m_elementId = elementId;
        m_model = model;
        
        if (NULL == ref)
            {
            m_isDeleted = true;
            if (NULL != model)
                m_elementRef = model->FindElementById (elementId);
            }
        }

    virtual void OnModelId (DgnModelId m, DgnModelP c, DgnProjectP) override
        {
        }

    bool IsElementDeleted ()
        {
        return m_isDeleted;
        }

    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Casey.Mullen                      01/2010
+---------------+---------------+---------------+---------------+---------------+------*/
void                    DgnECPointer::DisclosePointers (T_StdElementRefSet* disclosedPointers)
    {
    BeAssert (INVALID_PROVIDERID != m_localKey.m_providerId);
    BeAssert (NULL != m_hostElementRef);
    if (NULL == m_hostElementRef)
        return;

    if (m_pep.IsEmpty())
        {
        disclosedPointers->insert (m_hostElementRef);
        if(DgnECManager::GetManager().GetLogger().isSeverityEnabled(LOG_TRACE))
            DgnECManager::GetManager().GetLogger().tracev(L"                  Disclosed pointer to %ls (Which is the host element for this ECXDRelationship.)", Label(m_hostElementRef).c_str());
        return;
        }

    m_pep.DisclosePointers (disclosedPointers, m_homeDgnModel);
#ifdef DGNV10FORMAT_CHANGES_WIP
#ifndef NDEBUG
    PEPEvaluator pepEvaluator;
    m_pep.ProcessPath (pepEvaluator, m_homeDgnModel);
    wchar_t * message = L"";
    if (pepEvaluator.IsElementDeleted())
        message = L" (Which has been deleted.)";

    if (NULL != pepEvaluator.m_elementRef && DgnECManager::GetManager().GetLogger().isSeverityEnabled(LOG_TRACE))
        DgnECManager::GetManager().GetLogger().tracev(L"                  Disclosed pointer to %ls %ls", Label(pepEvaluator.m_elementRef).c_str(), message);
#endif
#endif
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    CaseyMullen     09/09
+---------------+---------------+---------------+---------------+---------------+------*/
ECXDRelationship::ECXDRelationship (ECXDRelationshipEnablerCR ecxRelationshipEnabler, DgnModelR modelRef, ElementRefP elementRef, UInt32 xAttrId) :
    ECXDInstance (ecxRelationshipEnabler, modelRef, elementRef, xAttrId)
    {
    }
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    CaseyMullen     09/09
+---------------+---------------+---------------+---------------+---------------+------*/
ECXDRelationship::ECXDRelationship (ECXDRelationshipEnablerCR ecxRelationshipEnabler, DgnModelR modelRef, ElementRefP elementRef, XAttributeHandleR ecxData) :
    ECXDInstance (ecxRelationshipEnabler, modelRef, elementRef, ecxData)
    {
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson      03/10
+---------------+---------------+---------------+---------------+---------------+------*/
XAttributeHandlerId ECXDRelationship::_GetXAttributeHandlerId()  const
    {
    return ECXDRelationshipXAttributeHandler::GetHandler().GetId(); 
    }    

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Bill.Steinbock                  02/2011
+---------------+---------------+---------------+---------------+---------------+------*/    
BentleyStatus       ECXDRelationship::_DeleteRelationship()
    {
    return _Delete();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    CaseyMullen     10/09
+---------------+---------------+---------------+---------------+---------------+------*/    
WString        ECXDRelationship::_ToString (const wchar_t * indent) const
    {
    return InstanceDataToString (indent, m_ecxInstanceEnabler->GetClassLayout());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    CaseyMullen     01/10
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt       ECXDRelationship::GetDgnECPointer (DgnECPointer& ecPointer, ECRelationshipEnd end, DgnModelR homeDgnModel, ElementRefP hostElementRef) const
    {
    ClassLayoutCR classLayout = m_ecxInstanceEnabler->GetClassLayout();
    int propertyIndex = classLayout.GetECPointerIndex (end);

    if (propertyIndex < 0) 
        { 
        BeAssert (false && "ClassLayouts for ECRelationshipClasses should always have positive propertyIndex for both ECPointers"); 
        return ERROR; 
        }

    // WIP_FUSION: start using those indices when we can find value by index
    ECValue binaryValue;
#ifdef OLD_WAY
    StatusInt status = this->GetValue (binaryValue, (end == ECRelationshipEnd_Source) ? PROPERTYLAYOUT_Source_ECPointer : PROPERTYLAYOUT_Target_ECPointer);
#endif

    StatusInt status = this->GetValue (binaryValue, propertyIndex);
    if (SUCCESS != status || binaryValue.IsNull()) 
        { 
        BeAssert(false && "Corrupt ECPointer in relationship");
        return ERROR;
        }
    
    return ecPointer.InitializeFromValue (binaryValue, homeDgnModel, hostElementRef);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    CaseyMullen     01/10
+---------------+---------------+---------------+---------------+---------------+------*/    
StatusInt       ECXDRelationship::GetDgnECPointers (DgnECPointer& source, DgnECPointer& target, DgnModelR homeDgnModel, ElementRefP hostElementRef)
    {
    StatusInt status = GetDgnECPointer (source, ECRelationshipEnd_Source, homeDgnModel, hostElementRef);
    if (SUCCESS != status)
        return status;
        
    return GetDgnECPointer (target, ECRelationshipEnd_Target, homeDgnModel, hostElementRef);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    sam.wilson                      03/2010
+---------------+---------------+---------------+---------------+---------------+------*/
DgnElementECInstancePtr ECXDRelationship::GetRelatedInstance (ECN::ECRelationshipEnd end) const
    {
    DgnECPointer ptr;
    if (GetDgnECPointer (ptr, end, GetDgnModel(), GetElementRef()) != SUCCESS || ptr.GetElementRef() == NULL)
        return NULL;
    DgnECManagerR dgnecManager = DgnECManager::GetManager();
    return dgnecManager.LoadInstance (*ptr.GetElementRef()->GetDgnModelP(), ptr.GetElementRef(), ptr.m_localKey);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Bill.Steinbock                  08/2010
+---------------+---------------+---------------+---------------+---------------+------*/
void                  ECXDRelationship::_SetSource (ECN::IECInstanceP instance)
    {
    DgnElementECInstanceP  dgnInstance = dynamic_cast<DgnElementECInstanceP>(instance);
    if (NULL != dgnInstance)
        {
        ECValue binaryValue;
        DgnECPointer::SetECPointerValue (binaryValue, *dgnInstance, dgnInstance->GetElementRef());
    
        StatusInt status = SetValue (m_ecxInstanceEnabler->GetClassLayout().GetECPointerIndex (ECRelationshipEnd_Source), binaryValue);
        BeAssert (SUCCESS == status);
        }
    else
        {
        BeAssert(false && "ECXDRelationship::_SetSource instance is not a DgnElementECInstanceP");
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Bill.Steinbock                  08/2010
+---------------+---------------+---------------+---------------+---------------+------*/
ECN::IECInstancePtr          ECXDRelationship::_GetSource () const
    {
    return GetRelatedInstance(ECRelationshipEnd_Source);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Bill.Steinbock                  08/2010
+---------------+---------------+---------------+---------------+---------------+------*/
 void                  ECXDRelationship::_SetTarget (ECN::IECInstanceP instance)
    {
    DgnElementECInstanceP  dgnInstance = dynamic_cast<DgnElementECInstanceP>(instance);
    if (NULL != dgnInstance)
        {
        ECValue binaryValue;
        DgnECPointer::SetECPointerValue (binaryValue, *dgnInstance, dgnInstance->GetElementRef());
    
        StatusInt status = SetValue (m_ecxInstanceEnabler->GetClassLayout().GetECPointerIndex (ECRelationshipEnd_Target), binaryValue);
        BeAssert (SUCCESS == status);
        }
    else
        {
        BeAssert(false && "ECXDRelationship::_SetTarget instance is not a DgnElementECInstanceP");
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Bill.Steinbock                  08/2010
+---------------+---------------+---------------+---------------+---------------+------*/
ECN::IECInstancePtr          ECXDRelationship::_GetTarget () const
    {
    return GetRelatedInstance(ECRelationshipEnd_Target);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Bill.Steinbock                  05/2011
+---------------+---------------+---------------+---------------+---------------+------*/
ECN::ECObjectsStatus              ECXDRelationship::_GetSourceOrderId(Int64 &orderId) const
    {
    WString propertyName;

    ECN::ECRelationshipClassCR relationshipClass = *(ECN::ECRelationshipClass const*)(&GetClass());
    if (ECOBJECTS_STATUS_Success != relationshipClass.GetOrderedRelationshipPropertyName (propertyName, ECRelationshipEnd_Source))
        return ECOBJECTS_STATUS_Error;

    ECValue orderIdVal;
    if (SUCCESS == GetValue (orderIdVal, propertyName.c_str()))
        {
        orderId = orderIdVal.GetLong();
        return ECOBJECTS_STATUS_Success;
        }

    return ECOBJECTS_STATUS_Error;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Bill.Steinbock                  05/2011
+---------------+---------------+---------------+---------------+---------------+------*/
ECN::ECObjectsStatus              ECXDRelationship::_GetTargetOrderId(Int64 &orderId) const
    {
    WString propertyName;

    ECN::ECRelationshipClassCR relationshipClass = *(ECN::ECRelationshipClass const*)(&GetClass());
    if (ECOBJECTS_STATUS_Success != relationshipClass.GetOrderedRelationshipPropertyName (propertyName, ECRelationshipEnd_Target))
        return ECOBJECTS_STATUS_Error;

    ECValue orderIdVal;
    if (SUCCESS == GetValue (orderIdVal, propertyName.c_str()))
        {
        orderId = orderIdVal.GetLong();
        return ECOBJECTS_STATUS_Success;
        }

    return ECOBJECTS_STATUS_Error;
    }

//////////////////////////////////////////////////////////////////////////////////////////
//  ECXDRelationshipEnabler
//////////////////////////////////////////////////////////////////////////////////////////
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    CaseyMullen     09/09
+---------------+---------------+---------------+---------------+---------------+------*/     
ECXDRelationshipEnabler::ECXDRelationshipEnabler(ECRelationshipClassCR ecClass, ClassLayoutCR classLayout, ECXDPerFileCacheR perFileCache) :
    ECXDInstanceEnabler (ecClass, classLayout, perFileCache),
    m_sharedWipInstance (InitializeSharedWipInstance(&perFileCache))
    {
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    CaseyMullen     09/09
+---------------+---------------+---------------+---------------+---------------+------*/     
ECXDRelationshipEnabler::~ECXDRelationshipEnabler()
    {
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    CaseyMullen     12/09
+---------------+---------------+---------------+---------------+---------------+------*/
wchar_t const *         ECXDRelationshipEnabler::_GetName() const
    {
    return L"Bentley::ECN::ECXDRelationshipEnabler";
    }
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Casey.Mullen    12/09
+---------------+---------------+---------------+---------------+---------------+------*/
StandaloneECInstancePtr ECXDRelationshipEnabler::InitializeSharedWipInstance (ECN::IStandaloneEnablerLocaterP standaloneInstanceEnablerLocater)
    {
    m_sharedWipEnabler = StandaloneECEnabler::CreateEnabler(GetClass(), GetClassLayout(), standaloneInstanceEnablerLocater, false);
    return m_sharedWipEnabler->CreateInstance();    
    }
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Casey.Mullen    12/09
+---------------+---------------+---------------+---------------+---------------+------*/
StandaloneECInstanceP   ECXDRelationshipEnabler::_GetSharedWipInstance() const
    {
    return m_sharedWipInstance.get();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    CaseyMullen     10/09
+---------------+---------------+---------------+---------------+---------------+------*/
ECXDRelationshipEnablerPtr ECXDRelationshipEnabler::CreateRelationshipEnabler(ECRelationshipClassCR ecClass, ClassLayoutCR classLayout, ECXDPerFileCacheR perFileCache)
    {
    return new ECXDRelationshipEnabler (ecClass, classLayout, perFileCache);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Bill.Steinbock                  03/2012
+---------------+---------------+---------------+---------------+---------------+------*/
ECN::StandaloneECInstanceP       ECXDRelationshipEnabler::_GetSharedStandaloneWipInstance() const
    {
    return  _GetSharedWipInstance();    // use base class implementation
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Bill.Steinbock                  04/2011
+---------------+---------------+---------------+---------------+---------------+------*/
ECN::ECRelationshipClassCR       ECXDRelationshipEnabler::_GetRelationshipClass() const
    {
    return GetRelationshipClass ();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Bill.Steinbock                  04/2012
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus               ECXDRelationshipEnabler::_CreateSingleEndedRelationship (ECN::IECWipRelationshipInstanceP wipRelationship,
                                                                        DgnElementECInstanceCR instanceA, ElementId relatedAttachmentElementId, 
                                                                        DgnModelId relatedModelId, ElementId relatedElementId, UInt32 relatedLocalId) const
    {
    // EIP: Not yet supported
    return ERROR;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Bill.Steinbock                  04/2011
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus               ECXDRelationshipEnabler::_CreateRelationship (IDgnECRelationshipInstancePtr *createdRelationship, ECN::IECWipRelationshipInstanceP wipRelationship,
                                                                          DgnElementECInstanceCR instanceA, DgnElementECInstanceCR instanceB, DgnModelP modelRef, ElementRefP  elementRef) const
    {
    ECXDRelationshipPtr  ecxdRelationship = NULL;
    ECXDRelationshipPtr* ecxdRelationshipP = NULL;

    // if we need to return a relationshipHolder the we must pass a ECXDRelationshipPtr into CreateRelationshipOnElement
    if (createdRelationship)
        ecxdRelationshipP = &ecxdRelationship;

    ECN::IECWipRelationshipInstancePtr wipInstance = wipRelationship;
    if (wipInstance.IsNull())
        {
        wipInstance = _CreateWipRelationshipInstance();
        }

    ECN::StandaloneECInstanceP standAloneInstanceP = dynamic_cast< ECN::StandaloneECInstanceP>(wipInstance.get());
    DgnECInstanceStatus  status = CreateRelationshipOnElement (*ecxdRelationshipP, *standAloneInstanceP, instanceA, instanceB, modelRef, elementRef);
    if (DGNECINSTANCESTATUS_Success == status)
        {
        if (createdRelationship)
            *createdRelationship = ecxdRelationship.get();

        return SUCCESS;
        }

    return ERROR;
    }

 /*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Bill.Steinbock                  04/2011
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus               ECXDRelationshipEnabler::_CreateRelationship (IDgnECRelationshipInstancePtr *createdRelationship, ECN::StandaloneECInstanceR wipRelationship,
                                                                          DgnElementECInstanceCR instanceA, DgnElementECInstanceCR instanceB, DgnModelP modelRef, ElementRefP  elementRef) const
    {
    ECXDRelationshipPtr  ecxdRelationship = NULL;
    ECXDRelationshipPtr* ecxdRelationshipP = NULL;

    // if we need to return a relationshipHolder the we must pass a ECXDRelationshipPtr into CreateRelationshipOnElement
    if (createdRelationship)
        ecxdRelationshipP = &ecxdRelationship;

    DgnECInstanceStatus  status = CreateRelationshipOnElement (*ecxdRelationshipP, wipRelationship, instanceA, instanceB, modelRef, elementRef);
    if (DGNECINSTANCESTATUS_Success == status)
        {
        if (createdRelationship)
            *createdRelationship = ecxdRelationship.get();

        return SUCCESS;
        }

    return ERROR;
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Bill.Steinbock                  02/2011
+---------------+---------------+---------------+---------------+---------------+------*/
static bool ClassMatchesRelationshipConstraint (ECRelationshipConstraintCR constraint, ECClassCR testClass)
    {
    ECConstraintClassesList const& classList = constraint.GetClasses();

    FOR_EACH (ECClassP ecClassP , classList)
        {
        if (1 == classList.size())
            {
            if (ecClassP->GetName().EqualsI (L"AnyClass"))
                return true;
            }

        if (constraint.GetIsPolymorphic())
            {
            if (testClass.Is (ecClassP))
                return true;
            }
        else
            {
            if (&testClass == ecClassP)
                return true;
            }
        }

    return false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    CaseyMullen     12/09
+---------------+---------------+---------------+---------------+---------------+------*/  
DgnECInstanceStatus       ECXDRelationshipEnabler::CreateRelationshipOnElement 
(
ECXDRelationshipPtr&        ecxRelationship, 
ECN::StandaloneECInstanceR   wipInstance, 
DgnElementECInstanceCR             instanceA, 
DgnElementECInstanceCR             instanceB, 
DgnModelP                modelRef, 
ElementRefP                 elementRef
) const
    {
    StatusInt status;

    DgnElementECInstanceCP  sourceInstance = NULL; 
    DgnElementECInstanceCP  targetInstance = NULL; 

    // determine which instance is source
    ECN::ECRelationshipClassCR relationshipClass = *(ECN::ECRelationshipClass const*)(&GetClass());

    if (ClassMatchesRelationshipConstraint (relationshipClass.GetSource(), instanceA.GetClass()))
        {
        sourceInstance = &instanceA;

        if (!ClassMatchesRelationshipConstraint (relationshipClass.GetTarget(), instanceB.GetClass()))
            return DGNECINSTANCESTATUS_Error;

        targetInstance = &instanceB;
        }
    else if (ClassMatchesRelationshipConstraint (relationshipClass.GetSource(), instanceB.GetClass()))
        {
        sourceInstance = &instanceB;

        if (!ClassMatchesRelationshipConstraint (relationshipClass.GetTarget(), instanceA.GetClass()))
            return DGNECINSTANCESTATUS_Error;

        targetInstance = &instanceA;
        }

    DgnModelP  sourceDgnModel = (NULL != modelRef) ? modelRef : &sourceInstance->GetDgnModel(); 
    ElementRefP   sourceElementRef = (NULL != elementRef) ? elementRef : targetInstance->GetElementRef();

    status = SetECPointerValue (wipInstance, ECRelationshipEnd_Source,  *sourceInstance, sourceElementRef);
    if (status != SUCCESS) return DGNECINSTANCESTATUS_Error;
    
    status = SetECPointerValue (wipInstance, ECRelationshipEnd_Target, *targetInstance, sourceElementRef);
    if (status != SUCCESS) return DGNECINSTANCESTATUS_Error;

    if (&wipInstance.GetClassLayout() != &GetClassLayout()) // DEFERRED_FUSION: we need a checksum on ClassLayout, so that we can have compatible but different ClassLayouts, e.g. from different files.
        return DGNECINSTANCESTATUS_IncompatibleWipInstance;
    
    if (SUCCESS != ECXDProvider::GetProvider().EnsureClassLayoutIsStored (GetClass(), GetClassLayout(), *sourceElementRef->GetDgnModelP()->GetDgnProject()))
        return DGNECINSTANCESTATUS_ClassLayoutNotStored;

    byte const * data = wipInstance.GetData();
    UInt32       size = wipInstance.GetBytesUsed();
       
    UInt32 id = XAttributeHandle::INVALID_XATTR_ID;
    status = ECXDRelationshipXAttributeHandler::GetHandler().CreateXAttribute (id, sourceElementRef, data, size);
    if (SUCCESS != status)
        return DGNECINSTANCESTATUS_RelationshipInstanceNotCreated;
        
    if (XAttributeHandle::INVALID_XATTR_ID == id)
        return DGNECINSTANCESTATUS_RelationshipInstanceNotCreated;
    
    ecxRelationship = new ECXDRelationship (*this, *sourceDgnModel, sourceElementRef, id);

    // The StandaloneECInstance will not know what to do with this.
    SetECPointerToNull (wipInstance, ECRelationshipEnd_Source);
    SetECPointerToNull (wipInstance, ECRelationshipEnd_Target);
    return DGNECINSTANCESTATUS_Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    CaseyMullen     12/09
+---------------+---------------+---------------+---------------+---------------+------*/  
DgnECInstanceStatus       ECXDRelationshipEnabler::_CreateInstanceOnElement (DgnElementECInstancePtr* dgnecInstance, StandaloneECInstanceR wipInstance, DgnModelR modelRef, ElementRefP elementRef) const
    {
    throw "It is illegal to call CreateInstanceOnElement with a DgnECRelationshipEnabler. Call CreateRelationshipOnElement instead.";
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    CaseyMullen     12/09
+---------------+---------------+---------------+---------------+---------------+------*/  
bool            ECXDRelationshipEnabler::_SupportsCreateInstanceOnElement () const
    {
    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    CaseyMullen    02/10
+---------------+---------------+---------------+---------------+---------------+------*/
ECXDRelationshipPtr  ECXDRelationshipEnabler::CreateECXDRelationshipInstance (DgnModelR modelRef, ElementRefP elementRef, UInt32 xAttrId) const
    {
    return new ECXDRelationship (*this, modelRef, elementRef, xAttrId);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    CaseyMullen    02/10
+---------------+---------------+---------------+---------------+---------------+------*/
ECXDInstancePtr  ECXDRelationshipEnabler::CreateInstance (DgnModelR modelRef, ElementRefP elementRef, UInt32 xAttrId) const
    {
    return CreateECXDRelationshipInstance (modelRef, elementRef, xAttrId).get ();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Bill.Steinbock                  04/2011
+---------------+---------------+---------------+---------------+---------------+------*/
 ECN::IECWipRelationshipInstancePtr  ECXDRelationshipEnabler::_CreateWipRelationshipInstance () const
    {
    return new ECXDWipRelationship (*this);
    }

 /*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Bill.Steinbock                  04/2011
+---------------+---------------+---------------+---------------+---------------+------*/
ECN::ECRelationshipClassCR ECXDRelationshipEnabler::GetRelationshipClass () const
    {
    return *(ECN::ECRelationshipClass const*)(&GetClass());
    }

/////////////////////////////////////////////////////////////////////////////////////////
#pragma region ECXDWipRelationship
/////////////////////////////////////////////////////////////////////////////////////////
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Bill.Steinbock                  04/2011
+---------------+---------------+---------------+---------------+---------------+------*/
ECXDWipRelationship::ECXDWipRelationship (ECXDRelationshipEnablerCR enabler) : IECWipRelationshipInstance (*enabler.m_sharedWipEnabler)
    {                        
    m_ecEnabler = const_cast<ECXDRelationshipEnablerP>(&enabler);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Bill.Steinbock                  04/2011
+---------------+---------------+---------------+---------------+---------------+------*/
ECXDWipRelationship::~ECXDWipRelationship ()
    {
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Bill.Steinbock                  04/2011
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus  ECXDWipRelationship::_SetName (WCharCP name) 
    {
    if (SUCCESS == SetDisplayLabel (name))
        return SUCCESS;

    return ERROR;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Bill.Steinbock                  04/2011
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus  ECXDWipRelationship::_SetSourceOrderId (Int64 sourceOrderId)
    {
    WString propertyName;

    ECN::ECRelationshipClassCR relationshipClass = *(ECN::ECRelationshipClass const*)(&GetClass());
    if (ECOBJECTS_STATUS_Success != relationshipClass.GetOrderedRelationshipPropertyName (propertyName, ECRelationshipEnd_Source))
        return ERROR;

    if (SUCCESS == SetValue (propertyName.c_str(), ECValue(sourceOrderId)))
        return SUCCESS;

    return ERROR;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Bill.Steinbock                  04/2011
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus  ECXDWipRelationship::_SetTargetOrderId (Int64 targetOrderId)
    {
    WString propertyName;

    ECN::ECRelationshipClassCR relationshipClass = *(ECN::ECRelationshipClass const*)(&GetClass());

    if (ECOBJECTS_STATUS_Success != relationshipClass.GetOrderedRelationshipPropertyName (propertyName, ECRelationshipEnd_Target))
        return ERROR;

    if (SUCCESS == SetValue (propertyName.c_str(), ECValue(targetOrderId)))
        return SUCCESS;

    return ERROR;
    }

#pragma endregion //ECXDWipRelationship


END_BENTLEY_DGNPLATFORM_NAMESPACE

