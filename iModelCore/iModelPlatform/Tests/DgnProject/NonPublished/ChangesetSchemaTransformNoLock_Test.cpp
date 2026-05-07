/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/

/**
 * Multi-user tests for TxnManager concurrent schema import (SetAllowConcurrentSchemaImport).
 *
 * Ported from TypeScript:
 *   itwinjs-core/core/backend/src/test/hubaccess/ChangesetSchemaTransformNoLock.test.ts
 *
 * These tests verify that when two briefcase users (far and local) operate concurrently,
 * schema changes and data changes interleave correctly when SetAllowConcurrentSchemaImport
 * is enabled before PullMergeApply.  Schema txns are reinstated by replaying the binary DDL
 * changeset with concurrent-import enabled; data txns are reinstated by replaying the binary
 * changeset unchanged.
 *
 * IMPORTANT CONSTRAINT: column-swap (transforming) schemas cannot be used in local pending
 * changes with this mode.  These tests therefore use ADDITIVE-only schemas (new properties,
 * no column remapping) for all pending-local-change scenarios.
 *
 * Scenarios covered:
 *   1. Additive schema pushed by far while local has a pending UPDATE — value preserved.
 *   2. Two sequential additive schemas pushed by far while local has pending UPDATEs.
 *   3. TxnManager squash (SaveSchemaAndDataTxns) — a transforming schema import squashes
 *      DDL + data-migration into a single Schema-type txn.  No rebase involved.
 *   4. Additive schema change without local pending data — incoming schema applied, local
 *      INSERTs unaffected (no transform needed).
 *   5. Local additive schema change reinstated over incoming remote data — local schema
 *      is reinstated at v01.00.02 and far's data is accessible.
 */

#include "ChangeTestFixture.h"

USING_NAMESPACE_BENTLEY_DGN
USING_NAMESPACE_BENTLEY_EC
USING_NAMESPACE_BENTLEY_SQLITE
USING_NAMESPACE_BENTLEY_SQLITE_EC

namespace {

//-----------------------------------------------------------------------------------------------
// Schema definitions.  All schemas use bis:PhysicalElement as the root base class so the test
// elements can live in the default PhysicalModel / SpatialCategory created by the seed DB.
//-----------------------------------------------------------------------------------------------

/** Base schema v01.00.00: A(PropA), C extends A (PropC), D extends A (PropD). */
static Utf8CP s_schemaV01x00x00 = R"xml(<?xml version="1.0" encoding="UTF-8"?>
    <ECSchema schemaName="TestDomain" alias="td" version="01.00.00"
              xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
      <ECSchemaReference name="BisCore" version="1.0.0" alias="bis"/>
      <ECEntityClass typeName="A">
        <BaseClass>bis:PhysicalElement</BaseClass>
        <ECProperty propertyName="PropA" typeName="string"/>
      </ECEntityClass>
      <ECEntityClass typeName="C">
        <BaseClass>A</BaseClass>
        <ECProperty propertyName="PropC" typeName="string"/>
      </ECEntityClass>
      <ECEntityClass typeName="D">
        <BaseClass>A</BaseClass>
        <ECProperty propertyName="PropD" typeName="string"/>
      </ECEntityClass>
    </ECSchema>)xml";

/** v01.00.01 — additive: adds PropC2 to class C.  No column remapping. */
static Utf8CP s_schemaV01x00x01_addPropC2 = R"xml(<?xml version="1.0" encoding="UTF-8"?>
    <ECSchema schemaName="TestDomain" alias="td" version="01.00.01"
              xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
      <ECSchemaReference name="BisCore" version="1.0.0" alias="bis"/>
      <ECEntityClass typeName="A">
        <BaseClass>bis:PhysicalElement</BaseClass>
        <ECProperty propertyName="PropA" typeName="string"/>
      </ECEntityClass>
      <ECEntityClass typeName="C">
        <BaseClass>A</BaseClass>
        <ECProperty propertyName="PropC" typeName="string"/>
        <ECProperty propertyName="PropC2" typeName="string"/>
      </ECEntityClass>
      <ECEntityClass typeName="D">
        <BaseClass>A</BaseClass>
        <ECProperty propertyName="PropD" typeName="string"/>
      </ECEntityClass>
    </ECSchema>)xml";

