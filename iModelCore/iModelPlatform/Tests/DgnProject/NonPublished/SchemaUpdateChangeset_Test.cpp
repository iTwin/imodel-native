/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "ChangeTestFixture.h"
#include <DgnPlatform/GenericDomain.h>
#include <ECDb/ChangeIterator.h>
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

    void ApplyChangeset(ChangesetPropsCR revision)
        {
        EXPECT_EQ(ChangesetStatus::Success, m_db->Txns().PullMergeApply(revision)) << revision.GetChangesetId().c_str();
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
    InsertTestElement(ECValue("2020-03-10T10:30:00.000Z"), 1);
    InsertTestElement(ECValue("2020-03-11T10:30:00.000Z"), 2);
    InsertTestElement(ECValue("2020-03-12T10:30:00.000Z"), 3);
    ASSERT_EQ(BE_SQLITE_OK, m_db->SaveChanges("Data under the old type"));
    ChangesetPropsPtr dataChangeset = CreateRevision("-data");
    ASSERT_TRUE(dataChangeset.IsValid());
    ASSERT_FALSE(dataChangeset->ContainsDdlChanges(*m_db));

    // A: schema v1.0.1, Name string -> dateTime.
    ImportSchemaXml(SchemaXml("01.00.01", "dateTime"), "Type change");
    ChangesetPropsPtr schemaChangeset = CreateRevision("-typechange");
    ASSERT_TRUE(schemaChangeset.IsValid());

    const bvector<Utf8String> rowsOnA = ReadNamesAsDateTime();
    ASSERT_EQ(3u, rowsOnA.size());

    // B: baseline image, then both changesets in order.
    RestoreTestFile();
    ApplyChangeset(*dataChangeset);
    ASSERT_TRUE(m_db->Schemas().GetClass("TestSchema", "TestElement") != nullptr);
    ApplyChangeset(*schemaChangeset);

    ECPropertyCP name = m_db->Schemas().GetClass("TestSchema", "TestElement")->GetPropertyP("Name");
    ASSERT_TRUE(name != nullptr && name->GetIsPrimitive());
    EXPECT_EQ(PRIMITIVETYPE_DateTime, name->GetAsPrimitiveProperty()->GetType()) << "B sees the new type after the schema changeset.";
    ExpectSameRows(rowsOnA, ReadNamesAsDateTime());
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

    m_db->Txns().ReverseChangeset(*schemaChangeset);

    ECPropertyCP name = m_db->Schemas().GetClass("TestSchema", "TestElement")->GetPropertyP("Name");
    ASSERT_TRUE(name != nullptr && name->GetIsPrimitive());
    EXPECT_EQ(PRIMITIVETYPE_String, name->GetAsPrimitiveProperty()->GetType());
    ExpectSameRows(rowsBefore, ReadNamesAsText());
    }

//---------------------------------------------------------------------------------------
// X08. A DateTimeInfo change is a custom attribute instance change only: no mapping, no data. B must
// read the values with the same (new) UTC marker as A, through ECSQL and through the element API.
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
    InsertTestElement(ECValue("2020-03-10T10:30:00.000Z"), 1);
    ASSERT_EQ(BE_SQLITE_OK, m_db->SaveChanges("Data"));
    ChangesetPropsPtr dataChangeset = CreateRevision("-data");
    ASSERT_TRUE(dataChangeset.IsValid());

    const bvector<Utf8String> underOldType = NameValuesInChangeset(*dataChangeset);
    ASSERT_EQ(1u, underOldType.size());

    ImportSchemaXml(SchemaXml("01.00.01", "dateTime"), "Type change");
    ASSERT_TRUE(CreateRevision("-typechange").IsValid());

    const bvector<Utf8String> underNewType = NameValuesInChangeset(*dataChangeset);
    ASSERT_EQ(1u, underNewType.size());
    EXPECT_STREQ(underOldType[0].c_str(), underNewType[0].c_str()) << "The reader does not decode by the current schema.";
    EXPECT_NE(nullptr, strstr(underNewType[0].c_str(), "sqlite type 3")) << "Still TEXT after the type change.";
    }
