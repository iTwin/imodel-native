/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnCore/ElemHandle.cpp $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include    <DgnPlatformInternal.h>

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   12/04
+---------------+---------------+---------------+---------------+---------------+------*/
ElementHandle::ElementHandle (DgnElementCR el, DgnModelR model)
    {
    Init (new MSElementDescr (el, model), false);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   10/11
+---------------+---------------+---------------+---------------+---------------+------*/
void ElementHandle::Clone(ElementHandle const& from)
    {
    if (this == &from)
        return;

    m_elmRef = from.m_elmRef;
    m_dscr   = from.m_dscr;
    m_state  = from.m_state;

    if (m_state)
        m_state->AddRef();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   02/14
+---------------+---------------+---------------+---------------+---------------+------*/
ElementHandle::ElementHandle (ElementHandleCR from) : m_elmRef(from.m_elmRef), m_dscr(from.m_dscr), m_state(from.m_state)
    {
    if (m_state)
        m_state->AddRef();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   10/04
+---------------+---------------+---------------+---------------+---------------+------*/
DgnElementCP     ElementHandle::GetUnstableMSElementCP() const
    {
    BeAssert (m_elmRef.IsValid());
    return m_elmRef->GetUnstableMSElementCP();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   08/07
+---------------+---------------+---------------+---------------+---------------+------*/
DgnModelP ElementHandle::GetDgnModelP() const
    {
    if (m_elmRef.IsValid ())
        return m_elmRef->GetDgnModelP ();

    return (m_dscr.IsValid () ? &m_dscr->GetDgnModel () : NULL);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   10/04
+---------------+---------------+---------------+---------------+---------------+------*/
MSElementDescrCP ElementHandle::GetElementDescrCP () const
    {
    if (!m_dscr.IsValid () && m_elmRef.IsValid ())
        m_dscr = m_elmRef->GetElementDescr ();

    return m_dscr.get ();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Keith.Bentley   11/03
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt EditElementHandle::ReplaceElement (DgnElementCP el)
    {
    if (NULL == GetElementDescrP() || NULL == el)
        return ERROR;

    m_dscr->ReplaceElement(*el); // (preserves XAttributes)
    ClearElementRef(); // clear old element ref...new element could be different type/handler...

    return m_dscr.IsValid() ? SUCCESS : ERROR;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Keith.Bentley   11/03
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt EditElementHandle::ReplaceElementDescr (MSElementDescrP newDescr)
    {
    if (newDescr == m_dscr.get())
        return SUCCESS;

    if (nullptr == newDescr)
        {
        BeAssert (0);
        return ERROR;
        }

    MSElementDescrCP oldDscr = GetElementDescrCP (); // make sure we have an MSElementDescr

    if (nullptr == oldDscr)
        {
        BeAssert (0);
        return ERROR;
        }

    // Even though the newDescr *could* have the XAttribute changes and clone source, the reality is that it often won't as it's
    // a diffiult thing to require of the handler implementor and to enforce. We can at least prevent changes from being lost on the 
    // hdr, but complex components are problematic. The call to preserve hdr XAttributes has been here for a long time, so it's 
    // really not an option to remove it now. The call to preserve the clone source is new for Vancouver...

    // Don't lose the scheduled XAttribute changes. NOTE: Will not overrwrite the XAttribute changes on destination.
    oldDscr->CopyXAttributesTo (*newDescr);

    AssignElemDescr (newDescr, false);

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Keith.Bentley   11/03
+---------------+---------------+---------------+---------------+---------------+------*/
void ElementHandle::AssignElemDescr (MSElementDescrP newDscr, bool isPersistent)
    {
    if (newDscr == m_dscr.get())
        return;

    if (newDscr)
        {
        AssignElementRef (isPersistent ? newDscr->GetElementRef() : nullptr);
        }
    else
        {
        ClearElementDescr();
        ClearElementRef();
        }

    m_dscr = newDscr;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      10/2008
+---------------+---------------+---------------+---------------+---------------+------*/
void EditElementHandle::Duplicate (ElementHandleCR eh)
    {
    MSElementDescrCP ed = eh.GetElementDescrCP ();

    if (NULL == ed)
        {
        AssignElemDescr (NULL, false);
        return;
        }

    MSElementDescrPtr cc = ed->Duplicate(true, true);

    if (PeekElementDescrCP())
        ReplaceElementDescr(cc.get());
    else
        SetElementDescr (cc.get(), eh.IsPersistent ());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  09/05
+---------------+---------------+---------------+---------------+---------------+------*/
void EditElementHandle::SetDgnModel (DgnModelR model)
    {
    BeAssert (m_dscr.IsValid () && "Only makes sense to change model when there is a MSElementDescr");

    if (!m_dscr.IsValid () || GetDgnModelP () == &model)
        return;

    m_dscr->SetDgnModel (model); // NOTE: Also clears m_dscr->m_elementRef...
    m_elmRef = NULL; // NOTE: Avoid an inconsistent state by clearing m_elRef...
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Keith.Bentley   11/03
+---------------+---------------+---------------+---------------+---------------+------*/
MSElementDescrPtr EditElementHandle::ExtractElementDescr ()
    {
    MSElementDescrPtr currDescr = m_dscr;
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

    if (duplicateDscr)
        m_dscr = m_dscr->Duplicate(true, true);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      11/2004
+---------------+---------------+---------------+---------------+---------------+------*/
bool ElementHandle::AnyXAttributeChanges () const
    {
    XAttributeChangeSetP set = QueryXAttributeChangeSet();
    return (NULL != set) && (set->Size() != 0);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      11/2004
+---------------+---------------+---------------+---------------+---------------+------*/
XAttributeChangeSetP ElementHandle::QueryXAttributeChangeSet() const
    {
    return m_dscr.IsValid() ? GetElementDescrCP()->QueryXAttributeChangeSet() : NULL;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      11/2004
+---------------+---------------+---------------+---------------+---------------+------*/
XAttributeChangeSetP ElementHandle::GetXAttributeChangeSet() const
    {
    return GetElementDescrCP()->GetXAttributeChangeSet ();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      11/2004
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt       EditElementHandle::ScheduleWriteXAttribute (XAttributeHandlerIdCR h, UInt32 xAttrId, size_t dataSize, void const* data)
    {
    XAttributeChangeSetP set = GetXAttributeChangeSet ();
    if (NULL == set)
        return  ERROR;

    //  Supercede prior scheduled write or deletion
    XAttributeChangeSet::T_Iterator it = set->Find (h,xAttrId);
    if (it != set->End())
        set->Cancel (it);

    SetNonPersistent (); // no longer exact image of cache element...

    return set->Schedule (h, xAttrId, dataSize, data, XAttributeChange::CHANGETYPE_Write);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      11/2004
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt       EditElementHandle::CancelWriteXAttribute (XAttributeHandlerIdCR h, UInt32 xAttrId)
    {
    XAttributeChangeSetP set = GetXAttributeChangeSet ();
    if (NULL == set)
        return  ERROR;

    XAttributeChangeSet::T_Iterator it = set->Find (h,xAttrId);
    if (it->GetChangeType() == XAttributeChange::CHANGETYPE_Write)
        return set->Cancel (it);
    return ERROR;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      11/2004
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt EditElementHandle::ScheduleDeleteXAttribute (XAttributeHandlerIdCR h, UInt32 xAttrId)
    {
    XAttributeChangeSetP set = GetXAttributeChangeSet ();
    if (NULL == set)
        return  ERROR;

    //  Supercede prior scheduled write or deletion
    XAttributeChangeSet::T_Iterator it = set->Find (h,xAttrId);
    if (it != set->End())
        set->Cancel (it);

    SetNonPersistent (); // no longer exact image of cache element...

    return set->Schedule (h, xAttrId, 0, NULL, XAttributeChange::CHANGETYPE_Delete);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      11/2004
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt EditElementHandle::CancelDeleteXAttribute (XAttributeHandlerIdCR h, UInt32 xAttrId)
    {
    XAttributeChangeSetP set = GetXAttributeChangeSet ();
    if (NULL == set)
        return  ERROR;

    XAttributeChangeSet::T_Iterator it = set->Find (h,xAttrId);
    if (it->GetChangeType() == XAttributeChange::CHANGETYPE_Delete)
        return set->Cancel (it);

    return ERROR;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   06/05
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt EditElementHandle::DeleteFromModel ()
    {
    if (!m_elmRef.IsValid() && m_dscr.IsValid())
        m_elmRef = m_dscr->GetElementRef();
        
    if (!m_elmRef.IsValid())
        {
        BeAssert (false);
        return ERROR;
        }

    StatusInt status = m_elmRef->GetDgnProject()->GetTxnManager().GetCurrentTxn().DeleteElement (m_elmRef.get());
    if (SUCCESS == status)
        Invalidate();

    return  status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   06/05
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt EditElementHandle::ReplaceInModel (ElementRefP replaceRef)
    {
    BeAssert (replaceRef);
    return  replaceRef->GetDgnProject()->GetTxnManager().GetCurrentTxn().ReplaceElement (*this, replaceRef);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      06/2005
+---------------+---------------+---------------+---------------+---------------+------*/
void            XAttributeChangeIter::Init (MSElementDescrCP descr)
    {
    if (NULL == descr)
        m_changeSet = NULL;
    else
        m_changeSet = descr->QueryXAttributeChangeSet ();

    ToFirst ();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Ray.Bentley                      11/2006
+---------------+---------------+---------------+---------------+---------------+------*/
XAttributeChangeIter::XAttributeChangeIter (XAttributeChangeSetP changeSet)
    {
    m_changeSet = changeSet;
    ToFirst ();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      07/2009
+---------------+---------------+---------------+---------------+---------------+------*/
bool XAttributeChangeIter::operator==(XAttributeChangeIter const& rhs) const
    {
    return m_changeSet == rhs.m_changeSet && m_changeSetIter == rhs.m_changeSetIter;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      07/2009
+---------------+---------------+---------------+---------------+---------------+------*/
bool XAttributeChangeIter::operator!=(XAttributeChangeIter const& rhs) const
    {
    return !(*this == rhs);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      07/2009
+---------------+---------------+---------------+---------------+---------------+------*/
XAttributeChangeIter& XAttributeChangeIter::operator++()
    {
    ToNext ();
    return *this;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      06/2005
+---------------+---------------+---------------+---------------+---------------+------*/
bool XAttributeChangeIter::Search (XAttributeHandlerIdCR handler, UInt32 id)
    {
    if (NULL == m_changeSet)
        return false;

    //  Special/common case: exact search from the beginning. Use std::set::find.
    if (id != XAttributeHandle::MATCH_ANY_ID && IsBeginning())
        {
        m_changeSetIter = m_changeSet->Find (handler, id);
        return IsValid();
        }

    //  Must use simple linear search, since XAttributeChangeSet does not support this concept directly.
    for ( ; IsValid(); ToNext())
        {
        XAttributeChangeCR xc = operator*();

        if (!handler.IsValid() || handler == xc.GetHandlerId())
            {
            if ( (id == XAttributeHandle::MATCH_ANY_ID) || (id == xc.GetId()) )
                return true;
            }
        }

    return false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      11/2004
+---------------+---------------+---------------+---------------+---------------+------*/
XAttributeChangeIter::XAttributeChangeIter (ElementHandleCR eh, XAttributeHandlerId xahid, UInt32 xAttrId)
    {
    Init (eh.PeekElementDescrCP());
    Search (xahid, xAttrId);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      11/2004
+---------------+---------------+---------------+---------------+---------------+------*/
bool XAttributeChangeIter::IsValid () const
    {
    if (NULL == m_changeSet)
        return false;

    return m_changeSetIter != m_changeSet->End ();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      06/2005
+---------------+---------------+---------------+---------------+---------------+------*/
bool XAttributeChangeIter::IsBeginning () const
    {
    if (NULL == m_changeSet)
        return false;

    return m_changeSetIter == m_changeSet->Begin();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      06/2005
+---------------+---------------+---------------+---------------+---------------+------*/
void XAttributeChangeIter::ToFirst ()
    {
    if (NULL != m_changeSet)
        m_changeSetIter = m_changeSet->Begin ();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      11/2004
+---------------+---------------+---------------+---------------+---------------+------*/
bool            XAttributeChangeIter::ToNext ()
    {
    if (NULL == m_changeSet)
        return false;

    if (m_changeSetIter == m_changeSet->End())
        return false;

    m_changeSetIter++;
    return m_changeSetIter != m_changeSet->End ();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      11/2004
+---------------+---------------+---------------+---------------+---------------+------*/
XAttributeChangeCR XAttributeChangeIter::operator *() const
    {
    if (NULL == m_changeSet || m_changeSetIter == m_changeSet->End())
        {
        BeAssert (false);
        static XAttributeChange s_nil (XAttributeChange::GetNilObject ());
        return s_nil;
        }
    return *m_changeSetIter;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      06/2005
+---------------+---------------+---------------+---------------+---------------+------*/
ElementHandle::XAttributeIter  ElementHandle::GetXAttributeIter() const
    {
    return XAttributeIter (*this);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      07/2009
+---------------+---------------+---------------+---------------+---------------+------*/
ElementHandle::XAttributeIter  ElementHandle::BeginXAttributes () const
    {
    return GetXAttributeIter ();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      07/2009
+---------------+---------------+---------------+---------------+---------------+------*/
ElementHandle::XAttributeIter  ElementHandle::EndXAttributes () const
    {
    XAttributeIter xi = GetXAttributeIter ();
    xi.SetEnd ();
    return xi;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      06/2005
+---------------+---------------+---------------+---------------+---------------+------*/
static bool    xaExistsNow (XAttributeChangeIterCR ci)
    { // count a deletion as "not found"
    return ci->GetChangeType() != XAttributeChange::CHANGETYPE_Delete
        && ci->GetChangeType() != XAttributeChange::CHANGETYPE_HistoricalVersionDidNotExist;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      06/2005
+---------------+---------------+---------------+---------------+---------------+------*/
ElementHandle::XAttributeIter::XAttributeIter (ElementHandleCR eh, XAttributeHandlerId handlerId, UInt32 id)
  : 
    m_id(id), m_ci(eh, handlerId, id),
    m_collection((eh.GetElementRef()? eh.GetElementRef(): eh.PeekElementDescrCP()? eh.GetElementDescrCP()->GetElementRef(): NULL), handlerId),
    m_state(STATE_StartChanges)
    {
    ToNext();   // Find first valid position of any kind
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      01/08
+---------------+---------------+---------------+---------------+---------------+------*/
XAttributeCollection::Entry const* ElementHandle::XAttributeIter::GetElementXAttributeIter () const
    {
    return (STATE_InPersistent == m_state)? &m_entry: NULL;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      06/2005
+---------------+---------------+---------------+---------------+---------------+------*/
static bool    isFoundInXaChanges (XAttributeChangeIterCR ci, XAttributeCollection::Entry const& xi)
    {
    XAttributeChangeSetP set = ci.GetXAttributeChangeSet();
    return (NULL == set) ? false : set->Find (xi.GetHandlerId(), xi.GetId()) != set->End();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      06/2005
+---------------+---------------+---------------+---------------+---------------+------*/
bool            ElementHandle::XAttributeIter::ToNext()
    {
    switch (m_state)
        {
        case STATE_StartChanges:

            m_state = STATE_InChanges;      // either way, the NEXT state is STATE_InChanges

            // See if the very first change results in a viable XA
            if (m_ci.IsValid() && xaExistsNow(m_ci))
                return true;                // Yes! First XA change identifies a viable XA

            //  First change is not viable

            // VVV fall thru VVV

        case STATE_InChanges:

            // Find the next change that results in a viable XA
            while (m_ci.ToNext() && m_ci.Search(m_collection.GetSearchHandlerId()) && !xaExistsNow(m_ci))
                ;

            if (m_ci.IsValid())
                return true;                // Yes! Next XA change identifies a viable XA

            //  No more changes yielding viable XAs

        // STATE_StartPersistent:
            if (m_id == XAttributeHandle::MATCH_ANY_ID)
                m_entry = m_collection.begin();
            else
                m_entry = XAttributeCollection::Entry(m_collection.GetElementRef(), m_collection.GetSearchHandlerId(), m_id);

            m_state = STATE_InPersistent;   // either way, the NEXT state is InPeristent

            //  See if the first persistent XA (if any) is one that we haven't already seen in the changes section.
            if (m_entry.IsValid() && !isFoundInXaChanges(m_ci, m_entry))
                return true;                // Yes! First persistent XA is a new discovery

            //  No persistent XAs or we've already seen the first one.

            // VVV fall thru VVV

        case STATE_InPersistent:

            //  Find the next persistent XA that we haven't already seen in changes section
            while ((++m_entry).IsValid() && isFoundInXaChanges(m_ci, m_entry))
                ;

            if (m_entry.IsValid())
                return true;                // Yes! Next persistent XA is a new discovery

            //  No more persistent XAs that we haven't already seen.

            m_state = STATE_NotFound;       // end of the line
        }

    //  We are stuck at the end of the iteration
    return false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      07/2009
+---------------+---------------+---------------+---------------+---------------+------*/
ElementHandle::XAttributeIter& ElementHandle::XAttributeIter::operator++()
    {
    ToNext ();
    return *this;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      07/2009
+---------------+---------------+---------------+---------------+---------------+------*/
bool ElementHandle::XAttributeIter::operator==(XAttributeIter const& rhs) const
    {
    if (m_state != rhs.m_state)
        return false;

    switch (m_state)
        {
        case STATE_StartChanges:
            return true;

        case STATE_InChanges:
            return m_ci == rhs.m_ci;

        case STATE_InPersistent:
            return m_entry == rhs.m_entry;
        }

    BeAssert (m_state == STATE_NotFound);
    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      06/2005
+---------------+---------------+---------------+---------------+---------------+------*/
bool            ElementHandle::XAttributeIter::IsValid () const
    {
    return STATE_NotFound != m_state;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      06/2005
+---------------+---------------+---------------+---------------+---------------+------*/
UInt32          ElementHandle::XAttributeIter::GetId() const
    {
    switch (m_state)
        {
        case STATE_InChanges:    return m_ci->GetId();
        case STATE_InPersistent: return m_entry.GetId();
        }
    return XAttributeHandle::MATCH_ANY_ID;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      06/2005
+---------------+---------------+---------------+---------------+---------------+------*/
XAttributeHandlerId ElementHandle::XAttributeIter::GetHandlerId() const
    {
    switch (m_state)
        {
        case STATE_InChanges:    return m_ci->GetHandlerId();
        case STATE_InPersistent: return m_entry.GetHandlerId();
        }
    return XAttributeHandlerId (0, 0);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      06/2005
+---------------+---------------+---------------+---------------+---------------+------*/
UInt32          ElementHandle::XAttributeIter::GetSize() const
    {
    switch (m_state)
        {
        case STATE_InChanges:    return m_ci->GetSize();
        case STATE_InPersistent: return m_entry.GetSize();
        }
    return 0;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      06/2005
+---------------+---------------+---------------+---------------+---------------+------*/
void const*     ElementHandle::XAttributeIter::PeekData() const
    {
    switch (m_state)
        {
        case STATE_InChanges:    return m_ci->PeekData();
        case STATE_InPersistent: return m_entry.PeekData();
        }
    return NULL;
    }

/*---------------------------------------------------------------------------------**//**
* The caller KNOWS that this is a graphics element and that it is not dynamic range.
* @bsimethod                                    Sam.Wilson                      04/2008
+---------------+---------------+---------------+---------------+---------------+------*/
DRange3dCP ElementHandle::GetIndexRange () const
    {
    ElementRefP ref = GetElementRef();
    if (NULL != ref && ELEMENT_REF_TYPE_Persistent == ref->GetRefType() && IsPersistent())
        return &((PersistentElementRefP)ref)->GetIndexRange ();

    return &GetElementCP()->GetRange();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   01/09
+---------------+---------------+---------------+---------------+---------------+------*/
void ElementHandle::Init (ElementId id, DgnProjectR project)
    {
    Init (project.Models().GetElementById(id).get());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      11/2008
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt EditElementHandle::FindById (ElementId elemID, DgnModelP model, bool allowDeleted)
    {
    if (NULL == model)
        return ERROR;
    
    PersistentElementRefPtr  elemRef = model->GetDgnProject().Models().GetElementById (elemID);
    if (!elemRef.IsValid())
        return DGNMODEL_STATUS_ElementNotFound;

    if (!allowDeleted && elemRef->IsDeleted())
        return DGNMODEL_STATUS_ElementNotFound;

    SetElementRef (elemRef.get());
    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      02/2009
+---------------+---------------+---------------+---------------+---------------+------*/
bool ConstElementLinkageIterator::ToNextFromValid ()
    {
    BeAssert (IsValid ());

    bool    isValid;
    do  {
        UShort linkWords = LinkageUtil::GetWords (m_thisLinkage);
        if (0 == linkWords)
            {
            m_thisLinkage = m_end;
            return false; // corrupt linkage => we cannot iterate!
            }
        m_thisLinkage = (LinkageHeader*) ((UShort*) m_thisLinkage + linkWords);
        }
    while ((isValid = IsValid()) && !IsRequestedLinkage());

    BeAssert (!IsValid() || IsRequestedLinkage());

    BeAssert (IsValid() == isValid);

    // We should never have stepped past the end of the element, but there are certain corrupt elements where that happens.
    // In that case, we want to set this iterator to equal the "end" iterator, rather than beyond it, because the check is for equality.
    if (m_thisLinkage > m_end)
        {
        BeDataAssert (false);
        m_thisLinkage = m_end;
        }

    return isValid;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      02/2009
+---------------+---------------+---------------+---------------+---------------+------*/
ConstElementLinkageIterator::ConstElementLinkageIterator (ElementHandleCR eh, IntialPos pos, UInt16 rl)
    : 
    m_requestedLinkage (rl)
    {
    if (pos == POS_ATEND)
        {
        SetEnd (eh.GetElementCP());
        m_thisLinkage = m_end;
        return;
        }

    DgnElementCP elm = eh.GetElementCP();
    BeAssert (NULL != elm);

    m_thisLinkage = (LinkageHeader*) ((UShort*) elm + elm->GetAttributeOffset());
    
    SetEnd (elm);

    BeAssert (m_end >= m_thisLinkage && "first attrOffset must be <= elementSize");

    if (IsValid() && !IsRequestedLinkage ())
        ToNextFromValid ();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      02/2009
+---------------+---------------+---------------+---------------+---------------+------*/
ConstElementLinkageIterator::ConstElementLinkageIterator (ElementHandleCR eh, byte* lstart, UInt16 rl) 
    :
    m_requestedLinkage (rl)
    {
    m_thisLinkage = (LinkageHeader*) lstart;
    SetEnd (eh.GetElementCP());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      02/2009
+---------------+---------------+---------------+---------------+---------------+------*/
void    ConstElementLinkageIterator::SetEnd (DgnElementCP elm)
    {
    m_end = (LinkageHeader*) ((UShort*) elm + elm->GetSizeWords());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      02/2009
+---------------+---------------+---------------+---------------+---------------+------*/
bool ConstElementLinkageIterator::IsRequestedLinkage () const
    {
    BeAssert (IsValid ());
    if (0 == m_requestedLinkage)
        return true;
    return m_thisLinkage->user && (m_requestedLinkage == m_thisLinkage->primaryID);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      02/2009
+---------------+---------------+---------------+---------------+---------------+------*/
bool ConstElementLinkageIterator::IsValid () const
    {
    return m_thisLinkage < m_end;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      02/2009
+---------------+---------------+---------------+---------------+---------------+------*/
bool ConstElementLinkageIterator::ToNext ()
    {
    if (!IsValid())
        return false;
    return ToNextFromValid ();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      02/2009
+---------------+---------------+---------------+---------------+---------------+------*/
ElementLinkageIterator::ElementLinkageIterator (EditElementHandleR eh, IntialPos pos, UInt16 rl) 
    :
    ConstElementLinkageIterator (eh, pos, rl)
    {
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      02/2009
+---------------+---------------+---------------+---------------+---------------+------*/
ElementLinkageIterator::ElementLinkageIterator (EditElementHandleR eh, byte* lstart, UInt16 rl) 
    :
    ConstElementLinkageIterator (eh, lstart, rl)
    {
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      02/2009
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt ElementLinkageIterator::CheckElementContainsThisLinkage (EditElementHandleR eh)
    {
    DgnElementCP el = eh.GetElementCP ();
    if ((UShort*)m_thisLinkage <  ((UShort*)el + el->GetAttributeOffset())
     || (UShort*)m_thisLinkage >= ((UShort*)el + el->GetSizeWords()))
        {
        BeAssert (false);
        return ERROR;
        }
    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      02/2009
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt ElementLinkageIterator::ReplaceLinkage (EditElementHandleR eh, LinkageHeader const& newLinkageHeader, void const* newLinkageData)
    {
    eh.GetElementDescrP ();

    if (!eh.m_dscr.IsValid ())
        return ERROR;
    
    if (CheckElementContainsThisLinkage (eh) != SUCCESS)
        return ERROR;

    if (eh.m_dscr->ReplaceLinkage((LinkageHeader**)&m_thisLinkage, newLinkageHeader, newLinkageData) != BSISUCCESS)
        return ERROR;

    eh.SetNonPersistent (); // no longer exact image of cache element...

    SetEnd (eh.GetElementCP()); // recompute the address beyond all linkages, in case elmdscr was reallocated

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      02/2009
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt ElementLinkageIterator::RemoveLinkage (EditElementHandleR eh)
    {
    if (CheckElementContainsThisLinkage (eh) != SUCCESS)
        return ERROR;

    UShort* linkStart = (UShort*)m_thisLinkage;
    size_t linkWords  = LinkageUtil::GetWords (m_thisLinkage);
    UShort* linkEnd   = linkStart + linkWords;
    UShort* pwEnd     = (UShort*)m_end;

    if (linkEnd > pwEnd)
        {   // in case size of linkage is bad
        linkEnd = pwEnd;
        linkWords = linkEnd - linkStart;
        }
    memmove (linkStart, linkEnd, (pwEnd - linkEnd) * 2);

    eh.GetElementP()->SetSizeWords(eh.GetElementP()->GetSizeWords() - static_cast<UInt32>(linkWords));

    eh.SetNonPersistent (); // no longer exact image of cache element...

    //  Leave this iterator pointing at what is now the next linkage
    SetEnd (eh.GetElementCP()); // recompute the address beyond all linkages, in case elmdscr was reallocated

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      02/2009
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt EditElementHandle::InsertElementLinkage (ElementLinkageIterator* newLinkageIt, LinkageHeaderCR newLinkageHeader, void const* newLinkageData, ElementLinkageIterator& wh, UInt16 rl)
    {
    bool atEnd = wh == EndElementLinkages();

    if (!atEnd && wh.CheckElementContainsThisLinkage (*this) != SUCCESS)
        return ERROR;

    GetElementDescrP ();

    if (!m_dscr.IsValid ())
        return ERROR;

    BentleyStatus status;
    size_t linkageOffset;
    if (atEnd)
        {
        linkageOffset = 2*GetElementP()->GetSizeWords();
        status = m_dscr->AppendLinkage(newLinkageHeader, newLinkageData);
        }
    else
        {
        linkageOffset = (byte*)wh.m_thisLinkage - (byte*)GetElementP();
        status = m_dscr->InsertLinkage(linkageOffset, newLinkageHeader, newLinkageData);
        }

    if (BSISUCCESS != status)
        return ERROR;
    
    SetNonPersistent (); // no longer exact image of cache element...

    //  Compute and optionally return iterator that points to the start of the new linkage
    if (NULL == newLinkageIt)
        newLinkageIt = (ElementLinkageIterator*)_alloca (sizeof (ElementLinkageIterator));

    *newLinkageIt = ElementLinkageIterator (*this, (byte*)GetElementP() + linkageOffset, rl);

    //  Update input iterator to point to next linkage beyond new one
    wh = ElementLinkageIterator (*this, (byte*)newLinkageIt->m_thisLinkage + 2*LinkageUtil::GetWords(&newLinkageHeader), wh.m_requestedLinkage);

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Jeff.Marker     04/09
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt EditElementHandle::AppendElementLinkage (ElementLinkageIterator* newLinkageIt, LinkageHeaderCR newLinkageHeader, void const* newLinkageData)
    {
    ElementLinkageIterator atEnd (EndElementLinkages ());
    return InsertElementLinkage (newLinkageIt, newLinkageHeader, newLinkageData, atEnd, 0);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      02/2009
+---------------+---------------+---------------+---------------+---------------+------*/
ConstElementLinkageIterator ElementHandle::BeginElementLinkages (UInt16 rl) const
    {
    return ConstElementLinkageIterator (*this, ConstElementLinkageIterator::POS_ATBEGIN, rl);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      02/2009
+---------------+---------------+---------------+---------------+---------------+------*/
ConstElementLinkageIterator ElementHandle::EndElementLinkages () const
    {
    return ConstElementLinkageIterator (*this, ConstElementLinkageIterator::POS_ATEND, 0);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Jeff.Marker     03/09
+---------------+---------------+---------------+---------------+---------------+------*/
ElementId ElementHandle::GetElementId () const
    {
    return GetElementCP()->GetElementId();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      02/2009
+---------------+---------------+---------------+---------------+---------------+------*/
ElementLinkageIterator EditElementHandle::BeginElementLinkages (UInt16 rl)
    {
    return ElementLinkageIterator (*this, ElementLinkageIterator::POS_ATBEGIN, rl);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      02/2009
+---------------+---------------+---------------+---------------+---------------+------*/
ElementLinkageIterator EditElementHandle::EndElementLinkages ()
    {
    return ElementLinkageIterator (*this, ElementLinkageIterator::POS_ATEND, 0);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      02/2009
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt EditElementHandle::ReplaceElementLinkage (ElementLinkageIteratorR it, LinkageHeader const& newLinkageHeader, void const* newLinkageData)
    {
    return it.ReplaceLinkage (*this, newLinkageHeader, newLinkageData);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      02/2009
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt EditElementHandle::RemoveElementLinkage (ElementLinkageIteratorR it)
    {
    return it.RemoveLinkage (*this);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void ElementHandle::Init (ElementRefP elRef)
    {
    m_elmRef = NULL;
    m_dscr = NULL;
    m_state = NULL;

    AssignElementRef (elRef);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void ElementHandle::Init (MSElementDescrCP elDscr, bool isPersistent)
    {
    m_dscr = (MSElementDescrP) elDscr;
    m_elmRef = (isPersistent && m_dscr.IsValid()) ? m_dscr->GetElementRef() : NULL;
    m_state = NULL;

    BeAssert (!m_dscr.IsValid() || GetDgnModelP()); // DgnModel is required.
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      08/2010
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt EditElementHandle::AddToModel()
    {
    DgnProjectP project = GetDgnProject();

    if (nullptr == project)
        return ERROR;

    return project->GetTxnManager().GetCurrentTxn().AddElement(*this);
    }
