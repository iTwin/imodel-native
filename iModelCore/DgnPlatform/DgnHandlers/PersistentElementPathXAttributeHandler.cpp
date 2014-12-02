/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnHandlers/PersistentElementPathXAttributeHandler.cpp $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include    <DgnPlatformInternal.h>

#include <RmgrTools/Tools/DataExternalizer.h>

#define CURRTXN(a) a->GetDgnProject()->GetTxnManager().GetCurrentTxn()

enum XAttrSubID
    {
    GENERICXATTRIBUTEHANDLER_PersistentElementPath   = 0,
    GENERICXATTRIBUTEHANDLER_PersistentSnapPath      = 1,
    };

//! Get HandlerID
XAttributeHandlerId PersistentElementPathXAttributeHandler::GetHandlerId ()
    {
    return XAttributeHandlerId (XATTRIBUTEID_Generic, GENERICXATTRIBUTEHANDLER_PersistentElementPath);
    }

//! Read
StatusInt readData (PersistentElementPath& path, PersistentElementPath::CopyOption& ccOpt, UInt32 xaDataSize, void const* xaData)
    {
    DataInternalizer source ((byte*)xaData, xaDataSize);
    StatusInt s = path.Load (source);
    if (SUCCESS != s)
        return s;

    if (source.getRemainingSize ())
        {
        UInt8 ccOptVal;
        source.get (&ccOptVal);
        ccOpt = (PersistentElementPath::CopyOption) ccOptVal;
        }
    else
        {
        ccOpt = PersistentElementPath::COPYOPTION_DeepCopyAcrossFiles;
        }

    return SUCCESS;
    }

//! Write
void writeData (DataExternalizer& data, PersistentElementPath const& path, PersistentElementPath::CopyOption ccOpt)
    {
    path.Store (&data);
    data.put ((UInt8)ccOpt);
    }

//! Get
StatusInt PersistentElementPathXAttributeHandler::GetPersistentElementPath (PersistentElementPath& path, XAttributeHandleCR xi)
    {
    return GetPersistentElementPath (path, xi.GetSize(), (byte*)xi.PeekData());
    }

//! Get
StatusInt PersistentElementPathXAttributeHandler::GetPersistentElementPath (PersistentElementPath& path, UInt32 xaDataSize, void const* xaData)
    {
    PersistentElementPath::CopyOption ccOpt;
    return readData (path, ccOpt, xaDataSize, xaData);
    }

//! Get
StatusInt PersistentElementPathXAttributeHandler::GetPersistentElementPath (PersistentElementPath& path, ElementHandleCR elem, UInt32 xAttrId)
    {
    ElementHandle::XAttributeIter xa (elem, GetHandlerId(), xAttrId);
    if (!xa.IsValid())
        return ERROR;
    return GetPersistentElementPath (path, xa.GetSize(), xa.PeekData());
    }

//! Set
StatusInt PersistentElementPathXAttributeHandler::SetPersistentElementPath (XAttributeHandleR xa, PersistentElementPath const& path, PersistentElementPath::CopyOption ccOpt)
    {
    DataExternalizer data;
    writeData (data, path, ccOpt);
    return CURRTXN(xa.GetElementRef()).ReplaceXAttributeData (xa, data.getBuf(), (UInt32) data.getBytesWritten());
    }

//! Add
StatusInt PersistentElementPathXAttributeHandler::AddPersistentElementPath (UInt32& xAttrId, ElementRefP hostElement, PersistentElementPath const& path, PersistentElementPath::CopyOption ccOpt)
    {
    DataExternalizer data;
    writeData (data, path, ccOpt);
    return  CURRTXN(hostElement).AddXAttribute (hostElement, GetHandlerId(), xAttrId, data.getBuf(), (UInt32) data.getBytesWritten(), &xAttrId);
    }

//! Add
StatusInt PersistentElementPathXAttributeHandler::ScheduleWritePersistentElementPath (EditElementHandleR hostElement, UInt32 xAttrId, PersistentElementPath const& path, PersistentElementPath::CopyOption ccOpt)
    {
    DataExternalizer data;
    writeData (data, path, ccOpt);
    return hostElement.ScheduleWriteXAttribute (GetHandlerId(), xAttrId, data.getBytesWritten(), data.getBuf());
    }

#ifdef DGN_IMPORTER_REORG_WIP
//! Callback Invoked when the XAttribute is cloned
//! @param replacer IN where to write the new version of the XAttribute
//! @param xa   IN  the XAttribute tha1t is being copied
//! @param eh   IN  the element that holds the XAttribute
//! @param cc   IN  the copy context
//! @returns non-zero if XAttribute should \em not be copied
StatusInt PersistentElementPathXAttributeHandler::_OnPreprocessCopy (IReplaceXAttribute* replacer, XAttributeHandleCR xa, ElementHandleCR eh, CopyContextP cc)
    {
#if defined (WIP_V10_PEP_REMAPPING)
    //  Trigger deep copies and/or mark persistent references as needing to be remapped
    PersistentElementPath path;
    PersistentElementPath::CopyOption ccOpt;
    readData (path, ccOpt, xa.GetSize(), xa.PeekData());
    if (path.OnPreprocessCopy (eh, cc, ccOpt) != SUCCESS)
        return ERROR;      // (or just strip this PersistentElementPath out of your XAttribute if you prefer)

    //  Store this intermediate state!
    DataExternalizer sink;
    writeData (sink, path, ccOpt);

    //  ***NB: Do NOT call CURRTXN.AddXAttribute!

    replacer->Add (xa, sink.getBytesWritten(), sink.getBuf());
#endif
    return SUCCESS;
    }
#endif

    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      07/2006
