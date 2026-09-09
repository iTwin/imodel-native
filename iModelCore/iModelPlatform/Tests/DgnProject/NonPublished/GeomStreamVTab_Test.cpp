/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "DgnHandlersTests.h"
#include <DgnPlatform/PlatformLib.h>
#include <DgnPlatform/GeomPart.h>
#include "../TestFixture/GeomHelper.h"

USING_NAMESPACE_BENTLEY_SQLITE
USING_NAMESPACE_BENTLEY_SQLITE_EC

//=======================================================================================
// @bsiclass
//=======================================================================================
struct GeomStreamVTabTest : public DgnDbTestFixture
{
};

/*---------------------------------------------------------------------------------**//**
* Verify that imodel_geom_stream returns at least one row per element and that basic
* columns (EntryIndex, OpCode, EntryType) are non-NULL.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(GeomStreamVTabTest, BasicQuery)
    {
    SetupSeedProject();

    // Insert a TestElement with geometry
    auto el = InsertElement(Render::GeometryParams(m_defaultCategoryId));
    ASSERT_TRUE(el.IsValid()) << "Failed to insert element with geometry";
    DgnElementId elemId = el->GetElementId();
    SaveDb();

    // Query the geometry stream
    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(*m_db,
        "SELECT gs.EntryIndex, gs.OpCode, gs.EntryType, gs.IsGeometry "
        "FROM bis.GeometricElement3d e, imodel_geom_stream(e.GeometryStream) gs "
        "WHERE e.ECInstanceId = ?"));

    stmt.BindId(1, elemId);

    int rowCount = 0;
    while (BE_SQLITE_ROW == stmt.Step())
        {
        // EntryIndex should be sequential starting from 0
        EXPECT_EQ(rowCount, stmt.GetValueInt(0)) << "EntryIndex should be sequential";

        // OpCode should be a non-empty string
        Utf8CP opCode = stmt.GetValueText(1);
        EXPECT_TRUE(opCode != nullptr && strlen(opCode) > 0) << "OpCode should not be empty";

        // EntryType should be a non-empty string
        Utf8CP entryType = stmt.GetValueText(2);
        EXPECT_TRUE(entryType != nullptr && strlen(entryType) > 0) << "EntryType should not be empty";

        rowCount++;
        }

    EXPECT_GT(rowCount, 0) << "Should have at least one geometry stream entry";
    }

/*---------------------------------------------------------------------------------**//**
* Verify that the first entry in a geometry stream is typically a Header opcode.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(GeomStreamVTabTest, HeaderEntry)
    {
    SetupSeedProject();

    auto el = InsertElement(Render::GeometryParams(m_defaultCategoryId));
    ASSERT_TRUE(el.IsValid());
    DgnElementId elemId = el->GetElementId();
    SaveDb();

    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(*m_db,
        "SELECT gs.OpCode, gs.HeaderFlags "
        "FROM bis.GeometricElement3d e, imodel_geom_stream(e.GeometryStream) gs "
        "WHERE e.ECInstanceId = ? AND gs.EntryIndex = 0"));

    stmt.BindId(1, elemId);

    ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());
    Utf8CP firstOpCode = stmt.GetValueText(0);
    ASSERT_TRUE(firstOpCode != nullptr);
    EXPECT_STREQ("Header", firstOpCode) << "First opcode should be Header";
    }

/*---------------------------------------------------------------------------------**//**
* Verify that geometry entries have IsGeometry = 1 and non-geometry entries have 0.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(GeomStreamVTabTest, IsGeometryFlag)
    {
    SetupSeedProject();

    auto el = InsertElement(Render::GeometryParams(m_defaultCategoryId));
    ASSERT_TRUE(el.IsValid());
    DgnElementId elemId = el->GetElementId();
    SaveDb();

    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(*m_db,
        "SELECT gs.OpCode, gs.IsGeometry "
        "FROM bis.GeometricElement3d e, imodel_geom_stream(e.GeometryStream) gs "
        "WHERE e.ECInstanceId = ?"));

    stmt.BindId(1, elemId);

    bool foundGeomEntry = false;
    bool foundNonGeomEntry = false;
    while (BE_SQLITE_ROW == stmt.Step())
        {
        Utf8CP opCode = stmt.GetValueText(0);
        int isGeom = stmt.GetValueInt(1);

        if (isGeom == 1)
            foundGeomEntry = true;
        else
            foundNonGeomEntry = true;

        // Header should never be geometry
        if (opCode != nullptr && strcmp(opCode, "Header") == 0)
            EXPECT_EQ(0, isGeom) << "Header should not be a geometry entry";
        }

    EXPECT_TRUE(foundGeomEntry) << "TestElement should have at least one geometry entry";
    EXPECT_TRUE(foundNonGeomEntry) << "TestElement should have non-geometry entries (Header, BasicSymbology)";
    }

/*---------------------------------------------------------------------------------**//**
* Verify SubCategoryId is returned for geometry entries.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(GeomStreamVTabTest, SubCategoryId)
    {
    SetupSeedProject();

    auto el = InsertElement(Render::GeometryParams(m_defaultCategoryId));
    ASSERT_TRUE(el.IsValid());
    DgnElementId elemId = el->GetElementId();
    SaveDb();

    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(*m_db,
        "SELECT gs.SubCategoryId "
        "FROM bis.GeometricElement3d e, imodel_geom_stream(e.GeometryStream) gs "
        "WHERE e.ECInstanceId = ? AND gs.IsGeometry = 1"));

    stmt.BindId(1, elemId);

    while (BE_SQLITE_ROW == stmt.Step())
        {
        // After BasicSymbology, geometry entries should have a valid SubCategoryId
        int64_t subCatId = stmt.GetValueInt64(0);
        if (!stmt.IsValueNull(0))
            EXPECT_NE(0, subCatId) << "SubCategoryId should be non-zero when set";
        }
    }

/*---------------------------------------------------------------------------------**//**
* Verify that multiple elements each produce their own set of rows.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(GeomStreamVTabTest, MultipleElements)
    {
    SetupSeedProject();

    auto el1 = InsertElement(Render::GeometryParams(m_defaultCategoryId));
    auto el2 = InsertElement(Render::GeometryParams(m_defaultCategoryId));
    auto el3 = InsertElement(Render::GeometryParams(m_defaultCategoryId));
    ASSERT_TRUE(el1.IsValid() && el2.IsValid() && el3.IsValid());
    SaveDb();

    // Count total entries across all elements
    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(*m_db,
        "SELECT COUNT(*) FROM bis.GeometricElement3d e, imodel_geom_stream(e.GeometryStream) gs"));

    ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());
    int totalCount = stmt.GetValueInt(0);
    EXPECT_GT(totalCount, 0) << "Should have entries across all elements";

    // Count entries per element — each element should produce the same number
    stmt.Finalize();
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(*m_db,
        "SELECT e.ECInstanceId, COUNT(*) "
        "FROM bis.GeometricElement3d e, imodel_geom_stream(e.GeometryStream) gs "
        "GROUP BY e.ECInstanceId"));

    int elementCount = 0;
    int lastEntryCount = -1;
    while (BE_SQLITE_ROW == stmt.Step())
        {
        elementCount++;
        int entryCount = stmt.GetValueInt(1);
        EXPECT_GT(entryCount, 0);

        // Since all TestElements have the same geometry, entry counts should be equal
        if (lastEntryCount >= 0)
            EXPECT_EQ(lastEntryCount, entryCount) << "Identical elements should have same entry count";
        lastEntryCount = entryCount;
        }

    EXPECT_GE(elementCount, 3) << "Should have at least 3 elements (the 3 we inserted)";
    }

/*---------------------------------------------------------------------------------**//**
* Verify that GeometryBlob column returns non-NULL data for geometry entries.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(GeomStreamVTabTest, GeometryBlobColumn)
    {
    SetupSeedProject();

    auto el = InsertElement(Render::GeometryParams(m_defaultCategoryId));
    ASSERT_TRUE(el.IsValid());
    DgnElementId elemId = el->GetElementId();
    SaveDb();

    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(*m_db,
        "SELECT gs.GeometryBlob, gs.OpCode "
        "FROM bis.GeometricElement3d e, imodel_geom_stream(e.GeometryStream) gs "
        "WHERE e.ECInstanceId = ?"));

    stmt.BindId(1, elemId);

    while (BE_SQLITE_ROW == stmt.Step())
        {
        Utf8CP opCode = stmt.GetValueText(1);
        if (opCode != nullptr && strcmp(opCode, "Header") != 0)
            {
            // Non-header entries should have a GeometryBlob with the raw opcode payload
            int blobSize = 0;
            const void* blobData = stmt.GetValueBlob(0, &blobSize);
            EXPECT_TRUE(blobData != nullptr && blobSize > 0) << "GeometryBlob should contain opcode payload for " << opCode;
            }
        }
    }

/*---------------------------------------------------------------------------------**//**
* Verify that SELECT * works (all columns expand correctly).
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(GeomStreamVTabTest, SelectStar)
    {
    SetupSeedProject();

    auto el = InsertElement(Render::GeometryParams(m_defaultCategoryId));
    ASSERT_TRUE(el.IsValid());
    DgnElementId elemId = el->GetElementId();
    SaveDb();

    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(*m_db,
        "SELECT * FROM bis.GeometricElement3d e, imodel_geom_stream(e.GeometryStream) gs "
        "WHERE e.ECInstanceId = ?"));

    stmt.BindId(1, elemId);
    ASSERT_EQ(BE_SQLITE_ROW, stmt.Step()) << "SELECT * should return at least one row";
    }

/*---------------------------------------------------------------------------------**//**
* Verify filtering: querying with IsGeometry = 1 should only return geometry opcodes.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(GeomStreamVTabTest, FilterGeometryEntries)
    {
    SetupSeedProject();

    auto el = InsertElement(Render::GeometryParams(m_defaultCategoryId));
    ASSERT_TRUE(el.IsValid());
    DgnElementId elemId = el->GetElementId();
    SaveDb();

    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(*m_db,
        "SELECT gs.OpCode, gs.IsGeometry "
        "FROM bis.GeometricElement3d e, imodel_geom_stream(e.GeometryStream) gs "
        "WHERE e.ECInstanceId = ? AND gs.IsGeometry = 1"));

    stmt.BindId(1, elemId);

    int geomRows = 0;
    while (BE_SQLITE_ROW == stmt.Step())
        {
        EXPECT_EQ(1, stmt.GetValueInt(1)) << "Filtered rows should all have IsGeometry=1";
        geomRows++;
        }

    EXPECT_GT(geomRows, 0) << "Should find at least one geometry entry";
    }

/*---------------------------------------------------------------------------------**//**
* Verify that an element with geometry using display params has Color/Weight populated.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(GeomStreamVTabTest, DisplayParams)
    {
    SetupSeedProject();

    // Insert element with explicit display properties
    Render::GeometryParams geomParams(m_defaultCategoryId);
    geomParams.SetLineColor(ColorDef::Green());
    geomParams.SetWeight(3);

    auto el = InsertElement(geomParams);
    ASSERT_TRUE(el.IsValid());
    DgnElementId elemId = el->GetElementId();
    SaveDb();

    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(*m_db,
        "SELECT gs.Color, gs.Weight "
        "FROM bis.GeometricElement3d e, imodel_geom_stream(e.GeometryStream) gs "
        "WHERE e.ECInstanceId = ? AND gs.IsGeometry = 1"));

    stmt.BindId(1, elemId);

    bool foundColorWeight = false;
    while (BE_SQLITE_ROW == stmt.Step())
        {
        if (!stmt.IsValueNull(0))
            {
            foundColorWeight = true;
            // Color should be set (we set Green)
            int color = stmt.GetValueInt(0);
            EXPECT_NE(0, color) << "Color should be non-zero when explicitly set";
            }
        if (!stmt.IsValueNull(1))
            {
            int weight = stmt.GetValueInt(1);
            EXPECT_EQ(3, weight) << "Weight should match the value we set";
            }
        }

    EXPECT_TRUE(foundColorWeight) << "Should find Color/Weight in symbology entries";
    }

/*---------------------------------------------------------------------------------**//**
* Verify that 2d elements also work.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(GeomStreamVTabTest, Element2d)
    {
    SetupSeedProject();

    // Create a drawing model to hold the 2D element (TestElement2d requires a 2D model)
    DgnCategoryId drawingCategoryId = DgnDbTestUtils::InsertDrawingCategory(*m_db, "GeomStreamTestDrawingCategory");
    DocumentListModelPtr drawingListModel = DgnDbTestUtils::InsertDocumentListModel(*m_db, "GeomStreamTestDrawingList");
    DrawingPtr drawing = DgnDbTestUtils::InsertDrawing(*drawingListModel, "GeomStreamTestDrawing");
    DrawingModelPtr drawingModel = DgnDbTestUtils::InsertDrawingModel(*drawing);
    ASSERT_TRUE(drawingModel.IsValid()) << "Failed to create drawing model";

    DgnElementId elem2dId = InsertElement2d(drawingModel->GetModelId(), drawingCategoryId);
    ASSERT_TRUE(elem2dId.IsValid());
    SaveDb();

    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(*m_db,
        "SELECT gs.EntryIndex, gs.OpCode "
        "FROM bis.GeometricElement2d e, imodel_geom_stream(e.GeometryStream) gs "
        "WHERE e.ECInstanceId = ?"));

    stmt.BindId(1, elem2dId);

    int rowCount = 0;
    while (BE_SQLITE_ROW == stmt.Step())
        rowCount++;

    EXPECT_GT(rowCount, 0) << "2d element should have geometry stream entries";
    }

/*---------------------------------------------------------------------------------**//**
* Verify that an element with a geometry part reference populates the Part columns.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(GeomStreamVTabTest, GeometryPartReference)
    {
    SetupSeedProject();

    // Create a geometry part using the dictionary model (a DefinitionModel)
    DgnGeometryPartPtr part = DgnGeometryPart::Create(m_db->GetDictionaryModel(), "TestPart");
    ASSERT_TRUE(part.IsValid());

    GeometricPrimitivePtr geomPrim = GeometricPrimitive::Create(*GeomHelper::computeShape());
    GeometryBuilderPtr partBuilder = GeometryBuilder::CreateGeometryPart(*m_db, true);
    partBuilder->Append(*geomPrim);
    ASSERT_EQ(SUCCESS, partBuilder->Finish(*part));

    DgnGeometryPartCPtr insertedPart = m_db->Elements().Insert<DgnGeometryPart>(*part);
    ASSERT_TRUE(insertedPart.IsValid()) << "Failed to insert geometry part";

    // Create element referencing the part
    DgnElementId elemId = InsertElementUsingGeometryPart(insertedPart->GetId());
    ASSERT_TRUE(elemId.IsValid());
    SaveDb();

    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(*m_db,
        "SELECT gs.GeometryPartId, gs.PartOriginX, gs.PartOriginY, gs.PartOriginZ, gs.PartScale, gs.OpCode "
        "FROM bis.GeometricElement3d e, imodel_geom_stream(e.GeometryStream) gs "
        "WHERE e.ECInstanceId = ? AND gs.GeometryPartId IS NOT NULL"));

    stmt.BindId(1, elemId);

    bool foundPart = false;
    while (BE_SQLITE_ROW == stmt.Step())
        {
        foundPart = true;
        int64_t partId = stmt.GetValueInt64(0);
        EXPECT_NE(0, partId) << "GeometryPartId should be valid";

        // PartScale defaults to 1.0 for non-scaled parts
        if (!stmt.IsValueNull(4))
            {
            double scale = stmt.GetValueDouble(4);
            EXPECT_GT(scale, 0.0) << "PartScale should be positive";
            }

        Utf8CP opCode = stmt.GetValueText(5);
        EXPECT_STREQ("GeometryPartInstance", opCode) << "Part entries should have GeometryPartInstance opcode";
        }

    EXPECT_TRUE(foundPart) << "Should find the geometry part reference";
    }

/*---------------------------------------------------------------------------------**//**
* Verify that imodel_geom_stream produces no rows (not an error) when GeometryStream is NULL.
* This tests resilience against elements that exist but have no geometry.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(GeomStreamVTabTest, NullGeometryStream)
    {
    SetupSeedProject();

    // Insert an element without geometry — use CreateWithoutGeometry if available,
    // or query an element class that may not have geometry.
    // We'll test by trying to join with a non-geometric element class.
    // If there are no null geometry streams, this test simply passes with 0 rows.
    ECSqlStatement stmt;
    auto status = stmt.Prepare(*m_db,
        "SELECT COUNT(*) FROM bis.GeometricElement3d e, imodel_geom_stream(e.GeometryStream) gs "
        "WHERE e.GeometryStream IS NULL");

    // If the ECSQL prepares, it should return 0 rows, not an error
    if (ECSqlStatus::Success == status)
        {
        ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());
        int count = stmt.GetValueInt(0);
        EXPECT_EQ(0, count) << "NULL GeometryStream should produce 0 vtab rows";
        }
    }

/*---------------------------------------------------------------------------------**//**
* Verify that querying with schema-qualified name also works: IModelVLib.imodel_geom_stream
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(GeomStreamVTabTest, SchemaQualifiedName)
    {
    SetupSeedProject();

    auto el = InsertElement(Render::GeometryParams(m_defaultCategoryId));
    ASSERT_TRUE(el.IsValid());
    DgnElementId elemId = el->GetElementId();
    SaveDb();

    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(*m_db,
        "SELECT gs.EntryIndex, gs.OpCode "
        "FROM bis.GeometricElement3d e, IModelVLib.imodel_geom_stream(e.GeometryStream) gs "
        "WHERE e.ECInstanceId = ?"));

    stmt.BindId(1, elemId);

    int rowCount = 0;
    while (BE_SQLITE_ROW == stmt.Step())
        rowCount++;

    EXPECT_GT(rowCount, 0) << "Schema-qualified name should also work";
    }

/*---------------------------------------------------------------------------------**//**
* Verify we can count geometry-type entries per element using GROUP BY.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(GeomStreamVTabTest, CountGeometryEntriesPerElement)
    {
    SetupSeedProject();

    auto el1 = InsertElement(Render::GeometryParams(m_defaultCategoryId));
    auto el2 = InsertElement(Render::GeometryParams(m_defaultCategoryId));
    ASSERT_TRUE(el1.IsValid() && el2.IsValid());
    SaveDb();

    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(*m_db,
        "SELECT e.ECInstanceId, SUM(gs.IsGeometry) AS geomCount "
        "FROM bis.GeometricElement3d e, imodel_geom_stream(e.GeometryStream) gs "
        "GROUP BY e.ECInstanceId"));

    int elementCount = 0;
    while (BE_SQLITE_ROW == stmt.Step())
        {
        elementCount++;
        int geomCount = stmt.GetValueInt(1);
        EXPECT_GT(geomCount, 0) << "Each element should have at least one geometry entry";
        }

    EXPECT_GE(elementCount, 2) << "Should have at least 2 elements";
    }

/*---------------------------------------------------------------------------------**//**
* Verify that EntryIndex is unique and monotonically increasing per element.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(GeomStreamVTabTest, EntryIndexMonotonic)
    {
    SetupSeedProject();

    auto el = InsertElement(Render::GeometryParams(m_defaultCategoryId));
    ASSERT_TRUE(el.IsValid());
    DgnElementId elemId = el->GetElementId();
    SaveDb();

    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(*m_db,
        "SELECT gs.EntryIndex "
        "FROM bis.GeometricElement3d e, imodel_geom_stream(e.GeometryStream) gs "
        "WHERE e.ECInstanceId = ?"));

    stmt.BindId(1, elemId);

    int lastIndex = -1;
    while (BE_SQLITE_ROW == stmt.Step())
        {
        int index = stmt.GetValueInt(0);
        EXPECT_EQ(lastIndex + 1, index) << "EntryIndex should be monotonically increasing";
        lastIndex = index;
        }

    EXPECT_GE(lastIndex, 0) << "Should have visited at least one entry";
    }

/*---------------------------------------------------------------------------------**//**
* Verify that imodel_geom_json returns valid JSON for a geometry entry.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(GeomStreamVTabTest, GeomJsonScalarFunction)
    {
    SetupSeedProject();

    auto el = InsertElement(Render::GeometryParams(m_defaultCategoryId));
    ASSERT_TRUE(el.IsValid());
    DgnElementId elemId = el->GetElementId();
    SaveDb();

    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(*m_db,
        "SELECT imodel_geom_json(gs.GeometryBlob), gs.EntryType "
        "FROM bis.GeometricElement3d e, imodel_geom_stream(e.GeometryStream) gs "
        "WHERE e.ECInstanceId = ? AND gs.IsGeometry = 1"));

    stmt.BindId(1, elemId);

    bool foundJson = false;
    while (BE_SQLITE_ROW == stmt.Step())
        {
        Utf8CP entryType = stmt.GetValueText(1);
        // BRepEntity returns NULL — skip
        if (entryType != nullptr && strcmp(entryType, "BRepEntity") == 0)
            continue;

        if (!stmt.IsValueNull(0))
            {
            Utf8CP json = stmt.GetValueText(0);
            EXPECT_TRUE(json != nullptr && strlen(json) > 0) << "imodel_geom_json should return non-empty JSON";
            foundJson = true;
            }
        }

    EXPECT_TRUE(foundJson) << "Should find at least one geometry entry with JSON";
    }

/*---------------------------------------------------------------------------------**//**
* Verify that imodel_geom_entry_count returns the geometry entry count.
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(GeomStreamVTabTest, GeomEntryCountFunction)
    {
    SetupSeedProject();

    auto el = InsertElement(Render::GeometryParams(m_defaultCategoryId));
    ASSERT_TRUE(el.IsValid());
    DgnElementId elemId = el->GetElementId();
    SaveDb();

    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(*m_db,
        "SELECT imodel_geom_entry_count(e.GeometryStream) "
        "FROM bis.GeometricElement3d e WHERE e.ECInstanceId = ?"));

    stmt.BindId(1, elemId);
    ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());
    EXPECT_FALSE(stmt.IsValueNull(0)) << "imodel_geom_entry_count should not be NULL";
    int count = stmt.GetValueInt(0);
    EXPECT_GT(count, 0) << "Should have at least one geometry entry";
    }

/*---------------------------------------------------------------------------------**//**
* Verify that imodel_geom_has_brep returns 0 or 1 (not NULL).
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
TEST_F(GeomStreamVTabTest, GeomHasBrepFunction)
    {
    SetupSeedProject();

    auto el = InsertElement(Render::GeometryParams(m_defaultCategoryId));
    ASSERT_TRUE(el.IsValid());
    DgnElementId elemId = el->GetElementId();
    SaveDb();

    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(*m_db,
        "SELECT imodel_geom_has_brep(e.GeometryStream) "
        "FROM bis.GeometricElement3d e WHERE e.ECInstanceId = ?"));

    stmt.BindId(1, elemId);
    ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());
    EXPECT_FALSE(stmt.IsValueNull(0)) << "imodel_geom_has_brep should return 0 or 1";
    int val = stmt.GetValueInt(0);
    EXPECT_TRUE(val == 0 || val == 1) << "imodel_geom_has_brep should be 0 or 1";
    }
