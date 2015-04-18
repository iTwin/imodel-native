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

    m_element = from.m_element;
    m_dscr   = from.m_dscr;
    m_state  = from.m_state;

    if (m_state)
        m_state->AddRef();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   02/14
+---------------+---------------+---------------+---------------+---------------+------*/
ElementHandle::ElementHandle (ElementHandleCR from) : m_element(from.m_element), m_dscr(from.m_dscr), m_state(from.m_state)
    {
    if (m_state)
        m_state->AddRef();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   08/07
+---------------+---------------+---------------+---------------+---------------+------*/
DgnModelP ElementHandle::GetDgnModelP() const
    {
    if (m_element.IsValid ())
        return &m_element->GetDgnModel();

    return (m_dscr.IsValid() ? &m_dscr->GetDgnModel () : nullptr);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   10/04
+---------------+---------------+---------------+---------------+---------------+------*/
DgnElementCP ElementHandle::GetElementDescrCP () const
    {
    if (!m_dscr.IsValid () && m_element.IsValid())
        m_dscr = m_element->Duplicate();

    return m_dscr.get();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Keith.Bentley   11/03
+---------------+---------------+---------------+---------------+---------------+------*/
void ElementHandle::AssignElemDescr (DgnElementP newDscr, bool isPersistent)
    {
    if (newDscr == m_dscr.get())
        return;

    if (newDscr)
        {
        AssignDgnElement (isPersistent ? newDscr : nullptr);
        }
    else
        {
        ClearElementDescr();
        ClearDgnElement();
        }

    m_dscr = newDscr;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      10/2008
+---------------+---------------+---------------+---------------+---------------+------*/
void EditElementHandle::Duplicate(ElementHandleCR eh)
    {
    DgnElementCP ed = eh.GetElementDescrCP ();

    if (nullptr == ed)
        {
        AssignElemDescr (nullptr, false);
        return;
        }

    DgnElementPtr cc = ed->Duplicate();
    
    if (PeekElementDescrCP())
        AssignElemDescr (cc.get(), false);
    else
        SetElementDescr (cc.get(), eh.IsPersistent ());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  03/15
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus EditElementHandle::CreateNewElement(DgnModelR model, DgnClassId elementClassId, DgnCategoryId categoryId, Utf8CP code)
    {
    ElementHandlerP elementHandler = ElementHandler::FindHandler(model.GetDgnDb(), elementClassId);
    if (nullptr == elementHandler)
        {
        BeAssert(false);
        return BentleyStatus::ERROR;
        }

    if (!categoryId.IsValid())
        {
        BeAssert(false);
        return BentleyStatus::ERROR;
        }

    m_element = nullptr;
    m_dscr = elementHandler->Create(DgnElement::CreateParams(model, elementClassId, categoryId, code, DgnElementId()));
    return BentleyStatus::SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Keith.Bentley   11/03
+---------------+---------------+---------------+---------------+---------------+------*/
DgnElementPtr EditElementHandle::ExtractElementDescr ()
    {
    DgnElementPtr currDescr = m_dscr;
    m_dscr = nullptr;
    return currDescr;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Keith.Bentley   11/03
+---------------+---------------+---------------+---------------+---------------+------*/
EditElementHandle::EditElementHandle (ElementHandleCR from, bool duplicateDscr) : ElementHandle(from)
    {
    if (!m_dscr.IsValid())
        return;

#if defined (NEEDS_WORK_ELEMDSCR_REWORK)
    if (duplicateDscr)
        m_dscr = m_dscr->Duplicate(true, true);
#endif
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   06/05
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt EditElementHandle::DeleteFromModel ()
    {
    if (!m_element.IsValid() && m_dscr.IsValid())
        m_element = m_dscr;
        
    if (!m_element.IsValid())
        {
        BeAssert (false);
        return ERROR;
        }

    StatusInt status = m_element->GetDgnDb().GetTxnManager().GetCurrentTxn().DeleteElement (m_element.get());
    if (SUCCESS == status)
        Invalidate();

    return  status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   06/05
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt EditElementHandle::ReplaceInModel()
    {
    if (!m_element.IsValid())
        {
        BeAssert(false);
        return ERROR;
        }
    return  m_element->GetDgnDb().GetTxnManager().GetCurrentTxn().ReplaceElement(*this);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   01/09
+---------------+---------------+---------------+---------------+---------------+------*/
void ElementHandle::Init (DgnElementId id, DgnDbR project)
    {
    Init (project.Elements().GetElementById(id).get());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      11/2008
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt EditElementHandle::FindById (DgnElementId elemID, DgnModelP model, bool allowDeleted)
    {
    if (nullptr == model)
        return ERROR;
    
    DgnElementPtr  element = model->GetDgnDb().Elements().GetElementById (elemID);
    if (!element.IsValid())
        return DGNMODEL_STATUS_ElementNotFound;

    if (!allowDeleted && element->IsDeleted())
        return DGNMODEL_STATUS_ElementNotFound;

    SetDgnElement (element.get());
    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Jeff.Marker     03/09
+---------------+---------------+---------------+---------------+---------------+------*/
DgnElementId ElementHandle::GetElementId () const
    {
    return m_element.IsValid() ? m_element->GetElementId() : 
             m_dscr.IsValid() ? m_dscr->GetElementId() : DgnElementId();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson      04/2015
+---------------+---------------+---------------+---------------+---------------+------*/
ElementItemKey ElementHandle::GetItemKey () const
    {
    GeometricElementCP geom = GetGeometricElement();
    return geom ? geom->GetItemKey() : ElementItemKey();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson      04/2015
+---------------+---------------+---------------+---------------+---------------+------*/
ElementItemHandlerP ElementHandle::GetItemHandler () const
    {
    GeometricElementCP geom = GetGeometricElement();
    return geom ? &geom->GetItemHandler() : nullptr;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson      04/2015
+---------------+---------------+---------------+---------------+---------------+------*/
DgnElementKey ElementHandle::GetElementKey() const
    {
    if (!IsValid())
        return DgnElementKey();

    return m_dscr.IsValid() ? m_dscr->GetElementKey() : m_element->GetElementKey();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void ElementHandle::Init (DgnElementCP elRef)
    {
    m_element = nullptr;
    m_dscr = nullptr;
    m_state = nullptr;

    AssignDgnElement (elRef);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void ElementHandle::Init (DgnElementCP elDscr, bool isPersistent)
    {
    m_dscr = (DgnElementP) elDscr;
    m_element = isPersistent ? m_dscr : nullptr;
    m_state = nullptr;

    BeAssert (!m_dscr.IsValid() || GetDgnModelP()); // DgnModel is required.
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

    return m_dscr.IsValid() ? m_dscr->GetElementHandler() : m_element->GetElementHandler();
    }
