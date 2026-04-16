/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
//
// PLAN / PROTOTYPE OVERVIEW
// ─────────────────────────
// DgnDb profile 2.0.0.8 introduces a "bis_GeometryStream" side-table:
//
//   CREATE TABLE bis_GeometryStream (
//       Id      INTEGER PRIMARY KEY,   -- ECInstanceId of the owning element/part
//       ClassId INTEGER NOT NULL,      -- ECClassId of the owning element/part
//       GeomStream BLOB NOT NULL       -- snappy-compressed geometry stream bytes
//   );
//
// Motivation: decoupling the large geometry blob from the core EC class tables
// (bis_GeometricElement3d, bis_GeometricElement2d, bis_GeometryPart) improves
// performance for queries that only need placement/metadata columns.
//
// WRITE PATH (profile >= 2.0.0.8)
//   After the normal ECSql INSERT/UPDATE of the element, `GeometryStream::MirrorToSideTable`
//   copies the blob that was just written to the EC table into bis_GeometryStream.
//   The original column is kept intact (dual-write) to remain safe/reversible.
//
// READ PATH (profile >= 2.0.0.8)
//   `_ReadSelectParams` for GeometricElement (3d/2d) and DgnGeometryPart checks the
//   profile version. If >= 2.0.0.8 it calls `GeometryStream::ReadFromSideTable` which
//   does a direct SELECT on bis_GeometryStream instead of consuming the ECSql blob column.
//
// PROFILE UPGRADE (old DB opened with new code)
//   `DgnDb::_UpgradeProfile` detects versionBeforeUpgrade < 2.0.0.8 and calls
//   `DgnDb::CreateAndPopulateGeomStreamSideTable` which:
//     1. CREATE TABLE IF NOT EXISTS bis_GeometryStream ...
//     2. INSERT OR IGNORE ... SELECT from bis_GeometricElement3d where GeometryStream IS NOT NULL
//     3. Same for bis_GeometricElement2d and bis_GeometryPart.
//
// TESTS IN THIS FILE
//   - ProfileVersionIs_2_0_0_8       : newly created DB reports the new version.
//   - NewProfile_Element_WriteToSideTable : insert PhysicalElement → side table has entry.
//   - NewProfile_Element_ReadFromSideTable: close/reopen DB, verify geometry can be read back.
//   - NewProfile_GeometryPart_SideTable   : same pair of checks for DgnGeometryPart.
//   - ManualMigration_PopulatesSideTable  : drop side table, call upgrade-level migration helper,
//                                           verify entries are restored from old columns.
//
// NOTE: The ManualMigration test requires a private-method back-door that is not yet wired; it is
// left as a future TODO and its body is guarded by GEOMSTREAM_MIGRATION_TEST_ENABLED.
//---------------------------------------------------------------------------------------------
#include "../TestFixture/DgnDbTestFixtures.h"
#include "../TestFixture/GeomHelper.h"
#include <UnitTests/BackDoor/DgnPlatform/DgnDbTestUtils.h>

USING_NAMESPACE_BENTLEY_DGN
USING_NAMESPACE_BENTLEY_SQLITE
USING_NAMESPACE_BENTLEY_SQLITE_EC
USING_NAMESPACE_BENTLEY_DPTEST

//----------------------------------------------------------------------------------------
// Helpers
//----------------------------------------------------------------------------------------

//! Returns the number of rows in bis_GeometryStream for the given element Id.
static int CountSideTableRows(DgnDbR db, DgnElementId elemId)
    {
    CachedStatementPtr stmt = db.GetCachedStatement(
        "SELECT count(*) FROM " BIS_GEOMSTREAM_SIDE_TABLE " WHERE Id=?");
    if (!stmt.IsValid())
        return -1;
    stmt->BindId(1, elemId);
    return (BE_SQLITE_ROW == stmt->Step()) ? stmt->GetValueInt(0) : -1;
    }

