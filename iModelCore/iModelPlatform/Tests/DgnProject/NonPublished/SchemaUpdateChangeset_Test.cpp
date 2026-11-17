/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "ChangeTestFixture.h"
#include <DgnPlatform/GenericDomain.h>
#include <ECDb/ChangeIterator.h>
#include <ECDb/ChangeSummary.h>
#include <array>

USING_NAMESPACE_BENTLEY_DGN
USING_NAMESPACE_BENTLEY_SQLITE
USING_NAMESPACE_BENTLEY_SQLITE_EC
USING_NAMESPACE_BENTLEY_DPTEST
USING_NAMESPACE_BENTLEY_EC

//=======================================================================================
// @bsiclass
//=======================================================================================
struct SchemaUpdateChangesetFixture : ChangeTestFixture
{
DEFINE_T_SUPER(ChangeTestFixture)
protected:
    WCharCP m_copyTestFileName = L"SchemaUpdateChangesetCopy.bim";
    DgnModelId m_definitionModelId;

    static DgnPlatformSeedManager::SeedDbInfo s_seedFileInfo;

public:
    static void SetUpTestCase()
        {
        ScopedDgnHost tempHost;
        DgnPlatformSeedManager::SeedDbInfo rootSeedInfo = DgnPlatformSeedManager::GetSeedDb(ChangeTestFixture::s_seedFileInfo.id, DgnPlatformSeedManager::SeedDbOptions(true, true));

        s_seedFileInfo = rootSeedInfo;
        s_seedFileInfo.fileName.SetName(L"SchemaUpdateChangesetFixture/SchemaUpdateChangesetFixture.bim");

        DgnDbPtr db = DgnPlatformSeedManager::OpenSeedDbCopy(rootSeedInfo.fileName, s_seedFileInfo.fileName);
        ASSERT_TRUE(db.IsValid());
        TestDataManager::SetAsStandaloneDb(db, Db::OpenMode::ReadWrite);

        m_defaultCodeSpecId = DgnDbTestUtils::InsertCodeSpec(*db, "TestCodeSpec");
        ASSERT_TRUE(m_defaultCodeSpecId.IsValid());
        db->SaveChanges();
        }

protected:
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
        ASSERT_TRUE(BeFileNameStatus::Success == BeFileName::BeCopyFile(fileName.c_str(), copyFile.c_str()));
        OpenIModelDb(fileName);
        }

    void RestoreTestFile()
        {
        BeFileName fileName = BeFileName(m_db->GetDbFileName(), true);
        CloseDgnDb();
        BeFileName copyFile(fileName.GetDirectoryName());
        copyFile.AppendToPath(m_copyTestFileName);
        ASSERT_TRUE(BeFileNameStatus::Success == BeFileName::BeCopyFile(copyFile.c_str(), fileName.c_str()));
        OpenIModelDb(fileName);
        }

    //! Named variants of Backup/Restore for tests that need more than one saved briefcase image.
    void SaveImageAs(WCharCP imageName)
        {
        BeFileName fileName = BeFileName(m_db->GetDbFileName(), true);
        CloseDgnDb();
        BeFileName copyFile(fileName.GetDirectoryName());
        copyFile.AppendToPath(imageName);
        ASSERT_TRUE(BeFileNameStatus::Success == BeFileName::BeCopyFile(fileName.c_str(), copyFile.c_str()));
        OpenIModelDb(fileName);
        }

    void LoadImage(WCharCP imageName)
        {
        BeFileName fileName = BeFileName(m_db->GetDbFileName(), true);
        CloseDgnDb();
        BeFileName copyFile(fileName.GetDirectoryName());
        copyFile.AppendToPath(imageName);
        ASSERT_TRUE(BeFileNameStatus::Success == BeFileName::BeCopyFile(copyFile.c_str(), fileName.c_str()));
        OpenIModelDb(fileName);
        }

    void ApplyChangeset(ChangesetPropsCR revision)
        {
        EXPECT_EQ(ChangesetStatus::Success, m_db->Txns().PullMergeApply(revision)) << revision.GetChangesetId().c_str();
        }

    //! The ElementProps JSON of an element as the TypeScript side receives it (DgnElement::ToJson).
    Utf8String ElementJson(DgnElementId id)
        {
        DgnElementCPtr element = m_db->Elements().GetElement(id);
        if (!element.IsValid())
            return "<no element>";
        BeJsDocument json;
        element->ToJson(json);
        return json.Stringify();
        }

    //! Tries to write @p name through the element API and returns the status, without saving on failure.
    DgnDbStatus TrySetName(DgnElementId id, ECValueCR name)
        {
        DgnElementPtr element = m_db->Elements().GetForEdit<DgnElement>(id);
        if (!element.IsValid())
            return DgnDbStatus::BadElement;
        DgnDbStatus status = element->SetPropertyValue("Name", name);
        if (DgnDbStatus::Success != status)
            return status;
        status = element->Update();
        if (DgnDbStatus::Success == status)
            m_db->SaveChanges("TrySetName");
        return status;
        }

    //! The new value of Name for one instance as ChangeSummary (the public change-history API) extracts it under the schema in m_db.
    Utf8String NameInChangeSummary(ChangesetPropsCR changeset, DgnElementId id)
        {
        ChangesetFileReader reader(changeset.GetFileName(), m_db.get());
        ChangeGroup group;
        reader.AddToChangeGroup(group);
        ChangeSet changeSet;
        changeSet.FromChangeGroup(group);
        ChangeSummary summary(*m_db);
        if (SUCCESS != summary.FromChangeSet(changeSet))
            return "<FromChangeSet failed>";
        ChangeSummary::Instance instance = summary.GetInstance(m_db->Schemas().GetClassId("TestSchema", "TestElement"), ECInstanceId(id.GetValue()));
        if (!instance.IsValid() || !instance.ContainsValue("Name"))
            return "<no Name value>";
        DbDupValue value = instance.GetNewValue("Name");
        return Utf8PrintfString("%s (sqlite type %d)", value.Format(4).c_str(), (int) value.GetValueType());
        }

    //---------------------------------------------------------------------------------
    // Schema and data helpers.
    //---------------------------------------------------------------------------------

    //! Dynamic schema (the only way through gate G in an iModel) with one class whose Name has the given type.
    static Utf8String SchemaXml(Utf8CP version, Utf8CP nameType)
        {
        return Utf8PrintfString(R"xml(<?xml version="1.0" encoding="utf-8"?>
            <ECSchema schemaName="TestSchema" alias="ts" version="%s" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
                <ECSchemaReference name="BisCore" version="01.00.00" alias="bis" />
                <ECSchemaReference name="CoreCustomAttributes" version="01.00.03" alias="CoreCA" />
                <ECCustomAttributes>
                    <DynamicSchema xmlns="CoreCustomAttributes.01.00.03" />
                </ECCustomAttributes>
                <ECEntityClass typeName="TestElement">
                    <BaseClass>bis:DefinitionElement</BaseClass>
                    <ECProperty propertyName="Name" typeName="%s" />
                    <ECProperty propertyName="Count" typeName="int" />
                </ECEntityClass>
            </ECSchema>)xml", version, nameType);
        }

    //! Name is a dateTime, with or without a DateTimeInfo(Utc) custom attribute (X08: CA-only change, no mapping effect).
    static Utf8String DateTimeSchemaXml(Utf8CP version, bool utc)
        {
        return Utf8PrintfString(R"xml(<?xml version="1.0" encoding="utf-8"?>
            <ECSchema schemaName="TestSchema" alias="ts" version="%s" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
                <ECSchemaReference name="BisCore" version="01.00.00" alias="bis" />
                <ECSchemaReference name="CoreCustomAttributes" version="01.00.03" alias="CoreCA" />
                <ECCustomAttributes>
                    <DynamicSchema xmlns="CoreCustomAttributes.01.00.03" />
                </ECCustomAttributes>
                <ECEntityClass typeName="TestElement">
                    <BaseClass>bis:DefinitionElement</BaseClass>
                    <ECProperty propertyName="Name" typeName="dateTime">%s</ECProperty>
                    <ECProperty propertyName="Count" typeName="int" />
                </ECEntityClass>
            </ECSchema>)xml", version,
            utc ? R"xml(<ECCustomAttributes><DateTimeInfo xmlns="CoreCustomAttributes.01.00.03"><DateTimeKind>Utc</DateTimeKind></DateTimeInfo></ECCustomAttributes>)xml" : "");
        }

    //! Two-level hierarchy for the data-transform case. Count is declared on TestElement (int) in v1 and moved
    //! to TestBase as a double in v2. ECDb frees the old column and moves the values (RemapManager), which
    //! needs AllowDataTransformDuringSchemaUpgrade; DgnDb::ImportSchemas grants it when the schema lock is held.
    static Utf8String HierarchySchemaXml(Utf8CP version, bool countOnBase)
        {
        return Utf8PrintfString(R"xml(<?xml version="1.0" encoding="utf-8"?>
            <ECSchema schemaName="TestSchema" alias="ts" version="%s" xmlns="http://www.bentley.com/schemas/Bentley.ECXML.3.2">
                <ECSchemaReference name="BisCore" version="01.00.00" alias="bis" />
                <ECSchemaReference name="CoreCustomAttributes" version="01.00.03" alias="CoreCA" />
                <ECCustomAttributes>
                    <DynamicSchema xmlns="CoreCustomAttributes.01.00.03" />
                </ECCustomAttributes>
                <ECEntityClass typeName="TestBase">
                    <BaseClass>bis:DefinitionElement</BaseClass>
                    <ECProperty propertyName="Name" typeName="string" />
                    %s
                </ECEntityClass>
                <ECEntityClass typeName="TestElement">
                    <BaseClass>TestBase</BaseClass>
                    %s
                </ECEntityClass>
            </ECSchema>)xml", version,
            countOnBase ? R"xml(<ECProperty propertyName="Count" typeName="double" />)xml" : "",
            countOnBase ? "" : R"xml(<ECProperty propertyName="Count" typeName="int" />)xml");
        }

    //! Reads Name through the element API (DgnElement::GetPropertyValue), as element code does, not through ECSQL.
    Utf8String ReadNameViaElementApi(DgnElementId id)
        {
        DgnElementCPtr element = m_db->Elements().GetElement(id);
        if (!element.IsValid())
            return "<no element>";
        ECValue value;
        if (DgnDbStatus::Success != element->GetPropertyValue(value, "Name"))
            return "<GetPropertyValue failed>";
        return value.IsNull() ? Utf8String("<null>") : value.ToString();
        }

    //! Every mapped Name value in a changeset as ChangeIterator decodes it under the schema currently in m_db.
    bvector<Utf8String> NameValuesInChangeset(ChangesetPropsCR changeset)
        {
        bvector<Utf8String> values;
        ChangesetFileReader reader(changeset.GetFileName(), m_db.get());
        ChangeGroup group;
        reader.AddToChangeGroup(group);
        ChangeSet changeSet;
        changeSet.FromChangeGroup(group);
        ChangeIterator iterator(*m_db, changeSet);
        for (auto const& entry : iterator)
            {
            if (!entry.IsMapped())
                continue;
            for (auto const& column : entry.MakePrimaryColumnIterator())
                {
                if (column.GetPropertyAccessString() != "Name")
                    continue;
                DbDupValue newValue = column.GetValue(Changes::Change::Stage::New);
                if (newValue.IsValid())
                    values.push_back(Utf8PrintfString("%s (sqlite type %d)", newValue.Format(4).c_str(), (int) newValue.GetValueType()));
                }
            }
        return values;
        }

    void ImportSchemaXml(Utf8StringCR xml, Utf8CP saveMessage)
        {
        ECSchemaReadContextPtr context = ECSchemaReadContext::CreateContext();
        context->AddSchemaLocater(m_db->GetSchemaLocater());
        ECSchemaPtr schema;
        ASSERT_EQ(SchemaReadStatus::Success, ECSchema::ReadFromXmlString(schema, xml.c_str(), *context));
        ASSERT_EQ(SchemaStatus::Success, m_db->ImportSchemas({schema.get()}, true));
        ASSERT_EQ(BE_SQLITE_OK, m_db->SaveChanges(saveMessage));
        }

    void InsertDefinitionModel()
        {
        auto partition = DefinitionPartition::CreateAndInsert(*m_db->Elements().GetRootSubject(), "SchemaUpdateChangesetPartition");
        ASSERT_TRUE(partition.IsValid());
        auto model = DefinitionModel::CreateAndInsert(*partition);
        ASSERT_TRUE(model.IsValid());
        m_definitionModelId = model->GetModelId();
        }

    DgnElementId InsertTestElement(ECValueCR name, int count)
        {
        DgnElementPtr element = new DefinitionElement(DefinitionElement::CreateParams(*m_db, m_definitionModelId, m_db->Schemas().GetClassId("TestSchema", "TestElement")));
        EXPECT_EQ(DgnDbStatus::Success, element->SetPropertyValue("Name", name));
        EXPECT_EQ(DgnDbStatus::Success, element->SetPropertyValue("Count", ECValue(count)));
        DgnElementCPtr inserted = m_db->Elements().Insert(*element);
        EXPECT_TRUE(inserted.IsValid());
        return inserted.IsValid() ? inserted->GetElementId() : DgnElementId();
        }

    void SetName(DgnElementId id, ECValueCR name)
        {
        DgnElementPtr element = m_db->Elements().GetForEdit<DgnElement>(id);
        ASSERT_TRUE(element.IsValid());
        ASSERT_EQ(DgnDbStatus::Success, element->SetPropertyValue("Name", name));
        ASSERT_EQ(DgnDbStatus::Success, element->Update());
        ASSERT_EQ(BE_SQLITE_OK, m_db->SaveChanges("SetName"));
        }

    //! One line per row: "<text getter> | <dateTime getter>", so a difference between briefcases in either view shows up.
    //! Only call after the type change: GetValueDateTime is not meaningful on a string property.
    bvector<Utf8String> ReadNamesAsDateTime()
        {
        bvector<Utf8String> rows;
        ECSqlStatement stmt;
        EXPECT_EQ(ECSqlStatus::Success, stmt.Prepare(*m_db, "SELECT Name, [Count] FROM ts.TestElement ORDER BY ECInstanceId"));
        while (BE_SQLITE_ROW == stmt.Step())
            {
            Utf8CP text = stmt.GetValueText(0);
            rows.push_back(Utf8PrintfString("%s | %s | count=%d", text != nullptr ? text : "<null>", stmt.GetValueDateTime(0).ToString().c_str(), stmt.GetValueInt(1)));
            }
        return rows;
        }

    bvector<Utf8String> ReadNamesAsText()
        {
        bvector<Utf8String> rows;
        ECSqlStatement stmt;
        EXPECT_EQ(ECSqlStatus::Success, stmt.Prepare(*m_db, "SELECT Name, [Count] FROM ts.TestElement ORDER BY ECInstanceId"));
        while (BE_SQLITE_ROW == stmt.Step())
            {
            Utf8CP text = stmt.GetValueText(0);
            rows.push_back(Utf8PrintfString("%s | count=%d", text != nullptr ? text : "<null>", stmt.GetValueInt(1)));
            }
        return rows;
        }

    static void ExpectSameRows(bvector<Utf8String> const& briefcaseA, bvector<Utf8String> const& briefcaseB)
        {
        ASSERT_EQ(briefcaseA.size(), briefcaseB.size());
        for (size_t i = 0; i < briefcaseA.size(); i++)
            EXPECT_STREQ(briefcaseA[i].c_str(), briefcaseB[i].c_str()) << "row " << i;
        }

    //! Baseline: schema v1 (Name is a string), definition model, then the baseline changeset. Briefcase B is the copy taken here.
    ChangesetPropsPtr SetUpBaseline(WCharCP fileName, Utf8StringCR baselineSchemaXml = SchemaXml("01.00.00", "string"))
        {
        SetupDgnDb(s_seedFileInfo.fileName, fileName);
        m_db->SaveChanges("Created Initial Model");
        EXPECT_TRUE(CreateRevision("-init").IsValid());

        ImportSchemaXml(baselineSchemaXml, "Imported baseline schema");
        InsertDefinitionModel();
        EXPECT_EQ(BE_SQLITE_OK, m_db->SaveChanges("Definition model"));
        ChangesetPropsPtr baseline = CreateRevision("-base");
        EXPECT_TRUE(baseline.IsValid());
        BackupTestFile();
        return baseline;
        }
};

