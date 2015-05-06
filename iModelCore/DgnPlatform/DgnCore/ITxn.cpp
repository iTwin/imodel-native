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

    // delete from cache, remove from range tree
    return elRef->_DeleteInDb();
    }