/** v01.00.02 — additive: adds PropD2 to class D.  Builds on v01.00.01. */
static Utf8CP s_schemaV01x00x02_addPropD2 = R"xml(<?xml version="1.0" encoding="UTF-8"?>
    <ECSchema schemaName="TestDomain" alias="td" version="01.00.02"
              xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
      <ECSchemaReference name="BisCore" version="1.0.0" alias="bis"/>
      <ECEntityClass typeName="A">
        <BaseClass>bis:PhysicalElement</BaseClass>
        <ECProperty propertyName="PropA" typeName="string"/>
      </ECEntityClass>
      <ECEntityClass typeName="C">
        <BaseClass>A</BaseClass>
        <ECProperty propertyName="PropC" typeName="string"/>
        <ECProperty propertyName="PropC2" typeName="string"/>
      </ECEntityClass>
      <ECEntityClass typeName="D">
        <BaseClass>A</BaseClass>
        <ECProperty propertyName="PropD" typeName="string"/>
        <ECProperty propertyName="PropD2" typeName="string"/>
      </ECEntityClass>
    </ECSchema>)xml";

/**
 * v01.00.02 — TRANSFORMING: moves PropC from class C up to base class A.
 * Used ONLY in test 3 (schema-squash); triggers data migration via RemapManager.
 * Applies on top of v01.00.01_addPropC2 (PropC2 stays on C).
 */
static Utf8CP s_schemaV01x00x02_movePropCToA = R"xml(<?xml version="1.0" encoding="UTF-8"?>
    <ECSchema schemaName="TestDomain" alias="td" version="01.00.02"
              xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
      <ECSchemaReference name="BisCore" version="1.0.0" alias="bis"/>
      <ECEntityClass typeName="A">
        <BaseClass>bis:PhysicalElement</BaseClass>
        <ECProperty propertyName="PropA" typeName="string"/>
        <ECProperty propertyName="PropC" typeName="string"/>
      </ECEntityClass>
      <ECEntityClass typeName="C">
        <BaseClass>A</BaseClass>
        <ECProperty propertyName="PropC2" typeName="string"/>
      </ECEntityClass>
      <ECEntityClass typeName="D">
        <BaseClass>A</BaseClass>
        <ECProperty propertyName="PropD" typeName="string"/>
      </ECEntityClass>
    </ECSchema>)xml";

} // namespace

//=======================================================================================
/**
 * Test fixture for concurrent schema import scenarios.
 *
 * Extends ChangeTestFixture to inherit SetUpTestCase, the seed file, and the default
 * PhysicalModel / SpatialCategory.  Adds helpers for revision creation, file backup /
 * restore, element insertion, and property assertions.
 *
 * @bsiclass
 */