DgnPlatformSeedManager::SeedDbInfo SchemaUpdateChangesetFixture::s_seedFileInfo;

//---------------------------------------------------------------------------------------
// Data changeset made under the old type, then the schema changeset with the type change.
// Briefcase B applies both in order. The change is expected to be metadata only, so the data
// changeset needs no special handling, and B should read exactly what A reads.
// @bsimethod
//---------------------------------------------------------------------------------------
TEST_F(SchemaUpdateChangesetFixture, TypeChange_DataFirstThenSchemaChangeset_BriefcaseBReadsSameAsA)
    {
    ChangesetPropsPtr baseline = SetUpBaseline(L"SchemaUpdateChangeset_DataThenSchema.bim");
    ASSERT_TRUE(baseline.IsValid());

    // A: rows written while Name is a string (the customer's ISO 8601 text).
    DgnElementId first = InsertTestElement(ECValue("2020-03-10T10:30:00.000Z"), 1);
    InsertTestElement(ECValue("2020-03-11T10:30:00.000Z"), 2);
    InsertTestElement(ECValue("2020-03-12T10:30:00.000Z"), 3);
    ASSERT_EQ(BE_SQLITE_OK, m_db->SaveChanges("Data under the old type"));
    ChangesetPropsPtr dataChangeset = CreateRevision("-data");
    ASSERT_TRUE(dataChangeset.IsValid());
    ASSERT_FALSE(dataChangeset->ContainsDdlChanges(*m_db));
    ASSERT_EQ(ChangesetProps::ChangesetType::Regular, dataChangeset->GetChangesetType());

    // A: schema v1.0.1, Name string -> dateTime.
    ImportSchemaXml(SchemaXml("01.00.01", "dateTime"), "Type change");
    ChangesetPropsPtr schemaChangeset = CreateRevision("-typechange");
    ASSERT_TRUE(schemaChangeset.IsValid());

    EXPECT_TRUE(schemaChangeset->ContainsDdlChanges(*m_db)) << "A type change is reported as a schema changeset even though it executes no DDL.";
    EXPECT_TRUE(schemaChangeset->ContainsEcChanges()) << "Flagged as a schema changeset so that consumers reload their class metadata.";
    EXPECT_EQ(ChangesetProps::ChangesetType::Schema, schemaChangeset->GetChangesetType());

    const bvector<Utf8String> rowsOnA = ReadNamesAsDateTime();
    ASSERT_EQ(3u, rowsOnA.size());
    const Utf8String firstElementJsonOnA = ElementJson(first);
    const Utf8String firstViaElementApiOnA = ReadNameViaElementApi(first);

    // B: baseline image, then both changesets in order.
    RestoreTestFile();
    ApplyChangeset(*dataChangeset);
    ASSERT_TRUE(m_db->Schemas().GetClass("TestSchema", "TestElement") != nullptr);
    ApplyChangeset(*schemaChangeset);

    ECPropertyCP name = m_db->Schemas().GetClass("TestSchema", "TestElement")->GetPropertyP("Name");
    ASSERT_TRUE(name != nullptr && name->GetIsPrimitive());
    EXPECT_EQ(PRIMITIVETYPE_DateTime, name->GetAsPrimitiveProperty()->GetType()) << "B sees the new type after the schema changeset.";
    ExpectSameRows(rowsOnA, ReadNamesAsDateTime());

    // Element API and ElementProps JSON: same on both briefcases, and the legacy TEXT row is handed out as
    // a dateTime made from julian day 2020, not as the stored text.
    EXPECT_STREQ(firstElementJsonOnA.c_str(), ElementJson(first).c_str());
    EXPECT_STREQ(firstViaElementApiOnA.c_str(), ReadNameViaElementApi(first).c_str());
    EXPECT_EQ(nullptr, strstr(firstViaElementApiOnA.c_str(), "2020-03-10")) << "GetPropertyValue does not return the stored ISO text: " << firstViaElementApiOnA.c_str();
    EXPECT_EQ(nullptr, strstr(firstElementJsonOnA.c_str(), "2020-03-10")) << "ToJson does not return the stored ISO text: " << firstElementJsonOnA.c_str();

    // Element API write of the old type after the change. Predicted: refused (ECValue string on a dateTime
    // property), so the element API cannot create new legacy rows. Correct from the real output.
    EXPECT_NE(DgnDbStatus::Success, TrySetName(first, ECValue("2022-01-01T00:00:00.000Z")));
    }

