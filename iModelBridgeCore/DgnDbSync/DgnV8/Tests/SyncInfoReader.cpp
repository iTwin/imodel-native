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
 void SyncInfoReader::MustFindFileByName(RepositoryLinkId& fileid, BentleyApi::BeFileNameCR v8FileName, int expectedCount)
    {
    SyncInfo::RepositoryLinkExternalSourceAspectIterator files(*m_dgndb, "fileName = :filename");
    files.GetStatement()->BindText(files.GetParameterIndex("filename"), BentleyApi::Utf8String(v8FileName).c_str(), BentleyApi::BeSQLite::EC::IECSqlBinder::MakeCopy::Yes);
    int count=0;
    for (SyncInfo::RepositoryLinkExternalSourceAspectIterator::Entry entry = files.begin(); entry != files.end(); ++entry)
        {
        ASSERT_STRCASEEQ( entry->GetFileName().c_str(), BentleyApi::Utf8String(v8FileName).c_str() ); 
        fileid = entry->GetRepositoryLinkId();
        ++count;
        }
    ASSERT_EQ( expectedCount, count );
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Sam.Wilson                      07/14
//---------------------------------------------------------------------------------------
void SyncInfoReader::MustFindModelByV8ModelId(DgnModelId& fmid, RepositoryLinkId ffid, DgnV8Api::ModelId v8ModelId, int expectedCount)
    {
    auto rlink = m_dgndb->Elements().Get<RepositoryLink>(ffid);

    SyncInfo::V8ModelExternalSourceAspectIteratorByV8Id models(*rlink, v8ModelId);
    int count=0;
    for (SyncInfo::V8ModelExternalSourceAspectIterator::Entry entry = models.begin(); entry != models.end(); ++entry)
        {
        fmid = entry->GetModelId();
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
void SyncInfoReader::MustFindElementByV8ElementId(DgnElementId& eid, DgnModelId fmid, DgnV8Api::ElementId v8ElementId, int expectedCount)
    {
    //Find the element id from aspect.
    auto model = m_dgndb->Models().GetModel(fmid);
    SyncInfo::V8ElementExternalSourceAspectIteratorByV8Id elements(*model, v8ElementId);
    int count=0;
    for (SyncInfo::V8ElementExternalSourceAspectIteratorByV8Id::Entry entry = elements.begin(); entry != elements.end(); ++entry) 
        {
        ++count;
        eid = entry->GetElementId();
        }
    ASSERT_EQ(expectedCount, count);

    if (m_params->GetWantProvenanceInBim())
        {
        if (1 == count)
            {
            auto estmt = m_dgndb->GetPreparedECSqlStatement("SELECT g.ECInstanceId FROM "
                          BIS_SCHEMA(BIS_CLASS_Element) " AS g,"
                          XTRN_SRC_ASPCT_FULLCLASSNAME " AS sourceInfo "
                          "WHERE (sourceInfo.Element.Id = g.ECInstanceId) AND ( CAST(sourceInfo.Identifier AS INTEGER) = ? )"
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
                          " WHERE (sourceInfo.Element.Id=g.ECInstanceId) AND ( CAST(sourceInfo.Identifier AS INTEGER) = ?)");
            estmt->BindInt64(1, v8ElementId);
            ASSERT_EQ(BE_SQLITE_ROW, estmt->Step());
            int aspectCount = estmt->GetValueId<int>(0);
            ASSERT_EQ(expectedCount, aspectCount);
            }
        }
    }
