/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once

#include "CompatibilityTests.h"
#include "Profiles.h"
#include <DgnPlatform/DgnPlatformApi.h>
#include <Bentley/BeDirectoryIterator.h>
#include <Bentley/BeTest.h>

USING_NAMESPACE_BENTLEY_SQLITE
USING_NAMESPACE_BENTLEY_SQLITE_EC
USING_NAMESPACE_BENTLEY_DGN

//================================Profile================================

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
Profile::Profile(ProfileType type, Utf8CP nameSpace, Utf8CP name) : m_type(type), m_versionPropertySpec("SchemaVersion", nameSpace), m_name(name)
    {
    BeTest::GetHost().GetOutputRoot(m_profileOutFolder);
    m_profileOutFolder.AppendToPath(WString(m_name, BentleyCharEncoding::Utf8).c_str());

    BeFileName baseFolder;
    BeTest::GetHost().GetOutputRoot(baseFolder);
    baseFolder.PopDir();

    m_profilePulledTestDataFolder = baseFolder;
    m_profilePulledTestDataFolder.AppendSeparator().AppendToPath(L"TestFiles").AppendToPath(WString(m_name, BentleyCharEncoding::Utf8).c_str());

    m_profileCreatedDataFolder = baseFolder;
    m_profileCreatedDataFolder.AppendSeparator().AppendToPath(L"NewFiles").AppendToPath(WString(m_name, BentleyCharEncoding::Utf8).c_str());
    }


//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus Profile::Init() const
    {
    if (!m_profileOutFolder.DoesPathExist())
        {
        BeFileNameStatus status = BeFileName::CreateNewDirectory(m_profileOutFolder.GetName());
        if (status != BeFileNameStatus::Success)
            {
            BeAssert(status == BeFileNameStatus::Success);
            return ERROR;
            }
        }

    if (!m_profileCreatedDataFolder.DoesPathExist())
        {
        BeFileNameStatus status = BeFileName::CreateNewDirectory(m_profileCreatedDataFolder.GetName());
        if (status != BeFileNameStatus::Success)
            {
            BeAssert(status == BeFileNameStatus::Success);
            return ERROR;
            }
        }

    return _Init();
    }