//---------------------------------------------------------------------------------------
// Rows written after the type change (real dateTimes) arrive at B after the schema
// changeset, and one old row is overwritten, so the table holds text and real values. B must end
// up with the same mix as A. Also checks a changeset that touches a row written before the change.
// @bsimethod
//---------------------------------------------------------------------------------------
TEST_F(SchemaUpdateChangesetFixture, TypeChange_DataWrittenAfterTheChange_BriefcaseBReadsSameAsA)
    {
    ChangesetPropsPtr baseline = SetUpBaseline(L"SchemaUpdateChangeset_DataAfterSchema.bim");
    ASSERT_TRUE(baseline.IsValid());

    DgnElementId first = InsertTestElement(ECValue("2020-03-10T10:30:00.000Z"), 1);
    InsertTestElement(ECValue("2020-03-11T10:30:00.000Z"), 2);
    ASSERT_EQ(BE_SQLITE_OK, m_db->SaveChanges("Data under the old type"));
    ChangesetPropsPtr dataChangeset = CreateRevision("-data1");
    ASSERT_TRUE(dataChangeset.IsValid());

    ImportSchemaXml(SchemaXml("01.00.01", "dateTime"), "Type change");
    ChangesetPropsPtr schemaChangeset = CreateRevision("-typechange");
    ASSERT_TRUE(schemaChangeset.IsValid());

    // After the change: overwrite one old row with a real dateTime and add a new row.
    ECValue written;
    ASSERT_EQ(SUCCESS, written.SetDateTime(DateTime(DateTime::Kind::Unspecified, 2021, 6, 1, 12, 0, 0, 0)));
    SetName(first, written);
    InsertTestElement(written, 3);
    ASSERT_EQ(BE_SQLITE_OK, m_db->SaveChanges("Data under the new type"));
    ChangesetPropsPtr laterDataChangeset = CreateRevision("-data2");
    ASSERT_TRUE(laterDataChangeset.IsValid());

    const bvector<Utf8String> rowsOnA = ReadNamesAsDateTime();
    ASSERT_EQ(3u, rowsOnA.size());

    RestoreTestFile();
    ApplyChangeset(*dataChangeset);
    ApplyChangeset(*schemaChangeset);
    ApplyChangeset(*laterDataChangeset);
    ExpectSameRows(rowsOnA, ReadNamesAsDateTime());
    }

