/*--------------------------------------------------------------------------------------+
|
|  $Source: Tests/DgnProject/Compatibility/DgnDbCompatibilityTests.cpp $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "CompatibilityTestFixture.h"

USING_NAMESPACE_BENTLEY_EC

struct DgnDbCompatibilityTestFixture : CompatibilityTestFixture
    {
    ScopedDgnHost m_host;

    protected:
       Profile& Profile() const { return ProfileManager().GetProfile(ProfileType::DgnDb); }

       DgnDbPtr OpenTestFile(DbResult* stat, BeFileNameCR path) { return DgnDb::OpenDgnDb(stat, path, DgnDb::OpenParams(DgnDb::OpenMode::Readonly)); }
       DgnDbPtr CreateNewTestFile(Utf8CP fileName)
           {
           BeFileName filePath = Profile().GetPathForNewTestFile(fileName);
           BeFileName folder = filePath.GetDirectoryName();
           if (!folder.DoesPathExist())
               {
               if (BeFileNameStatus::Success != BeFileName::CreateNewDirectory(folder))
                   return nullptr;
               }

           CreateDgnDbParams createParam(fileName);
           return DgnDb::CreateDgnDb(nullptr, filePath, createParam);
           }
    };

//---------------------------------------------------------------------------------------
// @bsimethod                                  Krischan.Eberle                      05/18
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(DgnDbCompatibilityTestFixture, CreatePreEC32EnumsTestFile)
    {
    ASSERT_TRUE(CreateNewTestFile("preec32enums.bim") != nullptr);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                  Krischan.Eberle                      05/18
//+---------------+---------------+---------------+---------------+---------------+------
TEST_F(DgnDbCompatibilityTestFixture, PreEC32Enums)
    {
    for (TestFile const& testFile : Profile().GetAllVersionsOfTestFile("preec32enums"))
        {
        DbResult stat = BE_SQLITE_OK;
        DgnDbPtr dgndb = OpenTestFile(&stat, testFile.GetPath());
        ASSERT_EQ(BE_SQLITE_OK, stat) << testFile.GetPath().GetNameUtf8();
        ASSERT_TRUE(dgndb != nullptr) << testFile.GetPath().GetNameUtf8();

        Utf8String assertMessage("BisCore.CustomHandledPropertyStatementType");
        assertMessage.append(" | ").append(testFile.GetPath().GetNameUtf8());

        ECEnumerationCP ecEnum = dgndb->Schemas().GetEnumeration("BisCore", "CustomHandledPropertyStatementType");
        ASSERT_TRUE(ecEnum != nullptr) << "BisCore.CustomHandledPropertyStatementType";

        ECEnumerationId enumId = ecEnum->GetId();
        ECSchemaId schemaId = ecEnum->GetSchema().GetId();

        EXPECT_EQ(PRIMITIVETYPE_Integer, ecEnum->GetType()) << assertMessage;
        EXPECT_TRUE(ecEnum->GetIsStrict()) << assertMessage;
        EXPECT_FALSE(ecEnum->GetIsDisplayLabelDefined()) << assertMessage;
        EXPECT_TRUE(ecEnum->GetDescription().empty()) << assertMessage;
        EXPECT_EQ(7, ecEnum->GetEnumeratorCount()) << assertMessage;

        std::vector<std::pair<int, Utf8CP>> expectedEnumValues {{0, "None"},
        {1, "Select"},
        {2, "Insert"},
        {3, "ReadOnly = Select|Insert"},
        {4, "Update"},
        {6, "InsertUpdate = Insert | Update"},
        {7, "All = Select | Insert | Update"}};

        size_t i = 0;
        for (ECEnumeratorCP enumerator : ecEnum->GetEnumerators())
            {
            std::pair<int, Utf8CP> const& expectedEnumValue = expectedEnumValues[i];
            EXPECT_STRCASEEQ(expectedEnumValue.second, enumerator->GetDisplayLabel().c_str());
            EXPECT_EQ(expectedEnumValue.first, enumerator->GetInteger()) << "Value: " << expectedEnumValue.first << " " << assertMessage;;
            i++;
            }

        ECSqlStatement stmt;
        ASSERT_EQ(ECSqlStatus::Success, stmt.Prepare(*dgndb, "SELECT ECInstanceId,Name,DisplayLabel,Description,IsStrict,Type,EnumValues FROM meta.ECEnumerationDef WHERE Name='CustomHandledPropertyStatementType' AND Schema.Id=?")) << assertMessage;
        ASSERT_EQ(ECSqlStatus::Success, stmt.BindId(1, schemaId)) << stmt.GetECSql() << " | " << assertMessage;
        ASSERT_EQ(BE_SQLITE_ROW, stmt.Step()) << stmt.GetECSql() << " | " << assertMessage;
        EXPECT_EQ(enumId, stmt.GetValueId<ECEnumerationId>(0)) << stmt.GetECSql() << " | " << assertMessage;
        EXPECT_TRUE(stmt.IsValueNull(1)) << stmt.GetECSql() << " | " << assertMessage;
        EXPECT_TRUE(stmt.IsValueNull(2)) << stmt.GetECSql() << " | " << assertMessage;
        EXPECT_TRUE(stmt.GetValueBoolean(3)) << stmt.GetECSql() << " | " << assertMessage;
        EXPECT_EQ((int) PRIMITIVETYPE_Integer, stmt.GetValueInt(4)) << stmt.GetECSql() << " | " << assertMessage;
        }
    }