//! Returns true when the side table itself exists in the schema.
static bool SideTableExists(DgnDbR db)
    {
    CachedStatementPtr stmt = db.GetCachedStatement(
        "SELECT count(*) FROM sqlite_master WHERE type='table' AND name='" BIS_GEOMSTREAM_SIDE_TABLE "'");
    if (!stmt.IsValid())
        return false;
    return (BE_SQLITE_ROW == stmt->Step()) && (stmt->GetValueInt(0) > 0);
    }

//! Insert a GenericPhysicalObject with cone geometry and return its id.
static DgnElementId InsertPhysicalObjectWithGeometry(DgnDbR db, DgnModelId modelId, DgnCategoryId catId)
    {
    DgnModelPtr model = db.Models().GetModel(modelId);
    if (!model.IsValid()) return DgnElementId();

    GeometryBuilderPtr builder = GeometryBuilder::Create(*model, catId, DPoint3d::FromZero());
    if (!builder.IsValid()) return DgnElementId();

    DgnConeDetail cone(DPoint3d::FromZero(), DPoint3d::From(0,0,2), 0.5, 0.5, true);
    ISolidPrimitivePtr prim = ISolidPrimitive::CreateDgnCone(cone);
    if (builder->Append(*prim) != SUCCESS) return DgnElementId();

    GenericPhysicalObjectPtr phys = GenericPhysicalObject::Create(*model, catId);
    if (!phys.IsValid()) return DgnElementId();

    if (DgnDbStatus::Success != builder->Finish(*phys)) return DgnElementId();
    if (!phys->GetGeometryStream().HasGeometry()) return DgnElementId();

    DgnElementCPtr inserted = phys->Insert();
    return inserted.IsValid() ? inserted->GetElementId() : DgnElementId();
    }

//======================================================================================
struct GeomStreamSideTableFixture : public DgnDbTestFixture
    {
    };

//======================================================================================
// Verify the profile version of a freshly created DgnDb is 2.0.0.8.
//======================================================================================
TEST_F(GeomStreamSideTableFixture, ProfileVersionIs_2_0_0_8)
    {
    SetupSeedProject();
    DgnDbProfileVersion ver = m_db->GetProfileVersion();
    EXPECT_EQ(2,  ver.GetMajor());
    EXPECT_EQ(0,  ver.GetMinor());
    EXPECT_EQ(0,  ver.GetSub1());
    EXPECT_EQ((uint16_t)DGNDB_GEOMSTREAM_TABLE_VERSION_Sub2, ver.GetSub2())
        << "Expected profile 2.0.0." << DGNDB_GEOMSTREAM_TABLE_VERSION_Sub2;
    }

//======================================================================================
// Insert a PhysicalElement with geometry and verify the bis_GeometryStream side table
// receives a mirrored row.
//======================================================================================
TEST_F(GeomStreamSideTableFixture, NewProfile_Element_WriteToSideTable)
    {
    SetupSeedProject();
    ASSERT_TRUE(SideTableExists(*m_db))
        << "bis_GeometryStream side table must be created during DgnDb initialization";

    DgnElementId geomElemId = InsertPhysicalObjectWithGeometry(*m_db, m_defaultModelId, m_defaultCategoryId);
    ASSERT_TRUE(geomElemId.IsValid()) << "Failed to insert physical object with geometry";

    // Side-table must contain exactly one row for this element.
    EXPECT_EQ(1, CountSideTableRows(*m_db, geomElemId))
        << "GeomStream data was not mirrored to bis_GeometryStream";

    m_db->SaveChanges();
    }

