/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnCore/ITxn.cpp $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <DgnPlatformInternal.h>
#include <DgnPlatform/DgnCore/RefCountedMSElementDescr.h>

USING_NAMESPACE_BENTLEY_SQLITE

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      01/2005
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt ITxn::WriteXaChanges (MSElementDescrP ed, bool isAdd)
    {
    return  XAttributeHandler::WriteXAttributeChangeSet (ed->GetElementRef(), ed->QueryXAttributeChangeSet(), isAdd);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   02/05
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt ITxn::CheckElementForWrite (ElementRefP elRef)
    {
    return (NULL == elRef) ? DGNHANDLERS_STATUS_BadElement : _CheckDgnModelForWrite (elRef->GetDgnModelP());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   02/05
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt ITxn::AddElement (EditElementHandleR newEl)
    {
    if (m_opts.m_writesIllegal)
        {
        BeAssert(false);
        return ERROR;
        }

    // Shouldn't matter if DgnModel comes from ElementRef or MSElementDescr. ElementRef should have been cleared by any 
    // operation (ex. copy to different model) that would other cause an inconsistent state for the EditElementHandle.
    DgnModelP modelP = newEl.GetDgnModelP (); 
    if (NULL == modelP)
        {
        BeAssert (false);// Invalid EditElementHandle passed to ITxn::AddElement...
        return ERROR;
        }

    DgnModelR model = *modelP;

    BeAssert (newEl.PeekElementDescrCP());

    StatusInt status = _CheckDgnModelForWrite (&model);
    if (SUCCESS != status)
        return  status;

    ClearReversedTxns(model.GetDgnProject());

    if (Handler::PRE_ACTION_Block == newEl.GetHandler()._OnAdd (newEl))
        return ERROR;

    MSElementDescrPtr elDescr = newEl.ExtractElementDescr();
    if (!elDescr.IsValid())
        return  ERROR;

    status = model.AddElementDescr (*elDescr, AddNewElementOptions(model));
    if (SUCCESS != status)
        return status;

    PersistentElementRefP newElemRef = (PersistentElementRefP) elDescr->GetElementRef(); // get new ElementRefP from descr

    //  If any attribute adds are scheduled, do them now. (This also processes XA changes on components.)
    status = WriteXaChanges (elDescr.get(), true);

    // this clears element descriptor from the elHandle, but that's OK (in fact necessary) for two reasons:
    //  1) AddElement may have written a different element than the one we gave it due to asynchs
    //  2) we want to get rid of the XAttr change set.
    newEl.AssignElementRef (newElemRef);

    newEl.GetHandler()._OnAdded (*newElemRef);

    return status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   02/05
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt ITxn::AddElemDescr (ElementRefP& newElemRef, DgnModelP model, MSElementDescrP elDescr)
    {
    BeAssert (NULL != elDescr);
    newElemRef = NULL;         // clear ElementRefP return

    elDescr->SetDgnModel(*model);  // don't assume this is right on input

    // we're going to treat this descr as an "owned" descr, but the caller still has a reference to it, addRef
    elDescr->AddRef();
    EditElementHandle newElem (elDescr, false);

    StatusInt   status = AddElement (newElem);
    if (SUCCESS != status)
        return  status;

    newElemRef = newElem.GetElementRef();
    elDescr->SetElementRef(newElemRef);

    return  SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* add an xAttr to an element. Saves in database and calls asynchs.
* @bsimethod                                    Keith.Bentley                   11/04
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt ITxn::AddXAttribute (ElementRefP elRef, XAttributeHandlerId handlerId, UInt32 xAttrIdIn, void const* data, UInt32 size, UInt32* outXAttrId, DgnModels::XAttributeFlags flags)
    {
    if (m_opts.m_writesIllegal)
        {
        BeAssert(false);
        return ERROR;
        }
    BeAssert (size > 0);

    StatusInt status = CheckElementForWrite (elRef);
    if (SUCCESS != status)
        return  status;
    
    DgnProjectP dgnProject = elRef->GetDgnProject();
    ClearReversedTxns (*dgnProject);       // must be before add

    UInt32 tAttrId;
    UInt32& xAttrId = (NULL == outXAttrId) ? tAttrId : *outXAttrId;
    
    xAttrId = xAttrIdIn;

    // PreNotify XAttributeHandler
    XAttributeHandlerP xattHandler = dgnProject->Domains().FindXAttributeHandler(handlerId);
    if (NULL != xattHandler)
        xattHandler->_OnPreAdd (elRef, handlerId, xAttrId);

    // if we're adding an XAttr to a child, we have to make sure there's an entry for the child element
#if defined (NEEDS_WORK_DGNITEM)
    ElementRefP parent = elRef->GetParentElementRef();
    if (NULL != parent)
        {
        while (NULL != parent->GetParentElementRef())
            parent=parent->GetParentElementRef();

        DbResult result = elRef->GetDgnProject()->Models().InsertChildElement (parent->GetElementId(), elRef->GetElementId());
        if (result != BE_SQLITE_DONE)
            { BeAssert(false); }
        }
#endif

    // Add the new xAttr to the element.
    Int64 rowid;
    elRef->ClearXAttCache(handlerId, xAttrId);
    DbResult rc =   elRef->GetDgnProject()->Models().InsertXAttribute (rowid, elRef->GetElementId(), handlerId, xAttrId, size, data, flags);
    return (rc != BE_SQLITE_DONE) ? ERROR : SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* delete an xAttr from an element. Saves in undo and calls asynchs.
* @bsimethod                                    Keith.Bentley                   11/04
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt ITxn::DeleteXAttribute (XAttributeHandleR xAttr)
    {
    if (m_opts.m_writesIllegal)
        {
        BeAssert(false);
        return ERROR;
        }

    // save ids, xAttr is invalidated by delete
    ElementRefP elRef = xAttr.GetElementRef();
    StatusInt status = CheckElementForWrite (elRef);
    if (SUCCESS != status)
        return  status;

    DgnProjectP dgnProject = elRef->GetDgnProject();
    ClearReversedTxns (*dgnProject);       // must be before add

    // PreNotify XAttributeHandler
    XAttributeHandlerP xattHandler = xAttr.GetHandler();
    if (xattHandler)
        xattHandler->_OnPreDelete (xAttr);

    DbResult rc = xAttr.DeleteFromFile();
    if (BE_SQLITE_DONE != rc)
        return  ERROR;         // delete failed, don't continue

    return  SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* modify (a part of) an xAttr of an element. Saves in undo and calls asynchs.
* @bsimethod                                    Keith.Bentley                   11/04
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt ITxn::ModifyXAttributeData (XAttributeHandleR xAttr, void const* data, UInt32 start, UInt32 length, DgnModels::XAttributeFlags flags)
    {
    if (m_opts.m_writesIllegal)
        {
        BeAssert(false);
        return  ERROR;
        }

    ElementRefP elRef = xAttr.GetElementRef();
    StatusInt status = CheckElementForWrite (elRef);
    if (SUCCESS != status)
        return  status;

    DgnProjectP dgnProject = elRef->GetDgnProject();
    ClearReversedTxns (*dgnProject);       // must be before add

    // PreNotify XAttributeHandler
    XAttributeHandlerP xattHandler = xAttr.GetHandler();
    if (xattHandler)
        xattHandler->_OnPreModifyData (xAttr, data, static_cast<UInt32>(start), static_cast<UInt32>(length));
    
    DbResult result = xAttr.ModifyData (data, start, length, flags);
    if (BE_SQLITE_OK != result)
        return ERROR;

    return  status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   11/04
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt ITxn::ReplaceXAttributeData (XAttributeHandleR xAttr, void const* data, UInt32 newSize, DgnModels::XAttributeFlags flags)
    {
    if (m_opts.m_writesIllegal)
        {
        BeAssert(false);
        return ERROR;
        }

    // for replace, we have to save the pre-change state in the undo buffer BEFORE we write to the cache.
    // This is (potentially) going to write in-place.
    ElementRefP elRef = xAttr.GetElementRef();
    StatusInt status = CheckElementForWrite (elRef);
    if (SUCCESS != status)
        return  status;

    // PreNotify XAttributeHandler
    XAttributeHandlerP handler = xAttr.GetHandler();
    if (handler)
        handler->_OnPreReplaceData (xAttr, data, static_cast<UInt32>(newSize));

    ClearReversedTxns (*elRef->GetDgnProject());

    DbResult result = xAttr.ReplaceData (data, newSize, flags);
    if (BE_SQLITE_DONE != result)
        return ERROR;
    xAttr.ClearElemRefCache();

    return  status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   01/05
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt ITxn::DeleteElement (ElementRefP elRef)
    {
    if (m_opts.m_writesIllegal)
        {
        BeAssert(false);
        return ERROR;
        }

    BeAssert (NULL != elRef);

    StatusInt status = CheckElementForWrite (elRef);
    if (SUCCESS != status)
        return  status;

    if (elRef->IsDeleted()) // Already deleted?
        {
        BeAssert (false);
        return SUCCESS;
        }

    ClearReversedTxns (*elRef->GetDgnProject());

    EditElementHandle  deleteHandle (elRef);
    MSElementDescrP elDescr = deleteHandle.GetElementDescrP();

    BeAssert (NULL != elDescr);

    // delete from cache, remove from range tree
    return elRef->DeleteElement();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   02/05
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt ITxn::ReplaceElement (EditElementHandleR eeh, ElementRefP inEl)
    {
    if (m_opts.m_writesIllegal)
        {
        BeAssert(false);
        return ERROR;
        }

    PersistentElementRefP inRef = dynamic_cast<PersistentElementRefP>(inEl);
    if (NULL == inRef)
        {
        BeAssert (false);
        return ERROR;
        }

    if (NULL == eeh.PeekElementDescrCP())
        return ERROR;

    StatusInt status = CheckElementForWrite (inRef);
    if (SUCCESS != status)
        return status;

    ClearReversedTxns (*inRef->GetDgnProject());

    EditElementHandle oldElem (inRef);
    MSElementDescrP oldDscr = oldElem.GetElementDescrP();
    if (NULL == oldDscr)
        return DGNMODEL_STATUS_ReplacingDeleted;

    DgnModelR dgnModel = *inRef->GetDgnModelP();
    if (eeh.GetDgnModelP () != &dgnModel)
        eeh.SetDgnModel (dgnModel); // Should maybe assert and early return instead?

    if (Handler::PRE_ACTION_Block == oldElem.GetHandler()._OnModify(eeh, oldElem))
        return DGNHANDLERS_STATUS_UserAbort;

    // NOTE: This used to have "GetElementDescrP". I changed it so that the callbacks don't attempt to change it if they're holding
    // on to eeh some other way. Let me know if this causes problems - KAB
    MSElementDescrPtr newElemDscr = eeh.ExtractElementDescr();
    if (!newElemDscr.IsValid())
        return ERROR;

    ElementListHandlerR list = inRef->GetListHandler();
    status = list.ReplaceElement (*newElemDscr, *inRef, AddNewElementOptions(dgnModel));
    if (SUCCESS != status)
        return  status;

    PersistentElementRefP newElemRef = (PersistentElementRefP) newElemDscr->GetElementRef(); 

    // If any attribute adds, deletes, or replacements are scheduled, do them now.
    status = WriteXaChanges (newElemDscr.get(), false);

    eeh.AssignElementRef (newElemRef);
    eeh.GetHandler()._OnModified(*newElemRef);

    return status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   02/05
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt ITxn::ReplaceElementDescr (ElementRefP& out, ElementRefP inRef, MSElementDescrP newElemDscr)
    {
    ElementRefP  in = inRef; // preserve original in case same as out
    out = NULL;

    if (NULL == in || NULL == newElemDscr)
        {
        BeAssert(0);
        return DGNHANDLERS_STATUS_BadElement;
        }

    newElemDscr->AddRef();    // we're going to treat this descr as an "owned" descr, but the caller still has a reference to it, addRef
    EditElementHandle  eeh (newElemDscr, false);

    StatusInt status = ReplaceElement (eeh, in);
    if (SUCCESS != status)
        return  status;

    out = eeh.GetElementRef();
    return  SUCCESS;
    }