//---------------------------------------------------------------------------------------
// The reverse direction: briefcase A undoes the type change by reversing its own schema
// changeset (ReverseChangeset), as happens when a pushed changeset is abandoned. Rows must read as
// strings again, unchanged.
// @bsimethod
//---------------------------------------------------------------------------------------
TEST_F(SchemaUpdateChangesetFixture, TypeChange_ReverseSchemaChangeset_RestoresOldReading)
    {
    SetUpBaseline(L"SchemaUpdateChangeset_Reverse.bim");

    InsertTestElement(ECValue("2020-03-10T10:30:00.000Z"), 1);
    InsertTestElement(ECValue("2020-03-11T10:30:00.000Z"), 2);
    ASSERT_EQ(BE_SQLITE_OK, m_db->SaveChanges("Data under the old type"));
    ASSERT_TRUE(CreateRevision("-data").IsValid());
    const bvector<Utf8String> rowsBefore = ReadNamesAsText();

    ImportSchemaXml(SchemaXml("01.00.01", "dateTime"), "Type change");
    ChangesetPropsPtr schemaChangeset = CreateRevision("-typechange");
    ASSERT_TRUE(schemaChangeset.IsValid());
    ASSERT_STREQ(schemaChangeset->GetChangesetId().c_str(), m_db->Txns().GetParentChangesetId().c_str());

    m_db->Txns().ReverseChangeset(*schemaChangeset); // returns void; the parent id and the schema show whether it took effect.
    EXPECT_STREQ(schemaChangeset->GetParentId().c_str(), m_db->Txns().GetParentChangesetId().c_str()) << "The parent changeset is the one before the type change again.";

    ECPropertyCP name = m_db->Schemas().GetClass("TestSchema", "TestElement")->GetPropertyP("Name");
    ASSERT_TRUE(name != nullptr && name->GetIsPrimitive());
    EXPECT_EQ(PRIMITIVETYPE_String, name->GetAsPrimitiveProperty()->GetType());
    ExpectSameRows(rowsBefore, ReadNamesAsText());
    }

