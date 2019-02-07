/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnV8/Tests/GeomPartTests.cpp $
|
|  $Copyright: (c) 2019 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ConverterTestsBaseFixture.h"

//----------------------------------------------------------------------------------------
// @bsiclass                                    Umar.Hayat                      08/15
//----------------------------------------------------------------------------------------
struct GeomPartTests : public ConverterTestBaseFixture
    {
    int GetGeomPartAspectCount(DgnDbR);
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      01/19
+---------------+---------------+---------------+---------------+---------------+------*/
static int getGeomPartCount(DgnDbR db)
    {
    auto sel = db.GetPreparedECSqlStatement("SELECT COUNT(*) from " BIS_SCHEMA(BIS_CLASS_GeometryPart));
    sel->Step();
    return sel->GetValueInt(0);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      01/19
+---------------+---------------+---------------+---------------+---------------+------*/
int GeomPartTests::GetGeomPartAspectCount(DgnDbR db)
    {
    if (!m_params.GetWantProvenanceInBim())
        {
        return 0;
        }
    auto sel = db.GetPreparedECSqlStatement("SELECT COUNT(*) from " XTRN_SRC_ASPCT_FULLCLASSNAME " WHERE (Kind=?)");
    sel->BindText(1, SyncInfo::ExternalSourceAspect::Kind::GeomPart, EC::IECSqlBinder::MakeCopy::No);
    sel->Step();
    return sel->GetValueInt(0);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      01/19
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(GeomPartTests, NormalCells)
    {
    const     bool useAspects = m_params.GetWantProvenanceInBim();
    constexpr int expectedGeomPartCount = 16;
    const     int expectedGeomPartAspectCount = useAspects? 16: 0;
    constexpr int expectedRefGeomPartCount = 13;
    const     int expectedRefGeomPartAspectCount = useAspects? 3: 0; // of the 13, only 3 are (shared) cells. The rest are LineStyles. We don't (yet) create aspects for them.

    LineUpFiles(L"chair_array.bim", L"chair_array.dgn", true);
    
    DgnDbPtr db = OpenExistingDgnDb(m_dgnDbFileName);
    ASSERT_EQ(expectedGeomPartCount, getGeomPartCount(*db));
    ASSERT_EQ(expectedGeomPartAspectCount, GetGeomPartAspectCount(*db));
    db->CloseDb();
    db = nullptr;
    
    // Do an update with no changes, verifying that the counts are unchanged
    DoUpdate(m_dgnDbFileName, m_v8FileName, false, false);

    db = OpenExistingDgnDb(m_dgnDbFileName);
    ASSERT_EQ(expectedGeomPartCount, getGeomPartCount(*db));
    ASSERT_EQ(expectedGeomPartAspectCount, GetGeomPartAspectCount(*db));
    db->CloseDb();
    db = nullptr;

    //  Add HalfScaleSCOverride1.dgn as an attachment.
        {
        V8FileEditor v8editor;
        v8editor.Open(m_v8FileName);
        Bentley::DgnDocumentMonikerPtr moniker = DgnV8Api::DgnDocumentMoniker::CreateFromFileName(GetInputFileName(L"HalfScaleSCOverride1.dgn").c_str());
        DgnV8Api::DgnAttachment* attachment;
        ASSERT_EQ(BentleyApi::SUCCESS, v8editor.m_defaultModel->CreateDgnAttachment(attachment, *moniker, L""));
        ASSERT_EQ(BentleyApi::SUCCESS, attachment->WriteToModel());
        v8editor.Save();
        }
    //  ... and do an update, verifying that the expected number of geomparts and aspects were added
    DoUpdate(m_dgnDbFileName, m_v8FileName, false, true);

    db = OpenExistingDgnDb(m_dgnDbFileName);
    ASSERT_EQ(expectedGeomPartCount + expectedRefGeomPartCount, getGeomPartCount(*db));
    ASSERT_EQ(expectedGeomPartAspectCount + expectedRefGeomPartAspectCount, GetGeomPartAspectCount(*db));
    db->CloseDb();
    db = nullptr;

    // Do another update with no changes, verifying that the counts are unchanged
    DoUpdate(m_dgnDbFileName, m_v8FileName, false, false);

    db = OpenExistingDgnDb(m_dgnDbFileName);
    ASSERT_EQ(expectedGeomPartCount + expectedRefGeomPartCount, getGeomPartCount(*db));
    ASSERT_EQ(expectedGeomPartAspectCount + expectedRefGeomPartAspectCount, GetGeomPartAspectCount(*db));
    db->CloseDb();
    db = nullptr;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      01/19
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(GeomPartTests, SharedCells)
    {
    const     bool useAspects = m_params.GetWantProvenanceInBim();
    constexpr int expectedGeomPartCount = 13;
    const     int expectedGeomPartAspectCount = useAspects? 3: 0; // of the 13, only 3 are (shared) cells. The rest are LineStyles. We don't (yet) create aspects for them.

    LineUpFiles(L"HalfScaleSCOverride1.bim", L"HalfScaleSCOverride1.dgn", true);

    DgnDbPtr db = OpenExistingDgnDb(m_dgnDbFileName);
    ASSERT_EQ(expectedGeomPartCount, getGeomPartCount(*db));
    ASSERT_EQ(expectedGeomPartAspectCount, GetGeomPartAspectCount(*db));

    db->CloseDb();
    db = nullptr;
    DoUpdate(m_dgnDbFileName, m_v8FileName, false, false);

    db = OpenExistingDgnDb(m_dgnDbFileName);
    ASSERT_EQ(expectedGeomPartCount, getGeomPartCount(*db));
    ASSERT_EQ(expectedGeomPartAspectCount, GetGeomPartAspectCount(*db));
    }
