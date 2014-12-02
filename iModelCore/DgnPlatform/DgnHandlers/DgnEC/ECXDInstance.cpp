/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnHandlers/DgnEC/ECXDInstance.cpp $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include    <DgnPlatformInternal.h>

USING_NAMESPACE_BENTLEY
USING_NAMESPACE_EC
USING_NAMESPACE_BENTLEY_DGNPLATFORM
USING_NAMESPACE_BENTLEY_LOGGING

BEGIN_BENTLEY_DGNPLATFORM_NAMESPACE

using namespace std;

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    CaseyMullen     09/09
+---------------+---------------+---------------+---------------+---------------+------*/
ECXDInstance::ECXDInstance (ECXDInstanceEnablerCR ecxInstanceEnabler, DgnModelR modelRef, ElementRefP elementRef, UInt32 xAttrId) :
    MemoryInstanceSupport (false),
    DgnElementECInstance (modelRef, elementRef, xAttrId), 
    m_ecxInstanceEnabler (&ecxInstanceEnabler), m_ecXData(NULL)
    {    
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    CaseyMullen     09/09
+---------------+---------------+---------------+---------------+---------------+------*/
ECXDInstance::ECXDInstance (ECXDInstanceEnablerCR ecxInstanceEnabler, DgnModelR modelRef, ElementRefP elementRef, XAttributeHandleR ecxData) :
    MemoryInstanceSupport (false),
    DgnElementECInstance (modelRef, elementRef, ecxData.GetId()),
    m_ecxInstanceEnabler (&ecxInstanceEnabler), m_ecXData(&ecxData)
    {
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    CaseyMullen     12/09
+---------------+---------------+---------------+---------------+---------------+------*/    
ECEnablerCR         ECXDInstance::_GetEnabler() const
    {
    return *m_ecxInstanceEnabler;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    CaseyMullen     01/10
+---------------+---------------+---------------+---------------+---------------+------*/    
DgnElementECInstanceEnablerCR ECXDInstance::_GetDgnElementECInstanceEnabler() const
    {
    return *m_ecxInstanceEnabler;
    }
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    CaseyMullen     09/09
+---------------+---------------+---------------+---------------+---------------+------*/
ECN::ECObjectsStatus ECXDInstance::_GetValue (ECValueR v, const wchar_t * propertyAccessString, bool useArrayIndex, UInt32 arrayIndex) const
    {
    if (IsAsleep())
        return ECOBJECTS_STATUS_Error; // IsAsleep is an ECXD concept, not an ECObjects concept, so for now just returning a generic error
    
    ECXDataGuard guard(const_cast<ECXDInstanceR>(*this)); // we change it, then change it back, so it is effectively const
    StatusInt status = guard.GetStatus();  // DEFERRED_FUSION: ECXDataGuard needs to not return StatusInt
    if (SUCCESS != status)
        return ECOBJECTS_STATUS_Error; 
    
    ClassLayoutCR classLayout = m_ecxInstanceEnabler->GetClassLayout();
    return GetValueFromMemory (classLayout, v, propertyAccessString, useArrayIndex, arrayIndex);
    }
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    05/10
+---------------+---------------+---------------+---------------+---------------+------*/
ECN::ECObjectsStatus           ECXDInstance::_GetValue (ECValueR v, UInt32 propertyIndex, bool useArrayIndex, UInt32 arrayIndex) const
    {
    if (IsAsleep())
        return ECOBJECTS_STATUS_Error; // IsAsleep is an ECXD concept, not an ECObjects concept, so for now just returning a generic error
    
    ECXDataGuard guard(const_cast<ECXDInstanceR>(*this)); // we change it, then change it back, so it is effectively const
    if (SUCCESS != guard.GetStatus())
        return ECOBJECTS_STATUS_Error; // DEFERRED_FUSION: ECXDataGuard needs to not return StatusInt

    ClassLayoutCR classLayout = m_ecxInstanceEnabler->GetClassLayout();
    return GetValueFromMemory (classLayout, v, propertyIndex, useArrayIndex, arrayIndex);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      03/2010
+---------------+---------------+---------------+---------------+---------------+------*/
ECN::ECObjectsStatus      ECXDInstance::GetValueFromPropertyLayout (ECN::ECValueR v, PropertyLayoutCR prop, bool useArrayIndex, UInt32 arrayIndex) const
    {
    if (IsAsleep())
        return ECOBJECTS_STATUS_Error; // IsAsleep is an ECXD concept, not an ECObjects concept, so for now just returning a generic error
    
    ECXDataGuard guard(const_cast<ECXDInstanceR>(*this)); // we change it, then change it back, so it is effectively const
    if (ECOBJECTS_STATUS_Success != guard.GetStatus())
        return ECOBJECTS_STATUS_Error;
    
    if (prop.GetTypeDescriptor().IsArray() && useArrayIndex)
        return GetValueFromMemory (v, prop, arrayIndex);

    return GetValueFromMemory (v, prop);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    CaseyMullen     09/09
+---------------+---------------+---------------+---------------+---------------+------*/
ECN::ECObjectsStatus           ECXDInstance::_SetValue (const wchar_t * propertyAccessString, ECValueCR v, bool useArrayIndex, UInt32 arrayIndex)
    {
    if (IsAsleep())
        return ECOBJECTS_STATUS_Error; // IsAsleep is an ECXD concept, not an ECObjects concept, so for now just returning a generic error
                        
    ECXDataGuard guard(*this);
    if (SUCCESS != guard.GetStatus())
        return ECOBJECTS_STATUS_Error; // DEFERRED_FUSION: ECXDataGuard needs to not return StatusInt
            
    ClassLayoutCR classLayout = m_ecxInstanceEnabler->GetClassLayout();
            
    ECObjectsStatus status = SetValueToMemory (classLayout, propertyAccessString, v, useArrayIndex, arrayIndex);
#ifdef EC_TRACE_MEMORY    
    //_Dump();
#endif    
    return status;
    }     
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    05/10
+---------------+---------------+---------------+---------------+---------------+------*/
ECN::ECObjectsStatus           ECXDInstance::_SetValue (UInt32 propertyIndex, ECValueCR v, bool useArrayIndex, UInt32 arrayIndex)
    {
    if (IsAsleep())
        return ECOBJECTS_STATUS_Error; // IsAsleep is an ECXD concept, not an ECObjects concept, so for now just returning a generic error
                        
    ECXDataGuard guard(*this);
    if (SUCCESS != guard.GetStatus())
        return ECOBJECTS_STATUS_Error; // DEFERRED_FUSION: ECXDataGuard needs to not return StatusInt
            
    ClassLayoutCR classLayout = m_ecxInstanceEnabler->GetClassLayout();

    ECObjectsStatus status = SetValueToMemory (classLayout, propertyIndex, v, useArrayIndex, arrayIndex);
#ifdef EC_TRACE_MEMORY    
    //_Dump();
#endif    
    return status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Adam.Klatzkin                   01/2010
+---------------+---------------+---------------+---------------+---------------+------*/
ECN::ECObjectsStatus           ECXDInstance::_InsertArrayElements (const wchar_t * propertyAccessString, UInt32 index, UInt32 size)
    {
    if (IsAsleep())
        return ECOBJECTS_STATUS_Error; // IsAsleep is an ECXD concept, not an ECObjects concept, so for now just returning a generic error
    
    ECXDataGuard guard(const_cast<ECXDInstanceR>(*this)); // we change it, then change it back, so it is effectively const
    StatusInt status = guard.GetStatus();
    if (SUCCESS != status)
        return ECOBJECTS_STATUS_Error;
    
    ClassLayoutCR classLayout = m_ecxInstanceEnabler->GetClassLayout();
    return InsertNullArrayElementsAt (classLayout, propertyAccessString, index, size);    
    } 
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Adam.Klatzkin                   01/2010
+---------------+---------------+---------------+---------------+---------------+------*/
ECN::ECObjectsStatus           ECXDInstance::_AddArrayElements (const wchar_t * propertyAccessString, UInt32 size)
    {
    if (IsAsleep())
        return ECOBJECTS_STATUS_Error; // IsAsleep is an ECXD concept, not an ECObjects concept, so for now just returning a generic error
    
    ECXDataGuard guard(const_cast<ECXDInstanceR>(*this)); // we change it, then change it back, so it is effectively const
    StatusInt status = guard.GetStatus();
    if (SUCCESS != status)
        return ECOBJECTS_STATUS_Error;
    
    ClassLayoutCR classLayout = m_ecxInstanceEnabler->GetClassLayout();
    return AddNullArrayElementsAt (classLayout, propertyAccessString, size);
    }        

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Adam.Klatzkin                   01/2010
+---------------+---------------+---------------+---------------+---------------+------*/
ECN::ECObjectsStatus           ECXDInstance::_RemoveArrayElement (const wchar_t * propertyAccessString, UInt32 index)
    {
    return ECOBJECTS_STATUS_OperationNotSupported;
    } 
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Adam.Klatzkin                   01/2010
+---------------+---------------+---------------+---------------+---------------+------*/
ECN::ECObjectsStatus           ECXDInstance::_ClearArray (const wchar_t * propertyAccessString)
    {
    return ECOBJECTS_STATUS_OperationNotSupported;
    }            

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Adam.Klatzkin                   01/2010
+---------------+---------------+---------------+---------------+---------------+------*/
ECN::ECObjectsStatus           ECXDInstance::_SetStructArrayValueToMemory (ECValueCR v, ClassLayoutCR classLayout, PropertyLayoutCR propertyLayout, UInt32 index)
    {    
    ECValue binaryValue (PRIMITIVETYPE_Binary);    
    
    // DEFERRED_FUSION Is there already a value at this index?  If so, we need to remove the XAttribute        
    
    if (v.IsNull())
        binaryValue.SetToNull();
    else
        {                            
        // DEFERRED_FUSION What do we expect the struct to be passed in as the struct value?  Currently it is restricted to a StandaloneECInstace, is that ok?        
        IECInstancePtr instancePtr = v.GetStruct();                
        StandaloneECInstanceP wipStructValue = dynamic_cast<StandaloneECInstanceP> (instancePtr.get());    
        if (wipStructValue == NULL)
            return ECOBJECTS_STATUS_NullPointerValue;  
        DgnECManagerR dgnecManager = DgnECManager::GetManager();
        DgnElementECInstanceEnablerP structValueEnabler = dgnecManager.ObtainElementInstanceEnabler (wipStructValue->GetClass(), *GetDgnModel().GetDgnProject());              
        if (NULL == structValueEnabler)
            return ECOBJECTS_STATUS_NullPointerValue; 
         
        if (!structValueEnabler->SupportsCreateStructValueOnElement())
            return ECOBJECTS_STATUS_OperationNotSupported;              
        DgnElementECInstancePtr structValue;
        StatusInt status = structValueEnabler->CreateStructValueOnElement (&structValue, *wipStructValue, GetDgnModel(), GetElementRef());        
        if (status != SUCCESS)
            return ECOBJECTS_STATUS_Error;        
        
        // Need to refresh the underlying XAttribute since creating the struct value may have invalidated the handle.
        RefreshXAttribute();                
        
        status = DgnECPointer::SetECPointerValue (binaryValue, *structValue, GetElementRef());        
        if (status != SUCCESS)
            return ECOBJECTS_STATUS_Error;
        }
    
    return SetPrimitiveValueToMemory (binaryValue, classLayout, propertyLayout, true, index);
    }                

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Bill.Steinbock                  07/2011
+---------------+---------------+---------------+---------------+---------------+------*/
ECN::ECObjectsStatus  ECXDInstance::_RemoveStructArrayElementsFromMemory (ClassLayoutCR classLayout, PropertyLayoutCR propertyLayout, UInt32 removeIndex, UInt32 removeCount) 
    {
    return ECOBJECTS_STATUS_OperationNotSupported;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Adam.Klatzkin                   01/2010
+---------------+---------------+---------------+---------------+---------------+------*/
ECN::ECObjectsStatus        ECXDInstance::_GetStructArrayValueFromMemory (ECValueR v, PropertyLayoutCR propertyLayout, UInt32 index) const
    {
    ECValue binaryValue;
    ECObjectsStatus status = GetPrimitiveValueFromMemory (binaryValue, propertyLayout, true, index);      
    if (status != ECOBJECTS_STATUS_Success) 
        return status;
        
    if (binaryValue.IsNull())
        {
        v.SetStruct(NULL);
        return ECOBJECTS_STATUS_Success;
        }

    DgnECPointer dgnECPointer;
    StatusInt status2 = dgnECPointer.InitializeFromValue (binaryValue, GetDgnModel(), GetElementRef());
    if (SUCCESS != status2)
        return ECOBJECTS_STATUS_Error;

    DgnECManagerR dgnecManager = DgnECManager::GetManager();
    DgnElementECInstancePtr structValue = dgnecManager.LoadStructValue (GetDgnModel(), GetElementRef(), dgnECPointer.m_localKey);    
    if (structValue == NULL)
        return ECOBJECTS_STATUS_NullPointerValue;
       
    return v.SetStruct (structValue.get()) == SUCCESS ? ECOBJECTS_STATUS_Success : ECOBJECTS_STATUS_Error;    
    }                

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Adam.Klatzkin                   03/2010
+---------------+---------------+---------------+---------------+---------------+------*/
XAttributeHandlerId ECXDInstance::_GetXAttributeHandlerId()  const
    {
    return ECXDInstanceXAttributeHandler::GetHandler().GetId(); 
    }    
   
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    CaseyMullen     11/09
+---------------+---------------+---------------+---------------+---------------+------*/   
StatusInt           ECXDInstance::AcquireXAttribute(XAttributeHandleR ecXData)
    {
    DEBUG_EXPECT (NULL == m_ecXData);
        
    m_ecXData = &ecXData;
    return RefreshXAttribute();
    }
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    CaseyMullen     11/09
+---------------+---------------+---------------+---------------+---------------+------*/   
DgnECInstanceStatus           ECXDInstance::RefreshXAttribute()
    {
    PRECONDITION (NULL != m_ecXData, DGNECINSTANCESTATUS_Error);    
     
#ifdef DGNV10FORMAT_CHANGES_WIP
    *m_ecXData = XAttributeHandle (GetElementRef(), _GetXAttributeHandlerId(), GetLocalId());
    if (!m_ecXData->IsValid())
        return DGNECINSTANCESTATUS_XAttributeHasBeenRemoved; // (not necessarily an error... maybe it was deleted while the ECInstance was "asleep")
#endif
  
#ifdef EC_TRACE_MEMORY
    size_t allocedBytes = m_ecXData->GetSize();
    byte const * data = (byte const *)m_ecXData->PeekData();
    DgnECManager::GetManager().GetLogger().tracev(L"Acquired ECXData for element at 0x%x, with handlerID=0x%x, xAttrId=0x%x, data=0x%x, size=%d",
        GetElementRef(), _GetXAttributeHandlerId(), GetLocalId(), data, allocedBytes);
#endif 
    return DGNECINSTANCESTATUS_Success;
    }    
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    CaseyMullen     11/09
+---------------+---------------+---------------+---------------+---------------+------*/
void                ECXDInstance::ReleaseXAttribute()
    {
    DEBUG_EXPECT (NULL != m_ecXData);
    
    m_ecXData = NULL;
    }
       
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    CaseyMullen     01/10
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus       ECXDInstance::_BeginPropertyAccess()
    {
    PRECONDITION (NULL == m_ecXData && "It is illegal to call BeginPropertyAccess when the DgnElementECInstance has already acquired its ECXData XAttributeHandle", ERROR);
    
    XAttributeHandleP ecXData = new XAttributeHandle();
    AcquireXAttribute (*ecXData);
    
    return SUCCESS;
    }
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    CaseyMullen     01/10
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus       ECXDInstance::_EndPropertyAccess()
    {
    PRECONDITION (NULL != m_ecXData && "It is illegal to call EndPropertyAccess when the DgnElementECInstance has not acquired its ECXData XAttributeHandle", ERROR);
        
    delete m_ecXData;
    m_ecXData = NULL;
    
    return SUCCESS;
    }    

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    CaseyMullen     09/09
+---------------+---------------+---------------+---------------+---------------+------*/
bool                ECXDInstance::_IsMemoryInitialized () const
    {
    return m_ecXData != NULL;
    }
        
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    CaseyMullen     09/09
+---------------+---------------+---------------+---------------+---------------+------*/
byte const *        ECXDInstance::_GetData () const
    {
    DEBUG_EXPECT (NULL != m_ecXData);
    DEBUG_EXPECT (m_ecXData->IsValid());
    
    byte const * data = (byte const *)m_ecXData->PeekData();
#ifdef EC_TRACE_MEMORY    
    DgnECManager::GetManager().GetLogger().tracev(L"_GetData for ECXDInstance=0x%x returns 0x%x.", this, data); 
#endif    
    return data;
    }
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Adam.Klatzkin                   03/2010
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt ECXDInstance::_ModifyECXData (XAttributeHandleCR xAttr, void const* newData, UInt32 offset, UInt32 size)
    {
    return ECXDInstanceXAttributeHandler::GetHandler().ModifyXAttribute ((XAttributeHandleR) xAttr, newData, offset, size);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    CaseyMullen     09/09
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus    ECXDInstance::_ModifyData (UInt32 offset, void const * newData, UInt32 size)
    {
    PRECONDITION (NULL != m_ecXData, ECOBJECTS_STATUS_PreconditionViolated);
    
#ifdef EC_TRACE_MEMORY    
    byte * data = (byte *)m_ecXData->GetPtrForWrite();
    DgnECManager::GetManager().GetLogger().tracev(L"_ModifyData for ECXDInstance=0x%x modifies %d bytes starting at offset of %d from 0x%x.", this, size, offset, data);
#endif 
    
    return _ModifyECXData (*m_ecXData, newData, offset, size) == SUCCESS ? ECOBJECTS_STATUS_Success : ECOBJECTS_STATUS_Error;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    CaseyMullen     09/09
+---------------+---------------+---------------+---------------+---------------+------*/
UInt32              ECXDInstance::_GetBytesUsed () const
    {
    DEBUG_EXPECT (NULL != m_ecXData);
    byte const *  data = _GetData();
    ClassLayoutCR classLayout = m_ecxInstanceEnabler->GetClassLayout();
    
    return classLayout.CalculateBytesUsed(data);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    CaseyMullen     09/09
+---------------+---------------+---------------+---------------+---------------+------*/
UInt32              ECXDInstance::_GetBytesAllocated () const
    {
    DEBUG_EXPECT (NULL != m_ecXData);
    
    return m_ecXData->GetSize();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    CaseyMullen     09/09
+---------------+---------------+---------------+---------------+---------------+------*/
void                ECXDInstance::_ShrinkAllocation (UInt32 newAllocation)
    {
    DEBUG_EXPECT (false && "DEFERRED_FUSION: needs implementation. Is there a way to just ask an XAttribute to trim itself?");
    } 

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    CaseyMullen     09/09
+---------------+---------------+---------------+---------------+---------------+------*/
void                ECXDInstance::_FreeAllocation ()
    {
    DEBUG_EXPECT (false && "I don't expect this to be called for ECXData... it gets allocated in other ways. DEFERRED_FUSION. Can we remove this?");
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Adam.Klatzkin                   03/2010
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt           ECXDInstance::_ReplaceECXData (XAttributeHandleR xAttr, void const* newData, UInt32 size)
    {
    return ECXDInstanceXAttributeHandler::GetHandler().ReplaceXAttribute (xAttr, newData, size);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Bill.Steinbock                  12/2010
+---------------+---------------+---------------+---------------+---------------+------*/
size_t                ECXDInstance::_GetOffsetToIECInstance () const
    {
    ECN::IECInstanceP iecInstanceP   = (ECN::IECInstanceP)this;
    byte const* baseAddressOfIECInstance = (byte const *)iecInstanceP;
    byte const* baseAddressOfConcrete = (byte const *)this;

    return (size_t)(baseAddressOfIECInstance - baseAddressOfConcrete);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    CaseyMullen    04/10
+---------------+---------------+---------------+---------------+---------------+------*/    
BentleyStatus       ECXDInstance::_Delete()
    {
    return (BentleyStatus)ECXDInstanceXAttributeHandler::GetHandler().DeleteXAttribute (GetElementRef(), GetLocalId());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    CaseyMullen     10/09
+---------------+---------------+---------------+---------------+---------------+------*/
ECObjectsStatus     ECXDInstance::_GrowAllocation (UInt32 additionalBytesNeeded)
    {
    UInt32 bytesAllocated = _GetBytesAllocated();
    DEBUG_EXPECT (bytesAllocated > 0);
    DEBUG_EXPECT (NULL != m_ecXData);
    DEBUG_EXPECT (m_ecXData->IsValid());
    // DEFERRED_FUSION: add performance counter
    
    UInt32       newBytesAllocated = bytesAllocated + additionalBytesNeeded;
    byte *       newData           = (byte*)malloc(newBytesAllocated); 
    void const * data              = m_ecXData->PeekData();
    memcpy (newData, data, bytesAllocated);
    
    StatusInt status = _ReplaceECXData (*m_ecXData, newData, newBytesAllocated);
#ifdef EC_TRACE_MEMORY    
    DgnECManager::GetManager().GetLogger().tracev(L"_GrowAllocation moved ECXData for ECXDInstance=0x%x from 0x%x to 0x%x. New size is %d.", this, data, m_ecXData->PeekData(), newBytesAllocated);
#endif
    free (newData);
    
    return status == SUCCESS ? ECOBJECTS_STATUS_Success : ECOBJECTS_STATUS_Error;
    }
        
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    CaseyMullen     10/09
+---------------+---------------+---------------+---------------+---------------+------*/    
WString            ECXDInstance::_ToString (const wchar_t * indent) const
    {
    ECXDataGuard guard(const_cast<ECXDInstanceR>(*this)); // we change it, then change it back, so it is effectively const
    if (SUCCESS != guard.GetStatus())
        return L"";
        
    return InstanceDataToString (indent, m_ecxInstanceEnabler->GetClassLayout());
    }
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Adam.Klatzkin                   03/2010
+---------------+---------------+---------------+---------------+---------------+------*/
ECXDStructValue::ECXDStructValue (ECXDInstanceEnablerCR ecxDataEnabler, DgnModelR modelRef, ElementRefP elementRef, UInt32 xAttrId) :
    ECXDInstance (ecxDataEnabler, modelRef, elementRef, xAttrId)
    {    
    }    

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Adam.Klatzkin                   03/2010
+---------------+---------------+---------------+---------------+---------------+------*/
XAttributeHandlerId ECXDStructValue::_GetXAttributeHandlerId() const
    {
    return ECXDStructValueXAttributeHandler::GetHandler().GetId(); 
    }    

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Adam.Klatzkin                   03/2010
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt           ECXDStructValue::_ModifyECXData  (XAttributeHandleCR xAttr, void const* newData, UInt32 offset, UInt32 size)
    {
    return ECXDStructValueXAttributeHandler::GetHandler().ModifyXAttribute ((XAttributeHandleR) xAttr, newData, offset, size);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Adam.Klatzkin                   03/2010
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt           ECXDStructValue::_ReplaceECXData (XAttributeHandleR xAttr, void const* newData, UInt32 size)
    {
    return ECXDStructValueXAttributeHandler::GetHandler().ReplaceXAttribute (xAttr, newData, size);
    }    
       
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    CaseyMullen     11/09
+---------------+---------------+---------------+---------------+---------------+------*/    
ECXDInstance::ECXDataGuard::ECXDataGuard(ECXDInstanceR ecxInstance) : m_ecxInstance (&ecxInstance), m_isGuarding(false), m_status(SUCCESS)
    {
    if (NULL == m_ecxInstance->m_ecXData)
        {
        m_isGuarding = true;
        m_status = m_ecxInstance->AcquireXAttribute(m_ecXData);
        }
    }
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    CaseyMullen     11/09
+---------------+---------------+---------------+---------------+---------------+------*/    
ECXDInstance::ECXDataGuard::~ECXDataGuard()
    {
    if (m_isGuarding && SUCCESS == m_status)
        m_ecxInstance->ReleaseXAttribute();
    }
    

END_BENTLEY_DGNPLATFORM_NAMESPACE