//---------------------------------------------------------------------------------------
// X08. Adding DateTimeInfo(Utc) is a custom attribute instance change only: no mapping, no data. B must
// read the values with the same (new) UTC marker as A, through ECSQL and through the element API.
// Note: this holds for Utc/Unspecified/Date. DateTimeKind=Local is also accepted but shifts every value
// by the machine's UTC offset (see PropertyTypeUpdateTests.DISABLED_ChangingDateTimeKindToLocalIsRejected).
// @bsimethod
//---------------------------------------------------------------------------------------
TEST_F(SchemaUpdateChangesetFixture, DateTimeInfoChange_BriefcaseBReadsSameAsA)
    {
    ASSERT_TRUE(SetUpBaseline(L"SchemaUpdateChangeset_DateTimeInfo.bim", DateTimeSchemaXml("01.00.00", false)).IsValid());
    ECValue unspecified;
    ASSERT_EQ(SUCCESS, unspecified.SetDateTime(DateTime(DateTime::Kind::Unspecified, 2021, 6, 1, 12, 0, 0, 0)));
    DgnElementId first = InsertTestElement(unspecified, 1);
    ASSERT_EQ(BE_SQLITE_OK, m_db->SaveChanges("Data"));
    ChangesetPropsPtr dataChangeset = CreateRevision("-data");
    ASSERT_TRUE(dataChangeset.IsValid());

    ImportSchemaXml(DateTimeSchemaXml("01.00.01", true), "Add DateTimeInfo Utc");
    ChangesetPropsPtr schemaChangeset = CreateRevision("-datetimeinfo");
    ASSERT_TRUE(schemaChangeset.IsValid());

    const bvector<Utf8String> rowsOnA = ReadNamesAsDateTime();
    const Utf8String viaElementOnA = ReadNameViaElementApi(first);

    RestoreTestFile();
    ApplyChangeset(*dataChangeset);
    ApplyChangeset(*schemaChangeset);
    ExpectSameRows(rowsOnA, ReadNamesAsDateTime());
    EXPECT_STREQ(viaElementOnA.c_str(), ReadNameViaElementApi(first).c_str());
    }

