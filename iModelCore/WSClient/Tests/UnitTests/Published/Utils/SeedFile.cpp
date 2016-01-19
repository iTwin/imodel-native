/*--------------------------------------------------------------------------------------+
|
|     $Source: Tests/UnitTests/Published/Utils/SeedFile.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#include "SeedFile.h"

BeFileName SeedFile::GetTestFile()
    {
    // Setup seed file
    if (m_seedFilePath.empty())
        {
        m_seedFilePath = GetTestsOutputDir().AppendToPath(L"SeedFiles").AppendToPath(BeFileName(m_name));
        if (m_seedFilePath.DoesPathExist())
            {
            EXPECT_EQ(BeFileNameStatus::Success, BeFileName::EmptyAndRemoveDirectory(m_seedFilePath));
            }
        EXPECT_EQ(BeFileNameStatus::Success, BeFileName::CreateNewDirectory(m_seedFilePath));

        m_seedFilePath.AppendToPath(BeFileName(m_name + "-seed"));
        SetupSeedFile(m_seedFilePath);

        EXPECT_TRUE(BeFileName::DoesPathExist(m_seedFilePath));
        }

    // Clear previous test files
    m_testFilePath = GetTestsTempDir().AppendToPath(BeFileName(m_name));
    if (m_testFilePath.DoesPathExist())
        {
        EXPECT_EQ(BeFileNameStatus::Success, BeFileName::EmptyAndRemoveDirectory(m_testFilePath));
        }
    EXPECT_EQ(BeFileNameStatus::Success, BeFileName::CreateNewDirectory(m_testFilePath));
    m_testFilePath.AppendToPath(BeFileName(m_name));

    // Setup test file
    EXPECT_EQ(BeFileNameStatus::Success, BeFileName::BeCopyFile(m_seedFilePath, m_testFilePath, false));
    SetupTestFile(m_testFilePath);

    return m_testFilePath;
    }