//=======================================================================================
struct ConcurrentSchemaImportTest : ChangeTestFixture
    {
    DEFINE_T_SUPER(ChangeTestFixture)

protected:
    /** Unique backup filename — keeps this fixture's backup separate from RevisionTestFixture's. */
    WCharCP m_copyTestFileName = L"ConcurrentSchemaTestCopy.ibim";

    //-------------------------------------------------------------------------------------
    // Schema helpers
    //-------------------------------------------------------------------------------------

    ECSchemaReadContextPtr MakeSchemaContext()
        {
        auto ctx = ECSchemaReadContext::CreateContext();
        ctx->AddSchemaLocater(m_db->GetSchemaLocater());
        return ctx;
        }

    void ImportSchema(Utf8CP xml)
        {
        auto ctx = MakeSchemaContext();
        ECSchemaPtr schema;
        ASSERT_EQ(SchemaReadStatus::Success, ECSchema::ReadFromXmlString(schema, xml, *ctx));
        // schemaLockHeld=true enables AllowDataTransformDuringSchemaUpgrade so
        // transforming schemas (column remapping) also work correctly in test 3.
        ASSERT_EQ(SchemaStatus::Success, m_db->ImportSchemas({schema.get()}, true));
        m_db->SaveChanges("import schema");
        }

    //-------------------------------------------------------------------------------------
    // Changeset helpers (mirrors RevisionTestFixture — defined here to avoid coupling)
    //-------------------------------------------------------------------------------------

    ChangesetPropsPtr CreateRevision(Utf8CP ext)
        {
        ChangesetPropsPtr revision = m_db->Txns().StartCreateChangeset(ext);
        if (!revision.IsValid())
            return nullptr;
        m_db->Txns().FinishCreateChangeset(-1, ext != nullptr);
        return revision;
        }

    void BackupTestFile()
        {
        BeFileName fileName = BeFileName(m_db->GetDbFileName(), true);
        CloseDgnDb();
        BeFileName copyFile(fileName.GetDirectoryName());
        copyFile.AppendToPath(m_copyTestFileName);
        ASSERT_EQ(BeFileNameStatus::Success, BeFileName::BeCopyFile(fileName.c_str(), copyFile.c_str()));
        OpenIModelDb(fileName);
        }

    void RestoreTestFile()
        {
        BeFileName fileName = BeFileName(m_db->GetDbFileName(), true);
        CloseDgnDb();
        BeFileName copyFile(fileName.GetDirectoryName());
        copyFile.AppendToPath(m_copyTestFileName);
        ASSERT_EQ(BeFileNameStatus::Success, BeFileName::BeCopyFile(copyFile.c_str(), fileName.c_str()));
        OpenIModelDb(fileName);
        }

    //-------------------------------------------------------------------------------------
    // Element helpers
    //-------------------------------------------------------------------------------------

    /**
     * Insert an element of the given TestDomain class into the default PhysicalModel.
     * @param className  Simple class name within "TestDomain" (e.g. "C" or "D").
     * @param props      List of {propertyName, value} pairs to set before insertion.
     * Calls SaveChanges after the insertion so the element's txn is committed.
     */
    DgnElementId InsertElement(Utf8CP className, std::initializer_list<std::pair<Utf8CP, Utf8CP>> props = {})
        {
        DgnClassId classId = m_db->Schemas().GetClassId("TestDomain", className);
        EXPECT_TRUE(classId.IsValid()) << "Class TestDomain:" << className << " not found";

        // Use the handler-based factory so the C++ type matches what the platform
        // expects when loading/editing (avoids typeid mismatch in CopyForEdit).
        auto handler = dgn_ElementHandler::Element::FindHandler(*m_db, classId);
        EXPECT_TRUE(handler != nullptr) << "No handler found for TestDomain:" << className;

        DgnElement::CreateParams params(*m_db, m_defaultModelId, classId);
        DgnElementPtr elem = handler->Create(params);
        EXPECT_TRUE(elem.IsValid()) << "Failed to create element for TestDomain:" << className;

        // Set required geometric element properties (category) before insert.
        // GeometricElement3d also inherits GeometrySource3d which has public SetCategoryId.
        auto geomSrc = dynamic_cast<GeometrySource*>(elem.get());
        EXPECT_TRUE(geomSrc != nullptr);
        if (geomSrc)
            geomSrc->SetCategoryId(m_defaultCategoryId);

        for (auto& p : props)
            elem->SetPropertyValue(p.first, p.second);

        DgnDbStatus status;
        DgnElementCPtr inserted = elem->Insert(&status);
        EXPECT_EQ(DgnDbStatus::Success, status);
        m_db->SaveChanges("insert element");
        return inserted.IsValid() ? inserted->GetElementId() : DgnElementId();
        }

    /** Update a single string property on an existing element and save the change. */
    void UpdateElementProp(DgnElementId elementId, Utf8CP propName, Utf8CP value)
        {
        auto elem = m_db->Elements().GetForEdit<DgnElement>(elementId);
        ASSERT_TRUE(elem.IsValid()) << "Element " << elementId.ToHexStr() << " not found";
        ASSERT_EQ(DgnDbStatus::Success, elem->SetPropertyValue(propName, value));
        ASSERT_EQ(DgnDbStatus::Success, elem->Update());
        m_db->SaveChanges("update element property");
        }

    /** Read a string property from an element and assert it equals the expected value. */
    void AssertElementProp(DgnElementId elementId, Utf8CP propName, Utf8CP expected)
        {
        auto elem = m_db->Elements().Get<DgnElement>(elementId);
        ASSERT_TRUE(elem.IsValid()) << "Element " << elementId.ToHexStr() << " not found";
        Utf8String actual = elem->GetPropertyValueString(propName);
        EXPECT_STREQ(expected, actual.c_str())
            << "\"" << propName << "\" mismatch on element " << elementId.ToHexStr();
        }

    //-------------------------------------------------------------------------------------
    // Schema / txn state helpers
    //-------------------------------------------------------------------------------------

    /** Return the minor version of the TestDomain schema currently loaded in the DB. */
    uint32_t GetSchemaMinorVersion()
        {
        ECSchemaCP s = m_db->Schemas().GetSchema("TestDomain");
        EXPECT_TRUE(s != nullptr) << "TestDomain schema not found in DB";
        return s ? s->GetVersionMinor() : static_cast<uint32_t>(-1);
        }

    /** Assert that the most recently saved txn (in the txn table) is a Schema-type txn. */
    void AssertLastTxnIsSchemaType()
        {
        TxnManager::TxnId lastId = m_db->Txns().GetLastTxnId();
        ASSERT_TRUE(lastId.IsValid()) << "No saved txn found (txn table is empty)";
        BeJsDocument props;
        ASSERT_TRUE(m_db->Txns().GetTxnProps(lastId, props)) << "GetTxnProps returned false";
        EXPECT_STREQ("Schema", props["type"].asCString())
            << "Expected last txn to be of type \"Schema\"; got \""
            << props["type"].asCString() << "\"";
        }

    /** Enable concurrent-schema-import mode and apply a far changeset (with rebase). */
    void PullMergeApplyWithConcurrentSchema(ChangesetPropsCR revision)
        {
        // The flag is cleared automatically by PullMergeRebaseEnd, so it must be set
        // before each PullMergeApply call.
        m_db->Txns().SetAllowConcurrentSchemaImport(true);
        EXPECT_EQ(ChangesetStatus::Success, m_db->Txns().PullMergeApply(revision));
        }
    };

