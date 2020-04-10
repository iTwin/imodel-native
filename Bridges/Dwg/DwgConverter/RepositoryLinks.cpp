/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include    "DwgImportInternal.h"

USING_NAMESPACE_BENTLEY
USING_NAMESPACE_BENTLEY_DGN
USING_NAMESPACE_DWGDB
USING_NAMESPACE_DWG

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          02/19
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String  RepositoryLinkFactory::ComputeURN (BeFileNameCR dwgFilename)
    {
    Utf8String  urn (m_bridgeparams.QueryDocumentURN(dwgFilename));
    if (!iModelBridge::IsPwUrn(urn))
        {
        urn = dwgFilename.GetUri ();
        urn.ToLower ();

        // replace ':' with '|' after the drive letter:
        if (urn.StartsWith("file:///"))
            {
            auto foundAt = urn.find (':', 8);
            if (foundAt != Utf8String::npos)
                urn.replace (foundAt, 1, "|");
            }
        }
    return  urn;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          02/19
+---------------+---------------+---------------+---------------+---------------+------*/
DgnElementId    RepositoryLinkFactory::CreateOrUpdate (DwgDbDatabaseR dwg)
    {
    BeFileName  dwgFilename(dwg.GetFileName().c_str());

    auto urn = this->ComputeURN (dwgFilename);
    auto newLink = iModelBridge::MakeRepositoryLink (m_dgndb, m_bridgeparams, dwgFilename, urn, urn);
    if (!newLink.IsValid())
        return  DgnElementId();

    auto linkId = newLink->GetElementId ();
    auto existingLink = m_dgndb.Elements().Get <RepositoryLink> (linkId);
    if (existingLink.IsValid())
        {
        auto newHash = iModelBridge::ComputeRepositoryLinkHash (*newLink);
        auto existingHash = iModelBridge::ComputeRepositoryLinkHash (*existingLink);
        if (newHash.GetHashString().Equals(existingHash.GetHashString()))
            {
            dwg.SetRepositoryLinkId (linkId.GetValue());
            return linkId;
            }
        }

    // create a repository link aspect
    auto dwgInfo = DwgSourceAspects::DwgFileInfo(dwg, m_importer);
    auto aspect = DwgSourceAspects::RepositoryLinkAspect::Create (m_dgndb, dwgInfo);
    if (!aspect.IsValid())
        return DgnElementId();

    aspect.AddAspect (*newLink);

    auto updatedLink = linkId.IsValid() ? newLink->Update(): newLink->Insert();
    if (!updatedLink.IsValid())
        return DgnElementId();

    linkId = updatedLink->GetElementId ();
    dwg.SetRepositoryLinkId (linkId.GetValue());

    // also set file id policy
    dwg.SetFileIdPolicy (static_cast<uint32_t>(m_importer.GetCurrentIdPolicy()));

    // for the root DWG, now we have a valid repository link ID, set itself as the root in the aspect
    if (!dwgInfo.GetRootRepositoryLinkId().IsValid())
        {
        dwgInfo.SetRootRepositoryLinkId (RepositoryLinkId(linkId.GetValue()));
        auto rlinkEdit = m_dgndb.Elements().GetForEdit <RepositoryLink> (linkId);
        if (rlinkEdit.IsValid())
            {
            aspect = DwgSourceAspects::RepositoryLinkAspect::GetForEdit (*rlinkEdit);
            aspect.Update (dwgInfo);
            rlinkEdit->Update ();
            }
        }

    return linkId;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          02/19
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   RepositoryLinkFactory::DeleteFromDb (BeFileNameCR dwgFilename)
    {
    auto urn = this->ComputeURN (dwgFilename);
    auto newLink = iModelBridge::MakeRepositoryLink (m_dgndb, m_bridgeparams, dwgFilename, urn, urn);
    if (!newLink.IsValid())
        return  static_cast<BentleyStatus>(DgnDbStatus::BadElement);

    auto linkId = newLink->GetElementId ();
    if (!linkId.IsValid())
        return  static_cast<BentleyStatus>(DgnDbStatus::InvalidId);

    auto status = m_dgndb.Elements().Delete (linkId);

    return  static_cast<BentleyStatus>(status);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          02/19
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   DwgImporter::InsertElementHasLinks (DgnModelR model, DwgDbDatabaseR dwg)
    {
    BentleyStatus status = BentleyStatus::BSISUCCESS;

    // if a Respository has not been created for this DWG, create one now:
    uint64_t    savedId = 0;
    if (dwg.GetRepositoryLinkId(savedId) != DwgDbStatus::Success || savedId == 0)
        status = this->CreateOrUpdateRepositoryLink(&dwg).IsValid() ? BSISUCCESS : BSIERROR;

    // get the RepositoryLinkId from the DWG
    DgnDbStatus dbStatus = DgnDbStatus::InvalidId;
    if (status == BSISUCCESS && dwg.GetRepositoryLinkId(savedId) == DwgDbStatus::Success && savedId != 0)
        {
        // update the ElementHasLinks for this model
        DgnElementId    linkId(savedId);
        dbStatus = iModelBridge::InsertElementHasLinksRelationship (model.GetDgnDb(), model.GetModeledElementId(), linkId);
        }

    if (dbStatus != DgnDbStatus::Success)
        this->ReportError (IssueCategory::Briefcase(), Issue::Message(), Utf8PrintfString("failed to insert an ElementHasLinks relationship for model %s", model.GetName().c_str()).c_str());

    status = static_cast <BentleyStatus> (dbStatus);
    return  status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          02/19
+---------------+---------------+---------------+---------------+---------------+------*/
DgnElementId    DwgImporter::CreateOrUpdateRepositoryLink (DwgDbDatabaseP source)
    {
    DgnElementId    repLinkId;
    DwgDbDatabaseP  dwg = source == nullptr ? m_dwgdb.get() : source;
    if (nullptr == dwg)
        return  repLinkId;

    RepositoryLinkFactory   factory(*m_dgndb, *this);
    repLinkId = factory.CreateOrUpdate (*dwg);
    
    if (!repLinkId.IsValid())
        this->ReportError (IssueCategory::Briefcase(), Issue::Message(), Utf8PrintfString("Failed to insert/update RepositoryLink for %ls", dwg->GetFileName().c_str()).c_str());
    else
        m_repositoryLinksInSync.insert (T_DwgRepositoryLink(dwg, repLinkId));

    return  repLinkId;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          04/20
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   DwgImporter::CreateOrUpdateRepositoryLinks ()
    {
    // create/update a repository link from the root file
    auto replinkId = this->CreateOrUpdateRepositoryLink ();
    if (!replinkId.IsValid())
        return  BentleyStatus::BSIERROR;

    // create/update repository links for xref files
    for (auto xref : m_loadedXrefFiles)
        {
        replinkId = this->CreateOrUpdateRepositoryLink (xref.GetDatabaseP());
        if (!replinkId.IsValid())
            return  BentleyStatus::BSIERROR;
        }

    return  BentleyStatus::BSISUCCESS;    
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          02/19
+---------------+---------------+---------------+---------------+---------------+------*/
DgnElementId    DwgImporter::GetRepositoryLink (DwgDbDatabaseP source)
    {
    DgnElementId    rlinkId;
    DwgDbDatabaseP  dwg = nullptr == source ? m_dwgdb.get() : source;

    auto found = m_repositoryLinksInSync.find(dwg);
    if (found == m_repositoryLinksInSync.end())
        rlinkId = this->CreateOrUpdateRepositoryLink (dwg);
    else
        rlinkId = found->second;
    return  rlinkId;
    }