//======================================================================================
// After a close/reopen cycle the geometry must still be readable from the side table.
//======================================================================================
TEST_F(GeomStreamSideTableFixture, NewProfile_Element_ReadFromSideTable)
    {
    SetupSeedProject();
    DgnElementId geomElemId = InsertPhysicalObjectWithGeometry(*m_db, m_defaultModelId, m_defaultCategoryId);
    ASSERT_TRUE(geomElemId.IsValid());

    // Remember geometry size to compare after round-trip.
    uint32_t expectedGeomSize = 0;
    {
    DgnElementCPtr el = m_db->Elements().GetElement(geomElemId);
    ASSERT_TRUE(el.IsValid());
    ASSERT_TRUE(el->ToGeometrySource3d() != nullptr);
    expectedGeomSize = el->ToGeometrySource3d()->GetGeometryStream().GetSize();
    ASSERT_GT(expectedGeomSize, 0u);
    }

    BeFileName fileName = m_db->GetFileName();
    m_db->SaveChanges();
    m_db->CloseDb();
    m_db = nullptr;

    // Reopen in ReadOnly to verify the read path uses the side table.
    OpenDb(m_db, fileName, Db::OpenMode::Readonly, true);
    ASSERT_TRUE(m_db.IsValid());

    // Profile must still report 2.0.0.8.
    EXPECT_EQ((uint16_t)DGNDB_GEOMSTREAM_TABLE_VERSION_Sub2, m_db->GetProfileVersion().GetSub2());

    DgnElementCPtr readBack = m_db->Elements().GetElement(geomElemId);
    ASSERT_TRUE(readBack.IsValid()) << "Element not found after reopen";
    GeometrySource3dCP geomSrc = readBack->ToGeometrySource3d();
    ASSERT_TRUE(geomSrc != nullptr);
    EXPECT_TRUE(geomSrc->GetGeometryStream().HasGeometry())
        << "Geometry was lost after close/reopen (side table read failed)";
    EXPECT_EQ(expectedGeomSize, geomSrc->GetGeometryStream().GetSize())
        << "Geometry stream size changed after round-trip";
    }

//======================================================================================
// Insert a DgnGeometryPart and verify the side table receives a mirrored row, and that
// the geometry can be retrieved correctly after a close/reopen cycle.
//======================================================================================
TEST_F(GeomStreamSideTableFixture, NewProfile_GeometryPart_SideTable)
    {
    SetupSeedProject();
    ASSERT_TRUE(SideTableExists(*m_db));

    DefinitionModelR defModel = m_db->GetDictionaryModel();

    // Create and insert a GeometryPart.
    GeometricPrimitivePtr prim = GeometricPrimitive::Create(*GeomHelper::computeShape());
    GeometryBuilderPtr builder = GeometryBuilder::CreateGeometryPart(*m_db, true);
    ASSERT_TRUE(builder.IsValid());
    ASSERT_EQ(SUCCESS, builder->Append(*prim));
    DgnGeometryPartPtr part = DgnGeometryPart::Create(defModel, "SideTableTestPart");
    ASSERT_TRUE(part.IsValid());
    ASSERT_EQ(DgnDbStatus::Success, builder->Finish(*part));
    ASSERT_TRUE(part->GetGeometryStream().HasGeometry());
    uint32_t expectedSize = part->GetGeometryStream().GetSize();

    DgnGeometryPartCPtr inserted = m_db->Elements().Insert<DgnGeometryPart>(*part);
    ASSERT_TRUE(inserted.IsValid()) << "GeometryPart insert failed";
    DgnElementId partId = inserted->GetId();

    // Side table must have the entry.
    EXPECT_EQ(1, CountSideTableRows(*m_db, partId))
        << "GeometryPart GeomStream was not mirrored to bis_GeometryStream";

    // Close and reopen; verify round-trip.
    BeFileName fileName = m_db->GetFileName();
    m_db->SaveChanges();
    m_db->CloseDb();
    m_db = nullptr;

    OpenDb(m_db, fileName, Db::OpenMode::Readonly, true);
    ASSERT_TRUE(m_db.IsValid());

    DgnGeometryPartCPtr readPart = m_db->Elements().Get<DgnGeometryPart>(partId);
    ASSERT_TRUE(readPart.IsValid()) << "GeometryPart not found after reopen";
    EXPECT_TRUE(readPart->GetGeometryStream().HasGeometry())
        << "GeometryPart geometry was lost after close/reopen";
    EXPECT_EQ(expectedSize, readPart->GetGeometryStream().GetSize())
        << "GeometryPart geometry stream size changed after round-trip";
    }