// ---------------------------------------------------------------------------
// Test 1 — Additive schema arrives while local UPDATE is pending.
//
// Workflow:
//   [setup]  import base schema; insert C element (propC="initial_c"); create revisions
//   [backup] S1 = base schema + C element ("sync point" for both users)
//   [far]    import v01.00.01 additive (adds PropC2 to C); create revision
//   [restore S1] local starts at the sync point
//   [local]  update C.PropC = "local_c_value" (pending, no revision)
//   [local]  PullMergeApply(farSchemaRevision) — rebase reinstates the UPDATE
//   verify:  schema minor version == 1; C.PropC == "local_c_value"
//
// The additive schema does not remap columns, so the binary UPDATE replay correctly
// places "local_c_value" in the same physical column after the schema is applied.
// ---------------------------------------------------------------------------
TEST_F(ConcurrentSchemaImportTest, AdditiveSchemaArriveWhileLocalUpdatePending)
    {
    SetupDgnDb(ChangeTestFixture::s_seedFileInfo.fileName, L"ConcurrentSchema_Test1.bim");
    m_db->SaveChanges("init");

    ImportSchema(s_schemaV01x00x00);
    ASSERT_TRUE(CreateRevision("-base-schema").IsValid());

    DgnElementId elementId = InsertElement("C", {{"PropA", "initial_a"}, {"PropC", "initial_c"}});
    ASSERT_TRUE(elementId.IsValid());
    ASSERT_TRUE(CreateRevision("-insert-c").IsValid());

    // Both users are now synced at S1 (base schema + C element).
    BackupTestFile();

    // Far: import the additive schema (adds PropC2 to C — no column remapping).
    ImportSchema(s_schemaV01x00x01_addPropC2);
    ChangesetPropsPtr farSchemaRevision = CreateRevision("-far-schema-v01.00.01");
    ASSERT_TRUE(farSchemaRevision.IsValid());

    // Local: start from the sync point.
    RestoreTestFile();
    EXPECT_EQ(0u, GetSchemaMinorVersion());

    // Local pending change: update PropC (not yet in a revision).
    UpdateElementProp(elementId, "PropC", "local_c_value");
    AssertElementProp(elementId, "PropC", "local_c_value");

    // Local: pull and rebase — binary replay of the UPDATE reinstates "local_c_value"
    // in the same physical column (additive schema does not remap columns).
    PullMergeApplyWithConcurrentSchema(*farSchemaRevision);

    EXPECT_EQ(1u, GetSchemaMinorVersion()) << "schema should be v01.00.01 after pull";
    AssertElementProp(elementId, "PropC", "local_c_value");
    }

