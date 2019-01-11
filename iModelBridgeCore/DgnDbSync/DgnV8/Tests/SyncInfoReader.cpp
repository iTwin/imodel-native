/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnV8/Tests/SyncInfoReader.cpp $
|
|  $Copyright: (c) 2019 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "Tests.h"

//---------------------------------------------------------------------------------------
// @bsimethod                                    Sam.Wilson                      07/14
//---------------------------------------------------------------------------------------
SyncInfoReader::SyncInfoReader(Converter::Params& p) : m_params(&p)
    {
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
 void SyncInfoReader::MustFindFileByName(SyncInfo::V8FileSyncInfoId& fileid, BentleyApi::BeFileNameCR v8FileName, int expectedCount)
    {
    SyncInfo::FileIterator files(*m_dgndb, "V8Name=?");
    files.GetStatement()->BindText(1, BentleyApi::Utf8String(v8FileName), BentleyApi::BeSQLite::Statement::MakeCopy::Yes);
    int count=0;
    for (SyncInfo::FileIterator::Entry entry = files.begin(); entry != files.end(); ++entry)
        {
        ASSERT_STRCASEEQ( entry.GetV8Name().c_str(), BentleyApi::Utf8String(v8FileName).c_str() ); 
        fileid = entry.GetV8FileSyncInfoId();
        ++count;
        }
    ASSERT_EQ( expectedCount, count );
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Sam.Wilson                      07/14
//---------------------------------------------------------------------------------------
void SyncInfoReader::MustFindModelByV8ModelId(SyncInfo::V8ModelSyncInfoId& fmid, SyncInfo::V8FileSyncInfoId ffid, DgnV8Api::ModelId v8ModelId, int expectedCount)
    {
    SyncInfo::ModelIterator models(*m_dgndb, "WHERE V8FileSyncInfoId=? AND V8Id=?");
    models.GetStatement()->BindInt(1, ffid.GetValue());
    models.GetStatement()->BindInt(2, v8ModelId);
    int count=0;
    for (SyncInfo::ModelIterator::Entry entry = models.begin(); entry != models.end(); ++entry)
        {
        fmid = entry.GetV8ModelSyncInfoId();
        ++count;
        }
    ASSERT_EQ( expectedCount, count );

    if (m_params->GetWantProvenanceInBim())
        {
        // *** TODO: Use the aspect to look up model by v8id
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Sam.Wilson                      07/14
//---------------------------------------------------------------------------------------
void SyncInfoReader::MustFindElementByV8ElementId(DgnElementId& eid, SyncInfo::V8ModelSyncInfoId fmid, DgnV8Api::ElementId v8ElementId, int expectedCount)
    {

    //Find the element id from aspect.
    SyncInfo::ByV8ElementIdIter elements(*m_dgndb);
    elements.Bind(fmid, v8ElementId);
    int count=0;
    for (SyncInfo::ByV8ElementIdIter::Entry entry = elements.begin(); entry != elements.end(); ++entry) 
        {
        ++count;
        eid = entry.GetElementId();
        }
    ASSERT_EQ(expectedCount, count);

    if (m_params->GetWantProvenanceInBim())
        {
        if (1 == count)
            {
            auto estmt = m_dgndb->GetPreparedECSqlStatement("SELECT g.ECInstanceId FROM "
                          BIS_SCHEMA(BIS_CLASS_Element) " AS g,"
                          XTRN_SRC_ASPCT_FULLCLASSNAME " AS sourceInfo "
                          "WHERE (sourceInfo.Element.Id = g.ECInstanceId) AND ( CAST(sourceInfo.SourceId AS INTEGER) = ? )"
                            );
                     
            estmt->BindInt64 (1, v8ElementId);
            ASSERT_EQ(BE_SQLITE_ROW, estmt->Step());
            DgnElementId aspectId = estmt->GetValueId<DgnElementId>(0);
            ASSERT_EQ(eid , aspectId);
            }
        else 
            {
            auto estmt = m_dgndb->GetPreparedECSqlStatement("SELECT COUNT (*) FROM "
                          BIS_SCHEMA(BIS_CLASS_Element) " AS g,"
                          XTRN_SRC_ASPCT_FULLCLASSNAME " AS sourceInfo"
                          " WHERE (sourceInfo.Element.Id=g.ECInstanceId) AND ( CAST(sourceInfo.SourceId AS INTEGER) = ?)");
            estmt->BindInt64(1, v8ElementId);
            ASSERT_EQ(BE_SQLITE_ROW, estmt->Step());
            int aspectCount = estmt->GetValueId<int>(0);
            ASSERT_EQ(expectedCount, aspectCount);
            }
        }
    }
