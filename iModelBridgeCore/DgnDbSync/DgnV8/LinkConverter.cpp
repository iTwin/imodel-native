/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#include "ConverterInternal.h"
#include <windows.h>

#define TEMP_LINK_SOURCES_TABLE_NO_PREFIX "LinkSources"
#define TEMP_LINK_SOURCES_TABLE "temp." TEMP_LINK_SOURCES_TABLE_NO_PREFIX

USING_NAMESPACE_BENTLEY_SQLITE_EC

BEGIN_DGNDBSYNC_DGNV8_NAMESPACE

typedef StatusInt(__cdecl *PwInitializeFn)(WCharCP pwDataSource, WCharCP pwWorkDir, WCharCP pwUser, WCharCP pwPassword);
typedef void(__cdecl *PwSetEnabledFn)(bool enable);
typedef StatusInt(__cdecl *PwTerminateFn)();

//=======================================================================================
// @bsiclass                                             Ramanujam.Raman        05/2016
//+===============+===============+===============+===============+===============+======
struct ProjectWiseExtension
    {
    HINSTANCE m_handle = nullptr;
    PwSetEnabledFn m_enableExtensionFn = nullptr;

    BentleyStatus Initialize(Converter::Params const& params);
    
    BentleyStatus Terminate();

    void SetEnabled(bool enable)
        {
        BeAssert(m_enableExtensionFn != nullptr);
        m_enableExtensionFn(enable);
        }
    };

