/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/DgnProject/Published/SchemaVersion_Test.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "../TestFixture/DgnDbTestFixtures.h"

USING_NAMESPACE_BENTLEY_EC

#define SCHEMA_VERSION_TEST_SCHEMA_NAME "SchemaVersionTest"
#define SCHEMA_VERSION_TEST_CLASS_NAME "SchemaVersionTestElement"

DEFINE_POINTER_SUFFIX_TYPEDEFS(SchemaVersionTestElement)
DEFINE_REF_COUNTED_PTR(SchemaVersionTestElement)

// Turn this on for debugging.
#define DUMP_REVISION 1

#define LOG (*BentleyApi::NativeLogging::LoggingManager::GetLogger (L"DgnCore"))

//=======================================================================================
// @bsiclass                                                 Ramanujam.Raman   03/17
//=======================================================================================
struct SchemaVersionTestElement : Dgn::PhysicalElement
    {
    friend struct SchemaVersionTestElementHandler;

    DGNELEMENT_DECLARE_MEMBERS(SCHEMA_VERSION_TEST_CLASS_NAME, Dgn::PhysicalElement)
    public:
        SchemaVersionTestElement(CreateParams const& params) : Dgn::PhysicalElement(params)
            {}

        static Dgn::DgnClassId QueryClassId(Dgn::DgnDbCR dgndb) { return Dgn::DgnClassId(dgndb.Schemas().GetECClassId(SCHEMA_VERSION_TEST_SCHEMA_NAME, SCHEMA_VERSION_TEST_CLASS_NAME)); }

        static SchemaVersionTestElementPtr Create(PhysicalModelR model, DgnCategoryId categoryId, DgnCode code = DgnCode())
            {
            return new SchemaVersionTestElement(CreateParams(model.GetDgnDb(), model.GetModelId(), QueryClassId(model.GetDgnDb()), categoryId, Placement3d(), code));
            }

        SchemaVersionTestElementCPtr Insert(DgnDbStatus* stat = nullptr)
            {
            return GetDgnDb().Elements().Insert<SchemaVersionTestElement>(*this, stat);
            }

        SchemaVersionTestElementCPtr Update(DgnDbStatus* stat = nullptr)
            {
            return GetDgnDb().Elements().Update<SchemaVersionTestElement>(*this, stat);
            }

        static SchemaVersionTestElementCPtr Get(DgnDbCR dgndb, Dgn::DgnElementId elementId)
            {
            return dgndb.Elements().Get<SchemaVersionTestElement>(elementId);
            }

        static SchemaVersionTestElementPtr GetForEdit(DgnDbR dgndb, Dgn::DgnElementId elementId)
            {
            return dgndb.Elements().GetForEdit<SchemaVersionTestElement>(elementId);
            }

        void SetProperty(Utf8CP propertyName, int propertyValue)
            {
            DgnDbStatus status = SetPropertyValue(propertyName, propertyValue);
            BeAssert(status == DgnDbStatus::Success);
            }

        int GetProperty(Utf8CP propertyName)
            {
            return GetPropertyValueInt32(propertyName);
            }

    };

//=======================================================================================
// @bsiclass                                                 Ramanujam.Raman   03/17
//=======================================================================================
struct SchemaVersionTestElementHandler : Dgn::dgn_ElementHandler::Physical
    {
    ELEMENTHANDLER_DECLARE_MEMBERS(SCHEMA_VERSION_TEST_CLASS_NAME, SchemaVersionTestElement, SchemaVersionTestElementHandler, Dgn::dgn_ElementHandler::Physical, )
    };

