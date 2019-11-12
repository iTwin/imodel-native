/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "Tests.h"

//---------------------------------------------------------------------------------------
// @bsimethod                                    Sam.Wilson                      07/14
//---------------------------------------------------------------------------------------
SyncInfoReader::SyncInfoReader(Converter::Params& p, DgnDbPtr db) : m_params(&p), m_dgndb(db)
    {
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Sam.Wilson                      07/14
//---------------------------------------------------------------------------------------
 void SyncInfoReader::MustFindFileByName(RepositoryLinkId& fileid, BentleyApi::BeFileNameCR v8FileNameIn, int expectedCount)
    {
    BentleyApi::Utf8String v8FileName(v8FileNameIn);

    SyncInfo::RepositoryLinkExternalSourceAspectIterator files(*m_dgndb);
    int count=0;
    for (SyncInfo::RepositoryLinkExternalSourceAspectIterator::Entry entry = files.begin(); entry != files.end(); ++entry)
        {
        if (entry->GetFileName().EqualsI(v8FileName))
            {
            fileid = entry->GetRepositoryLinkId();
            ++count;
            }
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
    }
