/*--------------------------------------------------------------------------------------+
|     $Source: PublicAPI/DgnPlatform/DgnHandlers/DgnMarkupLinks.h $
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
+--------------------------------------------------------------------------------------*/

//__PUBLISH_SECTION_START__

#pragma once

#include <DgnPlatform/DgnCore/DgnMarkupProject.h>

BEGIN_BENTLEY_DGNPLATFORM_NAMESPACE

struct DgnMarkupLinks;
struct DgnMarkupLinksCollection;
struct DgnMarkupLinksForRedlineModelCollection;

#if defined (NEEDS_WORK_DGNITEM)
//=======================================================================================
//! An iterator over a collection of redline links.
//=======================================================================================
struct DgnMarkupLinkIterator : std::iterator<std::input_iterator_tag, DgnMarkupLinkIterator const>
    {
  private:
    friend struct DgnMarkupLinksCollection;
    friend struct DgnMarkupLinksForRedlineModelCollection;
    DgnMarkupProjectR   m_markupProject;
    DgnDbR         m_targetProject;
    DgnLinkTable::EntryFactory m_dgnLinkTableIteratorEnd;
    DgnLinkTable::EntryFactory m_dgnLinkTableIterator;
    DgnModelId          m_redlineModel;
    ElementId           m_targetElement;
    Utf8String          m_uriString;

    bool                IsAtEnd() const;
    BentleyStatus       CaptureIfValid();
    void                ToFirst();
    void                ToNext();

    DgnMarkupLinkIterator (DgnMarkupProjectR, DgnDbR, DgnLinkTable::EntryFactory const& begin, DgnLinkTable::EntryFactory const& end);

  public:
    //! Advance to the next entry in the collection, if any.
    DGNPLATFORM_EXPORT DgnMarkupLinkIterator& operator++();
    //! Is this position in the collection equal to \a rhs?
    DGNPLATFORM_EXPORT bool operator==(DgnMarkupLinkIterator const& rhs) const;
    //! Is this position in the collection not equal to \a rhs?
    bool operator!=(DgnMarkupLinkIterator const& rhs) const {return !(*this == rhs);}
    //! Get the value at this position in the collection. Rarely needed. See GetRedlineModelId, GetTargetElementId.
    //! @private
    DgnMarkupLinkIterator const& operator* () const {return *this;}
    //! Get a pointer to the value at this position in the collection. Rarely needed. See GetRedlineModelId, GetTargetElementId.
    //! @private
    DgnMarkupLinkIterator const* operator->() const {return this;}
    //! Get the RedlineModel in the DgnMarkupProject that is the source of this link.
    DGNPLATFORM_EXPORT DgnModelId GetRedlineModelId() const;
    //! Get the element in the DgnDb that is the target of this link.
    DGNPLATFORM_EXPORT ElementId GetTargetElementId() const;
    //! Get the URI that identifies the target of this link
    DGNPLATFORM_EXPORT Utf8StringCR GetUriString() const;
    };
        
//=======================================================================================
//! The collection of all redline links in the markup project to elements in the target project
//=======================================================================================
struct DgnMarkupLinksCollection
{
private:
    friend struct DgnMarkupLinkIterator;
    friend struct DgnMarkupLinks;
    DgnDb&                     m_targetProject;
    DgnMarkupProject&               m_markupProject;
    DgnLinkTable::InProjectIterator m_allDgnLinksInProject;

    DgnMarkupLinksCollection (DgnDb&, DgnMarkupProject&);

public:
    //! Get the first markup link in the collection
    DGNPLATFORM_EXPORT DgnMarkupLinkIterator begin() const;

    //! Get one beyond the end of the collection
    DGNPLATFORM_EXPORT DgnMarkupLinkIterator end() const;
};

//=======================================================================================
//! The collection of redline links from a specific redline model to elements in the target project
//=======================================================================================
struct DgnMarkupLinksForRedlineModelCollection
{
private:
    friend struct DgnMarkupLinkIterator;
    friend struct DgnMarkupLinks;
    DgnDb&                     m_targetProject;
    DgnMarkupProject&               m_markupProject;
    DgnLinkTable::OnElementIterator m_linksOnElementIterator;

    DgnMarkupLinksForRedlineModelCollection (DgnDb&, DgnMarkupProject&, PersistentElementRefR anchor);

public:
    //! Get the first markup link in the collection
    DGNPLATFORM_EXPORT DgnMarkupLinkIterator begin() const;

    //! Get one beyond the end of the collection
    DGNPLATFORM_EXPORT DgnMarkupLinkIterator end() const;
};

//=======================================================================================
//! Helper class creates and queries links between markups and elements
//=======================================================================================
struct DgnMarkupLinks
{
    //! Create a DgnLink associating the redline model with the specified element.
    //! @param rdlModel The redline model to be linked with the element.
    //! @param targetElement The element to be linked with the redline model.
    //! @return non-zero error status if the link could not be created.
    DGNPLATFORM_EXPORT static BentleyStatus CreateRedlineLink (RedlineModelR rdlModel, PersistentElementRefR targetElement);

    //! Make an iterator over all redline links in the markup project that point to elements in the target project.
    //! @param targetProject    The DgnDb that contains the targets of the links.
    //! @param markupProject    The markup project that contains the links.
    DGNPLATFORM_EXPORT static DgnMarkupLinksCollection MakeIterator (DgnDb& targetProject, DgnMarkupProject& markupProject);

    //! Make an iterator over the links in the specified redline model that point to elements in the target project.
    //! @param targetProject    The DgnDb that contains the targets of the links.
    //! @param rdlModel         The redline model that contains the links.
    DGNPLATFORM_EXPORT static DgnMarkupLinksForRedlineModelCollection MakeIteratorForRedlineModel (DgnDb& targetProject, RedlineModelR rdlModel);

    //! Get the dummy element in this specified redline model that can be used as the anchor for DgnLinks.
    //! @param rdlModel The redline model
    //! @return The redline link anchor element
    DGNPLATFORM_EXPORT static PersistentElementRef& GetRedlineLinkSource (RedlineModel& rdlModel);
};
#endif

END_BENTLEY_DGNPLATFORM_NAMESPACE