// ---------------------------------------------------------------------------
// Test 2 — Two sequential additive schemas pushed by far while local has pending UPDATEs.
//
// Workflow:
//   [setup]  import base schema; insert C element (propC="c_initial") and
//            D element (propD="d_initial"); create revisions
//   [backup] S1 = base schema + C + D ("sync point")
//   [far]    import v01.00.01 (adds PropC2); create revision 1
//   [far]    import v01.00.02 (adds PropD2); create revision 2
//   [restore S1] local starts at the sync point
//   [local]  update C.PropC = "c_prop_value" (pending)
//   [local]  update D.PropD = "d_prop_value" (pending)
//   [local]  PullMergeApply(rev1) then PullMergeApply(rev2)
//   verify:  schema minor version == 2; PropC == "c_prop_value"; PropD == "d_prop_value"
// ---------------------------------------------------------------------------
TEST_F(ConcurrentSchemaImportTest, TwoSequentialAdditiveSchemasPreservePendingUpdates)
    {
    SetupDgnDb(ChangeTestFixture::s_seedFileInfo.fileName, L"ConcurrentSchema_Test2.bim");
    m_db->SaveChanges("init");

    ImportSchema(s_schemaV01x00x00);
    ASSERT_TRUE(CreateRevision("-base-schema").IsValid());

    DgnElementId cId = InsertElement("C", {{"PropA", "c_a_initial"}, {"PropC", "c_initial"}});
    ASSERT_TRUE(cId.IsValid());
    ASSERT_TRUE(CreateRevision("-insert-c").IsValid());

    DgnElementId dId = InsertElement("D", {{"PropA", "d_a_initial"}, {"PropD", "d_initial"}});
    ASSERT_TRUE(dId.IsValid());
    ASSERT_TRUE(CreateRevision("-insert-d").IsValid());

    // Both synced at S1.
    BackupTestFile();

    // Far: push two sequential additive schemas.
    ImportSchema(s_schemaV01x00x01_addPropC2);
    ChangesetPropsPtr farRevision1 = CreateRevision("-far-schema-v01.00.01");
    ASSERT_TRUE(farRevision1.IsValid());

    ImportSchema(s_schemaV01x00x02_addPropD2);
    ChangesetPropsPtr farRevision2 = CreateRevision("-far-schema-v01.00.02");
    ASSERT_TRUE(farRevision2.IsValid());

    // Local: start from the sync point.
    RestoreTestFile();

    // Local pending changes to both C.PropC and D.PropD.
    UpdateElementProp(cId, "PropC", "c_prop_value");
    UpdateElementProp(dId, "PropD", "d_prop_value");
    AssertElementProp(cId, "PropC", "c_prop_value");
    AssertElementProp(dId, "PropD", "d_prop_value");

    // Local: apply both additive schema changesets in sequence.
    // The flag must be re-set before each apply because PullMergeRebaseEnd clears it.
    PullMergeApplyWithConcurrentSchema(*farRevision1);
    PullMergeApplyWithConcurrentSchema(*farRevision2);

    EXPECT_EQ(2u, GetSchemaMinorVersion()) << "schema should be v01.00.02 after two pulls";
    AssertElementProp(cId, "PropC", "c_prop_value");
    AssertElementProp(dId, "PropD", "d_prop_value");
    }

// ---------------------------------------------------------------------------
// Test 3 — TxnManager squash: SaveSchemaAndDataTxns.
//
// When a schema import triggers data migration (column remapping via RemapManager),
// TxnManager squashes the DDL txn and the data-migration txns into a single
// Schema-type txn.  No rebase is involved; this tests the import-time squash only.
//
// Workflow:
//   [setup]  import base schema; insert a C element (rows to migrate); create revisions
//   import v01.00.01 additive (adds PropC2 to C)
//   import v01.00.02 transforming (moves PropC from C up to A — triggers data migration)
//   verify:  last saved txn has type "Schema"
// ---------------------------------------------------------------------------
TEST_F(ConcurrentSchemaImportTest, TransformingSchemaImportSquashedIntoSingleSchemaTxn)
    {
    SetupDgnDb(ChangeTestFixture::s_seedFileInfo.fileName, L"ConcurrentSchema_Test3.bim");
    m_db->SaveChanges("init");

    ImportSchema(s_schemaV01x00x00);
    ASSERT_TRUE(CreateRevision("-base-schema").IsValid());

    // Insert data so the subsequent transforming schema has rows to migrate.
    ASSERT_TRUE(InsertElement("C", {{"PropA", "migrate_a"}, {"PropC", "migrate_c"}}).IsValid());
    ASSERT_TRUE(CreateRevision("-pre-migration-data").IsValid());

    // Import the additive schema (adds PropC2 to C — no migration).
    ImportSchema(s_schemaV01x00x01_addPropC2);

    // Import the transforming schema (moves PropC from C up to A).
    // TxnManager::SaveSchemaAndDataTxns squashes the DDL txn and the data-migration
    // txn into a single EcSchema-type entry in the txns table (stored as NULL in the
    // IsSchemaChange column for backwards compatibility).
    ImportSchema(s_schemaV01x00x02_movePropCToA);

    // The last saved txn must be of type "Schema" (NULL IsSchemaChange column).
    AssertLastTxnIsSchemaType();
    }

