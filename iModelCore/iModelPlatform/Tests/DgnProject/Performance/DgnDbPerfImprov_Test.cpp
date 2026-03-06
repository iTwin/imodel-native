#include "../TestFixture/DgnDbTestFixtures.h"
#include <UnitTests/BackDoor/DgnPlatform/DgnDbTestUtils.h>
#include <DgnPlatform/PlatformLib.h>
/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
class DgnDbPerfImprovTest : public DgnDbTestFixture
    {
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(DgnDbPerfImprovTest, TestOn4gbFile)
    {
    DgnDbPtr db;
    db = DgnDbTestUtils::OpenSeedDbCopy(L"22.bim", L"Modifiable4GbDb.bim");
    ASSERT_TRUE(db.IsValid()) << "Failed to create AutoHandledGeometryProps test dgndb";
    
    db->CreateTable("bis_GeometryStream", "ElementId INTEGER PRIMARY KEY,"
                                "GeometryStream BLOB");
    Statement selectGeoEl3d;
    ASSERT_EQ(BE_SQLITE_OK, selectGeoEl3d.Prepare(*db, "SELECT ECInstanceId, GeometryStream FROM " BIS_TABLE(BIS_CLASS_GeometricElement3d)));
    Statement insertGeoEl3d;
    ASSERT_EQ(BE_SQLITE_OK, insertGeoEl3d.Prepare(*db, "INSERT INTO bis_GeometryStream (ElementId, GeometryStream) VALUES (?, ?)"));
    Statement updateGeoEl3d;
    ASSERT_EQ(BE_SQLITE_OK, updateGeoEl3d.Prepare(*db, "UPDATE " BIS_TABLE(BIS_CLASS_GeometricElement3d) " SET GeometryStream = NULL WHERE ElementId = ?"));
    StopWatch timer(true);
    int count = 0;
    while(selectGeoEl3d.Step() == BE_SQLITE_ROW)
        {
        count++;
        auto elementId = selectGeoEl3d.GetValueInt64(0);
        void const* blobData = selectGeoEl3d.GetValueBlob(1);
        insertGeoEl3d.Reset();
        insertGeoEl3d.ClearBindings();
        insertGeoEl3d.BindInt64(1, elementId);
        insertGeoEl3d.BindBlob(2, blobData, sizeof(blobData), Statement::MakeCopy::No);
        ASSERT_EQ(BE_SQLITE_DONE, insertGeoEl3d.Step()) << "Failed to insert GeometricElement3d with ElementId " << elementId;
        updateGeoEl3d.Reset();
        updateGeoEl3d.ClearBindings();
        updateGeoEl3d.BindInt64(1, elementId);
        ASSERT_EQ(BE_SQLITE_DONE, updateGeoEl3d.Step()) << "Failed to update GeometricElement3d with ElementId " << elementId;
        }
    timer.Stop();
    selectGeoEl3d.Finalize();
    insertGeoEl3d.Finalize();
    updateGeoEl3d.Finalize();
    db->SaveChanges();
    db->CloseDb();
    PERFORMANCELOG.infov("Transformed %d geometry streams in %.4lf seconds", count, timer.GetElapsedSeconds());
    LOGTODB(TEST_FIXTURE_NAME, TEST_NAME, timer.GetElapsedSeconds(), count);
    }

    /*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(DgnDbPerfImprovTest, TestOn8gbFile)
    {
    DgnDbPtr db;
    db = DgnDbTestUtils::OpenSeedDbCopy(L"17703.bim", L"Modifiable8GbDb.bim");
    ASSERT_TRUE(db.IsValid()) << "Failed to create AutoHandledGeometryProps test dgndb";
    
    db->CreateTable("bis_GeometryStream", "ElementId INTEGER PRIMARY KEY,"
                                "GeometryStream BLOB");
    Statement selectGeoEl3d;
    ASSERT_EQ(BE_SQLITE_OK, selectGeoEl3d.Prepare(*db, "SELECT ECInstanceId, GeometryStream FROM " BIS_TABLE(BIS_CLASS_GeometricElement3d)));
    Statement insertGeoEl3d;
    ASSERT_EQ(BE_SQLITE_OK, insertGeoEl3d.Prepare(*db, "INSERT INTO bis_GeometryStream (ElementId, GeometryStream) VALUES (?, ?)"));
    Statement updateGeoEl3d;
    ASSERT_EQ(BE_SQLITE_OK, updateGeoEl3d.Prepare(*db, "UPDATE " BIS_TABLE(BIS_CLASS_GeometricElement3d) " SET GeometryStream = NULL WHERE ElementId = ?"));
    StopWatch timer(true);
    int count = 0;
    while(selectGeoEl3d.Step() == BE_SQLITE_ROW)
        {
        count++;
        auto elementId = selectGeoEl3d.GetValueInt64(0);
        void const* blobData = selectGeoEl3d.GetValueBlob(1);
        insertGeoEl3d.Reset();
        insertGeoEl3d.ClearBindings();
        insertGeoEl3d.BindInt64(1, elementId);
        insertGeoEl3d.BindBlob(2, blobData,  sizeof(blobData), Statement::MakeCopy::No);
        ASSERT_EQ(BE_SQLITE_DONE, insertGeoEl3d.Step()) << "Failed to insert GeometricElement3d with ElementId " << elementId;
        updateGeoEl3d.Reset();
        updateGeoEl3d.ClearBindings();
        updateGeoEl3d.BindInt64(1, elementId);
        ASSERT_EQ(BE_SQLITE_DONE, updateGeoEl3d.Step()) << "Failed to update GeometricElement3d with ElementId " << elementId;
        }
    timer.Stop();
    selectGeoEl3d.Finalize();
    insertGeoEl3d.Finalize();
    updateGeoEl3d.Finalize();
    db->SaveChanges();
    db->CloseDb();
    PERFORMANCELOG.infov("Transformed %d geometry streams in %.4lf seconds", count, timer.GetElapsedSeconds());
    LOGTODB(TEST_FIXTURE_NAME, TEST_NAME, timer.GetElapsedSeconds(), count);
    }