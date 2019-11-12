/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/

#include "SeedFile.h"

BeAtomic<uint64_t> SeedFile::s_id(0);

SeedFile::SeedFile(Utf8String name, Callback onSetupSeedFile, Callback onSetupTestFile) :
m_name(name),
onSetupSeedFile(onSetupSeedFile),
onSetupTestFile(onSetupTestFile)
    {
    m_id = ++s_id;
    }

BeFileName SeedFile::GetTestFile()
    {
    // Setup seed file
    if (m_seedFilePath.empty())
        {
        m_seedFilePath = GetTestsOutputDir();
        m_seedFilePath.AppendToPath(L"SeedFiles");
        m_seedFilePath.AppendToPath(BeFileName(Utf8PrintfString("%llu-%s", m_id, m_name.c_str())).c_str());

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
