/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnCore/ElemHandle.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <DgnPlatformInternal.h>

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   06/05
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt EditElementHandle::DeleteFromModel()
    {
    if (!m_persistent.IsValid() && m_writeable.IsValid())
        m_persistent = m_writeable;
        
    if (!m_persistent.IsValid())
        {
        BeAssert(false);
        return ERROR;
        }

    StatusInt status = m_persistent->GetDgnDb().GetTxnManager().GetCurrentTxn().DeleteElement(m_persistent.get());
    if (SUCCESS == status)
        Invalidate();

    return  status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   06/05
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt EditElementHandle::ReplaceInModel()
    {
    if (!m_persistent.IsValid())
        {
        BeAssert(false);
        return ERROR;
        }
    return  m_persistent->GetDgnDb().GetTxnManager().GetCurrentTxn().ReplaceElement(*this);
    }

