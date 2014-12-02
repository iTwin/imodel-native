/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnHandlers/DgnEC/ECXDInstanceXAttributeHandler.cpp $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <DgnPlatformInternal.h>
#include <RmgrTools/Tools/DataExternalizer.h>
#include <Logging/bentleylogging.h>

#define CURRTXN(a) a->GetDgnProject()->GetTxnManager().GetCurrentTxn()
#define XATTRIBUTEID_ECXData        0xECDA
#define XATTRIBUTEID_ECXDStructValue 0xEC5C

USING_NAMESPACE_BENTLEY_DGNPLATFORM

USING_NAMESPACE_EC
USING_NAMESPACE_BENTLEY_LOGGING
BEGIN_BENTLEY_DGNPLATFORM_NAMESPACE

/*---------------------------------------------------------------------------------------

    ECXDBase XAttributeHandler

+---------------+---------------+---------------+---------------+---------------+------*/
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    05/10
+---------------+---------------+---------------+---------------+---------------+------*/
XAttributeHandlerId ECXDBaseXAttributeHandler::GetId ()      { return _GetId(); }
WString             ECXDBaseXAttributeHandler::GetLabel ()   { return _GetLabel(); }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Casey.Mullen                      11/2009
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt       ECXDBaseXAttributeHandler::CreateXAttribute (UInt32& xAttrId, ElementRefP hostElementRef, byte const * data, UInt32 size)
    {
    DEBUG_EXPECT (NULL != data);
    DEBUG_EXPECT (size > 0);
    
    StatusInt status = CURRTXN(hostElementRef).AddXAttribute (hostElementRef, GetId(), xAttrId, data, size, &xAttrId);
    
#ifdef EC_TRACE_MEMORY
    XAttributeHandle xAttr;
    
    xAttr.Init(hostElement, GetId(), xAttrId);
    DEBUG_EXPECT (xAttr.IsValidRef());
    DEBUG_EXPECT (xAttr.IsValid());
    DgnECManager::GetManager().GetLogger().tracev(L"%ls CreateXAttribute for element at 0x%x, with handlerID=0x%x, xAttrId=0x%x, data=0x%x, size=%d",
        GetLabel().c_str(), xAttr.GetElementRef(), xAttr.GetHandlerId(), xAttr.GetId(), xAttr.PeekData(), xAttr.GetSize());
#endif

    return status;   
    }
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Casey.Mullen                      11/2009
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt       ECXDBaseXAttributeHandler::ModifyXAttribute (XAttributeHandleR xAttr, void const* newData, UInt32 offset, UInt32 size)
    {
    DEBUG_EXPECT (NULL != newData);
    DEBUG_EXPECT (size > 0);
    
#ifdef EC_TRACE_MEMORY
    DgnECManager::GetManager().GetLogger().tracev(L"%ls ModifyXAttribute for element 0x%x, with handlerID=0x%x, xAttrId=0x%x, data=0x%x, size=0x%x",
        GetLabel().c_str(), xAttr.GetElementRef(), xAttr.GetHandlerId(), xAttr.GetId(), xAttr.PeekData(), xAttr.GetSize());
    DgnECManager::GetManager().GetLogger().tracev(L"%ls ModifyXAttribute offset=%d, sizeOfUpdate=%d", GetLabel().c_str(), offset, size);
#endif      
    return  CURRTXN(xAttr.GetElementRef()).ModifyXAttributeData (xAttr, newData, offset, size);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Casey.Mullen                      11/2009
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt       ECXDBaseXAttributeHandler::ReplaceXAttribute (XAttributeHandleR xAttr, void const* newData, UInt32 size)
    {
    DEBUG_EXPECT (NULL != newData);
    DEBUG_EXPECT (size > 0);
    
#ifdef EC_TRACE_MEMORY
    DgnECManager::GetManager().GetLogger().tracev(L"%ls ReplaceXAttribute for element 0x%x, with handlerID=0x%x, xAttrId=0x%x, data=0x%x, size=0x%x",
        GetLabel().c_str(), xAttr.GetElementRef(), xAttr.GetHandlerId(), xAttr.GetId(), xAttr.PeekData(), xAttr.GetSize());
    DgnECManager::GetManager().GetLogger().tracev(L"%ls ReplaceXAttribute new size=%d", GetLabel().c_str(), size);
#endif  
    
    StatusInt status = CURRTXN(xAttr.GetElementRef()).ReplaceXAttributeData (xAttr, newData, size);
    
#ifdef EC_TRACE_MEMORY
    DgnECManager::GetManager().GetLogger().tracev(L"After %ls ReplaceXAttribute for element 0x%x, with handlerID=0x%x, xAttrId=0x%x, data=0x%x, size=0x%x",
        GetLabel().c_str(), xAttr.GetElementRef(), xAttr.GetHandlerId(), xAttr.GetId(), xAttr.PeekData(), xAttr.GetSize());
#endif
    return status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Casey.Mullen                      11/2009
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt       ECXDBaseXAttributeHandler::DeleteXAttribute (XAttributeHandleR xAttr)
    {
    return  CURRTXN(xAttr.GetElementRef()).DeleteXAttribute (xAttr);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Casey.Mullen                      11/2009
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt       ECXDBaseXAttributeHandler::DeleteXAttribute (ElementRefP hostElementRef, UInt32 xAttrId)
    {
    XAttributeHandle xaIter (hostElementRef, GetId(), xAttrId);
    if (!xaIter.IsValid())
        return ERROR;

    return DeleteXAttribute (xaIter);
    }

/*---------------------------------------------------------------------------------------

    ECXDInstance XAttributeHandler

+---------------+---------------+---------------+---------------+---------------+------*/
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    05/10
+---------------+---------------+---------------+---------------+---------------+------*/
ECXDInstanceXAttributeHandler& ECXDInstanceXAttributeHandler::GetHandler ()
    {
    static ECXDInstanceXAttributeHandler s_this;
    
    return s_this;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    05/10
+---------------+---------------+---------------+---------------+---------------+------*/
XAttributeHandlerId ECXDInstanceXAttributeHandler::_GetId ()
    {
    return XAttributeHandlerId (XATTRIBUTEID_ECXData, 0);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    05/10
+---------------+---------------+---------------+---------------+---------------+------*/
WString         ECXDInstanceXAttributeHandler::_GetLabel ()
    {
    return L"ECXDInstance";
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    05/10
+---------------+---------------+---------------+---------------+---------------+------*/
void            ECXDInstanceXAttributeHandler::Register()
    {
    XAttributeHandlerManager::RegisterHandler         (GetId(), &GetHandler());
    }

/*---------------------------------------------------------------------------------------

    ECXDStructValue XAttributeHandler

+---------------+---------------+---------------+---------------+---------------+------*/
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    05/10
+---------------+---------------+---------------+---------------+---------------+------*/
ECXDStructValueXAttributeHandler& ECXDStructValueXAttributeHandler::GetHandler ()
    {
    static ECXDStructValueXAttributeHandler s_this;
    
    return s_this;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    05/10
+---------------+---------------+---------------+---------------+---------------+------*/
XAttributeHandlerId ECXDStructValueXAttributeHandler::_GetId ()
    {
    return XAttributeHandlerId (XATTRIBUTEID_ECXDStructValue, 0);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    05/10
+---------------+---------------+---------------+---------------+---------------+------*/
WString         ECXDStructValueXAttributeHandler::_GetLabel ()
    {
    return L"ECXDStructValue";
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    JoshSchifter    05/10
+---------------+---------------+---------------+---------------+---------------+------*/
void            ECXDStructValueXAttributeHandler::Register()
    {
    XAttributeHandlerManager::RegisterHandler          (GetId(), &GetHandler());
    }

END_BENTLEY_DGNPLATFORM_NAMESPACE