//=======================================================================================
// @bsiclass                                                 Ramanujam.Raman   03/17
//=======================================================================================
struct SchemaVersionTestDomain : DgnDomain
    {
    DOMAIN_DECLARE_MEMBERS(SchemaVersionTestDomain, )
    private:
        BeFileName m_relativePath;

        WCharCP _GetSchemaRelativePath() const override { return m_relativePath.GetName(); }
        void _OnSchemaImported(DgnDbR db) const override
            {
            PhysicalModelPtr model = DgnDbTestUtils::InsertPhysicalModel(db, "OnSchemaImportedPartition");
            BeAssert(model.IsValid());

            DgnCategoryId categoryId = DgnDbTestUtils::InsertSpatialCategory(db, "SchemaVersionTestCategory");
            BeAssert(categoryId.IsValid());

            DgnCode code = CreateCode(db, "OnSchemaImportedElement");
            
            SchemaVersionTestElementPtr el = SchemaVersionTestElement::Create(*model, categoryId, code);
            SchemaVersionTestElementCPtr cEl = el->Insert();
            BeAssert(cEl.IsValid());
            }

    public:
        SchemaVersionTestDomain() : DgnDomain(SCHEMA_VERSION_TEST_SCHEMA_NAME, "Version Test Domain", 1)
            {
            RegisterHandler(SchemaVersionTestElementHandler::GetHandler());
            }

        void SetVersion(Utf8CP version)
            {
            m_relativePath.SetNameUtf8(Utf8PrintfString("ECSchemas\\" SCHEMA_VERSION_TEST_SCHEMA_NAME ".%s.ecschema.xml", version));
            }

        static void Register(Utf8CP version, DgnDomain::Required isRequired, DgnDomain::Readonly isReadonly)
            {
            SchemaVersionTestDomain::GetDomain().SetVersion(version);
            DgnDomains::RegisterDomain(SchemaVersionTestDomain::GetDomain(), isRequired, isReadonly);
            }

        static DgnCode CreateCode(DgnDbR dgndb, Utf8StringCR value)
            {
            return CodeSpec::CreateCode(dgndb, "SchemaVersionTest", value);
            }

    };

DOMAIN_DEFINE_MEMBERS(SchemaVersionTestDomain)
HANDLER_DEFINE_MEMBERS(SchemaVersionTestElementHandler)

//=======================================================================================
// @bsiclass                                                 Ramanujam.Raman   03/17
//=======================================================================================
struct SchemaVersionTestFixture : public DgnDbTestFixture
{
    WCharCP m_copyTestFileName = L"SchemaVersionTestCopy.ibim";

    static void SetUpTestCase()
        {
        ScopedDgnHost tempHost;
        
        //  Request a root seed file.
        DgnPlatformSeedManager::SeedDbInfo rootSeedInfo = DgnPlatformSeedManager::GetSeedDb(DgnPlatformSeedManager::SeedDbId::OneSpatialModel, DgnPlatformSeedManager::SeedDbOptions(true, true));

        //  The group's seed file is essentially the same as the root seed file, but with a different relative path.
        //  Note that we must put our group seed file in a group-specific sub-directory
        SchemaVersionTestFixture::s_seedFileInfo = rootSeedInfo;
        SchemaVersionTestFixture::s_seedFileInfo.fileName.SetName(L"SchemaVersionTestFixture/seed.bim");

        // Customize for my tests
        DgnDbPtr db = DgnPlatformSeedManager::OpenSeedDbCopy(rootSeedInfo.fileName, SchemaVersionTestFixture::s_seedFileInfo.fileName); // our seed starts as a copy of the root seed
        ASSERT_TRUE(db.IsValid());

        CodeSpecPtr codeSpec = CodeSpec::Create(*db, "SchemaVersionTest");
        if (codeSpec.IsValid())
            codeSpec->Insert();

        db->SaveChanges();
        }

    SchemaVersionTestElementPtr CreateElement()
        {
        PhysicalModelPtr model = GetDefaultPhysicalModel();
        DgnCategoryId categoryId = GetDefaultCategoryId();
        return SchemaVersionTestElement::Create(*model, categoryId);
        }

    void BackupTestFile()
        {
        BeFileName fileName = BeFileName(m_db->GetFileName());
        CloseDb();
        BeFileName originalFile = fileName;
        BeFileName copyFile(originalFile.GetDirectoryName());
        copyFile.AppendToPath(m_copyTestFileName);

        BeFileNameStatus fileStatus = BeFileName::BeCopyFile(originalFile.c_str(), copyFile.c_str());
        ASSERT_TRUE(fileStatus == BeFileNameStatus::Success);
        }