//---------------------------------------------------------------------------------------
// ChangesetReader scope item. A data changeset recorded while Name was a string is read later, when
// the schema says dateTime. ChangeIterator returns the raw stored SQLite value (TEXT) either way, so a
// change-summary consumer sees the old ISO text for history that predates the type change and must
// interpret it itself.
// @bsimethod
//---------------------------------------------------------------------------------------
TEST_F(SchemaUpdateChangesetFixture, HistoricalChangesetReadUnderNewType_DecodedByCurrentSchema)
    {
    ASSERT_TRUE(SetUpBaseline(L"SchemaUpdateChangeset_History.bim").IsValid());
    DgnElementId first = InsertTestElement(ECValue("2020-03-10T10:30:00.000Z"), 1);
    ASSERT_EQ(BE_SQLITE_OK, m_db->SaveChanges("Data"));
    ChangesetPropsPtr dataChangeset = CreateRevision("-data");
    ASSERT_TRUE(dataChangeset.IsValid());

    const bvector<Utf8String> underOldType = NameValuesInChangeset(*dataChangeset);
    ASSERT_EQ(1u, underOldType.size());
    const Utf8String summaryUnderOldType = NameInChangeSummary(*dataChangeset, first);

    ImportSchemaXml(SchemaXml("01.00.01", "dateTime"), "Type change");
    ASSERT_TRUE(CreateRevision("-typechange").IsValid());

    const bvector<Utf8String> underNewType = NameValuesInChangeset(*dataChangeset);
    ASSERT_EQ(1u, underNewType.size());
    EXPECT_STREQ(underOldType[0].c_str(), underNewType[0].c_str()) << "The reader does not decode by the current schema.";
    EXPECT_NE(nullptr, strstr(underNewType[0].c_str(), "sqlite type 3")) << "Still TEXT after the type change.";

    // The public ChangeSummary API (used by ChangeSummaryManager) extracts the same raw TEXT. A consumer that
    // asks for the value as the schema's type gets the julian-day misreading, as with ECSQL.
    const Utf8String summaryUnderNewType = NameInChangeSummary(*dataChangeset, first);
    EXPECT_STREQ(summaryUnderOldType.c_str(), summaryUnderNewType.c_str());
    EXPECT_NE(nullptr, strstr(summaryUnderNewType.c_str(), "2020-03-10T10:30:00.000Z")) << summaryUnderNewType.c_str();
    }