//======================================================================================
// Simulate the profile upgrade scenario: downgrade the stored version to 2.0.0.7 and
// verify that reopening the DB (which triggers _UpgradeProfile) creates the side table
// and migrates the existing geometry data.
//======================================================================================
TEST_F(GeomStreamSideTableFixture, UpgradeFrom_2_0_0_7_MigratesSideTable)
    {
    SetupSeedProject();

    // Insert a physical element with geometry and a GeometryPart.
    DgnElementId geomElemId = InsertPhysicalObjectWithGeometry(*m_db, m_defaultModelId, m_defaultCategoryId);
    ASSERT_TRUE(geomElemId.IsValid());

    GeometricPrimitivePtr prim = GeometricPrimitive::Create(*GeomHelper::computeShape());
    GeometryBuilderPtr builder = GeometryBuilder::CreateGeometryPart(*m_db, true);
    ASSERT_TRUE(builder.IsValid());
    DgnGeometryPartPtr part = DgnGeometryPart::Create(m_db->GetDictionaryModel(), "SideTableMigrationPart");
    ASSERT_TRUE(part.IsValid());
    ASSERT_EQ(SUCCESS, builder->Append(*prim));
    ASSERT_EQ(DgnDbStatus::Success, builder->Finish(*part));
    DgnGeometryPartCPtr insertedPart = m_db->Elements().Insert<DgnGeometryPart>(*part);
    ASSERT_TRUE(insertedPart.IsValid());
    DgnElementId partId = insertedPart->GetId();

    // Verify side table has entries for both (written via new-format write path).
    EXPECT_EQ(1, CountSideTableRows(*m_db, geomElemId));
    EXPECT_EQ(1, CountSideTableRows(*m_db, partId));

    // Now manually downgrade the stored profile version to 2.0.0.7 to simulate an old DB.
    // Then drop the side table to simulate a truly old file that never had it.
    DgnDbProfileVersion oldVersion(2, 0, 0, 7);
    ASSERT_EQ(BE_SQLITE_OK,
        m_db->SavePropertyString(DgnProjectProperty::ProfileVersion(), oldVersion.ToJson().c_str()));
    ASSERT_EQ(BE_SQLITE_OK,
        m_db->ExecuteSql("DROP TABLE IF EXISTS " BIS_GEOMSTREAM_SIDE_TABLE));

    ASSERT_FALSE(SideTableExists(*m_db)) << "Side table should have been dropped";

    BeFileName fileName = m_db->GetFileName();
    m_db->SaveChanges();
    m_db->CloseDb();
    m_db = nullptr;

    // Reopen in ReadWrite. DgnDb::_UpgradeProfile should detect version < 2.0.0.8,
    // call CreateAndPopulateGeomStreamSideTable, and bump the version.
    OpenDb(m_db, fileName, Db::OpenMode::ReadWrite, /*needBriefcase=*/true);
    ASSERT_TRUE(m_db.IsValid());

    // Profile must now be the current version (2.0.0.8).
    EXPECT_EQ((uint16_t)DGNDB_GEOMSTREAM_TABLE_VERSION_Sub2, m_db->GetProfileVersion().GetSub2())
        << "Profile must be bumped to 2.0.0.8 after upgrade";

    // Side table must have been created and populated by the upgrade.
    ASSERT_TRUE(SideTableExists(*m_db)) << "Side table must exist after upgrade";
    EXPECT_EQ(1, CountSideTableRows(*m_db, geomElemId))
        << "Element GeomStream not migrated to side table during upgrade";
    EXPECT_EQ(1, CountSideTableRows(*m_db, partId))
        << "GeometryPart GeomStream not migrated to side table during upgrade";

    // Elements must still be readable with geometry intact.
    DgnElementCPtr readEl = m_db->Elements().GetElement(geomElemId);
    ASSERT_TRUE(readEl.IsValid());
    EXPECT_TRUE(readEl->ToGeometrySource3d()->GetGeometryStream().HasGeometry())
        << "Geometry lost after upgrade from 2.0.0.7";

    DgnGeometryPartCPtr readPart = m_db->Elements().Get<DgnGeometryPart>(partId);
    ASSERT_TRUE(readPart.IsValid());
    EXPECT_TRUE(readPart->GetGeometryStream().HasGeometry())
        << "GeometryPart geometry lost after upgrade from 2.0.0.7";
    }