// ---------------------------------------------------------------------------
// Test 4 — Additive schema change without local pending data.
//
// Local has pending INSERT txns (C and D elements inserted at v01.00.00).
// Far pushes an additive schema (adds PropC2 — no column remapping).
// After pull, the data values must be intact (no transformation needed).
//
// Workflow:
//   [setup]  import base schema; create revision
//   [backup] S0 = just base schema, no data ("sync point")
//   [far]    import v01.00.01 additive (adds PropC2); create revision
//   [restore S0] local starts at the sync point
//   [local]  insert C element (PropC="c_val") and D element (PropD="d_val") — pending INSERTs
//   [local]  PullMergeApply(farSchemaRevision) — rebase: reverse INSERTs, apply schema,
//            reinstate INSERTs (PropC2 defaults to null; PropC and PropD unchanged)
//   verify:  schema minor version == 1; PropC == "c_val"; PropD == "d_val"
// ---------------------------------------------------------------------------
TEST_F(ConcurrentSchemaImportTest, AdditiveSchemaChangeLocalInsertsUnaffected)
    {
    SetupDgnDb(ChangeTestFixture::s_seedFileInfo.fileName, L"ConcurrentSchema_Test4.bim");
    m_db->SaveChanges("init");

    ImportSchema(s_schemaV01x00x00);
    ASSERT_TRUE(CreateRevision("-base-schema").IsValid());

    // Sync point: just the base schema, no data yet.
    BackupTestFile();

    // Far: push the additive schema change.
    ImportSchema(s_schemaV01x00x01_addPropC2);
    ChangesetPropsPtr farSchemaRevision = CreateRevision("-far-schema-v01.00.01");
    ASSERT_TRUE(farSchemaRevision.IsValid());

    // Local: start from the sync point (base schema only).
    RestoreTestFile();

    // Local pending INSERTs (not yet in a revision).
    DgnElementId cId = InsertElement("C", {{"PropA", "a_val"}, {"PropC", "c_val"}});
    DgnElementId dId = InsertElement("D", {{"PropA", "a2_val"}, {"PropD", "d_val"}});
    ASSERT_TRUE(cId.IsValid());
    ASSERT_TRUE(dId.IsValid());

    // Local: apply the additive schema — rebase reinstates the INSERTs unchanged
    // (PropC2 is absent in the binary replay, so it defaults to null; PropC and
    // PropD stay in the same physical columns).
    PullMergeApplyWithConcurrentSchema(*farSchemaRevision);

    EXPECT_EQ(1u, GetSchemaMinorVersion()) << "schema should be v01.00.01 after pull";
    AssertElementProp(cId, "PropC", "c_val");
    AssertElementProp(dId, "PropD", "d_val");
    }

