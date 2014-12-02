/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnCore/MSElementDescr.cpp $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include    <DgnPlatformInternal.h>

static size_t s_nDscrExtant;
static size_t s_nDscrTotal;

size_t MSElementDescr::DebugGetExtantCount() {return s_nDscrExtant;}
size_t MSElementDescr::DebugGetTotalCount()  {return s_nDscrTotal;}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      11/2004
+---------------+---------------+---------------+---------------+---------------+------*/
XAttributeChangeSet* MSElementDescr::GetXAttributeChangeSet() const
    {
    if (NULL == m_attributes)
        m_attributes = new XAttributeChangeSet ();

    return m_attributes;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      11/2004
+---------------+---------------+---------------+---------------+---------------+------*/
void MSElementDescr::AppendXAttributeChangeSet(XAttributeChangeSet const& cs)
    {
    if ((cs.Size() == 0) || (NULL == GetXAttributeChangeSet()))
        return;

    if (NULL == GetXAttributeChangeSet())
        return;

    m_attributes->CopySchedule (cs); // append cs
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      11/2004
+---------------+---------------+---------------+---------------+---------------+------*/
void MSElementDescr::DonateXAttributeChangeSetTo (MSElementDescr* to)
    {
    to->m_attributes = m_attributes;
    m_attributes = NULL;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      03/2006
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus MSElementDescr::CopyXAttributesTo (MSElementDescrR sink) const
    {
    //  Copy over recently scheduled XAttributes, if any
    XAttributeChangeSetP sourceSet = QueryXAttributeChangeSet ();
    if (NULL != sourceSet)
        {
        XAttributeChangeSetP sinkSet = sink.GetXAttributeChangeSet ();
        if (NULL == sinkSet) // sink must be freed descr, or memory is exhausted
            return ERROR;

        sinkSet->CopySchedule (*sourceSet);
        }

    //  Load source persistent XAttributes into sink's change set
    if (SUCCESS != XAttributeChangeSet::CopyPersistentXas (&sink, this))
        return ERROR;

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   01/08
+---------------+---------------+---------------+---------------+---------------+------*/
MSElementDescrPtr ElementRef::GetElementDescr (bool allowDeleted)
    {
    if (!allowDeleted && IsDeleted ())
        return NULL;

    return new MSElementDescr (*this);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   07/13
+---------------+---------------+---------------+---------------+---------------+------*/
void MSElementDescr::GetHeaderFieldsFrom(ElementRefCR elRef)
    {
    m_handler    = elRef.GetHandler();
    BeAssert (NULL != m_handler);
    m_itemId = elRef.GetItemId();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   02/14
+---------------+---------------+---------------+---------------+---------------+------*/
void MSElementDescr::Init (DgnElementCR element, ElementRefP elRef, DgnModelR model)
    {
    m_attributes = NULL; 
    m_allocBytes = element.Size();
    m_el = (DgnElementP) malloc(m_allocBytes);
    memcpy (m_el, &element, m_allocBytes);
    m_dgnModel = &model;
    m_elementRef = elRef;
    m_handler = NULL;

    ++s_nDscrExtant;     // for debugging leaks
    ++s_nDscrTotal;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   02/14
+---------------+---------------+---------------+---------------+---------------+------*/
MSElementDescr::MSElementDescr (DgnElementCR element, DgnModelR model)
    {
    Init(element, NULL, model);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   02/14
+---------------+---------------+---------------+---------------+---------------+------*/
MSElementDescr::MSElementDescr (ElementRefR elRef)
    {
    Init(*elRef.GetUnstableMSElementCP(), &elRef, *elRef.GetDgnModelP ());
    GetHeaderFieldsFrom(elRef);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   02/14
+---------------+---------------+---------------+---------------+---------------+------*/
MSElementDescr::~MSElementDescr()
    {
    FREE_AND_CLEAR (m_el);
    DELETE_AND_CLEAR (m_attributes);

    m_allocBytes = 0;
#ifndef NDEBUG
    --s_nDscrExtant;
#endif
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   02/14
+---------------+---------------+---------------+---------------+---------------+------*/
void MSElementDescr::ReserveMemory(size_t newSize, bool preserveData)
    {
    if (m_allocBytes >= newSize) 
        return;                  // we already have enough memory

    m_allocBytes = newSize;
    if (preserveData && NULL != m_el) 
        {
        m_el = (DgnElementP) realloc(m_el, newSize);
        return;
        }

    free(m_el);
    m_el = (DgnElementP) malloc (newSize);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Keith.Bentley   11/90
+---------------+---------------+---------------+---------------+---------------+------*/
void MSElementDescr::ClearElementId()
    {
    m_el->InvalidateElementId();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  09/12
+---------------+---------------+---------------+---------------+---------------+------*/
void MSElementDescr::ClearElementRef()
    {
    SetElementRef(NULL);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  11/12
+---------------+---------------+---------------+---------------+---------------+------*/
void MSElementDescr::SetDgnModel (DgnModelR model)
    {
    m_dgnModel = &model;
    m_elementRef = NULL; // NOTE: Avoid an inconsistent state by clearing m_elementRef...
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Keith.Bentley   11/90
+---------------+---------------+---------------+---------------+---------------+------*/
static MSElementDescrP copyElemDescr (MSElementDescrP header, MSElementDescrCP oldDscr, bool copyScheduledXaChanges)
    {
    MSElementDescrP newDscr = new MSElementDescr(oldDscr->Element(), oldDscr->GetDgnModel());

    /* copy any user data from old to new */
    newDscr->SetElementRef(oldDscr->GetElementRef());
    newDscr->SetItemId(oldDscr->GetItemId());
    newDscr->SetElementHandler(oldDscr->GetElementHandler());

    if (copyScheduledXaChanges)
        {
        XAttributeChangeSet* set = oldDscr->QueryXAttributeChangeSet();
        if (NULL != set)
            newDscr->AppendXAttributeChangeSet(*set);
        }

    return newDscr;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Brien.Bastings  09/12
+---------------+---------------+---------------+---------------+---------------+------*/
MSElementDescrPtr MSElementDescr::Duplicate(bool copyScheduledXaChanges, bool loadPersistentXasAsChanges) const
    {
    MSElementDescrPtr newDscr = copyElemDescr (NULL, this, copyScheduledXaChanges);
    if (!loadPersistentXasAsChanges)
        return newDscr;

    // Note: if a given XA was copied to newDscr as a change, then LoadXasAsWrites will ignore that XA if it encounters it in the persistent set of XAs. That is, the copy of the change wins.
    return SUCCESS == XAttributeChangeSet::LoadXasAsWrites(newDscr.get()) ? newDscr : NULL;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Keith.Bentley   11/90
+---------------+---------------+---------------+---------------+---------------+------*/
void MSElementDescr::ReplaceElement (DgnElementCR element) 
    {
    size_t newSize = element.Size();

    ReserveMemory (newSize, false);
    memcpy (m_el, &element, newSize);
    }
