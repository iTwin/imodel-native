/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once
#include <DgnDbSync/DgnV8/Converter.h>
#include <DgnPlatform/LinkElement.h>

BEGIN_DGNDBSYNC_DGNV8_NAMESPACE

//=======================================================================================
// @bsiclass                                             Ramanujam.Raman        05/2016
//+===============+===============+===============+===============+===============+======
struct LinkConverter
{
private:
    Converter& m_converter;
    LinkModelPtr m_linkModel;
    bool m_needsPurge;
    struct ProjectWiseExtension* m_pwExtension;

    void InitializeTempTables();
    void InitializeProjectWiseExtension();
    void EnableProjectWiseExtension();
    void DisableProjectWiseExtension();
    void TerminateProjectWiseExtension();

    BentleyStatus ImportLinksOnElement(DgnV8LinkTreeNodeCR linkTreeNode, DgnElementId sourceElementId);

    BentleyStatus ImportUrlLink(DgnV8URLLinkCR urlLink, DgnElementId sourceElementId);
    DgnElementId QueryUrlLink(Utf8StringCR url, Utf8StringCR label, Utf8StringCR description) const;
    DgnElementId InsertUrlLink(Utf8StringCR url, Utf8StringCR label, Utf8StringCR description);

    BentleyStatus ImportEmbeddedFileLink(DgnV8FileLinkCR fileLink, DgnElementId sourceElementId);
    DgnElementId QueryEmbeddedFileLink(Utf8StringCR embeddedName, Utf8StringCR label, Utf8StringCR description) const;
    DgnElementId InsertEmbeddedFileLink(Utf8StringCR embeddedName, Utf8StringCR label, Utf8StringCR description);

    BentleyStatus ImportEmbeddedFile(Utf8StringR embeddedName, BeFileNameCR pathname, Utf8StringCR description);
    static DateTime GetFileLastModifiedTime(BeFileNameCR pathname);

    bool ElementHasLinks(DgnElementId sourceElementId) const;

    bool GetNeedsPurge() const { return m_needsPurge; }
    void SetNeedsPurge() { m_needsPurge = true; }
    DgnElementIdSet FindOrphanedEmbeddedFileLinks(bset<Utf8String>& embeddedNames);
    BentleyStatus PurgeOrphanedEmbeddedFileLinks(DgnElementIdSet const& orphanedLinkIds);
    BentleyStatus PurgeOrphanedEmbeddedFiles(bset<Utf8String> const& embeddedNames);

    BentleyStatus RecordProcessedElement(DgnV8EhCR v8eh, DgnElementId v9ElementId);
    bool WasElementProcessed(DgnV8EhCR v8eh) const;

    BentleyStatus ConvertLinksOnElementWithProjectWise(DgnV8Api::DgnLinkTreeSpec const& linkTreeSpec, DgnV8EhCP v8eh, DgnElementId sourceElementId, Converter::ChangeOperation changeOperation);

public:
    //! Constructor
    LinkConverter(Converter& converter);

    //! Destructor
    ~LinkConverter();

    //! Converts (i.e., imports or updates) all the links on the V8 supplied element, and associates them with the supplied sourceElement
    //! @remarks Called after an element has been converted this may cause orphaned links.  Call @ref PurgeOrphanedLinks() as necessary. 
    BentleyStatus ConvertLinksOnElement(DgnV8EhCP v8eh, DgnElementId sourceElementId, Converter::ChangeOperation changeOperation);

    //! Removes all the links on the supplied source element
    //! @remarks Called before an element has been deleted this may cause orphaned links. Call @ref PurgeOrphanedLinks() as necessary. 
    BentleyStatus RemoveLinksOnElement(DgnElementId sourceElementId);

    //! Purges orphaned links
    //! @remarks Called towards the end of the conversion process, this purges any links that have been orphaned through other calls
    //! to the link converter. 
    BentleyStatus PurgeOrphanedLinks();

    LinkModelP GetOrCreateLinkModel();
};

END_DGNDBSYNC_DGNV8_NAMESPACE

