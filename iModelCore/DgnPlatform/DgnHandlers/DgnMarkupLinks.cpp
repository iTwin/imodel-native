/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnHandlers/DgnMarkupLinks.cpp $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "DgnPlatformInternal.h"
#include <DgnPlatform/DgnHandlers/DgnMarkupLinks.h>
#include <DgnPlatform/DgnCore/DgnResourceURI.h>

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Sam.Wilson      06/13
//--------------+------------------------------------------------------------------------
BentleyStatus DgnMarkupLinks::CreateRedlineLink (RedlineModelR rdlModel, PersistentElementRefR ref)
    {
    if (ref.GetHandler() == nullptr)
        {
        BeAssert (false && "How can a persistent element ref not have a file or handler??");
        return ERROR;
        }

    DgnResourceURI uri;
    if (ref.GetHandler()->CreateDgnResourceURI (uri, ref) != SUCCESS)
        return ERROR;

    uri.SetTargetFile (*ref.GetDgnProject());  // we want the URI to include the scheme and target file identifier, so that we can recognize it in the dgnlinks table.

    PersistentElementRefR anchorInRedlineModel = GetRedlineLinkSource (rdlModel);
    Utf8String uriString = uri.ToEncodedString ();
    Utf8String linkDesc = "***RedlineLink***"; // *** WIP_URI - This link is on an element that the user will never see, so the link should never show up in any view or GUI.

    rdlModel.GetDgnMarkupProject()->DgnLinks().AttachUrlLink (anchorInRedlineModel.GetElementId(), linkDesc.c_str(), uriString.c_str(), nullptr);
    return SUCCESS;
    }

#if defined (NEEDS_WORK_DGNITEM)
//---------------------------------------------------------------------------------------
// @bsimethod                                                   Sam.Wilson      06/13
//--------------+------------------------------------------------------------------------
PersistentElementRefR DgnMarkupLinks::GetRedlineLinkSource (RedlineModelR rdlModel)
    {
    rdlModel.FillModel();
    for (PersistentElementRefP ref : *rdlModel.GetControlElementsP())
        {
        if (ref->GetHandler() == &ExtendedNonGraphicsHandler::GetInstance())
            return *ref;
        }

    EditElementHandle eeh;  
    ExtendedNonGraphicsHandler::InitializeElement (eeh, nullptr, rdlModel);
    XAttributeHandlerId hid (XATTRIBUTEID_Markup,0);
    eeh.ScheduleWriteXAttribute (hid, 0, 4, "URL");
    eeh.AddToModel();
    return *(PersistentElementRefP)eeh.GetElementRef();
    }
