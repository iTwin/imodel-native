/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnCore/DisplayStyleHandler.cpp $
|    $RCSfile: DisplayStyleHandler.cpp,v $
|   $Revision: 7.7 $
|       $Date: 2011/07/11 19:54:05 $
|     $Author: Ray.Bentley $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <DgnPlatformInternal.h>


static DisplayStyleHandlerManager       s_displayStyleHandlerManager;

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley       10/2010
+---------------+---------------+---------------+---------------+---------------+------*/
DisplayStyleHandlerManagerR DisplayStyleHandlerManager::GetManager () { return s_displayStyleHandlerManager; }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley       10/2010
+---------------+---------------+---------------+---------------+---------------+------*/
void    DisplayStyleHandlerManager::RegisterHandler (DisplayStyleHandlerR handler)
    {
    m_handlerMap[handler.GetHandlerId()] = &handler;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley       10/2010
+---------------+---------------+---------------+---------------+---------------+------*/
void    DisplayStyleHandlerManager::RegisterEditor (DisplayStyleHandlerSettingsEditorR editor)
    {
    m_editorMap[editor.GetHandlerId()] = &editor;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley       10/2010
+---------------+---------------+---------------+---------------+---------------+------*/
DisplayStyleHandlerP  DisplayStyleHandlerManager::GetHandler (XAttributeHandlerId id)
    {
    T_DisplayStyleHandlerMap::iterator   found;
    return ((found = m_handlerMap.find (id)) == m_handlerMap.end()) ? NULL : found->second;
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley       10/2010
+---------------+---------------+---------------+---------------+---------------+------*/
DisplayStyleHandlerSettingsEditorP  DisplayStyleHandlerManager::GetEditor (XAttributeHandlerId id)
    {
    T_DisplayStyleHandlerSettingsEditorMap::iterator   found;
    return ((found = m_editorMap.find (id)) == m_editorMap.end()) ? NULL : found->second;
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley       10/2010
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt DisplayStyleHandlerSettings::ReadData (bvector<byte>& data, ElementRefP elementRef, DisplayStyleHandler_SettingsXAttributeSubId settingsId, int styleIndex)
    {
    XAttributeHandle        handlerXAttr (elementRef, XAttributeHandlerId (XATTRIBUTEID_DisplayStyleHandler, (UInt16) settingsId), styleIndex);
    size_t                  handlerXAttrUncompressedSize;

    if (!handlerXAttr.IsValid() ||
        SUCCESS != CompressedXAttribute::GetUncompressedSize (handlerXAttr, &handlerXAttrUncompressedSize))
        return ERROR;

    data.resize(handlerXAttrUncompressedSize);
    return CompressedXAttribute::ExtractBuffer (handlerXAttr, &data[0], handlerXAttrUncompressedSize);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley       10/2010
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt DisplayStyleHandlerSettings::SaveData (void* pData, size_t dataSize,  ElementRefP elementRef, DisplayStyleHandler_SettingsXAttributeSubId settingsId, int styleIndex)
    {
    XAttributeHandlerId     handlerId (XATTRIBUTEID_DisplayStyleHandler, (byte) settingsId);
    XAttributeHandle        handlerXAttr (elementRef, handlerId, styleIndex);

    if (handlerXAttr.IsValid ())
        {
        return CompressedXAttribute::Replace (handlerXAttr, (byte const*) pData, dataSize, COMPRESSION_THRESHOLD_MinimumAccess);
        }
    else
        {
        return CompressedXAttribute::Add (elementRef, handlerId, styleIndex, (byte const*) pData, dataSize, COMPRESSION_THRESHOLD_MinimumAccess, NULL);
        }
    }


#ifdef WIP_ENUMERATE_HANDLERS
/*=================================================================================**//**
* @bsiclass                                                     RayBentley      10/2010
+===============+===============+===============+===============+===============+======*/
struct  DisplayStyleHandlerMap : IEnumerateAvailableHandlers
{
    std::map <WString, DisplayHandlerP>      m_handlerMap;
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      10/2010
+---------------+---------------+---------------+---------------+---------------+------*/
DisplayStyleHandlerMap ()
    {
    Handler::EnumerateAvailableHandlers  (*this);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt       ProcessHandler (Handler& handler)
    {
    DisplayHandlerP     displayHandler;
    if (NULL != (displayHandler = dynamic_cast <DisplayHandlerP> (&handler)))
        m_handlerMap[WString (typeid (handler).name())] = displayHandler;

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DisplayHandlerP FindHandler (char const* name)
    {
    std::map <WString, DisplayHandlerP>::iterator      found = m_handlerMap.find (WString (name));

    return found == m_handlerMap.end() ? NULL : found->second;
    }

};  // DisplayStyleHandlerMap

#endif

/*=================================================================================**//**
* @bsiclass                                                     RayBentley      10/2010
+===============+===============+===============+===============+===============+======*/
 enum   DisplayStyleHandlerSettingsSubType
 {
    DisplayStyleHandlerSettingsSubType_HandlerName              = 1,
 };


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      10/2010
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt DisplayStyleHandlerSettings::_Read (ElementRefP elementRef, int styleIndex) 
    {
    bvector<byte>       buffer;

    if (SUCCESS !=  ReadData (buffer, elementRef, DisplayStyleHandler_SettingsXAttributeSubId_Base, styleIndex))
        return ERROR;

    m_styleIndex = styleIndex;

    byte*                   data = &buffer[0];
    size_t                  dataSize = buffer.size();

    memset (&m_data, 0, sizeof (m_data));
    memcpy (&m_data, data, MIN (sizeof (m_data), dataSize));

    // Read SubTypes/
    byte*                               pFragment = (byte*) data + sizeof (m_data);
    byte*                               pEnd      = (byte*) data + dataSize;
//  DisplayStyleHandlerMap              handlerMap;

    while (pFragment < pEnd)
        {
        UInt16         fragmentSize, subType;

        memcpy (&fragmentSize, pFragment, sizeof (fragmentSize));
        if (pFragment + fragmentSize > pEnd)
            break;

        memcpy (&subType, pFragment + 2, sizeof (subType));

#ifdef WIP_ENUMERATE_HANDLERS
        byte*               fragmentData     = pFragment + 4;
        size_t              fragmentDataSize = fragmentSize - 4;

        switch (subType)
            {
            case DisplayStyleHandlerSettingsSubType_HandlerName:
                { 
                DisplayHandlerP     handler;

                if (NULL != (handler = handlerMap.FindHandler ((char const*) fragmentData)))
                    m_handlersToApply.insert (handler);
                
                break;
                }
            }
#endif
     
        pFragment += fragmentSize;
        }
    
    return SUCCESS;
    }
     
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      10/2010
+---------------+---------------+---------------+---------------+---------------+------*/
void    DisplayStyleHandlerSettings::AddSettingsSubType (bvector <byte>& buffer, UInt16 type, UInt16 dataSize, void const* data)
    {
    UInt16      fragmentSize = dataSize + 4;
    size_t      currSize = buffer.size();
    size_t      newSize  = currSize + fragmentSize;

    buffer.resize (newSize);
    memcpy (&buffer[currSize], &fragmentSize, 2);
    memcpy (&buffer[currSize+2], &type, 2);
    memcpy (&buffer[currSize+4], data, dataSize);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      10/2010
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt   DisplayStyleHandlerSettings::_Save (ElementRefP elementRef, int styleIndex)
    {
    bvector<byte>                   buffer;

    buffer.resize (sizeof (m_data));
    memcpy (&buffer[0], &m_data, sizeof (m_data));

#ifdef WIP_ENUMERATE_HANDLERS
    for (std::set<DisplayHandlerP>::iterator curr = m_handlersToApply.begin(); curr != m_handlersToApply.end(); curr++)
        {
        char  const*        handlerName = typeid (**curr).name();

        AddSettingsSubType (buffer, DisplayStyleHandlerSettingsSubType_HandlerName, strlen (handlerName)+1, handlerName);
        }
#endif
    return SaveData (&buffer[0], buffer.size(), elementRef, DisplayStyleHandler_SettingsXAttributeSubId_Base, styleIndex);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BrandonBohrer   02/2011
+---------------+---------------+---------------+---------------+---------------+------*/
#ifdef WIP_DISPLAYSTYLE
void    DisplayStyleHandlerSettings::ClearCache () const
    {
//  IViewManager::GetManager ().ClearAllQvElems ();
    }
#endif
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      10/2010
+---------------+---------------+---------------+---------------+---------------+------*/
#ifdef WIP_DISPLAYSTYLE
bool   DisplayStyleHandlerSettings::DoFilter (DisplayPath const& path) const  
    {
#ifdef WIP_ENUMERATE_HANDLERS
    // Always filter on the parent (else we'll end up with mismatched materials).
    switch (GetTypeFilter())
        {
        case    DisplayStyleHandlerTypeFilter_Listed:
            return  !HandlerInFilter (ElemHandle (path.GetPathElem(0), path.GetRoot()).GetDisplayHandler());

        case    DisplayStyleHandlerTypeFilter_Unlisted:
            return  HandlerInFilter (ElemHandle (path.GetPathElem(0), path.GetRoot()).GetDisplayHandler());

        }
#endif
    return false;
    }
#endif

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      10/2010
+---------------+---------------+---------------+---------------+---------------+------*/
#ifdef WIP_DISPLAYSTYLE
bool   DisplayStyleHandlerSettings::HandlerInFilter (DisplayHandlerP handler) const  { return m_handlersToApply.end() != m_handlersToApply.find (handler); }
#endif

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    RayBentley      10/2010
+---------------+---------------+---------------+---------------+---------------+------*/
#ifdef WIP_DISPLAYSTYLE
void   DisplayStyleHandlerSettings::SetHandlerInFilter (DisplayHandlerP handler, bool apply)
    {
    if (apply == HandlerInFilter (handler))
        return;

    if (apply)
        m_handlersToApply.insert (handler);
    else
        m_handlersToApply.erase (handler);

    ClearCache ();
    }
#endif

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BrandonBohrer   06/2011
+---------------+---------------+---------------+---------------+---------------+------*/
DisplayStyleHandlerKeyPtr  DisplayStyleHandler::_GetCacheKey (DgnModelR modelRef, DisplayStyleHandlerSettingsCP settings) const
    {
    return DisplayStyleHandlerKey::Create (*this);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BrandonBohrer   06/2011
+---------------+---------------+---------------+---------------+---------------+------*/
DisplayStyleHandlerKey::DisplayStyleHandlerKey (DisplayStyleHandlerCR handler)
    :
    m_handlerId (handler.GetHandlerId().GetId())
#ifdef WIP_DISPLAYSTYLE
#endif
    {
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    BrandonBohrer   06/2011
+---------------+---------------+---------------+---------------+---------------+------*/
bool    DisplayStyleHandlerKey::Matches (DisplayStyleHandlerKey const& other) const
    {
    #ifdef WIP_DISPLAYSTYLE

    return other.IsValid () && GetHandlerId () == other->GetHandlerId ();
    #endif
    BeAssert(false);
    return false;
    }