// ---------------------------------------------------------------------------
// Test 5 — Local additive schema change reinstated over incoming remote data.
//
// With SetAllowConcurrentSchemaImport, PullMergeRebaseReinstateTxn() can re-import
// an ADDITIVE schema txn (DDL binary replay just adds a column; no column-swap
// migration needed).  This exercises the full local-schema reinstatement path.
//
// Workflow:
//   [setup]  import base schema; create revision
//   [far]    import v01.00.01 + insert first C element (propC="far_c1"); create revision 1
//   [backup] S1 = v01.00.01 + far_c1 element ("sync point" — both users at v01.00.01)
//   [far]    insert second C element (propC="far_c2_data"); create revision 2 (data only)
//   [restore S1] local starts at the sync point
//   [local]  import v01.00.02AddPropD2 (additive — pending schema txn, NOT pushed)
//   verify:  local last txn is "Schema"; schema minor == 2
//   [local]  PullMergeApply(farRevision2) — rebase: reverse v01.00.02, apply far's INSERT,
//            reinstate v01.00.02 via concurrent import
//   verify:  schema minor version == 2; far_c1.PropC == "far_c1"; far_c2.PropC == "far_c2_data"
// ---------------------------------------------------------------------------
TEST_F(ConcurrentSchemaImportTest, LocalAdditiveSchemaReinstatedOverIncomingRemoteData)
    {
    SetupDgnDb(ChangeTestFixture::s_seedFileInfo.fileName, L"ConcurrentSchema_Test5.bim");
    m_db->SaveChanges("init");

    ImportSchema(s_schemaV01x00x00);
    ASSERT_TRUE(CreateRevision("-base-schema").IsValid());

    // Far: import v01.00.01 and insert the first C element in a single revision.
    ImportSchema(s_schemaV01x00x01_addPropC2);
    DgnElementId farId1 = InsertElement("C", {{"PropA", "far_a1"}, {"PropC", "far_c1"}, {"PropC2", "far_c2_1"}});
    ASSERT_TRUE(farId1.IsValid());
    ChangesetPropsPtr farRevision1 = CreateRevision("-far-v01.00.01-and-c1");
    ASSERT_TRUE(farRevision1.IsValid());

    // Both synced at S1 (v01.00.01 + far_c1 element).
    BackupTestFile();

    // Far: insert a second C element (data only — schema unchanged).
    DgnElementId farId2 = InsertElement("C", {{"PropA", "far_a2"}, {"PropC", "far_c2_data"}, {"PropC2", "far_c2_2"}});
    ASSERT_TRUE(farId2.IsValid());
    ChangesetPropsPtr farRevision2 = CreateRevision("-far-c2-data");
    ASSERT_TRUE(farRevision2.IsValid());

    // Local: start from the sync point (v01.00.01 + far_c1 element).
    RestoreTestFile();
    EXPECT_EQ(1u, GetSchemaMinorVersion());

    // Local: import v01.00.02 (additive — adds PropD2 to D).
    // This creates a local Schema-type txn that is NOT pushed as a revision yet.
    ImportSchema(s_schemaV01x00x02_addPropD2);
    AssertLastTxnIsSchemaType();
    EXPECT_EQ(2u, GetSchemaMinorVersion());

    // Local: pull far's second INSERT.
    // Rebase sequence: reverse the local v01.00.02 schema txn → apply far's data
    // changeset → reinstate v01.00.02 via concurrent schema import (adds PropD2 back).
    PullMergeApplyWithConcurrentSchema(*farRevision2);

    EXPECT_EQ(2u, GetSchemaMinorVersion())
        << "local schema should still be v01.00.02 after pull (schema txn reinstated)";
    AssertElementProp(farId1, "PropC", "far_c1");
    AssertElementProp(farId2, "PropC", "far_c2_data");
    }

// ---------------------------------------------------------------------------
// Test 6 — Transforming schema (column remap) arrives from far while local
// has a pending UPDATE.
//
// This is the critical two-user test for ChangesetTransformer integration with
// rebase.  When far pushes a transforming schema that remaps columns, local's
// pending data changeset must be transformed so the binary UPDATE targets the
// new column positions.
//
// Workflow:
//   [setup]  import base schema; insert C element (propC="initial_c"); create revisions
//   [backup] S1 = base schema + C element ("sync point")
//   [far]    import v01.00.01 additive (adds PropC2 to C); create revision 1
//   [far]    import v01.00.02 transforming (moves PropC from C up to A); create revision 2
//   [restore S1] local starts at the sync point
//   [local]  update C.PropC = "local_c_value" (pending, no revision)
//   [local]  PullMergeApply(rev1) — additive, no remap, UPDATE reinstated unchanged
//   [local]  PullMergeApply(rev2) — transforming, column remap, UPDATE must be transformed
//   verify:  schema minor version == 2; C.PropC == "local_c_value"
//
// PropC originally maps to one shared column on class C.  After the transform,
// PropC is defined on base class A and maps to a different shared column.  The
// pending UPDATE changeset (which targets the old column position) must be
// rewritten by ChangesetTransformer to target the new column position.
// ---------------------------------------------------------------------------
TEST_F(ConcurrentSchemaImportTest, TransformingSchemaArriveWhileLocalUpdatePending)
    {
    SetupDgnDb(ChangeTestFixture::s_seedFileInfo.fileName, L"ConcurrentSchema_Test6.bim");
    m_db->SaveChanges("init");

    ImportSchema(s_schemaV01x00x00);
    ASSERT_TRUE(CreateRevision("-base-schema").IsValid());

    DgnElementId cId = InsertElement("C", {{"PropA", "initial_a"}, {"PropC", "initial_c"}});
    ASSERT_TRUE(cId.IsValid());
    ASSERT_TRUE(CreateRevision("-insert-c").IsValid());

    // Both users synced at S1 (base schema + C element).
    BackupTestFile();

    // Far: import additive schema (adds PropC2 — no column remapping).
    ImportSchema(s_schemaV01x00x01_addPropC2);
    ChangesetPropsPtr farRevision1 = CreateRevision("-far-schema-v01.00.01");
    ASSERT_TRUE(farRevision1.IsValid());

    // Far: import transforming schema (moves PropC from C up to A — column remap).
    ImportSchema(s_schemaV01x00x02_movePropCToA);
    ChangesetPropsPtr farRevision2 = CreateRevision("-far-schema-v01.00.02-transform");
    ASSERT_TRUE(farRevision2.IsValid());

    // Local: start from the sync point.
    RestoreTestFile();
    EXPECT_EQ(0u, GetSchemaMinorVersion());

    // Local pending change: update PropC (targets old column position).
    UpdateElementProp(cId, "PropC", "local_c_value");
    AssertElementProp(cId, "PropC", "local_c_value");

    // Local: pull additive schema (no column remap — UPDATE reinstated unchanged).
    PullMergeApplyWithConcurrentSchema(*farRevision1);
    EXPECT_EQ(1u, GetSchemaMinorVersion());
    AssertElementProp(cId, "PropC", "local_c_value");

    // Local: pull transforming schema.  PropC moves from C to A, so the physical
    // column changes.  Rebase must transform the pending UPDATE changeset to target
    // the new column position.
    PullMergeApplyWithConcurrentSchema(*farRevision2);

    EXPECT_EQ(2u, GetSchemaMinorVersion()) << "schema should be v01.00.02 after transform pull";
    AssertElementProp(cId, "PropC", "local_c_value");
    }

