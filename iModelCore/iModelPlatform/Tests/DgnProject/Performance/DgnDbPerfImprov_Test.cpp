#include "../TestFixture/DgnDbTestFixtures.h"
#include <UnitTests/BackDoor/DgnPlatform/DgnDbTestUtils.h>
/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
class DgnDbPerfImprovTest : public DgnDbTestFixture
    {
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(DgnDbPerfImprovTest, GenerateEmptyIModel)
    {
    DgnDbPtr db;

    db = DgnDbTestUtils::OpenSeedDbCopy(L"D:\\imodel_schemas_full_time\\XL_Imodels_For_Perf_Testing\\0.bim", "ModifiableDb.bim");
    ASSERT_TRUE(db.IsValid()) << "Failed to create AutoHandledGeometryProps test dgndb";

    ECSqlStatement selectGeoEl3d;
    ASSERT_EQ(ECSqlStatus::Success, selectGeoEl3d.Prepare(*db, "SELECT ElementId, GeometryStream FROM bis.GeometricElement3d"));
    ECSqlStatement insertGeoEl3d;
    ASSERT_EQ(ECSqlStatus::Success, insertGeoEl3d.Prepare(*db, "INSERT INTO bis.GeometricElement3d (ElementId, GeometryStream) VALUES (?, ?)"));
    ECSqlStatement updateGeoEl3d;
    ASSERT_EQ(ECSqlStatus::Success, updateGeoEl3d.Prepare(*db, "UPDATE bis.GeometricElement3d SET GeometryStream = NULL WHERE ElementId = ?"));
    while(selectGeoEl3d.Step() == BE_SQLITE_ROW)
        {
        auto elementId = selectGeoEl3d.GetValueInt(0);
        int blobSize;
        void const* blobData = selectGeoEl3d.GetValueBlob(1, &blobSize);
        insertGeoEl3d.Reset();
        insertGeoEl3d.ClearBindings();
        insertGeoEl3d.BindInt(1, elementId);
        insertGeoEl3d.BindBlob(2, blobData, blobSize, IECSqlBinder::MakeCopy::No);
        ASSERT_EQ(BE_SQLITE_DONE, insertGeoEl3d.Step()) << "Failed to insert GeometricElement3d with ElementId " << elementId;
        updateGeoEl3d.Reset();
        updateGeoEl3d.ClearBindings();
        updateGeoEl3d.BindInt(1, elementId);
        ASSERT_EQ(BE_SQLITE_DONE, updateGeoEl3d.Step()) << "Failed to update GeometricElement3d with ElementId " << elementId;
        }
    }