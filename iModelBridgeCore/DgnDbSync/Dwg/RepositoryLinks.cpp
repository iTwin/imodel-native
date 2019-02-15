/*--------------------------------------------------------------------------------------+
|
|     $Source: Dwg/RepositoryLinks.cpp $
|
|  $Copyright: (c) 2019 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include    "DwgImportInternal.h"

USING_NAMESPACE_BENTLEY
USING_NAMESPACE_BENTLEY_DGN
USING_NAMESPACE_DWGDB
USING_NAMESPACE_DWG

/*=================================================================================**//**
* @bsiclass                                                     Don.Fu          02/19
+===============+===============+===============+===============+===============+======*/
struct RepositoryLinkFactory
{
private:
    DgnDbR          m_dgndb;
    DwgDbDatabaseR  m_dwgdb;
    BeFileName      m_dwgfilename;
    iModelBridge::Params const& m_bridgeparams;

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          02/19
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String  ComputeURN ()
    {
    Utf8String  urn (m_bridgeparams.QueryDocumentURN(m_dwgfilename));
    if (!iModelBridge::IsPwUrn(urn))
        {
        urn = m_dwgfilename.GetUri ();
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

public:
RepositoryLinkFactory (DgnDbR db, DwgDbDatabaseR dwg, iModelBridge::Params const& bp) 
    : m_dgndb(db), m_dwgdb(dwg), m_bridgeparams(bp)
    {
    m_dwgfilename.SetName (m_dwgdb.GetFileName().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          02/19
+---------------+---------------+---------------+---------------+---------------+------*/
DgnElementId    CreateOrUpdate ()
    {
    auto urn = this->ComputeURN ();
    auto newLink = iModelBridge::MakeRepositoryLink (m_dgndb, m_bridgeparams, m_dwgfilename, urn, urn);
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
            m_dwgdb.SetRepositoryLinkId (linkId.GetValue());
            return linkId;
            }
        }

    auto updatedLink = linkId.IsValid() ? newLink->Update(): newLink->Insert();
    if (!updatedLink.IsValid())
        return DgnElementId();

    linkId = updatedLink->GetElementId ();
    m_dwgdb.SetRepositoryLinkId (linkId.GetValue());

    return linkId;
    }

};  // RepositoryFactory

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          02/19
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   DwgImporter::UpdateRepositoryLink (DwgDbDatabaseP source)
    {
    DwgDbDatabaseP  dwg = source == nullptr ? m_dwgdb.get() : source;
    if (nullptr == dwg)
        return  BentleyStatus::BSIERROR;

    RepositoryLinkFactory   factory(*m_dgndb, *dwg, this->GetOptions());
    auto linkId = factory.CreateOrUpdate ();
    
    return  linkId.IsValid() ? BentleyStatus::BSISUCCESS : BentleyStatus::BSIERROR;
    }