+---------------+---------------+---------------+---------------+---------------+------*/
void  PersistentElementPathXAttributeHandler::StaticInitialize()
    {
    static PersistentElementPathXAttributeHandler s_this;

    DgnSystemDomain::GetInstance().RegisterXAttributeHandler (GetHandlerId(), s_this);
    }

//! Get HandlerID
XAttributeHandlerId PersistentSnapPathXAttributeHandler::GetHandlerId ()
    {
    return XAttributeHandlerId (XATTRIBUTEID_Generic, GENERICXATTRIBUTEHANDLER_PersistentSnapPath);
    }

//! Read
StatusInt readData (PersistentSnapPath& path, PersistentElementPath::CopyOption& ccOpt, UInt32 xaDataSize, void const* xaData)
    {
    DataInternalizer source ((byte*)xaData, xaDataSize);
    StatusInt s = path.Load (source);
    if (SUCCESS != s)
        return s;

    if (source.getRemainingSize ())
        {
        UInt8 ccOptVal;
        source.get (&ccOptVal);
        ccOpt = (PersistentElementPath::CopyOption) ccOptVal;
        }
    else
        {
        ccOpt = PersistentElementPath::COPYOPTION_DeepCopyAcrossFiles;
        }

    return SUCCESS;
    }

//! Write
void writeData (DataExternalizer& data, PersistentSnapPath const& path, PersistentElementPath::CopyOption ccOpt)
    {
    path.Store (data);
    data.put ((UInt8)ccOpt);
    }

//! Get
StatusInt PersistentSnapPathXAttributeHandler::GetPersistentSnapPath (PersistentSnapPath& path, XAttributeHandleCR xi)
    {
    return GetPersistentSnapPath (path, xi.GetSize(), (byte*)xi.PeekData());
    }

//! Get
StatusInt PersistentSnapPathXAttributeHandler::GetPersistentSnapPath (PersistentSnapPath& path, UInt32 xaDataSize, void const* xaData)
    {
    PersistentElementPath::CopyOption ccOpt;
    return readData (path, ccOpt, xaDataSize, xaData);
    }

//! Get
StatusInt PersistentSnapPathXAttributeHandler::GetPersistentSnapPath (PersistentSnapPath& path, ElementHandleCR elem, UInt32 xAttrId)
    {
    ElementHandle::XAttributeIter xa (elem, GetHandlerId(), xAttrId);
    if (!xa.IsValid())
        return ERROR;
    return GetPersistentSnapPath (path, xa.GetSize(), xa.PeekData());
    }

//! Set
StatusInt PersistentSnapPathXAttributeHandler::SetPersistentSnapPath (XAttributeHandleR xa, PersistentSnapPath const& path, PersistentElementPath::CopyOption ccOpt)
    {
    DataExternalizer data;
    writeData (data, path, ccOpt);
    return CURRTXN(xa.GetElementRef()).ReplaceXAttributeData (xa, data.getBuf(), (UInt32) data.getBytesWritten());
    }

//! Add
StatusInt PersistentSnapPathXAttributeHandler::AddPersistentSnapPath (UInt32& xAttrId, ElementRefP hostElement, PersistentSnapPath const& path, PersistentElementPath::CopyOption ccOpt)
    {
    DataExternalizer data;
    writeData (data, path, ccOpt);
    return CURRTXN(hostElement).AddXAttribute (hostElement, GetHandlerId(), xAttrId, data.getBuf(), (UInt32) data.getBytesWritten(), &xAttrId);
    }

//! Schedule
StatusInt PersistentSnapPathXAttributeHandler::ScheduleWritePersistentSnapPath (EditElementHandleR hostElement, UInt32 xAttrId, PersistentSnapPath const& path, PersistentElementPath::CopyOption ccOpt)
    {
    DataExternalizer data;
    writeData (data, path, ccOpt);
    return hostElement.ScheduleWriteXAttribute (GetHandlerId(), xAttrId, data.getBytesWritten(), data.getBuf());
    }

#ifdef DGN_IMPORTER_REORG_WIP
//! Callback Invoked when the XAttribute is cloned
//! @param replacer IN where to write the new version of the XAttribute
//! @param xa   IN  the XAttribute that is being copied
//! @param eh   IN  the element that holds the XAttribute
//! @param cc   IN  the copy context
//! @returns non-zero if XAttribute should \em not be copied
StatusInt PersistentSnapPathXAttributeHandler::_OnPreprocessCopy (IReplaceXAttribute* replacer, XAttributeHandleCR xa, ElementHandleCR eh, CopyContextP cc)
    {
#if defined (WIP_V10_PEP_REMAPPING)

    //  Trigger deep copies and/or mark persistent references as needing to be remapped
    PersistentSnapPath path (eh.GetDgnModel());
    PersistentElementPath::CopyOption ccOpt;
    readData (path, ccOpt, xa.GetSize(), xa.PeekData());
    if (path.OnPreprocessCopy (eh, cc, ccOpt) != SUCCESS)
        return ERROR;      // (or just strip this PersistentSnapPath out of your XAttribute if you prefer)

    //  Store this intermediate state!
    DataExternalizer sink;
    writeData (sink, path, ccOpt);

    //  ***NB: Do NOT call CURRTXN.AddXAttribute!

    replacer->Add (xa, sink.getBytesWritten(), sink.getBuf());
#endif
    return SUCCESS;
    }
#endif

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      07/2006
+---------------+---------------+---------------+---------------+---------------+------*/
void            PersistentSnapPathXAttributeHandler::StaticInitialize ()
    {
#if defined ELEMENT_LOADING_REWORK
    static PersistentSnapPathXAttributeHandler s_this;

    XAttributeHandlerManager::RegisterHandler        (GetHandlerId(), &s_this);
#endif
    }