// -------------------------------------------------------------------------------------- -
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
std::vector<TestFile> Profile::GetAllVersionsOfTestFile(Utf8CP testFileName, bool logFoundFiles) const
    {
    std::vector<TestFile> testFiles = GetAllVersionsOfTestFile(m_profileCreatedDataFolder, testFileName, logFoundFiles);
    std::vector<TestFile> pulledDataTestFiles = GetAllVersionsOfTestFile(m_profilePulledTestDataFolder, testFileName, logFoundFiles);
    testFiles.insert(testFiles.end(), pulledDataTestFiles.begin(), pulledDataTestFiles.end());
    return testFiles;
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
std::vector<TestFile> Profile::GetAllVersionsOfTestFile(BeFileNameCR rootFolder, Utf8CP testFileNameUtf8, bool logFoundFiles) const
    {
    BeFileName testFileName(testFileNameUtf8);
    WString extension = testFileName.GetExtension();
    WString fileNameWithoutExt = testFileName.GetFileNameWithoutExtension();
    WString fileNamePattern(fileNameWithoutExt);
    fileNamePattern.append(L".*").append(extension);
    return GetAllVersionsOfTestFile(rootFolder, fileNamePattern, logFoundFiles);
    }


//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
std::vector<TestFile> Profile::GetAllVersionsOfTestFile(BeFileNameCR rootFolder, WStringCR fileNamePattern, bool logFoundFiles) const
    {
    if (!rootFolder.DoesPathExist())
        return std::vector<TestFile>();

    bvector<BeFileName> fileMatches;
    BeDirectoryIterator::WalkDirsAndMatch(fileMatches, rootFolder, fileNamePattern.c_str(), true);
    std::vector<TestFile> testFiles;
    for (BeFileNameCR filePath : fileMatches)
        {
        if (filePath.IsDirectory())
            continue;

        BeFileName profileVersionFolderName = filePath.GetDirectoryName();
        //just get folder name without path
        if (profileVersionFolderName.EndsWith(L"/") || profileVersionFolderName.EndsWith(L"\\"))
            profileVersionFolderName.erase(profileVersionFolderName.size() - 1, 1);

        const size_t separatorPos = profileVersionFolderName.find_last_of(L"/\\");
        if (separatorPos != BeFileName::npos)
            profileVersionFolderName.erase(0, separatorPos + 1);

        bvector<Utf8String> profileVersions;
        BeStringUtilities::Split(profileVersionFolderName.GetNameUtf8().c_str(), PROFILEVERSIONFOLDER_VERSIONSEPARATOR, profileVersions);
        BeAssert(!profileVersions.empty());
        ProfileVersion bedbVersion(0, 0, 0, 0), ecdbVersion(0, 0, 0, 0), dgndbVersion(0, 0, 0, 0);
        bedbVersion.FromString(profileVersions.back().c_str());
        if (profileVersions.size() == 3)
            {
            dgndbVersion.FromString(profileVersions[0].c_str());
            ecdbVersion.FromString(profileVersions[1].c_str());
            }
        else if (profileVersions.size() == 2)
            ecdbVersion.FromString(profileVersions[0].c_str());

        // parse file name to find its original profile versions (in case it is an upgraded test file)
        ProfileVersion initialBeDbVersion(bedbVersion), initialECDbVersion(ecdbVersion), initialDgnDbVersion(dgndbVersion);
        bvector<WString> fileNameTokens;
        BeStringUtilities::Split(filePath.GetFileNameWithoutExtension().c_str(), L".", fileNameTokens);
        if (fileNameTokens.size() > 1)
            {
            constexpr Utf8CP versionStringTemplate = "%d-%d-%d-%d";
            bvector<Utf8String> initialProfileVersions;
            BeStringUtilities::Split(Utf8String(fileNameTokens.back()).c_str(), PROFILEVERSIONFOLDER_VERSIONSEPARATOR, initialProfileVersions);
            BeAssert(!initialProfileVersions.empty());
            initialBeDbVersion.FromString(initialProfileVersions.back().c_str(), versionStringTemplate);
            if (initialProfileVersions.size() == 3)
                {
                initialDgnDbVersion.FromString(initialProfileVersions[0].c_str(), versionStringTemplate);
                initialECDbVersion.FromString(initialProfileVersions[1].c_str(), versionStringTemplate);
                }
            else if (initialProfileVersions.size() == 2)
                initialECDbVersion.FromString(initialProfileVersions[0].c_str(), versionStringTemplate);
            }

        BeFileName testFilePath(GetOutFolder());
        testFilePath.AppendToPath(profileVersionFolderName).AppendToPath(filePath.GetFileNameAndExtension().c_str());
        testFiles.push_back(TestFile(Utf8String(filePath.GetFileNameAndExtension()), testFilePath, filePath, bedbVersion, ecdbVersion, dgndbVersion, initialBeDbVersion, initialECDbVersion, initialDgnDbVersion));
        }

    if (logFoundFiles && LOG.isSeverityEnabled(NativeLogging::LOG_INFO))
        {
        if (testFiles.empty())
            LOG.infov("Found test files in %s: none.", rootFolder.GetNameUtf8().c_str());
        else
            {
            Utf8PrintfString msg("Found test files in %s:\r\n", rootFolder.GetNameUtf8().c_str());
            for (TestFile const& testFile : testFiles)
                {
                msg.append(testFile.ToString()).append("\r\n");
                }
            LOG.info(msg.c_str());
            }
        }

    return testFiles;
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
BeFileName Profile::GetPathForNewUpgradedTestFile(TestFile const& oldSeedFile) const
    {
    if (oldSeedFile.GetAge() != ProfileState::Age::Older)
        {
        BeAssert(false && "Should have been caught before");
        return BeFileName();
        }

    BeFileName filePath = GetFolderForNewTestFile();

    // if seed file is already an upgraded one, the file name contains the initial profile versions already.
    // So it can just be used as is.
    if (oldSeedFile.IsUpgraded())
        return filePath.AppendToPath(BeFileName(oldSeedFile.GetPath().GetFileNameAndExtension()));

    constexpr Utf8CP versionStringTemplate = "%" PRIu16 "-%" PRIu16 "-%" PRIu16 "-%" PRIu16;
    BeFileName fileName(oldSeedFile.GetPath().GetFileNameWithoutExtension());
    fileName.append(L".");
    if (!oldSeedFile.GetInitialDgnDbVersion().IsEmpty())
        fileName.AppendUtf8(oldSeedFile.GetInitialDgnDbVersion().ToString(versionStringTemplate).c_str()).append(PROFILEVERSIONFOLDER_VERSIONSEPARATOR_W);

    if (!oldSeedFile.GetInitialECDbVersion().IsEmpty())
        fileName.AppendUtf8(oldSeedFile.GetInitialECDbVersion().ToString(versionStringTemplate).c_str()).append(PROFILEVERSIONFOLDER_VERSIONSEPARATOR_W);

    fileName.AppendUtf8(oldSeedFile.GetInitialBeDbVersion().ToString(versionStringTemplate).c_str());
    fileName.AppendExtension(oldSeedFile.GetPath().GetExtension().c_str());
    return filePath.AppendToPath(fileName);
    }

bool Profile::IsFileCreatedForCurrentTestRun(const TestFile& testFile) const
    {
    // Get just the test file directory name
    auto testFileDirectory = testFile.GetSeedPath().GetDirectoryName();
    if (testFileDirectory.EndsWith(L"/") || testFileDirectory.EndsWith(L"\\"))
        testFileDirectory.erase(testFileDirectory.size() - 1, 1);

    const auto separatorPos = testFileDirectory.find_last_of(L"/\\");
    if (separatorPos != BeFileName::npos)
        testFileDirectory.erase(0U, separatorPos + 1U);

    // Append the directory and test file name to the created data folder
    auto createdDataFolder = GetCreatedDataFolder();
    createdDataFolder.AppendToPath(testFileDirectory.c_str());
    createdDataFolder.AppendToPath(BeFileName(testFile.GetName()).c_str());

    // Check if the updated created data folder path matches the test file seed path 
    return createdDataFolder.GetNameUtf8().Equals(testFile.GetSeedPath().GetNameUtf8());
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
BeFileName Profile::GetFolderForNewTestFile() const
    {
    BeFileName path(m_profileCreatedDataFolder);
    path.AppendToPath(BeFileName(GetExpectedVersion().ToString()));

    switch (m_type)
        {
        case ProfileType::DgnDb:
        {
        Utf8String ecdbVersion = ECDbProfile::Get().GetExpectedVersion().ToString();
        Utf8String bedbVersion = BeDbProfile::Get().GetExpectedVersion().ToString();
        path.AppendString(PROFILEVERSIONFOLDER_VERSIONSEPARATOR_W).AppendString(WString(ecdbVersion.c_str(), BentleyCharEncoding::Utf8).c_str()).AppendString(PROFILEVERSIONFOLDER_VERSIONSEPARATOR_W).AppendString(WString(bedbVersion.c_str(), BentleyCharEncoding::Utf8).c_str());
        return path;
        }
        case ProfileType::ECDb:
        {
        Utf8String bedbVersion = BeDbProfile::Get().GetExpectedVersion().ToString();
        path.AppendString(PROFILEVERSIONFOLDER_VERSIONSEPARATOR_W).AppendString(WString(bedbVersion.c_str(), BentleyCharEncoding::Utf8).c_str());
        return path;
        }
        case ProfileType::BeDb:
            return path;

        default:
            EXPECT_TRUE(false) << "Unhandled ProfileType enum value";
            return BeFileName();
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
ProfileVersion Profile::ReadProfileVersion(Db const& db) const
    {
    Utf8String profileVersionJson;
    if (BE_SQLITE_ROW != db.QueryProperty(profileVersionJson, m_versionPropertySpec) || profileVersionJson.empty())
        return ProfileVersion(0, 0, 0, 0);

    return ProfileVersion(profileVersionJson.c_str());
    }


//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
ECDbProfile const* ECDbProfile::s_singleton = nullptr;

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus ECDbProfile::_Init() const
    {
    BeAssert(ECDb::IsInitialized());
    ECDb db;
    if (BE_SQLITE_OK != db.CreateNewDb(BEDB_MemoryDb))
        return ERROR;

    m_expectedVersion = ReadProfileVersion(db);
    return !m_expectedVersion.IsEmpty() ? SUCCESS : ERROR;
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
BeDbProfile const* BeDbProfile::s_singleton = nullptr;

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus BeDbProfile::_Init() const
    {
    Db db;
    if (BE_SQLITE_OK != db.CreateNewDb(BEDB_MemoryDb))
        return ERROR;

    m_expectedVersion = ReadProfileVersion(db);
    return !m_expectedVersion.IsEmpty() ? SUCCESS : ERROR;
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
DgnDbProfile const* DgnDbProfile::s_singleton = nullptr;

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus DgnDbProfile::_Init() const
    {
    BeAssert(DgnDb::IsInitialized());
    BeFileName temporaryDir;
    BeTest::GetHost().GetTempDir(temporaryDir);
    temporaryDir.AppendUtf8("temp");

    CreateDgnDbParams temp("ProfileVersion");
    DgnDbPtr db = DgnDb::CreateDgnDb(nullptr, temporaryDir, temp);
    if (db == nullptr)
        return ERROR;

    m_expectedVersion = ReadProfileVersion(*db);
    return !m_expectedVersion.IsEmpty() ? SUCCESS : ERROR;
    }

//=====================================TestFile====================================
//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TestFile::TestFile(Utf8StringCR name, BeFileName const& path, BeFileName const& seedFilePath, ProfileVersion const& bedbVersion, ProfileVersion const& ecdbVersion, ProfileVersion const& dgndbVersion,
                   ProfileVersion const& initialBeDbVersion, ProfileVersion const& initialECDbVersion, ProfileVersion const& initialDgnDbVersion) : 
    m_name(name), m_path(path), m_seedPath(seedFilePath), m_bedbVersion(bedbVersion), m_ecdbVersion(ecdbVersion), m_dgndbVersion(dgndbVersion),
    m_initialBeDbVersion(initialBeDbVersion), m_initialECDbVersion(initialECDbVersion), m_initialDgnDbVersion(initialDgnDbVersion) 
    {}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
BeFileNameStatus TestFile::CloneSeed(BeFileNameCR targetPath) const
    {
    BeFileName targetFolder = targetPath.GetDirectoryName();
    if (!targetFolder.DoesPathExist())
        {
        BeFileNameStatus stat = BeFileName::CreateNewDirectory(targetFolder);
        if (BeFileNameStatus::Success != stat)
            return stat;
        }

    if (targetPath.DoesPathExist())
        {
        BeFileNameStatus stat = targetPath.BeDeleteFile();
        if (BeFileNameStatus::Success != stat)
            return stat;
        }

    return BeFileName::BeCopyFile(m_seedPath, targetPath, true);
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
ProfileState::Age TestFile::GetAge() const
    {
    int comp = 0;
    if (!m_dgndbVersion.IsEmpty())
        comp = m_dgndbVersion.CompareTo(DgnDbProfile::Get().GetExpectedVersion());

    if (comp == 0 && !m_ecdbVersion.IsEmpty())
        comp = m_ecdbVersion.CompareTo(ECDbProfile::Get().GetExpectedVersion());

    if (comp == 0)
        comp = m_bedbVersion.CompareTo(BeDbProfile::Get().GetExpectedVersion());

    if (comp == 0)
        return ProfileState::Age::UpToDate;

    return comp > 0 ? ProfileState::Age::Newer : ProfileState::Age::Older;
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
ProfileState::Age TestFile::GetECDbAge() const
    {
    BeAssert(!m_ecdbVersion.IsEmpty());
    int comp = m_ecdbVersion.CompareTo(ECDbProfile::Get().GetExpectedVersion());

    if (comp == 0)
        comp = m_bedbVersion.CompareTo(BeDbProfile::Get().GetExpectedVersion());

    if (comp == 0)
        return ProfileState::Age::UpToDate;

    return comp > 0 ? ProfileState::Age::Newer : ProfileState::Age::Older;
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
Utf8String TestFile::ToString() const
    {
    Utf8String versionString;
    if (!m_dgndbVersion.IsEmpty())
        versionString.Sprintf(PROFILE_NAME_DGNDB " %s, " PROFILE_NAME_ECDB " %s, " PROFILE_NAME_BEDB " %s", m_dgndbVersion.ToString().c_str(), m_ecdbVersion.ToString().c_str(), m_bedbVersion.ToString().c_str());
    else if (!m_ecdbVersion.IsEmpty())
        versionString.Sprintf(PROFILE_NAME_ECDB " %s, " PROFILE_NAME_BEDB " %s", m_ecdbVersion.ToString().c_str(), m_bedbVersion.ToString().c_str());
    else
        versionString.Sprintf(PROFILE_NAME_BEDB " %s", m_bedbVersion.ToString().c_str());

    return Utf8PrintfString("%s | Pre-Test Version: %s, upgraded: %s | %s | Seed: %s ", m_name.c_str(), versionString.c_str(), IsUpgraded() ? "yes" : "no", m_path.GetNameUtf8().c_str(), m_seedPath.GetNameUtf8().c_str());
    }
