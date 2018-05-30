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
TEST_F(DgnDbCompatibilityTestFixture, PreEC32Enums)
    {
    DgnDbPtr dgndb = CreateNewTestFile("enums.bim");
    ASSERT_TRUE(dgndb != nullptr);

    ECEnumerationCP ecEnum = dgndb->Schemas().GetEnumeration("BisCore", "CustomHandledPropertyStatementType");
    ASSERT_TRUE(ecEnum != nullptr) << "BisCore.CustomHandledPropertyStatementType";
    EXPECT_EQ(PRIMITIVETYPE_Integer, ecEnum->GetType()) << ecEnum->GetFullName();
    EXPECT_TRUE(ecEnum->GetIsStrict()) << ecEnum->GetFullName();
    EXPECT_FALSE(ecEnum->GetIsDisplayLabelDefined()) << ecEnum->GetFullName();
    EXPECT_TRUE(ecEnum->GetDescription().empty()) << ecEnum->GetFullName();
    EXPECT_EQ(7, ecEnum->GetEnumeratorCount()) << ecEnum->GetFullName();

    std::vector<std::pair<int, Utf8CP>> expectedEnumValues {{0, "None"},
                                                            {1, "Select"},
                                                            {2, "Insert"},
                                                            {3, "ReadOnly = Select|Insert"},
                                                            {4, "Update"},
                                                            {6, "InsertUpdate = Insert | Update"},
                                                            {7, "All = Select | Insert | Update"} };

    size_t i = 0;
    for (ECEnumeratorCP enumerator : ecEnum->GetEnumerators())
        {
        std::pair<int, Utf8CP> const& expectedEnumValue = expectedEnumValues[i];
        EXPECT_STRCASEEQ(expectedEnumValue.second, enumerator->GetDisplayLabel().c_str());
        EXPECT_EQ(expectedEnumValue.first, enumerator->GetInteger()) << "Value: " << expectedEnumValue.first;
        i++;
        }
    }