// ---------------------------------------------------------------------------
// Test 7 — Transforming schema arrives from far while local has a pending INSERT.
//
// Similar to test 6, but the local pending changeset is an INSERT rather than
// an UPDATE.  The INSERT changeset contains column values at old ordinal positions;
// after the transforming schema is applied, the changeset must be rewritten so
// values appear at the new ordinal positions.
//
// Workflow:
//   [setup]  import base schema; create revision
//   [backup] S0 = base schema only ("sync point")
//   [far]    import v01.00.01 additive; import v01.00.02 transforming; create revisions
//   [restore S0] local starts at base schema
//   [local]  insert C element (PropC="inserted_c_value") — pending INSERT
//   [local]  PullMergeApply(rev1) then PullMergeApply(rev2)
//   verify:  schema minor version == 2; PropC == "inserted_c_value"
// ---------------------------------------------------------------------------
TEST_F(ConcurrentSchemaImportTest, TransformingSchemaArriveWhileLocalInsertPending)
    {
    SetupDgnDb(ChangeTestFixture::s_seedFileInfo.fileName, L"ConcurrentSchema_Test7.bim");
    m_db->SaveChanges("init");

    ImportSchema(s_schemaV01x00x00);
    ASSERT_TRUE(CreateRevision("-base-schema").IsValid());

    // Sync point: base schema, no data.
    BackupTestFile();

    // Far: push additive + transforming schemas.
    ImportSchema(s_schemaV01x00x01_addPropC2);
    ChangesetPropsPtr farRevision1 = CreateRevision("-far-schema-v01.00.01");
    ASSERT_TRUE(farRevision1.IsValid());

    ImportSchema(s_schemaV01x00x02_movePropCToA);
    ChangesetPropsPtr farRevision2 = CreateRevision("-far-schema-v01.00.02-transform");
    ASSERT_TRUE(farRevision2.IsValid());

    // Local: start from the sync point.
    RestoreTestFile();
    EXPECT_EQ(0u, GetSchemaMinorVersion());

    // Local pending INSERT: create a C element with PropC (targets old column position).
    DgnElementId cId = InsertElement("C", {{"PropA", "a_val"}, {"PropC", "inserted_c_value"}});
    ASSERT_TRUE(cId.IsValid());

    // Local: apply both revisions.  After the additive schema (no remap), the INSERT
    // is reinstated unchanged.  After the transforming schema, the INSERT changeset
    // must be rewritten so PropC is at the new column position.
    PullMergeApplyWithConcurrentSchema(*farRevision1);
    PullMergeApplyWithConcurrentSchema(*farRevision2);

    EXPECT_EQ(2u, GetSchemaMinorVersion()) << "schema should be v01.00.02 after transform pulls";
    AssertElementProp(cId, "PropC", "inserted_c_value");
    }
