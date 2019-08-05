/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
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

    auto updatedLink = linkId.IsValid() ? newLink->Update(): newLink->Insert();
    if (!updatedLink.IsValid())
        return DgnElementId();

    linkId = updatedLink->GetElementId ();
    dwg.SetRepositoryLinkId (linkId.GetValue());

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
        status = this->UpdateRepositoryLink (&dwg);

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
BentleyStatus   DwgImporter::UpdateRepositoryLink (DwgDbDatabaseP source)
    {
    DwgDbDatabaseP  dwg = source == nullptr ? m_dwgdb.get() : source;
    if (nullptr == dwg)
        return  BentleyStatus::BSIERROR;

    RepositoryLinkFactory   factory(*m_dgndb, this->GetOptions());
    auto linkId = factory.CreateOrUpdate (*dwg);
    
    if (!linkId.IsValid())
        {
        this->ReportError (IssueCategory::Briefcase(), Issue::Message(), Utf8PrintfString("Failed to insert/update RepositoryLink for %ls", dwg->GetFileName().c_str()).c_str());
        return  BentleyStatus::BSIERROR;
        }

    return  BentleyStatus::BSISUCCESS;
    }
