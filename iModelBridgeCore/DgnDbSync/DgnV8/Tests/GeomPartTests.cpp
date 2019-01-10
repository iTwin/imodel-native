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
static int getGeomPartAspectCount(DgnDbR db)
    {
    auto sel = db.GetPreparedECSqlStatement("SELECT COUNT(*) from " SOURCEINFO_ECSCHEMA_NAME "." SOURCEINFO_CLASS_SoureElementInfo " WHERE (Kind=?)");
    sel->BindText(1, SyncInfo::SyncInfoAspect::KindToString(SyncInfo::SyncInfoAspect::Kind::GeomPart), EC::IECSqlBinder::MakeCopy::No);
    sel->Step();
    return sel->GetValueInt(0);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      01/19
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(GeomPartTests, NormalCells)
    {
    constexpr int expectedGeomPartCount = 16;
    constexpr int expectedGeomPartAspectCount = 16;

    LineUpFiles(L"chair_array.bim", L"chair_array.dgn", true);
    
    DgnDbPtr db = OpenExistingDgnDb(m_dgnDbFileName);
    ASSERT_EQ(expectedGeomPartCount, getGeomPartCount(*db));
    ASSERT_EQ(expectedGeomPartAspectCount, getGeomPartAspectCount(*db));

    db->CloseDb();
    db = nullptr;
    DoUpdate(m_dgnDbFileName, m_v8FileName, false, false);

    db = OpenExistingDgnDb(m_dgnDbFileName);
    ASSERT_EQ(expectedGeomPartCount, getGeomPartCount(*db));
    ASSERT_EQ(expectedGeomPartAspectCount, getGeomPartAspectCount(*db));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      01/19
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(GeomPartTests, SharedCells)
    {
    constexpr int expectedGeomPartCount = 13;
    constexpr int expectedGeomPartAspectCount = 3; // of the 13, only 3 are (shared) cells. The rest are LineStyles. We don't (yet) create aspects for them.

    LineUpFiles(L"HalfScaleSCOverride1.bim", L"HalfScaleSCOverride1.dgn", true);

    DgnDbPtr db = OpenExistingDgnDb(m_dgnDbFileName);
    ASSERT_EQ(expectedGeomPartCount, getGeomPartCount(*db));
    ASSERT_EQ(expectedGeomPartAspectCount, getGeomPartAspectCount(*db));

    db->CloseDb();
    db = nullptr;
    DoUpdate(m_dgnDbFileName, m_v8FileName, false, false);

    db = OpenExistingDgnDb(m_dgnDbFileName);
    ASSERT_EQ(expectedGeomPartCount, getGeomPartCount(*db));
    ASSERT_EQ(expectedGeomPartAspectCount, getGeomPartAspectCount(*db));
    }
