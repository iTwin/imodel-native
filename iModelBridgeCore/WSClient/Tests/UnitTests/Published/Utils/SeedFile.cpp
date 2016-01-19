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
    if (m_seedFilePath.empty())
        {
        m_seedFilePath = GetTestsTempDir().AppendToPath(BeFileName(m_name + "-seed"));

        if (m_seedFilePath.DoesPathExist())
            {
            EXPECT_EQ(BeFileNameStatus::Success, BeFileName::EmptyAndRemoveDirectory(m_seedFilePath));
            }
        EXPECT_EQ(BeFileNameStatus::Success, BeFileName::CreateNewDirectory(m_seedFilePath));

        m_seedFilePath.AppendToPath(BeFileName(m_name + "-seed"));
        SetupSeedFile(m_seedFilePath);

        EXPECT_TRUE(BeFileName::DoesPathExist(m_seedFilePath));
        }

    if (m_testFilePath.empty())
        {
        m_testFilePath = GetTestsTempDir().AppendToPath(BeFileName(m_name + "-test"));

        if (m_testFilePath.DoesPathExist())
            {
            EXPECT_EQ(BeFileNameStatus::Success, BeFileName::EmptyAndRemoveDirectory(m_testFilePath));
            }
        EXPECT_EQ(BeFileNameStatus::Success, BeFileName::CreateNewDirectory(m_testFilePath));

        m_testFilePath.AppendToPath(BeFileName(m_name));
        }

    EXPECT_EQ(BeFileNameStatus::Success, BeFileName::BeCopyFile(m_seedFilePath, m_testFilePath, false));

    SetupTestFile(m_testFilePath);

    return m_testFilePath;
    }