    void RestoreTestFile()
        {
        BeFileName fileName = BeFileName(m_db->GetFileName());
        CloseDb();
        BeFileName originalFile = fileName;
        BeFileName copyFile(originalFile.GetDirectoryName());
        copyFile.AppendToPath(m_copyTestFileName);

        BeFileNameStatus fileStatus = BeFileName::BeCopyFile(copyFile.c_str(), originalFile.c_str());
        ASSERT_TRUE(fileStatus == BeFileNameStatus::Success);
        }

    DgnRevisionPtr CreateRevision()
        {
        DgnRevisionPtr revision = m_db->Revisions().StartCreateRevision();
        if (!revision.IsValid())
            return nullptr;

        RevisionStatus status = m_db->Revisions().FinishCreateRevision();
        if (RevisionStatus::Success != status)
            {
            BeAssert(false);
            return nullptr;
            }

        return revision;
        }

    void DumpRevision(DgnRevisionCR revision, Utf8CP summary)
        {
#ifdef DUMP_REVISION
        LOG.infov("---------------------------------------------------------");
        if (summary != nullptr)
            LOG.infov(summary);
        revision.Dump(*m_db);
        LOG.infov("---------------------------------------------------------");
#endif
        }

};

//---------------------------------------------------------------------------------------
// @bsimethod                                Ramanujam.Raman                    03/17
//---------------------------------------------------------------------------------------
TEST_F(SchemaVersionTestFixture, ImportDomainSchemas)
    {
    SetupSeedProject();
    BeFileName fileName = m_db->GetFileName();
    SaveDb();
    CloseDb();

    DbResult result = BE_SQLITE_OK;
    DgnDbStatus status = DgnDbStatus::Success;

    /*
    * Reopen Db after optional registration - BE_SQLITE_OK, schemas are not imported
    */
    SchemaVersionTestDomain::Register("02.02.02", DgnDomain::Required::No, DgnDomain::Readonly::No);
    m_db = DgnDb::OpenDgnDb(&result, fileName, DgnDb::OpenParams(DgnDb::OpenMode::ReadWrite));
    EXPECT_TRUE(result == BE_SQLITE_OK);
    EXPECT_FALSE(m_db->Schemas().ContainsECSchema(SCHEMA_VERSION_TEST_SCHEMA_NAME));
    EXPECT_FALSE(SchemaVersionTestDomain::GetDomain().IsSchemaImported(*m_db));
    CloseDb();

    /*
    * Reopen Db after required registration - BE_SQLITE_ERROR_SchemaImportRequired, schemas are not imported
    */
    SchemaVersionTestDomain::GetDomain().SetRequired(DgnDomain::Required::Yes);
    m_db = DgnDb::OpenDgnDb(&result, fileName, DgnDb::OpenParams(DgnDb::OpenMode::ReadWrite));
    EXPECT_TRUE(result == BE_SQLITE_ERROR_SchemaImportRequired);

    /*
    * Reopen Db after read-write registration with EnableSchemaImport flag - BE_SQLITE_OK, schemas can be manually imported, but cannot write to any domain, 
    */
    SchemaVersionTestDomain::GetDomain().SetRequired(DgnDomain::Required::Yes);
    m_db = DgnDb::OpenDgnDb(&result, fileName, DgnDb::OpenParams(DgnDb::OpenMode::ReadWrite, BeSQLite::DefaultTxn::Yes, DgnDb::OpenParams::EnableSchemaImport::Yes));
    EXPECT_TRUE(result == BE_SQLITE_OK);
    EXPECT_FALSE(m_db->Schemas().ContainsECSchema(SCHEMA_VERSION_TEST_SCHEMA_NAME));
    EXPECT_FALSE(SchemaVersionTestDomain::GetDomain().IsSchemaImported(*m_db));

    // Import domain schema
    SchemaVersionTestDomain::GetDomain().ImportSchema(*m_db);
    EXPECT_TRUE(m_db->Schemas().ContainsECSchema(SCHEMA_VERSION_TEST_SCHEMA_NAME));
    EXPECT_TRUE(SchemaVersionTestDomain::GetDomain().IsSchemaImported(*m_db));

    // Validate that _OnSchemaImport was called
    DgnCode code = SchemaVersionTestDomain::CreateCode(*m_db, "OnSchemaImportedElement");
    DgnElementId elId = m_db->Elements().QueryElementIdByCode(code);
    EXPECT_TRUE(elId.IsValid());

    SaveDb();
    CloseDb();

    /* 
     * Reopen Db - BE_SQLITE_OK, can write to the Db
     */
    SchemaVersionTestDomain::GetDomain().SetReadonly(DgnDomain::Readonly::No);
    m_db = DgnDb::OpenDgnDb(&result, fileName, DgnDb::OpenParams(DgnDb::OpenMode::ReadWrite));
    EXPECT_TRUE(result == BE_SQLITE_OK);
    EXPECT_TRUE(m_db->Schemas().ContainsECSchema(SCHEMA_VERSION_TEST_SCHEMA_NAME));
    EXPECT_TRUE(SchemaVersionTestDomain::GetDomain().IsSchemaImported(*m_db));

    SchemaVersionTestElementPtr el = CreateElement();
    el->SetProperty("IntegerProperty1", 1);
    SchemaVersionTestElementCPtr cEl = el->Insert(&status);
    EXPECT_TRUE(cEl.IsValid());

    elId = cEl->GetElementId();

    el = nullptr;
    cEl = nullptr;
    SaveDb();
    CloseDb();

    /*
    * Reopen Db after read-only registration - no error, but cannot insert or update or delete elements
    */
    SchemaVersionTestDomain::GetDomain().SetReadonly(DgnDomain::Readonly::Yes);
    m_db = DgnDb::OpenDgnDb(&result, fileName, DgnDb::OpenParams(DgnDb::OpenMode::ReadWrite));
    EXPECT_TRUE(result == BE_SQLITE_OK);

    el = CreateElement();
    el->SetProperty("IntegerProperty1", 1);
    cEl = el->Insert(&status);
    EXPECT_FALSE(cEl.IsValid());
    EXPECT_TRUE(status == DgnDbStatus::ReadOnlyDomain);

    el = SchemaVersionTestElement::GetForEdit(*m_db, elId);
    EXPECT_TRUE(el.IsValid());
    el->SetProperty("IntegerProperty1", 2);
    cEl = el->Update(&status);
    EXPECT_FALSE(cEl.IsValid());
    EXPECT_TRUE(status == DgnDbStatus::ReadOnlyDomain);

    status = m_db->Elements().Delete(elId);
    EXPECT_TRUE(status == DgnDbStatus::ReadOnlyDomain);

    el = nullptr;
    cEl = nullptr;
    CloseDb();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Ramanujam.Raman                    03/17
//---------------------------------------------------------------------------------------
TEST_F(SchemaVersionTestFixture, UpgradeDomainSchemas)
    {
    DbResult result = BE_SQLITE_OK;

    //! @note The ECSchemas supplied by registered DgnDomain-s are validated against the corresponding ones in the DgnDb, and 
    //! an appropriate error status is returned if necessary. See table below for the various ECSchema compatibility errors, 
    //! and @see OpenParams for setting the option to upgrade the ECSchema-s. 
    //! <pre>
    //! Sample schema compatibility validation results for an ECSchema in the BIM with Version 2.2.2 (Read.Write.Minor)
    //! ----------------------------------------------------------------------------------------------
    //! Application   |  Validation result                    | Validation result
    //! Version       |  (Readonly)                           | (ReadWrite)
    //! ----------------------------------------------------------------------------------------------
    //! 2.2.2 (same)  | BE_SQLITE_OK                          | BE_SQLITE_OK
    //! ----------------------------------------------------------------------------------------------
    //! 1.2.2 (older) | BE_SQLITE_ERROR_SchemaTooNew          | BE_SQLITE_ERROR_SchemaTooNew
    //! 2.1.2 (older) | BE_SQLITE_OK                          | BE_SQLITE_ERROR_SchemaTooNew
    //! 2.2.1 (older) | BE_SQLITE_OK                          | BE_SQLITE_OK
    //! ----------------------------------------------------------------------------------------------
    //! 3.2.2 (newer) | BE_SQLITE_ERROR_SchemaTooOld          | BE_SQLITE_ERROR_SchemaTooOld
    //! 2.3.2 (newer) | BE_SQLITE_ERROR_SchemaImportRequired  | BE_SQLITE_ERROR_SchemaImportRequired
    //! 2.2.3 (newer) | BE_SQLITE_ERROR_SchemaImportRequired  | BE_SQLITE_ERROR_SchemaImportRequired
    //! ----------------------------------------------------------------------------------------------
    //! </pre>

    /* 
     * Setup BIM with version 2.2.2 
     */
    SetupSeedProject();
    BeFileName fileName = m_db->GetFileName();
    SaveDb();

    SchemaVersionTestDomain::Register("02.02.02", DgnDomain::Required::Yes, DgnDomain::Readonly::No);
    m_db = DgnDb::OpenDgnDb(&result, fileName, DgnDb::OpenParams(DgnDb::OpenMode::ReadWrite, BeSQLite::DefaultTxn::Yes, DgnDb::OpenParams::EnableSchemaImport::Yes));
    EXPECT_TRUE(result == BE_SQLITE_OK);
    EXPECT_FALSE(m_db->Schemas().ContainsECSchema(SCHEMA_VERSION_TEST_SCHEMA_NAME));

    SchemaVersionTestDomain::GetDomain().ImportSchema(*m_db);
    EXPECT_TRUE(m_db->Schemas().ContainsECSchema(SCHEMA_VERSION_TEST_SCHEMA_NAME));

    SchemaVersionTestDomain::GetDomain().SetReadonly(DgnDomain::Readonly::No);
    SaveDb();

    BackupTestFile();

    /* 
     * Application same as BIM 
     */
    m_db = DgnDb::OpenDgnDb(&result, fileName, DgnDb::OpenParams(DgnDb::OpenMode::Readonly));
    EXPECT_TRUE(result == BE_SQLITE_OK);
    CloseDb();
    m_db = DgnDb::OpenDgnDb(&result, fileName, DgnDb::OpenParams(DgnDb::OpenMode::ReadWrite));
    EXPECT_TRUE(result == BE_SQLITE_OK);
    CloseDb();

    /* 
     * Application older than BIM 
     */
    SchemaVersionTestDomain::GetDomain().SetVersion("01.02.02");
    m_db = DgnDb::OpenDgnDb(&result, fileName, DgnDb::OpenParams(DgnDb::OpenMode::Readonly));
    EXPECT_TRUE(result == BE_SQLITE_ERROR_SchemaTooNew);
    m_db = DgnDb::OpenDgnDb(&result, fileName, DgnDb::OpenParams(DgnDb::OpenMode::ReadWrite));
    EXPECT_TRUE(result == BE_SQLITE_ERROR_SchemaTooNew);

    SchemaVersionTestDomain::GetDomain().SetVersion("02.01.02");
    m_db = DgnDb::OpenDgnDb(&result, fileName, DgnDb::OpenParams(DgnDb::OpenMode::ReadWrite));
    EXPECT_TRUE(result == BE_SQLITE_ERROR_SchemaTooNew);
    m_db = DgnDb::OpenDgnDb(&result, fileName, DgnDb::OpenParams(DgnDb::OpenMode::Readonly));
    EXPECT_TRUE(result == BE_SQLITE_OK);

    SchemaVersionTestDomain::GetDomain().SetVersion("02.02.01");
    m_db = DgnDb::OpenDgnDb(&result, fileName, DgnDb::OpenParams(DgnDb::OpenMode::ReadWrite));
    EXPECT_TRUE(result == BE_SQLITE_OK);
    SaveDb();
    RestoreTestFile();
    m_db = DgnDb::OpenDgnDb(&result, fileName, DgnDb::OpenParams(DgnDb::OpenMode::Readonly));
    EXPECT_TRUE(result == BE_SQLITE_OK);
    CloseDb();

    /* 
     * Application newer than BIM 
     */
    SchemaVersionTestDomain::GetDomain().SetVersion("03.02.02");
    m_db = DgnDb::OpenDgnDb(&result, fileName, DgnDb::OpenParams(DgnDb::OpenMode::Readonly));
    EXPECT_TRUE(result == BE_SQLITE_ERROR_SchemaTooOld);
    m_db = DgnDb::OpenDgnDb(&result, fileName, DgnDb::OpenParams(DgnDb::OpenMode::ReadWrite));
    EXPECT_TRUE(result == BE_SQLITE_ERROR_SchemaTooOld);
    BeTest::SetFailOnAssert(false);
    m_db = DgnDb::OpenDgnDb(&result, fileName, DgnDb::OpenParams(DgnDb::OpenMode::ReadWrite, BeSQLite::DefaultTxn::Yes, DgnDb::OpenParams::EnableSchemaImport::Yes));
    EXPECT_TRUE(result == BE_SQLITE_ERROR_SchemaTooOld);
    BeTest::SetFailOnAssert(true);

    SchemaVersionTestDomain::GetDomain().SetVersion("02.03.02");
    m_db = DgnDb::OpenDgnDb(&result, fileName, DgnDb::OpenParams(DgnDb::OpenMode::Readonly));
    EXPECT_TRUE(result == BE_SQLITE_ERROR_SchemaImportRequired);
    m_db = DgnDb::OpenDgnDb(&result, fileName, DgnDb::OpenParams(DgnDb::OpenMode::ReadWrite));
    EXPECT_TRUE(result == BE_SQLITE_ERROR_SchemaImportRequired);
    m_db = DgnDb::OpenDgnDb(&result, fileName, DgnDb::OpenParams(DgnDb::OpenMode::ReadWrite, BeSQLite::DefaultTxn::Yes, DgnDb::OpenParams::EnableSchemaImport::Yes));
    EXPECT_TRUE(result == BE_SQLITE_OK);
    result = m_db->Domains().ImportSchemas();
    EXPECT_TRUE(result == BE_SQLITE_OK);
    SaveDb();
    CloseDb();
    m_db = DgnDb::OpenDgnDb(&result, fileName, DgnDb::OpenParams(DgnDb::OpenMode::ReadWrite));
    EXPECT_TRUE(result == BE_SQLITE_OK);
    CloseDb();

    RestoreTestFile();
    SchemaVersionTestDomain::GetDomain().SetVersion("02.02.03");
    m_db = DgnDb::OpenDgnDb(&result, fileName, DgnDb::OpenParams(DgnDb::OpenMode::Readonly));
    EXPECT_TRUE(result == BE_SQLITE_ERROR_SchemaImportRequired);
    m_db = DgnDb::OpenDgnDb(&result, fileName, DgnDb::OpenParams(DgnDb::OpenMode::ReadWrite));
    EXPECT_TRUE(result == BE_SQLITE_ERROR_SchemaImportRequired);
    m_db = DgnDb::OpenDgnDb(&result, fileName, DgnDb::OpenParams(DgnDb::OpenMode::ReadWrite, BeSQLite::DefaultTxn::Yes, DgnDb::OpenParams::EnableSchemaImport::Yes));
    EXPECT_TRUE(result == BE_SQLITE_OK);
    result = m_db->Domains().ImportSchemas();
    EXPECT_TRUE(result == BE_SQLITE_OK);
    SaveDb();
    CloseDb();
    m_db = DgnDb::OpenDgnDb(&result, fileName, DgnDb::OpenParams(DgnDb::OpenMode::ReadWrite));
    EXPECT_TRUE(result == BE_SQLITE_OK);
    CloseDb();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                Ramanujam.Raman                    03/17
//---------------------------------------------------------------------------------------
TEST_F(SchemaVersionTestFixture, CreateAndMergeRevision)
    {
    DbResult result = BE_SQLITE_OK;
    ECPropertyCP testProperty = nullptr;
    RevisionStatus status = RevisionStatus::Success;

    /* Setup baseline */
    SetupSeedProject();
    SaveDb();
    DgnRevisionPtr revision0 = CreateRevision();
    EXPECT_TRUE(revision0.IsValid());
    BeFileName fileName = m_db->GetFileName();
    BackupTestFile();

    /* Create revision with schema import */
    SchemaVersionTestDomain::Register("02.02.02", DgnDomain::Required::No, DgnDomain::Readonly::No);
    m_db = DgnDb::OpenDgnDb(&result, fileName, DgnDb::OpenParams(DgnDb::OpenMode::ReadWrite));
    EXPECT_TRUE(result == BE_SQLITE_OK);
    EXPECT_FALSE(m_db->Schemas().ContainsECSchema(SCHEMA_VERSION_TEST_SCHEMA_NAME));

    result = SchemaVersionTestDomain::GetDomain().ImportSchema(*m_db);
    EXPECT_EQ(BE_SQLITE_OK, result);
    
    testProperty = m_db->Schemas().GetECClass(SchemaVersionTestElement::QueryClassId(*m_db))->GetPropertyP("IntegerProperty3");
    EXPECT_TRUE(testProperty != nullptr);
    testProperty = m_db->Schemas().GetECClass(SchemaVersionTestElement::QueryClassId(*m_db))->GetPropertyP("IntegerProperty4");
    EXPECT_TRUE(testProperty == nullptr);

    SaveDb();
    DgnRevisionPtr revision1 = CreateRevision();
    EXPECT_TRUE(revision1.IsValid());

    DumpRevision(*revision1, "Revision 1");

    /* Create revision with schema upgrade */
    SchemaVersionTestDomain::GetDomain().SetVersion("02.03.02");
    m_db = DgnDb::OpenDgnDb(&result, fileName, DgnDb::OpenParams(DgnDb::OpenMode::ReadWrite, BeSQLite::DefaultTxn::Yes, DgnDb::OpenParams::EnableSchemaImport::Yes));
    EXPECT_TRUE(result == BE_SQLITE_OK);

    result = m_db->Domains().ImportSchemas();
    EXPECT_EQ(BE_SQLITE_OK, result);

    testProperty = m_db->Schemas().GetECClass(SchemaVersionTestElement::QueryClassId(*m_db))->GetPropertyP("IntegerProperty4");
    EXPECT_TRUE(testProperty != nullptr);

    SaveDb();
    DgnRevisionPtr revision2 = CreateRevision();
    EXPECT_TRUE(revision2.IsValid());

    DumpRevision(*revision2, "Revision 2");

    /* Restore baseline */
    RestoreTestFile();
   
    SchemaVersionTestDomain::GetDomain().SetRequired(DgnDomain::Required::No);
    SchemaVersionTestDomain::GetDomain().SetVersion("02.02.02");
    m_db = DgnDb::OpenDgnDb(&result, fileName, DgnDb::OpenParams(DgnDb::OpenMode::ReadWrite));
    EXPECT_TRUE(result == BE_SQLITE_OK);

    EXPECT_FALSE(m_db->Schemas().ContainsECSchema(SCHEMA_VERSION_TEST_SCHEMA_NAME));

    /* Merge revision with schema import and validate */
    status = m_db->Revisions().MergeRevision(*revision1);
    ASSERT_TRUE(status == RevisionStatus::Success);

    EXPECT_TRUE(m_db->Schemas().ContainsECSchema(SCHEMA_VERSION_TEST_SCHEMA_NAME));

    testProperty = m_db->Schemas().GetECClass(SchemaVersionTestElement::QueryClassId(*m_db))->GetPropertyP("IntegerProperty3");
    EXPECT_TRUE(testProperty != nullptr);
    testProperty = m_db->Schemas().GetECClass(SchemaVersionTestElement::QueryClassId(*m_db))->GetPropertyP("IntegerProperty4");
    EXPECT_TRUE(testProperty == nullptr);

    /* Merge revision with schema upgrade and validate */
    status = m_db->Revisions().MergeRevision(*revision2);
    ASSERT_TRUE(status == RevisionStatus::Success);

    m_db->ClearECDbCache(); // NEEDS_WORK: The method should be automatically called when merging revisions containing schemas

    SchemaVersionTestDomain::GetDomain().SetVersion("02.03.02");
    m_db = DgnDb::OpenDgnDb(&result, fileName, DgnDb::OpenParams(DgnDb::OpenMode::ReadWrite));
    EXPECT_TRUE(result == BE_SQLITE_OK);

    testProperty = m_db->Schemas().GetECClass(SchemaVersionTestElement::QueryClassId(*m_db))->GetPropertyP("IntegerProperty3");
    EXPECT_TRUE(testProperty != nullptr);
    testProperty = m_db->Schemas().GetECClass(SchemaVersionTestElement::QueryClassId(*m_db))->GetPropertyP("IntegerProperty4");
    EXPECT_TRUE(testProperty != nullptr);
    }