//---------------------------------------------------------------------------------------
// @bsimethod                                                Ramanujam.Raman   05/2016
//---------------------------------------------------------------------------------------
BentleyStatus ProjectWiseExtension::Initialize(Converter::Params const& params)
    {
    BeFileNameCR pwExtensionDll = params.GetProjectWiseExtensionDll();
    if (pwExtensionDll.empty())
	    {
		BeAssert(false);
        return ERROR;
        }
		
    BeAssert(pwExtensionDll.DoesPathExist() && "Specified ProjectWise extension DLL does not exist");

    m_handle = ::LoadLibraryW(pwExtensionDll.c_str());
    if (!m_handle)
        {
        BeAssert(false && "Could not load specified ProjectWise extension");
        return ERROR;
        }

    PwInitializeFn initializeFn = (PwInitializeFn) GetProcAddress(m_handle, "Initialize");
    if (!initializeFn)
        {
        BeAssert(false && "Could not find method to initialize ProjectWise extension");
        return ERROR;
        }

    m_enableExtensionFn = (PwSetEnabledFn) GetProcAddress(m_handle, "SetEnabled");
    if (!m_enableExtensionFn)
        {
        BeAssert(false && "Could not find method to enable/disable ProjectWise extension");
        return ERROR;
        }

    WString pwDataSource(params.GetProjectWiseDataSource().c_str(), true);
    BeFileNameCR pwWorkDir = params.GetProjectWiseWorkDir();
    WString pwUser(params.GetProjectWiseUser().c_str(), true);
    WString pwPassword(params.GetProjectWisePassword().c_str(), true);

    StatusInt status = initializeFn(pwDataSource.c_str(), pwWorkDir.c_str(), pwUser.c_str(), pwPassword.c_str());
    if (status != SUCCESS)
        {
        BeAssert(false && "Could not initialize ProjectWise extension");
        return ERROR;
        }

    SetEnabled(false);

    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Ramanujam.Raman   05/2016
//---------------------------------------------------------------------------------------
BentleyStatus ProjectWiseExtension::Terminate()
    {
    if (!m_handle)
        return SUCCESS;

    PwTerminateFn terminateFn = (PwTerminateFn) GetProcAddress(m_handle, "Terminate");
    if (!terminateFn)
        {
        BeAssert(false && "Could not method to terminate ProjectWise extension");
        return ERROR;
        }

    StatusInt status = terminateFn();
    if (status != SUCCESS)
        {
        BeAssert(false && "Could not terminate ProjectWise extension");
        return ERROR;
        }

    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Ramanujam.Raman   05/2016
//---------------------------------------------------------------------------------------
LinkConverter::LinkConverter(Converter& converter) : m_converter(converter), m_needsPurge(false), m_pwExtension(nullptr)
    {
    InitializeTempTables();
    InitializeProjectWiseExtension();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Ramanujam.Raman   06/2016
//---------------------------------------------------------------------------------------
LinkConverter::~LinkConverter()
    {
    TerminateProjectWiseExtension();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Ramanujam.Raman   05/2016
//---------------------------------------------------------------------------------------
void LinkConverter::InitializeTempTables()
    {
    if (m_converter.GetDgnDb().TableExists(TEMP_LINK_SOURCES_TABLE))
        {
        Statement stmt;
        stmt.Prepare(m_converter.GetDgnDb(), "DELETE FROM " TEMP_LINK_SOURCES_TABLE);
        stmt.Step();
        }
    else
        {
        DbResult result = m_converter.GetDgnDb().CreateTable(TEMP_LINK_SOURCES_TABLE, "V8FileSyncInfoId INT NOT NULL, V8ElementId BIGINT NOT NULL, V9ElementId BIGINT NOT NULL");
        BeAssert(result == BE_SQLITE_OK);
        UNUSED_VARIABLE(result);

        m_converter.GetDgnDb().ExecuteSql("CREATE INDEX " TEMP_LINK_SOURCES_TABLE "ElementIdx ON " TEMP_LINK_SOURCES_TABLE_NO_PREFIX "(V9ElementId)");
        m_converter.GetDgnDb().ExecuteSql("CREATE INDEX " TEMP_LINK_SOURCES_TABLE "V8Idx ON " TEMP_LINK_SOURCES_TABLE_NO_PREFIX "(V8FileSyncInfoId,V8ElementId)");
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Ramanujam.Raman   06/2016
//---------------------------------------------------------------------------------------
void LinkConverter::InitializeProjectWiseExtension()
    {
    BeFileNameCR pwExtensionDll = m_converter.GetParams().GetProjectWiseExtensionDll();
    if (pwExtensionDll.empty())
        return;
        
    m_pwExtension = new ProjectWiseExtension();
    if (SUCCESS != m_pwExtension->Initialize(m_converter.GetParams()))
        {
        m_converter.ReportIssueV(Converter::IssueSeverity::Warning, Converter::IssueCategory::Unsupported(), Converter::Issue::InitProjectWiseLinkError(), nullptr);
			
        delete m_pwExtension;
        m_pwExtension = nullptr;
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Ramanujam.Raman   06/2016
//---------------------------------------------------------------------------------------
void LinkConverter::TerminateProjectWiseExtension()
    {
    if (!m_pwExtension)
        return;

    if (SUCCESS != m_pwExtension->Terminate())
        m_converter.ReportIssueV(Converter::IssueSeverity::Warning, Converter::IssueCategory::Unsupported(), Converter::Issue::TerminateProjectWiseLinkError(), nullptr);

    delete m_pwExtension;
    m_pwExtension = nullptr;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Ramanujam.Raman   06/2016
//---------------------------------------------------------------------------------------
void LinkConverter::EnableProjectWiseExtension()
    {
    if (!m_pwExtension)
        return;
    m_pwExtension->SetEnabled(true);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Ramanujam.Raman   06/2016
//---------------------------------------------------------------------------------------
void LinkConverter::DisableProjectWiseExtension()
    {
    if (!m_pwExtension)
        return;
    m_pwExtension->SetEnabled(false);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Ramanujam.Raman   05/2016
//---------------------------------------------------------------------------------------
BentleyStatus LinkConverter::RecordProcessedElement(DgnV8EhCR v8eh, DgnElementId v9ElementId)
    {
    CachedStatementPtr stmt;
    m_converter.GetDgnDb().GetCachedStatement(stmt, "INSERT INTO " TEMP_LINK_SOURCES_TABLE " (V8FileSyncInfoId, V8ElementId, V9ElementId) VALUES(?,?,?)");

    RepositoryLinkId v8FileId = m_converter.GetRepositoryLinkId(*v8eh.GetDgnFileP());
    stmt->BindId(1, v8FileId);

    DgnV8Api::ElementId v8ElementId = v8eh.GetElementId();
    stmt->BindInt64(2, v8ElementId);

    stmt->BindId(3, v9ElementId);

    DbResult result = stmt->Step();
    return (result == BE_SQLITE_DONE) ? SUCCESS : ERROR;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Ramanujam.Raman   05/2016
//---------------------------------------------------------------------------------------
bool LinkConverter::WasElementProcessed(DgnV8EhCR v8eh) const
    {
    CachedStatementPtr stmt;
    m_converter.GetDgnDb().GetCachedStatement(stmt, "SELECT V9ElementId FROM " TEMP_LINK_SOURCES_TABLE " WHERE V8FileSyncInfoId=? AND V8ElementId=?");

    RepositoryLinkId v8FileId = m_converter.GetRepositoryLinkId(*v8eh.GetDgnFileP());
    stmt->BindId(1, v8FileId);

    DgnV8Api::ElementId v8ElementId = v8eh.GetElementId();
    stmt->BindInt64(2, v8ElementId);

    DbResult result = stmt->Step();
    return result == BE_SQLITE_ROW;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Ramanujam.Raman   05/2016
//---------------------------------------------------------------------------------------
LinkModelP LinkConverter::GetOrCreateLinkModel()
    {
    if (m_linkModel.IsValid())
        return m_linkModel.get();

    Utf8CP partitionName = "Converted Links";
    DgnCode partitionCode = LinkPartition::CreateCode(m_converter.GetJobSubject(), partitionName);
    DgnElementId partitionId = m_converter.GetDgnDb().Elements().QueryElementIdByCode(partitionCode);
    if (partitionId.IsValid())
        {
        m_linkModel = LinkModel::Get(m_converter.GetDgnDb(), DgnModelId(partitionId.GetValue()));
        BeAssert(m_linkModel.IsValid());
        return m_linkModel.get();
        }

    LinkPartitionPtr partitioned = LinkPartition::Create(m_converter.GetJobSubject(), partitionName);
    LinkPartitionCPtr partition = partitioned->InsertT<LinkPartition>();
    if (!partition.IsValid())
        {
        BeAssert(false);
        return nullptr;
        }

    m_linkModel = LinkModel::Create(LinkModel::CreateParams(m_converter.GetDgnDb(), partition->GetElementId()));
    DgnDbStatus status;
    if ((status = m_linkModel->Insert()) != DgnDbStatus::Success)
        {
        BeAssert((DgnDbStatus::LockNotHeld != status) && "Failed to get or retain necessary locks");
        BeAssert(false);
        m_linkModel = nullptr;
        return nullptr;
        }

    return m_linkModel.get();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Ramanujam.Raman   05/2016
//---------------------------------------------------------------------------------------
BentleyStatus LinkConverter::RemoveLinksOnElement(DgnElementId sourceElementId)
    {
    if (!ElementHasLinks(sourceElementId))
        return SUCCESS;

    /* 
     * Note: We simply purge orphaned links at the end of the conversion process. This 
     * keeps things simple - if we do find performance issues, we can consider storing the 
     * potentially orphaned links, and direct the purge only to a subset of them that have 
     * been orphaned. 
     */
    SetNeedsPurge();

    BentleyStatus status = SUCCESS;
    
    if (SUCCESS != UrlLink::RemoveAllFromSource(m_converter.GetDgnDb(), sourceElementId))
        status = ERROR;

    if (SUCCESS != EmbeddedFileLink::RemoveAllFromSource(m_converter.GetDgnDb(), sourceElementId))
        status = ERROR;

    return status;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Ramanujam.Raman   05/2016
//---------------------------------------------------------------------------------------
bool LinkConverter::ElementHasLinks(DgnElementId sourceElementId) const
    {
    Utf8CP ecSql = "SELECT NULL FROM " BIS_SCHEMA(BIS_REL_ElementHasLinks) " WHERE SourceECInstanceId=? LIMIT 1";

    BeSQLite::EC::CachedECSqlStatementPtr stmt = m_converter.GetDgnDb().GetPreparedECSqlStatement(ecSql);
    BeAssert(stmt.IsValid());

    stmt->BindId(1, sourceElementId);
    return (BE_SQLITE_ROW == stmt->Step());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Ramanujam.Raman   05/2016
//---------------------------------------------------------------------------------------
BentleyStatus LinkConverter::ConvertLinksOnElementWithProjectWise(DgnV8Api::DgnLinkTreeSpec const& linkTreeSpec, DgnV8EhCP v8eh, DgnElementId sourceElementId, Converter::ChangeOperation changeOperation)
    {
    DgnV8Api::DgnLinkTreePtr linkTree = DgnV8Api::DgnLinkManager::ReadLinkTree(linkTreeSpec, false);
    if (linkTree.IsNull())
        return SUCCESS;

    /*
     * Note: We simply ignore previously processed v8 elements - they can be mapped to multiple v9 elements, but we are making the following assumptions: 
     * 1. The mapped V9 elements are all in the same parent-child hierarchy, AND
     * 2. The parent has been processed before the children. 
     */
    if (WasElementProcessed(*v8eh))
        return SUCCESS;
    
    RecordProcessedElement(*v8eh, sourceElementId);

    if (changeOperation == Converter::ChangeOperation::Update)
        {
        /*
         * Note: We recreate the links on update for simplicity. We can revisit this if 
         * it proves to be a performance drag. 
         */
        if (SUCCESS != RemoveLinksOnElement(sourceElementId))
            return ERROR;
        }
        
    BentleyStatus status = ImportLinksOnElement(linkTree->GetRoot(), sourceElementId);
    if (status != SUCCESS)
        {
        m_converter.ReportIssueV(Converter::IssueSeverity::Error, Converter::IssueCategory::MissingData(), Converter::Issue::FailedToImportLinkError(), nullptr, v8eh->GetElementId(), Utf8String(v8eh->GetDgnFileP()->GetFileName().c_str()).c_str(), sourceElementId.GetValue());
        }

    return status;
    }


//---------------------------------------------------------------------------------------
// @bsimethod                                                Ramanujam.Raman   05/2016
//---------------------------------------------------------------------------------------
BentleyStatus LinkConverter::ConvertLinksOnElement(DgnV8EhCP v8eh, DgnElementId sourceElementId, Converter::ChangeOperation changeOperation)
    {
    DgnV8Api::DgnLinkTreeSpecPtr linkTreeSpec = DgnV8Api::DgnLinkManager::CreateTreeSpec(*v8eh);
    if (linkTreeSpec.IsNull())
        return SUCCESS;

    EnableProjectWiseExtension();

    BentleyStatus status = ConvertLinksOnElementWithProjectWise(*linkTreeSpec, v8eh, sourceElementId, changeOperation);

    DisableProjectWiseExtension();
    /* Turns out that keeping the PWExtension continuously enabled messes up the rest of the conversion process - PW gets involved in even location of references. By
    * enabling and disabling the PWExtension only for links, we restrict the scope of PW involvement. Tracked by TFS#57113 */

    return status;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Ramanujam.Raman   05/2016
//---------------------------------------------------------------------------------------
BentleyStatus LinkConverter::ImportLinksOnElement(DgnV8LinkTreeNodeCR linkTreeNode, DgnElementId sourceElementId)
    {
    DgnLinkTreeBranchCP linkTreeBranch = linkTreeNode.AsDgnLinkTreeBranchCP();
    if (linkTreeBranch)
        {
        BentleyStatus status = SUCCESS;
        for (int ii = 0; ii < (int) linkTreeBranch->GetChildCount(); ii++)
            {
            Bentley::DgnLinkTreeNodeCP childLinkTreeNode = linkTreeBranch->GetChildCP(ii);
            if (!childLinkTreeNode)
                continue;

            if (SUCCESS != ImportLinksOnElement(*childLinkTreeNode, sourceElementId))
                status = ERROR;
            }

        return status;
        }

    DgnLinkTreeLeafCP linkTreeLeaf = linkTreeNode.AsDgnLinkTreeLeafCP();
    BeAssert(linkTreeLeaf != nullptr);

    Bentley::DgnLinkCP dgnLink = linkTreeLeaf->GetLinkCP();
    if (!dgnLink)
        return SUCCESS;

    Bentley::DgnURLLinkCP urlLink = dynamic_cast<Bentley::DgnURLLinkCP>(dgnLink);
    if (urlLink)
        return ImportUrlLink(*urlLink, sourceElementId);

    Bentley::DgnFileLinkCP fileLink = dynamic_cast<Bentley::DgnFileLinkCP>(dgnLink);
    if (fileLink)
        {
        if (m_converter.GetConfig().GetOptionValueBool("EmbedDgnLinkFiles", true))
            return ImportEmbeddedFileLink(*fileLink, sourceElementId);

        return SUCCESS;
        }
   
    return ERROR;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Ramanujam.Raman   05/2016
//---------------------------------------------------------------------------------------
BentleyStatus LinkConverter::ImportUrlLink(DgnV8URLLinkCR urlLink, DgnElementId sourceElementId)
    {
    Utf8String url(urlLink.GetAddress());
    Utf8String label(urlLink.GetTreeNodeCP()->GetName());
    Utf8String description(urlLink.GetShortDescription().c_str());

    DgnElementId linkId = QueryUrlLink(url, label, description);

    if (!linkId.IsValid())
        {
        linkId = InsertUrlLink(url, label, description);
        if (!linkId.IsValid())
            return ERROR;
        }
        
    return LinkElement::AddToSource(m_converter.GetDgnDb(), linkId, sourceElementId);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Ramanujam.Raman   05/2016
//---------------------------------------------------------------------------------------
DgnElementId LinkConverter::QueryUrlLink(Utf8StringCR url, Utf8StringCR label, Utf8StringCR description) const
    {
    DgnElementIdSet idSet = UrlLink::Query(m_converter.GetDgnDb(), url.c_str(), label.c_str(), description.c_str());
    return idSet.empty() ? DgnElementId() : *(idSet.begin());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Ramanujam.Raman   05/2016
//---------------------------------------------------------------------------------------
DgnElementId LinkConverter::InsertUrlLink(Utf8StringCR url, Utf8StringCR label, Utf8StringCR description)
    {
    LinkModelP linkModel = GetOrCreateLinkModel();
    if (!linkModel)
        return DgnElementId();

    UrlLinkPtr link = UrlLink::Create(UrlLink::CreateParams(*linkModel, url.c_str(), label.c_str(), description.c_str()));

    UrlLinkCPtr linkCPtr = link->Insert();

    if (!linkCPtr.IsValid())
        {
        BeAssert(false);
        return DgnElementId();
        }

    return linkCPtr->GetElementId();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Ramanujam.Raman   05/2016
//---------------------------------------------------------------------------------------
BentleyStatus LinkConverter::ImportEmbeddedFileLink(DgnV8FileLinkCR fileLink, DgnElementId sourceElementId)
    {
    StatusInt docStatus;
    Bentley::DgnDocumentMonikerR fileMoniker = const_cast<Bentley::DgnDocumentMonikerR>(fileLink.GetMoniker()); // TODO: CreateFromMonker() doesn't accept a CR for some reason
    Bentley::DgnDocumentPtr document = DgnV8Api::DgnDocument::CreateFromMoniker(docStatus, fileMoniker, DEFDGNFILE_ID, DgnV8Api::DgnDocument::FetchMode::Read,
                                                                                DgnV8Api::DgnDocument::FetchOptions::Export | DgnV8Api::DgnDocument::FetchOptions::Silent);

    if (!document.IsValid())
        {
        m_converter.ReportIssueV(Converter::IssueSeverity::Error, Converter::IssueCategory::MissingData(), Converter::Issue::FailedToImportDocLinkError(), nullptr, Utf8String(fileMoniker.GetPortableName().c_str()).c_str());
        return ERROR;
        }

    BeFileName pathname(document->GetFileName().c_str());
    Utf8String label(fileLink.GetTreeNodeCP()->GetName());
    Utf8String description(fileLink.GetShortDescription().c_str());

    Utf8String embeddedName;
    if (SUCCESS != ImportEmbeddedFile(embeddedName, pathname, description))
        return ERROR;
       
    DgnElementId linkId = QueryEmbeddedFileLink(embeddedName, label, description);

    if (!linkId.IsValid())
        {
        linkId = InsertEmbeddedFileLink(embeddedName, label, description);
        if (!linkId.IsValid())
            return ERROR;
        }

    return LinkElement::AddToSource(m_converter.GetDgnDb(), linkId, sourceElementId);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Ramanujam.Raman   05/2016
//---------------------------------------------------------------------------------------
BentleyStatus LinkConverter::ImportEmbeddedFile(Utf8StringR embeddedName, BeFileNameCR pathname, Utf8StringCR description)
    {
    if (!pathname.DoesPathExist())
        {
        BeAssert(false);
        return ERROR;
        }

    embeddedName.Assign(pathname.c_str()); // Note: We assume that the pathname can be reliably used to indicate that it's the same file

    DateTime fileLastModified = GetFileLastModifiedTime(pathname);

    DateTime embeddedLastModified;
    BeBriefcaseBasedId embeddedId = m_converter.GetDgnDb().EmbeddedFiles().QueryFile(embeddedName.c_str(), nullptr, nullptr, nullptr, nullptr, &embeddedLastModified);

    if (embeddedId.IsValid())
        {
        /*
         * Note: The embedded file last modified time undergoes a very slight change (a few hectonanoseconds) when stored - so we use the 
         * approximate value in msec-s instead to compare.
         */
        int64_t msec1 = 0, msec2 = 0;
        fileLastModified.ToUnixMilliseconds(msec1);
        embeddedLastModified.ToUnixMilliseconds(msec2);
        if (msec1 == msec2 && msec1 != 0)
            return SUCCESS; 

        DbResult result = m_converter.GetDgnDb().EmbeddedFiles().Replace(embeddedName.c_str(), embeddedName.c_str(), 500 * 1024, &fileLastModified);
        if (BE_SQLITE_OK != result)
            {
            BeAssert(false && "Could not replace the linked file");
            return ERROR;
            }

        return SUCCESS;
        }

    Utf8String extension(pathname.GetExtension());

    DbResult result;
    m_converter.GetDgnDb().EmbeddedFiles().Import(&result, embeddedName.c_str(), embeddedName.c_str(), extension.c_str(), description.c_str(), &fileLastModified);
    if (BE_SQLITE_OK != result)
        {
        BeAssert(false && "Could not embed the linked file");
        return ERROR;
        }

    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Ramanujam.Raman   05/2016
//---------------------------------------------------------------------------------------
// static
DateTime LinkConverter::GetFileLastModifiedTime(BeFileNameCR pathname)
    {
    time_t mtime;
    if (BeFileNameStatus::Success != pathname.GetFileTime(nullptr, nullptr, &mtime, pathname.c_str()))
        return DateTime();

    DateTime lastModifiedTime;
    if (SUCCESS != DateTime::FromUnixMilliseconds(lastModifiedTime, (uint64_t) mtime))
        return DateTime();

    return lastModifiedTime;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Ramanujam.Raman   05/2016
//---------------------------------------------------------------------------------------
DgnElementId LinkConverter::QueryEmbeddedFileLink(Utf8StringCR embeddedName, Utf8StringCR label, Utf8StringCR description) const
    {
    DgnElementIdSet idSet = EmbeddedFileLink::Query(m_converter.GetDgnDb(), embeddedName.c_str(), label.c_str(), description.c_str());
    return idSet.empty() ? DgnElementId() : *(idSet.begin());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Ramanujam.Raman   05/2016
//---------------------------------------------------------------------------------------
DgnElementId LinkConverter::InsertEmbeddedFileLink(Utf8StringCR embeddedName, Utf8StringCR label, Utf8StringCR description)
    {
    LinkModelP linkModel = GetOrCreateLinkModel();
    if (!linkModel)
        return DgnElementId();

    EmbeddedFileLinkPtr link = EmbeddedFileLink::Create(EmbeddedFileLink::CreateParams(*linkModel, embeddedName.c_str(), label.c_str(), description.c_str()));

    EmbeddedFileLinkCPtr linkCPtr = link->Insert();

    if (!linkCPtr.IsValid())
        {
        BeAssert(false);
        return DgnElementId();
        }

    return linkCPtr->GetElementId();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Ramanujam.Raman   05/2016
//---------------------------------------------------------------------------------------
BentleyStatus LinkConverter::PurgeOrphanedLinks()
    {
    /*
     * Note: In the interests of a performance, especially when the updater is being
     * executed, we purge *only* if there have been updates to the links. This implies
     * there may be cases with deleted elements causing orphaned links that will not 
     * be purged, until that's explicitly done by some process.
     */
    if (!GetNeedsPurge())
        return SUCCESS;

    if (SUCCESS != UrlLink::PurgeOrphaned(m_converter.GetDgnDb()))
        return ERROR;

    bset<Utf8String> embeddedNames;

    DgnElementIdSet orphanedLinkIds = FindOrphanedEmbeddedFileLinks(embeddedNames);
    if (orphanedLinkIds.empty())
        return SUCCESS;

    if (SUCCESS != PurgeOrphanedEmbeddedFileLinks(orphanedLinkIds))
        return ERROR;

    return PurgeOrphanedEmbeddedFiles(embeddedNames);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Ramanujam.Raman   05/2016
//---------------------------------------------------------------------------------------
DgnElementIdSet LinkConverter::FindOrphanedEmbeddedFileLinks(bset<Utf8String>& embeddedNames)
    {
    DgnElementIdSet orphanedLinkIds = EmbeddedFileLink::FindOrphaned(m_converter.GetDgnDb());
    if (orphanedLinkIds.empty())
        return orphanedLinkIds;

    embeddedNames.clear();
    for (DgnElementId linkId : orphanedLinkIds)
        {
        EmbeddedFileLinkCPtr link = EmbeddedFileLink::Get(m_converter.GetDgnDb(), linkId);
        embeddedNames.insert(link->GetName());
        }

    return orphanedLinkIds;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Ramanujam.Raman   05/2016
//---------------------------------------------------------------------------------------
BentleyStatus LinkConverter::PurgeOrphanedEmbeddedFileLinks(DgnElementIdSet const& orphanedLinkIds)
    {
    Utf8CP ecSql = "DELETE FROM ONLY " BIS_SCHEMA(BIS_CLASS_EmbeddedFileLink) " WHERE InVirtualSet(? , ECInstanceId)";

    CachedECSqlStatementPtr stmt = m_converter.GetDgnDb().GetPreparedECSqlStatement(ecSql);
    BeAssert(stmt.IsValid());

    stmt->BindInt64(1, (int64_t) &orphanedLinkIds);

    BeSQLite::DbResult stepStatus = stmt->Step();
    return (BeSQLite::DbResult::BE_SQLITE_DONE == stepStatus) ? SUCCESS : ERROR;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Ramanujam.Raman   05/2016
//---------------------------------------------------------------------------------------
BentleyStatus LinkConverter::PurgeOrphanedEmbeddedFiles(bset<Utf8String> const& embeddedNames)
    {
    BentleyStatus status = SUCCESS;

    for (Utf8StringCR embeddedName : embeddedNames)
        {
        DgnElementIdSet links = EmbeddedFileLink::Query(m_converter.GetDgnDb(), embeddedName.c_str(), nullptr, nullptr, 1); 
        if (links.empty())
            {
            DbResult result = m_converter.GetDgnDb().EmbeddedFiles().Remove(embeddedName.c_str());

            if (result != BE_SQLITE_OK)
                {
                BeAssert(false);
                status = ERROR;
                }
            }
        }

    return status;
    }

END_DGNDBSYNC_DGNV8_NAMESPACE

