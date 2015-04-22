/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnCore/ElemHandle.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include    <DgnPlatformInternal.h>

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   10/11
+---------------+---------------+---------------+---------------+---------------+------*/
void ElementHandle::Clone(ElementHandle const& from)
    {
    if (this == &from)
        return;

    m_persistent = from.m_persistent;
    m_writeable  = from.m_writeable;
    m_state = from.m_state;

    if (m_state)
        m_state->AddRef();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   02/14
+---------------+---------------+---------------+---------------+---------------+------*/
ElementHandle::ElementHandle(ElementHandleCR from) : m_persistent(from.m_persistent), m_writeable(from.m_writeable), m_state(from.m_state)
    {
    if (m_state)
        m_state->AddRef();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   08/07
+---------------+---------------+---------------+---------------+---------------+------*/
DgnModelP ElementHandle::GetDgnModelP() const
    {
    if (m_persistent.IsValid())
        return &m_persistent->GetDgnModel();

    return (m_writeable.IsValid() ? &m_writeable->GetDgnModel() : nullptr);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   10/04
+---------------+---------------+---------------+---------------+---------------+------*/
DgnElementP EditElementHandle::GetElementDescrP() 
    {
    if (!m_writeable.IsValid() && m_persistent.IsValid())
        m_writeable = m_persistent->Duplicate();

    return m_writeable.get();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Keith.Bentley   11/03
+---------------+---------------+---------------+---------------+---------------+------*/
void ElementHandle::AssignElemDescr(DgnElementP newDscr, bool isPersistent)
    {
    if (newDscr == m_writeable.get())
        return;

    if (newDscr)
        {
        AssignDgnElement(isPersistent ? newDscr : nullptr);
        }
    else
        {
        ClearWriteable();
        ClearPersistent();
        }

    m_writeable = newDscr;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Keith.Bentley   11/03
+---------------+---------------+---------------+---------------+---------------+------*/
DgnElementPtr EditElementHandle::ExtractElementDescr()
    {
    DgnElementPtr currDescr = m_writeable;
    m_writeable = nullptr;
    return currDescr;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Keith.Bentley   11/03
+---------------+---------------+---------------+---------------+---------------+------*/
EditElementHandle::EditElementHandle(ElementHandleCR from, bool duplicateDscr) : ElementHandle(from)
    {
    if (!m_writeable.IsValid())
        return;

    if (duplicateDscr)
        m_writeable = m_writeable->Duplicate();
    }

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

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   01/09
+---------------+---------------+---------------+---------------+---------------+------*/
void ElementHandle::Init(DgnElementId id, DgnDbR project)
    {
    Init(project.Elements().GetElementById(id).get());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      11/2008
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt EditElementHandle::FindById(DgnElementId elemID, DgnModelP model, bool allowDeleted)
    {
    if (nullptr == model)
        return ERROR;
    
    DgnElementPtr  element = model->GetDgnDb().Elements().GetElementById(elemID);
    if (!element.IsValid())
        return DGNMODEL_STATUS_ElementNotFound;

    if (!allowDeleted && element->IsDeleted())
        return DGNMODEL_STATUS_ElementNotFound;

    SetDgnElement(element.get());
    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void ElementHandle::Init(DgnElementCP elRef)
    {
    m_persistent = nullptr;
    m_writeable = nullptr;
    m_state = nullptr;

    AssignDgnElement(elRef);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void ElementHandle::Init(DgnElementCP elDscr, bool isPersistent)
    {
    m_writeable = (DgnElementP) elDscr;
    m_persistent = isPersistent ? m_writeable : nullptr;
    m_state = nullptr;

    BeAssert(!m_writeable.IsValid() || GetDgnModelP()); // DgnModel is required.
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   02/05
+---------------+---------------+---------------+---------------+---------------+------*/
ElementHandlerR ElementHandle::GetElementHandler() const
    {
    if (!IsValid())  
        {
        BeAssert(false);
        ElementHandlerP bad=nullptr;
        return *bad;
        }

    return GetDgnElement()->GetElementHandler();
    }
