/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnCore/ITxn.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <DgnPlatformInternal.h>

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   02/05
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt ITxn::CheckElementForWrite (DgnElementP elRef)
    {
    return (NULL == elRef) ? DGNHANDLERS_STATUS_BadElement : _CheckDgnModelForWrite (&elRef->GetDgnModel());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   01/05
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt ITxn::DeleteElement (DgnElementP elRef)
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

    ClearReversedTxns (elRef->GetDgnDb());

    // Assume that cascading delete will clean up aspects.

    // delete from cache, remove from range tree
    return elRef->_DeleteInDb(elRef->GetDgnDb().Elements().GetPool());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   02/05
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt ITxn::ReplaceElement(EditElementHandleR eeh)
    {
    if (m_opts.m_writesIllegal)
        {
        BeAssert(false);
        return ERROR;
        }

    DgnElementP original  = const_cast<DgnElementP>(eeh.GetPersistentElement());
    DgnElementP  modified = eeh.GetEditElement();
    if (nullptr == original || nullptr == modified)
        {
        BeAssert (false);
        return ERROR;
        }

    ClearReversedTxns(original->GetDgnDb());

#if defined (NEEDS_WORK_ELEMDSCR_REWORK)
    if (ElementHandler::PRE_ACTION_Block == original->GetElementHandler()._OnElementModify(eeh))
        return DGNHANDLERS_STATUS_UserAbort;
#endif

    DgnModelStatus status = original->GetDgnModel().ReplaceElement(*original, *modified);
    if (DGNMODEL_STATUS_Success != status)
        return  status;

    eeh.ClearWriteable();
    return SUCCESS;
    }