//---------------------------------------------------------------------------------------
// Old-schema writer. The type change is a read-version bump of a dynamic schema, so a client that still
// has v1.0.0 loaded keeps opening the iModel and writing *strings*. Hub ordering: B pushes its text row
// first; A, which holds the type change as a local unpushed txn, must pull B's changeset (rebasing the
// schema txn) before it can push. Nothing rejects any of it: A ends up with fresh TEXT in a column it
// reads as julian days. This is the multi-briefcase form of "valid update, wrong reads".
// @bsimethod
//---------------------------------------------------------------------------------------
TEST_F(SchemaUpdateChangesetFixture, TypeChange_OldSchemaWriterPushesTextFirst_ARebasesAndReadsItAsJulianDay)
    {
    ASSERT_TRUE(SetUpBaseline(L"SchemaUpdateChangeset_OldWriter.bim").IsValid());

    // A: type change as a local, unpushed txn.
    ImportSchemaXml(SchemaXml("01.00.01", "dateTime"), "Type change");
    ASSERT_TRUE(m_db->Txns().HasPendingTxns());
    SaveImageAs(L"SchemaUpdateChangeset_OldWriter_A.bim");

    // B (still on v1.0.0): writes text and pushes first.
    RestoreTestFile();
    ECPropertyCP nameOnB = m_db->Schemas().GetClass("TestSchema", "TestElement")->GetPropertyP("Name");
    ASSERT_EQ(PRIMITIVETYPE_String, nameOnB->GetAsPrimitiveProperty()->GetType());
    DgnElementId written = InsertTestElement(ECValue("2020-03-10T10:30:00.000Z"), 1);
    ASSERT_EQ(BE_SQLITE_OK, m_db->SaveChanges("Text written by the old-schema client"));
    ChangesetPropsPtr bDataChangeset = CreateRevision("-bdata");
    ASSERT_TRUE(bDataChangeset.IsValid());

    // A pulls B's row (rebases its schema txn), then pushes the type change.
    LoadImage(L"SchemaUpdateChangeset_OldWriter_A.bim");
    ApplyChangeset(*bDataChangeset);
    ECPropertyCP nameOnA = m_db->Schemas().GetClass("TestSchema", "TestElement")->GetPropertyP("Name");
    ASSERT_EQ(PRIMITIVETYPE_DateTime, nameOnA->GetAsPrimitiveProperty()->GetType()) << "The rebased schema txn is still in effect.";
    ChangesetPropsPtr schemaChangeset = CreateRevision("-typechange");
    ASSERT_TRUE(schemaChangeset.IsValid());
    EXPECT_EQ(ChangesetProps::ChangesetType::Schema, schemaChangeset->GetChangesetType());

    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(*m_db, "SELECT Name FROM ts.TestElement WHERE ECInstanceId = ?"));
    stmt.BindId(1, written);
    ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());
    EXPECT_STREQ("2020-03-10T10:30:00.000Z", stmt.GetValueText(0)) << "The text arrived unchanged.";
    EXPECT_NE(2020, stmt.GetValueDateTime(0).GetYear()) << "A reads it as julian day 2020, like the pre-change rows.";
    EXPECT_EQ(nullptr, strstr(ReadNameViaElementApi(written).c_str(), "2020-03-10")) << "The element API does not hand out the text either.";
    }

