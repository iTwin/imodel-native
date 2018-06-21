/*--------------------------------------------------------------------------------------+
|
|     $Source: Dwg/Tests/SyncInfoReader.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "Tests.h"
#include "ImporterTests.h"

//---------------------------------------------------------------------------------------
// @bsimethod                                    Sam.Wilson                      07/14
//---------------------------------------------------------------------------------------
SyncInfoReader::SyncInfoReader(BentleyApi::BeFileNameCR dbName)
    {
    AttachToDgnDb (dbName);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Sam.Wilson                      07/14
//---------------------------------------------------------------------------------------
void SyncInfoReader::AttachToDgnDb(BentleyApi::BeFileNameCR dbName)
    {
    m_dgndb = DgnDb::OpenDgnDb(nullptr, dbName, DgnDb::OpenParams(DgnDb::OpenMode::Readonly));
    BentleyApi::BeFileName syncInfoFileName = m_dgndb->GetFileName();
    syncInfoFileName.append(L".syncinfo");
    ASSERT_EQ( BE_SQLITE_OK , m_dgndb->AttachDb(BentleyApi::Utf8String(syncInfoFileName).c_str(), SYNCINFO_ATTACH_ALIAS) );
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Sam.Wilson                      07/14
//---------------------------------------------------------------------------------------
void SyncInfoReader::MustFindElementByDwgEntityHandle(DgnElementId& eid, DwgSyncInfo::DwgModelSyncInfoId const& fmid, DwgDbHandleCR entityHandle, int expectedCount) const
    {
    DwgSyncInfo::ByDwgObjectIdIter elements(*m_dgndb);
    elements.Bind(fmid, entityHandle.AsUInt64());
    int count=0;
    for (DwgSyncInfo::ByDwgObjectIdIter::Entry entry = elements.begin(); entry != elements.end(); ++entry) 
        {
        ++count;
        eid = entry.GetElementId();
        }
    ASSERT_EQ( expectedCount , count );
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Don.Fu          06/18
+---------------+---------------+---------------+---------------+---------------+------*/
void SyncInfoReader::MustFindModelSyncInfo (DwgSyncInfo::DwgModelSyncInfoId& out, uint64_t dwgModelId, DwgSyncInfo::ModelSourceType type) const
    {
    // always the master file
    DwgSyncInfo::DwgFileId   fileId(1);
    DwgSyncInfo::ModelIterator  iter(*m_dgndb, "DwgFileId=? AND DwgInstanceId=?");
    iter.GetStatement()->BindInt (1, fileId.GetValue());
    iter.GetStatement()->BindInt64 (2, dwgModelId);

    out.Invalidate ();
    for (auto entry=iter.begin(); entry!=iter.end(); ++entry)
        {
        if (entry.GetSourceType() == type)
            {
            out = entry.GetDwgModelSyncInfoId ();
            break;
            }
        }
    ASSERT_TRUE (out.IsValid()) << "Model not found in syncInfo!";
    }