#endif

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Sam.Wilson      06/13
//--------------+------------------------------------------------------------------------
DgnMarkupLinkIterator::DgnMarkupLinkIterator (DgnMarkupProjectR m, DgnProjectR t, DgnLinkTable::EntryFactory const& begin, DgnLinkTable::EntryFactory const& end)
    :
    m_markupProject(m),
    m_targetProject(t),
    m_dgnLinkTableIterator(begin),
    m_dgnLinkTableIteratorEnd(end)
    {
    ToFirst();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Sam.Wilson      06/13
//--------------+------------------------------------------------------------------------
DgnMarkupLinkIterator&  DgnMarkupLinkIterator::operator++()
    {
    ToNext();
    return *this;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Sam.Wilson      06/13
//--------------+------------------------------------------------------------------------
DgnModelId DgnMarkupLinkIterator::GetRedlineModelId() const
    {
    return m_redlineModel;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Sam.Wilson      06/13
//--------------+------------------------------------------------------------------------
ElementId DgnMarkupLinkIterator::GetTargetElementId() const
    {
    return m_targetElement;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Sam.Wilson      06/13
//--------------+------------------------------------------------------------------------
Utf8StringCR DgnMarkupLinkIterator::GetUriString() const
    {
    return m_uriString;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Sam.Wilson      06/13
//--------------+------------------------------------------------------------------------
bool  DgnMarkupLinkIterator::operator== (DgnMarkupLinkIterator const& rhs) const
    {
    return m_dgnLinkTableIterator == rhs.m_dgnLinkTableIterator;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Sam.Wilson      06/13
//--------------+------------------------------------------------------------------------
void DgnMarkupLinkIterator::ToFirst()
    {
    while (CaptureIfValid() != SUCCESS)
        ++m_dgnLinkTableIterator;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Sam.Wilson      06/13
//--------------+------------------------------------------------------------------------
void DgnMarkupLinkIterator::ToNext()
    {
    if (!IsAtEnd())
        {
        do  {
            ++m_dgnLinkTableIterator;
            } 
        while (CaptureIfValid() != SUCCESS);
        }
    }    

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Sam.Wilson      06/13
//--------------+------------------------------------------------------------------------
bool DgnMarkupLinkIterator::IsAtEnd() const {return m_dgnLinkTableIterator == m_dgnLinkTableIteratorEnd;}

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Sam.Wilson      06/13
//--------------+------------------------------------------------------------------------
BentleyStatus DgnMarkupLinkIterator::CaptureIfValid()
    {
    if (IsAtEnd())
        {
        m_redlineModel.Invalidate();
        m_targetElement.Invalidate();
        m_uriString.clear();
        return SUCCESS;
        }

    if (m_dgnLinkTableIterator.GetLinkType() != DgnLinkType::Url)
        return ERROR;

    DgnLinkTable::DgnLinkEntryPtr entry = m_dgnLinkTableIterator.CreateEntry();
        
    //  The DgnLink is associated with an element in the redline model
    PersistentElementRefPtr redlineElement = m_markupProject.Models().GetElementById(entry->GetElementId());
    if (redlineElement == nullptr)
        return ERROR;

    DgnModelP model = redlineElement->GetDgnModelP();
    if (model->GetModelType() != DgnModelType::Redline)
        return ERROR;

    m_redlineModel = model->GetModelId();

    //  The DgnLink holds a DgnResourceURI
    m_uriString = ((DgnLinkTable::UrlDgnLinkEntry*)entry.get())->GetUrl();

    DgnResourceURI uri;
    if (uri.FromEncodedString (m_uriString.c_str()) != DgnResourceURI::PARSE_STATUS_SUCCESS)
        return ERROR;

    //  The DgnResourceURI identifies an element in the target project.
    m_targetElement = m_targetProject.Models().GetElementByURI (uri);
    if (!m_targetElement.IsValid())
        return ERROR;

    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Sam.Wilson      06/13
//--------------+------------------------------------------------------------------------
DgnMarkupLinksCollection::DgnMarkupLinksCollection (DgnProject& targetProject, DgnMarkupProject& markupProject)
    :
    m_targetProject (targetProject),
    m_markupProject (markupProject),
    m_allDgnLinksInProject (markupProject)
    {
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Sam.Wilson      06/13
//--------------+------------------------------------------------------------------------
DgnMarkupLinkIterator DgnMarkupLinksCollection::begin() const
    {
    return DgnMarkupLinkIterator (m_markupProject, m_targetProject, m_allDgnLinksInProject.begin(), m_allDgnLinksInProject.end());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Sam.Wilson      06/13
//--------------+------------------------------------------------------------------------
DgnMarkupLinkIterator DgnMarkupLinksCollection::end() const
    {
    return DgnMarkupLinkIterator (m_markupProject, m_targetProject, m_allDgnLinksInProject.end(), m_allDgnLinksInProject.end());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Sam.Wilson      06/13
//--------------+------------------------------------------------------------------------
DgnMarkupLinksForRedlineModelCollection::DgnMarkupLinksForRedlineModelCollection (DgnProject& targetProject, DgnMarkupProject& markupProject, PersistentElementRefR anchor)
    :
    m_targetProject (targetProject),
    m_markupProject (markupProject),
    m_linksOnElementIterator (*anchor.GetDgnProject(), anchor.GetElementId())
    {
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Sam.Wilson      06/13
//--------------+------------------------------------------------------------------------
DgnMarkupLinkIterator DgnMarkupLinksForRedlineModelCollection::begin() const
    {
    return DgnMarkupLinkIterator (m_markupProject, m_targetProject, m_linksOnElementIterator.begin(), m_linksOnElementIterator.end());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Sam.Wilson      06/13
//--------------+------------------------------------------------------------------------
DgnMarkupLinkIterator DgnMarkupLinksForRedlineModelCollection::end() const
    {
    return DgnMarkupLinkIterator (m_markupProject, m_targetProject, m_linksOnElementIterator.end(), m_linksOnElementIterator.end());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Sam.Wilson      06/13
//--------------+------------------------------------------------------------------------
DgnMarkupLinksCollection DgnMarkupLinks::MakeIterator (DgnProject& targetProject, DgnMarkupProject& markupProject)
    {
    return DgnMarkupLinksCollection (targetProject, markupProject);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Sam.Wilson      06/13
//--------------+------------------------------------------------------------------------
DgnMarkupLinksForRedlineModelCollection DgnMarkupLinks::MakeIteratorForRedlineModel (DgnProject& targetProject, RedlineModelR rdlModel)
    {
    return DgnMarkupLinksForRedlineModelCollection (targetProject, *rdlModel.GetDgnMarkupProject(), GetRedlineLinkSource(rdlModel));
    }