//---------------------------------------------------------------------------------------
// Pull-merge with local changes. B has an unpushed txn that wrote text under the old type when the schema
// changeset arrives. PullMergeApply reverses the local txn, merges, and rebases it, so the local text is
// re-inserted into a column that is now a dateTime: it stays TEXT and reads as a julian day.
// A inserts no row: A and B are copies of one file with the same briefcase id, so a row inserted on both
// would collide on the element id (a test artifact; on a hub every briefcase has its own id range).
// @bsimethod
//---------------------------------------------------------------------------------------
TEST_F(SchemaUpdateChangesetFixture, TypeChange_PullMergeWithLocalTextTxn_LocalRowRebasedAsText)
    {
    ASSERT_TRUE(SetUpBaseline(L"SchemaUpdateChangeset_LocalTxn.bim").IsValid());

    // A: the type change only.
    ImportSchemaXml(SchemaXml("01.00.01", "dateTime"), "Type change");
    ChangesetPropsPtr schemaChangeset = CreateRevision("-typechange");
    ASSERT_TRUE(schemaChangeset.IsValid());

    // B: local, unpushed text row; then pull the schema changeset.
    RestoreTestFile();
    DgnElementId local = InsertTestElement(ECValue("2020-03-11T10:30:00.000Z"), 2);
    ASSERT_EQ(BE_SQLITE_OK, m_db->SaveChanges("Local text row"));
    ASSERT_TRUE(m_db->Txns().HasPendingTxns());

    ApplyChangeset(*schemaChangeset);

    EXPECT_TRUE(m_db->Txns().HasPendingTxns()) << "The local txn survives the merge (rebased).";
    ECPropertyCP name = m_db->Schemas().GetClass("TestSchema", "TestElement")->GetPropertyP("Name");
    EXPECT_EQ(PRIMITIVETYPE_DateTime, name->GetAsPrimitiveProperty()->GetType());

    ECSqlStatement stmt;
    ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(*m_db, "SELECT Name FROM ts.TestElement WHERE ECInstanceId = ?"));
    stmt.BindId(1, local);
    ASSERT_EQ(BE_SQLITE_ROW, stmt.Step());
    EXPECT_STREQ("2020-03-11T10:30:00.000Z", stmt.GetValueText(0)) << "The rebased local row is still the text it was written as.";
    EXPECT_NE(2020, stmt.GetValueDateTime(0).GetYear());
    }

//---------------------------------------------------------------------------------------
// Data-transform (T) schema changeset. Count moves from TestElement (int) to TestBase (double). ECDb moves
// the stored values to the new column (RemapManager), so this schema changeset carries row changes as well
// as ec_ metadata. B applies it and must read the same values; the changeset must be flagged as Schema.
// Predicted: accepted (DgnDb::ImportSchemas with schemaLockHeld grants AllowDataTransformDuringSchemaUpgrade,
// and the deletion from TestElement passes the dynamic-schema gate). Correct from the real output.
// @bsimethod
//---------------------------------------------------------------------------------------
TEST_F(SchemaUpdateChangesetFixture, DataTransform_MoveCountToBaseAsDouble_BriefcaseBReadsSameAsA)
    {
    ASSERT_TRUE(SetUpBaseline(L"SchemaUpdateChangeset_Transform.bim", HierarchySchemaXml("01.00.00", false)).IsValid());

    InsertTestElement(ECValue("one"), 1);
    InsertTestElement(ECValue("two"), 2);
    ASSERT_EQ(BE_SQLITE_OK, m_db->SaveChanges("Data"));
    ChangesetPropsPtr dataChangeset = CreateRevision("-data");
    ASSERT_TRUE(dataChangeset.IsValid());

    ImportSchemaXml(HierarchySchemaXml("01.00.01", true), "Move Count to TestBase as double");
    ChangesetPropsPtr schemaChangeset = CreateRevision("-transform");
    ASSERT_TRUE(schemaChangeset.IsValid());
    EXPECT_TRUE(schemaChangeset->ContainsEcChanges());
    EXPECT_EQ(ChangesetProps::ChangesetType::Schema, schemaChangeset->GetChangesetType());

    auto readCounts = [&] ()
        {
        bvector<Utf8String> rows;
        ECSqlStatement stmt;
        EXPECT_EQ(ECSqlStatus::Success, stmt.Prepare(*m_db, "SELECT Name, [Count] FROM ts.TestBase ORDER BY ECInstanceId"));
        EXPECT_EQ(PRIMITIVETYPE_Double, stmt.GetColumnInfo(1).GetDataType().GetPrimitiveType());
        while (BE_SQLITE_ROW == stmt.Step())
            rows.push_back(Utf8PrintfString("%s | %.1f", stmt.GetValueText(0), stmt.GetValueDouble(1)));
        return rows;
        };
    const bvector<Utf8String> rowsOnA = readCounts();
    ASSERT_EQ(2u, rowsOnA.size());
    EXPECT_STREQ("one | 1.0", rowsOnA[0].c_str()) << "The moved value is read back as a double through the base class.";

    RestoreTestFile();
    ApplyChangeset(*dataChangeset);
    ApplyChangeset(*schemaChangeset);
    ExpectSameRows(rowsOnA, readCounts());
    }
